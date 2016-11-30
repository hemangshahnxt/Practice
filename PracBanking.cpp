// PracBanking.cpp : implementation file
//

#include "stdafx.h"
#include "PracBanking.h"
#include "GlobalReportUtils.h"
#include "GlobalFinancialUtils.h"
#include "ReportInfo.h"
#include "Reports.h"
#include "AuditTrail.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "RetrievePastDepositsDlg.h"
#include "QBDepositAccountsDlg.h"
#include "PrepareRefundChecksDlg.h"
#include "FinancialRc.h"
#include "DontShowDlg.h"
#include "MultiSelectDlg.h"

// (a.walling 2012-08-03 14:31) - PLID 51956 - Compiler limits exceeded with imported NxAccessor typelib - get quickbooks out of stdafx
// (a.walling 2014-04-30 15:19) - PLID 61989 - import a typelibrary rather than a dll
#import "QBFC3.tlb" no_namespace

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37025 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.

// (r.gonet 2010-09-13 12:40) - PLID 37458 - ID for the {Multiple Categories} payment category filter option
#define MULTIPLE_CATEGORIES -2

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace ADODB;
using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// PracBanking dialog

// (a.walling 2008-05-28 14:01) - PLID 27591 - Use CDateTimePicker

// (j.jones 2008-05-08 11:48) - PLID 25338 - enumerated all payment columns
enum CashPaymentListColumns {

	cashplcID = 0,
	cashplcPatientID, // (j.jones 2011-06-24 12:49) - PLID 22833 - added PatientID
	cashplcPatientName,
	cashplcAmount,
	cashplcIsTip,
	cashplcDate,
	cashplcInputDate,
	cashplcProviderID,
	cashplcLocationID,
	cashplcCategoryID,
	cashplcRespID,
	// (j.jones 2011-06-16 13:31) - PLID 42038 - added UserName
	cashplcUserName,

	//If you add a new column, update
	//UpdateVisibleCashColumns to support it
};

enum CheckPaymentListColumns {

	checkplcID = 0,	
	checkplcPatientID, // (j.jones 2011-06-24 12:49) - PLID 22833 - added PatientID
	checkplcCheckNumber,
	checkplcPatientName,
	checkplcAmount,
	checkplcIsBatchPay,
	checkplcIsTip,
	checkplcDate,
	checkplcInputDate,
	checkplcProviderID,
	checkplcLocationID,
	checkplcCategoryID,
	checkplcRespID,
	// (j.jones 2011-06-16 13:31) - PLID 42038 - added UserName
	checkplcUserName,

	//If you add a new column, update
	//UpdateVisibleCheckColumns to support it
};

enum CreditPaymentListColumns {

	creditplcID = 0,	
	creditplcPatientID, // (j.jones 2011-06-24 12:49) - PLID 22833 - added PatientID
	creditplcCardName,
	creditplcLast4,
	creditplcPatientName,
	creditplcAmount,
	creditplcIsTip,
	creditplcCardID,
	creditplcDate,
	creditplcInputDate,
	creditplcProviderID,
	creditplcLocationID,
	creditplcCategoryID,
	creditplcRespID,
	// (j.jones 2011-06-16 13:31) - PLID 42038 - added UserName
	creditplcUserName,

	//If you add a new column, update
	//UpdateVisibleCreditColumns to support it
};

// (j.jones 2011-06-16 13:11) - PLID 42038 - added user combo
enum UserComboColumns {

	uccID = 0,
	uccUsername,
};

PracBanking::PracBanking(CWnd* pParent)
	: CNxDialog(PracBanking::IDD, pParent), 
	m_ProviderChecker(NetUtils::Providers),
	m_LocationChecker(NetUtils::LocationsT),
	m_PaymentGroupChecker(NetUtils::PaymentGroupsT),
	m_DepositedPaymentsChecker(NetUtils::DepositedPayments),
	m_UserChecker(NetUtils::Coordinators)
{
	//{{AFX_DATA_INIT(PracBanking)
	//}}AFX_DATA_INIT

	//(j.anspach 06-09-2005 10:26 PLID 16662) - Updating the help files to incorporate the new help .chm
	//PLID 21512: per Don, if we don't have anything to put here, default to the earliest thing we can which is new patient
	//m_strManualLocation = "NexTech_Practice_Manual.chm";
	//m_strManualBookmark = "Billing/banking.htm";

	m_bRememberColumns = false;
}


void PracBanking::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(PracBanking)
	DDX_Control(pDX, IDC_INCLUDE_REFUNDS_CHECK, m_btnIncludeRefunds);
	DDX_Control(pDX, IDC_INCLUDE_TIPS_CHECK, m_btnIncludeTips);
	DDX_Control(pDX, IDC_PREPARE_REFUNDS, m_btnPrepareRefunds);
	DDX_Control(pDX, IDC_RESTORE_PAST_DEPOSITS, m_btnRestoreDeposits);
	DDX_Control(pDX, IDC_CLEAR, m_btnDepositItems);
	DDX_Control(pDX, IDC_PRINT, m_btnPrintDeposit);
	DDX_Control(pDX, IDC_CREDITUNSELECTONE, m_btnCreditRemoveOne);
	DDX_Control(pDX, IDC_CREDITSELECTONE, m_btnCreditSelectOne);
	DDX_Control(pDX, IDC_CREDITUNSELECTALL, m_btnCreditRemoveAll);
	DDX_Control(pDX, IDC_CREDITSELECTALL, m_btnCreditSelectAll);
	DDX_Control(pDX, IDC_CHECKUNSELECTONE, m_btnCheckRemoveOne);
	DDX_Control(pDX, IDC_CHECKUNSELECTALL, m_btnCheckRemoveAll);
	DDX_Control(pDX, IDC_CHECKSELECTONE, m_btnCheckSelectOne);
	DDX_Control(pDX, IDC_CHECKSELECTALL, m_btnCheckSelectAll);
	DDX_Control(pDX, IDC_CASHUNSELECTONE, m_btnCashRemoveOne);
	DDX_Control(pDX, IDC_CASHUNSELECTALL, m_btnCashRemoveAll);
	DDX_Control(pDX, IDC_CASHSELECTONE, m_btnCashSelectOne);
	DDX_Control(pDX, IDC_CASHSELECTALL, m_btnCashSelectAll);
	DDX_Control(pDX, IDC_ALL_PAY_CATS, m_AllPayCats);
	DDX_Control(pDX, IDC_SELECT_PAYCAT_RADIO, m_rSinglePayCat);
	DDX_Control(pDX, IDC_SEND_TO_QUICKBOOKS, m_btnSendToQBooks);
	DDX_Control(pDX, IDC_SELECT_LOCATION_RADIO, m_rSingleLocation);
	DDX_Control(pDX, IDC_ALL_LOCATIONS, m_AllLocations);
	DDX_Control(pDX, IDC_RADIO_INPUT_DATE, m_radioInputDate);
	DDX_Control(pDX, IDC_RADIO_SERVICE_DATE, m_radioServiceDate);
	DDX_Control(pDX, IDC_ALLDATES, m_AllDates);
	DDX_Control(pDX, IDC_DATERANGE, m_DateRange);
	DDX_Control(pDX, IDC_ALLPROVIDERS, m_rAllProvider);
	DDX_Control(pDX, IDC_SELECT_PROVIDER_RADIO, m_rSingleProvider);
	DDX_Control(pDX, IDC_FROM, m_from);
	DDX_Control(pDX, IDC_TO, m_to);
	DDX_Control(pDX, IDC_DEPOSIT_DATE, m_dtDeposit);
	DDX_Control(pDX, IDC_CASH_AVAIL, m_nxeditCashAvail);
	DDX_Control(pDX, IDC_CASH_SEL, m_nxeditCashSel);
	DDX_Control(pDX, IDC_CHECK_AVAIL, m_nxeditCheckAvail);
	DDX_Control(pDX, IDC_CHECK_SEL, m_nxeditCheckSel);
	DDX_Control(pDX, IDC_CREDIT_AVAIL, m_nxeditCreditAvail);
	DDX_Control(pDX, IDC_CREDIT_SEL, m_nxeditCreditSel);
	DDX_Control(pDX, IDC_TOTAL_AVAIL, m_nxeditTotalAvail);
	DDX_Control(pDX, IDC_TOTAL_SEL, m_nxeditTotalSel);
	DDX_Control(pDX, IDC_CHECK_SHOW_PMT_DATES, m_checkShowPmtDates);
	DDX_Control(pDX, IDC_CHECK_SHOW_INPUT_DATES, m_checkShowInputDates);
	// (r.gonet 09-13-2010 12:40) - PLID 37458 - Label for a multi selection of payment categories
	DDX_Control(pDX, IDC_BANKING_PMNT_MULTI_CATS_LBL, m_nxlMultiPaymentCategories);
	// (j.jones 2011-06-16 11:36) - PLID 42038 - added filter for input user
	DDX_Control(pDX, IDC_ALLUSERS, m_rAllUsers);
	DDX_Control(pDX, IDC_SELECT_USER_RADIO, m_rSingleUser);
	DDX_Control(pDX, IDC_CHECK_REMEMBER_BANKING_COL_WIDTHS, m_checkRememberColWidths);
	//}}AFX_DATA_MAP
}

//	ON_EVENT(PracBanking, IDC_FROM, 2 /* Change */, OnChangeFrom, VTS_NONE)
//	ON_EVENT(PracBanking, IDC_TO, 2 /* Change */, OnChangeTo, VTS_NONE)	

BEGIN_MESSAGE_MAP(PracBanking, CNxDialog)
	//{{AFX_MSG_MAP(PracBanking)
	ON_WM_CTLCOLOR()
	ON_WM_PAINT()
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick)
	ON_WM_SETCURSOR()
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_FROM, OnChangeFrom)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_TO, OnChangeTo)
	ON_BN_CLICKED(IDC_CASHSELECTALL, OnCashSelectAll)
	ON_BN_CLICKED(IDC_CASHSELECTONE, OnCashSelectOne)
	ON_BN_CLICKED(IDC_CASHUNSELECTONE, OnCashUnSelectOne)
	ON_BN_CLICKED(IDC_CASHUNSELECTALL, OnCashUnSelectAll)
	ON_BN_CLICKED(IDC_CHECKSELECTALL, OnCheckSelectAll)
	ON_BN_CLICKED(IDC_CHECKSELECTONE, OnCheckSelectOne)
	ON_BN_CLICKED(IDC_CHECKUNSELECTALL, OnCheckUnSelectAll)
	ON_BN_CLICKED(IDC_CREDITSELECTALL, OnCreditSelectAll)
	ON_BN_CLICKED(IDC_CREDITSELECTONE, OnCreditSelectOne)
	ON_BN_CLICKED(IDC_CREDITUNSELECTALL, OnCreditUnSelectAll)
	ON_BN_CLICKED(IDC_CREDITUNSELECTONE, OnCreditUnSelectOne)
	ON_BN_CLICKED(IDC_PRINT, Print)
	ON_BN_CLICKED(IDC_CHECKUNSELECTONE, OnCheckUnSelectOne)
	ON_BN_CLICKED(IDC_CLEAR, OnBtnDeposit)
	ON_BN_CLICKED(IDC_ALLDATES, OnAlldates)
	ON_BN_CLICKED(IDC_ALLPROVIDERS, OnAllproviders)
	ON_BN_CLICKED(IDC_DATERANGE, OnDaterange)
	ON_BN_CLICKED(IDC_SELECT_PROVIDER_RADIO, OnSelectProviderRadio)
	ON_BN_CLICKED(IDC_RADIO_INPUT_DATE, OnRadioInputDate)
	ON_BN_CLICKED(IDC_RADIO_SERVICE_DATE, OnRadioServiceDate)
	ON_BN_CLICKED(IDC_ALL_LOCATIONS, OnAlllocations)
	ON_BN_CLICKED(IDC_SELECT_LOCATION_RADIO, OnSelectLocationRadio)
	ON_BN_CLICKED(IDC_SEND_TO_QUICKBOOKS, OnSendToQuickbooks)
	ON_BN_CLICKED(IDC_ALL_PAY_CATS, OnAllPayCats)
	ON_BN_CLICKED(IDC_SELECT_PAYCAT_RADIO, OnSelectPaycatRadio)
	ON_BN_CLICKED(IDC_RESTORE_PAST_DEPOSITS, OnRestorePastDeposits)
	ON_BN_CLICKED(IDC_INCLUDE_TIPS_CHECK, OnIncludeTipsCheck)
	ON_BN_CLICKED(IDC_PREPARE_REFUNDS, OnPrepareRefunds)
	ON_BN_CLICKED(IDC_INCLUDE_REFUNDS_CHECK, OnIncludeRefundsCheck)
	ON_MESSAGE(WM_TABLE_CHANGED, OnTableChanged)
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_FILE_PRINT, CNxTabView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CNxTabView::OnFilePrint)
	ON_BN_CLICKED(IDC_CHECK_SHOW_PMT_DATES, OnCheckShowPmtDates)
	ON_BN_CLICKED(IDC_CHECK_SHOW_INPUT_DATES, OnCheckShowInputDates)
	ON_BN_CLICKED(IDC_ALLUSERS, OnBnClickedAllusers)
	ON_BN_CLICKED(IDC_SELECT_USER_RADIO, OnBnClickedSelectUserRadio)
	ON_BN_CLICKED(IDC_CHECK_REMEMBER_BANKING_COL_WIDTHS, OnCheckRememberBankingColWidths)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// PracBanking message handlers

BOOL PracBanking::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	// (c.haag 2009-08-31 12:29) - PLID 13175 - Bulk ConfigRT caching
	g_propManager.CachePropertiesInBulk("PracBanking_1", propNumber,
		"(Username = '<None>' OR Username = '%s') AND ("
		"Name = 'BankingIncludeTips' OR "
		"Name = 'BankingShowPaymentDates' OR "
		"Name = 'BankingShowInputDates' OR "
		"Name = 'BankingIncludeRefunds' "
		"OR Name = 'BankingRememberColumnWidths' "// (j.jones 2013-06-21 10:51) - PLID 35059
		")",
		_Q(GetCurrentUserName()));

	// (j.jones 2013-06-21 10:51) - PLID 35059 - added text cache
	g_propManager.CachePropertiesInBulk("PracBanking_2", propText,
		"(Username = '<None>' OR Username = '%s') AND ("
		"Name = 'BankingColumnWidthsCashAvail' "
		"OR Name = 'BankingColumnWidthsCashSelected' "
		"OR Name = 'BankingColumnWidthsCheckAvail' "
		"OR Name = 'BankingColumnWidthsCheckSelected' "
		"OR Name = 'BankingColumnWidthsCreditAvail' "
		"OR Name = 'BankingColumnWidthsCreditSelected' "
		")",
		_Q(GetCurrentUserName()));

	m_btnCreditRemoveOne.AutoSet(NXB_LEFT);
	m_btnCreditSelectOne.AutoSet(NXB_RIGHT);
	m_btnCreditRemoveAll.AutoSet(NXB_LLEFT);
	m_btnCreditSelectAll.AutoSet(NXB_RRIGHT);
	m_btnCheckRemoveOne.AutoSet(NXB_LEFT);
	m_btnCheckRemoveAll.AutoSet(NXB_LLEFT);
	m_btnCheckSelectOne.AutoSet(NXB_RIGHT);
	m_btnCheckSelectAll.AutoSet(NXB_RRIGHT);
	m_btnCashRemoveOne.AutoSet(NXB_LEFT);
	m_btnCashRemoveAll.AutoSet(NXB_LLEFT);
	m_btnCashSelectOne.AutoSet(NXB_RIGHT);
	m_btnCashSelectAll.AutoSet(NXB_RRIGHT);

	// (j.jones 2008-05-08 10:21) - PLID 29953 - set more button styles for modernization
	m_btnPrepareRefunds.AutoSet(NXB_MODIFY);
	m_btnRestoreDeposits.AutoSet(NXB_MODIFY);
	m_btnDepositItems.AutoSet(NXB_MODIFY);
	m_btnPrintDeposit.AutoSet(NXB_PRINT);
	m_btnSendToQBooks.AutoSet(NXB_EXPORT);

	m_CashAvailList = BindNxDataListCtrl(IDC_CASHAVAILLIST,false);
	m_CashSelectedList = BindNxDataListCtrl(IDC_CASHSELECTEDLIST,false);
	m_CheckAvailList = BindNxDataListCtrl(IDC_CHECKAVAILLIST,false);
	m_CheckSelectedList = BindNxDataListCtrl(IDC_CHECKSELECTEDLIST,false);
	m_CreditAvailList = BindNxDataListCtrl(IDC_CREDITAVAILLIST,false);
	m_CreditSelectedList = BindNxDataListCtrl(IDC_CREDITSELECTEDLIST,false);
	m_ProvSelect = BindNxDataListCtrl(IDC_PROVSELECTOR);
	m_LocSelect = BindNxDataListCtrl(IDC_LOCSELECTOR);
	m_CategoryCombo = BindNxDataListCtrl(IDC_COMBO_CATEGORY);

	// (j.gruber 2010-10-22 16:08) - PLID 37457 - Resp filter
	m_pRespFilter = BindNxDataList2Ctrl(IDC_RESP_FILTER_LIST, TRUE);
	// (j.jones 2011-06-16 11:36) - PLID 42038 - added filter for input user
	m_UserSelect = BindNxDataList2Ctrl(IDC_COMBO_USER_FILTER, true);
	
	// (r.gonet 2010-09-13 12:40) - PLID 37458 - For label notification
	m_bNotifyOnce = TRUE;

	if(IsSpa(TRUE)) {
		CButton* pBtn = (CButton*)GetDlgItem(IDC_INCLUDE_TIPS_CHECK);
		pBtn->EnableWindow(TRUE);
		pBtn->ShowWindow(SW_SHOW);

		//remember their setting on whether this button is checked or not
		CheckDlgButton(IDC_INCLUDE_TIPS_CHECK, GetRemotePropertyInt("BankingIncludeTips", 0, 0, GetCurrentUserName(), true));
	}
	else {
		//no license for nexspa, disable this all
		CButton* pBtn = (CButton*)GetDlgItem(IDC_INCLUDE_TIPS_CHECK);
		pBtn->EnableWindow(FALSE);
		pBtn->ShowWindow(SW_HIDE);
	}

	// (j.jones 2013-06-21 10:51) - PLID 35059 - added ability to remember column widths
	m_bRememberColumns = (GetRemotePropertyInt("BankingRememberColumnWidths", 0, 0, GetCurrentUserName(), true) == 1 ? true : false);
	m_checkRememberColWidths.SetCheck(m_bRememberColumns);

	//PLID 13548: remember the setting for including refunds
	CheckDlgButton(IDC_INCLUDE_REFUNDS_CHECK, GetRemotePropertyInt("BankingIncludeRefunds", 1, 0, GetCurrentUserName(), true));

	// (c.haag 2009-08-31 12:08) - PLID 13175 - Show or hide payment and input date columns
	CheckDlgButton(IDC_CHECK_SHOW_PMT_DATES, GetRemotePropertyInt("BankingShowPaymentDates", 1, 0, GetCurrentUserName(), true));
	CheckDlgButton(IDC_CHECK_SHOW_INPUT_DATES, GetRemotePropertyInt("BankingShowInputDates", 1, 0, GetCurrentUserName(), true));
	UpdateVisibleColumns();

	IRowSettingsPtr pRow = m_ProvSelect->GetRow(-1);
	pRow->PutValue(0,(long)-1);
	pRow->PutValue(1,_bstr_t(" {No Provider}"));
	m_ProvSelect->InsertRow(pRow, 0);

	// (r.gonet 2010-09-13 12:40) - PLID 37458 - Add a multiple categories option to the categories combo box
	pRow = m_CategoryCombo->GetRow(-1);
	pRow->PutValue(0, (long)MULTIPLE_CATEGORIES);
	pRow->PutValue(1, _bstr_t(" {Multiple Categories}"));
	m_CategoryCombo->InsertRow(pRow, 0);
	// Insert a no payment category filter option
	pRow = m_CategoryCombo->GetRow(-1);
	pRow->PutValue(0,(long)0);
	pRow->PutValue(1,_bstr_t(" {No Category}"));
	m_CategoryCombo->InsertRow(pRow, 0);

	m_bInitPaint = true;

	m_from.SetValue(COleVariant(COleDateTime::GetCurrentTime()));
	m_to.SetValue(COleVariant(COleDateTime::GetCurrentTime()));
	m_dtDeposit.SetValue(COleVariant(COleDateTime::GetCurrentTime()));

	m_bAllSelected = false;
	
	m_brush.CreateSolidBrush(PaletteColor(GetNxColor(GNC_FINANCIAL, 0)));

	m_AllDates.SetCheck(FALSE);
	m_DateRange.SetCheck(TRUE);
	m_from.EnableWindow(true);
	m_to.EnableWindow(true);
	m_from.SetFocus();

	m_radioInputDate.SetCheck(TRUE);
	m_radioServiceDate.SetCheck(FALSE);	

	m_rSingleProvider.SetCheck(FALSE);
	m_rAllProvider.SetCheck(TRUE);
	m_ProvSelect->CurSel = 0; //set to first doctor, so there isn't a blank space there
	GetDlgItem(IDC_PROVSELECTOR)->EnableWindow(FALSE);

	m_rSingleLocation.SetCheck(FALSE);
	m_AllLocations.SetCheck(TRUE);
	m_LocSelect->CurSel = 0; //set to first location, so there isn't a blank space there
	GetDlgItem(IDC_LOCSELECTOR)->EnableWindow(FALSE);

	m_AllPayCats.SetCheck(TRUE);
	m_rSinglePayCat.SetCheck(FALSE);
	m_CategoryCombo->CurSel = 0;
	GetDlgItem(IDC_COMBO_CATEGORY)->EnableWindow(FALSE);

	// (r.gonet 2010-09-13 12:40) - PLID 37458 - Setup the label for containing the multiple payment categories
	m_nxlMultiPaymentCategories.SetText("");
	m_nxlMultiPaymentCategories.SetType(dtsHyperlink);
	UpdateCategoriesFilterControls();

	// (j.jones 2011-06-16 11:36) - PLID 42038 - added filter for input user
	m_rAllUsers.SetCheck(TRUE);
	m_rSingleUser.SetCheck(FALSE);
	m_UserSelect->SetSelByColumn(uccID, GetCurrentUserID());
	GetDlgItem(IDC_COMBO_USER_FILTER)->EnableWindow(FALSE);
			
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void PracBanking::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	if(m_bInitPaint)
	{
		m_bInitPaint = false;
	}
		
	// Do not call CNxDialog::OnPaint() for painting messages
}

void PracBanking::OnCashSelectAll() 
{
	try {

		// (j.jones 2008-05-08 16:37) - PLID 25338 - removed CurrentSelect from data,
		// all this work is done strictly through the datalists now

		m_CashSelectedList->TakeAllRows(m_CashAvailList);

		// (j.jones 2008-05-08 14:21) - PLID 25388 - split the old UpdateTotals() function into
		// UpdateAvailTotals() and UpdateSelectedTotals()
		UpdateAvailTotals();
		UpdateSelectedTotals();

	} NxCatchAll("Error in PracBanking::OnCashSelectAll()");
}

void PracBanking::OnCashUnSelectAll() 
{
	try {

		// (j.jones 2008-05-08 16:37) - PLID 25338 - removed CurrentSelect from data,
		// all this work is done strictly through the datalists now
				
		m_CashAvailList->TakeAllRows(m_CashSelectedList);

		// (j.jones 2008-05-08 14:21) - PLID 25388 - split the old UpdateTotals() function into
		// UpdateAvailTotals() and UpdateSelectedTotals()
		UpdateAvailTotals();
		UpdateSelectedTotals();

	} NxCatchAll("Error in PracBanking::OnCashUnSelectAll()");
}

void PracBanking::OnCashSelectOne() 
{
	try {

		// (j.jones 2008-05-08 16:37) - PLID 25338 - removed CurrentSelect from data,
		// all this work is done strictly through the datalists now

		if(m_CashAvailList->GetCurSel() != -1) {

			m_CashSelectedList->TakeCurrentRow(m_CashAvailList);

			// (j.jones 2008-05-08 14:21) - PLID 25388 - split the old UpdateTotals() function into
			// UpdateAvailTotals() and UpdateSelectedTotals()
			UpdateAvailTotals();
			UpdateSelectedTotals();
		}

	} NxCatchAll("Error in PracBanking::OnCashSelectOne()");
}

void PracBanking::OnCashUnSelectOne() 
{
	try {

		// (j.jones 2008-05-08 16:37) - PLID 25338 - removed CurrentSelect from data,
		// all this work is done strictly through the datalists now

		if(m_CashSelectedList->GetCurSel() != -1) {

			m_CashAvailList->TakeCurrentRow(m_CashSelectedList);

			// (j.jones 2008-05-08 14:21) - PLID 25388 - split the old UpdateTotals() function into
			// UpdateAvailTotals() and UpdateSelectedTotals()
			UpdateAvailTotals();
			UpdateSelectedTotals();
		}

	} NxCatchAll("Error in PracBanking::OnCashUnSelectOne()");
}

void PracBanking::OnCheckSelectAll() 
{
	try {

		// (j.jones 2008-05-08 16:37) - PLID 25338 - removed CurrentSelect from data,
		// all this work is done strictly through the datalists now

		m_CheckSelectedList->TakeAllRows(m_CheckAvailList);

		// (j.jones 2008-05-08 14:21) - PLID 25388 - split the old UpdateTotals() function into
		// UpdateAvailTotals() and UpdateSelectedTotals()
		UpdateAvailTotals();
		UpdateSelectedTotals();

	} NxCatchAll("Error in PracBanking::OnCheckSelectAll()");
}

void PracBanking::OnCheckSelectOne() 
{
	try {

		// (j.jones 2008-05-08 16:37) - PLID 25338 - removed CurrentSelect from data,
		// all this work is done strictly through the datalists now

		if(m_CheckAvailList->GetCurSel() != -1) {

			m_CheckSelectedList->TakeCurrentRow(m_CheckAvailList);

			// (j.jones 2008-05-08 14:21) - PLID 25388 - split the old UpdateTotals() function into
			// UpdateAvailTotals() and UpdateSelectedTotals()
			UpdateAvailTotals();
			UpdateSelectedTotals();
		}

	} NxCatchAll("Error in PracBanking::OnCheckSelectOne()");
}

void PracBanking::OnCheckUnSelectOne() 
{
	try {

		// (j.jones 2008-05-08 16:37) - PLID 25338 - removed CurrentSelect from data,
		// all this work is done strictly through the datalists now
		
		if(m_CheckSelectedList->GetCurSel() != -1) {

			m_CheckAvailList->TakeCurrentRow(m_CheckSelectedList);

			// (j.jones 2008-05-08 14:21) - PLID 25388 - split the old UpdateTotals() function into
			// UpdateAvailTotals() and UpdateSelectedTotals()
			UpdateAvailTotals();
			UpdateSelectedTotals();
		}

	} NxCatchAll("Error in PracBanking::OnCheckUnSelectOne()");
}

void PracBanking::OnCheckUnSelectAll() 
{
	try {

		// (j.jones 2008-05-08 16:37) - PLID 25338 - removed CurrentSelect from data,
		// all this work is done strictly through the datalists now
		
		m_CheckAvailList->TakeAllRows(m_CheckSelectedList);

		// (j.jones 2008-05-08 14:21) - PLID 25388 - split the old UpdateTotals() function into
		// UpdateAvailTotals() and UpdateSelectedTotals()
		UpdateAvailTotals();
		UpdateSelectedTotals();

	} NxCatchAll("Error in PracBanking::OnCheckUnSelectAll()");
}

void PracBanking::OnCreditSelectAll() 
{
	try {

		// (j.jones 2008-05-08 16:37) - PLID 25338 - removed CurrentSelect from data,
		// all this work is done strictly through the datalists now

		m_CreditSelectedList->TakeAllRows(m_CreditAvailList);

		// (j.jones 2008-05-08 14:21) - PLID 25388 - split the old UpdateTotals() function into
		// UpdateAvailTotals() and UpdateSelectedTotals()
		UpdateAvailTotals();
		UpdateSelectedTotals();

	} NxCatchAll("Error in PracBanking::OnCreditSelectAll()");
}

void PracBanking::OnCreditSelectOne() 
{
	try {

		// (j.jones 2008-05-08 16:37) - PLID 25338 - removed CurrentSelect from data,
		// all this work is done strictly through the datalists now
		
		if(m_CreditAvailList->GetCurSel() != -1) {

			m_CreditSelectedList->TakeCurrentRow(m_CreditAvailList);

			// (j.jones 2008-05-08 14:21) - PLID 25388 - split the old UpdateTotals() function into
			// UpdateAvailTotals() and UpdateSelectedTotals()
			UpdateAvailTotals();
			UpdateSelectedTotals();
		}

	} NxCatchAll("Error in PracBanking::OnCreditSelectOne()");
}

void PracBanking::OnCreditUnSelectAll() 
{
	try {

		// (j.jones 2008-05-08 16:37) - PLID 25338 - removed CurrentSelect from data,
		// all this work is done strictly through the datalists now

		m_CreditAvailList->TakeAllRows(m_CreditSelectedList);

		// (j.jones 2008-05-08 14:21) - PLID 25388 - split the old UpdateTotals() function into
		// UpdateAvailTotals() and UpdateSelectedTotals()
		UpdateAvailTotals();
		UpdateSelectedTotals();

	} NxCatchAll("Error in PracBanking::OnCreditUnSelectAll()");
}

void PracBanking::OnCreditUnSelectOne() 
{
	try {
		
		// (j.jones 2008-05-08 16:37) - PLID 25338 - removed CurrentSelect from data,
		// all this work is done strictly through the datalists now
		
		if(m_CreditSelectedList->GetCurSel() != -1) {

			m_CreditAvailList->TakeCurrentRow(m_CreditSelectedList);

			// (j.jones 2008-05-08 14:21) - PLID 25388 - split the old UpdateTotals() function into
			// UpdateAvailTotals() and UpdateSelectedTotals()
			UpdateAvailTotals();
			UpdateSelectedTotals();
		}

	} NxCatchAll("Error in PracBanking::OnCreditUnSelectOne()");
}

// (j.jones 2008-05-08 08:38) - PLID 25338 - removed RequeryAll and replaced it with
// RefreshLists() and RequeryAvailLists()
void PracBanking::RefreshLists()
{
	try {

		// (z.manning, 03/21/2007) - PLID 25294 - Clear the totals since we are going to be requerying all the lists.
		SetDlgItemText(IDC_CASH_AVAIL, "");
		SetDlgItemText(IDC_CHECK_AVAIL, "");
		SetDlgItemText(IDC_CREDIT_AVAIL, "");
		SetDlgItemText(IDC_TOTAL_AVAIL, "");
		SetDlgItemText(IDC_CASH_SEL, "");
		SetDlgItemText(IDC_CHECK_SEL, "");
		SetDlgItemText(IDC_CREDIT_SEL, "");
		SetDlgItemText(IDC_TOTAL_SEL, "");

		//store the currently selected & unselected values, so we can re-select them upon requery finished
		BOOL bAnythingCached = CacheSelectionStates();

		//now clear the selected lists
		m_CashSelectedList->Clear();
		m_CheckSelectedList->Clear();
		m_CreditSelectedList->Clear();

		//requery just the available lists, if anything was cached
		//we will force the requery to finish before the user can continue
		RequeryAvailLists(bAnythingCached);

		//when the requery completes, the selected values will be restored,
		//and the totals will be calculated

	} NxCatchAll("Error in PracBanking::RefreshLists");
}

// (j.jones 2008-05-08 14:35) - PLID 25338 - will populate the selected & unselected arrays with the current list values
// returns TRUE if we cached anything
BOOL PracBanking::CacheSelectionStates()
{
	BOOL bCachedSomething = FALSE;

	try {

		//ensure the arrays are empty
		ClearCachedValues();

		//loop through each selected list and populate the arrays

		{ //cash
			long nRow = m_CashSelectedList->GetFirstRowEnum();
			LPDISPATCH lpDisp = NULL;
			while(nRow) 
			{
				m_CashSelectedList->GetNextRowEnum(&nRow, &lpDisp);
				IRowSettingsPtr pRow(lpDisp); 
				lpDisp->Release();

				long nID = VarLong(pRow->GetValue(cashplcID));
				BOOL bIsTip = VarBool(pRow->GetValue(cashplcIsTip));

				if(bIsTip) {
					m_aryCashSelectedPaymentTipIDs.Add(nID);
				}
				else {
					m_aryCashSelectedPaymentIDs.Add(nID);
				}

				bCachedSomething = TRUE;
			}
		}

		{ //check
			long nRow = m_CheckSelectedList->GetFirstRowEnum();
			LPDISPATCH lpDisp = NULL;
			while(nRow) 
			{
				m_CheckSelectedList->GetNextRowEnum(&nRow, &lpDisp);
				IRowSettingsPtr pRow(lpDisp); 
				lpDisp->Release();

				long nID = VarLong(pRow->GetValue(checkplcID));
				BOOL bIsBatchPay = VarBool(pRow->GetValue(checkplcIsBatchPay));
				BOOL bIsTip = VarBool(pRow->GetValue(checkplcIsTip));

				if(bIsBatchPay) {
					m_aryCheckSelectedBatchPaymentIDs.Add(nID);
				}
				else if(bIsTip) {
					m_aryCheckSelectedPaymentTipIDs.Add(nID);
				}
				else {
					m_aryCheckSelectedPaymentIDs.Add(nID);
				}

				bCachedSomething = TRUE;
			}
		}

		{ //charge
			long nRow = m_CreditSelectedList->GetFirstRowEnum();
			LPDISPATCH lpDisp = NULL;
			while(nRow) 
			{
				m_CreditSelectedList->GetNextRowEnum(&nRow, &lpDisp);
				IRowSettingsPtr pRow(lpDisp); 
				lpDisp->Release();

				long nID = VarLong(pRow->GetValue(creditplcID));
				BOOL bIsTip = VarBool(pRow->GetValue(creditplcIsTip));

				if(bIsTip) {
					m_aryCreditSelectedPaymentTipIDs.Add(nID);
				}
				else {
					m_aryCreditSelectedPaymentIDs.Add(nID);
				}

				bCachedSomething = TRUE;
			}
		}

		//now loop through each unselected list and populate the arrays

		{ //cash
			long nRow = m_CashAvailList->GetFirstRowEnum();
			LPDISPATCH lpDisp = NULL;
			while(nRow) 
			{
				m_CashAvailList->GetNextRowEnum(&nRow, &lpDisp);
				IRowSettingsPtr pRow(lpDisp); 
				lpDisp->Release();

				long nID = VarLong(pRow->GetValue(cashplcID));
				BOOL bIsTip = VarBool(pRow->GetValue(cashplcIsTip));

				if(bIsTip) {
					m_aryCashUnselectedPaymentTipIDs.Add(nID);
				}
				else {
					m_aryCashUnselectedPaymentIDs.Add(nID);
				}

				bCachedSomething = TRUE;
			}
		}

		{ //check
			long nRow = m_CheckAvailList->GetFirstRowEnum();
			LPDISPATCH lpDisp = NULL;
			while(nRow) 
			{
				m_CheckAvailList->GetNextRowEnum(&nRow, &lpDisp);
				IRowSettingsPtr pRow(lpDisp); 
				lpDisp->Release();

				long nID = VarLong(pRow->GetValue(checkplcID));
				BOOL bIsBatchPay = VarBool(pRow->GetValue(checkplcIsBatchPay));
				BOOL bIsTip = VarBool(pRow->GetValue(checkplcIsTip));

				if(bIsBatchPay) {
					m_aryCheckUnselectedBatchPaymentIDs.Add(nID);
				}
				else if(bIsTip) {
					m_aryCheckUnselectedPaymentTipIDs.Add(nID);
				}
				else {
					m_aryCheckUnselectedPaymentIDs.Add(nID);
				}

				bCachedSomething = TRUE;
			}
		}

		{ //charge
			long nRow = m_CreditAvailList->GetFirstRowEnum();
			LPDISPATCH lpDisp = NULL;
			while(nRow) 
			{
				m_CreditAvailList->GetNextRowEnum(&nRow, &lpDisp);
				IRowSettingsPtr pRow(lpDisp); 
				lpDisp->Release();

				long nID = VarLong(pRow->GetValue(creditplcID));
				BOOL bIsTip = VarBool(pRow->GetValue(creditplcIsTip));

				if(bIsTip) {
					m_aryCreditUnselectedPaymentTipIDs.Add(nID);
				}
				else {
					m_aryCreditUnselectedPaymentIDs.Add(nID);
				}

				bCachedSomething = TRUE;
			}
		}

	} NxCatchAll("Error in PracBanking::CacheSelectionStates");

	return bCachedSomething;
}

// (j.jones 2008-05-08 14:51) - PLID 25338 - utility function used by RestoreCachedSelections(), will
// search a given array, return TRUE if it was found, and remove from the array if found
BOOL PracBanking::FindAndRemoveFromArray(CArray<long, long> &aryIDs, long nValue)
{
	for(int i=0; i<aryIDs.GetSize(); i++) {
		if((long)(aryIDs.GetAt(i)) == nValue) {
			//found it, remove it and return TRUE
			aryIDs.RemoveAt(i);
			return TRUE;
		}
	}

	return FALSE;
}

// (j.jones 2008-05-08 14:35) - PLID 25338 - will reset the selected & unselected payments to their proper lists
void PracBanking::RestoreCachedSelections()
{
	try {

		//first loop through each selected list, and move records to the
		//avail list if they are in one of the 'unselected' arrays
		//(do this first before we move more rows into the selected lists)

		{ //cash

			long nRow = m_CashSelectedList->GetFirstRowEnum();
			LPDISPATCH lpDisp = NULL;
			while(nRow &&
				(m_aryCashUnselectedPaymentIDs.GetSize() > 0
				|| m_aryCashUnselectedPaymentTipIDs.GetSize() > 0)) {

				m_CashSelectedList->GetNextRowEnum(&nRow, &lpDisp);
				IRowSettingsPtr pRow(lpDisp); 
				lpDisp->Release();

				long nID = VarLong(pRow->GetValue(cashplcID));
				BOOL bIsTip = VarBool(pRow->GetValue(cashplcIsTip));

				if(bIsTip) {					
					if(FindAndRemoveFromArray(m_aryCashUnselectedPaymentTipIDs, nID)) {
						//it was in our array, so move this record to the unselected list
						m_CashAvailList->TakeRow(pRow);
					}
				}
				else {
					if(FindAndRemoveFromArray(m_aryCashUnselectedPaymentIDs, nID)) {
						//it was in our array, so move this record to the unselected list
						m_CashAvailList->TakeRow(pRow);
					}
				}
			}

			//if anything is left in the lists, it means they are no longer available,
			//they could have been deleted or deposited, either way clear our lists
			m_aryCashUnselectedPaymentIDs.RemoveAll();
			m_aryCashUnselectedPaymentTipIDs.RemoveAll();
		}

		{ //check

			long nRow = m_CheckSelectedList->GetFirstRowEnum();
			LPDISPATCH lpDisp = NULL;
			while(nRow &&
				(m_aryCheckUnselectedPaymentIDs.GetSize() > 0
				|| m_aryCheckUnselectedBatchPaymentIDs.GetSize() > 0
				|| m_aryCheckUnselectedPaymentTipIDs.GetSize() > 0)) {

				m_CheckSelectedList->GetNextRowEnum(&nRow, &lpDisp);
				IRowSettingsPtr pRow(lpDisp); 
				lpDisp->Release();

				long nID = VarLong(pRow->GetValue(checkplcID));
				BOOL bIsBatchPay = VarBool(pRow->GetValue(checkplcIsBatchPay));
				BOOL bIsTip = VarBool(pRow->GetValue(checkplcIsTip));

				if(bIsBatchPay) {
					if(FindAndRemoveFromArray(m_aryCheckUnselectedBatchPaymentIDs, nID)) {
						//it was in our array, so move this record to the unselected list
						m_CheckAvailList->TakeRow(pRow);
					}

				}
				else if(bIsTip) {					
					if(FindAndRemoveFromArray(m_aryCheckUnselectedPaymentTipIDs, nID)) {
						//it was in our array, so move this record to the unselected list
						m_CheckAvailList->TakeRow(pRow);
					}
				}
				else {
					if(FindAndRemoveFromArray(m_aryCheckUnselectedPaymentIDs, nID)) {
						//it was in our array, so move this record to the unselected list
						m_CheckAvailList->TakeRow(pRow);
					}
				}
			}

			//if anything is left in the lists, it means they are no longer available,
			//they could have been deleted or deposited, either way clear our lists
			m_aryCheckUnselectedPaymentIDs.RemoveAll();
			m_aryCheckUnselectedBatchPaymentIDs.RemoveAll();
			m_aryCheckUnselectedPaymentTipIDs.RemoveAll();
		}

		{ //credit

			long nRow = m_CreditSelectedList->GetFirstRowEnum();
			LPDISPATCH lpDisp = NULL;
			while(nRow &&
				(m_aryCreditUnselectedPaymentIDs.GetSize() > 0
				|| m_aryCreditUnselectedPaymentTipIDs.GetSize() > 0)) {

				m_CreditSelectedList->GetNextRowEnum(&nRow, &lpDisp);
				IRowSettingsPtr pRow(lpDisp); 
				lpDisp->Release();

				long nID = VarLong(pRow->GetValue(creditplcID));
				BOOL bIsTip = VarBool(pRow->GetValue(creditplcIsTip));

				if(bIsTip) {					
					if(FindAndRemoveFromArray(m_aryCreditUnselectedPaymentTipIDs, nID)) {
						//it was in our array, so move this record to the unselected list
						m_CreditAvailList->TakeRow(pRow);
					}
				}
				else {
					if(FindAndRemoveFromArray(m_aryCreditUnselectedPaymentIDs, nID)) {
						//it was in our array, so move this record to the unselected list
						m_CreditAvailList->TakeRow(pRow);
					}
				}
			}

			//if anything is left in the lists, it means they are no longer available,
			//they could have been deleted or deposited, either way clear our lists
			m_aryCreditUnselectedPaymentIDs.RemoveAll();
			m_aryCreditUnselectedPaymentTipIDs.RemoveAll();
		}

		//now loop through each avail list, and move records to the select list
		//if they are in one of our 'selected' arrays

		{ //cash

			long nRow = m_CashAvailList->GetFirstRowEnum();
			LPDISPATCH lpDisp = NULL;
			while(nRow &&
				(m_aryCashSelectedPaymentIDs.GetSize() > 0
				|| m_aryCashSelectedPaymentTipIDs.GetSize() > 0)) {

				m_CashAvailList->GetNextRowEnum(&nRow, &lpDisp);
				IRowSettingsPtr pRow(lpDisp); 
				lpDisp->Release();

				long nID = VarLong(pRow->GetValue(cashplcID));
				BOOL bIsTip = VarBool(pRow->GetValue(cashplcIsTip));

				if(bIsTip) {					
					if(FindAndRemoveFromArray(m_aryCashSelectedPaymentTipIDs, nID)) {
						//it was in our array, so move this record to the selected list
						m_CashSelectedList->TakeRow(pRow);
					}
				}
				else {
					if(FindAndRemoveFromArray(m_aryCashSelectedPaymentIDs, nID)) {
						//it was in our array, so move this record to the selected list
						m_CashSelectedList->TakeRow(pRow);
					}
				}
			}

			//if anything is left in the lists, it means they are no longer available,
			//they could have been deleted or deposited, either way clear our lists
			m_aryCashSelectedPaymentIDs.RemoveAll();
			m_aryCashSelectedPaymentTipIDs.RemoveAll();
		}

		{ //check

			long nRow = m_CheckAvailList->GetFirstRowEnum();
			LPDISPATCH lpDisp = NULL;
			while(nRow &&
				(m_aryCheckSelectedPaymentIDs.GetSize() > 0
				|| m_aryCheckSelectedBatchPaymentIDs.GetSize() > 0
				|| m_aryCheckSelectedPaymentTipIDs.GetSize() > 0)) {

				m_CheckAvailList->GetNextRowEnum(&nRow, &lpDisp);
				IRowSettingsPtr pRow(lpDisp); 
				lpDisp->Release();

				long nID = VarLong(pRow->GetValue(checkplcID));
				BOOL bIsBatchPay = VarBool(pRow->GetValue(checkplcIsBatchPay));
				BOOL bIsTip = VarBool(pRow->GetValue(checkplcIsTip));

				if(bIsBatchPay) {
					if(FindAndRemoveFromArray(m_aryCheckSelectedBatchPaymentIDs, nID)) {
						//it was in our array, so move this record to the selected list
						m_CheckSelectedList->TakeRow(pRow);
					}

				}
				else if(bIsTip) {					
					if(FindAndRemoveFromArray(m_aryCheckSelectedPaymentTipIDs, nID)) {
						//it was in our array, so move this record to the selected list
						m_CheckSelectedList->TakeRow(pRow);
					}
				}
				else {
					if(FindAndRemoveFromArray(m_aryCheckSelectedPaymentIDs, nID)) {
						//it was in our array, so move this record to the selected list
						m_CheckSelectedList->TakeRow(pRow);
					}
				}
			}

			//if anything is left in the lists, it means they are no longer available,
			//they could have been deleted or deposited, either way clear our lists
			m_aryCheckSelectedPaymentIDs.RemoveAll();
			m_aryCheckSelectedBatchPaymentIDs.RemoveAll();
			m_aryCheckSelectedPaymentTipIDs.RemoveAll();
		}

		{ //credit

			long nRow = m_CreditAvailList->GetFirstRowEnum();
			LPDISPATCH lpDisp = NULL;
			while(nRow &&
				(m_aryCreditSelectedPaymentIDs.GetSize() > 0
				|| m_aryCreditSelectedPaymentTipIDs.GetSize() > 0)) {

				m_CreditAvailList->GetNextRowEnum(&nRow, &lpDisp);
				IRowSettingsPtr pRow(lpDisp); 
				lpDisp->Release();

				long nID = VarLong(pRow->GetValue(creditplcID));
				BOOL bIsTip = VarBool(pRow->GetValue(creditplcIsTip));

				if(bIsTip) {					
					if(FindAndRemoveFromArray(m_aryCreditSelectedPaymentTipIDs, nID)) {
						//it was in our array, so move this record to the selected list
						m_CreditSelectedList->TakeRow(pRow);
					}
				}
				else {
					if(FindAndRemoveFromArray(m_aryCreditSelectedPaymentIDs, nID)) {
						//it was in our array, so move this record to the selected list
						m_CreditSelectedList->TakeRow(pRow);
					}
				}
			}

			//if anything is left in the lists, it means they are no longer available,
			//they could have been deleted or deposited, either way clear our lists
			m_aryCreditSelectedPaymentIDs.RemoveAll();
			m_aryCreditSelectedPaymentTipIDs.RemoveAll();
		}

		//now recalculate the totals for both lists
		UpdateAvailTotals();
		UpdateSelectedTotals();

	} NxCatchAll("Error in PracBanking::RestoreCachedSelections");
}

// (j.jones 2008-05-09 15:39) - PLID 25338 - clears the cached IDs
void PracBanking::ClearCachedValues()
{
	try {

		m_aryCashSelectedPaymentIDs.RemoveAll();
		m_aryCashSelectedPaymentTipIDs.RemoveAll();
		m_aryCheckSelectedPaymentIDs.RemoveAll();
		m_aryCheckSelectedBatchPaymentIDs.RemoveAll();
		m_aryCheckSelectedPaymentTipIDs.RemoveAll();
		m_aryCreditSelectedPaymentIDs.RemoveAll();
		m_aryCreditSelectedPaymentTipIDs.RemoveAll();

		m_aryCashUnselectedPaymentIDs.RemoveAll();
		m_aryCashUnselectedPaymentTipIDs.RemoveAll();
		m_aryCheckUnselectedPaymentIDs.RemoveAll();
		m_aryCheckUnselectedBatchPaymentIDs.RemoveAll();
		m_aryCheckUnselectedPaymentTipIDs.RemoveAll();
		m_aryCreditUnselectedPaymentIDs.RemoveAll();
		m_aryCreditUnselectedPaymentTipIDs.RemoveAll();

	} NxCatchAll("Error in PracBanking::ClearCachedValues");
}

// (j.jones 2008-05-08 08:38) - PLID 25338 - removed RequeryAll and replaced it with
// RefreshLists() and RequeryAvailLists()
// (j.jones 2012-04-19 14:23) - PLID 48032 - changed all queries to used PaymentsT.BatchPaymentID
void PracBanking::RequeryAvailLists(BOOL bForceWaitForRequery)
{
	// (j.jones 2008-05-08 14:15) - PLID 25338 - this should never be called from anywhere but RefreshLists(),
	// since that caches the selected values first

	//DRT 3/10/2004 - Had to move all where clauses to this function because of the way the tips work and can 
	//or can not be included as the office needs.

	try {

		CWaitCursor pWait;

		CString strTip;
		if(!IsDlgButtonChecked(IDC_INCLUDE_TIPS_CHECK)) {
			strTip = "AND IsTip = 0";
		}

		// (j.jones 2008-05-08 15:00) - PLID 25338 - streamlined the queries to remove needless columns, and removed
		// batch payment sections from the cash and charge queries, then I added the following columns:
		// service date, input date, provider ID, location ID, and payment category

		// (z.manning 2015-11-16 17:06) - PLID 59317 - I cleaned up this code a bit here. We used to have 2 copies
		// of the same 3 queries with slightly different logic for refunds. Now the refund logic is handled here
		// and we no longer have copies of each set of from clauses.
		CString strCashRefundWhere, strCheckRefundWhere, strChargeRefundWhere;
		CString strCheckBatchPaymentAmountField, strCheckBatchPaymentTypeFilter;
		if (IsDlgButtonChecked(IDC_INCLUDE_REFUNDS_CHECK))
		{
			strCashRefundWhere = " OR PaymentsT.PayMethod = 7";
			strCheckRefundWhere = " OR PaymentsT.PayMethod = 8";
			strChargeRefundWhere = " OR PaymentsT.PayMethod = 9";
			strCheckBatchPaymentAmountField = "CASE WHEN BatchPaymentsT.Type = 3 THEN -1 * Amount ELSE Amount END AS Amount";
			strCheckBatchPaymentTypeFilter = " AND BatchPaymentsT.Type <> 2";
		}
		else
		{
			strCashRefundWhere = strCheckRefundWhere = strChargeRefundWhere = "";
			strCheckBatchPaymentAmountField = "Amount";
			strCheckBatchPaymentTypeFilter = " AND BatchPaymentsT.Type = 1";
		}

		// (z.manning, 08/15/2007) - PLID 26069 - Cash tips are now always separate from the main payment.
		// (j.gruber 2010-10-22 16:51) - PLID 37457 - added respID, tips are always pat resp, batch pays are always ins
		// (j.jones 2011-06-16 13:46) - PLID 42038 - added InputName
		// (j.jones 2011-06-24 12:49) - PLID 22833 - added PatientID
		// (j.dinatale 2011-09-13 14:10) - PLID 45371 - need to filter out voided and corrected line items (only the original line item should remain)
		// (b.eyers 2015-12-22) - PLID 67735 - fixed filter to show all payments that are not only voided
		CString strCashFrom = FormatString(
			"(SELECT PaymentsT.ID, LineItemT.PatientID, PersonT.FullName AS Name, "
			" LineItemT.Amount, PaymentsT.PayMethod, PaymentsT.Deposited, Convert(bit,0) AS IsTip, "
			" LineItemT.Date AS ServiceDate, LineItemT.InputDate, LineItemT.InputName, PaymentsT.ProviderID, LineItemT.LocationID, PaymentsT.PaymentGroupID AS CategoryID, "
			" CASE WHEN PaymentsT.InsuredPartyID = -1 OR PaymentsT.InsuredPartyID IS NULL THEN 1 ELSE 2 END AS RespID "
			" FROM LineItemT LEFT JOIN PatientsT ON LineItemT.PatientID = PatientsT.PersonID LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			" INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID LEFT JOIN ProvidersT ON PaymentsT.ProviderID = ProvidersT.PersonID "
			" LEFT JOIN LineItemCorrectionsT VoidingLineItemsT ON LineItemT.ID = VoidingLineItemsT.VoidingLineItemID "
			" LEFT JOIN LineItemCorrectionsT CorrectedLineItemsT ON LineItemT.ID = CorrectedLineItemsT.NewLineItemID "
			" LEFT JOIN LineItemCorrectionsT AS OriginalLineItemCorrection ON LineItemT.ID = OriginalLineItemCorrection.OriginalLineItemID "
			" WHERE (((PaymentsT.PayMethod >=1 AND PaymentsT.PayMethod <= 3)%s)  AND LineItemT.Deleted = 0) AND PaymentsT.BatchPaymentID Is Null "				
			" AND CorrectedLineItemsT.NewLineItemID IS NULL AND VoidingLineItemsT.VoidingLineItemID IS NULL "
			" AND LineItemT.ID NOT IN (SELECT OriginalLineItemID FROM LineItemCorrectionsT WHERE NewLineItemID IS NULL) "
			//AND OriginalLineItemCorrection.NewLineItemID IS NOT NULL "
			" UNION "
			" SELECT PaymentTipsT.ID, LineItemT.PatientID, PersonT.FullName + '  (Tip)' AS Name, PaymentTipsT.Amount, PaymentTipsT.PayMethod, "
			" PaymentTipsT.Deposited, Convert(bit,1) AS IsTip,  "
			" LineItemT.Date AS ServiceDate, LineItemT.InputDate, LineItemT.InputName, ProvidersT.PersonID AS ProviderID, LineItemT.LocationID, PaymentsT.PaymentGroupID AS CategoryID, 1 as RespID "
			" FROM PaymentTipsT INNER JOIN LineItemT ON PaymentTipsT.PaymentID = LineItemT.ID LEFT JOIN PersonT ON LineItemT.PatientID = PersonT.ID "
			" INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID LEFT JOIN ProvidersT ON PaymentTipsT.ProvID = ProvidersT.PersonID "
			" LEFT JOIN LineItemCorrectionsT AS OriginalLineItemCorrection ON LineItemT.ID = OriginalLineItemCorrection.OriginalLineItemID "
			" WHERE LineItemT.Deleted = 0 AND LineItemT.ID NOT IN (SELECT OriginalLineItemID FROM LineItemCorrectionsT WHERE NewLineItemID IS NULL)) AS Query"
			//AND OriginalLineItemCorrection.NewLineItemID IS NOT NULL) AS Query"
			, strCashRefundWhere, strCheckBatchPaymentAmountField);

		// (j.jones 2008-05-12 15:16) - PLID 30002 - fixed Batch Payment refunds so they show as a negative value
		// (j.jones 2011-06-16 13:46) - PLID 42038 - added InputName
		// (j.jones 2011-06-24 12:49) - PLID 22833 - added PatientID
		// (j.dinatale 2011-09-13 14:10) - PLID 45371 - need to filter out voided and corrected line items (only the original line item should remain)
		// (b.eyers 2015-12-22) - PLID 67735 - fixed filter to show all payments that are not only voided
		CString strCheckFrom = FormatString(
			"(SELECT PaymentsT.ID, LineItemT.PatientID, PaymentPlansT.CheckNo, PersonT.FullName AS Name, "
			"LineItemT.Amount + (SELECT CASE WHEN Sum(Amount) IS NULL THEN 0 ELSE Sum(Amount) END FROM PaymentTipsT WHERE PaymentID = PaymentsT.ID AND PayMethod = PaymentsT.PayMethod) AS Amount,  "
			"PaymentsT.PayMethod, PaymentsT.Deposited, Convert(bit,0) AS BatchPayment, Convert(bit,0) AS IsTip, "
			" LineItemT.Date AS ServiceDate, LineItemT.InputDate, LineItemT.InputName, PaymentsT.ProviderID, LineItemT.LocationID, PaymentsT.PaymentGroupID AS CategoryID, "
			" CASE WHEN PaymentsT.InsuredPartyID = -1 OR PaymentsT.InsuredPartyID IS NULL THEN 1 ELSE 2 END AS RespID "
			"FROM LineItemT LEFT JOIN PatientsT ON LineItemT.PatientID = PatientsT.PersonID  "
			"LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID  "
			"LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID LEFT JOIN ProvidersT ON PaymentsT.ProviderID = ProvidersT.PersonID  "
			"LEFT JOIN LineItemCorrectionsT VoidingLineItemsT ON LineItemT.ID = VoidingLineItemsT.VoidingLineItemID "
			"LEFT JOIN LineItemCorrectionsT CorrectedLineItemsT ON LineItemT.ID = CorrectedLineItemsT.NewLineItemID "
			"LEFT JOIN LineItemCorrectionsT AS OriginalLineItemCorrection ON LineItemT.ID = OriginalLineItemCorrection.OriginalLineItemID "
			"WHERE (((PaymentsT.PayMethod >=1 AND PaymentsT.PayMethod <= 3)%s)  AND LineItemT.Deleted = 0) AND PaymentsT.BatchPaymentID Is Null  "
			" AND CorrectedLineItemsT.NewLineItemID IS NULL AND VoidingLineItemsT.VoidingLineItemID IS NULL "
			" AND LineItemT.ID NOT IN (SELECT OriginalLineItemID FROM LineItemCorrectionsT WHERE NewLineItemID IS NULL) "
			//AND OriginalLineItemCorrection.NewLineItemID IS NOT NULL "
			"UNION  "
			"SELECT ID, NULL AS PatientID, CheckNo, Name, %s, 2 AS PayMethod, Deposited, Convert(bit,1) AS BatchPayment, Convert(bit,0) AS IsTip, "
			"BatchPaymentsT.Date AS ServiceDate, BatchPaymentsT.InputDate, UsersT.Username AS InputName, BatchPaymentsT.ProviderID, BatchPaymentsT.Location AS LocationID, BatchPaymentsT.PayCatID AS CategoryID, 2 as RespID "
			"FROM BatchPaymentsT INNER JOIN InsuranceCoT ON BatchPaymentsT.InsuranceCoID = InsuranceCoT.PersonID  "
			"LEFT JOIN UsersT ON BatchPaymentsT.UserID = UsersT.PersonID "
			"WHERE BatchPaymentsT.Deleted = 0%s "
			"UNION  "
			"SELECT PaymentTipsT.ID, LineItemT.PatientID, '', PersonT.FullName + '  (Tip)' AS Name, PaymentTipsT.Amount, PaymentTipsT.PayMethod,  "
			"PaymentTipsT.Deposited, Convert(bit,0) AS BatchPayment, Convert(bit,1) AS IsTip, "
			"LineItemT.Date AS ServiceDate, LineItemT.InputDate, LineItemT.InputName, ProvidersT.PersonID AS ProviderID, LineItemT.LocationID, PaymentsT.PaymentGroupID AS CategoryID, 1 as RespID "
			"FROM PaymentTipsT "
			"INNER JOIN LineItemT ON PaymentTipsT.PaymentID = LineItemT.ID LEFT JOIN PersonT ON LineItemT.PatientID = PersonT.ID  "
			"INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID LEFT JOIN ProvidersT ON PaymentTipsT.ProvID = ProvidersT.PersonID  "
			"LEFT JOIN LineItemCorrectionsT AS OriginalLineItemCorrection ON LineItemT.ID = OriginalLineItemCorrection.OriginalLineItemID "
			"WHERE LineItemT.Deleted = 0 AND PaymentsT.PayMethod <> PaymentTipsT.PayMethod"
			" AND LineItemT.ID NOT IN (SELECT OriginalLineItemID FROM LineItemCorrectionsT WHERE NewLineItemID IS NULL)) AS Query"
			//"	AND OriginalLineItemCorrection.NewLineItemID IS NOT NULL) AS Query "
			, strCheckRefundWhere, strCheckBatchPaymentAmountField, strCheckBatchPaymentTypeFilter);
		
		// (e.lally 2007-07-09) PLID 25993 - Changed credit card name to use ID link to CC table
		// - Formatted query to be more readable
		// (j.jones 2011-06-16 13:46) - PLID 42038 - added InputName
		// (j.jones 2011-06-24 12:49) - PLID 22833 - added PatientID
		// (j.dinatale 2011-09-13 14:10) - PLID 45371 - need to filter out voided and corrected line items (only the original line item should remain)
		// (b.eyers 2015-12-22) - PLID 67735 - fixed filter to show all payments that are not only voided
		CString strChargeFrom = FormatString(
			"(SELECT PaymentsT.ID, LineItemT.PatientID, CreditCardNamesT.CardName, Right(PaymentPlansT.CCNumber, 4) AS Last4, PersonT.FullName AS Name, "
			" LineItemT.Amount + (SELECT CASE WHEN Sum(Amount) IS NULL THEN 0 ELSE Sum(Amount) END FROM PaymentTipsT WHERE PaymentID = PaymentsT.ID AND PayMethod = PaymentsT.PayMethod) AS Amount, "
			" PaymentsT.PayMethod, PaymentsT.Deposited, Convert(bit,0) AS IsTip, PaymentPlansT.CreditCardID, "
			" LineItemT.Date AS ServiceDate, LineItemT.InputDate, LineItemT.InputName, PaymentsT.ProviderID, LineItemT.LocationID, PaymentsT.PaymentGroupID AS CategoryID, "
			" CASE WHEN PaymentsT.InsuredPartyID = -1 OR PaymentsT.InsuredPartyID IS NULL THEN 1 ELSE 2 END AS RespID "
			" FROM LineItemT "
			" LEFT JOIN PatientsT ON LineItemT.PatientID = PatientsT.PersonID "
			" LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			" INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			" LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
			" LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
			" LEFT JOIN LineItemCorrectionsT VoidingLineItemsT ON LineItemT.ID = VoidingLineItemsT.VoidingLineItemID "
			" LEFT JOIN LineItemCorrectionsT CorrectedLineItemsT ON LineItemT.ID = CorrectedLineItemsT.NewLineItemID "
			" LEFT JOIN LineItemCorrectionsT AS OriginalLineItemCorrection ON LineItemT.ID = OriginalLineItemCorrection.OriginalLineItemID "
			" LEFT JOIN ProvidersT ON PaymentsT.ProviderID = ProvidersT.PersonID "
			" WHERE (((PaymentsT.PayMethod >=1 AND PaymentsT.PayMethod <= 3)%s) AND LineItemT.Deleted = 0) AND PaymentsT.BatchPaymentID Is Null "				
			" AND CorrectedLineItemsT.NewLineItemID IS NULL AND VoidingLineItemsT.VoidingLineItemID IS NULL "
			" AND LineItemT.ID NOT IN (SELECT OriginalLineItemID FROM LineItemCorrectionsT WHERE NewLineItemID IS NULL) "
			//AND OriginalLineItemCorrection.NewLineItemID IS NOT NULL "
			" UNION "
			" SELECT PaymentTipsT.ID, LineItemT.PatientID, CreditCardNamesT.CardName, Right(PaymentPlansT.CCNumber, 4) AS Last4, PersonT.FullName + '  (Tip)' AS Name, PaymentTipsT.Amount, PaymentTipsT.PayMethod, "
			" PaymentTipsT.Deposited, Convert(bit,1) AS IsTip, PaymentPlansT.CreditCardID, "
			" LineItemT.Date AS ServiceDate, LineItemT.InputDate, LineItemT.InputName, ProvidersT.PersonID AS ProviderID, LineItemT.LocationID, PaymentsT.PaymentGroupID AS CategoryID, 1 as RespID "
			" FROM PaymentTipsT  "
			" INNER JOIN LineItemT ON PaymentTipsT.PaymentID = LineItemT.ID "
			" LEFT JOIN PaymentPlansT ON PaymentTipsT.PaymentID = PaymentPlansT.ID "
			" LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
			" LEFT JOIN PersonT ON LineItemT.PatientID = PersonT.ID "
			" INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			" LEFT JOIN ProvidersT ON PaymentTipsT.ProvID = ProvidersT.PersonID "
			" LEFT JOIN LineItemCorrectionsT AS OriginalLineItemCorrection ON LineItemT.ID = OriginalLineItemCorrection.OriginalLineItemID "
			" WHERE LineItemT.Deleted = 0 AND PaymentsT.PayMethod <> PaymentTipsT.PayMethod"
			" AND LineItemT.ID NOT IN (SELECT OriginalLineItemID FROM LineItemCorrectionsT WHERE NewLineItemID IS NULL)) AS Query"
			//"	AND OriginalLineItemCorrection.NewLineItemID IS NOT NULL) AS Query"
			, strChargeRefundWhere);

		//load the where clauses
		CString strWhere;
		if (IsDlgButtonChecked(IDC_INCLUDE_REFUNDS_CHECK)) {
			strWhere.Format("(PayMethod = '1' OR Paymethod = '7') AND Deposited = 0 AND Amount <> 0 %s", strTip);
		}
		else {
			strWhere.Format("PayMethod = '1' AND Deposited = 0 AND Amount <> 0 %s", strTip);
		}
		m_CashAvailList->PutWhereClause(_bstr_t(strWhere));
		m_CashAvailList->PutFromClause(_bstr_t(strCashFrom));

		if (IsDlgButtonChecked(IDC_INCLUDE_REFUNDS_CHECK)) {
			strWhere.Format("(PayMethod = '2' OR Paymethod = '8') AND Deposited = 0 AND Amount <> 0 %s", strTip);
		}
		else {
			strWhere.Format("PayMethod = '2' AND Deposited = 0 AND Amount <> 0 %s", strTip);
		}
		m_CheckAvailList->PutWhereClause(_bstr_t(strWhere));
		m_CheckAvailList->PutFromClause(_bstr_t(strCheckFrom));

		if (IsDlgButtonChecked(IDC_INCLUDE_REFUNDS_CHECK)) {
			strWhere.Format("(PayMethod = '3' OR PayMethod = '9') AND Deposited = 0 AND Amount <> 0 %s", strTip);
		}
		else {
			strWhere.Format("PayMethod = '3' AND Deposited = 0 AND Amount <> 0 %s", strTip);
		}
		m_CreditAvailList->PutWhereClause(_bstr_t(strWhere));
		m_CreditAvailList->PutFromClause(_bstr_t(strChargeFrom));

		//MessageBox(m_CheckSelectedList->GetSqlPending());

		m_bCashUnselectedDone = FALSE;
		m_bCreditUnselectedDone = FALSE;
		m_bCheckUnselectedDone = FALSE;

		m_CashAvailList->Requery();
		m_CheckAvailList->Requery();
		m_CreditAvailList->Requery();

		// (j.jones 2008-08-22 08:39) - PLID 25338 - if the caller says we must wait for the requery
		// to finish, then do so now
		if(bForceWaitForRequery) {
			m_CashAvailList->WaitForRequery(dlPatienceLevelWaitIndefinitely);
			m_CheckAvailList->WaitForRequery(dlPatienceLevelWaitIndefinitely);
			m_CreditAvailList->WaitForRequery(dlPatienceLevelWaitIndefinitely);
		}

	} NxCatchAll("Error in PracBanking::RequeryAvailLists");
}

// (j.jones 2008-05-07 17:34) - PLID 25338 - given the payment information and the filter information,
// this function determines whether the payment matches our filters, and if so, returns TRUE, else returns FALSE
// (r.gonet 09-13-2010 16:26) - PLID 37458 - We can now filter on an arbitrary number of categories
// (j.gruber 2010-10-22 16:24) - PLID 37457 - added resp filter
// (j.jones 2011-06-16 13:11) - PLID 42038 - added user filter
BOOL PracBanking::DoesPaymentMatchFilter(COleDateTime dtServiceDate, COleDateTime dtInputDate, long nProviderID,
	long nLocationID, long nCategoryID, long nRespID, CString strUserName,
	BOOL bUsingDateRange, BOOL bUsingServiceDate, COleDateTime dtFrom, COleDateTime dtTo,
	BOOL bUseProviderFilter, long nProviderFilterID,
	BOOL bUseLocationFilter, long nLocationFilterID,
	BOOL bUseCategoryFilter, CArray<long, long> &aryCategoryIDs,
	long nRespFilter,
	BOOL bUseUserFilter, CString strUserFilterName)
{
	try {

		//the calling function already built the filters, including advancing dtTo to the day after the filter date,
		//so just do the raw comparisons and determine if the given payment qualifies

		//date filter
		if (bUsingDateRange) {

			//return false if using a date range and either date is invalid
			if(dtFrom.m_status == COleDateTime::invalid) {
				//how did this happen?
				ASSERT(FALSE);
				return FALSE;
			}
			if(dtTo.m_status == COleDateTime::invalid) {
				//how did this happen?
				ASSERT(FALSE);
				return FALSE;
			}

			//service or input date?
			if(bUsingServiceDate) {
				//service date

				//return false if the date is invalid
				if(dtServiceDate.m_status == COleDateTime::invalid) {
					//how did this happen?
					ASSERT(FALSE);
					return FALSE;
				}

				if(dtServiceDate < dtFrom || dtServiceDate >= dtTo) {

					//the service date is outside of our filter range
					return FALSE;
				}
			}
			else {
				//input date

				//return false if the date is invalid
				if(dtInputDate.m_status == COleDateTime::invalid) {
					//how did this happen?
					ASSERT(FALSE);
					return FALSE;
				}

				if(dtInputDate < dtFrom || dtInputDate >= dtTo) {

					//the input date is outside of our filter range
					return FALSE;
				}
			}
		}

		//provider filter
		if(bUseProviderFilter) {

			//-1 is a valid filter ID, it filters specifically on payments with no provider
			if(nProviderFilterID != nProviderID) {

				//the filtered provider ID doesn't match the payment's provider
				return FALSE;
			}
		}

		//location filter
		if(bUseLocationFilter) {

			if(nLocationFilterID != -1 && nLocationFilterID != nLocationID) {

				//we have a location ID, but it doesn't match the payment's location
				return FALSE;
			}
		}

		//category filter
		if(bUseCategoryFilter) {
			// (r.gonet 09-13-2010 16:26) - PLID 37458 - Filter on an arbitrary number of categories.

			//0 is a valid filter ID, it filters specifically on payments with no category
			BOOL bCategoryMatch = FALSE;
			for(int i = 0; i < aryCategoryIDs.GetCount(); i++) {
				long nCategoryFilterID = aryCategoryIDs[i];
				if(nCategoryID == nCategoryFilterID) {
					bCategoryMatch = TRUE;
					break;
				}
			}

			// We have a category ID, but it doesn't match any of the payment categories
			if(nCategoryID != -1 && !bCategoryMatch) {
				return FALSE;
			}
		}

		// (j.gruber 2010-10-22 16:41) - PLID 37457 - added resp filter
		if (nRespFilter != -1) {
			if (nRespID != nRespFilter) {
				return FALSE;
			}
		}

		// (j.jones 2011-06-16 13:11) - PLID 42038 - added user filter
		if(bUseUserFilter) {
			if(strUserFilterName != "" && strUserFilterName != strUserName) {
				//we have a user name, but it does not match the payment's input user
				return FALSE;
			}
		}

		//if we got this far, it means none of the filters failed,
		//therefore our payment can be displayed in the given filters
		return TRUE;

	}NxCatchAll("Error in PracBanking::DoesPaymentMatchFilter");

	return FALSE;
}

// (j.jones 2008-05-07 17:33) - PLID 25338 - reviews the unselected lists and moves payments to the
// selected lists if they match our filters
void PracBanking::FilterAllPayments()
{
	try {

		//this function should only move payments from 'unselected' to 'selected', based on the
		//filters - if we want to remove payments from the 'selected' list, we should call
		//UnselectAllPayments() prior to calling this function

		//store the filter selections

		COleDateTime dtFrom = m_from.GetValue();
		COleDateTime dtTo = m_to.GetValue();

		dtFrom.SetDateTime(dtFrom.GetYear(),dtFrom.GetMonth(), dtFrom.GetDay(), 0, 0, 0);
		dtTo.SetDateTime(dtTo.GetYear(),dtTo.GetMonth(), dtTo.GetDay(), 0, 0, 0);

		COleDateTimeSpan dtSpan;
		dtSpan.SetDateTimeSpan(1,0,0,0);
		dtTo += dtSpan;

		BOOL bUsingDateRange = m_DateRange.GetCheck();
		BOOL bUsingServiceDate = m_radioServiceDate.GetCheck();
		BOOL bUseProviderFilter = m_rSingleProvider.GetCheck();
		BOOL bUseLocationFilter = m_rSingleLocation.GetCheck();
		BOOL bUseCategoryFilter = m_rSinglePayCat.GetCheck();
		// (j.jones 2011-06-16 13:27) - PLID 42038 - added user filter
		BOOL bUseUserFilter = m_rSingleUser.GetCheck();

		long nProviderFilterID = -1;
		if(bUseProviderFilter && m_ProvSelect->GetCurSel() != -1) {
			nProviderFilterID = VarLong(m_ProvSelect->GetValue(m_ProvSelect->GetCurSel(),0), -1);
		}

		long nLocationFilterID = -1;
		if(bUseLocationFilter && m_LocSelect->GetCurSel() != -1) {
			nLocationFilterID = VarLong(m_LocSelect->GetValue(m_LocSelect->GetCurSel(),0), -1);
		}

		NXDATALIST2Lib::IRowSettingsPtr pRespRow = m_pRespFilter->CurSel;
		long nRespFilter = -1;
		if (pRespRow) {
			nRespFilter = VarLong(pRespRow->GetValue(0));
		}

		// (j.jones 2011-06-16 13:27) - PLID 42038 - added user filter
		NXDATALIST2Lib::IRowSettingsPtr pUserRow = m_UserSelect->GetCurSel();
		CString strUserFilterName = "";
		if (pUserRow) {
			long nUserID = VarLong(pUserRow->GetValue(uccID), -1);
			if(nUserID != -1) {
				strUserFilterName = VarString(pUserRow->GetValue(uccUsername), "");
			}
		}

		//run through every payment in all three unselected lists,
		//grab its information, and move to the selected list
		//if its information matches the filter

		{ //cash
			long nRow = m_CashAvailList->GetFirstRowEnum();
			LPDISPATCH lpDisp = NULL;
			while(nRow) 
			{
				m_CashAvailList->GetNextRowEnum(&nRow, &lpDisp);
				IRowSettingsPtr pRow(lpDisp); 
				lpDisp->Release();

				//grab all the filterable values
				COleDateTime dtServiceDate = VarDateTime(pRow->GetValue(cashplcDate));
				COleDateTime dtInputDate = VarDateTime(pRow->GetValue(cashplcInputDate));
				long nProviderID = VarLong(pRow->GetValue(cashplcProviderID), -1);
				long nLocationID = VarLong(pRow->GetValue(cashplcLocationID), -1);
				long nCategoryID = VarLong(pRow->GetValue(cashplcCategoryID), 0); //0 means no category
				// (j.gruber 2010-10-22 16:46) - PLID 37457
				long nRespID = VarLong(pRow->GetValue(cashplcRespID), -1);
				// (j.jones 2011-06-16 13:31) - PLID 42038 - get the User Name
				CString strUserName = VarString(pRow->GetValue(cashplcUserName), "");

				// (r.gonet 2010-09-13 12:40) - PLID 37458 - Pass in the array of payment category ids for filtering on
				// (j.gruber 2010-10-22 16:46) - PLID 37457 - added resp filter
				// (j.jones 2011-06-16 13:26) - PLID 42038 - added user filter
				if(DoesPaymentMatchFilter(dtServiceDate, dtInputDate, nProviderID, nLocationID, nCategoryID, nRespID, strUserName,
					bUsingDateRange, bUsingServiceDate, dtFrom, dtTo,
					bUseProviderFilter, nProviderFilterID,
					bUseLocationFilter, nLocationFilterID,
					bUseCategoryFilter, m_aryCategoryIDs,
					nRespFilter,
					bUseUserFilter, strUserFilterName)) {

					m_CashSelectedList->TakeRow(pRow);
				}
			}
		}

		{ //check
			long nRow = m_CheckAvailList->GetFirstRowEnum();
			LPDISPATCH lpDisp = NULL;
			while(nRow) 
			{
				m_CheckAvailList->GetNextRowEnum(&nRow, &lpDisp);
				IRowSettingsPtr pRow(lpDisp); 
				lpDisp->Release();

				//grab all the filterable values
				COleDateTime dtServiceDate = VarDateTime(pRow->GetValue(checkplcDate));
				COleDateTime dtInputDate = VarDateTime(pRow->GetValue(checkplcInputDate));
				long nProviderID = VarLong(pRow->GetValue(checkplcProviderID), -1);
				long nLocationID = VarLong(pRow->GetValue(checkplcLocationID), -1);
				long nCategoryID = VarLong(pRow->GetValue(checkplcCategoryID), 0); //0 means no category
				// (j.gruber 2010-10-22 16:46) - PLID 37457
				long nRespID = VarLong(pRow->GetValue(checkplcRespID), -1);
				// (j.jones 2011-06-16 13:31) - PLID 42038 - get the User Name
				CString strUserName = VarString(pRow->GetValue(checkplcUserName), "");

				// (r.gonet 2010-09-13 12:40) - PLID 37458 - Pass in the array of payment category ids for filtering on
				// (j.jones 2011-06-16 13:26) - PLID 42038 - added user filter
				if(DoesPaymentMatchFilter(dtServiceDate, dtInputDate, nProviderID, nLocationID, nCategoryID, nRespID, strUserName,
					bUsingDateRange, bUsingServiceDate, dtFrom, dtTo,
					bUseProviderFilter, nProviderFilterID,
					bUseLocationFilter, nLocationFilterID,
					bUseCategoryFilter, m_aryCategoryIDs,
					nRespFilter,
					bUseUserFilter, strUserFilterName)) {

					m_CheckSelectedList->TakeRow(pRow);
				}
			}
		}
		
		{ //credit
			long nRow = m_CreditAvailList->GetFirstRowEnum();
			LPDISPATCH lpDisp = NULL;
			while(nRow) 
			{
				m_CreditAvailList->GetNextRowEnum(&nRow, &lpDisp);
				IRowSettingsPtr pRow(lpDisp); 
				lpDisp->Release();

				//grab all the filterable values
				COleDateTime dtServiceDate = VarDateTime(pRow->GetValue(creditplcDate));
				COleDateTime dtInputDate = VarDateTime(pRow->GetValue(creditplcInputDate));
				long nProviderID = VarLong(pRow->GetValue(creditplcProviderID), -1);
				long nLocationID = VarLong(pRow->GetValue(creditplcLocationID), -1);
				long nCategoryID = VarLong(pRow->GetValue(creditplcCategoryID), 0); //0 means no category
				// (j.gruber 2010-10-22 16:46) - PLID 37457
				long nRespID = VarLong(pRow->GetValue(creditplcRespID), -1);
				// (j.jones 2011-06-16 13:31) - PLID 42038 - get the User Name
				CString strUserName = VarString(pRow->GetValue(creditplcUserName), "");

				// (r.gonet 2010-09-13 12:40) - PLID 37458 - Pass in the array of payment category ids for filtering on
				// (j.jones 2011-06-16 13:26) - PLID 42038 - added user filter
				if(DoesPaymentMatchFilter(dtServiceDate, dtInputDate, nProviderID, nLocationID, nCategoryID, nRespID, strUserName,
					bUsingDateRange, bUsingServiceDate, dtFrom, dtTo,
					bUseProviderFilter, nProviderFilterID,
					bUseLocationFilter, nLocationFilterID,
					bUseCategoryFilter, m_aryCategoryIDs,
					nRespFilter,
					bUseUserFilter, strUserFilterName)) {

					m_CreditSelectedList->TakeRow(pRow);
				}
			}
		}

		UpdateAvailTotals();
		UpdateSelectedTotals();

	}NxCatchAll("Error in PracBanking::FilterAllPayments");
}

BEGIN_EVENTSINK_MAP(PracBanking, CNxDialog)
    //{{AFX_EVENTSINK_MAP(PracBanking)
	ON_EVENT(PracBanking, IDC_CASHAVAILLIST, 3 /* DblClickCell */, OnDblClickCellCashAvailList, VTS_I4 VTS_I2)
	ON_EVENT(PracBanking, IDC_CHECKAVAILLIST, 3 /* DblClickCell */, OnDblClickCellCheckAvailList, VTS_I4 VTS_I2)
	ON_EVENT(PracBanking, IDC_CHECKSELECTEDLIST, 3 /* DblClickCell */, OnDblClickCellCheckSelectedList, VTS_I4 VTS_I2)
	ON_EVENT(PracBanking, IDC_CREDITAVAILLIST, 3 /* DblClickCell */, OnDblClickCellCreditAvailList, VTS_I4 VTS_I2)
	ON_EVENT(PracBanking, IDC_CREDITSELECTEDLIST, 3 /* DblClickCell */, OnDblClickCellCreditSelectedList, VTS_I4 VTS_I2)
	ON_EVENT(PracBanking, IDC_CASHSELECTEDLIST, 3 /* DblClickCell */, OnDblClickCellCashSelectedList, VTS_I4 VTS_I2)
	ON_EVENT(PracBanking, IDC_CASHAVAILLIST, 18 /* RequeryFinished */, OnRequeryFinishedCashavaillist, VTS_I2)
	ON_EVENT(PracBanking, IDC_CHECKAVAILLIST, 18 /* RequeryFinished */, OnRequeryFinishedCheckavaillist, VTS_I2)
	ON_EVENT(PracBanking, IDC_CREDITAVAILLIST, 18 /* RequeryFinished */, OnRequeryFinishedCreditavaillist, VTS_I2)
	ON_EVENT(PracBanking, IDC_PROVSELECTOR, 16 /* SelChosen */, OnSelChosenProvselector, VTS_I4)
	ON_EVENT(PracBanking, IDC_LOCSELECTOR, 16 /* SelChosen */, OnSelChosenLocselector, VTS_I4)	
	ON_EVENT(PracBanking, IDC_COMBO_CATEGORY, 16 /* SelChosen */, OnSelChosenComboCategory, VTS_I4)
	ON_EVENT(PracBanking, IDC_CREDITAVAILLIST, 6 /* RButtonDown */, OnRButtonDownCreditavaillist, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(PracBanking, IDC_CREDITSELECTEDLIST, 6 /* RButtonDown */, OnRButtonDownCreditselectedlist, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
	ON_EVENT(PracBanking, IDC_RESP_FILTER_LIST, 16, SelChosenRespFilterList, VTS_DISPATCH)
	ON_EVENT(PracBanking, IDC_RESP_FILTER_LIST, 18, RequeryFinishedRespFilterList, VTS_I2)
	ON_EVENT(PracBanking, IDC_COMBO_USER_FILTER, 16, OnSelChosenComboUserFilter, VTS_DISPATCH)
	ON_EVENT(PracBanking, IDC_CASHAVAILLIST, 6, OnRButtonDownCashavaillist, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(PracBanking, IDC_CASHSELECTEDLIST, 6, OnRButtonDownCashselectedlist, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(PracBanking, IDC_CHECKAVAILLIST, 6, OnRButtonDownCheckavaillist, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(PracBanking, IDC_CHECKSELECTEDLIST, 6, OnRButtonDownCheckselectedlist, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(PracBanking, IDC_CASHAVAILLIST, 22, OnColumnSizingFinishedCashavaillist, VTS_I2 VTS_BOOL VTS_I4 VTS_I4)
	ON_EVENT(PracBanking, IDC_CASHSELECTEDLIST, 22, OnColumnSizingFinishedCashselectedlist, VTS_I2 VTS_BOOL VTS_I4 VTS_I4)
	ON_EVENT(PracBanking, IDC_CHECKAVAILLIST, 22, OnColumnSizingFinishedCheckavaillist, VTS_I2 VTS_BOOL VTS_I4 VTS_I4)
	ON_EVENT(PracBanking, IDC_CHECKSELECTEDLIST, 22, OnColumnSizingFinishedCheckselectedlist, VTS_I2 VTS_BOOL VTS_I4 VTS_I4)
	ON_EVENT(PracBanking, IDC_CREDITAVAILLIST, 22, OnColumnSizingFinishedCreditavaillist, VTS_I2 VTS_BOOL VTS_I4 VTS_I4)
	ON_EVENT(PracBanking, IDC_CREDITSELECTEDLIST, 22, OnColumnSizingFinishedCreditselectedlist, VTS_I2 VTS_BOOL VTS_I4 VTS_I4)
END_EVENTSINK_MAP()

void PracBanking::Print() 
{

	try {

		if(m_CashSelectedList->GetRowCount() == 0 &&
			m_CheckSelectedList->GetRowCount() == 0 &&
			m_CreditSelectedList->GetRowCount() == 0) {

			AfxMessageBox("There are no payments in the 'Selected' lists.");
			return;
		}

		// (j.jones 2008-05-12 10:09) - PLID 25338 - changed to work solely off of the datalists,
		// so we'll cache the IDs, turn them into comma-delimited strings, and pass them into the report

		//fill our lists of selected & unselected values (though right now we only need the selected values)
		CacheSelectionStates();

		//populate our ID lists

		//default the lists to -1, because the filter will always be used
		CString strPaymentIDs = "-1", strBatchPaymentIDs = "-1", strPaymentTipIDs = "-1";
		
		AppendArrayIntoString(m_aryCashSelectedPaymentIDs, strPaymentIDs);
		AppendArrayIntoString(m_aryCheckSelectedPaymentIDs, strPaymentIDs);
		AppendArrayIntoString(m_aryCreditSelectedPaymentIDs, strPaymentIDs);

		AppendArrayIntoString(m_aryCheckSelectedBatchPaymentIDs, strBatchPaymentIDs);

		AppendArrayIntoString(m_aryCashSelectedPaymentTipIDs, strPaymentTipIDs);
		AppendArrayIntoString(m_aryCheckSelectedPaymentTipIDs, strPaymentTipIDs);
		AppendArrayIntoString(m_aryCreditSelectedPaymentTipIDs, strPaymentTipIDs);

		//after our strings are built, we do not need the cached information anymore
		ClearCachedValues();

		//The where clause needs to be ORed, not ANDed, because each record
		//is only going to have one ID filled in, and the other two be NULL,
		//as the report is a UNION query of all payments, batch payments, and tips.
		//As such, we have to filter on something, so each ID string includes -1
		CString strWhere;
		strWhere.Format("PaymentID IN (%s) OR BatchPaymentID IN (%s) OR TipID IN (%s)",
			strPaymentIDs, strBatchPaymentIDs, strPaymentTipIDs);

		CWaitCursor pWait;
		COleDateTime today;
		
		CReportInfo  infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(229)]);
		CPrintDialog* dlg;
		dlg = new CPrintDialog(FALSE);
		CPrintInfo prInfo;
		prInfo.m_bPreview = false;
		prInfo.m_bDirect = false;
		prInfo.m_bDocObject = false;
		if (prInfo.m_pPD) delete prInfo.m_pPD;
		prInfo.m_pPD = dlg;

		infReport.strExtraText = "WHERE " + strWhere;

		COleDateTime dtDeposit = m_dtDeposit.GetValue();
		SetRemotePropertyDateTime("CurrentDepositDate",dtDeposit,0,GetCurrentUserName());

		//Made new function for running reports - JMM 5-28-04
		RunReport(&infReport, false, this, "Deposit Slip for " + FormatDateTimeForInterface(dtDeposit, NULL, dtoDate), &prInfo);

	} NxCatchAll("Error in PracBanking::Print()");
}

void PracBanking::PrintTotals(COleCurrency cash, COleCurrency check, COleCurrency credit, COleCurrency tot, CDC *pDC)
{
	try {
		pDC->TextOut(225,-155, FormatCurrencyForInterface(cash));
		pDC->TextOut(225,-175, FormatCurrencyForInterface(check));
		pDC->TextOut(225,-195, FormatCurrencyForInterface(credit));
		pDC->TextOut(225,-215, FormatCurrencyForInterface(tot));	
	} NxCatchAll("Error in PracBanking::PrintTotals()");
}

// (j.jones 2008-05-08 14:19) - PLID 25338 - the selected lists are never requeried now,
// so I broke up the UpdateTotals() function into UpdateAvailTotals() and UpdateSelectedTotals()
void PracBanking::UpdateAvailTotals()
{
	try {

		// if any of these are true, at least one of the lists is requerying and the
		// OnRequeryFinished... will handle updating the totals.
		if(!m_bCashUnselectedDone || !m_bCashUnselectedDone || !m_bCreditUnselectedDone) {
			return;
		}

		COleCurrency cyTmp, cyTotal;
		cyTmp = COleCurrency(0,0);
		cyTotal = COleCurrency(0,0);

		cyTotal+= cyTmp = GetTotal(m_CashAvailList, cashplcAmount);
		SetDlgItemText (IDC_CASH_AVAIL, FormatCurrencyForInterface(cyTmp) );
		cyTotal+= cyTmp = GetTotal(m_CheckAvailList, checkplcAmount);
		SetDlgItemText (IDC_CHECK_AVAIL, FormatCurrencyForInterface(cyTmp) );
		cyTotal+= cyTmp = GetTotal(m_CreditAvailList, creditplcAmount);
		SetDlgItemText (IDC_CREDIT_AVAIL, FormatCurrencyForInterface(cyTmp) );	
		SetDlgItemText (IDC_TOTAL_AVAIL, FormatCurrencyForInterface(cyTotal) );

	} NxCatchAll("Error in PracBanking::UpdateAvailTotals()");
}

void PracBanking::UpdateSelectedTotals()
{
	try {

		COleCurrency cyTmp, cyTotal;
		cyTmp = COleCurrency(0,0);
		cyTotal = COleCurrency(0,0);

		cyTotal+= cyTmp = GetTotal(m_CashSelectedList, cashplcAmount);
		SetDlgItemText (IDC_CASH_SEL, FormatCurrencyForInterface(cyTmp) );
		cyTotal+= cyTmp = GetTotal(m_CheckSelectedList, checkplcAmount);
		SetDlgItemText (IDC_CHECK_SEL, FormatCurrencyForInterface(cyTmp) );
		cyTotal+= cyTmp = GetTotal(m_CreditSelectedList, creditplcAmount);
		SetDlgItemText (IDC_CREDIT_SEL, FormatCurrencyForInterface(cyTmp) );
		SetDlgItemText (IDC_TOTAL_SEL, FormatCurrencyForInterface(cyTotal) );

	} NxCatchAll("Error in PracBanking::UpdateSelectedTotals()");
}

COleCurrency PracBanking::GetTotal(NXDATALISTLib::_DNxDataListPtr nxdl, short nAmountColumnIndex)
{
	// (j.jones 2005-04-07 13:54) - PLID 16177 - we used to calculate the total by adding up the "Amount" field
	// in the datalist itself, which froze Practice if the list was gigantic. This is far faster.
	// (z.manning, 03/21/2007) - PLID 25294 - I could not disagree more. I tested this on a database with a ton
	// of payments where the highest list had over 10,000 rows and this still only took 16 ms. So, I
	// reimplemented it this way (though I go through the rows using the enum datalist functions which weren't
	// used before, so maybe that's the difference).

	COleCurrency cyTotal;
	cyTotal = COleCurrency(0,0);

	long nRow = nxdl->GetFirstRowEnum();
	LPDISPATCH lpDisp = NULL;
	while(nRow) 
	{
		nxdl->GetNextRowEnum(&nRow, &lpDisp);
		IRowSettingsPtr pRow(lpDisp); 
		lpDisp->Release();

		cyTotal += VarCurrency(pRow->GetValue(nAmountColumnIndex), COleCurrency(0,0));
	}

	return cyTotal;
}

// (j.jones 2008-05-07 17:33) - PLID 25338 - ensures all payments are moved to the unselected lists
void PracBanking::UnselectAllPayments()
{
	try {

		m_CashAvailList->TakeAllRows(m_CashSelectedList);
		m_CheckAvailList->TakeAllRows(m_CheckSelectedList);
		m_CreditAvailList->TakeAllRows(m_CreditSelectedList);

	}NxCatchAll("Error in PracBanking::UnselectAllPayments.");	
}

// (j.jones 2008-05-08 16:01) - PLID 25338 - renamed OnClear() to OnBtnDeposit()
void PracBanking::OnBtnDeposit() 
{
	try  {

		if(m_CashSelectedList->GetRowCount() == 0 &&
			m_CheckSelectedList->GetRowCount() == 0 &&
			m_CreditSelectedList->GetRowCount() == 0) {

			AfxMessageBox("There are no payments in the 'Selected' lists.");
			return;
		}

		// (j.jones 2009-08-03 10:33) - PLID 33036 - added a permission
		if(!CheckCurrentUserPermissions(bioBankingTab, sptDynamic0)) {
			return;
		}

		if(MessageBox("This will deposit the payments currently in the 'Selected' lists. Are you ready to do this?","Practice",MB_ICONQUESTION|MB_YESNO) == IDNO) {
			return;
		}

		COleDateTime dtNow = COleDateTime::GetCurrentTime();
		COleDateTime dtDeposit = m_dtDeposit.GetValue();
		//JMJ - keep the current time on their specified deposit date
		//it makes filtering easier later on
		dtDeposit.SetDateTime(dtDeposit.GetYear(),dtDeposit.GetMonth(),dtDeposit.GetDay(),dtNow.GetHour(),dtNow.GetMinute(),dtNow.GetSecond());

		// (j.jones 2008-05-08 16:02) - PLID 25338 - we need to build a list of Payment IDs, Batch Payment IDs,
		// and Payment Tip IDs from the selected lists, and update those IDs specifically

		//fill our lists of selected & unselected values (though right now we only need the selected values)
		CacheSelectionStates();

		//populate our ID lists

		CString strPaymentIDs, strBatchPaymentIDs, strPaymentTipIDs;		
		
		AppendArrayIntoString(m_aryCashSelectedPaymentIDs, strPaymentIDs);
		AppendArrayIntoString(m_aryCheckSelectedPaymentIDs, strPaymentIDs);
		AppendArrayIntoString(m_aryCreditSelectedPaymentIDs, strPaymentIDs);

		AppendArrayIntoString(m_aryCheckSelectedBatchPaymentIDs, strBatchPaymentIDs);

		AppendArrayIntoString(m_aryCashSelectedPaymentTipIDs, strPaymentTipIDs);
		AppendArrayIntoString(m_aryCheckSelectedPaymentTipIDs, strPaymentTipIDs);
		AppendArrayIntoString(m_aryCreditSelectedPaymentTipIDs, strPaymentTipIDs);

		//these execute statements can't be parameterized due to the IN clauses,
		//but we can batch them in bulk, and thus not assume success unless everything
		//was deposited

		CString strSqlBatch = BeginSqlBatch();
		
		AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @dtDepositDate datetime");
		AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @dtDepositInputDate datetime");
		AddStatementToSqlBatch(strSqlBatch, "SET @dtDepositDate = '%s'", FormatDateTimeForSql(dtDeposit, dtoDateTime));
		AddStatementToSqlBatch(strSqlBatch, "SET @dtDepositInputDate = '%s'", FormatDateTimeForSql(dtNow, dtoDateTime));

		//payments
		if(!strPaymentIDs.IsEmpty()) {
			//can't be parameterized due to the IN clause
			AddStatementToSqlBatch(strSqlBatch, "UPDATE PaymentsT SET Deposited = 1, DepositDate = @dtDepositDate, DepositInputDate = @dtDepositInputDate "
				"WHERE ID IN (%s) AND Deposited = 0 AND BatchPaymentID Is Null", strPaymentIDs);
		}

		//batch payments (and their children - children first)
		if(!strBatchPaymentIDs.IsEmpty()) {
			AddStatementToSqlBatch(strSqlBatch, "UPDATE PaymentsT SET Deposited = 1, DepositDate = @dtDepositDate, DepositInputDate = @dtDepositInputDate "
				"WHERE BatchPaymentID IN (SELECT ID FROM BatchPaymentsT WHERE ID IN (%s) AND Deposited = 0)", strBatchPaymentIDs);
			AddStatementToSqlBatch(strSqlBatch, "UPDATE BatchPaymentsT SET Deposited = 1, DepositDate = @dtDepositDate, DepositInputDate = @dtDepositInputDate "
				"WHERE ID IN (%s) AND Deposited = 0", strBatchPaymentIDs);
		}
		
		//tips
		if(!strPaymentTipIDs.IsEmpty()) {
			AddStatementToSqlBatch(strSqlBatch, "UPDATE PaymentTipsT SET Deposited = 1, DepositDate = @dtDepositDate, DepositInputDate = @dtDepositInputDate "
				"WHERE ID IN (%s) AND Deposited = 0", strPaymentTipIDs);
		}
		
		// (z.manning, 08/15/2007) - PLID 26069 - Tips added into check or charge payments are not split out 
		// and the include tips option has no effect on them, so we need to mark them deposited no matter what.
		if(!strPaymentIDs.IsEmpty()) {
			//TES 4/16/2015 - PLID 65613 - Added the refund-type paymethods
			AddStatementToSqlBatch(strSqlBatch, 
				"UPDATE PaymentTipsT SET Deposited = 1, DepositDate = @dtDepositDate, DepositInputDate = @dtDepositInputDate "
				"WHERE PaymentID IN (%s) "
				"AND Deposited = 0 AND PayMethod IN (2,3,8,9) "
				"AND PayMethod = (SELECT PayMethod FROM PaymentsT WHERE PaymentsT.ID = PaymentTipsT.PaymentID) ", strPaymentIDs);
		}

		ExecuteSqlBatch(strSqlBatch);

		//auditing
		long nAuditID = BeginNewAuditEvent();
		AuditEvent(-1, "", nAuditID, aeiDepositMade, -1, "", "Deposited Items", aepHigh, aetChanged);

		//now clear our cached values
		ClearCachedValues();

		m_CashSelectedList->Clear();
		m_CheckSelectedList->Clear();
		m_CreditSelectedList->Clear();

		// (j.jones 2008-05-08 14:25) - PLID 25338 - changed the UpdateTotals() call to just UpdateSelectedTotals()
		UpdateSelectedTotals();

		// (j.jones 2007-03-13 08:58) - PLID 25118 - send a tablechecker to inform others that we restored
		m_DepositedPaymentsChecker.Refresh();

	} NxCatchAll("Error in PracBanking::OnBtnDeposit");
}

// (j.jones 2008-05-08 16:09) - PLID 25338 - utility function used by OnBtnDeposit(), will
// convert a given array into a comma-delimited string
void PracBanking::AppendArrayIntoString(CArray<long, long> &aryIDs, CString &strIDs)
{
	for(int i=0;i<aryIDs.GetSize();i++) {
		long nID = (long)(aryIDs.GetAt(i));
		if(!strIDs.IsEmpty()) {
			strIDs += ",";
		}
		strIDs += AsString(nID);
	}
}

void PracBanking::OnDblClickCellCashAvailList(long nRowIndex, short nColIndex) 
{
	OnCashSelectOne();
}

void PracBanking::OnDblClickCellCashSelectedList(long nRowIndex, short nColIndex) 
{
	OnCashUnSelectOne();
}

void PracBanking::OnDblClickCellCheckAvailList(long nRowIndex, short nColIndex) 
{
	OnCheckSelectOne();
}

void PracBanking::OnDblClickCellCheckSelectedList(long nRowIndex, short nColIndex) 
{
	OnCheckUnSelectOne();
}

void PracBanking::OnDblClickCellCreditAvailList(long nRowIndex, short nColIndex) 
{
	OnCreditSelectOne();
}

void PracBanking::OnDblClickCellCreditSelectedList(long nRowIndex, short nColIndex) 
{
	OnCreditUnSelectOne();
}

void PracBanking::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	if(g_pLicense->GetHasQuickBooks() && GetRemotePropertyInt("DisableQuickBooks",0,0,"<None>",TRUE) == 0)
		m_btnSendToQBooks.ShowWindow(SW_SHOW);
	else
		m_btnSendToQBooks.ShowWindow(SW_HIDE);

	if(m_ProviderChecker.Changed()) {
		//DRT 2/12/04 - PLID 6373 - Requery if the table checker changes, might be an inactive provider.
		_variant_t var = m_ProvSelect->GetValue(m_ProvSelect->GetCurSel(), 0);	//save old value

		//requery
		m_ProvSelect->Requery();

		//add "no prov" row
		IRowSettingsPtr pRow = m_ProvSelect->GetRow(-1);
		pRow->PutValue(0,(long)-1);
		pRow->PutValue(1,_bstr_t(" {No Provider}"));
		m_ProvSelect->InsertRow(pRow, 0);

		//set the value again
		if(m_ProvSelect->SetSelByColumn(0, var) == -1)
			m_ProvSelect->PutCurSel(0);
	}

	if(m_LocationChecker.Changed()) {
		_variant_t var = m_LocSelect->GetValue(m_LocSelect->GetCurSel(), 0);	//save old value

		//requery
		m_LocSelect->Requery();

		//set the value again
		if(m_LocSelect->SetSelByColumn(0, var) == -1)
			m_LocSelect->PutCurSel(0);
	}

	if(m_PaymentGroupChecker.Changed()) {

		_variant_t var = m_CategoryCombo->GetValue(m_CategoryCombo->GetCurSel(), 0);	//save old value

		//requery
		m_CategoryCombo->Requery();

		// (r.gonet 2010-09-13 12:40) - PLID 37458 - Add a multiple categories option to the categories combo box
		IRowSettingsPtr pRow = m_CategoryCombo->GetRow(-1);
		pRow->PutValue(0, (long)MULTIPLE_CATEGORIES);
		pRow->PutValue(1, _bstr_t(" {Multiple Categories}"));
		m_CategoryCombo->InsertRow(pRow, 0);
		//add "no prov" row
		pRow = m_CategoryCombo->GetRow(-1);
		pRow->PutValue(0,(long)0);
		pRow->PutValue(1,_bstr_t(" {No Category}"));
		m_CategoryCombo->InsertRow(pRow, 0);

		//set the value again
		if(m_CategoryCombo->SetSelByColumn(0, var) == -1)
			m_CategoryCombo->PutCurSel(0);
	}

	// (j.jones 2011-06-16 13:11) - PLID 42038 - added user checker
	if(m_UserChecker.Changed()) {

		_variant_t var = g_cvarNull; 
		
		IRowSettingsPtr pOldRow = m_UserSelect->GetCurSel();
		if(pOldRow) {
			var = pOldRow->GetValue(uccID);
		}

		//requery
		m_UserSelect->Requery();

		//set the value again
		if(var.vt == VT_NULL || m_UserSelect->SetSelByColumn(uccID, var) == NULL) {
			m_UserSelect->SetSelByColumn(uccID, GetCurrentUserID());
		}
	}

	// (j.jones 2009-08-03 10:47) - PLID 33036 - if the user does not have permission to deposit, disable the button
	BOOL bCanDeposit = (GetCurrentUserPermissions(bioBankingTab) & (sptDynamic0|sptDynamic0WithPass));
	m_btnDepositItems.EnableWindow(bCanDeposit);

	// (j.jones 2008-05-09 14:11) - PLID 25338 - this will cache all the selected IDs,
	// then clear all lists, requery, re-select the previously selected IDs
	// if they are still available, and then apply filters
	RefreshLists();
}

void PracBanking::OnRequeryFinishedCashavaillist(short nFlags) 
{
	m_bCashUnselectedDone = TRUE;
	if(m_bCashUnselectedDone && m_bCheckUnselectedDone && m_bCreditUnselectedDone) {
		// (j.jones 2008-05-08 14:25) - PLID 25338 - changed the UpdateTotals() call to just UpdateAvailTotals()
		UpdateAvailTotals();

		//and now filter any other payments (simply moves from left to right, not the reverse)
		FilterAllPayments();

		//reselect the previously selected payments, unselect previously unselected ones
		RestoreCachedSelections();
	}
}

void PracBanking::OnRequeryFinishedCheckavaillist(short nFlags) 
{
	m_bCheckUnselectedDone = TRUE;
	if(m_bCashUnselectedDone && m_bCheckUnselectedDone && m_bCreditUnselectedDone) {
		// (j.jones 2008-05-08 14:25) - PLID 25338 - changed the UpdateTotals() call to just UpdateAvailTotals()
		UpdateAvailTotals();

		//and now filter any other payments (simply moves from left to right, not the reverse)
		FilterAllPayments();

		//reselect the previously selected payments, unselect previously unselected ones
		RestoreCachedSelections();
	}
}

void PracBanking::OnRequeryFinishedCreditavaillist(short nFlags) 
{
	m_bCreditUnselectedDone = TRUE;
	if(m_bCashUnselectedDone && m_bCheckUnselectedDone && m_bCreditUnselectedDone) {
		// (j.jones 2008-05-08 14:25) - PLID 25338 - changed the UpdateTotals() call to just UpdateAvailTotals()
		UpdateAvailTotals();

		//and now filter any other payments (simply moves from left to right, not the reverse)
		FilterAllPayments();

		//reselect the previously selected payments, unselect previously unselected ones
		RestoreCachedSelections();
	}
}

void PracBanking::OnAlldates() 
{
	if (m_AllDates.GetCheck()) {
		m_from.EnableWindow(FALSE);
		m_to.EnableWindow(FALSE);
		m_radioInputDate.EnableWindow(FALSE);
		m_radioServiceDate.EnableWindow(FALSE);


		if (m_AllDates.GetCheck() || !m_bAllSelected)
		{
			// (j.jones 2008-05-07 17:09) - PLID 25338 - changed the filtering behavior of this dialog
			FilterAllPayments();
		}
	}
}

void PracBanking::OnDaterange() 
{
	m_bAllSelected = FALSE;
	m_from.EnableWindow(TRUE);
	m_to.EnableWindow(TRUE);
	m_from.SetFocus();

	m_radioInputDate.EnableWindow(TRUE);
	m_radioServiceDate.EnableWindow(TRUE);

	// (j.jones 2008-05-07 17:09) - PLID 25338 - changed the filtering behavior of this dialog
	UnselectAllPayments();
	FilterAllPayments();
}

void PracBanking::OnAllproviders() 
{
	// (j.jones 2008-05-07 17:09) - PLID 25338 - changed the filtering behavior of this dialog
	UnselectAllPayments();
	FilterAllPayments();

	GetDlgItem(IDC_PROVSELECTOR)->EnableWindow(FALSE);
}

void PracBanking::OnSelectProviderRadio() 
{
	GetDlgItem(IDC_PROVSELECTOR)->EnableWindow(TRUE);
	// (j.jones 2008-05-07 17:09) - PLID 25338 - changed the filtering behavior of this dialog
	UnselectAllPayments();
	FilterAllPayments();
}

void PracBanking::OnChangeFrom(NMHDR* pNMHDR, LRESULT* pResult)
{
	if(m_DateRange.GetCheck() && COleDateTime(m_from.GetValue()) > COleDateTime(m_to.GetValue())) {
		AfxMessageBox("Your 'from' date is after your 'to' date.\n"
			"Please correct the date range.");
		//go ahead and requery, because the tab will do it sooner or later anyways
	}

	// (j.jones 2008-05-07 17:09) - PLID 25338 - changed the filtering behavior of this dialog
	UnselectAllPayments();
	FilterAllPayments();

	*pResult = 0;
}

void PracBanking::OnChangeTo(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if(m_DateRange.GetCheck() && COleDateTime(m_from.GetValue()) > COleDateTime(m_to.GetValue())) {
		AfxMessageBox("Your 'from' date is after your 'to' date.\n"
			"Please correct the date range.");
		//go ahead and requery, because the tab will do it sooner or later anyways
	}

	// (j.jones 2008-05-07 17:09) - PLID 25338 - changed the filtering behavior of this dialog
	UnselectAllPayments();
	FilterAllPayments();

	*pResult = 0;
}

void PracBanking::OnRadioInputDate() 
{
	// (j.jones 2008-05-07 17:09) - PLID 25338 - changed the filtering behavior of this dialog
	UnselectAllPayments();
	FilterAllPayments();
}

void PracBanking::OnRadioServiceDate() 
{
	// (j.jones 2008-05-07 17:09) - PLID 25338 - changed the filtering behavior of this dialog
	UnselectAllPayments();
	FilterAllPayments();
}

HBRUSH PracBanking::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	/*
	//to avoid drawing problems with transparent text and disabled ites
	//override the NxDialog way of doing text with a non-grey background
	//NxDialog relies on the NxColor to draw the background, then draws text transparently
	//instead, we actually color the background of the STATIC text

	if(nCtlColor == CTLCOLOR_STATIC) {
		if (pWnd->GetDlgCtrlID() == IDC_RADIO_INPUT_DATE
			|| pWnd->GetDlgCtrlID() == IDC_RADIO_SERVICE_DATE) {
			extern CPracticeApp theApp;
			pDC->SelectPalette(&theApp.m_palette, FALSE);
			pDC->RealizePalette();
			pDC->SetBkColor(PaletteColor(0x00D1B8AF));
			return m_brush;
		}		
	}
	*/
	
	// (a.walling 2008-04-01 16:47) - PLID 29497 - Deprecated; use parent class' implementation
	return CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}

void PracBanking::OnAlllocations() 
{
	// (j.jones 2008-05-07 17:09) - PLID 25338 - changed the filtering behavior of this dialog
	UnselectAllPayments();
	FilterAllPayments();

	GetDlgItem(IDC_LOCSELECTOR)->EnableWindow(FALSE);
}

void PracBanking::OnSelectLocationRadio() 
{
	GetDlgItem(IDC_LOCSELECTOR)->EnableWindow(TRUE);
	// (j.jones 2008-05-07 17:09) - PLID 25338 - changed the filtering behavior of this dialog
	UnselectAllPayments();
	FilterAllPayments();
}

void PracBanking::OnSelChosenProvselector(long nRow) 
{
	// (j.jones 2008-05-07 17:09) - PLID 25338 - changed the filtering behavior of this dialog
	UnselectAllPayments();
	FilterAllPayments();
}

void PracBanking::OnSelChosenLocselector(long nRow) 
{
	// (j.jones 2008-05-07 17:09) - PLID 25338 - changed the filtering behavior of this dialog
	UnselectAllPayments();
	FilterAllPayments();
}

void PracBanking::OnSendToQuickbooks() 
{
	if(!CheckCurrentUserPermissions(bioQuickbooksLink,sptView))
		return;

	try {		

		if(m_CashSelectedList->GetRowCount() == 0 &&
			m_CheckSelectedList->GetRowCount() == 0 &&
			m_CreditSelectedList->GetRowCount() == 0) {

			AfxMessageBox("There are no payments in the 'Selected' lists.");
			return;
		}

		//0 - Make Deposits, 1 - Receive Payments
		int nResult = GetRemotePropertyInt("QuickBooksExportStyle",0,0,"<None>",TRUE);

		CWaitCursor pWait1;

		// Open the qb connection
		IQBSessionManagerPtr qb = QB_OpenSession();

		if(qb == NULL)
			return;

		CString strLatestVersion = QB_GetQBFCLatestVersion(qb);
		strLatestVersion.Replace("CA","");
		if(strLatestVersion < "2.0") {
			//Must be 2.0 (QB 2003) or higher to use the deposits
			nResult = 1;
			SetRemotePropertyInt("QuickBooksExportStyle",1,0,"<None>");
		}
		qb->EndSession();

		CWaitCursor pWait2;

		if(nResult == 0) {
			SendDepositToQuickBooks();
		}
		else if(nResult == 1) {
			SendPaymentsToQuickBooks();
		}
	
	}NxCatchAll("Error sending to QuickBooks.");
}

void PracBanking::SendDepositToQuickBooks()
{
	/****************************************************************************
	* This function replicates the "Make Deposits" functionality in QuickBooks. *
	****************************************************************************/

	IQBSessionManagerPtr qb;

	try {

		CString strMsg;

		int nResult = GetRemotePropertyInt("QBDepositWithPatients",0,0,"<None>",TRUE);
		BOOL bExportPatients = ((nResult == 1) ? TRUE : FALSE);

		strMsg = "This action will deposit all selected payments to Quickbooks.\n\n";

		//0 - do not send over patients, 1 - do send over patients
		if(bExportPatients) {
			strMsg += "Any patients that do not exist in QuickBooks will be created.\n\n";
		}

		strMsg += "Are you sure you wish to do this?";

		if(IDNO == MessageBox(strMsg,"Practice",MB_ICONQUESTION|MB_YESNO))
			return;

		BOOL bSkipExisting = FALSE;

		// (j.jones 2008-05-09 15:35) - PLID 25338 - changed to work solely off of the datalists,
		// so we'll cache the IDs, turn them into comma-delimited strings, and export those IDs to QB

		//fill our lists of selected & unselected values (though right now we only need the selected values)
		CacheSelectionStates();

		//populate our ID lists

		CString strPaymentIDs, strBatchPaymentIDs, strPaymentTipIDs;
		
		AppendArrayIntoString(m_aryCashSelectedPaymentIDs, strPaymentIDs);
		AppendArrayIntoString(m_aryCheckSelectedPaymentIDs, strPaymentIDs);
		AppendArrayIntoString(m_aryCreditSelectedPaymentIDs, strPaymentIDs);

		AppendArrayIntoString(m_aryCheckSelectedBatchPaymentIDs, strBatchPaymentIDs);

		AppendArrayIntoString(m_aryCashSelectedPaymentTipIDs, strPaymentTipIDs);
		AppendArrayIntoString(m_aryCheckSelectedPaymentTipIDs, strPaymentTipIDs);
		AppendArrayIntoString(m_aryCreditSelectedPaymentTipIDs, strPaymentTipIDs);

		//after our strings are built, we do not need the cached information anymore
		ClearCachedValues();

		//if any have been previously sent, ask the user what to do
		if((!strPaymentIDs.IsEmpty() && ReturnsRecords("SELECT PaymentsT.ID FROM PaymentsT INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
			"WHERE PaymentsT.ID IN (%s) AND Deposited = 0 AND Deleted = 0 AND BatchPaymentID Is Null AND Amount <> 0 AND SentToQB = 1", strPaymentIDs))
			|| (!strPaymentTipIDs.IsEmpty() && ReturnsRecords("SELECT PaymentTipsT.ID FROM PaymentTipsT INNER JOIN LineItemT ON PaymentTipsT.PaymentID = LineItemT.ID "
			"WHERE PaymentTipsT.ID IN (%s) AND Deposited = 0 AND Deleted = 0 AND PaymentTipsT.Amount <> 0 AND SentToQB = 1", strPaymentTipIDs))
			|| (!strBatchPaymentIDs.IsEmpty() && ReturnsRecords("SELECT ID FROM BatchPaymentsT WHERE BatchPaymentsT.ID IN (%s) AND Deposited = 0 AND Deleted = 0 AND Amount <> 0 AND SentToQB = 1", strBatchPaymentIDs))) {
			if(IDYES == MessageBox("Some of the payments you are exporting have already been sent to QuickBooks before.\n"
				"Do you want to skip these payments?\n\n"
				"'Yes' will export only payments that have not previously been exported.\n"
				"'No' will export all selected payments.\n","Practice",MB_ICONQUESTION|MB_YESNO)) {
				bSkipExisting = TRUE;
			}
				
		}

		long CountOfPayments = 0, CountOfPatientsCreated = 0,
			CountOfPaymentsSkipped = 0,	CountOfSentPayments = 0;

		CWaitCursor pWait1;

		// Open the qb connection
		qb = QB_OpenSession();

		if(qb == NULL) {
			return;
		}

		//Choose the account to identify these payments as coming FROM
		CString strFromAccount, strToAccount;

		//check defaults first
		strFromAccount = GetRemotePropertyText("QBDefaultDepositSourceAcct","-1",0,"<None>",TRUE);
		strToAccount = GetRemotePropertyText("QBDefaultDepositDestAcct","-1",0,"<None>",TRUE);

		long QBPromptSourceAcct= GetRemotePropertyInt("QBPromptSourceAcct",0,0,"<None>",TRUE);

		if(QBPromptSourceAcct == 0 && (strFromAccount == "-1" || strToAccount == "-1")) {

			// Choose the "from" and "to" accounts
			if (!QB_ChooseDepositAccounts(qb, strFromAccount, strToAccount, this)) {
				// The user canceled, so don't proceed
				qb->EndSession();
				return;
			}
		}

		CWaitCursor pWait2;
		
		CPtrArray aryPaymentIDs;
		CPtrArray aryBatchPaymentIDs;
		CPtrArray aryPaymentTipIDs;
		CPtrArray aryRefundIDs;
		CPtrArray aryBatchPaymentRefundIDs;

		CString strSkipExisting = "";
		if(bSkipExisting)
			strSkipExisting = " AND SentToQB = 0";

		//we need to make sure the deposit total is greater than zero
		COleCurrency cyDepositTotal = COleCurrency(0,0);
		
		// (j.jones 2008-05-09 15:45) - PLID 25338 - now uses our comma-delimited strings of IDs, and not CurrentSelect

		if(!strPaymentIDs.IsEmpty()) {

			CString strSql;
			// (j.jones 2008-05-12 11:26) - PLID 30007 - ensured that payments always included their tips in the total,
			// regardless of the "Show Tips" setting, and only if the payment and tip methods are the same, but not cash
			// (r.gonet 2015-05-05 14:38) - PLID 65870 - Exclude Gift Certificate Refunds
			strSql.Format("SELECT PaymentsT.ID, LineItemT.PatientID, QBooksAcctID, PaymentsT.PayMethod, PaymentPlansT.CheckNo, "
				"LineItemT.Amount + (SELECT CASE WHEN Sum(Amount) IS NULL THEN 0 ELSE Sum(Amount) END FROM PaymentTipsT WHERE PaymentID = PaymentsT.ID AND PaymentTipsT.PayMethod = PaymentsT.PayMethod AND PaymentTipsT.PayMethod <> 1) AS Amount "
				"FROM PaymentsT "
				"INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
				"INNER JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
				"LEFT JOIN QBooksAcctsToProvidersT ON PaymentsT.ProviderID = QBooksAcctsToProvidersT.ProviderID "
				"WHERE PaymentsT.ID IN (%s) AND Deposited = 0 AND Deleted = 0 "
				"AND PaymentsT.BatchPaymentID Is Null AND LineItemT.Type = 1 AND PaymentsT.PayMethod NOT IN (4,10) %s", strPaymentIDs, strSkipExisting);

			if (IsDlgButtonChecked(IDC_INCLUDE_REFUNDS_CHECK)) {
				strSql.Replace("LineItemT.Type = 1", "(LineItemT.Type = 1 OR LineItemT.Type = 3)");
			}

			//regular payments
			_RecordsetPtr rs = CreateRecordsetStd(strSql);
			while(!rs->eof) {

				//add the payment to the array
				long nPaymentID = AdoFldLong(rs, "ID");
				long nPatientID = AdoFldLong(rs, "PatientID");

				//first check to see that the Check No # is not more than 11 characters
				long nPayMethod = AdoFldLong(rs, "PayMethod",-1);
				if(nPayMethod == 2 || nPayMethod == 8) {
					//if a check
					CString strCheckNo = AdoFldString(rs, "CheckNo","");
					if(strCheckNo.GetLength() > 11) {
						//it can't be more than 11 characters, so report the offending payment and cancel
						_RecordsetPtr rsPay = CreateRecordset("SELECT Last, First, Date, Amount FROM LineItemT "
							"INNER JOIN PersonT ON LineItemT.PatientID = PersonT.ID WHERE LineItemT.ID = %li", nPaymentID);
						if(!rsPay->eof) {
							CString str;
							str.Format("The %s check payment for patient '%s, %s' made on %s\n"
								"has a check number of %s, which is too long for QuickBooks.\n\n"
								"QuickBooks only supports check numbers up to 11 characters long.\n\n"
								"The deposit will be cancelled.",
								FormatCurrencyForInterface(AdoFldCurrency(rsPay, "Amount"),TRUE,TRUE),
								AdoFldString(rsPay, "Last",""),
								AdoFldString(rsPay, "First",""),
								FormatDateTimeForInterface(AdoFldDateTime(rsPay, "Date"),NULL,dtoDate),
								strCheckNo);
							AfxMessageBox(str);
						}
						rsPay->Close();
						qb->EndSession();
						return;
					}
				}

				QBDepositInfo *pPaymentInfo = new QBDepositInfo;

				pPaymentInfo->nPaymentID = nPaymentID;
				pPaymentInfo->nPatientID = nPatientID;
				pPaymentInfo->strDepositFromAccountListID = strFromAccount;

				COleCurrency cyAmount = AdoFldCurrency(rs, "Amount");
				cyDepositTotal += cyAmount;

				if(cyAmount < COleCurrency(0,0))
					// (j.jones 2005-02-01 14:22) - have to store refund IDs separately, as they have to be sent last
					aryRefundIDs.Add(pPaymentInfo);
				else
					aryPaymentIDs.Add(pPaymentInfo);

				rs->MoveNext();
			}
			rs->Close();
		}

		if(bSkipExisting)
			strSkipExisting = " AND PaymentTipsT.SentToQB = 0";


		BOOL bWarnedCCTip = FALSE;

		//tips
		if(!strPaymentTipIDs.IsEmpty() && GetRemotePropertyInt("BankingIncludeTips", 0, 0, GetCurrentUserName(), true) == 1) {
			// (j.jones 2008-05-09 15:45) - PLID 25338 - now uses our comma-delimited strings of IDs, and not CurrentSelect
			// (j.jones 2008-05-12 11:27) - PLID 30007 - cash tips are always separated
			_RecordsetPtr rs = CreateRecordset("SELECT PaymentTipsT.ID, LineItemT.PatientID, PaymentTipsT.Amount, QBooksAcctID "
				"FROM PaymentsT "
				"INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
				"INNER JOIN PaymentTipsT ON PaymentsT.ID = PaymentTipsT.PaymentID "
				"LEFT JOIN QBooksAcctsToProvidersT ON PaymentsT.ProviderID = QBooksAcctsToProvidersT.ProviderID "
				"WHERE PaymentTipsT.ID IN (%s) AND PaymentTipsT.Deposited = 0 AND Deleted = 0 "
				"AND PaymentsT.BatchPaymentID Is Null AND LineItemT.Type = 1 AND PaymentTipsT.Amount <> 0 "
				"AND (PaymentTipsT.PayMethod = 1 OR PaymentTipsT.PayMethod <> PaymentsT.PayMethod) %s", strPaymentTipIDs, strSkipExisting);
			while(!rs->eof) {

				//add the payment to the array
				long nPaymentTipID = AdoFldLong(rs, "ID");
				long nPatientID = AdoFldLong(rs, "PatientID");

				if(ReturnsRecords("SELECT PaymentTipsT.ID FROM PaymentTipsT INNER JOIN PaymentsT ON PaymentTipsT.PaymentID = PaymentsT.ID "
					"WHERE PaymentTipsT.ID = %li AND PaymentTipsT.PayMethod = 3 AND PaymentsT.PayMethod <> 3", nPaymentTipID)) {
					if(!bWarnedCCTip) {
						AfxMessageBox("At least one tip payment could not be exported, because the tip is identified as being from a credit card, \n"
							"and the payment is identified as being a cash or check. Quickbooks will not accept credit card line items \n"
							"without a credit card type associated with them.");
						bWarnedCCTip = TRUE;
					}
					rs->MoveNext();
					continue;
				}

				QBDepositInfo *pPaymentTipInfo = new QBDepositInfo;

				pPaymentTipInfo->nPaymentID = nPaymentTipID;
				pPaymentTipInfo->nPatientID = nPatientID;
				pPaymentTipInfo->strDepositFromAccountListID = strFromAccount;

				cyDepositTotal += AdoFldCurrency(rs, "Amount");

				aryPaymentTipIDs.Add(pPaymentTipInfo);

				rs->MoveNext();
			}
			rs->Close();
		}

		if(bSkipExisting)
			strSkipExisting = " AND SentToQB = 0";

		//batch payments

		// (j.jones 2008-05-09 15:45) - PLID 25338 - now uses our comma-delimited strings of IDs, and not CurrentSelect

		if(!strBatchPaymentIDs.IsEmpty()) {

			CString strSql;
			strSql.Format("SELECT BatchPaymentsT.ID, CheckNo, CASE WHEN Type <> 1 THEN -(Amount) ELSE Amount END AS Amount "
				"FROM BatchPaymentsT "
				"LEFT JOIN QBooksAcctsToProvidersT ON BatchPaymentsT.ProviderID = QBooksAcctsToProvidersT.ProviderID "
				"WHERE BatchPaymentsT.ID IN (%s) AND Deposited = 0 AND Deleted = 0 AND BatchPaymentsT.Amount <> 0 %s AND Type <> 2",
				strBatchPaymentIDs, strSkipExisting);

			if (!IsDlgButtonChecked(IDC_INCLUDE_REFUNDS_CHECK)) {
				strSql += " AND Type = 1 ";
			}

			_RecordsetPtr rs = CreateRecordsetStd(strSql);
			while(!rs->eof) {

				//add the batch payment to the array
				long BatchPaymentID = AdoFldLong(rs, "ID");

				CString strCheckNo = AdoFldString(rs, "CheckNo","");
				if(strCheckNo.GetLength() > 11) {
					//it can't be more than 11 characters, so report the offending payment and cancel
					_RecordsetPtr rsPay = CreateRecordset("SELECT Name, Date, CASE WHEN Type <> 1 THEN -(Amount) ELSE Amount END AS Amount FROM BatchPaymentsT "
						"INNER JOIN InsuranceCoT ON BatchPaymentsT.InsuranceCoID = InsuranceCoT.PersonID WHERE BatchPaymentsT.ID = %li", BatchPaymentID);
					if(!rsPay->eof) {
						CString str;
						str.Format("The %s batch payment for insurance company '%s' made on %s\n"
							"has a check number of %s, which is too long for QuickBooks.\n\n"
							"QuickBooks only supports check numbers up to 11 characters long.\n\n"
							"The deposit will be cancelled.",
							FormatCurrencyForInterface(AdoFldCurrency(rsPay, "Amount"),TRUE,TRUE),
							AdoFldString(rsPay, "Name",""),
							FormatDateTimeForInterface(AdoFldDateTime(rsPay, "Date"),NULL,dtoDate),
							strCheckNo);
						AfxMessageBox(str);
					}
					rsPay->Close();
					qb->EndSession();
					return;
				}

				QBDepositInfo *pBatchPaymentInfo = new QBDepositInfo;

				pBatchPaymentInfo->nPaymentID = BatchPaymentID;
				pBatchPaymentInfo->nPatientID = -1; //batch payments don't have patients
				pBatchPaymentInfo->strDepositFromAccountListID = strFromAccount;

				COleCurrency cyBatchPayAmt = AdoFldCurrency(rs, "Amount");
				cyDepositTotal += cyBatchPayAmt;

				if(cyBatchPayAmt < COleCurrency(0,0)) 
					aryBatchPaymentRefundIDs.Add(pBatchPaymentInfo);
				else
					aryBatchPaymentIDs.Add(pBatchPaymentInfo);

				rs->MoveNext();
			}
			rs->Close();
		}

		if(aryPaymentIDs.GetSize() == 0 && aryBatchPaymentIDs.GetSize() == 0 && aryPaymentTipIDs.GetSize() == 0 && aryRefundIDs.GetSize() == 0 && aryBatchPaymentRefundIDs.GetSize() == 0) {
			CString str;
			str.Format("No deposit was made, because no exportable payments were selected.\n\n"
				"This can happen if the selected payments are all %s%s.",FormatCurrencyForInterface(COleCurrency(0,0),TRUE,TRUE),
				bSkipExisting ? ", or if the payments had all been previously sent." : "");
			AfxMessageBox(str);
			qb->EndSession();
			return;
		}

		if(cyDepositTotal < COleCurrency(0,0)) {
			CString str;
			str.Format("No deposit was made, because the total amount to be deposited is negative. QuickBooks does not allow a negative deposit to be entered.\n"
				"Negative amounts (such as refunds) can only be sent if the total deposit is not negative.\n\n"
				"Your deposit total of payments exportable to QuickBooks was: %s",
				FormatCurrencyForInterface(cyDepositTotal,TRUE,TRUE));
			AfxMessageBox(str);
			qb->EndSession();
			return;
		}

		//the following screen will allow the user to enter different source accounts per payment
		if(QBPromptSourceAcct == 1) {
			CQBDepositAccountsDlg dlg(this);
			dlg.m_paryPaymentIDs = &aryPaymentIDs;
			// (j.jones 2009-02-18 09:03) - PLID 33136 - added refund support
			dlg.m_paryRefundIDs = &aryRefundIDs;
			dlg.m_paryBatchPaymentIDs = &aryBatchPaymentIDs;
			dlg.m_paryPaymentTipIDs = &aryPaymentTipIDs;
			dlg.qb = qb;
			if(dlg.DoModal() == IDCANCEL) {
				qb->EndSession();
				return;
			}

			strToAccount = dlg.m_strToAccount;
		}

		//if we are exporting patients, do it now
		if(bExportPatients) {

			for(int i=0;i<aryPaymentIDs.GetSize();i++) {

				CString strCustomerListID = "";

				QBDepositInfo *pPaymentInfo = (QBDepositInfo*)aryPaymentIDs.GetAt(i);
			
				// Make sure the customer exists
				if(pPaymentInfo->nPatientID == -1) {
					ThrowNxException("Attempted to load a payment with no patient!");
				}

				CString strEditSequence = "";
				if (!QB_GetCustomerListID(qb, pPaymentInfo->nPatientID, strCustomerListID, strEditSequence)) {
					// Customer doesn't exist, so create it.
					if(!QB_CreateCustomer(qb, pPaymentInfo->nPatientID, strCustomerListID)) {
						//should be impossible to get here without a warning, so abort the export

						for(int j=aryPaymentIDs.GetSize()-1;j>=0;j--) {
							delete (QBDepositInfo*)aryPaymentIDs.GetAt(j);				
							aryPaymentIDs.RemoveAt(j);
						}

						for(j=aryRefundIDs.GetSize()-1;j>=0;j--) {
							delete (QBDepositInfo*)aryRefundIDs.GetAt(j);					
							aryRefundIDs.RemoveAt(j);
						}

						for(j=aryBatchPaymentIDs.GetSize()-1;j>=0;j--) {
							delete (QBDepositInfo*)aryBatchPaymentIDs.GetAt(j);					
							aryBatchPaymentIDs.RemoveAt(j);
						}

						for(j=aryBatchPaymentRefundIDs.GetSize()-1;j>=0;j--) {
							delete (QBDepositInfo*)aryBatchPaymentRefundIDs.GetAt(j);					
							aryBatchPaymentRefundIDs.RemoveAt(j);
						}

						for(j=aryPaymentTipIDs.GetSize()-1;j>=0;j--) {
							delete (QBDepositInfo*)aryPaymentTipIDs.GetAt(j);					
							aryPaymentTipIDs.RemoveAt(j);
						}

						return;
					}
					else {
						ExecuteSql("UPDATE PatientsT SET SentToQuickbooks = 1 WHERE PersonID = %li", pPaymentInfo->nPatientID);
						CountOfPatientsCreated++;
					}
				}

				pPaymentInfo->strCustomerListID = strCustomerListID;
			}

			for(i=0;i<aryRefundIDs.GetSize();i++) {

				CString strCustomerListID = "";

				QBDepositInfo *pPaymentInfo = (QBDepositInfo*)aryRefundIDs.GetAt(i);
			
				// Make sure the customer exists
				if(pPaymentInfo->nPatientID == -1) {
					ThrowNxException("Attempted to load a refund with no patient!");
				}

				CString strEditSequence = "";
				if (!QB_GetCustomerListID(qb, pPaymentInfo->nPatientID, strCustomerListID, strEditSequence)) {
					// Customer doesn't exist, so create it.
					if(!QB_CreateCustomer(qb, pPaymentInfo->nPatientID, strCustomerListID)) {
						//should be impossible to get here without a warning, so abort the export

						for(int j=aryPaymentIDs.GetSize()-1;j>=0;j--) {
							delete (QBDepositInfo*)aryPaymentIDs.GetAt(j);				
							aryPaymentIDs.RemoveAt(j);
						}

						for(j=aryRefundIDs.GetSize()-1;j>=0;j--) {
							delete (QBDepositInfo*)aryRefundIDs.GetAt(j);					
							aryRefundIDs.RemoveAt(j);
						}

						for(j=aryBatchPaymentIDs.GetSize()-1;j>=0;j--) {
							delete (QBDepositInfo*)aryBatchPaymentIDs.GetAt(j);					
							aryBatchPaymentIDs.RemoveAt(j);
						}

						for(j=aryBatchPaymentRefundIDs.GetSize()-1;j>=0;j--) {
							delete (QBDepositInfo*)aryBatchPaymentRefundIDs.GetAt(j);					
							aryBatchPaymentRefundIDs.RemoveAt(j);
						}

						for(j=aryPaymentTipIDs.GetSize()-1;j>=0;j--) {
							delete (QBDepositInfo*)aryPaymentTipIDs.GetAt(j);					
							aryPaymentTipIDs.RemoveAt(j);
						}

						return;
					}
					else {
						ExecuteSql("UPDATE PatientsT SET SentToQuickbooks = 1 WHERE PersonID = %li", pPaymentInfo->nPatientID);
						CountOfPatientsCreated++;
					}
				}

				pPaymentInfo->strCustomerListID = strCustomerListID;
			}

			for(i=0;i<aryPaymentTipIDs.GetSize();i++) {

				CString strCustomerListID = "";

				QBDepositInfo *pPaymentTipInfo = (QBDepositInfo*)aryPaymentTipIDs.GetAt(i);
			
				// Make sure the customer exists
				if(pPaymentTipInfo->nPatientID == -1) {
					ThrowNxException("Attempted to load a tip with no patient!");
				}

				CString strEditSequence = "";
				if (!QB_GetCustomerListID(qb, pPaymentTipInfo->nPatientID, strCustomerListID, strEditSequence)) {
					// Customer doesn't exist, so create it.
					if(!QB_CreateCustomer(qb, pPaymentTipInfo->nPatientID, strCustomerListID)) {
						//should be impossible to get here without a warning, so abort the export

						for(int j=aryPaymentIDs.GetSize()-1;j>=0;j--) {
							delete (QBDepositInfo*)aryPaymentIDs.GetAt(j);				
							aryPaymentIDs.RemoveAt(j);
						}

						for(j=aryRefundIDs.GetSize()-1;j>=0;j--) {
							delete (QBDepositInfo*)aryRefundIDs.GetAt(j);					
							aryRefundIDs.RemoveAt(j);
						}

						for(j=aryBatchPaymentIDs.GetSize()-1;j>=0;j--) {
							delete (QBDepositInfo*)aryBatchPaymentIDs.GetAt(j);					
							aryBatchPaymentIDs.RemoveAt(j);
						}

						for(j=aryBatchPaymentRefundIDs.GetSize()-1;j>=0;j--) {
							delete (QBDepositInfo*)aryBatchPaymentRefundIDs.GetAt(j);					
							aryBatchPaymentRefundIDs.RemoveAt(j);
						}

						for(j=aryPaymentTipIDs.GetSize()-1;j>=0;j--) {
							delete (QBDepositInfo*)aryPaymentTipIDs.GetAt(j);					
							aryPaymentTipIDs.RemoveAt(j);
						}

						return;
					}
					else {
						ExecuteSql("UPDATE PatientsT SET SentToQuickbooks = 1 WHERE PersonID = %li", pPaymentTipInfo->nPatientID);
						CountOfPatientsCreated++;
					}
				}

				pPaymentTipInfo->strCustomerListID = strCustomerListID;
			}
		}

		COleDateTime dtDeposit = m_dtDeposit.GetValue();

		if(QB_CreateDeposit(qb, aryPaymentIDs, aryBatchPaymentIDs, aryPaymentTipIDs, aryRefundIDs, aryBatchPaymentRefundIDs, strToAccount, bExportPatients, dtDeposit)) {

			//update the payments
			for(int i=0;i<aryPaymentIDs.GetSize();i++) {
				ExecuteParamSql("UPDATE PaymentsT SET SentToQB = 1 WHERE ID = {INT}", ((QBDepositInfo*)aryPaymentIDs.GetAt(i))->nPaymentID);
				// (j.jones 2008-05-12 11:46) - PLID 30007 - ensure that any linked tips with the same payment method are also updated (if not cash)
				ExecuteParamSql("UPDATE PaymentTipsT SET SentToQB = 1 WHERE PaymentID = {INT} "
					"AND PayMethod IN (2,3) AND PayMethod = (SELECT PayMethod FROM PaymentsT WHERE PaymentsT.ID = PaymentTipsT.PaymentID) ", ((QBDepositInfo*)aryPaymentIDs.GetAt(i))->nPaymentID);
			}
			//update the refunds
			for(i=0;i<aryRefundIDs.GetSize();i++) {
				ExecuteParamSql("UPDATE PaymentsT SET SentToQB = 1 WHERE ID = {INT}",((QBDepositInfo*)aryRefundIDs.GetAt(i))->nPaymentID);
			}
			//update the batch payments
			for(i=0;i<aryBatchPaymentIDs.GetSize();i++) {
				ExecuteParamSql("UPDATE BatchPaymentsT SET SentToQB = 1 WHERE ID = {INT}",((QBDepositInfo*)aryBatchPaymentIDs.GetAt(i))->nPaymentID);
			}
			//update the batch payment refunds
			for(i=0;i<aryBatchPaymentRefundIDs.GetSize();i++) {
				ExecuteParamSql("UPDATE BatchPaymentsT SET SentToQB = 1 WHERE ID = {INT}",((QBDepositInfo*)aryBatchPaymentRefundIDs.GetAt(i))->nPaymentID);
			}
			//update the tips
			for(i=0;i<aryPaymentTipIDs.GetSize();i++) {
				ExecuteParamSql("UPDATE PaymentTipsT SET SentToQB = 1 WHERE ID = {INT}",((QBDepositInfo*)aryPaymentTipIDs.GetAt(i))->nPaymentID);
			}

			//AfxMessageBox("All payments were successfully deposited into QuickBooks.");
			if(IDYES == MessageBox("All payments were successfully deposited into QuickBooks.\n\n"
				"Would you like to mark these payments as deposited now?","Practice",MB_ICONQUESTION|MB_YESNO)) {
				OnBtnDeposit();
			}
		}
		else {
			AfxMessageBox("An error prevented the deposit from being made. Please contact NexTech for assistance.");
		}
		
		qb->EndSession();

		for(int i=aryPaymentIDs.GetSize()-1;i>=0;i--) {
			delete (QBDepositInfo*)aryPaymentIDs.GetAt(i);				
			aryPaymentIDs.RemoveAt(i);
		}

		for(i=aryRefundIDs.GetSize()-1;i>=0;i--) {
			delete (QBDepositInfo*)aryRefundIDs.GetAt(i);					
			aryRefundIDs.RemoveAt(i);
		}

		for(i=aryBatchPaymentIDs.GetSize()-1;i>=0;i--) {
			delete (QBDepositInfo*)aryBatchPaymentIDs.GetAt(i);					
			aryBatchPaymentIDs.RemoveAt(i);
		}

		for(i=aryBatchPaymentRefundIDs.GetSize()-1;i>=0;i--) {
			delete (QBDepositInfo*)aryBatchPaymentRefundIDs.GetAt(i);					
			aryBatchPaymentRefundIDs.RemoveAt(i);
		}

		for(i=aryPaymentTipIDs.GetSize()-1;i>=0;i--) {
			delete (QBDepositInfo*)aryPaymentTipIDs.GetAt(i);					
			aryPaymentTipIDs.RemoveAt(i);
		}

	} NxCatchAllCall("Error exporting deposit to Quickbooks.", {
		if(qb != NULL)
			qb->EndSession();
		SAFE_SET_PROGRESS(pProgress, "ERROR encountered while trying to create a deposit.");
		(void)0;
	});
}

void PracBanking::SendPaymentsToQuickBooks()
{
	/*******************************************************************************
	* This function replicates the "Receive Payments" functionality in QuickBooks. *
	*******************************************************************************/

	IQBSessionManagerPtr qb;

	try {		

		if(IDNO == MessageBox("This action will send all selected payments to Quickbooks.\n\n"
			"Payments that already exist in Quickbooks will be skipped,\n"
			"and patients that do not exist in Quickbooks will be created.\n\n"
			"Are you sure you wish to do this?","Practice",MB_ICONQUESTION|MB_YESNO))
			return;

		long CountOfPayments = 0, CountOfPatientsCreated = 0,
			CountOfPaymentsSkipped = 0,	CountOfSentPayments = 0;

		CWaitCursor pWait1;

		// (j.jones 2008-05-09 15:35) - PLID 25338 - changed to work solely off of the datalists,
		// so we'll cache the IDs, turn them into comma-delimited strings, and export those IDs to QB

		//fill our lists of selected & unselected values (though right now we only need the selected values)
		CacheSelectionStates();

		//populate our ID lists

		CString strPaymentIDs = "-1", strBatchPaymentIDs = "-1", strPaymentTipIDs = "-1";
		
		AppendArrayIntoString(m_aryCashSelectedPaymentIDs, strPaymentIDs);
		AppendArrayIntoString(m_aryCheckSelectedPaymentIDs, strPaymentIDs);
		AppendArrayIntoString(m_aryCreditSelectedPaymentIDs, strPaymentIDs);

		AppendArrayIntoString(m_aryCheckSelectedBatchPaymentIDs, strBatchPaymentIDs);

		AppendArrayIntoString(m_aryCashSelectedPaymentTipIDs, strPaymentTipIDs);
		AppendArrayIntoString(m_aryCheckSelectedPaymentTipIDs, strPaymentTipIDs);
		AppendArrayIntoString(m_aryCreditSelectedPaymentTipIDs, strPaymentTipIDs);

		//after our strings are built, we do not need the cached information anymore
		ClearCachedValues();

		// Open the qb connection
		qb = QB_OpenSession();

		if(qb == NULL)
			return;

		//we have a link now between providers and deposit accounts, so if all providers from all payments have
		//been linked to a deposit account, we won't need to prompt for one

		CString strPaymentAccount = "";

		// (r.gonet 2015-05-05 14:38) - PLID 65870 - Exclude Gift Certificate Refunds
		if(!IsRecordsetEmpty("SELECT TOP 1 QBooksAcctID FROM PaymentsT "
			"INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
			"LEFT JOIN QBooksAcctsToProvidersT ON PaymentsT.ProviderID = QBooksAcctsToProvidersT.ProviderID "
			"WHERE Deposited = 0 AND Deleted = 0 AND LineItemT.Type = 1 AND PaymentsT.PayMethod NOT IN (4,10) "
			"AND ((PaymentsT.ID IN (%s) AND PaymentsT.BatchPaymentID Is Null) OR PaymentsT.BatchPaymentID IN (%s) OR PaymentsT.ID IN (SELECT PaymentID FROM PaymentTipsT WHERE ID IN (%s))) "
			"AND QBooksAcctID Is NULL", strPaymentIDs, strBatchPaymentIDs, strPaymentTipIDs)) {

			// Choose the payment account			
			if (!QB_ChooseIndividualAccount(qb, strPaymentAccount, this)) {
				// The user canceled, so don't proceed
				qb->EndSession();
				return;
			}
		}

		BOOL bWarnedCCTip = FALSE;

		//send over all selected payments, tips, and all child payments of selected batch payments
		// (r.gonet 2015-05-05 14:38) - PLID 65870 - Exclude Gift Certificate Refunds
		_RecordsetPtr rs = CreateRecordset("SELECT 0 AS IsTip, PaymentsT.ID, PatientID, PaymentsT.PayMethod, PaymentPlansT.CheckNo, PaymentsT.QuickbooksID, QBooksAcctID "
			"FROM PaymentsT INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
			"INNER JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
			"LEFT JOIN QBooksAcctsToProvidersT ON PaymentsT.ProviderID = QBooksAcctsToProvidersT.ProviderID WHERE Deposited = 0 AND Deleted = 0 AND LineItemT.Type = 1 AND PaymentsT.PayMethod NOT IN (4,10) "
			"AND ((PaymentsT.ID IN (%s) AND PaymentsT.BatchPaymentID Is Null) OR PaymentsT.BatchPaymentID IN (%s)) "
			"UNION "
			"SELECT 1 AS IsTip, PaymentTipsT.ID, PatientID, 0 AS PayMethod, '' AS CheckNo, "
			"PaymentsT.QuickbooksID, QBooksAcctID "
			"FROM PaymentsT "
			"INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
			"INNER JOIN PaymentTipsT ON PaymentsT.ID = PaymentTipsT.PaymentID "
			"LEFT JOIN QBooksAcctsToProvidersT ON PaymentsT.ProviderID = QBooksAcctsToProvidersT.ProviderID "
			"WHERE PaymentTipsT.Deposited = 0 AND Deleted = 0 AND LineItemT.Type = 1 "
			"AND PaymentTipsT.ID IN (%s) AND (PaymentTipsT.PayMethod = 1 OR PaymentTipsT.PayMethod <> PaymentsT.PayMethod) ", strPaymentIDs, strBatchPaymentIDs, strPaymentTipIDs);
		while(!rs->eof) {

			CWaitCursor pWait2;
			
			CountOfPayments++;

			// Make sure the customer exists
			long PaymentID = AdoFldLong(rs, "ID");
			long PatientID = AdoFldLong(rs, "PatientID");
			long IsTip = AdoFldLong(rs, "IsTip");

			//check to see that the Check No # is not more than 11 characters
			if(AdoFldLong(rs, "PayMethod",-1) == 2) {
				//if a check
				CString strCheckNo = AdoFldString(rs, "CheckNo","");
				if(strCheckNo.GetLength() > 11) {
					//it can't be more than 11 characters, so report the offending payment and cancel
					_RecordsetPtr rsPay = CreateRecordset("SELECT Last, First, Date, Amount FROM LineItemT "
						"INNER JOIN PersonT ON LineItemT.PatientID = PersonT.ID WHERE LineItemT.ID = %li", PaymentID);
					if(!rsPay->eof) {
						CString str;
						str.Format("The %s check payment for patient '%s, %s' made on %s\n"
							"has a check number of %s, which is too long for QuickBooks.\n\n"
							"QuickBooks only supports check numbers up to 11 characters long.\n\n"
							"The payment will not be sent to QuickBooks.",
							FormatCurrencyForInterface(AdoFldCurrency(rsPay, "Amount"),TRUE,TRUE),
							AdoFldString(rsPay, "Last",""),
							AdoFldString(rsPay, "First",""),
							FormatDateTimeForInterface(AdoFldDateTime(rsPay, "Date"),NULL,dtoDate),
							strCheckNo);
						AfxMessageBox(str);
					}
					rsPay->Close();
					CountOfPaymentsSkipped++;
					rs->MoveNext();
					continue;
				}
			}

			if(IsTip == 1) {
				if(ReturnsRecords("SELECT PaymentTipsT.ID FROM PaymentTipsT INNER JOIN PaymentsT ON PaymentTipsT.PaymentID = PaymentsT.ID "
					"WHERE PaymentTipsT.ID = %li AND PaymentTipsT.PayMethod = 3 AND PaymentsT.PayMethod <> 3",PaymentID)) {
					if(!bWarnedCCTip) {
						AfxMessageBox("At least one tip payment could not be exported, because the tip is identified as being from a credit card, \n"
							"and the payment is identified as being a cash or check. Quickbooks will not accept credit card line items \n"
							"without a credit card type associated with them.");
						bWarnedCCTip = TRUE;
					}
					CountOfPaymentsSkipped++;
					rs->MoveNext();
					continue;
				}
			}

			CString strQuickbooksID = AdoFldString(rs, "QuickbooksID","");
			CString strCustomerListID = "", strEditSequence = "";
			if (!QB_GetCustomerListID(qb, PatientID, strCustomerListID, strEditSequence)) {
				// Customer doesn't exist, so create it.
				if(!QB_CreateCustomer(qb, PatientID, strCustomerListID)) {
					rs->MoveNext();
					continue;
				}
				else {
					ExecuteSql("UPDATE PatientsT SET SentToQuickbooks = 1 WHERE PersonID = %li",PatientID);
					CountOfPatientsCreated++;
				}
			}

			//see if the payment already exists
			if(strQuickbooksID != "" && QB_VerifyPaymentTxnID(qb,strQuickbooksID)) {
				//it does, skip it
				CountOfPaymentsSkipped++;
				rs->MoveNext();
				continue;
			}
			else
				//we will send it new
				strQuickbooksID = "";

			// Send the payment
			CString strPaymentTxnID = strQuickbooksID;
			if(strPaymentTxnID == "") {

				CString strAcct = AdoFldString(rs, "QBooksAcctID","");
				if(strAcct == "" || !QB_VerifyPaymentAccountID(qb, strAcct))
					strAcct = strPaymentAccount;

				if(strAcct == "") {
					// Choose the payment account			
					if (!QB_ChooseIndividualAccount(qb, strAcct, this)) {
						// The user canceled, so don't proceed
						CountOfPaymentsSkipped++;
						rs->MoveNext();
						continue;
					}
					// (j.jones 2005-11-01 10:14) - PLID 18140 - save for the remaining payments
					strPaymentAccount = strAcct;
				}

				//create new payment
				COleDateTime dtDeposit = m_dtDeposit.GetValue();
				if(QB_CreatePayment(qb, strCustomerListID, strPaymentTxnID, PaymentID, TRUE, strAcct, (BOOL)IsTip, dtDeposit)) {
					if(!IsTip) {
						ExecuteSql("UPDATE PaymentsT SET QuickbooksID = '%s', SentToQB = 1 WHERE ID = %li",_Q(strPaymentTxnID),PaymentID);
						if(GetRemotePropertyInt("BankingIncludeTips", 0, 0, GetCurrentUserName(), true) == 1) {
							ExecuteSql("UPDATE PaymentTipsT SET QuickbooksID = '%s', SentToQB = 1 WHERE ID = %li AND PaymentID IN (SELECT ID FROM PaymentsT WHERE PaymentsT.PayMethod = PaymentTipsT.PayMethod)",_Q(strPaymentTxnID),PaymentID);
						}
					}
					else {
						ExecuteSql("UPDATE PaymentTipsT SET QuickbooksID = '%s', SentToQB = 1 WHERE ID = %li",_Q(strPaymentTxnID),PaymentID);
					}
					CountOfSentPayments++;
				}
			}
			else {
				//if we ever have the ability to modify existing payments, we would do it here
			}

			rs->MoveNext();
		}

		CString str, strResults;
		if(CountOfPayments == CountOfSentPayments && CountOfPaymentsSkipped == 0)
			str.Format("All %li payments were successfully exported to Quickbooks.",CountOfPayments);
		else
			str.Format("%li payments were exported to Quickbooks. %li already existed or were otherwise skipped.",CountOfSentPayments, CountOfPaymentsSkipped);

		strResults = str;

		if(CountOfPatientsCreated == 1) {
			str.Format("\n\nOne patient was created in Quickbooks.");
			strResults += str;
		}
		else if(CountOfPatientsCreated > 1) {
			str.Format("\n\n%li patients were created in Quickbooks.",CountOfPatientsCreated);
			strResults += str;
		}

		qb->EndSession();

		//AfxMessageBox(strResults);
		strResults += "\n\nWould you like to mark these payments as deposited now?";
		if(IDYES == MessageBox(strResults,"Practice",MB_ICONQUESTION|MB_YESNO)) {
			OnBtnDeposit();
		}

	} NxCatchAllCall("Error exporting payments to Quickbooks.", {
		if(qb != NULL)
			qb->EndSession();
		SAFE_SET_PROGRESS(pProgress, "ERROR encountered while trying to create a payment.");
		(void)0;
	});
}

// (r.gonet 2010-09-13 12:40) - PLID 37458 - Retrieves multiple payment categories of the user's selection using a multi selct dialog
//  Has the side effects of filling the category ids array and also updating the NxLabel text for multiple categories
void PracBanking::GetMultipleCategories()
{
	// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
	CMultiSelectDlg dlg(this, "PaymentGroupsT");
	dlg.PreSelect(m_aryCategoryIDs);
	
	CString strFrom = AsString(m_CategoryCombo->FromClause);
	// (r.gonet 2010-09-13 12:40) - PLID 37458 - Add in the no category option to the category select dialog
	strFrom = 
		"(SELECT 0 AS ID, ' {No Category}' AS GroupName "
		"UNION "
		"SELECT ID, GroupName FROM " + strFrom + ") SubQ";
	CString strWhere = AsString(m_CategoryCombo->WhereClause);
	
	if(dlg.Open(strFrom, strWhere, "ID", "GroupName", "Payment Categories", 1) == IDOK){
		// The user has selected 1+ categories, store the ids, and update the nxlabel to reflect the chosen categories
		dlg.FillArrayWithIDs(m_aryCategoryIDs);
		CString strCategories = dlg.GetMultiSelectString("; ");
		m_nxlMultiPaymentCategories.SetText(strCategories);
		m_nxlMultiPaymentCategories.SetType(dtsHyperlink);
	} else {
		// The user has cancelled category selection.
	}
	m_nxlMultiPaymentCategories.Invalidate();
}

// (r.gonet 2010-09-13 12:40) - PLID 37458 - Update the payment categories filter controls to reflect the user's selections
void PracBanking::UpdateCategoriesFilterControls()
{
	// (r.gonet 2010-09-13 12:40) - PLID 37458 - Set the selections
	if(m_aryCategoryIDs.GetSize() == 1) {
		m_CategoryCombo->SetSelByColumn(0, m_aryCategoryIDs[0]);
	} else {
		if(m_aryCategoryIDs.GetSize() == 0) {
			m_CategoryCombo->SetSelByColumn(0, (long)0);
		}
	}

	// (r.gonet 2010-09-13 12:40) - PLID 37458 - Now redraw the controls
	if(m_aryCategoryIDs.GetSize() > 1){
		ShowDlgItem(IDC_BANKING_PMNT_MULTI_CATS_LBL, SW_SHOWNA);
		ShowDlgItem(IDC_COMBO_CATEGORY, SW_HIDE);
	}
	else {
		ShowDlgItem(IDC_BANKING_PMNT_MULTI_CATS_LBL, SW_HIDE);
		ShowDlgItem(IDC_COMBO_CATEGORY, SW_SHOWNA);
	}

	m_nxlMultiPaymentCategories.AskParentToRedrawWindow();
}

void PracBanking::OnAllPayCats() 
{
	// (j.jones 2008-05-07 17:09) - PLID 25338 - changed the filtering behavior of this dialog
	UnselectAllPayments();
	FilterAllPayments();
	GetDlgItem(IDC_COMBO_CATEGORY)->EnableWindow(FALSE);
	// (r.gonet 2010-09-13 17:33 - PLID 37458 - Also disable the multiple categories label
	m_nxlMultiPaymentCategories.EnableWindow(FALSE);
	m_nxlMultiPaymentCategories.SetType(dtsDisabledHyperlink);
}

void PracBanking::OnSelectPaycatRadio() 
{
	GetDlgItem(IDC_COMBO_CATEGORY)->EnableWindow(TRUE);
	// (r.gonet 2010-09-13 17:33 - PLID 37458 - Also enable the multiple categories label
	m_nxlMultiPaymentCategories.EnableWindow(TRUE);
	m_nxlMultiPaymentCategories.SetType(dtsHyperlink);
	// (j.jones 2008-05-07 17:09) - PLID 25338 - changed the filtering behavior of this dialog
	UnselectAllPayments();
	FilterAllPayments();
}

void PracBanking::OnSelChosenComboCategory(long nRow) 
{
	try {
		// (r.gonet 2010-09-13 12:40) - PLID 37458 - If the user selected {Multiple Categories}, then open up the category list
		NXDATALISTLib::IRowSettingsPtr pRow = m_CategoryCombo->GetRow(nRow);
		if(pRow) {
			long nCategoryID = VarLong(pRow->GetValue(0));
			if(nCategoryID == MULTIPLE_CATEGORIES) {
				GetMultipleCategories();
			} else {
				m_aryCategoryIDs.RemoveAll();
				m_aryCategoryIDs.Add(pRow->GetValue(0));
			}
			UpdateCategoriesFilterControls();
		}
		
		// (j.jones 2008-05-07 17:09) - PLID 25338 - changed the filtering behavior of this dialog
		UnselectAllPayments();
		FilterAllPayments();
	} NxCatchAll(__FUNCTION__);
}

void PracBanking::OnRestorePastDeposits() 
{
	CRetrievePastDepositsDlg dlg(this);
	dlg.DoModal();

	// (j.jones 2007-03-13 09:50) - PLID 25118 - tablecheckers now
	// update the screen accordingly if anything was restored
}

void PracBanking::OnIncludeTipsCheck() 
{
	RefreshLists();

	// (z.manning, 08/15/2007) - PLID 26069 - Give a brief explanation of how exactly this option works.
	DontShowMeAgain(this,
		"The \"Include Tips\" option changes whether or not to have tips available for deposit.\r\n\r\n"
		"Cash tips are always affected by this option. However, tips that get added into a check or a "
		"charge payment are not affected because it is not practical to only deposit part of a check or a charge."
		, "FinancialShowTipExplanation", "Depositing Tips", FALSE, FALSE);

	//remember the state
	SetRemotePropertyInt("BankingIncludeTips", IsDlgButtonChecked(IDC_INCLUDE_TIPS_CHECK), 0, GetCurrentUserName());
}

void PracBanking::OnRButtonDownCreditavaillist(long nRow, short nCol, long x, long y, long nFlags) 
{
	m_CreditAvailList->CurSel = nRow;

	if(nRow == -1)
		return;

	// (j.jones 2011-06-24 12:42) - PLID 22833 - added ability to go to patient
	enum {
		miGoToPatient = -1,
		miSelectAllCardType = -2,
	};

	long nPatientID = VarLong(m_CreditAvailList->GetValue(nRow, creditplcPatientID), -1);

	CMenu pMenu;
	pMenu.CreatePopupMenu();
	pMenu.InsertMenu(0, MF_BYPOSITION|(nPatientID != -1 ? 0 : MF_GRAYED), miGoToPatient, "&Go to Patient");

	CString strCardName = VarString(m_CreditAvailList->GetValue(nRow, creditplcCardName),"");
	CString strCaption;
	if(strCardName != "")
		strCaption.Format("Select All %s Payments",strCardName);
	else
		strCaption.Format("Select All Payments With No Card Type",strCardName);
	pMenu.InsertMenu(1, MF_BYPOSITION, miSelectAllCardType, strCaption);

	CPoint pt;
	GetCursorPos(&pt);

	int nCmdId = pMenu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD|TPM_TOPALIGN, pt.x, pt.y, this, NULL);
	switch(nCmdId) {

		case miGoToPatient:

			GoToPatient(nPatientID);
			break;

		case miSelectAllCardType:
			OnSelectAllCardType();
			break;
	}
}

void PracBanking::OnRButtonDownCreditselectedlist(long nRow, short nCol, long x, long y, long nFlags) 
{
	m_CreditSelectedList->CurSel = nRow;

	if(nRow == -1)
		return;

	// (j.jones 2011-06-24 12:42) - PLID 22833 - added ability to go to patient
	enum {
		miGoToPatient = -1,
		miUnselectAllCardType = -2,
	};

	long nPatientID = VarLong(m_CreditSelectedList->GetValue(nRow, creditplcPatientID), -1);

	CMenu pMenu;
	pMenu.CreatePopupMenu();
	pMenu.InsertMenu(0, MF_BYPOSITION|(nPatientID != -1 ? 0 : MF_GRAYED), miGoToPatient, "&Go to Patient");

	CString strCardName = VarString(m_CreditSelectedList->GetValue(nRow, creditplcCardName),"");
	CString strCaption;
	if(strCardName != "")
		strCaption.Format("Unselect All %s Payments",strCardName);
	else
		strCaption.Format("Unselect All Payments With No Card Type",strCardName);
	pMenu.InsertMenu(1, MF_BYPOSITION, miUnselectAllCardType, strCaption);

	CPoint pt;
	GetCursorPos(&pt);

	int nCmdId = pMenu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD|TPM_TOPALIGN, pt.x, pt.y, this, NULL);
	switch(nCmdId) {

		case miGoToPatient:

			GoToPatient(nPatientID);
			break;

		case miUnselectAllCardType:
			OnUnselectAllCardType();
			break;
	}
}

void PracBanking::OnSelectAllCardType()
{
	try {

		if(m_CreditAvailList->CurSel == -1)
			return;

		// (e.lally 2007-07-09) PLID 25993 - Changed credit card to pull from its new ID field
		long nCreditCardID = VarLong(m_CreditAvailList->GetValue(m_CreditAvailList->CurSel, creditplcCardID),-1);
		
		// (j.jones 2008-05-09 15:53) - PLID 25338 - this now works exclusively off of the datalist

		long nRow = m_CreditAvailList->GetFirstRowEnum();
		LPDISPATCH lpDisp = NULL;
		while(nRow) 
		{
			m_CreditAvailList->GetNextRowEnum(&nRow, &lpDisp);
			IRowSettingsPtr pRow(lpDisp); 
			lpDisp->Release();

			long nCardID = VarLong(pRow->GetValue(creditplcCardID), -1);
			if(nCardID == nCreditCardID) {  //this intentionally allows -1 comparisons
				m_CreditSelectedList->TakeRow(pRow);
			}			
		}

		UpdateAvailTotals();
		UpdateSelectedTotals();
		
	}NxCatchAll("Error selecting card type.");
}

void PracBanking::OnUnselectAllCardType()
{
	try {

		if(m_CreditSelectedList->CurSel == -1)
			return;

		// (e.lally 2007-07-09) PLID 25993 - Changed credit card to pull from its new ID field
		long nCreditCardID = VarLong(m_CreditSelectedList->GetValue(m_CreditSelectedList->CurSel, creditplcCardID),-1);

		// (j.jones 2008-05-09 15:53) - PLID 25338 - this now works exclusively off of the datalist

		long nRow = m_CreditSelectedList->GetFirstRowEnum();
		LPDISPATCH lpDisp = NULL;
		while(nRow) 
		{
			m_CreditSelectedList->GetNextRowEnum(&nRow, &lpDisp);
			IRowSettingsPtr pRow(lpDisp); 
			lpDisp->Release();

			long nCardID = VarLong(pRow->GetValue(creditplcCardID), -1);
			if(nCardID == nCreditCardID) {  //this intentionally allows -1 comparisons
				m_CreditAvailList->TakeRow(pRow);
			}			
		}

		UpdateAvailTotals();
		UpdateSelectedTotals();

	}NxCatchAll("Error unselecting card type.");
}

void PracBanking::OnPrepareRefunds() 
{
	CPrepareRefundChecksDlg dlg(this);
	dlg.DoModal();
}

void PracBanking::OnIncludeRefundsCheck() 
{
	RefreshLists();

	//remember the state
	SetRemotePropertyInt("BankingIncludeRefunds", IsDlgButtonChecked(IDC_INCLUDE_REFUNDS_CHECK), 0, GetCurrentUserName());
	
}

// (j.jones 2007-03-13 09:04) - PLID 25118 - supported OnTableChanged, for the purposes of DepositedPayments
LRESULT PracBanking::OnTableChanged(WPARAM wParam, LPARAM lParam)
{
	try {

		switch(wParam) {
		case NetUtils::DepositedPayments:
			{
				try {
					
					if(m_DepositedPaymentsChecker.Changed()) {
						RefreshLists();
					}
					
				} NxCatchAll("Error in PracBanking::OnTableChanged:DepositedPayments");
			}
			break;
		}
	}NxCatchAll("Error in PracBanking::OnTableChanged()");
	return 0;
}

// (c.haag 2009-08-31 12:10) - PLID 13175 - Updates the widths for optional columns
// (j.jones 2013-06-24 16:15) - PLID 35059 - Added booleans to tell this function that
// we just caused new columns to be shown, which will force columns "remembered" at 0 width
// to expand to a new default width.
void PracBanking::UpdateVisibleColumns(bool bPaymentDatesJustShown /*= false*/, bool bInputDatesJustShown /*= false*/)
{
	// (j.jones 2013-06-24 08:51) - PLID 35059 - reworked this function to support
	// remembering column widths

	//cash
	UpdateVisibleCashColumns(m_CashAvailList, "BankingColumnWidthsCashAvail", bPaymentDatesJustShown, bInputDatesJustShown);
	UpdateVisibleCashColumns(m_CashSelectedList, "BankingColumnWidthsCashSelected", bPaymentDatesJustShown, bInputDatesJustShown);

	//check
	UpdateVisibleCheckColumns(m_CheckAvailList, "BankingColumnWidthsCheckAvail", bPaymentDatesJustShown, bInputDatesJustShown);
	UpdateVisibleCheckColumns(m_CheckSelectedList, "BankingColumnWidthsCheckSelected", bPaymentDatesJustShown, bInputDatesJustShown);

	//credit
	UpdateVisibleCreditColumns(m_CreditAvailList, "BankingColumnWidthsCreditAvail", bPaymentDatesJustShown, bInputDatesJustShown);
	UpdateVisibleCreditColumns(m_CreditSelectedList, "BankingColumnWidthsCreditSelected", bPaymentDatesJustShown, bInputDatesJustShown);
}

// (j.jones 2013-06-24 09:48) - PLID 35059 - split out modular code for updating the cash,
// check, and credit card list columns with default or remembered values
void PracBanking::UpdateVisibleCashColumns(NXDATALISTLib::_DNxDataListPtr pCashList, CString strRememberedWidthsConfigRTName, bool bPaymentDatesJustShown, bool bInputDatesJustShown)
{
	CString strRememberedWidths = GetRemotePropertyText(strRememberedWidthsConfigRTName, "", 0, GetCurrentUserName(), true);
	bool bNeedsResave = false;

	//convert the remembered widths, if any, to an array
	CArray<int, int> aryRememberedWidths;
	if(!strRememberedWidths.IsEmpty()) {
		ParseCommaDeliminatedText(aryRememberedWidths, strRememberedWidths);

		//if the remembered width array is not equal to our column count,
		//we cannot possibly apply it, and therefore have to reset their widths
		if(aryRememberedWidths.GetSize() != pCashList->ColumnCount) {
			strRememberedWidths = "";
			SetRemotePropertyText(strRememberedWidthsConfigRTName, "", 0, GetCurrentUserName());
			aryRememberedWidths.RemoveAll();
		}
	}

	bool bShowPaymentDates = IsDlgButtonChecked(IDC_CHECK_SHOW_PMT_DATES) ? true : false;
	bool bShowInputDates = IsDlgButtonChecked(IDC_CHECK_SHOW_INPUT_DATES) ? true : false;

	for (int i = 0; i < pCashList->ColumnCount; i++)
	{
		NXDATALISTLib::IColumnSettingsPtr pCol = pCashList->GetColumn(i);
		
		//these columns are always hidden and not sizeable
		if(i == cashplcID
			|| i == cashplcPatientID
			|| i == cashplcIsTip
			|| i == cashplcProviderID
			|| i == cashplcLocationID
			|| i == cashplcCategoryID
			|| i == cashplcRespID)
		{
			pCol->PutStoredWidth(0);
			pCol->PutColumnStyle(csVisible|csFixedWidth);
		}
		//these columns are potentially resizeable
		// (r.farnworth 2013-10-07 12:06) - PLID 58900 - Allows the InputName column to be resizeable
		else if(i == cashplcPatientName
			|| i == cashplcAmount
			|| i == cashplcDate
			|| i == cashplcInputDate
			|| i == cashplcUserName) {

			//now apply our stored width, if we have one
			if(m_bRememberColumns && aryRememberedWidths.GetSize() >= (i+1)) {
				long nWidth = aryRememberedWidths.GetAt(i);

				//hide dates if not shown
				if(!bShowPaymentDates && i == cashplcDate) {
					pCol->PutStoredWidth(0);
					pCol->PutColumnStyle(csVisible|csFixedWidth);
				}
				else if(!bShowInputDates && i == cashplcInputDate) {
					pCol->PutStoredWidth(0);
					pCol->PutColumnStyle(csVisible|csFixedWidth);
				}
				else {
					//if we're in the process of showing a payment or input date,
					//and the saved width is zero, force them to be their default width
					//if either date, it's data width 80, only if shown
					if(nWidth == 0 &&
						((bShowPaymentDates && bPaymentDatesJustShown && i == cashplcDate)
						|| (bShowInputDates && bInputDatesJustShown && i == cashplcInputDate))) {

						nWidth = 80;
						//flag that we need to save this new size
						bNeedsResave = true;
					}

					//use their saved width
					pCol->PutStoredWidth(nWidth);
					pCol->PutColumnStyle(csVisible);
				}
			}
			//apply our default width
			else {
				if(i == cashplcPatientName || i == cashplcAmount) {
					//width auto
					pCol->PutStoredWidth(-1);
					pCol->PutColumnStyle(csVisible|csWidthAuto);
				}
				//if either date, it's data width 80, only if shown
				else if(i == cashplcDate || i == cashplcInputDate) {

					if((bShowPaymentDates && i == cashplcDate)
						|| (bShowInputDates && i == cashplcInputDate)) {
						//width data
						pCol->PutStoredWidth(80);
						pCol->PutColumnStyle(csVisible|csWidthData);
					}
					else {
						pCol->PutStoredWidth(0);
						pCol->PutColumnStyle(csVisible|csFixedWidth);
					}
				}
				// (r.farnworth 2013-10-07 12:06) - PLID 58900 - Allows the InputName column to be resizeable
				else if(i == cashplcUserName) {
					pCol->PutStoredWidth(0);
					pCol->PutColumnStyle(csVisible);
				}
			}
		}
		else {
			//shouldn't be possible unless a developer added a new column
			//and did not update this function
			ASSERT(FALSE);
		}
	}

	//if we need to re-save the column widths, do so now
	if(bNeedsResave) {
		SaveColumnWidths(pCashList, strRememberedWidthsConfigRTName);
	}
}

void PracBanking::UpdateVisibleCheckColumns(NXDATALISTLib::_DNxDataListPtr pCheckList, CString strRememberedWidthsConfigRTName, bool bPaymentDatesJustShown, bool bInputDatesJustShown)
{
	CString strRememberedWidths = GetRemotePropertyText(strRememberedWidthsConfigRTName, "", 0, GetCurrentUserName(), true);
	bool bNeedsResave = false;

	//convert the remembered widths, if any, to an array
	CArray<int, int> aryRememberedWidths;
	if(!strRememberedWidths.IsEmpty()) {
		ParseCommaDeliminatedText(aryRememberedWidths, strRememberedWidths);

		//if the remembered width array is not equal to our column count,
		//we cannot possibly apply it, and therefore have to reset their widths
		if(aryRememberedWidths.GetSize() != pCheckList->ColumnCount) {
			strRememberedWidths = "";
			SetRemotePropertyText(strRememberedWidthsConfigRTName, "", 0, GetCurrentUserName());
			aryRememberedWidths.RemoveAll();
		}
	}

	bool bShowPaymentDates = IsDlgButtonChecked(IDC_CHECK_SHOW_PMT_DATES) ? true : false;
	bool bShowInputDates = IsDlgButtonChecked(IDC_CHECK_SHOW_INPUT_DATES) ? true : false;

	for (int i = 0; i < pCheckList->ColumnCount; i++)
	{
		NXDATALISTLib::IColumnSettingsPtr pCol = pCheckList->GetColumn(i);
		
		//these columns are always hidden and not sizeable
		if(i == checkplcID
			|| i == checkplcPatientID
			|| i == checkplcIsBatchPay
			|| i == checkplcIsTip
			|| i == checkplcProviderID
			|| i == checkplcLocationID
			|| i == checkplcCategoryID
			|| i == checkplcRespID)
		{
			pCol->PutStoredWidth(0);
			pCol->PutColumnStyle(csVisible|csFixedWidth);
		}
		//these columns are potentially resizeable
		// (r.farnworth 2013-10-07 12:06) - PLID 58900 - Allows the InputName column to be resizeable
		else if(i == checkplcCheckNumber
			|| i == checkplcPatientName
			|| i == checkplcAmount
			|| i == checkplcDate
			|| i == checkplcInputDate
			|| i == checkplcUserName) {

			//now apply our stored width, if we have one
			if(m_bRememberColumns && aryRememberedWidths.GetSize() >= (i+1)) {
				long nWidth = aryRememberedWidths.GetAt(i);

				//hide dates if not shown
				if(!bShowPaymentDates && i == checkplcDate) {
					pCol->PutStoredWidth(0);
					pCol->PutColumnStyle(csVisible|csFixedWidth);
				}
				else if(!bShowInputDates && i == checkplcInputDate) {
					pCol->PutStoredWidth(0);
					pCol->PutColumnStyle(csVisible|csFixedWidth);
				}
				else {
					//if we're in the process of showing a payment or input date,
					//and the saved width is zero, force them to be their default width
					//if either date, it's data width 80, only if shown
					if(nWidth == 0 &&
						((bShowPaymentDates && bPaymentDatesJustShown && i == checkplcDate)
						|| (bShowInputDates && bInputDatesJustShown && i == checkplcInputDate))) {

						nWidth = 80;
						//flag that we need to save this new size
						bNeedsResave = true;
					}

					//use their saved width
					pCol->PutStoredWidth(nWidth);
					pCol->PutColumnStyle(csVisible);
				}
			}
			//apply our default width
			else {
				if(i == checkplcCheckNumber || i == checkplcPatientName || i == checkplcAmount) {
					//width auto
					pCol->PutStoredWidth(-1);
					pCol->PutColumnStyle(csVisible|csWidthAuto);
				}
				//if either date, it's data width 80, only if shown
				else if(i == checkplcDate || i == checkplcInputDate) {

					if((bShowPaymentDates && i == checkplcDate)
						|| (bShowInputDates && i == checkplcInputDate)) {
						//width data
						pCol->PutStoredWidth(80);
						pCol->PutColumnStyle(csVisible|csWidthData);
					}
					else {
						pCol->PutStoredWidth(0);
						pCol->PutColumnStyle(csVisible|csFixedWidth);
					}
				}
				// (r.farnworth 2013-10-07 12:06) - PLID 58900 - Allows the InputName column to be resizeable
				else if(i == checkplcUserName) {
					pCol->PutStoredWidth(0);
					pCol->PutColumnStyle(csVisible);
				}
			}
		}
		else {
			//shouldn't be possible unless a developer added a new column
			//and did not update this function
			ASSERT(FALSE);
		}
	}

	//if we need to re-save the column widths, do so now
	if(bNeedsResave) {
		SaveColumnWidths(pCheckList, strRememberedWidthsConfigRTName);
	}
}

void PracBanking::UpdateVisibleCreditColumns(NXDATALISTLib::_DNxDataListPtr pCreditList, CString strRememberedWidthsConfigRTName, bool bPaymentDatesJustShown, bool bInputDatesJustShown)
{
	CString strRememberedWidths = GetRemotePropertyText(strRememberedWidthsConfigRTName, "", 0, GetCurrentUserName(), true);
	bool bNeedsResave = false;

	//convert the remembered widths, if any, to an array
	CArray<int, int> aryRememberedWidths;
	if(!strRememberedWidths.IsEmpty()) {
		ParseCommaDeliminatedText(aryRememberedWidths, strRememberedWidths);

		//if the remembered width array is not equal to our column count,
		//we cannot possibly apply it, and therefore have to reset their widths
		if(aryRememberedWidths.GetSize() != pCreditList->ColumnCount) {
			strRememberedWidths = "";
			SetRemotePropertyText(strRememberedWidthsConfigRTName, "", 0, GetCurrentUserName());
			aryRememberedWidths.RemoveAll();
		}
	}

	bool bShowPaymentDates = IsDlgButtonChecked(IDC_CHECK_SHOW_PMT_DATES) ? true : false;
	bool bShowInputDates = IsDlgButtonChecked(IDC_CHECK_SHOW_INPUT_DATES) ? true : false;

	for (int i = 0; i < pCreditList->ColumnCount; i++)
	{
		NXDATALISTLib::IColumnSettingsPtr pCol = pCreditList->GetColumn(i);
		
		//these columns are always hidden and not sizeable
		if(i == creditplcID
			|| i == creditplcPatientID
			|| i == creditplcIsTip
			|| i == creditplcCardID
			|| i == creditplcProviderID
			|| i == creditplcLocationID
			|| i == creditplcCategoryID
			|| i == creditplcRespID)
		{
			pCol->PutStoredWidth(0);
			pCol->PutColumnStyle(csVisible|csFixedWidth);
		}
		//these columns are potentially resizeable
		// (r.farnworth 2013-10-07 12:06) - PLID 58900 - Allows the InputName column to be resizeable
		else if(i == creditplcCardName
			|| i == creditplcLast4
			|| i == creditplcPatientName
			|| i == creditplcAmount
			|| i == creditplcDate
			|| i == creditplcInputDate
			|| i == creditplcUserName) {

			//now apply our stored width, if we have one
			if(m_bRememberColumns && aryRememberedWidths.GetSize() >= (i+1)) {
				long nWidth = aryRememberedWidths.GetAt(i);

				//hide dates if not shown
				if(!bShowPaymentDates && i == creditplcDate) {
					pCol->PutStoredWidth(0);
					pCol->PutColumnStyle(csVisible|csFixedWidth);
				}
				else if(!bShowInputDates && i == creditplcInputDate) {
					pCol->PutStoredWidth(0);
					pCol->PutColumnStyle(csVisible|csFixedWidth);
				}
				else {
					//if we're in the process of showing a payment or input date,
					//and the saved width is zero, force them to be their default width
					//if either date, it's data width 80, only if shown
					if(nWidth == 0 &&
						((bShowPaymentDates && bPaymentDatesJustShown && i == creditplcDate)
						|| (bShowInputDates && bInputDatesJustShown && i == creditplcInputDate))) {

						nWidth = 80;
						//flag that we need to save this new size
						bNeedsResave = true;
					}

					//use their saved width
					pCol->PutStoredWidth(nWidth);
					pCol->PutColumnStyle(csVisible);
				}
			}
			//apply our default width
			else {
				if(i == creditplcPatientName) {
					//width auto
					pCol->PutStoredWidth(-1);
					pCol->PutColumnStyle(csVisible|csWidthAuto);
				}
				else if(i == creditplcCardName) {
					//width data
					pCol->PutStoredWidth(60);
					pCol->PutColumnStyle(csVisible|csWidthData);
				}
				else if(i == creditplcLast4) {
					//width data
					pCol->PutStoredWidth(40);
					pCol->PutColumnStyle(csVisible|csWidthData);
				}
				else if(i == creditplcAmount) {
					//width data
					pCol->PutStoredWidth(75);
					pCol->PutColumnStyle(csVisible|csWidthData);
				}
				// (r.farnworth 2013-10-07 12:06) - PLID 58900 - Allows the InputName column to be resizeable
				else if(i == creditplcUserName) {
					//width data
					pCol->PutStoredWidth(0);
					pCol->PutColumnStyle(csVisible);
				}
				//if either date, it's data width 80, only if shown
				else if(i == creditplcDate || i == creditplcInputDate) {

					if((bShowPaymentDates && i == creditplcDate)
						|| (bShowInputDates && i == creditplcInputDate)) {
						//width data
						pCol->PutStoredWidth(80);
						pCol->PutColumnStyle(csVisible|csWidthData);
					}
					else {
						pCol->PutStoredWidth(0);
						pCol->PutColumnStyle(csVisible|csFixedWidth);
					}
				}
			}
		}
		else {
			//shouldn't be possible unless a developer added a new column
			//and did not update this function
			ASSERT(FALSE);
		}
	}

	//if we need to re-save the column widths, do so now
	if(bNeedsResave) {
		SaveColumnWidths(pCreditList, strRememberedWidthsConfigRTName);
	}
}

void PracBanking::OnCheckShowPmtDates()
{
	try {

		bool bChecked = IsDlgButtonChecked(IDC_CHECK_SHOW_PMT_DATES) ? true : false;

		SetRemotePropertyInt("BankingShowPaymentDates", bChecked ? 1 : 0, 0, GetCurrentUserName());

		// (j.jones 2013-06-24 16:32) - PLID 35059 - let UpdateVisibleColumns know if the user
		// just checked this box
		UpdateVisibleColumns(bChecked, false);
	}
	NxCatchAll(__FUNCTION__);
}

void PracBanking::OnCheckShowInputDates()
{
	try {
		bool bChecked = IsDlgButtonChecked(IDC_CHECK_SHOW_INPUT_DATES) ? true : false;

		SetRemotePropertyInt("BankingShowInputDates", bChecked ? 1 : 0, 0, GetCurrentUserName());

		// (j.jones 2013-06-24 16:32) - PLID 35059 - let UpdateVisibleColumns know if the user
		// just checked this box
		UpdateVisibleColumns(false, bChecked);
	}
	NxCatchAll(__FUNCTION__);
}

// (r.gonet 2010-09-13 12:40) - PLID 37458 - Handle the case where a label is left clicked
LRESULT PracBanking::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	try {
		UINT nIdc = (UINT)wParam;
		switch(nIdc) {
		// (r.gonet 2010-09-13 12:40) - PLID 37458 - Handle the multi categories label being clicked
		case IDC_BANKING_PMNT_MULTI_CATS_LBL:
			GetMultipleCategories();
			UpdateCategoriesFilterControls();
			// (r.gonet 2010-09-28 14:55) - PLID 37458 - Now refilter the payments
			UnselectAllPayments();
			FilterAllPayments();
			break;
		
		default:
			// (r.gonet 2010-09-13 12:40) - PLID 37458 - Unknown label. All labels that post messages in this dialog should be handled in this function.
			ASSERT(FALSE);
			break;
		}
	}NxCatchAll(__FUNCTION__);
	
	return 0;
}

// (r.gonet 09-13-2010 17:42) - PLID 37458 - Change the cursor when hovering over links.
//  This code was mostly lifted from the TopsSearch dialog.
BOOL PracBanking::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	try{
		CPoint pt;
		CRect rc;
		GetCursorPos(&pt);
		ScreenToClient(&pt);

		// (r.gonet 2010-09-13 12:40) - PLID 37458 - Test if this area is the multi categories link. If it is, change the cursor to the hand
		if(m_nxlMultiPaymentCategories.IsWindowVisible() && m_nxlMultiPaymentCategories.IsWindowEnabled()) {
			m_nxlMultiPaymentCategories.GetWindowRect(rc);
			ScreenToClient(&rc);

			if(rc.PtInRect(pt)) {
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}
	}NxCatchAllCallIgnore({
		if(m_bNotifyOnce){ 
			m_bNotifyOnce = FALSE; 
			try{ throw;}NxCatchAll(__FUNCTION__);
		}
	});
	return CNxDialog::OnSetCursor(pWnd, nHitTest, message);
}
// (j.gruber 2010-10-25 15:48) - PLID 37457
void PracBanking::SelChosenRespFilterList(LPDISPATCH lpRow)
{
	try {

		UnselectAllPayments();
		FilterAllPayments();

	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2010-10-25 15:48) - PLID 37457
void PracBanking::RequeryFinishedRespFilterList(short nFlags)
{
	try {
		//set it to all
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pRespFilter->FindByColumn(0, (long)-1, NULL, TRUE);		
	}NxCatchAll(__FUNCTION__);
		
		

}

// (j.jones 2011-06-16 13:54) - PLID 42038 - added user filter
void PracBanking::OnBnClickedAllusers()
{
	try {

		UnselectAllPayments();
		FilterAllPayments();

		GetDlgItem(IDC_COMBO_USER_FILTER)->EnableWindow(FALSE);

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2011-06-16 13:54) - PLID 42038 - added user filter
void PracBanking::OnBnClickedSelectUserRadio()
{
	try {

		//force a selection
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_UserSelect->GetCurSel();
		if(pRow == NULL) {
			m_UserSelect->SetSelByColumn(uccID, GetCurrentUserID());
		}

		UnselectAllPayments();
		FilterAllPayments();

		GetDlgItem(IDC_COMBO_USER_FILTER)->EnableWindow(TRUE);

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2011-06-16 13:54) - PLID 42038 - added user filter
void PracBanking::OnSelChosenComboUserFilter(LPDISPATCH lpRow)
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			m_UserSelect->SetSelByColumn(uccID, GetCurrentUserID());
		}

		UnselectAllPayments();
		FilterAllPayments();

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2011-06-24 12:40) - PLID 22833 - added right click handler
void PracBanking::OnRButtonDownCashavaillist(long nRow, short nCol, long x, long y, long nFlags)
{
	try {

		m_CashAvailList->CurSel = nRow;

		if(nRow == -1) {
			return;
		}

		// (j.jones 2011-06-24 12:42) - PLID 22833 - added ability to go to patient
		enum {
			miGoToPatient = -1,
		};

		long nPatientID = VarLong(m_CashAvailList->GetValue(nRow, cashplcPatientID), -1);

		CMenu pMenu;
		pMenu.CreatePopupMenu();
		pMenu.InsertMenu(0, MF_BYPOSITION|(nPatientID != -1 ? 0 : MF_GRAYED), miGoToPatient, "&Go to Patient");

		CPoint pt;
		GetCursorPos(&pt);

		int nCmdId = pMenu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD|TPM_TOPALIGN, pt.x, pt.y, this, NULL);
		switch(nCmdId) {

			case miGoToPatient:

				GoToPatient(nPatientID);
				break;
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2011-06-24 12:40) - PLID 22833 - added right click handler
void PracBanking::OnRButtonDownCashselectedlist(long nRow, short nCol, long x, long y, long nFlags)
{
	try {

		m_CashSelectedList->CurSel = nRow;

		if(nRow == -1) {
			return;
		}

		// (j.jones 2011-06-24 12:42) - PLID 22833 - added ability to go to patient
		enum {
			miGoToPatient = -1,
		};

		long nPatientID = VarLong(m_CashSelectedList->GetValue(nRow, cashplcPatientID), -1);

		CMenu pMenu;
		pMenu.CreatePopupMenu();
		pMenu.InsertMenu(0, MF_BYPOSITION|(nPatientID != -1 ? 0 : MF_GRAYED), miGoToPatient, "&Go to Patient");

		CPoint pt;
		GetCursorPos(&pt);

		int nCmdId = pMenu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD|TPM_TOPALIGN, pt.x, pt.y, this, NULL);
		switch(nCmdId) {

			case miGoToPatient:

				GoToPatient(nPatientID);
				break;
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2011-06-24 12:40) - PLID 22833 - added right click handler
void PracBanking::OnRButtonDownCheckavaillist(long nRow, short nCol, long x, long y, long nFlags)
{
	try {

		m_CheckAvailList->CurSel = nRow;

		if(nRow == -1) {
			return;
		}

		// (j.jones 2011-06-24 12:42) - PLID 22833 - added ability to go to patient
		enum {
			miGoToPatient = -1,
		};

		long nPatientID = VarLong(m_CheckAvailList->GetValue(nRow, checkplcPatientID), -1);

		CMenu pMenu;
		pMenu.CreatePopupMenu();
		pMenu.InsertMenu(0, MF_BYPOSITION|(nPatientID != -1 ? 0 : MF_GRAYED), miGoToPatient, "&Go to Patient");

		CPoint pt;
		GetCursorPos(&pt);

		int nCmdId = pMenu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD|TPM_TOPALIGN, pt.x, pt.y, this, NULL);
		switch(nCmdId) {

			case miGoToPatient:

				GoToPatient(nPatientID);
				break;
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2011-06-24 12:40) - PLID 22833 - added right click handler
void PracBanking::OnRButtonDownCheckselectedlist(long nRow, short nCol, long x, long y, long nFlags)
{
	try {

		m_CheckSelectedList->CurSel = nRow;

		if(nRow == -1) {
			return;
		}

		// (j.jones 2011-06-24 12:42) - PLID 22833 - added ability to go to patient
		enum {
			miGoToPatient = -1,
		};

		long nPatientID = VarLong(m_CheckSelectedList->GetValue(nRow, checkplcPatientID), -1);

		CMenu pMenu;
		pMenu.CreatePopupMenu();
		pMenu.InsertMenu(0, MF_BYPOSITION|(nPatientID != -1 ? 0 : MF_GRAYED), miGoToPatient, "&Go to Patient");

		CPoint pt;
		GetCursorPos(&pt);

		int nCmdId = pMenu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD|TPM_TOPALIGN, pt.x, pt.y, this, NULL);
		switch(nCmdId) {

			case miGoToPatient:

				GoToPatient(nPatientID);
				break;
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2011-06-24 12:42) - PLID 22833 - added ability to go to patient
void PracBanking::GoToPatient(long nPatientID)
{
	try {

		if(nPatientID != -1) {
			//Set the active patient
			CMainFrame *pMainFrame;
			pMainFrame = GetMainFrame();
			if(pMainFrame != NULL) {
				
				if(pMainFrame->m_patToolBar.TrySetActivePatientID(nPatientID)) {

					//Now just flip to the patient's module and set the active Patient
					pMainFrame->FlipToModule(PATIENT_MODULE_NAME);

					CNxTabView *pView = pMainFrame->GetActiveView();
					if(pView) {
						pView->SetActiveTab(PatientsModule::BillingTab);
						pView->UpdateView();
					}
				}
			}
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2013-06-21 10:51) - PLID 35059 - added ability to remember column widths
void PracBanking::OnCheckRememberBankingColWidths()
{
	try {

		m_bRememberColumns = m_checkRememberColWidths.GetCheck() ? true : false;

		if (m_bRememberColumns) {
			SaveColumns(eAll);
		}

		SetRemotePropertyInt("BankingRememberColumnWidths", m_bRememberColumns ? 1 : 0, 0, GetCurrentUserName());

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2013-06-21 10:51) - PLID 35059 - added ability to remember column widths
void PracBanking::SaveColumns(EDetailedListType eListToSave /*= eAll*/)
{
	try {

		if (!m_bRememberColumns) {
			return;
		}

		// Store the columns in a xx,xx,xx,xx format

		//cash available
		if(eListToSave == eAll || eListToSave == eCashAvail)
		{
			SaveColumnWidths(m_CashAvailList, "BankingColumnWidthsCashAvail");

			//immediately apply these widths to the list, to force Pixel width on anything
			//that might currently be width Auto or Data
			UpdateVisibleCashColumns(m_CashAvailList, "BankingColumnWidthsCashAvail", false, false);
		}

		//cash selected
		if(eListToSave == eAll || eListToSave == eCashSelected)
		{
			SaveColumnWidths(m_CashSelectedList, "BankingColumnWidthsCashSelected");

			//immediately apply these widths to the list, to force Pixel width on anything
			//that might currently be width Auto or Data
			UpdateVisibleCashColumns(m_CashSelectedList, "BankingColumnWidthsCashSelected", false, false);
		}

		//check available
		if(eListToSave == eAll || eListToSave == eCheckAvail)
		{
			SaveColumnWidths(m_CheckAvailList, "BankingColumnWidthsCheckAvail");

			//immediately apply these widths to the list, to force Pixel width on anything
			//that might currently be width Auto or Data
			UpdateVisibleCheckColumns(m_CheckAvailList, "BankingColumnWidthsCheckAvail", false, false);
		}

		//check selected
		if(eListToSave == eAll || eListToSave == eCheckSelected)
		{
			SaveColumnWidths(m_CheckSelectedList, "BankingColumnWidthsCheckSelected");

			//immediately apply these widths to the list, to force Pixel width on anything
			//that might currently be width Auto or Data
			UpdateVisibleCheckColumns(m_CheckSelectedList, "BankingColumnWidthsCheckSelected", false, false);
		}

		//credit available
		if(eListToSave == eAll || eListToSave == eCreditAvail)
		{
			SaveColumnWidths(m_CreditAvailList, "BankingColumnWidthsCreditAvail");

			//immediately apply these widths to the list, to force Pixel width on anything
			//that might currently be width Auto or Data
			UpdateVisibleCreditColumns(m_CreditAvailList, "BankingColumnWidthsCreditAvail", false, false);
		}

		//credit selected
		if(eListToSave == eAll || eListToSave == eCreditSelected)
		{
			SaveColumnWidths(m_CreditSelectedList, "BankingColumnWidthsCreditSelected");

			//immediately apply these widths to the list, to force Pixel width on anything
			//that might currently be width Auto or Data
			UpdateVisibleCreditColumns(m_CreditSelectedList, "BankingColumnWidthsCreditSelected", false, false);
		}

	} NxCatchAll(__FUNCTION__);
}

// (j.jones 2013-06-24 09:48) - PLID 35059 - made one modular function for saving column widths	
void PracBanking::SaveColumnWidths(NXDATALISTLib::_DNxDataListPtr pList, CString strRememberedWidthsConfigRTName)
{
	CString strColumnWidths;
	for (int i = 0; i < pList->ColumnCount; i++)
	{
		NXDATALISTLib::IColumnSettingsPtr pCol = pList->GetColumn(i);
		CString str;
		
		
		str.Format("%d", pCol->StoredWidth);
		
		if (i > 0)
			strColumnWidths += ",";

		strColumnWidths += str;
	}

	SetRemotePropertyText(strRememberedWidthsConfigRTName, strColumnWidths, 0, GetCurrentUserName());
}

// (j.jones 2013-06-21 10:51) - PLID 35059 - added ability to remember column widths
void PracBanking::OnColumnSizingFinishedCashavaillist(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth)
{
	try {

		if (m_bRememberColumns) {
			SaveColumns(eCashAvail);
		}

	} NxCatchAll(__FUNCTION__);
}

// (j.jones 2013-06-21 10:51) - PLID 35059 - added ability to remember column widths
void PracBanking::OnColumnSizingFinishedCashselectedlist(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth)
{
	try {

		if (m_bRememberColumns) {
			SaveColumns(eCashSelected);
		}

	} NxCatchAll(__FUNCTION__);
}

// (j.jones 2013-06-21 10:51) - PLID 35059 - added ability to remember column widths
void PracBanking::OnColumnSizingFinishedCheckavaillist(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth)
{
	try {

		if (m_bRememberColumns) {
			SaveColumns(eCheckAvail);
		}

	} NxCatchAll(__FUNCTION__);
}

// (j.jones 2013-06-21 10:51) - PLID 35059 - added ability to remember column widths
void PracBanking::OnColumnSizingFinishedCheckselectedlist(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth)
{
	try {

		if (m_bRememberColumns) {
			SaveColumns(eCheckSelected);
		}

	} NxCatchAll(__FUNCTION__);
}

// (j.jones 2013-06-21 10:51) - PLID 35059 - added ability to remember column widths
void PracBanking::OnColumnSizingFinishedCreditavaillist(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth)
{
	try {

		if (m_bRememberColumns) {
			SaveColumns(eCreditAvail);
		}

	} NxCatchAll(__FUNCTION__);
}

// (j.jones 2013-06-21 10:51) - PLID 35059 - added ability to remember column widths
void PracBanking::OnColumnSizingFinishedCreditselectedlist(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth)
{
	try {

		if (m_bRememberColumns) {
			SaveColumns(eCreditSelected);
		}

	} NxCatchAll(__FUNCTION__);
}
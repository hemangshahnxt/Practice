// BillingFollowUpDlg.cpp : implementation file
//

#include "stdafx.h"
#include "GlobalReportUtils.h"
#include "ReportInfo.h"
#include "Reports.h"
#include "globalfinancialutils.h"
#include "BillingFollowUpDlg.h"
#include "LetterWriting.h"
#include "MergeEngine.h"
#include "InternationalUtils.h"
#include "GetNewIDName.h"
#include "DateTimeUtils.h"
#include "AllowedAmountSearchDlg.h"
#include "AccountWriteOffDlg.h"
#include "Financialrc.h"
#include "Practice.h"
#include "ConfigureColumnsDlg.h"
#include "PracticeRc.h"
#include "PatientView.h"
#include "NotesDlg.h"
#include "NxModalParentDlg.h"
#include "NxWordProcessorLib\GenericWordProcessorManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;

#define ID_REMOVE_SELECTED_BILLS	48101
// (r.gonet 07/25/2014) - PLID 62920 - Menu item identifiers for the results list context menu.
#define ID_MARK_BILL_ON_HOLD	48102
#define ID_REMOVE_BILL_HOLD		48103
//TES 7/28/2014 - PLID 62785 - Added Edit Charges menu option
#define ID_OPEN_BILL			48104
// (s.tullis 2014-07-16 10:23) - PLID 62575 -Added Last Assignment Date
// (s.tullis 2014-07-08 12:52) - PLID 62527 - Added Bill Status Note
// (j.jones 2013-08-19 17:41) - PLID 42319 - added assign date,
// and turned defines into enums
// (r.gonet 07/25/2014) - PLID 62920 - Added ebfcNumColumns
// since we add columns dynamically and we need to know how many columns we have total.
// (r.gonet 07/28/2014) - PLID 63078 - Added Bill On Hold Icon column.
// (a.wilson 2014-08-01 09:59) - PLID 62784
enum BillingFUColumns {
	ebfcBillID = 0,
	ebfcInsuredPartyID,
	ebfcPatientID,
	ebfcBillNoteIcon,
	ebfcBillDate,
	ebfcBillInputDate,
	ebfcDateSent,
	ebfcFirstAssigned,
	ebfcLastAssigned,
	ebfcBillOnHoldIcon,
	ebfcBillStatus,
	ebfcPatientName,
	ebfcInsCoName,
	ebfcRespTypeName,
	ebfcTotalCharges,
	ebfcAppliedAmount,
	ebfcInsResp,
	ebfcLocationName,
	ebfcProviderName,
	ebfcPrimaryInsCoName,
	// Must be the last enumeration value.
	ebfcNumColumns,
};

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CBillingFollowUpDlg dialog


// (j.gruber 2013-03-27 10:28) - PLID 55872
namespace SecondaryRespList {
	enum RespListColumn {
		rlcID = 0,
		rlcName,
	};
};

// (s.tullis 2014-07-01 09:59) - PLID 62577 - In the Financial Module > Billing Follow Up tab, under the ‘Filters’ section, add a drop down for the user to be able to search for bills based on a status. The drop down should default to < All Statuses >. There should also be an option for the user to select < No Status >.
enum BillStatusList{
	bslNoteID=0,
	bslNote,
};

// (s.tullis 2014-06-30 11:56) - PLID 62574 - Enumerations for On Hold Filter selections
enum OnHoldFilter{
	ohfBoth =0,
	ohfYes ,
	ohfNo,

};
// (s.tullis 2014-06-30 11:56) - PLID 62574 - Enumerations for On Hold Filter Columns
enum OnHoldFilterColumn{
	ohfID = 0,
	ohfName,

};

// (s.tullis 2014-07-01 09:26) - PLID 62577 - Enums for Special fields for CMultiSelect
enum  ECriteriaSpecialIds {
	All = -3,
	Multiple = -2,
	None = -1,
};

// (a.wilson 2014-07-03 15:33) - PLID 62809 - filter options within the top dropdowns other than specific choices.
enum eFilterIDs {
	efidAll = -3,
	efidMultiple = -2,
};

enum ePatientFilterColumns {
	epfcID = 0,
	epfcUID = 1,
	epfcName = 2,
};

enum eMainFilterColumns{
	emfcID = 0,
	emfcName = 1,
};

enum eRespFilterOptions {
	erfoPatient = -3,
	erfoInsurance = -2,
};

// (a.wilson 2014-07-10 10:37) - PLID 62529 - adding a new date range.
enum eDateFilterOptions {
	edfoAllDates = -3,
	edfoClaimLastSentDate = 0,
	edfoBillServiceDate = 1,
	edfoBillInputDate = 2,
	edfoLastPaymentDate = 3,
	edfoFirstAssignmentDate = 4,
	edfoLastAssignmentDate = 5,
	edfoStatusDate = 6,
	edfoLastStatusNoteDate = 7,
};

enum eClaimSentFilter {
	ecsfYes,
	ecsfNo,
	ecsfBoth,
};

// (a.walling 2012-02-09 17:13) - PLID 45448 - NxAdo unification - Use CIncreaseCommandTimeout instead of accessing g_ptrRemoteData
// (s.tullis 2014-08-11 11:54) - PLID 62577 -  added bill status tablechecker
CBillingFollowUpDlg::CBillingFollowUpDlg(CWnd* pParent)
	: CNxDialog(CBillingFollowUpDlg::IDD, pParent),
	m_companyChecker(NetUtils::InsuranceCoT),
	m_patientChecker(NetUtils::PatCombo),
	m_locationChecker(NetUtils::LocationsT),
	m_providerChecker(NetUtils::Providers),
	m_BillStatusChecker(NetUtils::BillStatusT)
{
	//{{AFX_DATA_INIT(CBillingFollowUpDlg)
		m_strCurrentFrom = "";
		m_strCurrentDateRangeForReport = "All Dates";
	//}}AFX_DATA_INIT

	//(j.anspach 06-13-2005 13:42 PLID 16662) - Adding a specific help file reference to Financial -> Billing Follow-Up
	//PLID 21512: per Don, if we don't have anything to put here, default to the earliest thing we can which is new patient
	//m_strManualLocation = "NexTech_Practice_Manual.chm";
	//m_strManualBookmark = "Billing/billing_followup.htm";
	// (a.wilson 2014-07-07 09:23) - PLID 62809 - default values.
	m_nLocationID = efidAll;
	m_nCurrentPatientID = efidAll;
	m_nCurrentProviderID = efidAll;
	m_nCurrentInsuranceID = efidAll;
	// (r.gonet 07/25/2014) - PLID 62920 - Initialize the total number of static columns currently in the results list
	m_nResultsListStaticColumnCount = 0;
	// (r.gonet 07/28/2014) - PLID 63078 - Initialize the hand icon to null.
	m_hIconBillOnHold = NULL;
	m_hBillNote = NULL;	// (a.wilson 2014-08-01 10:01) - PLID 62784
}

// (r.gonet 07/28/2014) - PLID 63078 - Added a destructor
CBillingFollowUpDlg::~CBillingFollowUpDlg()
{
	// (r.gonet 07/28/2014) - PLID 63078 - Clean up the on hold icon.
	if (m_hIconBillOnHold != NULL) {
		DestroyIcon(m_hIconBillOnHold);
		m_hIconBillOnHold = NULL;
	}
	// (a.wilson 2014-08-01 10:02) - PLID 62784
	if (m_hBillNote != NULL) {
		DestroyIcon(m_hBillNote);
		m_hBillNote = NULL;
	}
}

void CBillingFollowUpDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBillingFollowUpDlg)
	DDX_Control(pDX, IDC_BTN_WRITE_OFF_ACCTS, m_btnWriteOffAccts);
	DDX_Control(pDX, IDC_BTN_SEARCH_ALLOWABLES, m_btnSearchAllowables);
	DDX_Control(pDX, IDC_CHECK_SAVE_MERGE_AS_TRACER, m_checkSaveAsTracer);
	DDX_Control(pDX, IDC_CHECK_NO_PATIENT_PAYMENTS, m_checkNoPatientPays);
	DDX_Control(pDX, IDC_BTN_LOAD, m_btnDisplayResults);
	DDX_Control(pDX, IDC_PREVIEW_LIST, m_btnPreviewList);
	DDX_Control(pDX, IDC_CHECK_NO_INSURANCE_PAYMENTS, m_checkNoInsPays);
	DDX_Control(pDX, IDC_SEND_TO_PAPER_BATCH, m_btnSendToPaper);
	DDX_Control(pDX, IDC_SEND_TO_EBILLING, m_btnSendToEbilling);
	DDX_Control(pDX, IDC_RADIO_BETWEEN_DATE_RANGE, m_radioBetweenDates);
	DDX_Control(pDX, IDC_RADIO_GREATER_THAN_DAYS, m_radioGreaterThanDays);
	DDX_Control(pDX, IDC_CREATE_LW_GROUP, m_btnCreateMergeGroup);
	DDX_Control(pDX, IDC_PREVIEW_FORMS, m_btnPreviewTracers);
	DDX_Control(pDX, IDC_MERGE_DOCUMENT, m_btnMergeToWord);
	DDX_Control(pDX, IDC_CHECK_TRACER_FORM_SENT, m_checkTracerFormSent);
	DDX_Control(pDX, IDC_CHECK_USE_BALANCE, m_checkUseBalance);
	DDX_Control(pDX, IDC_RADIO_TF_SENT_YES, m_radioTFSentYes);
	DDX_Control(pDX, IDC_RADIO_TF_SENT_NO, m_radioTFSentNo);
	DDX_Control(pDX, IDC_RADIO_TF_SENT_DATE, m_radioTFSentDate);
	DDX_Control(pDX, IDC_CLAIM_FROM_DATE, m_dtFrom);
	DDX_Control(pDX, IDC_CLAIM_TO_DATE, m_dtTo);
	DDX_Control(pDX, IDC_MERGE_TO_PRINTER, m_btnMergeToPrinter);
	DDX_Control(pDX, IDC_EDIT_DAYS_OLD, m_nxeditEditDaysOld);
	DDX_Control(pDX, IDC_EDIT_TF_DAYS, m_nxeditEditTfDays);
	DDX_Control(pDX, IDC_EDIT_DOLLARS, m_nxeditEditDollars);
	DDX_Control(pDX, IDC_UNPAID_BILLS_LABEL, m_nxstaticUnpaidBillsLabel);
	DDX_Control(pDX, IDC_RESULTS_COUNT, m_nxstaticResultsCount);
	DDX_Control(pDX, IDC_MULTI_LOC_LABEL, m_nxlLocationLabel); 
	DDX_Control(pDX, IDC_MULTI_BILL_STATUS, m_nxlMultipleBillStatus); 
	DDX_Control(pDX, IDC_BILLINGFOLLOWUP_REMEMBER_COLUMNS, m_checkRememberColumnWidths);
	DDX_Control(pDX, IDC_BILLINGFU_RESET, m_icoReset);
	//DDX_Control(pDX, IDC_BILLING_FU_SETUP_COLOR,)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CBillingFollowUpDlg, CNxDialog)
	//{{AFX_MSG_MAP(CBillingFollowUpDlg)
	ON_BN_CLICKED(IDC_GOTOPATIENT, OnGoToPatient)
	ON_BN_CLICKED(ID_REMOVE_SELECTED_BILLS, OnRemoveSelectedBills)
	ON_BN_CLICKED(ID_MARK_BILL_ON_HOLD, OnMarkBillOnHold)
	ON_BN_CLICKED(ID_REMOVE_BILL_HOLD, OnRemoveBillHold)
	ON_BN_CLICKED(IDC_CHECK_USE_BALANCE, OnCheckUseBalance)
	ON_BN_CLICKED(IDC_BTN_LOAD, OnBtnLoad)
	ON_BN_CLICKED(IDC_PREVIEW_FORMS, OnPreviewForms)
	ON_BN_CLICKED(IDC_SEND_TO_PAPER_BATCH, OnSendToPaperBatch)
	ON_BN_CLICKED(IDC_SEND_TO_EBILLING, OnSendToEbilling)
	ON_BN_CLICKED(IDC_MERGE_DOCUMENT, OnMergeDocument)
	ON_BN_CLICKED(IDC_CHECK_TRACER_FORM_SENT, OnCheckTracerFormSent)
	ON_BN_CLICKED(IDC_RADIO_TF_SENT_YES, OnRadioTfSentYes)
	ON_BN_CLICKED(IDC_RADIO_TF_SENT_NO, OnRadioTfSentNo)
	ON_BN_CLICKED(IDC_RADIO_TF_SENT_DATE, OnRadioTfSentDate)	
	ON_BN_CLICKED(IDC_CREATE_LW_GROUP, OnCreateLwGroup)
	ON_BN_CLICKED(IDC_RADIO_GREATER_THAN_DAYS, OnRadioGreaterThanDays)
	ON_BN_CLICKED(IDC_RADIO_BETWEEN_DATE_RANGE, OnRadioBetweenDateRange)
	ON_BN_CLICKED(IDC_PREVIEW_LIST, OnPreviewList)
	ON_BN_CLICKED(IDC_CHECK_SAVE_MERGE_AS_TRACER, OnCheckSaveMergeAsTracer)
	ON_BN_CLICKED(IDC_BTN_SEARCH_ALLOWABLES, OnBtnSearchAllowables)
	ON_BN_CLICKED(IDC_BTN_WRITE_OFF_ACCTS, OnBtnWriteOffAccts)
	ON_WM_SETCURSOR()
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick) // (j.gruber 2013-02-27 15:32) - PLID 54882
	//}}AFX_MSG_MAP	
	ON_BN_CLICKED(IDC_BILLINGFOLLOWUP_REMEMBER_COLUMNS, &CBillingFollowUpDlg::OnBnClickedBillingfollowupRememberColumns)
	ON_BN_CLICKED(IDC_CONFIGURE_COLUMNS_BTN, &CBillingFollowUpDlg::OnBnClickedConfigureColumnsBtn)
	ON_WM_DESTROY()
	ON_STN_CLICKED(IDC_BILLINGFU_RESET, &CBillingFollowUpDlg::OnStnClickedBillingfuReset)
	ON_COMMAND(ID_OPEN_BILL, OnOpenBill)
END_MESSAGE_MAP()

// (s.tullis 2014-07-01 09:53) - PLID 62577 - CallBack for Right CLick 
BOOL CALLBACK CMultiSelectDlg_ContextMenu(IN CMultiSelectDlg *pwndMultSelDlg, IN LPARAM pParam, IN NXDATALIST2Lib::IRowSettings *lpRow, IN CWnd* pContextWnd, IN const CPoint &point, IN OUT CArray<long, long> &m_aryOtherChangedMasterIDs)
{
	// The context menu for the data element list is based on the current selection
	if (lpRow) {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		// Build the menu for the current row
		CMenu mnu;
		mnu.CreatePopupMenu();
		mnu.AppendMenu(MF_ENABLED | MF_STRING | MF_BYPOSITION, 1, "&Select All");
		mnu.AppendMenu(MF_ENABLED | MF_STRING | MF_BYPOSITION, 2, "&Unselect All");

		// Pop up the menu and gather the immediate response
		CPoint pt = CalcContextMenuPos(pContextWnd, point);
		long nMenuResult = mnu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, pt.x, pt.y, pwndMultSelDlg);
		switch (nMenuResult) {
		case 1: // select all
		{
			// Check all the rows in the multi-select list that are unchecked
			NXDATALIST2Lib::IRowSettingsPtr pRowIter = pwndMultSelDlg->GetDataList2()->GetFirstRow();
			while (pRowIter) {
				if (!VarBool(pRowIter->GetValue(CMultiSelectDlg::mslcSelected), FALSE)) {
					pRowIter->PutValue(CMultiSelectDlg::mslcSelected, g_cvarTrue);
				}
				pRowIter = pRowIter->GetNextRow();
			}
			break;
		}
		case 2: // unselect all
		{
			// Uncheck all the rows in the multi-select list that are checked
			NXDATALIST2Lib::IRowSettingsPtr pRowIter = pwndMultSelDlg->GetDataList2()->GetFirstRow();
			while (pRowIter) {
				if (VarBool(pRowIter->GetValue(CMultiSelectDlg::mslcSelected), TRUE)) {
					pRowIter->PutValue(CMultiSelectDlg::mslcSelected, g_cvarFalse);
				}
				pRowIter = pRowIter->GetNextRow();
			}
			break;
		}
		case 0:
			// The user canceled, do nothing
			break;
		default:
			// Unexpected response!
			ASSERT(FALSE);
			ThrowNxException("%s : Unexpected return value %li from context menu!", __FUNCTION__, nMenuResult);
		}
		return TRUE;
	}
	else {
		return FALSE;
	}
}
/////////////////////////////////////////////////////////////////////////////
// CBillingFollowUpDlg message handlers

BOOL CBillingFollowUpDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	// (s.tullis 2014-07-14 16:47) - PLID 62574 - In the Billing Follow Up tab, in the ‘Claim Status’ section; add a filter drop down for “On Hold”.
	// (a.wilson 2014-07-09 11:09) - PLID 62528 - cache properties
	g_propManager.CachePropertiesInBulk("BillingFollowUpDlg", propNumber,
		R"((Username = '<None>' OR Username = '%s') AND (
		Name = 'RememberBillingFollowUpColumnWidths' OR 
		Name = 'SaveBillingFUMergeAsTracer' OR Name = 'BillOnHoldFilter')
		)", _Q(GetCurrentUserName()));

	// (j.jones 2008-05-07 14:39) - PLID 29854 - set button styles for modernization
	m_btnSearchAllowables.AutoSet(NXB_INSPECT);
	m_btnDisplayResults.AutoSet(NXB_INSPECT);
	m_btnSendToPaper.AutoSet(NXB_MODIFY);
	m_btnSendToEbilling.AutoSet(NXB_MODIFY);
	m_btnPreviewList.AutoSet(NXB_PRINT_PREV);
	m_btnPreviewTracers.AutoSet(NXB_PRINT_PREV);
	m_btnMergeToWord.AutoSet(NXB_MERGE);
	m_btnCreateMergeGroup.AutoSet(NXB_NEW);
	m_btnWriteOffAccts.AutoSet(NXB_INSPECT); // (j.jones 2008-06-27 13:45) - PLID 27647 - added m_btnWriteOffAccts

	m_hIconBillOnHold = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_HOLD), IMAGE_ICON, 16, 16, 0);
	m_hBillNote = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_BILL_NOTES), IMAGE_ICON, 16, 16, 0);	// (a.wilson 2014-08-01 10:03) - PLID 62784

	m_PatientList = BindNxDataListCtrl(IDC_PATIENT_LIST);
	m_pResultsList = BindNxDataList2Ctrl(IDC_BILLFU_RESULTS_LIST, false);
	m_pProviderCombo = BindNxDataListCtrl(IDC_FILTER_PROVIDER); //(e.lally 2009-08-14) PLID 30119
	m_pRespFilter = BindNxDataList2Ctrl(IDC_FILTER_RESP); // (a.wilson 2014-07-07 13:07) - PLID 62809
	m_pInsuranceFilter = BindNxDataList2Ctrl(IDC_FILTER_INSURANCE_LIST); // (a.wilson 2014-07-07 13:42) - PLID 62809
	m_pHCFAFilter = BindNxDataList2Ctrl(IDC_FILTER_HCFA_LIST); // (a.wilson 2014-07-07 13:42) - PLID 62809

	// (a.wilson 2014-07-07 12:07) - PLID 62809 - date filter to replace radio buttons.
	m_pDateFilter = BindNxDataList2Ctrl(IDC_FILTER_DATE_LIST, false);
	BuildDateFilter();

	// (j.jones 2009-08-11 08:37) - PLID 28131 - added location filter
	m_nLocationID = GetCurrentLocationID();
	m_LocationCombo = BindNxDataListCtrl(IDC_FILTER_LOCATION);
	{
	// (j.gruber 2013-04-26 09:25) - PLID 56467 - add to the array also
	m_dwLocations.RemoveAll(); //it should be empty but let's be sure
	m_dwLocations.Add(m_nLocationID);
	// (j.gruber 2013-02-27 13:07) - PLID 54882 - multiple locations
	//m_nxlLocationLabel.SetColor(0x00DEB05C);
	m_nxlLocationLabel.SetText("");
	m_nxlLocationLabel.SetType(dtsHyperlink);
	m_nxlLocationLabel.ShowWindow(FALSE);
	}

	//s.tullis- PLID 62577-  Bill status filter
	n_CurrentBillStatusID = ECriteriaSpecialIds::All;
	m_pBillStatusFilter = BindNxDataList2Ctrl(IDC_BILLSTATUS_CONTROL, false); 
	{
		// (s.tullis 2014-07-01 12:08) - PLID 62577 - need to hide this unless multiple bill status is selected
		m_nxlMultipleBillStatus.SetType(dtsDisabledHyperlink);
		m_nxlMultipleBillStatus.SetSingleLine(true);
		RequeryBillStatus(n_CurrentBillStatusID, false);
	}

	// (a.wilson 2014-07-07 15:40) - PLID 62809 - Claim Sent Filter
	m_pClaimSentFilter = BindNxDataList2Ctrl(IDC_FILTER_CLAIM_LIST, false);
	{
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pClaimSentFilter->GetNewRow();
		if (pRow) {
			pRow->PutValue(emfcID, ecsfYes);
			pRow->PutValue(emfcName, _bstr_t("Yes"));
			m_pClaimSentFilter->AddRowAtEnd(pRow, NULL);
		}
		pRow = m_pClaimSentFilter->GetNewRow();
		if (pRow) {
			pRow->PutValue(emfcID, ecsfNo);
			pRow->PutValue(emfcName, _bstr_t("No"));
			m_pClaimSentFilter->AddRowAtEnd(pRow, NULL);
		}
		pRow = m_pClaimSentFilter->GetNewRow();
		if (pRow) {
			pRow->PutValue(emfcID, ecsfBoth);
			pRow->PutValue(emfcName, _bstr_t("Both"));
			m_pClaimSentFilter->AddRowAtEnd(pRow, NULL);
		}
		m_pClaimSentFilter->FindByColumn(emfcID, ecsfBoth, NULL, VARIANT_TRUE);
	}

	// (s.tullis 2014-06-30 09:55) - PLID 62574 - Need to remember the filter per user
	m_pOnHoldFilterList = BindNxDataList2Ctrl(IDC_ONHOLD_FILTER, false); 
	{	
		long nOnHoldFilterSelect = GetRemotePropertyInt("BillOnHoldFilter", 0, 0, GetCurrentUserName(), TRUE);
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pOnHoldFilterList->GetNewRow();
		pRow->PutValue(ohfID, ohfBoth);
		pRow->PutValue(ohfName, bstr_t("Both"));

		m_pOnHoldFilterList->AddRowAtEnd(pRow, NULL);

		pRow = m_pOnHoldFilterList->GetNewRow();
		pRow->PutValue(ohfID, ohfYes);
		pRow->PutValue(ohfName, bstr_t("Yes"));
		m_pOnHoldFilterList->AddRowAtEnd(pRow, NULL);

		pRow = m_pOnHoldFilterList->GetNewRow();
		pRow->PutValue(ohfID, ohfNo);
		pRow->PutValue(ohfName, bstr_t("No"));
		m_pOnHoldFilterList->AddRowAtEnd(pRow, NULL);

		switch (nOnHoldFilterSelect)
		{
		case ohfYes:{// filter was set to yes
			m_pOnHoldFilterList->SetSelByColumn(ohfID, ohfYes);

		}
			break;
		case ohfNo:{// filter was set to no
			m_pOnHoldFilterList->SetSelByColumn(ohfID, ohfNo);
		}
			break;
		default:{// defaulted to both 
			m_pOnHoldFilterList->SetSelByColumn(ohfID, ohfBoth);
		}
			break;
		}
	}

	m_radioTFSentNo.SetCheck(TRUE);
	OnCheckTracerFormSent();

	//set the balance to 0 by default	
	m_checkUseBalance.SetCheck(TRUE);
	GetDlgItem(IDC_EDIT_DOLLARS)->EnableWindow(TRUE);
	SetDlgItemText(IDC_EDIT_DOLLARS,FormatCurrencyForInterface(COleCurrency(0,0),FALSE));

	//remember the last setting
	m_checkSaveAsTracer.SetCheck(GetRemotePropertyInt("SaveBillingFUMergeAsTracer", 0, 0, GetCurrentUserName(), true));

	// (a.wilson 2014-07-16 08:18) - PLID 62905 - load reset icon.
	m_icoReset.LoadToolTipIcon(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDB_RESET), CString("Click to reset top filters to their original state."), false, false, false);

	// (a.wilson 2014-07-09 10:58) - PLID 62528 - getting remote property to remember column widths.
	m_checkRememberColumnWidths.SetCheck(GetRemotePropertyInt("RememberBillingFollowUpColumnWidths", 0, 0, GetCurrentUserName(), true));

	// (a.wilson 2014-07-08 16:44) - PLID 62526 - create results list columns.
	SetupResultListColumns();
	// (s.tullis 2014-06-27 10:45) - PLID 62506 - Permission: Billing Followup User Permission to Control Read and Writing in the Billing Followup Tab
	SecureControls();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CBillingFollowUpDlg::OnCheckUseBalance() 
{
	if(m_checkUseBalance.GetCheck()) {
		GetDlgItem(IDC_EDIT_DOLLARS)->EnableWindow(TRUE);
		// (j.jones 2012-01-03 11:11) - PLID 46101 - "with no insurance pays applied" is not available for patient resp. filters
		// (j.gruber 2013-02-21 15:22) - PLID 47780 - changed to patient resp function
		m_checkNoInsPays.EnableWindow(!IsFilteringPatientResp());
		m_checkNoPatientPays.EnableWindow(TRUE);
		CString str;
		GetDlgItemText(IDC_EDIT_DOLLARS,str);
		if(str.GetLength() == 0)
			SetDlgItemText(IDC_EDIT_DOLLARS,FormatCurrencyForInterface(COleCurrency(0,0),FALSE));
	}
	else {
		GetDlgItem(IDC_EDIT_DOLLARS)->EnableWindow(FALSE);
		m_checkNoInsPays.EnableWindow(FALSE);
		m_checkNoPatientPays.EnableWindow(FALSE);
	}
}

void CBillingFollowUpDlg::OnBtnLoad() 
{
	try {
		CWaitCursor pWait;

		m_aryRemovedBills.RemoveAll();

		SetDlgItemInt(IDC_RESULTS_COUNT,0);

		m_pResultsList->Clear();

		CRect rc;
		GetDlgItem(IDC_RESULTS_COUNT)->GetWindowRect(rc);
		ScreenToClient(rc);
		InvalidateRect(rc);

		CString strFrom, strWhere = "";

		//DRT 6/11/03 - Do not change the fields selected in the strFrom clause - This strFrom is passed into the report, so if you change it, you
		//		need to make a new .ttx file for the report!

		CString strWhereClause = GetWhereClause();
		if(strWhereClause != "")
			strWhere.Format("HAVING %s", strWhereClause);

		// (j.gruber 2013-02-26 14:53) - PLID 47780 - are we filtering on patient resp?
		BOOL bIsFilterPatientResp = FALSE;
		if (IsFilteringPatientResp()) {
			bIsFilterPatientResp = TRUE;
		}
		// (j.jones 2008-04-23 11:00) - PLID 29760 - added InsuranceCoT.PersonID to the GROUP BY clause
		// (j.jones 2009-08-11 09:11) - PLID 28131 - added LocationID and name
		//(e.lally 2009-08-14) PLID 30119 - added ProviderID and name
		// (j.jones 2011-03-10 11:00) - PLID 41792 - added the patient's primary insurance company name
		// (j.jones 2011-04-01 14:34) - PLID 43110 - the primary insurance company is now the primary
		// company for the same type (Medical or Vision) as the displayed responsibility,
		// or just Medical if we are showing patient resp.
		// (j.jones 2011-08-17 11:00) - PLID 44888 - ignore "original" and "void" line items
		// (j.dinatale 2013-02-20 14:09) - PLID 16522 - added HistoryQ.BillID to the group by
		// (j.gruber 2013-02-26 14:54) - PLID 47780 - changed to account for multi-resp filtering
		// (j.jones 2013-08-19 15:15) - PLID 42319 - added RespAssignmentDate for each charge,
		// calculated as the earliest date there is a non-zero ChargeRespDetailT record, or the LineItemT.Date
		// if it is a $0.00 charge
		// (s.tullis 2014-06-30 10:56) - PLID 62574 - Added BillStatusT 
		// (s.tullis 2014-07-07 16:50) - PLID 62527 - Added Bill Status Note 
		// (a.wilson 2014-07-10 10:58) - PLID 62529 - Added Bill Status Note Date
		// (a.wilson 2014-08-01 10:59) - PLID 62784 - added note icon
		// (j.jones 2014-11-03 11:32) - PLID 63855 - Fixed First & Last Resp Assignment date to return the first or last
		// date from any charge on the bill. Previously, differing dates would cause multiple rows to be returned per bill.
		strFrom.Format(
			"(SELECT TotalsPerChargeQ.BillID, TotalsPerChargeQ.PatientID, TotalsPerChargeQ.LocationID, LocationsT.Name AS LocName, "
				"Sum(TotalCharges - TotalPays) AS Bal, Sum(TotalCharges) AS TC, Sum(TotalPays) AS TP, "
				"CASE WHEN TotalsPerChargeQ.BillID IN (SELECT BillID FROM Notes WHERE Notes.BillID IS NOT NULL) THEN 1 ELSE 0 END AS HasNotes, "
				"TotalsPerChargeQ.InsuredPartyID, InsuranceCoT.Name AS InsCoName, InsuredPartyT.RespTypeID, RespTypeT.TypeName AS RespTypeName, InsuranceCoT.HCFASetupGroupID, HistoryQ.LastDate, "
				"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, TotalsPerChargeQ.BillDate, ClaimHistoryTracerQ.LastTracerDate, TotalsPerChargeQ.BillInputDate AS InputDate, LastPaymentQ.LastPaymentDate, "
				"TotalsPerChargeQ.ProviderID, TotalsPerChargeQ.ProviderName, PrimaryInsuranceCoQ.Name AS PriInsCoName, Min(TotalsPerChargeQ.RespAssignmentDate) AS RespAssignmentDate, Max(TotalsPerChargeQ.LastRespAssignmentDate) AS LastRespAssignmentDate, "
				"TotalsPerChargeQ.OnHold, TotalsPerChargeQ.BillStatus, TotalsPerChargeQ.StatusDateModified, TotalsPerChargeQ.BillNoteID, TotalsPerChargeQ.BillStatusNote, TotalsPerChargeQ.BillStatusNoteDate "
				"FROM "
				"	(SELECT BillsT.ID AS BillID, BillsT.PatientID, LineItemT.LocationID, RespQ.ChargeID, RespQ.TotalCharges, RespQ.InsuredPartyID, BillsT.Date AS BillDate, "
				"	Sum(CASE WHEN PaysQ.Amount IS NULL THEN 0 ELSE PaysQ.Amount END) AS TotalPays, BillsT.InputDate AS BillInputDate, "
				"	ChargesT.DoctorsProviders AS ProviderID, ProviderPersonT.Last + ', ' + ProviderPersonT.First + ' ' + ProviderPersonT.Middle AS ProviderName, "
				"	Min(IsNull(RespQ.FirstRespAssignmentDate, LineItemT.Date)) AS RespAssignmentDate, Max(IsNull(RespQ.LastRespAssignmentDate, LineItemT.Date)) AS LastRespAssignmentDate, "
				"   BillStatusT.Type AS OnHold, BillStatusT.ID AS BillStatusID, BillStatusT.Name as BillStatus, BillsT.StatusModifiedDate as StatusDateModified, "
				"   NoteDataT.ID As BillNoteID, NoteDataT.Note As BillStatusNote, NoteDataT.NoteInputDate AS BillStatusNoteDate "
				"	FROM BillsT "
				"	INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
				"	LEFT JOIN PersonT ProviderPersonT ON ChargesT.DoctorsProviders = ProviderPersonT.ID "
				"	INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"	LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID " 
				"	LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID " 
				"	LEFT JOIN BillStatusT ON BillStatusT.ID= BillsT.StatusID "
				"	LEFT JOIN NoteDataT ON BillsT.StatusNoteID = NoteDataT.ID "
				"		INNER JOIN "
				"		(SELECT ChargeRespT.ID, ChargeID, Sum(ChargeRespT.Amount) AS TotalCharges, "
				"		CASE WHEN InsuredPartyID IS NULL THEN -1 ELSE InsuredPartyID END AS InsuredPartyID, "
				"		Min(RespAssignmentDateQ.FirstDate) AS FirstRespAssignmentDate, MAX(RespAssignmentDateQ.LastDate) AS LastRespAssignmentDate "
				"		FROM ChargeRespT "
				"		LEFT JOIN (SELECT Min(Date) AS FirstDate, Max(Date) as LastDate, ChargeRespID "
				"			FROM ChargeRespDetailT "
				"			WHERE Amount > 0 "
				"			GROUP BY ChargeRespID) AS RespAssignmentDateQ ON ChargeRespT.ID = RespAssignmentDateQ.ChargeRespID "
				"		LEFT JOIN ChargesT ON ChargeRespT.ChargeID = ChargesT.ID INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"		LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID " 
				"		LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID " 
				"		WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 10 "
				"		AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"		AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"		AND ChargeRespT.InsuredPartyID %s "
				"		GROUP BY ChargeRespT.ID, ChargeID, InsuredPartyID "
				"		) RespQ "
				"	ON ChargesT.ID = RespQ.ChargeID "
				"	LEFT JOIN "
				"		(SELECT AppliesT.RespID, Sum(AppliesT.Amount) AS Amount "
				"			FROM PaymentsT "
				"			INNER JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
				"			LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalPaymentsQ ON PaymentsT.ID = LineItemCorrections_OriginalPaymentsQ.OriginalLineItemID " 
				"			LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingPaymentsQ ON PaymentsT.ID = LineItemCorrections_VoidingPaymentsQ.VoidingLineItemID " 
				"			WHERE LineItemT.Deleted = 0 AND PaymentsT.InsuredPartyID %s "
				"			GROUP BY AppliesT.RespID "
				"		) PaysQ "
				"	ON RespQ.ID = PaysQ.RespID "
				"	WHERE LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"	AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"	GROUP BY BillsT.ID, BillsT.PatientID, LineItemT.LocationID, RespQ.ChargeID, RespQ.TotalCharges, RespQ.InsuredPartyID, BillsT.Date, BillsT.InputDate, "
				"		ChargesT.DoctorsProviders, ProviderPersonT.Last + ', ' + ProviderPersonT.First + ' ' + ProviderPersonT.Middle, "
				"		BillStatusT.Type, BillStatusT.ID, BillStatusT.Name, BillsT.StatusModifiedDate,  NoteDataT.ID, NoteDataT.Note, NoteDataT.NoteInputDate "
				"	) TotalsPerChargeQ "
				"LEFT JOIN LocationsT ON TotalsPerChargeQ.LocationID = LocationsT.ID "
				"LEFT JOIN InsuredPartyT ON TotalsPerChargeQ.InsuredPartyID = InsuredPartyT.PersonID "
				"LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
				"LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				"LEFT JOIN PersonT ON TotalsPerChargeQ.PatientID = PersonT.ID "
				"LEFT JOIN "
				"	(SELECT InsuredPartyT.PatientID, InsuranceCoT.Name, RespTypeT.CategoryType "
				"	FROM InsuranceCoT "
				"	INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID "
				"	INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				"	WHERE RespTypeT.CategoryPlacement = 1 "
				"	) AS PrimaryInsuranceCoQ ON "
				"		TotalsPerChargeQ.PatientID = PrimaryInsuranceCoQ.PatientID "
				"		AND Coalesce(RespTypeT.CategoryType, 1) = PrimaryInsuranceCoQ.CategoryType "
				"LEFT JOIN "
				"	(SELECT Max(Date) AS LastDate, ClaimHistoryT.BillID, InsuredPartyID "
				"	FROM ClaimHistoryT "
				//(r.wilson 10/2/2012) plid 53082 - the line below was "   WHERE SendType >= 0 "
				"	WHERE SendType >= %li "
				"	GROUP BY ClaimHistoryT.BillID, InsuredPartyID "
				"	) HistoryQ "
				"ON TotalsPerChargeQ.BillID = HistoryQ.BillID AND TotalsPerChargeQ.InsuredPartyID = HistoryQ.InsuredPartyID "
				"LEFT JOIN "
				//(r.wilson 10/2/2012) plid 53082 - below SendType was -1
				"	(SELECT BillID, Max(Date) AS LastTracerDate, InsuredPartyID FROM ClaimHistoryT WHERE SendType = %li GROUP BY BillID, InsuredPartyID "
				"	) AS ClaimHistoryTracerQ "
				"ON TotalsPerChargeQ.BillID = ClaimHistoryTracerQ.BillID AND TotalsPerChargeQ.InsuredPartyID = ClaimHistoryTracerQ.InsuredPartyID "
				"LEFT JOIN "
				"	(SELECT Max(LineItemT.Date) AS LastPaymentDate, PaymentsT.InsuredPartyID, ChargesT.BillID "
				"	FROM PaymentsT "
				"	INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
				"	INNER JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID "
				"	INNER JOIN ChargesT ON AppliesT.DestID = ChargesT.ID "
				"	INNER JOIN LineItemT LineItemT2 ON ChargesT.ID = LineItemT2.ID "
				"	LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON ChargesT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID " 
				"	LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON ChargesT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID " 
				"	LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalPaymentsQ ON PaymentsT.ID = LineItemCorrections_OriginalPaymentsQ.OriginalLineItemID " 
				"	LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingPaymentsQ ON PaymentsT.ID = LineItemCorrections_VoidingPaymentsQ.VoidingLineItemID " 
				"	WHERE LineItemT.Deleted = 0 AND LineItemT2.Deleted = 0 "
				"	AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"	AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"	AND LineItemCorrections_OriginalPaymentsQ.OriginalLineItemID Is Null "
				"	AND LineItemCorrections_VoidingPaymentsQ.VoidingLineItemID Is Null "
				"	GROUP BY PaymentsT.InsuredPartyID, ChargesT.BillID "
				"	) LastPaymentQ "
				"ON TotalsPerChargeQ.BillID = LastPaymentQ.BillID AND TotalsPerChargeQ.InsuredPartyID = LastPaymentQ.InsuredPartyID "
				
				"GROUP BY TotalsPerChargeQ.BillID, TotalsPerChargeQ.PatientID, TotalsPerChargeQ.LocationID, LocationsT.Name, "
				"TotalsPerChargeQ.InsuredPartyID, InsuranceCoT.Name, InsuredPartyT.RespTypeID, InsuranceCoT.PersonID, RespTypeT.TypeName, InsuranceCoT.HCFASetupGroupID, HistoryQ.BillID, HistoryQ.LastDate, "
				"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, TotalsPerChargeQ.BillDate, "
				"TotalsPerChargeQ.ProviderID, TotalsPerChargeQ.ProviderName, PrimaryInsuranceCoQ.Name, "
				"ClaimHistoryTracerQ.LastTracerDate, TotalsPerChargeQ.BillInputDate, LastPaymentQ.LastPaymentDate, "
				"TotalsPerChargeQ.OnHold, TotalsPerChargeQ.BillStatusID, TotalsPerChargeQ.BillStatus, "
				"TotalsPerChargeQ.StatusDateModified, TotalsPerChargeQ.BillNoteID, TotalsPerChargeQ.BillStatusNote, TotalsPerChargeQ.BillStatusNoteDate " 
				""
				"/*Apply filters here */ "
				"	%s "
				"/* Done filters */ "
				""
				") Q ",
				(bIsFilterPatientResp ? " IS NULL " : " IS NOT NULL "), 
				(bIsFilterPatientResp ? " = -1 " : " <> -1 "), 
				ClaimSendType::Electronic, ClaimSendType::TracerLetter,strWhere);

		//disable buttons while it is requerying
		EnableListControls(FALSE);

		//Change the timeout so we don't get an error.
		m_pIncreaseCommandTimeout.reset(new CIncreaseCommandTimeout(600));

		m_pResultsList->FromClause = _bstr_t(strFrom);
		m_strCurrentFrom = strFrom;
		m_pResultsList->Requery();

		//the timeout is reset in OnRequeryFinished

		// (a.wilson 2014-07-08 11:30) - PLID 62573 - rewrote this for updated functionality.
		//DRT 6/14/2007 - PLID 21395 - We wish to display the date ranges selected on the report.  However, the "Display Results" button is
		//	not tied to the "Preview List" in any way.  You could display results then change all your filters and not update the results.  So
		//	we save the date range here in a member variable, and if the report is previewed, we display it.
		m_strCurrentDateRangeForReport = "All Dates";
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pDateFilter->GetCurSel();
		if (pRow) {
			CString strDateType;
			eDateFilterOptions eOption = (eDateFilterOptions)VarLong(pRow->GetValue(emfcID), edfoAllDates);

			if (eOption != edfoAllDates) {
				switch (eOption) {
					case edfoFirstAssignmentDate:
						strDateType = "Assignment Date";
						break;
					case edfoBillServiceDate:
				strDateType = "Bill Date";
						break;
					case edfoBillInputDate:
				strDateType = "Bill Input Date";
						break;
					case edfoClaimLastSentDate:
						strDateType = "Claim Last Sent";
						break;
					case edfoLastPaymentDate:
				strDateType = "Last Payment Date";
						break;
					case edfoStatusDate:
						strDateType = "Status Date";
						break;
					case edfoLastAssignmentDate:
						strDateType = "Last Assignment Date";
					case edfoLastStatusNoteDate: // (a.wilson 2014-07-10 10:53) - PLID 62529
						strDateType = "Last Status Note Date";
					case edfoAllDates:
					default:
						//this should not happen. leave as all dates.
						break;
			}

				COleDateTime dtFrom = m_dtFrom.GetValue();
				COleDateTime dtTo = m_dtTo.GetValue();

				m_strCurrentDateRangeForReport.Format("%s between %s and %s", strDateType,  FormatDateTimeForInterface(dtFrom, 0, dtoDate),  FormatDateTimeForInterface(dtTo, 0, dtoDate));
			}
			else if(IsDlgButtonChecked(IDC_RADIO_GREATER_THAN_DAYS)) {
				long nDays = GetDlgItemInt(IDC_EDIT_DAYS_OLD);

				m_strCurrentDateRangeForReport.Format("%s greater than %li days old", strDateType, nDays);
			}
			else {
				//Should not be possible, leave the text on "All Dates"
			}
		}
		
	}NxCatchAll("Error loading data.");
}

// (s.tullis 2014-06-24 14:51) - PLID 62506 - Permission: Billing Followup User Permission to Control Read and Writing in the Billing Followup Tab
void CBillingFollowUpDlg::SecureControls() {

	try{

		BOOL bCanWrite = (GetCurrentUserPermissions(bioBillingfollowup)  & SPT___W_______);

		GetDlgItem(IDC_BTN_WRITE_OFF_ACCTS)->EnableWindow(bCanWrite);
		GetDlgItem(IDC_SEND_TO_PAPER_BATCH)->EnableWindow(bCanWrite);
		GetDlgItem(IDC_SEND_TO_EBILLING)->EnableWindow(bCanWrite);
		GetDlgItem(IDC_CREATE_LW_GROUP)->EnableWindow(bCanWrite);
		GetDlgItem(IDC_MERGE_TO_PRINTER)->EnableWindow(bCanWrite);
		GetDlgItem(IDC_CHECK_SAVE_MERGE_AS_TRACER)->EnableWindow(bCanWrite);
		GetDlgItem(IDC_MERGE_DOCUMENT)->EnableWindow(bCanWrite);
		GetDlgItem(IDC_PREVIEW_FORMS)->EnableWindow(bCanWrite);
		
	}NxCatchAll(__FUNCTION__)
}

void CBillingFollowUpDlg::OnPreviewForms() 
{
	try {

		CWaitCursor pWait;
		CString filter, param;

		if(m_strCurrentFrom == "") {
			MsgBox("There are no results to display!");
			return;
		}

		CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(365)]);
		CString str;
		str.Format("(SELECT * FROM %s) SubQ", m_strCurrentFrom);
		infReport.SetExtraValue(GetIgnoredBillList());	//filter on bills
		infReport.strExtendedSql = str;	//pass in the subquery through the extended filter

		//Made new function for running reports - JMM 5-28-04
		RunReport(&infReport, true, this, "Billing Follow-Up");
		
	}NxCatchAll("Error previewing tracer forms.");
}

// (a.wilson 2014-07-08 09:03) - PLID 62573 - cleaned up a lot of this while implementing the updated version of the filters.
CString CBillingFollowUpDlg::GetWhereClause()
{
	//Builds a filter of everything chosen in this dialog (including the resp filters).  This filter is specifically
	//	designed to build into the existing from clause (it's not really the Where clause, the the where clause of
	//	part of the FROM clause subqueries).
	//1)  Responsibility - Patient / Insurance Co / HCFA Group
	//2)  Greater than XX days old (this could be last sent date, bill date, bill input date)
	//3)  Had tracer form sent (yes/no/over xx days ago)
	//4)  Balance greater than YY.YY amount (ins bal or total bal) TODO:  does this total still apply?
	//5)  Patient ZZZZZ
	//6)  Location (the bill location of the charges)
	//7)  Charge Provider

	CString strWhere = "";
	long nCurSel = -1;

#pragma region RESPONSIBILITY FILTER
	//Patient Responsibility
	//if (IsFilteringPatientResp()) {
	//	//hide any columns to do with insurance.
	//	m_pResultsList->GetColumn(ebfcInsCoName)->PutStoredWidth(0);
	//	m_pResultsList->GetColumn(ebfcInsCoName)->ColumnStyle = csFixedWidth | csVisible;
	//	m_pResultsList->GetColumn(ebfcRespTypeName)->PutStoredWidth(0);
	//	m_pResultsList->GetColumn(ebfcRespTypeName)->ColumnStyle = csFixedWidth | csVisible;
	//	m_pResultsList->GetColumn(ebfcPrimaryInsCoName)->PutStoredWidth(0);
	//	m_pResultsList->GetColumn(ebfcPrimaryInsCoName)->ColumnStyle = csFixedWidth | csVisible;
	//}
	//Insurance Responsibility
	if (!IsFilteringPatientResp()) {
		NXDATALIST2Lib::IRowSettingsPtr pRespRow = m_pRespFilter->GetCurSel();
		if (pRespRow) {
			CString strRespWhere;
			long nRespTypeID = VarLong(pRespRow->GetValue(emfcID), erfoInsurance);

			if (nRespTypeID > erfoInsurance) {
				strRespWhere.Format("InsuredPartyT.RespTypeID = %li AND ", nRespTypeID);
		}

			//m_pResultsList->GetColumn(ebfcInsCoName)->PutStoredWidth(80);
			//m_pResultsList->GetColumn(ebfcInsCoName)->ColumnStyle = csWidthData | csVisible;
			//m_pResultsList->GetColumn(ebfcRespTypeName)->PutStoredWidth(50);
			//m_pResultsList->GetColumn(ebfcRespTypeName)->ColumnStyle = csWidthData | csVisible;
			//m_pResultsList->GetColumn(ebfcPrimaryInsCoName)->PutStoredWidth(80);
			//m_pResultsList->GetColumn(ebfcPrimaryInsCoName)->ColumnStyle = csWidthData | csVisible;

			strWhere += strRespWhere;
		}
	}
#pragma endregion RESPONSIBILITY FILTER

#pragma region HCFA FILTER
	//this is not an option if they are filtering on patient responsibility.
	if (!IsFilteringPatientResp()) {
		NXDATALIST2Lib::IRowSettingsPtr pHCFARow = m_pHCFAFilter->GetCurSel();
		if (pHCFARow && VarLong(pHCFARow->GetValue(emfcID), efidAll) != (long)efidAll) {
			CString strHCFAWhere;
			long nHCFAID = VarLong(pHCFARow->GetValue(emfcID), efidAll);
			strHCFAWhere.Format("InsuranceCoT.HCFASetupGroupID = %li AND ", nHCFAID);
				
			//m_pResultsList->GetColumn(ebfcInsCoName)->PutStoredWidth(80);
			//m_pResultsList->GetColumn(ebfcInsCoName)->ColumnStyle = csWidthData | csVisible;
			//m_pResultsList->GetColumn(ebfcRespTypeName)->PutStoredWidth(50);
			//m_pResultsList->GetColumn(ebfcRespTypeName)->ColumnStyle = csWidthData | csVisible;
			//m_pResultsList->GetColumn(ebfcPrimaryInsCoName)->PutStoredWidth(80);
			//m_pResultsList->GetColumn(ebfcPrimaryInsCoName)->ColumnStyle = csWidthData | csVisible;
			
			strWhere += strHCFAWhere;
		}
	}
#pragma endregion HCFA FILTER

#pragma region INSURANCE FILTER
	//this is not an option if they are filtering on patient responsibility.
	if (!IsFilteringPatientResp()) {
		NXDATALIST2Lib::IRowSettingsPtr pInsRow = m_pInsuranceFilter->GetCurSel();
		if (pInsRow && VarLong(pInsRow->GetValue(emfcID), efidAll) != (long)efidAll) {
			CString strInsWhere;
			long nInsID = VarLong(pInsRow->GetValue(emfcID), efidAll);
			strInsWhere.Format("InsuranceCoT.PersonID = %li AND ", nInsID);

			//m_pResultsList->GetColumn(ebfcInsCoName)->PutStoredWidth(0);
			//m_pResultsList->GetColumn(ebfcInsCoName)->ColumnStyle = csFixedWidth | csVisible;
			//m_pResultsList->GetColumn(ebfcRespTypeName)->PutStoredWidth(50);
			//m_pResultsList->GetColumn(ebfcRespTypeName)->ColumnStyle = csWidthData | csVisible;
			//m_pResultsList->GetColumn(ebfcPrimaryInsCoName)->PutStoredWidth(80);
			//m_pResultsList->GetColumn(ebfcPrimaryInsCoName)->ColumnStyle = csWidthData | csVisible;

			strWhere += strInsWhere;
		}
	}
#pragma endregion INSURANCE FILTER

#pragma region CLAIM SENT
	NXDATALIST2Lib::IRowSettingsPtr pClaimRow = m_pClaimSentFilter->GetCurSel();
	if (pClaimRow) {
		eClaimSentFilter eClaimID = (eClaimSentFilter)VarLong(pClaimRow->GetValue(emfcID), (long)ecsfBoth);

		switch (eClaimID) {
			case ecsfYes:
				strWhere += "HistoryQ.BillID IS NOT NULL AND ";
				break;
			case ecsfNo:
				strWhere += "HistoryQ.BillID IS NULL AND ";
				break;
			case ecsfBoth:
			default:
				//do nothing.
				break;
		}
	}
#pragma endregion CLAIM SENT

#pragma region ON HOLD FILTER
	//ON HOLD FILTER
	// (s.tullis 2014-06-30 14:12) - PLID 62574 - In the Billing Follow Up tab, in the ‘Claim Status’ section; add a filter drop down for “On Hold”.
	NXDATALIST2Lib::IRowSettingsPtr pOnHoldRow = m_pOnHoldFilterList->GetCurSel();
	if (pOnHoldRow) {
		long selection = VarLong(pOnHoldRow->GetValue(0));
		OnHoldFilter eChoice;
		eChoice = (OnHoldFilter)selection;

		switch (eChoice) {
		case ohfYes: // we want bills that are on Hold
			strWhere += " (TotalsPerChargeQ.OnHold=1)  AND ";
			break;

		case ohfNo: //  we want bills that are not on hold
			strWhere += " (TotalsPerChargeQ.OnHold <> 1 OR TotalsPerChargeQ.OnHold IS NULL ) AND ";

			break;
		case ohfBoth:
			//don't need to filter 
			break;
		default:
			// Only three choices this should not happen
			ASSERT(FALSE);
			break;
	}
	}
#pragma endregion ON HOLD FILTER
	
#pragma region BILL STATUS FILTER
	//BILL STATUS FILTER
	// (s.tullis 2014-07-01 12:08) - PLID 62577 - Bill status where clause builder
	NXDATALIST2Lib::IRowSettingsPtr pBillStatusRow = m_pBillStatusFilter->GetCurSel();
	if (pBillStatusRow) {
		long nSelection = VarLong(pBillStatusRow->GetValue(bslNoteID));

		switch ((ECriteriaSpecialIds)nSelection)
		{
		case ECriteriaSpecialIds::All:
			// we want all dont need to filter
			break;
		case ECriteriaSpecialIds::Multiple:
			// we want the ID's in our MultiSelect Array
			strWhere += " (TotalsPerChargeQ.BillStatusID IN ( " + GetArrayOfIDsForSQL(m_aryStatusesSelected) + " ))  AND ";
			break;
		case ECriteriaSpecialIds::None:
			//We only want bills with no Bill Note
			strWhere += " (TotalsPerChargeQ.BillStatusID IS NULL) AND ";
			break;
		default:
			// Just want one ID 
			strWhere += " (TotalsPerChargeQ.BillStatusID = " + GetArrayOfIDsForSQL(m_aryStatusesSelected) + ") AND ";
			break;
				}
			}
#pragma endregion BILL STATUS FILTER

#pragma region DATE FILTER
	// (a.wilson 2014-07-07 13:58) - PLID 62573 - rewritten to handle new dropdown redesign.
	{
		NXDATALIST2Lib::IRowSettingsPtr pDateRow = m_pDateFilter->GetCurSel();
		if (pDateRow) {
			eDateFilterOptions eDateOption = (eDateFilterOptions)VarLong(pDateRow->GetValue(emfcID), (long)edfoAllDates);
			//ingore all of this if the dropdown is set for all dates.
			if (eDateOption != edfoAllDates) {
				//if greater than is set do this.
		if(m_radioGreaterThanDays.GetCheck()) {
					long nDays = atol(m_nxeditEditDaysOld.GetText());
			CString strDayWhere;
					switch (eDateOption) 
					{
						case edfoClaimLastSentDate:
							strDayWhere.Format("(HistoryQ.BillID IS NULL OR (HistoryQ.LastDate IS NOT NULL AND convert(datetime, convert(nvarchar, HistoryQ.LastDate, 10)) <= (convert(datetime, convert(nvarchar, GetDate(), 10)) - %li))) ", nDays);
							break;
						case edfoBillServiceDate:
							strDayWhere.Format("(convert(datetime, convert(nvarchar, TotalsPerChargeQ.BillDate, 10)) <= (convert(datetime, convert(nvarchar, GetDate(), 10)) - %li)) ", nDays);
							break;
						case edfoBillInputDate:
							strDayWhere.Format("(convert(datetime, convert(nvarchar, TotalsPerChargeQ.BillInputDate, 10)) <= (convert(datetime, convert(nvarchar, GetDate(), 10)) - %li)) ", nDays);
							break;
						case edfoLastPaymentDate:
							strDayWhere.Format("LastPaymentQ.LastPaymentDate IS NOT NULL AND (convert(datetime, convert(nvarchar, LastPaymentQ.LastPaymentDate, 10)) <= (convert(datetime, convert(nvarchar, GetDate(), 10)) - %li)) ", nDays);
							break;
						case edfoFirstAssignmentDate:
							// (j.jones 2014-11-03 11:32) - PLID 63855 - Fixed First & Last Resp Assignment date to return the first or last
							// date from any charge on the bill. Previously, differing dates would cause multiple rows to be returned per bill.
							strDayWhere.Format("(convert(datetime, convert(nvarchar, Min(TotalsPerChargeQ.RespAssignmentDate), 10)) <= (convert(datetime, convert(nvarchar, GetDate(), 10)) - %li)) ", nDays);
							break;
						case edfoStatusDate:// (s.tullis 2014-07-07 16:11) - PLID 62576 - Added Bill Status Last Modified
							strDayWhere.Format("(convert(datetime, convert(nvarchar, TotalsPerChargeQ.StatusDateModified, 10)) <= (convert(datetime, convert(nvarchar, GetDate(), 10)) - %li)) ", nDays);
							break;
						case edfoLastAssignmentDate:// (s.tullis 2014-07-07 14:39) - PLID 62575 - Added Last Assignment
							// (j.jones 2014-11-03 11:32) - PLID 63855 - Fixed First & Last Resp Assignment date to return the first or last
							// date from any charge on the bill. Previously, differing dates would cause multiple rows to be returned per bill.
							strDayWhere.Format("(convert(datetime, convert(nvarchar, Max(TotalsPerChargeQ.LastRespAssignmentDate), 10)) <= (convert(datetime, convert(nvarchar, GetDate(), 10)) - %li)) ", nDays);
							break;
						case edfoLastStatusNoteDate: // (a.wilson 2014-07-10 10:59) - PLID 62529
							strDayWhere.Format("(convert(datetime, convert(nvarchar, TotalsPerChargeQ.BillStatusNoteDate, 10)) <= (convert(datetime, convert(nvarchar, GetDate(), 10)) - %li)) ", nDays);
							break;
						
			}
			strWhere += strDayWhere + " AND ";
		}
				//if between is set use this filter.
		else if(m_radioBetweenDates.GetCheck()) {
					COleDateTime dtFrom = COleDateTime(m_dtFrom.GetValue()), dtTo = COleDateTime(m_dtTo.GetValue());

					if (dtFrom.GetStatus() != COleDateTime::valid) {
				MsgBox("Your 'from' date is invalid. Please correct your date range.");
				return "0 = 1";
			}
					if (dtTo.GetStatus() != COleDateTime::valid) {
				MsgBox("Your 'to' date is invalid. Please correct your date range.");
				return "0 = 1";
			}
			if(dtFrom > dtTo) {
				MsgBox("Your 'from' date is after your 'to' date. Please correct your date range.");
				return "0 = 1";
			}
					//add a single day to the range to include the day set.
					dtTo += COleDateTimeSpan(1, 0, 0, 0);

					CString strFrom = FormatDateTimeForSql(dtFrom, dtoDate), strTo = FormatDateTimeForSql(dtTo, dtoDate);		
					CString strDayWhere;
					switch (eDateOption)
					{
					case edfoClaimLastSentDate:
						strDayWhere.Format("(HistoryQ.BillID IS NULL OR (HistoryQ.LastDate IS NOT NULL AND convert(datetime, convert(nvarchar, HistoryQ.LastDate, 10)) >= convert(datetime, convert(nvarchar, '%s', 10)) AND convert(datetime, convert(nvarchar, HistoryQ.LastDate, 10)) < convert(datetime, convert(nvarchar, '%s', 10)))) ",_Q(strFrom),_Q(strTo));
						break;
					case edfoBillServiceDate:
						strDayWhere.Format("(convert(datetime, convert(nvarchar, TotalsPerChargeQ.BillDate, 10)) >= convert(datetime, convert(nvarchar, '%s', 10)) AND convert(datetime, convert(nvarchar, TotalsPerChargeQ.BillDate, 10)) < convert(datetime, convert(nvarchar, '%s', 10))) ",_Q(strFrom), _Q(strTo));
						break;
					case edfoBillInputDate:
						strDayWhere.Format("(convert(datetime, convert(nvarchar, TotalsPerChargeQ.BillInputDate, 10)) >= convert(datetime, convert(nvarchar, '%s', 10)) AND convert(datetime, convert(nvarchar, TotalsPerChargeQ.BillInputDate, 10)) < convert(datetime, convert(nvarchar, '%s', 10))) ",_Q(strFrom),_Q(strTo));
						break;
					case edfoLastPaymentDate:
						strDayWhere.Format("LastPaymentQ.LastPaymentDate IS NOT NULL AND (convert(datetime, convert(nvarchar, LastPaymentQ.LastPaymentDate, 10)) >= convert(datetime, convert(nvarchar, '%s', 10)) AND convert(datetime, convert(nvarchar, LastPaymentQ.LastPaymentDate, 10)) < convert(datetime, convert(nvarchar, '%s', 10))) ",_Q(strFrom),_Q(strTo));
						break;
					case edfoFirstAssignmentDate:
						// (j.jones 2014-11-03 11:32) - PLID 63855 - Fixed First & Last Resp Assignment date to return the first or last
						// date from any charge on the bill. Previously, differing dates would cause multiple rows to be returned per bill.
						strDayWhere.Format("(convert(datetime, convert(nvarchar, Min(TotalsPerChargeQ.RespAssignmentDate), 10)) >= convert(datetime, convert(nvarchar, '%s', 10)) AND convert(datetime, convert(nvarchar, Min(TotalsPerChargeQ.RespAssignmentDate), 10)) < convert(datetime, convert(nvarchar, '%s', 10))) ", _Q(strFrom),_Q(strTo));
						break;
					case edfoStatusDate:// (s.tullis 2014-07-07 16:11) - PLID 62576 -- Added Bill status last modified
						strDayWhere.Format("(convert(datetime, convert(nvarchar, TotalsPerChargeQ.StatusDateModified, 10)) >= convert(datetime, convert(nvarchar, '%s', 10)) AND convert(datetime, convert(nvarchar, TotalsPerChargeQ.StatusDateModified, 10)) < convert(datetime, convert(nvarchar, '%s', 10))) ", _Q(strFrom), _Q(strTo));
						break;
					case edfoLastAssignmentDate:// (s.tullis 2014-07-07 14:39) - PLID 62575 - Added Last Assignment
						// (j.jones 2014-11-03 11:32) - PLID 63855 - Fixed First & Last Resp Assignment date to return the first or last
						// date from any charge on the bill. Previously, differing dates would cause multiple rows to be returned per bill.
						strDayWhere.Format("(convert(datetime, convert(nvarchar, Max(TotalsPerChargeQ.LastRespAssignmentDate), 10)) >= convert(datetime, convert(nvarchar, '%s', 10)) AND convert(datetime, convert(nvarchar, Max(TotalsPerChargeQ.LastRespAssignmentDate), 10)) < convert(datetime, convert(nvarchar, '%s', 10))) ", _Q(strFrom), _Q(strTo));
						break;
					case edfoLastStatusNoteDate: // (a.wilson 2014-07-10 11:01) - PLID 62529
						strDayWhere.Format("(convert(datetime, convert(nvarchar, TotalsPerChargeQ.BillStatusNoteDate, 10)) >= convert(datetime, convert(nvarchar, '%s', 10)) AND convert(datetime, convert(nvarchar, TotalsPerChargeQ.BillStatusNoteDate, 10)) < convert(datetime, convert(nvarchar, '%s', 10))) ", _Q(strFrom), _Q(strTo));
						break;
			}
			strWhere += strDayWhere + " AND ";
		}
	}
		}
	}
#pragma endregion DATE FILTER

#pragma region TRACER FORM
	//Tracer form sent (3)
	if(m_checkTracerFormSent.GetCheck()) {
		int days;
		days = GetDlgItemInt(IDC_EDIT_TF_DAYS);
		CString strDayWhere;
		if(m_radioTFSentYes.GetCheck())
			strDayWhere.Format("(ClaimHistoryTracerQ.LastTracerDate Is Not Null) ");
		else if(m_radioTFSentNo.GetCheck())
			strDayWhere.Format("(ClaimHistoryTracerQ.LastTracerDate Is Null) ");
		else if(m_radioTFSentDate.GetCheck())
			strDayWhere.Format("(ClaimHistoryTracerQ.LastTracerDate IS NOT NULL AND convert(datetime, convert(nvarchar, ClaimHistoryTracerQ.LastTracerDate, 10)) <= (convert(datetime, convert(nvarchar, GetDate(), 10)) - %li)) ",days);

		strWhere += strDayWhere + " AND ";
	}
#pragma endregion TRACER FORM

#pragma region BALANCES FILTER
	//Bill Balance greater than YY.YY (4)
	if(m_checkUseBalance.GetCheck()) {
		CString strBalance;
		GetDlgItemText(IDC_EDIT_DOLLARS,strBalance);
		if(strBalance.GetLength() == 0 || !IsValidCurrencyText(strBalance)) {
			MsgBox("Please enter a valid currency in the 'unpaid balance' box.");
			return "0 = 1";
		}
		strBalance = FormatCurrencyForSql(ParseCurrencyFromInterface(strBalance));

		CString strBalWhere;
		strBalWhere.Format("(Sum(TotalCharges - TotalPays) > Convert(money,'%s')) ",_Q(strBalance));

		strWhere += strBalWhere + " AND ";

		//with no insurance applies?
		// (j.jones 2012-01-03 11:11) - PLID 46101 - "with no insurance pays applied" is not available for patient resp. filters
		// (j.gruber 2013-02-21 15:24) - PLID 47780 - changed to user patient resp function
		if(m_checkNoInsPays.GetCheck() && !IsFilteringPatientResp()) {

			// (j.jones 2011-08-17 11:00) - PLID 44888 - ignore "original" and "void" line items
			CString strPays = "NOT EXISTS (SELECT PaymentsT.ID "
				"FROM PaymentsT "
				"INNER JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID "
				"INNER JOIN ChargesT ON AppliesT.DestID = ChargesT.ID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON ChargesT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID " 
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON ChargesT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID " 
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalPaymentsQ ON PaymentsT.ID = LineItemCorrections_OriginalPaymentsQ.OriginalLineItemID " 
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingPaymentsQ ON PaymentsT.ID = LineItemCorrections_VoidingPaymentsQ.VoidingLineItemID " 
				"WHERE PaymentsT.InsuredPartyID = TotalsPerChargeQ.InsuredPartyID "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null " 
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null " 
				"AND LineItemCorrections_OriginalPaymentsQ.OriginalLineItemID Is Null " 
				"AND LineItemCorrections_VoidingPaymentsQ.VoidingLineItemID Is Null " 
				"AND ChargesT.BillID = TotalsPerChargeQ.BillID)";

			strWhere += strPays + " AND ";
		}

		//with no patient applies?
		if(m_checkNoPatientPays.GetCheck()) {

			// (j.jones 2011-08-17 11:00) - PLID 44888 - ignore "original" and "void" line items
			CString strPays = "NOT EXISTS (SELECT PaymentsT.ID "
				"FROM PaymentsT "
				"INNER JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID "
				"INNER JOIN ChargesT ON AppliesT.DestID = ChargesT.ID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON ChargesT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID " 
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON ChargesT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID " 
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalPaymentsQ ON PaymentsT.ID = LineItemCorrections_OriginalPaymentsQ.OriginalLineItemID " 
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingPaymentsQ ON PaymentsT.ID = LineItemCorrections_VoidingPaymentsQ.VoidingLineItemID " 
				"WHERE PaymentsT.InsuredPartyID = -1 "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null " 
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null " 
				"AND LineItemCorrections_OriginalPaymentsQ.OriginalLineItemID Is Null " 
				"AND LineItemCorrections_VoidingPaymentsQ.VoidingLineItemID Is Null " 
				"AND ChargesT.BillID = TotalsPerChargeQ.BillID)";

			strWhere += strPays + " AND ";
		}
	}
#pragma endregion BALANCES FILTER

#pragma region PATIENT FILTER
	//Patient = ZZZZ (5)
	// (a.wilson 2014-07-03 15:32) - PLID 62573 - rewrite for new filter.
		nCurSel = m_PatientList->GetCurSel();
	if (nCurSel != NXDATALISTLib::sriNoRow && VarLong(m_PatientList->GetValue(nCurSel, epfcID), efidAll) > 0)
	{
		long nPatID = VarLong(m_PatientList->GetValue(nCurSel, epfcID));
		CString strPatWhere = FormatString("(TotalsPerChargeQ.PatientID = %li) AND ", nPatID);

		//m_pResultsList->GetColumn(ebfcPatientName)->PutStoredWidth(0);
		//m_pResultsList->GetColumn(ebfcPatientName)->ColumnStyle = csFixedWidth | csVisible;

		strWhere += strPatWhere;
		}
	//else {
	//	m_pResultsList->GetColumn(ebfcPatientName)->PutStoredWidth(80);
	//	m_pResultsList->GetColumn(ebfcPatientName)->ColumnStyle = csWidthData | csVisible;
	//}
#pragma endregion PATIENT FILTER

#pragma region LOCATION FILTER
	// (j.jones 2009-08-11 08:37) - PLID 28131 - added location filter
	//6)  Location (the bill location of the charges)
	// (a.wilson 2014-07-03 16:58) - PLID 62573 - rewrite for new selection.
	nCurSel = m_LocationCombo->GetCurSel();
	if (nCurSel != NXDATALISTLib::sriNoRow && VarLong(m_LocationCombo->GetValue(nCurSel, emfcID), efidAll) > efidAll) {

		//hide the location column, it's not necessary
		// (j.gruber 2013-02-27 15:26) - PLID 54882 - except if we are showing multiple
		//if (m_dwLocations.GetSize() == 1) {
		//	m_pResultsList->GetColumn(ebfcLocationName)->PutStoredWidth(0);
		//	m_pResultsList->GetColumn(ebfcLocationName)->ColumnStyle = csFixedWidth | csVisible;
		//} else {
		//	m_pResultsList->GetColumn(ebfcLocationName)->PutStoredWidth(80);
		//	m_pResultsList->GetColumn(ebfcLocationName)->ColumnStyle = csWidthData | csVisible;
		//}

			CString strLocations = GenerateDelimitedListFromLongArray(m_dwLocations, ",");
		CString strLocWhere = FormatString("(TotalsPerChargeQ.LocationID IN (%s)) AND ", strLocations);

		strWhere += strLocWhere;

	}
	//else {
	//	//when showing all locations, include the location column
	//	m_pResultsList->GetColumn(ebfcLocationName)->PutStoredWidth(80);
	//	m_pResultsList->GetColumn(ebfcLocationName)->ColumnStyle = csWidthData | csVisible;
	//}
#pragma endregion LOCATION FILTER

#pragma region PROVIDER FILTER
	//7) Charge Provider
	// (a.wilson 2014-07-07 09:34) - PLID 62573
	nCurSel = m_pProviderCombo->GetCurSel();
	if (nCurSel != NXDATALISTLib::sriNoRow && VarLong(m_pProviderCombo->GetValue(nCurSel, emfcID), efidAll) != efidAll) {
		long nProviderID = VarLong(m_pProviderCombo->GetValue(nCurSel, emfcID));
		CString strProviderWhere = FormatString("(TotalsPerChargeQ.ProviderID = %li) AND ", nProviderID);

		//m_pResultsList->GetColumn(ebfcProviderName)->PutStoredWidth(0);
		//m_pResultsList->GetColumn(ebfcProviderName)->ColumnStyle = csFixedWidth | csVisible;

		strWhere += strProviderWhere;
		}
	//else {
	//	//when showing all providers, include the provider column
	//	m_pResultsList->GetColumn(ebfcProviderName)->PutStoredWidth(80);
	//	m_pResultsList->GetColumn(ebfcProviderName)->ColumnStyle = csWidthData | csVisible;
	//}
#pragma endregion PROVIDER FILTER

	//clean up any trailing 'AND'
	if (strWhere.Right(5) == " AND ") {
		strWhere = strWhere.Left(strWhere.GetLength() - 5);
	}

	return strWhere;
}

BEGIN_EVENTSINK_MAP(CBillingFollowUpDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CBillingFollowUpDlg)
	ON_EVENT(CBillingFollowUpDlg, IDC_FILTER_LOCATION, 16 /* SelChosen */, OnSelChosenLocation, VTS_I4)
	ON_EVENT(CBillingFollowUpDlg, IDC_FILTER_LOCATION, 18 /* RequeryFinished */, OnRequeryFinishedLocation, VTS_I2)
	ON_EVENT(CBillingFollowUpDlg, IDC_PATIENT_LIST, 18 /* RequeryFinished */, OnRequeryFinishedPatient, VTS_I2)
	ON_EVENT(CBillingFollowUpDlg, IDC_PATIENT_LIST, 1, CBillingFollowUpDlg::SelChangingPatientList, VTS_PI4)
	ON_EVENT(CBillingFollowUpDlg, IDC_FILTER_LOCATION, 1, CBillingFollowUpDlg::SelChangingLocationCombo, VTS_PI4)
	ON_EVENT(CBillingFollowUpDlg, IDC_FILTER_PROVIDER, 18 /* RequeryFinished */, OnRequeryFinishedProvider, VTS_I2)
	ON_EVENT(CBillingFollowUpDlg, IDC_FILTER_PROVIDER, 1, CBillingFollowUpDlg::SelChangingProviderCombo, VTS_PI4)
	//}}AFX_EVENTSINK_MAP
	
	ON_EVENT(CBillingFollowUpDlg, IDC_BILLSTATUS_CONTROL, 16, CBillingFollowUpDlg::SelChosenBillstatusControl, VTS_DISPATCH)
	ON_EVENT(CBillingFollowUpDlg, IDC_FILTER_DATE_LIST, 1, CBillingFollowUpDlg::SelChangingFilterDateList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CBillingFollowUpDlg, IDC_FILTER_RESP, 18, CBillingFollowUpDlg::RequeryFinishedFilterResp, VTS_I2)
	ON_EVENT(CBillingFollowUpDlg, IDC_FILTER_RESP, 1, CBillingFollowUpDlg::SelChangingFilterResp, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CBillingFollowUpDlg, IDC_FILTER_RESP, 2, CBillingFollowUpDlg::SelChangedFilterResp, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CBillingFollowUpDlg, IDC_FILTER_DATE_LIST, 2, CBillingFollowUpDlg::SelChangedFilterDateList, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CBillingFollowUpDlg, IDC_FILTER_INSURANCE_LIST, 18, CBillingFollowUpDlg::RequeryFinishedFilterInsuranceList, VTS_I2)
	ON_EVENT(CBillingFollowUpDlg, IDC_FILTER_HCFA_LIST, 18, CBillingFollowUpDlg::RequeryFinishedFilterHcfaList, VTS_I2)
	ON_EVENT(CBillingFollowUpDlg, IDC_FILTER_INSURANCE_LIST, 1, CBillingFollowUpDlg::SelChangingFilterInsuranceList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CBillingFollowUpDlg, IDC_FILTER_HCFA_LIST, 1, CBillingFollowUpDlg::SelChangingFilterHcfaList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CBillingFollowUpDlg, IDC_FILTER_CLAIM_LIST, 1, CBillingFollowUpDlg::SelChangingFilterClaimList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CBillingFollowUpDlg, IDC_ONHOLD_FILTER, 1, CBillingFollowUpDlg::SelChangingOnholdFilter, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CBillingFollowUpDlg, IDC_BILLSTATUS_CONTROL, 1, CBillingFollowUpDlg::SelChangingBillstatusControl, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CBillingFollowUpDlg, IDC_BILLFU_RESULTS_LIST, 6, CBillingFollowUpDlg::RButtonDownBillfuResultsList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CBillingFollowUpDlg, IDC_BILLFU_RESULTS_LIST, 18, CBillingFollowUpDlg::RequeryFinishedBillfuResultsList, VTS_I2)
	ON_EVENT(CBillingFollowUpDlg, IDC_BILLFU_RESULTS_LIST, 22, CBillingFollowUpDlg::ColumnSizingFinishedBillfuResultsList, VTS_I2 VTS_BOOL VTS_I4 VTS_I4)
	ON_EVENT(CBillingFollowUpDlg, IDC_ONHOLD_FILTER, 16, CBillingFollowUpDlg::SelChosenOnholdFilter, VTS_DISPATCH)
	ON_EVENT(CBillingFollowUpDlg, IDC_BILLFU_RESULTS_LIST, 19, CBillingFollowUpDlg::LeftClickBillfuResultsList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
END_EVENTSINK_MAP()

void CBillingFollowUpDlg::OnSendToPaperBatch() 
{
	//TODO: I think it is dangerous to have them change the batch of potentially large amounts of bills.
	//Still, consider having this function generate the data without being dependent on the datalist.
	//It is, after all, not the same behavior as the "Preview Tracer Forms"

	if(m_pResultsList->GetRowCount() <= 0) {
		AfxMessageBox("As this feature could potentally change the batch of large amounts of bills, \n"
		"please click on 'Display Results' on the left panel to preview the claims first.");
		return;
	}

	if(IDNO==MessageBox("This action will send all claims currently in the results list to the paper HCFA batch.\n"
		"If any of the selected claims are already in another batch, they will be moved to the paper HCFA batch.\n\n"
		"Are you sure you wish to do this?","Practice",MB_ICONQUESTION|MB_YESNO))
		return;

	CWaitCursor pWait;

	// (a.wilson 2014-07-08 13:24) - PLID 62528 - rewrote this for upgraded datalist results.
	for (NXDATALIST2Lib::IRowSettingsPtr pRow = m_pResultsList->GetFirstRow(); pRow; pRow = pRow->GetNextRow())
	{
		// (j.jones 2008-02-11 16:51) - PLID 28847 - added option to disable batching/printing claims with 100% patient resp.
		// (but you can unbatch claims)
		long nBillID = VarLong(pRow->GetValue(ebfcBillID));
		// (j.jones 2010-10-21 14:48) - PLID 41051 - this function can warn why the claim can't be created,
		// but does not do so here, we will handle it ourselves
		if(!CanCreateInsuranceClaim(nBillID, TRUE)) {
			CString str = FormatString("The %s claim for patient %s can not be batched either because no charges on the bill are batched "
				"or because your preference to disallow batching or printing claims with no insurance responsibility has been enabled.\r\n\r\n"
				"Batching will continue for the next claim in the list.",
				FormatDateTimeForInterface(VarDateTime(pRow->GetValue(ebfcBillDate)), NULL, dtoDate), VarString(pRow->GetValue(ebfcPatientName)));

			if (MessageBox(str, "Practice", MB_OKCANCEL | MB_ICONINFORMATION) == IDOK) {
				continue;
			}
			else {
				return;
			}
		}
		//pass TRUE to skip the responsibility check, because we just did it here
		BatchBill(nBillID, 1, TRUE);
	}
}


void CBillingFollowUpDlg::OnSendToEbilling() 
{
	//TODO: I think it is dangerous to have them change the batch of potentially large amounts of bills.
	//Still, consider having this function generate the data without being dependent on the datalist.
	//It is, after all, not the same behavior as the "Preview Tracer Forms"

	if(m_pResultsList->GetRowCount() <= 0) {
		AfxMessageBox("As this feature could potentally change the batch of large amounts of bills, \n"
		"please click on 'Display Results' on the left panel to preview the claims first.");
		return;
	}

	if(IDNO==MessageBox("This action will send all claims currently in the results list to the E-Billing batch.\n"
		"If any of the selected claims are already in another batch, they will be moved to the E-Billing batch.\n\n"
		"Are you sure you wish to do this?","Practice",MB_ICONQUESTION|MB_YESNO))
		return;

	CWaitCursor pWait;

	// (a.wilson 2014-07-08 13:24) - PLID 62528 - rewrote this for upgraded datalist results.
	for (NXDATALIST2Lib::IRowSettingsPtr pRow = m_pResultsList->GetFirstRow(); pRow; pRow = pRow->GetNextRow())
	{
		// (j.jones 2008-02-11 16:51) - PLID 28847 - added option to disable batching/printing claims with 100% patient resp.
		// (but you can unbatch claims)
		long nBillID = VarLong(pRow->GetValue(ebfcBillID));
		// (j.jones 2010-10-21 14:48) - PLID 41051 - this function can warn why the claim can't be created,
		// but does not do so here, we will handle it ourselves
		if(!CanCreateInsuranceClaim(nBillID, TRUE)) {
			CString str = FormatString("The %s claim for patient %s can not be batched either because no charges on the bill are batched "
				"or because your preference to disallow batching or printing claims with no insurance responsibility has been enabled.\r\n\r\n"
				"Batching will continue for the next claim in the list.",
				FormatDateTimeForInterface(VarDateTime(pRow->GetValue(ebfcBillDate)), NULL, dtoDate), VarString(pRow->GetValue(ebfcPatientName)));

			if (MessageBox(str, "Practice", MB_OKCANCEL | MB_ICONINFORMATION) == IDOK) {
				continue;
			}
			else {
				return;
			}
		}
		//pass TRUE to skip the responsibility check, because we just did it here
		BatchBill(nBillID, 2, TRUE);
	}
}

void CBillingFollowUpDlg::OnMergeDocument() 
{
	try {

		//TODO: That's it, I'm going to have to make these buttons all disabled if there are no claims in the list
		//OR force it to run the query itself

		if(m_pResultsList->GetRowCount() <= 0) {
			AfxMessageBox("Please click on 'Display Results' on the left panel to preview the claims before merging.");
			return;
		}

		//make sure word exists
		if (!GetWPManager()->CheckWordProcessorInstalled()) {
			return;
		}

		//JMJ - 5/6/2003 - if we were merging just bill information, then we could only invoke
		//one merge engine and add all the IDs to this array, but because we have multiple charge records,
		//we have to only add one Bill ID and invoke the merge engine once per bill.
		//So loop per bill and merge a new document per bill

		//since we're generating a different word document per bill, warn them first
		if(IDNO == MessageBox("Merging charge information will merge multiple records per bill, and therefore cannot go to a new page for each bill.\n"
							"As a result, a new Word document will be generated for each bill selected, which will cause the merge to take a long time\n"
							"if you are merging a lot of bills.\n\n"
							"Are you sure you wish to merge these bills?","Practice",MB_ICONEXCLAMATION|MB_YESNO))
			return;

		BOOL bPauseEvery10 = TRUE;

		if(m_pResultsList->GetRowCount() > 10) {
			if(IDNO == MessageBox("You are merging a large number of Word documents - it is recommended that the merge pauses every 10 records\n"
								  "so you will have the option to print and close the documents before continuing.\n\n"
								  "Would you like to pause the merge every 10 records? (Clicking 'No' will merge all the records without stopping.)",
								  "Practice",MB_ICONQUESTION|MB_YESNO)) {
				bPauseEvery10 = FALSE;
			}
		}

		// Get template to merge to
		// (a.walling 2007-06-14 16:02) - PLID 26342 - Support Word 2007 templates
		// (a.walling 2007-10-12 14:04) - PLID 26342 - Also support macro-enabled 2007 documents
		CString strFilter;
		// Always support Word 2007 templates
		strFilter = "Microsoft Word Templates|*.dot;*.dotx;*.dotm||";

		CFileDialog dlg(TRUE, "dot", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, strFilter);
		CString initialDir = GetTemplatePath();
		dlg.m_ofn.lpstrInitialDir = initialDir;
		if (dlg.DoModal() == IDOK) {

			// If the user clicked OK do the merge
			CWaitCursor wc;
			CString strTemplateName = dlg.GetPathName();

			// (a.wilson 2014-07-08 13:24) - PLID 62528 - rewrote this for upgraded datalist results.
			long nCount = 0, nTotalCount = m_pResultsList->GetRowCount();
			for (NXDATALIST2Lib::IRowSettingsPtr pRow = m_pResultsList->GetFirstRow(); pRow; pRow = pRow->GetNextRow(), nCount++)
			{
				if (bPauseEvery10 && (nCount % 10) == 0 && nCount != 0) {
					//stop every 10 records
					CString str;
					long nNumLeft = 10;
					if (nTotalCount - nCount <= 10) {
						nNumLeft = nTotalCount - nCount;
						if(nNumLeft == 1) {
							str.Format("Press OK to merge the last record.");
						}
						else {
							str.Format("Press OK to merge the last %li records.",nNumLeft);
						}
					}
					else {
						str.Format("Press OK to merge the next %li records.",nNumLeft);
					}
					
					AfxMessageBox(str);
				}
				
				/// Generate the temp table
				CString strSql;
				//if we were merging all the bills to one merge engine, we would use this line
				//strSql.Format("SELECT PatientID AS ID FROM %s WHERE %s ", CString((LPCTSTR)(m_ResultsList->GetFromClause())),GetWhereClause());
				strSql.Format("SELECT ID FROM PersonT WHERE ID = %li", VarLong(pRow->GetValue(ebfcPatientID)));
				CString strMergeT = CreateTempIDTable(strSql, "ID");
				
				// Merge
				CMergeEngine mi;
				
				//if we were merging all the bills to one merge engine, we would use this loop and remove the outer loop
				//for(int i=0;i<m_ResultsList->GetRowCount();i++) {
				mi.m_arydwBillIDs.Add(VarLong(pRow->GetValue(ebfcBillID)));
				//}

				if (g_bMergeAllFields)
					mi.m_nFlags |= BMS_IGNORE_TEMPLATE_FIELD_LIST;
				
				//yes, save in history
				mi.m_nFlags |= BMS_SAVE_FILE_AND_HISTORY;

				// (z.manning, 03/05/2008) - PLID 29131 - Now have a function to load sender information.
				mi.LoadSenderInfo(FALSE);

				if (IsDlgButtonChecked(IDC_MERGE_TO_PRINTER)) {
					mi.m_nFlags = (mi.m_nFlags | BMS_MERGETO_PRINTER) & ~BMS_MERGETO_SCREEN;
				}

				// Do the merge
				// (z.manning 2016-06-03 8:41) - NX-100806 - Check if the merge was successful
				if (mi.MergeToWord(strTemplateName, std::vector<CString>(), strMergeT))
				{
					if (m_checkSaveAsTracer.GetCheck()) {
						//Send Type -1 - Tracer Letter

						// (j.jones 2013-01-23 09:19) - PLID 54734 - Moved the claim history addition to its own function.
						// (j.jones 2005-04-06 13:35) - for tracer letters, show all charges, not just batched ones
						AddToClaimHistory(VarLong(pRow->GetValue(ebfcBillID)), VarLong(pRow->GetValue(ebfcInsuredPartyID)), ClaimSendType::TracerLetter, "", FALSE, FALSE);
					}
				}
			}
		//end our loop of merging
		}
	} NxCatchAll("CBillingFollowupDlg::OnMergeDocument");
}

void CBillingFollowUpDlg::OnGoToPatient() {

	if(!m_pResultsList->GetCurSel())
		return;

	try {
		long nPatientID = VarLong(m_pResultsList->GetCurSel()->GetValue(ebfcPatientID));

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
				MsgBox(MB_ICONSTOP|MB_OK, "ERROR 1236 - BillingFollowUpDlg.cpp: Cannot Open Mainframe");
			}//end else pMainFrame
		}//end if nPatientID
	}NxCatchAll("Error in OnGoToPatient");
}

void CBillingFollowUpDlg::OnCheckTracerFormSent() 
{
	//enable/disable the radio buttons
	BOOL bEnabled = m_checkTracerFormSent.GetCheck();
	m_radioTFSentYes.EnableWindow(bEnabled);
	m_radioTFSentNo.EnableWindow(bEnabled);
	m_radioTFSentDate.EnableWindow(bEnabled);

	if(bEnabled)
		ClickRadioTFInfo();
	else
		GetDlgItem(IDC_EDIT_TF_DAYS)->EnableWindow(FALSE);
}

void CBillingFollowUpDlg::OnRadioTfSentYes() 
{
	ClickRadioTFInfo();
}

void CBillingFollowUpDlg::OnRadioTfSentNo() 
{
	ClickRadioTFInfo();
}

void CBillingFollowUpDlg::OnRadioTfSentDate() 
{
	ClickRadioTFInfo();
}

void CBillingFollowUpDlg::ClickRadioTFInfo() {

	//enable/disable the 'days' edit box

	GetDlgItem(IDC_EDIT_TF_DAYS)->EnableWindow(m_radioTFSentDate.GetCheck());
}

void CBillingFollowUpDlg::Refresh() {

	try {
		//PATIENT
		// (a.wilson 2014-07-03 16:26) - PLID 62809
		if (m_patientChecker.Changed()) {
			if (m_PatientList->GetCurSel() != -1) {
				m_nCurrentPatientID = VarLong(m_PatientList->GetValue(m_PatientList->GetCurSel(), epfcID), efidAll);
			}
			m_PatientList->Requery();
		}

		//INSURANCE
		// (a.wilson 2014-07-08 10:51) - PLID 62809 - rewritten for insurance change.
		if (m_companyChecker.Changed()) {
			if (m_pInsuranceFilter->GetCurSel() != NULL) {
				m_nCurrentInsuranceID = VarLong(m_pInsuranceFilter->GetCurSel()->GetValue(emfcID), (long)efidAll);
		}
			m_pInsuranceFilter->Requery();
		}

		//LOCATION
		// (j.jones 2009-08-11 09:06) - PLID 28131 - added location filter
		// (a.wilson 2014-07-03 17:08) - PLID 62809
		if(m_locationChecker.Changed()) {

			if(m_LocationCombo->GetCurSel() != -1) {
				m_nLocationID = VarLong(m_LocationCombo->GetValue(m_LocationCombo->GetCurSel(), emfcID), efidAll);
			}
			m_LocationCombo->Requery();
			}

		// (s.tullis 2014-08-14 11:54) - PLID 62577 - check if its changed
		if (m_BillStatusChecker.Changed()){

			if (m_pBillStatusFilter->GetCurSel() != NULL){
				n_CurrentBillStatusID = VarLong(m_pBillStatusFilter->GetCurSel()->GetValue(bslNoteID), (long)All);
			}
			
			RequeryBillStatus(n_CurrentBillStatusID, true);

		}
		//PROVIDER
		//(e.lally 2009-08-14) PLID 30119
		// (a.wilson 2014-07-07 09:32) - PLID 62809
		if(m_providerChecker.Changed()) {

			if(m_pProviderCombo->GetCurSel() != sriNoRow) {
				m_nCurrentProviderID = VarLong(m_pProviderCombo->GetValue(m_pProviderCombo->GetCurSel(), emfcID), efidAll);
			}
			m_pProviderCombo->Requery();
			}

	}NxCatchAll("Error refreshing data.");
}

// (a.wilson 2014-07-08 13:53) - PLID 62528 - rewritten to handle datalist upgrade.
void CBillingFollowUpDlg::OnRemoveSelectedBills() {

	try {
		NXDATALIST2Lib::IRowSettingsPtr pCurrentRow = m_pResultsList->GetCurSel();

		if (!pCurrentRow) {
			ASSERT(FALSE);
			return;
		}

		long nSelectedRows = 0;
		for (NXDATALIST2Lib::IRowSettingsPtr pSelectedRow = m_pResultsList->GetFirstSelRow(); pSelectedRow; ) {
			m_aryRemovedBills.Add(VarLong(pSelectedRow->GetValue(ebfcBillID)));
			NXDATALIST2Lib::IRowSettingsPtr pRemoveRow = pSelectedRow;
			pSelectedRow = pSelectedRow->GetNextSelRow();
			m_pResultsList->RemoveRow(pRemoveRow);
			}
		m_pResultsList->PutCurSel(NULL);

		SetDlgItemInt(IDC_RESULTS_COUNT, m_pResultsList->GetRowCount());

		CRect rc;
		GetDlgItem(IDC_RESULTS_COUNT)->GetWindowRect(rc);
		ScreenToClient(rc);
		InvalidateRect(rc);

	}NxCatchAll("Error removing bill from list.");
}

// (r.gonet 07/25/2014) - PLID 62920 - Handles when the user clicks the Mark Bill On Hold menu option
// from the context menu of the bill results list.
void CBillingFollowUpDlg::OnMarkBillOnHold()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pCurrentRow = m_pResultsList->GetCurSel();
		if (!pCurrentRow) {
			ASSERT(FALSE);
			return;
		}

		long nBillID = VarLong(pCurrentRow->GetValue(ebfcBillID), -1);
		if (!SetBillOnHold(nBillID, TRUE)) {
			return;
		}

		// (r.gonet 08/22/2014) - PLID 63078 - Get the on hold column, which is dynamically positioned.
		BillingColumn bcOnHoldIconColumn;
		if (m_mapResultColumns.Lookup((short)ebfcBillOnHoldIcon, bcOnHoldIconColumn)) {
			// (r.gonet 07/28/2014) - PLID 63078 - Mark all the charges in the list that belong to this bill on hold.
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pResultsList->GetFirstRow();
			while (pRow) {
				short nColumnCount = m_pResultsList->GetColumnCount();
				// The billID column is a static column
				if (VarLong(pRow->GetValue(ebfcBillID), -1) == nBillID) {
					// Found the dynamic position of the on hold icon
					pRow->PutValue((short)bcOnHoldIconColumn.nOrderIndex, (long)m_hIconBillOnHold);
				} else {
					// Not a charge associated with the same bill
				}
				pRow = pRow->GetNextRow();
			}
		} else {
			// The on hold column is not present in the list.
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 07/25/2014) - PLID 62920 - Handles when the user clicks the Remove Bill Hold menu option
// from the context menu of the bill results list.
void CBillingFollowUpDlg::OnRemoveBillHold()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pCurrentRow = m_pResultsList->GetCurSel();
		if (!pCurrentRow) {
			ASSERT(FALSE);
			return;
		}

		long nBillID = VarLong(pCurrentRow->GetValue(ebfcBillID), -1);
		if (!SetBillOnHold(nBillID, FALSE)) {
			return;
		}

		// (r.gonet 08/22/2014) - PLID 63078 - Get the on hold column, which is dynamically positioned.
		BillingColumn bcOnHoldIconColumn;
		if (m_mapResultColumns.Lookup((short)ebfcBillOnHoldIcon, bcOnHoldIconColumn)) {
			// (r.gonet 07/28/2014) - PLID 63078 - Mark all the charges in the list that belong to this bill on hold.
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pResultsList->GetFirstRow();
			while (pRow) {
				short nColumnCount = m_pResultsList->GetColumnCount();
				// The billID column is a static column
				if (VarLong(pRow->GetValue(ebfcBillID), -1) == nBillID) {
					// Found the dynamic position of the on hold icon
					pRow->PutValue((short)bcOnHoldIconColumn.nOrderIndex, g_cvarNull);
				} else {
					// Not a charge associated with the same bill
				}
				pRow = pRow->GetNextRow();
			}
		} else {
			// The on hold column is not present in the list.
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 07/25/2014) - PLID 62920 - Changes the bill's on hold state.
bool CBillingFollowUpDlg::SetBillOnHold(long nBillID, BOOL bOnHold)
{
	// (r.gonet 07/07/2014) - PLID 62571 - Should be allowed even under financial closes.
	if (!CanChangeHistoricFinancial("Bill", nBillID, bioBill, sptWrite, FALSE, NULL, TRUE)) {
		return false;
	}

	if (IsVoidedBill(nBillID)) {
		AfxMessageBox("This bill has been corrected, and can no longer be modified.");
		return false;
	}

	CWaitCursor waitCursor;
	return ::SetBillOnHold(nBillID, bOnHold, FALSE);
}

CString CBillingFollowUpDlg::GetIgnoredBillList() {

	try {
		
		if(m_aryRemovedBills.GetSize() == 0)
			return "";

		CString strRet = "(";

		for(int i=0;i<m_aryRemovedBills.GetSize();i++) {
			CString str;
			str.Format(" SubQ.BillID != %li AND",(long)m_aryRemovedBills.GetAt(i));
			strRet += str;
		}
		strRet.TrimRight("AND");
		strRet += ") ";

		return strRet;

	}NxCatchAll("Error generating bill ignore list.");

	return "";
}

void CBillingFollowUpDlg::OnCreateLwGroup() 
{
	try {

		if(m_pResultsList->GetRowCount() <= 0) {
			AfxMessageBox("Please click on 'Display Results' on the left panel to preview the claims before merging.");
			return;
		}

		if (!UserPermission(EditGroup))
			return;

		CString strNewGroupName;
		CGetNewIDName dlgGetNew(this);
		dlgGetNew.m_pNewName = &strNewGroupName;
		dlgGetNew.m_strCaption = "Enter a Name for the New Group";
		dlgGetNew.m_nMaxLength = 50;

		//Change the timeout so we don't get an error.
		m_pIncreaseCommandTimeout.reset(new CIncreaseCommandTimeout(600));

		if (dlgGetNew.DoModal() == IDOK) {
			//does this group already exist?
			_RecordsetPtr rs;
			rs = CreateRecordset("SELECT Name FROM GroupsT WHERE Name = '%s'", _Q(strNewGroupName));
			if(!rs->eof)
			{
				rs->Close();
				AfxMessageBox("Group name already exists.  Please try again with a different name.");
				return;
			}
			rs->Close();

			CWaitCursor pWait;

			CString strSql, strFrom, strWhere;

			strWhere = GetIgnoredBillList();
			strWhere.Replace("SubQ.","");
			if(!strWhere.IsEmpty())
				strWhere = " WHERE " + strWhere;

			_variant_t var;
			var = _variant_t(m_pResultsList->FromClause);
			strFrom = CString(var.bstrVal);

			
			strSql.Format("SELECT PatientID FROM %s %s",strFrom,strWhere);

			// Figure out what the new group ID will be
			int nGroupId = NewNumber("GroupsT", "ID");

			// Create a new group with that ID
			ExecuteSql("INSERT INTO GroupsT(ID, Name) VALUES (%li, '%s')", nGroupId, _Q(strNewGroupName));

			ExecuteSql("INSERT INTO GroupDetailsT(GroupID, PersonID) "
				"SELECT %li AS GroupID, PatientID AS PersonID "
				"FROM (%s) RunTimeReportQ GROUP BY PatientID", nGroupId, strSql);

			//update the group checker
			CClient::RefreshTable(NetUtils::Groups);

			// Tell the user that the group has been successfully created
			MsgBox("Group has been created.");
		}

		//Reset the timeout
		m_pIncreaseCommandTimeout.reset();

	}NxCatchAll("Error creating merge group.");	
}

void CBillingFollowUpDlg::OnRadioGreaterThanDays() 
{
	if(m_radioGreaterThanDays.GetCheck()) {
		GetDlgItem(IDC_EDIT_DAYS_OLD)->EnableWindow(TRUE);
		m_dtFrom.EnableWindow(FALSE);
		m_dtTo.EnableWindow(FALSE);
	}
	else if(m_radioBetweenDates.GetCheck()) {
		GetDlgItem(IDC_EDIT_DAYS_OLD)->EnableWindow(FALSE);
		m_dtFrom.EnableWindow(TRUE);
		m_dtTo.EnableWindow(TRUE);
	}
	else {
		//neither were checked! Check one, and call this function again
		m_radioGreaterThanDays.SetCheck(TRUE);
		OnRadioGreaterThanDays();
	}
}

void CBillingFollowUpDlg::OnRadioBetweenDateRange() 
{
	OnRadioGreaterThanDays();
}

void CBillingFollowUpDlg::OnPreviewList() 
{
	try {

		CWaitCursor pWait;
		CString filter, param;

		if(m_strCurrentFrom == "") {
			MsgBox("There are no results to display!");
			return;
		}

		CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(443)]);
		CString str;
		str.Format("(SELECT * FROM %s) SubQ", m_strCurrentFrom);
		infReport.SetExtraValue(GetIgnoredBillList());	//filter on bills
		infReport.strExtendedSql = str;	//pass in the subquery through the extended filter

		//DRT 6/14/2007 - PLID 21395 - If date range is selected, tell the user what range they're using on the report.  We read
		//	this from the member variable that is updated in OnBtnLoad(), when the user actually generates the list.
		CPtrArray paramList;
		CRParameterInfo* pParam;
		pParam = new CRParameterInfo;
		pParam->m_Data = m_strCurrentDateRangeForReport;
		pParam->m_Name = "DateRange";
		paramList.Add( (void*) pParam);

		//Made new function for running reports - JMM 5-28-04
		RunReport(&infReport, &paramList, true, this, "Billing Follow-Up List");
		ClearRPIParameterList(&paramList);
		
	}NxCatchAll("Error previewing list.");
}

void CBillingFollowUpDlg::OnCheckSaveMergeAsTracer() 
{
	SetRemotePropertyInt("SaveBillingFUMergeAsTracer",m_checkSaveAsTracer.GetCheck() ? 1 : 0, 0, GetCurrentUserName());
}

void CBillingFollowUpDlg::EnableListControls(BOOL bEnabled)
{
	GetDlgItem(IDC_BTN_LOAD)->EnableWindow(bEnabled);
	GetDlgItem(IDC_SEND_TO_PAPER_BATCH)->EnableWindow(bEnabled);
	GetDlgItem(IDC_SEND_TO_EBILLING)->EnableWindow(bEnabled);
	GetDlgItem(IDC_PREVIEW_FORMS)->EnableWindow(bEnabled);
	GetDlgItem(IDC_PREVIEW_LIST)->EnableWindow(bEnabled);
	GetDlgItem(IDC_MERGE_DOCUMENT)->EnableWindow(bEnabled);
	GetDlgItem(IDC_CREATE_LW_GROUP)->EnableWindow(bEnabled);
}

void CBillingFollowUpDlg::OnBtnSearchAllowables() 
{
	// (j.jones 2006-11-29 16:02) - PLID 22293 - added the allowed amount search
	CAllowedAmountSearchDlg dlg(this);
	dlg.DoModal();
}

// (j.jones 2008-06-27 13:45) - PLID 27647 - added account write-off ability
void CBillingFollowUpDlg::OnBtnWriteOffAccts()
{
	try {

		//check the WriteOffAccts permission to see if they can use this feature
		if(!CheckCurrentUserPermissions(bioWriteOffAccts, sptCreate)) {
			return;
		}

		CAccountWriteOffDlg dlg(this);

		//this dialog will return IDOK if any adjustments were made, IDCANCEL otherwise
		if(dlg.DoModal() == IDOK) {

			//prompt to reload the screen, if we have any results
			if(m_pResultsList->GetRowCount() > 0) {

				if(IDYES == MessageBox("Because some accounts were adjusted, your currently displayed results may no longer be accurate.\n\n"
					"Would you like to reload the current list?", "Practice", MB_ICONQUESTION|MB_YESNO)) {

					//reload
					OnBtnLoad();
				}
			}
		}

	}NxCatchAll("Error in CBillingFollowUpDlg::OnBtnWriteOffAccts");
}

// (j.gruber 2013-02-26 14:54) - PLID 47780 - added 
// (a.wilson 2014-07-07 16:20) - PLID 62573 - rewritten for new functionality
BOOL CBillingFollowUpDlg::IsFilteringPatientResp()
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pRespFilter->GetCurSel();
		if (pRow) {
		long nRespID = VarLong(pRow->GetValue(emfcID), (long)erfoPatient);
		if (nRespID == (long)erfoPatient) {
				return TRUE;
			}
			}
		return FALSE;
	}

// (j.gruber 2013-02-27 14:42) - PLID 54882 
void CBillingFollowUpDlg::OnSelChosenLocation(long nRow) 
{
	try {
		// We must have a current report or else what are we doing?		
		if (nRow >= 0) {
			if(VarLong(m_LocationCombo->GetValue(nRow, emfcID), efidAll) == efidMultiple) {
				//They selected "<Multiple Locations>"
				OnLocationList();
			}			
			else {
				//They selected a location or all locations.
				m_dwLocations.RemoveAll();
				m_nLocationID = VarLong(m_LocationCombo->GetValue(nRow, emfcID), efidAll);
				m_dwLocations.Add((DWORD)m_nLocationID);
			}
		}
	} NxCatchAll(__FUNCTION__);	
}

// (j.gruber 2013-02-27 14:42) - PLID 54882
void CBillingFollowUpDlg::OnLocationList() 
{
	try {		
		CMultiSelectDlg dlg(this, "LocationsT");
		dlg.PreSelect(m_dwLocations);
		//Rather than hard-code this, let's grab the filter criteria from the regular dropdown.
		CString strLocationFromClause = VarString(_variant_t(m_LocationCombo->GetFromClause()));
		CString strLocationWhereClause = VarString(_variant_t(m_LocationCombo->GetWhereClause()));
		if(IDOK == dlg.Open(strLocationFromClause, strLocationWhereClause, "LocationsT.ID", "Name", "Select one or more locations to filter on:", 1)) {
			dlg.FillArrayWithIDs(m_dwLocations);
		
			if(m_dwLocations.GetSize() > 1) {
				ShowDlgItem(IDC_FILTER_LOCATION, SW_HIDE);
				m_nxlLocationLabel.SetText(dlg.GetMultiSelectString());
				m_nxlLocationLabel.SetType(dtsHyperlink);
				ShowDlgItem(IDC_MULTI_LOC_LABEL, SW_SHOW);
				InvalidateDlgItem(IDC_MULTI_LOC_LABEL);
				m_nLocationID = efidMultiple;
			}
			else if(m_dwLocations.GetSize() == 1) {
				//They selected exactly one.
				ShowDlgItem(IDC_MULTI_LOC_LABEL, SW_HIDE);
				ShowDlgItem(IDC_FILTER_LOCATION, SW_SHOW);
				m_LocationCombo->SetSelByColumn(emfcID, (long)m_dwLocations.GetAt(0));
				m_nLocationID = (long)m_dwLocations.GetAt(0);
			}
			else {
				//They didn't select any.  But we told multiselect dlg they had to pick at least one!
				ASSERT(FALSE);
			}
		}
		else {
			//Check if they have "multiple" selected
			if(m_dwLocations.GetSize() > 1) {
				ShowDlgItem(IDC_FILTER_LOCATION, SW_HIDE);
				CString strIDList, strID, strLocationList;
				for (int i = 0; i < m_dwLocations.GetSize(); i++) {
					long nFindValue = m_dwLocations.GetAt(i);
					long nFindRow = m_LocationCombo->FindByColumn(emfcID, nFindValue, 0, FALSE);
					if (nFindRow != -1) {
						NXDATALISTLib::IRowSettingsPtr pFindRow = m_LocationCombo->GetRow(nFindRow);
						if (pFindRow) {
							strLocationList += VarString(pFindRow->GetValue(emfcName)) + ", ";
						}
					}
				}
				strLocationList = strLocationList.Left(strLocationList.GetLength()-2);
				m_nxlLocationLabel.SetText(strLocationList);
				m_nxlLocationLabel.SetType(dtsHyperlink);
				ShowDlgItem(IDC_MULTI_LOC_LABEL, SW_SHOW);
				InvalidateDlgItem(IDC_MULTI_LOC_LABEL);
			}
			else {
				//They selected exactly one. (even if that one was "<No Location>"
				ShowDlgItem(IDC_MULTI_LOC_LABEL, SW_HIDE);
				ShowDlgItem(IDC_FILTER_LOCATION, SW_SHOW);
				m_LocationCombo->SetSelByColumn(emfcID, m_nLocationID);
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2013-02-27 14:42) - PLID 54882
BOOL CBillingFollowUpDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{	
	CPoint pt;
	CRect rc;
	GetCursorPos(&pt);
	ScreenToClient(&pt);	
	
	//LOCATION
	if (m_nLocationID == efidMultiple){
		GetDlgItem(IDC_MULTI_LOC_LABEL)->GetWindowRect(rc);
		ScreenToClient(&rc);

		if (rc.PtInRect(pt)) {
			SetCursor(GetLinkCursor());
			return TRUE;
		}
	}		
	
	//BILL STATUS
	// (s.tullis 2014-07-01 12:08) - PLID 62577 - When hovering over multiselect hyperlink set the curser to a hand
	if (m_nxlMultipleBillStatus.IsWindowVisible() && m_nxlMultipleBillStatus.IsWindowEnabled()){
		GetDlgItem(IDC_MULTI_BILL_STATUS)->GetWindowRect(rc);
		ScreenToClient(&rc);

		if (rc.PtInRect(pt)) {
			SetCursor(GetLinkCursor());
			return TRUE;
		}
	}

	//RESET
	// (a.wilson 2014-07-16 08:22) - PLID 62905 - show hand if they are over reset icon.
	m_icoReset.GetWindowRect(rc);
	ScreenToClient(&rc);
	if (rc.PtInRect(pt)) {
		SetCursor(GetLinkCursor());
		return TRUE;
	}

	return CNxDialog::OnSetCursor(pWnd, nHitTest, message);
}

// (a.wilson 2014-07-03 16:53) - PLID 62809 - add the all row.
void CBillingFollowUpDlg::OnRequeryFinishedLocation(short nFlags) 
{
	try {
		// (j.gruber 2013-02-27 14:42) - PLID 54882- add the multi row
		NXDATALISTLib::IRowSettingsPtr pLocRow = m_LocationCombo->GetRow(sriGetNewRow);
		if (pLocRow) {
			pLocRow->PutValue(emfcID, (long)efidMultiple);
			pLocRow->PutValue(emfcName, _variant_t("< Multiple Locations >"));
			m_LocationCombo->AddRow(pLocRow);
		}
		pLocRow = m_LocationCombo->GetRow(sriGetNewRow);
		if (pLocRow) {
			pLocRow->PutValue(emfcID, (long)efidAll);
			pLocRow->PutValue(emfcName, _variant_t("< All Locations >"));
			m_LocationCombo->AddRow(pLocRow);
		}
		m_LocationCombo->TrySetSelByColumn(emfcID, m_nLocationID);
	}NxCatchAll(__FUNCTION__);
}

// (a.wilson 2014-07-03 15:52) - PLID 62809 - include all selection.
void CBillingFollowUpDlg::OnRequeryFinishedPatient(short nFlags)
{
	try {
		NXDATALISTLib::IRowSettingsPtr pPatRow = m_PatientList->GetRow(sriGetNewRow);
		if (pPatRow) {
			pPatRow->PutValue(epfcID, (long)efidAll);
			pPatRow->PutValue(epfcUID, g_cvarNull);
			pPatRow->PutValue(epfcName, _bstr_t("< All Patients >"));
			m_PatientList->AddRow(pPatRow);
			m_PatientList->TrySetSelByColumn(epfcID, m_nCurrentPatientID);
}
	} NxCatchAll(__FUNCTION__);
}

// (a.wilson 2014-07-07 09:29) - PLID 62809 - include all selection.
void CBillingFollowUpDlg::OnRequeryFinishedProvider(short nFlags)
{
	try {
		NXDATALISTLib::IRowSettingsPtr pProRow = m_pProviderCombo->GetRow(sriGetNewRow);
		if (pProRow) {
			pProRow->PutValue(emfcID, (long)efidAll);
			pProRow->PutValue(emfcName, _bstr_t("< All Providers >"));
			m_pProviderCombo->AddRow(pProRow);
			m_pProviderCombo->TrySetSelByColumn(emfcID, m_nCurrentProviderID);
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.wilson 2014-07-07 13:07) - PLID  62809
void CBillingFollowUpDlg::RequeryFinishedFilterResp(short nFlags)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pRespFilter->GetNewRow();
		if (pRow) {
			pRow->PutValue(emfcID, (long)erfoPatient);
			pRow->PutValue(emfcName, _bstr_t("< Patient >"));
			m_pRespFilter->AddRowSorted(pRow, NULL);
		}
		pRow = m_pRespFilter->GetNewRow();
		if (pRow) {
			pRow->PutValue(emfcID, (long)erfoInsurance);
			pRow->PutValue(emfcName, _bstr_t("< All Insurance >"));
			m_pRespFilter->AddRowSorted(pRow, NULL);
		}
		m_pRespFilter->FindByColumn(emfcID, (long)erfoPatient, NULL, VARIANT_TRUE);
		UpdateResponsibilityFilterState();
	} NxCatchAll(__FUNCTION__);
}

// (a.wilson 2014-07-07 14:35) - PLID 62809 
void CBillingFollowUpDlg::RequeryFinishedFilterInsuranceList(short nFlags)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pInsuranceFilter->GetNewRow();
		if (pRow) {
			pRow->PutValue(emfcID, (long)efidAll);
			pRow->PutValue(emfcName, _bstr_t("< All Insurance Companies >"));
			m_pInsuranceFilter->AddRowBefore(pRow, m_pInsuranceFilter->GetFirstRow());
		}
		m_pInsuranceFilter->FindByColumn(emfcID, m_nCurrentInsuranceID, NULL, VARIANT_TRUE);
	} NxCatchAll(__FUNCTION__);
}

// (a.wilson 2014-07-07 14:35) - PLID 62809 
void CBillingFollowUpDlg::RequeryFinishedFilterHcfaList(short nFlags)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pHCFAFilter->GetNewRow();
		if (pRow) {
			pRow->PutValue(emfcID, (long)efidAll);
			pRow->PutValue(emfcName, _bstr_t("< All HCFA Groups >"));
			m_pHCFAFilter->AddRowBefore(pRow, m_pHCFAFilter->GetFirstRow());
		}
		m_pHCFAFilter->FindByColumn(emfcID, (long)efidAll, NULL, VARIANT_TRUE);
	} NxCatchAll(__FUNCTION__);
}

// (j.gruber 2013-02-27 14:42) - PLID 54882- add the multi locations
LRESULT CBillingFollowUpDlg::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	try {
		UINT nIdc = (UINT)wParam;
		//(e.lally 2008-09-05) PLID 6780 - Add option for locations
		switch(nIdc) {
		case IDC_MULTI_LOC_LABEL:
			OnLocationList();
		break;	// (s.tullis 2014-07-01 12:08) - PLID 62577 - when the Hyperlinked is clicked Launch Multiple Bill status dialog
		case IDC_MULTI_BILL_STATUS:
			HandleMultiStatusSelection();
		break;	
		default:
			//What?  Some strange NxLabel is posting messages to us?
			ASSERT(FALSE);
			break;
		}
	}NxCatchAll(__FUNCTION__);
	return 0;
}

// (a.wilson 2014-07-03 16:31) - PLID 62809 - ensure valid rows.
void CBillingFollowUpDlg::SelChangingPatientList(long FAR* nNewSel)
{
	try {
		if (*nNewSel == NXDATALISTLib::sriNoRow) {
			*nNewSel = m_PatientList->CurSel;
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.wilson 2014-07-03 16:31) - PLID 62809 - ensure valid rows.
void CBillingFollowUpDlg::SelChangingLocationCombo(long FAR* nNewSel)
{
	try {
		if (*nNewSel == NXDATALISTLib::sriNoRow) {
			*nNewSel = m_LocationCombo->CurSel;
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.wilson 2014-07-03 16:31) - PLID 62809 - ensure valid rows.
void CBillingFollowUpDlg::SelChangingProviderCombo(long FAR* nNewSel)
{
	try {
		if (*nNewSel == NXDATALISTLib::sriNoRow) {
			*nNewSel = m_pProviderCombo->CurSel;
		}
	} NxCatchAll(__FUNCTION__);
}




// (s.tullis 2014-07-01 09:53) - PLID 62577 - Build the Bill Status Filter Datalist

void CBillingFollowUpDlg::RequeryBillStatus(long nCurSelect, BOOL bTableCheckerRefresh){


	try {
		
		m_pBillStatusFilter->Requery();
	
		NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_pBillStatusFilter->GetNewRow();
		if (pNewRow){
			pNewRow->PutValue((short)bslNoteID, _variant_t((long)ECriteriaSpecialIds::None, VT_I4));
			pNewRow->PutValue((short)bslNote, _bstr_t(" < None >"));
			m_pBillStatusFilter->AddRowBefore(pNewRow, m_pBillStatusFilter->GetFirstRow());
		}
		
		pNewRow = m_pBillStatusFilter->GetNewRow();
		if (pNewRow){
			pNewRow->PutValue((short)bslNoteID, _variant_t((long)ECriteriaSpecialIds::Multiple, VT_I4));
			pNewRow->PutValue((short)bslNote, _bstr_t(" < Multiple >"));
			m_pBillStatusFilter->AddRowBefore(pNewRow, m_pBillStatusFilter->GetFirstRow());
		}
		
		pNewRow = m_pBillStatusFilter->GetNewRow();
		if (pNewRow){
			pNewRow->PutValue((short)bslNoteID, _variant_t((long)ECriteriaSpecialIds::All, VT_I4));
			pNewRow->PutValue((short)bslNote, _bstr_t(" < All >"));
			m_pBillStatusFilter->AddRowBefore(pNewRow, m_pBillStatusFilter->GetFirstRow());
		}

		if (bTableCheckerRefresh){
			HandleBillStatusTableCheckerRefresh(nCurSelect);
		}
		else{
			m_pBillStatusFilter->FindByColumn((short)bslNoteID, _variant_t(nCurSelect, VT_I4), m_pBillStatusFilter->GetFirstRow(), VARIANT_TRUE);

		}


		
	} NxCatchAll(__FUNCTION__);


}

// (s.tullis 2014-07-01 10:28) - PLID 62577 - Handle Selections of Bill Status Filter
void CBillingFollowUpDlg::SelChosenBillstatusControl(LPDISPATCH lpRow)
{
	try
	{
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (!pRow) {
			return;
		}

		long nSelId = VarLong(pRow->GetValue((short)bslNoteID));

		if (nSelId == (long)ECriteriaSpecialIds::Multiple) {
			HandleMultiStatusSelection();
			return;
		}
		else {
			if (nSelId == (long)ECriteriaSpecialIds::All) {
				// The all row's ids are implicitly known.
				m_aryStatusesSelected.RemoveAll();
				m_bAllStatusesSelected = true;
			}
			else {
				m_aryStatusesSelected.RemoveAll();
				m_aryStatusesSelected.Add(nSelId);
				m_bAllStatusesSelected = false;
			}

			
		}

	}NxCatchAll(__FUNCTION__);
}

// (s.tullis 2014-07-01 09:53) - PLID 62577 - Handle Multiple Bill Status Filter Selection
void CBillingFollowUpDlg::HandleMultiStatusSelection(){
	try{


		HandleMultipleSelection(m_nxlMultipleBillStatus, m_pBillStatusFilter, IDC_BILLSTATUS_CONTROL, m_aryStatusesSelected, &m_bAllStatusesSelected,
			"BillstatusT", "Select one or more Bill status",
			(short)bslNoteID, (short)bslNote);




	}NxCatchAll(__FUNCTION__)

}

// (s.tullis 2014-07-01 09:53) - PLID 62577 - Handle Multiple Bill Status Selection
void CBillingFollowUpDlg::HandleMultipleSelection(
	CNxLabel &nxlMultiLabel, NXDATALIST2Lib::_DNxDataListPtr pCombo, UINT nComboControlID, CArray<long, long> &aryCurrentSelectionIDs,
	bool *pbAllSelected, CString strConfigRTName, CString strDescription,
	short nIDColumnIndex/* = 0*/, short nDescriptionColumnIndex/* = 1*/, CArray<short, short> *paryExtraColumnIndices/* = NULL*/)
{
	if (pCombo == NULL) {
		ThrowNxException("%s : pCombo is NULL (strConfigRTName = %s).", __FUNCTION__, strConfigRTName);
	}

	CMultiSelectDlg dlg(this, strConfigRTName);
	dlg.m_pfnContextMenuProc = CMultiSelectDlg_ContextMenu;
	dlg.m_nContextMenuProcParam = (LPARAM)this;
	dlg.m_bPutSelectionsAtTop = TRUE;

	if (aryCurrentSelectionIDs.GetSize() > 1) {
		// Preselect whatever we had selected before.
		dlg.PreSelect(aryCurrentSelectionIDs);
	}

	// Ensure that we don't end up showing the special rows.
	CVariantArray vaIDsToSkip;
	vaIDsToSkip.Add(_variant_t((long)ECriteriaSpecialIds::All, VT_I4));
	vaIDsToSkip.Add(_variant_t((long)ECriteriaSpecialIds::Multiple, VT_I4));
	vaIDsToSkip.Add(_variant_t((long)ECriteriaSpecialIds::None, VT_I4));

	// Now show the user the multi-select dialog.
	if (IDOK == dlg.OpenWithDataList2(pCombo, vaIDsToSkip, strDescription, 1, 0xFFFFFFFF, nIDColumnIndex, nDescriptionColumnIndex, paryExtraColumnIndices)) {
		// OK, they selected some records. Reflect their selections in the combo box and multi-label.

		// Get rid of the old selections. We have some new ones.
		aryCurrentSelectionIDs.RemoveAll();
		dlg.FillArrayWithIDs(aryCurrentSelectionIDs);
		// We need to tell if the user selected all the rows or just some of them.
		CArray<long, long> aryUnselectedIDs;
		dlg.FillArrayWithUnselectedIDs(&aryUnselectedIDs);

		// Did they select everything?
		if (pbAllSelected != NULL) {
			if (aryCurrentSelectionIDs.GetSize() > 1 && aryUnselectedIDs.GetSize() == 0) {
				// Yep. Note that if there was only 1 record available to select, we don't count that as all because:
				// If only one check box is checked, only display that X in the drop down.
				*pbAllSelected = true;
			}
			else {
				// Nope. Either there are some left over rows they didn't select or there was only 1 thing to select and they selected it.
				*pbAllSelected = false;
			}
		}
		else {
			// We don't care about selecting the All Row.
		}

		// Grab the all row. We need to make sure we have it available and maybe select it.
		NXDATALIST2Lib::IRowSettingsPtr pAllRow = pCombo->FindByColumn(nIDColumnIndex, _variant_t((long)ECriteriaSpecialIds::All, VT_I4), pCombo->GetFirstRow(), VARIANT_FALSE);
		// If the all row is available in the combo box and the user selected all the rows, then select the All row in the combo box.
		if (pbAllSelected != NULL && *pbAllSelected == true && pAllRow != NULL) {
			// Hide any hyperlink we might have showing and show the combo box instead.
			nxlMultiLabel.SetText("");
			nxlMultiLabel.SetType(dtsDisabledHyperlink);
			nxlMultiLabel.ShowWindow(SW_HIDE);
			GetDlgItem(nComboControlID)->ShowWindow(SW_SHOW);

			pCombo->CurSel = pAllRow;
			// All means all. Not some subset. We don't want to record the Ids.
			aryCurrentSelectionIDs.RemoveAll();
		}
		else if (aryCurrentSelectionIDs.GetSize() == 1) {
			// Show single selections in the combo box.
			nxlMultiLabel.SetText("");
			nxlMultiLabel.SetType(dtsDisabledHyperlink);
			nxlMultiLabel.ShowWindow(SW_HIDE);
			GetDlgItem(nComboControlID)->ShowWindow(SW_SHOW);

			long nSelectedId = aryCurrentSelectionIDs.GetAt(0);
			NXDATALIST2Lib::IRowSettingsPtr pSelectedRow = pCombo->FindByColumn(nIDColumnIndex, _variant_t(nSelectedId, VT_I4), pCombo->GetFirstRow(), VARIANT_TRUE);
			if (pSelectedRow == NULL) {
				ThrowNxException("%s : Could not find selected row (strConfigRTName = %s).", __FUNCTION__, strConfigRTName);
			}
		}
		else if (aryCurrentSelectionIDs.GetSize() > 1) {
			// Show multiple selections (other than All selections) in the multi select label
			CString strMultiSelectString = dlg.GetMultiSelectString();
			if (strMultiSelectString.GetLength() > 255) {
				strMultiSelectString = strMultiSelectString.Left(255);
			}

			GetDlgItem(nComboControlID)->ShowWindow(SW_HIDE);
			nxlMultiLabel.ShowWindow(SW_SHOW);
			nxlMultiLabel.SetText(strMultiSelectString);
			nxlMultiLabel.SetType(dtsHyperlink);
			nxlMultiLabel.SetToolTip(dlg.GetMultiSelectString("\r\n"));
		}
		else {
			// OK was pressed but no row was selected. This should be impossible.
			ThrowNxException("%s : No row was selected in the multi-select dialog but OK was pressed (strConfigRTName = %s).", __FUNCTION__, strConfigRTName);
		}

		nxlMultiLabel.AskParentToRedrawWindow();
		//ReloadSettings();
	}
	else {
		// They cancelled. Revert to the old selections
		if (pbAllSelected && *pbAllSelected == true) {
			// All records were selected before. Revert to the All row if we have it.
			NXDATALIST2Lib::IRowSettingsPtr pOldSelection = pCombo->FindByColumn(nIDColumnIndex, _variant_t((long)ECriteriaSpecialIds::All, VT_I4), pCombo->GetFirstRow(), VARIANT_TRUE);
		}
		else if ((pbAllSelected == NULL || *pbAllSelected == false) && aryCurrentSelectionIDs.GetSize() == 1) {
			// Only one record was selected before. Revert to it in the combo box.
			NXDATALIST2Lib::IRowSettingsPtr pOldSelection = pCombo->FindByColumn(nIDColumnIndex, _variant_t(aryCurrentSelectionIDs.GetAt(0), VT_I4), pCombo->GetFirstRow(), VARIANT_TRUE);
		}
		else if (aryCurrentSelectionIDs.GetSize() > 1) {
			// The multiple label is shown. We don't need to revert anything.
		}
		else if (aryCurrentSelectionIDs.GetSize() == 0) {
			// Highly unusual situation here. Don't think it is possible under normal circumstances. Try to select the All row and if unsuccessful, select NULL.
			if (!pCombo->FindByColumn(nIDColumnIndex, _variant_t((long)ECriteriaSpecialIds::All, VT_I4), pCombo->GetFirstRow(), VARIANT_TRUE)) {
				// Woah. Um.
				pCombo->CurSel = NULL;
				if (pbAllSelected) {
					*pbAllSelected = false;
				}
			}
			else if (pbAllSelected) {
				*pbAllSelected = true;
			}
		}
	}
	//EnsureControls();
}


// (s.tullis 2014-07-01 09:53) - PLID 62577 - Convert the Id's to Strings for the query
CString CBillingFollowUpDlg::GetArrayOfIDsForSQL(CArray<long, long> &aryCurrentSelections){
	
	
	
	CString ArrayIds = "";
	
		CString ID;


		for (int i = 0; i < aryCurrentSelections.GetSize(); i++)
		{
			ID.Format("%ld", aryCurrentSelections[i]);

			if (i == aryCurrentSelections.GetSize() - 1){


				ArrayIds = ArrayIds + ID + " ";
			}
			else{

				ArrayIds = ArrayIds + ID + " , ";

			}
 
		}

	
		return ArrayIds;

}

// (a.wilson 2014-07-07 12:24) - PLID 62809 - create the fields for the date filter and select the default option.
void CBillingFollowUpDlg::BuildDateFilter()
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pDateFilter->GetNewRow();
	if (pRow) {
		pRow->PutValue(emfcID, (long)edfoAllDates);
		pRow->PutValue(emfcName, _bstr_t("< All Dates >"));
		m_pDateFilter->AddRowAtEnd(pRow, NULL);
	}
	pRow = m_pDateFilter->GetNewRow();
	if (pRow) {
		pRow->PutValue(emfcID, (long)edfoClaimLastSentDate);
		pRow->PutValue(emfcName, _bstr_t("Claim Last Sent Date"));
		m_pDateFilter->AddRowAtEnd(pRow, NULL);
	}
	pRow = m_pDateFilter->GetNewRow();
	if (pRow) {
		pRow->PutValue(emfcID, (long)edfoBillServiceDate);
		pRow->PutValue(emfcName, _bstr_t("Bill Service Date"));
		m_pDateFilter->AddRowAtEnd(pRow, NULL);
	}
	pRow = m_pDateFilter->GetNewRow();
	if (pRow) {
		pRow->PutValue(emfcID, (long)edfoBillInputDate);
		pRow->PutValue(emfcName, _bstr_t("Bill Input Date"));
		m_pDateFilter->AddRowAtEnd(pRow, NULL);
	}
	pRow = m_pDateFilter->GetNewRow();
	if (pRow) {
		pRow->PutValue(emfcID, (long)edfoLastPaymentDate);
		pRow->PutValue(emfcName, _bstr_t("Last Payment Date"));
		m_pDateFilter->AddRowAtEnd(pRow, NULL);
	}
	pRow = m_pDateFilter->GetNewRow();
	if (pRow) {
		pRow->PutValue(emfcID, (long)edfoFirstAssignmentDate);
		pRow->PutValue(emfcName, _bstr_t("First Assignment Date"));
		m_pDateFilter->AddRowAtEnd(pRow, NULL);
	}
	// (s.tullis 2014-07-07 13:06) - PLID 62575 - Added last Assignment Date
	pRow = m_pDateFilter->GetNewRow();
	if (pRow) {
		pRow->PutValue(emfcID, (long)edfoLastAssignmentDate);
		pRow->PutValue(emfcName, _bstr_t("Last Assignment Date"));
		m_pDateFilter->AddRowAtEnd(pRow, NULL);
	}
	// (s.tullis 2014-07-07 13:08) - PLID 62576 - Status Date
	pRow = m_pDateFilter->GetNewRow();
	if (pRow) {
		pRow->PutValue(emfcID, (long)edfoStatusDate);
		pRow->PutValue(emfcName, _bstr_t("Status Date"));
		m_pDateFilter->AddRowAtEnd(pRow, NULL);
	}

	// (a.wilson 2014-07-10 10:53) - PLID 62529 - new date filter option.
	pRow = m_pDateFilter->GetNewRow();
	if (pRow) {
		pRow->PutValue(emfcID, (long)edfoLastStatusNoteDate);
		pRow->PutValue(emfcName, _bstr_t("Last Status Note Date"));
		m_pDateFilter->AddRowAtEnd(pRow, NULL);
	}
	m_pDateFilter->FindByColumn(emfcID, (long)edfoAllDates, NULL, VARIANT_TRUE);
	//setup controls regarding the filter.
	m_radioGreaterThanDays.SetCheck(TRUE);
	SetDlgItemInt(IDC_EDIT_DAYS_OLD, 0);
	m_dtFrom.SetValue(_variant_t(COleDateTime::GetCurrentTime()));
	m_dtTo.SetValue(_variant_t(COleDateTime::GetCurrentTime()));

	UpdateDateFilterState();
}

// (a.wilson 2014-07-07 12:24) - PLID 62809 - prevent null rows
void CBillingFollowUpDlg::SelChangingFilterDateList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.wilson 2014-07-07 13:12) - PLID 62809 - prevent null rows
void CBillingFollowUpDlg::SelChangingFilterResp(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.wilson 2014-07-07 13:47) - PLID 62809 - disable / enable contorls based on responsibility filter.
void CBillingFollowUpDlg::UpdateResponsibilityFilterState()
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pRespFilter->GetCurSel();
	if (pRow) {
		long nID = VarLong(pRow->GetValue(emfcID), erfoPatient);

		if (nID == (long)erfoPatient) {
			m_pInsuranceFilter->PutEnabled(VARIANT_FALSE);
			m_pHCFAFilter->PutEnabled(VARIANT_FALSE);
			m_pClaimSentFilter->PutEnabled(VARIANT_FALSE);
			m_checkNoInsPays.EnableWindow(FALSE);
		}
		else {
			m_pInsuranceFilter->PutEnabled(VARIANT_TRUE);
			m_pHCFAFilter->PutEnabled(VARIANT_TRUE);
			m_pClaimSentFilter->PutEnabled(VARIANT_TRUE);
			if (m_checkUseBalance.GetCheck()) {
				m_checkNoInsPays.EnableWindow(TRUE);
			}
			else {
				m_checkNoInsPays.EnableWindow(FALSE);
			}
		}
	}
}

// (a.wilson 2014-07-07 14:35) - PLID 62809 - disable / enable controls based on date filter.
void CBillingFollowUpDlg::UpdateDateFilterState()
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pDateFilter->GetCurSel();
	if (pRow) {
		long nID = VarLong(pRow->GetValue(emfcID), edfoAllDates);
		//disable date options if set to all dates.
		if (nID == (long)edfoAllDates) {
			m_radioGreaterThanDays.EnableWindow(FALSE);
			m_nxeditEditDaysOld.EnableWindow(FALSE);
			m_radioBetweenDates.EnableWindow(FALSE);
			m_dtFrom.EnableWindow(FALSE);
			m_dtTo.EnableWindow(FALSE);
		}			
		//enable date options if a selection is made other than all dates.
		else {
			m_radioGreaterThanDays.EnableWindow(TRUE);
			m_radioBetweenDates.EnableWindow(TRUE);
			//enable / disable controls based on radio button checked.
			if (m_radioGreaterThanDays.GetCheck()) {
				m_nxeditEditDaysOld.EnableWindow(TRUE);
				m_dtFrom.EnableWindow(FALSE);
				m_dtTo.EnableWindow(FALSE);
	}
			else if (m_radioBetweenDates.GetCheck()) {
				m_nxeditEditDaysOld.EnableWindow(FALSE);
				m_dtFrom.EnableWindow(TRUE);
				m_dtTo.EnableWindow(TRUE);
			}
		}
	}
}

// (a.wilson 2014-07-07 14:34) - PLID 62809 - update controls if selection changed.
void CBillingFollowUpDlg::SelChangedFilterResp(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try {
		if (NXDATALIST2Lib::IRowSettingsPtr(lpOldSel) != NXDATALIST2Lib::IRowSettingsPtr(lpNewSel)) {
			UpdateResponsibilityFilterState();
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.wilson 2014-07-07 14:35) - PLID 62809 - update controls if selection changed.
void CBillingFollowUpDlg::SelChangedFilterDateList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try {
		if (NXDATALIST2Lib::IRowSettingsPtr(lpOldSel) != NXDATALIST2Lib::IRowSettingsPtr(lpNewSel)) {
			UpdateDateFilterState();
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.wilson 2014-07-07 14:35) - PLID 62809 
void CBillingFollowUpDlg::SelChangingFilterInsuranceList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.wilson 2014-07-07 14:35) - PLID 62809 
void CBillingFollowUpDlg::SelChangingFilterHcfaList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.wilson 2014-07-07 15:47) - PLID 62809
void CBillingFollowUpDlg::SelChangingFilterClaimList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	} NxCatchAll(__FUNCTION__);
}

// (s.tullis 2014-07-08 09:19) - PLID 62574 - Prevent Null Selections
void CBillingFollowUpDlg::SelChangingOnholdFilter(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	} NxCatchAll(__FUNCTION__);
}

// (s.tullis 2014-07-08 09:20) - PLID 62577 - Prevent Null Selections
void CBillingFollowUpDlg::SelChangingBillstatusControl(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.wilson 2014-07-08 12:24) - PLID 62526
void CBillingFollowUpDlg::SetupResultListColumns()
{
	try {
		//remove all existing columns.
		m_mapResultColumns.RemoveAll();
		int nColCount = m_pResultsList->GetColumnCount();
		for (int i = 0; i < nColCount; i++){
			m_pResultsList->RemoveColumn(0);
		}
		// (r.gonet 07/25/2014) - PLID 62920 - Reset the count of static columns in the results list.
		m_nResultsListStaticColumnCount = 0;

		//this will always exist as such.
		// (r.gonet 07/25/2014) - PLID 62920 - When adding a static column to the datalist, you'll need to add it after the last static column but before the dynamic columns
		NXDATALIST2Lib::IColumnSettingsPtr(m_pResultsList->GetColumn(m_pResultsList->InsertColumn(ebfcBillID, _T("BillID"), _T("BillID"), 0, csVisible | csFixedWidth)))->FieldType = NXDATALIST2Lib::cftTextSingleLine;
		NXDATALIST2Lib::IColumnSettingsPtr(m_pResultsList->GetColumn(m_pResultsList->InsertColumn(ebfcInsuredPartyID, _T("InsuredPartyID"), _T("InsuredPartyID"), 0, csVisible | csFixedWidth)))->FieldType = NXDATALIST2Lib::cftTextSingleLine;
		NXDATALIST2Lib::IColumnSettingsPtr(m_pResultsList->GetColumn(m_pResultsList->InsertColumn(ebfcPatientID, _T("PatientID"), _T("PatientID"), 0, csVisible | csFixedWidth)))->FieldType = NXDATALIST2Lib::cftTextSingleLine;
		
		// (r.gonet 07/25/2014) - PLID 62920 - Now that the static columns are added, set the count of them.
		m_nResultsListStaticColumnCount = m_pResultsList->GetColumnCount();


		//First determine if they have a setup in data.
		_RecordsetPtr prs = CreateParamRecordset("SELECT * FROM BillingFollowUpColumnsT WHERE UserID = {INT} ORDER BY OrderIndex ASC", GetCurrentUserID());
		if (!prs->eof) {
			BillingColumn bcColumn;

			for (int i = m_nResultsListStaticColumnCount; !prs->eof; i++) {

				bcColumn = BillingColumn(AdoFldLong(prs, "ColumnID"), i, (m_checkRememberColumnWidths.GetCheck() == BST_CHECKED ? AdoFldLong(prs, "StoredWidth") : -1), "");
				CreateResultListColumn(bcColumn);
				prs->MoveNext();
			}
			//ensure all columns were created.
			for (int i = m_nResultsListStaticColumnCount; i < (long)ebfcNumColumns; i++)
			{
				BillingColumn bc;
				m_mapResultColumns.Lookup((short)i, bc);

				if (bc.nColumnID == -1) {
					bc = BillingColumn(i, m_pResultsList->GetColumnCount(), -1, "");
					CreateResultListColumn(bc);
				}
			}
		}
		//they do not have a remembered sizing or column order, setup defaults.
		else {
			BillingColumn bcColumn;
			for (int i = m_nResultsListStaticColumnCount; i < (int)ebfcNumColumns; i++) {
				bcColumn = BillingColumn(i, i, -1, "");

				CreateResultListColumn(bcColumn);
			}
		}
		//bool bHasColumnSetup = false;
		//_RecordsetPtr prs = CreateParamRecordset("SELECT * FROM BillingFollowUpColumnsT WHERE UserID = {INT} ORDER BY OrderIndex ASC", GetCurrentUserID());
		//if (!prs->eof)
		//	bHasColumnSetup = true;

		//// (r.gonet 07/25/2014) - PLID 62920 - Start from just after the last static column.
		//// Go up to the column enumeration's end.
		//for (long i = m_nResultsListStaticColumnCount; i < (long)ebfcNumColumns; i++) {
		//	BillingColumn bcColumn;
		//	if (bHasColumnSetup && !prs->eof) {
		//		bcColumn = BillingColumn(AdoFldLong(prs, "ColumnID"), AdoFldLong(prs, "OrderIndex"), (m_checkRememberColumnWidths.GetCheck() == BST_CHECKED ? AdoFldLong(prs, "StoredWidth") : -1), "");
		//		prs->MoveNext();
		//	}
		//	else if (!bHasColumnSetup) {
		//		bcColumn = BillingColumn(i, i, -1, "");
		//	} else {
		//		//something happened during the column setup.  this should not happen.
		//		ASSERT(FALSE);
		//		return;
		//	}
		//	CreateResultListColumn(bcColumn);
		//}

	} NxCatchAll(__FUNCTION__);
}
// (s.tullis 2014-07-16 10:23) - PLID 62575 -Added Last Assignment Date
// (a.wilson 2014-08-01 10:06) - PLID 62784 - added note icon.
void CBillingFollowUpDlg::CreateResultListColumn(BillingColumn bcColumn)
{
	_bstr_t bstrField;
	long nStyle = -1;
	//if storedwidth == -1 then use default length otherwise use set width.
	switch ((BillingFUColumns)bcColumn.nColumnID) 
	{
	case ebfcBillNoteIcon:	//NOTE ICON
		bcColumn.strName = "";
		bstrField = _T("CASE WHEN HasNotes = 1 THEN '' ELSE 'BITMAP:FILE' END");
		bcColumn.nStoredWidth = (bcColumn.nStoredWidth != -1 ? bcColumn.nStoredWidth : 40);
		break;
	case ebfcBillDate: //BILL DATE
		bcColumn.strName = "Bill Date";
		bstrField = _T("BillDate");
		bcColumn.nStoredWidth = (bcColumn.nStoredWidth != -1 ? bcColumn.nStoredWidth : 65);
		break;
	case ebfcBillInputDate: //INPUT DATE
		bcColumn.strName = "Input Date";
		bstrField = _T("InputDate");
		bcColumn.nStoredWidth = (bcColumn.nStoredWidth != -1 ? bcColumn.nStoredWidth : 65);
		break;
	case ebfcDateSent: //DATE SENT
		bcColumn.strName = "Date Sent";
		bstrField = _T("LastDate");
		bcColumn.nStoredWidth = (bcColumn.nStoredWidth != -1 ? bcColumn.nStoredWidth : 65);
		break;
	case ebfcFirstAssigned: //FIRST ASSIGNED
		bcColumn.strName = "First Assigned";
		bstrField = _T("RespAssignmentDate");
		bcColumn.nStoredWidth = (bcColumn.nStoredWidth != -1 ? bcColumn.nStoredWidth : 83);
		break;
	case ebfcLastAssigned: //Last ASSIGNED
		bcColumn.strName = "Last Assigned";
		bstrField = _T("LastRespAssignmentDate");
		bcColumn.nStoredWidth = (bcColumn.nStoredWidth != -1 ? bcColumn.nStoredWidth : 83);
		break;
	case ebfcBillOnHoldIcon: //BILL ON HOLD ICON
		bcColumn.strName = "";
		bstrField = _T(FormatString("CASE WHEN OnHold = %li THEN %lu ELSE NULL END", EBillStatusType::OnHold, (DWORD)m_hIconBillOnHold));
		bcColumn.nStoredWidth = (bcColumn.nStoredWidth != -1 ? bcColumn.nStoredWidth : 40);
		break;
	case ebfcBillStatus: //STATUS NOTE
		bcColumn.strName = "Status Note";
		bstrField = _T("BillStatusNote");
		nStyle = (bcColumn.nStoredWidth == -1 ? csVisible | csWidthAuto : -1);
		bcColumn.nStoredWidth = (bcColumn.nStoredWidth != -1 ? bcColumn.nStoredWidth : 400);
		break;
	case ebfcPatientName: //PATIENT NAME
		bcColumn.strName = "Patient Name";
		bstrField = _T("PatName");
		bcColumn.nStoredWidth = (bcColumn.nStoredWidth != -1 ? bcColumn.nStoredWidth : 80);
		break;
	case ebfcInsCoName: //INSURANCE COMPANY
		bcColumn.strName = "Insurance Co.";
		bstrField = _T("InsCoName");
		bcColumn.nStoredWidth = (bcColumn.nStoredWidth != -1 ? bcColumn.nStoredWidth : 80);
		break;
	case ebfcRespTypeName: //RESPONSIBILITY TYPE
		bcColumn.strName = "Resp. Type";
		bstrField = _T("RespTypeName");
		bcColumn.nStoredWidth = (bcColumn.nStoredWidth != -1 ? bcColumn.nStoredWidth : 60);
		break;
	case ebfcTotalCharges: //TOTAL RESPONSIBILITY
		bcColumn.strName = "Total Resp.";
		bstrField = _T("TC");
		bcColumn.nStoredWidth = (bcColumn.nStoredWidth != -1 ? bcColumn.nStoredWidth : 80);
		break;
	case ebfcAppliedAmount: //APPLIED AMOUNT
		bcColumn.strName = "Applied Amt.";
		bstrField = _T("TP");
		bcColumn.nStoredWidth = (bcColumn.nStoredWidth != -1 ? bcColumn.nStoredWidth : 80);
		break;
	case ebfcInsResp: //RESPONSIBILITY BALANCE
		bcColumn.strName = "Resp. Balance";
		bstrField = _T("Bal");
		bcColumn.nStoredWidth = (bcColumn.nStoredWidth != -1 ? bcColumn.nStoredWidth : 80);
		break;
	case ebfcLocationName: //LOCATION
		bcColumn.strName = "Location";
		bstrField = _T("LocName");
		bcColumn.nStoredWidth = (bcColumn.nStoredWidth != -1 ? bcColumn.nStoredWidth : 60);
		break;
	case ebfcProviderName: //PROVIDER
		bcColumn.strName = "Provider";
		bstrField = _T("ProviderName");
		bcColumn.nStoredWidth = (bcColumn.nStoredWidth != -1 ? bcColumn.nStoredWidth : 60);
		break;
	case ebfcPrimaryInsCoName: //PRIMARY INSURANCE
		bcColumn.strName = "Primary Insurance Co.";
		bstrField = _T("PriInsCoName");
		bcColumn.nStoredWidth = (bcColumn.nStoredWidth != -1 ? bcColumn.nStoredWidth : 80);
		break;
	case ebfcBillID:
	case ebfcInsuredPartyID:
	case ebfcPatientID:
	default:
		//do nothing these aren't suppose to have settings.
		ASSERT(FALSE);
		return;
	}
	//save to map.
	m_mapResultColumns.SetAt((short)bcColumn.nColumnID, bcColumn);
	//insert the column into the datalist.
	NXDATALIST2Lib::IColumnSettingsPtr pColumn = m_pResultsList->GetColumn(m_pResultsList->InsertColumn((short)bcColumn.nOrderIndex, 
		bstrField, _bstr_t(bcColumn.strName), bcColumn.nStoredWidth, (nStyle == -1 ? csVisible : nStyle)));
	if (bcColumn.nColumnID == ebfcBillOnHoldIcon || bcColumn.nColumnID == ebfcBillNoteIcon) {
		pColumn->PutFieldType(NXDATALIST2Lib::cftBitmapBuiltIn);
	} else {
		pColumn->PutFieldType(NXDATALIST2Lib::cftTextSingleLine);
	}

}

// (a.wilson 2014-07-08 12:26) - PLID 62526
void CBillingFollowUpDlg::OnBnClickedConfigureColumnsBtn()
{
	try {
		//generate the list of columns to rearrange.
		// (r.gonet 07/25/2014) - PLID 62920 - Use the static count member variable rather than a magic number.
		long nCount = m_mapResultColumns.GetCount() + m_nResultsListStaticColumnCount;
		CArray<ConfigureColumn> aryColumns;

		// (r.gonet 07/25/2014) - PLID 62920 - Use the static count member variable rather than a magic number.
		for (int i = m_nResultsListStaticColumnCount; i < nCount; i++) {
			BillingColumn bc;
			m_mapResultColumns.Lookup((short)i, bc);
			// (r.gonet 07/25/2014) - PLID 62920 - Use the static count member variable rather than a magic number.
			aryColumns.SetAtGrow(bc.nOrderIndex - m_nResultsListStaticColumnCount, ConfigureColumn(bc.nColumnID, bc.strName, bc.nOrderIndex));
			// (r.gonet 07/28/2014) - PLID 63078 - The bill on hold icon is nameless. We need a name though in the configure dialog.
			if (bc.nColumnID == ebfcBillOnHoldIcon) {
				aryColumns[bc.nOrderIndex - m_nResultsListStaticColumnCount].strName = "Bill On Hold Icon";
			}
			else if (bc.nColumnID == ebfcBillNoteIcon) {
				aryColumns[bc.nOrderIndex - m_nResultsListStaticColumnCount].strName = "Bill Note Icon";
			}
		}

		CConfigureColumnsDlg dlg(aryColumns, this);
		//if the order changed then we need to update and reset the datalist.
		if (IDOK == dlg.DoModal() && dlg.m_bOrderChanged == true) {
			aryColumns.RemoveAll();
			aryColumns.Copy(dlg.GetOrderedColumns());

			for (int i = 0; i < aryColumns.GetCount(); i++) {
				BillingColumn bc;
				ConfigureColumn cc = aryColumns.GetAt(i);
				if (m_mapResultColumns.Lookup((short)cc.nColumnID, bc)) {
					bc.nOrderIndex = cc.nOrderIndex;
					m_mapResultColumns[(short)cc.nColumnID] = bc;
				}
				else {
					//this should never happen.
					ASSERT(FALSE);
				}
			}
			//save the changes made to the reorder.  only save the column widths if remember is on.
			SaveResultListColumnsSetup((m_checkRememberColumnWidths.GetCheck() ? true : false));
			//rebuild the datalist with the column changes.
			bool bReloadList = (m_pResultsList->GetRowCount() > 0 ? true : false);
			SetupResultListColumns();
			//requery the list if there were results during the configuration.
			if (bReloadList) {
				OnBtnLoad();
			}
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.wilson 2014-07-08 13:51) - PLID 62528 - rewritten for datalist upgrade.
void CBillingFollowUpDlg::RButtonDownBillfuResultsList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if (!pRow) {
			return;
		}

		//(e.lally 2009-09-01) PLID 13134 - Support multiple selections
		//Make sure the list is not still requerying, or it could mess up everything.
		if (m_pResultsList->IsRequerying() != VARIANT_FALSE){
			return;
		}

		// (j.jones 2013-03-18 09:31) - PLID 55712 - if they right clicked a row that is not currently selected,
		// we'll change the selection to that row, otherwise do not try to change the selection
		bool bIsCurrentlySelected = false;
		long nSelectedRows = 0;
		for (NXDATALIST2Lib::IRowSettingsPtr pSelectedRow = m_pResultsList->GetFirstSelRow(); pSelectedRow; pSelectedRow = pSelectedRow->GetNextSelRow()) {
			nSelectedRows++;
			if (pRow == pSelectedRow) {
				bIsCurrentlySelected = true;
			}
		}

		if (!bIsCurrentlySelected) {
			m_pResultsList->PutCurSel(pRow);
			nSelectedRows = 1;
		}

		if (pRow->GetValue(ebfcBillID).vt != VT_EMPTY) {

			// Build a menu popup
			CMenu pMenu;
			pMenu.CreatePopupMenu();
			if (nSelectedRows == 1){
				// (r.gonet 07/25/2014) - PLID 62920 - Get the bill's current status type.
				long nBillID = pRow->GetValue(ebfcBillID);
				EBillStatusType eBillStatusType = EBillStatusType::None;
				_RecordsetPtr prsBill = CreateParamRecordset(
					"SELECT BillStatusT.Type AS BillStatusType "
					"FROM BillsT "
					"LEFT JOIN BillStatusT ON BillsT.StatusID = BillStatusT.ID "
					"WHERE BillsT.ID = {INT} "
					, nBillID);
				if (!prsBill->eof) {
					eBillStatusType = (EBillStatusType)AdoFldLong(prsBill->Fields, "BillStatusType", (long)EBillStatusType::None);
				}
				prsBill->Close();
				
				pMenu.InsertMenu(-1, MF_BYPOSITION, IDC_GOTOPATIENT, "Go To Patient");
				pMenu.InsertMenu(-1, MF_BYPOSITION | MF_SEPARATOR);
				
				// (r.gonet 07/25/2014) - PLID 62920 - If the bill is on hold, then let them remove the hold. If the bill is not on hold, then let them
				// mark it on hold.
				if (eBillStatusType != EBillStatusType::OnHold) {
					pMenu.InsertMenu(-1, MF_BYPOSITION, ID_MARK_BILL_ON_HOLD, "Mark Bill On Hold");
				}
				else {
					pMenu.InsertMenu(-1, MF_BYPOSITION, ID_REMOVE_BILL_HOLD, "Remove Bill Hold");
				}
				
				pMenu.InsertMenu(-1, MF_BYPOSITION, ID_REMOVE_SELECTED_BILLS, "Remove Bill From List");
				//TES 7/28/2014 - PLID 62785 - Added Edit Charges menu option
				pMenu.InsertMenu(-1, MF_BYPOSITION, ID_OPEN_BILL, "Edit Charges");
			}
			else if (nSelectedRows > 1){
				pMenu.InsertMenu(-1, MF_BYPOSITION, ID_REMOVE_SELECTED_BILLS, FormatString("Remove All (%li) Selected Bills From List", nSelectedRows));
			}
			else{
				//This should not be possible
				ASSERT(FALSE);
			}
			CPoint pt;
			GetCursorPos(&pt);
			pMenu.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this);
		}
	} NxCatchAll(__FUNCTION__);
}
// (a.wilson 2014-07-08 14:26) - PLID 62528 - for datalist2 upgrade.
void CBillingFollowUpDlg::RequeryFinishedBillfuResultsList(short nFlags)
{
	try {
		// (a.wilson 2014-08-01 11:12) - PLID 62784
		BillingColumn bc;
		for (NXDATALIST2Lib::IRowSettingsPtr pRow = m_pResultsList->GetFirstRow(); pRow; pRow = pRow->GetNextRow()) {
			m_mapResultColumns.Lookup(ebfcBillNoteIcon, bc);
			CString strHasNote = AsString(pRow->GetValue((short)bc.nOrderIndex));
			if (strHasNote.IsEmpty()) {
				pRow->PutValue((short)bc.nOrderIndex, (long)m_hBillNote);
			}
		}

		//re-enable the controls
		EnableListControls(TRUE);

		//reset the timeout
		m_pIncreaseCommandTimeout.reset();

		SetDlgItemInt(IDC_RESULTS_COUNT, m_pResultsList->GetRowCount());

		CRect rc;
		GetDlgItem(IDC_RESULTS_COUNT)->GetWindowRect(rc);
		ScreenToClient(rc);
		InvalidateRect(rc);
		// (s.tullis 2014-06-24 14:51) - PLID 62506 - Permission: Billing Followup User Permission to Control Read and Writing in the Billing Followup Tab
		SecureControls();
	} NxCatchAll(__FUNCTION__);
}

void CBillingFollowUpDlg::ColumnSizingFinishedBillfuResultsList(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth)
{
	try {
		if (m_checkRememberColumnWidths.GetCheck()) {
			// (r.gonet 07/25/2014) - PLID 62920 - Use the static count member variable rather than a magic number.
			long nCount = m_mapResultColumns.GetCount() + m_nResultsListStaticColumnCount;
			for (int i = m_nResultsListStaticColumnCount; i < nCount; i++) {
				BillingColumn bc;
				m_mapResultColumns.Lookup((short)i, bc);
				if (bc.nOrderIndex == nCol) {
					bc.nStoredWidth = nNewWidth;
					m_mapResultColumns.SetAt((short)i, bc);
					return;
				}
			}
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.wilson 2014-07-08 12:01) - PLID 62528 - alter the datalist based on remembering column widths.
void CBillingFollowUpDlg::OnBnClickedBillingfollowupRememberColumns()
{
	try {
		SetRemotePropertyInt("RememberBillingFollowUpColumnWidths", m_checkRememberColumnWidths.GetCheck(), 0, GetCurrentUserName());

		if (m_checkRememberColumnWidths.GetCheck()) {
			//existing column sizes aren't important. load original setup.
			SetupResultListColumns();
		}
		else {
			//save current columns
			SaveResultListColumnsSetup();
			//reload original setup widths
			bool bReloadList = (m_pResultsList->GetRowCount() > 0 ? true : false);
			SetupResultListColumns();
			//requery the list if there were results during the configuration.
			if (bReloadList) {
				OnBtnLoad();
			}
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.wilson 2014-07-08 12:01) - PLID 62526
void CBillingFollowUpDlg::SaveResultListColumnsSetup(bool bSaveColumnWidths /*= true*/)
{
	//build
	CString strXml = "<Columns>\r\n";
	// (r.gonet 07/25/2014) - PLID 62920 - Use the static count member variable rather than a magic number.
	for (int i = m_nResultsListStaticColumnCount; i < m_mapResultColumns.GetCount() + m_nResultsListStaticColumnCount; i++) {
		BillingColumn bc;
		m_mapResultColumns.Lookup(i, bc);
		strXml += FormatString("\t<Column ColumnID=\"%li\" OrderIndex=\"%li\" StoredWidth=\"%li\" />\r\n", bc.nColumnID, bc.nOrderIndex, bc.nStoredWidth);
	}
	strXml += "</Columns>";
	//insert or update the table with the new data.  if its an insert then we still save the default column widths.
	ExecuteParamSql(R"(
		BEGIN TRAN
			DECLARE @UserID INT
			SET @UserID = {INT}
			DECLARE @RememberColumnWidths BIT
			SET @RememberColumnWidths = {BIT}
			DECLARE @ColumnsXml XML
			SET @ColumnsXml = {STRING}

			IF EXISTS(SELECT TOP 1 * FROM BillingFollowUpColumnsT WHERE UserID = @UserID)
			BEGIN
				UPDATE BillingFollowUpColumnsT SET OrderIndex = XmlQ.OrderIndex, StoredWidth = CASE WHEN @RememberColumnWidths = 1 THEN XmlQ.StoredWidth ELSE BillingFollowUpColumnsT.StoredWidth END
				FROM BillingFollowUpColumnsT
				INNER JOIN (
					SELECT	Columns.Record.value('@ColumnID', 'INT') ColumnID,
							Columns.Record.value('@OrderIndex', 'INT') OrderIndex,
							Columns.Record.value('@StoredWidth', 'INT') StoredWidth
					FROM	@ColumnsXml.nodes('Columns/Column') Columns(Record)
				) XmlQ ON BillingFollowUpColumnsT.ColumnID = XmlQ.ColumnID
				WHERE UserID = @UserID
				--Insert any missing columns.
				INSERT INTO BillingFollowUpColumnsT
				SELECT @UserID, ColumnID, OrderIndex, StoredWidth FROM (
					SELECT	Columns.Record.value('@ColumnID', 'INT') ColumnID,
							Columns.Record.value('@OrderIndex', 'INT') OrderIndex,
							Columns.Record.value('@StoredWidth', 'INT') StoredWidth
					FROM	@ColumnsXml.nodes('Columns/Column') Columns(Record)
				) XmlQ WHERE ColumnID NOT IN (SELECT ColumnID FROM BillingFollowUpColumnsT WHERE UserID = @UserID)
			END
			ELSE
			BEGIN
				INSERT INTO BillingFollowUpColumnsT
				SELECT @UserID, ColumnID, OrderIndex, StoredWidth FROM (
					SELECT	Columns.Record.value('@ColumnID', 'INT') ColumnID,
							Columns.Record.value('@OrderIndex', 'INT') OrderIndex,
							Columns.Record.value('@StoredWidth', 'INT') StoredWidth
					FROM	@ColumnsXml.nodes('Columns/Column') Columns(Record)
				) XmlQ
			END

			SELECT * FROM BillingFollowUpColumnsT WHERE UserID = @UserID
		COMMIT TRAN
		)", GetCurrentUserID(), bSaveColumnWidths, strXml);
}

// (a.wilson 2014-07-10 10:25) - PLID 62528 save any column changes if remember is checked.
void CBillingFollowUpDlg::OnDestroy()
{
	try {
		if (m_checkRememberColumnWidths.GetCheck()) {
			SaveResultListColumnsSetup();
		}
	} NxCatchAll(__FUNCTION__);

	CNxDialog::OnDestroy();
}

// (s.tullis 2014-07-14 16:11) - PLID 62574 - Need to save this filter for later
void CBillingFollowUpDlg::SelChosenOnholdFilter(LPDISPATCH lpRow)
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow = lpRow;

		if (pRow){
			SetRemotePropertyInt("BillOnHoldFilter", VarLong(pRow->GetValue(ohfID)), 0, GetCurrentUserName());
		}

	}NxCatchAll(__FUNCTION__)

}

// (a.wilson 2014-07-16 08:36) - PLID 62905 - reset all the top filters to their original state when the dialog was first opened.
void CBillingFollowUpDlg::OnStnClickedBillingfuReset()
{
	try {
		//PATIENT
		m_PatientList->SetSelByColumn(epfcID, (long)efidAll);
		//LOCATION
		ShowDlgItem(IDC_MULTI_LOC_LABEL, SW_HIDE);
		ShowDlgItem(IDC_FILTER_LOCATION, SW_SHOW);
		m_nLocationID = GetCurrentLocationID();
		m_LocationCombo->SetSelByColumn(emfcID, m_nLocationID);
		//PROVIDER
		m_pProviderCombo->SetSelByColumn(emfcID, (long)efidAll);
		//RESPONSIBILITY
		m_pRespFilter->SetSelByColumn(emfcID, (long)erfoPatient);
		//INSURANCE COMPANY
		m_pInsuranceFilter->SetSelByColumn(emfcID, (long)efidAll);
		//HCFA GROUP
		m_pHCFAFilter->SetSelByColumn(emfcID, (long)efidAll);
		//Update any places affected by the change of the responsibility filter.
		UpdateResponsibilityFilterState();
	} NxCatchAll(__FUNCTION__);
}

// (s.tullis 2014-08-25 09:10) - PLID 62577 - In the Financial Module > Billing Follow Up tab, under the ‘Filters’ section, add a drop down for the user to be able to search for bills based on a status. The drop down should default to < All Statuses >. There should also be an option for the user to select < No Status >.
void CBillingFollowUpDlg::SetBillStatusText(CString strText, CString strNewToopTip){
	CString strTextDelim = ",";
	CString strToolTipDelim = "\r\n";

	try{
		// get rid of the extra delimeter
		if (strText.Right(strTextDelim.GetLength()).Compare(strTextDelim) == 0){
			strText = strText.Left(strText.GetLength() - strTextDelim.GetLength());
		}
		// get rid of the extra delimeter
		if (strNewToopTip.Right(strToolTipDelim.GetLength()).Compare(strToolTipDelim) == 0){
			strNewToopTip = strNewToopTip.Left(strNewToopTip.GetLength() - strToolTipDelim.GetLength());
		}
		//Ensure its within 255 so no overflow
		if (strText.GetLength() > 255) {
			strText = strText.Left(255);
		}

		GetDlgItem(IDC_BILLSTATUS_CONTROL)->ShowWindow(SW_HIDE);
		m_nxlMultipleBillStatus.SetText(strText);


		m_nxlMultipleBillStatus.SetToolTip(strNewToopTip);

		m_nxlMultipleBillStatus.SetType(dtsHyperlink);
		m_nxlMultipleBillStatus.ShowWindow(SW_SHOW);
	}NxCatchAll(__FUNCTION__)



}
// (s.tullis 2014-08-25 09:10) - PLID 62577 - MultiSelect Response to tablecheckers
void CBillingFollowUpDlg::HandleBillStatusTableCheckerRefresh(long nCurSel){
	
	try{//single select is active

		NXDATALIST2Lib::IRowSettingsPtr pRow;
		if (!m_nxlMultipleBillStatus.IsWindowVisible()){
			pRow = m_pBillStatusFilter->SetSelByColumn((short)bslNoteID, _variant_t(nCurSel, VT_I4));

			if (pRow == NULL){
				m_pBillStatusFilter->SetSelByColumn((short)bslNoteID, _variant_t((long)ECriteriaSpecialIds::All, VT_I4));
				
			}
		}// multiselect is active
		else if (m_nxlMultipleBillStatus.IsWindowVisible()){
			
			CString strNewText = "";
			CString strNewToolTip = "";
			for (int i = 0; i < m_aryStatusesSelected.GetSize(); i++){
				pRow = m_pBillStatusFilter->SetSelByColumn((short)bslNoteID, _variant_t(m_aryStatusesSelected[i], VT_I4));
			
				// the bill status was deleted need to remove it
				if (pRow == NULL){

					m_aryStatusesSelected.RemoveAt(i--);

				}
				else{// include it in our new list

					strNewText = strNewText + VarString(pRow->GetValue(bslNote));
					strNewToolTip = strNewToolTip + VarString(pRow->GetValue(bslNote));


					strNewText = strNewText + ",";
					strNewToolTip = strNewToolTip + "\r\n";



				}



			}
			
			// diable the mutliselect and select ALL in the combo
			if ((m_aryStatusesSelected.GetSize() == (m_pBillStatusFilter->GetRowCount()-3))){

			
				m_nxlMultipleBillStatus.SetText("");
				m_nxlMultipleBillStatus.SetToolTip("");
				m_nxlMultipleBillStatus.SetType(dtsDisabledHyperlink);
				m_nxlMultipleBillStatus.ShowWindow(SW_HIDE);
				GetDlgItem(IDC_BILLSTATUS_CONTROL)->ShowWindow(SW_SHOW);
				m_pBillStatusFilter->SetSelByColumn((short)bslNoteID, _variant_t((long)ECriteriaSpecialIds::All, VT_I4));

			}// set the new Id's in the label
			else if (m_aryStatusesSelected.GetSize() > 1){

				SetBillStatusText(strNewText, strNewToolTip);
			}// diable the label and select the id in the combo
			else if (m_aryStatusesSelected.GetSize() == 1 ){
				// Show single selections in the combo box.
				m_nxlMultipleBillStatus.SetText("");
				m_nxlMultipleBillStatus.SetToolTip("");
				m_nxlMultipleBillStatus.SetType(dtsDisabledHyperlink);
				m_nxlMultipleBillStatus.ShowWindow(SW_HIDE);
				GetDlgItem(IDC_BILLSTATUS_CONTROL)->ShowWindow(SW_SHOW);

				long nSelectedId = m_aryStatusesSelected.GetAt(0);
				m_pBillStatusFilter->SetSelByColumn((short)bslNoteID, _variant_t(m_aryStatusesSelected[0], VT_I4));
			

			}// Everything that was currently selected was deleted set it back to default
			else if (m_aryStatusesSelected.IsEmpty()){
				m_nxlMultipleBillStatus.SetText("");
				m_nxlMultipleBillStatus.SetToolTip("");
				m_nxlMultipleBillStatus.SetType(dtsDisabledHyperlink);
				m_nxlMultipleBillStatus.ShowWindow(SW_HIDE);
				GetDlgItem(IDC_BILLSTATUS_CONTROL)->ShowWindow(SW_SHOW);
				m_pBillStatusFilter->SetSelByColumn((short)bslNoteID, _variant_t((long)ECriteriaSpecialIds::All, VT_I4));

			}
			else{// should not happen
				ASSERT(FALSE);
			}


		}


	}NxCatchAll(__FUNCTION__)


	
}
void CBillingFollowUpDlg::OnOpenBill()
{
	try {
		//TES 7/28/2014 - PLID 62785 - Added Edit Charges menu option
		//Gather the data we need
		if (m_pResultsList->CurSel == NULL) {
			ASSERT(FALSE);
			return;
		}
		long nBillID = VarLong(m_pResultsList->CurSel->GetValue(ebfcBillID), -1);
		long nPatientID = VarLong(m_pResultsList->CurSel->GetValue(ebfcPatientID), -1);
		if (nBillID == -1 || nPatientID == -1) {
			ASSERT(FALSE);
			return;
		}

		//Select the patient
		if (!GetMainFrame()->m_patToolBar.TrySetActivePatientID(nPatientID)) {
			return;
		}
		//Open the Patients module
		if (!GetMainFrame()->FlipToModule(PATIENT_MODULE_NAME)) {
			ThrowNxException("Failed to open Patients module!");
			return;
		}
		CPatientView* pView = (CPatientView *)GetMainFrame()->GetOpenView(PATIENT_MODULE_NAME);
		
		//Check permissions
		if (!CanChangeHistoricFinancial("Bill", nBillID, bioBill, sptRead)) {
			return;
		}
		//Now call the function.
		pView->OpenBill(nBillID);
	}NxCatchAll(__FUNCTION__);
}

// (a.wilson 2014-08-01 10:14) - PLID 62784 - launch the note dialog if they click the icon.
void CBillingFollowUpDlg::LeftClickBillfuResultsList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (!pRow) {
			return;
		}

		//Note Icon
		BillingColumn bcNoteColumn;
		m_mapResultColumns.Lookup(ebfcBillNoteIcon, bcNoteColumn);
		if (bcNoteColumn.nOrderIndex == nCol) {
			int nBillID = AsLong(pRow->GetValue(ebfcBillID));
			int nPatientID = AsLong(pRow->GetValue(ebfcPatientID));
			
			if (nBillID > 0 && nPatientID > 0) {
				CNotesDlg dlgNotes(this);
				dlgNotes.SetPersonID(nPatientID);
				dlgNotes.m_bIsBillingNote = true;
				dlgNotes.m_bntBillingNoteType = bntBill;
				dlgNotes.m_nBillID = nBillID;
				CNxModalParentDlg dlg(this, &dlgNotes, CString("Bill Notes"));
				dlg.DoModal();

				BOOL bHasNotes = FALSE;
				_RecordsetPtr prs = CreateParamRecordset("SELECT CAST( CASE WHEN {INT} IN (SELECT BillID FROM Notes WHERE Notes.BillID IS NOT NULL) THEN 1 ELSE 0 END AS BIT) AS HasNotes", nBillID);
				if (!prs->eof) {
					bHasNotes = AdoFldBool(prs, "HasNotes");
				}

				NXDATALIST2Lib::IRowSettingsPtr pRow = m_pResultsList->GetFirstRow();
				while (pRow) {
					if (VarLong(pRow->GetValue(ebfcBillID), -1) == nBillID) {
						pRow->PutValue(nCol, (bHasNotes == FALSE ? _bstr_t("BITMAP:FILE") : _variant_t((long)m_hBillNote, VT_I4)));
					}
					pRow = pRow->GetNextRow();
				}
			}
		}
	} NxCatchAll(__FUNCTION__);
}

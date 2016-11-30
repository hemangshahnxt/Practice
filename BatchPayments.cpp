// BatchPayments.cpp : implementation file
//

#include "stdafx.h"
#include "BatchPayments.h"
#include "FinancialApply.h"
#include "PaymentDlg.h"
#include "GlobalFinancialUtils.h"
#include "BatchPaymentEditDlg.h"
#include "CalculatePercentageDlg.h"
#include "AuditTrail.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "GoStatements.h"
#include "EOBDlg.h"
#include "FinancialLineItemPostingDlg.h"
#include "RefundedBatchPaysDlg.h"
#include "GlobalDrawingUtils.h"
#include "DontShowDlg.h"
#include "UserWarningDlg.h"
#include "FinancialCorrection.h"
#include "BatchPaymentInsReversalDlg.h"	// (j.armen 2012-05-24 17:33) - PLID 50520
#include "LockboxManageDlg.h"
#include "ShiftInsRespsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37021 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.


// (a.walling 2010-04-06 13:51) - PLID 23643 - inappropriate command ID range (0x8000 -> 0xDFFF / 32768 -> 57343)
#define ID_APPLY_TO_CHARGE				43345
#define ID_APPLY_TO_BILL				43346
#define	ID_INVOKE_CHARGE_POSTING_DLG	43347
#define	ID_INVOKE_BILL_POSTING_DLG		43348
#define ID_PRINT_STATEMENT				43349
#define ID_CREATE_ADJUSTMENT			43350
#define ID_POST_PAYMENT					43351
#define ID_GOTO_PATIENT					43352 // (a.vengrofski 2010-05-04 12:13) - PLID <36205> - Added to be able to handle the context menu selection.
// (j.jones 2012-08-20 16:38) - PLID 52116 - added auto-apply options
#define ID_AUTO_APPLY_TO_BILL			43354
#define ID_AUTO_APPLY_TO_CHARGE			43355

#define COMBO_COLUMN_ID					0
#define COMBO_COLUMN_INSCO_ID			1
#define COMBO_COLUMN_DATE				2
#define COMBO_COLUMN_INSCO_NAME			3
#define COMBO_COLUMN_DESCRIPTION		4
#define COMBO_COLUMN_CHECK_NO			5
#define COMBO_COLUMN_AMOUNT				6
#define COMBO_COLUMN_BALANCE			7
#define COMBO_COLUMN_HCFA_GROUP_ID		8
// (z.manning, 03/22/2007) - PLID 25305 - Added Location, PayCatID, ProviderID, InputDate columns to batch payment datalist.
#define COMBO_COLUMN_LOCATION			9
#define COMBO_COLUMN_PAY_CAT_ID			10
#define COMBO_COLUMN_PROVIDER_ID		11
#define COMBO_COLUMN_INPUT_DATE			12
#define COMBO_COLUMN_HCFA_GROUP_NAME	13
// (j.jones 2009-06-10 16:17) - PLID 30478 - added IsEOB, which detects if the BatchPaymentID exists in ERemittanceHistoryT
#define COMBO_COLUMN_IS_EOB				14
// (j.jones 2009-06-26 12:59) - PLID 33856 - added original amount
#define COMBO_COLUMN_ORIGINAL_AMT		15
// (j.jones 2014-06-27 09:17) - PLID 62547 - added PayType
#define COMBO_COLUMN_PAY_TYPE			16
// (j.jones 2015-10-08 14:04) - PLID 67309 - added capitation fields
#define COMBO_COLUMN_CAPITATION			17
#define COMBO_COLUMN_SERVICE_DATE_FROM	18
#define COMBO_COLUMN_SERVICE_DATE_TO	19
// (j.jones 2015-10-20 10:38) - PLID 67385 - added capitation locked status
#define COMBO_COLUMN_LOCKED_CAPITATION	20

#define CHARGE_COLUMN_BILL_ID			0
#define CHARGE_COLUMN_CHARGE_ID			1
#define CHARGE_COLUMN_PATIENT_ID		2
#define CHARGE_COLUMN_USERDEFINED_ID	3
#define CHARGE_COLUMN_INSCO_ID			4
#define CHARGE_COLUMN_INSURED_PARTY_ID	5
#define CHARGE_COLUMN_RESP_TYPE_ID		6
#define CHARGE_COLUMN_PAT_NAME			7
#define CHARGE_COLUMN_BILL_DATE			8
#define CHARGE_COLUMN_CHARGE_DATE		9
#define CHARGE_COLUMN_LOCATION_NAME		10	//TES 5/22/2008 - PLID 29719 - Added.
#define CHARGE_COLUMN_PROV_NAME			11
#define CHARGE_COLUMN_INSCO_NAME		12
#define CHARGE_COLUMN_RESP_NAME			13
#define CHARGE_COLUMN_BILL_DESC			14
#define CHARGE_COLUMN_REV_CODE_ID		15
#define CHARGE_COLUMN_ITEM_CODE			16
#define CHARGE_COLUMN_CHARGE_DESC		17
#define CHARGE_COLUMN_BILL_AMT			18
#define CHARGE_COLUMN_CHARGE_AMT		19
#define CHARGE_COLUMN_ALLOWABLE			20		// (b.eyers 2015-10-16) - PLID 67360
#define CHARGE_COLUMN_PAT_AMT			21		// (j.jones 2015-10-15 09:18) - PLID 67366 - added pat. resp, for capitation only
#define CHARGE_COLUMN_INS_AMT			22
#define CHARGE_COLUMN_INS_BALANCE		23
#define CHARGE_COLUMN_COPAY				24		// (b.eyers 2015-10-28) - PLID 67430
#define CHARGE_COLUMN_COPAY_INSURED_PARTY_ID	25	// (j.jones 2015-10-29 16:21) - PLID 67431 - the InsuredPartyID for the resp in the Copay column, -1 for patient
#define CHARGE_COLUMN_EST_BILL_PAYMENT		26	// (j.jones 2012-08-16 08:59) - PLID 52116 - added estimated payment columns
#define CHARGE_COLUMN_EST_CHARGE_PAYMENT	27
#define CHARGE_COLUMN_IS_ALLOWABLE		28		// (b.eyers 2015-10-21) - PLID 67361

using namespace ADODB;
using namespace NXDATALISTLib;

// (r.wilson 8/6/2012) - PLID 51852 - columns for the patient drop down
enum ePatientDropdownCols
{
	pddcID = 0,
	pddcUserDefinedID,
	pddcLastName,
	pddcFirstName,
	pddcMiddleName,
	pddcFullName,
};

/////////////////////////////////////////////////////////////////////////////
// CBatchPayments dialog

// (s.tullis 2014-08-12 16:52) - PLID 63240 - Batch Payments needs a CTableChecker object for PatCombo.
CBatchPayments::CBatchPayments(CWnd* pParent)
	: CNxDialog(CBatchPayments::IDD, pParent), m_patients(NetUtils::PatCombo)
{
	//{{AFX_DATA_INIT(CBatchPayments)
		m_bIsScreenEnabled = TRUE;
		m_SavedRow = 0;
		m_RedrawRefCount = 0;
		m_SavedTopRow = 0;
		m_bFilteredByBill = FALSE;
		// (r.wilson 8/6/2012) - PLID 51852 - Default this to TRUE because now we do a query of the single patient
		//										drop down on the initialization of the dialog
		m_bNeedRequeryPatientCombo = TRUE; 
		m_nLastPatientIDWarned = -1;
		m_bAutoSelectFirstChargeOnRequeryFinished = FALSE;
	//}}AFX_DATA_INIT

	//(j.anspach 06-13-2005 13:41 PLID 16662) - Adding a specific help file reference to Financial -> Batch Payments
	m_strManualLocation = "NexTech_Practice_Manual.chm";
	m_strManualBookmark = "Payments/Insurance_Payments/enter_a_batch_payment.htm";
	// (r.wilson 8/6/2012) - PLID 51852 - 
	m_bDialogInitializing = TRUE;
}


void CBatchPayments::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBatchPayments)
	DDX_Control(pDX, IDC_CHECK_SHOW_ALL_PATIENT_INS_RESPS, m_checkShowAllPatientInsResps);
	DDX_Control(pDX, IDC_BTN_NEW_ADJUSTMENT, m_btnCreateAdjustment);
	DDX_Control(pDX, IDC_BTN_POST_TO_PATIENT, m_btnPostToPatient);
	DDX_Control(pDX, IDC_CHECK_SHOW_ZERO_BALANCE_PAYMENTS, m_checkShowPaysWithZeroBal);
	DDX_Control(pDX, IDC_BTN_FILTER_BY_BILL_ID, m_btnFilterByBillID);
	DDX_Control(pDX, IDC_BTN_VIEW_APPLIED_REFUNDS, m_btnRefundedPays);
	DDX_Control(pDX, IDC_BTN_NEW_REFUND, m_btnCreateRefund);
	DDX_Control(pDX, IDC_BTN_NEW_EOB, m_btnNewEOB);
	DDX_Control(pDX, IDC_DELETE_PAYMENT, m_btnDelPay);
	DDX_Control(pDX, IDC_EDIT_PAYMENT, m_btnEditPay);
	DDX_Control(pDX, IDC_ADD_PAYMENT, m_btnAddPay);
	DDX_Control(pDX, IDC_RADIO_ALL_PATIENTS, m_radioAllPatients);
	DDX_Control(pDX, IDC_RADIO_ALL_PATIENTS_IN_GROUP, m_radioPatientsInGroup);
	DDX_Control(pDX, IDC_RADIO_INDIV_PATIENT, m_radioIndivPatient);
	DDX_Control(pDX, IDC_PAY_AMOUNT, m_nxstaticPayAmount);
	DDX_Control(pDX, IDC_REM_AMOUNT, m_nxstaticRemAmount);
	DDX_Control(pDX, IDC_BTN_UNAPPLY_EOB, m_btnUnapplyEOB);
	DDX_Control(pDX, IDC_ORIGINAL_PAYMENT_AMT_LABEL, m_nxstaticOriginalAmtLabel);
	DDX_Control(pDX, IDC_ORIG_PAY_AMOUNT, m_nxstaticOriginalAmt);
	DDX_Control(pDX, IDC_BTN_GO_TO_PATIENT, m_btnGoToPatient);
	DDX_Control(pDX, IDC_BTN_GO_TO_PATIENT_ID, m_btnGoToPatientID);
	DDX_Control(pDX, IDC_INSURANCE_REVERSAL, m_btnInsuranceReversal);	// (j.armen 2012-05-24 17:33) - PLID 50520 - Added Insurance Reversal button
	DDX_Control(pDX, IDC_RADIO_GROUP_BY_CHARGE, m_radioGroupByCharge);
	DDX_Control(pDX, IDC_RADIO_GROUP_BY_BILL, m_radioGroupByBill);
	DDX_Control(pDX, IDC_RADIO_GROUP_BY_REV_CODE, m_radioGroupByRevenueCode);
	DDX_Control(pDX, IDC_CHARGE_LIST_LABEL, m_nxstaticChargeListLabel);
	DDX_Control(pDX, IDC_BTN_MANAGE_LOCKBOX_PAYMENTS, m_btnLockBox);
	DDX_Control(pDX, IDC_TOTAL_ALLOWABLE_LABEL, m_nxstaticTotalAllowableLabel); // (b.eyers 2015-11-18) - PLID 67384
	DDX_Control(pDX, IDC_TOTAL_ALLOWABLE_AMT, m_nxstaticTotalAllowableAmt); // (b.eyers 2015-11-18) - PLID 67384
	DDX_Control(pDX, IDC_REIMBURSEMENT_RATE_LABEL, m_nxstaticReimbursementRateLabel); // (b.eyers 2015-10-23) - PLID 67384
	DDX_Control(pDX, IDC_REIMBURSEMENT_PERCENTAGE, m_nxstaticReimbursementPercentage); // (b.eyers 2015-10-23) - PLID 67384
	DDX_Control(pDX, IDC_REIMBURSEMENT_INFO, m_icoReimbursementInfo); // (b.eyers 2015-10-23) - PLID 67384
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBatchPayments, CNxDialog)
	ON_BN_CLICKED(IDC_RADIO_ALL_PATIENTS, OnRadioAllPatients)
	ON_BN_CLICKED(IDC_RADIO_INDIV_PATIENT, OnRadioIndivPatient)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_ADD_PAYMENT, OnBtnAddPayment)
	ON_BN_CLICKED(IDC_EDIT_PAYMENT, OnEditPayment)
	ON_BN_CLICKED(IDC_DELETE_PAYMENT, OnDeletePayment)
	ON_BN_CLICKED(IDC_RADIO_ALL_PATIENTS_IN_GROUP, OnRadioAllPatientsInGroup)
	ON_BN_CLICKED(IDC_BTN_NEW_EOB, OnBtnNewEob)
	ON_BN_CLICKED(IDC_BTN_NEW_REFUND, OnBtnNewRefund)
	ON_BN_CLICKED(IDC_BTN_VIEW_APPLIED_REFUNDS, OnBtnViewAppliedRefunds)
	ON_BN_CLICKED(IDC_BTN_FILTER_BY_BILL_ID, OnBtnFilterByBillId)
	ON_BN_CLICKED(IDC_CHECK_SHOW_ZERO_BALANCE_PAYMENTS, OnCheckShowZeroBalancePayments)
	ON_BN_CLICKED(IDC_BTN_POST_TO_PATIENT, OnBtnPostToPatient)
	ON_BN_CLICKED(IDC_BTN_NEW_ADJUSTMENT, OnBtnNewAdjustment)
	ON_BN_CLICKED(IDC_CHECK_SHOW_ALL_PATIENT_INS_RESPS, OnCheckShowAllPatientInsResps)
	ON_MESSAGE(WM_TABLE_CHANGED, OnTableChanged)
	ON_BN_CLICKED(IDC_BTN_UNAPPLY_EOB, OnBtnUnapplyBatchPayment) // (b.eyers 2015-10-14) - PLID 67308 - renamed function
	ON_BN_CLICKED(IDC_BTN_GO_TO_PATIENT, OnBtnGoToPatient)
	ON_BN_CLICKED(IDC_BTN_GO_TO_PATIENT_ID, OnBtnGoToPatientId)
	ON_BN_CLICKED(IDC_INSURANCE_REVERSAL, OnBnClickedInsuranceReversal)	// (j.armen 2012-05-24 17:34) - PLID 50520 - Added Insurance Reversal Button
	ON_WM_CONTEXTMENU()
	ON_BN_CLICKED(IDC_RADIO_GROUP_BY_CHARGE, OnRadioGroupByCharge)
	ON_BN_CLICKED(IDC_RADIO_GROUP_BY_BILL, OnRadioGroupByBill)
	ON_BN_CLICKED(IDC_RADIO_GROUP_BY_REV_CODE, OnRadioGroupByRevCode)
	ON_MESSAGE(WM_TABLE_CHANGED_EX, OnTableChangedEx)
	ON_BN_CLICKED(IDC_BTN_MANAGE_LOCKBOX_PAYMENTS, &CBatchPayments::OnBnClickedBtnManageLockboxPayments)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBatchPayments message handlers

BEGIN_EVENTSINK_MAP(CBatchPayments, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CBatchPayments)
	ON_EVENT(CBatchPayments, IDC_BATCH_PAYMENTS_COMBO, 16 /* SelChosen */, OnSelChosenBatchPaymentsCombo, VTS_I4)
	ON_EVENT(CBatchPayments, IDC_SELECTED_PATIENT_COMBO, 16 /* SelChosen */, OnSelChosenSelectedPatientCombo, VTS_I4)
	ON_EVENT(CBatchPayments, IDC_APPLY_LIST, 3 /* DblClickCell */, OnDblClickCellApplyList, VTS_I4 VTS_I2)
	ON_EVENT(CBatchPayments, IDC_APPLY_LIST, 6 /* RButtonDown */, OnRButtonDownApplyList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CBatchPayments, IDC_APPLY_LIST, 18 /* RequeryFinished */, OnRequeryFinishedApplyList, VTS_I2)
	//}}AFX_EVENTSINK_MAP
	ON_EVENT(CBatchPayments, IDC_APPLY_LIST, 17, OnColumnClickingApplyList, VTS_I2 VTS_PBOOL)
	ON_EVENT(CBatchPayments, IDC_APPLY_LIST, 19, OnLeftClickApplyList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CBatchPayments, IDC_APPLY_LIST, 25, OnFocusGainedApplyList, VTS_NONE)
	ON_EVENT(CBatchPayments, IDC_SELECTED_PATIENT_COMBO, 25, FocusGainedSelectedPatientCombo, VTS_NONE)
	ON_EVENT(CBatchPayments, IDC_SELECTED_PATIENT_COMBO, 23, ChangeColumnSortFinishedSelectedPatientCombo, VTS_I2 VTS_BOOL VTS_I2 VTS_BOOL)
	ON_EVENT(CBatchPayments, IDC_BATCH_PAYMENTS_COMBO, 18 /* RequeryFinished */, OnRequeryFinishedBatchPaymentsCombo, VTS_I2)
	ON_EVENT(CBatchPayments, IDC_APPLY_LIST, 12, OnDragBeginApplyList, VTS_PBOOL VTS_I4 VTS_I2 VTS_I4)
	ON_EVENT(CBatchPayments, IDC_APPLY_LIST, 13, OnDragOverCellApplyList, VTS_PBOOL VTS_I4 VTS_I2 VTS_I4 VTS_I2 VTS_I4)
	ON_EVENT(CBatchPayments, IDC_APPLY_LIST, 14, OnDragEndApplyList, VTS_I4 VTS_I2 VTS_I4 VTS_I2 VTS_I4)	
END_EVENTSINK_MAP()

BOOL CBatchPayments::OnInitDialog() 
{
	try {
		//if (!g_pLicense->CheckForLicense(CLicense::lcERemittance, CLicense::cflrSilent)) {

		// (b.spivey - July 21st, 2014) - PLID 62957 - If we're licensed, we need to size this button down and add the lockbox button to the right. 
		if (!g_pLicense->CheckForLicense(CLicense::lcLockboxPayments, CLicense::cflrSilent)) {
			CRect rcAnchor;
			CWnd& btnRefundedPays = *GetDlgItem(IDC_BTN_VIEW_APPLIED_REFUNDS);
			CWnd& btnLockBox = *GetDlgItem(IDC_BTN_MANAGE_LOCKBOX_PAYMENTS);

			btnLockBox.GetWindowRect(&rcAnchor);
			ScreenToClient(&rcAnchor);

			CRect rcRefund;
			btnRefundedPays.GetWindowRect(&rcRefund);
			ScreenToClient(&rcRefund);

			rcRefund.right = rcAnchor.right;

			btnRefundedPays.MoveWindow(rcRefund);
			btnLockBox.EnableWindow(FALSE);
			btnLockBox.ShowWindow(SW_HIDE);

		}

	} NxCatchAll(__FUNCTION__" - Resize control");
	CNxDialog::OnInitDialog();

	try {

		m_brush.CreateSolidBrush(PaletteColor(GetNxColor(GNC_FINANCIAL, 0)));

		// (j.jones 2010-05-17 13:13) - PLID 33941 - added bulk caching
		g_propManager.CachePropertiesInBulk("CBatchPayments", propNumber,
			"(Username = '<None>' OR Username = '%s') AND Name IN ( \r\n"
			"	'ShowFinancialGridLines', "
			"	'ShowZeroBalanceBatchPays', "
			"	'DefBatchPayClickAction', "
			"	'BatchPayShowPatWarnings', "
			"	'ShowInsuranceReferralWarning', "
			"	'WarnCopays', "	// (j.jones 2010-08-03 10:17) - PLID 39937
			"	'BatchPayUseChargeProviderOnApplies', "	// (j.jones 2011-07-08 14:31) - PLID 18687
			"	'BatchPayUseChargeLocationOnApplies' "	// (j.jones 2011-07-14 16:42) - PLID 18686
			",	'BatchPayment_GoToChargesWhenSelectingPatient' " // (j.jones 2012-08-15 14:04) - PLID 52115
			",  'BatchPaymentsChargeSortColumn' " // (j.jones 2012-08-15 15:23) - PLID 52151 - added two sort prefs.
			",  'BatchPaymentsChargeSortAsc' "
			",	'BatchPayments_ShowEstPaysColumn' "	// (j.jones 2012-08-20 10:04) - PLID 52116
			",	'BatchPayments_GroupChargesBy' "	// (j.jones 2012-08-22 15:19) - PLID 52153
			",  'BatchPayments_PatientComboSortType' " // (r.wilson 8/6/2012) - PLID 51852
			",  'BatchPayments_ShowEstPaysColumn_Secondary' " // (j.jones 2012-08-28 17:07) - PLID 52351
			// (j.jones 2013-07-19 15:38) - PLID 57653 - used in CheckUnbatchCrossoverClaim
			",	'ERemit_UnbatchMA18orNA89_MarkForwardToSecondary' "
			// (b.eyers 2015-10-29) - PLID 67463
			", 'BatchPayments_CapitationModifiers' "
			")"
			, _Q(GetCurrentUserName()));

		// (j.jones 2008-05-07 11:12) - PLID 29854 - set button styles for modernization
		m_btnAddPay.AutoSet(NXB_NEW);
		m_btnEditPay.AutoSet(NXB_MODIFY);
		m_btnDelPay.AutoSet(NXB_DELETE);
		m_btnCreateRefund.AutoSet(NXB_NEW);
		m_btnCreateAdjustment.AutoSet(NXB_NEW);
		m_btnPostToPatient.AutoSet(NXB_MODIFY);
		m_btnFilterByBillID.AutoSet(NXB_INSPECT);
		m_btnNewEOB.AutoSet(NXB_NEW);
		m_btnRefundedPays.AutoSet(NXB_MODIFY);
		// (b.spivey - July 21st, 2014) - PLID 62957 - Set the icon. 
		m_btnLockBox.AutoSet(NXB_MODIFY); 

		// (j.jones 2009-06-10 15:48) - PLID 30478 - added ability to unapply an EOB
		m_btnUnapplyEOB.AutoSet(NXB_DELETE);
		GetDlgItem(IDC_BTN_UNAPPLY_EOB)->EnableWindow(FALSE);

		// (j.jones 2009-06-26 12:59) - PLID 33856 - hide the original amount information by default
		GetDlgItem(IDC_ORIGINAL_PAYMENT_AMT_LABEL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_ORIG_PAY_AMOUNT)->ShowWindow(SW_HIDE);

		// (b.eyers 2015-10-23) - PLID 67384 - hide reimbursement labels when tab opens
		m_nxstaticTotalAllowableLabel.ShowWindow(SW_HIDE);
		m_nxstaticTotalAllowableAmt.ShowWindow(SW_HIDE);
		m_nxstaticReimbursementRateLabel.ShowWindow(SW_HIDE);
		m_nxstaticReimbursementPercentage.ShowWindow(SW_HIDE);
		m_icoReimbursementInfo.LoadToolTipIcon(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDB_QUESTION_MARK), CString(""), true, true, false);
		m_icoReimbursementInfo.ShowWindow(SW_HIDE);

		// (j.jones 2007-06-29 08:53) - PLID 23951 - hide the E-Remittance button
		// if they do not have the E-Remittance license
		// *Note: prior to the E-Remittance license existing, this used to use the Ebilling license
		if(!g_pLicense->CheckForLicense(CLicense::lcERemittance, CLicense::cflrSilent)) {
			// (b.savon 2015-07-07 10:15) - PLID 66484 - Allow lockbox payments to be imported even without the ERemittance license
			if (g_pLicense->CheckForLicense(CLicense::lcLockboxPayments, CLicense::cflrSilent)){
				GetDlgItem(IDC_BTN_NEW_EOB)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_BTN_NEW_EOB)->EnableWindow();
			}
			else{
				GetDlgItem(IDC_BTN_NEW_EOB)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_BTN_NEW_EOB)->EnableWindow(FALSE);
			}

			// (j.jones 2009-06-10 15:52) - PLID 30478 - hide the unapply EOB button as well
			// (b.eyers 2015-10-14) - PLID 67308 - don't need to hide this since normal batch payments can use this button now
			//GetDlgItem(IDC_BTN_UNAPPLY_EOB)->ShowWindow(SW_HIDE);
			//GetDlgItem(IDC_BTN_UNAPPLY_EOB)->EnableWindow(FALSE);
		}
		else {
			// (j.jones 2009-06-10 16:02) - PLID 30478 - If the user doesn't have permission to
			// undo EOB postings, then hide the ability entirely. They would have to get permission
			// and close/reopen the financial module to gain access.
			// (b.eyers 2015-10-14) - PLID 67308 - renamed permission
			if (!(GetCurrentUserPermissions(bioUnapplyBatchPayment) & SPT______0_____ANDPASS)) {
				GetDlgItem(IDC_BTN_UNAPPLY_EOB)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_BTN_UNAPPLY_EOB)->EnableWindow(FALSE);
			}
		}
		
		m_btnAddPay.AutoSet(NXB_NEW);
		m_btnEditPay.AutoSet(NXB_MODIFY);
		m_btnDelPay.AutoSet(NXB_DELETE);
		
		m_BatchPayments = BindNxDataListCtrl(IDC_BATCH_PAYMENTS_COMBO,false);
		m_PatientCombo = BindNxDataListCtrl(IDC_SELECTED_PATIENT_COMBO, false);
		m_ChargeList = BindNxDataListCtrl(IDC_APPLY_LIST,false);

		// (j.jones 2012-04-25 10:53) - PLID 48032 - Supported line item corrections that were takebacks,
		// returning the value of the original payment to the batch payment. Also fixed to ignore
		// payments that were voided (unless part of another batch payment's takeback), and moved the
		// from/where clause from resources to code.
		// (s.tullis 2014-08-12 09:38) - PLID 60686 - Changed EOB flag to be in batchpayments
		// (j.jones 2014-06-27 09:17) - PLID 62547 - added PayType
		// (j.jones 2015-10-08 13:53) - PLID 67309 - added capitation data
		// (j.jones 2015-10-20 10:39) - PLID 67385 - added IsCapitationLocked
		m_BatchPayments->PutFromClause("(SELECT BatchPaymentsT.ID, BatchPaymentsT.Type, BatchPaymentsT.PayType, "
			"InsuranceCoT.PersonID, BatchPaymentsT.Amount, BatchPaymentsT.OriginalAmount, BatchPaymentsT.Description, "
			"BatchPaymentsT.CheckNo, BatchPaymentsT.Date, InsuranceCoT.Name, "
			"Coalesce(LineItemsInUseQ.TotalApplied, Convert(money,0)) AS AppliedAmount, "
			"BatchPaymentsT.Amount "
			" - Coalesce(LineItemsInUseQ.TotalApplied, Convert(money,0)) "
			" + Coalesce(LineItemsReversedQ.TotalReversed, Convert(money,0)) "
			" - Coalesce(AppliedPaymentsT.TotalApplied, Convert(money,0)) "
			" AS RemainingAmount, "
			"BatchPaymentsT.Deleted AS BatchPayDeleted, InsuranceCoT.HCFASetupGroupID, Location, PayCatID, "
			"BatchPaymentsT.ProviderID, BatchPaymentsT.InputDate, HCFASetupT.Name AS HCFAGroupName, "
			"BatchPaymentsT.ERemittance as IsEOB, "
			"BatchPaymentsT.Capitation, BatchPaymentsT.ServiceDateFrom, BatchPaymentsT.ServiceDateTo, "
			"Convert(bit, CASE WHEN BatchPaymentCapitationDetailsQ.BatchPaymentID Is Not Null THEN 1 ELSE 0 END) AS IsCapitationLocked "
			"FROM BatchPaymentsT "
			""
			//find child payments that are not voided, but include them if they are part of a takeback
			"LEFT JOIN (SELECT PaymentsT.BatchPaymentID, Sum(LineItemT.Amount) AS TotalApplied "
			"	FROM LineItemT "
			"	INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			"	LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT WHERE BatchPaymentID Is Null) AS LineItemCorrections_OriginalPaymentQ ON LineItemT.ID = LineItemCorrections_OriginalPaymentQ.OriginalLineItemID "
			"	LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingPaymentQ ON LineItemT.ID = LineItemCorrections_VoidingPaymentQ.VoidingLineItemID "
			"	WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 1 "
			"	AND LineItemCorrections_OriginalPaymentQ.OriginalLineItemID Is Null "
			"	AND LineItemCorrections_VoidingPaymentQ.VoidingLineItemID Is Null "
			"	AND PaymentsT.BatchPaymentID Is Not Null "
			"	GROUP BY PaymentsT.BatchPaymentID "
			") AS LineItemsInUseQ ON BatchPaymentsT.ID = LineItemsInUseQ.BatchPaymentID "
			""
			//find payments that were part of takebacks, crediting this batch payment
			"LEFT JOIN (SELECT LineItemCorrectionsT.BatchPaymentID, Sum(LineItemT.Amount) AS TotalReversed "
			"	FROM LineItemT "
			"	INNER JOIN LineItemCorrectionsT ON LineItemT.ID = LineItemCorrectionsT.OriginalLineItemID "
			"	WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 1 "
			"	AND LineItemCorrectionsT.BatchPaymentID Is Not Null "
			"	GROUP BY LineItemCorrectionsT.BatchPaymentID "
			") AS LineItemsReversedQ ON BatchPaymentsT.ID = LineItemsReversedQ.BatchPaymentID "
			""
			//find the batch payment's adjustments or refunds
			"LEFT JOIN (SELECT Sum(Amount) AS TotalApplied, AppliedBatchPayID "
			"	FROM BatchPaymentsT "
			"	WHERE Type <> 1 AND Deleted = 0 "
			"	GROUP BY AppliedBatchPayID, Deleted "
			") AS AppliedPaymentsT ON BatchPaymentsT.ID = AppliedPaymentsT.AppliedBatchPayID "
			""
			//is this a capitation payment with locked charges?
			"LEFT JOIN (SELECT BatchPaymentID "
			"	FROM BatchPaymentCapitationDetailsT "
			"	GROUP BY BatchPaymentID "
			") AS BatchPaymentCapitationDetailsQ ON BatchPaymentsT.ID = BatchPaymentCapitationDetailsQ.BatchPaymentID "
			""
			"LEFT JOIN InsuranceCoT ON BatchPaymentsT.InsuranceCoID = InsuranceCoT.PersonID "
			"LEFT JOIN HCFASetupT ON InsuranceCoT.HCFASetupGroupID = HCFASetupT.ID "
			""
			"GROUP BY BatchPaymentsT.ID, BatchPaymentsT.Type, BatchPaymentsT.PayType, InsuranceCoT.PersonID, BatchPaymentsT.Amount, "
			"BatchPaymentsT.OriginalAmount, LineItemsInUseQ.TotalApplied,LineItemsReversedQ.TotalReversed, AppliedPaymentsT.TotalApplied, BatchPaymentsT.Description, "
			"BatchPaymentsT.CheckNo, BatchPaymentsT.Date, InsuranceCoT.Name, InsuranceCoT.HCFASetupGroupID, "
			"BatchPaymentsT.Deleted, Location, PayCatID, BatchPaymentsT.ProviderID, BatchPaymentsT.InputDate, "
			"HCFASetupT.Name, BatchPaymentsT.ERemittance, BatchPaymentsT.Capitation, BatchPaymentsT.ServiceDateFrom, BatchPaymentsT.ServiceDateTo, "
			"BatchPaymentCapitationDetailsQ.BatchPaymentID "
			"HAVING BatchPaymentsT.Deleted = 0) AS BatchPaymentsQ");
		m_BatchPayments->WhereClause = "RemainingAmount <> Convert(money,'$0.00') AND Type = 1";

		// (j.jones 2007-05-10 17:57) - PLID 25973 - When I added a tablechecker for patients,
		// I made it so the list didn't requery until we chose to view it, which is uncommon.
		// Similarly, there's no need to load it to begin with, so just set the value that
		// we need to load on demand.
		m_bNeedRequeryPatientCombo = TRUE;

		//m_ChargeList->DragVisible = TRUE;

		//m_ChargeList->PutHighlightVisible(FALSE);

		if(GetRemotePropertyInt("ShowFinancialGridLines",1,0,GetCurrentUserName(),TRUE)==1) {
			m_ChargeList->GridVisible = TRUE;
		}
		else {
			m_ChargeList->GridVisible = FALSE;
		}

		// (j.jones 2012-08-22 15:17) - PLID 52153 - added radio buttons to group by charge/bill/rev. code,
		// it now saves the last setting per user
		long nGroupChargesBy = GetRemotePropertyInt("BatchPayments_GroupChargesBy", 0, 0, GetCurrentUserName(), true);
		//0 - charges (default), 1 - bills, 2 - revenue codes
		m_radioGroupByCharge.SetCheck(nGroupChargesBy != 1 && nGroupChargesBy != 2);
		m_radioGroupByBill.SetCheck(nGroupChargesBy == 1);
		m_radioGroupByRevenueCode.SetCheck(nGroupChargesBy == 2);
		
		//this will fill the charge list from clause, and will also show/hide the estimated payment columns
		OnChargeGroupingChanged(FALSE);

		m_ChargeList->GetColumn(CHARGE_COLUMN_BILL_ID)->PutBackColor(0x0097CCE8);
		m_ChargeList->GetColumn(CHARGE_COLUMN_USERDEFINED_ID)->PutBackColor(0x0097DCE8);
		m_ChargeList->GetColumn(CHARGE_COLUMN_PAT_NAME)->PutBackColor(0x0080EEFF);
		m_ChargeList->GetColumn(CHARGE_COLUMN_CHARGE_DATE)->PutBackColor(0x0080DDDD);
		m_ChargeList->GetColumn(CHARGE_COLUMN_LOCATION_NAME)->PutBackColor(0x0080DDEE);
		m_ChargeList->GetColumn(CHARGE_COLUMN_PROV_NAME)->PutBackColor(0x0080DDEE);
		m_ChargeList->GetColumn(CHARGE_COLUMN_INSCO_NAME)->PutBackColor(0x0080EEEE);
		m_ChargeList->GetColumn(CHARGE_COLUMN_RESP_NAME)->PutBackColor(0x0080EEEF);
		m_ChargeList->GetColumn(CHARGE_COLUMN_ITEM_CODE)->PutBackColor(0x0080EEFF);
		m_ChargeList->GetColumn(CHARGE_COLUMN_BILL_DESC)->PutBackColor(0x0080EEFF);
		m_ChargeList->GetColumn(CHARGE_COLUMN_CHARGE_DESC)->PutBackColor(0x0080EEFF);
		m_ChargeList->GetColumn(CHARGE_COLUMN_BILL_AMT)->PutBackColor(0x0080EEEF);
		m_ChargeList->GetColumn(CHARGE_COLUMN_CHARGE_AMT)->PutBackColor(0x00FFC0C0);
		m_ChargeList->GetColumn(CHARGE_COLUMN_ALLOWABLE)->PutBackColor(0x00C0FFC0); // (b.eyers 2015-10-16) - PLID 67360
		// (j.jones 2015-10-15 09:23) - PLID 67366 - added pat resp column
		m_ChargeList->GetColumn(CHARGE_COLUMN_PAT_AMT)->PutBackColor(0x0080EEFF);
		m_ChargeList->GetColumn(CHARGE_COLUMN_INS_AMT)->PutBackColor(0x00B0B0EE);
		m_ChargeList->GetColumn(CHARGE_COLUMN_INS_BALANCE)->PutBackColor(0x00C0C0FF);
		// (b.eyers 2015-10-28) - PLID 67430
		m_ChargeList->GetColumn(CHARGE_COLUMN_COPAY)->PutBackColor(0x0080EEFF);
		// (j.jones 2012-08-16 09:13) - PLID 52116 - added est. payment columns, they use the same color
		// as the payment column in the Billing tab
		m_ChargeList->GetColumn(CHARGE_COLUMN_EST_BILL_PAYMENT)->PutBackColor(0x00C0FFC0);
		m_ChargeList->GetColumn(CHARGE_COLUMN_EST_CHARGE_PAYMENT)->PutBackColor(0x00C0FFC0);

		m_radioAllPatients.SetCheck(TRUE);
		// (r.wilson 8/6/2012) - PLID 51852 - The patient dropdown now defaults to ENABLED even if it's associated radio button hasn't been clicked yet.
		m_PatientCombo->Enabled = TRUE;
		//m_PatientCombo->PutTextSearchCol(1);
		m_ePatientComboSortType = (PatientComboSortType::SortType) GetRemotePropertyInt("BatchPayments_PatientComboSortType", PatientComboSortType::LastFirstASC, 0, GetCurrentUserName(), true);
		PatientDropdownSetSortCol();

		m_btnPostToPatient.EnableWindow(FALSE);
		// (j.jones 2012-02-16 17:28) - PLID 48137 - the regular go to patient button is tied to the patient dropdown
		m_btnGoToPatient.EnableWindow(FALSE);
		m_checkShowAllPatientInsResps.EnableWindow(FALSE);

		m_checkShowPaysWithZeroBal.SetCheck(GetRemotePropertyInt("ShowZeroBalanceBatchPays",0,0,GetCurrentUserName(),FALSE));
		OnCheckShowZeroBalancePayments();

		// (j.jones 2012-08-15 15:24) - PLID 52151 - added ability to remember the last sort column
		RestoreLastChargeSortOrder();

		// (r.wilson 8/6/2012) - PLID 51852 -
		OnRadioIndivPatient();
		// (s.tullis 2014-06-23 11:21) - PLID 49455 - Permission: Write Batch Payment
		SecureControls();
	
		m_bDialogInitializing = FALSE;
	}NxCatchAll("CBatchPayments::OnInitDialog"); // (z.manning, 03/27/2007) - PLID 25357
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CBatchPayments::OnSelChosenBatchPaymentsCombo(long nRow) 
{
	try {

		CRect rc;

		CWaitCursor pWait;
		
		if(nRow != -1) {

			IRowSettingsPtr pRow = m_BatchPayments->GetRow(nRow);
			long BatchPayID = pRow->GetValue(COMBO_COLUMN_ID).lVal;

			CString strInsCoName = VarString(m_BatchPayments->GetValue(m_BatchPayments->CurSel, COMBO_COLUMN_INSCO_NAME), "<No Name>");

			// (j.jones 2015-10-08 14:15) - PLID 67309 - for capitation payments we want to show a lot more data in the combo box
			bool bIsCapitation = IsCapitationPayment();
			if (bIsCapitation) {
				// (b.eyers 2015-10-27) - PLID 67384 - set rate to --%  and total allowable to < n/a >
				CString strNotCalcPercent = "--%";
				SetDlgItemText(IDC_REIMBURSEMENT_PERCENTAGE, strNotCalcPercent);
				CString strTotalAmt = "< n/a >";
				SetDlgItemText(IDC_TOTAL_ALLOWABLE_AMT, strTotalAmt);
				CString strCapitationComboBoxText;
				//these should never be null on a capitation payment, if so it will just show bad dates
				COleDateTime dtServiceDateFrom = VarDateTime(pRow->GetValue(COMBO_COLUMN_SERVICE_DATE_FROM), COleDateTime::GetCurrentTime());
				COleDateTime dtServiceDateTo = VarDateTime(pRow->GetValue(COMBO_COLUMN_SERVICE_DATE_TO), COleDateTime::GetCurrentTime());
				strCapitationComboBoxText.Format("%s - Capitation Payment for %s - %s", strInsCoName, FormatDateTimeForInterface(dtServiceDateFrom, NULL, dtoDate), FormatDateTimeForInterface(dtServiceDateTo, NULL, dtoDate));
				m_BatchPayments->PutComboBoxText(_bstr_t(strCapitationComboBoxText));

				// (b.eyers 2015-10-23) - PLID 67384 - calculate the reimbursement % for locked payments from saved allowable fees
				if (IsLockedCapitationPayment()) {
					IRowSettingsPtr pBatchPaymentRow = m_BatchPayments->GetRow(m_BatchPayments->CurSel);
					COleCurrency cyBatchPaymentAmount = VarCurrency(pBatchPaymentRow->GetValue(COMBO_COLUMN_AMOUNT), COleCurrency(0, 0));					
					COleCurrency cyTotalAllowable = COleCurrency(0, 0);
					COleCurrency cyTotalCopays = COleCurrency(0, 0);
					_RecordsetPtr rs = CreateParamRecordset("SELECT AllowableUsed, Copay FROM BatchPaymentCapitationDetailsT WHERE BatchPaymentID = {INT}", BatchPayID);
					while (!rs->eof) {
						cyTotalAllowable += AdoFldCurrency(rs, "AllowableUsed", COleCurrency(0, 0));
						cyTotalCopays += AdoFldCurrency(rs, "Copay", COleCurrency(0, 0));
						rs->MoveNext();
					}
					rs->Close();

					RoundCurrency(cyTotalAllowable);
					RoundCurrency(cyTotalCopays);

					// show the total allowable on the screen
					CString strTotalAllowableAmt = FormatCurrencyForInterface(cyTotalAllowable);
					SetDlgItemText(IDC_TOTAL_ALLOWABLE_AMT, strTotalAllowableAmt);

					double dblReimbursementRate = 0.0;

					{
						// (b.eyers 2015-10-27) - PLID 67384 - calculate and show reimbursement rate, update tooltip for icon
						dblReimbursementRate = CalculateReimbursementRate(cyBatchPaymentAmount, cyTotalCopays, cyTotalAllowable);
						dblReimbursementRate = dblReimbursementRate * 100;
						CString strPercent;
						strPercent.Format("%.2f%%", dblReimbursementRate);
						SetDlgItemText(IDC_REIMBURSEMENT_PERCENTAGE, strPercent);
						CString strReimbursementInfo;
						strReimbursementInfo.Format("Batch Payment Amount: %s\r\n"
							"Total Copay: %s\r\n"
							"Total Allowable: %s ",
							FormatCurrencyForInterface(cyBatchPaymentAmount),
							FormatCurrencyForInterface(cyTotalCopays),
							strTotalAllowableAmt);
						m_icoReimbursementInfo.SetToolTip(strReimbursementInfo);
					}
				}
			}

			// (j.jones 2015-10-16 15:24) - PLID 67382 - Added function to show/hide controls for capitation payments.
			UpdateControlsForCapitation(bIsCapitation);

			CString strPay, strRem;

			// (z.manning, 03/22/2007) - PLID 25305 - Don't pull these from data.
			COleCurrency cy = VarCurrency(pRow->GetValue(COMBO_COLUMN_AMOUNT), COleCurrency(0,0));
			strPay = FormatCurrencyForInterface(cy);
			SetDlgItemText(IDC_PAY_AMOUNT,strPay);
			cy = VarCurrency(pRow->GetValue(COMBO_COLUMN_BALANCE), COleCurrency(0,0));
			strRem = FormatCurrencyForInterface(cy);
			SetDlgItemText(IDC_REM_AMOUNT,strRem);

			// (j.jones 2009-06-26 12:59) - PLID 33856 - added original amount information
			_variant_t varOriginal = pRow->GetValue(COMBO_COLUMN_ORIGINAL_AMT);
			if(varOriginal.vt == VT_CY) {
				//if not null, show it
				SetDlgItemText(IDC_ORIG_PAY_AMOUNT, FormatCurrencyForInterface(VarCurrency(varOriginal)));
				GetDlgItem(IDC_ORIGINAL_PAYMENT_AMT_LABEL)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_ORIG_PAY_AMOUNT)->ShowWindow(SW_SHOW);
			}
			else {
				//if null, hide it
				GetDlgItem(IDC_ORIGINAL_PAYMENT_AMT_LABEL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_ORIG_PAY_AMOUNT)->ShowWindow(SW_HIDE);
			}

			CString str;
			str.Format("All patients with balances for this insurance company (%s)", strInsCoName);
			m_radioAllPatients.SetWindowText(str);

			_variant_t vtHcfaGroupName = pRow->GetValue(COMBO_COLUMN_HCFA_GROUP_NAME);
			if(vtHcfaGroupName.vt == VT_BSTR) {
				CString str;
				str.Format("All patients with balances for this insurance company's HCFA group (%s)", VarString(vtHcfaGroupName));
				m_radioPatientsInGroup.SetWindowText(str);
			}
			else {
				m_radioPatientsInGroup.SetWindowText("All patients with balances for this insurance company's HCFA group (<No Group>)");
			}

			ChangePatientSelectionType();

			// (j.jones 2009-06-10 16:18) - PLID 30478 - enable the unapply EOB button if the batch payment
			// is from an EOB - the button will already be hidden if they don't have the license or permissions
			//GetDlgItem(IDC_BTN_UNAPPLY_EOB)->EnableWindow(VarBool(pRow->GetValue(COMBO_COLUMN_IS_EOB)));
			GetDlgItem(IDC_BTN_UNAPPLY_EOB)->EnableWindow(TRUE); // (b.eyers 2015-10-14) - PLID 67308 - this always enables now when any batch payment is selected

			// (j.armen 2012-05-24 17:34) - PLID 50520 - Show insurance reversal button when we have a selected batch payment
			m_btnInsuranceReversal.EnableWindow(TRUE);

			// (j.jones 2014-07-02 15:08) - PLID 62652 - if a vision payment, the revenue code option is not available
			if (IsVisionPayment()) {
				if (m_radioGroupByRevenueCode.GetCheck()) {
					//revert to grouping by charge, which will also remember as their last setting
					m_radioGroupByCharge.SetCheck(TRUE);
					m_radioGroupByBill.SetCheck(FALSE);
					m_radioGroupByRevenueCode.SetCheck(FALSE);
					OnRadioGroupByCharge();
				}
				m_radioGroupByRevenueCode.ShowWindow(SW_HIDE);
			}
			else {
				m_radioGroupByRevenueCode.ShowWindow(SW_SHOW);
			}
			// (b.eyers 2015-10-21) - PLID 67361 - call restorelastchargesortorder, in case of capitatio in use, this will make sure that the capitation sorting isn't saved
			RestoreLastChargeSortOrder();
		}
		else {
			m_ChargeList->Clear();
			if(!m_bIsScreenEnabled) {
				m_RedrawRefCount = m_SavedTopRow = m_SavedRow = 0;
				EnableScreen();
			}
			SetDlgItemText(IDC_PAY_AMOUNT,FormatCurrencyForInterface(COleCurrency(0,0)));
			SetDlgItemText(IDC_REM_AMOUNT,FormatCurrencyForInterface(COleCurrency(0,0)));

			// (j.jones 2015-10-16 15:24) - PLID 67382 - Added function to show/hide controls for capitation payments.
			UpdateControlsForCapitation(false);

			// (j.jones 2009-06-26 12:59) - PLID 33856 - hide the original amount information			
			SetDlgItemText(IDC_ORIG_PAY_AMOUNT,FormatCurrencyForInterface(COleCurrency(0,0)));
			GetDlgItem(IDC_ORIGINAL_PAYMENT_AMT_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_ORIG_PAY_AMOUNT)->ShowWindow(SW_HIDE);

			m_radioAllPatients.SetWindowText("All patients with balances for this insurance company");
			m_radioPatientsInGroup.SetWindowText("All patients with balances for this insurance company's HCFA group");

			// (j.jones 2009-06-10 16:18) - PLID 30478 - disable the unapply EOB button
			GetDlgItem(IDC_BTN_UNAPPLY_EOB)->EnableWindow(FALSE);

			// (j.armen 2012-05-24 17:35) - PLID 50520 - disable the insurance reversal button
			m_btnInsuranceReversal.EnableWindow(FALSE);
		}

		// (s.tullis 2015-10-22 16:28) - PLID 67383 - Users posting capitation batch payments should not see the grouping settings for charge/bill/revenue code. It should always list out each charge.
		OnChargeGroupingChanged(FALSE);

		GetDlgItem(IDC_PAY_AMOUNT)->GetWindowRect(rc);
		ScreenToClient(rc);
		InvalidateRect(rc);

		GetDlgItem(IDC_REM_AMOUNT)->GetWindowRect(rc);
		ScreenToClient(rc);
		InvalidateRect(rc);

		GetDlgItem(IDC_ORIG_PAY_AMOUNT)->GetWindowRect(rc);
		ScreenToClient(rc);
		InvalidateRect(rc);

		GetDlgItem(IDC_ORIGINAL_PAYMENT_AMT_LABEL)->GetWindowRect(rc);
		ScreenToClient(rc);
		InvalidateRect(rc);
		// (s.tullis 2014-06-24 09:20) - PLID 49455 - Permission: Batch payment User Permission to Control Read and Wring in the batch Payment Tab
		SecureControls();
	}NxCatchAll("Error loading charges.");
}

void CBatchPayments::OnRadioAllPatients() 
{
	try {

		ChangePatientSelectionType();
		// (s.tullis 2014-06-24 09:20) - PLID 49455 - Permission: Batch payment User Permission to Control Read and Wring in the batch Payment Tab
		SecureControls();

	}NxCatchAll("CBatchPayments::OnRadioAllPatients"); // (z.manning, 03/27/2007) - PLID 25357
}

void CBatchPayments::OnRadioIndivPatient() 
{
	try {

		// (j.jones 2007-05-10 17:58) - PLID 25973 - requery on demand, when necessary
		if(m_bNeedRequeryPatientCombo) {
			
			//keep the current selection if one exists
			long nPatientID = -1;
			if(m_PatientCombo->CurSel != -1)
				nPatientID = VarLong(m_PatientCombo->GetValue(m_PatientCombo->CurSel, 0), -1);
			
			m_PatientCombo->Requery();

			m_bNeedRequeryPatientCombo = FALSE;

			if(nPatientID != -1) {
				long nRow = m_PatientCombo->SetSelByColumn(0, nPatientID);
				if(nRow == -1 && m_radioIndivPatient.GetCheck()) {
					//the patient doesn't exist anymore and we were
					//currently showing their charges, so clear the list
					m_ChargeList->Clear();
				}
			}

			m_bNeedRequeryPatientCombo = FALSE;
		}

		ChangePatientSelectionType();
		
		// (r.wilson 8/6/2012) - PLID 51852		
		this->GetDlgItem(IDC_SELECTED_PATIENT_COMBO)->SetFocus();
		// (s.tullis 2014-06-24 09:20) - PLID 49455 - Permission: Batch payment User Permission to Control Read and Wring in the batch Payment Tab
		SecureControls();
	}NxCatchAll("CBatchPayments::OnRadioIndivPatient"); // (z.manning, 03/27/2007) - PLID 25357
}

void CBatchPayments::OnRadioAllPatientsInGroup() 
{
	try {

		ChangePatientSelectionType();

	}NxCatchAll("CBatchPayments::OnRadioAllPatientsInGroup"); // (z.manning, 03/27/2007) - PLID 25357
}


HBRUSH CBatchPayments::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	/*
	if(nCtlColor == CTLCOLOR_STATIC) {

		extern CPracticeApp theApp;
		pDC->SelectPalette(&theApp.m_palette, FALSE);
		pDC->RealizePalette();
		pDC->SetBkColor(PaletteColor(0x00D1B8AF));
		return m_brush;
	}
	*/
	
	// (a.walling 2008-04-01 16:47) - PLID 29497 - Deprecated; use parent class' implementation
	return CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}

// (j.jones 2012-08-15 14:06) - PLID 52115 - added ability to auto-select the first charge when requery finishes
void CBatchPayments::ChangePatientSelectionType(BOOL bSelectFirstCharge /*= FALSE*/)
{
	CWaitCursor pWait;

	//remove filter
	m_bFilteredByBill = FALSE;
	m_btnFilterByBillID.SetWindowText("Filter By Bill ID");
	
	// (j.jones 2015-10-21 09:19) - PLID 67416 - if this is a locked capitation payment,
	// these radio buttons should be disabled, and the where clause should not be changed
	if (IsLockedCapitationPayment()) {
		//clear the where clause
		m_ChargeList->WhereClause = "";
		RequeryChargeList();
		return;
	}

	if(m_radioAllPatients.GetCheck()) {

		if(m_BatchPayments->CurSel != -1) {

			CString str;
			str.Format("InsuranceCoID = %li",m_BatchPayments->GetValue(m_BatchPayments->CurSel,COMBO_COLUMN_INSCO_ID).lVal);

			// (j.jones 2014-06-30 11:51) - PLID 62652 - vision payments filter only on vision resp. types
			if (IsVisionPayment()) {
				str += FormatString(" AND RespTypeCategory = %li", rctVision);
			}

			m_ChargeList->WhereClause = _bstr_t(str);

			RequeryChargeList();
		}
		else
			m_ChargeList->Clear();

		// (r.wilson 8/6/2012) - PLID 51852 - Since the Patient Drop down will always be active now I commented out this line that makes it disable
		//m_PatientCombo->Enabled = FALSE;
		m_btnPostToPatient.EnableWindow(FALSE);
		// (j.jones 2012-02-16 17:28) - PLID 48137 - the regular go to patient button is tied to the patient dropdown
		m_btnGoToPatient.EnableWindow(FALSE);
		m_checkShowAllPatientInsResps.EnableWindow(FALSE);
	}
	else if(m_radioPatientsInGroup.GetCheck()) {

		if(m_BatchPayments->CurSel != -1) {

			long HCFASetupGroupID = VarLong(m_BatchPayments->GetValue(m_BatchPayments->CurSel,COMBO_COLUMN_HCFA_GROUP_ID),-1);
			if(HCFASetupGroupID == -1) {
				m_ChargeList->Clear();
				// (r.wilson 8/6/2012) - PLID 51852 - Since the Patient Drop down will always be active now I commented out this line that makes it disable
				//m_PatientCombo->Enabled = FALSE;
				m_btnPostToPatient.EnableWindow(FALSE);
				// (j.jones 2012-02-16 17:28) - PLID 48137 - the regular go to patient button is tied to the patient dropdown
				m_btnGoToPatient.EnableWindow(FALSE);
				m_checkShowAllPatientInsResps.EnableWindow(FALSE);
				return;
			}

			CString str;	
			str.Format("InsuranceCoID IN (SELECT PersonID FROM InsuranceCoT WHERE HCFASetupGroupID = %li)",HCFASetupGroupID);

			// (j.jones 2014-06-30 11:51) - PLID 62652 - vision payments filter only on vision resp. types
			if (IsVisionPayment()) {
				str += FormatString(" AND RespTypeCategory = %li", rctVision);
			}

			m_ChargeList->WhereClause = _bstr_t(str);

			RequeryChargeList();
		}
		else
			m_ChargeList->Clear();

		// (r.wilson 8/6/2012) - PLID 51852 - Since the Patient Drop down will always be active now I commented out this line that makes it disable
		//m_PatientCombo->Enabled = FALSE;
		m_btnPostToPatient.EnableWindow(FALSE);
		// (j.jones 2012-02-16 17:28) - PLID 48137 - the regular go to patient button is tied to the patient dropdown
		m_btnGoToPatient.EnableWindow(FALSE);
		m_checkShowAllPatientInsResps.EnableWindow(FALSE);
	}
	else {

		if(m_BatchPayments->CurSel != -1 && m_PatientCombo->CurSel != -1) {

			CString str;

			long nPatientID = VarLong(m_PatientCombo->GetValue(m_PatientCombo->CurSel,0), -1);
			long nInsCoID = VarLong(m_BatchPayments->GetValue(m_BatchPayments->CurSel,COMBO_COLUMN_INSCO_ID));
			long nHCFASetupGroupID = VarLong(m_BatchPayments->GetValue(m_BatchPayments->CurSel,COMBO_COLUMN_HCFA_GROUP_ID),-1);

			// (j.jones 2007-03-27 14:34) - PLID 23987 - added ability to show
			//all insurance responsibilities for the selected patient
			if(!m_checkShowAllPatientInsResps.GetCheck()) {
				//show only this insurance company's responsibilities

				// (j.jones 2012-08-28 12:50) - PLID 52338 - instead of filtering just on the insurance company,
				// always filter by HCFA group as well, so we can see all potentially relevant charges that may be paid
				str.Format("(InsuranceCoID = %li OR InsuranceCoID IN (SELECT PersonID FROM InsuranceCoT WHERE HCFASetupGroupID = %li)) AND PatientID = %li",
					nInsCoID, nHCFASetupGroupID, nPatientID);
			}
			else {
				// (j.jones 2011-11-02 14:15) - PLID 38686 - this option now also shows patient resps
				//show all responsibilities
				str.Format("PatientID = %li", nPatientID);
			}

			// (j.jones 2014-06-30 11:51) - PLID 62652 - vision payments filter only on vision resp. types
			if (IsVisionPayment()) {
				str += FormatString(" AND RespTypeCategory = %li", rctVision);
			}

			m_ChargeList->WhereClause = _bstr_t(str);

			// (j.jones 2012-08-15 14:06) - PLID 52115 - pass in the bSelectFirstCharge parameter
			RequeryChargeList(bSelectFirstCharge);
		}
		else
			m_ChargeList->Clear();
		// (r.wilson 8/6/2012) - PLID 51852 - Since the Patient Drop down will always be active now I commented this out
		//m_PatientCombo->Enabled = TRUE;		
		m_btnPostToPatient.EnableWindow(TRUE);
		// (j.jones 2012-02-16 17:28) - PLID 48137 - the regular go to patient button is tied to the patient dropdown
		m_btnGoToPatient.EnableWindow(TRUE);
		m_checkShowAllPatientInsResps.EnableWindow(TRUE);

		// (s.tullis 2014-06-24 09:20) - PLID 49455 - Permission: Batch payment User Permission to Control Read and Wring in the batch Payment Tab
		SecureControls();

	}
}

void CBatchPayments::OnSelChosenSelectedPatientCombo(long nRow) 
{
	try {

		CWaitCursor pWait;

		if(m_BatchPayments->CurSel != -1 && nRow != -1) {

			// (j.jones 2012-08-15 14:04) - PLID 52115 - added ability to move focus to the charge list after selecting a patient
			BOOL bSelectFirstCharge = FALSE;
			if(GetRemotePropertyInt("BatchPayment_GoToChargesWhenSelectingPatient", 0, 0, GetCurrentUserName(), true) == 1) {
				bSelectFirstCharge = TRUE;
			}

			// (d.thompson 2009-08-25) - PLID 31179 - We need to respect the status of the "Show all of this patient's responsibilities"
			//	button.  Plus, all the requery code handling that is already written in another function, so let's just call that function.
			ChangePatientSelectionType(bSelectFirstCharge);
		}
		else
			m_ChargeList->Clear();

	}NxCatchAll("CBatchPayments::OnSelChosenSelectedPatientCombo"); // (z.manning, 03/27/2007) - PLID 25357
}

void CBatchPayments::OnDblClickCellApplyList(long nRowIndex, short nColIndex) 
{
	try {

		if(nRowIndex == -1) {
			return;
		}

		m_ChargeList->PutCurSel(nRowIndex);

		// (j.jones 2015-10-16 16:02) - PLID 67382 - double-clicking should do nothing if no
		// batch payment is selected
		IRowSettingsPtr pBatchPaymentRow = m_BatchPayments->GetRow(m_BatchPayments->CurSel);
		if (pBatchPaymentRow == NULL) {
			return;
		}

		// (j.jones 2014-06-27 09:25) - PLID 62457 - vision payments always open line item posting directly, for the bill
		if (IsVisionPayment()) {
			InvokePostingDlg(TRUE);
			return;
		}

		// (j.jones 2012-08-16 09:00) - PLID 52116 - if it's the estimated payment column,
		// it's a hyperlink with a special action, so do not use the default action
		// (this ought to be impossible to hit because OnLButtonDown should have caught these columns)
		if(nColIndex == CHARGE_COLUMN_EST_BILL_PAYMENT) {
			// (j.jones 2012-08-16 09:02) - PLID 52116 - added ability to auto-apply using line item posting,
			// but we won't post if a payment wasn't calculated
			TryAutoPostPayment(TRUE);
			return;
		}
		else if(nColIndex == CHARGE_COLUMN_EST_CHARGE_PAYMENT) {
			// (j.jones 2012-08-16 09:02) - PLID 52116 - added ability to auto-apply using line item posting,
			// but we won't post if a payment wasn't calculated
			TryAutoPostPayment(FALSE);
			return;
		}

		// (j.jones 2015-10-16 16:00) - PLID 67382 - if a capitation payment, dbl. click should always
		// simulate clicking the hyperlink for auto-posting to that charge
		bool bIsCapitation = IsCapitationPayment();
		if (bIsCapitation) {
			TryAutoPostPayment(FALSE);
			return;
		}

		long DefBatchPayClickAction = GetRemotePropertyInt("DefBatchPayClickAction",0,0,"<None>",true);

		// (j.jones 2015-10-16 16:30) - PLID 67382 - returns true if a capitation payment or the 
		// Est. Payment column preference is on, false if a vision payment
		bool bShowEstPaymentColumns = ShowEstPaymentColumns();

		BOOL bIsPatientResp = (m_ChargeList->GetValue(m_ChargeList->CurSel,CHARGE_COLUMN_RESP_TYPE_ID).vt == VT_NULL);

		// (j.jones 2012-08-22 17:26) - PLID 52153 - We have a lot of possible default double-click actions,
		// but we can configure the screen such that some are not possible in certain situations.
		// This function will convert the desired default action into the next best alternative.
		ConvertDefBatchPayClickAction(DefBatchPayClickAction, bShowEstPaymentColumns, bIsPatientResp);

		if(DefBatchPayClickAction == 0)	{		//apply to charge
			ApplyToPatient(FALSE);
		}
		else if(DefBatchPayClickAction == 3) {	//apply to bill
			ApplyToPatient(!bIsPatientResp);
		}
		else if(DefBatchPayClickAction == 1) {	//line item post to charge
			InvokePostingDlg(FALSE);
		}
		// (j.jones 2012-08-20 16:56) - PLID 52116 - added auto-apply options
		else if(DefBatchPayClickAction == 4) {	//auto-apply to charge
			TryAutoPostPayment(FALSE);
		}
		else if(DefBatchPayClickAction == 5) {	//auto-apply to bill
			TryAutoPostPayment(TRUE);
		}
		else //(2)								//line item post to bill
			InvokePostingDlg(TRUE);

	}NxCatchAll("CBatchPayments::OnDblClickCellApplyList"); // (z.manning, 03/27/2007) - PLID 25357
}

void CBatchPayments::OnRButtonDownApplyList(long nRow, short nCol, long x, long y, long nFlags) 
{
	try {

		m_ChargeList->PutCurSel(nRow);

		// (j.jones 2012-08-20 15:58) - PLID 52116 - moved the contents to OnContextMenu

	}NxCatchAll("CBatchPayments::OnRButtonDownApplyList"); // (z.manning, 03/27/2007) - PLID 25357
}

BOOL CBatchPayments::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	if(wParam == ID_APPLY_TO_CHARGE) {
		try {
			ApplyToPatient(FALSE);
		}NxCatchAll("CBatchPayments::OnCommand - ID_APPLY_TO_CHARGE"); // (z.manning, 03/27/2007) - PLID 25357
	}
	else if(wParam == ID_APPLY_TO_BILL) {
		try {
			ApplyToPatient(TRUE);
		}NxCatchAll("CBatchPayments::OnCommand - ID_APPLY_TO_BILL"); // (z.manning, 03/27/2007) - PLID 25357
	}
	else if(wParam == ID_POST_PAYMENT) {
		try {
			if(m_ChargeList->CurSel != -1 && 
				IDYES == MessageBox("This will post a portion of the batch payment to the patient's account, and not apply it to any charge.\n"
				"Are you sure you wish to do this?","Practice",MB_YESNO|MB_ICONQUESTION)) {
				long nPatientID = m_ChargeList->GetValue(m_ChargeList->CurSel,CHARGE_COLUMN_PATIENT_ID).lVal;
				PostToPatient(nPatientID);
			}
		}NxCatchAll("CBatchPayments::OnCommand - ID_POST_PAYMENT"); // (z.manning, 03/27/2007) - PLID 25357
	}
	else if(wParam == ID_CREATE_ADJUSTMENT) {
		try {
			CreateAdjustment();
		}NxCatchAll("CBatchPayments::OnCommand - ID_CREATE_ADJUSTMENT"); // (z.manning, 03/27/2007) - PLID 25357
	}
	else if(wParam == ID_INVOKE_CHARGE_POSTING_DLG) {
		try {
			InvokePostingDlg(FALSE);
		}NxCatchAll("CBatchPayments::OnCommand - ID_INVOKE_CHARGE_POSTING_DLG"); // (z.manning, 03/27/2007) - PLID 25357
	}
	else if(wParam == ID_INVOKE_BILL_POSTING_DLG) {
		try {
			InvokePostingDlg(TRUE);
		}NxCatchAll("CBatchPayments::OnCommand - ID_INVOKE_BILL_POSTING_DLG"); // (z.manning, 03/27/2007) - PLID 25357
	}
	else if(wParam == ID_PRINT_STATEMENT) {
		try {
			PreviewStatement();
		}NxCatchAll("CBatchPayments::OnCommand - ID_PRINT_STATEMENT"); // (z.manning, 03/27/2007) - PLID 25357
	}
	else if(wParam == ID_GOTO_PATIENT) {// (a.vengrofski 2010-05-04 12:20) - PLID <36205>
		try {
			// (a.vengrofski 2010-05-04 12:20) - PLID <36205>
			// (j.jones 2012-02-16 17:23) - PLID 48137 - this now takes in a patient ID
			GotoPatient(VarLong(m_ChargeList->GetValue(m_ChargeList->GetCurSel(), CHARGE_COLUMN_PATIENT_ID),(long)-1));
		}NxCatchAll("CBatchPayments::OnCommand - ID_GOTO_PATIENT"); // (a.vengrofski 2010-05-04 12:20) - PLID <36205>
	}
	// (j.jones 2012-08-20 16:38) - PLID 52116 - added auto-apply options
	else if(wParam == ID_AUTO_APPLY_TO_BILL) {
		try {
			TryAutoPostPayment(TRUE);
		}NxCatchAll("CBatchPayments::OnCommand - ID_AUTO_APPLY_TO_BILL"); // (a.vengrofski 2010-05-04 12:20) - PLID <36205>
	}
	else if(wParam == ID_AUTO_APPLY_TO_CHARGE) {
		try {
			TryAutoPostPayment(FALSE);
		}NxCatchAll("CBatchPayments::OnCommand - ID_AUTO_APPLY_TO_CHARGE"); // (a.vengrofski 2010-05-04 12:20) - PLID <36205>
	}
	
	return CNxDialog::OnCommand(wParam, lParam);
}

// (a.vengrofski 2010-05-04 12:20) - PLID <36205> - Function to goto the selected patient.
// (j.jones 2012-02-16 17:23) - PLID 48137 - this now takes in a patient ID
void CBatchPayments::GotoPatient(long nPatientID)
{
	try {

		if(nPatientID == -1) {
			//do nothing
			return;
		}

		//Set the active patient
		CMainFrame *pMainFrame;
		pMainFrame = GetMainFrame();
		if(pMainFrame != NULL) {

			if(!pMainFrame->m_patToolBar.DoesPatientExistInList(nPatientID)) {
				if(IDNO == MessageBox("This patient is not in the current lookup. \n"
					"Do you wish to reset the lookup to include all patients?","Practice",MB_ICONQUESTION|MB_YESNO)) {
					return;
				}
			}
			
			if(pMainFrame->m_patToolBar.TrySetActivePatientID(nPatientID)) {

				//Now just flip to the patient's module and set the active Patient
				pMainFrame->FlipToModule(PATIENT_MODULE_NAME);

				CNxTabView *pView = pMainFrame->GetActiveView();
				if(pView) {
					// (j.jones 2012-02-16 17:24) - PLID 48137 - switch to the billing tab, if possible
					if((GetCurrentUserPermissions(bioPatientBilling) & SPT__R________) && g_pLicense->CheckForLicense(CLicense::lcBill, CLicense::cflrSilent)) {
						pView->SetActiveTab(PatientsModule::BillingTab);
					}
					pView->UpdateView();
				}
			}
		}//end if MainFrame
		else {
			MsgBox(MB_ICONSTOP|MB_OK, "BatchPayments.cpp: Cannot Open Mainframe");
		}//end else pMainFrame	

	}NxCatchAll(__FUNCTION__);
}
void CBatchPayments::PreviewStatement()
{
	try {

		if(m_ChargeList->CurSel == -1)
			return;

		// (j.jones 2010-05-17 13:22) - PLID 33941 - try to prompt the patient warning first
		long nPatientID = VarLong(m_ChargeList->GetValue(m_ChargeList->CurSel,CHARGE_COLUMN_PATIENT_ID));
		PromptPatientWarning(nPatientID);

		CGoStatements dlg(this);
		dlg.m_nPatientID = nPatientID;
		dlg.DoModal();

	}NxCatchAll(__FUNCTION__);
}

void CBatchPayments::ApplyToPatient(BOOL bApplyToWholeBill)
{
	if(m_BatchPayments->CurSel == -1 || m_ChargeList->CurSel == -1)
		return;

	CWaitCursor pWait;

	try {

		// (j.jones 2008-04-30 14:15) - PLID 28358 - added permission check
		// (j.jones 2011-08-24 17:39) - PLID 45176 - do not call CanApplyLineItem here,
		// because the line item in question has not actually been created, so we only
		// need to check the normal apply permission
		// (s.tullis 2014-06-23 17:46) - PLID 49455 - Permission: Batch 
		if (!CheckCurrentUserPermissions(bioBatchPayment, sptWrite)){
			return;
		}else if(!CheckCurrentUserPermissions(bioApplies, sptCreate)) {
			return;
		}


		
		// (j.jones 2012-08-22 17:34) - PLID 52153 - if grouping by bill, and this is called for a charge,
		// ASSERT, because this functionality should be disabled
		if(!IsCapitationPayment() && m_radioGroupByBill.GetCheck() && !bApplyToWholeBill) {
			//we shouldn't have provided the option to apply to just one charge,
			//find out how it happened, and stop it
			ASSERT(FALSE);
			//change to apply to the bill
			bApplyToWholeBill = TRUE;
		}

		IRowSettingsPtr pBatchPaymentRow = m_BatchPayments->GetRow(m_BatchPayments->CurSel);
		COleDateTime dt = AsDateNoTime(VarDateTime(pBatchPaymentRow->GetValue(COMBO_COLUMN_DATE)));

		// (j.jones 2015-11-05 08:55) - PLID 65567 - check whether or not the user
		// is allowed to enter backdated payments
		if (!CanChangeHistoricFinancial_ByServiceDate("Payment", dt, TRUE)) {
			//the user will have already been told they can't save this payment
			return;
		}

		// (j.jones 2010-05-17 13:22) - PLID 33941 - try to prompt the patient warning first
		long nPatientID = VarLong(m_ChargeList->GetValue(m_ChargeList->CurSel,CHARGE_COLUMN_PATIENT_ID));
		PromptPatientWarning(nPatientID);

		// (j.jones 2006-12-28 10:09) - PLID 23160 - store the setting for grouped by rev. code
		BOOL bApplyToRevCode = m_radioGroupByRevenueCode.GetCheck() ? TRUE : FALSE;
		long nRevCodeID = VarLong(m_ChargeList->GetValue(m_ChargeList->GetCurSel(),CHARGE_COLUMN_REV_CODE_ID), -1);

		// (j.jones 2006-04-10 16:14) - PLID 19962 - added ability to apply to the whole bill,
		// instead of just one charge at a time

		long nChargeListCurSel = m_ChargeList->CurSel;
		
		long BatchPayID = pBatchPaymentRow->GetValue(COMBO_COLUMN_ID).lVal;
		long BillID = m_ChargeList->GetValue(m_ChargeList->GetCurSel(),CHARGE_COLUMN_BILL_ID).lVal;
		long ChargeID = m_ChargeList->GetValue(m_ChargeList->GetCurSel(),CHARGE_COLUMN_CHARGE_ID).lVal;
		// (j.jones 2011-11-02 14:22) - PLID 38686 - these can now be null, use 0 for RespTypeID to not be confused with inactive
		long nInsCoID = VarLong(m_ChargeList->GetValue(m_ChargeList->CurSel,CHARGE_COLUMN_INSCO_ID), -1);
		long nRespTypeID = VarLong(m_ChargeList->GetValue(m_ChargeList->CurSel,CHARGE_COLUMN_RESP_TYPE_ID), 0);
		long nInsuredPartyID = VarLong(m_ChargeList->GetValue(m_ChargeList->CurSel,CHARGE_COLUMN_INSURED_PARTY_ID), -1);

		// (j.jones 2011-11-02 14:37) - PLID 38686 - If RespTypeID is null, then this is patient resp.
		BOOL bIsPatientResp = (m_ChargeList->GetValue(m_ChargeList->CurSel,CHARGE_COLUMN_RESP_TYPE_ID).vt == VT_NULL);
		long nInsuredPartyIDToShiftTo = -1;
		long nRespTypeIDToShiftTo = -2;	//-2 is invalid
		if(bIsPatientResp) {
			// If patient resp., we cannot "Apply To Bill", because it does not have the
			// "increase ins. balance" functionality.
			if(bApplyToWholeBill) {
				//While it's ok to get here and set bApplyToWholeBill to FALSE,
				//this should really be impossible to get to, as we do not want to give them this
				//ability in the interface if it's not allowed (Tom's principle of least taunting).

				//If you get here, find out how, and disable that ability when patient resp.
				ASSERT(FALSE);
				bApplyToWholeBill = FALSE;
			}

			//the problem we face now is that we do not have an insured party in which to actually post to,
			//so we have to pick the highest ranking insured party that matches the batch payment company,
			//or is in the batch payment company's HCFA group
			long nBatchPayInsCoID = VarLong(m_BatchPayments->GetValue(m_BatchPayments->CurSel,COMBO_COLUMN_INSCO_ID));
			long nBatchPayInsCoGroup = VarLong(m_BatchPayments->GetValue(m_BatchPayments->CurSel,COMBO_COLUMN_HCFA_GROUP_ID),-1);
			
			//this query puts inactive resps. last
			_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 InsuredPartyT.PersonID AS InsuredPartyID, "
				"RespTypeT.ID AS RespTypeID "
				"FROM InsuranceCoT "
				"INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID "
				"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				"WHERE InsuredPartyT.PatientID = {INT} "
				"AND (InsuranceCoT.PersonID = {INT} OR InsuranceCoT.HCFASetupGroupID = {INT}) "
				"ORDER BY (CASE WHEN RespTypeT.Priority = -1 THEN 1 ELSE 0 END) ASC, RespTypeT.Priority ASC",
				nPatientID, nBatchPayInsCoID, nBatchPayInsCoGroup);
			if(!rs->eof) {
				nInsuredPartyIDToShiftTo = VarLong(rs->Fields->Item["InsuredPartyID"]->Value);
				nRespTypeIDToShiftTo = VarLong(rs->Fields->Item["RespTypeID"]->Value);
			}
			rs->Close();

			//if -1, we could not determine an insured party to use, so they cannot post to a patient resp.
			if(nInsuredPartyIDToShiftTo == -1) {
				AfxMessageBox("Practice cannot post to this patient responsibility charge because this patient does not "
					"have an insurance company that is in the same HCFA Group as the company this batch payment is for.\n\n"
					"You will need to manually shift the patient responsibility to the desired insured party "
					"before you can post to this charge.");
				GetDlgItem(IDC_APPLY_LIST)->SetFocus();
				return;
			}
		}

		COleCurrency cyRespBalance = COleCurrency(0,0), cyPayBalance = COleCurrency(0,0);

		if(!bApplyToWholeBill) {
			
			//if applying to just the charge, grab the insurance balance from the datalist

			_variant_t var = m_ChargeList->GetValue(m_ChargeList->GetCurSel(),CHARGE_COLUMN_INS_BALANCE);

			if(var.vt == VT_CY)
				cyRespBalance = var.cyVal;
			else
				cyRespBalance = COleCurrency(0,0);
		}
		else {

			// (j.jones 2011-11-02 14:45) - PLID 38686 - throw an exception if we have an invalid ins. co.
			if(nInsCoID == -1) {
				ThrowNxException("ApplyToPatient failed: bApplyToWholeBill set to TRUE for a patient resp charge!");
			}

			//if appling to the entire bill, calculate the insurance balance for that bill

			// (j.jones 2007-02-28 11:30) - PLID 24997 - fixed bug where we did not filter by Responsibility type
			// (j.jones 2007-09-04 09:36) - PLID 27283 - fixed problem where we filtered on the batch payment InsCoID,
			// not the InsCoID from the responsibility of the charge we clicked on
			// (j.jones 2011-08-17 11:17) - PLID 44889 - ignore "original" and "void" charges
			_RecordsetPtr rs = CreateParamRecordset("SELECT Sum(ChargeRespT.Amount) - Sum(Coalesce(AppliesT.SumOfAmount, 0)) AS InsBalance "
				"FROM BillsT "
				"INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"INNER JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
				"INNER JOIN InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID "
				"INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
				"LEFT JOIN (SELECT SUM(Amount) AS SumOfAmount, RespID FROM AppliesT GROUP BY RespID) AS AppliesT ON ChargeRespT.ID = AppliesT.RespID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID " 
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID " 
				"WHERE BillsT.Deleted = 0 AND LineItemT.Deleted = 0 "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null " 
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null " 
				"AND InsuranceCoID = {INT} AND BillsT.ID = {INT} "
				"AND RespTypeID = {INT}",
				nInsCoID, BillID, nRespTypeID);

			if(!rs->eof) {
				cyRespBalance = AdoFldCurrency(rs, "InsBalance", COleCurrency(0,0));
			}
			rs->Close();
		}

		// (z.manning, 03/22/2007) - PLID 25305 - Pull this from the datalist rather than recalculating it.
		cyPayBalance = VarCurrency(pBatchPaymentRow->GetValue(COMBO_COLUMN_BALANCE), COleCurrency(0,0));
		
		CFinancialApply dlg(this);
		dlg.m_PatientID = nPatientID;
		dlg.m_cyNetCharges = cyRespBalance;
		// (j.jones 2011-11-02 14:47) - PLID 38686 - handle applying to patient resp,
		// the m_boShowIncreaseCheck option will shift to insurance
		if(bIsPatientResp) {
			dlg.m_nResponsibility = nRespTypeIDToShiftTo;
			dlg.m_boShowIncreaseCheck = TRUE;
			dlg.m_boIncreaseInsBalance = TRUE;
		}
		else {
			dlg.m_nResponsibility = nRespTypeID;
			dlg.m_boShowIncreaseCheck = FALSE;
			dlg.m_boIncreaseInsBalance = FALSE;
		}
		dlg.m_boShowAdjCheck = TRUE;
		// (j.jones 2011-11-02 16:40) - PLID 38686 - it makes no sense to blindly
		// default the payment amount to the batch payment balance, you'd never
		// apply all of it to one patient unless it was the last payment applied
		COleCurrency cyMaxToApply = cyPayBalance;
		if(cyMaxToApply > cyRespBalance) {
			cyMaxToApply = cyRespBalance;
		}
		dlg.m_cyNetPayment = cyMaxToApply;
		dlg.m_boZeroAmountAllowed = TRUE;

		if(dlg.DoModal() == IDCANCEL) {
			//try to set the focus back to the datalist
			GetDlgItem(IDC_APPLY_LIST)->SetFocus();
			return;
		}

		// (j.jones 2011-11-02 14:47) - PLID 38686 - if they tried to increase the balance,
		// shift to insurance now
		if(bIsPatientResp) {
			//they may have unchecked this box, if so we will leave the payment unapplied
			if(dlg.m_boIncreaseInsBalance) {
				if(!IncreaseInsBalance(ChargeID, nPatientID, nInsuredPartyIDToShiftTo, dlg.m_cyApplyAmount)) {
					//try to set the focus back to the datalist
					GetDlgItem(IDC_APPLY_LIST)->SetFocus();
					return;
				}
			}

			//the rest of this dialog now has an insured party to use,
			//so fill our variables accordingly
			nInsuredPartyID = nInsuredPartyIDToShiftTo;
			nRespTypeID = nRespTypeIDToShiftTo;
		}

		if(nInsuredPartyID == -1) {
			// (j.jones 2011-11-02 15:17) - PLID 38686 - something went horribly wrong
			ThrowNxException("ApplyToPatient failed: nInsuredPartyID == -1, bPatientResp = %li", bIsPatientResp ? 1 : 0);
		}

		COleCurrency cyApplyAmount = dlg.m_cyApplyAmount;		

		//////////////
		//make payment

		long nLocationID, PaymentGroupID, ProviderID;
		CString strDescription, strCheckNo;

		// (z.manning, 03/22/2007) - PLID 25305 - Pull these from the datalist instead of the database.
		nLocationID = VarLong(pBatchPaymentRow->GetValue(COMBO_COLUMN_LOCATION));
		PaymentGroupID = VarLong(pBatchPaymentRow->GetValue(COMBO_COLUMN_PAY_CAT_ID), -1);
		strDescription = VarString(pBatchPaymentRow->GetValue(COMBO_COLUMN_DESCRIPTION), "(no description)");
		ProviderID = VarLong(pBatchPaymentRow->GetValue(COMBO_COLUMN_PROVIDER_ID), -1);
		strCheckNo = VarString(pBatchPaymentRow->GetValue(COMBO_COLUMN_CHECK_NO), "");


		// (j.dinatale 2012-02-06 16:00) - PLID 51181 - while we are at it, may as well parameterize
		long nProviderID = ProviderID;
		
		// (j.jones 2005-02-02 10:48) - PLID 14862 - if no provider is selected on the 
		// batch payment, attempt to assign the charge provider to the child payment
		// (j.jones 2011-07-08 14:37) - PLID 18687 - now we have a preference to always use the charge provider
		// if we are applying to a charge
		// (d.thompson 2012-08-07) - PLID 51969 - Changed default to Yes
		if(ChargeID != -1 && (ProviderID == -1 || GetRemotePropertyInt("BatchPayUseChargeProviderOnApplies", 1, 0, "<None>", true) == 1)) {
			_RecordsetPtr rsDoc = CreateParamRecordset("SELECT DoctorsProviders FROM ChargesT WHERE ID = {INT}",ChargeID);
			if(!rsDoc->eof) {
				long nProvID = AdoFldLong(rsDoc, "DoctorsProviders",-1);
				if(nProvID > -1) {
					nProviderID = nProvID;
				}
			}
			rsDoc->Close();
		}

		// (j.jones 2011-07-14 16:43) - PLID 18686 - added a preference to always use the charge location
		// if we are applying to a charge
		// (d.thompson 2012-08-07) - PLID 51969 - Changed default to Yes
		if(ChargeID != -1 && GetRemotePropertyInt("BatchPayUseChargeLocationOnApplies", 1, 0, "<None>", true) == 1) {
			_RecordsetPtr rsLoc = CreateParamRecordset("SELECT LocationID FROM LineItemT WHERE ID = {INT}", ChargeID);
			if(!rsLoc->eof) {
				nLocationID = AdoFldLong(rsLoc, "LocationID");
			}
			rsLoc->Close();
		}

		CSqlFragment sqlFrag(
			"SET NOCOUNT ON\r\n"
			"DECLARE @nLineItemID INT\r\n");

		// (j.jones 2011-09-29 14:55) - PLID 45500 - ensured that InputDate uses GetDate(), NOT BatchPaymentsT.InputDate
		// (j.armen 2013-06-28 12:47) - PLID 57373 - Idenitate LineItemT
		sqlFrag += CSqlFragment(
			"INSERT INTO LineItemT (\r\n"
			"	PatientID, LocationID, Type, Date, InputDate, InputName, Amount, Description\r\n"
			") VALUES (\r\n"
			"	{INT}, {INT}, 1, {OLEDATETIME}, GetDate(), {STRING}, {OLECURRENCY}, {STRING}) \r\n", 
			nPatientID, nLocationID, dt, GetCurrentUserName(), cyApplyAmount, strDescription);

		sqlFrag += CSqlFragment("SET @nLineItemID = SCOPE_IDENTITY() \r\n");

		// (j.armen 2013-06-29 15:34) - PLID 57375 - PaymentsT.PaymentUniqueID now gets it's ID from an identity seeded table
		sqlFrag += 
			"DECLARE @nPaymentUniqueID INT\r\n"
			"INSERT INTO PaymentUniqueT DEFAULT VALUES\r\n"
			"SET @nPaymentUniqueID = SCOPE_IDENTITY()\r\n";

		// (j.jones 2007-03-28 10:57) - PLID 25385 - supported CashReceived
		sqlFrag += CSqlFragment("INSERT INTO PaymentsT (ID, InsuredPartyID, ProviderID, PaymentGroupID, PayMethod, BatchPaymentID, PaymentUniqueID, CashReceived) VALUES "
			"(@nLineItemID, {INT}, {VT_I4}, {INT}, 2, {INT}, @nPaymentUniqueID, {OLECURRENCY}) \r\n",
			nInsuredPartyID, ((nProviderID == -1) ? g_cvarNull : _variant_t(nProviderID)), PaymentGroupID, BatchPayID, cyApplyAmount);

		//(e.lally 2006-08-10) PLID 21907 - We need add the _Q to check no.. We should also put these queries into a batch in case one fails.
		sqlFrag += CSqlFragment(
			"INSERT INTO PaymentPlansT (ID, CheckNo) VALUES (@nLineItemID, {STRING}) \r\n", strCheckNo);

		// (j.dinatale 2012-02-06 13:15) - PLID 51181 - get the line item ID that we just inserted
		sqlFrag += 
			"SELECT @nLineItemID AS LineItemID \r\n"
			"SET NOCOUNT OFF \r\n\r\n";
 
		_RecordsetPtr rsLineItem = CreateParamRecordset(sqlFrag);

		long iLineItemID = -1;
		if(!rsLineItem->eof){
			iLineItemID = AdoFldLong(rsLineItem, "LineItemID");
		}

		long nAuditID = BeginNewAuditEvent();
		CString strAuditDesc;
		strAuditDesc.Format("%s Payment", FormatCurrencyForInterface(cyApplyAmount, TRUE, TRUE));
		AuditEvent(nPatientID, GetExistingPatientName(nPatientID), nAuditID, aeiPaymentCreated, iLineItemID, "", strAuditDesc, aepHigh, aetCreated);

		AutoUpdateBatchPaymentDepositDates(BatchPayID);

		//////////////////////////

		///////////////
		//apply payment

		// (j.jones 2011-03-23 10:56) - PLID 42936 - disabled these functions from checking the allowable,
		// we will do that later in this function

		if(!bApplyToWholeBill) {

			// (j.jones 2006-12-28 10:08) - PLID 23160 - supported applying by revenue code
			if(bApplyToRevCode && nRevCodeID != -1) {
				//apply to all charges with this revenue code
				// (j.jones 2010-06-02 17:44) - PLID 37200 - renamed to not be insurance-specific
				ApplyPayToBillWithRevenueCode(iLineItemID, nPatientID, cyApplyAmount, BillID, nRevCodeID, nInsuredPartyID, nInsuredPartyID, dlg.m_boShiftBalance, dlg.m_boAdjustBalance, FALSE, FALSE, FALSE, TRUE);
			}
			else {
				//apply only to the charge
				AutoApplyPayToBill(iLineItemID, nPatientID,"Charge",ChargeID, dlg.m_boShiftBalance, dlg.m_boAdjustBalance, FALSE, TRUE, TRUE, FALSE);
			}
		}
		else {
			//apply to the whole bill
			AutoApplyPayToBill(iLineItemID, nPatientID,"Bill",BillID, dlg.m_boShiftBalance, dlg.m_boAdjustBalance, FALSE, TRUE, TRUE, FALSE);
		}

		//////////////////////////

		if (dlg.m_boAdjustBalance) {
			CPaymentDlg paydlg(this);
			paydlg.m_iDefaultPaymentType = 1;
			if(!bApplyToWholeBill) {
				//adjust charge
				if(bApplyToRevCode && nRevCodeID != -1) {
					paydlg.m_cyFinalAmount = AdjustBalance(nRevCodeID, BillID, nPatientID, 3, nRespTypeID, nInsuredPartyID);
					paydlg.m_varBillID = BillID;
				}
				else {
					paydlg.m_cyFinalAmount = AdjustBalance(ChargeID, BillID, nPatientID, 2, nRespTypeID, nInsuredPartyID);
					paydlg.m_varChargeID = ChargeID;
				}
			}
			else {
				//adjust bill
				paydlg.m_cyFinalAmount = AdjustBalance(BillID, BillID, nPatientID, 1, nRespTypeID, nInsuredPartyID);
				paydlg.m_varBillID = BillID;
			}			
			paydlg.m_PatientID = nPatientID;
			paydlg.m_ApplyOnOK = !(bApplyToRevCode && nRevCodeID != -1);
			paydlg.m_PromptToShift = FALSE;
			paydlg.m_iDefaultInsuranceCo = nInsuredPartyID;
			GetMainFrame()->DisableHotKeys();
			// (j.jones 2008-05-01 11:26) - PLID 29287 - the payment dialog
			// can return more than just IDOK, we can't assume that IDOK is 
			// the only result that indicates success
			if (paydlg.DoModal(__FUNCTION__, __LINE__) != IDCANCEL && bApplyToRevCode && nRevCodeID != -1) {
				//if a revenue code, handle the apply ourselves
				// (j.jones 2010-06-02 17:44) - PLID 37200 - renamed to not be insurance-specific
				ApplyPayToBillWithRevenueCode(VarLong(paydlg.m_varPaymentID), nPatientID, paydlg.m_cyFinalAmount, BillID, nRevCodeID, nInsuredPartyID, nInsuredPartyID, dlg.m_boShiftBalance, dlg.m_boAdjustBalance, FALSE, TRUE, FALSE, TRUE);
			}
			GetMainFrame()->EnableHotKeys();
		}

		// See if we need to shift the remaining balance to the 
		// patient in the case of an insurance apply.
		//JMJ - 7/24/2003 - only do this if there is still a balance

		if(!bApplyToWholeBill) {		
			//shift charge

			// (j.jones 2013-08-21 09:00) - PLID 58194 - added a descriptive audit string to ShiftInsBalance

			// (j.jones 2006-12-28 13:54) - PLID 23160 - supported revenue code grouping
			if(bApplyToRevCode && nRevCodeID != -1) {
				//revenue code
				if (dlg.m_boShiftBalance && -AdjustBalance(nRevCodeID, BillID, nPatientID, 3, nRespTypeID, nInsuredPartyID) > COleCurrency(0,0)) {
					try {
						ShiftInsBalance(BillID, nPatientID, nInsuredPartyID, GetInsuranceIDFromType(nPatientID, dlg.m_nShiftToResp), "RevCode",
							"after posting a batch payment",
							true, nRevCodeID);
					}NxCatchAll("Error in ShiftInsBalance");
				}
			}
			else {
				//normal charge
				if (dlg.m_boShiftBalance && -AdjustBalance(ChargeID, BillID, nPatientID, 2, nRespTypeID, nInsuredPartyID) > COleCurrency(0,0)) {
					try {
						ShiftInsBalance(ChargeID, nPatientID, nInsuredPartyID, GetInsuranceIDFromType(nPatientID, dlg.m_nShiftToResp), "Charge",
							"after posting a batch payment");
					}NxCatchAll("Error in ShiftInsBalance");
				}
			}
		}
		else {
			//shift bill
			if (dlg.m_boShiftBalance && -AdjustBalance(BillID, BillID, nPatientID, 1, nRespTypeID, nInsuredPartyID) > COleCurrency(0,0)) {
				try {
					ShiftInsBalance(BillID, nPatientID, nInsuredPartyID, GetInsuranceIDFromType(nPatientID, dlg.m_nShiftToResp), "Bill",
						"after posting a batch payment");
				}NxCatchAll("Error in ShiftInsBalance");
			}
		}

		CheckUnbatchClaim(BillID);

		// (j.jones 2013-07-22 10:14) - PLID 57653 - If they have configured insurance companies
		// to force unbatching due to primary crossover to secondary, force unbatching now.
		// This needs to be after shifting/batching has occurred in the normal posting flow.
		if(nInsuredPartyID != -1) {
			//This ConfigRT name is misleading, it actually just means that if we do unbatch a crossed over claim,
			//claim history will only include batched charges. If false, then claim history includes all charges.
			bool bBatchedChargesOnlyInClaimHistory = (GetRemotePropertyInt("ERemit_UnbatchMA18orNA89_MarkForwardToSecondary", 1, 0, "<None>", true) == 1);

			//This function assumes that the bill's current insured party ID is now the "secondary" insured party
			//we crossed over to, and the insured party who paid was primary.
			//If the payer really was the patient's Primary, and crossing over is enabled, the bill will be unbatched.
			CheckUnbatchCrossoverClaim(nPatientID, BillID, nInsuredPartyID, dt,
				bBatchedChargesOnlyInClaimHistory, aeiClaimBatchStatusChangedByManualCrossover, "Batched", "Unbatched due to manual Primary/Secondary crossover");
		}

		// (j.jones 2011-03-23 10:57) - PLID 42936 - now we have to check the allowables for what we applied
		// (j.jones 2011-08-17 11:17) - PLID 44889 - ignore "original" and "void" charges
		_RecordsetPtr rsAppliedTo = CreateParamRecordset("SELECT ChargesT.ServiceID, InsuredPartyT.InsuranceCoID, "
			"ChargesT.DoctorsProviders, LineItemT.LocationID, BillsT.Location AS POSID, "
			"InsuredPartyT.PersonID, ChargesT.ID, AppliesT.Amount "
			"FROM LineItemT "
			"INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
			"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
			"INNER JOIN AppliesT ON ChargesT.ID = AppliesT.DestID "
			"INNER JOIN ChargeRespT ON AppliesT.RespID = ChargeRespT.ID "
			"INNER JOIN InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID "
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID " 
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID " 
			"WHERE LineItemT.Deleted = 0 "
			"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null " 
			"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null " 
			"AND AppliesT.SourceID = {INT}", iLineItemID);
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

		//check to see if this payment is done
		COleCurrency cyRem = CalculateRemainingBatchPaymentBalance(BatchPayID);
		// (j.jones 2014-07-28 08:28) - PLID 63065 - changed this to only apply to zero dollar balances,
		// it should not apply if the balance is negative
		if(cyRem == COleCurrency(0,0) && !m_checkShowPaysWithZeroBal.GetCheck()) {
			m_BatchPayments->RemoveRow(m_BatchPayments->CurSel);
			// (j.jones 2015-10-19 16:21) - PLID 67309 - call RequeryFinished to recalculate the capitation column
			OnRequeryFinishedBatchPaymentsCombo(dlRequeryFinishedCompleted);
			m_BatchPayments->CurSel = -1;
			MessageBox("The selected batch payment has been completely applied and will be removed from the list.");
			OnSelChosenBatchPaymentsCombo(-1);
		}
		else {
			m_BatchPayments->PutValue(m_BatchPayments->CurSel,COMBO_COLUMN_BALANCE,_variant_t(cyRem));

			CString strRem = FormatCurrencyForInterface(cyRem);
			SetDlgItemText(IDC_REM_AMOUNT,strRem);
		}

		// (z.manning, 03/23/2007) - PLID 25323 - We used to requery the entire charge list here, but now
		// we simply manually update the possibly affected rows.
		if(bApplyToWholeBill || bApplyToRevCode) {
			UpdateChargeListByBillID(BillID);
		}
		else {
			UpdateChargeListByChargeID(ChargeID);
		}

		//try to set the focus back to the datalist, and the last item or nearby item
		if(nChargeListCurSel != -1) {
			GetDlgItem(IDC_APPLY_LIST)->SetFocus();
			if(m_ChargeList->GetRowCount() - 1 < nChargeListCurSel)
				m_ChargeList->CurSel = m_ChargeList->GetRowCount()-1;
			else
				m_ChargeList->CurSel = nChargeListCurSel;
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2012-08-16 09:06) - PLID 52116 - Added bAutoPost, which will try
// to open line item posting invisible and auto-post the expected amount.
// The dialog would be forcibly displayed if there is a problem or warning
// and it can't silently post successfully.
// (j.jones 2015-10-20 08:48) - PLID 67377 - changed the autopost parameter to an enum of options
void CBatchPayments::InvokePostingDlg(BOOL bPostToWholeBill, EAutoPostType eAutoPostType /*= eDoNotAutoPost*/)
{
	try {

		CWaitCursor pWait;

		// (j.jones 2008-04-30 13:53) - PLID 28358 - the user needs to be able to apply payments,
		// not necessarily edit them, for line item posting to be allowed
		// (note: line item posting, like regular applies, allows shifting and adjustment creation,
		// but that is existing behavior and compares to other apply permissions system-wide)
		// (j.jones 2011-08-24 17:39) - PLID 45176 - do not call CanApplyLineItem here,
		// because the line item in question has not actually been created, so we only
		// need to check the normal apply permission
		if(!CheckCurrentUserPermissions(bioApplies, sptCreate)) {
			return;
		}

		if(m_BatchPayments->CurSel == -1 || m_ChargeList->CurSel == -1)
			return;

		// (j.jones 2012-08-22 17:34) - PLID 52153 - if grouping by bill, and this is called for a charge,
		// ASSERT, because this functionality should be disabled
		if(!IsCapitationPayment() && m_radioGroupByBill.GetCheck() && !bPostToWholeBill) {
			//we shouldn't have provided the option to apply to just one charge,
			//find out how it happened, and stop it
			ASSERT(FALSE);
			//change to apply to the bill
			bPostToWholeBill = TRUE;
		}

		// (j.jones 2014-06-27 09:31) - PLID 62547 - if a vision payment,
		// we should definitely be posting to the whole bill
		bool bIsVisionPayment = IsVisionPayment();
		if (bIsVisionPayment) {
			if (!bPostToWholeBill) {
				//we shouldn't have provided the option to apply to just one charge,
				//find out how it happened, and stop it
				ASSERT(FALSE);
				//change to apply to the bill
				bPostToWholeBill = TRUE;
			}
			if (eAutoPostType != eDoNotAutoPost) {
				//we shouldn't have provided the option to auto-post
				//find out how it happened, and stop it
				ASSERT(FALSE);
				//disable auto-posting
				eAutoPostType = eDoNotAutoPost;
			}
		}

		bool bAutoPostSuccess = false;

		long nChargeListCurSel = m_ChargeList->CurSel;

		long nBatchPayID = m_BatchPayments->GetValue(m_BatchPayments->CurSel,0).lVal;
		long nPatientID = m_ChargeList->GetValue(nChargeListCurSel,CHARGE_COLUMN_PATIENT_ID).lVal; 
		long nChargeID = m_ChargeList->GetValue(nChargeListCurSel,CHARGE_COLUMN_CHARGE_ID).lVal;
		
		// (j.jones 2007-02-28 11:48) - PLID 24521 - we used to calculate the nBillID if needed,
		// but it's in the datalist so there was no point in calling a recordset
		long nBillID = m_ChargeList->GetValue(m_ChargeList->GetCurSel(),CHARGE_COLUMN_BILL_ID).lVal;
		
		// (j.jones 2007-02-28 11:48) - PLID 24521 - we used to calculate the nInsuredPartyID, improperly
		// as it was not using the RespType, but since it's in the datalist I removed the bad query
		// and both fixed the ID loading as well as removed a roundtrip to the server
		// (j.jones 2011-11-02 14:22) - PLID 38686 - this can now be null, it might be patient resp
		long nInsuredPartyID = VarLong(m_ChargeList->GetValue(m_ChargeList->CurSel,CHARGE_COLUMN_INSURED_PARTY_ID), -1);

		// (j.jones 2011-11-02 14:37) - PLID 38686 - If RespTypeID is null, then this is patient resp.
		BOOL bIsPatientResp = (m_ChargeList->GetValue(m_ChargeList->CurSel,CHARGE_COLUMN_RESP_TYPE_ID).vt == VT_NULL);
		if(bIsPatientResp) {

			//the problem we face now is that we do not have an insured party in which to actually post to,
			//so we have to pick the highest ranking insured party that matches the batch payment company,
			//or is in the batch payment company's HCFA group
			long nBatchPayInsCoID = VarLong(m_BatchPayments->GetValue(m_BatchPayments->CurSel,COMBO_COLUMN_INSCO_ID));
			long nBatchPayInsCoGroup = VarLong(m_BatchPayments->GetValue(m_BatchPayments->CurSel,COMBO_COLUMN_HCFA_GROUP_ID),-1);
			
			//this query puts inactive resps. last
			_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 InsuredPartyT.PersonID AS InsuredPartyID, "
				"RespTypeT.ID AS RespTypeID "
				"FROM InsuranceCoT "
				"INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID "
				"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				"WHERE InsuredPartyT.PatientID = {INT} "
				"AND (InsuranceCoT.PersonID = {INT} OR InsuranceCoT.HCFASetupGroupID = {INT}) "
				"ORDER BY (CASE WHEN RespTypeT.Priority = -1 THEN 1 ELSE 0 END) ASC, RespTypeT.Priority ASC",
				nPatientID, nBatchPayInsCoID, nBatchPayInsCoGroup);
			if(!rs->eof) {
				nInsuredPartyID = VarLong(rs->Fields->Item["InsuredPartyID"]->Value);
			}
			rs->Close();

			//if -1, we could not determine an insured party to use, so they cannot post to a patient resp.
			if(nInsuredPartyID == -1) {
				AfxMessageBox("Practice cannot post to this patient responsibility charge because this patient does not "
					"have an insurance company that is in the same HCFA Group as the company this batch payment is for.\n\n"
					"You will need to manually shift the patient responsibility to the desired insured party "
					"before you can post to this charge.");
				GetDlgItem(IDC_APPLY_LIST)->SetFocus();
				return;
			}
		}
		
		// (j.jones 2006-12-28 10:09) - PLID 24030 - check if we are grouped by rev. code
		BOOL bApplyToRevCode = m_radioGroupByRevenueCode.GetCheck() ? TRUE : FALSE;
		long nRevCodeID = VarLong(m_ChargeList->GetValue(m_ChargeList->GetCurSel(),CHARGE_COLUMN_REV_CODE_ID), -1);

		// (j.jones 2014-07-02 15:18) - PLID 62652 - vision payments do not support revenue codes
		if (IsVisionPayment() && bApplyToRevCode) {
			//the code should never have allowed this option to be checked for vision payments
			ASSERT(FALSE);
			bApplyToRevCode = FALSE;
			nRevCodeID = -1;
		}

		// (j.jones 2015-10-20 08:46) - PLID 67377 - capitation payments can only post to
		// one charge at a time
		if (IsCapitationPayment()) {
			ASSERT(bPostToWholeBill == FALSE);
			bPostToWholeBill = FALSE;
			bApplyToRevCode = FALSE;
			nRevCodeID = -1;
		}

		// (j.jones 2012-08-16 10:20) - PLID 52162 - changed the way we launch line item posting to always
		// require the bill ID, and only send the chargeID if filtering on just one charge
		long nOnlyShowChargeID = -1;
		if(!bPostToWholeBill && (!bApplyToRevCode || nRevCodeID == -1)) {
			nOnlyShowChargeID = nChargeID;
		}

		//if we are posting by revenue code but posting to a non-rev.code charge, tweak the parameters a bit
		if(!bPostToWholeBill && bApplyToRevCode && nRevCodeID == -1) {
			bApplyToRevCode = FALSE;
		}
		//if posting to the whole bill, don't send a revenue code
		if(bPostToWholeBill) {
			nRevCodeID = -1;
		}

		// (j.jones 2010-05-17 13:22) - PLID 33941 - try to prompt the patient warning first
		PromptPatientWarning(nPatientID);

		// (j.jones 2012-08-16 09:54) - PLID 52116 - The auto-post option will try to silently
		// open line item posting in the background, tell it to post, and then close.
		// If it can't auto-post, the dialog will show itself as a normal modal dialog and
		// force them to use the dialog normally.
		// (j.jones 2015-10-20 08:48) - PLID 67377 - changed the autopost parameter to an enum of options
		if(eAutoPostType != eDoNotAutoPost) {
			CFinancialLineItemPostingDlg dlg(this);

			// (j.jones 2015-10-20 08:52) - PLID 67377 - capitation payments have already
			// calculated the payment amount, pass that amount in
			COleCurrency cyDefaultPaymentAmount = g_ccyInvalid;
			if (IsCapitationPayment()) {
				cyDefaultPaymentAmount = VarCurrency(m_ChargeList->GetValue(m_ChargeList->GetCurSel(), CHARGE_COLUMN_EST_CHARGE_PAYMENT));
			}

			// (j.jones 2014-06-27 10:43) - PLID 62548 - added PayType, always medical for auto-posted payments
			dlg.Create(IDD_FINANCIAL_LINE_ITEM_POSTING_DLG, this, eMedicalPayment, nBillID, nOnlyShowChargeID, nBatchPayID, nInsuredPartyID, bApplyToRevCode, bApplyToRevCode ? nRevCodeID : -1, cyDefaultPaymentAmount, IsCapitationPayment());

			//Try to auto-post. It will fail if it can't do so, with the most likely reason being
			//that it couldn't calculate a payment for all charges.
			//Will return FALSE if it failed. If so, we will retry showing this screen as a modal dialog.
			bAutoPostSuccess = dlg.AutoPost();

			//grab the failure (if any) before destroying the dialog
			CString strFailure = dlg.m_strAutoPostFailure;

			//close the hidden window (if you don't, you get lots of issues where the code is still running but the dialog is gone)
			dlg.DestroyWindow();

			//now warn if the posting failed
			if(!bAutoPostSuccess) {
				//The posting could not be automated. The reason for failure should have been provided.
				CString strWarning;
				if(strFailure.IsEmpty()) {
					//There isn't anything inherently wrong if there's no message given,
					//but ideally every failure should provide a message. We should
					//find out what caused this to return FALSE, and make sure it filled
					//m_strAutoPostFailure with some sort of explanation.
					ASSERT(FALSE);

					//no failure was provided, give a generic message
					strWarning = "The estimated payment could not be automatically applied.";
				}
				else {
					//report the failure
					strWarning.Format("The estimated payment could not be automatically applied for the following reason:\n\n"
						"%s", strFailure);
				}

				// (j.jones 2015-10-20 08:48) - PLID 67377 - we now have an ability to not open line item posting
				// upon failure, so the message has to be context-sensitive
				if (eAutoPostType == eAutoPostOnly_NoDlg) {
					//the dialog will not open
					strWarning += "\n\n"
						"You must correct this patient's account before the estimated payment can be posted.";	

					MessageBox(strWarning, "Practice", MB_ICONINFORMATION | MB_OK);
					//abort posting
					return;
				}
				else {
					//the dialog will open
					strWarning += "\n\n"
						"The line item posting screen will now open in order to post payments manually.";

					//offer to cancel posting
					if (MessageBox(strWarning, "Practice", MB_ICONINFORMATION | MB_OKCANCEL) == IDCANCEL) {
						//they cancelled posting
						return;
					}
				}				
			}
		}
		
		//The normal case will be false, and we would DoModal.
		//However, if we tried to auto-post, and could not for some reason, then
		//it would be set to false again and now we would DoModal to let them
		//manually post.
		// (j.jones 2015-10-20 08:48) - PLID 67377 - for capitation the posting
		// type is eAutoPostOnly_NoDlg, so if auto posting failed, we do NOT
		// open line item posting
		if(!bAutoPostSuccess && eAutoPostType != eAutoPostOnly_NoDlg) {
			CFinancialLineItemPostingDlg dlg(this);
			// (j.jones 2014-06-27 10:43) - PLID 62548 - added PayType
			dlg.DoModal(bIsVisionPayment ? eVisionPayment : eMedicalPayment, nBillID, nOnlyShowChargeID, nBatchPayID, nInsuredPartyID, bApplyToRevCode, bApplyToRevCode ? nRevCodeID : -1);
		}

		//check to see if this payment is done
		COleCurrency cyRem = CalculateRemainingBatchPaymentBalance(nBatchPayID);
		// (j.jones 2014-07-28 08:28) - PLID 63065 - changed this to only apply to zero dollar balances,
		// it should not apply if the balance is negative
		if(cyRem == COleCurrency(0,0) && !m_checkShowPaysWithZeroBal.GetCheck()) {
			m_BatchPayments->RemoveRow(m_BatchPayments->CurSel);
			// (j.jones 2015-10-19 16:21) - PLID 67309 - call RequeryFinished to recalculate the capitation column
			OnRequeryFinishedBatchPaymentsCombo(dlRequeryFinishedCompleted);
			m_BatchPayments->CurSel = -1;
			nChargeListCurSel = -1;
			MessageBox("The selected batch payment has been completely applied and will be removed from the list.");
			OnSelChosenBatchPaymentsCombo(-1);
		}
		else {
			m_BatchPayments->PutValue(m_BatchPayments->CurSel,COMBO_COLUMN_BALANCE,_variant_t(cyRem));

			CString strRem = FormatCurrencyForInterface(cyRem);
			SetDlgItemText(IDC_REM_AMOUNT,strRem);
		}

		// (z.manning, 03/23/2007) - PLID 25323 - We used to requery the entire charge list here, but now
		// we simply manually update the possibly affected rows.
		UpdateChargeListByBillID(nBillID);

		//try to set the focus back to the datalist, and the last item or nearby item
		if(nChargeListCurSel != -1) {
			GetDlgItem(IDC_APPLY_LIST)->SetFocus();
			if(m_ChargeList->GetRowCount() - 1 < nChargeListCurSel)
				m_ChargeList->CurSel = m_ChargeList->GetRowCount()-1;
			else
				m_ChargeList->CurSel = nChargeListCurSel;
		}

	}NxCatchAll("Error creating posting screen.");
}

// (j.jones 2012-08-15 14:06) - PLID 52115 - added ability to auto-select the first charge when requery finishes
void CBatchPayments::RequeryChargeList(BOOL bSelectFirstCharge /*= FALSE*/)
{
	CWaitCursor pWait;

	// (j.jones 2012-08-15 14:06) - PLID 52115 - set the member variable, it will be
	// handled in OnRequeryFinishedApplyList 
	m_bAutoSelectFirstChargeOnRequeryFinished = bSelectFirstCharge;

	// (j.jones 2015-10-16 16:30) - PLID 67382 - returns true if a capitation payment or the 
	// Est. Payment column preference is on, false if a vision payment
	bool bShowEstPaymentColumns = ShowEstPaymentColumns();

	// (j.jones 2012-08-29 10:16) - PLID 52351 - added ability to calculate secondary payments as well
	BOOL bEstSecondaryPayments = (GetRemotePropertyInt("BatchPayments_ShowEstPaysColumn_Secondary", 0, 0, GetCurrentUserName(), true) == 1);
	// (j.jones 2014-06-27 09:31) - PLID 62547 - if a vision payment, always hide these columns
	if (IsVisionPayment()) {
		bShowEstPaymentColumns = false;
		bEstSecondaryPayments = FALSE;
	}
	
	m_ChargeList->FromClause = _bstr_t(GenerateFromClause(bShowEstPaymentColumns, bEstSecondaryPayments));

	//show/hide the est. payment columns
	UpdateEstPaymentColumns(bShowEstPaymentColumns);

	//the screen will be re-enabled in OnRequeryFinished
	DisableScreen();

	m_ChargeList->Requery();
}

void CBatchPayments::OnRequeryFinishedApplyList(short nFlags) 
{
	try {

		CWaitCursor pWait;

		// (j.jones 2012-08-15 14:06) - PLID 52115 - set the member variable, it will be
		// handled in OnRequeryFinishedApplyList 
		if(m_bAutoSelectFirstChargeOnRequeryFinished) {

			//always reset to false
			m_bAutoSelectFirstChargeOnRequeryFinished = FALSE;

			//now set focus to the first charge, only if one exists
			//(don't mess with focus if no charge exists)
			if(m_ChargeList->GetRowCount() > 0) {
				//should be impossible in this function, but if for some reason
				//a selection is already in place, don't change it
				if(m_ChargeList->GetCurSel() == -1) {
					m_ChargeList->PutCurSel(0);
				}

				//set focus on the charge list
				GetDlgItem(IDC_APPLY_LIST)->SetFocus();
			}
		}

		// (j.jones 2015-10-19 14:41) - PLID 67376 - calculate capitation payment amounts
		if (IsCapitationPayment()) {
			CalculateCapitation();
			// (b.eyers 2015-10-21) - PLID 67361
			//color empty allowable a red color
			IRowSettingsPtr pRow;
			bool bHasMissingAllowables = false;
			for (int i = 0; i < m_ChargeList->GetRowCount(); i++) {
				pRow = m_ChargeList->GetRow(i);
				bool bIsAllowable = VarBool(m_ChargeList->GetValue(i, CHARGE_COLUMN_IS_ALLOWABLE), FALSE) ? true : false;
				if (!bIsAllowable) {
					pRow->PutCellBackColor(CHARGE_COLUMN_ALLOWABLE, RGB(255, 0, 0));
					bHasMissingAllowables = true;
				}
			}
			if (bHasMissingAllowables) {
				//sort with empty allowable at top
				m_ChargeList->GetColumn(CHARGE_COLUMN_IS_ALLOWABLE)->PutSortPriority(0);
				m_ChargeList->GetColumn(CHARGE_COLUMN_IS_ALLOWABLE)->PutSortAscending(VARIANT_TRUE);

				long nColumn = GetRemotePropertyInt("BatchPaymentsChargeSortColumn", -1, 0, GetCurrentUserName(), false);
				long nAsc = GetRemotePropertyInt("BatchPaymentsChargeSortAsc", -1, 0, GetCurrentUserName(), false);

				BOOL bApplyDefaultSort = TRUE;
				if (nColumn >= 0 && (nColumn + 1) <= m_ChargeList->GetColumnCount()) {
					//the column index is valid, is the column even displayed?
					if (m_ChargeList->GetColumn((short)nColumn)->GetStoredWidth() > 0) {
						//it is displayed, so this is indeed a valid, visible column
						bApplyDefaultSort = FALSE;
					}
				}

				if (bApplyDefaultSort) {
					m_ChargeList->GetColumn(CHARGE_COLUMN_PAT_NAME)->PutSortPriority(1);
					m_ChargeList->GetColumn(CHARGE_COLUMN_PAT_NAME)->PutSortAscending(VARIANT_TRUE);
				}
				else {
					m_ChargeList->GetColumn((short)nColumn)->PutSortPriority(1);
					m_ChargeList->GetColumn((short)nColumn)->PutSortAscending(nAsc == 1 ? VARIANT_TRUE : VARIANT_FALSE);
				}

				m_ChargeList->Sort();
			}
		}

		EnableScreen();

	}NxCatchAll("CBatchPayments::OnRequeryFinishedApplyList"); // (z.manning, 03/27/2007) - PLID 25357
}

// (s.tullis 2014-06-23 11:21) - PLID 49455 - Permission: Batch payment User Permission to Control Read and Wring in the batch Payment Tab
void CBatchPayments::SecureControls()
{
	try{
		
		BOOL bCanWrite = (GetCurrentUserPermissions(bioBatchPayment) & SPT___W_______);

		GetDlgItem(IDC_EDIT_PAYMENT)->EnableWindow(bCanWrite);
		GetDlgItem(IDC_DELETE_PAYMENT)->EnableWindow(bCanWrite);
		GetDlgItem(IDC_ADD_PAYMENT)->EnableWindow(bCanWrite);
		GetDlgItem(IDC_BTN_POST_TO_PATIENT)->EnableWindow(bCanWrite);
		GetDlgItem(IDC_BTN_NEW_ADJUSTMENT)->EnableWindow(bCanWrite);
		GetDlgItem(IDC_INSURANCE_REVERSAL)->EnableWindow(bCanWrite);
		//GetDlgItem(IDC_BTN_UNAPPLY_EOB)->EnableWindow(bCanWrite); // (b.eyers 2015-10-14) - PLID 67308 - this was enabling even when there isn't a batch selected
		GetDlgItem(IDC_BTN_NEW_REFUND)->EnableWindow(bCanWrite);
		GetDlgItem(IDC_BTN_NEW_EOB)->EnableWindow(bCanWrite);
		GetDlgItem(IDC_INSURANCE_REVERSAL)->EnableWindow(bCanWrite);

	}NxCatchAll(__FUNCTION__)
}

// (j.jones 2014-06-26 17:30) - PLID 62545 - renamed to indicate this is the button handler only
void CBatchPayments::OnBtnAddPayment()
{
	try {
		
		// (j.jones 2008-04-30 14:14) - PLID 28358 - added check for create permissions
		if(!CheckCurrentUserPermissions(bioPayment, sptCreate)) {
			return;
		}
		
		// (j.jones 2014-06-26 17:34) - PLID 62545 - this now either pops out a menu
		// if you have the Vision Payments license, or directly makes a new payment
		// if you do not have that license
		if (g_pLicense->CheckForLicense(CLicense::lcVisionPayments, CLicense::cflrSilent)) {

			enum {
				eMedical = 1,
				eVision,
			};

			CMenu menu;
			menu.m_hMenu = CreatePopupMenu();
			menu.InsertMenu(0, MF_BYPOSITION, eMedical, "&Medical Batch Payment");
			menu.InsertMenu(1, MF_BYPOSITION, eVision, "&Vision Batch Payment");

			CRect rc;
			GetDlgItem(IDC_ADD_PAYMENT)->GetWindowRect(&rc);

			long nResult = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD, rc.right, rc.top, this, NULL);
			menu.DestroyMenu();
			switch (nResult) {
				case eMedical:
					AddNewPayment(eMedicalPayment);
					break;
				case eVision:
					AddNewPayment(eVisionPayment);
					break;
				default:
					break;
			}
		}
		else {
			//immediately open the payment dialog
			AddNewPayment(eMedicalPayment);
		}

	}NxCatchAll(__FUNCTION__); // (z.manning, 03/27/2007) - PLID 25357
}

// (j.jones 2014-06-26 17:31) - PLID 62545 - added a dedicated AddNewPayment function
void CBatchPayments::AddNewPayment(EBatchPaymentPayType ePayType)
{
	try {
		// (j.jones 2008-04-30 14:14) - PLID 28358 - added check for create permissions
		if(!CheckCurrentUserPermissions(bioPayment, sptCreate)) {
			return;
		}

		// (j.jones 2014-07-11 16:41) - PLID 62545 - the vision payment license check is silent in most places,
		// if we get here, "use" the license, which will revert to a medical payment if they have lost the license
		if (ePayType == eVisionPayment && !g_pLicense->CheckForLicense(CLicense::lcVisionPayments, CLicense::cflrUse)) {
			ePayType = eMedicalPayment;
		}

		CBatchPaymentEditDlg dlg(this);

		// (j.jones 2014-06-26 17:29) - PLID 62546 - set the PayType to Medical or Vision payment
		dlg.m_ePayType = ePayType;

		if(IDOK == dlg.DoModal()) {

			long nOldVal = m_checkShowPaysWithZeroBal.GetCheck();
			m_checkShowPaysWithZeroBal.SetCheck(GetRemotePropertyInt("ShowZeroBalanceBatchPays",0,0,GetCurrentUserName(),FALSE));		
			m_BatchPayments->CurSel = -1;
			OnCheckShowZeroBalancePayments();
			
			// (j.jones 2004-06-30 14:06) - if they made a $0.00 payment, the option to show zero dollar payments will be enabled
			//but let them know that this happened
			if(nOldVal != m_checkShowPaysWithZeroBal.GetCheck()) {
				AfxMessageBox("Since you entered a zero dollar payment, the option to show these payments has been automatically enabled.");
			}

			long PaymentID = dlg.m_ID;

			long row = m_BatchPayments->SetSelByColumn(0,PaymentID);

			OnSelChosenBatchPaymentsCombo(row);
		}

	}NxCatchAll(__FUNCTION__);
}

void CBatchPayments::OnEditPayment() 
{
	try {

		if(m_BatchPayments->CurSel == -1) {
			AfxMessageBox("Please select a payment from the list.");
			return;
		}

		long nPaymentID = VarLong(m_BatchPayments->GetValue(m_BatchPayments->CurSel,0));;

		// (j.jones 2008-04-30 14:14) - PLID 28358 - Added check for write permissions,
		// which also uses CanEdit(). In most cases CanEdit is not needed, so first we must
		// silently see if they have permission, and if they do, then move ahead normally. But if
		// they don't, or they need a password, then check CanEdit prior to the permission check that
		// would stop or password-prompt the user.
		// (c.haag 2009-03-10 10:25) - PLID 32433 - This code has been replaced with CanChangeHistoricFinancial
		if (!CanChangeHistoricFinancial("BatchPayment", nPaymentID, bioPayment, sptWrite)) {
			return;
		}
		/*if(!(GetCurrentUserPermissions(bioPayment) & sptWrite)
			&& !CanEdit("BatchPayment", nPaymentID)
			&& !CheckCurrentUserPermissions(bioPayment, sptWrite)) {
			return;
		}*/

		CBatchPaymentEditDlg dlg(this);
		dlg.m_ID = nPaymentID;
		
		if(IDOK == dlg.DoModal()) {

			long PaymentID = dlg.m_ID;

			// (b.savon 2012-06-13 15:58) - PLID 49879 - If the batch date has changed, update the payments/line items as well
			if( dlg.m_bBatchDateChanged ){
				UpdatePaymentDatesToReflectBatchDate(PaymentID, dlg.m_strBatchDate);
			}

			m_BatchPayments->Requery();

			long row = m_BatchPayments->SetSelByColumn(0, nPaymentID);

			OnSelChosenBatchPaymentsCombo(row);
		}

	}NxCatchAll("CBatchPayments::OnEditPayment"); // (z.manning, 03/27/2007) - PLID 25357
}

void CBatchPayments::OnDeletePayment() 
{
	long CurSel = m_BatchPayments->CurSel;

	if(CurSel == -1) {
		MessageBox("Please select a payment from the list.");
		return;
	}

	try {

		long PaymentID = m_BatchPayments->GetValue(CurSel,0).lVal;

		// (j.jones 2008-04-30 14:48) - PLID 28358 - Added check for delete permissions,
		// which also uses CanEdit(). In most cases CanEdit is not needed, so first we must
		// silently see if they have permission, and if they do, then move ahead normally. But if
		// they don't, or they need a password, then check CanEdit prior to the permission check that
		// would stop or password-prompt the user.
		// (c.haag 2009-03-10 10:25) - PLID 32433 - This code has been replaced with CanChangeHistoricFinancial
		if (!CanChangeHistoricFinancial("BatchPayment", PaymentID, bioPayment, sptDelete)) {
			return;
		}
		/*if(!(GetCurrentUserPermissions(bioPayment) & sptDelete)
			&& !CanEdit("BatchPayment", PaymentID)
			&& !CheckCurrentUserPermissions(bioPayment, sptDelete)) {
			return;
		}*/

		// (j.jones 2009-06-09 14:57) - PLID 34549 - added context-sensitive deletion warning
		// (j.jones 2009-06-10 17:35) - PLID 34592 - disallow deleting if there is another batch payment
		// (j.jones 2012-04-25 11:15) - PLID 48032 - diallow deleting if there is a linked correction
		_RecordsetPtr rs = CreateParamRecordset("SELECT Sum(CASE WHEN Type = 2 THEN 1 ELSE 0 END) AS AdjustmentCount, "
			"Count(PaymentsT.ID) AS LineItemCount FROM PaymentsT "
			"INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID WHERE BatchPaymentID = {INT} AND Deleted = 0 "
			""
			"SELECT TOP 1 ID FROM BatchPaymentsT WHERE Deleted = 0 AND AppliedBatchPayID = {INT} "
			""
			"SELECT TOP 1 ID FROM LineItemCorrectionsT WHERE BatchPaymentID = {INT}",
			PaymentID, PaymentID, PaymentID);
		if(!rs->eof) {

			long nAdjustmentCount = AdoFldLong(rs, "AdjustmentCount", 0);
			long nLineItemCount = AdoFldLong(rs, "LineItemCount", 0);

			if(nAdjustmentCount > 0 && nLineItemCount == nAdjustmentCount) {
				AfxMessageBox("This payment currently has related adjustments applied to one or more patient accounts. Please delete these applied adjustments before continuing.");				
				return;
			}
			else if(nAdjustmentCount == 0 && nLineItemCount > 0) {
				AfxMessageBox("This payment is currently applied to one or more patient accounts. Please delete these applied payments before continuing.");
				return;
			}
			else if(nLineItemCount > 0 || nAdjustmentCount > 0) {
				AfxMessageBox("This payment is currently applied to one or more patient accounts. Please delete these applied payments and related adjustments before continuing.");				
				return;
			}			
		}

		// (j.jones 2009-06-10 17:35) - PLID 34592 - disallow deleting if there is another batch payment
		rs = rs->NextRecordset(NULL);
		if(!rs->eof) {
			AfxMessageBox("This batch payment currently has applied adjustments or refunds, and cannot be deleted.\n"
				"Please delete these applies from the \"View Refunded / Adjusted Payments\" screen before continuing.");
			return;
		}

		// (j.jones 2012-04-25 11:15) - PLID 48032 - diallow deleting if there is a linked correction
		rs = rs->NextRecordset(NULL);
		if(!rs->eof) {
			AfxMessageBox("This batch payment currently has financial corrections linked to it from voided payments and adjustments, and cannot be deleted.\n"
				"Please undo these corrections before continuing.");
			return;
		}

		rs->Close();

		if(IDYES==MessageBox("Are you sure you wish to delete this payment?","Practice",MB_ICONEXCLAMATION|MB_YESNO)) {
			
			ExecuteParamSql("UPDATE BatchPaymentsT SET Deleted = 1, DeleteDate = GetDate(), DeletedBy = {INT} WHERE ID = {INT}",GetCurrentUserID(),PaymentID);

			long AuditID = -1;
			AuditID = BeginNewAuditEvent();
			if(AuditID!=-1) {

				//audit deleting the batch payment

				// (j.jones 2009-06-11 10:50) - PLID 30478 - improved the auditing here to match how we do it
				// when unapplying an E-Remittance
				COleDateTime dtBatchPayDate = VarDateTime(m_BatchPayments->GetValue(CurSel,COMBO_COLUMN_DATE));
				CString strInsuranceCoName = VarString(m_BatchPayments->GetValue(CurSel,COMBO_COLUMN_INSCO_NAME));
				CString strBatchPayDesc = VarString(m_BatchPayments->GetValue(CurSel,COMBO_COLUMN_DESCRIPTION));
				COleCurrency cyBatchPayAmt = VarCurrency(m_BatchPayments->GetValue(CurSel,COMBO_COLUMN_AMOUNT));

				CString strOldValue;
				strOldValue.Format("Amount: %s, Description: %s, Payment Date: %s",
					FormatCurrencyForInterface(cyBatchPayAmt), strBatchPayDesc, FormatDateTimeForInterface(dtBatchPayDate, NULL, dtoDate));

				AuditEvent(-1, strInsuranceCoName, AuditID, aeiBatchPayDeleted, PaymentID, strOldValue, "<Deleted>", aepHigh);
			}
			
			m_BatchPayments->RemoveRow(CurSel);
			// (j.jones 2015-10-19 16:21) - PLID 67309 - call RequeryFinished to recalculate the capitation column
			OnRequeryFinishedBatchPaymentsCombo(dlRequeryFinishedCompleted);
			m_BatchPayments->CurSel = -1;
			OnSelChosenBatchPaymentsCombo(-1);
		}	

	}NxCatchAll("Error deleting batch payment.");
}

void CBatchPayments::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	try {

		long PaymentID = -1;

		if(m_BatchPayments->CurSel != -1) {
			PaymentID = m_BatchPayments->GetValue(m_BatchPayments->CurSel,0).lVal;
		}

		// (j.jones 2012-08-17 16:47) - PLID 52116 - Check the preference for whether the
		// Estimated Payment columns are shown. If they are, we need to reference our dbo functions,
		// which will show in the next requery.
		// (j.jones 2015-10-16 16:30) - PLID 67382 - returns true if a capitation payment or the 
		// Est. Payment column preference is on, false if a vision payment
		bool bShowEstPaymentColumns = ShowEstPaymentColumns();
		// (j.jones 2012-08-29 10:16) - PLID 52351 - added ability to calculate secondary payments as well
		BOOL bEstSecondaryPayments = (GetRemotePropertyInt("BatchPayments_ShowEstPaysColumn_Secondary", 0, 0, GetCurrentUserName(), true) == 1);
		// (j.jones 2014-06-27 09:31) - PLID 62547 - if a vision payment, always hide these columns
		if (IsVisionPayment()) {
			bShowEstPaymentColumns = false;
			bEstSecondaryPayments = FALSE;
		}

		//change the from clause to calculate these values as needed
		m_ChargeList->FromClause = _bstr_t(GenerateFromClause(bShowEstPaymentColumns, bEstSecondaryPayments));

		//show/hide the est. payment columns
		UpdateEstPaymentColumns(bShowEstPaymentColumns);

		// (s.tullis 2014-08-29 16:38) - PLID 63240 - Batch Payments needs a CTableChecker object for PatCombo.
		EnsurePatComboUpdated();

		// (j.jones 2010-05-04 08:51) - PLID 35610 - the Bill ID column is now always shown
		// (j.jones 2010-05-18 17:52) - PLID 38685 - the Filter By Bill ID button is now always shown
		m_BatchPayments->Requery();

		if(PaymentID != -1) {
			long row = m_BatchPayments->SetSelByColumn(0,PaymentID);
			OnSelChosenBatchPaymentsCombo(row);
		}
		else
			OnSelChosenBatchPaymentsCombo(-1);

	}NxCatchAll("Error in UpdateView()");
}

//This function will save the current selected row, and disable the redrawing
//of the screen so that any datalist operations can occur stealthily.
void CBatchPayments::DisableScreen() {

	//Increment the reference count for these function pairs
	m_RedrawRefCount++;

	//if the screen is already hidden, leave
	if(!m_bIsScreenEnabled)
		return;
	else {
		//save the current row
		m_SavedRow = m_ChargeList->GetCurSel();

		m_SavedTopRow = m_ChargeList->TopRowIndex;

		//turn off the redraw
		m_ChargeList->SetRedraw(FALSE);

		//set the boolean to let future instances of this function
		//know that this function has already been called
		m_bIsScreenEnabled = FALSE;
	}

}


//This function uses reference counting, so if a Disable/Enable pair gets called within
//another Disable/Enable pair, only the outer calls get executed.
//The reasoning behind this is, these functions hide the screen while various
//operations get carried out. If one of those enables the screen, it defeats the whole purpose.
void CBatchPayments::EnableScreen() {

	//Decrement the reference count for these function pairs
	m_RedrawRefCount--;

	//if the screen is already enabled (should not be possible)
	//or the reference count is not zero (much more likely),
	//leave this function
	if(m_bIsScreenEnabled || m_RedrawRefCount > 0)
		return;
	else {
		//if the reference count is zero, we are at the top level
		//of te Disable/Enable pairs, so let's go to work!

		//first set the top row, so we aren't disoriented
		m_ChargeList->TopRowIndex = m_SavedTopRow;

		//then set the selection to where we previously were
		m_ChargeList->PutCurSel(m_SavedRow);

		//all done! Let's redraw the window now
		m_ChargeList->SetRedraw(TRUE);
		UpdateWindow();

		//let future calls of these functions know the screen is ready to go
		m_bIsScreenEnabled = TRUE;
	}
}

void CBatchPayments::OnBtnNewEob() 
{
	try {
		

		if (!g_pLicense->CheckForLicense(CLicense::lcLockboxPayments, CLicense::cflrSilent)) {
			if (GetMainFrame()) {
				GetMainFrame()->ShowEOBDlg();
			}
			return;
		}
		// (b.savon 2015-07-07 10:17) - PLID 66484 - If they have lockbox and don't have ERemittance open the lockbox dialog on button click
		else if (g_pLicense->CheckForLicense(CLicense::lcLockboxPayments, CLicense::cflrSilent) && !g_pLicense->CheckForLicense(CLicense::lcERemittance, CLicense::cflrSilent)){
			if (GetMainFrame()) {
				GetMainFrame()->ShowLockboxPaymentImportDlg();
			}
			return;
		}
		else{
			//They have both licenses -- Continue on and show the context menu
		}

		// (d.singleton 2014-07-11 13:06) - PLID 62866 - create context menu when clicking "Import Electronic Payment" in batch payments tab
		enum NewEobItems {
			neiEOB = -1,
			neiLockBox = -2,
		};

		CMenu pMenu;
		pMenu.CreatePopupMenu();
		pMenu.InsertMenu(0, MF_BYPOSITION, neiEOB, "E-Remittance Payment File");
		pMenu.InsertMenu(1, MF_BYPOSITION, neiLockBox, "Lockbox Payment File");

		CRect rect;
		GetDlgItem(IDC_BTN_NEW_EOB)->GetWindowRect(&rect);

		int nMenuCommand = pMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD | TPM_TOPALIGN, rect.right, rect.top, this, NULL);

		switch (nMenuCommand)
		{
		case neiEOB:
		{
			// (j.jones 2012-05-25 11:03) - PLID 44367 - this is now a modeless dialog
			if (GetMainFrame()) {
				GetMainFrame()->ShowEOBDlg();
			}
			break;
		}
		case neiLockBox:
		{
			if (GetMainFrame()) {
				GetMainFrame()->ShowLockboxPaymentImportDlg();
			}
			break;
		}
		}
	
	}NxCatchAll("CBatchPayments::OnBtnNewEob"); // (z.manning, 03/27/2007) - PLID 25357
}

void CBatchPayments::OnBtnNewRefund() 
{
	try {

		if(m_BatchPayments->CurSel == -1) {
			AfxMessageBox("Please select a payment from the list to refund.");
			return;
		}

		// (j.jones 2008-04-30 14:52) - PLID 28358 - added check for create permissions
		if(!CheckCurrentUserPermissions(bioRefund, sptCreate)) {
			return;
		}

		long PaymentID = m_BatchPayments->GetValue(m_BatchPayments->CurSel,0).lVal;

		// (j.jones 2014-04-21 09:16) - PLID 61740 - we no longer disallow refunding zero dollar payments,
		// we instead only disallow based on the balance
		/*
		if(ReturnsRecords("SELECT ID FROM BatchPaymentsT WHERE Amount = Convert(money,0) AND ID = %li",PaymentID)) {
			AfxMessageBox("You cannot refund a zero dollar payment.");
			return;
		}
		*/

		CBatchPaymentEditDlg dlg(this);

		dlg.m_nType = 3;
		// (j.jones 2014-06-26 17:29) - PLID 62546 - set the PayType to NoPayment
		dlg.m_ePayType = eNoPayment;
		dlg.m_nAppliedPayID = PaymentID;
		// (z.manning, 03/22/2007) - PLID 25305 - Get the remaining balance from the datalist instead of calculating it again.
		// (j.jones 2012-02-13 15:34) - PLID 48121 - disallow applying a refund if the balance is less than or equal to zero
		COleCurrency cyBalance = VarCurrency(m_BatchPayments->GetValue(m_BatchPayments->CurSel, COMBO_COLUMN_BALANCE), COleCurrency(0,0));
		if(cyBalance == COleCurrency(0,0)) {
			AfxMessageBox("You cannot refund a payment with a zero balance.");
			return;
		}
		else if(cyBalance < COleCurrency(0,0)) {
			AfxMessageBox("You cannot refund a payment with a negative balance.");
			return;
		}
		dlg.m_cyMaxApplyAmt = cyBalance;

		if(IDOK == dlg.DoModal()) {

			m_BatchPayments->Requery();

			long row = m_BatchPayments->SetSelByColumn(0,PaymentID);

			OnSelChosenBatchPaymentsCombo(row);
		}

	}NxCatchAll("CBatchPayments::OnBtnNewRefund"); // (z.manning, 03/27/2007) - PLID 25357
}

void CBatchPayments::OnBtnViewAppliedRefunds() 
{
	try {

		CRefundedBatchPaysDlg dlg(this);
		dlg.DoModal();

		//now reflect any changes that could have been made

		long nCurPaymentID = -1;
		if(m_BatchPayments->CurSel != -1)
			nCurPaymentID = VarLong(m_BatchPayments->GetValue(m_BatchPayments->GetCurSel(),0));

		m_BatchPayments->Requery();

		long row = -1;
		if(nCurPaymentID != -1)
			row = m_BatchPayments->SetSelByColumn(0,nCurPaymentID);

		OnSelChosenBatchPaymentsCombo(row);

	}NxCatchAll("CBatchPayments::OnBtnViewAppliedRefunds"); // (z.manning, 03/27/2007) - PLID 25357
}

void CBatchPayments::CreateAdjustment()
{
	if(m_BatchPayments->CurSel == -1 || m_ChargeList->CurSel == -1)
		return;

	if(!CheckCurrentUserPermissions(bioAdjustment,sptCreate))
		return;

	try {

		// (j.jones 2010-05-17 13:22) - PLID 33941 - try to prompt the patient warning first
		long PatientID = VarLong(m_ChargeList->GetValue(m_ChargeList->CurSel,CHARGE_COLUMN_PATIENT_ID));
		PromptPatientWarning(PatientID);

		long nChargeListCurSel = m_ChargeList->CurSel;

		CPaymentDlg dlg(this);

		_variant_t var = m_ChargeList->GetValue(m_ChargeList->GetCurSel(),CHARGE_COLUMN_INS_BALANCE);
		COleCurrency cyInsBalance;
		if(var.vt == VT_CY)
			cyInsBalance = var.cyVal;
		else
			cyInsBalance = COleCurrency(0,0);

		long ChargeID = m_ChargeList->GetValue(m_ChargeList->GetCurSel(),CHARGE_COLUMN_CHARGE_ID).lVal;
		long BillID = m_ChargeList->GetValue(m_ChargeList->GetCurSel(),CHARGE_COLUMN_BILL_ID).lVal;
		// (j.jones 2011-11-02 14:22) - PLID 38686 - these can now be null, use 0 for RespTypeID to not be confused with inactive
		long nRespTypeID = VarLong(m_ChargeList->GetValue(m_ChargeList->CurSel,CHARGE_COLUMN_RESP_TYPE_ID), 0);
		long nInsuredPartyID = VarLong(m_ChargeList->GetValue(m_ChargeList->CurSel,CHARGE_COLUMN_INSURED_PARTY_ID), -1);

		// (b.eyers 2015-10-16) - PLID 67357 - get batch id
		IRowSettingsPtr pBatchPaymentRow = m_BatchPayments->GetRow(m_BatchPayments->CurSel);
		long PaymentGroupID = VarLong(pBatchPaymentRow->GetValue(COMBO_COLUMN_ID).lVal, -1);

		// (j.jones 2006-12-28 10:09) - PLID 23160 - store the setting for grouped by rev. code
		BOOL bApplyToRevCode = m_radioGroupByRevenueCode.GetCheck() ? TRUE : FALSE;
		long nRevCodeID = VarLong(m_ChargeList->GetValue(m_ChargeList->GetCurSel(),CHARGE_COLUMN_REV_CODE_ID), -1);

		dlg.m_PatientID = PatientID;
		dlg.m_cyFinalAmount = cyInsBalance;
		dlg.m_cyMaxAmount = dlg.m_cyFinalAmount;
		// (j.jones 2012-08-22 17:45) - PLID 52153 - if grouping by bill, we can only apply to the bill
		if(!IsCapitationPayment() && m_radioGroupByBill.GetCheck()) {
			dlg.m_varBillID = BillID;
		}
		else {
			dlg.m_varChargeID = ChargeID;
		}
		dlg.m_iDefaultInsuranceCo = nInsuredPartyID;
		dlg.m_ApplyOnOK = !(bApplyToRevCode && nRevCodeID != -1);			
		dlg.m_iDefaultPaymentType = 1;
		dlg.m_cyFinalAmount *= -1;

		// (b.eyers 2015-10-16) - PLID 67357 - send batch id to payment dlg
		dlg.m_nBatchPaymentID = PaymentGroupID;

		if (IDCANCEL != dlg.DoModal(__FUNCTION__, __LINE__)) {

			//if a revenue code, handle the apply ourselves
			if(bApplyToRevCode && nRevCodeID != -1) {
				// (j.jones 2010-06-02 17:44) - PLID 37200 - renamed to not be insurance-specific
				ApplyPayToBillWithRevenueCode(VarLong(dlg.m_varPaymentID), PatientID, dlg.m_cyFinalAmount, BillID, nRevCodeID, nInsuredPartyID, nInsuredPartyID, FALSE, FALSE, FALSE);

				//we're bypassing what would have been a prompt to shift, so ask now
				if (-AdjustBalance(nRevCodeID, BillID,  PatientID, 3, nRespTypeID, nInsuredPartyID) > COleCurrency(0,0)) {
					// (j.jones 2013-07-22 11:43) - PLID 57653 - added the payment ID and date
					PostInsuranceApply(VarLong(dlg.m_varPaymentID), nInsuredPartyID, PatientID, BillID, "RevCode", nRevCodeID);
				}
			}

			// (z.manning, 03/23/2007) - PLID 25323 - We used to requery the entire charge list here, but now
			// we simply manually update the possibly affected rows.
			UpdateChargeListByBillID(BillID);
		}

		//try to set the focus back to the datalist, and the last item or nearby item
		if(nChargeListCurSel != -1) {
			GetDlgItem(IDC_APPLY_LIST)->SetFocus();
			if(m_ChargeList->GetRowCount() - 1 < nChargeListCurSel)
				m_ChargeList->CurSel = m_ChargeList->GetRowCount()-1;
			else
				m_ChargeList->CurSel = nChargeListCurSel;
		}

	}NxCatchAll("Error creating adjustment.");
}

void CBatchPayments::OnBtnFilterByBillId() 
{
	try {

		//if filtering, unfilter!
		if(m_bFilteredByBill) {			
			m_bFilteredByBill = FALSE;
			m_btnFilterByBillID.SetWindowText("Filter By Bill ID");
			ChangePatientSelectionType();
			return;
		}

		long nBillID;
		CString strID;
		if (InputBox(this, "Enter a Bill / Claim ID", strID, "") == IDOK) {
			nBillID = atol(strID);
		}
		else {
			return;
		}

		if(IsRecordsetEmpty("SELECT ID FROM BillsT WHERE Deleted = 0 AND ID = %li",nBillID)) {
			AfxMessageBox("That Bill ID does not exist in the system.");
			return;
		}

		if(m_ChargeList->FindByColumn(CHARGE_COLUMN_BILL_ID,(long)nBillID,0,FALSE) == -1) {
			AfxMessageBox("That Bill ID does not exist in the current list.");
			return;
		}

		CWaitCursor pWait;

		DisableScreen();

		//at this point, it was found, so now go through the list and remove all rows that are not this bill ID
		for(int i = m_ChargeList->GetRowCount() - 1; i >= 0; i--) {
			if(m_ChargeList->GetValue(i,CHARGE_COLUMN_BILL_ID).lVal != nBillID) {
				m_ChargeList->RemoveRow(i);
			}
		}

		m_bFilteredByBill = TRUE;

		EnableScreen();

		m_btnFilterByBillID.SetWindowText("Remove Bill ID Filter");

	}NxCatchAll("Error filtering by Bill ID");
}

BOOL CBatchPayments::PreTranslateMessage(MSG* pMsg) 
{
	switch (pMsg->message) {
	case WM_KEYDOWN:
		switch (pMsg->wParam) {
		case VK_RETURN:
			{
				if(GetFocus() == GetDlgItem(IDC_APPLY_LIST)) {
					long nCurSel = m_ChargeList->GetCurSel();

					if(nCurSel != -1) {
						OnDblClickCellApplyList(nCurSel,0);
					}
					return TRUE;
				}
			}
			break;
		// (j.jones 2012-08-20 17:20) - PLID 52152 - added tabbing ability in the charge list so it jumps to the next bill
		case VK_TAB:
			{
				//we don't do anything special unless focus is in the charge list
				if(GetFocus() == GetDlgItem(IDC_APPLY_LIST)) {
					BOOL bIsShiftKeyDown = (GetAsyncKeyState(VK_SHIFT) & 0x80000000);

					long nCurSel = m_ChargeList->GetCurSel();
					//don't interrupt the normal tab order if:
					//- no row is selected
					//- the first row is selected and they are shift-tabbing
					//- the last row is selected and they are tabbing normally
					if(nCurSel == -1 || (nCurSel == 0 && bIsShiftKeyDown)
						|| (nCurSel + 1 == m_ChargeList->GetRowCount() && !bIsShiftKeyDown)) {
						break;
					}
					else {
						//if we get here, the focus is on a charge row,
						//and the default tab or shift-tab action may need
						//to be overridden

						//find the next BillID
						long nBillID = VarLong(m_ChargeList->GetValue(m_ChargeList->GetCurSel(),CHARGE_COLUMN_BILL_ID), -1);
						if(!bIsShiftKeyDown) {
							//search down
							for(int i=nCurSel+1; i<m_ChargeList->GetRowCount(); i++) {
								long nBillIDToCheck = VarLong(m_ChargeList->GetValue(i,CHARGE_COLUMN_BILL_ID), -1);
								if(nBillIDToCheck != nBillID) {
									//found a new, unique bill, select it and disable normal tab behavior
									m_ChargeList->PutCurSel(i);
									return TRUE;
								}
							}

							//if we got here, there was no bill below this one, so tab normally
							break;
						}
						else {
							//search up
							for(int i=nCurSel-1; i>=0; i--) {
								long nBillIDToCheck = VarLong(m_ChargeList->GetValue(i,CHARGE_COLUMN_BILL_ID), -1);
								if(nBillIDToCheck != nBillID) {
									//found a new, unique bill, select it and disable normal tab behavior
									m_ChargeList->PutCurSel(i);
									return TRUE;
								}
							}

							//if we got here, there was no bill above this one, so tab normally
							break;
						}
					}
				}
			}
			break;
		default:
		break;
		}
	default:
		break;
	}	
	return CNxDialog::PreTranslateMessage(pMsg);
}

void CBatchPayments::OnCheckShowZeroBalancePayments() 
{
	try {

		if(m_checkShowPaysWithZeroBal.GetCheck()) {
			m_BatchPayments->WhereClause = "Type = 1";
			SetRemotePropertyInt("ShowZeroBalanceBatchPays",1,0,GetCurrentUserName());
		}
		else {
			// (j.jones 2012-02-13 15:33) - PLID 48121 - changed this to specifically hide zero balance payments,
			// but still show payments that have a negative balance
			m_BatchPayments->WhereClause = "RemainingAmount <> Convert(money,'$0.00') AND Type = 1";
			SetRemotePropertyInt("ShowZeroBalanceBatchPays",0,0,GetCurrentUserName());
		}

		long nID = -1;
		
		if(m_BatchPayments->GetCurSel() != -1)
			nID = m_BatchPayments->GetValue(m_BatchPayments->GetCurSel(),0).lVal;

		m_BatchPayments->Requery();

		if(nID != -1)
			m_BatchPayments->SetSelByColumn(0,nID);

		OnSelChosenBatchPaymentsCombo(m_BatchPayments->CurSel);		

	}NxCatchAll("Error filtering batch payment list.");
}

void CBatchPayments::OnBtnPostToPatient() 
{
	if(m_BatchPayments->CurSel == -1 || m_PatientCombo->CurSel == -1)
		return;

	CWaitCursor pWait;

	try {

		long nPatientID = VarLong(m_PatientCombo->GetValue(m_PatientCombo->CurSel,0));

		PostToPatient(nPatientID);

	}NxCatchAll("Error in OnBtnPostToPatient().");
}

void CBatchPayments::PostToPatient(long nPatientID)
{
	if(m_BatchPayments->CurSel == -1)
		return;

	try {

		// (j.jones 2008-04-30 14:14) - PLID 28358 - added apply permissions,
		// which should be checked even though we're not actually applying to a charge,
		// it's still considered to be like an apply in the context of batch payments
		// (j.jones 2011-08-24 17:39) - PLID 45176 - do not call CanApplyLineItem here,
		// because the line item in question has not actually been created, so we only
		// need to check the normal apply permission
		if(!CheckCurrentUserPermissions(bioApplies, sptCreate)) {
			return;
		}

		IRowSettingsPtr pBatchPaymentRow = m_BatchPayments->GetRow(m_BatchPayments->CurSel);
		COleDateTime dt = AsDateNoTime(VarDateTime(pBatchPaymentRow->GetValue(COMBO_COLUMN_DATE)));

		// (j.jones 2015-11-05 08:53) - PLID 65567 - check whether or not the user
		// is allowed to enter backdated payments
		if (!CanChangeHistoricFinancial_ByServiceDate("Payment", dt, TRUE)) {
			//the user will have already been told they can't save this payment
			return;
		}

		// (j.jones 2010-05-17 13:22) - PLID 33941 - try to prompt the patient warning first
		PromptPatientWarning(nPatientID);
				
		long nBatchPayID = pBatchPaymentRow->GetValue(COMBO_COLUMN_ID).lVal;

		// (z.manning, 03/22/2007) - PLID 25305 - Get the remaining balance from the datalist rather than recalculating it.
		COleCurrency cyPayBalance = VarCurrency(pBatchPaymentRow->GetValue(COMBO_COLUMN_BALANCE), COleCurrency(0,0));

		//calculate the insured party ID
		long nInsCoID = VarLong(m_BatchPayments->GetValue(m_BatchPayments->CurSel,COMBO_COLUMN_INSCO_ID),-1);
		long nHCFASetupGroupID = VarLong(m_BatchPayments->GetValue(m_BatchPayments->CurSel,COMBO_COLUMN_HCFA_GROUP_ID),-1);
		long nInsuredPartyID = -1;

		// (j.jones 2008-04-29 10:49) - PLID 29821 - this never properly handled inactive insurances, I fixed it, and parameterized
		_RecordsetPtr rs = CreateParamRecordset("SELECT PersonID FROM InsuredPartyT WHERE PatientID = {INT} AND InsuranceCoID = {INT} "
			"ORDER BY (CASE WHEN RespTypeID = -1 THEN 1 ELSE 0 END), RespTypeID ", nPatientID, nInsCoID);
		if(!rs->eof) {
			nInsuredPartyID = AdoFldLong(rs, "PersonID",-1);
		}
		rs->Close();

		//if not the exact same insurance co as the batch payment, try the hcfa group
		if(nInsuredPartyID == -1) {
			// (j.jones 2008-04-29 10:49) - PLID 29821 - this never properly handled inactive insurances, I fixed it, and parameterized
			rs = CreateParamRecordset("SELECT PersonID FROM InsuredPartyT WHERE PatientID = {INT} AND InsuranceCoID IN (SELECT PersonID FROM InsuranceCoT WHERE HCFASetupGroupID = {INT}) "
				"ORDER BY (CASE WHEN RespTypeID = -1 THEN 1 ELSE 0 END), RespTypeID ", nPatientID, nHCFASetupGroupID);
			if(!rs->eof) {
				nInsuredPartyID = AdoFldLong(rs, "PersonID",-1);
			}
			rs->Close();
		}

		//if still no insurance, then tell them they cannot post to the patient
		if(nInsuredPartyID == -1) {
			AfxMessageBox("The selected patient doesn't have an Insured Party that uses the selected payment's Insurance Company\n"
						  "nor a company in the same HCFA Group as the selected payment's Insurance Company.\n\n"
						  "You cannot apply a Batch Payment to a patient that does not have the insurance carrier from the payment,\n"
						  "unless you show all insurance responsibilities and apply directly to a specific insurance responsibility.");
			return;
		}

		COleCurrency cyAmount = COleCurrency(0,0);

		BOOL bDone = FALSE;
		while(!bDone) {
			
			CString strAmount = FormatCurrencyForInterface(cyPayBalance,TRUE,TRUE);

			if (InputBox(this, "Enter the amount to post a payment for:", strAmount, "") == IDOK) {

				if (strAmount.GetLength() == 0) {
					MsgBox("Please fill in an amount to post a payment for.");
					continue;
				}

				cyAmount = ParseCurrencyFromInterface(strAmount);
				if(cyAmount.GetStatus() == COleCurrency::invalid) {
					MsgBox("Please enter a valid amount to post a payment for.");
					continue;
				}

				if (cyAmount < COleCurrency::COleCurrency(0,0))
				{
					MsgBox("Payments cannot be negative. Please enter a positive payment amount.");
					continue;
				}

				//see how much the regional settings allows to the right of the decimal
				CString strICurr;
				NxGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ICURRDIGITS, strICurr.GetBuffer(3), 3, TRUE);
				int nDigits = atoi(strICurr);
				CString strDecimal;
				NxGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, strDecimal.GetBuffer(4), 4, TRUE);	
				if(cyAmount.Format().Find(strDecimal) != -1 && cyAmount.Format().Find(strDecimal) + (nDigits+1) < cyAmount.Format().GetLength()) {
					MsgBox("Please fill only %li places to the right of the %s in the amount box.",nDigits,strDecimal == "," ? "comma" : "decimal");
					continue;
				}

				if(cyAmount > cyPayBalance) {
					CString str;
					str.Format("You may not enter a payment amount greater than the batch payment's remaining balance of %s.",FormatCurrencyForInterface(cyPayBalance,TRUE,TRUE));
					MsgBox(str);
					continue;
				}

				//if we get here, the amount is solid
				bDone = TRUE;
			}
			else {
				return;
			}
		}

		CWaitCursor pWait;
		
		//////////////
		//make payment

		// (j.dinatale 2012-02-07 11:45) - PLID 51181 - removed unnecessary variables now that we are parameterizing
		long LocationID, PaymentGroupID, ProviderID;
		CString strDescription, strCheckNo;

		// (z.manning, 03/22/2007) - PLID 25305 - Pull these from the datalist instead of data
		LocationID = VarLong(pBatchPaymentRow->GetValue(COMBO_COLUMN_LOCATION));
		PaymentGroupID = VarLong(pBatchPaymentRow->GetValue(COMBO_COLUMN_PAY_CAT_ID), -1);
		strDescription = VarString(pBatchPaymentRow->GetValue(COMBO_COLUMN_DESCRIPTION), "(no description)");
		ProviderID = VarLong(pBatchPaymentRow->GetValue(COMBO_COLUMN_PROVIDER_ID), -1);
		strCheckNo = VarString(pBatchPaymentRow->GetValue(COMBO_COLUMN_CHECK_NO), "");


		// (j.dinatale 2012-02-07 09:42) - PLID 51181 - parameterize while we are at it.
		long nProviderID = ProviderID;

		if(ProviderID != -1) {
			// (j.dinatale 2012-02-07 09:43) - PLID 51181 - dont need this here anymore
			//strProviderID.Format("%li",ProviderID);
		}
		else {
			// (j.jones 2005-07-06 10:17) - if no provider is selected on the 
			// batch payment, attempt to assign the patient's provider to the child payment
			// (j.dinatale 2012-02-07 09:43) - PLID 51181 - dont need this here anymore
			//strProviderID = "NULL";

			_RecordsetPtr rsDoc = CreateParamRecordset("SELECT MainPhysician FROM PatientsT WHERE PersonID = {INT}",nPatientID);
			if(!rsDoc->eof) {
				long nProvID = AdoFldLong(rsDoc, "MainPhysician",-1);
				if(nProvID > -1) {
					nProviderID = nProvID;
				}
			}
			rsDoc->Close();
		}

		// (j.dinatale 2012-02-07 09:42) - PLID 51181 - parameterize while we are at it.
		//strDate = FormatDateTimeForSql(dt, dtoDate);
		//strAmount = FormatCurrencyForSql(cyAmount);

		CSqlFragment sqlFrag(
			"SET NOCOUNT ON\r\n"
			"DECLARE @nLineItemID INT\r\n");

		// (j.jones 2011-09-29 14:55) - PLID 45500 - ensured that InputDate uses GetDate(), NOT BatchPaymentsT.InputDate
		// (j.armen 2013-06-28 12:47) - PLID 57373 - Idenitate LineItemT
		sqlFrag += CSqlFragment(
			"INSERT INTO LineItemT (\r\n"
			"	PatientID, LocationID, Type, Date, InputDate, InputName, Amount, Description\r\n"
			") VALUES ("
			"	{INT}, {INT}, 1, {OLEDATETIME}, GetDate(), {STRING}, {OLECURRENCY}, {STRING}) \r\n", 
			nPatientID, LocationID, dt, GetCurrentUserName(), cyAmount, strDescription);

		sqlFrag += CSqlFragment(
			"SET @nLineItemID = SCOPE_IDENTITY() \r\n");

		// (j.armen 2013-06-29 15:34) - PLID 57375 - PaymentsT.PaymentUniqueID now gets it's ID from an identity seeded table
		sqlFrag += 
			"DECLARE @nPaymentUniqueID INT\r\n"
			"INSERT INTO PaymentUniqueT DEFAULT VALUES\r\n"
			"SET @nPaymentUniqueID = SCOPE_IDENTITY() \r\n\r\n";

		// (j.jones 2007-03-28 10:57) - PLID 25385 - supported CashReceived
		sqlFrag += CSqlFragment("INSERT INTO PaymentsT (ID, InsuredPartyID, ProviderID, PaymentGroupID, PayMethod, BatchPaymentID, PaymentUniqueID, CashReceived) VALUES "
			"(@nLineItemID, {INT}, {VT_I4}, {INT}, 2, {INT}, @nPaymentUniqueID, {OLECURRENCY}) \r\n",
			nInsuredPartyID, ((nProviderID == -1) ? g_cvarNull : _variant_t(nProviderID)), PaymentGroupID, nBatchPayID, cyAmount);

		//(e.lally 2006-08-10) PLID 21907 - We need add the _Q to check no.. We should also put these queries into a batch in case one fails.
		sqlFrag += CSqlFragment(
			"INSERT INTO PaymentPlansT (ID, CheckNo) VALUES (@nLineItemID, {STRING}) \r\n", strCheckNo);

		// (j.dinatale 2012-02-06 13:15) - PLID 51181 - get the line item ID that we just inserted
		sqlFrag += 
			"SELECT @nLineItemID AS LineItemID \r\n"
			"SET NOCOUNT OFF \r\n\r\n";
 
		_RecordsetPtr rsLineItem = CreateParamRecordset(sqlFrag);

		long iLineItemID = -1;
		if(!rsLineItem->eof){
			iLineItemID = AdoFldLong(rsLineItem, "LineItemID");
		}

		long nAuditID = BeginNewAuditEvent();
		CString strAuditDesc;
		strAuditDesc.Format("%s Payment", FormatCurrencyForInterface(cyAmount, TRUE, TRUE));
		AuditEvent(nPatientID, GetExistingPatientName(nPatientID), nAuditID, aeiPaymentCreated, iLineItemID, "", strAuditDesc, aepHigh, aetCreated);

		AutoUpdateBatchPaymentDepositDates(nBatchPayID);

		//check to see if this payment is done
		COleCurrency cyRem = CalculateRemainingBatchPaymentBalance(nBatchPayID);
		// (j.jones 2014-07-28 08:28) - PLID 63065 - changed this to only apply to zero dollar balances,
		// it should not apply if the balance is negative
		if(cyRem == COleCurrency(0,0) && !m_checkShowPaysWithZeroBal.GetCheck()) {
			m_BatchPayments->RemoveRow(m_BatchPayments->CurSel);
			// (j.jones 2015-10-19 16:21) - PLID 67309 - call RequeryFinished to recalculate the capitation column
			OnRequeryFinishedBatchPaymentsCombo(dlRequeryFinishedCompleted);
			m_BatchPayments->CurSel = -1;
			MessageBox("The selected batch payment has been completely applied and will be removed from the list.");
			OnSelChosenBatchPaymentsCombo(-1);
		}
		else {
			m_BatchPayments->PutValue(m_BatchPayments->CurSel,COMBO_COLUMN_BALANCE,_variant_t(cyRem));

			CString strRem = FormatCurrencyForInterface(cyRem);
			SetDlgItemText(IDC_REM_AMOUNT,strRem);
		}

		// (z.manning, 03/23/2007) - There is absolutely no reason to requery the charge list here.
	
	}NxCatchAll("Error posting payment to patient.");
}

void CBatchPayments::OnBtnNewAdjustment() 
{
	try {

		if(m_BatchPayments->CurSel == -1) {
			AfxMessageBox("Please select a payment from the list to adjust.");
			return;
		}

		// (j.jones 2008-04-30 14:52) - PLID 28358 - added check for create permissions
		if(!CheckCurrentUserPermissions(bioAdjustment, sptCreate)) {
			return;
		}

		long PaymentID = m_BatchPayments->GetValue(m_BatchPayments->CurSel,0).lVal;

		// (j.jones 2014-04-21 09:16) - PLID 61740 - we no longer disallow adjusting zero dollar payments,
		// we instead only disallow this on zero dollar balances
		/*
		if(ReturnsRecords("SELECT ID FROM BatchPaymentsT WHERE Amount = Convert(money,0) AND ID = %li",PaymentID)) {
			AfxMessageBox("You cannot adjust a zero dollar payment.");
			return;
		}
		*/

		CBatchPaymentEditDlg dlg(this);

		dlg.m_nType = 2;
		// (j.jones 2014-06-26 17:29) - PLID 62546 - set the PayType to NoPayment
		dlg.m_ePayType = eNoPayment;
		dlg.m_nAppliedPayID = PaymentID;
		// (z.manning, 03/22/2007) - PLID 25305 - Get the balance from the datalist rather than recalculating it.
		dlg.m_cyMaxApplyAmt = VarCurrency(m_BatchPayments->GetValue(m_BatchPayments->CurSel,COMBO_COLUMN_BALANCE), COleCurrency(0,0));

		if(dlg.m_cyMaxApplyAmt == COleCurrency(0,0)) {
			AfxMessageBox("You cannot adjust a payment with a zero balance.");
			return;
		}

		if(IDOK == dlg.DoModal()) {

			m_BatchPayments->Requery();

			long row = m_BatchPayments->SetSelByColumn(0,PaymentID);

			OnSelChosenBatchPaymentsCombo(row);
		}

	}NxCatchAll("CBatchPayments::OnBtnNewAdjustment"); // (z.manning, 03/27/2007) - PLID 25357
}

void CBatchPayments::UpdateChargeListByChargeID(long nChargeID)
{
	if(m_BatchPayments->CurSel < 0) {
		// (z.manning, 03/23/2007) - PLID 25323 - If we don't have a selected batch payment, we shouldn't
		// be updating anything.
		return;
	}

	// (z.manning, 03/23/2007) - PLID 25323 - First remove all rows for this charge ID.
	long nCurrentRow = m_ChargeList->FindByColumn(CHARGE_COLUMN_CHARGE_ID, nChargeID, 0, FALSE);
	while(nCurrentRow != -1) {
		m_ChargeList->RemoveRow(nCurrentRow);
		nCurrentRow = m_ChargeList->FindByColumn(CHARGE_COLUMN_CHARGE_ID, nChargeID, nCurrentRow, FALSE);
	}

	// (z.manning, 03/23/2007) - PLID 25323 - Now generate the query to reload the rows for this charge ID.
	CString strWhere = (LPCTSTR)(m_ChargeList->WhereClause);
	if (!strWhere.IsEmpty()) {
		strWhere = " AND " + strWhere;
	}

	CString strQuery = FormatString("SELECT * FROM %s WHERE ChargeID = %li %s"
		, (LPCTSTR)(m_ChargeList->FromClause), nChargeID, strWhere);

	AddChargesToListFromQuery(strQuery);
}

void CBatchPayments::UpdateChargeListByBillID(long nBillID)
{
	if(m_BatchPayments->CurSel < 0) {
		// (z.manning, 03/23/2007) - PLID 25323 - If we don't have a selected batch payment, we shouldn't
		// be updating anything.
		return;
	}

	// (z.manning, 03/23/2007) - PLID 25323 - First remove all rows for this bill ID.
	long nCurrentRow = m_ChargeList->FindByColumn(CHARGE_COLUMN_BILL_ID, nBillID, 0, FALSE);
	while(nCurrentRow != -1) {
		m_ChargeList->RemoveRow(nCurrentRow);
		nCurrentRow = m_ChargeList->FindByColumn(CHARGE_COLUMN_BILL_ID, nBillID, nCurrentRow, FALSE);
	}

	// (z.manning, 03/23/2007) - PLID 25323 - Now generate the query to reload the rows for this bill ID.
	CString strWhere = (LPCTSTR)(m_ChargeList->WhereClause);
	if (!strWhere.IsEmpty()) {
		strWhere = " AND " + strWhere;
	}
	
	CString strQuery = FormatString("SELECT * FROM %s WHERE BillID = %li %s"
		, (LPCTSTR)(m_ChargeList->FromClause), nBillID, strWhere);

	AddChargesToListFromQuery(strQuery);
}

void CBatchPayments::AddChargesToListFromQuery(CString strQuery)
{
	// (z.manning, 03/23/2007) - PLID 25323 - Reload all applicable charges.
	_RecordsetPtr prsCharges = CreateRecordset("%s", strQuery);
	FieldsPtr flds = prsCharges->Fields;

	while(!prsCharges->eof)
	{
		IRowSettingsPtr pNewRow = m_ChargeList->GetRow(-1);

		// (z.manning, 03/23/2007) - PLID 25323 - Set the values for each column in the charge datalist.
		short nTotalColumns = m_ChargeList->GetColumnCount();
		for(short nColumn = 0; nColumn < nTotalColumns; nColumn++) {
			pNewRow->PutValue(nColumn, flds->GetItem((LPCTSTR)(m_ChargeList->GetColumn(nColumn)->GetFieldName()))->Value);
		}

		m_ChargeList->InsertRow(pNewRow, 0);

		prsCharges->MoveNext();
	}

	m_ChargeList->Sort();

	// (j.jones 2015-10-30 08:58) - PLID 67431 - shifting balances can cause this function
	// to be called on unlocked capitation charges, if so we have to recalculate capitation
	if (IsCapitationPayment() && !IsLockedCapitationPayment()) {
		
		CalculateCapitation();

		//There is a very unlikely possibility that a reimbursement rate was previously calculated,
		//but reloading this charge causes no estimated payment to be possible.
		//The remaining charges will make it appear like you can post capitation, but you can't.
		//This is an acceptable trade-off from a full requery.

		// (b.eyers 2015-10-21) - PLID 67361
		IRowSettingsPtr pRow;
		bool bHasMissingAllowables = false;
		for (int i = 0; i < m_ChargeList->GetRowCount(); i++) {
			pRow = m_ChargeList->GetRow(i);
			bool bIsAllowable = VarBool(m_ChargeList->GetValue(i, CHARGE_COLUMN_IS_ALLOWABLE), FALSE) ? true : false;
			if (!bIsAllowable) {
				pRow->PutCellBackColor(CHARGE_COLUMN_ALLOWABLE, RGB(255, 0, 0));
				bHasMissingAllowables = true;
			}
		}

	}
}

// (j.jones 2007-03-27 14:34) - PLID 23987 - added ability to show
//all insurance responsibilities for the selected patient
void CBatchPayments::OnCheckShowAllPatientInsResps() 
{
	try {

		if(m_checkShowAllPatientInsResps.GetCheck()) {

			//warn about the potential folly of using this feature
			// (j.jones 2011-11-02 14:15) - PLID 38686 - this option now also shows patient resps, so I tweaked
			// the wording to reflect that
			DontShowMeAgain(this, "When showing all insurance responsibilities, you can apply a batch payment to a\n"
				"responsibility for a different insurance company, or to patient responsibility.\n\n"
				"Be careful that you are selecting the correct responsibilty when applying.", "BatchPaymentsShowAllInsResps", "Practice", FALSE, FALSE);
		}

		ChangePatientSelectionType();

	}NxCatchAll("CBatchPayments::OnCheckShowAllPatientInsResps");
}

// (j.jones 2007-05-10 17:34) - PLID 25973 - supported OnTableChanged
LRESULT CBatchPayments::OnTableChanged(WPARAM wParam, LPARAM lParam)
{
	try {
		switch(wParam) {
			// (j.jones 2007-05-10 17:34) - PLID 25973 - supported the patient list tablechecker
		case NetUtils::PatCombo:{

			// (s.tullis 2014-08-12 16:53) - PLID 63240 - Batch Payments needs a CTableChecker object for PatCombo.
			EnsurePatComboUpdated();
			break;
							}
						}
	}NxCatchAll("Error in CBatchPayments::OnTableChanged()");
	return 0;
					}

//TES 8/13/2014 - PLID 63520 - Added support for EX tablecheckers
LRESULT CBatchPayments::OnTableChangedEx(WPARAM wParam, LPARAM lParam)
{
	try {
		switch (wParam) {
		case NetUtils::PatCombo:
		{
			EnsurePatComboUpdated();
			break;
					}
			}
	}NxCatchAll(__FUNCTION__);
	return 0;
}

// (j.jones 2009-06-10 15:56) - PLID 30478 - added ability to unapply an EOB
// (b.eyers 2015-10-14) - PLID 67308 - renamed function
void CBatchPayments::OnBtnUnapplyBatchPayment()
{
	long nAuditTransactionID = -1;

	try {

		//get our batch payment ID
		long nCurSel = m_BatchPayments->GetCurSel();
		if(nCurSel == -1) {
			AfxMessageBox("There is no batch payment selected.");
			GetDlgItem(IDC_BTN_UNAPPLY_EOB)->EnableWindow(FALSE);
			return;
		}

		IRowSettingsPtr pRow = m_BatchPayments->GetRow(nCurSel);
		long nBatchPaymentID = VarLong(pRow->GetValue(COMBO_COLUMN_ID));
		
		// (b.eyers 2015-10-19) - PLID 67308 - keep track if this is eob or not
		bool bIsEob = VarBool(pRow->GetValue(COMBO_COLUMN_IS_EOB), FALSE) ? true : false;

		//make sure our selected batch payment is an EOB
		// (b.eyers 2015-10-14) - PLID 67308 - no longer needed since normal batch payments can unapply now
		/*if(!VarBool(pRow->GetValue(COMBO_COLUMN_IS_EOB))) {
			AfxMessageBox("The selected batch payment is not an E-Remittance payment.");
			GetDlgItem(IDC_BTN_UNAPPLY_EOB)->EnableWindow(FALSE);
			return;
		}*/
		
		//get data for auditing
		COleDateTime dtBatchPayDate = VarDateTime(pRow->GetValue(COMBO_COLUMN_DATE));
		CString strInsuranceCoName = VarString(pRow->GetValue(COMBO_COLUMN_INSCO_NAME));
		CString strBatchPayDesc = VarString(pRow->GetValue(COMBO_COLUMN_DESCRIPTION));
		COleCurrency cyBatchPayAmt = VarCurrency(pRow->GetValue(COMBO_COLUMN_AMOUNT));

		CString strOldValue;
		strOldValue.Format("Amount: %s, Description: %s, Payment Date: %s",
			FormatCurrencyForInterface(cyBatchPayAmt), strBatchPayDesc, FormatDateTimeForInterface(dtBatchPayDate, NULL, dtoDate));

		// (b.eyers 2015-10-19) - PLID 67308 - only check and warn if this is an eob
		if (bIsEob) {
			//if anything is applied to this batch payment, stop this action
			_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 ID FROM BatchPaymentsT WHERE Deleted = 0 AND AppliedBatchPayID = {INT}", nBatchPaymentID);
			if (!rs->eof) {
				AfxMessageBox("This batch payment currently has applied adjustments or refunds, and cannot be deleted.\n"
					"Please delete these applies from the \"View Refunded / Adjusted Payments\" screen before unapplying this posting.");
				return;
			}
			rs->Close();
		}

		//ok, we're allowed to undo this eob - so find out how many payments and adjustments
		//will be affected by this deletion, and warn heavily regarding the outcome

		// (j.jones 2011-01-19 11:43) - PLID 42149 - Also calculate the earliest Apply date
		// that this payment or adjustment is applied to a charge.
		// It is impossible for anything to be applied to that payment or adjustment with an
		// earlier date, so ignore those applies.
		_RecordsetPtr rsTotals = CreateParamRecordset("SELECT Max(CloseDate) AS HardCloseDate FROM FinancialCloseHistoryT "
			""
			"SELECT InputDate FROM BatchPaymentsT WHERE ID = {INT} "
			""
			"SELECT "
			"Sum(CASE WHEN Type = 1 THEN LineItemT.Amount ELSE Convert(money,0) END) AS PaymentTotal, "
			"Sum(CASE WHEN Type = 2 THEN LineItemT.Amount ELSE Convert(money,0) END) AS AdjustmentTotal, "
			"Sum(CASE WHEN Type = 1 THEN 1 ELSE 0 END) AS PaymentCount, "
			"Sum(CASE WHEN Type = 2 THEN 1 ELSE 0 END) AS AdjustmentCount "
			"FROM PaymentsT "
			"INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
			"WHERE BatchPaymentID = {INT} AND LineItemT.Deleted = 0", nBatchPaymentID, nBatchPaymentID);

		//first see if there is a hard close
		COleDateTime dtHardClose = g_cdtInvalid;
		if (!rsTotals->eof) {
			//this is nullable since this is a Max()
			dtHardClose = AdoFldDateTime(rsTotals, "HardCloseDate", g_cdtInvalid);
		}

		//now get the batch payment's input date
		rsTotals = rsTotals->NextRecordset(NULL);

		BOOL bCanOverridePerms = FALSE;

		// (b.eyers 2015-10-19) - PLID 67308 - only check and warn if this is an eob
		if (!rsTotals->eof && dtHardClose.GetStatus() != COleDateTime::invalid && bIsEob) {
			//there is a closed date, compare it to the batch payment's input date
			COleDateTime dtInputDate = AdoFldDateTime(rsTotals, "InputDate");
			if (dtInputDate <= dtHardClose) {
				//this is closed, check the permission

				ECanOverrideHardClose eCanOverride = cohcNo;
				if (GetCurrentUserPermissions(bioUnapplyBatchPaymentPastClose) & sptDynamic0) { // (b.eyers 2015-10-14) - PLID 67308 - renamed permission
					//they can write normally
					eCanOverride = cohcYes;
				}
				else if (GetCurrentUserPermissions(bioUnapplyBatchPaymentPastClose) & sptDynamic0WithPass) { // (b.eyers 2015-10-14) - PLID 67308 - renamed permission
					//they can write normally
					eCanOverride = cohcWithPass;
				}

				//if they can't bypass it, warn and quit
				if (eCanOverride == cohcNo) {
					MsgBox(MB_OK | MB_ICONERROR, "This E-Remittance posting cannot be unapplied because the input date of %s "
						"occurs before the most recent closing date of %s.", FormatDateTimeForInterface(dtInputDate, DTF_STRIP_SECONDS, dtoDateTime), FormatDateTimeForInterface(dtHardClose, DTF_STRIP_SECONDS, dtoDateTime));
					return;
				}

				//if they can bypass it, warn first
				CString strWarn;
				strWarn.Format("The %s input date of this E-Remittance posting occurs before the most recent closing date of %s.\n\n"
					"Are you absolutely certain you wish to unapply this E-Remittance posting?", FormatDateTimeForInterface(dtInputDate, DTF_STRIP_SECONDS, dtoDateTime), FormatDateTimeForInterface(dtHardClose, DTF_STRIP_SECONDS, dtoDateTime));
				if (IDNO == MessageBox(strWarn, "Practice", MB_ICONEXCLAMATION | MB_YESNO)) {
					return;
				}

				//check the password if needed
				if (eCanOverride == cohcWithPass && !CheckCurrentUserPassword()) {
					//fail, but clarify why we failed
					MsgBox(MB_OK | MB_ICONERROR, "You must enter your password in order to unapply this closed posting.");
					return;
				}

				//if they can unapply the closed EOB, track that
				bCanOverridePerms = TRUE;
			}
		}


		// (j.jones 2011-01-19 13:34) - PLID 42149 - moved the permission check to here
		// so that if they already have permission to unapply after the close, their
		// regular unapply permission is irrelevant (mainly this is to reduce the possibility
		// of entering a password twice)
		if (!bCanOverridePerms && !CheckCurrentUserPermissions(bioUnapplyBatchPayment, sptDynamic0)) { // (b.eyers 2015-10-14) - PLID 67308 - renamed permission
			return;
		}

		//now we can load the totals for our scary warning
		rsTotals = rsTotals->NextRecordset(NULL);

		//in the atypical case where no child payments/adjustments will be undone,
		//have a different, less scary warning by default
		CString strWarning = "This batch payment is not in use on any patient accounts. "
				"Undoing the E-Remittance posting will simply delete the selected batch payment.\n\n"
				"Are you sure you wish to delete this batch payment?";

		if(!rsTotals->eof) {

			COleCurrency cyPaymentTotal = AdoFldCurrency(rsTotals, "PaymentTotal", COleCurrency(0,0));
			COleCurrency cyAdjustmentTotal = AdoFldCurrency(rsTotals, "AdjustmentTotal", COleCurrency(0,0));
			long nPaymentCount = AdoFldLong(rsTotals, "PaymentCount", 0);
			long nAdjustmentCount = AdoFldLong(rsTotals, "AdjustmentCount", 0);

			if(nPaymentCount > 0 || nAdjustmentCount > 0) {
				//give a big scary warning
				// (j.jones 2011-01-07 14:29) - PLID 41785 - clarified that billing notes are not deleted
				// (b.eyers 2015-10-19) - PLID 67308 - eob warning and batch warnings are different
				if (bIsEob) {
					strWarning.Format("Warning: Undoing this E-Remittance posting will delete the batch payment and "
						"the following information off of patient accounts:\n\n"
						"- %li payments totalling %s\n"
						"- %li adjustments totalling %s\n\n"
						"Any billing notes that may have been created from this posting will not be deleted.\n\n"
						"Are you absolutely sure you wish to undo this E-Remittance posting?",
						nPaymentCount, FormatCurrencyForInterface(cyPaymentTotal),
						nAdjustmentCount, FormatCurrencyForInterface(cyAdjustmentTotal));
				}
				else {
					strWarning.Format("Warning: Undoing this batch payment posting will unapply the batch payment and "
						"delete the following information off of patient accounts:\n\n"
						"- %li payments totalling %s\n"
						"- %li adjustments totalling %s\n\n"
						"Any billing notes that may have been created from this posting will not be deleted.\n\n"
						"Are you absolutely sure you wish to undo this batch payment posting?",
						nPaymentCount, FormatCurrencyForInterface(cyPaymentTotal),
						nAdjustmentCount, FormatCurrencyForInterface(cyAdjustmentTotal));
				}
			}
			else if (nPaymentCount == 0 && nAdjustmentCount == 0 && !bIsEob) {

				// (j.jones 2015-10-20 11:08) - PLID 67385 - It's not typical that a capitation
				// payment will be locked but have no payments, but the lock needs cleared regardless.
				// Do so even if the interface thinks it is unlocked.
				if (ReturnsRecordsParam("SELECT TOP 1 BatchPaymentID "
					" FROM BatchPaymentCapitationDetailsT "
					" WHERE BatchPaymentID = {INT}", nBatchPaymentID)) {

					ExecuteParamSql("DELETE FROM BatchPaymentCapitationDetailsT WHERE BatchPaymentID = {INT}", nBatchPaymentID);
					AuditEvent(-1, strInsuranceCoName, BeginNewAuditEvent(), aeiUnapplyBatchPayment, nBatchPaymentID, strOldValue, "<Posting Unapplied>", aepHigh);

					//refresh the screen
					UpdateView(true);

					//give a unique message
					AfxMessageBox("The capitation payment has been unlocked. There were no payments or adjustments to unapply.");
				}
				else {
					AfxMessageBox("There are no payments or adjustments to unapply.");
				}

				return;
			}
		}
		rsTotals->Close();

		if(IDNO == MessageBox(strWarning, "Practice", MB_ICONEXCLAMATION|MB_YESNO)) {
			return;
		}

		//alright, they asked for it, let's blow it away

		CWaitCursor pWait;

		CString strSqlBatch;
		CNxParamSqlArray aryParams;

		//we're going to audit *everything* we undo, so we need to cycle through
		//each apply, payment, and adjustment, then delete the batch payment
		// (j.jones 2012-04-24 12:42) - PLID 49804 - we now also undo corrections that takeback into this batch payment
		_RecordsetPtr rs = CreateParamRecordset("SELECT AppliesT.SourceID, SourceItem.Type AS SourceType, "
			"AppliesT.DestID, DestItem.Type AS DestType, "
			"AppliesT.Amount, DestItem.PatientID "
			"FROM AppliesT "
			"LEFT JOIN LineItemT SourceItem ON AppliesT.SourceID = SourceItem.ID "
			"LEFT JOIN PaymentsT SourcePaymentsT ON SourceItem.ID = SourcePaymentsT.ID "
			"LEFT JOIN LineItemT DestItem ON AppliesT.DestID = DestItem.ID "
			"LEFT JOIN PaymentsT DestPaymentsT ON DestItem.ID = DestPaymentsT.ID "
			"WHERE (SourcePaymentsT.BatchPaymentID = {INT} AND SourceItem.Deleted = 0) "
			"OR (DestPaymentsT.BatchPaymentID = {INT} AND DestItem.Deleted = 0) "
			""
			"SELECT LineItemT.ID, LineItemT.PatientID, LineItemT.Type, "
			"LineItemT.Date, LineItemT.Amount, LineItemT.Description "
			"FROM LineItemT "
			"INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			"WHERE BatchPaymentID = {INT} AND LineItemT.Deleted = 0 "
			""
			"SELECT LineItemCorrectionsT.ID, LineItemCorrectionsT.InputDate, "
			"LineItemCorrectionsT.OriginalLineItemID, LineItemCorrectionsT.VoidingLineItemID, LineItemCorrectionsT.NewLineItemID, "
			"LineItemT.PatientID, LineItemT.Type "
			"FROM LineItemCorrectionsT "
			"INNER JOIN LineItemT ON LineItemCorrectionsT.OriginalLineItemID = LineItemT.ID "
			"WHERE LineItemCorrectionsT.BatchPaymentID = {INT} "
			"GROUP BY LineItemCorrectionsT.ID, LineItemCorrectionsT.InputDate, "
			"LineItemCorrectionsT.OriginalLineItemID, LineItemCorrectionsT.VoidingLineItemID, LineItemCorrectionsT.NewLineItemID, "
			"LineItemT.PatientID, LineItemT.Type",
			nBatchPaymentID, nBatchPaymentID, nBatchPaymentID, nBatchPaymentID);

		//first audit all the applies
		while(!rs->eof) {

			if(nAuditTransactionID == -1) {
				nAuditTransactionID = BeginAuditTransaction();
			}

			// (j.jones 2009-06-11 11:00) - the following apply auditing is identical to how it audits in the billing tab

			long nPatientID = AdoFldLong(rs, "PatientID", -1);
			COleCurrency cyAmount = AdoFldCurrency(rs, "Amount",COleCurrency(0,0));
			CString strAmount;
			strAmount.Format("%s", FormatCurrencyForInterface(cyAmount,TRUE,TRUE));
			strAmount.Replace("(","");
			strAmount.Replace(")","");
			long nSourceID = AdoFldLong(rs, "SourceID", -1);
			long nSourceType = AdoFldLong(rs, "SourceType", -1);
			long nDestID = AdoFldLong(rs, "DestID", -1);
			long nDestType = AdoFldLong(rs, "DestType", -1);
			
			//we audit for both the source and destination
			// (j.jones 2011-03-21 17:55) - PLID 24273 - these are now unique audits for the EOB removal process
			AuditEventItems aeiSourceType;
			switch(nSourceType) {
			case 1:
				aeiSourceType = aeiPaymentUnappliedByBatchPaymentDeletion; // (b.eyers 2015-10-14) - PLID 67308 - renamed
				break;
			case 2:
				aeiSourceType = aeiAdjustmentUnappliedByBatchPaymentDeletion; // (b.eyers 2015-10-14) - PLID 67308 - renamed
				break;
			case 3:
				aeiSourceType = aeiRefundUnappliedByBatchPaymentDeletion; // (b.eyers 2015-10-14) - PLID 67308 - renamed
				break;
			default:
				AfxThrowNxException("Bad Source Type %li found while auditing unapply!", nSourceType);
				break;
			}
			AuditEvent(nPatientID, GetExistingPatientName(nPatientID), nAuditTransactionID, aeiSourceType, nSourceID, "", strAmount + " Unapplied", aepHigh, aetDeleted);
			
			// (j.jones 2011-03-21 17:55) - PLID 24273 - these are now unique audits for the EOB removal process
			AuditEventItems aeiDestType;
			switch(nDestType) {
				case 10:
					aeiDestType = aeiItemUnappliedFromChargeByBatchPaymentDeletion; // (b.eyers 2015-10-14) - PLID 67308 - renamed
					break;
				case 1:
					aeiDestType = aeiItemUnappliedFromPaymentByBatchPaymentDeletion; // (b.eyers 2015-10-14) - PLID 67308 - renamed
					break;
				case 2:
					aeiDestType = aeiItemUnappliedFromAdjustmentByBatchPaymentDeletion; // (b.eyers 2015-10-14) - PLID 67308 - renamed
					break;
				case 3:
					aeiDestType = aeiItemUnappliedFromRefundByBatchPaymentDeletion; // (b.eyers 2015-10-14) - PLID 67308 - renamed
					break;
				default:
					AfxThrowNxException("Bad Dest Type %li found while auditing unapply!", nDestType);
				break;
			}
			AuditEvent(nPatientID, GetExistingPatientName(nPatientID), nAuditTransactionID, aeiDestType, nDestID, "", "Applies reduced by " + strAmount, aepHigh, aetDeleted);

			rs->MoveNext();
		}

		rs = rs->NextRecordset(NULL);

		//now audit all the payments & adjustments
		while(!rs->eof) {

			if(nAuditTransactionID == -1) {
				nAuditTransactionID = BeginAuditTransaction();
			}

			// (j.jones 2009-06-11 11:00) - the following payment/adjustment auditing is identical to how it audits in the billing tab

			AuditEventItems aeiItem = aeiPaymentDeleted;

			long nID = AdoFldLong(rs, "ID", -1);
			long nPatientID = AdoFldLong(rs, "PatientID", -1);

			CString desc;
			int Type = rs->Fields->Item["Type"]->Value.lVal;
			CString strType;
			// (j.jones 2011-03-21 17:55) - PLID 24273 - these are now unique audits for the EOB removal process
			if(Type == 3) {
				strType = "Refund";
				aeiItem = aeiRefundDeletedByBatchPaymentDeletion; // (b.eyers 2015-10-14) - PLID 67308 - renamed
			}
			else if(Type == 2) {
				strType = "Adjustment";
				aeiItem = aeiAdjustmentDeletedByBatchPaymentDeletion; // (b.eyers 2015-10-14) - PLID 67308 - renamed
			}
			else {
				strType = "Payment";
				aeiItem = aeiPaymentDeletedByBatchPaymentDeletion; // (b.eyers 2015-10-14) - PLID 67308 - renamed
			}
			COleDateTime date = rs->Fields->Item["Date"]->Value.date;
			CString strAmount = FormatCurrencyForInterface(rs->Fields->Item["Amount"]->Value.cyVal);
			CString strDescription = AdoFldString(rs, "Description","");
			desc.Format("%s %s, %s, %s",strAmount,strType,FormatDateTimeForInterface(date, NULL, dtoDate),_Q(strDescription));

			AuditEvent(nPatientID, GetExistingPatientName(nPatientID), nAuditTransactionID, aeiItem, nID, desc, "<Deleted>", aepHigh, aetDeleted);

			rs->MoveNext();
		}

		// (j.jones 2012-04-24 12:42) - PLID 49804 - we now also undo corrections that takeback into this batch payment
		rs = rs->NextRecordset(NULL);

		CFinancialCorrection finCor;
		BOOL bHasCorrections = FALSE;
		std::vector<long> aryAppliesToUndo;

		while(!rs->eof) {

			if(nAuditTransactionID == -1) {
				nAuditTransactionID = BeginAuditTransaction();
			}

			long nCorrectionID_ToUndo = VarLong(rs->Fields->Item["ID"]->Value);
			COleDateTime dtCorrectionInputDate = VarDateTime(rs->Fields->Item["InputDate"]->Value);
			long nCorrectionOriginalID = VarLong(rs->Fields->Item["OriginalLineItemID"]->Value);
			long nCorrectionVoidingID = VarLong(rs->Fields->Item["VoidingLineItemID"]->Value);
			long nCorrectionNewID = VarLong(rs->Fields->Item["NewLineItemID"]->Value, -1);	//should be null
			long nCorrectionPatientID = VarLong(rs->Fields->Item["PatientID"]->Value);
			long nType = VarLong(rs->Fields->Item["Type"]->Value);
			CString strLineItemTypeName = "payment";
			if (nType == 2) {
				strLineItemTypeName = "adjustment";
			}

			//The only corrections tied to batch payments should be voided payments/adjustments only, with no new correction.
			//Fail if this assumption is not true.
			if(nCorrectionNewID != -1) {
				ThrowNxException("Financial correction %li is tied to this batch payment with a corrected line item in use.", nCorrectionID_ToUndo); // (b.eyers 2015-10-14) - PLID 67308 - update text to batch payments
			}
			else if(!ReturnsRecordsParam("SELECT PaymentsT.ID FROM PaymentsT "
				"INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
				"WHERE PaymentsT.ID = {INT} AND LineItemT.Type IN (1,2)", nCorrectionOriginalID)) {
				ThrowNxException("Financial correction %li is tied to this batch payment with an original line item that is not a payment or adjustment.", nCorrectionID_ToUndo); // (b.eyers 2015-10-14) - PLID 67308 - updated text to batch payments
			}

			// (j.jones 2015-06-16 10:50) - PLID 50008 - add a check to make sure that undoing the correction
			// will not result in invalid applies			
			if (!UndoCorrection_CheckInvalidApplies(strLineItemTypeName, nCorrectionOriginalID, nCorrectionVoidingID, nCorrectionNewID, true, aryAppliesToUndo)) {
				//if this returned false, cancel the undo process
				return;
			}

			finCor.AddCorrectionToUndo(cutUndoPayment, nCorrectionID_ToUndo, dtCorrectionInputDate, nCorrectionOriginalID, nCorrectionVoidingID, nCorrectionNewID, nCorrectionPatientID);
			bHasCorrections = TRUE;

			rs->MoveNext();
		}

		rs->Close();

		if(nAuditTransactionID == -1) {
			nAuditTransactionID = BeginAuditTransaction();
		}

		//audit deleting the batch payment

		// (b.eyers 2015-10-19) - PLID 67308 - only audit the delete if eob
		if (bIsEob)
		// (j.jones 2011-03-21 17:55) - PLID 24273 - these are now unique audits for the EOB removal process
			AuditEvent(-1, strInsuranceCoName, nAuditTransactionID, aeiBatchPayDeletedByBatchPaymentDeletion, nBatchPaymentID, strOldValue, "<Deleted>", aepHigh); // (b.eyers 2015-10-14) - PLID 67308 - renamed

		//audit unapplying the e-remittance as an extra audit to identify how the batch payment was deleted
		// (j.jones 2011-03-21 17:57) - PLID 24273 - this is somewhat obsolete now that we have unique audits for
		// this process, but I am keeping it in because it is easy to filter on in Auditing
		AuditEvent(-1, strInsuranceCoName, nAuditTransactionID, aeiUnapplyBatchPayment, nBatchPaymentID, strOldValue, "<Posting Unapplied>", aepHigh);		// (b.eyers 2015-10-14) - PLID 67308 - renamed

		//delete all applies
		AddParamStatementToSqlBatch(strSqlBatch, aryParams, "DELETE FROM ApplyDetailsT WHERE ApplyID IN (SELECT ID FROM AppliesT WHERE SourceID IN (SELECT ID FROM PaymentsT WHERE BatchPaymentID = {INT}))", nBatchPaymentID);
		AddParamStatementToSqlBatch(strSqlBatch, aryParams, "DELETE FROM AppliesT WHERE SourceID IN (SELECT ID FROM PaymentsT WHERE BatchPaymentID = {INT})", nBatchPaymentID);
		AddParamStatementToSqlBatch(strSqlBatch, aryParams, "DELETE FROM ApplyDetailsT WHERE ApplyID IN (SELECT ID FROM AppliesT WHERE DestID IN (SELECT ID FROM PaymentsT WHERE BatchPaymentID = {INT}))", nBatchPaymentID);
		AddParamStatementToSqlBatch(strSqlBatch, aryParams, "DELETE FROM AppliesT WHERE DestID IN (SELECT ID FROM PaymentsT WHERE BatchPaymentID = {INT})", nBatchPaymentID);
		//mark payments & adjustments deleted
		AddParamStatementToSqlBatch(strSqlBatch, aryParams, "UPDATE LineItemT SET Deleted = 1, DeleteDate = GetDate(), DeletedBy = {STRING} WHERE Deleted = 0 AND ID IN (SELECT ID FROM PaymentsT WHERE BatchPaymentID = {INT})", GetCurrentUserName(), nBatchPaymentID);
		
		// (j.jones 2015-10-20 11:08) - PLID 67385 - unlock capitation
		AddParamStatementToSqlBatch(strSqlBatch, aryParams, "DELETE FROM BatchPaymentCapitationDetailsT WHERE BatchPaymentID = {INT}", nBatchPaymentID);

		// (b.eyers 2015-10-19) - PLID 67308 - only delete if eob
		//mark the batch payment deleted
		if (bIsEob)
			AddParamStatementToSqlBatch(strSqlBatch, aryParams, "UPDATE BatchPaymentsT SET Deleted = 1, DeleteDate = GetDate(), DeletedBy = {INT} WHERE Deleted = 0 AND ID = {INT}", GetCurrentUserID(), nBatchPaymentID);

		{
			CSqlTransaction trans("UndoBatchPayment"); // (b.eyers 2015-10-14) - PLID 67308 - renamed
			trans.Begin();

			NxAdo::PushMaxRecordsWarningLimit pmr(100000);

			if(!strSqlBatch.IsEmpty()) {
				// (e.lally 2009-06-21) PLID 34679 - Leave as execute batch
				ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, aryParams);

				// (j.jones 2012-04-24 12:54) - PLID 49804 - we now also undo corrections that takeback into this batch payment
				if(bHasCorrections) {
					finCor.ExecuteCorrectionsToUndo(cutUndoPayment, TRUE);

					// (j.jones 2015-06-16 09:56) - PLID 50008 - if there were invalid applies, unapply them now
					for each(long nApplyID in aryAppliesToUndo) {
						//this will audit each unapply
						DeleteApply(nApplyID, FALSE);
					}
				}
			}

			if(nAuditTransactionID != -1) {
				CommitAuditTransaction(nAuditTransactionID);
				nAuditTransactionID = -1;
			}

			trans.Commit();
		}

		//now remove the row, because we just deleted the current batch payment
		// (b.eyers 2015-10-20) - PLID 67308 - only remove the row if eob, if batch payment then just update view
		if (bIsEob) {
			m_BatchPayments->RemoveRow(nCurSel);
			m_BatchPayments->CurSel = -1;
			OnSelChosenBatchPaymentsCombo(-1);
		}
		else {
			UpdateView();
		}
		// (j.jones 2015-10-19 16:21) - PLID 67309 - call RequeryFinished to recalculate the capitation column
		OnRequeryFinishedBatchPaymentsCombo(dlRequeryFinishedCompleted);
		// (b.eyers 2015-10-20) - PLID 67308 - moved these to the if a few rows up
		//m_BatchPayments->CurSel = -1;
		//OnSelChosenBatchPaymentsCombo(-1);

		// (b.eyers 2015-10-19) - PLID 67308 - different warnings if eob or batch payment
		if (bIsEob)
			AfxMessageBox("This E-Remittance payment has been completely unapplied from patient accounts, and the batch payment has been deleted.");
		else 
			AfxMessageBox("This batch payment has been completely unapplied from patient accounts.");

	}NxCatchAllCall("Error in CBatchPayments::OnBtnUnapplyBatchPayment",
		if(nAuditTransactionID != -1) {
			RollbackAuditTransaction(nAuditTransactionID);
		}
	);
}

// (j.jones 2010-05-17 13:05) - PLID 33941 - added ability to prompt for a patient warning,
// also we track who we last warned about
void CBatchPayments::PromptPatientWarning(long nPatientID)
{
	try {

		//do we need to warn?
		if(GetRemotePropertyInt("BatchPayShowPatWarnings", 1, 0, GetCurrentUserName(), true) == 0) {
			//nope
			return;
		}

		//is this a new patient?
		if(m_nLastPatientIDWarned == nPatientID) {
			//we already warned about this patient and have not
			//warned about a new one since then, so don't bother
			//re-warning about the same patient
			return;
		}

		m_nLastPatientIDWarned = nPatientID;

		//we do not need to prompt about everything that we normally prompt for,
		//just show the normal G2 warning, copays, and insurance referrals

		CString strQuery;
		strQuery.Format(
			"DECLARE @PatID int;\r\n"
			"SET @PatID = {INT};\r\n");

		//patient general 2 warning field.  This always shows if data exists.
		{
			// (a.walling 2010-07-01 16:25) - PLID 18081 - Warning categories
			CString strTmp;
			strTmp.Format("SELECT PersonT.DisplayWarning, PersonT.Last + ', ' + PersonT.First AS FullName, PersonT.WarningMessage, UserName, PersonT.WarningDate, PersonT.WarningUseExpireDate, PersonT.WarningExpireDate, WarningCategoriesT.Color AS WarningColor, WarningCategoriesT.ID AS WarningID, WarningCategoriesT.Name AS WarningName "
				"FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
				"LEFT JOIN WarningCategoriesT ON PersonT.WarningCategoryID = WarningCategoriesT.ID "
				"LEFT JOIN UsersT ON UsersT.PersonID = PersonT.WarningUserID "
				"LEFT JOIN (ReferringPhysT INNER JOIN PersonT PersonRefPhys ON ReferringPhysT.PersonID = PersonRefPhys.ID) "
				"ON PatientsT.DefaultReferringPhyID = ReferringPhysT.PersonID "
				"WHERE (((PatientsT.PersonID)=@PatID));\r\n");
			strQuery += strTmp;
		}

		//Insurance referral warnings.  These only show up if the preference is enabled to warn.
		if(GetRemotePropertyInt("ShowInsuranceReferralWarning",0,0,"<None>",TRUE) == 1) {
			CString strTmp;
			// (j.jones 2011-09-29 15:53) - PLID 44980 - ignore referrals if in use on a voided bill
			// (j.jones 2011-09-29 15:53) - PLID 44980 - ignore referrals if in use on a voided bill
			// (j.gruber 2011-10-06 10:24) - PLID 45837 - add fields
			// (j.gruber 2011-10-06 10:25) - PLID 45838 - show anything not expired
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
				"WHERE NumVisits > (CASE WHEN NumUsed Is NULL THEN 0 ELSE NumUsed END) AND "
				"EndDate >= convert(datetime, convert(nvarchar, getdate(), 101)) AND InsuredPartyT.PatientID = @PatID "
				"AND InsuredPartyT.RespTypeID <> -1 "
				"ORDER BY StartDate;\r\n");
			strQuery += strTmp;
		}

		//CoPay warning
		// (j.jones 2010-08-02 11:23) - PLID 39937 - WarnCoPay is now a preference, and no longer a setting per insured party
		// (j.jones 2010-09-01 15:58) - PLID 40356 - changed so we never warn about $0.00 / 0% copays
		BOOL bWarnCopays = (GetRemotePropertyInt("WarnCopays", 0, 0, GetCurrentUserName(), true) == 1);
		if(bWarnCopays) {
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

		//Create the recordset, parameterized with our patient ID.
		_RecordsetPtr prsAllWarnings = CreateParamRecordset(strQuery, nPatientID);

		//From here we'll just go through and pull off the recordsets that were generated as they are needed.

		//normal G2 warning
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
					// (a.walling 2010-07-01 16:25) - PLID 18081 - Warning categories
					BOOL bKeepWarning = dlgWarning.DoModalWarning(AdoFldString(flds, "WarningMessage"), 
						TRUE, strTitle, "Patient Warning", AdoFldLong(flds, "WarningColor", 0), AdoFldLong(flds, "WarningID", -1), AdoFldString(flds, "WarningName", ""));

					//DRT - 11/25/02 - We cannot compare booleans like this - 0 is false, nonzero is true. 
					//				In some cases one will be 1, the other -1, both true, but not equal
					//if (bKeepWarning != bDispWarning) {

					// If the user asked to change the displaywarning status of the patient
					if((bKeepWarning == 0 && bDispWarning != 0) || (bKeepWarning != 0 && bDispWarning == 0)) {
						// Change it
						ExecuteParamSql("UPDATE PersonT SET DisplayWarning = {INT} WHERE ID = {INT}", bKeepWarning?1:0, nPatientID);

						//auditing
						// (a.walling 2008-08-20 13:01) - PLID 29900 - Avoid GetActivePatient*
						long nAuditID = BeginNewAuditEvent();
						AuditEvent(nPatientID, GetExistingPatientName(nPatientID), nAuditID, aeiPatientWarning, nPatientID, (bKeepWarning == 0 ? "Warn" : "Don't Warn"), (bKeepWarning == 0 ? "Don't Warn" : "Warn"), aepMedium, aetChanged);

						// Update the view given the change
						UpdateView();
					}
				}
			}

			//we do not need to check for a referring physician's warning
		}

		//insurance referrals
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
		
		//CoPay Warning
		// (j.jones 2010-08-02 11:23) - PLID 39937 - WarnCoPay is now a preference, and no longer a setting per insured party
		if(bWarnCopays) {
			
			_RecordsetPtr rsCoPay = (prsAllWarnings = prsAllWarnings->NextRecordset(NULL));

			if(!rsCoPay->eof) {

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
			}
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2012-02-16 17:21) - PLID 48137 - added button to go to the selected patient
void CBatchPayments::OnBtnGoToPatient()
{
	try {

		if(m_PatientCombo->CurSel == -1) {
			AfxMessageBox("You must select a patient from the dropdown list.");
			return;
		}

		long nPatientID = VarLong(m_PatientCombo->GetValue(m_PatientCombo->CurSel,0));

		GotoPatient(nPatientID);

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2012-02-16 17:21) - PLID 48137 - added button to go to a patient after prompting for an ID
void CBatchPayments::OnBtnGoToPatientId()
{
	try {

		long nUserDefinedID = -1;
		CString strID = "";
		if(InputBox(this, "Enter a patient ID to go to:", strID, "", false, false, 0, TRUE) == IDOK) {
			nUserDefinedID = atol(strID);
		}
		else {
			return;
		}

		if(nUserDefinedID <= 0) {
			AfxMessageBox("The patient ID entered was not a valid ID.");
			return;
		}

		_RecordsetPtr rs = CreateParamRecordset("SELECT PersonID FROM PatientsT WHERE UserDefinedID = {INT}", nUserDefinedID);
		if(!rs->eof) {
			GotoPatient(VarLong(rs->Fields->Item["PersonID"]->Value));
		}
		else {
			AfxMessageBox("The patient ID entered does not exist in the system.");
			return;
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2012-05-24 17:35) - PLID 50520 - Handle clicking insurance reversal button
void CBatchPayments::OnBnClickedInsuranceReversal()
{
	try
	{
		if(!CheckCurrentUserPermissions(bioFinancialCorrections, sptWrite))
			return;

		IRowSettingsPtr pRow = m_BatchPayments->GetRow(m_BatchPayments->CurSel);
		if(IDOK == CBatchPaymentInsReversalDlg(this, pRow->GetValue(COMBO_COLUMN_ID)).DoModal())
			UpdateView();

	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2012-06-13 16:02) - PLID 49879 - Update the date for the line item payments associated with the specified batch id
void CBatchPayments::UpdatePaymentDatesToReflectBatchDate(long nBatchPaymentID, CString strBatchDate)
{
	try{

		//Use OUTPUT clause for Auditing
		CSqlFragment sqlFrag = CSqlFragment(
			"UPDATE	LineItemT SET Date = {STRING} \r\n"
			"OUTPUT DELETED.ID, DELETED.PatientID, DELETED.Date \r\n" 
			"FROM	LineItemT \r\n"
			"		INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID \r\n"
			"WHERE	PaymentsT.BatchPaymentID = {INT} \r\n",
			strBatchDate, nBatchPaymentID);

		_RecordsetPtr prs = CreateParamRecordset(GetRemoteData(), "{SQL}", sqlFrag);

		//Audit the date change on the payment in a transaction
		CAuditTransaction auditTran;
		for(; !prs->eof; prs->MoveNext())
		{
			AuditEvent(
				AdoFldLong(prs, "PatientID"), 
				GetExistingPatientName(AdoFldLong(prs, "PatientID")), 
				auditTran, 
				aeiPaymentDate, 
				AdoFldLong(prs, "ID"), 
				FormatDateTimeForInterface(AdoFldDateTime(prs, "Date"), 0, dtoDate), 
				strBatchDate, 
				aepMedium, 
				aetChanged);
		}
		auditTran.Commit();

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2012-08-15 15:24) - PLID 52151 - Added ability to remember the last sort column.
// This function will not requery nor re-sort, as it is meant to be called before the initial
// requery even takes places. Also there's some extra logic where we try to intelligently
// add secondary sorts of our own, to try and make the default sort more sensible.
void CBatchPayments::RestoreLastChargeSortOrder()
{
	try {
		long nColumn = GetRemotePropertyInt("BatchPaymentsChargeSortColumn", -1, 0, GetCurrentUserName(), false);
		long nAsc = GetRemotePropertyInt("BatchPaymentsChargeSortAsc", -1, 0, GetCurrentUserName(), false);

		BOOL bApplyDefaultSort = TRUE;
		if(nColumn >= 0 && (nColumn + 1) <= m_ChargeList->GetColumnCount()) {
			//the column index is valid, is the column even displayed?
			if(m_ChargeList->GetColumn((short)nColumn)->GetStoredWidth() > 0) {
				//it is displayed, so this is indeed a valid, visible column
				bApplyDefaultSort = FALSE;
			}
		}

		if(bApplyDefaultSort) {
			//default the sort by patient name
			m_ChargeList->GetColumn(CHARGE_COLUMN_PAT_NAME)->PutSortPriority(0);
			m_ChargeList->GetColumn(CHARGE_COLUMN_PAT_NAME)->PutSortAscending(VARIANT_TRUE);
			m_ChargeList->GetColumn(CHARGE_COLUMN_CHARGE_DATE)->PutSortPriority(1);
			m_ChargeList->GetColumn(CHARGE_COLUMN_CHARGE_DATE)->PutSortAscending(VARIANT_TRUE);
		}
		else {
			//apply the user's last sort column, and sort order
			m_ChargeList->GetColumn((short)nColumn)->PutSortPriority(0);
			m_ChargeList->GetColumn((short)nColumn)->PutSortAscending(nAsc == 1 ? VARIANT_TRUE : VARIANT_FALSE);
	
			//if they didn't sort by date, always add it as a secondary ascending sort
			if(nColumn != CHARGE_COLUMN_CHARGE_DATE) {
				m_ChargeList->GetColumn(CHARGE_COLUMN_CHARGE_DATE)->PutSortPriority(1);
				m_ChargeList->GetColumn(CHARGE_COLUMN_CHARGE_DATE)->PutSortAscending(VARIANT_TRUE);
			}
			else {
				//if they did sort by date, add patient name as a secondary ascending sort
				m_ChargeList->GetColumn(CHARGE_COLUMN_PAT_NAME)->PutSortPriority(1);
				m_ChargeList->GetColumn(CHARGE_COLUMN_PAT_NAME)->PutSortAscending(VARIANT_TRUE);		
			}
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2012-08-15 15:24) - PLID 52151 - added ability to remember the last sort column
void CBatchPayments::OnColumnClickingApplyList(short nCol, BOOL* bAllowSort)
{
	try {

		if(nCol != -1) {
			//save the sort order
			SetRemotePropertyInt("BatchPaymentsChargeSortColumn", nCol, 0, GetCurrentUserName());
			long nAsc = 1;
			if(m_ChargeList->GetColumn(nCol)->GetSortPriority() == 0 && m_ChargeList->GetColumn(nCol)->GetSortAscending()) {
				//The sort doesn't change until this function exits, so we have to calculate the sort order.
				//This click would toggle a descending sort only if the column is currently the primary
				//sort, and is currently sorting ascending.
				nAsc = 0;
			}
			SetRemotePropertyInt("BatchPaymentsChargeSortAsc", nAsc, 0, GetCurrentUserName());
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2012-08-16 09:02) - PLID 52116 - added left click handler for hyperlink cells
// (j.jones 2015-10-29 16:06) - PLID 67431 - converted to left click, not lbuttondown
void CBatchPayments::OnLeftClickApplyList(long nRow, short nCol, long x, long y, long nFlags)
{
	try {

		if(nRow == -1 || nCol == -1) {
			return;
		}

		m_ChargeList->PutCurSel(nRow);

		// (j.jones 2014-06-27 09:25) - PLID 62457 - never do this for vision payments
		if (!IsVisionPayment()) {
			if(nCol == CHARGE_COLUMN_EST_BILL_PAYMENT) {
				// (j.jones 2012-08-16 09:02) - PLID 52116 - added ability to auto-apply using line item posting,
				// but we won't post if a payment wasn't calculated
				TryAutoPostPayment(TRUE);
				return;
			}
			else if(nCol == CHARGE_COLUMN_EST_CHARGE_PAYMENT) {
				// (j.jones 2012-08-16 09:02) - PLID 52116 - added ability to auto-apply using line item posting,
				// but we won't post if a payment wasn't calculated
				TryAutoPostPayment(FALSE);
				return;
			}
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2012-08-20 09:56) - PLID 52116 - moved the defined from clauses into a function
// (j.jones 2012-08-29 10:16) - PLID 52351 - added ability to calculate secondary payments as well
CString CBatchPayments::GenerateFromClause(BOOL bShowEstPaymentColumns, BOOL bEstSecondaryPayments)
{
	CString strFromClause;
	
	long nBatchPaymentID = -1;
	if (m_BatchPayments->GetCurSel() != -1) {
		IRowSettingsPtr pBatchPaymentRow = m_BatchPayments->GetRow(m_BatchPayments->GetCurSel());
		nBatchPaymentID = VarLong(pBatchPaymentRow->GetValue(COMBO_COLUMN_ID));
	}

	// (j.jones 2014-06-27 09:31) - PLID 62547 - if a vision payment, always hide these columns
	if (IsVisionPayment()) {
		bShowEstPaymentColumns = FALSE;
		bEstSecondaryPayments = FALSE;
	}

	// (j.jones 2015-10-16 16:00) - PLID 67382 - if a capitation payment, we are showing the est. columns
	bool bIsCapitation = IsCapitationPayment();
	if (bIsCapitation) {
		bShowEstPaymentColumns = TRUE;
	}

	// (j.jones 2006-12-28 09:35) - PLID 23160 - the new 'group by revenue code' feature
	// changes the from clause, and I felt it was better to keep the from clauses in one spot
	// rather than duplicate them
	//TES 5/22/2008 - PLID 29719 - Added LocationName
	// (j.jones 2011-08-17 11:17) - PLID 44889 - ignore "original" and "void" charges
	// (j.jones 2011-11-02 14:20) - PLID 38686 - these queries now left join insurance, as they can potentially show patient resps. now
	// (j.jones 2012-08-20 10:06) - PLID 52116 - added estimated payment columns, they use slow dbo functions so we won't include
	// the functions in the query unless the column is being displayed
	// (j.jones 2012-08-22 15:27) - PLID 52153 - added support for grouping by bill
	if(m_radioGroupByBill.GetCheck() && !IsCapitationPayment()) {

		//group by bill

		//set the est. payment column source (we do not show the charge column when grouping by bill)
		CString strEstBillPayment = "Convert(SQL_VARIANT, Convert(nvarchar(10), '...'))";
		if(bShowEstPaymentColumns) {
			// (j.jones 2012-08-29 10:19) - PLID 52351 - added support for secondary payment estimation,
			// it just shows the insurance balance, but shows ... if the balance is zero
			// (however, showing ... should be impossible since zero balances shouldn't be shown in this list!)
			CString strBillSecondaryWhen = "";
			if(bEstSecondaryPayments) {
				strBillSecondaryWhen = " WHEN InsuredPartyT.PersonID Is Not Null THEN Convert(SQL_VARIANT, Coalesce(CASE WHEN Sum(ChargeRespT.Amount - (CASE WHEN SumOfAmount Is Null THEN 0 ELSE SumOfAmount END)) > 0 THEN Sum(ChargeRespT.Amount - (CASE WHEN SumOfAmount Is Null THEN 0 ELSE SumOfAmount END)) ELSE NULL END, Convert(SQL_VARIANT, Convert(nvarchar(10), '...')))) ";
			}

			//if the column is shown, we always estimate payments for primary insured parties, or ones meant to send as primary,
			//and only optionally estimate non-primary postings
			strEstBillPayment = "CASE WHEN (RespTypeT.Priority = 1 OR RespTypeT.CategoryPlacement = 1 OR InsuredPartyT.SubmitAsPrimary = 1) "
				"THEN Convert(SQL_VARIANT, Coalesce(dbo.EstimatedBillPayment(BillsT.ID, InsuredPartyT.PersonID), Convert(SQL_VARIANT, Convert(nvarchar(10), '...')))) "
				+ strBillSecondaryWhen +
				"ELSE Convert(SQL_VARIANT, Convert(nvarchar(10), '...')) END";
		}

		// (j.jones 2014-06-30 11:53) - PLID 62652 - added RespTypeCategory
		// (j.jones 2015-10-15 09:20) - PLID 67366 - added PatResp
		// (b.eyers 2015-10-16) - PLID 67360 - added allowable
		// (b.eyers 2015-10-22) - PLID 67361 - added isallowable, always null since this is unused by bill filter
		// (b.eyers 2015-10-28) - PLID 67430 - CopayAmount, added, set to null since this column will never show for bills
		// (j.jones 2015-10-29 16:23) - PLID 67431 - added CopayInsuredPartyID, null in this query
		// (d.lange 2015-11-30 14:29) - PLID 67624 - Calculate the allowable based on ChargesT.AllowableInsuredPartyID
		strFromClause.Format("(SELECT BillsT.ID AS BillID, NULL AS ChargeID, LineItemT.PatientID, PatientsT.UserDefinedID, InsuredPartyT.InsuranceCoID, " 
			"InsuredPartyT.PersonID AS InsuredPartyID, InsuredPartyT.RespTypeID AS RespTypeID, RespTypeT.CategoryType AS RespTypeCategory, " 
			"InsuranceCoT.Name AS InsCoName, Coalesce(RespTypeT.TypeName, 'Patient') AS RespName, PatientPersonT.Last + ', ' + PatientPersonT.First + ' ' + PatientPersonT.Middle AS PatientName, " 
			"dbo.GetBillProviderList(BillsT.ID) AS ProviderName, "
			"BillsT.Date AS BillDate, Min(LineItemT.Date) AS ChargeDate, BillsT.Description AS BillDesc, NULL AS RevCodeID, "
			"NULL AS ItemCode, NULL AS ChargeDesc, "
			"dbo.GetBillTotal(BillsT.ID) AS BillAmount, NULL AS ChargeAmount, "
			"Sum(Round(Convert(money, dbo.GetChargeAllowableForInsuranceCo(ChargesT.ID, COALESCE(AllowableInsuredQ.InsuranceCoID, -1)) "
			"* ChargesT.Quantity * "
			"(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END) * "
			"(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END) * "
			"(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END) * "
			"(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END) "
			"), 2)) AS Allowable, "
			"Sum(Coalesce(PatChargeRespT.Amount, Convert(money,0))) AS PatResp, Sum(ChargeRespT.Amount) AS TotalInsResp, "
			"Sum(ChargeRespT.Amount - (CASE WHEN SumOfAmount Is Null THEN 0 ELSE SumOfAmount END)) AS UnpaidInsResp, "
			"NULL AS CopayAmount, NULL AS CopayInsuredPartyID, LocationsT.Name AS LocationName, "
			"%s AS EstBillPayment, "
			"NULL AS EstChargePayment, "
			"NULL AS IsAllowable "
			"FROM "
			"BillsT INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"INNER JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
			"INNER JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
			"LEFT JOIN (SELECT ChargeID, Amount FROM ChargeRespT WHERE InsuredPartyID Is Null) AS PatChargeRespT ON ChargesT.ID = PatChargeRespT.ChargeID "
			"LEFT JOIN InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID "
			"LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
			"LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
			"INNER JOIN PersonT AS PatientPersonT ON LineItemT.PatientID = PatientPersonT.ID "
			"INNER JOIN PatientsT ON PatientPersonT.ID = PatientsT.PersonID "
			"LEFT JOIN (SELECT SUM(Amount) AS SumOfAmount, RespID FROM AppliesT GROUP BY RespID) AS AppliesT ON ChargeRespT.ID = AppliesT.RespID "
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
			"LEFT JOIN ( "
			"	SELECT InsuredPartyT.PersonID, InsuranceCoT.PersonID AS InsuranceCoID "
			"	FROM InsuredPartyT "
			"	INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
			") AS AllowableInsuredQ ON ChargesT.AllowableInsuredPartyID = AllowableInsuredQ.PersonID "
			"WHERE BillsT.Deleted = 0 AND LineItemT.Deleted = 0 "
			"AND BillsT.EntryType = 1 AND LineItemT.Type = 10 "
			"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
			"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
			"AND ChargeRespT.Amount - (CASE WHEN SumOfAmount Is Null THEN 0 ELSE SumOfAmount END) > 0 "
			"GROUP BY BillsT.ID, LineItemT.PatientID, PatientsT.UserDefinedID, InsuredPartyT.InsuranceCoID, " 
			"InsuredPartyT.PersonID, InsuredPartyT.RespTypeID, RespTypeT.CategoryType, " 
			"InsuranceCoT.Name, Coalesce(RespTypeT.TypeName, 'Patient'), PatientPersonT.Last + ', ' + PatientPersonT.First + ' ' + PatientPersonT.Middle, " 
			"BillsT.Date, BillsT.Description, LocationsT.Name, "
			"RespTypeT.Priority, RespTypeT.CategoryPlacement, InsuredPartyT.SubmitAsPrimary "
			") AS Query", strEstBillPayment);
	}
	else if(m_radioGroupByRevenueCode.GetCheck() && !IsCapitationPayment()) {

		//group by revenue code

		//set the est. payment column source
		CString strEstBillPayment = "Convert(SQL_VARIANT, Convert(nvarchar(10), '...'))";
		CString strEstRevCodePayment = "Convert(SQL_VARIANT, Convert(nvarchar(10), '...'))";
		if(bShowEstPaymentColumns) {
			// (j.jones 2012-08-29 10:19) - PLID 52351 - added support for secondary payment estimation,
			// it just shows the insurance balance, but shows ... if the balance is zero
			// (however, showing ... should be impossible since zero balances shouldn't be shown in this list!)
			CString strBillSecondaryWhen = "";
			CString strRevCodeSecondaryWhen = "";
			if(bEstSecondaryPayments) {
				strBillSecondaryWhen = " WHEN InsuredPartyT.PersonID Is Not Null THEN Convert(SQL_VARIANT, Coalesce(CASE WHEN BillInsTotalsQ.RespBalance > 0 THEN BillInsTotalsQ.RespBalance ELSE NULL END, Convert(SQL_VARIANT, Convert(nvarchar(10), '...'))))";
				strRevCodeSecondaryWhen = " WHEN InsuredPartyT.PersonID Is Not Null THEN Convert(SQL_VARIANT, Coalesce(CASE WHEN Sum(ChargeRespT.Amount - (CASE WHEN SumOfAmount Is Null THEN 0 ELSE SumOfAmount END)) > 0 THEN Sum(ChargeRespT.Amount - (CASE WHEN SumOfAmount Is Null THEN 0 ELSE SumOfAmount END)) ELSE NULL END, Convert(SQL_VARIANT, Convert(nvarchar(10), '...')))) ";
			}

			//if the column is shown, we always estimate payments for primary insured parties, or ones meant to send as primary,
			//and only optionally estimate non-primary postings
			strEstBillPayment = "CASE WHEN (RespTypeT.Priority = 1 OR RespTypeT.CategoryPlacement = 1 OR InsuredPartyT.SubmitAsPrimary = 1) "
				"THEN Convert(SQL_VARIANT, Coalesce(dbo.EstimatedBillPayment(BillsT.ID, InsuredPartyT.PersonID), Convert(SQL_VARIANT, Convert(nvarchar(10), '...')))) "
				+ strBillSecondaryWhen +
				"ELSE Convert(SQL_VARIANT, Convert(nvarchar(10), '...')) END";
			//if we have a revenue code, calculate its payment, otherwise if we have one charge calculate the charge's payment
			strEstRevCodePayment = "CASE WHEN (RespTypeT.Priority = 1 OR RespTypeT.CategoryPlacement = 1 OR InsuredPartyT.SubmitAsPrimary = 1) "
				"THEN Convert(SQL_VARIANT, Coalesce("
				"	CASE WHEN (CASE WHEN ServiceT.RevCodeUse = 1 THEN UB92CategoriesT1.ID WHEN ServiceT.RevCodeUse = 2 THEN UB92CategoriesT2.ID ELSE NULL END) Is Not Null "
				"	THEN dbo.EstimatedRevenueCodePayment(BillsT.ID, "
				"		(CASE WHEN ServiceT.RevCodeUse = 1 THEN UB92CategoriesT1.ID WHEN ServiceT.RevCodeUse = 2 THEN UB92CategoriesT2.ID ELSE NULL END), "
				"		InsuredPartyT.PersonID) "
				"	WHEN (Count(*) = 1) "
				"	THEN dbo.EstimatedChargePayment(Min(ChargesT.ID), InsuredPartyT.PersonID) "
				"	ELSE NULL END, Convert(SQL_VARIANT, Convert(nvarchar(10), '...')))) "
				+ strRevCodeSecondaryWhen +
				"ELSE Convert(SQL_VARIANT, Convert(nvarchar(10), '...')) END ";
		}

		// (j.jones 2014-06-30 11:53) - PLID 62652 - added RespTypeCategory
		// (j.jones 2015-10-15 09:20) - PLID 67366 - added PatResp
		// (b.eyers 2015-10-16) - PLID 67360 - added allowable
		// (b.eyers 2015-10-22) - PLID 67361 - added isallowable, always null since this is unused by rev filter
		// (b.eyers 2015-10-28) - PLID 67430 - CopayAmount added, set to null since it'll never show for rev filter
		// (j.jones 2015-10-29 16:23) - PLID 67431 - added CopayInsuredPartyID, null in this query
		strFromClause.Format("(SELECT BillsT.ID AS BillID, Min(ChargesT.ID) AS ChargeID, LineItemT.PatientID AS PatientID, PatientsT.UserDefinedID, InsuredPartyT.InsuranceCoID, "
			"InsuredPartyT.PersonID AS InsuredPartyID, InsuredPartyT.RespTypeID AS RespTypeID, RespTypeT.CategoryType AS RespTypeCategory, "
			"InsuranceCoT.Name AS InsCoName, Coalesce(RespTypeT.TypeName, 'Patient') AS RespName, PatientPersonT.Last + ', ' + PatientPersonT.First + ' ' + PatientPersonT.Middle AS PatientName, "
			"ProviderPersonT.Last + ', ' + ProviderPersonT.First + ' ' + ProviderPersonT.Middle AS ProviderName, "
			"BillsT.Date AS BillDate, Min(LineItemT.Date) AS ChargeDate, BillsT.Description AS BillDesc, "
			"(CASE WHEN ServiceT.RevCodeUse = 1 THEN UB92CategoriesT1.ID WHEN ServiceT.RevCodeUse = 2 THEN UB92CategoriesT2.ID ELSE NULL END) AS RevCodeID, "
			"(CASE WHEN ServiceT.RevCodeUse = 1 THEN UB92CategoriesT1.Code WHEN ServiceT.RevCodeUse = 2 THEN UB92CategoriesT2.Code ELSE ItemCode END) AS ItemCode, "
			"COALESCE((CASE WHEN ServiceT.RevCodeUse = 1 THEN UB92CategoriesT1.Name WHEN ServiceT.RevCodeUse = 2 THEN UB92CategoriesT2.Name ELSE LineItemT.Description END), LineItemT.Description) AS ChargeDesc, "
			"dbo.GetBillTotal(BillsT.ID) AS BillAmount, Sum(dbo.GetChargeTotal(ChargesT.ID)) AS ChargeAmount, "
			"Sum(Round(Convert(money, dbo.GetChargeAllowableForInsuranceCo(ChargesT.ID, COALESCE(AllowableInsuredQ.InsuranceCoID, -1)) "
			"* ChargesT.Quantity * "
			"(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END) * "
			"(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END) * "
			"(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END) * "
			"(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END) "
			"), 2)) AS Allowable, "
			"Sum(Coalesce(PatChargeRespT.Amount, Convert(money,0))) AS PatResp, Sum(ChargeRespT.Amount) AS TotalInsResp, "
			"Sum(ChargeRespT.Amount - (CASE WHEN SumOfAmount Is Null THEN 0 ELSE SumOfAmount END)) AS UnpaidInsResp, "
			"NULL AS CopayAmount, NULL AS CopayInsuredPartyID, LocationsT.Name AS LocationName, "
			"%s AS EstBillPayment, "
			"%s AS EstChargePayment, "
			"NULL AS IsAllowable "
			"FROM "
			"BillsT INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"INNER JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
			"INNER JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
			"LEFT JOIN (SELECT ChargeID, Amount FROM ChargeRespT WHERE InsuredPartyID Is Null) AS PatChargeRespT ON ChargesT.ID = PatChargeRespT.ChargeID "
			"LEFT JOIN InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID "
			"LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
			"LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
			"INNER JOIN PersonT AS PatientPersonT ON LineItemT.PatientID = PatientPersonT.ID "
			"LEFT JOIN PersonT AS ProviderPersonT ON ChargesT.DoctorsProviders = ProviderPersonT.ID "
			"INNER JOIN PatientsT ON PatientPersonT.ID = PatientsT.PersonID "
			"LEFT JOIN (SELECT SUM(Amount) AS SumOfAmount, RespID FROM AppliesT GROUP BY RespID) AS AppliesT ON ChargeRespT.ID = AppliesT.RespID "
			"LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
			"LEFT JOIN UB92CategoriesT UB92CategoriesT1 ON ServiceT.UB92Category = UB92CategoriesT1.ID "
			"LEFT JOIN (SELECT ServiceRevCodesT.*, PersonID AS InsuredPartyID FROM ServiceRevCodesT "
			"	INNER JOIN InsuredPartyT ON ServiceRevCodesT.InsuranceCompanyID = InsuredPartyT.InsuranceCoID) AS ServiceRevCodesT ON ChargeRespT.InsuredPartyID = ServiceRevCodesT.InsuredPartyID AND ServiceT.ID = ServiceRevCodesT.ServiceID "
			"LEFT JOIN UB92CategoriesT UB92CategoriesT2 ON ServiceRevCodesT.UB92CategoryID = UB92CategoriesT2.ID "
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
			"LEFT JOIN ("
			"	SELECT BillsT.ID AS BillID, InsuredPartyT.PersonID AS InsuredPartyID, "
			"	Sum(Coalesce(ChargeRespT.Amount,Convert(money,0))) - Sum(Coalesce(AppliesT.SumOfAmount,Convert(money,0))) AS RespBalance "
			"	FROM BillsT "
			"	INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
			"	INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"	INNER JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
			"	INNER JOIN InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID "
			"	LEFT JOIN (SELECT SUM(Amount) AS SumOfAmount, RespID FROM AppliesT GROUP BY RespID) AS AppliesT ON ChargeRespT.ID = AppliesT.RespID "
			"	LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
			"	LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
			"	WHERE BillsT.Deleted = 0 AND LineItemT.Deleted = 0 "
			"	AND BillsT.EntryType = 1 AND LineItemT.Type = 10 "
			"	AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
			"	AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
			"	GROUP BY BillsT.ID, InsuredPartyT.PersonID "
			") AS BillInsTotalsQ ON BillsT.ID = BillInsTotalsQ.BillID AND InsuredPartyT.PersonID = BillInsTotalsQ.InsuredPartyID "
			"LEFT JOIN ( "
			"	SELECT InsuredPartyT.PersonID, InsuranceCoT.PersonID AS InsuranceCoID "
			"	FROM InsuredPartyT "
			"	INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
			") AS AllowableInsuredQ ON ChargesT.AllowableInsuredPartyID = AllowableInsuredQ.PersonID "
			"WHERE BillsT.Deleted = 0 AND LineItemT.Deleted = 0 "
			"AND BillsT.EntryType = 1 AND LineItemT.Type = 10 "
			"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
			"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
			"AND ChargeRespT.Amount - (CASE WHEN SumOfAmount Is Null THEN 0 ELSE SumOfAmount END) > 0 "
			"GROUP BY BillsT.ID, LineItemT.PatientID, PatientsT.UserDefinedID, InsuredPartyT.InsuranceCoID, InsuredPartyT.PersonID, InsuredPartyT.RespTypeID, RespTypeT.CategoryType, InsuranceCoT.Name, Coalesce(RespTypeT.TypeName, 'Patient'), PatientPersonT.Last, PatientPersonT.First, PatientPersonT.Middle, "
			"ProviderPersonT.Last, ProviderPersonT.First, ProviderPersonT.Middle, BillsT.Date, BillsT.Description, "
			"(CASE WHEN ServiceT.RevCodeUse = 1 THEN UB92CategoriesT1.ID WHEN ServiceT.RevCodeUse = 2 THEN UB92CategoriesT2.ID ELSE NULL END), "
			"(CASE WHEN ServiceT.RevCodeUse = 1 THEN UB92CategoriesT1.Code WHEN ServiceT.RevCodeUse = 2 THEN UB92CategoriesT2.Code ELSE ItemCode END), "
			"COALESCE((CASE WHEN ServiceT.RevCodeUse = 1 THEN UB92CategoriesT1.Name WHEN ServiceT.RevCodeUse = 2 THEN UB92CategoriesT2.Name ELSE LineItemT.Description END), LineItemT.Description), "
			"LocationsT.Name, RespTypeT.Priority, RespTypeT.CategoryPlacement, InsuredPartyT.SubmitAsPrimary, BillInsTotalsQ.RespBalance "
			") AS Query", strEstBillPayment, strEstRevCodePayment);
	}
	else {
		
		//normal from clause, group by charge

		//set the est. payment column source
		CString strEstBillPayment = "Convert(SQL_VARIANT, Convert(nvarchar(10), '...'))";
		CString strEstChargePayment = "Convert(SQL_VARIANT, Convert(nvarchar(10), '...'))";		
		if (bShowEstPaymentColumns || IsLockedCapitationPayment()) {

			// (j.jones 2015-10-20 13:52) - PLID 67416 - for capitation, skip these estimation calculations
			if (!bIsCapitation) {
				// (j.jones 2012-08-29 10:19) - PLID 52351 - added support for secondary payment estimation,
				// it just shows the insurance balance, but shows ... if the balance is zero
				// (however, showing ... should be impossible since zero balances shouldn't be shown in this list!)
				CString strBillSecondaryWhen = "";
				CString strChargeSecondaryWhen = "";
				if (bEstSecondaryPayments) {
					strBillSecondaryWhen = " WHEN InsuredPartyT.PersonID Is Not Null THEN Convert(SQL_VARIANT, Coalesce(CASE WHEN BillInsTotalsQ.RespBalance > 0 THEN BillInsTotalsQ.RespBalance ELSE NULL END, Convert(SQL_VARIANT, Convert(nvarchar(10), '...'))))";
					strChargeSecondaryWhen = " WHEN InsuredPartyT.PersonID Is Not Null THEN Convert(SQL_VARIANT, Coalesce(CASE WHEN ChargeRespT.Amount - (CASE WHEN SumOfAmount Is Null THEN 0 ELSE SumOfAmount END) > 0 THEN ChargeRespT.Amount - (CASE WHEN SumOfAmount Is Null THEN 0 ELSE SumOfAmount END) ELSE NULL END, Convert(SQL_VARIANT, Convert(nvarchar(10), '...')))) ";
				}

				//if the column is shown, we always estimate payments for primary insured parties, or ones meant to send as primary,
				//and only optionally estimate non-primary postings
				strEstBillPayment = "CASE WHEN (RespTypeT.Priority = 1 OR RespTypeT.CategoryPlacement = 1 OR InsuredPartyT.SubmitAsPrimary = 1) "
					"THEN Convert(SQL_VARIANT, Coalesce(dbo.EstimatedBillPayment(BillsT.ID, InsuredPartyT.PersonID), Convert(SQL_VARIANT, Convert(nvarchar(10), '...')))) "
					+ strBillSecondaryWhen +
					"ELSE Convert(SQL_VARIANT, Convert(nvarchar(10), '...')) END";
				strEstChargePayment = "CASE WHEN (RespTypeT.Priority = 1 OR RespTypeT.CategoryPlacement = 1 OR InsuredPartyT.SubmitAsPrimary = 1) "
					"THEN Convert(SQL_VARIANT, Coalesce(dbo.EstimatedChargePayment(ChargesT.ID, InsuredPartyT.PersonID), Convert(SQL_VARIANT, Convert(nvarchar(10), '...')))) "
					+ strChargeSecondaryWhen +
					"ELSE Convert(SQL_VARIANT, Convert(nvarchar(10), '...')) END";
			}
			else {
				//capitation

				// (j.jones 2015-10-20 13:53) - PLID 67416 - Unlocked capitation payments will just load "...",
				// the calculation is done in OnRequeryFinished. Locked capitation payments, however, need to
				// load the locked payment amounts that had previously been calculated.
				if (IsLockedCapitationPayment()) {
					//this should never be null
					strEstChargePayment = "BatchPaymentCapitationDetailsQ.CalculatedPayment";
				}
			}
		}

		// (j.jones 2015-10-20 14:52) - PLID 67416 - locked capitation payments not only lock the payments, but
		// lock the allowable, so in the unlikely event that the user edited the charge allowable, we still show
		// the allowable that was used in the calculation
		// (j.jones 2015-11-30 11:27) - PLID 67492 - the copay value is now also locked
		// (b.eyers 2015-10-16) - PLID 67430 - added copay, a total of all patient/insurance responsibilities aside from the current resp.
		CString strCopayAmount = "dbo.GetChargeTotal(ChargesT.ID) - ChargeRespT.Amount";

		// (b.eyers 2015-10-16) - PLID 67360 - added allowable
		// (b.eyers 2015-10-29) - PLID 67463 - if the subpreference is on, add this case when to the query
		CString strAllowedAmount, strMultiplier = "";		

		if (GetRemotePropertyInt("BatchPayments_CapitationModifiers", 1, 0, "<None>", true) == 1) {
			// if the modifier assigned is a 50 or 51, add a multiplier of 50%
			strMultiplier = " * (CASE WHEN (CPTModifier IN ('50','51') OR CPTModifier2 IN ('50','51') "
				" OR CPTModifier3 IN ('50','51') OR CPTModifier4 IN ('50','51')) THEN 0.5 ELSE 1 END) ";
		}

		// (d.lange 2015-11-30 14:32) - PLID 67624 - Calculate the allowable based on ChargesT.AllowableInsuredPartyID
		strAllowedAmount.Format("Round(Convert(money, dbo.GetChargeAllowableForInsuranceCo(ChargesT.ID, COALESCE(AllowableInsuredQ.InsuranceCoID, -1)) "
			"* ChargesT.Quantity * "
			"(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END) * "
			"(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END) * "
			"(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END) * "
			"(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END) %s "
			"), 2)", strMultiplier);

		// (j.jones 2015-10-20 13:53) - PLID 67416 - Unlocked capitation payments will just load "...",
		// the calculation is done in OnRequeryFinished. Locked capitation payments, however, need to
		// load the locked payment amounts that had previously been calculated.
		// (j.jones 2015-11-30 11:27) - PLID 67492 - the copay value is now also locked
		if (IsLockedCapitationPayment()) {
			strAllowedAmount = "BatchPaymentCapitationDetailsQ.AllowableUsed";
			strCopayAmount = "IsNull(BatchPaymentCapitationDetailsQ.Copay,Convert(money,0))";
		}

		CString strLockedCapitationJoin = "";
		CString strCapitationFilter = "";
		if (IsCapitationPayment()) {
			// (j.jones 2015-10-20 13:58) - PLID 67416 - if locked capitation, just load locked charges only			
			if (IsLockedCapitationPayment()) {
				// (j.jones 2015-11-30 11:31) - PLID 67492 - the copay is now also locked
				strLockedCapitationJoin.Format("INNER JOIN (SELECT ChargeID, InsuredPartyID, AllowableUsed, Copay, CalculatedPayment "
					"	FROM BatchPaymentCapitationDetailsT "
					"	WHERE BatchPaymentID = %li) "
					"AS BatchPaymentCapitationDetailsQ ON ChargesT.ID = BatchPaymentCapitationDetailsQ.ChargeID "
					"AND InsuredPartyT.PersonID = BatchPaymentCapitationDetailsQ.InsuredPartyID ", nBatchPaymentID);
			}
			else {
				// (j.jones 2015-10-19 13:56) - PLID 67376 - if unlocked capitation, add a date range filter
				COleDateTime dtServiceDateFrom = VarDateTime(m_BatchPayments->GetValue(m_BatchPayments->CurSel, COMBO_COLUMN_SERVICE_DATE_FROM), COleDateTime::GetCurrentTime());
				COleDateTime dtServiceDateTo = VarDateTime(m_BatchPayments->GetValue(m_BatchPayments->CurSel, COMBO_COLUMN_SERVICE_DATE_TO), COleDateTime::GetCurrentTime());
				strCapitationFilter.Format(" AND dbo.AsDateNoTime(LineItemT.Date) >= '%s' AND dbo.AsDateNoTime(LineItemT.Date) <= '%s'", FormatDateTimeForSql(dtServiceDateFrom, dtoDate), FormatDateTimeForSql(dtServiceDateTo, dtoDate));

				// (j.jones 2015-11-23 08:48) - PLID 67625 - if unlocked capitation, only show responsibilities
				// that are for the AllowableInsuredPartyID, if one exists
				strCapitationFilter += " AND (ChargesT.AllowableInsuredPartyID Is Null OR ChargesT.AllowableInsuredPartyID = InsuredPartyT.PersonID)";
			}
		}

		// (j.jones 2014-06-30 11:53) - PLID 62652 - added RespTypeCategory
		// (j.jones 2015-10-15 09:20) - PLID 67366 - added PatResp
		// (b.eyers 2015-10-22) - PLID 67361 - added isallowable
		// (b.eyers 2015-10-28) - PLID 67430 - added CopayAmount, a total of all patient/insurance responsibilities aside from the current resp.
		// (j.jones 2015-10-29 16:23) - PLID 67431 - added CopayInsuredPartyID, the insured party used for CopayAmount
		// (d.lange 2015-11-30 14:33) - PLID 67624 - Calculate the allowable based on ChargesT.AllowableInsuredPartyID
		strFromClause.Format("(SELECT BillsT.ID AS BillID, ChargesT.ID AS ChargeID, LineItemT.PatientID, PatientsT.UserDefinedID, InsuredPartyT.InsuranceCoID, " 
			"InsuredPartyT.PersonID AS InsuredPartyID, InsuredPartyT.RespTypeID AS RespTypeID, RespTypeT.CategoryType AS RespTypeCategory, " 
			"InsuranceCoT.Name AS InsCoName, Coalesce(RespTypeT.TypeName, 'Patient') AS RespName, PatientPersonT.Last + ', ' + PatientPersonT.First + ' ' + PatientPersonT.Middle AS PatientName, " 
			"ProviderPersonT.Last + ', ' + ProviderPersonT.First + ' ' + ProviderPersonT.Middle AS ProviderName, "
			"BillsT.Date AS BillDate, LineItemT.Date AS ChargeDate, BillsT.Description AS BillDesc, NULL AS RevCodeID, ChargesT.ItemCode, LineItemT.Description AS ChargeDesc, "
			"dbo.GetBillTotal(BillsT.ID) AS BillAmount, dbo.GetChargeTotal(ChargesT.ID) AS ChargeAmount, "
			"%s AS Allowable, "
			"Coalesce(PatChargeRespT.Amount, Convert(money,0)) AS PatResp, ChargeRespT.Amount AS TotalInsResp, "
			"ChargeRespT.Amount - (CASE WHEN SumOfAmount Is Null THEN 0 ELSE SumOfAmount END) AS UnpaidInsResp, "
			"%s AS CopayAmount, "
			"Coalesce(NextInsuredPartyT.PersonID, -1) AS CopayInsuredPartyID, "
			"LocationsT.Name AS LocationName, "
			"%s AS EstBillPayment, "
			"%s AS EstChargePayment, "
			"Convert(bit, (CASE WHEN %s IS Null THEN 0 ELSE 1 END)) AS IsAllowable "
			"FROM "
			"BillsT INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"INNER JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
			"INNER JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
			"LEFT JOIN (SELECT ChargeID, Amount FROM ChargeRespT WHERE InsuredPartyID Is Null) AS PatChargeRespT ON ChargesT.ID = PatChargeRespT.ChargeID "
			"LEFT JOIN InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID "
			"LEFT JOIN ( "
                "SELECT Min(AllRespTypesQ.Priority) AS NextPriority, InsuredPartyT.PersonID AS InsuredPartyID "
				"FROM InsuredPartyT "
				"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				"LEFT JOIN ("
					"SELECT RespTypeT.Priority, RespTypeT.CategoryType, InsuredPartyT.PatientID "
					"FROM InsuredPartyT "
					"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				") AS AllRespTypesQ ON InsuredPartyT.PatientID = AllRespTypesQ.PatientID AND RespTypeT.CategoryType = AllRespTypesQ.CategoryType "
				"WHERE AllRespTypesQ.Priority > RespTypeT.Priority "
				"GROUP BY InsuredPartyT.PersonID "
			") AS NextRespQ ON InsuredPartyT.PersonID = NextRespQ.InsuredPartyID "
			"LEFT JOIN RespTypeT as NextRespT ON NextRespQ.NextPriority = NextRespT.Priority "
			"LEFT JOIN InsuredPartyT NextInsuredPartyT ON InsuredPartyT.PatientID = NextInsuredPartyT.PatientID AND NextRespT.ID = NextInsuredPartyT.RespTypeID "
			"LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
			"LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
			"INNER JOIN PersonT AS PatientPersonT ON LineItemT.PatientID = PatientPersonT.ID "
			"LEFT JOIN PersonT AS ProviderPersonT ON ChargesT.DoctorsProviders = ProviderPersonT.ID "
			"INNER JOIN PatientsT ON PatientPersonT.ID = PatientsT.PersonID "
			"LEFT JOIN (SELECT SUM(Amount) AS SumOfAmount, RespID FROM AppliesT GROUP BY RespID) AS AppliesT ON ChargeRespT.ID = AppliesT.RespID "
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
			"LEFT JOIN ("
			"	SELECT BillsT.ID AS BillID, InsuredPartyT.PersonID AS InsuredPartyID, "
			"	Sum(Coalesce(ChargeRespT.Amount,Convert(money,0))) - Sum(Coalesce(AppliesT.SumOfAmount,Convert(money,0))) AS RespBalance "
			"	FROM BillsT "
			"	INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
			"	INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"	INNER JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
			"	INNER JOIN InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID "
			"	LEFT JOIN (SELECT SUM(Amount) AS SumOfAmount, RespID FROM AppliesT GROUP BY RespID) AS AppliesT ON ChargeRespT.ID = AppliesT.RespID "
			"	LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
			"	LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
			"	WHERE BillsT.Deleted = 0 AND LineItemT.Deleted = 0 "
			"	AND BillsT.EntryType = 1 AND LineItemT.Type = 10 "
			"	AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
			"	AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
			"	GROUP BY BillsT.ID, InsuredPartyT.PersonID "
			") AS BillInsTotalsQ ON BillsT.ID = BillInsTotalsQ.BillID AND InsuredPartyT.PersonID = BillInsTotalsQ.InsuredPartyID "
			"LEFT JOIN ( "
			"	SELECT InsuredPartyT.PersonID, InsuranceCoT.PersonID AS InsuranceCoID "
			"	FROM InsuredPartyT "
			"	INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
			") AS AllowableInsuredQ ON ChargesT.AllowableInsuredPartyID = AllowableInsuredQ.PersonID "
			"%s "
			"WHERE BillsT.Deleted = 0 AND LineItemT.Deleted = 0 "
			"AND BillsT.EntryType = 1 AND LineItemT.Type = 10 "
			"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
			"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
			"AND ChargeRespT.Amount - (CASE WHEN SumOfAmount Is Null THEN 0 ELSE SumOfAmount END) > 0 "
			"%s "
			") AS Query",
			strAllowedAmount, strCopayAmount,
			strEstBillPayment, strEstChargePayment,
			strAllowedAmount,
			strLockedCapitationJoin,
			strCapitationFilter);
	}

	return strFromClause;
}

// (j.jones 2012-08-20 13:42) - PLID 52116 - updates the Est. Payment column display
void CBatchPayments::UpdateEstPaymentColumns(BOOL bShowEstPaymentColumns)
{
	try {

		NXDATALISTLib::IColumnSettingsPtr pEstBillCol = m_ChargeList->GetColumn(CHARGE_COLUMN_EST_BILL_PAYMENT);
		NXDATALISTLib::IColumnSettingsPtr pEstChargeCol = m_ChargeList->GetColumn(CHARGE_COLUMN_EST_CHARGE_PAYMENT);

		// (j.jones 2014-06-27 09:31) - PLID 62547 - if a vision payment, always hide these columns
		if (IsVisionPayment()) {
			bShowEstPaymentColumns = FALSE;
		}

		// (j.jones 2015-10-16 16:00) - PLID 67382 - if a capitation payment, we are showing the est. charge
		// column, and not the est. bill column
		bool bIsCapitation = IsCapitationPayment();
		if (bIsCapitation) {
			pEstBillCol->PutStoredWidth(0);
			pEstBillCol->PutColumnStyle(csVisible | csFixedWidth);

			pEstChargeCol->PutStoredWidth(65);
			pEstChargeCol->PutColumnStyle(csVisible);
			return;
		}

		if(bShowEstPaymentColumns) {
			//restore the column widths and allow the user to resize them
			pEstBillCol->PutStoredWidth(75);
			pEstBillCol->PutColumnStyle(csVisible);

			// (j.jones 2012-08-22 16:56) - PLID 52153 - always hide the charge column if grouping by bill
			if(m_radioGroupByBill.GetCheck()) {
				pEstChargeCol->PutStoredWidth(0);
				pEstChargeCol->PutColumnStyle(csVisible|csFixedWidth);
			}
			else {
				pEstChargeCol->PutStoredWidth(65);
				pEstChargeCol->PutColumnStyle(csVisible);
			}
		}
		else {
			//make the columns be zero width and disallow the user's ability to resize them
			pEstBillCol->PutStoredWidth(0);
			pEstBillCol->PutColumnStyle(csVisible|csFixedWidth);

			pEstChargeCol->PutStoredWidth(0);
			pEstChargeCol->PutColumnStyle(csVisible|csFixedWidth);
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2012-08-20 15:59) - PLID 52116 - moved contents of RButtonDown into here
void CBatchPayments::OnContextMenu(CWnd* pWnd, CPoint pos)
{
	try {

		long nRow = m_ChargeList->GetCurSel();

		// (j.jones 2015-10-16 16:02) - PLID 67382 - right-clicking should do nothing if no
		// batch payment is selected
		IRowSettingsPtr pBatchPaymentRow = m_BatchPayments->GetRow(m_BatchPayments->CurSel);
		if (pBatchPaymentRow == NULL) {
			return;
		}

		if(nRow != -1) {

			CMenu pMenu;
			pMenu.CreatePopupMenu();

			long nPatientID = m_ChargeList->GetValue(nRow,CHARGE_COLUMN_PATIENT_ID).lVal;
			CString strLabel;
			strLabel.Format("Preview &Statement For '%s'",VarString(m_ChargeList->GetValue(nRow,CHARGE_COLUMN_PAT_NAME),""));

			// (j.jones 2007-01-04 10:32) - PLID 24030 - store the setting for grouped by rev. code
			BOOL bApplyToRevCode = m_radioGroupByRevenueCode.GetCheck() ? TRUE : FALSE;
			long nRevCodeID = VarLong(m_ChargeList->GetValue(m_ChargeList->GetCurSel(),CHARGE_COLUMN_REV_CODE_ID), -1);

			BOOL bUseRevCode = bApplyToRevCode && nRevCodeID != -1;

			// (j.jones 2005-11-01 16:53) - if the menu order is ever changed, be sure to update
			// the SetDefaultItem() indexes regarding the DefBatchPayClickAction preference

			// (j.jones 2011-11-02 14:37) - PLID 38686 - If RespTypeID is null, then this is patient resp.
			// If patient resp., we cannot "Apply To Bill", because it does not have the
			// "increase ins. balance" functionality.
			BOOL bIsPatientResp = (m_ChargeList->GetValue(m_ChargeList->CurSel,CHARGE_COLUMN_RESP_TYPE_ID).vt == VT_NULL);

			// (j.jones 2012-08-20 16:06) - PLID 52116 - Added ability to auto-apply
			// (j.jones 2015-10-16 16:30) - PLID 67382 - returns true if a capitation payment or the 
			// Est. Payment column preference is on, false if a vision payment
			bool bShowEstPaymentColumns = ShowEstPaymentColumns();

			long DefBatchPayClickAction = GetRemotePropertyInt("DefBatchPayClickAction",0,0,"<None>",true);

			// (j.jones 2012-08-22 17:26) - PLID 52153 - We have a lot of possible default double-click actions,
			// but we can configure the screen such that some are not possible in certain situations.
			// This function will convert the desired default action into the next best alternative.
			ConvertDefBatchPayClickAction(DefBatchPayClickAction, bShowEstPaymentColumns, bIsPatientResp);

			long nMenuID = 0;
			long nApplyToCharge = -1;
			long nApplyToBill = -1;
			long nLineItemPostToCharge = -1;
			long nLineItemPostToBill = -1;
			long nAutoApplyToCharge = -1;
			long nAutoApplyToBill = -1;

			// (s.tullis 2014-06-24 09:20) - PLID 49455 - Permission: Batch payment User Permission to Control Read and Write in the batch Payment Tab
			if ((GetCurrentUserPermissions(bioBatchPayment) & SPT___W_______)) {

				BOOL bGroupingByBill = FALSE;

				// (j.jones 2015-10-16 16:00) - PLID 67382 - if a capitation payment, many right click options will be hidden
				if (!IsCapitationPayment()) {

					// (j.jones 2014-06-27 09:25) - PLID 62457 - vision payments have a special menu item,
					// and also hide several other options
					if (IsVisionPayment()) {
						pMenu.InsertMenu(nMenuID++, MF_BYPOSITION, ID_INVOKE_BILL_POSTING_DLG, "Post &Vision Payment");
						pMenu.SetDefaultItem(nMenuID - 1, TRUE);
						bShowEstPaymentColumns = FALSE;
					}

					// (j.jones 2012-08-22 17:32) - PLID 52153 - disable charge-specific right-click actions if grouping by bill
					BOOL bGroupingByBill = m_radioGroupByBill.GetCheck();

					nApplyToCharge = nMenuID++;
					pMenu.InsertMenu(nApplyToCharge, MF_BYPOSITION | (bGroupingByBill ? MF_DISABLED | MF_GRAYED : MF_ENABLED), ID_APPLY_TO_CHARGE, bUseRevCode ? "&Apply To Revenue Code" : "&Apply To Charge");
					nApplyToBill = nMenuID++;
					pMenu.InsertMenu(nApplyToBill, MF_BYPOSITION | (bIsPatientResp ? MF_DISABLED | MF_GRAYED : MF_ENABLED), ID_APPLY_TO_BILL, "Apply To &Bill");
					pMenu.InsertMenu(nMenuID++, MF_BYPOSITION, ID_POST_PAYMENT, "&Post To Patient");
					pMenu.InsertMenu(nMenuID++, MF_BYPOSITION, ID_CREATE_ADJUSTMENT, "&New Adjustment");

					// (j.jones 2014-06-27 09:25) - PLID 62457 - vision payments hide normal line item posting abilities
					if (!IsVisionPayment()) {
						nLineItemPostToCharge = nMenuID++;
						pMenu.InsertMenu(nLineItemPostToCharge, MF_BYPOSITION | (bGroupingByBill ? MF_DISABLED | MF_GRAYED : MF_ENABLED), ID_INVOKE_CHARGE_POSTING_DLG, bUseRevCode ? "Apply To &Revenue Code Using Line Item Posting" : "Apply To &Charge Using Line Item Posting");
						nLineItemPostToBill = nMenuID++;
						pMenu.InsertMenu(nLineItemPostToBill, MF_BYPOSITION, ID_INVOKE_BILL_POSTING_DLG, "Apply To Bill Using &Line Item Posting");
					}
				}

				// (j.jones 2014-06-27 09:25) - PLID 62457 - vision payments hide normal line item posting abilities
				if (!IsVisionPayment()) {
					// (j.jones 2012-08-20 16:06) - PLID 52116 - Added ability to auto-apply, we'll only show the right
					// click options if they are showing the columns. Even if the current values are '...' we will still
					// enable the menu items, they will just perform the left-click action, which opens line item posting.
					nAutoApplyToCharge = nLineItemPostToCharge;
					nAutoApplyToBill = nLineItemPostToBill;

					// (j.jones 2015-10-19 16:09) - PLID 67382 - for capitation, only show est. pays
					if (bShowEstPaymentColumns || IsCapitationPayment()) {
						nAutoApplyToCharge = nMenuID++;
						pMenu.InsertMenu(nAutoApplyToCharge, MF_BYPOSITION | (bGroupingByBill ? MF_DISABLED | MF_GRAYED : MF_ENABLED), ID_AUTO_APPLY_TO_CHARGE, bUseRevCode ? "Auto-Apply Es&timated Payment To Revenue Code" : "Auto-Apply Es&timated Payment To Charge");
					}
					if (bShowEstPaymentColumns && !IsCapitationPayment()) {
						nAutoApplyToBill = nMenuID++;
						pMenu.InsertMenu(nAutoApplyToBill, MF_BYPOSITION, ID_AUTO_APPLY_TO_BILL, "Auto-Apply &Estimated Payment To Bill");
					}
				}

if (DefBatchPayClickAction == 1 && nLineItemPostToCharge != -1)			//line item post to charge
pMenu.SetDefaultItem(nLineItemPostToCharge, TRUE);
else if (DefBatchPayClickAction == 2 && nLineItemPostToBill != -1)	//line item post to bill
pMenu.SetDefaultItem(nLineItemPostToBill, TRUE);
// (j.jones 2011-11-02 14:37) - PLID 38686 - not permitted on patient resp
else if (DefBatchPayClickAction == 3 && !bIsPatientResp && nApplyToBill != -1)	//apply to bill
pMenu.SetDefaultItem(nApplyToBill, TRUE);
// (j.jones 2012-08-20 16:56) - PLID 52116 - added auto-apply options
else if (DefBatchPayClickAction == 4 && nAutoApplyToCharge != -1) {	//auto-apply to charge			
	pMenu.SetDefaultItem(nAutoApplyToCharge, TRUE);
}
else if (DefBatchPayClickAction == 5 && nAutoApplyToBill != -1) {	//auto-apply to bill
	pMenu.SetDefaultItem(nAutoApplyToBill, TRUE);
}
else if (nApplyToCharge != -1) {									//apply to charge
	pMenu.SetDefaultItem(nApplyToCharge, TRUE);
}

// (j.jones 2015-10-16 16:00) - PLID 67382 - if a capitation payment, default is always auto-apply to charge
if (IsCapitationPayment() && nAutoApplyToCharge != -1) {
	pMenu.SetDefaultItem(nAutoApplyToCharge, TRUE);
}

pMenu.InsertMenu(nMenuID++, MF_BYPOSITION | MF_SEPARATOR);
			}

			pMenu.InsertMenu(nMenuID++, MF_BYPOSITION, ID_PRINT_STATEMENT, strLabel);
			pMenu.InsertMenu(nMenuID++, MF_BYPOSITION, ID_GOTO_PATIENT, "&Go to Patient...");// (a.vengrofski 2010-05-04 12:15) - PLID <36205> - Added so you can go to the patient.

			CPoint pt;
			GetCursorPos(&pt);
			pMenu.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this);
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2012-08-20 16:40) - PLID 52116 - added ability to auto-post, this will
// just open line item posting normally if we have no est. payment
void CBatchPayments::TryAutoPostPayment(BOOL bPostToWholeBill)
{
	try {

		// (j.jones 2014-06-27 09:25) - PLID 62457 - never do this for vision payments
		if (IsVisionPayment()) {
			//We shouldn't have provided the option to do this.
			//Find out how it happened, and stop it.
			ASSERT(FALSE);
			return;
		}

		if (m_BatchPayments->GetCurSel() == -1) {
			return;
		}

		IRowSettingsPtr pBatchPaymentRow = m_BatchPayments->GetRow(m_BatchPayments->GetCurSel());
		long nBatchPaymentID = VarLong(pBatchPaymentRow->GetValue(COMBO_COLUMN_ID));

		long nChargeRow = m_ChargeList->GetCurSel();

		if (nChargeRow == -1) {
			//this should not have been allowed without a selected row
			ASSERT(FALSE);
			return;
		}

		_variant_t varEstPayment = m_ChargeList->GetValue(nChargeRow, bPostToWholeBill ? CHARGE_COLUMN_EST_BILL_PAYMENT : CHARGE_COLUMN_EST_CHARGE_PAYMENT);
		bool bHasEstPayment = (varEstPayment.vt == VT_CY);

		// (j.jones 2015-10-20 08:48) - PLID 67377 - changed the autopost parameter to an enum of options
		EAutoPostType eAutoPostType = eDoNotAutoPost;
		if (bHasEstPayment) {
			//we can auto post if we have an estimated payment, if it fails
			//we will show the line item posting dialog
			eAutoPostType = eTryAutoPost_ElseShowDlg;
		}

		// (j.jones 2015-10-20 10:46) - PLID 67385 - if capitation, we need to know if it is locked or not
		if (IsCapitationPayment()) {

			//capitation payments can't post to the whole bill
			if (bPostToWholeBill) {
				//the calling code needs changed to disallow this
				ASSERT(FALSE);
				bPostToWholeBill = FALSE;
			}

			//we can't post capitation without an estimated payment,
			//which itself should not be possible unless all charges
			//have an allowable
			if (!bHasEstPayment) {
				AfxMessageBox("Capitation payment amounts cannot be calculated and posted until all charges in the service date range have an allowable.");
				return;
			}

			//capitation payments will not show the line item posting dialog 
			//if the payment cannot be posted - the process will simply fail
			eAutoPostType = eAutoPostOnly_NoDlg;

			//if unlocked, we need to warn first, then lock the capitation payment
			if (!IsLockedCapitationPayment()) {
				
				//get our current charge ID, we'll need to re-select it later
				long nChargeID = VarLong(m_ChargeList->GetValue(m_ChargeList->GetCurSel(), CHARGE_COLUMN_CHARGE_ID));

				if (!LockCapitationPayment()) {
					//the lock failed - this function should have already told the user why
					return;
				}

				//locking refreshes the list - so we need to reselect our current row
				nChargeRow = m_ChargeList->SetSelByColumn(CHARGE_COLUMN_CHARGE_ID, (long)nChargeID);
				if (nChargeRow == -1) {
					//this is highly unlikely unless someone really is posting this payment,
					//or happened to delete/pay off the charge
					AfxMessageBox("Another user has begun posting this capitation payment, and the selected charge is no longer available.");
					return;
				}
			}
		}
		
		// (j.jones 2015-10-16 16:30) - PLID 67382 - returns true if a capitation payment or the 
		// Est. Payment column preference is on, false if a vision payment
		bool bShowEstPaymentColumns = ShowEstPaymentColumns();

		if (nChargeRow != -1 && bShowEstPaymentColumns) {			
			InvokePostingDlg(bPostToWholeBill, eAutoPostType);
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2012-08-22 15:10) - PLID 52153 - added ability to group by bill, which also turned the rev. code group into a radio button,
// all three radio button handlers will just call OnChargeGroupingChanged
void CBatchPayments::OnRadioGroupByCharge()
{
	try {

		//save the grouping per user
		//0 - charges (default), 1 - bills, 2 - revenue codes
		if(m_radioGroupByCharge.GetCheck()) {
			SetRemotePropertyInt("BatchPayments_GroupChargesBy", 0, 0, GetCurrentUserName());
		}

		OnChargeGroupingChanged();

	}NxCatchAll(__FUNCTION__);
}

void CBatchPayments::OnRadioGroupByBill()
{
	try {

		//save the grouping per user
		//0 - charges (default), 1 - bills, 2 - revenue codes
		if(m_radioGroupByBill.GetCheck()) {
			SetRemotePropertyInt("BatchPayments_GroupChargesBy", 1, 0, GetCurrentUserName());
		}

		OnChargeGroupingChanged();

	}NxCatchAll(__FUNCTION__);
}

void CBatchPayments::OnRadioGroupByRevCode()
{
	try {

		//save the grouping per user
		//0 - charges (default), 1 - bills, 2 - revenue codes
		if(m_radioGroupByRevenueCode.GetCheck()) {
			SetRemotePropertyInt("BatchPayments_GroupChargesBy", 2, 0, GetCurrentUserName());
		}

		OnChargeGroupingChanged();

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2012-08-22 15:10) - PLID 52153 - added ability to group by bill, which also turned the rev. code group into a radio button,
// all three radio button handlers will just call this function
void CBatchPayments::OnChargeGroupingChanged(BOOL bRequery /*= TRUE*/)
{
	try {
		// (s.tullis 2015-10-22 16:28) - PLID 67383 - Users posting capitation batch payments should not see the grouping settings for charge/bill/revenue code. It should always list out each charge.
		BOOL bCapitationPayment = IsCapitationPayment();
		BOOL bGroupByBill = m_radioGroupByBill.GetCheck() && !bCapitationPayment;
		BOOL bGroupByRevCode = m_radioGroupByRevenueCode.GetCheck() && !bCapitationPayment;
		//change the label
	
		GetDlgItem(IDC_GROUP_BY_TEXT)->ShowWindow(bCapitationPayment ? SW_HIDE: SW_SHOW);
		m_radioGroupByBill.ShowWindow(bCapitationPayment ? SW_HIDE : SW_SHOW);
		m_radioGroupByCharge.ShowWindow(bCapitationPayment ? SW_HIDE : SW_SHOW);
		m_radioGroupByRevenueCode.ShowWindow(bCapitationPayment ? SW_HIDE : SW_SHOW);
		// (s.tullis 2015-10-22 16:28) - PLID 67383 -Changed to new boolean which includes capitation
		if(bGroupByBill) {
			m_nxstaticChargeListLabel.SetWindowText("Bills");
		}
		else if(bGroupByRevCode) {
			m_nxstaticChargeListLabel.SetWindowText("Charges By Revenue Code");
		}
		else {
			m_nxstaticChargeListLabel.SetWindowText("Charges");
		}

		// (j.jones 2015-10-16 16:30) - PLID 67382 - returns true if a capitation payment or the 
		// Est. Payment column preference is on, false if a vision payment
		bool bShowEstPaymentColumns = ShowEstPaymentColumns();

		// (j.jones 2012-08-29 10:16) - PLID 52351 - added ability to calculate secondary payments as well
		BOOL bEstSecondaryPayments = (GetRemotePropertyInt("BatchPayments_ShowEstPaysColumn_Secondary", 0, 0, GetCurrentUserName(), true) == 1);
		// (j.jones 2014-06-27 09:31) - PLID 62547 - if a vision payment, always hide these columns
		if (IsVisionPayment()) {
			bShowEstPaymentColumns = false;
			bEstSecondaryPayments = FALSE;
		}

		m_ChargeList->FromClause = _bstr_t(GenerateFromClause(bShowEstPaymentColumns, bEstSecondaryPayments));

		//if we're going to requery, clear the list before we resize columns
		if(bRequery) {
			m_ChargeList->Clear();
		}

		DisableScreen();

		NXDATALISTLib::IColumnSettingsPtr pItemCodeCol = m_ChargeList->GetColumn(CHARGE_COLUMN_ITEM_CODE);
		NXDATALISTLib::IColumnSettingsPtr pBillDescCol = m_ChargeList->GetColumn(CHARGE_COLUMN_BILL_DESC);
		NXDATALISTLib::IColumnSettingsPtr pChargeDescCol = m_ChargeList->GetColumn(CHARGE_COLUMN_CHARGE_DESC);
		NXDATALISTLib::IColumnSettingsPtr pChargeAmtCol = m_ChargeList->GetColumn(CHARGE_COLUMN_CHARGE_AMT);
		// (s.tullis 2015-10-22 16:28) - PLID 67383 -Changed to boolean which includes capitation
		if(bGroupByBill) {
			// (j.jones 2012-08-22 15:10) - PLID 52153 - added ability to group by bill,
			// when we do we can hide the charge columns, and show the bill desc.

			pItemCodeCol->PutStoredWidth(0);
			pItemCodeCol->PutColumnStyle(csVisible|csFixedWidth);

			pBillDescCol->PutStoredWidth(125);
			pBillDescCol->PutColumnStyle(csVisible|csWidthAuto);

			pChargeDescCol->PutStoredWidth(0);
			pChargeDescCol->PutColumnStyle(csVisible|csFixedWidth);

			pChargeAmtCol->PutStoredWidth(0);
			pChargeAmtCol->PutColumnStyle(csVisible|csFixedWidth);

			//ignore the est. payment columns, they will be handled next
		}
		else {
			//for revenue codes and charges, show the charge columns

			pItemCodeCol->PutStoredWidth(55);
			pItemCodeCol->PutColumnStyle(csVisible);

			pBillDescCol->PutStoredWidth(0);
			pBillDescCol->PutColumnStyle(csVisible|csFixedWidth);

			pChargeDescCol->PutStoredWidth(125);
			pChargeDescCol->PutColumnStyle(csVisible|csWidthAuto);

			pChargeAmtCol->PutStoredWidth(82);
			pChargeAmtCol->PutColumnStyle(csVisible);

			//ignore the est. payment columns, they will be handled next
		}

		//show/hide the est. payment columns
		UpdateEstPaymentColumns(bShowEstPaymentColumns);

		EnableScreen();

		//we would have already cleared the list, so now only requery
		//if a batch payment is selected
		if(bRequery && m_BatchPayments->CurSel != -1) {
			RequeryChargeList();
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2012-08-22 17:26) - PLID 52153 - We have a lot of possible default double-click actions,
// but we can configure the screen such that some are not possible in certain situations.
// This function will convert the desired default action into the next best alternative.
void CBatchPayments::ConvertDefBatchPayClickAction(IN OUT long &nDefBatchPayClickAction, BOOL bShowEstPaymentColumns, BOOL bIsPatientResp)
{
	// (j.jones 2012-08-20 17:09) - PLID 52116 - if the default is to auto-apply, but
	// the columns are hidden, change the default to regular line item posting		
	if(nDefBatchPayClickAction == 4 && !bShowEstPaymentColumns) {	//auto-apply to charge
		nDefBatchPayClickAction = 1;	//line item post to charge
	}
	else if(nDefBatchPayClickAction == 5 && !bShowEstPaymentColumns) {	//auto-apply to bill
		nDefBatchPayClickAction = 2;	//line item post to bill
	}

	// (j.jones 2012-08-22 17:19) - PLID 52153 - if grouping by bill, we can't apply to a charge,
	// so change the defaults accordingly
	if(!IsCapitationPayment() && m_radioGroupByBill.GetCheck()) {
		if(nDefBatchPayClickAction == 0) {	//apply to charge
			nDefBatchPayClickAction = 3;	//apply to bill
		}
		else if(nDefBatchPayClickAction == 1) {	//line item post to charge
			nDefBatchPayClickAction = 2;	//line item post to bill
		}
		else if(nDefBatchPayClickAction == 4) {	//auto-apply to charge
			nDefBatchPayClickAction = 5;	//auto-apply to bill
		}
	}

	// (j.jones 2011-11-02 14:37) - PLID 38686 - If RespTypeID is null, then this is patient resp.
	// If patient resp., we cannot "Apply To Bill", because it does not have the
	// "increase ins. balance" functionality.
	
	// (j.jones 2012-08-22 17:19) - PLID 52153 - If grouping by bill, we can't apply to a charge,
	// and a patient resp. can't apply to a bill! Just open line item posting.
	if(!IsCapitationPayment() && bIsPatientResp && m_radioGroupByBill.GetCheck() && nDefBatchPayClickAction == 3) {
		nDefBatchPayClickAction = 2;	//line item post to bill
	}
}

// (j.jones 2012-08-23 17:53) - PLID 52152 - if a user tabs to the charge list, select a row
void CBatchPayments::OnFocusGainedApplyList()
{
	try {

		BOOL bIsShiftDown = (GetAsyncKeyState(VK_SHIFT) & 0x80000000);
		BOOL bIsTabDown = (GetAsyncKeyState(VK_TAB) & 0x80000000) || IsMessageInQueue(NULL, WM_KEYUP, VK_TAB, 0, IMIQ_MATCH_WPARAM);

		if(m_ChargeList->GetRowCount() > 0 && m_ChargeList->GetCurSel() == -1 && bIsTabDown) {

			//they tabbed to the list, and there is no current selection

			//are they tabbing in from above or below?
			if(!bIsShiftDown) {	//above
				//select the first row
				m_ChargeList->PutCurSel(0);
			}
			else { //below
				//select the last row
				m_ChargeList->PutCurSel(m_ChargeList->GetRowCount()-1);
			}
		}

	}NxCatchAll(__FUNCTION__);
}

// (r.wilson 8/6/2012) - PLID 51852 - When Patient drop down gains focus..
void CBatchPayments::FocusGainedSelectedPatientCombo()
{
	try
	{			
		
		// (r.wilson 8/6/2012) - PLID 51852 - Only set this check if we are not initializing the dialog
		if(m_bDialogInitializing != TRUE){
			m_radioIndivPatient.SetCheck(TRUE);	

			// (r.wilson 8/6/2012) - PLID 51852 - Make sure the other this radio buttons get unselected 
			m_radioAllPatients.SetCheck(FALSE);
			m_radioPatientsInGroup.SetCheck(FALSE);
		}

		OnRadioIndivPatient();						

	}NxCatchAll(__FUNCTION__);
}

// (r.wilson 8/6/2012) - PLID 51852 - When Patient drop down gets its sort column or order changed
void CBatchPayments::ChangeColumnSortFinishedSelectedPatientCombo(short nOldSortCol, BOOL bOldSortAscending, short nNewSortCol, BOOL bNewSortAscending)
{
	try
	{	
		// (r.wilson 8/6/2012) - PLID 51852 - If the selected column changed or if the ASC/DESC status has changed..
		if(nOldSortCol != nNewSortCol || bOldSortAscending != bNewSortAscending)
		{
			IColumnSettingsPtr pColumn = m_PatientCombo->GetColumn(nNewSortCol);

			m_ePatientComboSortType = GetPatientSortType(nNewSortCol,bNewSortAscending);
			if(m_ePatientComboSortType != PatientComboSortType::OtherASC && m_ePatientComboSortType != PatientComboSortType::OtherDESC){
				SetRemotePropertyInt("BatchPayments_PatientComboSortType",(long) m_ePatientComboSortType,0,GetCurrentUserName());
			}
			PatientDropdownSetSortCol();
			
		}

	}NxCatchAll(__FUNCTION__);
}

// (r.wilson 8/6/2012) - PLID 51852 -
PatientComboSortType::SortType CBatchPayments::GetPatientSortType(long nColumn, BOOL bSortAscending)
{
	switch(nColumn)
	{	
		case pddcFirstName:
			if(bSortAscending == TRUE){
				return PatientComboSortType::FirstLastASC;		
			}
			else{
				return PatientComboSortType::FirstLastDESC;
			}
			 break;

		case pddcLastName:	
			if(bSortAscending == TRUE){
				return PatientComboSortType::LastFirstASC;		
			}
			else{
				return PatientComboSortType::LastFirstDESC;
			}
			 break;

		case pddcUserDefinedID:
			if(bSortAscending == TRUE){
				return PatientComboSortType::UserDefinedIdASC;
			}
			else{
				return PatientComboSortType::UserDefinedIdDESC;
			}
			break;

		default:
			if(bSortAscending == TRUE){
				return PatientComboSortType::OtherASC;
			}
			else{
				return PatientComboSortType::OtherDESC;
			}
			break;
	}
}


// (r.wilson 8/6/2012) - PLID 51852 - Function that updates sort priority of the patient's drop down menu
//									  based on the value of the member variable m_ePatientComboSortType
void CBatchPayments::PatientDropdownSetSortCol()
{
	CList<long,long> arr_IndexList;
	BOOL bASC = TRUE;
	long nColIndex = -1;

	// (r.wilson 8/6/2012) - PLID 51852 - The order in which a column's index gets added is directly related to 
	//									  the column's sort priority
	switch(m_ePatientComboSortType)
	{
		case PatientComboSortType::LastFirstASC: 
			arr_IndexList.AddTail((long) pddcLastName);
			arr_IndexList.AddTail((long) pddcFirstName);
			arr_IndexList.AddTail((long) pddcMiddleName);
			arr_IndexList.AddTail((long) pddcUserDefinedID);
		break;

		case PatientComboSortType::LastFirstDESC:
			arr_IndexList.AddTail((long) pddcLastName);
			arr_IndexList.AddTail((long) pddcFirstName);
			arr_IndexList.AddTail((long) pddcMiddleName);
			arr_IndexList.AddTail((long) pddcUserDefinedID);
			bASC = FALSE;			
		break;

		case PatientComboSortType::UserDefinedIdASC:
			arr_IndexList.AddTail((long) pddcUserDefinedID);			
			arr_IndexList.AddTail((long) pddcLastName);
			arr_IndexList.AddTail((long) pddcFirstName);
			arr_IndexList.AddTail((long) pddcMiddleName);
		break;

		case PatientComboSortType::UserDefinedIdDESC:
			arr_IndexList.AddTail((long) pddcUserDefinedID);
			arr_IndexList.AddTail((long) pddcLastName);
			arr_IndexList.AddTail((long) pddcFirstName);
			arr_IndexList.AddTail((long) pddcMiddleName);
			bASC = FALSE;
		break;

		case PatientComboSortType::FirstLastASC: 
			arr_IndexList.AddTail((long) pddcFirstName);
			arr_IndexList.AddTail((long) pddcLastName);
			arr_IndexList.AddTail((long) pddcMiddleName);
			arr_IndexList.AddTail((long) pddcUserDefinedID);
		break;

		case PatientComboSortType::FirstLastDESC:
			arr_IndexList.AddTail((long) pddcFirstName);
			arr_IndexList.AddTail((long) pddcLastName);
			arr_IndexList.AddTail((long) pddcMiddleName);
			arr_IndexList.AddTail((long) pddcUserDefinedID);
			bASC = FALSE;			
		break;

		case PatientComboSortType::OtherASC:
			arr_IndexList.AddTail((long) pddcMiddleName);
			arr_IndexList.AddTail((long) pddcFirstName);
			arr_IndexList.AddTail((long) pddcLastName);
			arr_IndexList.AddTail((long) pddcUserDefinedID);			
		break;
		
		case PatientComboSortType::OtherDESC:
			arr_IndexList.AddTail((long) pddcMiddleName);
			arr_IndexList.AddTail((long) pddcFirstName);
			arr_IndexList.AddTail((long) pddcLastName);
			arr_IndexList.AddTail((long) pddcUserDefinedID);			
			bASC = FALSE;
		break;

	}

	IColumnSettingsPtr pColumn;
	// (r.wilson 8/6/2012) - PLID 51852 - switch all column sort values back to -1
	for(int j = 0; j < m_PatientCombo->ColumnCount ; j++)
	{
		pColumn = m_PatientCombo->GetColumn(j);
		if(pColumn == NULL){
			continue;
		}
		pColumn->PutSortPriority((short)-1); 
	}

	pColumn = NULL;
	int nSortPriority = 0;
	
	// (r.wilson 8/6/2012) - PLID 51852 - Assign new sort priorities to each column
	for(int i = 0; i < arr_IndexList.GetCount(); i++)
	{
		POSITION position = arr_IndexList.FindIndex(i);	
		pColumn = m_PatientCombo->GetColumn((short)arr_IndexList.GetAt(position));
		if(pColumn != NULL)
		{
			pColumn->PutSortAscending(bASC);
			pColumn->PutSortPriority(nSortPriority);
			nSortPriority++;
		}
	}
}

// (j.jones 2014-06-27 09:20) - PLID 62547 - tells us if the selected payment is a vision payment,
// always false if they do not have the vision payment license
bool CBatchPayments::IsVisionPayment()
{
	if (!g_pLicense->CheckForLicense(CLicense::lcVisionPayments, CLicense::cflrSilent)) {
		//even if they formerly had this license, and the payment is actually
		//a vision payment, it's treated like a normal medical payment if
		//they no longer have the license
		return false;
	}

	if (m_BatchPayments->CurSel == -1) {
		return false;
	}

	EBatchPaymentPayType ePayType = (EBatchPaymentPayType)VarLong(m_BatchPayments->GetValue(m_BatchPayments->CurSel, COMBO_COLUMN_PAY_TYPE), (long)eMedicalPayment);
	return ePayType == eVisionPayment;
}

// (s.tullis 2014-08-12 16:53) - PLID 63240 - Batch Payments needs a CTableChecker object for PatCombo.
void CBatchPayments::EnsurePatComboUpdated()
{
	try {

		//don't requery unless the individual patient list is actually
		//in use, otherwise just set m_bNeedRequeryPatientCombo to TRUE,
		//and we'll requery next time we select the individual patient option
		if (m_patients.Changed()){


			if (m_radioIndivPatient.GetCheck()) {

				//keep the current selection if one exists
				long nPatientID = -1;
				if (m_PatientCombo->CurSel != -1)
					nPatientID = VarLong(m_PatientCombo->GetValue(m_PatientCombo->CurSel, 0), -1);

				m_PatientCombo->Requery();

				m_bNeedRequeryPatientCombo = FALSE;

				if (nPatientID != -1) {
					long nRow = m_PatientCombo->SetSelByColumn(0, nPatientID);
					if (nRow == -1 && m_radioIndivPatient.GetCheck()) {
						//the patient doesn't exist anymore and we were
						//currently showing their charges, so clear the list
						m_ChargeList->Clear();
					}
				}
			}
			else {
				//requery the list when we manually select a patient
				m_bNeedRequeryPatientCombo = TRUE;
			}

		}

	} NxCatchAll("Error in CBatchPayments::OnTableChanged:PatCombo");
}

// (b.spivey - July 21st, 2014) - PLID 62957 - Open manage payments dialog. 
void CBatchPayments::OnBnClickedBtnManageLockboxPayments()
{
	try {

		// (b.spivey, September 22, 2014) - PLID 62924 - set the holder. 
		CLockboxManageDlg dlg;
		WaitingOnImportDialog::Holder holder(&dlg); 

		dlg.DoModal(); 
	} NxCatchAll(__FUNCTION__); 
}

// (j.jones 2015-10-16 16:26) - PLID 67382 - returns true if the currently selected payment
// is a capitation payment
bool CBatchPayments::IsCapitationPayment()
{
	//throw exceptions to the caller

	if (m_BatchPayments->CurSel == -1) {
		return false;
	}

	NXDATALISTLib::IRowSettingsPtr pRow = m_BatchPayments->GetRow(m_BatchPayments->CurSel);
	if (pRow) {
		bool bIsCapitation = VarBool(pRow->GetValue(COMBO_COLUMN_CAPITATION), FALSE) ? true : false;
		return bIsCapitation;
	}

	//shouldn't actually be able to get here
	ASSERT(FALSE);
	return false;
}

// (j.jones 2015-10-16 16:30) - PLID 67382 - returns true if a capitation payment or the 
// Est. Payment column preference is on, false if a vision payment
bool CBatchPayments::ShowEstPaymentColumns()
{
	try {

		// (j.jones 2015-10-16 16:31) - PLID 67382 - if a capitation payment, always show these columns
		if (IsCapitationPayment()) {
			return true;
		}

		// (j.jones 2014-06-27 09:31) - PLID 62547 - if a vision payment, always hide these columns
		if (IsVisionPayment()) {
			return false;
		}

		//otherwise, respect the preference
		bool bShowEstPaymentColumns = (GetRemotePropertyInt("BatchPayments_ShowEstPaysColumn", 0, 0, GetCurrentUserName(), true) == 1) ? true : false;
		return bShowEstPaymentColumns;

	}NxCatchAll(__FUNCTION__);

	return false;
}

// (j.jones 2015-10-16 15:24) - PLID 67382 - Added function to show/hide controls for capitation payments.
// Send in true if we are posting a capitation payment, false if not posting a capitation payment.
void CBatchPayments::UpdateControlsForCapitation(bool bCapitationInUse)
{
	try {

		if (bCapitationInUse) {
			//the user is posting a capitation payment, show all controls
			//that are only for capitation, hide all controls that can't
			//be used for capitation

			//hide the Post To Patient and Post Reversals buttons
			m_btnPostToPatient.ShowWindow(SW_HIDE);
			m_btnInsuranceReversal.ShowWindow(SW_HIDE);

			// (j.jones 2015-10-15 09:28) - PLID 67366 - capitation payments need to show the Pat. Resp column
			IColumnSettingsPtr pPatRespCol = m_ChargeList->GetColumn(CHARGE_COLUMN_PAT_AMT);
			if (pPatRespCol) {
				pPatRespCol->PutColumnStyle(csVisible);
				pPatRespCol->PutStoredWidth(65);
			}

			// (j.jones 2015-10-15 09:28) - PLID 67366 - capitation payments need to hide the insurance type column
			IColumnSettingsPtr pInsTypeCol = m_ChargeList->GetColumn(CHARGE_COLUMN_RESP_NAME);
			if (pInsTypeCol) {
				//leave it resizeable, just 0 width
				pInsTypeCol->PutColumnStyle(csVisible);
				pInsTypeCol->PutStoredWidth(0);
			}

			// (j.jones 2015-11-17 15:11) - PLID 67366 - capitation payments should hide the Bill Total column
			IColumnSettingsPtr pBillAmtCol = m_ChargeList->GetColumn(CHARGE_COLUMN_BILL_AMT);
			if (pBillAmtCol) {
				//leave it resizeable, just 0 width
				pBillAmtCol->PutColumnStyle(csVisible);
				pBillAmtCol->PutStoredWidth(0);
			}

			// (b.eyers 2015-10-16) - PLID 67360 - allowable shows for capitation payments
			IColumnSettingsPtr pAllowableCol = m_ChargeList->GetColumn(CHARGE_COLUMN_ALLOWABLE);
			if (pAllowableCol) {
				pAllowableCol->PutColumnStyle(csVisible);
				pAllowableCol->PutStoredWidth(65);
			}

			// (b.eyers 2015-10-28) - PLID 67430 - show copay column for capitation
			IColumnSettingsPtr pCopayCol = m_ChargeList->GetColumn(CHARGE_COLUMN_COPAY);
			if (pCopayCol) {
				pCopayCol->PutColumnStyle(csVisible);
				pCopayCol->PutStoredWidth(65);
			}

			// (b.eyers 2015-10-23) - PLID 67384 - show reimbursement labels for capitation
			m_nxstaticReimbursementRateLabel.ShowWindow(SW_SHOW);
			m_nxstaticReimbursementPercentage.ShowWindow(SW_SHOW);
			m_icoReimbursementInfo.ShowWindow(SW_SHOW);
			m_nxstaticTotalAllowableLabel.ShowWindow(SW_SHOW);
			m_nxstaticTotalAllowableAmt.ShowWindow(SW_SHOW);

			// (b.eyers 2015-10-22) - PLID 67432 - disable the patient filter and filter by bill id
			m_radioIndivPatient.EnableWindow(SW_HIDE);
			m_PatientCombo->Enabled = FALSE;
			m_btnFilterByBillID.EnableWindow(SW_HIDE);
			// if patient filter was selected, unselect it and select the insurance company filter
			if (m_radioIndivPatient.GetCheck()) {
				m_radioAllPatients.SetCheck(TRUE);
				m_radioIndivPatient.SetCheck(FALSE);
			}
			// if this is a locked capitation, select insurance company filter and disable the other two filters
			if (IsLockedCapitationPayment()) {
				if (m_radioPatientsInGroup.GetCheck()) {
					m_radioAllPatients.SetCheck(TRUE);
					m_radioPatientsInGroup.SetCheck(FALSE);
				}
				m_radioAllPatients.EnableWindow(SW_HIDE);
				m_radioPatientsInGroup.EnableWindow(SW_HIDE);
			}
			else {
				m_radioAllPatients.EnableWindow(SW_SHOW);
				m_radioPatientsInGroup.EnableWindow(SW_SHOW);
			}
		}
		else {
			//the user is posting a normal batch payment, hide all controls
			//that are only for capitation, show all controls that may have
			//been hidden for capitation

			//show the Post To Patient and Post Reversals buttons
			m_btnPostToPatient.ShowWindow(SW_SHOW);
			m_btnInsuranceReversal.ShowWindow(SW_SHOW);

			// (j.jones 2015-10-15 09:28) - PLID 67366 - non-capitation payments need to hide the Pat. Resp column
			IColumnSettingsPtr pPatRespCol = m_ChargeList->GetColumn(CHARGE_COLUMN_PAT_AMT);
			if (pPatRespCol) {
				//leave it resizeable, just 0 width
				pPatRespCol->PutColumnStyle(csVisible);
				pPatRespCol->PutStoredWidth(0);
			}

			// (j.jones 2015-10-15 09:28) - PLID 67366 - non-capitation payments need to show the insurance type column
			IColumnSettingsPtr pInsTypeCol = m_ChargeList->GetColumn(CHARGE_COLUMN_RESP_NAME);
			if (pInsTypeCol) {
				pInsTypeCol->PutColumnStyle(csVisible);
				pInsTypeCol->PutStoredWidth(70);
			}

			// (j.jones 2015-11-17 15:11) - PLID 67366 - non-capitation payments should show the Bill Total column
			IColumnSettingsPtr pBillAmtCol = m_ChargeList->GetColumn(CHARGE_COLUMN_BILL_AMT);
			if (pBillAmtCol) {
				pBillAmtCol->PutColumnStyle(csVisible);
				pBillAmtCol->PutStoredWidth(65);
			}

			// (b.eyers 2015-10-16) - PLID 67360 - allowable hides for non-capitation
			IColumnSettingsPtr pAllowableCol = m_ChargeList->GetColumn(CHARGE_COLUMN_ALLOWABLE);
			if (pAllowableCol) {
				//leave it resizeable, just 0 width
				pAllowableCol->PutColumnStyle(csVisible);
				pAllowableCol->PutStoredWidth(0);
			}

			// (b.eyers 2015-10-28) - PLID 67430 - hide copay for non-capitation
			IColumnSettingsPtr pCopayCol = m_ChargeList->GetColumn(CHARGE_COLUMN_COPAY);
			if (pCopayCol) {
				pCopayCol->PutColumnStyle(csVisible|csFixedWidth);
				pCopayCol->PutStoredWidth(0);
			}
			// (b.eyers 2015-10-23) - PLID 67384 - hide reimbursement labels for non-capitation
			m_nxstaticReimbursementRateLabel.ShowWindow(SW_HIDE);
			m_nxstaticReimbursementPercentage.ShowWindow(SW_HIDE);
			m_icoReimbursementInfo.ShowWindow(SW_HIDE);
			m_nxstaticTotalAllowableLabel.ShowWindow(SW_HIDE);
			m_nxstaticTotalAllowableAmt.ShowWindow(SW_HIDE);

			// (b.eyers 2015-10-22) - PLID 67432 - enable these if they were disabled before
			m_radioIndivPatient.EnableWindow(SW_SHOW);
			m_PatientCombo->Enabled = TRUE;
			m_btnFilterByBillID.EnableWindow(SW_SHOW);
			m_radioAllPatients.EnableWindow(SW_SHOW);
			m_radioPatientsInGroup.EnableWindow(SW_SHOW);
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2015-10-19 13:40) - PLID 67376 - Calculates the capitation payment amounts per charge
// from the allowables, and fills the Est. Payment columns with the results.
// Returns true if successful, false if a calculation was not possible.
bool CBatchPayments::CalculateCapitation()
{
	try {

		if (m_BatchPayments->CurSel == -1) {
			return false;
		}

		if (!IsCapitationPayment()) {
			return false;
		}

		// (j.jones 2015-10-20 13:54) - PLID 67416 - locked capitation payments do not
		// need their payment amounts recalculated
		if (IsLockedCapitationPayment()) {
			return false;
		}

		IRowSettingsPtr pBatchPaymentRow = m_BatchPayments->GetRow(m_BatchPayments->CurSel);
		if (pBatchPaymentRow == NULL) {
			return false;
		}

		//calculate capitation off the batch payment total amount - not the current balance
		COleCurrency cyBatchPaymentAmount = VarCurrency(pBatchPaymentRow->GetValue(COMBO_COLUMN_AMOUNT), COleCurrency(0, 0));
		if (cyBatchPaymentAmount <= COleCurrency(0, 0)) {
			//you need a positive payment to use capitation
			return false;
		}

		COleCurrency cyTotalAllowable = COleCurrency(0, 0);
		COleCurrency cyTotalCopays = COleCurrency(0, 0);

		//find all allowables - if the allowable is null for any row, return false
		for (int i = 0; i < m_ChargeList->GetRowCount(); i++) {
			_variant_t varAllowable = m_ChargeList->GetValue(i, CHARGE_COLUMN_ALLOWABLE);
			if (varAllowable.vt != VT_CY) {
				// (b.eyers 2015-10-23) - PLID 67384 - there are null allowables so we can't calculate a reimbursement %
				CString strNotCalcPercent = "--%";
				SetDlgItemText(IDC_REIMBURSEMENT_PERCENTAGE, strNotCalcPercent);
				CString strTotalAmt = "< n/a >";
				SetDlgItemText(IDC_TOTAL_ALLOWABLE_AMT, strTotalAmt);
				m_icoReimbursementInfo.SetToolTip("The reimbursement rate cannot be calculated because not all charges have an allowable.");
				//if any row has no allowable, capitation cannot be calculated
				return false;
			}

			//add this allowable to our total
			cyTotalAllowable += VarCurrency(varAllowable);

			//add up our copays
			cyTotalCopays += VarCurrency(m_ChargeList->GetValue(i, CHARGE_COLUMN_COPAY), COleCurrency(0,0));
		}

		RoundCurrency(cyTotalAllowable);
		RoundCurrency(cyTotalCopays);

		if (cyTotalAllowable <= COleCurrency(0, 0)) {
			//you can't divide by zero, and having all zero allowables
			//makes no sense, so you can't use capitation
			return false;
		}

		//now figure out our reimbursement rate as
		//(batch payment total + total copays) / total allowable
		double dblReimbursementRate = 0.0;
		
		//you can't divide currencies without bizarro coding magic
		{
			// (b.eyers 2015-10-27) - PLID 67384 - calculate reimbursement rate
			dblReimbursementRate = CalculateReimbursementRate(cyBatchPaymentAmount, cyTotalCopays, cyTotalAllowable);
			// (b.eyers 2015-10-23) - PLID 67384 - calculate and display reimbursement %
			double dblReimbursementPercent = 0.0;
			dblReimbursementPercent = dblReimbursementRate * 100;
			CString strPercent; 
			strPercent.Format("%.2f%%", dblReimbursementPercent);
			SetDlgItemText(IDC_REIMBURSEMENT_PERCENTAGE, strPercent);
			CString strTotalAllowableAmt = FormatCurrencyForInterface(cyTotalAllowable);
			SetDlgItemText(IDC_TOTAL_ALLOWABLE_AMT, strTotalAllowableAmt);
			CString strReimbursementInfo;
			strReimbursementInfo.Format("Batch Payment Amount: %s\r\n"
				"Total Copay: %s\r\n"
				"Total Allowable: %s ",
				FormatCurrencyForInterface(cyBatchPaymentAmount),
				FormatCurrencyForInterface(cyTotalCopays),
				strTotalAllowableAmt);
			m_icoReimbursementInfo.SetToolTip(strReimbursementInfo);
		}

		if (dblReimbursementRate < 0.0) {
			//how was this possible?
			ASSERT(FALSE);
			return false;
		}

		//now calculate the estimated payment column
		//as the (reimbursement rate * allowable) - copay

		COleCurrency cyTotalPayments = COleCurrency(0, 0);

		for (int i = 0; i < m_ChargeList->GetRowCount(); i++) {		

			//calculate the payment amount using the reimbursement rate
			COleCurrency cyAllowable = VarCurrency(m_ChargeList->GetValue(i, CHARGE_COLUMN_ALLOWABLE));			
			COleCurrency cyCopay = VarCurrency(m_ChargeList->GetValue(i, CHARGE_COLUMN_COPAY));
			COleCurrency cyPayment = (cyAllowable * dblReimbursementRate) - cyCopay;
			if (cyPayment < COleCurrency(0, 0)) {
				//this shouldn't happen unless you have some weird data
				//like a copay that is really high in proportion to the allowable,
				//which is not realistic
				cyPayment = COleCurrency(0, 0);
			}
			RoundCurrency(cyPayment);

			//if you had a really small check, and the copay percentages
			//aren't consistent across all patients, a patient with a small
			//copay could eat up the rest of the balance before you hit the
			//last row!
			if (cyBatchPaymentAmount < cyTotalPayments + cyPayment) {
				cyPayment = cyBatchPaymentAmount - cyTotalPayments;
				RoundCurrency(cyPayment);
				if (cyPayment < COleCurrency(0, 0)) {
					cyPayment = COleCurrency(0, 0);
				}
			}

			//track the total payment
			cyTotalPayments += cyPayment;

			//if this is the last row, adjust for differences in the balance, if any
			if (i == m_ChargeList->GetRowCount() - 1) {
				RoundCurrency(cyTotalPayments);

				COleCurrency cyDiff = cyBatchPaymentAmount - cyTotalPayments;
				RoundCurrency(cyDiff);

				if (cyDiff != COleCurrency(0, 0)) {
					//there is a balance - hopefully only a couple pennies					
					if (cyDiff < COleCurrency(-1, 0) || cyDiff > COleCurrency(1, 0)) {
						//The difference shouldn't be more than a dollar unless you
						//have some weird data like a copay that is really high in proportion
						//to the allowable, which is not realistic.
						//If you have any payments estimated to be $0.00,
						//that's likely the culprit.
						ASSERT(FALSE);
					}

					//reduce/increase the amount paid
					cyPayment += cyDiff;
					cyTotalPayments += cyDiff;

					if (cyPayment < COleCurrency(0, 0)) {
						//this shouldn't happen unless you have some weird data
						//like a copay that is really high in proportion to the allowable,
						//which is not realistic
						cyPayment = COleCurrency(0, 0);
					}

					RoundCurrency(cyPayment);
				}
			}

			m_ChargeList->PutValue(i, CHARGE_COLUMN_EST_CHARGE_PAYMENT, _variant_t(cyPayment));
		}

		return true;

	}NxCatchAll(__FUNCTION__);

	return false;
}

// (j.jones 2015-10-19 16:13) - PLID 67309 - added ability to show/hide Capitation column
void CBatchPayments::OnRequeryFinishedBatchPaymentsCombo(short nFlags)
{
	try {

		// (j.jones 2015-10-19 16:14) - PLID 67309 - if any row has Capitation? set to TRUE,
		// show the capitation column
		long nRow = m_BatchPayments->FindByColumn(COMBO_COLUMN_CAPITATION, g_cvarTrue, 0, VARIANT_FALSE);
		if (nRow != -1) {
			//found it
			m_BatchPayments->GetColumn(COMBO_COLUMN_CAPITATION)->PutColumnStyle(csVisible);
			m_BatchPayments->GetColumn(COMBO_COLUMN_CAPITATION)->PutStoredWidth(70);
		}
		else {
			//not found
			m_BatchPayments->GetColumn(COMBO_COLUMN_CAPITATION)->PutColumnStyle(csVisible | csFixedWidth);
			m_BatchPayments->GetColumn(COMBO_COLUMN_CAPITATION)->PutStoredWidth(0);			
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2015-10-20 10:37) - PLID 67385 - returns true if the current payment is
// a locked capitation payment
bool CBatchPayments::IsLockedCapitationPayment()
{
	//throw exceptions to the caller

	//return false if not capitation
	if (!IsCapitationPayment()) {
		return false;
	}

	if (m_BatchPayments->CurSel == -1) {
		//then how are we on a capitation payment?
		ASSERT(FALSE);
		return false;
	}

	NXDATALISTLib::IRowSettingsPtr pRow = m_BatchPayments->GetRow(m_BatchPayments->CurSel);
	if (pRow) {
		bool bIsLockedCapitation = VarBool(pRow->GetValue(COMBO_COLUMN_LOCKED_CAPITATION), FALSE) ? true : false;
		return bIsLockedCapitation;
	}

	//shouldn't actually be able to get here
	ASSERT(FALSE);
	return false;
}

// (j.jones 2015-10-22 08:41) - PLID 67385 - locks the current capitation payment,
// returns false if the batch could not be locked
bool CBatchPayments::LockCapitationPayment()
{
	//throw exceptions to the caller

	long nCurSel = m_BatchPayments->GetCurSel();
	if (nCurSel == -1) {
		//the calling code should not have called this function
		ASSERT(FALSE);
		return false;
	}
		
	if (!IsCapitationPayment()) {
		//the calling code should not have called this function
		ASSERT(FALSE);
		return false;
	}

	if (IsLockedCapitationPayment()) {
		//the calling code should not have called this function
		ASSERT(FALSE);
		return false;
	}

	//make sure that no charges are missing an allowable
	m_ChargeList->WaitForRequery(dlPatienceLevelWaitIndefinitely);
	if (m_ChargeList->FindByColumn(CHARGE_COLUMN_ALLOWABLE, g_cvarNull, 0, VARIANT_TRUE) != -1) {

		//the calling code shouldn't have called this function until the user tried to
		//post a payment, and payments shouldn't have been calculated unless all charges
		//have an allowable
		ASSERT(FALSE);

		AfxMessageBox("Capitation payment amounts cannot be calculated and posted until all charges in the service date range have an allowable.");
		return false;
	}
	
	IRowSettingsPtr pBatchPaymentRow = m_BatchPayments->GetRow(nCurSel);
	long nBatchPaymentID = VarLong(pBatchPaymentRow->GetValue(COMBO_COLUMN_ID));

	//warn the user
	if (IDNO == MessageBox("Once you begin posting a capitation payment, the allowable, copay, and calculated payment amounts will be locked. "
		"Changes to charges in this date range will not be reflected in the list. "
		"To start over, you will need to click 'Unapply Batch Payment'.\n\n"
		"Are you sure you wish to begin posting this capitation payment?", "Practice", MB_ICONQUESTION | MB_YESNO)) {
		return false;
	}

	//make sure someone else hasn't already locked the capitation payment
	if (!ReturnsRecordsParam("SELECT TOP 1 BatchPaymentID "
		" FROM BatchPaymentCapitationDetailsT "
		" WHERE BatchPaymentID = {INT}", nBatchPaymentID)) {

		//lock the capitation payment
		CParamSqlBatch sqlBatch;
		for (int i = 0; i < m_ChargeList->GetRowCount(); i++) {
			long nChargeID = VarLong(m_ChargeList->GetValue(i, CHARGE_COLUMN_CHARGE_ID));
			long nInsuredPartyID = VarLong(m_ChargeList->GetValue(i, CHARGE_COLUMN_INSURED_PARTY_ID));
			COleCurrency cyAllowable = VarCurrency(m_ChargeList->GetValue(i, CHARGE_COLUMN_ALLOWABLE), COleCurrency(0, 0));
			COleCurrency cyCopay = VarCurrency(m_ChargeList->GetValue(i, CHARGE_COLUMN_COPAY), COleCurrency(0, 0));
			COleCurrency cyPayment = VarCurrency(m_ChargeList->GetValue(i, CHARGE_COLUMN_EST_CHARGE_PAYMENT), COleCurrency(0, 0));

			// (j.jones 2015-11-30 11:24) - PLID 67492 - added the Copay column
			sqlBatch.Add("INSERT INTO BatchPaymentCapitationDetailsT (BatchPaymentID, ChargeID, InsuredPartyID, AllowableUsed, Copay, CalculatedPayment) "
				"VALUES ({INT}, {INT}, {INT}, {OLECURRENCY}, {OLECURRENCY}, {OLECURRENCY})", nBatchPaymentID, nChargeID, nInsuredPartyID, cyAllowable, cyCopay, cyPayment);
		}
		if (!sqlBatch.IsEmpty()) {
			sqlBatch.Execute(GetRemoteData());
		}

		//update our combo
		pBatchPaymentRow->PutValue(COMBO_COLUMN_LOCKED_CAPITATION, g_cvarTrue);

		//now audit this locking
		long nAuditID = BeginNewAuditEvent();
		COleDateTime dtBatchPayDate = VarDateTime(pBatchPaymentRow->GetValue(COMBO_COLUMN_DATE));
		CString strInsuranceCoName = VarString(pBatchPaymentRow->GetValue(COMBO_COLUMN_INSCO_NAME));
		CString strBatchPayDesc = VarString(pBatchPaymentRow->GetValue(COMBO_COLUMN_DESCRIPTION));
		COleCurrency cyBatchPayAmt = VarCurrency(pBatchPaymentRow->GetValue(COMBO_COLUMN_AMOUNT));

		CString strOldValue;
		strOldValue.Format("Amount: %s, Description: %s, Payment Date: %s",
			FormatCurrencyForInterface(cyBatchPayAmt), strBatchPayDesc, FormatDateTimeForInterface(dtBatchPayDate, NULL, dtoDate));

		AuditEvent(-1, strInsuranceCoName, nAuditID, aeiBatchPayCapitationLocked, nBatchPaymentID, strOldValue, "<Locked>", aepHigh);
	}

	//we need to refresh the screen to now load only the locked charges,
	//this will change the from/where clauses in the charge list
	UpdateView(true);

	return true;
}

// (b.eyers 2015-10-27) - PLID 67384 - calculate the reimbursement rate
double CBatchPayments::CalculateReimbursementRate(COleCurrency cyBatchPaymentAmount, COleCurrency cyTotalCopays, COleCurrency cyTotalAllowable)
{
	double dblReimbursementRate = 0.0;
	_variant_t varNumer = (cyBatchPaymentAmount + cyTotalCopays);
	_variant_t varDenom = cyTotalAllowable;
	_variant_t varResult;
	HR(VarDiv(&varNumer, &varDenom, &varResult));
	_variant_t varRate = varResult;
	varRate.ChangeType(VT_R8);
	dblReimbursementRate = VarDouble(varRate);

	return dblReimbursementRate;
}

// (j.jones 2015-10-29 15:24) - PLID 67431 - added dragging abilities
void CBatchPayments::OnDragBeginApplyList(BOOL* pbShowDrag, long nRow, short nCol, long nFlags)
{
	try {

		if (nRow == -1) {
			*pbShowDrag = FALSE;
			return;
		}

		//if not an unlocked capitation payment, we aren't allowing dragging abilities
		if (!IsCapitationPayment() || IsLockedCapitationPayment()) {
			*pbShowDrag = FALSE;
			return;
		}

		//if not one of the four draggable columns, disallow
		if (nCol != CHARGE_COLUMN_PAT_AMT
			&& nCol != CHARGE_COLUMN_INS_AMT
			&& nCol != CHARGE_COLUMN_INS_BALANCE
			&& nCol != CHARGE_COLUMN_COPAY) {

			*pbShowDrag = FALSE;
			return;
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2015-10-29 15:24) - PLID 67431 - added dragging abilities
void CBatchPayments::OnDragOverCellApplyList(BOOL* pbShowDrop, long nRow, short nCol, long nFromRow, short nFromCol, long nFlags)
{
	try {

		if (nRow == -1 || nFromRow == -1) {
			*pbShowDrop = FALSE;
			return;
		}

		//can't drag to different rows
		if (nRow != nFromRow) {
			*pbShowDrop = FALSE;
			return;
		}

		//if not an unlocked capitation payment, we aren't allowing dragging abilities
		if (!IsCapitationPayment() || IsLockedCapitationPayment()) {
			*pbShowDrop = FALSE;
			return;
		}
		
		//if not one of the four draggable columns, disallow
		if (nCol != CHARGE_COLUMN_PAT_AMT
			&& nCol != CHARGE_COLUMN_INS_AMT
			&& nCol != CHARGE_COLUMN_INS_BALANCE
			&& nCol != CHARGE_COLUMN_COPAY) {

			*pbShowDrop = FALSE;
			return;
		}

		//dragging from ins. amt. to ins. balance and vice versa is meaningless
		if ((nCol == CHARGE_COLUMN_INS_AMT && nFromCol == CHARGE_COLUMN_INS_BALANCE)
			|| (nFromCol == CHARGE_COLUMN_INS_AMT && nCol == CHARGE_COLUMN_INS_BALANCE)) {

			*pbShowDrop = FALSE;
			return;
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2015-10-29 15:24) - PLID 67431 - added dragging abilities
void CBatchPayments::OnDragEndApplyList(long nRow, short nCol, long nFromRow, short nFromCol, long nFlags)
{
	try {

		if (nRow == -1 || nFromRow == -1) {
			return;
		}

		//can't drag to different rows
		if (nRow != nFromRow) {
			return;
		}

		//dragging to the same column does nothing
		if (nCol == nFromCol) {
			return;
		}

		//if not an unlocked capitation payment, we aren't allowing dragging abilities
		if (!IsCapitationPayment() || IsLockedCapitationPayment()) {
			return;
		}

		//if not one of the four draggable columns, disallow
		if (nCol != CHARGE_COLUMN_PAT_AMT
			&& nCol != CHARGE_COLUMN_INS_AMT
			&& nCol != CHARGE_COLUMN_INS_BALANCE
			&& nCol != CHARGE_COLUMN_COPAY) {

			return;
		}
		if (nFromCol != CHARGE_COLUMN_PAT_AMT
			&& nFromCol != CHARGE_COLUMN_INS_AMT
			&& nFromCol != CHARGE_COLUMN_INS_BALANCE
			&& nFromCol != CHARGE_COLUMN_COPAY) {

			return;
		}

		//dragging from ins. amt. to ins. balance and vice versa is meaningless
		if ((nCol == CHARGE_COLUMN_INS_AMT && nFromCol == CHARGE_COLUMN_INS_BALANCE)
			|| (nFromCol == CHARGE_COLUMN_INS_AMT && nCol == CHARGE_COLUMN_INS_BALANCE)) {

			return;
		}

		//now we know we are on the same row, using valid columns

		long nPatientID = VarLong(m_ChargeList->GetValue(nRow, CHARGE_COLUMN_PATIENT_ID));
		long nChargeID = VarLong(m_ChargeList->GetValue(nRow, CHARGE_COLUMN_CHARGE_ID));

		long nSourceInsuredPartyID = -1;
		if (nFromCol == CHARGE_COLUMN_PAT_AMT) {
			nSourceInsuredPartyID = -1;			
		}
		else if (nFromCol == CHARGE_COLUMN_INS_AMT || nFromCol == CHARGE_COLUMN_INS_BALANCE) {
			nSourceInsuredPartyID = VarLong(m_ChargeList->GetValue(nRow, CHARGE_COLUMN_INSURED_PARTY_ID), -1);
		}
		else if (nFromCol == CHARGE_COLUMN_COPAY) {
			nSourceInsuredPartyID = VarLong(m_ChargeList->GetValue(nRow, CHARGE_COLUMN_COPAY_INSURED_PARTY_ID), -1);
		}
		else {
			//shouldn't be possible
			ASSERT(FALSE);
			ThrowNxException("Drag & drop failed due to invalid source column ID %li.", nFromCol);
		}

		COleCurrency cySourceBalance = COleCurrency(0, 0);
		if (nSourceInsuredPartyID != -1) {
			cySourceBalance = GetChargeInsBalance(nChargeID, nPatientID, nSourceInsuredPartyID);
		}
		else {
			cySourceBalance = GetChargePatientBalance(nChargeID, nPatientID);
		}

		//if the source balance is zero, disallow shifting
		if (cySourceBalance == COleCurrency(0, 0)) {
			AfxMessageBox(FormatString("%s responsibility balance is %s, and therefore cannot be shifted.\n\n"
				"You must go to the patient's account to unapply items prior to shifting this balance.",
				(nSourceInsuredPartyID == -1 ? "The patient" : "This insurance"), FormatCurrencyForInterface(g_ccyZero)));
			return;
		}// (j.jones 2015-10-29 16:48) - PLID 

		long nDestInsuredPartyID = -1;
		if (nCol == CHARGE_COLUMN_PAT_AMT) {
			nDestInsuredPartyID = -1;
		}
		else if (nCol == CHARGE_COLUMN_INS_AMT || nCol == CHARGE_COLUMN_INS_BALANCE) {
			nDestInsuredPartyID = VarLong(m_ChargeList->GetValue(nRow, CHARGE_COLUMN_INSURED_PARTY_ID), -1);
		}
		else if (nCol == CHARGE_COLUMN_COPAY) {
			nDestInsuredPartyID = VarLong(m_ChargeList->GetValue(nRow, CHARGE_COLUMN_COPAY_INSURED_PARTY_ID), -1);
		}
		else {
			//shouldn't be possible
			ASSERT(FALSE);
			ThrowNxException("Drag & drop failed due to invalid destination column ID %li.", nCol);
		}

		if (nDestInsuredPartyID == nSourceInsuredPartyID) {
			//can't shift to itself
			if (nDestInsuredPartyID == -1 && (nFromCol == CHARGE_COLUMN_COPAY || nCol == CHARGE_COLUMN_COPAY)) {
				AfxMessageBox("The Copay column is showing patient responsibility, and therefore cannot be shifted to and from itself.");
			}
			else {
				//how else is this possible? return anyways, but a more accurate warning would be preferable
				ASSERT(FALSE);
				AfxMessageBox("You are attempting to shift responsibility for the same insured party. No shifting will occur.");
			}

			return;
		}

		//prompt to shift a balance
		CShiftInsRespsDlg dlg(this);
		dlg.m_ID = nChargeID;
		dlg.m_PatientID = nPatientID;
		dlg.m_strAuditFromProcess = "by manually dragging responsibility in the batch payments tab";

		dlg.m_cySrcAmount = cySourceBalance;
		dlg.m_nSrcInsPartyID = nSourceInsuredPartyID;
		dlg.m_nDstInsPartyID = nDestInsuredPartyID;

		//for capitation, we want to warn if they are going to shift the entire balance
		//if the source insurance is the one we're trying to post to
		long nCurInsuredPartyID = VarLong(m_ChargeList->GetValue(nRow, CHARGE_COLUMN_INSURED_PARTY_ID), -1);
		if (nSourceInsuredPartyID == nCurInsuredPartyID) {
			dlg.m_bWarnWhenShiftingEntireBalance = true;
		}

		//we also do not want to move the batch that the claim is in, if any
		dlg.m_bDoNotSwitchClaimBatches = true;

		dlg.m_strLineType = "Charge";
		if (dlg.DoModal() == IDOK) {

			//update the screen
			UpdateChargeListByChargeID(nChargeID);

			//try to set the focus back to the datalist, and the last item or nearby item
			GetDlgItem(IDC_APPLY_LIST)->SetFocus();
			if (m_ChargeList->SetSelByColumn(CHARGE_COLUMN_CHARGE_ID, nChargeID) == -1) {
				
				if (m_ChargeList->GetRowCount() - 1 < nRow)
					m_ChargeList->CurSel = m_ChargeList->GetRowCount() - 1;
				else
					m_ChargeList->CurSel = nRow;
			}
		}
		
	}NxCatchAll(__FUNCTION__);
}

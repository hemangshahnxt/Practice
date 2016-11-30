// SecondaryANSIClaimConfigDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SecondaryANSIClaimConfigDlg.h"
#include "GlobalFinancialUtils.h"
#include "SecondaryANSIAllowedAdjConfigDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;

// (j.jones 2006-11-24 13:37) - PLID 23415 - Created

/////////////////////////////////////////////////////////////////////////////
// CSecondaryANSIClaimConfigDlg dialog


CSecondaryANSIClaimConfigDlg::CSecondaryANSIClaimConfigDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CSecondaryANSIClaimConfigDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSecondaryANSIClaimConfigDlg)
		m_GroupID = -1;
		m_bIsUB92 = FALSE;
		m_bIs5010Enabled = FALSE;
	//}}AFX_DATA_INIT
}


void CSecondaryANSIClaimConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSecondaryANSIClaimConfigDlg)
	DDX_Control(pDX, IDC_RADIO_APPROVED_SEND_BALANCE, m_btnApprovedBalance);
	DDX_Control(pDX, IDC_RADIO_APPROVED_SEND_CLAIM_TOTAL, m_btnApprovedClaimTotal);
	DDX_Control(pDX, IDC_RADIO_APPROVED_SEND_ALLOWED, m_btnApprovedAllowed);
	DDX_Control(pDX, IDC_RADIO_USE_ADJUSTMENTS_IN_2430, m_btnSend2430Adj);
	DDX_Control(pDX, IDC_RADIO_USE_ADJUSTMENTS_IN_2320, m_btnSend2320Adj);
	DDX_Control(pDX, IDC_CHECK_SEND_2400_CONTRACT, m_btnSend2400Contract);
	DDX_Control(pDX, IDC_CHECK_SEND_2400_ALLOWED, m_btnSend2400Allowed);
	DDX_Control(pDX, IDC_CHECK_SEND_2320_APPROVED_AMOUNT, m_btnSend2320Approved);
	DDX_Control(pDX, IDC_CHECK_SEND_2320_ALLOWED, m_btnSend2320Allowed);
	DDX_Control(pDX, IDC_CHECK_SEND_2300_CONTRACT, m_btnSend2300Contract);
	DDX_Control(pDX, IDC_CHECK_ENABLE_SENDING_PAYMENT_INFORMATION, m_btnEnablePayNotPrimary);
	DDX_Control(pDX, IDC_2400_LABEL, m_nxstatic2400Label);
	DDX_Control(pDX, IDC_APPROVED_CALC_LABEL, m_nxstaticApprovedCalcLabel);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);	
	DDX_Control(pDX, IDC_CHECK_SEND_CAS_PR, m_checkSendCASPR);
	DDX_Control(pDX, IDC_BTN_CONFIG_ALLOWED_ADJ_CODES, m_btnConfigAllowedAdjCodes);
	DDX_Control(pDX, IDC_CHECK_HIDE_2430_SVD06_ONE_CHARGE, m_checkHide2430_SVD06_OneCharge);	
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSecondaryANSIClaimConfigDlg, CNxDialog)
	//{{AFX_MSG_MAP(CSecondaryANSIClaimConfigDlg)
	ON_BN_CLICKED(IDC_CHECK_ENABLE_SENDING_PAYMENT_INFORMATION, OnCheckEnableSendingPaymentInformation)
	ON_BN_CLICKED(IDC_RADIO_USE_ADJUSTMENTS_IN_2320, OnRadioUseAdjustmentsIn2320)
	ON_BN_CLICKED(IDC_RADIO_USE_ADJUSTMENTS_IN_2430, OnRadioUseAdjustmentsIn2430)	
	ON_BN_CLICKED(IDC_BTN_CONFIG_ALLOWED_ADJ_CODES, OnBtnConfigAllowedAdjCodes)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSecondaryANSIClaimConfigDlg message handlers

BOOL CSecondaryANSIClaimConfigDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
	
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		// (j.jones 2010-02-02 14:00) - PLID 37159 - added ability to configure adjustments
		// to be included in the allowed amount calculation
		m_btnConfigAllowedAdjCodes.AutoSet(NXB_MODIFY);

		// (j.jones 2010-10-19 13:33) - PLID 40931 - in 5010 these settings are obsolete
		if(m_bIs5010Enabled) {

			//none of these exist in 5010
			GetDlgItem(IDC_APPROVED_CALC_LABEL)->EnableWindow(FALSE);
			m_btnApprovedBalance.EnableWindow(FALSE);
			m_btnApprovedClaimTotal.EnableWindow(FALSE);
			m_btnApprovedAllowed.EnableWindow(FALSE);
			m_btnSend2400Allowed.EnableWindow(FALSE);
			m_btnSend2320Allowed.EnableWindow(FALSE);
			m_btnSend2320Approved.EnableWindow(FALSE);

			//remove "recommended" from the disabled controls, let's not taunt them
			m_btnSend2400Allowed.SetWindowText("Send Allowed Amount");
			m_btnSend2320Allowed.SetWindowText("Send Allowed Amount");
			m_btnApprovedAllowed.SetWindowText("Send the Allowed amount as the Approved amount");
		}

		// (j.jones 2007-06-15 09:43) - PLID 26309 - added group/reason code defaults
		// (j.jones 2009-03-11 09:32) - PLID 33446 - renamed the original combos to say "remaining",
		// and added new combos for "insurance"
		// (j.jones 2010-09-23 15:21) - PLID 40653 - these are now requeryable
		m_pRemainingGroupCodeList = BindNxDataList2Ctrl(this, IDC_DEFAULT_REM_ADJ_GROUPCODE, GetRemoteData(), true);
		m_pRemainingReasonList = BindNxDataList2Ctrl(this, IDC_DEFAULT_REM_ADJ_REASON, GetRemoteData(), true);

		m_pInsuranceGroupCodeList = BindNxDataList2Ctrl(this, IDC_DEFAULT_INS_ADJ_GROUPCODE, GetRemoteData(), true);
		m_pInsuranceReasonList = BindNxDataList2Ctrl(this, IDC_DEFAULT_INS_ADJ_REASON, GetRemoteData(), true);

		// (j.jones 2006-11-27 17:30) - PLID 23652 - supported UB92

		// (j.jones 2007-02-15 11:48) - PLID 24762 - added 2400 Allowed Amount option

		// (j.jones 2007-03-29 17:10) - PLID 25409 - added allowable calculation option

		if(m_bIsUB92) {

			//hide irrelevant controls
			// (j.jones 2009-03-11 10:20) - PLID 33446 - I rearranged these controls and
			// now the UB screen looks pretty ugly when these are hidden, so now I just
			// disable them - counting the labels, for effect
			GetDlgItem(IDC_CHECK_SEND_2320_APPROVED_AMOUNT)->EnableWindow(FALSE);
			GetDlgItem(IDC_2400_LABEL)->EnableWindow(FALSE);
			GetDlgItem(IDC_CHECK_SEND_2400_CONTRACT)->EnableWindow(FALSE);
			GetDlgItem(IDC_CHECK_SEND_2400_ALLOWED)->EnableWindow(FALSE);
			// (j.jones 2008-02-25 09:30) - PLID 29077 - hide the approved calculations,
			// there is no approved amount in a UB Institutional claim
			GetDlgItem(IDC_APPROVED_CALC_LABEL)->EnableWindow(FALSE);
			GetDlgItem(IDC_RADIO_APPROVED_SEND_BALANCE)->EnableWindow(FALSE);
			GetDlgItem(IDC_RADIO_APPROVED_SEND_CLAIM_TOTAL)->EnableWindow(FALSE);
			GetDlgItem(IDC_RADIO_APPROVED_SEND_ALLOWED)->EnableWindow(FALSE);

			// (j.jones 2008-11-12 12:02) - PLID 31912 - I coded the Send CAS PR feature
			// for the UB, but then discovered that we should never need to send this
			// CAS PR segment unless we send the allowed amount, and we don't send the
			// allowed amount on UB claims. Thus, this option is not necessary. However,
			// since I already coded it, I don't want to have to repeat this in the future,
			// so I am leaving the code in place and merely hiding the superfluous checkbox.
			GetDlgItem(IDC_CHECK_SEND_CAS_PR)->EnableWindow(FALSE);

			// (j.jones 2007-06-15 09:55) - PLID 26309 - added group/reason code defaults
			// (j.jones 2008-11-12 10:43) - PLID 31912 - added ANSI_SendCASPR
			// (j.jones 2009-03-11 09:58) - PLID 33446 - added insurance group/reason codes
			// (j.jones 2009-08-28 17:45) - PLID 32993 - removed ANSI_SecondaryAllowableCalc
			// (j.jones 2010-03-31 14:43) - PLID 37918 - added ANSI_Hide2430_SVD06_OneCharge
			// (j.jones 2010-09-23 15:42) - PLID 40653 - the group & reason codes are now IDs
			_RecordsetPtr rs = CreateParamRecordset("SELECT ANSI_EnablePaymentInfo, ANSI_AdjustmentLoop, "
				"ANSI_2300Contract, ANSI_2320Allowed, "
				"ANSI_DefaultAdjGroupCodeID, ANSI_DefaultAdjReasonCodeID, ANSI_SendCASPR, "
				"ANSI_DefaultInsAdjGroupCodeID, ANSI_DefaultInsAdjReasonCodeID, "
				"ANSI_Hide2430_SVD06_OneCharge "
				"FROM UB92SetupT WHERE ID = {INT}", m_GroupID);

			if(!rs->eof) {

				long nEnablePaymentInfo = AdoFldLong(rs, "ANSI_EnablePaymentInfo", 1);
				long nAdjustmentLoop = AdoFldLong(rs, "ANSI_AdjustmentLoop", 1);
				long n2300Contract = AdoFldLong(rs, "ANSI_2300Contract", 0);
				long n2320Allowed = AdoFldLong(rs, "ANSI_2320Allowed", 1);
				BOOL bSendCASPR = AdoFldBool(rs, "ANSI_SendCASPR", TRUE);

				// (j.jones 2007-06-15 09:55) - PLID 26309 - added group/reason code defaults
				_variant_t var = rs->Fields->Item["ANSI_DefaultAdjGroupCodeID"]->Value;
				// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
				m_pRemainingGroupCodeList->TrySetSelByColumn_Deprecated(0, var);
				var = rs->Fields->Item["ANSI_DefaultAdjReasonCodeID"]->Value;
				// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
				m_pRemainingReasonList->TrySetSelByColumn_Deprecated(0, var);

				// (j.jones 2009-03-11 09:56) - PLID 33446 - added defaults for "real" insurance adjustments
				var = rs->Fields->Item["ANSI_DefaultInsAdjGroupCodeID"]->Value;
				m_pInsuranceGroupCodeList->SetSelByColumn(0, var);
				var = rs->Fields->Item["ANSI_DefaultInsAdjReasonCodeID"]->Value;
				m_pInsuranceReasonList->SetSelByColumn(0, var);

				CheckDlgButton(IDC_CHECK_ENABLE_SENDING_PAYMENT_INFORMATION, nEnablePaymentInfo == 1);
				OnCheckEnableSendingPaymentInformation();
				
				CheckDlgButton(IDC_CHECK_ENABLE_SENDING_PAYMENT_INFORMATION, nEnablePaymentInfo == 1);
				CheckDlgButton(nAdjustmentLoop == 0 ? IDC_RADIO_USE_ADJUSTMENTS_IN_2320 : IDC_RADIO_USE_ADJUSTMENTS_IN_2430, TRUE);
				CheckDlgButton(IDC_CHECK_SEND_2300_CONTRACT, n2300Contract == 1);
				CheckDlgButton(IDC_CHECK_SEND_2320_ALLOWED, n2320Allowed == 1);

				// (j.jones 2007-03-29 17:15) - PLID 25409 - added ANSI_SecondaryAllowableCalc
				// (j.jones 2009-08-28 17:45) - PLID 32993 - removed ANSI_SecondaryAllowableCalc
				/*
				if(nSecondaryAllowableCalc == 1)
					CheckDlgButton(IDC_RADIO_ALLOWED_USE_FEE_SCHEDULE, TRUE);
				else if(nSecondaryAllowableCalc == 2)
					CheckDlgButton(IDC_RADIO_ALLOWED_USE_PAYMENT, TRUE);
				else
					CheckDlgButton(IDC_RADIO_ALLOWED_USE_PAYMENT_AND_RESP, TRUE);
				*/

				// (j.jones 2008-11-12 10:43) - PLID 31912 - added ANSI_SendCASPR
				CheckDlgButton(IDC_CHECK_SEND_CAS_PR, bSendCASPR);

				// (j.jones 2010-03-31 14:43) - PLID 37918 - added ability to hide 2430 SVD06 for single-charge claims
				m_checkHide2430_SVD06_OneCharge.SetCheck(AdoFldLong(rs, "ANSI_Hide2430_SVD06_OneCharge", 0) == 1);
			}
			rs->Close();

		}
		else {

			// (j.jones 2007-06-15 09:55) - PLID 26309 - added group/reason code defaults
			// (j.jones 2008-02-25 09:20) - PLID 29077 - added ANSI_SecondaryApprovedCalc
			// (j.jones 2008-11-12 10:43) - PLID 31912 - added ANSI_SendCASPR
			// (j.jones 2009-03-11 09:58) - PLID 33446 - added insurance group/reason codes
			// (j.jones 2009-08-28 17:45) - PLID 32993 - removed ANSI_SecondaryAllowableCalc
			// (j.jones 2010-03-31 14:43) - PLID 37918 - added ANSI_Hide2430_SVD06_OneCharge
			// (j.jones 2010-09-23 15:42) - PLID 40653 - the group & reason codes are now IDs
			_RecordsetPtr rs = CreateParamRecordset("SELECT ANSI_EnablePaymentInfo, ANSI_AdjustmentLoop, "
				"ANSI_2300Contract, ANSI_2320Allowed, "
				"ANSI_2320Approved, ANSI_2400Contract, ANSI_2400Allowed, "
				"ANSI_DefaultAdjGroupCodeID, ANSI_DefaultAdjReasonCodeID, "
				"ANSI_SecondaryApprovedCalc, ANSI_SendCASPR, "
				"ANSI_DefaultInsAdjGroupCodeID, ANSI_DefaultInsAdjReasonCodeID, "
				"ANSI_Hide2430_SVD06_OneCharge "
				"FROM HCFASetupT WHERE ID = {INT}", m_GroupID);

			if(!rs->eof) {

				long nEnablePaymentInfo = AdoFldLong(rs, "ANSI_EnablePaymentInfo", 1);
				long nAdjustmentLoop = AdoFldLong(rs, "ANSI_AdjustmentLoop", 1);
				long n2300Contract = AdoFldLong(rs, "ANSI_2300Contract", 0);
				long n2320Allowed = AdoFldLong(rs, "ANSI_2320Allowed", 1);
				long n2320Approved = AdoFldLong(rs, "ANSI_2320Approved", 0);
				long n2400Contract = AdoFldLong(rs, "ANSI_2400Contract", 0);
				long n2400Allowed = AdoFldLong(rs, "ANSI_2400Allowed", 1);
				long nSecondaryApprovedCalc = AdoFldLong(rs, "ANSI_SecondaryApprovedCalc", 3);
				BOOL bSendCASPR = AdoFldBool(rs, "ANSI_SendCASPR", TRUE);
				
				// (j.jones 2007-06-15 09:55) - PLID 26309 - added group/reason code defaults
				_variant_t var = rs->Fields->Item["ANSI_DefaultAdjGroupCodeID"]->Value;
				// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
				m_pRemainingGroupCodeList->TrySetSelByColumn_Deprecated(0, var);
				var = rs->Fields->Item["ANSI_DefaultAdjReasonCodeID"]->Value;
				// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
				m_pRemainingReasonList->TrySetSelByColumn_Deprecated(0, var);

				// (j.jones 2009-03-11 09:56) - PLID 33446 - added defaults for "real" insurance adjustments
				var = rs->Fields->Item["ANSI_DefaultInsAdjGroupCodeID"]->Value;
				m_pInsuranceGroupCodeList->SetSelByColumn(0, var);
				var = rs->Fields->Item["ANSI_DefaultInsAdjReasonCodeID"]->Value;
				m_pInsuranceReasonList->SetSelByColumn(0, var);

				CheckDlgButton(IDC_CHECK_ENABLE_SENDING_PAYMENT_INFORMATION, nEnablePaymentInfo == 1);
				OnCheckEnableSendingPaymentInformation();
				
				CheckDlgButton(IDC_CHECK_ENABLE_SENDING_PAYMENT_INFORMATION, nEnablePaymentInfo == 1);
				CheckDlgButton(nAdjustmentLoop == 0 ? IDC_RADIO_USE_ADJUSTMENTS_IN_2320 : IDC_RADIO_USE_ADJUSTMENTS_IN_2430, TRUE);
				CheckDlgButton(IDC_CHECK_SEND_2300_CONTRACT, n2300Contract == 1);
				CheckDlgButton(IDC_CHECK_SEND_2320_ALLOWED, n2320Allowed == 1);
				CheckDlgButton(IDC_CHECK_SEND_2320_APPROVED_AMOUNT, n2320Approved == 1);
				CheckDlgButton(IDC_CHECK_SEND_2400_CONTRACT, n2400Contract == 1);
				CheckDlgButton(IDC_CHECK_SEND_2400_ALLOWED, n2400Allowed == 1);

				// (j.jones 2007-03-29 17:18) - PLID 25409 - added ANSI_SecondaryAllowableCalc
				// (j.jones 2009-08-28 17:45) - PLID 32993 - removed ANSI_SecondaryAllowableCalc
				/*
				if(nSecondaryAllowableCalc == 1)
					CheckDlgButton(IDC_RADIO_ALLOWED_USE_FEE_SCHEDULE, TRUE);
				else if(nSecondaryAllowableCalc == 2)
					CheckDlgButton(IDC_RADIO_ALLOWED_USE_PAYMENT, TRUE);
				else
					CheckDlgButton(IDC_RADIO_ALLOWED_USE_PAYMENT_AND_RESP, TRUE);
				*/

				// (j.jones 2008-02-25 09:32) - PLID 29077 - added ANSI_SecondaryApprovedCalc
				if(nSecondaryApprovedCalc == 1) {
					CheckDlgButton(IDC_RADIO_APPROVED_SEND_BALANCE, TRUE);
				}
				else if(nSecondaryApprovedCalc == 2) {
					CheckDlgButton(IDC_RADIO_APPROVED_SEND_CLAIM_TOTAL, TRUE);
				}
				else {
					CheckDlgButton(IDC_RADIO_APPROVED_SEND_ALLOWED, TRUE);
				}

				// (j.jones 2008-11-12 10:43) - PLID 31912 - added ANSI_SendCASPR
				CheckDlgButton(IDC_CHECK_SEND_CAS_PR, bSendCASPR);

				// (j.jones 2010-03-31 14:43) - PLID 37918 - added ability to hide 2430 SVD06 for single-charge claims
				m_checkHide2430_SVD06_OneCharge.SetCheck(AdoFldLong(rs, "ANSI_Hide2430_SVD06_OneCharge", 0) == 1);
			}
			rs->Close();
		}

	}NxCatchAll("Error in CSecondaryANSIClaimConfigDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSecondaryANSIClaimConfigDlg::OnCheckEnableSendingPaymentInformation() 
{
	BOOL bEnabled = IsDlgButtonChecked(IDC_CHECK_ENABLE_SENDING_PAYMENT_INFORMATION);

	// (j.jones 2009-03-11 10:20) - PLID 33446 - non-UB controls are now disabled,
	// rather than hidden, so they need to stay that way in this function

	GetDlgItem(IDC_CHECK_SEND_2300_CONTRACT)->EnableWindow(bEnabled);
	GetDlgItem(IDC_CHECK_SEND_2400_CONTRACT)->EnableWindow(bEnabled && !m_bIsUB92);
	GetDlgItem(IDC_RADIO_USE_ADJUSTMENTS_IN_2320)->EnableWindow(bEnabled);
	GetDlgItem(IDC_RADIO_USE_ADJUSTMENTS_IN_2430)->EnableWindow(bEnabled);

	// (j.jones 2010-10-19 13:33) - PLID 40931 - in 5010 these settings are obsolete
	GetDlgItem(IDC_CHECK_SEND_2320_ALLOWED)->EnableWindow(bEnabled && !m_bIs5010Enabled);	
	GetDlgItem(IDC_CHECK_SEND_2320_APPROVED_AMOUNT)->EnableWindow(bEnabled && !m_bIsUB92 && !m_bIs5010Enabled);		
	GetDlgItem(IDC_CHECK_SEND_2400_ALLOWED)->EnableWindow(bEnabled && !m_bIsUB92 && !m_bIs5010Enabled);		
	GetDlgItem(IDC_RADIO_APPROVED_SEND_BALANCE)->EnableWindow(bEnabled && !m_bIsUB92 && !m_bIs5010Enabled);
	GetDlgItem(IDC_RADIO_APPROVED_SEND_CLAIM_TOTAL)->EnableWindow(bEnabled && !m_bIsUB92 && !m_bIs5010Enabled);
	GetDlgItem(IDC_RADIO_APPROVED_SEND_ALLOWED)->EnableWindow(bEnabled && !m_bIsUB92 && !m_bIs5010Enabled);
	
	m_pRemainingGroupCodeList->Enabled = bEnabled;
	m_pRemainingReasonList->Enabled = bEnabled;
	m_pInsuranceGroupCodeList->Enabled = bEnabled;
	m_pInsuranceReasonList->Enabled = bEnabled;

	GetDlgItem(IDC_CHECK_SEND_CAS_PR)->EnableWindow(bEnabled && !m_bIsUB92);

	// (j.jones 2010-02-02 14:00) - PLID 37159 - the allowed adjustment configuration
	// is used on both HCFAs and UBs
	GetDlgItem(IDC_BTN_CONFIG_ALLOWED_ADJ_CODES)->EnableWindow(bEnabled);

	// (j.jones 2010-03-31 14:43) - PLID 37918 - added ability to hide 2430 SVD06 for single-charge claims
	GetDlgItem(IDC_CHECK_HIDE_2430_SVD06_ONE_CHARGE)->EnableWindow(bEnabled);
}

void CSecondaryANSIClaimConfigDlg::OnOK()
{
	try {

		//save the settings

		long nEnablePaymentInfo = IsDlgButtonChecked(IDC_CHECK_ENABLE_SENDING_PAYMENT_INFORMATION) ? 1 : 0;
		long nAdjustmentLoop = IsDlgButtonChecked(IDC_RADIO_USE_ADJUSTMENTS_IN_2430) ? 1 : 0;
		long n2300Contract = IsDlgButtonChecked(IDC_CHECK_SEND_2300_CONTRACT) ? 1 : 0;
		long n2320Allowed = IsDlgButtonChecked(IDC_CHECK_SEND_2320_ALLOWED) ? 1 : 0;		
		long n2320Approved = IsDlgButtonChecked(IDC_CHECK_SEND_2320_APPROVED_AMOUNT) ? 1 : 0;
		long n2400Contract = IsDlgButtonChecked(IDC_CHECK_SEND_2400_CONTRACT) ? 1 : 0;
		// (j.jones 2007-02-15 11:48) - PLID 24762 - added 2400 Allowed Amount option
		long n2400Allowed = IsDlgButtonChecked(IDC_CHECK_SEND_2400_ALLOWED) ? 1: 0;
		// (j.jones 2008-11-12 10:47) - PLID 31912 - added m_checkSendCASPR
		BOOL bSendCASPR = m_checkSendCASPR.GetCheck();

		// (j.jones 2007-03-29 17:10) - PLID 25409 - added allowable calculation option
		// (j.jones 2009-08-31 09:16) - PLID 32993 - removed ANSI_SecondaryAllowableCalc
		/*
		long nSecondaryAllowableCalc = 3;
		if(IsDlgButtonChecked(IDC_RADIO_ALLOWED_USE_FEE_SCHEDULE))
			nSecondaryAllowableCalc = 1;
		else if(IsDlgButtonChecked(IDC_RADIO_ALLOWED_USE_PAYMENT))
			nSecondaryAllowableCalc = 2;
		*/

		// (j.jones 2008-02-25 09:37) - PLID 29077 - added approved calculation option
		long nSecondaryApprovedCalc = 3;
		if(IsDlgButtonChecked(IDC_RADIO_APPROVED_SEND_BALANCE)) {
			nSecondaryApprovedCalc = 1;
		}
		else if(IsDlgButtonChecked(IDC_RADIO_APPROVED_SEND_CLAIM_TOTAL)) {
			nSecondaryApprovedCalc = 2;
		}

		// (j.jones 2007-06-15 09:58) - PLID 26309 - added group/reason code defaults
		// (j.jones 2010-09-23 15:42) - PLID 40653 - the group & reason codes are now IDs
		long nRemGroupCodeID = -1, nRemReasonCodeID = -1;
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pRemainingGroupCodeList->GetCurSel();
		if (pRow) {
			nRemGroupCodeID = VarLong(pRow->GetValue(0));
		}
		else {
			AfxMessageBox("You must select a default adjustment group code for remaining balances before saving.");
			return;
		}

		pRow = m_pRemainingReasonList->GetCurSel();
		if (pRow) {
			nRemReasonCodeID = VarLong(pRow->GetValue(0));
		}
		else {
			AfxMessageBox("You must select a default adjustment reason code for remaining balances before saving.");
			return;
		}

		// (j.jones 2009-03-11 09:56) - PLID 33446 - added defaults for "real" insurance adjustments
		// (j.jones 2010-09-23 15:42) - PLID 40653 - the group & reason codes are now IDs
		long nInsGroupCodeID = -1, nInsReasonCodeID = -1;
		pRow = m_pInsuranceGroupCodeList->GetCurSel();
		if (pRow) {
			nInsGroupCodeID = VarLong(pRow->GetValue(0));
		}
		else {
			AfxMessageBox("You must select a default adjustment group code for insurance adjustments before saving.");
			return;
		}

		pRow = m_pInsuranceReasonList->GetCurSel();
		if (pRow) {
			nInsReasonCodeID = VarLong(pRow->GetValue(0));
		}
		else {
			AfxMessageBox("You must select a default adjustment reason code for insurance adjustments before saving.");
			return;
		}

		// (j.jones 2010-03-31 14:43) - PLID 37918 - added ability to hide 2430 SVD06 for single-charge claims
		long nANSI_Hide2430_SVD06_OneCharge = m_checkHide2430_SVD06_OneCharge.GetCheck() ? 1 : 0;

		// (j.jones 2006-11-27 17:30) - PLID 23652 - supported UB92

		if(m_bIsUB92) {
		
			// (j.jones 2007-06-15 09:58) - PLID 26309 - added group/reason code defaults
			// (j.jones 2008-11-12 10:47) - PLID 31912 - added ANSI_SendCASPR
			// (j.jones 2009-03-11 09:58) - PLID 33446 - added insurance group/reason codes
			// (j.jones 2009-08-28 17:45) - PLID 32993 - removed ANSI_SecondaryAllowableCalc
			// (j.jones 2010-03-31 14:43) - PLID 37918 - added ANSI_Hide2430_SVD06_OneCharge
			// (j.jones 2010-09-23 15:42) - PLID 40653 - the group & reason codes are now IDs
			ExecuteSql("UPDATE UB92SetupT SET ANSI_EnablePaymentInfo = %li, ANSI_AdjustmentLoop = %li, "
				"ANSI_2300Contract = %li, ANSI_2320Allowed = %li, "
				"ANSI_DefaultAdjGroupCodeID = %li, ANSI_DefaultAdjReasonCodeID = %li, "
				"ANSI_SendCASPR = %li, "
				"ANSI_DefaultInsAdjGroupCodeID = %li, ANSI_DefaultInsAdjReasonCodeID = %li, "
				"ANSI_Hide2430_SVD06_OneCharge = %li "
				"WHERE ID = %li",
				nEnablePaymentInfo, nAdjustmentLoop,
				n2300Contract, n2320Allowed,
				nRemGroupCodeID, nRemReasonCodeID, bSendCASPR ? 1 : 0,
				nInsGroupCodeID, nInsReasonCodeID,
				nANSI_Hide2430_SVD06_OneCharge,
				m_GroupID);
		}
		else {

			// (j.jones 2007-06-15 09:58) - PLID 26309 - added group/reason code defaults
			// (j.jones 2008-02-25 09:20) - PLID 29077 - added ANSI_SecondaryApprovedCalc
			// (j.jones 2008-11-12 10:47) - PLID 31912 - added ANSI_SendCASPR
			// (j.jones 2009-03-11 09:58) - PLID 33446 - added insurance group/reason codes
			// (j.jones 2009-08-28 17:45) - PLID 32993 - removed ANSI_SecondaryAllowableCalc
			// (j.jones 2010-03-31 14:43) - PLID 37918 - added ANSI_Hide2430_SVD06_OneCharge
			// (j.jones 2010-09-23 15:42) - PLID 40653 - the group & reason codes are now IDs
			ExecuteSql("UPDATE HCFASetupT SET ANSI_EnablePaymentInfo = %li, ANSI_AdjustmentLoop = %li, "
				"ANSI_2300Contract = %li, ANSI_2320Allowed = %li, "
				"ANSI_2320Approved = %li, ANSI_2400Contract = %li, ANSI_2400Allowed = %li, "
				"ANSI_DefaultAdjGroupCodeID = %li, ANSI_DefaultAdjReasonCodeID = %li, "
				"ANSI_SecondaryApprovedCalc = %li, ANSI_SendCASPR = %li, "
				"ANSI_DefaultInsAdjGroupCodeID = %li, ANSI_DefaultInsAdjReasonCodeID = %li, "
				"ANSI_Hide2430_SVD06_OneCharge = %li "
				"WHERE ID = %li",
				nEnablePaymentInfo, nAdjustmentLoop,
				n2300Contract, n2320Allowed,
				n2320Approved, n2400Contract, n2400Allowed, 
				nRemGroupCodeID, nRemReasonCodeID,
				nSecondaryApprovedCalc, bSendCASPR ? 1 : 0,
				nInsGroupCodeID, nInsReasonCodeID,
				nANSI_Hide2430_SVD06_OneCharge,
				m_GroupID);
		}

		CDialog::OnOK();

	}NxCatchAll("Error saving secondary ANSI claim information.");
}

void CSecondaryANSIClaimConfigDlg::OnRadioUseAdjustmentsIn2320() 
{
	//only one adjustment option can be checked ("in the end, there can be only one")
	if(IsDlgButtonChecked(IDC_RADIO_USE_ADJUSTMENTS_IN_2320)) {
		CheckDlgButton(IDC_RADIO_USE_ADJUSTMENTS_IN_2430, FALSE);
	}
}

void CSecondaryANSIClaimConfigDlg::OnRadioUseAdjustmentsIn2430() 
{
	//only one adjustment option can be checked ("in the end, there can be only one")
	if(IsDlgButtonChecked(IDC_RADIO_USE_ADJUSTMENTS_IN_2430)) {
		CheckDlgButton(IDC_RADIO_USE_ADJUSTMENTS_IN_2320, FALSE);
	}
}

// (j.jones 2010-02-02 14:00) - PLID 37159 - added ability to configure adjustments
// to be included in the allowed amount calculation
void CSecondaryANSIClaimConfigDlg::OnBtnConfigAllowedAdjCodes()
{
	try {

		CSecondaryANSIAllowedAdjConfigDlg dlg(this);
		dlg.m_nGroupID = m_GroupID;
		dlg.m_bIsUB = m_bIsUB92;
		dlg.DoModal();

	}NxCatchAll(__FUNCTION__);
}
// AccountWriteOffDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AccountWriteOffDlg.h"
#include "EditComboBox.h"
#include "GlobalFinancialUtils.h"
#include "InternationalUtils.h"
#include "AuditTrail.h"
#include "GlobalAuditUtils.h"
#include "GetNewIDName.h"
#include "Reports.h"
#include "ReportInfo.h"
#include "GlobalReportUtils.h"
#include "PayCatDlg.h" // (j.gruber 2012-11-15 13:55) - PLID 53752
#include "FinancialRc.h"

// (a.walling 2010-01-21 16:43) - PLID 37021 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (j.jones 2008-06-27 13:37) - PLID 27647 - created

/////////////////////////////////////////////////////////////////////////////
// CAccountWriteOffDlg dialog

using namespace NXDATALIST2Lib;
using namespace ADODB;

enum ResultsListColumn {

	rlcChargeID = 0, // (c.haag 2015-04-24) - PLID 65693 - We need to get results by charge, not by bill
	rlcPatientID,
	rlcPatName,
	rlcBillDate,
	rlcInputDate,
	rlcLastSentDate,
	rlcFirstAssigned,
	rlcLastPayDate,
	rlcInsuredPartyID,
	rlcInsCoName,
	rlcRespName,
	rlcTotalCharges,
	rlcTotalPayments,
	rlcBalance,
};

enum CategoryComboColumn {

	cccID = 0,
	cccName,
};

// (j.jones 2010-09-23 11:15) - PLID 40653 - added enums for group & reason codes
enum GroupCodeComboColumn {

	gcccID = 0,
	gcccCode,
	gcccDescription,
};

enum ReasonCodeComboColumn {

	rcccID = 0,
	rcccCode,
	rcccDescription,
};

// (a.walling 2012-02-09 17:13) - PLID 45448 - NxAdo unification - Use CIncreaseCommandTimeout instead of accessing g_ptrRemoteData

CAccountWriteOffDlg::CAccountWriteOffDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CAccountWriteOffDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAccountWriteOffDlg)
		m_bAdjustmentsMade = FALSE;
	//}}AFX_DATA_INIT
}


void CAccountWriteOffDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);	
	//{{AFX_DATA_MAP(CAccountWriteOffDlg)
	DDX_Control(pDX, IDC_BTN_EDIT_PAY_CAT, m_btnEditPayCat);
	DDX_Control(pDX, IDC_ADJ_DESC, m_editAdjDesc);
	DDX_Control(pDX, IDC_PATIENTS_TOTAL_LABEL, m_nxstaticPatientTotalLabel);
	DDX_Control(pDX, IDC_RESULTS_INFO_LABEL, m_nxstaticResultsInfoLabel);
	DDX_Control(pDX, IDC_RADIO_WRITE_OFF_ALL_BALANCES, m_radioAllBalances);
	DDX_Control(pDX, IDC_RADIO_WRITE_OFF_PATIENT_BALANCES, m_radioPatBalances);
	DDX_Control(pDX, IDC_RADIO_WRITE_OFF_INSURANCE_BALANCES, m_radioInsBalances);
	DDX_Control(pDX, IDC_PATIENT_TOTAL_COUNT, m_nxstaticPatientTotal);
	DDX_Control(pDX, IDC_CLAIM_TO_DATE, m_dtTo);
	DDX_Control(pDX, IDC_CLAIM_FROM_DATE, m_dtFrom);
	DDX_Control(pDX, IDC_LABEL_SEARCH_PARAMETERS, m_nxstaticSearchLabel);
	DDX_Control(pDX, IDC_ADJUSTMENT_INFO_LABEL, m_nxstaticAdjInfoLabel);
	DDX_Control(pDX, IDC_EDIT_LESS_THAN_DOLLARS, m_editLessThanAmount);
	DDX_Control(pDX, IDC_EDIT_DAYS_OLD, m_editDaysOld);
	DDX_Control(pDX, IDC_RADIO_WRITE_OFF_CLAIM_DATE, m_radioLastSentDate);
	DDX_Control(pDX, IDC_RADIO_WRITE_OFF_BILL_DATE, m_radioBillServiceDate);
	DDX_Control(pDX, IDC_RADIO_WRITE_OFF_BILL_INPUT_DATE, m_radioBillInputDate);
	DDX_Control(pDX, IDC_RADIO_WRITE_OFF_LAST_PAYMENT_DATE, m_radioLastPaymentDate);
	DDX_Control(pDX, IDC_RADIO_WRITE_OFF_GREATER_THAN_DAYS, m_radioGreaterThanDays);
	DDX_Control(pDX, IDC_RADIO_WRITE_OFF_BETWEEN_DATE_RANGE, m_radioDateRange);
	DDX_Control(pDX, IDC_CHECK_USE_DATE_RANGE, m_checkUseDateFilters);
	DDX_Control(pDX, IDC_CHECK_USE_BALANCE_LESS_THAN, m_checkBalanceLessThan);
	DDX_Control(pDX, IDC_EDIT_ADJ_DESC, m_btnEditAdjDesc);
	DDX_Control(pDX, IDC_RADIO_WRITE_OFF_BY_BILL, m_radioSearchByBill);
	DDX_Control(pDX, IDC_RADIO_WRITE_OFF_BY_ACCOUNT, m_radioSearchByAccount);
	DDX_Control(pDX, IDC_BTN_WRITE_OFF_DISPLAY_RESULTS, m_btnDisplayResults);
	DDX_Control(pDX, IDC_BTN_WRITE_OFF_CLOSE, m_btnClose);
	DDX_Control(pDX, IDC_BTN_ADJUST, m_btnAdjust);
	DDX_Control(pDX, IDC_RADIO_WRITE_OFF_ASSIGNMENT_DATE, m_radioAssignmentDate);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAccountWriteOffDlg, CNxDialog)
	//{{AFX_MSG_MAP(CAccountWriteOffDlg)
	ON_BN_CLICKED(IDC_BTN_ADJUST, OnBtnAdjust)
	ON_BN_CLICKED(IDC_BTN_WRITE_OFF_CLOSE, OnBtnWriteOffClose)
	ON_BN_CLICKED(IDC_BTN_WRITE_OFF_DISPLAY_RESULTS, OnBtnDisplayResults)
	ON_BN_CLICKED(IDC_RADIO_WRITE_OFF_BY_ACCOUNT, OnRadioWriteOffByAccount)
	ON_BN_CLICKED(IDC_RADIO_WRITE_OFF_BY_BILL, OnRadioWriteOffByBill)
	ON_BN_CLICKED(IDC_EDIT_ADJ_DESC, OnEditAdjDesc)
	ON_BN_CLICKED(IDC_CHECK_USE_DATE_RANGE, OnCheckUseDateRange)
	ON_BN_CLICKED(IDC_CHECK_USE_BALANCE_LESS_THAN, OnCheckUseBalanceLessThan)
	ON_BN_CLICKED(IDC_RADIO_WRITE_OFF_GREATER_THAN_DAYS, OnRadioWriteOffGreaterThanDays)
	ON_BN_CLICKED(IDC_RADIO_WRITE_OFF_BETWEEN_DATE_RANGE, OnRadioWriteOffBetweenDateRange)
	ON_BN_CLICKED(IDC_RADIO_WRITE_OFF_ALL_BALANCES, OnRadioWriteOffAllBalances)
	ON_BN_CLICKED(IDC_RADIO_WRITE_OFF_PATIENT_BALANCES, OnRadioWriteOffPatientBalances)
	ON_BN_CLICKED(IDC_RADIO_WRITE_OFF_INSURANCE_BALANCES, OnRadioWriteOffInsuranceBalances)
	ON_BN_CLICKED(IDC_BTN_EDIT_PAY_CAT, OnBtnEditPayCat)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAccountWriteOffDlg message handlers

BOOL CAccountWriteOffDlg::OnInitDialog() 
{		
	try {

		CNxDialog::OnInitDialog();

		m_btnClose.AutoSet(NXB_CLOSE);
		m_btnAdjust.AutoSet(NXB_MODIFY);
		m_btnDisplayResults.AutoSet(NXB_INSPECT);

		m_ResultsList = BindNxDataList2Ctrl(IDC_WRITE_OFF_LIST, false);
		m_DescriptionCombo = BindNxDataList2Ctrl(IDC_ADJ_DESCRIPTION, true);
		m_CategoryCombo = BindNxDataList2Ctrl(IDC_ADJ_CATEGORY, true);

		// (j.jones 2010-09-23 11:15) - PLID 40653 - these are now requeryable lists
		m_GroupCodeCombo = BindNxDataList2Ctrl(IDC_ADJ_GROUPCODE, true);
		m_ReasonCodeCombo = BindNxDataList2Ctrl(IDC_ADJ_REASON, true);

		// (j.jones 2010-09-23 11:15) - PLID 40653 - need to add "no code" options
		IRowSettingsPtr pRow = m_GroupCodeCombo->GetNewRow();
		pRow->PutValue(gcccID, (long)-1);
		pRow->PutValue(gcccCode, "");
		pRow->PutValue(gcccDescription, "<No Group Code>");
		m_GroupCodeCombo->AddRowSorted(pRow, NULL);
		m_GroupCodeCombo->SetSelByColumn(gcccID, (long)-1);

		pRow = m_ReasonCodeCombo->GetNewRow();
		pRow->PutValue(rcccID, (long)-1);
		pRow->PutValue(rcccCode, "");
		pRow->PutValue(rcccDescription, "<No Reason Code>");
		m_ReasonCodeCombo->AddRowSorted(pRow, NULL);
		m_ReasonCodeCombo->SetSelByColumn(rcccID, (long)-1);

		pRow = m_CategoryCombo->GetNewRow();
		pRow->PutValue(cccID, (long)0);
		pRow->PutValue(cccName,_bstr_t(" <No Category Selected>"));
		m_CategoryCombo->AddRowSorted(pRow, NULL);
		m_CategoryCombo->SetSelByColumn(cccID, (long)0);

		//default searching by account and all balances
		m_radioSearchByAccount.SetCheck(TRUE);
		m_radioAllBalances.SetCheck(TRUE);
		OnChangeSearchOptions();

		//set default to filter on bills older than 120 days
		SetDlgItemInt(IDC_EDIT_DAYS_OLD, 120);
		m_checkUseDateFilters.SetCheck(TRUE);
		m_radioGreaterThanDays.SetCheck(TRUE);
		m_radioBillServiceDate.SetCheck(TRUE);
		m_dtFrom.SetValue(_variant_t(COleDateTime::GetCurrentTime()));
		m_dtTo.SetValue(_variant_t(COleDateTime::GetCurrentTime()));
		OnCheckUseDateRange();

		//set no default for balance
		OnCheckUseBalanceLessThan();

		m_editAdjDesc.SetLimitText(255);

	}NxCatchAll("Error in CAccountWriteOffDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAccountWriteOffDlg::OnBtnAdjust() 
{
	long nAuditTransactionID = -1;

	try {

		if(m_ResultsList->GetRowCount() == 0) {
			AfxMessageBox("You must have results in the list before you can make adjustments.");
			return;
		}

		//first warn about whether they wish to continue
		if(IDNO == MessageBox("This action will adjust all accounts in this list to zero. Are you sure you wish to begin this process?", "Practice", MB_ICONEXCLAMATION|MB_YESNO)) {
			return;
		}

		CWaitCursor pWait1;

		//Run through the list, calculate what adjustments need to be made,
		//and create batch execute statements and audits for each adjustment.
		//Meanwhile, tally up the number of patients affected, the number of
		//adjustments created, and total amount to be adjusted. Warn with these
		//numbers before committing the adjustments.

		CString strSqlBatch;
		long nAdjustmentCount = 0;
		COleCurrency cyAdjustmentTotals = COleCurrency(0,0);
		CArray<long, long> aryPatientIDs;

		CString strDescription;
		GetDlgItemText(IDC_ADJ_DESC, strDescription);

		long nCategoryID = 0;
		IRowSettingsPtr pCatRow = m_CategoryCombo->GetCurSel();
		if(pCatRow) {
			nCategoryID = VarLong(pCatRow->GetValue(cccID), 0);
		}

		CString strGroupCodeID = "NULL";
		// (j.jones 2010-09-23 11:15) - PLID 40653 - we now use an ID, not the code itself
		IRowSettingsPtr pGroupRow = m_GroupCodeCombo->GetCurSel();
		if (pGroupRow) {
			long nGroupCode = VarLong(pGroupRow->GetValue(gcccID), -1);
			if(nGroupCode != -1) {
				strGroupCodeID.Format("%li", nGroupCode);
			}
		}

		CString strReasonCodeID = "NULL";
		IRowSettingsPtr pReasonRow = m_ReasonCodeCombo->GetCurSel();
		if (pReasonRow) {
			long nReasonCode = VarLong(pReasonRow->GetValue(rcccID), -1);
			if(nReasonCode != -1) {
				strReasonCodeID.Format("%li", nReasonCode);
			}
		}

		IRowSettingsPtr pRow = m_ResultsList->GetFirstRow();
		while(pRow) {

			long nChargeID = VarLong(pRow->GetValue(rlcChargeID), -1);
			long nPatientID = VarLong(pRow->GetValue(rlcPatientID), -1);
			long nInsuredPartyID = VarLong(pRow->GetValue(rlcInsuredPartyID), -1);

			_RecordsetPtr rs;

			if (nChargeID != -1) {
				//find all charges on this bill with a balance for this insured party ID, and adjust each one
				
				// (j.jones 2011-08-17 11:00) - PLID 44888 - ignore "original" and "void" line items
				// (c.haag 2015-04-24) - PLID 65693 - This query is now contained in a R"( literal for easier debugging and maintenance
				rs = CreateParamRecordset(R"(
SELECT PatientID, ChargeRespID, ChargeID, TotalCharges - TotalPays AS Balance, 
InsuredPartyID, PatientName, DoctorsProviders, LocationID 
FROM 
(SELECT BillsT.ID AS BillID, BillsT.PatientID, RespQ.ID AS ChargeRespID, RespQ.ChargeID, RespQ.TotalCharges, RespQ.InsuredPartyID, 
Convert(money, SUM(CASE WHEN PaysQ.Amount IS NULL THEN 0 ELSE PaysQ.Amount END)) AS TotalPays, 
PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatientName, 
ChargesT.DoctorsProviders, LineItemT.LocationID 
FROM BillsT 
INNER JOIN PersonT ON BillsT.PatientID = PersonT.ID 
INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID 
INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID 
LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID 
LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID 
INNER JOIN (SELECT ChargeRespT.ID, ChargeID, SUM(ChargeRespT.Amount) AS TotalCharges, 
	CASE WHEN InsuredPartyID IS NULL THEN -1 ELSE InsuredPartyID END AS InsuredPartyID 
	FROM ChargeRespT 
	LEFT JOIN ChargesT ON ChargeRespT.ChargeID = ChargesT.ID 
	INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID 
	LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID 
	LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID 
	WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 10 
	AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null 
	AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null 
	GROUP BY ChargeRespT.ID, ChargeID, InsuredPartyID 
	) RespQ ON ChargesT.ID = RespQ.ChargeID 
LEFT JOIN (SELECT AppliesT.RespID, SUM(AppliesT.Amount) AS Amount 
	FROM PaymentsT 
	INNER JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID 
	INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID 
	LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalPaymentsQ ON LineItemT.ID = LineItemCorrections_OriginalPaymentsQ.OriginalLineItemID 
	LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingPaymentsQ ON LineItemT.ID = LineItemCorrections_VoidingPaymentsQ.VoidingLineItemID 
	WHERE LineItemT.Deleted = 0 
	AND LineItemCorrections_OriginalPaymentsQ.OriginalLineItemID Is Null 
	AND LineItemCorrections_VoidingPaymentsQ.VoidingLineItemID Is Null 
	GROUP BY AppliesT.RespID
	) PaysQ ON RespQ.ID = PaysQ.RespID 
WHERE LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null 
AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null 
GROUP BY BillsT.ID, BillsT.PatientID, RespQ.ID, RespQ.ChargeID, RespQ.TotalCharges, RespQ.InsuredPartyID, 
PersonT.Last, PersonT.First, PersonT.Middle, ChargesT.DoctorsProviders, LineItemT.LocationID 
) AS SubQ 
WHERE (TotalCharges - TotalPays) > CONVERT(money,'0') 
AND ChargeID = {INT} AND InsuredPartyID = {INT} 
)"
				, nChargeID, nInsuredPartyID);
			}
			else {
				//find all charges for this patient with a balance for this insured party ID, and adjust each one

				// (j.jones 2011-08-17 11:00) - PLID 44888 - ignore "original" and "void" line items
				// (c.haag 2015-04-24) - PLID 65693 - This query is now contained in a R"( literal for easier debugging and maintenance
				rs = CreateParamRecordset(R"(
SELECT PatientID, ChargeRespID, ChargeID, TotalCharges - TotalPays AS Balance, 
InsuredPartyID, PatientName, DoctorsProviders, LocationID 
FROM 
(SELECT BillsT.ID AS BillID, BillsT.PatientID, RespQ.ID AS ChargeRespID, RespQ.ChargeID, RespQ.TotalCharges, RespQ.InsuredPartyID, 
Convert(money, SUM(CASE WHEN PaysQ.Amount IS NULL THEN 0 ELSE PaysQ.Amount END)) AS TotalPays, 
PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatientName, 
ChargesT.DoctorsProviders, LineItemT.LocationID 
FROM BillsT 
INNER JOIN PersonT ON BillsT.PatientID = PersonT.ID 
INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID 
INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID 
LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID 
LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID 
INNER JOIN (SELECT ChargeRespT.ID, ChargeID, SUM(ChargeRespT.Amount) AS TotalCharges, 
	CASE WHEN InsuredPartyID IS NULL THEN -1 ELSE InsuredPartyID END AS InsuredPartyID 
	FROM ChargeRespT 
	LEFT JOIN ChargesT ON ChargeRespT.ChargeID = ChargesT.ID 
	INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID 
	LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID 
	LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID 
	WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 10 
	AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null 
	AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null 
	GROUP BY ChargeRespT.ID, ChargeID, InsuredPartyID 
	) RespQ ON ChargesT.ID = RespQ.ChargeID 
LEFT JOIN (SELECT AppliesT.RespID, SUM(AppliesT.Amount) AS Amount 
	FROM PaymentsT 
	INNER JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID 
	INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID 
	LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalPaymentsQ ON LineItemT.ID = LineItemCorrections_OriginalPaymentsQ.OriginalLineItemID 
	LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingPaymentsQ ON LineItemT.ID = LineItemCorrections_VoidingPaymentsQ.VoidingLineItemID 
	WHERE LineItemT.Deleted = 0 
	AND LineItemCorrections_OriginalPaymentsQ.OriginalLineItemID Is Null 
	AND LineItemCorrections_VoidingPaymentsQ.VoidingLineItemID Is Null 
	GROUP BY AppliesT.RespID
	) PaysQ ON RespQ.ID = PaysQ.RespID 
WHERE LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null 
AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null 
GROUP BY BillsT.ID, BillsT.PatientID, RespQ.ID, RespQ.ChargeID, RespQ.TotalCharges, RespQ.InsuredPartyID, 
PersonT.Last, PersonT.First, PersonT.Middle, ChargesT.DoctorsProviders, LineItemT.LocationID 
) AS SubQ 
WHERE (TotalCharges - TotalPays) > CONVERT(money,'0') 
AND PatientID = {INT} AND InsuredPartyID = {INT}
)"
						, nPatientID, nInsuredPartyID);
			}

			while(!rs->eof) {

				//each result is a responsibility of a charge, so create a new adjustment to pay off that balance,
				//and apply the adjustment to that responsibility

				long nProviderID = AdoFldLong(rs, "DoctorsProviders", -1);
				CString strProviderID = "NULL";
				if(nProviderID != -1) {
					strProviderID = AsString(nProviderID);
				}

				long nChargeID = AdoFldLong(rs, "ChargeID");
				long nLocationID = AdoFldLong(rs, "LocationID");
				CString strPatientName = AdoFldString(rs, "PatientName", "");

				COleCurrency cyAmount = AdoFldCurrency(rs, "Balance", COleCurrency(0,0));

				// (j.armen 2013-06-28 12:47) - PLID 57373 - Idenitate LineItemT
				AddStatementToSqlBatch(strSqlBatch, 
					"INSERT INTO LineItemT (\r\n"
					"	PatientID, LocationID, Type, Date, InputName, InputDate, Amount, Description\r\n"
					") VALUES ("
					"	%li, %li, 2, GetDate(), '%s', GetDate(), Convert(money,'%s'), '%s')",
					nPatientID, nLocationID, _Q(GetCurrentUserName()), _Q(FormatCurrencyForSql(cyAmount)), _Q(strDescription));

				AddStatementToSqlBatch(strSqlBatch, "SET @nNewAdjID = SCOPE_IDENTITY() ");

				// (j.jones 2010-09-23 11:17) - PLID 40653 - we now save an ID for group & reason codes (nullable)
				AddStatementToSqlBatch(strSqlBatch, "INSERT INTO PaymentsT (ID, InsuredPartyID, ProviderID, PaymentGroupID, PayMethod, GroupCodeID, ReasonCodeID) "
					"VALUES (@nNewAdjID, %li, %s, %li, 0, %s, %s)", nInsuredPartyID, strProviderID, nCategoryID, strGroupCodeID, strReasonCodeID);
				
				//add info. to @NewAdjustmentsT to use later for applying, auditing, and reporting
				AddStatementToSqlBatch(strSqlBatch, "INSERT INTO @NewAdjustmentsT (ID, Amount, PatientName, PatientID, ChargeID) "
					"VALUES (@nNewAdjID, Convert(money, '%s'), '%s', %li, %li)", _Q(FormatCurrencyForSql(cyAmount)),
					_Q(strPatientName), nPatientID, nChargeID);

				nAdjustmentCount++;
				cyAdjustmentTotals += cyAmount;

				rs->MoveNext();
			}
			rs->Close();

			BOOL bFound = FALSE;
			for(int i=0; i<aryPatientIDs.GetSize() && !bFound; i++) {
				if((long)(aryPatientIDs.GetAt(i)) == nPatientID) {
					bFound = TRUE;
				}
			}

			if(!bFound) {
				aryPatientIDs.Add(nPatientID);
			}

			pRow = pRow->GetNextRow();
		}

		if(nAdjustmentCount == 0) {
			
			//shouldn't be possible
			
			ASSERT(FALSE);			
			
			if(nAuditTransactionID != -1) {
				RollbackAuditTransaction(nAuditTransactionID);
			}

			ThrowNxException("No adjustments created!");
		}

		long nPatientCount = aryPatientIDs.GetSize();
		CString strPatientIDs;
		for(int i=0; i<aryPatientIDs.GetSize(); i++) {
			if(!strPatientIDs.IsEmpty()) {
				strPatientIDs += ",";
			}
			strPatientIDs += AsString((long)(aryPatientIDs.GetAt(i)));
		}

		//audit the totals, using aeiWriteOffAccounts
		if(nAuditTransactionID == -1) {
			nAuditTransactionID = BeginAuditTransaction();
		}
		CString strAudit;
		strAudit.Format("Created %li adjustments for %li patients, totalling %s.", nAdjustmentCount, nPatientCount, FormatCurrencyForInterface(cyAdjustmentTotals, TRUE, TRUE, TRUE));
		AuditEvent(-1, "", nAuditTransactionID, aeiWriteOffAccounts, -1, "", strAudit, aepHigh, aetCreated);

		//now warn heavily before committing

		CString strWarn;
		strWarn.Format("Applying adjustments to the results in this list will create %li adjustment%s on %li patient account%s, totalling %s in value.\n\n"
			"Do you wish to continue applying %s now?", nAdjustmentCount, nAdjustmentCount > 1 ? "s" : "",
			nPatientCount, nPatientCount > 1 ? "s" : "", FormatCurrencyForInterface(cyAdjustmentTotals, TRUE, TRUE, TRUE),
			nAdjustmentCount > 1 ? "these adjustments" : "this adjustment");

		if(IDNO == MessageBox(strWarn, "Practice", MB_ICONEXCLAMATION|MB_YESNO)
			|| IDNO == MessageBox("Are you SURE you wish to apply these adjustments?", "Practice", MB_ICONEXCLAMATION|MB_YESNO)) {

			if(nAuditTransactionID != -1) {
				RollbackAuditTransaction(nAuditTransactionID);
			}

			return;
		}

		CWaitCursor pWait2;

		CString strAdjustmentIDs;

		if(!strSqlBatch.IsEmpty()) {
			
			//batch execute the changes, and retrieve the adjustment IDs, dollar amounts, and patient names, so we can audit
			_RecordsetPtr rs = CreateRecordset(
				"SET NOCOUNT ON \r\n"
				"BEGIN TRAN \r\n"
				"DECLARE @nNewAdjID INT \r\n"
				"DECLARE @nNewApplyID INT \r\n"
				"DECLARE @NewAdjustmentsT TABLE ( \r\n"
				"	ID int, \r\n"
				"	Amount money, \r\n"
				"	PatientName ntext, \r\n"
				"	PatientID int, \r\n"
				"	ChargeID int) \r\n"
				"%s "
				"COMMIT TRAN \r\n"
				"SET NOCOUNT OFF \r\n"
				"SELECT ID, Amount, PatientName, PatientID, ChargeID FROM @NewAdjustmentsT", strSqlBatch);

			while(!rs->eof) {

				long nAdjustmentID = AdoFldLong(rs, "ID");
				COleCurrency cyAdjustmentAmt = AdoFldCurrency(rs, "Amount", COleCurrency(0,0));
				CString strPatientName = AdoFldString(rs, "PatientName", "");
				long nPatientID = AdoFldLong(rs, "PatientID");
				long nChargeID = AdoFldLong(rs, "ChargeID");

				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}
				CString strAuditDesc;				
				strAuditDesc.Format("%s Adjustment", FormatCurrencyForInterface(cyAdjustmentAmt, TRUE, TRUE, TRUE));
				AuditEvent(nPatientID, strPatientName, nAuditTransactionID, aeiAdjustmentCreated, nAdjustmentID, "", strAuditDesc, aepHigh, aetCreated);

				//not done yet, NOW we can apply the adjustment
				// (j.jones 2011-08-17 11:10) - PLID 45067 - set bDisableAllowableCheck to TRUE, because we are adjusting to zero
				// and not shifting resp, so we do not need to check for allowed amounts
				AutoApplyPayToBill(nAdjustmentID, nPatientID, "Charge", nChargeID, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE);

				//track the ID in a comma-delimited list
				if(!strAdjustmentIDs.IsEmpty()) {
					strAdjustmentIDs += ",";
				}
				strAdjustmentIDs += AsString(nAdjustmentID);

				rs->MoveNext();
			}
			rs->Close();

			//track that adjustments were made
			m_bAdjustmentsMade = TRUE;
		}

		if(nAuditTransactionID != -1) {
			CommitAuditTransaction(nAuditTransactionID);
		}

		//clear the results list
		m_ResultsList->Clear();

		//update the label
		m_nxstaticPatientTotal.SetWindowText("0");

		//prompt to create a LW group if they have permission
		if(!strAdjustmentIDs.IsEmpty() && !strPatientIDs.IsEmpty()
			&& GetCurrentUserPermissions(bioLWGroup) & (SPT___W________ANDPASS)
			&& IDYES == MessageBox("Would you like to create a letter writing merge group containing these patients?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
			CreateMergeGroup(strPatientIDs);
		}

		//prompt to run a report of these adjustments
		if(!strAdjustmentIDs.IsEmpty()
			&& IDYES == MessageBox("The adjustments have been successfully created and applied.\n"
				"Would you like to preview a report of all the adjustments that were made?", "Practice", MB_ICONQUESTION|MB_YESNO)) {

			//Adjustments report
			CReportInfo rep(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(120)]);
			CString strWhere;
			strWhere.Format(" HAVING AdjustmentsSubQ.ID IN (%s)", strAdjustmentIDs);
			rep.AddExtraValue(strWhere);

			rep.nDateFilter = 1;

			//Set up the parameters.
			CPtrArray paParams;
			CRParameterInfo *paramInfo;
			
			paramInfo = new CRParameterInfo;
			paramInfo->m_Data = GetCurrentUserName();
			paramInfo->m_Name = "CurrentUserName";
			paParams.Add(paramInfo);

			COleDateTime dt = COleDateTime::GetCurrentTime();
			CString strDate = dt.Format("%m/%d/%Y");

			paramInfo = new CRParameterInfo;
			paramInfo->m_Data = strDate;
			paramInfo->m_Name = "DateFrom";
			paParams.Add((void *)paramInfo);

			paramInfo = new CRParameterInfo;
			paramInfo->m_Data = strDate;
			paramInfo->m_Name = "DateTo";
			paParams.Add((void *)paramInfo);

			//check to see if there is a default report
			_RecordsetPtr rsDefault = CreateRecordset("SELECT ID, CustomReportID FROM DefaultReportsT WHERE ID = 120");
			CString strFileName;

			if (rsDefault->eof) {
				strFileName = "AdjustmentsService";
			}
			else {
				
				long nDefaultCustomReport = AdoFldLong(rsDefault, "CustomReportID", -1);

				if (nDefaultCustomReport > 0) {

					_RecordsetPtr rsFileName = CreateParamRecordset("SELECT FileName FROM CustomReportsT WHERE ID = 120 AND Number = {INT}", nDefaultCustomReport);

					if (rsFileName->eof) {

						//this should never happen
						MessageBox("Practice could not find the custom report.  Please contact NexTech for assistance");
					}
					else {
						
						//set the default
						rep.nDefaultCustomReport = nDefaultCustomReport;
						strFileName =  AdoFldString(rsFileName, "FileName");
					}
				}
				else {
					//if this occurs it just means they want the default, which in this case, there is only one
					strFileName = "AdjustmentsService";					
				}
			}

			rep.strReportFile = strFileName;

			RunReport(&rep, &paParams, TRUE, (CWnd *)this, "Adjustments");
			ClearRPIParameterList(&paParams);

			//close the dialog if they ran the report
			CNxDialog::OnOK();
		}

	}NxCatchAllCall("Error in CAccountWriteOffDlg::OnBtnAdjust",
		if(nAuditTransactionID != -1) {
			RollbackAuditTransaction(nAuditTransactionID);
		}
	);
}

void CAccountWriteOffDlg::OnBtnWriteOffClose() 
{
	try {

		//return IDOK if any adjustments were made, IDCANCEL otherwise
		if(m_bAdjustmentsMade) {
			CDialog::OnOK();
		}
		else {
			CDialog::OnCancel();
		}

	}NxCatchAll("Error in CAccountWriteOffDlg::OnBtnWriteOffClose");
}

CString CAccountWriteOffDlg::GetFromClauseByAccount()
{
	//pass exceptions to the caller

	CString strFrom = "";
	CString strWhere = "";

	//generate a where clause such that we filter on account balance,
	//and dates should apply to the most recent charge on the account

	CString strChargeRespFilter;

	if(m_radioPatBalances.GetCheck()) {
		strChargeRespFilter.Format(" AND ChargeRespT.InsuredPartyID IS NULL ");
		// (j.jones 2011-08-17 14:21) - PLID 45075 - should not filter payments
		//strPaymentFilter.Format(" AND PaymentsT.InsuredPartyID = -1 ");
	}
	else if(m_radioInsBalances.GetCheck()) {
		strChargeRespFilter.Format(" AND ChargeRespT.InsuredPartyID IS NOT NULL ");
		// (j.jones 2011-08-17 14:21) - PLID 45075 - should not filter payments
		//strPaymentFilter.Format(" AND PaymentsT.InsuredPartyID <> -1 ");
	}

	if(m_checkUseDateFilters.GetCheck()) {

		if(m_radioGreaterThanDays.GetCheck()) {
			long nDays = 0;
			nDays = GetDlgItemInt(IDC_EDIT_DAYS_OLD);
			CString strDayWhere;
			if(m_radioLastSentDate.GetCheck()) {
				strDayWhere.Format("(Max(HistoryQ.LastDate) IS NOT NULL AND convert(datetime, convert(nvarchar, Max(HistoryQ.LastDate), 10)) <= (convert(datetime, convert(nvarchar, GetDate(), 10)) - %li)) ", nDays);
			}
			else if(m_radioBillServiceDate.GetCheck()) {
				strDayWhere.Format("(convert(datetime, convert(nvarchar, Max(TotalsPerChargeQ.BillDate), 10)) <= (convert(datetime, convert(nvarchar, GetDate(), 10)) - %li)) ", nDays);
			}
			else if(m_radioBillInputDate.GetCheck()) {
				strDayWhere.Format("(convert(datetime, convert(nvarchar, Max(TotalsPerChargeQ.BillInputDate), 10)) <= (convert(datetime, convert(nvarchar, GetDate(), 10)) - %li)) ", nDays);
			}
			else if(m_radioLastPaymentDate.GetCheck()) {
				strDayWhere.Format("Max(LastPaymentQ.LastPaymentDate) IS NOT NULL AND (convert(datetime, convert(nvarchar, Max(LastPaymentQ.LastPaymentDate), 10)) <= (convert(datetime, convert(nvarchar, GetDate(), 10)) - %li)) ", nDays);
			}
			// (j.jones 2013-08-19 16:59) - PLID 58088 - added an assignment date filter
			else if(m_radioAssignmentDate.GetCheck()) {
				CString strNewChargeRespFilter;
				strNewChargeRespFilter.Format(" AND (convert(datetime, convert(nvarchar, RespAssignmentDateQ.FirstDate, 10)) <= (convert(datetime, convert(nvarchar, GetDate(), 10)) - %li)) ", nDays);
				strChargeRespFilter += strNewChargeRespFilter;
				strDayWhere = "";
			}

			if(!strDayWhere.IsEmpty()) {
				strWhere += " AND ";
				strWhere += strDayWhere;
			}
		}
		else if(m_radioDateRange.GetCheck()) {
			COleDateTime dtFrom, dtTo;
			dtFrom = COleDateTime(m_dtFrom.GetValue());
			dtTo = COleDateTime(m_dtTo.GetValue());

			if(dtFrom == COleDateTime::invalid) {
				MsgBox("Your 'from' date is invalid. Please correct your date range.");
				return "";
			}

			if(dtTo == COleDateTime::invalid) {
				MsgBox("Your 'to' date is invalid. Please correct your date range.");
				return "";
			}

			if(dtFrom > dtTo) {
				MsgBox("Your 'from' date is after your 'to' date. Please correct your date range.");
				return "";
			}

			COleDateTimeSpan dtOneDay;
			dtOneDay.SetDateTimeSpan(1,0,0,0);
			dtTo += dtOneDay;

			CString strFrom, strTo;
			strFrom = FormatDateTimeForSql(dtFrom,dtoDate);
			strTo = FormatDateTimeForSql(dtTo,dtoDate);
			
			CString strDayWhere;
			if(m_radioLastSentDate.GetCheck()) {
				strDayWhere.Format("(Max(HistoryQ.LastDate) IS NOT NULL AND convert(datetime, convert(nvarchar, Max(HistoryQ.LastDate), 10)) >= convert(datetime, convert(nvarchar, '%s', 10)) AND convert(datetime, convert(nvarchar, Max(HistoryQ.LastDate), 10)) < convert(datetime, convert(nvarchar, '%s', 10))) ",_Q(strFrom),_Q(strTo));
			}
			else if(m_radioBillServiceDate.GetCheck()) {
				strDayWhere.Format("(convert(datetime, convert(nvarchar, Max(TotalsPerChargeQ.BillDate), 10)) >= convert(datetime, convert(nvarchar, '%s', 10)) AND convert(datetime, convert(nvarchar, Max(TotalsPerChargeQ.BillDate), 10)) < convert(datetime, convert(nvarchar, '%s', 10))) ",_Q(strFrom), _Q(strTo));
			}
			else if(m_radioBillInputDate.GetCheck()) {
				strDayWhere.Format("(convert(datetime, convert(nvarchar, Max(TotalsPerChargeQ.BillInputDate), 10)) >= convert(datetime, convert(nvarchar, '%s', 10)) AND convert(datetime, convert(nvarchar, Max(TotalsPerChargeQ.BillInputDate), 10)) < convert(datetime, convert(nvarchar, '%s', 10))) ",_Q(strFrom),_Q(strTo));
			}
			else if(m_radioLastPaymentDate.GetCheck()) {
				strDayWhere.Format("Max(LastPaymentQ.LastPaymentDate) IS NOT NULL AND (convert(datetime, convert(nvarchar, Max(LastPaymentQ.LastPaymentDate), 10)) >= convert(datetime, convert(nvarchar, '%s', 10)) AND convert(datetime, convert(nvarchar, Max(LastPaymentQ.LastPaymentDate), 10)) < convert(datetime, convert(nvarchar, '%s', 10))) ",_Q(strFrom),_Q(strTo));
			}
			// (j.jones 2013-08-19 16:59) - PLID 58088 - added an assignment date filter
			else if(m_radioAssignmentDate.GetCheck()) {
				CString strNewChargeRespFilter;
				strNewChargeRespFilter.Format(" AND (convert(datetime, convert(nvarchar, RespAssignmentDateQ.FirstDate, 10)) >= convert(datetime, convert(nvarchar, '%s', 10)) AND convert(datetime, convert(nvarchar, RespAssignmentDateQ.FirstDate, 10)) < convert(datetime, convert(nvarchar, '%s', 10))) ", _Q(strFrom),_Q(strTo));
				strChargeRespFilter += strNewChargeRespFilter;
				strDayWhere = "";
			}

			if(!strDayWhere.IsEmpty()) {
				strWhere += " AND ";
				strWhere += strDayWhere;
			}
		}
	}

	if(m_checkBalanceLessThan.GetCheck()) {

		CString strBalance;
		GetDlgItemText(IDC_EDIT_LESS_THAN_DOLLARS,strBalance);
		if(strBalance.GetLength() == 0 || !IsValidCurrencyText(strBalance)) {
			MsgBox("Please enter a valid currency in the 'balance less than' box.");
			return "";
		}
		strBalance = FormatCurrencyForSql(ParseCurrencyFromInterface(strBalance));

		CString strBalWhere;
		strBalWhere.Format("(Sum(TotalCharges - TotalPays) < Convert(money,'%s')) ",_Q(strBalance));

		strWhere += " AND ";
		strWhere += strBalWhere;
	}

	//this filter is by total account, not per bill, though it will be split by insured party ID

	// (j.jones 2011-08-17 11:00) - PLID 44888 - ignore "original" and "void" line items
	// (j.jones 2011-08-17 14:21) - PLID 45075 - this code previously filtered on patient or insurance payments,
	// but that is meaningless because the payments join by resp ID
	//(r.wilson 10/2/2012) plid 53082 - Replace hardcoded SendTypes with enumerated values
	// (j.jones 2013-08-19 16:59) - PLID 58088 - Added RespAssignmentDate for each charge,
	// calculated as the earliest date there is a non-zero ChargeRespDetailT record, or the LineItemT.Date
	// if it is a $0.00 charge.
	// (c.haag 2015-04-24) - PLID 65693 - This query is now contained in a R"( literal for easier debugging and maintenance, and we now pull a null charge ID instead of bill ID
	strFrom.Format(R"(
(SELECT NULL AS ChargeID, TotalsPerChargeQ.PatientID, Sum(TotalCharges - TotalPays) AS Bal, Sum(TotalCharges) AS TC, Sum(TotalPays) AS TP, 
	TotalsPerChargeQ.InsuredPartyID, InsuranceCoT.Name AS InsCoName, RespTypeT.TypeName AS RespTypeName, InsuranceCoT.HCFASetupGroupID, Max(HistoryQ.LastDate) AS LastDate, 
	PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, Max(TotalsPerChargeQ.BillDate) AS BillDate, Max(TotalsPerChargeQ.BillInputDate) AS InputDate, Max(LastPaymentQ.LastPaymentDate) AS LastPaymentDate, Min(TotalsPerChargeQ.RespAssignmentDate) AS RespAssignmentDate 
	FROM 
		(SELECT BillsT.ID AS BillID, BillsT.PatientID, RespQ.ChargeID, RespQ.TotalCharges, RespQ.InsuredPartyID, BillsT.Date AS BillDate, 
		Sum(CASE WHEN PaysQ.Amount IS NULL THEN 0 ELSE PaysQ.Amount END) AS TotalPays, BillsT.InputDate AS BillInputDate, 
		Min(IsNull(RespQ.FirstRespAssignmentDate, LineItemT.Date)) AS RespAssignmentDate 
		FROM BillsT 
		INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID 
		INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID 
		INNER JOIN 
			(SELECT ChargeRespT.ID, ChargeID, Sum(ChargeRespT.Amount) AS TotalCharges, 
			CASE WHEN InsuredPartyID IS NULL THEN -1 ELSE InsuredPartyID END AS InsuredPartyID, 
			Min(RespAssignmentDateQ.FirstDate) AS FirstRespAssignmentDate 
			FROM ChargeRespT 
			LEFT JOIN (SELECT Min(Date) AS FirstDate, ChargeRespID 
				FROM ChargeRespDetailT 
				WHERE Amount > 0 
				GROUP BY ChargeRespID) AS RespAssignmentDateQ ON ChargeRespT.ID = RespAssignmentDateQ.ChargeRespID 
			LEFT JOIN ChargesT ON ChargeRespT.ChargeID = ChargesT.ID INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID 
			LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID 
			LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID 
			WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 10 
			AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null 
			AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null 
			%s 
			GROUP BY ChargeRespT.ID, ChargeID, InsuredPartyID 
			) RespQ 
		ON ChargesT.ID = RespQ.ChargeID 
		LEFT JOIN 
			(SELECT AppliesT.RespID, Sum(AppliesT.Amount) AS Amount 
			FROM PaymentsT 
				INNER JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID 
				LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalPaymentsQ ON LineItemT.ID = LineItemCorrections_OriginalPaymentsQ.OriginalLineItemID 
				LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingPaymentsQ ON LineItemT.ID = LineItemCorrections_VoidingPaymentsQ.VoidingLineItemID 
				WHERE LineItemT.Deleted = 0 
				AND LineItemCorrections_OriginalPaymentsQ.OriginalLineItemID Is Null 
				AND LineItemCorrections_VoidingPaymentsQ.VoidingLineItemID Is Null 
				GROUP BY AppliesT.RespID 
			) PaysQ 
		ON RespQ.ID = PaysQ.RespID 
		WHERE LineItemT.Deleted = 0 
		GROUP BY BillsT.ID, BillsT.PatientID, RespQ.ChargeID, RespQ.TotalCharges, RespQ.InsuredPartyID, BillsT.Date, BillsT.InputDate 
		) TotalsPerChargeQ 
	LEFT JOIN InsuredPartyT ON TotalsPerChargeQ.InsuredPartyID = InsuredPartyT.PersonID 
	LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID 
	LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID 
	LEFT JOIN PersonT ON TotalsPerChargeQ.PatientID = PersonT.ID 
	LEFT JOIN 
		(SELECT Max(Date) AS LastDate, ClaimHistoryT.BillID, InsuredPartyID 
		FROM ClaimHistoryT 
	/*(r.wilson 10/2/2012) plid 53082 - Line Below use to be  WHERE SendType >= 0 */
		WHERE SendType >= %li 
		GROUP BY ClaimHistoryT.BillID, InsuredPartyID 
		) HistoryQ 
	ON TotalsPerChargeQ.BillID = HistoryQ.BillID AND TotalsPerChargeQ.InsuredPartyID = HistoryQ.InsuredPartyID 
	LEFT JOIN 
		(SELECT Max(LineItemT.Date) AS LastPaymentDate, PaymentsT.InsuredPartyID, ChargesT.BillID 
		FROM PaymentsT 
		INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID 
		INNER JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID 
		INNER JOIN ChargesT ON AppliesT.DestID = ChargesT.ID 
		INNER JOIN LineItemT LineItemT2 ON ChargesT.ID = LineItemT2.ID 
		LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON ChargesT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID 
		LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON ChargesT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID 
		LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalPaymentsQ ON PaymentsT.ID = LineItemCorrections_OriginalPaymentsQ.OriginalLineItemID 
		LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingPaymentsQ ON PaymentsT.ID = LineItemCorrections_VoidingPaymentsQ.VoidingLineItemID 
		WHERE LineItemT.Deleted = 0 AND LineItemT2.Deleted = 0 
		AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null 
		AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null 
		AND LineItemCorrections_OriginalPaymentsQ.OriginalLineItemID Is Null 
		AND LineItemCorrections_VoidingPaymentsQ.VoidingLineItemID Is Null 
		GROUP BY PaymentsT.InsuredPartyID, ChargesT.BillID 
		) LastPaymentQ 
	ON TotalsPerChargeQ.BillID = LastPaymentQ.BillID AND TotalsPerChargeQ.InsuredPartyID = LastPaymentQ.InsuredPartyID 
	GROUP BY TotalsPerChargeQ.PatientID, 
	TotalsPerChargeQ.InsuredPartyID, InsuranceCoT.Name, InsuranceCoT.PersonID, RespTypeT.TypeName, InsuranceCoT.HCFASetupGroupID, 
	PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle 
			
	HAVING (Sum(TotalCharges - TotalPays) > Convert(money,'0')) 
	/*Apply filters here */ 
		%s 
	/* Done filters */ 
			
	) Q 
			)"
			,strChargeRespFilter, ClaimSendType::Electronic,strWhere);

	return strFrom;
}

CString CAccountWriteOffDlg::GetFromClauseByBill()
{
	//pass exceptions to the caller

	CString strFrom = "";
	CString strWhere = "";

	//generate a from clause such that we filter per bill balance,
	//and dates should apply to each bill

	CString strChargeRespFilter;
	//CString strPaymentFilter;

	if(m_radioPatBalances.GetCheck()) {
		strChargeRespFilter.Format(" AND ChargeRespT.InsuredPartyID IS NULL ");
		// (j.jones 2011-08-17 14:21) - PLID 45075 - should not filter payments
		//strPaymentFilter.Format(" AND PaymentsT.InsuredPartyID = -1 ");
	}
	else if(m_radioInsBalances.GetCheck()) {
		strChargeRespFilter.Format(" AND ChargeRespT.InsuredPartyID IS NOT NULL ");
		// (j.jones 2011-08-17 14:21) - PLID 45075 - should not filter payments
		//strPaymentFilter.Format(" AND PaymentsT.InsuredPartyID <> -1 ");
	}

	if(m_checkUseDateFilters.GetCheck()) {

		if(m_radioGreaterThanDays.GetCheck()) {
			long nDays = 0;
			nDays = GetDlgItemInt(IDC_EDIT_DAYS_OLD);
			CString strDayWhere;
			if(m_radioLastSentDate.GetCheck()) {
				strDayWhere.Format("(HistoryQ.LastDate IS NOT NULL AND convert(datetime, convert(nvarchar, HistoryQ.LastDate, 10)) <= (convert(datetime, convert(nvarchar, GetDate(), 10)) - %li)) ", nDays);
			}
			else if(m_radioBillServiceDate.GetCheck()) {
				strDayWhere.Format("(convert(datetime, convert(nvarchar, TotalsPerChargeQ.BillDate, 10)) <= (convert(datetime, convert(nvarchar, GetDate(), 10)) - %li)) ", nDays);
			}
			else if(m_radioBillInputDate.GetCheck()) {
				strDayWhere.Format("(convert(datetime, convert(nvarchar, TotalsPerChargeQ.BillInputDate, 10)) <= (convert(datetime, convert(nvarchar, GetDate(), 10)) - %li)) ", nDays);
			}
			else if(m_radioLastPaymentDate.GetCheck()) {
				strDayWhere.Format("LastPaymentQ.LastPaymentDate IS NOT NULL AND (convert(datetime, convert(nvarchar, LastPaymentQ.LastPaymentDate, 10)) <= (convert(datetime, convert(nvarchar, GetDate(), 10)) - %li)) ", nDays);
			}
			// (j.jones 2013-08-19 16:59) - PLID 58088 - added an assignment date filter
			else if(m_radioAssignmentDate.GetCheck()) {				
				CString strNewChargeRespFilter;
				strNewChargeRespFilter.Format(" AND (convert(datetime, convert(nvarchar, RespAssignmentDateQ.FirstDate, 10)) <= (convert(datetime, convert(nvarchar, GetDate(), 10)) - %li)) ", nDays);
				strChargeRespFilter += strNewChargeRespFilter;
				strDayWhere = "";
			}

			if(!strDayWhere.IsEmpty()) {
				strWhere += " AND ";
				strWhere += strDayWhere;
			}
		}
		else if(m_radioDateRange.GetCheck()) {
			COleDateTime dtFrom, dtTo;
			dtFrom = COleDateTime(m_dtFrom.GetValue());
			dtTo = COleDateTime(m_dtTo.GetValue());

			if(dtFrom == COleDateTime::invalid) {
				MsgBox("Your 'from' date is invalid. Please correct your date range.");
				return "";
			}

			if(dtTo == COleDateTime::invalid) {
				MsgBox("Your 'to' date is invalid. Please correct your date range.");
				return "";
			}

			if(dtFrom > dtTo) {
				MsgBox("Your 'from' date is after your 'to' date. Please correct your date range.");
				return "";
			}

			COleDateTimeSpan dtOneDay;
			dtOneDay.SetDateTimeSpan(1,0,0,0);
			dtTo += dtOneDay;

			CString strFrom, strTo;
			strFrom = FormatDateTimeForSql(dtFrom,dtoDate);
			strTo = FormatDateTimeForSql(dtTo,dtoDate);
			
			CString strDayWhere;
			if(m_radioLastSentDate.GetCheck()) {
				strDayWhere.Format("(HistoryQ.LastDate IS NOT NULL AND convert(datetime, convert(nvarchar, HistoryQ.LastDate, 10)) >= convert(datetime, convert(nvarchar, '%s', 10)) AND convert(datetime, convert(nvarchar, HistoryQ.LastDate, 10)) < convert(datetime, convert(nvarchar, '%s', 10))) ",_Q(strFrom),_Q(strTo));
			}
			else if(m_radioBillServiceDate.GetCheck()) {
				strDayWhere.Format("(convert(datetime, convert(nvarchar, TotalsPerChargeQ.BillDate, 10)) >= convert(datetime, convert(nvarchar, '%s', 10)) AND convert(datetime, convert(nvarchar, TotalsPerChargeQ.BillDate, 10)) < convert(datetime, convert(nvarchar, '%s', 10))) ",_Q(strFrom), _Q(strTo));
			}
			else if(m_radioBillInputDate.GetCheck()) {
				strDayWhere.Format("(convert(datetime, convert(nvarchar, TotalsPerChargeQ.BillInputDate, 10)) >= convert(datetime, convert(nvarchar, '%s', 10)) AND convert(datetime, convert(nvarchar, TotalsPerChargeQ.BillInputDate, 10)) < convert(datetime, convert(nvarchar, '%s', 10))) ",_Q(strFrom),_Q(strTo));
			}
			else if(m_radioLastPaymentDate.GetCheck()) {
				strDayWhere.Format("LastPaymentQ.LastPaymentDate IS NOT NULL AND (convert(datetime, convert(nvarchar, LastPaymentQ.LastPaymentDate, 10)) >= convert(datetime, convert(nvarchar, '%s', 10)) AND convert(datetime, convert(nvarchar, LastPaymentQ.LastPaymentDate, 10)) < convert(datetime, convert(nvarchar, '%s', 10))) ",_Q(strFrom),_Q(strTo));
			}
			// (j.jones 2013-08-19 16:59) - PLID 58088 - added an assignment date filter
			else if(m_radioAssignmentDate.GetCheck()) {
				CString strNewChargeRespFilter;
				strNewChargeRespFilter.Format(" AND (convert(datetime, convert(nvarchar, RespAssignmentDateQ.FirstDate, 10)) >= convert(datetime, convert(nvarchar, '%s', 10)) AND convert(datetime, convert(nvarchar, RespAssignmentDateQ.FirstDate, 10)) < convert(datetime, convert(nvarchar, '%s', 10))) ", _Q(strFrom),_Q(strTo));
				strChargeRespFilter += strNewChargeRespFilter;
				strDayWhere = "";
			}

			if(!strDayWhere.IsEmpty()) {
				strWhere += " AND ";
				strWhere += strDayWhere;
			}
		}
	}

	if(m_checkBalanceLessThan.GetCheck()) {

		CString strBalance;
		GetDlgItemText(IDC_EDIT_LESS_THAN_DOLLARS,strBalance);
		if(strBalance.GetLength() == 0 || !IsValidCurrencyText(strBalance)) {
			MsgBox("Please enter a valid currency in the 'balance less than' box.");
			return "";
		}
		strBalance = FormatCurrencyForSql(ParseCurrencyFromInterface(strBalance));

		CString strBalWhere;
		strBalWhere.Format("(Sum(TotalCharges - TotalPays) < Convert(money,'%s')) ",_Q(strBalance));

		strWhere += " AND ";
		strWhere += strBalWhere;
	}

	//this is somewhat based off of the way billing follow-up filters, since these two dialogs
	//roughly filter on the same information

	// (j.jones 2011-08-17 11:00) - PLID 44888 - ignore "original" and "void" line items
	// (j.jones 2011-08-17 14:21) - PLID 45075 - this code previously filtered on patient or insurance payments,
	// but that is meaningless because the payments join by resp ID
	//(r.wilson 10/2/2012) plid 53082 - Replace hardcoded SendTypes with enumerated values
	// (j.jones 2013-08-19 16:59) - PLID 58088 - added RespAssignmentDate for each charge,
	// calculated as the earliest date there is a non-zero ChargeRespDetailT record, or the LineItemT.Date
	// if it is a $0.00 charge
	// (c.haag 2015-04-24) - PLID 65693 - This query is now contained in a R"( literal for easier debugging and maintenance, and we now pull charge ID's instead of bill ID's
	strFrom.Format(R"(
(SELECT TotalsPerChargeQ.ChargeID, TotalsPerChargeQ.PatientID, Sum(TotalCharges - TotalPays) AS Bal, Sum(TotalCharges) AS TC, Sum(TotalPays) AS TP, 
	TotalsPerChargeQ.InsuredPartyID, InsuranceCoT.Name AS InsCoName, RespTypeT.TypeName AS RespTypeName, InsuranceCoT.HCFASetupGroupID, HistoryQ.LastDate, 
	PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, TotalsPerChargeQ.BillDate, TotalsPerChargeQ.BillInputDate AS InputDate, LastPaymentQ.LastPaymentDate, TotalsPerChargeQ.RespAssignmentDate 
	FROM 
		(SELECT BillsT.ID AS BillID, BillsT.PatientID, RespQ.ChargeID, RespQ.TotalCharges, RespQ.InsuredPartyID, BillsT.Date AS BillDate, 
		Sum(CASE WHEN PaysQ.Amount IS NULL THEN 0 ELSE PaysQ.Amount END) AS TotalPays, BillsT.InputDate AS BillInputDate, 
		Min(IsNull(RespQ.FirstRespAssignmentDate, LineItemT.Date)) AS RespAssignmentDate 
		FROM BillsT 
		INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID 
		INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID 
		INNER JOIN 
			(SELECT ChargeRespT.ID, ChargeID, Sum(ChargeRespT.Amount) AS TotalCharges, CASE WHEN InsuredPartyID IS NULL THEN -1 ELSE InsuredPartyID END AS InsuredPartyID, 
			Min(RespAssignmentDateQ.FirstDate) AS FirstRespAssignmentDate 
			FROM ChargeRespT 
			LEFT JOIN (SELECT Min(Date) AS FirstDate, ChargeRespID 
				FROM ChargeRespDetailT 
				WHERE Amount > 0 
				GROUP BY ChargeRespID) AS RespAssignmentDateQ ON ChargeRespT.ID = RespAssignmentDateQ.ChargeRespID 
			LEFT JOIN ChargesT ON ChargeRespT.ChargeID = ChargesT.ID INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID 
			LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID 
			LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID 
			WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 10 
			AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null 
			AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null 
			%s 
			GROUP BY ChargeRespT.ID, ChargeID, InsuredPartyID 
			) RespQ 
		ON ChargesT.ID = RespQ.ChargeID 
		LEFT JOIN 
			(SELECT AppliesT.RespID, Sum(AppliesT.Amount) AS Amount 
			FROM PaymentsT 
				INNER JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID 
				LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalPaymentsQ ON LineItemT.ID = LineItemCorrections_OriginalPaymentsQ.OriginalLineItemID 
				LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingPaymentsQ ON LineItemT.ID = LineItemCorrections_VoidingPaymentsQ.VoidingLineItemID 
				WHERE LineItemT.Deleted = 0 
				AND LineItemCorrections_OriginalPaymentsQ.OriginalLineItemID Is Null 
				AND LineItemCorrections_VoidingPaymentsQ.VoidingLineItemID Is Null 
				GROUP BY AppliesT.RespID 
			) PaysQ 
		ON RespQ.ID = PaysQ.RespID 
		WHERE LineItemT.Deleted = 0 
		GROUP BY BillsT.ID, BillsT.PatientID, RespQ.ChargeID, RespQ.TotalCharges, RespQ.InsuredPartyID, BillsT.Date, BillsT.InputDate 
		) TotalsPerChargeQ 
	LEFT JOIN InsuredPartyT ON TotalsPerChargeQ.InsuredPartyID = InsuredPartyT.PersonID 
	LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID 
	LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID 
	LEFT JOIN PersonT ON TotalsPerChargeQ.PatientID = PersonT.ID 
	LEFT JOIN 
		(SELECT Max(Date) AS LastDate, ClaimHistoryT.BillID, InsuredPartyID 
		FROM ClaimHistoryT 
	/*(r.wilson 10/2/2012) plid 53082 - Line below use to be "   WHERE SendType >= 0"*/
		WHERE SendType >= %li 
		GROUP BY ClaimHistoryT.BillID, InsuredPartyID 
		) HistoryQ 
	ON TotalsPerChargeQ.BillID = HistoryQ.BillID AND TotalsPerChargeQ.InsuredPartyID = HistoryQ.InsuredPartyID 
	LEFT JOIN 
		(SELECT Max(LineItemT.Date) AS LastPaymentDate, PaymentsT.InsuredPartyID, ChargesT.BillID 
		FROM PaymentsT 
		INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID 
		INNER JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID 
		INNER JOIN ChargesT ON AppliesT.DestID = ChargesT.ID 
		INNER JOIN LineItemT LineItemT2 ON ChargesT.ID = LineItemT2.ID 
		LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON ChargesT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID 
		LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON ChargesT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID 
		LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalPaymentsQ ON PaymentsT.ID = LineItemCorrections_OriginalPaymentsQ.OriginalLineItemID 
		LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingPaymentsQ ON PaymentsT.ID = LineItemCorrections_VoidingPaymentsQ.VoidingLineItemID 
		WHERE LineItemT.Deleted = 0 AND LineItemT2.Deleted = 0 
		AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null 
		AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null 
		AND LineItemCorrections_OriginalPaymentsQ.OriginalLineItemID Is Null 
		AND LineItemCorrections_VoidingPaymentsQ.VoidingLineItemID Is Null 
		GROUP BY PaymentsT.InsuredPartyID, ChargesT.BillID 
		) LastPaymentQ 
	ON TotalsPerChargeQ.BillID = LastPaymentQ.BillID AND TotalsPerChargeQ.InsuredPartyID = LastPaymentQ.InsuredPartyID 
	GROUP BY TotalsPerChargeQ.ChargeID, TotalsPerChargeQ.PatientID, 
	TotalsPerChargeQ.InsuredPartyID, InsuranceCoT.Name, InsuranceCoT.PersonID, RespTypeT.TypeName, InsuranceCoT.HCFASetupGroupID, HistoryQ.LastDate, 
	PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, TotalsPerChargeQ.BillDate, 
	TotalsPerChargeQ.BillInputDate, LastPaymentQ.LastPaymentDate, TotalsPerChargeQ.RespAssignmentDate 
			
	HAVING (Sum(TotalCharges - TotalPays) > Convert(money,'0')) 
	/*Apply filters here */ 
		%s 
	/* Done filters */ 
			
	) Q 
)",
			strChargeRespFilter, ClaimSendType::Electronic,strWhere);

	return strFrom;
}

void CAccountWriteOffDlg::OnBtnDisplayResults() 
{
	try {

		//clear our current results
		m_ResultsList->Clear();
		m_nxstaticPatientTotal.SetWindowText("0");

		//now load the list based on our filters

		//set our from clause
		CString strFrom = "";
		if(m_radioSearchByAccount.GetCheck()) {
			strFrom = GetFromClauseByAccount();
		}
		else if(m_radioSearchByBill.GetCheck()) {
			strFrom = GetFromClauseByBill();
		}

		if(strFrom.IsEmpty()) {
			//should only have occurred if a message popped up
			return;
		}

		//show/hide insurance columns
		if(m_radioPatBalances.GetCheck()) {
			
			m_ResultsList->GetColumn(rlcInsCoName)->PutStoredWidth(0);
			m_ResultsList->GetColumn(rlcInsCoName)->ColumnStyle = csFixedWidth|csVisible;
			m_ResultsList->GetColumn(rlcRespName)->PutStoredWidth(0);
			m_ResultsList->GetColumn(rlcRespName)->ColumnStyle = csFixedWidth|csVisible;
			m_ResultsList->GetColumn(rlcLastSentDate)->PutStoredWidth(0);
			m_ResultsList->GetColumn(rlcLastSentDate)->ColumnStyle = csFixedWidth|csVisible;
		}
		else {			
			m_ResultsList->GetColumn(rlcInsCoName)->ColumnStyle = csWidthAuto|csVisible;
			m_ResultsList->GetColumn(rlcRespName)->PutStoredWidth(75);
			m_ResultsList->GetColumn(rlcRespName)->ColumnStyle = csWidthData|csVisible;
			m_ResultsList->GetColumn(rlcLastSentDate)->PutStoredWidth(60);
			m_ResultsList->GetColumn(rlcLastSentDate)->ColumnStyle = csWidthData|csVisible;
		}

		m_ResultsList->FromClause = _bstr_t(strFrom);

		m_nxstaticPatientTotal.SetWindowText("Loading...");

		//disable the adjust button while it is requerying
		m_btnAdjust.EnableWindow(FALSE);

		//Change the timeout so we don't get an error.
		m_pIncreaseCommandTimeout.reset(new CIncreaseCommandTimeout(600));

		m_ResultsList->Requery();

		//OnRequeryFinished will re-enable the adjust button and reset the timeout

	}NxCatchAll("Error in CAccountWriteOffDlg::OnBtnDisplayResults");
}

void CAccountWriteOffDlg::OnRadioWriteOffByAccount() 
{
	try {

		OnChangeSearchOptions();
		
	}NxCatchAll("Error in CAccountWriteOffDlg::OnRadioWriteOffByAccount");
}

void CAccountWriteOffDlg::OnRadioWriteOffByBill() 
{
	try {

		OnChangeSearchOptions();
		
	}NxCatchAll("Error in CAccountWriteOffDlg::OnRadioWriteOffByBill");
}

void CAccountWriteOffDlg::OnChangeSearchOptions()
{
	try {

		//change our labels according to our search types

		CString strSearchTextType = "patient accounts";
		CString strAdjustmentTextSearchType = "accounts";
		
		CString strSearchTextRespType = " ";
		CString strAdjustmentTextRespType = " ";
		
		if(m_radioSearchByAccount.GetCheck()) {			
			//searching by patient			
			strSearchTextType = "patient accounts";
			strAdjustmentTextSearchType = "accounts";

			m_checkUseDateFilters.SetWindowText("Most recent bill date:");
			m_checkBalanceLessThan.SetWindowText("Unpaid account balance less than:");
			m_nxstaticResultsInfoLabel.SetWindowText("Each entry in the results list represents the total balances of all bills on a patient account, split by responsibility. "
				"Bill and payment dates reflect the most recent dates on the account.");			
		}
		else {
			//searching by bill
			strSearchTextType = "bills";
			strAdjustmentTextSearchType = "bills";

			m_checkUseDateFilters.SetWindowText("In date range:");
			m_checkBalanceLessThan.SetWindowText("Unpaid bill balance less than:");
			m_nxstaticResultsInfoLabel.SetWindowText("Each entry in the results list represents the totals for a given bill, split by responsibility.");			
		}

		if(m_radioAllBalances.GetCheck()) {
			//searching all balances

			strSearchTextRespType = "any";
			strAdjustmentTextRespType = " ";
		}
		else if(m_radioPatBalances.GetCheck()) {
			//searching patient balances
			strSearchTextRespType = "patient";
			strAdjustmentTextRespType = " patient ";
		}
		else {
			//searching insurance balances
			strSearchTextRespType = "insurance";
			strAdjustmentTextRespType = " insurance ";
		}

		CString strSearchText, strAdjustmentText;

		strSearchText.Format("Display all %s that have %s balance, and match the following criteria:", strSearchTextType, strSearchTextRespType);
		m_nxstaticSearchLabel.SetWindowText(strSearchText);

		strAdjustmentText.Format("When adjusting %s to zero, an adjustment will be created for each%sresp. "
								"with a balance, using the provider and location for the associated charge.", strAdjustmentTextSearchType, strAdjustmentTextRespType);
		m_nxstaticAdjInfoLabel.SetWindowText(strAdjustmentText);
		
	}NxCatchAll("Error in CAccountWriteOffDlg::OnChangeSearchOptions");
}

void CAccountWriteOffDlg::OnEditAdjDesc() 
{
	try {
		// (j.armen 2012-06-06 15:51) - PLID 49856 - Refactored CEditComboBox
		CEditComboBox(this, 2, m_DescriptionCombo, "Edit Combo Box").DoModal();

		//requery the description combo, don't re-select a value
		m_DescriptionCombo->Requery();
		
	}NxCatchAll("Error in CAccountWriteOffDlg::OnEditAdjDesc");
}

void CAccountWriteOffDlg::OnCheckUseDateRange() 
{
	try {

		if(m_checkUseDateFilters.GetCheck()) {
			m_radioGreaterThanDays.EnableWindow(TRUE);
			m_radioDateRange.EnableWindow(TRUE);
			m_radioLastSentDate.EnableWindow(TRUE);
			m_radioBillServiceDate.EnableWindow(TRUE);
			m_radioBillInputDate.EnableWindow(TRUE);
			m_radioLastPaymentDate.EnableWindow(TRUE);
			// (j.jones 2013-08-19 16:59) - PLID 58088 - added an assignment date filter
			m_radioAssignmentDate.EnableWindow(TRUE);
			OnChangeDateFilterRadio();
		}
		else {
			m_radioGreaterThanDays.EnableWindow(FALSE);
			m_radioDateRange.EnableWindow(FALSE);
			m_radioLastSentDate.EnableWindow(FALSE);
			m_radioBillServiceDate.EnableWindow(FALSE);
			m_radioBillInputDate.EnableWindow(FALSE);
			m_radioLastPaymentDate.EnableWindow(FALSE);
			// (j.jones 2013-08-19 16:59) - PLID 58088 - added an assignment date filter
			m_radioAssignmentDate.EnableWindow(FALSE);
			m_editDaysOld.EnableWindow(FALSE);
			m_dtFrom.EnableWindow(FALSE);
			m_dtTo.EnableWindow(FALSE);
		}
		
	}NxCatchAll("Error in CAccountWriteOffDlg::OnCheckUseDateRange");
}

void CAccountWriteOffDlg::OnCheckUseBalanceLessThan() 
{
	try {

		m_editLessThanAmount.EnableWindow(m_checkBalanceLessThan.GetCheck());
		
	}NxCatchAll("Error in CAccountWriteOffDlg::OnCheckUseBalanceLessThan");
}

void CAccountWriteOffDlg::OnRadioWriteOffGreaterThanDays() 
{
	try {

		OnChangeDateFilterRadio();
		
	}NxCatchAll("Error in CAccountWriteOffDlg::OnRadioWriteOffGreaterThanDays");
}

void CAccountWriteOffDlg::OnRadioWriteOffBetweenDateRange() 
{
	try {

		OnChangeDateFilterRadio();
		
	}NxCatchAll("Error in CAccountWriteOffDlg::OnRadioWriteOffBetweenDateRange");
}

void CAccountWriteOffDlg::OnChangeDateFilterRadio() 
{
	try {

		if(m_radioGreaterThanDays.GetCheck()) {
			m_editDaysOld.EnableWindow(TRUE);
			m_dtFrom.EnableWindow(FALSE);
			m_dtTo.EnableWindow(FALSE);
		}
		else {
			m_editDaysOld.EnableWindow(FALSE);
			m_dtFrom.EnableWindow(TRUE);
			m_dtTo.EnableWindow(TRUE);
		}
		
	}NxCatchAll("Error in CAccountWriteOffDlg::OnChangeDateFilterRadio");
}

BEGIN_EVENTSINK_MAP(CAccountWriteOffDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CAccountWriteOffDlg)
	ON_EVENT(CAccountWriteOffDlg, IDC_WRITE_OFF_LIST, 18 /* RequeryFinished */, OnRequeryFinishedWriteOffList, VTS_I2)
	ON_EVENT(CAccountWriteOffDlg, IDC_WRITE_OFF_LIST, 6 /* RButtonDown */, OnRButtonDownWriteOffList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CAccountWriteOffDlg, IDC_ADJ_DESCRIPTION, 16 /* SelChosen */, OnSelChosenAdjDescription, VTS_DISPATCH)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CAccountWriteOffDlg::OnRequeryFinishedWriteOffList(short nFlags) 
{
	try {

		//re-enable the adjust button
		m_btnAdjust.EnableWindow(TRUE);

		m_pIncreaseCommandTimeout.reset();

		UpdatePatientTotalLabel();
		
	}NxCatchAll("Error in CAccountWriteOffDlg::OnRequeryFinishedWriteOffList");
}

void CAccountWriteOffDlg::UpdatePatientTotalLabel()
{
	try {

		//calculate the total of patient IDs
		CArray<long, long> aryPatientIDs;
		IRowSettingsPtr pRow = m_ResultsList->GetFirstRow();		
		while(pRow) {

			long nPatientID = VarLong(pRow->GetValue(rlcPatientID));

			BOOL bFound = FALSE;
			for(int i=0; i<aryPatientIDs.GetSize() && !bFound; i++) {
				if((long)(aryPatientIDs.GetAt(i)) == nPatientID) {
					bFound = TRUE;
				}
			}

			if(!bFound) {
				aryPatientIDs.Add(nPatientID);
			}

			pRow = pRow->GetNextRow();
		}

		long nPatientTotalCount = aryPatientIDs.GetSize();

		m_nxstaticPatientTotal.SetWindowText(AsString(nPatientTotalCount));

	}NxCatchAll("Error in CAccountWriteOffDlg::UpdatePatientTotalLabel");
}

void CAccountWriteOffDlg::OnRadioWriteOffAllBalances() 
{
	try {

		OnChangeSearchOptions();

	}NxCatchAll("Error in CAccountWriteOffDlg::OnRadioWriteOffAllBalances");
}

void CAccountWriteOffDlg::OnRadioWriteOffPatientBalances() 
{
	try {

		OnChangeSearchOptions();

	}NxCatchAll("Error in CAccountWriteOffDlg::OnRadioWriteOffAllBalances");
}

void CAccountWriteOffDlg::OnRadioWriteOffInsuranceBalances() 
{
	try {

		OnChangeSearchOptions();

	}NxCatchAll("Error in CAccountWriteOffDlg::OnRadioWriteOffAllBalances");
}

void CAccountWriteOffDlg::OnRButtonDownWriteOffList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		m_ResultsList->CurSel = pRow;

		//add an ability to remove the row
		enum {
			eRemoveFromList = 1,
		};

		// Create the menu
		CMenu mnu;
		mnu.CreatePopupMenu();
		mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, eRemoveFromList, "&Remove From List");

		CPoint pt;
		GetCursorPos(&pt);

		int nRet = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this);

		if(nRet == eRemoveFromList) {

			//simply remove the row from the list
			m_ResultsList->RemoveRow(pRow);

			//update the total
			UpdatePatientTotalLabel();
		}

	}NxCatchAll("Error in CAccountWriteOffDlg::OnRButtonDownWriteOffList");
}

void CAccountWriteOffDlg::CreateMergeGroup(CString strPatientIDs)
{
	try {
		
		if(strPatientIDs.IsEmpty()) {
			ASSERT(FALSE);
			return;
		}

		if(!CheckCurrentUserPermissions(bioLWGroup, sptWrite)) {
			return;
		}

		CString strNewGroupName;

		BOOL bContinue = TRUE;
		while(bContinue) {
			
			bContinue = FALSE;
		
			CGetNewIDName dlgGetNew(this);
			dlgGetNew.m_pNewName = &strNewGroupName;
			dlgGetNew.m_strCaption = "Enter a Name for the New Group";
			dlgGetNew.m_nMaxLength = 50;

			if (dlgGetNew.DoModal() == IDOK) {
				//does this group already exist?
				_RecordsetPtr rs;
				rs = CreateParamRecordset("SELECT Name FROM GroupsT WHERE Name = {STRING}", strNewGroupName);
				if(!rs->eof) {
					AfxMessageBox("Group name already exists.  Please try again with a different name.");
					bContinue = TRUE;
				}
				rs->Close();
			}
			else {
				return;
			}
		}

		if(strNewGroupName.IsEmpty()) {
			return;
		}

		CWaitCursor pWait;

		CString strSqlBatch;
		AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @nGroupID INT");
		AddStatementToSqlBatch(strSqlBatch, "SET @nGroupID = (SELECT Coalesce(Max(ID), 0) + 1 AS NewID FROM GroupsT)");
		AddStatementToSqlBatch(strSqlBatch, "INSERT INTO GroupsT(ID, Name) VALUES (@nGroupID, '%s')", _Q(strNewGroupName));
		AddStatementToSqlBatch(strSqlBatch, "INSERT INTO GroupDetailsT(GroupID, PersonID) "
			"SELECT @nGroupID AS GroupID, ID "
			"FROM PersonT WHERE ID IN (%s) GROUP BY ID", strPatientIDs);
		ExecuteSqlBatch(strSqlBatch);

		//update the group checker
		CClient::RefreshTable(NetUtils::Groups);

		// Tell the user that the group has been successfully created
		MsgBox("Group has been created.");

	}NxCatchAll("Error in CAccountWriteOffDlg::CreateMergeGroup");
}

void CAccountWriteOffDlg::OnSelChosenAdjDescription(LPDISPATCH lpRow) 
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		CString strDesc = VarString(pRow->GetValue(0), "");
		GetDlgItem(IDC_ADJ_DESC)->SetWindowText(strDesc);

		m_DescriptionCombo->CurSel = NULL;

	}NxCatchAll("Error in CAccountWriteOffDlg::OnSelChosenAdjDescription");
}

void CAccountWriteOffDlg::OnBtnEditPayCat() 
{
	try {

		long nCurCatID = 0;

		{
			IRowSettingsPtr pRow = m_CategoryCombo->GetCurSel();		
			if(pRow) {
				nCurCatID = VarLong(pRow->GetValue(cccID), 0);
			}
		}

		// (j.armen 2012-06-06 15:51) - PLID 49856 - Refactored CEditComboBox
		// (j.gruber 2012-11-15 13:53) - PLID 53752 - change the dialog
		CPayCatDlg dlg(this);
		if (dlg.DoModal()) {
			m_CategoryCombo->Requery();			
			m_CategoryCombo->WaitForRequery(dlPatienceLevelWaitIndefinitely);
		}
		//CEditComboBox(this, 3, m_CategoryCombo, "Edit Combo Box").DoModal();

		IRowSettingsPtr pRow = m_CategoryCombo->GetNewRow();
		pRow->PutValue(cccID, (long)0);
		pRow->PutValue(cccName,_bstr_t(" <No Category Selected>"));
		m_CategoryCombo->AddRowSorted(pRow, NULL);

		m_CategoryCombo->SetSelByColumn(cccID, (long)nCurCatID);
		
	}NxCatchAll("Error in CAccountWriteOffDlg::OnBtnEditPayCat");
}

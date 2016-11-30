// ReviewCCTransactionsDlg.cpp : implementation file
//

// (d.lange 2010-09-02 10:09) - PLID 40311 - Created to replace QBMS_ReviewTransactionsDlg
#include "stdafx.h"
#include "Practice.h"
#include "ReviewCCTransactionsDlg.h"
#include "CreditCardProcessingDlg.h"
#include "PaymentDlg.h"
#include "GlobalFinancialUtils.h"

using namespace NXDATALIST2Lib;
using namespace ADODB;

// (d.thompson 2010-11-08) - PLID 41379 - Added acct id, description
enum eListColumns {
	elcID = 0,
	elcType,
	elcPatientID,
	elcUserDefinedID,	//b.eyers 2013-11-1 - PLID 43819
	elcPatientName,
	elcTotalAmount,
	elcInputDate,
	elcIsApproved,
	elcApprovalCode,
	elcSentDate,
	elcSentByUser,
	elcAccountID,
	elcAccountDesc,
	elcIsQBMS,			// (d.thompson 2010-11-11) - PLID 40368
	elcIsChase,			// (d.thompson 2010-11-11) - PLID 40368
};

// (d.thompson 2010-11-08) - PLID 41379
enum eAccountFilterColumns {
	eafcID = 0,
	eafcDescription,
};

// CReviewCCTransactionsDlg dialog

IMPLEMENT_DYNAMIC(CReviewCCTransactionsDlg, CNxDialog)

CReviewCCTransactionsDlg::CReviewCCTransactionsDlg(CWnd* pParent)
	: CNxDialog(CReviewCCTransactionsDlg::IDD, pParent)
{

}

CReviewCCTransactionsDlg::~CReviewCCTransactionsDlg()
{
}

void CReviewCCTransactionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CC_PENDING, m_btnPending);
	DDX_Control(pDX, IDC_CC_APPROVED, m_btnApproved);
	DDX_Control(pDX, IDC_CC_ALL, m_btnAll);
	DDX_Control(pDX, IDC_CC_REPROCESS, m_btnReprocess);
}


BEGIN_MESSAGE_MAP(CReviewCCTransactionsDlg, CNxDialog)
	ON_BN_CLICKED(IDC_CC_PENDING, &OnBnClickedQbmsPending)
	ON_BN_CLICKED(IDC_CC_APPROVED, &OnBnClickedQbmsApproved)
	ON_BN_CLICKED(IDC_CC_ALL, &OnBnClickedQbmsAll)
	ON_BN_CLICKED(IDC_CC_REPROCESS, &OnBnClickedQbmsReprocess)
END_MESSAGE_MAP()


// CReviewCCTransactionsDlg message handlers
BOOL CReviewCCTransactionsDlg::OnInitDialog()
{
	try {
		CNxDialog::OnInitDialog();

		//setup interface
		m_btnReprocess.AutoSet(NXB_MODIFY);
		m_pList = BindNxDataList2Ctrl(IDC_CC_LIST, false);

		// (d.thompson 2010-11-11) - PLID 40368 - We now support both QBMS and Chase.  But since this is basically
		//	a reporting window, we cannot just ignore the one that is not licensed, we need to show both.  The
		//	FROM clause should pull from both tables.  I moved it here so it's easier to read/modify instead of
		//	in the datalist resources.
		// b.eyers 2013-11-1 - PLID 43819 - Added inner join to patientst, to display patient ID in datalist 
		m_pList->FromClause = _bstr_t("PaymentsT  "
			"INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID  "
			"INNER JOIN PersonT ON LineItemT.PatientID = PersonT.ID  "
			"LEFT JOIN PaymentPlansT ON LineItemT.ID = PaymentPlansT.ID  "
			"INNER JOIN PatientsT ON LineItemT.PatientID = PatientsT.PersonID "
			" "
			"LEFT JOIN  "
			"( "
			//QBMS-specific information
			"	SELECT QBMS_CreditTransactionsT.ID AS PaymentID, TotalAmount, IsApproved, LocalApprovalDateTime, SentByUserID,  "
			"	AccountID, QBMS_SetupData.Description AS AccountName, 1 AS IsQBMS, 0 AS IsChase "
			"	FROM QBMS_CreditTransactionsT  "
			"	LEFT JOIN QBMS_SetupData ON QBMS_CreditTransactionsT.AccountID = QBMS_SetupData.ID "
			" "
			"	UNION "
			" "
			//Chase-specific information
			"	SELECT Chase_CreditTransactionsT.ID AS PaymentID, TotalAmount, IsApproved, LocalApprovalDateTime, SentByUserID, "
			"	AccountID, Chase_SetupDataT.Description AS AccountName, 0 AS IsQBMS, 1 AS IsChase "
			"	FROM Chase_CreditTransactionsT "
			"	LEFT JOIN Chase_SetupDataT ON Chase_CreditTransactionsT.AccountID = Chase_SetupDataT.ID "
			") ProcessorSubQ "
			"ON PaymentsT.ID = ProcessorSubQ.PaymentID");

		// (d.thompson 2010-11-08) - PLID 41379
		m_pAccountFilter = BindNxDataList2Ctrl(IDC_CC_ACCT_FILTER_LIST, false);
		// (d.thompson 2010-11-11) - PLID 40368 - We now support QBMS and Chase, support both here.
		LoadAccountList();

		//Defaults
		CheckDlgButton(IDC_CC_PENDING, TRUE);
		// (d.thompson 2010-11-08) - PLID 41379 - All accounts by default
		m_pAccountFilter->PutCurSel( m_pAccountFilter->GetFirstRow() );

		//load the interface
		Reload();

	} NxCatchAll("Error in OnInitDialog");

	return TRUE;
}

void CReviewCCTransactionsDlg::LoadAccountList()
{
	//The list is simply ID and Description, which all account types support.  So just update the FROM clause appropriately
	//	and the rest of the dialog will work with no further effort.
	CString strFrom;
	if(g_pLicense && g_pLicense->HasCreditCardProc_QBMS(CLicense::cflrSilent)) {
		//QBMS processing enabled
		strFrom = "QBMS_SetupData";
	}
	else if(g_pLicense && g_pLicense->HasCreditCardProc_Chase(CLicense::cflrSilent)) {
		//Chase processing enabled
		strFrom = "Chase_SetupDataT";
	}
	else {
		//Bad licensing, this tab shouldn't exist.  Alerts happen in other places, so just quit here.  You'll get nothing in
		//	your account list.
		return;
	}

	//Apply the from clause and requery
	m_pAccountFilter->FromClause = _bstr_t(strFrom);
	//I put this here instead of the resources for consistency, so it's easy to find.
	m_pAccountFilter->WhereClause = _bstr_t("Inactive = 0");
	m_pAccountFilter->Requery();

	//Lastly, add an 'all accounts' row at the top
	{
		m_pAccountFilter->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pAccountFilter->GetNewRow();
		pRow->PutValue(eafcID, g_cvarNull);
		pRow->PutValue(eafcDescription, _bstr_t("{All Accounts}"));
		m_pAccountFilter->AddRowBefore( pRow, m_pAccountFilter->GetFirstRow() );
	}
}

// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
void CReviewCCTransactionsDlg::UpdateView(bool bForceRefresh)
{
	try {
		Reload();
	} NxCatchAll(__FUNCTION__);
}

void CReviewCCTransactionsDlg::Reload()
{
	// (d.thompson 2010-11-11) - PLID 40368 - Due to FROM clause changes, we also need to filter out non-credit card-processing line items
	CString strWhere = "LineItemT.Deleted = 0 AND ProcessorSubQ.PaymentID IS NOT NULL ";

	if(IsDlgButtonChecked(IDC_CC_PENDING)) {
		strWhere += "AND ProcessorSubQ.IsApproved = 0 ";
	}
	else if(IsDlgButtonChecked(IDC_CC_APPROVED)) {
		strWhere += "AND ProcessorSubQ.IsApproved = 1 ";
	}
	else {
		//No filter for all
	}

	// (d.thompson 2010-11-08) - PLID 41379 - Handle the account filter
	// (d.thompson 2010-11-11) - PLID 40368 - We now support QBMS and Chase, so we need to filter on the proper
	//	processor as well.
	NXDATALIST2Lib::IRowSettingsPtr pSelRow = m_pAccountFilter->GetCurSel();
	if(pSelRow) {
		_variant_t varID = pSelRow->GetValue(eafcID);
		if(varID.vt == VT_NULL) {
			//All Accounts selection.  No filter required
		}
		else {
			//Filter on this particular ID
			long nIsQBMS = 0, nIsChase = 0;
			if(g_pLicense && g_pLicense->HasCreditCardProc_QBMS(CLicense::cflrSilent)) {
				//QBMS processing enabled
				nIsQBMS = 1;
			}
			else if(g_pLicense && g_pLicense->HasCreditCardProc_Chase(CLicense::cflrSilent)) {
				//Chase processing enabled
				nIsChase = 1;
			}
			else {
				//Bad licensing, this tab shouldn't exist.  Alerts happen in other places, so just let this filter generate 0 records.
			}

			strWhere += FormatString("AND ProcessorSubQ.AccountID = %li AND ProcessorSubQ.IsQBMS = %li AND ProcessorSubQ.IsChase = %li", 
				VarLong(varID), nIsQBMS, nIsChase);
		}
	}
	else {
		//no selection, treat it as "all"
	}

	m_pList->WhereClause = _bstr_t(strWhere);
	m_pList->Requery();
}

void CReviewCCTransactionsDlg::OnBnClickedQbmsPending()
{
	try {
		Reload();

	} NxCatchAll(__FUNCTION__);
}

void CReviewCCTransactionsDlg::OnBnClickedQbmsApproved()
{
	try {
		Reload();

	} NxCatchAll(__FUNCTION__);
}

void CReviewCCTransactionsDlg::OnBnClickedQbmsAll()
{
	try {
		Reload();

	} NxCatchAll(__FUNCTION__);
}

void CReviewCCTransactionsDlg::OnBnClickedQbmsReprocess()
{
	try {

		// (j.jones 2015-09-30 10:46) - PLID 67177 - make sure you can't do this if ICCP is enabled
		if (IsICCPEnabled()) {
			AfxMessageBox("Re-processing credit cards is not available when Integrated Credit Card Processing is enabled.");
			return;
		}

		IRowSettingsPtr pRow = m_pList->CurSel;

		//ensure there's a row selected
		if(pRow == NULL) {
			AfxMessageBox("You must select a row to process.");
			return;
		}

		//ensure that row is unapproved, we can't re-process an approved transaction
		if(VarBool(pRow->GetValue(elcIsApproved)) != FALSE) {
			AfxMessageBox("You cannot re-process a transaction that has already been approved.");
			return;
		}

		// (d.thompson 2010-11-11) - PLID 40368 - Special handling.  This dialog, being report-like, needs to support
		//	displaying data from all licensable credit card processors, even though only 1 can be in place.  This is
		//	in case we switch someone due to disputes/dislike with the processor.  We (NexTech) should be ensuring
		//	all open payments are either processed or deleted before we switch, but we'll add a "catch" here just in case.
		BOOL bIsQBMS = VarLong(pRow->GetValue(elcIsQBMS));
		BOOL bIsChase = VarLong(pRow->GetValue(elcIsChase));
		if(g_pLicense && g_pLicense->HasCreditCardProc_QBMS(CLicense::cflrSilent)) {
			if(!bIsQBMS) {
				//We are on a QBMS licensing setup, but this payment is not a QBMS payment!  We can't try to reprocess it, so blow up.
				AfxMessageBox("The payment you are attempting to re-process was made under a different processor.  Please delete "
					"this payment and attempt to process it from scratch if you wish to resubmit.");
				return;
			}
		}
		else if(g_pLicense && g_pLicense->HasCreditCardProc_Chase(CLicense::cflrSilent)) {
			if(!bIsChase) {
				//We are on a Chase licensing setup, but this payment is not a Chase payment!  We can't try to reprocess it, so blow up.
				AfxMessageBox("The payment you are attempting to re-process was made under a different processor.  Please delete "
					"this payment and attempt to process it from scratch if you wish to resubmit.");
				return;
			}
		}
		else {
			//Bad licensing, this tab shouldn't exist.  Alerts happen in other places, so do nothing here.
		}


		//Fields we'll be a needin'
		long nPaymentID = VarLong(pRow->GetValue(elcID));
		long nPaymentType = VarLong(pRow->GetValue(elcType));		//Payment vs Refund

		//Reuse the processing dialog.  To do so, we need to query a bit of the payment information
		CString strSqlBatch;
		CNxParamSqlArray args;

		//Only required if we're doing a refund
		if(nPaymentType == 3) {
			AddParamStatementToSqlBatch(strSqlBatch, args, 
				"SELECT QBMSAlreadyApprovedTransT.TransID "
				"FROM QBMS_CreditTransactionsT "
				"INNER JOIN AppliesT ON QBMS_CreditTransactionsT.ID = AppliesT.SourceID "
				"INNER JOIN PaymentsT PaymentAppliedToT ON AppliesT.DestID = PaymentAppliedToT.ID "
				"INNER JOIN QBMS_CreditTransactionsT QBMSAlreadyApprovedTransT ON PaymentAppliedToT.ID = QBMSAlreadyApprovedTransT.ID "
				"WHERE QBMS_CreditTransactionsT.ID = {INT} AND QBMSAlreadyApprovedTransT.IsApproved = 1", nPaymentID);
		}


		//Basic saved info for the transaction, good for payments or refunds
		// (a.walling 2010-03-15 12:30) - PLID 37751 - Include KeyIndex
		AddParamStatementToSqlBatch(strSqlBatch, args, 
			"SELECT SecurePAN, KeyIndex, CCNumber, CCHoldersName, CCExpDate, CreditCardID, "
			"LineItemT.Amount, LineItemT.PatientID, PaymentsT.ProviderID, LineItemT.DrawerID, LineItemT.LocationID "
			"FROM PaymentsT INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
			"LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
			"WHERE PaymentsT.ID = {INT} AND Deleted = 0;\r\n", nPaymentID);

		// (e.lally 2009-06-21) PLID 34680 - Fixed to use create recordset function.
		_RecordsetPtr prs = CreateParamRecordsetBatch(GetRemoteData(), strSqlBatch, args);

		CString strTransID;
		if(nPaymentType == 3) {
			//First query is the refund info
			if(prs->eof) {
				//Some failure in the query, most likely the payment was not approved, or there's an issue with the apply
				AfxThrowNxException("The payment this refund is applied to does not have the information required to process a credit.");
			}

			strTransID = AdoFldString(prs, "TransID", "");

			//Move to the next recordset
			prs = prs->NextRecordset(NULL);
		}

		if(prs->eof) {
			AfxThrowNxException("Payment %li could not be found to re-process!", nPaymentID);
		}

		//We have to pass the exp date as MM/YY
		CString strExpDate;
		{
			_variant_t varExpDate = prs->Fields->Item["CCExpDate"]->Value;
			if(varExpDate.vt == VT_DATE) {
				COleDateTime dtExpDate = VarDateTime(varExpDate);
				
				strExpDate.Format("%02li/%s", dtExpDate.GetMonth(), FormatString("%li", dtExpDate.GetYear()).Right(2));
			}
		}

		//if we're on a refund, we want to make sure the amount is positive -- you can't refund ($5.00)
		COleCurrency cyAmount = AdoFldCurrency(prs, "Amount");
		if(nPaymentType == 3 && cyAmount < COleCurrency(0, 0)) {
			cyAmount *= -1;
		}

		//This object is only used for modifying tips.  We do not allow that to happen here.
		CPaymentSaveInfo psi;

		//Use the existing dialog
		// (a.walling 2010-03-15 12:30) - PLID 37751 - Use NxCrypto
		CString strCCNumber;
		NxCryptosaur.DecryptStringFromVariant(prs->Fields->Item["SecurePAN"]->Value, AdoFldLong(prs, "KeyIndex", -1), strCCNumber);
		// (d.lange 2010-09-01 14:58) - PLID 40310 - no longer using CQBMS_ProcessCreditCardDlg, its now called CCreditCardProcessingDlg
		CCreditCardProcessingDlg dlg(nPaymentID, strCCNumber, 
			AdoFldString(prs, "CCHoldersName", ""), strExpDate, AdoFldLong(prs, "CreditCardID", -1), 
			cyAmount, nPaymentType, AdoFldLong(prs, "PatientID"), 
			AdoFldLong(prs, "ProviderID", -1), AdoFldLong(prs, "DrawerID", -1), 
			AdoFldLong(prs, "LocationID"), false, "", NULL, &psi, false, strTransID);
		if(dlg.DoModal() == IDOK) {
			//If they successfully passed that dialog, reload our list, the payment could have been modified or processed
			Reload();
		}

	} NxCatchAll(__FUNCTION__);
}

BEGIN_EVENTSINK_MAP(CReviewCCTransactionsDlg, CNxDialog)
	ON_EVENT(CReviewCCTransactionsDlg, IDC_CC_LIST, 3, DblClickCellQbmsList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CReviewCCTransactionsDlg, IDC_CC_ACCT_FILTER_LIST, 16, CReviewCCTransactionsDlg::SelChosenCcAcctFilterList, VTS_DISPATCH)
END_EVENTSINK_MAP()

void CReviewCCTransactionsDlg::DblClickCellQbmsList(LPDISPATCH lpRow, short nColIndex)
{
	try {
		if(lpRow == NULL) {
			return;
		}

		//Just act like they clicked the re-process button, it handles getting the current selection
		OnBnClickedQbmsReprocess();

	} NxCatchAll(__FUNCTION__);
}

// (d.thompson 2010-11-08) - PLID 41379
void CReviewCCTransactionsDlg::SelChosenCcAcctFilterList(LPDISPATCH lpRow)
{
	try {
		Reload();

	} NxCatchAll(__FUNCTION__);
}

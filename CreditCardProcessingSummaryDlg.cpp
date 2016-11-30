// CreditCardProcessingSummaryDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "CreditCardProcessingSummaryDlg.h"
#include "PaymentechUtils.h"
#include "internationalutils.h"
#include "GlobalFinancialUtils.h"
#include "GlobalReportUtils.h"
#include "ReportInfo.h"
#include "Reports.h"
#include "AuditTrail.h"
#include "GlobalAuditUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37021 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.


// (j.gruber 2007-08-22 10:47) - PLID 26584 - created for
enum TransactionListColumns {

	tlcID = 0,
	tlcPatientID = 1,
	tlcPatientName = 2,
	tlcCardHolderName = 3,
	tlcCardTypeName = 4,
	tlcTransTypeName = 5,
	tlcAmount = 6,
	tlcProcessed = 7,
	tlcResult = 8,
	tlcDenialReason = 9,
	tlcCardType = 10,
	tlcBatchNumber = 11,
	tlcManuallyApproved = 12,
	
};

/////////////////////////////////////////////////////////////////////////////
// CCreditCardProcessingSummaryDlg dialog

// (a.walling 2010-04-06 13:51) - PLID 23643 - inappropriate command ID range (0x8000 -> 0xDFFF / 32768 -> 57343)
#define ID_VOID_TRANSACTION 54965
#define ID_PROCESS_TRANSACTION 54966

CCreditCardProcessingSummaryDlg::CCreditCardProcessingSummaryDlg(CWnd* pParent)
	: CNxDialog(CCreditCardProcessingSummaryDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCreditCardProcessingSummaryDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CCreditCardProcessingSummaryDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCreditCardProcessingSummaryDlg)
	DDX_Control(pDX, IDC_NOT_PROCESSED_COLOR, m_NotProcessedBlock);
	DDX_Control(pDX, IDC_NOT_IN_BATCH_BLOCK, m_NotInBatchBlock);
	DDX_Control(pDX, IDC_MANUALLY_APPROVED_BLOCK, m_ManuallyApprovedBlock);
	DDX_Control(pDX, IDC_APPROVED_BLOCK, m_ApprovedBlock);
	DDX_Control(pDX, IDC_BATCH_NUMBER, m_nxeditBatchNumber);
	DDX_Control(pDX, IDC_BATCH_STATUS, m_nxstaticBatchStatus);
	DDX_Control(pDX, IDC_GENERATED_TOTALS_GROUPBOX, m_btnGeneratedTotalsGroupbox);
	DDX_Control(pDX, IDC_PAYMENTECH_TOTALS_GROUPBOX, m_btnPaymentechTotalsGroupbox);
	DDX_Control(pDX, IDC_LEGEND_GROUPBOX, m_btnLegendGroupbox);
	DDX_Control(pDX, IDC_COMMIT_BATCH, m_btnCommitBatch);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCreditCardProcessingSummaryDlg, CNxDialog)
	//{{AFX_MSG_MAP(CCreditCardProcessingSummaryDlg)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_COMMIT_BATCH, OnCommitBatch)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCreditCardProcessingSummaryDlg message handlers

BEGIN_EVENTSINK_MAP(CCreditCardProcessingSummaryDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CCreditCardProcessingSummaryDlg)
	ON_EVENT(CCreditCardProcessingSummaryDlg, IDC_TRANSACTION_LIST, 7 /* RButtonUp */, OnRButtonUpTransactionList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CCreditCardProcessingSummaryDlg, IDC_TRANSACTION_LIST, 18 /* RequeryFinished */, OnRequeryFinishedTransactionList, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()


long CCreditCardProcessingSummaryDlg::GetSortFromPayType(CString strPayType) {

	strPayType.TrimLeft();
	strPayType.TrimRight();

	if (strPayType.CompareNoCase("CR") == 0) {
		return 1;
	}
	else if (strPayType.CompareNoCase("DB") == 0) {
		return 2;
	}
	else if (strPayType.CompareNoCase("AE") == 0) {
		return 3;
	}
	else if (strPayType.CompareNoCase("EB") == 0) {
		return 4;
	}
	else if (strPayType.CompareNoCase("Total")  == 0) {
		return 5;
	}
	else {
		ASSERT(FALSE);
		return -1;
	}


}

BOOL CCreditCardProcessingSummaryDlg::SendBatchInquiry() {
	
	try {
		if (!PaymentechUtils::IsPaymentechInstalled()) {
			MsgBox("Paymentech is not installed properly. Please check the installation and try again.");
			return FALSE;
		}

		PaymentechUtils::ResponseStruct Resp;
		PaymentechUtils::TransStruct tsStruct;

		tsStruct.tcTransactionCode = PaymentechUtils::tcBatchInquiry;
		
		if (! PaymentechUtils::SendBatchMessage(tsStruct, Resp)) {
			
			//this means the communication failed
			MessageBox("The batch information could not be gathered from Paymentech, this is the result of a communication error.\n Please check your internet connection and try again.");

			//set all the boxes to undefined or whatnot
			return FALSE;
			
		}
		
		//now we can fill our values that we got back from the message
		if (!Resp.bApproved) {

			//they will fail it if there are no items in the batch
			if (Resp.nBatchTransactionCount == 0) {
				//gray out the commit batch button and keep going
				GetDlgItem(IDC_COMMIT_BATCH)->EnableWindow(FALSE);
			}
			else {
				ASSERT(FALSE);
				return FALSE;
			}
			
		}
		else {
			//Enable the button in case its not enabled
			GetDlgItem(IDC_COMMIT_BATCH)->EnableWindow(TRUE);
		}

		NXDATALIST2Lib::IRowSettingsPtr pRow;
		if (Resp.nBatchTransactionCount > 0) {
			if (Resp.nNumTransactionsPayType1 > 0) {
				pRow = m_pPaymentechList->GetNewRow();
				pRow->PutValue(0, (long)1);
				pRow->PutValue(1, _variant_t(PaymentechUtils::GetNameFromType(Resp.strPayType1)));
				pRow->PutValue(2, (long)Resp.nNumTransactionsPayType1);
				pRow->PutValue(3, _variant_t(Resp.cyNetAmountPayType1));
				pRow->PutValue(4, (long)GetSortFromPayType(Resp.strPayType1));
				m_pPaymentechList->AddRowAtEnd(pRow, NULL);

				
			}

			if (Resp.nNumTransactionsPayType2 > 0) {
				pRow = m_pPaymentechList->GetNewRow();
				pRow->PutValue(0, (long)2);
				pRow->PutValue(1, _variant_t(PaymentechUtils::GetNameFromType(Resp.strPayType2)));
				pRow->PutValue(2, (long)Resp.nNumTransactionsPayType2);
				pRow->PutValue(3, _variant_t(Resp.cyNetAmountPayType2));
				pRow->PutValue(4, (long)GetSortFromPayType(Resp.strPayType2));
				m_pPaymentechList->AddRowAtEnd(pRow, NULL);
			}

			if (Resp.nNumTransactionsPayType3 > 0) {
				pRow = m_pPaymentechList->GetNewRow();
				pRow->PutValue(0, (long)3);
				pRow->PutValue(1, _variant_t(PaymentechUtils::GetNameFromType(Resp.strPayType3)));
				pRow->PutValue(2, (long)Resp.nNumTransactionsPayType3);
				pRow->PutValue(3, _variant_t(Resp.cyNetAmountPayType3));
				pRow->PutValue(4, (long)GetSortFromPayType(Resp.strPayType3));
				m_pPaymentechList->AddRowAtEnd(pRow, NULL);
			}

			if (Resp.nNumTransactionsPayType4 > 0) {
				pRow = m_pPaymentechList->GetNewRow();
				pRow->PutValue(0, (long)4);
				pRow->PutValue(1, _variant_t(PaymentechUtils::GetNameFromType(Resp.strPayType4)));
				pRow->PutValue(2, (long)Resp.nNumTransactionsPayType4);
				pRow->PutValue(3, _variant_t(Resp.cyNetAmountPayType4));
				pRow->PutValue(4, (long)GetSortFromPayType(Resp.strPayType4));
				m_pPaymentechList->AddRowAtEnd(pRow, NULL);
			}

			SetDlgItemText(IDC_BATCH_STATUS, "Open");	
		}
		else {
			SetDlgItemText(IDC_BATCH_STATUS, "No Transactions in Open Batch");
		}
		
		
		//now the total line
		pRow = m_pPaymentechList->GetNewRow();
		
		if (pRow) {
			//make it red so it sticks out
			pRow->PutForeColor(RGB(255,0,0));
			
			pRow->PutValue(0, (long)5);
			pRow->PutValue(1, _variant_t("Totals:"));
			pRow->PutValue(2, (long)(Resp.nBatchTransactionCount));
			pRow->PutValue(3, _variant_t(Resp.cyBatchNetAmount));
			pRow->PutValue(4, (long)GetSortFromPayType("Total"));
			m_pPaymentechList->AddRowAtEnd(pRow, NULL);
		}

		if (Resp.dtBatchOpen.GetStatus() == COleDateTime::valid) {
			m_pdtBatchOpen->SetDateTime(Resp.dtBatchOpen);
			SetDlgItemText(IDC_BATCH_NUMBER, Resp.strBatchNumber);

			//update the data if the batch open date isn't filled in yet
			ExecuteSql("UPDATE TransactionBatchT SET BatchOpenDate = '%s' WHERE BatchNumber = '%s'", FormatDateTimeForSql(Resp.dtBatchOpen), Resp.strBatchNumber);
		}

		m_strBatchID = Resp.strBatchNumber;

		return TRUE;

		
	}NxCatchAll("Error in CCreditCardProcessingSummaryDlg::SendBatchInquiry");

	return FALSE;

}


BOOL CCreditCardProcessingSummaryDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		//Initialize the datalists
		m_pTransList = BindNxDataList2Ctrl(IDC_TRANSACTION_LIST, false);
		m_pGeneratedList = BindNxDataList2Ctrl(IDC_GENERATED_TOTALS, false);
		m_pPaymentechList = BindNxDataList2Ctrl(IDC_PAYMENTECH_TOTALS, false);		

		m_pdtBatchClose = BindNxTimeCtrl(this, IDC_BATCH_CLOSE_DATE);
		m_pdtBatchOpen = BindNxTimeCtrl(this, IDC_BATCH_OPEN_DATE);

		m_btnCommitBatch.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		// (a.walling 2007-08-03 17:35) - PLID 26922 - Added waitcursor
		CWaitCursor cws;

		//get the batch ID from Paymentech
		if (!SendBatchInquiry()) {
			//end the dialog
			EndDialog(IDCANCEL);
			return TRUE;
		}


		//set up the transaction datalist
		CString strFrom;
		strFrom.Format("(SELECT UserDefinedID, PersonT.Last + ', ' + PersonT.First as PatName, CreditTransactionsT.ID, PaymentPlansT.CCHoldersName AS CardHolderName,  "
			"   CASE WHEN CreditTransactionsT.TransactionCode = 1 then 'Credit Sale' ELSE  "
			"   CASE WHEN CreditTransactionsT.TransactionCode = 2 then 'Credit Authorization' ELSE "
			"   CASE WHEN CreditTransactionsT.TransactionCode = 6 then 'Credit Return' ELSE "
			"   CASE WHEN CreditTransactionsT.TransactionCode = 3 then 'Credit Forced Sale' ELSE "
			"   CASE WHEN CreditTransactionsT.TransactionCode = 27 then 'Interac Sale' ELSE "
			"   CASE WHEN CreditTransactionsT.TransactionCode = 28 then 'Interac Sale W/Cash Back' ELSE "  
			"   CASE WHEN CreditTransactionsT.TransactionCode = 29 then 'Interac Return' END END END END END END END AS TransType, "
			"   CreditCardNamesT.CardName, CASE WHEN TransactionCode IN (6,29) THEN -1 * CreditTransactionsT.TransactionAmount ELSE CreditTransactionsT.TransactionAmount END AS TransactionAmount, IsProcessed,  "
			"   CASE WHEN IsProcessed = 1 THEN CASE WHEN CreditTransactionsT.IsApproved = 1 then 'Approved' else 'Declined'  "
			"   END ELSE '' END AS Result, "
			"   CASE WHEN IsProcessed = 1 THEN CASE WHEN CreditTransactionsT.IsApproved= 1 then '' else RespMessage END  "
			"   ELSE '' END AS ReasonMessage, CreditCardNamesT.CardType, TransactionBatchT.BatchNumber, CreditTransactionsT.ManuallyProcessed   "
			"   FROM CreditTransactionsT INNER JOIN PaymentPlansT ON   "
			"   CreditTransactionsT.ID = PaymentPlansT.ID  "
			"   INNER JOIN LineItemT ON CreditTransactionsT.ID = LineItemT.ID "
			"   INNER JOIN PersonT ON LineItemT.PatientID = PersonT.ID "
			"   INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			"   LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
			"   LEFT JOIN TransactionBatchT ON CreditTransactionsT.BatchID = TransactionBatchT.ID " 
			"   WHERE (CreditTransactionsT.BatchID IN (SELECT ID FROM TransactionBatchT WHERE BatchNumber = '%s') OR  CreditTransactionsT.BatchID IN (SELECT ID FROM TransactionBatchT WHERE BatchNumber = '')) "
			"   AND (LineItemT.Deleted = 0) AND (CreditTransactionsT.IsVoid = 0) "
			"   )Q ", m_strBatchID);

		m_pTransList->FromClause = _bstr_t(strFrom);

		m_pTransList->Requery();

	
		GetDlgItem(IDC_NOT_PROCESSED_COLOR)->SetWindowText("");
		GetDlgItem(IDC_MANUALLY_APPROVED_BLOCK)->SetWindowText("");
		GetDlgItem(IDC_APPROVED_BLOCK)->SetWindowText("");
		GetDlgItem(IDC_NOT_IN_BATCH_BLOCK)->SetWindowText("");

		m_brushNotProcessed.CreateSolidBrush(RGB(150,150,150));
		m_brushManuallyApproved.CreateSolidBrush(RGB(128,255,255));
		m_brushApproved.CreateSolidBrush(RGB(128,255,128));
		m_brushNotInBatch.CreateSolidBrush(RGB(255,255,179));



		
		
	}NxCatchAll("Error in CCreditCardProcessingSummaryDlg::OnInitDialog");

	//we can't load the generated list until onRequeryFinished
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

HBRUSH CCreditCardProcessingSummaryDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	switch (pWnd->GetDlgCtrlID()) {
		case IDC_NOT_PROCESSED_COLOR:
			return m_brushNotProcessed;
		break;

		case IDC_MANUALLY_APPROVED_BLOCK:
			return m_brushManuallyApproved;
		break;

		case IDC_APPROVED_BLOCK:
			return m_brushApproved;
		break;

		case IDC_NOT_IN_BATCH_BLOCK:
			return m_brushNotInBatch;
		break;
	}


	
	
	
	// TODO: Return a different brush if the default is not desired
	return hbr;
}

void CCreditCardProcessingSummaryDlg::OnRButtonUpTransactionList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {
		//make a menu to void or process a transaction
		CMenu menPopup;
		menPopup.m_hMenu = CreatePopupMenu();

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if (pRow) {

			m_pTransList->CurSel = pRow;

			if (VarBool(pRow->GetValue(tlcProcessed))) {

				//this transaction has been processed already, so their only option is to void it
				menPopup.InsertMenu(1, MF_BYPOSITION, ID_VOID_TRANSACTION, "&Void Transaction");

			}
			else {
				//this transaction hasn't been processed it, so let them process it if they would like
				menPopup.InsertMenu(1, MF_BYPOSITION, ID_PROCESS_TRANSACTION, "&Process Transaction");
			}

			CPoint pt(x,y);
			CWnd* pWnd = GetDlgItem(IDC_TRANSACTION_LIST);
			if (pWnd != NULL)
			{	pWnd->ClientToScreen(&pt);
				menPopup.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this, NULL);
			}
			
		}	
	}NxCatchAll("Error In CCreditCardProcessingSummaryDlg::InRButtonUpTransactionList");
}

BOOL CCreditCardProcessingSummaryDlg::CheckBatch() 
{
	try {

		//run through the datalist and make sure that everything in the batch is processed
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pTransList->GetFirstRow();

		long nManAuthCount = 0;
		long nNotProcessedCount = 0;
		long nNotInBatchCount = 0;
		while (pRow) {

			CString strBatchNumber = VarString(pRow->GetValue(tlcBatchNumber), "");
			strBatchNumber.TrimLeft();
			strBatchNumber.TrimRight();
			if (strBatchNumber.IsEmpty()) {
				nNotInBatchCount++;
			}
			else {
				if (! VarBool(pRow->GetValue(tlcProcessed))) {

					if (VarBool(pRow->GetValue(tlcManuallyApproved))) {

						nManAuthCount++;
					}
					else {

						//its just not processed
						nNotProcessedCount++;
					}
				}
			}

			

			pRow = pRow->GetNextRow();
		}

		CString strMessage;

		if (nNotInBatchCount > 0) {
			strMessage.Format("There are %li transaction(s) not in a batch.  These will be added to the current batch.", nNotInBatchCount);
					
			MsgBox(strMessage);

			//they want to put them in the current batch
			//first see if we already have a batch with the current batch number
			ADODB::_RecordsetPtr rsBatch = CreateRecordset("SELECT ID FROM TransactionBatchT WHERE BatchNumber = '%s'", m_strBatchID);
			if (rsBatch->eof) {
				ExecuteSql("UPDATE TransactionBatchT SET BatchNumber = '%s' WHERE BatchNumber = ''", m_strBatchID);
			}
			else {
					
				long nCurrentBatchID = AdoFldLong(rsBatch, "ID");
						
				//first update the transaction to the new batch ID
				ExecuteSql("UPDATE CreditTransactionsT SET BatchID = %li WHERE BatchID IN (SELECT ID FROM TransactionBatchT WHERE BatchNumber = '')", nCurrentBatchID);

				//now delete from the batch table
				ExecuteSql("DELETE FROM TransactionBatchT WHERE BatchNumber = ''");
			}


			m_pTransList->Requery();
			return FALSE;
	
		}


		if (nManAuthCount > 0 && nNotProcessedCount == 0) {
			strMessage.Format("There are %li manually approved transaction(s) in this batch. These need to be processed in order to commit the sale.", nManAuthCount);
		}
		else if (nManAuthCount > 0 && nNotProcessedCount > 0) {
			strMessage.Format("There are %li manually approved transaction(s) and %li unprocessed transaction(s).  These need to be processed in order to commit the sale.", nManAuthCount, nNotProcessedCount);
		}
		else if (nManAuthCount == 0 && nNotProcessedCount > 0 ) {
			strMessage.Format("There are %li unprocessed transaction(s) in this batch.  These need to be processed in order to commit the sale.", nNotProcessedCount);
		}
		
		
		if (!strMessage.IsEmpty()) {
			CString strOutput;
			strOutput.Format("%s\nWould you like to cancel committing this batch?", strMessage);

			if (IDYES == MsgBox(MB_YESNO, strOutput)) {

				//they are cancelling
				return FALSE;
			}
			
		}

		//now check that the our batch matches paymentech's
		COleCurrency cyGeneratedTotal, cyPaymentechTotal;
		long nGeneratedCount = 0, nPaymentechCount = 0;
		pRow = m_pGeneratedList->FindByColumn(0, (long)5, m_pGeneratedList->GetFirstRow(), FALSE);
		if (pRow) {

			cyGeneratedTotal = VarCurrency(pRow->GetValue(3));
			nGeneratedCount = VarLong(pRow->GetValue(2));
		}
		else {
			ASSERT(FALSE);
		}

		pRow = m_pPaymentechList->FindByColumn(0, (long)5, m_pPaymentechList->GetFirstRow(), FALSE);
		if (pRow) {

			cyPaymentechTotal = VarCurrency(pRow->GetValue(3));
			nPaymentechCount = VarLong(pRow->GetValue(2));
		}
		else {
			ASSERT(FALSE);
		}

		if (cyGeneratedTotal != cyPaymentechTotal) {
			CString strOutput = "The transaction total generated by Practice does not match the transaction total generated by Paymentech.   It is HIGHLY recommended that you cancel commiting this batch.\nWould you like to cancel committing this batch?";
			if (IDYES == MsgBox(MB_YESNO, strOutput)) {

				//they are cancelling
				return FALSE;
			}
		}

		if (nGeneratedCount != nPaymentechCount) {
			CString strOutput = "The transaction count generated by Practice does not match the transaction count generated by Paymentech.   It is HIGHLY recommended that you cancel commiting this batch.\nWould you like to cancel committing this batch?";
			if (IDYES == MsgBox(MB_YESNO, strOutput)) {

				//they are cancelling
				return FALSE;
			}
		}

		//if we got here, they are going through with it
		return TRUE;
	}NxCatchAll("Error in CheckBatch");

	return FALSE;
}


void CCreditCardProcessingSummaryDlg::OnCommitBatch() 
{
	try {
		// (a.walling 2007-08-03 17:49) - PLID 26922 - Check permissions

		// (d.thompson 2009-07-01) - PLID 34230 - Removed these permissions, this dialog should not be in live use
		AfxThrowNxException("CCProcessingSummary is not valid for use.");
		//if (!CheckCurrentUserPermissions(bioCCBatchCommit, sptWrite))
		//	return;
		
		//first make sure they don't have any unprocessed or unbatched items in the list
		if (! CheckBatch()) {
			return;
		}

		PaymentechUtils::TransStruct tsStruct;
		PaymentechUtils::ResponseStruct Response;

		tsStruct.tcTransactionCode = PaymentechUtils::tcBatchRelease;

		if (! PaymentechUtils::SendBatchMessage(tsStruct, Response) ) {

			//this is a communication error
			ThrowNxException("Communication Error committing batch, please check your internet connection and try again");
			return;
		}

		if (!Response.bApproved) {
			//we had another sort of error
			ThrowNxException("Error committing batch, Paymentech returned errorCode: %s, Message: %s", Response.strErrorCode, PaymentechUtils::MessageLookup(Response.strErrorCode,Response.strResponseMessage, PaymentechUtils::lcEnglish));
			return;
		}
			


		m_pdtBatchClose->SetDateTime(Response.dtBatchClose);
		SetDlgItemText(IDC_BATCH_STATUS, "Closed");

		//update the data for this batch
		if (Response.dtBatchClose.GetStatus() == COleDateTime::valid) {
			ExecuteSql("UPDATE TransactionBatchT SET BatchCloseDate = '%s', BatchReleased = 1 WHERE BatchNumber = '%s' ", FormatDateTimeForSql(Response.dtBatchClose), m_strBatchID);
		}

		
		// (j.gruber 2007-08-03 09:18) - 26926 - audit
		long nBatchID = -1;
		ADODB::_RecordsetPtr rsBatch = CreateRecordset("SELECT ID FROM TransactionBatchT WHERE BatchNumber = %s", _Q(m_strBatchID));
		if (!rsBatch->eof) {
			nBatchID = AdoFldLong(rsBatch, "ID");
		}
			
		long nAuditID = -1;
		nAuditID = BeginNewAuditEvent();
		if (nAuditID != -1) {
			CString strDesc;
			
			CString strResultDesc;
			strResultDesc = "Approved";
			
			strDesc.Format("Batch Committed: %s - Result: %s", m_strBatchID,
				strResultDesc);
			AuditEvent(-1, "", nAuditID, aeiCommitBatch, nBatchID, "", strDesc, 1, aetCreated);
		}

		//refresh the list just in case something got added
		m_pTransList->Requery();

		//clear the paymentech list
		m_pPaymentechList->Clear();

		//now change some of the validations based on the response
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		if (Response.nNumTransactionsPayType1 > 0) {
			pRow = m_pPaymentechList->GetNewRow();
			pRow->PutValue(0, (long)1);
			pRow->PutValue(1, _variant_t(PaymentechUtils::GetNameFromType(Response.strPayType1)));
			pRow->PutValue(2, (long)Response.nNumTransactionsPayType1);
			pRow->PutValue(3, _variant_t(Response.cyNetAmountPayType1));
			m_pPaymentechList->AddRowAtEnd(pRow, NULL);
		}

		if (Response.nNumTransactionsPayType2 > 0) {
			pRow = m_pPaymentechList->GetNewRow();
			pRow->PutValue(0, (long)2);
			pRow->PutValue(1, _variant_t(PaymentechUtils::GetNameFromType(Response.strPayType2)));
			pRow->PutValue(2, (long)Response.nNumTransactionsPayType2);
			pRow->PutValue(3, _variant_t(Response.cyNetAmountPayType2));
			m_pPaymentechList->AddRowAtEnd(pRow, NULL);
		}

		if (Response.nNumTransactionsPayType3 > 0) {
			pRow = m_pPaymentechList->GetNewRow();
			pRow->PutValue(0, (long)3);
			pRow->PutValue(1, _variant_t(PaymentechUtils::GetNameFromType(Response.strPayType3)));
			pRow->PutValue(2, (long)Response.nNumTransactionsPayType3);
			pRow->PutValue(3, _variant_t(Response.cyNetAmountPayType3));
			m_pPaymentechList->AddRowAtEnd(pRow, NULL);
		}

		if (Response.nNumTransactionsPayType4 > 0) {
			pRow = m_pPaymentechList->GetNewRow();
			pRow->PutValue(0, (long)4);
			pRow->PutValue(1, _variant_t(PaymentechUtils::GetNameFromType(Response.strPayType4)));
			pRow->PutValue(2, (long)Response.nNumTransactionsPayType4);
			pRow->PutValue(3, _variant_t(Response.cyNetAmountPayType4));
			m_pPaymentechList->AddRowAtEnd(pRow, NULL);
		}

		//now the total line
		pRow = m_pPaymentechList->GetNewRow();
		
		//make it red so it sticks out
		pRow->PutForeColor(RGB(255,0,0));
		
		pRow->PutValue(0, (long)5);
		pRow->PutValue(1, _variant_t("Totals:"));
		pRow->PutValue(2, (long)Response.nBatchTransactionCount);
		pRow->PutValue(3, _variant_t(Response.cyBatchNetAmount));
		m_pPaymentechList->AddRowAtEnd(pRow, NULL);


		// (j.gruber 2007-07-31 15:19) - PLID 26719 - Print a Transaction report
		CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(606)]);
		
		CPtrArray params;
		CRParameterInfo *tmpParam;
	
		//sets up params to quote
		tmpParam = new CRParameterInfo;
		tmpParam->m_Name = "DateTo";
		tmpParam->m_Data = FormatDateTimeForInterface(Response.dtBatchClose, NULL, dtoDate, FALSE);
		params.Add((void *)tmpParam);

		tmpParam = new CRParameterInfo;
		tmpParam->m_Name = "DateFrom";
		tmpParam->m_Data = FormatDateTimeForInterface(Response.dtBatchClose, NULL, dtoDate, FALSE);
		params.Add((void *)tmpParam);

		infReport.strFilterField = m_strBatchID;

		CPrintInfo prInfo;

		BOOL bReturn = RunReport(&infReport, &params, TRUE, this, "Credit Card Batch Report", &prInfo);
		ClearRPIParameterList(&params);

		EndDialog(IDOK);


	}NxCatchAll("Error in CCreditCardProcessingSummaryDlg::OnCommitBatch");
	
}

void CCreditCardProcessingSummaryDlg::OnOK() 
{
	// TODO: Add extra validation here
	
	CDialog::OnOK();
}


void CCreditCardProcessingSummaryDlg::OnCancel() 
{
	// TODO: Add extra validation here
	
	CDialog::OnCancel();
}


BOOL CCreditCardProcessingSummaryDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	switch(wParam) {

		case ID_VOID_TRANSACTION:
			OnVoidTransaction();
		break;

		case ID_PROCESS_TRANSACTION:
			OnProcessTransaction();
		break;
	}

	return CNxDialog::OnCommand(wParam, lParam);
}



BOOL CCreditCardProcessingSummaryDlg::VoidPayment(long nPaymentID, long &nVoidRetRefNum) {

	try {
		PaymentechUtils::TransStruct  tsStruct;
		PaymentechUtils::ResponseStruct rsResp;

		//fill up the transaction 
		tsStruct.tcTransactionCode = PaymentechUtils::tcVOID;
		if (AsString(nPaymentID).GetLength() > 6) {
			//just send 0
			tsStruct.nSequenceNumber = 0;
		}
		else {
			tsStruct.nSequenceNumber = nPaymentID;
		}

		//get the rest of the information we will need from the database
		// (a.walling 2007-10-31 09:43) - PLID 27891 - Get the encrypted CC number instead of the CCNumber (which is the last 4 digits)
			// (a.walling 2010-03-15 12:18) - PLID 37751 - Include KeyIndex
		ADODB::_RecordsetPtr rsTrans = CreateRecordset("SELECT RetrievalRefNum, SecurePAN as AccountNumber, KeyIndex, "
			"  (SELECT Top 1 RetrievalRefNum FROM CreditTransactionsT WHERE RetrievalRefNum <> '' ORDER BY ID DESC) as LastRetrievalRefNum  "
			"  FROM CreditTransactionsT LEFT JOIN PaymentPlansT ON  "
			"  CreditTransactionsT.ID = PaymentPlansT.ID  "
			"  WHERE CreditTransactionsT.ID = %li", nPaymentID);

		if (! rsTrans->eof) {

			tsStruct.nLastRetrievalReferenceNumber  = AdoFldLong(rsTrans, "LastRetrievalRefNum");
			tsStruct.nRetrievalReferenceNumber = AdoFldLong(rsTrans, "RetrievalRefNum");
			// (a.walling 2007-10-31 09:44) - PLID 27891 - Now decrypt it.
			// (a.walling 2010-03-15 12:18) - PLID 37751 - Use NxCrypto
			//tsStruct.strAccountNumber = DecryptStringFromVariant(rsTrans->Fields->Item["AccountNumber"]->Value);
			NxCryptosaur.DecryptStringFromVariant(rsTrans->Fields->Item["AccountNumber"]->Value, AdoFldLong(rsTrans, "KeyIndex", -1), tsStruct.strAccountNumber);
		}
		else {
			ThrowNxException("Error in CCreditCardProcessingSummaryDlg::VoidPayment, Could not find transaction");
		}

		//now send the message to Paymentech
		if (! PaymentechUtils::SendVoidMessage(tsStruct, rsResp)) {
			//there must have been a communications error
			ThrowNxException("Communication Error Voiding transaction, please check your internet connection and try again");
			nVoidRetRefNum = -1;
			return FALSE;
		}
		else {

			if (!rsResp.bApproved) {

				//we had another sort of error
				ThrowNxException("Error voiding transaction, Paymentech returned errorCode: %s, Message: %s", rsResp.strErrorCode, rsResp.strResponseMessage);
				return FALSE;
			}
			
			nVoidRetRefNum = rsResp.nRetreivalReferenceNumber;

			return TRUE;
		}
	}NxCatchAllCall("Error in CCreditCardProcessingSummaryDlg::VoidPayment", nVoidRetRefNum = -1; return FALSE);

}


void CCreditCardProcessingSummaryDlg::UpdateScreen() {

	try {

		//clear the lists
		m_pPaymentechList->Clear();
		m_pGeneratedList->Clear();

		//start reloading the datalist
		m_pTransList->Requery();

		//now reload the paymentech infomation
		if (!SendBatchInquiry() ) {
			EndDialog(IDCANCEL);
			return;
		}

		//on requery finished will take care of the rest
	}NxCatchAll("Error in CCreditCardProcessingSummaryDlg::UpdateScreen");

}

void CCreditCardProcessingSummaryDlg::OnVoidTransaction() {

	try {
		// (a.walling 2007-08-03 17:49) - PLID 26922 - Check permissions
		// (a.walling 2007-09-27 13:21) - PLID 26759 - Note that the CCVoid permission trumps that of bioPayment's delete permission

		// (d.thompson 2009-07-01) - PLID 34230 - Removed these permissions, this dialog should not be in live use
		AfxThrowNxException("CCProcessingSummary is not valid for use.");

		//if (!CheckCurrentUserPermissions(bioCCVoid, sptWrite))
		//	return;

		//make sure that this transaction has been processed
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pTransList->CurSel;

		if (pRow) {

			if (VarBool(pRow->GetValue(tlcProcessed))) {

				//it is processed, let's make sure they want to do this
				CString strAmount, strPatient;
				strPatient = VarString(pRow->GetValue(tlcPatientName));
				strAmount = FormatCurrencyForInterface(VarCurrency(pRow->GetValue(tlcAmount)));

				CString strMessage;
				strMessage.Format("Are you sure you want to void the transaction for %s for %s?\n "
					" Doing this will also delete the payment/refund for this transaction", strPatient, strAmount);

				if (IDYES == MsgBox(MB_YESNO, strMessage)) {

					//they said yes

					//check to make sure they have permission to do this
					if (CheckCurrentUserPermissions(bioPayment, sptDelete)) {

						long nPaymentID = VarLong(pRow->GetValue(tlcID));
						long nVoidRetRefNum = -1;

						//first void the payment
						if (VoidPayment(nPaymentID, nVoidRetRefNum)) {

							//now delete the payment
							DeletePayment(nPaymentID, TRUE);

							//for now just remove the row and update the screen
							ExecuteSql("UPDATE CreditTransactionsT SET IsVoid = 1, VoidRetrievalRefNum = %li WHERE ID = %li", nVoidRetRefNum, nPaymentID);

							// (j.gruber 2007-08-03 09:18) - 26926 - audit
							long nAuditID = -1;
							nAuditID = BeginNewAuditEvent();
							if (nAuditID != -1) {
								CString strDesc;
								CString strProcessDesc;
								strProcessDesc = "Manually Entered";
								

								CString strResultDesc;
								//if we got here, its approved because otherwise it would've returned false
								strResultDesc = "Approved";								


								strDesc.Format("Void Transaction: %s - %s - Result: %s", 
									FormatCurrencyForInterface(VarCurrency(pRow->GetValue(tlcAmount))), 
									strProcessDesc, strResultDesc);
								AuditEvent(VarLong(pRow->GetValue(tlcPatientID), -1), VarString(pRow->GetValue(tlcPatientName), ""), nAuditID, aeiVoidTransaction, nPaymentID, "", strDesc, 1, aetCreated);
							}

							
							UpdateScreen();
						}
										
					}
				}
			}
			else {

				//this payment hasn't been processed
				MsgBox("You may not void a transaction that hasn't been processed through Paymentech.");
			}
		}			
	}NxCatchAll("Error in CCreditCardProcessingSummaryDlg::OnVoidTransaction");

			

}


void CCreditCardProcessingSummaryDlg::OnProcessTransaction() {

	try {
		// (a.walling 2007-08-03 17:49) - PLID 26922 - Check permissions
		if (!CheckCurrentUserPermissions(bioCCProcess, sptWrite))
			return;

		//make sure that this transaction has been processed
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pTransList->CurSel;

		if (pRow) {

			if (! VarBool(pRow->GetValue(tlcProcessed))) {

				//it is not processed, let's make sure they want to do this
				CString strAmount, strPatient;
				strPatient = VarString(pRow->GetValue(tlcPatientName));
				strAmount = FormatCurrencyForInterface(VarCurrency(pRow->GetValue(tlcAmount)));

				CString strMessage;
				strMessage.Format("Are you sure you want to process the transaction for %s for %s?", strPatient, strAmount);

				if (IDYES == MsgBox(MB_YESNO, strMessage)) {

					long nPaymentID = VarLong(pRow->GetValue(tlcID));
					PaymentechUtils::ResponseStruct rsStruct;
					PaymentechUtils::TransactionCode tcCode;

					//let's process this transaction
					if (ProcessTransaction(nPaymentID, rsStruct, tcCode)) {

						CString strMsg = rsStruct.strResponseMessage;
						strMsg.TrimRight();
						strMsg.TrimLeft();

						//the force sale went through, so we can update everything
						ExecuteSql("UPDATE CreditTransactionsT SET RetrievalRefNum = %li, RespMessage = '%s', "
							" IsProcessed = 1, IsApproved = 1 WHERE ID = %li", 
							rsStruct.nRetreivalReferenceNumber, _Q(strMsg),
							nPaymentID);
						ExecuteSql("UPDATE PaymentPlansT SET CCAuthNo = '%s' WHERE ID = %li", 
							_Q(rsStruct.strAuthCode), nPaymentID);

						// (j.gruber 2007-08-03 09:18) - 26926 - audit
						long nAuditID = -1;
						nAuditID = BeginNewAuditEvent();
						if (nAuditID != -1) {
							CString strDesc;
							CString strProcessDesc;
							strProcessDesc = "Manually Entered";

							CString strResultDesc;
							strResultDesc = "Approved";
							
							strDesc.Format("%s Transaction From Batch Screen: %s - %s - Result: %s", GetDescFromTransactionType(tcCode),
								FormatCurrencyForInterface(VarCurrency(pRow->GetValue(tlcAmount))), 
								strProcessDesc, strResultDesc);
							AuditEvent(VarLong(pRow->GetValue(tlcPatientID), -1), VarString(pRow->GetValue(tlcPatientName), ""), nAuditID, aeiProcessTransaction, nPaymentID, "", strDesc, 1, aetCreated);
						}

						//update the screen
						UpdateScreen();
					}
				}
			}
			else {

				//this payment hasn't been processed
				ASSERT(FALSE);
				MsgBox("This transaction has already been processed.");
			}
		}			
	}NxCatchAll("Error In CCreditCardProcessingSummaryDlg::OnProcessTransaction");


}


CString CCreditCardProcessingSummaryDlg::GetDescFromTransactionType(PaymentechUtils::TransactionCode tcCode) {

	switch (tcCode) {

		case PaymentechUtils::tcSale:
			return "Sale";
		break;

		case PaymentechUtils::tcReturn:
			return "Refund";
		break;

		case PaymentechUtils::tcVOID:
			return "Void";
		break;

		case PaymentechUtils::tcForceSale:
			return "Force Sale";
		break;

		default:
			return "";
		break;
	}
}


BOOL CCreditCardProcessingSummaryDlg::ProcessTransaction(long nPaymentID, PaymentechUtils::ResponseStruct &rsResp, PaymentechUtils::TransactionCode &tcCode) {

	try  {
		PaymentechUtils::TransStruct  tsStruct;
		
		//fill up the transaction 
		

		//get the rest of the information we will need from the database
		// (a.walling 2007-10-31 09:45) - PLID 27891 - Get the encrypted PAN rather than the CCNumber (last 4 digits)
			// (a.walling 2010-03-15 12:18) - PLID 37751 - Include KeyIndex
		ADODB::_RecordsetPtr rsTrans = CreateRecordset("SELECT PaymentPlansT.CCHoldersName, "
			"  PaymentPlansT.SecurePAN, PaymentPlansT.KeyIndex, "
			"  PaymentPlansT.CCExpDate, CreditTransactionsT.TransactionCode, PaymentPlansT.CCAuthNo, "
			"  CreditTransactionsT.PinCapabilityCode, CreditTransactionsT.EntryDataSource, "
			"  CreditTransactionsT.TransactionAmount, CreditTransactionsT.CardHolderStreetAddress, "
			"  CreditTransactionsT.CardHolderStreetAddress2, CardHolderZip, CreditTransactionsT.IsProcessed, "
			"  CreditTransactionsT.ManuallyProcessed, LineItemT.Type "
			"  FROM CreditTransactionsT INNER JOIN PaymentPlansT ON  "
			"  CreditTransactionsT.ID = PaymentPlansT.ID  "
			"  LEFT JOIN LineItemT ON PaymentPlansT.ID = LineItemT.ID "
			"  WHERE CreditTransactionsT.ID = %li", nPaymentID);

		if (! rsTrans->eof) {

			
			tsStruct.bSwiped = FALSE;
			
			if (AdoFldBool(rsTrans, "ManuallyProcessed")) {

				tsStruct.tcTransactionCode = PaymentechUtils::tcForceSale;
				tcCode = PaymentechUtils::tcForceSale;
				tsStruct.strAuthorizationCode = AdoFldString(rsTrans, "CCAuthNo");
			}
			else {
				if (AdoFldLong(rsTrans, "Type") == 3) {
					tsStruct.tcTransactionCode = PaymentechUtils::tcReturn;
					tcCode = PaymentechUtils::tcReturn;
				}
				else {
					tsStruct.tcTransactionCode = PaymentechUtils::tcSale;
					tcCode = PaymentechUtils::tcSale;
				}
			}
			tsStruct.piPresenceIndicator = PaymentechUtils::piBypassed;
			tsStruct.nPINCapabilityMode = AdoFldLong(rsTrans, "PinCapabilityCode");
			//this is ALWAYS manual
			tsStruct.edsEntryDataSource = PaymentechUtils::edsManuallyEntered;
			tsStruct.strCardHolderStreet = AdoFldString(rsTrans, "CardHolderStreetAddress", "");
			tsStruct.strExtendedStreetInfo = AdoFldString(rsTrans, "CardHolderStreetAddress2", "");
			tsStruct.strCardHolderZip = AdoFldString(rsTrans, "CardHolderZip", "");
			tsStruct.cyTransAmount = AdoFldCurrency(rsTrans, "TransactionAmount");
			// (a.walling 2007-10-31 09:45) - PLID 27891 - Decrypt the CC number
			// (a.walling 2010-03-15 12:18) - PLID 37751 - Use NxCrypto
			//tsStruct.strAccountNumber = DecryptStringFromVariant(rsTrans->Fields->Item["SecurePAN"]->Value);
			NxCryptosaur.DecryptStringFromVariant(rsTrans->Fields->Item["SecurePAN"]->Value, AdoFldLong(rsTrans, "KeyIndex", -1), tsStruct.strAccountNumber);
			COleDateTime dtExpire = AdoFldDateTime(rsTrans, "CCExpDate");

			tsStruct.strExpireDate = AsString((long)dtExpire.GetMonth()) + AsString((long)dtExpire.GetYear()).Right(2);
			tsStruct.nTransactionSequenceFlag = 1;

		}
		else {
			ThrowNxException("Error in CCreditCardProcessingSummaryDlg::ProcessTransaction, Could not find transaction.");
		}

		//now send the message to Paymentech
		if (! PaymentechUtils::AuthorizeTransaction(tsStruct, rsResp)) {
			//there must have been a communications error
			ThrowNxException("Communication Error processing transaction, please check your internet connection and try again");
			return FALSE;
		}
		else {

			if (!rsResp.bApproved) {

				//we had another sort of error
				ThrowNxException("Error processing transaction, Paymentech returned errorCode: %s, Message: %s", rsResp.strErrorCode, PaymentechUtils::MessageLookup(rsResp.strErrorCode,rsResp.strResponseMessage, PaymentechUtils::lcEnglish));
				return FALSE;
			}

			return TRUE;
		}
	}NxCatchAllCall("Error in CCreditCardProcessingSummaryDlg::ProcessTransaction", return FALSE);

}

void CCreditCardProcessingSummaryDlg::OnRequeryFinishedTransactionList(short nFlags) 
{
	try {

		//clean the generated list
		m_pGeneratedList->Clear();

		BOOL bTransInNoBatch = FALSE;
	
		//loop thorough the list, calculating the totals and coloring them appropriately
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pTransList->GetFirstRow();
		
		COleCurrency cyTotal, cyCRTotal, cyDBTotal, cyAETotal, cyEBTotal;
		long nNumTotal, nNumCR, nNumDB, nNumAE, nNumEB;
		nNumTotal = nNumCR = nNumDB = nNumAE = nNumEB = 0;

		while (pRow) {

			BOOL bIsProcessed = TRUE;
			BOOL bIsApproved = FALSE;
			
			if (! VarBool(pRow->GetValue(tlcProcessed))) {
				
				if (VarBool(pRow->GetValue(tlcManuallyApproved))) {
					pRow->PutBackColor(RGB(128,255,255));
				}
				else {
					//it's not processed, so it's gray
					pRow->PutBackColor(RGB(150,150,150));
					bIsProcessed = FALSE;
				}
			}
			else if (VarString(pRow->GetValue(tlcResult)).CompareNoCase("Approved") == 0) {

				//it was approved
				pRow->PutBackColor(RGB(128,255,128));
				bIsApproved = TRUE;
			}
			else {
				ASSERT(FALSE);
				pRow->PutBackColor(RGB(255,128,128));
			}
			
			long nTypeID = VarLong(pRow->GetValue(tlcCardType));
			COleCurrency cyTransAmount = VarCurrency(pRow->GetValue(tlcAmount));
			
			if (bIsProcessed && bIsApproved) {
			
				switch (nTypeID) {

					case pctCredit: //Credit Cards
					case pctVisa:
						cyCRTotal += cyTransAmount;
						nNumCR++;
					break;

					case pctUSDebit: //Debit Cards
					case pctInterac:
						cyDBTotal += cyTransAmount;
						nNumDB++;
					break;

					case pctAmex: // American Express
						cyAETotal += cyTransAmount;
						nNumAE++;
					break;

					default: //EBT
						ASSERT(FALSE);
						cyEBTotal += cyTransAmount;
						nNumEB++;
					break;
				}

				cyTotal += cyTransAmount;
				nNumTotal++;
			}

			CString strBatchNumber = VarString(pRow->GetValue(tlcBatchNumber), "");
			strBatchNumber.TrimLeft();
			strBatchNumber.TrimRight();
			if (strBatchNumber.IsEmpty()) {
				bTransInNoBatch = TRUE;
				pRow->PutBackColor(RGB(255,255,179));
			}


			pRow = pRow->GetNextRow();
		}

		//now add the columns to the datalist
		if (nNumCR > 0) {
			pRow = m_pGeneratedList->GetNewRow();
			pRow->PutValue(0, (long)1);
			pRow->PutValue(1, _variant_t("Credit Cards"));
			pRow->PutValue(2, (long)nNumCR);
			pRow->PutValue(3, _variant_t(cyCRTotal));
			m_pGeneratedList->AddRowAtEnd(pRow, NULL);
		}

		if (nNumDB > 0) {
			pRow = m_pGeneratedList->GetNewRow();
			pRow->PutValue(0, (long)2);
			pRow->PutValue(1, _variant_t("Debit Cards"));
			pRow->PutValue(2, (long)nNumDB);
			pRow->PutValue(3, _variant_t(cyDBTotal));
			m_pGeneratedList->AddRowAtEnd(pRow, NULL);
		}

		if (nNumAE > 0) {
			pRow = m_pGeneratedList->GetNewRow();
			pRow->PutValue(0, (long)3);
			pRow->PutValue(1, _variant_t("American Express"));
			pRow->PutValue(2, (long)nNumAE);
			pRow->PutValue(3, _variant_t(cyAETotal));
			m_pGeneratedList->AddRowAtEnd(pRow, NULL);
		}

		if (nNumEB > 0) {
			pRow = m_pGeneratedList->GetNewRow();
			pRow->PutValue(0, (long)4);
			pRow->PutValue(1, _variant_t("Electronic Benefits Cards"));
			pRow->PutValue(2, (long)nNumEB);
			pRow->PutValue(3, _variant_t(cyEBTotal));
			m_pGeneratedList->AddRowAtEnd(pRow, NULL);
		}

		//now the total line
		pRow = m_pGeneratedList->GetNewRow();
		
		if (pRow) {
			//make it red so it sticks out
			pRow->PutForeColor(RGB(255,0,0));
			
			pRow->PutValue(0, (long)5);
			pRow->PutValue(1, _variant_t("Totals:"));
			pRow->PutValue(2, (long)(nNumTotal));
			pRow->PutValue(3, _variant_t(cyTotal));
			m_pGeneratedList->AddRowAtEnd(pRow, NULL);
		}

		if (bTransInNoBatch) {
			//add them to the current batch
			MsgBox("You have transaction(s) that are not in a valid Paymentech batch.  This can happen when you've had an internet outage.\nThese transactions will be added to the current batch so that they can be processed");

			//first see if we already have a batch with the current batch number
			ADODB::_RecordsetPtr rsBatch = CreateRecordset("SELECT ID FROM TransactionBatchT WHERE BatchNumber = '%s'", m_strBatchID);
			if (rsBatch->eof) {
				ExecuteSql("UPDATE TransactionBatchT SET BatchNumber = '%s' WHERE BatchNumber = ''", m_strBatchID);
			}
			else {
					
				long nCurrentBatchID = AdoFldLong(rsBatch, "ID");
					
				//first update the transaction to the new batch ID
				ExecuteSql("UPDATE CreditTransactionsT SET BatchID = %li WHERE BatchID IN (SELECT ID FROM TransactionBatchT WHERE BatchNumber = '')", nCurrentBatchID);

				//now delete from the batch table
				ExecuteSql("DELETE FROM TransactionBatchT WHERE BatchNumber = ''");
			}


			m_pTransList->Requery();

		}


	}NxCatchAll("Error in CCreditCardProcessingSummaryDlg::OnRequeryFinishedTransactionList");
	
	
	
}

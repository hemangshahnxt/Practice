// RetrievePastDepositsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "RetrievePastDepositsDlg.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "AuditTrail.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;

// (a.walling 2010-01-21 16:43) - PLID 37026 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.


/////////////////////////////////////////////////////////////////////////////
// CRetrievePastDepositsDlg dialog


CRetrievePastDepositsDlg::CRetrievePastDepositsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CRetrievePastDepositsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRetrievePastDepositsDlg)
	//}}AFX_DATA_INIT
}


void CRetrievePastDepositsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRetrievePastDepositsDlg)
	DDX_Control(pDX, IDOK, m_btnClose);
	DDX_Control(pDX, IDC_BTN_RESTORE_PAYMENTS, m_btnRestorePayments);
	DDX_Control(pDX, IDC_BTN_RESTORE_DEPOSIT, m_btnRestoreDeposit);
	DDX_Control(pDX, IDC_RESTORE_PAYMENT_FROM_DATE, m_dtFrom);
	DDX_Control(pDX, IDC_RESTORE_PAYMENT_TO_DATE, m_dtTo);
	//}}AFX_DATA_MAP
}

//	ON_EVENT(CRetrievePastDepositsDlg, IDC_RESTORE_PAYMENT_FROM_DATE, 2 /* Change */, OnChangeRestorePaymentFromDate, VTS_NONE)
//	ON_EVENT(CRetrievePastDepositsDlg, IDC_RESTORE_PAYMENT_FROM_DATE, 3 /* CloseUp */, OnCloseUpRestorePaymentFromDate, VTS_NONE)
//	ON_EVENT(CRetrievePastDepositsDlg, IDC_RESTORE_PAYMENT_TO_DATE, 2 /* Change */, OnChangeRestorePaymentToDate, VTS_NONE)
//	ON_EVENT(CRetrievePastDepositsDlg, IDC_RESTORE_PAYMENT_TO_DATE, 3 /* CloseUp */, OnCloseUpRestorePaymentToDate, VTS_NONE)

BEGIN_MESSAGE_MAP(CRetrievePastDepositsDlg, CNxDialog)
	//{{AFX_MSG_MAP(CRetrievePastDepositsDlg)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_RESTORE_PAYMENT_FROM_DATE, OnChangeRestorePaymentFromDate)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_RESTORE_PAYMENT_TO_DATE, OnChangeRestorePaymentToDate)
	ON_NOTIFY(DTN_CLOSEUP, IDC_RESTORE_PAYMENT_FROM_DATE, OnCloseUpRestorePaymentFromDate)
	ON_NOTIFY(DTN_CLOSEUP, IDC_RESTORE_PAYMENT_TO_DATE, OnCloseUpRestorePaymentToDate)
	ON_BN_CLICKED(IDC_BTN_RESTORE_PAYMENTS, OnBtnRestorePayments)
	ON_BN_CLICKED(IDC_BTN_RESTORE_DEPOSIT, OnBtnRestoreDeposit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRetrievePastDepositsDlg message handlers

BOOL CRetrievePastDepositsDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	// (j.jones 2008-05-08 10:59) - PLID 29953 - added nxiconbuttons for modernization
	m_btnClose.AutoSet(NXB_CLOSE);
	m_btnRestorePayments.AutoSet(NXB_MODIFY);
	m_btnRestoreDeposit.AutoSet(NXB_MODIFY);

	// (j.jones 2009-08-03 10:47) - PLID 33036 - if the user does not have permission to restore, disable the buttons
	BOOL bCanRestore = (GetCurrentUserPermissions(bioBankingTab) & (sptDynamic1|sptDynamic1WithPass));
	m_btnRestorePayments.EnableWindow(bCanRestore);
	m_btnRestoreDeposit.EnableWindow(bCanRestore);

	COleDateTime dtFrom, dtTo;
	COleDateTime dtNow = COleDateTime::GetCurrentTime();
	COleDateTimeSpan dtSpan;
	dtSpan.SetDateTimeSpan(30,0,0,0);
	dtFrom = dtNow - dtSpan;
	m_dtFrom.SetValue(_variant_t(dtFrom));
	m_dtTo.SetValue(_variant_t(dtNow));

	dtSpan.SetDateTimeSpan(1,0,0,0);
	dtTo = dtNow + dtSpan;

	CString strWhere;
	// (r.gonet 2015-05-05 14:38) - PLID 65870 - Exclude Gift Certificate Refunds
	strWhere.Format("PayMethod NOT IN (4,10) AND PayMethod <> 0 AND DepositDate >= '%s' AND DepositDate < '%s'", FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
	
	m_DepositList = BindNxDataListCtrl(this,IDC_PAST_BATCH_LIST,GetRemoteData(),false);
	m_DepositList->WhereClause = _bstr_t(strWhere);
	m_DepositList->Requery();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CRetrievePastDepositsDlg::OnBtnRestorePayments() 
{
	try {

		if(m_DepositList->CurSel==-1)
			return;

		// (j.jones 2009-08-03 10:33) - PLID 33036 - added a permission
		if(!CheckCurrentUserPermissions(bioBankingTab, sptDynamic1)) {
			return;
		}
		
		long i = 0;
		long p = m_DepositList->GetFirstSelEnum();

		LPDISPATCH pDisp = NULL;

		CWaitCursor pWait;

		while (p)
		{	i++;
			m_DepositList->GetNextSelEnum(&p, &pDisp);

			IRowSettingsPtr pRow(pDisp);

			// (j.jones 2007-09-10 15:00) - PLID 27341 - restored deposits should show up in the UNSELECTED list
			// (j.jones 2008-05-09 15:22) - PLID 25338 - removed CurrentSelect from data, restoring will now refresh
			// the screen, and the filters will handle the restored payments accordingly

			long PaymentID = VarLong(pRow->GetValue(0));
			long Type = VarLong(pRow->GetValue(1));	//1 - Payment, 2 - Batch Payment
			if(Type == 1) {
				ExecuteParamSql("UPDATE PaymentsT SET Deposited = 0, DepositDate = NULL, DepositInputDate = NULL WHERE ID = {INT}",PaymentID);

				// (j.jones 2008-05-12 12:05) - PLID 30000 - any linked tips need restored as well,
				// a linked payment/tip is a credit tip on a credit payment, or a check tip on a check payment,
				// these types cannot be separated
				//TES 4/16/2015 - PLID 65614 - Include refund paymethods
				ExecuteParamSql("UPDATE PaymentTipsT SET Deposited = 0 "
					"WHERE PaymentID = {INT} AND PayMethod IN (2,3,8,9) "
					"AND PayMethod = (SELECT PayMethod FROM PaymentsT WHERE PaymentsT.ID = PaymentTipsT.PaymentID) ", PaymentID);
			}
			else if(Type == 2) {
				//also restore their children
				// (j.jones 2009-06-09 15:42) - PLID 34549 - filter out adjustments
				ExecuteParamSql("UPDATE PaymentsT SET Deposited = 0, DepositDate = NULL, DepositInputDate = NULL WHERE BatchPaymentID = {INT} AND PayMethod <> 0",PaymentID);
				ExecuteParamSql("UPDATE BatchPaymentsT SET Deposited = 0, DepositDate = NULL, DepositInputDate = NULL WHERE ID = {INT}",PaymentID);
			}
			else if(Type == 3) {
				ExecuteParamSql("UPDATE PaymentTipsT SET Deposited = 0 WHERE ID = {INT}", PaymentID);

				// (j.jones 2008-05-12 12:05) - PLID 30000 - any linked payments need restored as well,
				// a linked payment/tip is a credit tip on a credit payment, or a check tip on a check payment,
				// these types cannot be separated
				//TES 4/16/2015 - PLID 65614 - Include refund paymethods
				ExecuteParamSql("UPDATE PaymentsT SET Deposited = 0, DepositDate = NULL, DepositInputDate = NULL "
					"WHERE ID IN (SELECT PaymentID FROM PaymentTipsT WHERE ID = {INT} AND PayMethod IN (2,3,8,9) "
					"AND PayMethod = (SELECT PayMethod FROM PaymentsT WHERE PaymentsT.ID = PaymentTipsT.PaymentID)) ", PaymentID);
			}

			pDisp->Release();
		}

		//auditing
		long nAuditID = BeginNewAuditEvent();
		AuditEvent(-1, "", nAuditID, aeiDepositRestored, -1, "", "Restored Deposited Items", aepHigh, aetChanged);

		m_DepositList->Requery();

		// (j.jones 2007-03-13 08:58) - PLID 25118 - send a tablechecker to inform others that we restored
		CClient::RefreshTable(NetUtils::DepositedPayments);

		AfxMessageBox("The selected payments have been restored.");

	}NxCatchAll("Error restoring selected payments.");
}

void CRetrievePastDepositsDlg::OnBtnRestoreDeposit() 
{
	try {

		if(m_DepositList->CurSel==-1)
			return;

		// (j.jones 2009-08-03 10:33) - PLID 33036 - added a permission
		if(!CheckCurrentUserPermissions(bioBankingTab, sptDynamic1)) {
			return;
		}
		
		long i = 0;
		long p = m_DepositList->GetFirstSelEnum();

		LPDISPATCH pDisp = NULL;

		CWaitCursor pWait;

		while (p)
		{	i++;
			m_DepositList->GetNextSelEnum(&p, &pDisp);

			IRowSettingsPtr pRow(pDisp);

			long PaymentID = VarLong(pRow->GetValue(0));
			long Type = VarLong(pRow->GetValue(1));	//1 - Payment, 2 - Batch Payment

			// (j.jones 2007-09-10 15:00) - PLID 27341 - restored deposits should show up in the UNSELECTED list
			// (j.jones 2008-05-09 15:22) - PLID 25338 - removed CurrentSelect from data, restoring will now refresh
			// the screen, and the filters will handle the restored payments accordingly

			//assume a deposit is everything at the same time
			if(Type == 1) {
				//batch payments (and their children)
				// (j.jones 2009-06-09 15:42) - PLID 34549 - filter out adjustments
				ExecuteParamSql("UPDATE PaymentsT SET Deposited = 0, DepositDate = NULL, DepositInputDate = NULL WHERE PayMethod <> 0 AND BatchPaymentID IN (SELECT ID FROM BatchPaymentsT WHERE DepositInputDate IN (SELECT DepositInputDate FROM PaymentsT WHERE ID = {INT}))",PaymentID);
				ExecuteParamSql("UPDATE BatchPaymentsT SET Deposited = 0, DepositDate = NULL, DepositInputDate = NULL WHERE DepositInputDate IN (SELECT DepositInputDate FROM PaymentsT WHERE ID = {INT})",PaymentID);
				//tips
				ExecuteParamSql("UPDATE PaymentTipsT SET Deposited = 0 WHERE DepositInputDate IN (SELECT DepositInputDate FROM PaymentsT WHERE ID = {INT})", PaymentID);
				//regular payments
				ExecuteParamSql("UPDATE PaymentsT SET Deposited = 0, DepositDate = NULL, DepositInputDate = NULL WHERE DepositInputDate IN (SELECT DepositInputDate FROM PaymentsT WHERE ID = {INT})",PaymentID);

				// (j.jones 2008-05-12 12:05) - PLID 30000 - any linked tips need restored as well,
				// a linked payment/tip is a credit tip on a credit payment, or a check tip on a check payment,
				// these types cannot be separated
				//TES 4/16/2015 - PLID 65614 - Include refund paymethods
				ExecuteParamSql("UPDATE PaymentTipsT SET Deposited = 0 "
					"WHERE PayMethod IN (2,3,8,9) AND PayMethod = (SELECT PayMethod FROM PaymentsT WHERE PaymentsT.ID = PaymentTipsT.PaymentID) "
					"AND DepositInputDate IN (SELECT DepositInputDate FROM PaymentsT WHERE ID = {INT})", PaymentID);
			}
			else if(Type == 2) {
				//regular payments
				ExecuteParamSql("UPDATE PaymentsT SET Deposited = 0, DepositDate = NULL, DepositInputDate = NULL WHERE DepositInputDate IN (SELECT DepositInputDate FROM BatchPaymentsT WHERE ID = {INT})",PaymentID);
				//tips
				ExecuteParamSql("UPDATE PaymentTipsT SET Deposited = 0 WHERE DepositInputDate IN (SELECT DepositInputDate FROM BatchPaymentsT WHERE ID = {INT})", PaymentID);
				//batch payments (and their children)
				ExecuteParamSql("UPDATE PaymentsT SET Deposited = 0, DepositDate = NULL, DepositInputDate = NULL  WHERE BatchPaymentID IN (SELECT ID FROM BatchPaymentsT WHERE DepositInputDate IN (SELECT DepositInputDate FROM BatchPaymentsT WHERE ID = {INT}))",PaymentID);
				ExecuteParamSql("UPDATE BatchPaymentsT SET Deposited = 0, DepositDate = NULL, DepositInputDate = NULL WHERE DepositInputDate IN (SELECT DepositInputDate FROM BatchPaymentsT WHERE ID = {INT})",PaymentID);
			}
			else if(Type == 3) {
				//batch payments (and their children)
				// (j.jones 2009-06-09 15:42) - PLID 34549 - filter out adjustments
				ExecuteParamSql("UPDATE PaymentsT SET Deposited = 0, DepositDate = NULL, DepositInputDate = NULL WHERE PayMethod <> 0 AND BatchPaymentID IN (SELECT ID FROM BatchPaymentsT WHERE DepositInputDate IN (SELECT DepositInputDate FROM PaymentTipsT WHERE ID = {INT}))",PaymentID);
				ExecuteParamSql("UPDATE BatchPaymentsT SET Deposited = 0, DepositDate = NULL, DepositInputDate = NULL WHERE DepositInputDate IN (SELECT DepositInputDate FROM PaymentTipsT WHERE ID = {INT})",PaymentID);
				//regular payments
				ExecuteParamSql("UPDATE PaymentsT SET Deposited = 0, DepositDate = NULL, DepositInputDate = NULL WHERE DepositInputDate IN (SELECT DepositInputDate FROM PaymentTipsT WHERE ID = {INT})",PaymentID);
				//tips
				ExecuteParamSql("UPDATE PaymentTipsT SET Deposited = 0 WHERE DepositInputDate IN (SELECT DepositInputDate FROM PaymentTipsT WHERE ID = {INT})", PaymentID);

				// (j.jones 2008-05-12 12:05) - PLID 30000 - any linked payments need restored as well,
				// a linked payment/tip is a credit tip on a credit payment, or a check tip on a check payment,
				// these types cannot be separated
				//TES 4/16/2015 - PLID 65614 - Include refund paymethods
				ExecuteParamSql("UPDATE PaymentsT SET Deposited = 0, DepositDate = NULL, DepositInputDate = NULL "
					"WHERE ID IN (SELECT PaymentID FROM PaymentTipsT WHERE PayMethod IN (2,3,8,9) AND PayMethod = (SELECT PayMethod FROM PaymentsT WHERE PaymentsT.ID = PaymentTipsT.PaymentID) "
					"AND DepositInputDate IN (SELECT DepositInputDate FROM PaymentTipsT WHERE ID = {INT}))", PaymentID);
			}

			pDisp->Release();
		}

		//auditing
		long nAuditID = BeginNewAuditEvent();
		AuditEvent(-1, "", nAuditID, aeiDepositRestored, -1, "", "Restored Deposited Items", aepHigh, aetChanged);

		m_DepositList->Requery();

		// (j.jones 2007-03-13 08:58) - PLID 25118 - send a tablechecker to inform others that we restored
		CClient::RefreshTable(NetUtils::DepositedPayments);

		AfxMessageBox("The selected deposits have been restored.");

	}NxCatchAll("Error restoring selected deposits.");
}

BEGIN_EVENTSINK_MAP(CRetrievePastDepositsDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CRetrievePastDepositsDlg)
	ON_EVENT(CRetrievePastDepositsDlg, IDC_PAST_BATCH_LIST, 3 /* DblClickCell */, OnDblClickCellPastBatchList, VTS_I4 VTS_I2)
	ON_EVENT(CRetrievePastDepositsDlg, IDC_PAST_BATCH_LIST, 6 /* RButtonDown */, OnRButtonDownPastBatchList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CRetrievePastDepositsDlg::OnDblClickCellPastBatchList(long nRowIndex, short nColIndex) 
{
	
}

void CRetrievePastDepositsDlg::OnRButtonDownPastBatchList(long nRow, short nCol, long x, long y, long nFlags) 
{
	CMenu pMenu;

	if(nRow == -1)
		return;

	m_DepositList->CurSel = nRow;

	// (j.jones 2009-08-03 10:47) - PLID 33036 - if the user does not have permission to restore, disable the buttons
	BOOL bCanRestore = (GetCurrentUserPermissions(bioBankingTab) & (sptDynamic1|sptDynamic1WithPass));
	if(!bCanRestore) {
		return;
	}

	pMenu.CreatePopupMenu();
	pMenu.InsertMenu(0, MF_BYPOSITION, IDC_BTN_RESTORE_PAYMENTS, "Restore Selected Payment(s)");
	pMenu.InsertMenu(1, MF_BYPOSITION, IDC_BTN_RESTORE_DEPOSIT, "Restore Entire Selected Deposit");

	CPoint pt;
	GetCursorPos(&pt);
	pMenu.TrackPopupMenu(TPM_LEFTALIGN,pt.x, pt.y,this);
}

void CRetrievePastDepositsDlg::OnChangeRestorePaymentFromDate(NMHDR* pNMHDR, LRESULT* pResult)
{
	OnCloseUpRestorePaymentToDate(pNMHDR, pResult);

	*pResult = 0;
}

void CRetrievePastDepositsDlg::OnCloseUpRestorePaymentFromDate(NMHDR* pNMHDR, LRESULT* pResult)
{
	OnCloseUpRestorePaymentToDate(pNMHDR, pResult);

	*pResult = 0;
}

void CRetrievePastDepositsDlg::OnChangeRestorePaymentToDate(NMHDR* pNMHDR, LRESULT* pResult) 
{
	OnCloseUpRestorePaymentToDate(pNMHDR, pResult);

	*pResult = 0;
}

void CRetrievePastDepositsDlg::OnCloseUpRestorePaymentToDate(NMHDR* pNMHDR, LRESULT* pResult) 
{
	try {

		COleDateTime dtFrom, dtTo;
		dtFrom = m_dtFrom.GetValue();
		dtTo = m_dtTo.GetValue();

		/*
		if(dtFrom > dtTo) {
			AfxMessageBox("Your 'from' date is after your 'to' date.\n"
				"Please correct the date range.");
			//go ahead and requery, it will clear the list
		}
		*/

		COleDateTimeSpan dtSpan;
		dtSpan.SetDateTimeSpan(1,0,0,0);
		dtTo += dtSpan;

		CString strWhere;
		// (r.gonet 2015-05-05 14:38) - PLID 65870 - Exclude Gift Certificate Refunds
		strWhere.Format("PayMethod NOT IN (4,10) AND PayMethod <> 0 AND DepositDate >= '%s' AND DepositDate < '%s'", FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
		
		if(m_DepositList->IsRequerying()) {
			m_DepositList->WaitForRequery(dlPatienceLevelWaitIndefinitely);
		}

		m_DepositList->WhereClause = _bstr_t(strWhere);
		m_DepositList->Requery();

	}NxCatchAll("Error changing date filters.");

	*pResult = 0;
}

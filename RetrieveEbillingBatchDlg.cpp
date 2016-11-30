// RetrieveEbillingBatchDlg.cpp : implementation file
//

#include "stdafx.h"
#include "RetrieveEbillingBatchDlg.h"
#include "GlobalFinancialUtils.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;

#define COLUMN_ID			0
#define COLUMN_BILL_ID		1
#define COLUMN_INSURED_PARTY_ID	2
#define COLUMN_PAT_NAME		3
#define COLUMN_ID_FOR_INS	4
#define COLUMN_INSCO_NAME	5
#define COLUMN_BILL_DESC	6
#define COLUMN_DATE_SENT	7
#define COLUMN_SENT_BY		8
#define COLUMN_IS_BATCHED	9
#define COLUMN_COUNT_CHARGES_SENT	10
#define COLUMN_COUNT_CHARGES		11
#define COLUMN_CHARGES_SENT	12		

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CRetrieveEbillingBatchDlg dialog
// (a.walling 2008-05-28 14:01) - PLID 27591 - Use CDateTimePicker


CRetrieveEbillingBatchDlg::CRetrieveEbillingBatchDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CRetrieveEbillingBatchDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRetrieveEbillingBatchDlg)
		m_pEbillingTabPtr = NULL;
	//}}AFX_DATA_INIT
}


void CRetrieveEbillingBatchDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRetrieveEbillingBatchDlg)
	DDX_Control(pDX, IDOK, m_btnClose);
	DDX_Control(pDX, IDC_BTN_RESTORE_CLAIMS, m_btnRestoreClaims);
	DDX_Control(pDX, IDC_BTN_RESTORE_BATCH, m_btnRestoreBatch);
	DDX_Control(pDX, IDC_RESTORE_CLAIM_FROM_DATE, m_dtFrom);
	DDX_Control(pDX, IDC_RESTORE_CLAIM_TO_DATE, m_dtTo);
	//}}AFX_DATA_MAP
}

//	ON_EVENT(CRetrieveEbillingBatchDlg, IDC_RESTORE_CLAIM_FROM_DATE, 2 /* Change */, OnChangeRestoreClaimFromDate, VTS_NONE)
//	ON_EVENT(CRetrieveEbillingBatchDlg, IDC_RESTORE_CLAIM_FROM_DATE, 3 /* CloseUp */, OnCloseUpRestoreClaimFromDate, VTS_NONE)
//	ON_EVENT(CRetrieveEbillingBatchDlg, IDC_RESTORE_CLAIM_TO_DATE, 2 /* Change */, OnChangeRestoreClaimToDate, VTS_NONE)
//	ON_EVENT(CRetrieveEbillingBatchDlg, IDC_RESTORE_CLAIM_TO_DATE, 3 /* CloseUp */, OnCloseUpRestoreClaimToDate, VTS_NONE)

BEGIN_MESSAGE_MAP(CRetrieveEbillingBatchDlg, CNxDialog)
	//{{AFX_MSG_MAP(CRetrieveEbillingBatchDlg)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_RESTORE_CLAIM_FROM_DATE, OnChangeRestoreClaimFromDate)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_RESTORE_CLAIM_TO_DATE, OnChangeRestoreClaimToDate)
	ON_NOTIFY(DTN_CLOSEUP, IDC_RESTORE_CLAIM_FROM_DATE, OnCloseUpRestoreClaimFromDate)
	ON_NOTIFY(DTN_CLOSEUP, IDC_RESTORE_CLAIM_TO_DATE, OnCloseUpRestoreClaimToDate)
	ON_BN_CLICKED(IDC_BTN_RESTORE_CLAIMS, OnBtnRestoreClaims)
	ON_BN_CLICKED(IDC_BTN_RESTORE_BATCH, OnBtnRestoreBatch)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRetrieveEbillingBatchDlg message handlers

BOOL CRetrieveEbillingBatchDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	// (j.jones 2008-05-08 10:59) - PLID 29953 - added nxiconbuttons for modernization
	m_btnClose.AutoSet(NXB_CLOSE);
	m_btnRestoreClaims.AutoSet(NXB_MODIFY);
	m_btnRestoreBatch.AutoSet(NXB_MODIFY);

	COleDateTime dtFrom, dtTo;
	COleDateTime dtNow = COleDateTime::GetCurrentTime();
	COleDateTimeSpan dtSpan;
	dtSpan.SetDateTimeSpan(60,0,0,0);
	dtFrom = dtNow - dtSpan;
	m_dtFrom.SetValue(_variant_t(dtFrom));
	m_dtTo.SetValue(_variant_t(dtNow));

	dtSpan.SetDateTimeSpan(1,0,0,0);
	dtTo = dtNow + dtSpan;

	CString strWhere;
	strWhere.Format("Date >= '%s' AND Date < '%s'", FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
	
	m_BatchList = BindNxDataListCtrl(this,IDC_PAST_BATCH_LIST,GetRemoteData(),false);
	m_BatchList->WhereClause = _bstr_t(strWhere);
	m_BatchList->Requery();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CRetrieveEbillingBatchDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CRetrieveEbillingBatchDlg)
	ON_EVENT(CRetrieveEbillingBatchDlg, IDC_PAST_BATCH_LIST, 6 /* RButtonDown */, OnRButtonDownPastBatchList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CRetrieveEbillingBatchDlg, IDC_PAST_BATCH_LIST, 18 /* RequeryFinished */, OnRequeryFinishedPastBatchList, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CRetrieveEbillingBatchDlg::OnRButtonDownPastBatchList(long nRow, short nCol, long x, long y, long nFlags) 
{
	CMenu pMenu;

	if(nRow == -1)
		return;

	m_BatchList->CurSel = nRow;

	pMenu.CreatePopupMenu();
	pMenu.InsertMenu(0, MF_BYPOSITION, IDC_BTN_RESTORE_CLAIMS, "Restore Selected Claim(s)");
	pMenu.InsertMenu(1, MF_BYPOSITION, IDC_BTN_RESTORE_BATCH, "Restore Selected Batch");

	CPoint pt;
	GetCursorPos(&pt);
	pMenu.TrackPopupMenu(TPM_LEFTALIGN,pt.x, pt.y,this);	
}

void CRetrieveEbillingBatchDlg::OnBtnRestoreClaims() 
{
	try {

		if(m_BatchList->CurSel==-1)
			return;
		
		long i = 0;
		long p = m_BatchList->GetFirstSelEnum();

		LPDISPATCH pDisp = NULL;

		CWaitCursor pWait;

		BOOL bSkipped = FALSE;

		while (p)
		{	i++;
			m_BatchList->GetNextSelEnum(&p, &pDisp);

			IRowSettingsPtr pRow(pDisp);

			long nBillID = VarLong(pRow->GetValue(COLUMN_BILL_ID));

			// (j.jones 2008-02-11 16:51) - PLID 28847 - added option to disable batching/printing claims with 100% patient resp.
			// (but you can unbatch claims)
			// (j.jones 2010-10-21 14:48) - PLID 41051 - this function can warn why the claim can't be created,
			// but does not do so here, we will handle it ourselves
			if(!CanCreateInsuranceClaim(nBillID, TRUE)) {				
				_RecordsetPtr rs = CreateParamRecordset("SELECT BillsT.Date, Last + ', ' + First + ' ' + Middle AS PatName "
					"FROM BillsT INNER JOIN PersonT ON BillsT.PatientID = PersonT.ID "
					"WHERE BillsT.ID = {INT}", nBillID);
				if(!rs->eof) {
					CString str;
					str.Format("The %s claim for patient %s will not be batched either because no charges on the bill are batched "
						"or because your preference to disallow batching or printing claims with no insurance responsibility has been enabled.\n\n"
						"Batching will continue for the next claim in the list.",
						FormatDateTimeForInterface(AdoFldDateTime(rs, "Date"), NULL, dtoDate),
						AdoFldString(rs, "PatName"));
					int nRet = MessageBox(str, "Practice", MB_OKCANCEL|MB_ICONINFORMATION);
					if(nRet == IDOK) {
						bSkipped = TRUE;
						continue;
					}
					else {
						pDisp->Release();
						return;
					}					
				}
				rs->Close();
				pDisp->Release();
				continue;
			}

			//pass TRUE to skip the responsibility check, because we just did it here
			BatchBill(nBillID,2, TRUE);

			//theoretically, this should still be faster than requerying
			for(int i=0;i<m_BatchList->GetRowCount();i++) {
				//color each row of this claim (if it is in the list multiple times)
				if(VarLong(m_BatchList->GetValue(i,COLUMN_BILL_ID)) == nBillID)
					IRowSettingsPtr(m_BatchList->GetRow(i))->PutForeColor(RGB(127,127,127));
			}

			pDisp->Release();
		}

		m_pEbillingTabPtr->UpdateView();

		if(bSkipped) {
			AfxMessageBox("The selected claims have been re-batched. Some claims were skipped.");
		}
		else {
			AfxMessageBox("The selected claims have been re-batched.");
		}

	}NxCatchAll("Error restoring selected claims.");
}

void CRetrieveEbillingBatchDlg::OnBtnRestoreBatch() 
{
	try {

		if(m_BatchList->CurSel==-1)
			return;
		
		long i = 0;
		long p = m_BatchList->GetFirstSelEnum();

		LPDISPATCH pDisp = NULL;

		CWaitCursor pWait;

		BOOL bSkipped = FALSE;

		while (p)
		{	i++;
			m_BatchList->GetNextSelEnum(&p, &pDisp);

			IRowSettingsPtr pRow(pDisp);

			//JMJ - Yes, this likely loops way too much, but at the moment I can't think of a way
			//to do this that is accurate, until we re-do the interface to this dialog.

			//assume a batch is everything at the same time or within 5 seconds of the selected time
			_RecordsetPtr rs = CreateRecordset("SELECT BillID FROM ClaimHistoryT "
				"WHERE Date < DateAdd(second,5,(SELECT Date FROM ClaimHistoryT WHERE ID = %li)) "
				"AND Date > DateAdd(second,-5,(SELECT Date FROM ClaimHistoryT WHERE ID = %li))",
				VarLong(pRow->GetValue(COLUMN_ID)), VarLong(pRow->GetValue(COLUMN_ID)));
			while(!rs->eof) {

				// (j.jones 2008-02-11 16:51) - PLID 28847 - added option to disable batching/printing claims with 100% patient resp.
				// (but you can unbatch claims)
				long nBillID = AdoFldLong(rs, "BillID");
				// (j.jones 2010-10-21 14:48) - PLID 41051 - this function can warn why the claim can't be created,
				// but does not do so here, we will handle it ourselves
				if(!CanCreateInsuranceClaim(nBillID, TRUE)) {				
					_RecordsetPtr rsPat = CreateParamRecordset("SELECT BillsT.Date, Last + ', ' + First + ' ' + Middle AS PatName "
						"FROM BillsT INNER JOIN PersonT ON BillsT.PatientID = PersonT.ID "
						"WHERE BillsT.ID = {INT}", nBillID);
					if(!rsPat->eof) {
						CString str;
						str.Format("The %s claim for patient %s can not be batched either because no charges on the bill are batched "
							"or because your preference to disallow batching or printing claims with no insurance responsibility has been enabled.\n\n"
							"Batching will continue for the next claim in the list.",
							FormatDateTimeForInterface(AdoFldDateTime(rsPat, "Date"), NULL, dtoDate),
							AdoFldString(rsPat, "PatName"));						
						int nRet = MessageBox(str, "Practice", MB_OKCANCEL|MB_ICONINFORMATION);
						if(nRet == IDOK) {
							bSkipped = TRUE;
							continue;
						}
						else {
							pDisp->Release();
							return;
						}
					}
					rsPat->Close();
					rs->MoveNext();
					continue;
				}

				//pass TRUE to skip the responsibility check, because we just did it here
				BatchBill(nBillID,2,TRUE);
				rs->MoveNext();
			}
			rs->Close();

			pDisp->Release();
		}

		m_pEbillingTabPtr->UpdateView();

		int CurSel = m_BatchList->GetCurSel();
		int TopRow = m_BatchList->GetTopRowIndex();

		m_BatchList->Requery();		
		m_BatchList->WaitForRequery(dlPatienceLevelWaitIndefinitely);

		m_BatchList->CurSel = CurSel;
		m_BatchList->TopRowIndex = TopRow;

		if(bSkipped) {
			AfxMessageBox("The selected claims have been re-batched. Some claims were skipped.");
		}
		else {
			AfxMessageBox("The selected claims have been re-batched.");
		}

	}NxCatchAll("Error restoring selected batches.");
}

void CRetrieveEbillingBatchDlg::OnChangeRestoreClaimFromDate(NMHDR* pNMHDR, LRESULT* pResult)
{
	OnCloseUpRestoreClaimToDate(pNMHDR, pResult);

	*pResult = 0;
}

void CRetrieveEbillingBatchDlg::OnCloseUpRestoreClaimFromDate(NMHDR* pNMHDR, LRESULT* pResult)
{
	OnCloseUpRestoreClaimToDate(pNMHDR, pResult);

	*pResult = 0;
}

void CRetrieveEbillingBatchDlg::OnChangeRestoreClaimToDate(NMHDR* pNMHDR, LRESULT* pResult)
{
	OnCloseUpRestoreClaimToDate(pNMHDR, pResult);

	*pResult = 0;
}

void CRetrieveEbillingBatchDlg::OnCloseUpRestoreClaimToDate(NMHDR* pNMHDR, LRESULT* pResult)
{
	try {		

		COleDateTime dtFrom, dtTo;
		dtFrom = m_dtFrom.GetValue();
		dtTo = m_dtTo.GetValue();

		if(dtFrom > dtTo) {
			AfxMessageBox("Your 'from' date is after your 'to' date.\n"
				"Please correct the date range.");
			//go ahead and requery, it will clear the list
		}

		COleDateTimeSpan dtSpan;
		dtSpan.SetDateTimeSpan(1,0,0,0);
		dtTo += dtSpan;

		CString strWhere;
		strWhere.Format("Date >= '%s' AND Date < '%s'", FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
		
		m_BatchList->WhereClause = _bstr_t(strWhere);
		m_BatchList->Requery();

	}NxCatchAll("Error changing date filters.");

	*pResult = 0;
}

void CRetrieveEbillingBatchDlg::OnRequeryFinishedPastBatchList(short nFlags) 
{
	try {

		//in this function we will
		// - gray out any claims currently batched
		// - determine if we need to show the count of charges sent

		BOOL bShowChargesSent = FALSE;

		for(int i=0;i<m_BatchList->GetRowCount();i++) {
			IRowSettingsPtr pRow = m_BatchList->GetRow(i);

			//see if we need to gray out the row
			if(VarLong(pRow->GetValue(COLUMN_IS_BATCHED),0) == 1) {
				pRow->PutForeColor(RGB(127,127,127));
			}

			//calculate the charges sent
			if(VarLong(pRow->GetValue(COLUMN_COUNT_CHARGES_SENT),0) < VarLong(pRow->GetValue(COLUMN_COUNT_CHARGES),0)) {
				bShowChargesSent = TRUE;
			}
		}

		if(bShowChargesSent) {
			//if any charges sent are less than the total charges on the bill, show the charges sent column
			IColumnSettingsPtr pCol = m_BatchList->GetColumn(COLUMN_CHARGES_SENT);
			pCol->PutStoredWidth(75);
		}
		else {
			//otherwise we can hide it
			IColumnSettingsPtr pCol = m_BatchList->GetColumn(COLUMN_CHARGES_SENT);
			pCol->PutStoredWidth(0);
		}

	}NxCatchAll("Error colorizing list.");
}

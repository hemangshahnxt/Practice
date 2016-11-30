// QBDepositAccountsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "QBDepositAccountsDlg.h"

// (a.walling 2012-08-03 14:31) - PLID 51956 - Compiler limits exceeded with imported NxAccessor typelib - get quickbooks out of stdafx
// (a.walling 2014-04-30 15:19) - PLID 61989 - import a typelibrary rather than a dll
#import "QBFC3.tlb" no_namespace

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define COLUMN_PAYMENT_ID		0
#define COLUMN_SOURCE_ACCOUNT	1
#define COLUMN_PAYER_NAME		2
#define COLUMN_PAY_DESC			3
#define COLUMN_PAY_DATE			4
#define COLUMN_PAY_AMOUNT		5
#define COLUMN_BATCH_PAY		6

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CQBDepositAccountsDlg dialog

using namespace ADODB;


CQBDepositAccountsDlg::CQBDepositAccountsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CQBDepositAccountsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CQBDepositAccountsDlg)
		m_paryPaymentIDs = NULL;
		m_paryRefundIDs = NULL;	// (j.jones 2009-02-18 08:49) - PLID 33136
		m_paryBatchPaymentIDs = NULL;
		m_paryPaymentTipIDs = NULL;
	//}}AFX_DATA_INIT
}

CQBDepositAccountsDlg::~CQBDepositAccountsDlg()
{
	for(int i = m_parySourceAccounts.GetSize() - 1; i >= 0; i--) {
		QBSourceAccounts *pAcctInfo = (QBSourceAccounts*)m_parySourceAccounts.GetAt(i);
		if(pAcctInfo)
			delete pAcctInfo;
		m_parySourceAccounts.RemoveAt(i);
	}
}

void CQBDepositAccountsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CQBDepositAccountsDlg)
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CQBDepositAccountsDlg, CNxDialog)
	//{{AFX_MSG_MAP(CQBDepositAccountsDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CQBDepositAccountsDlg message handlers

BOOL CQBDepositAccountsDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	if(!m_paryPaymentIDs || !m_paryBatchPaymentIDs || !m_paryRefundIDs) {
		HandleException(NULL,"Error: no payment lists defined.");
	}

	if(qb == NULL)
		HandleException(NULL,"Error: QuickBooks link not connected.");

	m_SourceAccountList = BindNxDataListCtrl(this,IDC_SOURCE_ACCOUNT_LIST,GetRemoteData(),false);
	m_DestAccountCombo = BindNxDataListCtrl(this,IDC_DEST_ACCOUNT_COMBO,GetRemoteData(),false);

	try {

		// (j.jones 2008-05-08 10:42) - PLID 29953 - added nxiconbuttons for modernization
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnOK.AutoSet(NXB_OK);

		long nDefSourceAcctID = -1;
		CString strDefSourceAcct = GetRemotePropertyText("QBDefaultDepositSourceAcct","-1",0,"<None>",TRUE);
		CString strDefDestAcct = GetRemotePropertyText("QBDefaultDepositDestAcct","-1",0,"<None>",TRUE);

		//load up the destination combo

		{
			// Create the request manager
			IMsgSetRequestPtr pReqSet = QB_CreateMsgSetRequest(qb);
			pReqSet->Attributes->OnError = roeContinue;

			// Create and prepare the request for the list all bank accounts
			IAccountQueryPtr pAcctsReq = pReqSet->AppendAccountQueryRq();
			IAccountListFilterPtr pFilter = pAcctsReq->ORAccountListQuery->AccountListFilter;
			pFilter->ActiveStatus->SetValue(asActiveOnly);
			pFilter->AccountTypeList->Add(atBank);
			pFilter->AccountTypeList->Add(atOtherCurrentAsset);
			
			// Do the request, and get the result
			IResponsePtr pResp = qb->DoRequests(pReqSet)->ResponseList->GetAt(0);
			if (pResp->StatusCode == 0) {
				// The result was success, so parse the return value to get the list of accounts
				IAccountRetListPtr pAcctsRetList = pResp->Detail;
				long nAcctCount = pAcctsRetList->GetCount();
				for (long i=0; i<nAcctCount; i++) {
					IAccountRetPtr pAcctRet = pAcctsRetList->GetAt(i);

					IRowSettingsPtr pRow = m_DestAccountCombo->GetRow(-1);
					pRow->PutValue(0,_variant_t((LPCTSTR)pAcctRet->ListID->GetValue()));
					pRow->PutValue(1,_variant_t((LPCTSTR)pAcctRet->Name->GetValue()));
					m_DestAccountCombo->AddRow(pRow);
				}
			}
		}

		if(strDefDestAcct != "-1")
			m_DestAccountCombo->SetSelByColumn(0,_bstr_t(strDefDestAcct));

		//load up the source combo text

		CString strComboSource;

		{
			// Create the request manager for the "from" accounts
			IMsgSetRequestPtr pReqSet = QB_CreateMsgSetRequest(qb);
			pReqSet->Attributes->OnError = roeContinue;

			// Create and prepare the request for the list all bank accounts
			IAccountQueryPtr pAcctsReq = pReqSet->AppendAccountQueryRq();
			IAccountListFilterPtr pFilter = pAcctsReq->ORAccountListQuery->AccountListFilter;
			pFilter->ActiveStatus->SetValue(asActiveOnly);

			IResponsePtr pResp = qb->DoRequests(pReqSet)->ResponseList->GetAt(0);
			if (pResp->StatusCode == 0) {
				// The result was success, so parse the return value to get the list of accounts
				IAccountRetListPtr pAcctsRetList = pResp->Detail;
				long nAcctCount = pAcctsRetList->GetCount();
				
				for (int i=0; i<nAcctCount; i++) {
					IAccountRetPtr pAcctRet = pAcctsRetList->GetAt(i);
					//cannot deposit FROM these accounts
					if((LPCTSTR)pAcctRet->Name->GetValue() != "Undeposited Funds" &&
						(LPCTSTR)pAcctRet->Name->GetValue() != "Accounts Receivable") {

						QBSourceAccounts *pNewAcct = new QBSourceAccounts;

						pNewAcct->ID = m_parySourceAccounts.GetSize()+1;
						pNewAcct->strAcctID = (LPCTSTR)pAcctRet->ListID->GetValue();
						pNewAcct->strAcctName = (LPCTSTR)pAcctRet->Name->GetValue();

						CString str;
						str.Format("%li;%s;",pNewAcct->ID,pNewAcct->strAcctName);
						strComboSource += str;

						if(strDefSourceAcct == pNewAcct->strAcctID)
							nDefSourceAcctID = pNewAcct->ID;

						m_parySourceAccounts.Add(pNewAcct);
					}
				}
			} else {
				// There weren't any accounts that matched the criteria, so we couldn't offer the user any to select
				AfxMessageBox("No QuickBooks accounts are available for addition of payments.");
			}
		}

		//set the combo source
		m_SourceAccountList->GetColumn(COLUMN_SOURCE_ACCOUNT)->ComboSource = _bstr_t(strComboSource);

		//load up payments
		for(int i=0; i<m_paryPaymentIDs->GetSize();i++) {

			QBDepositInfo *pPaymentInfo = (QBDepositInfo*)m_paryPaymentIDs->GetAt(i);

			_RecordsetPtr rs;
			//find out if they wish to include tips
			if(GetRemotePropertyInt("BankingIncludeTips", 0, 0, GetCurrentUserName(), true) == 0) {
				//no tips
				rs = CreateParamRecordset("SELECT Last + ', ' + First + ' ' + Middle AS PatName, Description, LineItemT.Date, Amount FROM LineItemT INNER JOIN PersonT ON LineItemT.PatientID = PersonT.ID WHERE LineItemT.ID = {INT}",pPaymentInfo->nPaymentID);
			}
			else {
				//include tips
				rs = CreateParamRecordset("SELECT Last + ', ' + First + ' ' + Middle AS PatName, Description, LineItemT.Date, Amount + (SELECT CASE WHEN Sum(Amount) IS NULL THEN 0 ELSE Sum(Amount) END FROM PaymentTipsT WHERE PaymentTipsT.PaymentID = PaymentsT.ID AND PaymentTipsT.PayMethod = PaymentsT.PayMethod) AS Amount FROM LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID INNER JOIN PersonT ON LineItemT.PatientID = PersonT.ID WHERE LineItemT.ID = {INT}", pPaymentInfo->nPaymentID);
			}
			if(!rs->eof) {

				IRowSettingsPtr pRow = m_SourceAccountList->GetRow(-1);
				pRow->PutValue(COLUMN_PAYMENT_ID,(long)pPaymentInfo->nPaymentID);
				if(nDefSourceAcctID != -1)
					pRow->PutValue(COLUMN_SOURCE_ACCOUNT,(long)nDefSourceAcctID);
				pRow->PutValue(COLUMN_PAYER_NAME,_bstr_t(AdoFldString(rs, "PatName","")));
				pRow->PutValue(COLUMN_PAY_DESC,_bstr_t(AdoFldString(rs, "Description","")));
				pRow->PutValue(COLUMN_PAY_DATE,rs->Fields->Item["Date"]->Value);
				pRow->PutValue(COLUMN_PAY_AMOUNT,_variant_t(AdoFldCurrency(rs, "Amount")));
				pRow->PutValue(COLUMN_BATCH_PAY,(long)0);

				m_SourceAccountList->AddRow(pRow);
			}
			rs->Close();
		}

		// (j.jones 2009-02-18 08:49) - PLID 33136 - supported refumds
		for(i=0; i<m_paryRefundIDs->GetSize();i++) {

			QBDepositInfo *pRefundInfo = (QBDepositInfo*)m_paryRefundIDs->GetAt(i);

			_RecordsetPtr rs = CreateParamRecordset("SELECT Last + ', ' + First + ' ' + Middle AS PatName, Description, LineItemT.Date, Amount FROM LineItemT INNER JOIN PersonT ON LineItemT.PatientID = PersonT.ID WHERE LineItemT.ID = {INT}",pRefundInfo->nPaymentID);
			if(!rs->eof) {

				IRowSettingsPtr pRow = m_SourceAccountList->GetRow(-1);
				pRow->PutValue(COLUMN_PAYMENT_ID,(long)pRefundInfo->nPaymentID);
				if(nDefSourceAcctID != -1)
					pRow->PutValue(COLUMN_SOURCE_ACCOUNT,(long)nDefSourceAcctID);
				pRow->PutValue(COLUMN_PAYER_NAME,_bstr_t(AdoFldString(rs, "PatName","")));
				pRow->PutValue(COLUMN_PAY_DESC,_bstr_t(AdoFldString(rs, "Description","")));
				pRow->PutValue(COLUMN_PAY_DATE,rs->Fields->Item["Date"]->Value);
				pRow->PutValue(COLUMN_PAY_AMOUNT,_variant_t(AdoFldCurrency(rs, "Amount")));
				pRow->PutValue(COLUMN_BATCH_PAY,(long)0);

				m_SourceAccountList->AddRow(pRow);
			}
			rs->Close();
		}		

		//load up payment tips
		for(i=0; i<m_paryPaymentTipIDs->GetSize();i++) {

			QBDepositInfo *pPaymentTipInfo = (QBDepositInfo*)m_paryPaymentTipIDs->GetAt(i);

			_RecordsetPtr rs = CreateParamRecordset("SELECT Last + ', ' + First + ' ' + Middle AS PatName, Description, LineItemT.Date, PaymentTipst.Amount FROM LineItemT INNER JOIN PersonT ON LineItemT.PatientID = PersonT.ID INNER JOIN PaymentTipsT ON LineItemT.ID = PaymentTipsT.PaymentID WHERE PaymentTipsT.ID = {INT}", pPaymentTipInfo->nPaymentID);
			if(!rs->eof) {

				IRowSettingsPtr pRow = m_SourceAccountList->GetRow(-1);
				pRow->PutValue(COLUMN_PAYMENT_ID,(long)pPaymentTipInfo->nPaymentID);
				if(nDefSourceAcctID != -1)
					pRow->PutValue(COLUMN_SOURCE_ACCOUNT,(long)nDefSourceAcctID);
				pRow->PutValue(COLUMN_PAYER_NAME,_bstr_t(AdoFldString(rs, "PatName","")));
				pRow->PutValue(COLUMN_PAY_DESC,_bstr_t(AdoFldString(rs, "Description","")));
				pRow->PutValue(COLUMN_PAY_DATE,rs->Fields->Item["Date"]->Value);
				pRow->PutValue(COLUMN_PAY_AMOUNT,_variant_t(AdoFldCurrency(rs, "Amount")));
				pRow->PutValue(COLUMN_BATCH_PAY,(long)0);

				m_SourceAccountList->AddRow(pRow);
			}
			rs->Close();
		}

		//load up batch payments
		for(i=0; i<m_paryBatchPaymentIDs->GetSize();i++) {

			QBDepositInfo *pBatchPaymentInfo = (QBDepositInfo*)m_paryBatchPaymentIDs->GetAt(i);

			_RecordsetPtr rs = CreateParamRecordset("SELECT Name, Description, Date, Amount FROM BatchPaymentsT INNER JOIN InsuranceCoT ON BatchPaymentsT.InsuranceCoID = InsuranceCoT.PersonID WHERE ID = {INT}", pBatchPaymentInfo->nPaymentID);
			if(!rs->eof) {

				IRowSettingsPtr pRow = m_SourceAccountList->GetRow(-1);
				pRow->PutValue(COLUMN_PAYMENT_ID,(long)pBatchPaymentInfo->nPaymentID);
				if(nDefSourceAcctID != -1)
					pRow->PutValue(COLUMN_SOURCE_ACCOUNT,(long)nDefSourceAcctID);
				pRow->PutValue(COLUMN_PAYER_NAME,_bstr_t(AdoFldString(rs, "Name","")));
				pRow->PutValue(COLUMN_PAY_DESC,_bstr_t(AdoFldString(rs, "Description","")));
				pRow->PutValue(COLUMN_PAY_DATE,rs->Fields->Item["Date"]->Value);
				pRow->PutValue(COLUMN_PAY_AMOUNT,_variant_t(AdoFldCurrency(rs, "Amount")));
				pRow->PutValue(COLUMN_BATCH_PAY,(long)1);

				m_SourceAccountList->AddRow(pRow);
			}
			rs->Close();
		}

	}NxCatchAll("Error initializing dialog.");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CQBDepositAccountsDlg::OnOK() 
{
	try {

		//make sure that a source account has been chosen for each payment, and that a dest. account is chosen,
		//and that they are not the same account

		if(m_DestAccountCombo->CurSel == -1) {
			AfxMessageBox("You must choose a destination account before depositing.");
			return;
		}

		BOOL bIsSourceChosen = TRUE;
		BOOL bIsSourceSame = FALSE;
		for(int i=0;i<m_SourceAccountList->GetRowCount() && bIsSourceChosen;i++) {
			_variant_t var = m_SourceAccountList->GetValue(i,COLUMN_SOURCE_ACCOUNT);
			long AcctID = -1;
			if(var.vt == VT_BSTR) {
				AcctID = atoi(VarString(var,"1"));
				if(AcctID == -1)
					bIsSourceChosen = FALSE;
			}
			else if(var.vt == VT_I4) {
				AcctID = VarLong(var,1);
				if(AcctID == -1)
					bIsSourceChosen = FALSE;
			}
			else {
				bIsSourceChosen = FALSE;
			}

			if(bIsSourceChosen) {
				//now see if it matches the destination account
				CString strSource;

				for(int k=0;k<m_parySourceAccounts.GetSize();k++) {
					QBSourceAccounts *pAcctInfo = (QBSourceAccounts*)m_parySourceAccounts.GetAt(k);
					if(pAcctInfo->ID == AcctID) {
						strSource = pAcctInfo->strAcctID;
					}
				}

				if(VarString(m_DestAccountCombo->GetValue(m_DestAccountCombo->GetCurSel(),0),"-1") == strSource) {
					bIsSourceSame = TRUE;
				}
			}
		}

		if(!bIsSourceChosen) {
			AfxMessageBox("You must choose a source account for each payment before depositing.");
			return;
		}

		

		if(bIsSourceSame) {
			AfxMessageBox("At least one payment has the same source account as your destination account.\n"
				"You cannot send funds from and to the same account. Please choose a different source account for these payments.");
			return;
		}

		//now put the correct account number in each payment's record

		for(i=0;i<m_SourceAccountList->GetRowCount();i++) {

			long PayID = VarLong(m_SourceAccountList->GetValue(i,COLUMN_PAYMENT_ID),-1);
			long AcctID;
			_variant_t var = m_SourceAccountList->GetValue(i,COLUMN_SOURCE_ACCOUNT);
			if(var.vt == VT_BSTR) {
				AcctID = atoi(VarString(var,"1"));
			}
			else if(var.vt == VT_I4) {
				AcctID = VarLong(var,1);
			}
			long IsBatchPay = VarLong(m_SourceAccountList->GetValue(i,COLUMN_BATCH_PAY),0);

			if(IsBatchPay == 1) {
				//batch payment
				for(int j=0;j<m_paryBatchPaymentIDs->GetSize();j++) {
					QBDepositInfo *pBatchPaymentInfo = (QBDepositInfo*)m_paryBatchPaymentIDs->GetAt(j);
					if(pBatchPaymentInfo->nPaymentID == PayID) {
						//now get the account number
						for(int k=0;k<m_parySourceAccounts.GetSize();k++) {
							QBSourceAccounts *pAcctInfo = (QBSourceAccounts*)m_parySourceAccounts.GetAt(k);
							if(pAcctInfo->ID == AcctID) {
								pBatchPaymentInfo->strDepositFromAccountListID = pAcctInfo->strAcctID;
							}
						}
					}
				}
			}
			else {
				//regular payment
				for(int j=0;j<m_paryPaymentIDs->GetSize();j++) {
					QBDepositInfo *pPaymentInfo = (QBDepositInfo*)m_paryPaymentIDs->GetAt(j);
					if(pPaymentInfo->nPaymentID == PayID) {
						//now get the account number
						for(int k=0;k<m_parySourceAccounts.GetSize();k++) {
							QBSourceAccounts *pAcctInfo = (QBSourceAccounts*)m_parySourceAccounts.GetAt(k);
							if(pAcctInfo->ID == AcctID) {
								pPaymentInfo->strDepositFromAccountListID = pAcctInfo->strAcctID;
							}
						}
					}
				}

				// (j.jones 2009-02-18 08:49) - PLID 33136 - supported refumds
				for(int j=0;j<m_paryRefundIDs->GetSize();j++) {
					QBDepositInfo *pRefundInfo = (QBDepositInfo*)m_paryRefundIDs->GetAt(j);
					if(pRefundInfo->nPaymentID == PayID) {
						//now get the account number
						for(int k=0;k<m_parySourceAccounts.GetSize();k++) {
							QBSourceAccounts *pAcctInfo = (QBSourceAccounts*)m_parySourceAccounts.GetAt(k);
							if(pAcctInfo->ID == AcctID) {
								pRefundInfo->strDepositFromAccountListID = pAcctInfo->strAcctID;
							}
						}
					}
				}				
			}
		}

		m_strToAccount = VarString(m_DestAccountCombo->GetValue(m_DestAccountCombo->GetCurSel(),0),"-1");
			
		CNxDialog::OnOK();
	
	}NxCatchAll("Error processing account numbers.");
}

void CQBDepositAccountsDlg::OnCancel() 
{
	if(IDNO == MessageBox("You must select a source account for each payment before depositing.\n"
		"If you cancel, the payments will not be deposited.\n\n"
		"Are you sure you wish to cancel?","Practice",MB_ICONQUESTION|MB_YESNO)) {
		return;
	}
	
	CNxDialog::OnCancel();
}

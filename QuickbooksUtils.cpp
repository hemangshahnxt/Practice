// QuickbooksUtils.cpp

#include "stdafx.h"
#include "QuickbooksUtils.h"
#include "NxException.h"
#include "ChooseIdDlg.h"
#include "ChooseTwoQBAcctsDlg.h"
#include "InternationalUtils.h"
#include "RegUtils.h"

// (a.walling 2012-08-03 14:31) - PLID 51956 - Compiler limits exceeded with imported NxAccessor typelib - get quickbooks out of stdafx
// (a.walling 2014-04-30 15:19) - PLID 61989 - import a typelibrary rather than a dll
#import "QBFC3.tlb" no_namespace

BOOL g_bCanadianQuickbooks = FALSE;

using namespace ADODB;

void IterStrings(IBSTRListPtr pStrings)
{
	long nCount = pStrings->GetCount();
	for (long i=0; i<nCount; i++) {
		AfxMessageBox((LPCTSTR)pStrings->GetAt(i));
	}
}

CString _S(IQBStringTypePtr &val)
{
	if (val == NULL) return "(null)";
	return (LPCTSTR)val->GetValue();
}

CString _S(IQBENAccountTypeTypePtr &val)
{
	if (val == NULL) return "(null)";
	return (LPCTSTR)val->GetAsString();
}

CString _S(IQBAmountTypePtr &val)
{
	if (val == NULL) return "(null)";
	return (LPCTSTR)val->GetAsString();
}

CString _S(IQBENDetailAccountTypeTypePtr &val)
{
	if (val == NULL) return "(null)";
	return (LPCTSTR)val->GetAsString();
}

CString _S(IQBBoolTypePtr &val)
{
	if (val == NULL) return "(null)";
	return (val->GetValue() == VARIANT_FALSE) ? "False" : "True";
}

CString _S(IQBIDTypePtr &val)
{
	if (val == NULL) return "(null)";
	return (LPCTSTR)val->GetValue();
}

CString _S(IQBIntTypePtr &val)
{
	if (val == NULL) return "(null)";
	CString str;
	str.Format("%li", val->GetValue());
	return str;
}

CString _S(IQBDateTimeTypePtr &val)
{
	if (val == NULL) return "(null)";
	return COleDateTime(val->GetValue()).Format("%Y-%m-%d %H:%M:%S");
}

CString _S(IObjectTypePtr &val)
{
	if (val == NULL) return "(null)";
	return (LPCTSTR)val->GetAsString();
}

BOOL QB_ChooseIndividualAccount(IQBSessionManager *lpqbSessionManager, OUT CString &strID, CWnd *pParent, CWnd *pProgress /* = NULL*/, EQBAcctType qbatAccountType /* = qbatReceivePaymentsAccts*/)
{
	SAFE_SET_PROGRESS(pProgress, "Loading accounts...");

	IQBSessionManagerPtr qb(lpqbSessionManager);

	// Create the request manager
	IMsgSetRequestPtr pReqSet = QB_CreateMsgSetRequest(qb);
	pReqSet->Attributes->OnError = roeContinue;

	// Create and prepare the request for the list all bank accounts
	IAccountQueryPtr pAcctsReq = pReqSet->AppendAccountQueryRq();
	IAccountListFilterPtr pFilter = pAcctsReq->ORAccountListQuery->AccountListFilter;
	pFilter->ActiveStatus->SetValue(asActiveOnly);
	if(qbatAccountType == qbatReceivePaymentsAccts) {
		pFilter->AccountTypeList->Add(atBank);
		pFilter->AccountTypeList->Add(atOtherCurrentAsset);
	}
	else if(qbatAccountType == qbatBankOnly) {
		pFilter->AccountTypeList->Add(atBank);
	}
	else if(qbatAccountType == qbatAllTypes) {
		//nothing
	}
	
	// Do the request, and get the results
	IResponsePtr pResp = qb->DoRequests(pReqSet)->ResponseList->GetAt(0);
	if (pResp->StatusCode == 0) {
		// The result was success, so parse the return value to get the list of accounts
		IAccountRetListPtr pAcctsRetList = pResp->Detail;
		long nAcctCount = pAcctsRetList->GetCount();

		CChooseIdDlg dlg(pParent);
		for (long i=0; i<nAcctCount; i++) {
			IAccountRetPtr pAcctRet = pAcctsRetList->GetAt(i);
			dlg.m_strInIDs.Add(_S(pAcctRet->ListID));
			dlg.m_strInNames.Add(_S(pAcctRet->Name));
		}
		
		// Prompt the user to choose an account
		if (dlg.DoModal() == IDOK) {
			// Success, we have an account
			strID = dlg.m_strOutID;
			SAFE_SET_PROGRESS(pProgress, "Account selected.");
			return TRUE;
		} else {
			// Failure, the user canceled
			SAFE_SET_PROGRESS(pProgress, "No account selected.");
			return FALSE;
		}
	} else {
		// There weren't any accounts that matched the criteria, so we couldn't offer the user any to select
		pParent->MessageBox("No QuickBooks accounts are available for addition of payments.", NULL, MB_OK|MB_ICONEXCLAMATION);
		SAFE_SET_PROGRESS(pProgress, "No accounts found.");
		return FALSE;
	}
	//*/
}

BOOL QB_ChooseDepositAccounts(IQBSessionManager *lpqbSessionManager, OUT CString &strFromID, OUT CString &strToID, CWnd *pParent, CWnd *pProgress /* = NULL*/, BOOL bSetDefaults /* = FALSE*/)
{
	SAFE_SET_PROGRESS(pProgress, "Loading accounts...");

	IQBSessionManagerPtr qb(lpqbSessionManager);

	// Create the request manager for the "from" accounts
	IMsgSetRequestPtr pReqSet1 = QB_CreateMsgSetRequest(qb);
	pReqSet1->Attributes->OnError = roeContinue;

	// Create and prepare the request for the list all bank accounts
	IAccountQueryPtr pAcctsReq1 = pReqSet1->AppendAccountQueryRq();
	IAccountListFilterPtr pFilter1 = pAcctsReq1->ORAccountListQuery->AccountListFilter;
	pFilter1->ActiveStatus->SetValue(asActiveOnly);
	
	CChooseTwoQBAcctsDlg dlg(pParent);
	
	// Do the request, and get the results
	IResponsePtr pResp1 = qb->DoRequests(pReqSet1)->ResponseList->GetAt(0);
	if (pResp1->StatusCode == 0) {
		// The result was success, so parse the return value to get the list of accounts
		IAccountRetListPtr pAcctsRetList = pResp1->Detail;
		long nAcctCount = pAcctsRetList->GetCount();
		
		for (long i=0; i<nAcctCount; i++) {
			IAccountRetPtr pAcctRet = pAcctsRetList->GetAt(i);
			//cannot deposit FROM these accounts
			if(_S(pAcctRet->Name) != "Undeposited Funds" &&
				_S(pAcctRet->Name) != "Accounts Receivable") {
				dlg.m_strInFromIDs.Add(_S(pAcctRet->ListID));
				dlg.m_strInFromNames.Add(_S(pAcctRet->Name));
			}
		}
		
	} else {
		// There weren't any accounts that matched the criteria, so we couldn't offer the user any to select
		pParent->MessageBox("No QuickBooks accounts are available for addition of payments.", NULL, MB_OK|MB_ICONEXCLAMATION);
		SAFE_SET_PROGRESS(pProgress, "No accounts found.");
		return FALSE;
	}

	// Create the request manager for the "to" accounts
	IMsgSetRequestPtr pReqSet2 = QB_CreateMsgSetRequest(qb);
	pReqSet2->Attributes->OnError = roeContinue;

	// Create and prepare the request for the list all bank accounts
	IAccountQueryPtr pAcctsReq2 = pReqSet2->AppendAccountQueryRq();
	IAccountListFilterPtr pFilter2 = pAcctsReq2->ORAccountListQuery->AccountListFilter;
	pFilter2->ActiveStatus->SetValue(asActiveOnly);
	pFilter2->AccountTypeList->Add(atBank);
	
	// Do the request, and get the results
	IResponsePtr pResp2 = qb->DoRequests(pReqSet2)->ResponseList->GetAt(0);
	if (pResp2->StatusCode == 0) {
		// The result was success, so parse the return value to get the list of accounts
		IAccountRetListPtr pAcctsRetList = pResp2->Detail;
		long nAcctCount = pAcctsRetList->GetCount();
		
		for (long i=0; i<nAcctCount; i++) {
			IAccountRetPtr pAcctRet = pAcctsRetList->GetAt(i);
			dlg.m_strInToIDs.Add(_S(pAcctRet->ListID));
			dlg.m_strInToNames.Add(_S(pAcctRet->Name));
		}	
		
	} else {
		// There weren't any accounts that matched the criteria, so we couldn't offer the user any to select
		pParent->MessageBox("No QuickBooks accounts are available for addition of payments.", NULL, MB_OK|MB_ICONEXCLAMATION);
		SAFE_SET_PROGRESS(pProgress, "No accounts found.");
		return FALSE;
	}

	dlg.m_bSettingDefaults = bSetDefaults;

	if(bSetDefaults) {
		dlg.m_strFromAccountLabel = "Please choose a default source account for payments to come from.";
		dlg.m_strToAccountLabel = "Please choose a default bank account for payments to deposit to.";
	}

	// Prompt the user to choose an account
	if (dlg.DoModal() == IDOK) {
		// Success, we have an account
		strFromID = dlg.m_strFromOutID;
		strToID = dlg.m_strToOutID;
		SAFE_SET_PROGRESS(pProgress, "Accounts selected.");
		return TRUE;
	} else {
		// Failure, the user canceled
		SAFE_SET_PROGRESS(pProgress, "No accounts selected.");
		return FALSE;
	}

	return FALSE;
}


//void QB_PerformPendingRequests(IQBSessionManager *lpSessionManager, IMsgSetRequest *lpMsgSetRequest, OUT CString *pstrErrors /*= NULL*/)
/*
{
	IQBSessionManagerPtr pqb(lpSessionManager);
	IMsgSetRequestPtr pRequests(lpMsgSetRequest);

	// Perform the request
	IMsgSetResponsePtr pResponseSet = pqb->DoRequests(lpMsgSetRequest);
	IResponseListPtr pRespList = pResponseSet->ResponseList;
	
	// Loop through the responses (even though usually there's only one)
	long nCount = pRespList->GetCount();
	for (long i=0; i<nCount; i++) {
		// Get this repsonse
		IResponsePtr pResp = pRespList->GetAt(i);
		if (pResp->StatusCode == 0) {			
			// Success, keep looping
		} else {
			// This request failed
			CString strErr;
			strErr.Format("%s %li on request %li: %s", (LPCTSTR)pResp->GetStatusSeverity(), pResp->GetStatusCode(), i, (LPCTSTR)pResp->GetStatusMessage());
			if (pstrErrors) {
				(*pstrErrors) += strErr + "\n";
			}
			switch (pRequests->Attributes->GetOnError()) {
			case roeContinue:
				// Keep looping
				break;
			case roeRollback:
			case roeStop:
				// Throw an exception immediately and don't keep looping
				AfxThrowNxException("%s", strErr);
				return;
			}
		}
	}
}
*/

BOOL QB_CreateDeposit(IQBSessionManager *lpSessionManager, IN CPtrArray &aryPaymentIDs, IN CPtrArray &aryBatchPaymentIDs, IN CPtrArray &aryPaymentTipIDs, IN CPtrArray &aryRefundIDs, IN CPtrArray &aryBatchPaymentRefundIDs, const CString &strDepositToAccountListID, BOOL bExportPatients, COleDateTime dtDepositDate)
{
	IQBSessionManagerPtr qb(lpSessionManager);

	// Create the request set
	IMsgSetRequestPtr pReqSet = QB_CreateMsgSetRequest(qb);
	pReqSet->Attributes->OnError = roeStop;

	IDepositAddPtr pqbDeposit = pReqSet->AppendDepositAddRq();
	//TODO: Only allow Bank accounts
	pqbDeposit->DepositToAccountRef->ListID->SetValue((LPCTSTR)strDepositToAccountListID);
	pqbDeposit->Memo->SetValue("Deposited by NexTech Practice");
	pqbDeposit->TxnDate->SetValue((DATE)dtDepositDate);

	BOOL bGCWarning = FALSE; //have they been warned about GCs?
	
	//loop through all payments
	for(int i=0;i<aryPaymentIDs.GetSize();i++) {

		QBDepositInfo *pPaymentInfo = (QBDepositInfo*)aryPaymentIDs.GetAt(i);

		long nPaymentID = pPaymentInfo->nPaymentID;
		CString strCustomerListID = pPaymentInfo->strCustomerListID;
		CString strDepositFromAccountListID = pPaymentInfo->strDepositFromAccountListID;

		//get payment information

		// (e.lally 2007-07-09) PLID 25993 - Changed credit card name to use ID link to CC table
		// - Formatted query to be more readable
		// (j.jones 2008-05-12 11:26) - PLID 30007 - ensured that payments always included their tips in the total,
		// regardless of the "Show Tips" setting, and only if the payment and tip methods are the same, but not cash
		// (r.gonet 2015-05-05 14:38) - PLID 65870 - Include Gift Certificate Refunds
		// (r.gonet 2016-01-05 17:57) - PLID 67813 - Fixed a SQL syntax error.
		_RecordsetPtr rs = CreateParamRecordset(
				"SELECT LineItemT.PatientID, "
				"Convert(float, LineItemT.Amount + "
				"	(SELECT CASE WHEN Sum(Amount) IS NULL THEN 0 ELSE Sum(Amount) END "
				"		FROM PaymentTipsT "
				"		WHERE PaymentID = PaymentsT.ID AND PayMethod = PaymentsT.PayMethod AND PaymentTipsT.PayMethod <> 1)) AS Amount, "
				"LineItemT.Description, LineItemT.InputDate, LineItemT.Date, PaymentPlansT.CheckNo, "
				"PaymentsT.PayMethod, "
				"(CASE	WHEN (PaymentsT.PayMethod = 1 OR PaymentsT.PayMethod = 7) THEN 'Cash' "
				"		WHEN (PaymentsT.PayMethod = 2 OR PaymentsT.PayMethod = 8) THEN 'Check' "
				"		WHEN (PaymentsT.PayMethod = 3 OR PaymentsT.PayMethod = 9) THEN "
				"			(CASE	WHEN PaymentPlansT.CreditCardID Is NULL THEN '' "
				"					ELSE CreditCardNamesT.CardName END) "
				"		WHEN (PaymentsT.PayMethod IN (4,10)) THEN 'GC' ELSE '' END) "
				"AS PaymentMethod "
				"FROM LineItemT "
				"INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
				"INNER JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
				"LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
				"WHERE LineItemT.ID = {INT}",nPaymentID);

		if(rs->eof) {
			return FALSE;
		}

		if(!rs->eof) {

			CString strPayMethod = AdoFldString(rs, "PaymentMethod");

			// (j.jones 2004-06-10 17:06) - in theory we should have made it impossible
			// to deposit a gift certificate, and moreso impossible to send a GC to QB,
			// but incase we get this far, we need to skip the GC payment
			if(strPayMethod == "GC") {
				if(!bGCWarning) {
					MsgBox("Gift Certificate payments are not allowed in QuickBooks.\n"
						"However, the payment that purchased this Gift Certificate is allowed in QuickBooks.\n\n"
						"All Gift Certificate payments will be skipped.");
					bGCWarning = TRUE;
				}
				continue;
			}

			IDepositLineAddPtr pqbDepositLine = pqbDeposit->DepositLineAddList->Append();

			//account to deposit FROM
			//TODO: should we give the ability to select a different account per payment?
			pqbDepositLine->ORDepositLineAdd->DepositInfo->AccountRef->ListID->SetValue((LPCTSTR)strDepositFromAccountListID);
			
			//payment amount
			pqbDepositLine->ORDepositLineAdd->DepositInfo->Amount->SetValue(AdoFldDouble(rs, "Amount",0.0));			

			//check number
			if(strPayMethod == "Check")
				pqbDepositLine->ORDepositLineAdd->DepositInfo->CheckNumber->SetValue((LPCTSTR)AdoFldString(rs, "CheckNo",""));

			if(g_bCanadianQuickbooks && strPayMethod == "Check")
				strPayMethod = "Cheque";

			//payment method (Cash, Check (or Cheque), or Credit Card Name)
			pqbDepositLine->ORDepositLineAdd->DepositInfo->PaymentMethodRef->FullName->SetValue((LPCTSTR)strPayMethod);
			
			//TODO: if we ever support classes, this is where they go
			//pqbDepositLine->ORDepositLineAdd->DepositInfo->ClassRef->ListID->SetValue((LPCTSTR)"Class");
			
			//customer ID
			if(bExportPatients)
				pqbDepositLine->ORDepositLineAdd->DepositInfo->EntityRef->ListID->SetValue((LPCTSTR)strCustomerListID);
			
			//description
			CString strDescription = AdoFldString(rs, "Description","");

			// (j.jones 2005-02-02 09:34) - if they are not sending the patient, check their settings
			// to export the patient's name and/or ID
			if(GetRemotePropertyInt("QBDepositWithPatients",0,0,"<None>",TRUE) == 0) {
				
				//not exporting patients, but do they want to send any demographic information?
				
				BOOL bSendName = GetRemotePropertyInt("QuickBooksDescAppendPatientName",0,0,"<None>",TRUE) == 1;
				BOOL bSendID = GetRemotePropertyInt("QuickBooksDescAppendPatientID",0,0,"<None>",TRUE) == 1;

				if(bSendName || bSendID) {
					
					//they do want to send some demographic information, but on what payments?
					
					BOOL bCash = GetRemotePropertyInt("QuickBooksDescAppendOnCash",0,0,"<None>",TRUE) == 1;
					BOOL bCheck = GetRemotePropertyInt("QuickBooksDescAppendOnCheck",0,0,"<None>",TRUE) == 1;
					BOOL bCharge = GetRemotePropertyInt("QuickBooksDescAppendOnCredit",0,0,"<None>",TRUE) == 1;

					long nPayMethod = AdoFldLong(rs, "PayMethod",-1);

					if((bCash && (nPayMethod == 1 || nPayMethod == 7)) || (bCheck && (nPayMethod == 2 || nPayMethod == 8)) || (bCharge && (nPayMethod == 3 || nPayMethod == 9))) {
					
						//we need to append the patient name and/or ID
						
						_RecordsetPtr rsPat = CreateRecordset("SELECT Last + ', ' + First + ' ' + Middle AS PatName, UserDefinedID "
							"FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID WHERE ID = %li",AdoFldLong(rs, "PatientID",-1));

						if(!rsPat->eof) {
							CString strPatName = AdoFldString(rsPat, "PatName","");
							long nPatID = AdoFldLong(rsPat, "UserDefinedID",-1);
							CString strPatID;
							strPatID.Format("%li",nPatID);

							CString strAppend;

							strAppend.Format("(%s%s%s)",bSendName ? strPatName : "", (bSendName && bSendID) ? ", " : "", bSendID ? strPatID : "");

							if(strDescription == "")
								strDescription = strAppend;
							else
								strDescription = strDescription + " " + strAppend;
						}
						rsPat->Close();
					}
				}
			}

			pqbDepositLine->ORDepositLineAdd->DepositInfo->Memo->SetValue((LPCTSTR)strDescription);
		}
		rs->Close();
	}

	//loop through all payment tips
	for(i=0;i<aryPaymentTipIDs.GetSize();i++) {

		QBDepositInfo *pPaymentTipInfo = (QBDepositInfo*)aryPaymentTipIDs.GetAt(i);

		long nPaymentTipID = pPaymentTipInfo->nPaymentID;
		CString strCustomerListID = pPaymentTipInfo->strCustomerListID;
		CString strDepositFromAccountListID = pPaymentTipInfo->strDepositFromAccountListID;

		//get payment information
		// (e.lally 2007-07-09) PLID 25993 - Changed credit card name to use ID link to CC table
			// - Formatted query to be more readable
		_RecordsetPtr rs = CreateParamRecordset(
			"SELECT LineItemT.PatientID, "
			"Convert(float, PaymentTipsT.Amount) AS Amount, LineItemT.Description, LineItemT.InputDate, "
			"LineItemT.Date, PaymentPlansT.CheckNo, "
			"(CASE	WHEN PaymentTipsT.PayMethod = 1 THEN 'Cash' "
			"		WHEN PaymentTipsT.PayMethod = 2 THEN 'Check' "
			"		WHEN PaymentTipsT.PayMethod = 3 THEN "
			"			(CASE WHEN PaymentPlansT.CreditCardID Is NULL THEN '' "
			"				ELSE CreditCardNamesT.CardName END) "
			"		WHEN PaymentTipsT.PayMethod = 4 THEN 'GC' ELSE '' END) "
			"AS PaymentMethod "
			"FROM LineItemT "
			"INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			"INNER JOIN PaymentTipsT ON PaymentsT.ID = PaymentTipsT.PaymentID "
			"INNER JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
			"LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
			"WHERE PaymentTipsT.ID = {INT} AND PaymentTipsT.Amount <> 0 ", nPaymentTipID);

		if(rs->eof) {
			return FALSE;
		}

		if(!rs->eof) {

			CString strPayMethod = AdoFldString(rs, "PaymentMethod");

			// (j.jones 2004-06-10 17:06) - in theory we should have made it impossible
			// to deposit a gift certificate, and moreso impossible to send a GC to QB,
			// but incase we get this far, we need to skip the GC payment
			if(strPayMethod == "GC") {
				if(!bGCWarning) {
					MsgBox("Gift Certificate payments are not allowed in QuickBooks.\n"
						"However, the payment that purchased this Gift Certificate is allowed in QuickBooks.\n\n"
						"All Gift Certificate payments will be skipped.");
					bGCWarning = TRUE;
				}
				continue;
			}

			IDepositLineAddPtr pqbDepositLine = pqbDeposit->DepositLineAddList->Append();

			//account to deposit FROM
			//TODO: should we give the ability to select a different account per payment?
			pqbDepositLine->ORDepositLineAdd->DepositInfo->AccountRef->ListID->SetValue((LPCTSTR)strDepositFromAccountListID);
			
			//payment amount
			pqbDepositLine->ORDepositLineAdd->DepositInfo->Amount->SetValue(AdoFldDouble(rs, "Amount",0.0));			

			//check number
			if(strPayMethod == "Check")
				pqbDepositLine->ORDepositLineAdd->DepositInfo->CheckNumber->SetValue((LPCTSTR)AdoFldString(rs, "CheckNo",""));

			if(g_bCanadianQuickbooks && strPayMethod == "Check")
				strPayMethod = "Cheque";

			//payment method (Cash, Check (or Cheque), or Credit Card Name)
			pqbDepositLine->ORDepositLineAdd->DepositInfo->PaymentMethodRef->FullName->SetValue((LPCTSTR)strPayMethod);
			
			//TODO: if we ever support classes, this is where they go
			//pqbDepositLine->ORDepositLineAdd->DepositInfo->ClassRef->ListID->SetValue((LPCTSTR)"Class");
			
			//customer ID
			if(bExportPatients)
				pqbDepositLine->ORDepositLineAdd->DepositInfo->EntityRef->ListID->SetValue((LPCTSTR)strCustomerListID);
			
			//description
			pqbDepositLine->ORDepositLineAdd->DepositInfo->Memo->SetValue((LPCTSTR)AdoFldString(rs, "Description",""));

		}
		rs->Close();
	}

	//loop through all batch payments
	for(i=0;i<aryBatchPaymentIDs.GetSize();i++) {

		QBDepositInfo *pBatchPaymentInfo = (QBDepositInfo*)aryBatchPaymentIDs.GetAt(i);

		long nBatchPaymentID = pBatchPaymentInfo->nPaymentID;
		CString strDepositFromAccountListID = pBatchPaymentInfo->strDepositFromAccountListID;

		//get batch payment information
		_RecordsetPtr rs = CreateParamRecordset("SELECT CASE WHEN Type <> 1 THEN -(Convert(float, Amount)) ELSE Convert(float, Amount) END AS Amount, Description, InputDate, Date, CheckNo, "
		"'Check' AS PaymentMethod FROM BatchPaymentsT WHERE ID = {INT} AND Type <> 2 AND Amount <> 0", nBatchPaymentID);
		if(!rs->eof) {			

			IDepositLineAddPtr pqbDepositLine = pqbDeposit->DepositLineAddList->Append();

			//account to deposit FROM
			//TODO: should we give the ability to select a different account per payment?
			pqbDepositLine->ORDepositLineAdd->DepositInfo->AccountRef->ListID->SetValue((LPCTSTR)strDepositFromAccountListID);
			
			//payment amount
			pqbDepositLine->ORDepositLineAdd->DepositInfo->Amount->SetValue(AdoFldDouble(rs, "Amount",0.0));

			CString strPayMethod = AdoFldString(rs, "PaymentMethod");

			//check number
			if(strPayMethod == "Check")
				pqbDepositLine->ORDepositLineAdd->DepositInfo->CheckNumber->SetValue((LPCTSTR)AdoFldString(rs, "CheckNo",""));

			if(g_bCanadianQuickbooks && strPayMethod == "Check")
				strPayMethod = "Cheque";

			//payment method (Cash, Check (or Cheque), or Credit Card Name)
			pqbDepositLine->ORDepositLineAdd->DepositInfo->PaymentMethodRef->FullName->SetValue((LPCTSTR)strPayMethod);
			
			//TODO: if we ever support classes, this is where they go
			//pqbDepositLine->ORDepositLineAdd->DepositInfo->ClassRef->ListID->SetValue((LPCTSTR)"Class");
						
			//description
			pqbDepositLine->ORDepositLineAdd->DepositInfo->Memo->SetValue((LPCTSTR)AdoFldString(rs, "Description",""));
		}
		else {
			return FALSE;
		}
		rs->Close();
	}

	//loop through all refunds
	for(i=0;i<aryRefundIDs.GetSize();i++) {

		QBDepositInfo *pRefundInfo = (QBDepositInfo*)aryRefundIDs.GetAt(i);

		long nPaymentID = pRefundInfo->nPaymentID;
		CString strCustomerListID = pRefundInfo->strCustomerListID;
		CString strDepositFromAccountListID = pRefundInfo->strDepositFromAccountListID;

		//get refund information
		
		// (e.lally 2007-07-09) PLID 25993 - Changed credit card name to use ID link to CC table
		// - Formatted query to be more readable
		// (j.jones 2008-05-12 11:26) - PLID 30007 - refunds should not have tips!
		// however, since the tip code was already here, I ensured that refunds behaved like payments,
		// in that they always included their tips in the total, regardless of the "Show Tips" setting,
		// and only if the payment and tip methods are the same, but not cash
		// (r.gonet 2015-05-05 14:38) - PLID 65870 - Include Gift Certificate Refunds
		_RecordsetPtr rs = CreateParamRecordset(
				"SELECT LineItemT.PatientID, "
				"Convert(float, LineItemT.Amount + "
				"	(SELECT CASE WHEN Sum(Amount) IS NULL THEN 0 ELSE Sum(Amount) END "
				"		FROM PaymentTipsT "
				"		WHERE PaymentID = PaymentsT.ID AND PayMethod = PaymentsT.PayMethod AND PaymentTipsT.PayMethod <> 1)) AS Amount, "
				"LineItemT.Description, LineItemT.InputDate, LineItemT.Date, PaymentPlansT.CheckNo, "
				"(CASE	WHEN (PaymentsT.PayMethod = 1 OR PaymentsT.PayMethod = 7) THEN 'Cash' "
				"		WHEN (PaymentsT.PayMethod = 2 OR PaymentsT.PayMethod = 8) THEN 'Check' "
				"		WHEN (PaymentsT.PayMethod = 3 OR PaymentsT.PayMethod = 9) THEN "
				"			(CASE WHEN PaymentPlansT.CreditCardID Is NULL THEN '' "
				"				ELSE CreditCardNamesT.CardName END) "
				"		WHEN (PaymentsT.PayMethod IN (4,10)) THEN 'GC' ELSE '' END) "
				"AS PaymentMethod "
				"FROM LineItemT "
				"INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
				"INNER JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
				"LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
				"WHERE LineItemT.ID = {INT} ",nPaymentID);

		if(rs->eof) {
			return FALSE;
		}

		if(!rs->eof) {

			CString strPayMethod = AdoFldString(rs, "PaymentMethod");

			// (j.jones 2004-06-10 17:06) - in theory we should have made it impossible
			// to deposit a gift certificate, and moreso impossible to send a GC to QB,
			// but incase we get this far, we need to skip the GC payment
			if(strPayMethod == "GC") {
				if(!bGCWarning) {
					MsgBox("Gift Certificate payments are not allowed in QuickBooks.\n"
						"However, the payment that purchased this Gift Certificate is allowed in QuickBooks.\n\n"
						"All Gift Certificate payments will be skipped.");
					bGCWarning = TRUE;
				}
				continue;
			}

			IDepositLineAddPtr pqbDepositLine = pqbDeposit->DepositLineAddList->Append();

			//account to deposit FROM
			//TODO: should we give the ability to select a different account per payment?
			pqbDepositLine->ORDepositLineAdd->DepositInfo->AccountRef->ListID->SetValue((LPCTSTR)strDepositFromAccountListID);
			
			//payment amount
			pqbDepositLine->ORDepositLineAdd->DepositInfo->Amount->SetValue(AdoFldDouble(rs, "Amount",0.0));			

			//check number
			if(strPayMethod == "Check")
				pqbDepositLine->ORDepositLineAdd->DepositInfo->CheckNumber->SetValue((LPCTSTR)AdoFldString(rs, "CheckNo",""));

			if(g_bCanadianQuickbooks && strPayMethod == "Check")
				strPayMethod = "Cheque";

			//payment method (Cash, Check (or Cheque), or Credit Card Name)
			pqbDepositLine->ORDepositLineAdd->DepositInfo->PaymentMethodRef->FullName->SetValue((LPCTSTR)strPayMethod);
			
			//TODO: if we ever support classes, this is where they go
			//pqbDepositLine->ORDepositLineAdd->DepositInfo->ClassRef->ListID->SetValue((LPCTSTR)"Class");
			
			//customer ID
			if(bExportPatients)
				pqbDepositLine->ORDepositLineAdd->DepositInfo->EntityRef->ListID->SetValue((LPCTSTR)strCustomerListID);
			
			//description
			pqbDepositLine->ORDepositLineAdd->DepositInfo->Memo->SetValue((LPCTSTR)AdoFldString(rs, "Description",""));
		}
		rs->Close();
	}

	//loop through all batch payment refunds
	for(i=0;i<aryBatchPaymentRefundIDs.GetSize();i++) {

		QBDepositInfo *pBatchPaymentInfo = (QBDepositInfo*)aryBatchPaymentRefundIDs.GetAt(i);

		long nBatchPaymentID = pBatchPaymentInfo->nPaymentID;
		CString strDepositFromAccountListID = pBatchPaymentInfo->strDepositFromAccountListID;

		//get batch payment information
		_RecordsetPtr rs = CreateParamRecordset("SELECT CASE WHEN Type <> 1 THEN -(Convert(float, Amount)) ELSE Convert(float, Amount) END AS Amount, Description, InputDate, Date, CheckNo, "
			"'Check' AS PaymentMethod FROM BatchPaymentsT WHERE ID = {INT} AND Type <> 2 AND Amount <> 0",nBatchPaymentID);
		if(!rs->eof) {			

			IDepositLineAddPtr pqbDepositLine = pqbDeposit->DepositLineAddList->Append();

			//account to deposit FROM
			//TODO: should we give the ability to select a different account per payment?
			pqbDepositLine->ORDepositLineAdd->DepositInfo->AccountRef->ListID->SetValue((LPCTSTR)strDepositFromAccountListID);
			
			//payment amount
			pqbDepositLine->ORDepositLineAdd->DepositInfo->Amount->SetValue(AdoFldDouble(rs, "Amount",0.0));

			CString strPayMethod = AdoFldString(rs, "PaymentMethod");

			//check number
			if(strPayMethod == "Check")
				pqbDepositLine->ORDepositLineAdd->DepositInfo->CheckNumber->SetValue((LPCTSTR)AdoFldString(rs, "CheckNo",""));

			if(g_bCanadianQuickbooks && strPayMethod == "Check")
				strPayMethod = "Cheque";

			//payment method (Cash, Check (or Cheque), or Credit Card Name)
			pqbDepositLine->ORDepositLineAdd->DepositInfo->PaymentMethodRef->FullName->SetValue((LPCTSTR)strPayMethod);
			
			//TODO: if we ever support classes, this is where they go
			//pqbDepositLine->ORDepositLineAdd->DepositInfo->ClassRef->ListID->SetValue((LPCTSTR)"Class");
						
			//description
			pqbDepositLine->ORDepositLineAdd->DepositInfo->Memo->SetValue((LPCTSTR)AdoFldString(rs, "Description",""));
		}
		else {
			return FALSE;
		}
		rs->Close();
	}

	// Do the request, and get the result
	IResponsePtr pResp = qb->DoRequests(pReqSet)->ResponseList->GetAt(0);
	if (pResp->StatusCode == 0) {
		// The result was success, so return TRUE
		IDepositRetPtr pDepositRet = pResp->Detail;
		/*This code is what you would think you could use to assign the payment ID to each
		//deposited payment, but QB doesn't return them. I think it only makes payment
		//IDs if you do Receive Payments. That means that unless we have a "QBDeposited"
		//field in addition to our existing "Deposited" field, we can't track when they 
		//sent to QB. (We could, however, put garbage numbers in PaymentsT.QuickBooksID.)
		//But, maybe they want to delete a deposit in QB and re-send?

		for(int i=0;i<pDepositRet->DepositLineRetList->GetCount();i++) {
			CString strPaymentTxnID = _S(pDepositRet->DepositLineRetList->GetAt(i)->TxnID);
		}
		*/
		return TRUE;
	} else {		
		// Couldn't create, throw the exception
		if(pResp->GetStatusCode() == 3140) {
			//For reasons as yet unknown, GetStatusCode - 3140 means many, many things,
			//including invalid payment method and invalid accounts. So check to see if
			//it is the PaymentMethod error and report accordingly, else just give the
			//QB error.
			CString str;
			CString strError = (LPCTSTR)pResp->GetStatusMessage();
			if(strError.Find("invalid reference to QuickBooks PaymentMethod") != -1) {
				str.Format("You are exporting a payment with an invalid 'Payment Method'.\n"
				"This likely happens if the payment is a Credit Card Payment and has either no Card Type selected, or a type that does not exist in QuickBooks.\n"
				"Please ensure that all Credit Card payments sent to QuickBooks have a valid Card Type that exists in QuickBooks.\n\n"
				"QuickBooks Warning:\n%s",(LPCTSTR)pResp->GetStatusMessage());
			}
			else {
				str.Format("The deposit has been cancelled, due to the following warning:\n\n"
				"QuickBooks Warning:\n%s",(LPCTSTR)pResp->GetStatusMessage());
			}
			AfxMessageBox(str);
		}
		else if(pResp->GetStatusCode() == 3210) {
			CString str;
			str.Format("You are exporting a payment with negative amount, which is not allowed in QuickBooks.\n"
			"Please ensure that all payments sent to QuickBooks have a positive amount.\n\n"
			"QuickBooks Warning:\n%s",(LPCTSTR)pResp->GetStatusMessage());
			AfxMessageBox(str);
		}
		else if(pResp->GetStatusCode() == 3180) {
			CString str;
			str.Format("If you are exporting from an Accounts Receivable account, you must export patients as well.\n\n"
			"QuickBooks Warning:\n%s",(LPCTSTR)pResp->GetStatusMessage());
			AfxMessageBox(str);
		}
		else if(pResp->GetStatusCode() == 3040) {
			CString str;
			str.Format("You are exporting a deposit amount that is less than %s. QuickBooks does not allow the total of a deposit to be negative.\n"
				"Negative amounts (such as refunds) are only allowed to be sent to QuickBooks if the total deposit is not negative.\n\n"
			"QuickBooks Warning:\n%s",FormatCurrencyForInterface(COleCurrency(0,0),TRUE,TRUE),(LPCTSTR)pResp->GetStatusMessage());
			AfxMessageBox(str);
		}
		else
			AfxThrowNxException("%s %li on QB_CreateDeposit request: %s", (LPCTSTR)pResp->GetStatusSeverity(), pResp->GetStatusCode(), (LPCTSTR)pResp->GetStatusMessage());
		return FALSE;
	}

	return FALSE;
}

BOOL QB_CreatePayment(IQBSessionManager *lpSessionManager, const CString &strCustomerListID, OUT CString &strPaymentTxnID, IN const long nPaymentID, BOOL bIsAutoApply, const CString &strDepositToAccountListID, BOOL bIsTip, COleDateTime dtDeposit /*= COleDateTime::GetCurrentTime()*/)
{
	IQBSessionManagerPtr qb(lpSessionManager);
 
	// Set the payment request fields
	_RecordsetPtr rs;
	//find out if they wish to include tips
	if(GetRemotePropertyInt("BankingIncludeTips", 0, 0, GetCurrentUserName(), true) == 0) {
		//no tips
		// (e.lally 2007-07-09) PLID 25993 - Changed credit card name to use ID link to CC table
			// - Formatted query to be more readable
		rs = CreateRecordset("SELECT Convert(float, LineItemT.Amount) AS Amount, LineItemT.Description, "
			"LineItemT.InputDate, LineItemT.Date, "
			"(CASE	WHEN PaymentsT.PayMethod = 1 THEN 'Cash' "
			"		WHEN PaymentsT.PayMethod = 2 THEN 'Check' "
			"		WHEN PaymentsT.PayMethod = 3 THEN "
			"			(CASE WHEN PaymentPlansT.CreditCardID Is NULL THEN '' "
			"				ELSE CreditCardNamesT.CardName END) "
			"		WHEN PaymentsT.Paymethod = 4 THEN 'GC' ELSE '' END) "
			"AS PaymentMethod "
			"FROM LineItemT "
			"INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			"INNER JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
			"LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
			"WHERE LineItemT.ID = %li AND Amount <> 0",nPaymentID);
	}
	else {
		//include tips
		// (e.lally 2007-07-09) PLID 25993 - Changed credit card name to use ID link to CC table
			// - Formatted query to be more readable
		rs = CreateRecordset(
			"SELECT Convert(float, LineItemT.Amount + "
			"	(SELECT CASE WHEN Sum(Amount) IS NULL THEN 0 ELSE Sum(Amount) END "
			"		FROM PaymentTipsT "
			"		WHERE PaymentID = PaymentsT.ID AND PayMethod = PaymentsT.PayMethod)) AS Amount, "
			"LineItemT.Description, LineItemT.InputDate, LineItemT.Date, "
			"(CASE	WHEN PaymentsT.PayMethod = 1 THEN 'Cash' "
			"		WHEN PaymentsT.PayMethod = 2 THEN 'Check' "
			"		WHEN PaymentsT.PayMethod = 3 THEN "
			"			(CASE WHEN PaymentPlansT.CreditCardID Is NULL THEN '' "
			"				ELSE CreditCardNamesT.CardName END) "
			"		WHEN PaymentsT.PayMethod = 4 THEN 'GC' ELSE '' END) AS PaymentMethod "
			"FROM LineItemT "
			"INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			"INNER JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
			"LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
			"WHERE LineItemT.ID = %li AND Amount <> 0 ", nPaymentID);
	}

	if(bIsTip) {
		//if this is set, then they are intending to send just the tip over, and nPaymentID is in fact the tip ID
		// (e.lally 2007-07-09) PLID 25993 - Changed credit card name to use ID link to CC table
			// - Formatted query to be more readable
		rs = CreateRecordset(
			"SELECT Convert(float, PaymentTipsT.Amount) AS Amount, LineItemT.Description, "
			"LineItemT.InputDate, LineItemT.Date, "
			"(CASE	WHEN PaymentTipsT.PayMethod = 1 THEN 'Cash' "
			"		WHEN PaymentTipsT.PayMethod = 2 THEN 'Check' "
			"		WHEN PaymentTipsT.PayMethod = 3 THEN "
			"			(CASE WHEN PaymentPlansT.CreditCardID Is NULL THEN '' "
			"				ELSE CreditCardNamesT.CardName END) "
			"		WHEN PaymentTipsT.PayMethod = 4 THEN 'GC' ELSE '' END) AS PaymentMethod "
			"FROM PaymentTipsT  "
			"INNER JOIN LineItemT ON PaymentTipsT.PaymentID = LineItemT.ID "
			"LEFT JOIN PersonT ON LineItemT.PatientID = PersonT.ID "
			"INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			"LEFT JOIN ProvidersT ON PaymentTipsT.ProvID = ProvidersT.PersonID "
			"INNER JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
			"LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
			"WHERE PaymentTipsT.ID = %li AND PaymentTipsT.Amount <> 0",nPaymentID);
	}

	if(rs->eof)
		return FALSE;

	if(!rs->eof) {
		CString strPayMethod = AdoFldString(rs, "PaymentMethod");
		if(g_bCanadianQuickbooks && strPayMethod == "Check")
			strPayMethod = "Cheque";

		// (j.jones 2004-06-10 17:06) - in theory we should have made it impossible
		// to export a gift certificate a GC to QB,
		// but incase we get this far, we need to skip the GC payment
		if(strPayMethod == "GC") {
			MsgBox("Gift Certificate payments are not allowed in QuickBooks.\n"
				"However, the payment that purchased this Gift Certificate is allowed in QuickBooks.\n\n"
				"All Gift Certificate payments will be skipped.");
			return FALSE;
		}

		// Create the request set
		IMsgSetRequestPtr pReqSet = QB_CreateMsgSetRequest(qb);
		pReqSet->Attributes->OnError = roeStop;

		// Create the payment request
		IReceivePaymentAddPtr pqbPay = pReqSet->AppendReceivePaymentAddRq();
		pqbPay->CustomerRef->ListID->SetValue((LPCTSTR)strCustomerListID);
		pqbPay->ORApplyPayment->IsAutoApply->SetValue(bIsAutoApply ? VARIANT_TRUE : VARIANT_FALSE);

		pqbPay->PaymentMethodRef->FullName->SetValue((LPCTSTR)strPayMethod);
		pqbPay->Memo->SetValue((LPCTSTR)AdoFldString(rs, "Description",""));
		pqbPay->TotalAmount->SetValue(AdoFldDouble(rs, "Amount",0.0));
		pqbPay->DepositToAccountRef->ListID->SetValue((LPCTSTR)strDepositToAccountListID);

		//decide the Transaction Date style - default is the current date
		long QBTxnDate = GetRemotePropertyInt("QuickbooksTxnDate",3,0,"<None>",TRUE);
		if(QBTxnDate == 1)	//service date
			pqbPay->TxnDate->SetValue((DATE)AdoFldDateTime(rs, "Date",COleDateTime::GetCurrentTime()));
		else if(QBTxnDate == 2)	//input date
			pqbPay->TxnDate->SetValue((DATE)AdoFldDateTime(rs, "InputDate",COleDateTime::GetCurrentTime()));
		else if(QBTxnDate == 3) //transfer date
			pqbPay->TxnDate->SetValue((DATE)COleDateTime::GetCurrentTime());
		else if(QBTxnDate == 4) //deposit date (or transfer date if not in Banking tab)
			pqbPay->TxnDate->SetValue((DATE)dtDeposit);

		// Do the request, and get the result
		IResponsePtr pResp = qb->DoRequests(pReqSet)->ResponseList->GetAt(0);
		if (pResp->StatusCode == 0) {
			// The result was success, so set the list id of the new payment and return true
			IReceivePaymentRetPtr pPaymentRet = pResp->Detail;
			strPaymentTxnID = _S(pPaymentRet->TxnID);		
		} else {
			// Couldn't create, throw the exception
			if(pResp->GetStatusCode() == 3140) {
				CString str;
				str.Format("You are exporting a payment with an invalid 'Payment Method'.\n"
				"This likely happens if the payment is a Credit Card Payment and has either no Card Type selected, or a type that does not exist in QuickBooks.\n"
				"Please ensure that all Credit Card payments sent to QuickBooks have a valid Card Type that exists in QuickBooks.\n\n"
				"QuickBooks Warning:\n%s",(LPCTSTR)pResp->GetStatusMessage());
				AfxMessageBox(str);
			}
			else if(pResp->GetStatusCode() == 3210) {
				CString str;
				str.Format("You are exporting a payment with negative amount, which is not allowed in QuickBooks.\n"
				"Please ensure that all payments sent to QuickBooks have a positive amount.\n\n"
				"QuickBooks Warning:\n%s",(LPCTSTR)pResp->GetStatusMessage());
				AfxMessageBox(str);
			}
			else
				AfxThrowNxException("%s %li on QB_CreatePayment request: %s", (LPCTSTR)pResp->GetStatusSeverity(), pResp->GetStatusCode(), (LPCTSTR)pResp->GetStatusMessage());
			return FALSE;
		}
	}
	rs->Close();

	return TRUE;
}

BOOL QB_CreateCustomer(IQBSessionManager *lpqbSessionManager, IN const long &nCustomerPracticeID, OUT CString &strCustomerListID)
{
	IQBSessionManagerPtr qb(lpqbSessionManager);

	// Create the request manager
	IMsgSetRequestPtr pReqSet = QB_CreateMsgSetRequest(qb);
	pReqSet->Attributes->OnError = roeContinue;

	// Prepare the request
	ICustomerAddPtr pCustomerAddReq = pReqSet->AppendCustomerAddRq();

	//now set their demographics
	_RecordsetPtr rs = CreateRecordset("SELECT PersonT.*, PatientsT.* FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID WHERE ID = %li",nCustomerPracticeID);
	if(!rs->eof) {
		CString strLast, strFirst, strMiddle;
		CString strLFM, strFML;

		//if we truncate things, let's do it silently
		strLast = AdoFldString(rs, "Last","");
		if(strLast.GetLength() > 25) {
			strLast = strLast.Left(25);
		}
		pCustomerAddReq->LastName->SetValue((LPCTSTR)strLast);

		strFirst = AdoFldString(rs, "First","");
		if(strFirst.GetLength() > 25) {
			strFirst = strFirst.Left(25);
		}
		pCustomerAddReq->FirstName->SetValue((LPCTSTR)strFirst);

		strMiddle = AdoFldString(rs, "Middle","");
		if(strMiddle.GetLength() > 5) {
			strMiddle = strMiddle.Left(5);
		}
		pCustomerAddReq->MiddleName->SetValue((LPCTSTR)strMiddle);

		//for some unknown reason, "Name" must be set after the first/middle/last components
		strLFM.Format("%s, %s%s",strLast,strFirst,(strMiddle == "" ? "" : " " + strMiddle.Left(1)));
		strFML.Format("%s%s %s",strFirst,(strMiddle == "" ? "" : " " + strMiddle.Left(1)),strLast);

		if(strLFM.GetLength() > 40) {
			strLFM = strLFM.Left(40);
		}
		if(strFML.GetLength() > 40) {
			strFML = strFML.Left(40);
		}

		pCustomerAddReq->Name->SetValue((LPCTSTR)strLFM);
		pCustomerAddReq->Contact->SetValue((LPCTSTR)strFML);
		
		//Quickbooks always puts the FML name in the address and the contact when making a new patient, so we must follow suit
		pCustomerAddReq->BillAddress->Addr1->SetValue((LPCTSTR)strFML);

		CString str;
		str = AdoFldString(rs, "Address1","");
		if(str.GetLength() > 40) {
			str = str.Left(40);
		}
		pCustomerAddReq->BillAddress->Addr2->SetValue((LPCTSTR)str);

		str = AdoFldString(rs, "Address2","");
		if(str.GetLength() > 40) {
			str = str.Left(40);
		}
		pCustomerAddReq->BillAddress->Addr3->SetValue((LPCTSTR)str);

		str = AdoFldString(rs, "City","");
		if(str.GetLength() > 30) {
			str = str.Left(30);
		}
		pCustomerAddReq->BillAddress->City->SetValue((LPCTSTR)str);

		str = AdoFldString(rs, "State","");
		if(str.GetLength() > 2) {
			str = str.Left(2);
		}
		if(!g_bCanadianQuickbooks)
			pCustomerAddReq->BillAddress->State->SetValue((LPCTSTR)str);
		else
			pCustomerAddReq->BillAddress->Province->SetValue((LPCTSTR)str);

		str = AdoFldString(rs, "Zip","");
		if(str.GetLength() > 10) {
			str = str.Left(10);
		}
		pCustomerAddReq->BillAddress->PostalCode->SetValue((LPCTSTR)str);

		CString strUserDefinedID;
		strUserDefinedID.Format("%li",AdoFldLong(rs, "UserDefinedID"));
		pCustomerAddReq->AccountNumber->SetValue((LPCTSTR)strUserDefinedID);
		pCustomerAddReq->Email->SetValue((LPCTSTR)AdoFldString(rs, "Email",""));
		pCustomerAddReq->Phone->SetValue((LPCTSTR)AdoFldString(rs, "HomePhone",""));

		//if we ever choose to send the SocialSecurity number over, remember is is SIN in Canada,
		//so we will have to accomodate that using the g_bCanadianQuickbooks indicator
	}
	rs->Close();

	// Do the request, and get the result
	IResponsePtr pResp = qb->DoRequests(pReqSet)->ResponseList->GetAt(0);
	if (pResp->StatusCode == 0) {
		// The result was success, so set the list id of the new customer and return true
		ICustomerRetPtr pCustomerRet = pResp->Detail;
		strCustomerListID = _S(pCustomerRet->ListID);
		return TRUE;
	} else {
		// Couldn't create, throw the exception
		if(pResp->GetStatusCode() == 3100) {
			CString str;
			CString strError = (LPCTSTR)pResp->GetStatusMessage();
			if(strError.Find("list element is already in use") != -1) {
				str.Format("The name of the person you are exporting already exists in QuickBooks.\n\n"
				"This can often happen if there is an Employee of the same name in QuickBooks. \n"
				"QuickBooks does not allow two people, even Customers and Employees, to have the same name.\n\n"
				"Please update the name of either this patient or the similarly-named record in QuickBooks\n"
				"to be different from one another (ie. entering a middle initial).\n\n"
				"QuickBooks Warning:\n%s",(LPCTSTR)pResp->GetStatusMessage());
			}
			else {
				str.Format("The customer creation has been cancelled, due to the following warning:\n\n"
				"QuickBooks Warning:\n%s",(LPCTSTR)pResp->GetStatusMessage());
			}
			AfxMessageBox(str);
		}
		else {
			AfxThrowNxException("%s %li on QB_CreateCustomer request: %s", (LPCTSTR)pResp->GetStatusSeverity(), pResp->GetStatusCode(), (LPCTSTR)pResp->GetStatusMessage());
		}
		return FALSE;
	}
}

BOOL QB_GetCustomerListID(IQBSessionManager *lpqbSessionManager, IN const long &nCustomerPracticeID, OUT CString &strCustomerListID, OUT CString &strCustomerEditSequence)
{
	IQBSessionManagerPtr qb(lpqbSessionManager);

	//Get the name
	CString strCustomerFullNameLFM = "",
			strCustomerFullNameFML = "",
			strQuickbooksID = "";

	//DRT 10/12/2005 - PLID 17876 - The QB_CreateCustomer code submits this format with a 1 digit middle initial.  This
	//	code has been checking for a full middle name.  Therefore, QB returns FALSE, this patient with a full middle name
	//	does not exist.  But we really need to be looking for the middle initial.
	_RecordsetPtr rs = CreateRecordset("SELECT (Last + ', ' + First + CASE WHEN Middle = '' THEN '' ELSE (' ' + left(Middle, 1)) END) AS NameLFM, (First + CASE WHEN Middle = '' THEN '' ELSE (' ' + left(Middle, 1)) END + ' ' + Last) AS NameFML FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID WHERE ID = %li",nCustomerPracticeID);
	if(!rs->eof) {
		strCustomerFullNameLFM = AdoFldString(rs, "NameLFM","");
		strCustomerFullNameFML = AdoFldString(rs, "NameFML","");
	}
	rs->Close();

	//We'll try three ways, in this order:
	//ID (if we have it) --- not anymore!
	//LFM
	//FML

	//first try with the ID, if we have one
	/*
	if(strQuickbooksID != "") {

		IMsgSetRequestPtr pReqSet = QB_CreateMsgSetRequest(qb);
		pReqSet->Attributes->OnError = roeContinue;

		// Create and prepare the request for the list of the specific customer account
		ICustomerQueryPtr pCustomersReq = pReqSet->AppendCustomerQueryRq();
		if(strQuickbooksID == "")
			//we have no ID, so we check based on name
			pCustomersReq->ORCustomerListQuery->FullNameList->Add((LPCTSTR)strCustomerFullNameLFM);
		else
			//we have an ID, let's verify it exists
			pCustomersReq->ORCustomerListQuery->ListIDList->Add((LPCTSTR)strQuickbooksID);
		
		// Do the request, and get the result
		IResponsePtr pResp = qb->DoRequests(pReqSet)->ResponseList->GetAt(0);
		if (pResp->StatusCode == 0) {
			// The result was success, so parse the return value to get the list of accounts
			ICustomerRetListPtr pCustomerRetList = pResp->Detail;
			ASSERT(pCustomerRetList->GetCount() == 1);
			ICustomerRetPtr pCustomerRet = pCustomerRetList->GetAt(0);
			strCustomerEditSequence = _S(pCustomerRet->EditSequence);
			strCustomerListID = _S(pCustomerRet->ListID);
			return TRUE;
		}
	}*/

	//LFM
	IMsgSetRequestPtr pReqSet = QB_CreateMsgSetRequest(qb);
	pReqSet->Attributes->OnError = roeContinue;

	ICustomerQueryPtr pCustomersReq = pReqSet->AppendCustomerQueryRq();
	if(strQuickbooksID == "") {
		//we have no ID, so we check based on name
		pCustomersReq->ORCustomerListQuery->FullNameList->Add((LPCTSTR)strCustomerFullNameLFM);
	}
	
	// Do the request, and get the result
	IResponsePtr pResp = qb->DoRequests(pReqSet)->ResponseList->GetAt(0);
	if (pResp->StatusCode == 0) {
		// The result was success, so parse the return value to get the list of accounts
		ICustomerRetListPtr pCustomerRetList = pResp->Detail;
		ASSERT(pCustomerRetList->GetCount() == 1);
		ICustomerRetPtr pCustomerRet = pCustomerRetList->GetAt(0);
		strCustomerEditSequence = _S(pCustomerRet->EditSequence);
		strCustomerListID = _S(pCustomerRet->ListID);
		return TRUE;
	}

	//FML
	IMsgSetRequestPtr pReqSet2 = QB_CreateMsgSetRequest(qb);
	pReqSet2->Attributes->OnError = roeContinue;

	ICustomerQueryPtr pCustomersReq2 = pReqSet2->AppendCustomerQueryRq();
	if(strQuickbooksID == "") {
		//we have no ID, so we check based on name
		pCustomersReq2->ORCustomerListQuery->FullNameList->Add((LPCTSTR)strCustomerFullNameFML);
	}
	
	// Do the request, and get the result
	IResponsePtr pResp2 = qb->DoRequests(pReqSet2)->ResponseList->GetAt(0);
	if (pResp2->StatusCode == 0) {
		// The result was success, so parse the return value to get the list of accounts
		ICustomerRetListPtr pCustomerRetList = pResp2->Detail;
		ASSERT(pCustomerRetList->GetCount() == 1);
		ICustomerRetPtr pCustomerRet = pCustomerRetList->GetAt(0);
		strCustomerEditSequence = _S(pCustomerRet->EditSequence);
		strCustomerListID = _S(pCustomerRet->ListID);
		return TRUE;
	}

	//not found!

	return FALSE;
}

BOOL QB_UpdateCustomer(IQBSessionManager *lpqbSessionManager, IN const long &nCustomerPracticeID, IN const CString &strCustomerListID, IN const CString &strCustomerEditSequence)
{
	IQBSessionManagerPtr qb(lpqbSessionManager);



	// Create the request manager
	IMsgSetRequestPtr pReqSet = QB_CreateMsgSetRequest(qb);
	pReqSet->Attributes->OnError = roeContinue;

	// Prepare the request
	ICustomerModPtr pCustomerModReq = pReqSet->AppendCustomerModRq();
	// their Quickbooks ID
	pCustomerModReq->ListID->SetValue((LPCTSTR)strCustomerListID);
	// the Edit sequence is required for Modification changes
	pCustomerModReq->EditSequence->SetValue((LPCTSTR)strCustomerEditSequence);


	//now set their demographics
	_RecordsetPtr rs = CreateRecordset("SELECT (Last + ', ' + First + CASE WHEN Middle = '' THEN '' ELSE (' ' + Middle) END) AS NameLFM, (First + CASE WHEN Middle = '' THEN '' ELSE (' ' + Middle) END + ' ' + Last) AS NameFML, PersonT.*, PatientsT.* FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID WHERE PersonID = %li",nCustomerPracticeID);
	if(!rs->eof) {
		CString strLast, strFirst, strMiddle;
		CString strLFM, strFML;

		//if we truncate things, let's do it silently
		strLast = AdoFldString(rs, "Last","");		
		if(strLast.GetLength() > 25) {
			strLast = strLast.Left(25);
		}
		pCustomerModReq->LastName->SetValue((LPCTSTR)strLast);

		strFirst = AdoFldString(rs, "First","");
		if(strFirst.GetLength() > 25) {
			strFirst = strFirst.Left(25);
		}
		pCustomerModReq->FirstName->SetValue((LPCTSTR)strFirst);

		strMiddle = AdoFldString(rs, "Middle","");
		if(strMiddle.GetLength() > 5) {
			strMiddle = strMiddle.Left(5);
		}
		pCustomerModReq->MiddleName->SetValue((LPCTSTR)strMiddle);

		strLFM.Format("%s, %s%s",strLast,strFirst,(strMiddle == "" ? "" : " " + strMiddle.Left(1)));
		strFML.Format("%s%s %s",strFirst,(strMiddle == "" ? "" : " " + strMiddle.Left(1)),strLast);

		if(strLFM.GetLength() > 40) {
			strLFM = strLFM.Left(40);
		}
		if(strFML.GetLength() > 40) {
			strFML = strFML.Left(40);
		}

		pCustomerModReq->Name->SetValue((LPCTSTR)strLFM);
		pCustomerModReq->Contact->SetValue((LPCTSTR)strFML);
		
		//Quickbooks always puts the FML name in the address and the contact when making a new patient, so we must follow suit
		pCustomerModReq->BillAddress->Addr1->SetValue((LPCTSTR)strFML);

		CString str;
		str = AdoFldString(rs, "Address1","");
		if(str.GetLength() > 40) {
			str = str.Left(40);
		}
		pCustomerModReq->BillAddress->Addr2->SetValue((LPCTSTR)str);

		str = AdoFldString(rs, "Address2","");
		if(str.GetLength() > 40) {
			str = str.Left(40);
		}
		pCustomerModReq->BillAddress->Addr3->SetValue((LPCTSTR)str);

		str = AdoFldString(rs, "City","");
		if(str.GetLength() > 30) {
			str = str.Left(30);
		}
		pCustomerModReq->BillAddress->City->SetValue((LPCTSTR)str);

		str = AdoFldString(rs, "State","");
		if(str.GetLength() > 2) {
			str = str.Left(2);
		}
		if(!g_bCanadianQuickbooks)
			pCustomerModReq->BillAddress->State->SetValue((LPCTSTR)str);
		else
			pCustomerModReq->BillAddress->Province->SetValue((LPCTSTR)str);

		str = AdoFldString(rs, "Zip","");
		if(str.GetLength() > 10) {
			str = str.Left(10);
		}
		pCustomerModReq->BillAddress->PostalCode->SetValue((LPCTSTR)str);

		CString strUserDefinedID;
		strUserDefinedID.Format("%li",AdoFldLong(rs, "UserDefinedID"));
		pCustomerModReq->AccountNumber->SetValue((LPCTSTR)strUserDefinedID);
		pCustomerModReq->Email->SetValue((LPCTSTR)AdoFldString(rs, "Email",""));
		pCustomerModReq->Phone->SetValue((LPCTSTR)AdoFldString(rs, "HomePhone",""));

		//if we ever choose to send the SocialSecurity number over, remember is is SIN in Canada,
		//so we will have to accomodate that using the g_bCanadianQuickbooks indicator
	}
	rs->Close();

	// Do the request, and get the result
	IResponsePtr pResp = qb->DoRequests(pReqSet)->ResponseList->GetAt(0);
	if (pResp->StatusCode == 0) {
		// The result was success, so return TRUE
		return TRUE;
	} else {
		// Couldn't edit, throw the exception
		AfxThrowNxException("%s %li on QB_UpdateCustomer request: %s", (LPCTSTR)pResp->GetStatusSeverity(), pResp->GetStatusCode(), (LPCTSTR)pResp->GetStatusMessage());
		return FALSE;
	}
}

IQBSessionManagerPtr QB_OpenSession() {

	try {

		IQBSessionManagerPtr qb(__uuidof(QBSessionManager));	

		//the appid is our uuid for the program, from Practice.odl
		qb->OpenConnection("F2B94DA9-9A7D-11D1-B2C7-00001B4B970B", "NexTech Practice");

		// Begin the qb session

		CString strPath = GetRemotePropertyText("QuickbooksDataPath","",0,"<None>",TRUE);

		//if they didn't specify where the QBW file is at, they need Quickbooks already open.
		//we would pass either the path or a blank string, and in either case strPath would be correct
		qb->BeginSession(_bstr_t(strPath), omDontCare);

		return qb;
	
	}catch(_com_error &e) {
		switch (e.Error()) {
			case 0x80040154:	//QuickBooks is not installed
				if(IDYES == MessageBox(GetActiveWindow(), "The Quickbooks Link files are not installed on this computer.\n\n"
					"You can install these files from your NexTech CD or NxCD folder on the server, under the ThirdParty\\QBSetup folder.\n\n"
					"Would you like to auto-install the Quickbooks Link files now?\n"
					"(After installing, you will need to restart both Practice and Quickbooks.)", "Practice", MB_YESNO|MB_ICONEXCLAMATION)) {
					TryInstallQBSDK();
				}
				else {
					AfxMessageBox("The Quickbooks Link setup files must be installed before the link can be utilized.");
				}
				break;
			case 0x8007007e:	//The SDK dll was not found
				if(IDYES == MessageBox(GetActiveWindow(), "The QuickBooks link file, qbFC3.dll, could not be found.\n\n"
					"You can install this from your NexTech CD or NxCD folder on the server, under the ThirdParty\\QBSetup folder.\n\n"
					"Would you like to auto-install the Quickbooks Link files now?\n"
					"(After installing, you will need to restart both Practice and Quickbooks.)", "Practice", MB_YESNO|MB_ICONEXCLAMATION)) {
					TryInstallQBSDK();
				}
				else {
					AfxMessageBox("The Quickbooks Link setup files must be installed before the link can be utilized.");
				}
				break;
			case 0x80040416:	//QuickBooks is not open, and we are trying to open it without a .QBW path
			case 0x80040417:	//QuickBooks is open, but no company file is open, and we are trying to open it without a .QBW path
				AfxMessageBox("You must have your QuickBooks company file open on this computer in order to export data.");
				break;
			case 0x80040406:	//invalid file name or invalid file itself
				AfxMessageBox("The specified QuickBooks company data file was not found. Please check your settings in Practice.");
				break;
			case 0x80040420:	//the user was prompted to let Practice access QuickBooks, and they denied access
				AfxMessageBox("You specified that you did not want Practice to access QuickBooks. The export will not be processed.");
				break;

				//The rest of these messages are straight from QB, modified slightly to change references to
				//"this application" to say "Practice"
			case 0x80040308:
				//this one, of course, has our instructions added to the end of it.
				//you'll get this error if they have an older SDK file than we're trying to use, or (less likely) if the QuickBooks installation is corrupt
				AfxMessageBox("Could not communicate with the QuickBooks SDK.  The QuickBooks Link files may be missing or QuickBooks may not installed on this computer.\n\n"
					"You can install the QuickBooks Link files from your NexTech CD or NxCD folder on the server, under the ThirdParty\\QBSetup folder.\n"
					"Contact NexTech if you need assistance.");
				break;

			case 0x80040418:
				AfxMessageBox("Practice has not accessed this QuickBooks company data file before. Only the\n"
							"QuickBooks administrator can grant an application permission to access a QuickBooks company\n"
							"data file for the first time.");
				break;
			case 0x8004041A:
				AfxMessageBox("Access denied. Practice does not have permission to access this QuickBooks company\n"
					"data file. If access is required, the QuickBooks administrator can grant permission through the\n"
					"Integrated Application preferences of QuickBooks.");
				break;
			case 0x8004041D:
				AfxMessageBox("Practice is not allowed to log into this QuickBooks company data file automatically. If\n"
					"automatic login is required, the QuickBooks administrator can grant permission through the\n"
					"Integrated Application preferences of QuickBooks.");
				break;
			case 0x8004041F:
				AfxMessageBox("QuickBooks Basic does not support software integration.\n"
					"Another product in the QuickBooks line, such as QuickBooks Pro or Premier, is required.");
				break;
			case 0x80040422:
				AfxMessageBox("Practice requires single-user file access mode and there is another application already\n"
					"accessing the specified QuickBooks company data file. Multiple applications that require single-user\n"
					"file access mode cannot begin sessions with QuickBooks simultaneously.");
				break;
			case 0x80040424:
				AfxMessageBox("QuickBooks did not finish its initialization. Please try again later.");
				break;
			case 0x80040403:
				AfxMessageBox("Could not open specified QuickBooks company data file.");
				break;
			case 0x8004040A:
				AfxMessageBox("There is already a QuickBooks company data file open, and it is different from file specified in the QuickBooks Link Setup.\n"
					"Please close your current data file or correct the default file path in the link setup.");
				break;			
			default:
				HandleException(e,"Error opening QuickBooks.",__LINE__,__FILE__,"","");
				break;
		}
	}

	return NULL;	
}

BOOL QB_VerifyPaymentTxnID(IQBSessionManager *lpqbSessionManager, IN const CString &strPaymentTxnID)
{
	IQBSessionManagerPtr qb(lpqbSessionManager);

	// Create the request manager
	IMsgSetRequestPtr pReqSet = QB_CreateMsgSetRequest(qb);
	pReqSet->Attributes->OnError = roeContinue;

	// Create and prepare the request for the list of the specific customer account
	IReceivePaymentQueryPtr pPaymentsReq = pReqSet->AppendReceivePaymentQueryRq();
	if(strPaymentTxnID == "")
		return FALSE;
	else
		//we have an ID, let's verify it exists
		pPaymentsReq->ORTxnQuery->TxnIDList->Add((LPCTSTR)strPaymentTxnID);
	
	// Do the request, and get the result
	IResponsePtr pResp = qb->DoRequests(pReqSet)->ResponseList->GetAt(0);
	if (pResp->StatusCode == 0) {
		// The payment does exist, so return true
		return TRUE;
	} else {
		// The payment doesn't exist, return false
		return FALSE;
	}
}

BOOL QB_VerifyPaymentAccountID(IQBSessionManager *lpqbSessionManager, IN const CString &strPaymentAcctID)
{
	IQBSessionManagerPtr qb(lpqbSessionManager);

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

			if(strPaymentAcctID == (LPCTSTR)pAcctRet->ListID->GetValue()) {
				//found it!
				return TRUE;
			}
		}
	}

	//we didn't find the account
	return FALSE;
}

//try to create the right request for the right version
IMsgSetRequestPtr QB_CreateMsgSetRequest(IQBSessionManager *lpqbSessionManager)
{
	IQBSessionManagerPtr qb(lpqbSessionManager);

	CString strLatestVersion = QB_GetQBFCLatestVersion(qb);

	IMsgSetRequestPtr pReqSet;

	//first see if we're in Canada

	CString strCountry = "US";
	g_bCanadianQuickbooks = FALSE;

	if(strLatestVersion.Find("CA") != -1) {
		g_bCanadianQuickbooks = TRUE;
		strCountry = "CA";
		strLatestVersion.Replace("CA","");
	}

	if(strLatestVersion >= "3")
		return pReqSet = qb->CreateMsgSetRequest(_bstr_t(strCountry), 3, 0);
    else if(strLatestVersion >= "2.1")
		return pReqSet = qb->CreateMsgSetRequest(_bstr_t(strCountry), 2, 1);
	else if(strLatestVersion >= "2.0")
		return pReqSet = qb->CreateMsgSetRequest(_bstr_t(strCountry), 2, 0);
    else if(strLatestVersion >= "1.1")
		return pReqSet = qb->CreateMsgSetRequest(_bstr_t(strCountry), 1, 1);
    else {
        AfxMessageBox("You are apparently running QuickBooks 2002 Release 1, we strongly recommend that you use\n"
			"QuickBooks' online update feature to obtain the latest fixes and enhancements");
        return pReqSet = qb->CreateMsgSetRequest(_bstr_t(strCountry), 1, 0);
    }
}



CString QB_GetQBFCLatestVersion(IQBSessionManager *lpqbSessionManager)
{
	IQBSessionManagerPtr qb(lpqbSessionManager);

	CString strVersions;

	try {

	//Use oldest version to ensure that we work with any QuickBooks (US)
	IMsgSetRequestPtr pReqSet = qb->CreateMsgSetRequest("US", 1, 0);

	pReqSet->AppendHostQueryRq();

	IResponsePtr pResp = qb->DoRequests(pReqSet)->ResponseList->GetAt(0);
	if(pResp->StatusCode == 0) {
		//success, now retrieve the versions
		IHostRetPtr pHostRetList = pResp->Detail;

		IBSTRListPtr pSupportedVersions;
		pSupportedVersions = pHostRetList->SupportedQBXMLVersionList;

		CString version, lastversion = "1.0";

		//now parse and calculate the latest version supported
		//remember, it may contain CA!
		for(int i=0;i<pSupportedVersions->Count;i++) {			
			_bstr_t bVer = _bstr_t((pSupportedVersions->GetAt(i)));
			version = CString(BSTR(bVer));
			if(version > lastversion)
				lastversion = version;
		}
		return lastversion;
	}

	} catch(...) {

		//JMJ 1/23/2003 - Intuit has yet to define a way to request the country code from
		//the application without first specifying the country code. Thus making it impossible
		//to tell what country it is without trial and error. So if the above request fails,
		//we assume it is the Canadian version. If this fails, then it should return the failure,
		//because it means we legitimately would not support the version of the software.

		//Use oldest version to ensure that we work with any QuickBooks (CA)
		IMsgSetRequestPtr pReqSet = qb->CreateMsgSetRequest("CA", 2, 0);

		pReqSet->AppendHostQueryRq();

		IResponsePtr pResp = qb->DoRequests(pReqSet)->ResponseList->GetAt(0);
		if(pResp->StatusCode == 0) {
			//success, now retrieve the versions
			IHostRetPtr pHostRetList = pResp->Detail;

			IBSTRListPtr pSupportedVersions;
			pSupportedVersions = pHostRetList->SupportedQBXMLVersionList;

			CString version, lastversion = "1.0";

			//now parse and calculate the latest version supported
			//remember, it may contain CA!
			for(int i=0;i<pSupportedVersions->Count;i++) {			
				_bstr_t bVer = _bstr_t((pSupportedVersions->GetAt(i)));
				version = CString(BSTR(bVer));
				if(version > lastversion)
					lastversion = version;
			}
			return lastversion;
		}

	}

	return "1.1";
}

BOOL TryInstallQBSDK()
{
	CString strShadowPath = NxRegUtils::ReadString("HKEY_LOCAL_MACHINE\\SOFTWARE\\Nextech\\Setup\\NextPath");
	
	if(strShadowPath == "") {
		AfxMessageBox("The NextPath was not found in your dock settings. You may need\n"
			"to run this file from the \\ThirdParty\\QBSetup folder from your install CD.\n"
			"(After installing, you will need to restart both Practice and Quickbooks.)");
		return FALSE;
	}

	strShadowPath += "\\ThirdParty\\QBSetup";
	CString strPathWithFile = strShadowPath ^ "QBFC3_0Installer.exe";
	
	if(!DoesExist(strPathWithFile)) {
		CString str;
		str.Format("The file %s could not be found.\n\n"
			"It is possible your dock settings do not have the correct NextPath entered, or you may\n"
			"need to run this file from the \\ThirdParty\\QBSetup folder from your install CD.\n"
			"(After installing, you will need to restart both Practice and Quickbooks.)",strPathWithFile);
		AfxMessageBox(str);
		return FALSE;
	}
	else {
		HINSTANCE hInst;
		// (a.walling 2009-08-10 16:17) - PLID 35153 - Use NULL instead of "open" when calling ShellExecute(Ex). Usually equivalent; see PL notes.
		if ((int)(hInst=ShellExecute(GetDesktopWindow(), NULL, strPathWithFile, NULL, strShadowPath, SW_SHOWDEFAULT)) < 32) {
			CString str;
			str.Format("The file %s was found,\n"
				"but failed to run. You may need to run this file manually from from the above listed folder\n"
				"or from \\ThirdParty\\QBSetup folder from your install CD.\n"
				"(After installing, you will need to restart both Practice and Quickbooks.)",strPathWithFile);
			AfxMessageBox(str);
			return FALSE;
		}
		else {
			AfxMessageBox("The Quickbooks Link installation is in progress, please restart both Practice and Quickbooks when the installation is complete.");
			return TRUE;
		}
	}
}
// QuickbooksUtils.h

#pragma once

// (a.walling 2012-08-03 14:31) - PLID 51956 - Compiler limits exceeded with imported NxAccessor typelib - get quickbooks out of stdafx

struct __declspec(uuid("ac5d522f-776b-4135-b73a-2fc4da1e6cdf"))
/* dual interface */ IQBSessionManager;
struct __declspec(uuid("eacd36fa-f767-43e6-ad9a-93d27a6dd44f"))
/* dual interface */ IMsgSetRequest;

_COM_SMARTPTR_TYPEDEF(IQBSessionManager, __uuidof(IQBSessionManager));
_COM_SMARTPTR_TYPEDEF(IMsgSetRequest, __uuidof(IMsgSetRequest));

/*
#define SAFE_SET_PROGRESS(prog, text) if (prog) { prog->SetDlgItemText(IDC_STATUS_LABEL, text); }
*/
#define SAFE_SET_PROGRESS(prog, text)

enum EQBAcctType {

	qbatBankOnly = 1,
	qbatAllTypes,
	qbatReceivePaymentsAccts,
};

// (j.jones 2008-08-05 11:06) - PLID 25338 - added a patient ID value, may be -1 if a batch payment
struct QBDepositInfo {

	long nPaymentID;
	long nPatientID;
	CString strCustomerListID;
	CString strDepositFromAccountListID;
};

IQBSessionManagerPtr QB_OpenSession();

//void QB_PerformPendingRequests(IQBSessionManager *lpSessionManager, IMsgSetRequest *lpMsgSetRequest, OUT CString *pstrErrors = NULL);

BOOL QB_ChooseIndividualAccount(IQBSessionManager *lpqbSessionManager, OUT CString &strID, CWnd *pParent, CWnd *pProgress = NULL, EQBAcctType qbatAccountType = qbatReceivePaymentsAccts);

BOOL QB_ChooseDepositAccounts(IQBSessionManager *lpqbSessionManager, OUT CString &strFromID, OUT CString &strToID, CWnd *pParent, CWnd *pProgress = NULL, BOOL bSetDefaults = FALSE);

BOOL QB_CreateDeposit(IQBSessionManager *lpSessionManager, IN CPtrArray &aryPaymentIDs, IN CPtrArray &aryBatchPaymentIDs, IN CPtrArray &aryPaymentTipIDs, IN CPtrArray &aryRefundIDs, IN CPtrArray &aryBatchPaymentRefundIDs, const CString &strDepositToAccountListID, BOOL bExportPatients, COleDateTime dtDepositDate);

BOOL QB_CreatePayment(IQBSessionManager *lpSessionManager, const CString &strCustomerListID, OUT CString &strPaymentTxnID, IN const long nPaymentID, BOOL bIsAutoApply, const CString &strDepositToAccountListID, BOOL bIsTip, COleDateTime dtDeposit);

BOOL QB_VerifyPaymentTxnID(IQBSessionManager *lpqbSessionManager, IN const CString &strPaymentTxnID);

BOOL QB_VerifyPaymentAccountID(IQBSessionManager *lpqbSessionManager, IN const CString &strPaymentAcctID);

BOOL QB_CreateCustomer(IQBSessionManager *lpqbSessionManager, IN const long &nCustomerPracticeID, OUT CString &strCustomerListID);

BOOL QB_GetCustomerListID(IQBSessionManager *lpqbSessionManager, IN const long &nCustomerPracticeID, OUT CString &strCustomerListID, OUT CString &strCustomerEditSequence);

BOOL QB_UpdateCustomer(IQBSessionManager *lpqbSessionManager, IN const long &nCustomerPracticeID, IN const CString &strCustomerListID, IN const CString &strCustomerEditSequence);

IMsgSetRequestPtr QB_CreateMsgSetRequest(IQBSessionManager *lpqbSessionManager);

CString QB_GetQBFCLatestVersion(IQBSessionManager *lpqbSessionManager);

BOOL TryInstallQBSDK();
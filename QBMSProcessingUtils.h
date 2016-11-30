//QBMSProcessingUtils.h
//
// (d.thompson 2009-06-22) - PLID 34688 - QBMS replaces IGS processing, copied the old file entirely.

#ifndef QBMSProcessingUtils_H
#define QBMSProcessingUtils_H

#pragma once


//NOTE:  These are saved to data and may never change
enum QBMS_TransTypes {
	qbttSale = 1,
	qbttVoidOrReturn = 2,
	qbttVoiceAuth = 3,
	//We are not currently supporting:
	//	Auth - Covered by sale for our needs
	//	Capture - Covered by sale for our needs
	//	Void - Covered by VoidOrReturn
	//	Return - Covered by VoidOrReturn
};


// (d.thompson 2010-11-04) - PLID 40628 - We now support multiple accounts, so reworked the dialog setup fairly 
//	heavily to support that sort of interface.  We now use this class to track the data.
class CQBMSProcessingAccount {
public:
	//Constructor
	CQBMSProcessingAccount();

	//Creation functions
	void LoadFromData(long _nID, CString _strDesc, bool _bInactive, bool _bIsProd, _variant_t _varTicket);
	void LoadAsNewAccount(long _nID);

	//Accessor functions
	void SetDescription(CString _strDesc);
	CString GetDescription();

	void SetInactive(bool bInactive = true);
	bool GetIsInactive();

	void SetProduction(bool bProd = true);
	bool GetIsProduction();

	void SetAndEncryptTicket(CString strNewTicket, bool bForceOverride = false);
	CString GetDecryptedTicket();
	VARIANT GetEncryptedTicketForDatabase();

	long GetID();
	void ChangeIDAfterSave(long nNewID);

	bool GetIsModified();
	void SetNotModified();

protected:
	//Status
	bool bModified;

	//Member variables for data
	long nID;
	CString strDescription;
	bool bInactive;
	bool bIsProduction;
	_variant_t varEncryptedTicket;
};


namespace QBMSProcessingUtils {
	//Input data to start
	// (d.thompson 2009-07-01) - PLID 34230 - Added auditing data
	// (d.thompson 2010-11-08) - PLID 41367 - Added connection ticket for multiple QBMS account support
	// (d.thompson 2010-11-08) - PLID 41367 - Also added production flag
	bool ProcessQBMSTransaction(IN bool bIsProduction, IN CString strConnectionTicket, QBMS_TransTypes qbTransType, long nPaymentID, CString strCCNumber, COleCurrency cyTotalAmount, 
		CString strNameOnCard, CString strMM, CString strYY, CString strSecurityCode, CString strAddress, 
		CString strZip, bool bHasTrack2Data, CString strTrack2Data, bool bCardPresent, CString strPreviousTransID,
		long nAuditPatientID, // (a.walling 2010-01-25 09:02) - PLID 37026 - Pass in the patient ID
		CString strAuditPatientName, long nAuditRecordID,
		//Output data here
		OUT CString &strCreditCardTransID, OUT CString &strAuthorizationCode, OUT CString &strAVSStreet, 
		OUT CString &strAVSZip, OUT CString &strCardSecurityCodeMatch, OUT CString &strPaymentStatus, OUT COleDateTime &dtTxnAuthorizationTime);

	//Session ticket, only saved in memory.
	// (d.thompson 2010-11-16) - PLID 41367 - Commented out until it's necessary
	//bool GetCurrentQBMSSessionTicket(CString &strSessionTicket);

	//Cached setup information.  Should only be loaded once.
	// (d.thompson 2010-06-07) - PLID 39030 - Added flag to use production / pre-production
	CString GetAppID(BOOL bUseProduction);
	CString GetAppLogin();

	// (d.thompson 2010-11-08) - PLID 41367 - Now supporting multiple accounts, change the behind-the-scenes caching mechanisms
	void ClearCachedAccounts();
	void LoadCachedAccountsFromData();
	void GetAccountIDsAndNames(CArray<CQBMSProcessingAccount, CQBMSProcessingAccount&> *pary);
	CQBMSProcessingAccount GetAccountByID(long nIDToFind);


	//QBMS URLs
	CString GetConnectionTicketURL(BOOL bGetProduction);
	// (d.thompson 2010-11-08) - PLID 41367 - Added production flag
	CString GetIntermediateSessionTicketURL(bool bIsProduction);
	// (d.thompson 2010-11-08) - PLID 41367 - added connection ticket and production flag
	CString GetTransformIntermediateSessionTicketURL(CString strIntermediateSessionTicket, CString strConnectionTicket, bool bIsProduction);
	// (d.thompson 2010-11-08) - PLID 41367 - Added production flag
	CString GetTransactionRequestURL(bool bIsProduction);
}

#endif	//QBMSProcessingUtils_H
//PaymentechUtils.h

#if !defined(AFX_PAYMENTECHUTILS_H__CF02ABAC_BC8C_4E31_A27B_6C706F6DC1FE__INCLUDED_)
#define AFX_PAYMENTECHUTILS_H__CF02ABAC_BC8C_4E31_A27B_6C706F6DC1FE__INCLUDED_

#pragma once



#include "afxtempl.h"

#import "paymentech.dll"

// (j.gruber 2007-08-27 17:22) - PLID 15416 - cc processing
namespace PaymentechUtils {

	const char chStartText = 0x02;
	const char chFieldSeparator = 0x1C;
	const char chEndText = 0x03;

	enum LanguageCode {
		lcEnglish = 0,
		lcFrench  = 1,
	};
	

	struct ResponseStruct {

		BOOL bApproved;
		char chAddressVerificationCode;
		CString strAuthCode;
		CString strErrorCode;
		CString strBatchNumber;		
		long nRetreivalReferenceNumber;
		long nSequenceNumber;
		CString strResponseMessage;
		CString strCardType;
		CString strInterchangeCompliance;
		long nAuthorizingNetworkID;
		char chAuthSource;
		CString strOptionalData;
		CString strTokenIndicator;
		CString strTokenResponseData;

		//these are for batch responses
		char chDownloadFlag;
		char chMultiMessageFlag;
		COleDateTime dtBatchOpen;
		COleDateTime dtBatchClose;
		long nBatchTransactionCount;
		COleCurrency cyBatchNetAmount;
		
		CString strPayType1;
		long nNumTransactionsPayType1;
		COleCurrency cyNetAmountPayType1;

		CString strPayType2;
		long nNumTransactionsPayType2;
		COleCurrency cyNetAmountPayType2;

		CString strPayType3;
		long nNumTransactionsPayType3;
		COleCurrency cyNetAmountPayType3;

		CString strPayType4;
		long nNumTransactionsPayType4;
		COleCurrency cyNetAmountPayType4;

		CString strTraceNumber;
		LanguageCode lcLangCode;

		CMap <CString, LPCTSTR, CString, LPCTSTR> mapTokens;

	};

	enum EntryDataSource {
		edsNone = -123,
		edsCardSwipeOriginUnknown = 1,
		edsManuallyEntered = 2,
		edsTrack2 = 3,
		edsTrack1 = 4,
		edsScannedCheckReader = 5,
		edsIVR = 6,
		edsUnattendedTerminalManuallyEntered = 12,
		edsUnattendedTerminalTrack2 = 13,
		edsUnattendedTerminalTrack1 = 14,
		edsContactlessDeviceTrack1FromRFID_Visa_MasterCard_Amex_Only = 31,
		edsContactlessDeviceTrack2FromRFID_Visa_MasterCard_Amex_Only = 32,
		edsContactlessDeviceTrack1FromMagStripe_AllCardTypes = 33,
		edsContactlessDeviceTrack2FromMagStripe_AllCardTypes = 34,
		edsContactlessDeviceManuallyEntered_AllCardTypes = 35,
		edsChipCapableDevice_ChipReadTrack2_Visa_MasterCard_Only = 36,
		edsChipCapableDevice_ChipReadAttempted_FallBackToMagStripeTrack1_Visa_MasterCard_Only = 37,
		edsChipCapableDevice_ChipReadAttempted_FallBackToMagStripeTrack2_Visa_MasterCard_Only = 38,
		edsChipCapableDevice_ChipReadAttempted_MagReadAttempted_FallBackToManuallyEntered_Visa_MasterCard_Only = 39,
		edsChipCapableDevice_RFIDRead_ChipTechnologyToReadTheCard_Track1_Visa_MasterCard_Only = 40,
		edsChipCapableDevice_RFIDRead_ChipTechnologyToReadTheCard_Track2_Visa_MasterCard_Only = 41,
		edsChipCapableDevice_RFIDRead_MagStripeTechnologyToReadTheCard_Track1_Visa_MasterCard_Only = 42,
		edsChipCapableDevice_RFIDRead_MagStripeTechnologyToReadTheCard_Track2_Visa_MasterCard_Only = 43,
		edsChipCapableDevice_MagStripeRead_Track1_AllCardTypes = 44,
		edsChipCapableDevice_MagStripeRead_Track2_AllCardTypes = 45,
		edsChipCapableDevice_ManuallyEntered_AllCardTypes = 46,
		
	};

	enum TransactionCode {
		tcNone = -123,
		tcSale = 1,
		tcAuthOnly = 2,
		tcReturn = 6,
		tcForceSale = 3,
		tcVOID = 41,
		tcBatchInquiry = 50,
		tcBatchRelease = 51,
		tcDebit = 12312,
		tcInteracSale = 27,
		tcInteracSaleWithCashBack = 28,
		tcInteracReturn = 29,
		tcMACReversal = 48,
		tcCurrentKeyRequest = 56,
	};

	enum CardType {
		ctNone = -123,
		ctCredit = 0,
		ctDebit = 1,
		ctStoredValue = 2,
		ctInterac = 3,
	};

	enum InteracTransactionID {
		itiNone = -123,
		itiPurchase = 0,
		itiRefund = 1,
		itiBalanceInquiry = 2,
		itiSaleCorrection = 3,
		itiRefundCorrection = 4,
	};

	// (a.walling 2007-07-02 10:22) - Presence of card/cvd code indicator
	enum PresenceIndicator {
		piNone = -123,
		piBypassed = 0,
		piProvided = 1, //default
		piIllegible = 2,
		piUnavailable = 9,
	};

	struct TransStruct {
		
		CString strSystemIndicator;
		CString strRoutingIndicator;
		long nClientNumber;
		CString strMerchantNumber;
		long nTerminalNumber;
		long nTransactionSequenceFlag;
		long nSequenceNumber;
		char chTransactionClass;
		TransactionCode tcTransactionCode;
		long nPINCapabilityMode;
		EntryDataSource edsEntryDataSource;
		CString strMagneticStripe;
		CString strAccountNumber;
		CString strExpireDate;
		COleCurrency cyTransAmount;
		CString strLRR;
		long nIndustryCode;
		long nInvoiceNumber;
		CString strItemCode;
		CString strCardHolderStreet;
		CString strExtendedStreetInfo;
		CString strCardHolderZip;
		CString strCustomerReferenceNumber;
		char chTaxFlag;
		CString strDestinationZip;
		COleCurrency cySalesTaxAmt;
		CString strTokenIndicator;
		CString strTokenData;
		BOOL bSwiped;
		CardType ctCardType;
		CString strAuthorizationCode;
		BOOL bTipAllowed;
		BOOL bCashBackAllowed;
		LanguageCode lcLangCode;
		BOOL bShowSurcharge;
		COleCurrency cySurchargeAmount;
		COleCurrency cyTipAmount;
		BOOL bSurchargeAccepted;

			
		//used only in VOID transactions
		long nRetrievalReferenceNumber;
		long nLastRetrievalReferenceNumber;

		//used only in BATCH transactions
		CString strSysInfo;
		long nBatchNumber;
		long nBatchSequenceNumber;
		long nTransactionCount;
		COleCurrency cyNetAmount;
		long nBatchOffset;


		//used only in DEBIT transactions
		CString strSerialApplicationName;
		CString strCoreLibraryVersion;
		CString strSecurityLibraryVersion;
		CString strPinPadSerialNumber;
		CString strROMVersion;
		CString strRAMVersion;


		//used only in Interac transactions
		CString strPinBlock;
		long nAccountType;
		CString strMACBlock;
		InteracTransactionID iTransID;
		COleCurrency cyCashBackAmount;
		COleCurrency cyInteracSurchargeAmount;

		// (a.walling 2007-07-02 10:22) - Used for token data
		CString strCustomerDefinedData;
		PresenceIndicator piPresenceIndicator;
		CString strCVD;	


		
		
	};

	// (a.walling 2007-08-02 09:25) - Token functions
	void ParseToken(CString strToken, ResponseStruct& Resp);
	void ProcessTokens(ResponseStruct &Resp, OPTIONAL long nFlags = 0);

	BOOL GetTerminalRetailMessage(CString &strMessage, TransStruct Message);
	BOOL GetTerminalVoidMessage(CString &strMessage, TransStruct Message);
	BOOL InitTerminalMessage(CString &strMessage, TransStruct Message);
	BOOL GetTerminalBatchMessage(CString &strMessage, TransStruct TransMessage);
	BOOL ProcessResponseHeader(CString &strResponse, ResponseStruct &Resp);
	BOOL ProcessRetailResponse(CString &strResponse, ResponseStruct &Resp);
	BOOL ProcessBatchResponse(CString &strResponse, ResponseStruct &Resp);
	CString MessageLookup(CString strErrorCode, CString strResponseMessage, LanguageCode lcCode);
	void InitTransaction(Paymentech::IIPOSTransactionPtr &pTrans, TransStruct &pMessage);
	CString ZeroFill(long nNumberToFill, long nLengthOfField, CString strCurrentField);
	CString ZeroFill(CString strStringToFill, long nLengthOfField, CString strCurrentField);
	CString SpaceFillString(CString strStringToFill, long nLengthOfField, CString strCurrentField);
	CString ZeroFillFloat(double dToFill, long nLengthOfField, CString strCurrentField);
	CString CheckAndConvertToString(long nNumber, CString strCurrentField);
	CString CheckString(CString str, CString strCurrentField,  long nLength = -1);
	// (a.walling 2007-11-07 09:34) - PLID 27998 - VS2008 - Overload for no field parameter
	CString CheckString(CString str, long nLength = -1);
	BOOL GetTerminalDebitMessage(CString &strMessage, TransStruct Message);
	BOOL GetTerminalRetailDebitMessage(CString &strMessage, TransStruct Message);
	BOOL GetTerminalInteracMessage(CString &strMessage, TransStruct Message);
	BOOL GetTerminalCurrentKeyRequest(CString &strMessage, TransStruct Message);
	BOOL ProcessCurrentKeyResponse(CString &strResponse, ResponseStruct &Resp);
	BOOL ProcessInteracResponse(CString &strResponse, ResponseStruct &Resp);
	BOOL ProcessInteracTokenData(CString &strResponse, ResponseStruct &Resp);
	BOOL AuthorizeTransaction(TransStruct &tsTransaction, ResponseStruct &Resp);
	void HandleResult(CString strTerminalID, const VARIANT &v, ResponseStruct &pResp, PaymentechUtils::TransactionCode tcTransType, PaymentechUtils::LanguageCode lcLangCode);
	void PerformInteracResponseAnalysis(PaymentechUtils::ResponseStruct &Resp);
	BOOL SendBatchMessage(TransStruct &ptsTransaction, ResponseStruct &Resp);
	CString GetNameFromType(CString strPaymentType);
	BOOL SendVoidMessage(TransStruct &ptsTransaction, ResponseStruct &Resp);
	BOOL SendCurrentKeyRequest();
	void ClearMessage(TransStruct &Message);
	BOOL IsPaymentechInstalled();

	// (a.walling 2007-07-02 10:21)
	void ParseToken(CString strToken, ResponseStruct& Resp);
}

#endif // !defined(AFX_PAYMENTECH_H__CF02ABAC_BC8C_4E31_A27B_6C706F6DC1FE__INCLUDED_)
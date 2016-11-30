#pragma once
// (d.thompson 2010-09-01) - PLID 40314 - Created
// (d.thompson 2010-11-17) - PLID 41516 - Updated to support multiple accounts

namespace ChaseProcessingUtils {

	enum Chase_TransTypes {
		cttAuthCapture = 0,		//TransType AC
		cttRefund = 1,			//TransType R

		//Not yet supported maybe in future
		//cttAuthOnly = 2,		//TransType A
		cttPriorAuth = 3,		//TransType FC

		// (d.thompson 2010-11-18) - PLID 41532
		//Special:  These are refund subtypes.  These should only be used by the ProcessChaseTransaction function.  Order
		//	is important for how they are processed.
		cttReversal_IndY = 4,	//Reversal message type, indicator of Y
		cttReversal_IndN = 5,	//Reversal message type, indicator of N
		cttRefundMoney = 6,		//NewOrder message, type of R
	};

	// (d.thompson 2010-11-18) - PLID 41532 - Reworked to better handle refunds
	bool GetXMLByType(IN Chase_TransTypes cpTransType, OUT CString &strXMLMessage, OUT CString &strFunction,
		//Input Data
		BOOL bIsProduction, CString strUsername, CString strTextPassword, 
		CString strMerchantID, CString strTerminalID,
		long nPaymentID, CString strCCNumber, COleCurrency cyTotalAmount, 
		CString strNameOnCard, CString strMM, CString strYYYY, CString strSecurityCode, CString strAddress, 
		CString strZip, bool bHasTrack2Data, CString strTrack2Data, CString strPreviousTransID);

	// (d.thompson 2010-11-18) - PLID 41532
	bool ProcessChaseTransaction_Refund(IN OUT Chase_TransTypes &cpTransType, OUT CString &strResponseXML,
		//Input data
		BOOL bIsProduction, CString strUsername, CString strTextPassword, 
		CString strMerchantID, CString strTerminalID,
		long nPaymentID, CString strCCNumber, COleCurrency cyTotalAmount, 
		CString strNameOnCard, CString strMM, CString strYYYY, CString strSecurityCode, CString strAddress, 
		CString strZip, bool bHasTrack2Data, CString strTrack2Data, CString strPreviousTransID);

	// (d.thompson 2010-11-17) - PLID 41516 - added parameters for account details
	// (d.thompson 2010-12-20) - PLID 41895 - Added auth code
	bool ProcessChaseTransaction(Chase_TransTypes cpTransType, BOOL bIsProduction, CString strUsername, CString strTextPassword, 
		CString strMerchantID, CString strTerminalID,
		long nPaymentID, CString strCCNumber, COleCurrency cyTotalAmount, 
		CString strNameOnCard, CString strMM, CString strYY, CString strSecurityCode, CString strAddress, 
		CString strZip, bool bHasTrack2Data, CString strTrack2Data, CString strPreviousTransID, 
		long nAuditPatientID, // (a.walling 2010-01-25 09:02) - PLID 37026 - Pass in the patient ID
		CString strAuditPatientName, long nAuditRecordID,
		OUT CString &strOrderID, OUT CString &strTxRefNum,
		OUT COleDateTime &dtProcDateTime, OUT CString &strProcStatus, OUT CString &strApprovalStatus, OUT CString &strRespCode,
		OUT CString &strAVSRespCode, OUT CString &strCVVRespCode, OUT CString &strRetryTrace, OUT CString &strRetryAttemptCount, 
		OUT COleCurrency &cyReversalAmtVoided, OUT COleCurrency &cyReversalOutstandingAmt, OUT CString& strAuthorizationCode);

	// (d.thompson 2010-11-17) - PLID 41516 - Added account details as parameters
	// (d.thompson 2010-11-18) - PLID 41532 - Renamed, added param for type.  The XML is identical for charges as for refunds, 
	//	with the exception of the type flag.
	// (d.thompson 2010-12-20) - PLID 41895 - Added authcode to parseresponse
	bool GenerateValidNewOrderXML(OUT CString &strXMLMessage, long nPaymentID, CString strCCNumber,
		CString strExpMonth, CString strExpFullYear, COleCurrency cyAmount, CString strNameOnCard,
		bool bHasTrack2Data, CString strTrack2Data, CString strAddress, CString strZip, 
		CString strSecurityCode, CString strUsername, CString strTextPassword, CString strMerchantID, 
		CString strTerminalID, CString strTransactionType);
	bool ParseNewOrderResponse(const CString strXMLResponse, OUT CString &strOrderID, OUT CString &strTxRefNum,
		OUT COleDateTime &dtProcDateTime, OUT CString &strProcStatus, OUT CString &strApprovalStatus,
		OUT CString &strRespCode, OUT CString &strAVSRespCode, OUT CString &strCVVRespCode,
		OUT CString &strRetryTrace, OUT CString &strRetryAttemptCount, OUT CString &strAuthorizationCode);

	// (d.thompson 2010-11-17) - PLID 41516 - Added account details as parameters
	bool GenerateValidReversalXML(CString &strXMLMessage, long nPaymentID, CString strPreviousTransID, COleCurrency cyTotalAmount, CString strReversalInd,
		CString strUsername, CString strTextPassword, CString strMerchantID, CString strTerminalID);
	bool ParseReversalResponse(const CString strResponseXML, OUT CString &strOrderID, OUT CString &strTxRefNum, 
			OUT COleDateTime &dtProcDateTime, OUT CString &strProcStatus, OUT CString &strRetryTrace, OUT CString &strRetryAttemptCount, 
			OUT COleCurrency &cyReversalAmtVoided, OUT COleCurrency &cyReversalOutstandingAmt, OUT CString &strApprovalStatus, OUT CString &strRespCode);


	// (d.thompson 2010-11-17) - PLID 41516 - Requires username/password params now
	bool PostChaseHTTPRequest(IN const CString strUsername, IN const CString strTextPassword, IN const CString strURL, 
		IN const CString strURI, IN const CString strFunction, 
		IN const CString &strParamXml, OUT CString &strResponseText, OUT long &nStatus, OUT CString &strStatusText);

	// (d.thompson 2010-11-17) - PLID 41516 - Requires production as parameter now
	CString GetTransactionRequestURL(BOOL bIsProduction);
	CString GetTransactionURI();
	void Chase_HandleGenericError(long nHTTPStatus, CString strHTTPStatusText, CString strResponseXML);
}
// (d.thompson 2010-09-01) - PLID 40314 - Created
// (d.thompson 2010-11-17) - PLID 41516 - Updated to support multiple accounts

#include "stdafx.h"
#include "ChaseProcessingUtils.h"
#include "SOAPUtils.h"
#include "GlobalAuditUtils.h"
#include "AuditTrail.h"
#include "Base64.h"
#include "OPOSMSRDevice.h"		//Required to map account number to cc type (visa, etc)
using namespace ADODB;


namespace ChaseProcessingUtils {


#pragma region AuditData
	// (d.thompson 2009-07-01) - PLID 40314 - Copied from QBMS implementation
	bool l_bAuditDataSet = false;
	CString l_strAuditPatientName = "";
	long l_nAuditRecordID = -1;
	// (a.walling 2010-01-25 09:02) - PLID 37026 - Handle the patient ID
	long l_nAuditPatientID = -1;

	// (a.walling 2010-01-25 09:02) - PLID 37026 - Handle the patient ID
	void SetAuditData(CString strAuditPatientName, long nAuditPatientID, long nAuditRecordID)
	{
		l_strAuditPatientName = strAuditPatientName;
		l_nAuditRecordID = nAuditRecordID;
		l_nAuditPatientID = nAuditPatientID;
		l_bAuditDataSet = true;
	}

	// (a.walling 2010-01-25 09:02) - PLID 37026 - Handle the patient ID
	void ClearAuditData()
	{
		l_bAuditDataSet = false;
		l_strAuditPatientName = "";
		l_nAuditRecordID = -1;
		l_nAuditPatientID = -1;
	}

	// (a.walling 2010-01-25 09:02) - PLID 37026 - Handle the patient ID
	bool GetAuditData(CString &strPatName, long &nPatientID, long &nRecordID)
	{
		if(l_bAuditDataSet) {
			strPatName = l_strAuditPatientName;
			nRecordID = l_nAuditRecordID;
			nPatientID = l_nAuditPatientID;
		}

		return l_bAuditDataSet;
	}
#pragma endregion

	//(s.dhole 2/6/2015 11:29 AM ) - PLID 64809 Check if Payment id is greter than 999999 then return only right 6 digits 
	CString GetValidOrderID(long nPaymentID)
	{
		//(s.dhole 2/5/2015 12:32 PM ) - PLID 64809 there is limitation  of six digit in order id
		//we will use only last six digits from Payment id, if payment id is greater then 6 digits 
		CString strNxOrderID = AsString(nPaymentID);
		if (strNxOrderID.GetLength() > 6)
		{

			strNxOrderID = strNxOrderID.Right(6);
			// since we modify order id , We need this information when read order id drom transaction response 
			//000000 is not valid order id .. 
			if (strNxOrderID.CollateNoCase("000000") == 0)
			{
				//Since 000000  is not cvalide order number we have to pick any other number whic may not be in rage of current order nuber , considering we are using olny last six digits
				// will use older  id,  we cant use 1 since it can be duplicate,   safer side i am using 9999     
				strNxOrderID = "9999";
			}
			else
			{
				// nothing
			}
		}
		else
		{
			//Will use payment id as order id
		}
		return strNxOrderID;
	}

	//Generalized this because I got tired of writing it all out, and we can more easily
	//	make changes throughout the interface by just changing this function.
	MSXML2::IXMLDOMDocument2Ptr CreateChaseCompatibleXMLDocument()
	{
		//Using version 6.0 of the XML DOM
		MSXML2::IXMLDOMDocument2Ptr pDoc(__uuidof(MSXML2::DOMDocument60));

		//This namespace is applied to most/all of the return elements (and some of the sent elements).  Set it here before
		//	we attempt to do any searches.  Use our own defined namespace - I'm using CP for chase paymentech.
		pDoc->setProperty("SelectionNamespaces", "xmlns:cp='urn:ws.paymentech.net/PaymentechGateway'");

		//We don't want any validation to happen at this point.  Had this on QB, seems to work fine for Chase.
		pDoc->validateOnParse = VARIANT_FALSE;

		return pDoc;
	}

	// (d.thompson 2010-11-18) - PLID 41532 - Reworked to better handle refunds
	bool GetXMLByType(IN Chase_TransTypes cpTransType, OUT CString &strXMLMessage, OUT CString &strFunction,
		//Input Data
		BOOL bIsProduction, CString strUsername, CString strTextPassword, 
		CString strMerchantID, CString strTerminalID,
		long nPaymentID, CString strCCNumber, COleCurrency cyTotalAmount, 
		CString strNameOnCard, CString strMM, CString strYYYY, CString strSecurityCode, CString strAddress, 
		CString strZip, bool bHasTrack2Data, CString strTrack2Data, CString strPreviousTransID)
	{
		switch(cpTransType) {
		case cttAuthCapture:
			{
				strFunction = "NewOrder";		//Use the NewOrder message
				if(!GenerateValidNewOrderXML(strXMLMessage, nPaymentID, strCCNumber, strMM, strYYYY, 
					cyTotalAmount, strNameOnCard, bHasTrack2Data, strTrack2Data,
					strAddress, strZip, strSecurityCode, strUsername, strTextPassword, strMerchantID, strTerminalID, "AC"))
				{
					AfxMessageBox("Failed to generate Charge request.  Please check your input data and try again.");
					return false;
				}
			}
			break;

		case cttRefund:
			{
				//Developers should never pass in the generic "refund" type.  Please specify which refund subtype you want to use.  See
				//	comments in ProcessChaseTransaction_Refund
				AfxThrowNxException("GetXMLByType:  Refund is an invalid type.");
				return false;
			}
			break;

		case cttReversal_IndY:
			{
				strFunction = "Reversal";		//Use the Reversal message
				if(!GenerateValidReversalXML(strXMLMessage, nPaymentID, strPreviousTransID, cyTotalAmount, "Y",
					strUsername, strTextPassword, strMerchantID, strTerminalID))
				{
					AfxMessageBox("Failed to generate refund request.  Please check your input data and try again.");
					return false;
				}
			}
			break;

		case cttReversal_IndN:
			{
				strFunction = "Reversal";		//Use the Reversal message
				if(!GenerateValidReversalXML(strXMLMessage, nPaymentID, strPreviousTransID, cyTotalAmount, "N",
					strUsername, strTextPassword, strMerchantID, strTerminalID))
				{
					AfxMessageBox("Failed to generate refund request.  Please check your input data and try again.");
					return false;
				}
			}
			break;

		case cttRefundMoney:
			{
				strFunction = "NewOrder";		//Use the NewOrder message
				if(!GenerateValidNewOrderXML(strXMLMessage, nPaymentID, strCCNumber, strMM, strYYYY, 
					cyTotalAmount, strNameOnCard, bHasTrack2Data, strTrack2Data,
					strAddress, strZip, strSecurityCode, strUsername, strTextPassword, strMerchantID, strTerminalID, "R"))
				{
					AfxMessageBox("Failed to generate Refund request.  Please check your input data and try again.");
					ClearAuditData();
					return false;
				}
			}
			break;

		case cttPriorAuth:
			{
				// (d.thompson 2010-09-07) - 40314 - This will be done at a later date.  No code should reach this now.
				strFunction = "???";		//Use the ??? message
				AfxThrowNxException("Voice authorization is not currently supported.");
				return false;
			}
			break;

		default:
			{
				//Should be impossible -- only call this with known valid values!
				AfxThrowNxException("GetXMLByType called with invalid type %li", cpTransType);
				return false;
			}
			break;
		}

		return true;
	}

	// (d.thompson 2010-11-18) - PLID 41532 - Process refunds in their own function to deal with the funny logic.  This function
	//	is in charge of devising the proper XML, posting it to chase, and evaluating the success/failure of those POSTs.  This
	//	function should not handle processing a successful return message.
	//cpTransType:	This value will change to reflect the final "successful" type of refund transaction performed.
	//
	bool ProcessChaseTransaction_Refund(IN OUT Chase_TransTypes &cpTransType, OUT CString &strResponseXML,
		//Input data
		BOOL bIsProduction, CString strUsername, CString strTextPassword, 
		CString strMerchantID, CString strTerminalID,
		long nPaymentID, CString strCCNumber, COleCurrency cyTotalAmount, 
		CString strNameOnCard, CString strMM, CString strYYYY, CString strSecurityCode, CString strAddress, 
		CString strZip, bool bHasTrack2Data, CString strTrack2Data, CString strPreviousTransID)
	{
		// (d.thompson 2010-11-17) - PLID 41532 - Discussion of refunds.
		//	Chase provides no simple method, like QBMS, to "void or refund".  It is our job (as the software) to
		//	determine the proper course of action.  So here is what we need to attempt.  Firstly, we need to 
		//	understand how the payment is processed.  It goes like this:
		//(a)  Payment from customer (patient) is charged from merchant (our client).  The processor (Chase) batches 
		//	this on their end, and puts a hold on the issuer account (through the patient's bank).
		//(b)  At end of day (or by manual override), all payments in the processor's hold get "committed".  Payment
		//	is now deducted from the customer and money goes toward the merchant.  There's a bit of lag time here
		//	in how the money moves between accounts, it's not relevant to this discussion.
		//(c)  If I want to cancel a transaction before the "committed" event happens, it is called a void.  This immediately removes
		//	the transaction from the processor's queue.  Chase has an onelineReversalInd, which when set to Y, says
		//	to contact the issuer and remove the hold as well.  2 caveats here for Chase-specifically:
		//	(i)  If you do not send this, then the hold remains on the account for x days (up to 10, depends on issuer).  It
		//	will eventually expire, but the merchant is then charged a misuse fee by Visa for not clearing it.  This is
		//	poor customer service, as the patient has a hold on their account for up to 10 days.
		//	(ii)  Not all banks support this.  If they don't, Chase will reject the entire attempt to void, and nothing 
		//	happens, the money remains claimed.  Need to send the reversal with a 'N' to avoid the "bank doesn't support" issue.
		//	Also, some entire cards don't support this.  JCB and AmEx at least will reject immediately on any attempt.
		//(d)  If I want to cancel a transaction after the commitment, it becomes a normal refund.  This process can take several 
		//	days, and is basically the reverse of the original charge.  You do not want to process a refund if you could instead
		//	process a void, due to the time lag between the money moving around (again, improves customer service).
		//
		//So, in specific, here's how we need to handle all this stuff on our end.
		//1)  We have no way to detect if it's voidable or not.  Attempt to "reverse" (void) the transaction with an 
		//	onlineReversalInd of 'Y'.  This will clear the bank hold if possible and return success.  If the bank does
		//	not support it, we get an entire failure (no voiding happens).
		//2)  If failure, re-run the same "reverse" (void) with an onlineReversalInd of 'N'.  This will void through the processor, 
		//	and not attempt to touch the bank (since we know it failed in step 1).  You can simulate this by using an AmEX or
		//	JCB card number.  These issuers do not support the Y methodology at all and must always go as N.
		//3)  If still failure, then we can assume that the payment cannot be voided.  Go ahead and attempt a refund of the amount
		//	specified.  Refunds are done by issuing a NewOrder with a type of R.

		//Iterate 3 times, using the refund subtypes (4, 5, 6).  Order is important.  Y, N, Refund
		for(int nSubType = 4; nSubType < 7; nSubType++) {
			//First off, generate the XML required
			CString strXMLMessage;
			CString strFunction;
			//Attempt with the current trans type
			cpTransType = (Chase_TransTypes)nSubType;
			if(!GetXMLByType(cpTransType, strXMLMessage, strFunction,
				//Input parameters
				bIsProduction, strUsername, strTextPassword, 
				strMerchantID, strTerminalID, nPaymentID, strCCNumber, cyTotalAmount, 
				strNameOnCard, strMM, strYYYY, strSecurityCode, strAddress, 
				strZip, bHasTrack2Data, strTrack2Data, strPreviousTransID))
			{
				//GetXMLByType will alert the user to the reasoning, we just need to abort
				ClearAuditData();
				return false;
			}

			//Now we've generated valid XML for Chase, let's go ahead and post it to them.
			CString strHTTPStatusText;
			long nHTTPStatus;
			bool bShouldWeAbort = false;
			if(!PostChaseHTTPRequest(strUsername, strTextPassword, GetTransactionRequestURL(bIsProduction), GetTransactionURI(), strFunction, strXMLMessage, 
				strResponseXML, nHTTPStatus, strHTTPStatusText))
			{
				//Special handling for errors.  We fully expect that posting a Reversal may fail for various valid reasons.  But there also may be
				//	failures like network errors, username issues, etc.  So try to detect ONLY the known cases.
				//			882 This transaction is locked down. You cannot mark or unmark it.
				//			9810 Partial online reversals are not allowed.
				//			9811 Online reversals are not allowed for cardtype [AX]  (others too)
				//Unfortunately the code is "SOAP-ENV:Server" and HTTP error is 500, so we can't really parse it any better.
				// (d.thompson 2010-12-20) - PLID 41897 - I found, at random (undocumented), this error.  I read it to be the same thing.  Happens when attempting to void something weeks/months old.
				//			9812 Age of auth is [56072] minutes, max age for online reversal of this method of payment is [4320] minutes
				if(strResponseXML.Find("882 This transaction is locked down") > -1 ||
					strResponseXML.Find("9810 Partial online reversals are not allowed") > -1 ||
					strResponseXML.Find("9811 Online reversals are not allowed for cardtype") > -1 ||
					strResponseXML.Find("9812 Age of auth") > -1)
				{
					//Known condition, somewhat expected.  We should continue onward.
				}
				else {
					//This is an unknown failure, so we'll need to quit where we stand.
					bShouldWeAbort = true;
				}
			}
			else {
				//Our attempt to perform this refund subtype succeeded.  responseXML is filled with the proper XML, and we can quit out successfully.
				return true;
			}

			if(bShouldWeAbort) {
				//Complete failure, alert the user and quit this function
				Chase_HandleGenericError(nHTTPStatus, strHTTPStatusText, strResponseXML);
				return false;
			}
		}

		//We could only get here if the refund type returned a status of "locked down" or "partial reversal not allowed", both of which are supposed
		//	to be impossible for a refund type (the 3rd and last test).  If we happen to reach here, just quit out as failure, and the process will
		//	not happen.
		//This should be impossible to reach.
		AfxMessageBox("Invalid response message when attempting to process a refund.  Please re-enter your refund and try again.");
		return false;
	}

	// (d.thompson 2010-11-17) - PLID 41516 - Added parameters for account information
	// (d.thompson 2010-12-20) - PLID 41895 - Added authorization code (which is not the same as txrefnum)
	bool ProcessChaseTransaction(Chase_TransTypes cpTransType, BOOL bIsProduction, CString strUsername, CString strTextPassword, 
		CString strMerchantID, CString strTerminalID,
		long nPaymentID, CString strCCNumber, COleCurrency cyTotalAmount, 
		CString strNameOnCard, CString strMM, CString strYYYY, CString strSecurityCode, CString strAddress, 
		CString strZip, bool bHasTrack2Data, CString strTrack2Data, CString strPreviousTransID, 
		long nAuditPatientID, CString strAuditPatientName, long nAuditRecordID,
		/* Output Fields */
		OUT CString &strOrderID, OUT CString &strTxRefNum,
		OUT COleDateTime &dtProcDateTime, OUT CString &strProcStatus, OUT CString &strApprovalStatus, OUT CString &strRespCode,
		OUT CString &strAVSRespCode, OUT CString &strCVVRespCode, OUT CString &strRetryTrace, OUT CString &strRetryAttemptCount, 
		OUT COleCurrency &cyReversalAmtVoided, OUT COleCurrency &cyReversalOutstandingAmt, OUT CString& strAuthorizationCode)
	{
		//Auditing.  This is fairly bizarre way of doing it, but it just seems unreasonable to try passing a patient name
		//	and record ID down through the chain of at least 5 functions that have no need for the data.
		// (a.walling 2010-01-25 09:02) - PLID 37026 - Pass in the patient ID
		SetAuditData(strAuditPatientName, nAuditPatientID, nAuditRecordID);


		CString strResponseXML;
		//Special handling for refunds.  They're just a mess, so we'll move their entire logic to another function.
		if(cpTransType == cttRefund) {
			if(!ProcessChaseTransaction_Refund(cpTransType, strResponseXML,
				//Input parameters
				bIsProduction, strUsername, strTextPassword, 
				strMerchantID, strTerminalID, nPaymentID, strCCNumber, cyTotalAmount, 
				strNameOnCard, strMM, strYYYY, strSecurityCode, strAddress, 
				strZip, bHasTrack2Data, strTrack2Data, strPreviousTransID))
			{
				//ProcessChaseTransaction_Refund will alert the user to the reasoning, we just need to abort
				ClearAuditData();
				return false;
			}
		}
		else {
			//Normal transaction types
			//First off, generate the XML required
			CString strXMLMessage;
			CString strFunction;
			//1)  Acquire the XML that we want
			if(!GetXMLByType(cpTransType, strXMLMessage, strFunction,
				//Input parameters
				bIsProduction, strUsername, strTextPassword, 
				strMerchantID, strTerminalID, nPaymentID, strCCNumber, cyTotalAmount, 
				strNameOnCard, strMM, strYYYY, strSecurityCode, strAddress, 
				strZip, bHasTrack2Data, strTrack2Data, strPreviousTransID))
			{
				//GetXMLByType will alert the user to the reasoning, we just need to abort
				ClearAuditData();
				return false;
			}

			//Now we've generated valid XML for Chase, let's go ahead and post it to them.
			CString strHTTPStatusText;
			long nHTTPStatus;
			if(!PostChaseHTTPRequest(strUsername, strTextPassword, GetTransactionRequestURL(bIsProduction), GetTransactionURI(), strFunction, strXMLMessage, 
				strResponseXML, nHTTPStatus, strHTTPStatusText))
			{
				//We only need warn the user about the status if this happens.
				Chase_HandleGenericError(nHTTPStatus, strHTTPStatusText, strResponseXML);
				ClearAuditData();
				return false;
			}
		}	//end "not refund" if statement


		//At this point refunds and normal requests all merge back together to handle the response data


		//Next up, handle the response we got back
		switch(cpTransType) {
			case cttAuthCapture:
				//Parse out the details of the NewOrderResponse message.  Put them in the output variables (sent
				//	back to our caller)
				// (d.thompson 2010-12-20) - PLID 41895 - Added authorizationCode to output
				if(!ParseNewOrderResponse(strResponseXML, strOrderID, strTxRefNum, dtProcDateTime, strProcStatus, strApprovalStatus,
					strRespCode, strAVSRespCode, strCVVRespCode, strRetryTrace, strRetryAttemptCount, strAuthorizationCode))
				{
					ClearAuditData();
					return false;
				}
				break;

			case cttRefund:
				{
					//Developers should never pass in the generic "refund" type.  Please specify which refund subtype you want to use.  See
					//	comments in ProcessChaseTransaction_Refund
					AfxThrowNxException("ProcessChaseTransaction:  Refund is an invalid type.");
					return false;
				}
				break;

			//Both reversal cases are identical with a different parameter.  Return values will not differ.
			case cttReversal_IndY:
			case cttReversal_IndN:
				{
					//Parse our response to fill in the "real" data.
					if(!ParseReversalResponse(strResponseXML, strOrderID, strTxRefNum, dtProcDateTime, strProcStatus, 
						strRetryTrace, strRetryAttemptCount, cyReversalAmtVoided, cyReversalOutstandingAmt, strApprovalStatus, strRespCode))
					{
						ClearAuditData();
						return false;
					}
				}
				break;

			case cttRefundMoney:
				{
					//Parse out the details of the NewOrderResponse message.  Put them in the output variables (sent
					//	back to our caller).
					// (d.thompson 2010-12-20) - PLID 41895 - Added authorization code (not the same as txrefnum)
					if(!ParseNewOrderResponse(strResponseXML, strOrderID, strTxRefNum, dtProcDateTime, strProcStatus, strApprovalStatus,
						strRespCode, strAVSRespCode, strCVVRespCode, strRetryTrace, strRetryAttemptCount, strAuthorizationCode))
					{
						ClearAuditData();
						return false;
					}
				}
				break;

			case cttPriorAuth:
				{
					// (d.thompson 2010-09-07) - PLID 40314 - This will be done at a later date.  No code should reach this now.
					ClearAuditData();
					AfxThrowNxException("Voice authorization is not currently supported.");
					return false;
				}
				break;

			default:
				{
					//This should never happen, please add a case for your new object type.
					AfxThrowNxException("ProcessChaseTransaction:  Unhandled message type");
				}
				break;
		}

		//Success, so audit it
		// (a.walling 2010-01-25 09:05) - PLID 37026 - Use the patient ID
		// (d.thompson 2010-11-11) - PLID 40351 - Renamed to be processor-agnostic events.
		AuditEvent(nAuditPatientID, strAuditPatientName, BeginNewAuditEvent(), aeiProcessingAccepted, nAuditRecordID, "", "Accepted by Chase Paymentech", aepHigh, aetCreated);
		return true;
	}

	CString ChaseCurrencyToImpliedDecimal(COleCurrency cy)
	{
		//Implied decimal format.  From documentation:  "For example $100.00 should be sent as 10000"
		CString str = FormatCurrencyForSql(cy);
		//Will give us a guaranteed US-style format, but not something that looks good.  i.e. $1.00 will come out as "1", but
		//	$1.01 will come out as "1.01", so we have to do a lot of cleanup.

		// (d.thompson 2011-06-30) - PLID 44415 - $1.10 comes back as 1.1, we need to handle that case too.

		//First, look for a decimal
		long nDecPos = str.Find(".");
		if(nDecPos >= 0) {
			//count the chars after the decimal
			if(str.Mid(nDecPos+1).GetLength() == 1) {
				//This is something like $1.10, so we need to add a single 0.
				str += "0";
			}

			//There is a decimal already, do nothing
		}
		else {
			//No decimal, and we must display them, so add the cents
			str += ".00";
		}

		//We now always should have something like dddd.cc, so strip the decimal
		str.Remove('.');

		//Note:  Standard formatting for US does not include $ or , so we don't need to worry about them.
		return str;
	}

	//Will always return a valid currency. Invalid input data will be translated to $0.
	COleCurrency ChaseImpliedDecimalToCurrency(CString str)
	{
		//Implied decimal format.  From documentation:  "For example $100.00 will be sent as 10000"
		//Just a simple string format to insert the decimal before the last 2 digits.
		CString strMoney = str;
		if(strMoney.GetLength() >= 2) {
			strMoney.Insert(strMoney.GetLength() - 2, ".");
		}
		else {
			//Can't be less than 2 digits, just quit out as 0
			return COleCurrency(0, 0);
		}

		//Use ParseFromSql because we're assuming american dollars (Chase is US/Canada only)
		COleCurrency cy = ParseCurrencyFromSql(strMoney);
		if(cy.GetStatus() == COleCurrency::valid) {
			return cy;
		}

		//Invalid currency should always return as 0
		return COleCurrency(0, 0);
	}

	CString GetTextFromNode(MSXML2::IXMLDOMNodePtr pNode)
	{
		if(pNode) {
			return VarString(pNode->text, "");
		}

		//No node, no text
		return "";
	}

	COleDateTime ChaseStringToDate(CString strDate)
	{
		COleDateTime dt;
		dt.SetStatus(COleDateTime::invalid);

		//Expected format:
		//YYYYMMDDhh[24]miss
		//Ensure 14 characters
		if(strDate.GetLength() != 14) {
			return dt;
		}

		//Parse the contents
		dt.SetDateTime(atoi(strDate.Left(4)), atoi(strDate.Mid(4, 2)), atoi(strDate.Mid(6, 2)), atoi(strDate.Mid(8, 2)), atoi(strDate.Mid(10, 2)), atoi(strDate.Mid(12, 2)));
		return dt;
	}

	// (d.thompson 2010-11-17) - PLID 41516 - Requires the production status be passed in now
	CString GetTransactionRequestURL(BOOL bIsProduction)
	{
		if(bIsProduction) {
			//Production
			// (d.thompson 2011-06-16) PLID 44148 - Wrong address!
			/*
				ws.paymentech.net/PaymentechGateway      (Primary)
				ws2.paymentech.net/PaymentechGateway    (Secondary)
			*/
			return GetRemotePropertyText("ChaseProductionURL", "https://ws.paymentech.net/PaymentechGateway", 0, "<None>", true);
		}
		else {
			//Pre-Production
			// (d.thompson 2011-06-16) - PLID 44148 - This address was right, but make it configurable at the same time just in case.
			return GetRemotePropertyText("ChasePreProductionURL", "https://wsvar.paymentech.net/PaymentechGateway", 0, "<None>", true);
		}
	}

	CString GetTransactionURI()
	{
		return "urn:ws.paymentech.net/PaymentechGateway";
	}

	void Chase_HandleXMLError(CString strApprovalStatus, CString strRespCode, CString strAVSRespCode, 
		CString strCVVRespCode, CString strProcStatusMessage)
	{
		//Audit this error
		CString strPatientName;
		long nRecordID;
		long nPatientID;
		// (a.walling 2010-01-25 09:04) - PLID 37026 - Get the patient ID
		if(GetAuditData(strPatientName, nPatientID, nRecordID)) {
			CString strAuditText;
			strAuditText.Format("(%s) %s", strApprovalStatus, strProcStatusMessage);
			// (d.thompson 2010-11-11) - PLID 40351 - Renamed to be processor-agnostic events.
			AuditEvent(nPatientID, strPatientName, BeginNewAuditEvent(), aeiProcessingError, nRecordID, "", strAuditText, aepHigh, aetChanged);
		}

		//When an error is returned, you get the following relevant information.
		//	ProcStatus:  You can ignore this.  It will be 0 if the XML message was accepted.  If there was any refusal
		//		to accept the message, you'd get a SOAP fault instead, not a valid return result.
		//	ApprovalStatus:  1 is accepted.  0 is declined.  2 is "Message/System Error".  I have never seen an example of 2.
		//	ProcStatusMessage:  This is the processor-given reason why the transaction failed.
		//	RespCode:			This is the primary error code.  You can look this up in Appendix 7.2 of the Orbital Interface Spec guide
		//	AVSRespCode:  
		//	CVVRespCode:
		CString strMsg;
		// (d.thompson 2010-11-11) - This number was provided to me by Julie Randolph at Chase.  See email titled "Chase Paymentech Advanced Product Support #"
		strMsg.Format("Your attempt to process this credit card through Chase Paymentech has been declined.  You may need to "
			"contact Chase at 1-800-254-9556 or the credit card issuer to resolve the issue.\r\n\r\n"
			"Reason:  (%s)  %s\r\n\r\n"
			"You may need these details for support purposes:\r\n", strRespCode, strProcStatusMessage);

		//ApprovalStatus.  We know it's not 1 (that's accepted and you don't get here).  It's almost always a 0 (declined).  So only attach it 
		//	if it's not a 0.
		if(strApprovalStatus != "0") {
			strMsg += "ApprovalStatus:  " + strApprovalStatus + "\r\n";
		}
		//AVSRespCode tells us what happened with the address verification.  It may or may not relate to the decline reason.  See Appendix 7.3
		strMsg += "AVSRespCode:  " + strAVSRespCode + "\r\n";
		//CVVRespCode tells us what happened with the CVV match.  This seems to often be blank, and may not apply.  See Appendix 7.4
		strCVVRespCode.TrimRight();
		if(!strCVVRespCode.IsEmpty()) {
			strMsg += "CVVRespCode:  " + strCVVRespCode + "\r\n";
		}

		//Prompt the user about it
		AfxMessageBox(strMsg);
	}

	//Given a URL and Chase XML message, posts the message to that URL.  The response text is returned, as well as the HTTP status and
	//	the status text.
	//This function will return true for any 1xx or 2xx status codes (server accepted the request), and false for anything else.
	// (d.thompson 2010-11-17) - PLID 41516 - Added account details as parameters
	bool PostChaseHTTPRequest(IN const CString strUsername, IN const CString strTextPassword, IN const CString strURL, IN const CString strURI, IN const CString strFunction, 
		IN const CString &strParamXml, OUT CString &strResponseText, OUT long &nStatus, OUT CString &strStatusText)
	{
		try {
			//Note:  The URI is always the same, regardless of production vs pre-production
			//We need to pass TRUE for always getting a response.  Chase will return HTTP 500 errors in any failure
			//	condition, along with valid XML responses.
			MSXML2::IXMLDOMNodePtr pResult = CallSoapFunction(strURL, strURI,
				strFunction, strParamXml, strUsername, strTextPassword, TRUE,
				"", &nStatus, &strStatusText);

			if(pResult != NULL) {
				//Treat as a Document so we can set the namespace
				MSXML2::IXMLDOMDocument2Ptr pResponseDoc = pResult;
				//This namespace is applied to most/all of the return elements (and some of the sent elements).  Set it here before
				//	we attempt to do any searches.
				pResponseDoc->setProperty("SelectionNamespaces", "xmlns:urn='ws.paymentech.net/PaymentechGateway'");
				//Select out the node for the response
				MSXML2::IXMLDOMNodePtr pFunctionResponse = pResponseDoc->selectSingleNode(_bstr_t("/SOAP-ENV:Envelope/SOAP-ENV:Body/" + strFunction + "Response"));

				if(pFunctionResponse) {
					//Put the response in our output variable and let the caller handle it
					strResponseText = VarString(pFunctionResponse->Getxml());
					return true;
				}
				else {
					//Upon failure, we'll ensure the ResponseText is the FAILURE XML, not the success XML.
					strResponseText = VarString(pResult->Getxml());
					return false;
				}
			}
			else {
				//I have no idea how this is possible, I couldn't force it to happen with any manner of bizarre attempts.  Just give a generic
				//	error and we'll have to forcefully abort.
				AfxThrowNxException("NULL response received from %s.  Please check your internet connection and try again.", strURL);
				return false;
			}
		} catch(CSOAPFaultException *e) {
			//SOAP Faults should be quite rare from the above functions and the way we're calling them.  I'm not actually sure if it's even
			//	possible to ever reach this point.  This is all a "just in case" setup.
			//Stole from eligibility and overhauled.
			char errorMessage[4096];
			if (!e->GetErrorMessage(errorMessage, 4096)) {
				//throw a regular exception
				throw e;
			}

			CString strError;
			strError.Format("The Chase Paymentech submission failed with the following error:\n\n"
				"%s\n"
				"Please ensure your Chase Paymentech username and password is correct and that you have access to the internet.\n\n"
				"If this message persists, please contact NexTech Technical Support.", errorMessage);
			AfxMessageBox(strError);

			//cleanup and fail
			e->Delete();
			return false;
		}
	}

	// (d.thompson 2010-11-17) - PLID 41516 - Added parameters for account details
	// (d.thompson 2010-11-18) - PLID 41532 - Renamed, added param for type.  The XML is identical for charges as for refunds, 
	//	with the exception of the type flag.
	//strTransactionType - AC or R currently supported
	bool GenerateValidNewOrderXML(OUT CString &strXMLMessage, long nPaymentID, CString strCCNumber,
		CString strExpMonth, CString strExpFullYear, COleCurrency cyAmount, CString strNameOnCard,
		bool bHasTrack2Data, CString strTrack2Data, CString strAddress, CString strZip, 
		CString strSecurityCode, CString strUsername, CString strTextPassword, CString strMerchantID, 
		CString strTerminalID, CString strTransactionType)
	{
		CString strXML;
		strXML = //"<NewOrder xmlns=\"urn:ws.paymentech.net/PaymentechGateway\"> "		//This is covered in the SOAP generation
			"<newOrderRequest xsi:type=\"ns1:newOrderRequest\" xmlns:ns1=\"urn:ws.paymentech.net/PaymentechGateway\"> ";

		//If you are not doing IP-based security (and we're not), you must provide the username/password as part of the message itself
		strXML += GetXMLElementValuePair("orbitalConnectionUsername", strUsername);
		strXML += GetXMLElementValuePair("orbitalConnectionPassword", strTextPassword);
		//Add all the tags required for the NewOrder message
		strXML += GetXMLElementValuePair("industryType", "RE");		//RE = Retail, no other option
		strXML += GetXMLElementValuePair("transType", strTransactionType);	//AC = Auth & Charge, R = Refund.  [A - PreAuth, FC - Prior/Voice Auth | Not supported]
		strXML += GetXMLElementValuePair("bin", "000002");			//000002 = Tampa, which is the platform we route through, always
		strXML += GetXMLElementValuePair("merchantID", strMerchantID);		//The merchant ID from the settings, assigned by Chase
		strXML += GetXMLElementValuePair("terminalID", strTerminalID);		//The terminal ID from the settings, assigned by Chase

		//Transaction specific data
		strXML += GetXMLElementValuePair("ccAccountNum", strCCNumber);		//Card Number.  Unlike QBMS, they want this here and in the track2 data
		//Chase confirmed this is in the format of YYYYMM (the PDF documentation I received says MMYY)
		strXML += GetXMLElementValuePair("ccExp", strExpFullYear + strExpMonth);	//Exp date, YYYYMM
		// (d.thompson 2010-12-20) - PLID 40314 - ccCardVerifyPresenceInd.  This is another incorrectly documented feature.  Here's the "should be documented" approach:
		//	Only applies to visa & discover, not the others.
		//	1 = Value is filled
		//	2 = Value on card but illegible
		//	9 = Cardholder states data not available
		//
		//HOWEVER... Chase says that the 2/9 values are optional, we can just not send them if there's no CVV.  So this boils down to:
		//	If ccCardVerifyNum is filled, then send 1 on Visa and Discover only, otherwise send nothing.
		if(!strSecurityCode.IsEmpty()) {
			CString strCardType = "", strCD = "";
			//Returns text names
			GetCreditCardTypeFromNumber(strCCNumber, strCardType, strCD);

			//Only flag if we're on discover or visa
			if(strCardType.CompareNoCase("visa") == 0 || strCardType.CompareNoCase("discover") == 0) {
				strXML += GetXMLElementValuePair("ccCardVerifyPresenceInd", "1");
			}
		}
		strXML += GetXMLElementValuePair("ccCardVerifyNum", strSecurityCode);	//Card Verification Number
		strXML += GetXMLElementValuePair("avsZip", strZip);				//Cardholder billing zip code
		strXML += GetXMLElementValuePair("avsAddress1", strAddress);	//Cardholder billing address
		//strXML += GetXMLElementValuePair("avsAddress2", );			//Not supported.  We do not support Addr2, just tack them together
		//strXML += GetXMLElementValuePair("priorAuthCd", "");			//Future:  Only for FC transactions
		//(s.dhole 2/6/2015 11:37 AM ) - PLID  64809 allow max 6 digit order id
		strXML += GetXMLElementValuePair("orderID", GetValidOrderID(nPaymentID));	//Defined by us, echoed back in response.  PaymentID is a good target.
		strXML += GetXMLElementValuePair("amount", ChaseCurrencyToImpliedDecimal(cyAmount));
		//strXML += GetXMLElementValuePair("comments", "");				//Not currently supported.  Merchant comments, shows online
		//strXML += GetXMLElementValuePair("taxInd", "");				//Not currently supported, for PCII data.
		//strXML += GetXMLElementValuePair("taxAmount", "");			//Not currently supported, for PCII data.  Same money format as Amount
		//strXML += GetXMLElementValuePair("pCardOrderID", "");			//Not currently supported, for PCII data.
		//strXML += GetXMLElementValuePair("pCardDestZip", "");			//Not currently supported, for PCII data.

		// (d.thompson 2010-11-12) - PLID 40314 - Here's my understanding of the retryTrace.  We give a unique value (Payment ID seems good)
		//	for this transaction.  Then we submit but get transaction failure.  Practice isn't sure if it succeeded (we got an error), but Chase
		//	might have authorized.  Practice has to record the transaction as a failure, so the next time we send it, we pass in the same value.  This
		//	next time, since the RetryTrace is the same, Chase says "Yep, we already authorized that, here's the info", and it behaves just
		//	like it authorized again.  Without this functionality, our second attempt would re-authorize, and the patient would be charged twice.
		//There is no need to maintain this information (as it will be the payment id) and there is no sharing between payments and refunds.
		strXML += GetXMLElementValuePair("retryTrace", FormatString("%li", nPaymentID));			//RetryTrace value used for Chase-specific "Retry Logic" feature.  See documentation
		//strXML += GetXMLElementValuePair("magStripeTrack1", "");		//Not supported.  Track1 data, if swiped.  Includes element separators, not the begin/end sentinels and LRC chars.
		if(bHasTrack2Data) {
			//Track2 data, if swiped.  Includes element separators, not the begin/end sentinels and LRC chars.  Do not use if Track1 in use.
			// (d.thompson 2010-12-06) - PLID 40314 - Another incorrectly documented feature.  The track2 data must be submitted
			//	in a base64 encoded format, not plain text.
			strXML += GetXMLElementValuePair("magStripeTrack2", CBase64::Encode((unsigned char*)strTrack2Data.GetBuffer(), strTrack2Data.GetLength()));
		}
		strXML += GetXMLElementValuePair("customerName", strNameOnCard);	//NameOnCard
		//CardPresentInd:  According to Chase, this does not mean "is the card present?".  It means, "was the card swiped?"  Basically, if track2 is filled, 
		//	set this to Y, otherwise leave it N.
		CString strPresent = "N";
		if(bHasTrack2Data) {
			strPresent = "Y";
		}
		strXML += GetXMLElementValuePair("cardPresentInd", strPresent);	//Y = Present or swiped.  Either magstripe1 or magstripe2 must be present.  N = manual entry. Neither swipe present.
		strXML += "</newOrderRequest>";
		//</NewOrder>";	//Covered by soap generation

		strXMLMessage = strXML;
		return true;
	}

	//Parses the given XML response, returning the elements marked as OUT parameters.
	//Note:  OUT fields may be partially filled on failure condition.
	// (d.thompson 2010-11-18) - PLID 41532 - This function is identical whether you are doing an AC or a R type
	// (d.thompson 2010-12-20) - PLID 41895 - Added authorization code to output.  Not the same as TxRefNum
	bool ParseNewOrderResponse(const CString strXMLResponse, OUT CString &strOrderID, OUT CString &strTxRefNum,
		OUT COleDateTime &dtProcDateTime, OUT CString &strProcStatus, OUT CString &strApprovalStatus, 
		OUT CString &strRespCode, OUT CString &strAVSRespCode, OUT CString &strCVVRespCode,
		OUT CString &strRetryTrace, OUT CString &strRetryAttemptCount, OUT CString &strAuthorizationCode)
	{
		MSXML2::IXMLDOMDocument2Ptr pResponseDoc = CreateChaseCompatibleXMLDocument();
		if(VARIANT_FALSE == pResponseDoc->loadXML(AsBstr(strXMLResponse))) {
			AfxMessageBox("Failed to parse NewOrderResponse XML.  Please check your source and try again."
				//If we're in debug mode, include the XML!
#ifdef _DEBUG
				+ strXMLResponse
#endif
				);
			return false;
		}

		//IMPORTANT:  0 = success.  Anything else is an error, and a SOAPFault
		strProcStatus = GetTextFromNode(pResponseDoc->selectSingleNode("/cp:NewOrderResponse/cp:return/cp:procStatus"));
		//If ProcStatus = 0, the 'approvalStatus' message text is returned.
		CString strProcStatusMessage = GetTextFromNode(pResponseDoc->selectSingleNode("/cp:NewOrderResponse/cp:return/cp:procStatusMessage"));

		//Select out the node for the response

		//This should be the order we submitted
		strOrderID = GetTextFromNode(pResponseDoc->selectSingleNode("/cp:NewOrderResponse/cp:return/cp:orderID"));
		//Unique value for each transaction.  This is required to void the transaction later.
		strTxRefNum = GetTextFromNode(pResponseDoc->selectSingleNode("/cp:NewOrderResponse/cp:return/cp:txRefNum"));
		//YYYYMMDDhh[24]miss.  When processed by server
		dtProcDateTime = ChaseStringToDate(GetTextFromNode(pResponseDoc->selectSingleNode("/cp:NewOrderResponse/cp:return/cp:respDateTime")));
		//0 = Decline, 1 = Approved, 2 = Message/System Error
		strApprovalStatus = GetTextFromNode(pResponseDoc->selectSingleNode("/cp:NewOrderResponse/cp:return/cp:approvalStatus"));
		//This is the actual error condition if not approved.  See appendix 7.2
		strRespCode = GetTextFromNode(pResponseDoc->selectSingleNode("/cp:NewOrderResponse/cp:return/cp:respCode"));
		//See AVS appendix 7.3
		strAVSRespCode = GetTextFromNode(pResponseDoc->selectSingleNode("/cp:NewOrderResponse/cp:return/cp:avsRespCode"));
		//See appendix 7.4
		strCVVRespCode = GetTextFromNode(pResponseDoc->selectSingleNode("/cp:NewOrderResponse/cp:return/cp:cvvRespCode"));
		// (d.thompson 2010-11-12) - PLID 40314 - This is just an echo of what we sent.  We really don't care what the value is.
		strRetryTrace = GetTextFromNode(pResponseDoc->selectSingleNode("/cp:NewOrderResponse/cp:return/cp:retryTrace"));
		//This gives us a count of the retry attempts.  See my example in the GenerateValidChargeAuthCapture function - in that
		//	case, if this was a re-attempt, we would get a "1" back from this.  Most of the time this should be 0.  It may be
		//	interesting information if it's non-zero.
		strRetryAttemptCount = GetTextFromNode(pResponseDoc->selectSingleNode("/cp:NewOrderResponse/cp:return/cp:retryAttempCount"));
		// (d.thompson 2010-12-20) - PLID 41895 - Added authorizationCode
		strAuthorizationCode = GetTextFromNode(pResponseDoc->selectSingleNode("/cp:NewOrderResponse/cp:return/cp:authorizationCode"));

		//Validate the request
		//This should be guaranteed.  You either get 0 or you got a soap fault, which we catch earlier.
		if(strProcStatus == "0") {
			if(strApprovalStatus == "1") {
				//The one and ONLY success status.  Everything else is a failure.
				//ProcStatus:  This appears to indicate basically "the message was accepted"
				//ApprovalStatus:  Was this message actually approved?  0 = declined, 1 = approved, 2 = message/system error
				//If we get here, everyone is happy and the request was accepted.
			}
			else {
				//This function will handle informing the user, in a nice format, of why the attempt was declined
				Chase_HandleXMLError(strApprovalStatus, strRespCode, strAVSRespCode, strCVVRespCode, strProcStatusMessage);
				return false;
			}
		}
		else {
			//This is completely unexpected, per documentation rules.
			AfxThrowNxException("Unexpected ProcStatus value returned by Chase Paymentech.  Value:  %s.", strProcStatus);
		}
/*
		//Less important fields that we are not parsing
		//industryType		//We don't care, always retail
		//transType			//Don't care, just echo'ing
		//bin				//Don't care, just echo'ing
		//merchantID		//Don't care, just echo'ing
		//terminalID		//Don't care, just echo'ing
		//cardBrand			//We don't care at this point, we already knew when submitting
		//txRefIdx			//Always 0 on trans and 1 on void.  We don't care.  [EDIT:  Their cert team tells me it's 0 for auth, 1 for MFC, 2 for void]
		//hostRespCode		//Actual code from host.  This looks to be legacy.
		//hostAVSRespCode	//Legacy
		//hostCVVRespCode	//Legacy
		//lastRetryDate		//YYYYMMDDhh[24]miss.  Date/time the PREVIOUS transaction with same retryTrace was processed.  We don't need this.
*/
		return true;
	}

	// (d.thompson 2010-11-17) - PLID 41516 - Added account details as parameters
	bool GenerateValidReversalXML(CString &strXMLMessage, long nPaymentID, CString strPreviousTransID, COleCurrency cyTotalAmount, CString strReversalInd,
		CString strUsername, CString strTextPassword, CString strMerchantID, CString strTerminalID)
	{
		CString strXML;
		strXML = //"<Reversal xmlns=\"urn:ws.paymentech.net/PaymentechGateway\"> "		//This is covered in the SOAP generation
			"<reversalRequest xsi:type=\"ns1:reversal\" xmlns:ns1=\"urn:ws.paymentech.net/PaymentechGateway\"> ";

		//If you are not doing IP-based security (and we're not), you must provide the username/password as part of the message itself
		strXML += GetXMLElementValuePair("orbitalConnectionUsername", strUsername);
		strXML += GetXMLElementValuePair("orbitalConnectionPassword", strTextPassword);
		//Add all the tags required for the Reversal message
		strXML += GetXMLElementValuePair("bin", "000002");			//000002 = Tampa, which is the platform we route through, always
		strXML += GetXMLElementValuePair("merchantID", strMerchantID);		//The merchant ID from the settings, assigned by Chase
		strXML += GetXMLElementValuePair("terminalID", strTerminalID);		//The terminal ID from the settings, assigned by Chase

		//Transaction specific data
		//(s.dhole 2/6/2015 11:38 AM ) - PLID 64809 allow max 6 digit order id
		strXML += GetXMLElementValuePair("orderID", GetValidOrderID(nPaymentID));	//Defined by us, echoed back in response.  PaymentID is a good target.
		strXML += GetXMLElementValuePair("txRefNum", strPreviousTransID);				//The transaction reference number returned by the original authorization
		strXML += "<txRefIdx />\r\n";		//Documentation says this should always fill NULL, but it is mandatory

		//strXML += GetXMLElementValuePair("version", );
		strXML += GetXMLElementValuePair("adjustedAmount", ChaseCurrencyToImpliedDecimal(cyTotalAmount));
		//strXML += GetXMLElementValuePair("reversalRetryNumber", );
		// (d.thompson 2010-11-18) - PLID 41532 - This flag indicates if the processor (Chase) should contact the bank and take
		//	the hold off their account.  It should really always be Y, except in the rare cases where a bank doesn't support it.
		strXML += GetXMLElementValuePair("onlineReversalInd", strReversalInd);

		// (d.thompson 2010-11-12) - PLID 40314 - See GenerateChargeXML function for description.  We're going to just use the Payment ID here.
		// (d.thompson 2010-11-18) - PLID 41532 - The existence of retryTrace on the reversals is causing problems making a refund afterward
		//		See refund logic, but we'll just leave this out on the reversal attempt.
		//strXML += GetXMLElementValuePair("retryTrace", FormatString("%li", nPaymentID));		//RetryTrace value used for Chase-specific "Retry Logic" feature.  See documentation
		strXML += "</reversalRequest>";//</Reversal>";	//Covered by soap generation
		strXMLMessage = strXML;

		return true;
	}

	//Given a response XML document, fills the parameters marked as OUT.  If the return value
	//	is false, no parameters are filled.
	bool ParseReversalResponse(const CString strResponseXML, OUT CString &strOrderID, OUT CString &strTxRefNum, 
			OUT COleDateTime &dtProcDateTime, OUT CString &strProcStatus, OUT CString &strRetryTrace, OUT CString &strRetryAttemptCount, 
			OUT COleCurrency &cyReversalAmtVoided, OUT COleCurrency &cyReversalOutstandingAmt, OUT CString &strApprovalStatus, OUT CString &strRespCode)
	{
		MSXML2::IXMLDOMDocument2Ptr pResponseDoc = CreateChaseCompatibleXMLDocument();
		if(VARIANT_FALSE == pResponseDoc->loadXML(AsBstr(strResponseXML))) {
			AfxMessageBox("Failed to parse ReversalResponse XML.  Please check your source and try again."
				//If we're in debug mode, include the XML!
#ifdef _DEBUG
				+ strResponseXML
#endif
				);
			return false;
		}

		//IMPORTANT:  0 = success.  Anything else is an error, and a SOAPFault
		strProcStatus = GetTextFromNode(pResponseDoc->selectSingleNode("/cp:ReversalResponse/cp:return/cp:procStatus"));
		//If ProcStatus = 0, the 'approvalStatus' message text is returned.
		CString strProcStatusMessage = GetTextFromNode(pResponseDoc->selectSingleNode("/cp:ReversalResponse/cp:return/cp:procStatusMessage"));

		//0 = Decline, 1 = Approved, 2 = Message/System Error
		strApprovalStatus = GetTextFromNode(pResponseDoc->selectSingleNode("/cp:NewOrderResponse/cp:return/cp:approvalStatus"));
		//This is the actual error condition if not approved.  See appendix 7.2
		strRespCode = GetTextFromNode(pResponseDoc->selectSingleNode("/cp:NewOrderResponse/cp:return/cp:respCode"));
		//No AVS or CVV resp happens for reversals

		//Amount remaining on the original charge that could be voided in the future
		//Time it was processed.  YYYYMMDDHH[24]miss.
		dtProcDateTime = ChaseStringToDate(GetTextFromNode(pResponseDoc->selectSingleNode("/cp:ReversalResponse/cp:return/cp:respDateTime")));
		//This should be the order we submitted
		strOrderID = GetTextFromNode(pResponseDoc->selectSingleNode("/cp:ReversalResponse/cp:return/cp:orderID"));
		//Unique value for each transaction.  This is required to void the transaction later.
		strTxRefNum = GetTextFromNode(pResponseDoc->selectSingleNode("/cp:ReversalResponse/cp:return/cp:txRefNum"));

		// (d.thompson 2010-11-12) - PLID 40314 - This is just an echo of what we sent.  We really don't care what the value is.
		strRetryTrace = GetTextFromNode(pResponseDoc->selectSingleNode("/cp:ReversalResponse/cp:return/cp:retryTrace"));
		//This gives us a count of the retry attempts.  See my example in the GenerateValidChargeAuthCapture function - in that
		//	case, if this was a re-attempt, we would get a "1" back from this.  Most of the time this should be 0.  It may be
		//	interesting information if it's non-zero.
		strRetryAttemptCount = GetTextFromNode(pResponseDoc->selectSingleNode("/cp:ReversalResponse/cp:return/cp:retryAttempCount"));

		// (d.thompson 2010-11-19) - PLID 41532 - This seems to frequently be empty in my test cases.  I'm going to go ahead and try to 
		//	parse it anyways, in the hopes that it gets filled in someday.  It is in the WSDL.
		CString strVoidOutstandingAmount = GetTextFromNode(pResponseDoc->selectSingleNode("/cp:ReversalResponse/cp:return/cp:outstandingAmt"));
		cyReversalOutstandingAmt = ChaseImpliedDecimalToCurrency(strVoidOutstandingAmount);
		//This looks useful, but none of my test messages ever actually return them to me.
		//strVoidAmount = GetTextFromNode(pResponseDoc->selectSingleNode("/cp:ReversalResponse/cp:return/cp:Amount"));

		//Validate the request
		if(strProcStatus == "0") {
			//The one and ONLY success status.  Everything else is a failure.
		}
		else {
			//Handle error conditions here
			Chase_HandleXMLError(strApprovalStatus, strRespCode, "", "", strProcStatusMessage);
			return false;
		}
/*
		//Less important fields that we are not parsing
		//bin				//Don't care, just echo'ing
		//merchantID		//Don't care, just echo'ing
		//terminalID		//Don't care, just echo'ing
		//txRefIdx			//Always 0 on trans and 1 on void.  We don't care.  [EDIT:  Their cert team tells me it's 0 for auth, 1 for MFC, 2 for void]
		//lastRetryDate		//YYYYMMDDhh[24]miss.  Date/time the PREVIOUS transaction with same retryTrace was processed.  We don't need this.
		
*/
		return true;
	}

	//This function will attempt to handle, as best it can, a variety of possible error states.  We cannot
	//	differentiate between HTTP errors and XML errors, because Chase sends an HTTP error (often 412 or 500)
	//	when there is a failure with the XML.  XML errors seem to most often come back as SOAP faults
	void Chase_HandleGenericError(long nHTTPStatus, CString strHTTPStatusText, CString strResponseXML)
	{
		//Sample Error, caused by invalid content in the XML
		/*
		<?xml version="1.0"?>
		<SOAP-ENV:Envelope xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope/" xmlns:SOAP-ENC="http://schemas.xmlsoap.org/soap/encoding/" 
			xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:xsd="http://www.w3.org/2001/XMLSchema" 
			xmlns:ns="urn:ws.paymentech.net/PaymentechGateway">
			<SOAP-ENV:Body id="_0">
				<SOAP-ENV:Fault>
					<faultcode>SOAP-ENV:Server</faultcode>
					<faultstring> 9719 Invalid Date Length: Format is YYYYMM</faultstring>
				</SOAP-ENV:Fault>
			</SOAP-ENV:Body>
		</SOAP-ENV:Envelope>
		*/
		CString strMsg = "An error has occurred while attempting to process your payment:\r\n";
		//I've decided to explicitly ignore the error status of a 500.  All of the "normal" XML errors come back saying "500 - Internal Server Error", 
		//	when in fact there's no "server error" outside of the XML validation.  I feel like I'll just be constant reports that say "so and so 
		//	called and said they got a server error".
		if(nHTTPStatus != 500) {
			//If it's not a 500 error, then we want to know more about it.
			strMsg += FormatString("Status:  %li - %s\r\n", nHTTPStatus, strHTTPStatusText);
		}

		//Next, evaluate the XML that we got in response.  This is typically similar to the example above.
		if(!strResponseXML.IsEmpty()) {
			bool bAppendFullXML = false;
			//Parse the XML into a document and look for a soapfault.  We'll output the info
			MSXML2::IXMLDOMDocument2Ptr pDoc = CreateChaseCompatibleXMLDocument();
			if(pDoc->loadXML(_bstr_t(strResponseXML))) {
				MSXML2::IXMLDOMNodePtr nodeErrCode = pDoc->selectSingleNode("//faultcode");
				MSXML2::IXMLDOMNodePtr nodeErrText = pDoc->selectSingleNode("//faultstring");
				CString strCode, strErrText;
				if(nodeErrCode != NULL) {
					strCode = GetTextFromNode(nodeErrCode);
				}
				if(nodeErrText != NULL) {
					strErrText = GetTextFromNode(nodeErrText);
				}

				if(!strCode.IsEmpty() && !strErrText.IsEmpty()) {
					//Add this to the error message
					strMsg += "A SOAP error has occurred:  (" + strCode + ")  " + strErrText + "\r\n";
				}
				else {
					//Not a SOAP fault, something else.  If we have known error states, add else if statements
					//	here to capture them.  Otherwise, unknown ones will just append the full XML.
					bAppendFullXML = true;
				}
			}
			else {
				//Invalid XML to load into a document, so just output as is so we can figure it out manually.
				bAppendFullXML = true;
			}

			if(bAppendFullXML) {
				//Despite what happened above, we've been asked to append the full XML to the error string.
				strMsg += "\r\nResponse Text:  " + strResponseXML;
			}
		}

		//Special:  Chase likes to give a 412 error with no XML in some cases.  This most often
		//	means that the username and password are incorrect.  Let's give a much more sensible message.
		if(nHTTPStatus == 412) {
			strMsg += "\r\n\r\nThe most likely cause for this error is due to an incorrect username or password.  Please check "
				"that your username and password are entered correctly and try submitting again.";
		}

		//Log this error
		Log("%s", strMsg);

		//Prompt the user about it
		AfxMessageBox(strMsg);
	}

}	//end namespace

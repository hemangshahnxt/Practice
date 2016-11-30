//QBMSProcessingUtils.cpp
//
// (d.thompson 2009-06-22) - PLID 34688 - QBMS replaces IGS processing, copied the old file entirely.

#include "stdafx.h"
#include "QBMSProcessingUtils.h"
#include "SOAPUtils.h"
#include "GlobalAuditUtils.h"
#include "AuditTrail.h"
#include "GlobalFinancialUtils.h"
using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37026 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



// (a.walling 2009-10-13 10:01) - PLID 35930
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif




#pragma region CQBMSProcessingAccount definition

// (d.thompson 2010-11-04) - PLID 40628 - Implemented CQBMSProcessingAccount class
CQBMSProcessingAccount::CQBMSProcessingAccount()
{
	bModified = false;
}

void CQBMSProcessingAccount::LoadFromData(long _nID, CString _strDesc, bool _bInactive, bool _bIsProd, _variant_t _varTicket)
{
	nID = _nID;
	strDescription = _strDesc;
	bInactive = _bInactive;
	bIsProduction = _bIsProd;
	varEncryptedTicket = _varTicket;

	bModified = false;
}

void CQBMSProcessingAccount::LoadAsNewAccount(long _nID)
{
	nID = _nID;
	strDescription = "QuickBooks Merchant Services";
	bInactive = false;
	bIsProduction = false;
	SetAndEncryptTicket("", true);

	//Since it's new, we need to flag it as modified
	bModified = true;
}

void CQBMSProcessingAccount::SetDescription(CString _strDesc)
{
	//ensure an actual change happened
	if(strDescription == _strDesc) {
		return;
	}

	bModified = true;
	strDescription = _strDesc;
}

CString CQBMSProcessingAccount::GetDescription()
{
	return strDescription;
}

void CQBMSProcessingAccount::SetInactive(bool _bInactive /*= true*/)
{
	//Ensure an actual change happened
	if(bInactive == _bInactive) {
		return;
	}

	bModified = true;
	bInactive = _bInactive;
}

bool CQBMSProcessingAccount::GetIsInactive()
{
	return bInactive;
}

void CQBMSProcessingAccount::SetProduction(bool _bProd /*= true*/)
{
	//Ensure an actual change happened
	if(bIsProduction == _bProd) {
		return;
	}

	bModified = true;
	bIsProduction = _bProd;

	//Changing the production status ALWAYS resets the connection ticket
	SetAndEncryptTicket("");
}

bool CQBMSProcessingAccount::GetIsProduction()
{
	return bIsProduction;
}

//bForceOverride allows you to force a change in the ticket (and flags as modified) regardless of the check
//	to see if the ticket has changed.
void CQBMSProcessingAccount::SetAndEncryptTicket(CString strNewTicket, bool bForceOverride /*= false*/)
{
	//Ensure an actual change happened
	if(!bForceOverride && strNewTicket == GetDecryptedTicket()) {
		return;
	}

	bModified = true;

	//Use existing encryption functions
	varEncryptedTicket = EncryptStringToVariant(strNewTicket);
}

VARIANT CQBMSProcessingAccount::GetEncryptedTicketForDatabase()
{
	//Copy the member variant, detach it, and return it
	_variant_t varCopy = varEncryptedTicket;
	VARIANT variantTicket = varCopy.Detach();

	//Cleanup memory used by copy
	VariantClear(&varCopy);

	return variantTicket;
}

CString CQBMSProcessingAccount::GetDecryptedTicket()
{
	//Use decryption routines to get the string form
	return DecryptStringFromVariant(varEncryptedTicket);
}

long CQBMSProcessingAccount::GetID()
{
	return nID;
}

//This should only be called if you've saved a new record to data.  Existing 
//	records should never have their ID changed.  This function will NOT flag
//	the account as being modified.
void CQBMSProcessingAccount::ChangeIDAfterSave(long nNewID)
{
	if(nID > 0) {
		//You REALLY should not be changing existing saved IDs, they are linked to other data.  Are you 
		//	sure this is what you intended?
		ASSERT(FALSE);
	}

	nID = nNewID;
}

bool CQBMSProcessingAccount::GetIsModified()
{
	return bModified;
}

//Should be called after the account is saved, and is no longer considered modified.
void CQBMSProcessingAccount::SetNotModified()
{
	bModified = false;
}

#pragma endregion


namespace QBMSProcessingUtils {
	//This is the DATETIMETYPE format required by some XML elements
	CString FormatDateTimeAsQBDateTimeType(COleDateTime dt)
	{
		//YYYY-MM-DDTHH:MM:SS
		return dt.Format("%Y-%m-%dT%H:%M:%S");
	}

	//Reverse of the above
	COleDateTime QBMSDateTimeTypeToDateTime(CString strQBDateTimeType)
	{
		COleDateTime dt;
		//Guaranteed to be 10-char date + T + 8 character time.  Just grab the ends
		dt.ParseDateTime(strQBDateTimeType.Left(10) + " " + strQBDateTimeType.Right(8));
		return dt;
	}

	//Given a URL and QBMS XML message, posts the message to that URL.  The response text is returned, as well as the HTTP status and
	//	the status text.
	//This function will return true for any 1xx or 2xx status codes (server accepted the request), and false for anything else.
	bool PostQBMSHTTPRequest(IN const CString &strURL, IN const CString &strParamXml, OUT CString &strResponseText, OUT long &nStatus, 
		OUT CString &strStatusText)
	{
		MSXML2::IXMLHTTPRequestPtr req("Msxml2.XMLHTTP.6.0");
		req->open("POST", AsBstr(strURL), false);

		//Required header info for all QBMS messages
		req->setRequestHeader("content-type", "application/x-qbmsxml");

		//Configure the XML to be the body of the message.
		_bstr_t bstrXml = AsBstr(strParamXml);

		//Actually submit the request to the server
		req->send(bstrXml);

		//Status of the request
		nStatus = req->status;
		strStatusText = VarString(req->statusText);

		//Get a response back
		strResponseText = VarString(req->responseText);


		// (d.thompson 2010-06-09) - No PLID - Do NOT ever make this live, even by option.  PCI compliance requires we
		//	never log/etc this data.  However if you have a problem and must see it in action, you can build an exe
		//	with these code commented out and upload it / run it manually on the client's computer.  I leave it here
		//	to save having to run it often.
		/*AfxMessageBox("PostQBMSHTTPRequest has been submitted to " + strURL + "\r\n\r\n"
			"XML Param Content:\r\n" + strParamXml +
			"\r\n\r\n\r\n"
			"Status Code:  " + FormatString("%li", nStatus) + "\r\n"
			"Status:  " + strStatusText + "\r\n"
			"Response:\r\n" + strResponseText + "\r\n");
		*/

		//Return values are based on RFC 2616 (http://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html).  Values of 1xx (continue)
		//	and 2xx (success) are considered "pass".  Everything else is considered a failure.
		if(nStatus >= 300) {
			return false;
		}
		else {
			return true;
		}
	}

#pragma region AuditData
	// (d.thompson 2009-07-01) - PLID 34230
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

#pragma region QBMS Error Handling

	void QBMS_HandleXMLError(CString strStatusCode, CString strSeverity, CString strMessage)
	{
		//Audit this error
		CString strPatientName;
		long nRecordID;
		long nPatientID;
		// (a.walling 2010-01-25 09:04) - PLID 37026 - Get the patient ID
		if(GetAuditData(strPatientName, nPatientID, nRecordID)) {
			CString strAuditText;
			strAuditText.Format("(%s) (%s) %s", strStatusCode, strSeverity, strMessage);
			// (d.thompson 2010-11-11) - PLID 40351 - Renamed to be processor-agnostic events.
			AuditEvent(nPatientID, strPatientName, BeginNewAuditEvent(), aeiProcessingError, nRecordID, "", strAuditText, aepHigh, aetChanged);
		}

		CString strMsg;
		strMsg.Format("A message has been returned while processing your transaction through QuickBooks Merchant Services.  "
			"You may need to contact Merchant Services at (800) 397-0707 or the credit card issuer to resolve the issue.\r\n\r\n"
			"Message:  %s\r\n\r\n"
			"You may need these details for support purposes:\r\n"
			"Code:  %s\r\n"
			"Severity:  %s"
			, strMessage, strStatusCode, strSeverity);

		//Prompt the user about it
		AfxMessageBox(strMsg);
	}

	void QBMS_HandleHTTPError(long nHTTPStatus, CString strHTTPStatusText, CString strResponseXML)
	{
		CString strMsg;
		strMsg.Format("An HTTP error occurred in QuickBooks Merchant Services processing:\r\n"
			"Status:  %li - %s\r\n\r\n"
			"Response Text:  %s", nHTTPStatus, strHTTPStatusText, strResponseXML);

		//Log this error
		Log("%s", strMsg);

		//Prompt the user about it
		AfxMessageBox(strMsg);
	}

#pragma endregion

#pragma region QBMS GeneralXMLGenerationFunctions

	//Generalized this because I got tired of writing it all out, and we can more easily
	//	make changes throughout the interface by just changing this function.
	MSXML2::IXMLDOMDocument2Ptr CreateQBMSCompatibleXMLDocument()
	{
		//Using version 6.0 of the XML DOM
		MSXML2::IXMLDOMDocument2Ptr pDoc(__uuidof(MSXML2::DOMDocument60));
		//XML6 prohibits DTD by default.  QB uses DTD, so we must turn off that prohibition
		pDoc->setProperty("ProhibitDTD", VARIANT_FALSE);
		//We don't want any validation to happen at this point.
		pDoc->validateOnParse = VARIANT_FALSE;

		return pDoc;
	}

	CString GenerateQBMSXMLHeader()
	{
		//We're going with version 3.0 of QBMS XML.
		return "<?xml version=\"1.0\"?><?qbmsxml version=\"3.0\"?><QBMSXML>";
	}

	CString GenerateQBMSXMLFooter()
	{
		return "</QBMSXML>";
	}

	// (d.thompson 2010-06-21) - PLID 39272 - Revised.  This function should no longer be necessary. Unless directly specifying
	//	'Session Auth' setups, there is no reason to post the session ticket to QBMS at all, we can avoid it.
	//I will leave this commented out -- if we support session auth in the future, you'll want to re-enable this.
	/*
	CString GenerateSignonTicket()
	{
		//I implemented this as an exception because we need to fail.  If there's an issue getting the 
		//	session ticket, we probably already warned the user, but we need to make sure it fails entirely.  This
		//	is most likely a configuration issue, so it doesn't hurt to notify support.  Note that if you get 
		//	this and are trying to debug it, the user should have been previously warned about a failure state
		CString strTicket;
		if(!GetCurrentQBMSSessionTicket(strTicket)) {
			AfxThrowNxException("Failed to get session ticket for signon header!");
		}

		CString strXML = "<SignonMsgsRq><SignonTicketRq>"
			+ GetXMLElementValuePair("ClientDateTime", FormatDateTimeAsQBDateTimeType(COleDateTime::GetCurrentTime()))
			+ GetXMLElementValuePair("SessionTicket", strTicket)
			+ "</SignonTicketRq></SignonMsgsRq>";

		return strXML;
	}
	*/

#pragma endregion

	// (d.thompson 2010-06-21) - PLID 39272 - This retrieves the "request for ticket" XML, using the connection ticket.
	// (d.thompson 2010-11-08) - PLID 41367 - Since we now have to support multiple QBMS accounts, the connection ticket
	//	will need to be passed down to us.
	CString GetQBMSSignonMsgsRqDesktopRqXML(CString strConnectionTicket)
	{
		CString strXML = 
			"<SignonMsgsRq><SignonDesktopRq>"
			"<ClientDateTime>" + FormatDateTimeAsQBDateTimeType(COleDateTime::GetCurrentTime()) + "</ClientDateTime><ApplicationLogin>" 
			+ GetAppLogin() + "</ApplicationLogin>"
			"<ConnectionTicket>" + strConnectionTicket + "</ConnectionTicket></SignonDesktopRq></SignonMsgsRq>";

		return strXML;
	}

	//This can only ever be saved in memory.  Cache it there once per session.  Do not
	//	EVER save it to data.
	// (d.thompson 2010-11-08) - PLID 41367 - This code should have been removed when PLID 39272 was implemented.  We no longer 
	//	track or use a session ticket.  We need only provide the connection ticket in XML for each submission to QBMS.  Commented
	//	out until we decide to support session authentication.
	/*
	CString l_strSessionTicket;
	bool GetCurrentQBMSSessionTicket(CString &strSessionTicket)
	{
		//In the future (PLID 34732) we may decide to support login security.  I believe we can probably just launch a dialog
		//	from here to get the transaction ticket each time.

		if(l_strSessionTicket.IsEmpty()) {
			//If it's not yet loaded, we need to acquire it.  These are steps 4-7 in the QBMS Developer Guide
			//	"Posting qbmsXML to QBMS (No Session Authentication)" section.
			CString strXML = GenerateQBMSXMLHeader() + 
				GetQBMSSignonMsgsRqDesktopRqXML() +
				+ GenerateQBMSXMLFooter();

			//We now have the XML with the request for a session ticket
			CString strResponseXML, strHTTPStatusText;
			long nHTTPStatus;
			if(!PostQBMSHTTPRequest(GetTransactionRequestURL(), strXML, strResponseXML, nHTTPStatus, strHTTPStatusText)) {
				//We need to handle HTTP failures differently, because the status text contains error information.
				QBMS_HandleHTTPError(nHTTPStatus, strHTTPStatusText, strResponseXML);
				return false;
			}

			//The response returned is XML, so let's load a document and get the info we need back.
			MSXML2::IXMLDOMDocument2Ptr pDoc = CreateQBMSCompatibleXMLDocument();

			if(pDoc->loadXML(_bstr_t(strResponseXML)) == VARIANT_FALSE) {
				AfxMessageBox("Failed to load response XML during session negotiation.  Response contents:\r\n\r\n" + strResponseXML);
				return false;
			}

			// (d.thompson 2010-06-11) - PLID 39124 - Improve error handling.
			MSXML2::IXMLDOMNodePtr pRoot = FindChildNode(pDoc->GetdocumentElement(), "SignonMsgsRs");
			if(pRoot == NULL) {
				//No root element??
				CString str = "GetCurrentQMBSSessionTicket:  Bad response.\r\n\r\n"
					"Contents:  " + strResponseXML;
				AfxMessageBox(str);
				//For current purposes, log this.
				Log("%s", str);
				return false;
			}


			//Misread this on the first go.  0 does not mean success -- there are warning events or info events with 
			//	StatusCode values, but the transaction succeeds. What we really need to consider is a severity of "ERROR"
			CString strSeverity = GetTextFromXPath(pDoc->GetdocumentElement(), "//QBMSXML/SignonMsgsRs/SignonDesktopRs/@statusSeverity");
			CString strStatusCode = GetTextFromXPath(pDoc->GetdocumentElement(), "//QBMSXML/SignonMsgsRs/SignonDesktopRs/@statusCode");
			CString strMessage = GetTextFromXPath(pDoc->GetdocumentElement(), "//QBMSXML/SignonMsgsRs/SignonDesktopRs/@statusMessage");

			// (d.thompson 2010-06-11) - PLID 39124 - For safety, check the string value, not the atoi() value.  We don't want anything
			//	that isn't exactly "0".
			if(!strStatusCode.IsEmpty() && strStatusCode == "0") {
				//This is a generic "Success OK" Message.  We don't need to alert the user about it.
			}
			else if(atoi(strStatusCode) == 2020) {
				//A response of 2020 means "Session Authentication is required".  This is an alternate login mode, please
				//	see the QBMS devguide for more details (page 58 roughly).  We are not currently supporting this.  The
				//	client can request a new connection ticket with login security disabled, put it in their settings, and
				//	use that mode instead.
				//PLID 34732 is to consider supporting this mode.
				CString strMsg = "Session authentication mode is not currently supported.  Please open the Credit Card Settings "
					"options, click the button to request a connection ticket, and ensure that session authentication is disabled.  "
					"The new ticket number will then allow you to post credit card transactions.";
				//Log this
				Log(strMsg);
				AfxMessageBox(strMsg);
				return false;
			}
			else {
				//Any other kind of message we want to let the user know what happened.
				QBMS_HandleXMLError(strStatusCode, strSeverity, strMessage);

				//Additionally, if this was an ERROR condition, we need to abort entirely.
				// (d.thompson 2010-06-11) - PLID 39124 - Check specifically for known successes, anything else is
				//	failure.  At this point, "WARN" (rare, but possible) or "INFO" (most common) are most of our cases.
				if(strSeverity.CompareNoCase("WARN") != 0 && strSeverity.CompareNoCase("INFO") != 0 ) {
					return false;
				}
			}

			//Severity was not error, so we should have a ticket.
			l_strSessionTicket = GetTextFromXPath(pDoc->GetdocumentElement(), "//QBMSXML/SignonMsgsRs/SignonDesktopRs/SessionTicket");
		}

		//It's loaded, set the output value and return success.
		strSessionTicket = l_strSessionTicket;
		return true;
	}
	*/

#pragma region QBMS CustomerCreditCardCharge Functions

	// (d.thompson 2010-11-08) - PLID 41367 - Since we now support multiple QBMS accounts, the connection ticket will need passed down to us.
	bool GenerateValidChargeRqXML(IN CString strConnectionTicket, OUT CString &strXMLMessage, long nPaymentID, CString strCCNumber,
		CString strExpMonth, CString strExpYear, COleCurrency cyAmount, CString strNameOnCard,
		bool bHasTrack2Data, CString strTrack2Data, bool bCardPresent, CString strAddress, CString strZip, 
		CString strSecurityCode)
	{
		strXMLMessage = GenerateQBMSXMLHeader();

		//Every message will have the signon information first.
		// (d.thompson 2010-06-21) - PLID 39272 - This is no longer needed.  See next line.
		//strXMLMessage += GenerateSignonTicket();

		// (d.thompson 2010-06-21) - PLID 39272 - Instead of a signon (HTTP POST then parse result), we are just
		//	going to send the request at the same time.  Intuit informs me (https://ipp.developer.intuit.com/sdk/qbms/Documentation/Sending_Requests)
		//	that you can safely do this.  You will get a SessionTicket in response, but you can just ignore it.
		// (d.thompson 2010-11-08) - PLID 41367 - Since we now support multiple QBMS accounts, pass along the
		//	connection ticket we were requested to use.
		strXMLMessage += GetQBMSSignonMsgsRqDesktopRqXML(strConnectionTicket);

		//Now for the code specific to this message
		strXMLMessage += "<QBMSXMLMsgsRq><CustomerCreditCardChargeRq>" + 
			//This is just required to be a unique value.  For now I'm going to use the payment ID.  I think this should
			//	be unique for our purposes.
			// (d.thompson 2010-06-01) - PLID 38445 - Guess I was wrong.  This is a unique submission ID, not payment ID.  If 
			//	you submit, and get failure, you can't resubmit the same ID again for 5 minutes.  So we need to better unique-ify
			//	this value.  I'm going to tack on the tickcount.
			GetXMLElementValuePair("TransRequestID", FormatString("%li_%lu", nPaymentID, GetTickCount()));

		// (a.walling 2010-04-26 16:27) - PLID 38370 - Sanity check for valid track 2 data
		bool bTrack2DataValid = false;
		// (a.walling 2010-04-26 16:23) - PLID 38370 - The Track2Data that is sent needs to include the start and end sentinels.
		CString strFinalTrack2Data;

		//If we have Track2 Data, we use that in place of much of the other information
		if(bHasTrack2Data && !strTrack2Data.IsEmpty()) {
			if (::isdigit((unsigned char)strTrack2Data.GetAt(0))) {
				// begins with a digit, we must need the beginning sentinel
				strFinalTrack2Data += ";";
			}
			strFinalTrack2Data += strTrack2Data;
			if (::isdigit((unsigned char)strTrack2Data.GetAt(strTrack2Data.GetLength() - 1))) {
				// ends with a digit, so we must not have the end sentinel either.
				strFinalTrack2Data += "?";
			}

			// Now we have our final track2 data. Let's double check the length.

			if (strFinalTrack2Data.GetLength() < 23 || strFinalTrack2Data.GetLength() > 37) {
				bTrack2DataValid = false;
			} else {
				bTrack2DataValid = true;
			}
		} else {
			bTrack2DataValid = false;
		}

		if (bTrack2DataValid) {
			strXMLMessage += GetXMLElementValuePair("Track2Data", strFinalTrack2Data);
		} else {
			//If not, we have to pass all the info individually		
			strXMLMessage += GetXMLElementValuePair("CreditCardNumber", strCCNumber) +
				GetXMLElementValuePair("ExpirationMonth", strExpMonth) +
				GetXMLElementValuePair("ExpirationYear", strExpYear) +
				//Note:  This is ORd with ECommerce or IsRecurring in the SDK, but we support neither of those options
				GetXMLElementValuePair("IsCardPresent", bCardPresent ? "true" : "false");
		}

		//More required fields
		//Note:  We are providing currency in the SQL format.  QB only accepts decimal values, with 2 decimal places 
		//	afterward.  I can't find an official comment, but I believe they are purely limited to US (and possibly Canada)
		//	for processing.
		strXMLMessage += GetXMLElementValuePair("Amount", FormatCurrencyForSql(cyAmount)) +
			GetXMLElementValuePair("NameOnCard", strNameOnCard);

		//These fields are sent if we have them.  I am not doing specific string length manipulation here at this time.  The fields
		//	are limited when typed into, but not when loaded from data.  Due to the AVS facilities built into QB, if the fields
		//	are too long, I want the user to know that specifically, so that if an AVS failure is triggered, the user is aware why.
		if(!strAddress.IsEmpty()) {
			strXMLMessage += GetXMLElementValuePair("CreditCardAddress", strAddress);
		}
		if(!strZip.IsEmpty()) {
			strXMLMessage += GetXMLElementValuePair("CreditCardPostalCode", strZip);
		}
		if(!strSecurityCode.IsEmpty()) {
			strXMLMessage += GetXMLElementValuePair("CardSecurityCode", strSecurityCode);
		}

		//For now, we are never sending these.
		//GetXMLElementValuePair("CommercialCardCode", );	//This doesn't seem applicable to our clients
		//GetXMLElementValuePair("SalesTaxAmount", );		//This is only used if you're using the CommercialCardCode
		//GetXMLElementValuePair("BatchID", );				//If you leave this blank, it's auto batched.  That's preferable.

		//Wrap up 
		strXMLMessage += "</CustomerCreditCardChargeRq></QBMSXMLMsgsRq>";

		//and wrap it up with the footer
		strXMLMessage += GenerateQBMSXMLFooter();

		return true;
	}

	//Parses the given XML response, returning the elements marked as OUT parameters.  If the
	//	return value is false, no parameters will have been filled.
	bool ParseChargeRsResponse(const CString strXMLResponse,
		//Output parameters
		OUT CString &strCreditCardTransID, OUT CString &strAuthorizationCode, OUT CString &strAVSStreet, 
		OUT CString &strAVSZip, OUT CString &strCardSecurityCodeMatch, OUT CString &strPaymentStatus, OUT COleDateTime &dtTxnAuthorizationTime)
	{
		//First thing, load the XML into a document
		MSXML2::IXMLDOMDocument2Ptr pDoc = CreateQBMSCompatibleXMLDocument();
		if(pDoc->loadXML(_bstr_t(strXMLResponse)) == VARIANT_FALSE) {
			//Failed!  Usually bad XML.  We cannot log the response here for debug purposes -- it may contain critical
			//	info such as the credit card number.
			AfxMessageBox("Failed to parse XML in ChargeRs response, please check your XML and try again.\r\n\r\n" 
			//If we're debugging, let's include the entire XML in the message box.  I don't think this should be presented to
			//	live users.  Bad XML really should be caught before we reach this function.
#ifdef _DEBUG
				+ strXMLResponse
#endif
			);
			return false;
		}

		//Otherwise it loaded fine, so let's start parsing the response.

		//Header:  The entirely of <SignonMsgsRs> seems useless for our purposes.  It returns the server time and the session ticket
		//	that we gave it to start with.

		// (d.thompson 2010-06-11) - PLID 39124 - Improve error handling.
		MSXML2::IXMLDOMNodePtr pRoot = FindChildNode(pDoc->GetdocumentElement(), "QBMSXMLMsgsRs");
		if(pRoot == NULL) {
			//No root element??
			CString str = "ParseChargeRsResponse:  Bad response.\r\n\r\n"
				"Contents:  " + strXMLResponse;
			AfxMessageBox(str);
			//For current purposes, log this.
			Log("%s", str);
			return false;
		}

		//Misread this on the first go.  0 is not the only indicator of success -- there are warning events or info events with 
		//	StatusCode values, but the transaction succeeds. What we really need to consider is a severity of "ERROR"
		CString strSeverity = GetTextFromXPath(pDoc->GetdocumentElement(), "//QBMSXML/QBMSXMLMsgsRs/CustomerCreditCardChargeRs/@statusSeverity");
		CString strStatusCode = GetTextFromXPath(pDoc->GetdocumentElement(), "//QBMSXML/QBMSXMLMsgsRs/CustomerCreditCardChargeRs/@statusCode");
		CString strMessage = GetTextFromXPath(pDoc->GetdocumentElement(), "//QBMSXML/QBMSXMLMsgsRs/CustomerCreditCardChargeRs/@statusMessage");
		// (d.thompson 2010-06-11) - PLID 39124 - For safety, check the string value, not the atoi() value.  We don't want anything
		//	that isn't exactly "0".
		if(!strStatusCode.IsEmpty() && strStatusCode == "0") {
			//This is a generic "Success OK" Message.  We don't need to alert the user about it.
		}
		else {
			//Any other kind of message we want to let the user know what happened.
			QBMS_HandleXMLError(strStatusCode, strSeverity, strMessage);

			//Additionally, if this was an ERROR condition, we need to abort entirely.
			// (d.thompson 2010-06-11) - PLID 39124 - Check specifically for known successes, anything else is
			//	failure.  At this point, "WARN" (rare, but possible) or "INFO" (most common) are most of our cases.
			if(strSeverity.CompareNoCase("WARN") != 0 && strSeverity.CompareNoCase("INFO") != 0 ) {
				return false;
			}
		}

		//No errors, then we have a successful response!  Let's pull out the data that the caller may wish to save or present to the user.
		strCreditCardTransID = GetTextFromXPath(pDoc->GetdocumentElement(), "//QBMSXML/QBMSXMLMsgsRs/CustomerCreditCardChargeRs/CreditCardTransID");
		strAuthorizationCode = GetTextFromXPath(pDoc->GetdocumentElement(), "//QBMSXML/QBMSXMLMsgsRs/CustomerCreditCardChargeRs/AuthorizationCode");
		//Both of these are "Pass", "Fail", or "NotAvailable"
		strAVSStreet = GetTextFromXPath(pDoc->GetdocumentElement(), "//QBMSXML/QBMSXMLMsgsRs/CustomerCreditCardChargeRs/AVSStreet");
		strAVSZip = GetTextFromXPath(pDoc->GetdocumentElement(), "//QBMSXML/QBMSXMLMsgsRs/CustomerCreditCardChargeRs/AVSZip");
		//"Y" or "N" or "NotAvailable"
		strCardSecurityCodeMatch = GetTextFromXPath(pDoc->GetdocumentElement(), "//QBMSXML/QBMSXMLMsgsRs/CustomerCreditCardChargeRs/CardSecurityCodeMatch");
		//Indicates the status against the Credit Card issuer.  "Completed" means the card was successfully processed.
		//	"Unknown" indicates anything else. This does not mean failure, it just means we don't know what happened to the issuer.
		strPaymentStatus = GetTextFromXPath(pDoc->GetdocumentElement(), "//QBMSXML/QBMSXMLMsgsRs/CustomerCreditCardChargeRs/PaymentStatus");
		//Time it was authorized (convert to COleDateTime)
		dtTxnAuthorizationTime = QBMSDateTimeTypeToDateTime(GetTextFromXPath(pDoc->GetdocumentElement(), "//QBMSXML/QBMSXMLMsgsRs/CustomerCreditCardChargeRs/TxnAuthorizationTime"));

		//Not needed for our purposes, it is recommended to save if you're syncing to Quickbooks
		//MerchantAccountNumber
		//These features are all used in Quickbooks reconcilliation (we don't sync to QB currently)
		//ReconBatchID
		//PaymentGroupingCode
		//TxnAuthorizationStamp
		//ClientTransID

		return true;
	}


#pragma endregion

#pragma region CustomerCreditCardTxnVoidOrRefund Functions

	// (d.thompson 2010-11-08) - PLID 41367 - Since we now support multiple QBMS accounts, the connection ticket will need passed down to us.
	bool GenerateValidVoidOrRefundXML(IN CString strConnectionTicket, CString &strXMLMessage, long nPaymentID, CString strPreviousTransID, COleCurrency cyTotalAmount)
	{
		strXMLMessage = GenerateQBMSXMLHeader();

		//Every message will have the signon information first.
		// (d.thompson 2010-06-21) - PLID 39272 - This is no longer needed.  See next line.
		//strXMLMessage += GenerateSignonTicket();

		// (d.thompson 2010-06-21) - PLID 39272 - Instead of a signon (HTTP POST then parse result), we are just
		//	going to send the request at the same time.  Intuit informs me (https://ipp.developer.intuit.com/sdk/qbms/Documentation/Sending_Requests)
		//	that you can safely do this.  You will get a SessionTicket in response, but you can just ignore it.
		// (d.thompson 2010-11-08) - PLID 41367 - We now support multiple QBMS accounts, pass along the connection ticket that we want to use.
		strXMLMessage += GetQBMSSignonMsgsRqDesktopRqXML(strConnectionTicket);

		//Now for the code specific to this message
		strXMLMessage += "<QBMSXMLMsgsRq><CustomerCreditCardTxnVoidOrRefundRq>" + 
			//This is just required to be a unique value.  For now I'm going to use the payment ID.  I think this should
			//	be unique for our purposes.
			// (d.thompson 2010-06-01) - PLID 38445 - Guess I was wrong.  This is a unique submission ID, not payment ID.  If 
			//	you submit, and get failure, you can't resubmit the same ID again for 5 minutes.  So we need to better unique-ify
			//	this value.  I'm going to tack on the tickcount.
			GetXMLElementValuePair("TransRequestID", FormatString("%li_%lu", nPaymentID, GetTickCount()));

		//This is the value returned during the initial Charge (or Capture) event that we are voiding/refunding
		strXMLMessage += GetXMLElementValuePair("CreditCardTransID", strPreviousTransID);

		//More required fields
		//Note:  We are providing currency in the SQL format.  QB only accepts decimal values, with 2 decimal places 
		//	afterward.  I can't find an official comment, but I believe they are purely limited to US (and possibly Canada)
		//	for processing.
		strXMLMessage += GetXMLElementValuePair("Amount", FormatCurrencyForSql(cyTotalAmount));

		//For now, we are never sending these.
		//ClientTransID				//For QB reconciliation
		//CommercialCardCode		//Not supported
		//SalesTaxAmount			//Not supported
		//ForceRefund				//Not currently supported
		//BatchID					//Same as ChargeRq - BatchID that is empty will auto batch, which is preferable.

		//Wrap up 
		strXMLMessage += "</CustomerCreditCardTxnVoidOrRefundRq></QBMSXMLMsgsRq>";

		//and wrap it up with the footer
		strXMLMessage += GenerateQBMSXMLFooter();

		return true;
	}

	//Given a response XML document, fills the parameters marked as OUT.  If the return value
	//	is false, no parameters are filled.
	bool ParseVoidOrReturnRsResponse(const CString strResponseXML, OUT CString &strCreditCardTransID, OUT CString &strPaymentStatus, 
		OUT COleDateTime &dtTxnAuthorizationTime)
	{
		//First thing, load the XML into a document
		MSXML2::IXMLDOMDocument2Ptr pDoc = CreateQBMSCompatibleXMLDocument();
		if(pDoc->loadXML(_bstr_t(strResponseXML)) == VARIANT_FALSE) {
			//Failed!  Usually bad XML.  We cannot log the response here for debug purposes -- it may contain critical
			//	info such as the credit card number.
			AfxMessageBox("Failed to parse XML in VoidOrReturnRs response, please check your XML and try again.\r\n\r\n" 
			//If we're debugging, let's include the entire XML in the message box.  I don't think this should be presented to
			//	live users.  Bad XML really should be caught before we reach this function.
#ifdef _DEBUG
				+ strResponseXML
#endif
			);
			return false;
		}

		//Otherwise it loaded fine, so let's start parsing the response.

		//Header:  The entirely of <SignonMsgsRs> seems useless for our purposes.  It returns the server time and the session ticket
		//	that we gave it to start with.

		// (d.thompson 2010-06-11) - PLID 39124 - Improve error handling.
		MSXML2::IXMLDOMNodePtr pRoot = FindChildNode(pDoc->GetdocumentElement(), "QBMSXMLMsgsRs");
		if(pRoot == NULL) {
			//No root element??
			CString str = "ParseVoidOrReturnsResponse:  Bad response.\r\n\r\n"
				"Contents:  " + strResponseXML;
			AfxMessageBox(str);
			//For current purposes, log this.
			Log("%s", str);
			return false;
		}

		//Misread this on the first go.  0 is not the only indicator of success -- there are warning events or info events with 
		//	StatusCode values, but the transaction succeeds. What we really need to consider is a severity of "ERROR"
		CString strSeverity = GetTextFromXPath(pDoc->GetdocumentElement(), "//QBMSXML/QBMSXMLMsgsRs/CustomerCreditCardTxnVoidOrRefundRs/@statusSeverity");
		CString strStatusCode = GetTextFromXPath(pDoc->GetdocumentElement(), "//QBMSXML/QBMSXMLMsgsRs/CustomerCreditCardTxnVoidOrRefundRs/@statusCode");
		CString strMessage = GetTextFromXPath(pDoc->GetdocumentElement(), "//QBMSXML/QBMSXMLMsgsRs/CustomerCreditCardTxnVoidOrRefundRs/@statusMessage");
		// (d.thompson 2010-06-11) - PLID 39124 - For safety, check the string value, not the atoi() value.  We don't want anything
		//	that isn't exactly "0".
		if(!strStatusCode.IsEmpty() && strStatusCode == "0") {
			//This is a generic "Success OK" Message.  We don't need to alert the user about it.
		}
		else {
			//Any other kind of message we want to let the user know what happened.
			QBMS_HandleXMLError(strStatusCode, strSeverity, strMessage);

			//Additionally, if this was an ERROR condition, we need to abort entirely.
			// (d.thompson 2010-06-11) - PLID 39124 - Check specifically for known successes, anything else is
			//	failure.  At this point, "WARN" (rare, but possible) or "INFO" (most common) are most of our cases.
			if(strSeverity.CompareNoCase("WARN") != 0 && strSeverity.CompareNoCase("INFO") != 0 ) {
				return false;
			}
		}

		//No errors, then we have a successful response!  Let's pull out the data that the caller may wish to save or present to the user.
		strCreditCardTransID = GetTextFromXPath(pDoc->GetdocumentElement(), "//QBMSXML/QBMSXMLMsgsRs/CustomerCreditCardTxnVoidOrRefundRs/CreditCardTransID");
		strPaymentStatus = GetTextFromXPath(pDoc->GetdocumentElement(), "//QBMSXML/QBMSXMLMsgsRs/CustomerCreditCardTxnVoidOrRefundRs/PaymentStatus");
		//Time it was authorized (convert to COleDateTime)
		dtTxnAuthorizationTime = QBMSDateTimeTypeToDateTime(GetTextFromXPath(pDoc->GetdocumentElement(), "//QBMSXML/QBMSXMLMsgsRs/CustomerCreditCardTxnVoidOrRefundRs/TxnAuthorizationTime"));

		//This just states whether a void or a refund was performed.  At this point, I don't see any
		//	value in this, as long as it's done we're happy.
		//VoidOrRefundTxnType
		//Not needed for our purposes, it is recommended to save if you're syncing to Quickbooks
		//MerchantAccountNumber
		//These features are all used in Quickbooks reconcilliation (we don't sync to QB currently)
		//ReconBatchID
		//PaymentGroupingCode
		//TxnAuthorizationStamp
		//ClientTransID

		return true;
	}


#pragma endregion

	//Given a transaction, and all the information required for that transaction, fills the transaction object
	//	appropriately with the field data.  Does validation on fields, will return false upon any catastrophic
	//	failure.  Returns true otherwise.
	// (d.thompson 2010-11-08) - PLID 41367 - We now support multiple QBMS transactions, so the caller needs
	//	to tell us which connection ticket should be used.
	// (d.thompson 2010-11-08) - PLID 41367 - Also need to know production status
	bool ProcessQBMSTransaction(IN bool bIsProduction, IN CString strConnectionTicket, QBMS_TransTypes qbTransType, long nPaymentID, CString strCCNumber, COleCurrency cyTotalAmount, 
		CString strNameOnCard, CString strMM, CString strYY, CString strSecurityCode, CString strAddress, 
		CString strZip, bool bHasTrack2Data, CString strTrack2Data, bool bCardPresent, CString strPreviousTransID,
		long nAuditPatientID, // (a.walling 2010-01-25 09:02) - PLID 37026 - Pass in the patient ID
		CString strAuditPatientName, long nAuditRecordID,
		//Output data here
		OUT CString &strCreditCardTransID, OUT CString &strAuthorizationCode, OUT CString &strAVSStreet, 
		OUT CString &strAVSZip, OUT CString &strCardSecurityCodeMatch, OUT CString &strPaymentStatus, OUT COleDateTime &dtTxnAuthorizationTime)
	{
		//Auditing.  This is fairly bizarre way of doing it, but it just seems unreasonable to try passing a patient name
		//	and record ID down through the chain of at least 5 functions that have no need for the data.
		// (a.walling 2010-01-25 09:02) - PLID 37026 - Pass in the patient ID
		SetAuditData(strAuditPatientName, nAuditPatientID, nAuditRecordID);

		CString strXMLMessage;
		switch(qbTransType) {
			case qbttSale:
				// (d.thompson 2010-11-08) - PLID 41367 - Since we now support multiple QBMS accounts, we need to pass along
				//	the connection ticket.
				if(!GenerateValidChargeRqXML(strConnectionTicket, strXMLMessage, nPaymentID, strCCNumber, strMM, strYY, 
					cyTotalAmount, strNameOnCard, bHasTrack2Data, strTrack2Data, bCardPresent, 
					strAddress, strZip, strSecurityCode))
				{
					AfxMessageBox("Failed to generate Charge request.  Please check your input data and try again.");
					ClearAuditData();
					return false;
				}
				break;

			case qbttVoidOrReturn:
				// (d.thompson 2010-11-08) - PLID 41367 - Since we now support multiple QBMS accounts, we need to pass along
				//	the connection ticket.
				if(!GenerateValidVoidOrRefundXML(strConnectionTicket, strXMLMessage, nPaymentID, strPreviousTransID, cyTotalAmount))
				{
					AfxMessageBox("Failed to generate return request.  Please check your input data and try again.");
					ClearAuditData();
					return false;
				}
				break;

			case qbttVoiceAuth:
				// (d.thompson 2009-07-01) - PLID 34771 - This will be done at a later date.  No code should reach this now.
				AfxThrowNxException("Voice authorization is not currently supported.");
				ClearAuditData();
				return false;
				break;
		}

		//We have our XML, so let's post it.
		CString strResponseXML, strHTTPStatusText;
		long nHTTPStatus;
		if(!PostQBMSHTTPRequest(GetTransactionRequestURL(bIsProduction), strXMLMessage, strResponseXML, nHTTPStatus, strHTTPStatusText)) {
			//We only need warn the user about the status if this happens.
			QBMS_HandleHTTPError(nHTTPStatus, strHTTPStatusText, strResponseXML);
			ClearAuditData();
			return false;
		}

		//Now handle the response
		switch(qbTransType) {
			case qbttSale:
				if(!ParseChargeRsResponse(strResponseXML, strCreditCardTransID, strAuthorizationCode, strAVSStreet, 
					strAVSZip, strCardSecurityCodeMatch, strPaymentStatus, dtTxnAuthorizationTime))
				{
					ClearAuditData();
					return false;
				}
				break;

			case qbttVoidOrReturn:
				//These fields are not used, ensure they are blank.
				strAuthorizationCode = strAVSStreet = strAVSZip = strCardSecurityCodeMatch = "";
				//Parse our response to fill in the "real" data.
				if(!ParseVoidOrReturnRsResponse(strResponseXML, strCreditCardTransID, strPaymentStatus, dtTxnAuthorizationTime))
				{
					ClearAuditData();
					return false;
				}
				break;

			case qbttVoiceAuth:
				// (d.thompson 2009-07-01) - PLID 34771 - This will be done at a later date.  No code should reach this now.
				AfxThrowNxException("Voice authorization is not currently supported.");
				ClearAuditData();
				return false;
				break;
		}

		//Success, so audit it
		// (a.walling 2010-01-25 09:05) - PLID 37026 - Use the patient ID
		// (d.thompson 2010-11-11) - PLID 40351 - Renamed to be processor-agnostic events.
		AuditEvent(nAuditPatientID, strAuditPatientName, BeginNewAuditEvent(), aeiProcessingAccepted, nAuditRecordID, "", "Accepted by Quickbooks Merchant Services", aepHigh, aetCreated);

		return true;
	}

#pragma region QBMS Cached Setup Data

	//Variables that are cached
	//Application ID.  We signed up for this through QB.
	//		108748584 is our PreProduction AppID.
	//		151241744 is our Production AppID
	// (d.thompson 2010-06-07) - PLID 39030 - No longer use the string, just return the app ID as-needed.
	//CString g_strQBMS_AppID;
	//Application Login.  We signed up for this through QB.
	//		practice.nextech.com is our PreProduction AppLogin.
	//		practice.nextech.com is out Production AppLogin
	// (d.thompson 2010-11-08) - PLID 41367 - No need for a variable here either, it's always the same string.
	//CString g_strQBMS_AppLogin;

	//Connection Ticket.  The user must do a 1 time authorization for our software to 
	//		access their QBMS account. This is the connection ticket to allow that auth.
	CString g_strQBMS_ConnectionTicket;
	//Flag for production turned on/off.
	BOOL g_bQBMS_IsProduction;
	//States if the cache has been loaded or not loaded.
	bool g_bIsDataCached = false;

	// (d.thompson 2010-11-08) - PLID 41367 - We now support multiple accounts instead of a single account, so the cache will 
	//	have to change to update that.  The design of the cache is that the program will ALWAYS pull from the cache, not from
	//	data, when the cache is known.  These values change very infrequently (typically once, ever during initial setup).  Making
	//	a change in accounts on the local machine will reset the cache, but remote changes will require an application restart.
	//Futur request:  Add PLID to enable tablecheckers to do this.  (PLID 41381)
	CMap<long, long, CQBMSProcessingAccount, CQBMSProcessingAccount&> g_mapQBMSAccounts;

	// (d.thompson 2010-11-08) - PLID 41367 - Clears anything from the cache of accounts.
	void ClearCachedAccounts()
	{
		//No memory to clean up, so just tell the map to empty itself
		g_mapQBMSAccounts.RemoveAll();

		//Reset our "is cached" status
		g_bIsDataCached = false;
	}

	// (d.thompson 2010-11-08) - PLID 41367 - Will clear the cache before it begins.  This function will load all accounts
	//	that are currently saved in the database and keep them in a cache for future program use.
	void LoadCachedAccountsFromData()
	{
		ClearCachedAccounts();

		_RecordsetPtr prs = CreateRecordset("SELECT ID, Description, Inactive, IsProduction, ConnectionTicket FROM QBMS_SetupData");
		while(!prs->eof) {
			CQBMSProcessingAccount acct;
			long nID = AdoFldLong(prs, "ID");
			acct.LoadFromData(nID, AdoFldString(prs, "Description"), AdoFldBool(prs, "Inactive") == FALSE ? false : true, 
				AdoFldBool(prs, "IsProduction") == FALSE ? false : true, prs->Fields->Item["ConnectionTicket"]->Value);

			//Load into the map
			g_mapQBMSAccounts.SetAt(nID, acct);
			prs->MoveNext();
		}

		//Update our "is cached" status.
		g_bIsDataCached = true;
	}

	// (d.thompson 2010-11-08) - PLID 41367 - Retrieves a list of all accounts.
	void GetAccountIDsAndNames(CArray<CQBMSProcessingAccount, CQBMSProcessingAccount&> *pary)
	{
		if(!g_bIsDataCached) {
			LoadCachedAccountsFromData();
		}

		POSITION pos = g_mapQBMSAccounts.GetStartPosition();
		while(pos != NULL) {
			long nID;
			CQBMSProcessingAccount acct;
			g_mapQBMSAccounts.GetNextAssoc(pos, nID, acct);

			pary->Add(acct);
		}
	}

	// (d.thompson 2010-11-08) - PLID 41367 - Given an account ID, will return a copy of the account information.  Requires that the
	//	ID you are looking for exists, will throw an exception otherwise.
	//If the data has not yet been cached, it will be loaded first.
	CQBMSProcessingAccount GetAccountByID(long nIDToFind)
	{
		if(!g_bIsDataCached) {
			LoadCachedAccountsFromData();
		}

		POSITION pos = g_mapQBMSAccounts.GetStartPosition();
		while(pos != NULL) {
			long nID;
			CQBMSProcessingAccount acct;
			g_mapQBMSAccounts.GetNextAssoc(pos, nID, acct);

			if(acct.GetID() == nIDToFind) {
				//match!
				return acct;
			}
		}

		//If not found, we throw an exception.  This should only be called for known cached accounts.
		AfxThrowNxException("Could not find account by ID %li, it does not exist!", nIDToFind);
		throw;		//avoid compiler warning, not actually executed
	}

	CString GetAppID(BOOL bUseProduction)
	{
		if(!g_bIsDataCached) {
			LoadCachedAccountsFromData();
		}

		// (d.thompson 2010-06-07) - PLID 39030 - Return the values here, get rid of the saved string approach entirely.  This
		//	function should be the only one to know these IDs.
		if(bUseProduction) {
			//We are in production mode, use the appropriate ID.  This is hardcoded and registered through the IDN.
			return "151241744";
		}
		else {
			//Pre-production test site
			return "108748584";
		}
	}

	CString GetAppLogin()
	{
		if(!g_bIsDataCached) {
			LoadCachedAccountsFromData();
		}

		// (d.thompson 2010-11-08) - PLID 41367 - No longer keep in a variable, I made this value the same
		//	for both pre-prod and prod setups.
		return "practice.nextech.com";
	}

#pragma endregion

#pragma region QBMS URLs
	CString GetConnectionTicketURL(BOOL bGetProduction)
	{
		if(bGetProduction) {
			return FormatString("https://merchantaccount.quickbooks.com/j/sdkconnection?appid=%s&sessionEnabled=false", GetAppID(bGetProduction));
		}
		else {
			return FormatString(
				"https://merchantaccount.ptc.quickbooks.com/j/sdkconnection/connectionList?appid=%s&sessionEnabled=false", GetAppID(bGetProduction));
		}
	}

	//This URL is only needed if we support session login security.
	// (d.thompson 2010-11-08) - PLID 41367 - Due to multiple account support, we now have to be told production vs non-production status.
	CString GetIntermediateSessionTicketURL(bool bIsProduction)
	{
		if(bIsProduction) {
			return FormatString("https://login.quickbooks.com/j/qbn/sdkapp/sessionauth2?serviceid=1002&appid=%s&service_flags=qbmssdk%3dtrue", 
					GetAppID(TRUE));
		}
		else {
			return FormatString("https://login.ptc.quickbooks.com/j/qbn/sdkapp/sessionauth2?serviceid=1002&appid=%s&service_flags=qbmssdk%3dtrue", 
				GetAppID(FALSE));
		}
	}

	//Also only needed if we support session login security. The Intermediate ticket parameter
	//	should be the value returned after navigating to the URL returned by GetIntermediateSessionTicketURL().
	// (d.thompson 2010-11-08) - PLID 41367 - Any callers will need to pass in the connection ticket.
	// (d.thompson 2010-11-08) - PLID 41367 - Due to multiple account support, we now have to be told production vs non-production status.
	CString GetTransformIntermediateSessionTicketURL(CString strIntermediateSessionTicket, CString strConnectionTicket, bool bIsProduction)
	{
		//This may eventually be covered in PLID 34732, but for now we do not support this mode.  Make sure noone
		//	accidentally uses it.
		AfxThrowNxException("QBMS Login Security mode is not currently supported.");
		return "";

		if(bIsProduction) {
			return FormatString("https://login.quickbooks.com/j/qbn/sdkapp/connauth?serviceid=1002&appid=%s&conntkt=%s"
				"&sessiontkt=%s", GetAppID(TRUE), strConnectionTicket, strIntermediateSessionTicket);
		}
		else {
			return FormatString("https://login.ptc.quickbooks.com/j/qbn/sdkapp/connauth?serviceid=1002&appid=%s&conntkt=%s"
				"&sessiontkt=%s", GetAppID(FALSE), strConnectionTicket, strIntermediateSessionTicket);
		}
	}

	//URL for all transactions
	// (d.thompson 2010-11-08) - PLID 41367 - Due to multiple account support, we now have to be told production vs non-production status.
	CString GetTransactionRequestURL(bool bIsProduction)
	{
		if(bIsProduction) {
			return "https://merchantaccount.quickbooks.com/j/AppGateway";
		}
		else {
			return "https://merchantaccount.ptc.quickbooks.com/j/AppGateway";
		}
	}

#pragma endregion


}	//end namespace

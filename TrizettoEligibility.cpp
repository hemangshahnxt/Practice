#include "stdafx.h"
#include "TrizettoEligibility.h"
#include "SOAPUtils.h"
#include "MsgBox.h"

// (j.jones 2010-07-01 11:58) - PLID 39319 - created

namespace TrizettoEligibility
{

CString GetTrizettoURL(CString strServiceName)
{
	//the eligibility test/production setting defaults to the ebilling setting
	long nEbillingProduction = GetRemotePropertyInt("EnvoyProduction", 0, 0, _T("<None>"), true);
	long nEligibilityProduction = GetRemotePropertyInt("EligibilityProduction", nEbillingProduction, 0, _T("<None>"), true);

	// (j.jones 2016-07-12 12:32) - I confirmed with Trizetto that we are allowed to use the production
	// mode when testing, so I removed a call that forced test mode in debug

	// Per Trizetto: "The test URL just offers the debugging link where the production URL will not."
	// Although I admit I have no idea what that means.

	if (nEligibilityProduction == 1) {
		//production
		return "https://services.gatewayedi.com/eligibility/" + strServiceName;
	}
	else {
		//test
		return "https://testservices.gatewayedi.com/eligibility/" + strServiceName;		
	}
}

CString GetTrizettoURI()
{
	return "GatewayEDI.WebServices";
}

//sends the request to Trizetto, and retrieves the response in the same transaction
// (j.jones 2015-11-13 08:36) - PLID 67578 - added a timeout, in seconds, and a timeout retry count
ETrizettoInquiryReturnValues PerformEligibilityInquiry(CString strSiteID, CString strPassword, CString strPayerID,
							long nTimeoutSeconds, long nTimeoutRetryCount,
							IN CString str270Request, OUT CString &str271Response)
{
	str271Response = "";

	//generate the header
	CString strHeader;
	strHeader.Format("<AuthSOAPHeader xmlns=\"%s\">", GetTrizettoURI());
	strHeader += GetXMLElementValuePair("User", strSiteID);
	strHeader += GetXMLElementValuePair("Password", strPassword);
	strHeader += "</AuthSOAPHeader>";

	//now generate the actual request
	CString strInquiryXML;
	strInquiryXML += GetXMLElementValuePair("X12Input", str270Request);
	strInquiryXML += GetXMLElementValuePair("GediPayerID", strPayerID);

	//sending RawPayerData tells them we want the 271 response,
	//the other option is "Xml" but we already have a parser for the 271 file
	strInquiryXML += GetXMLElementValuePair("ResponseDataType", "RawPayerData");

	CString strXMLRequest = GetXMLElementValuePair_Embedded("Inquiry", strInquiryXML);

/*
#ifdef _DEBUG
	CMsgBox dlg;
	dlg.msg = strXMLRequest;
	dlg.DoModal();
#endif
*/

	CWaitCursor pWait;

	MSXML2::IXMLDOMNodePtr xmlResponse = NULL;

	// (j.jones 2011-10-05 08:43) - PLID 44121 - in the case of a timeout, we will
	// attempt the inquiry 2 more times before giving up and firing off an exception
	// (j.jones 2015-11-12 11:11) - PLID 67578 - moved the timeout retry logic
	// to the actual exception handler
	long nTimeoutCount = 0;
	bool bContinue = true;
	while(bContinue) {

		try {

			bContinue = false;

			// (j.jones 2015-11-12 11:12) - PLID 67578 - added a configurable timeout
			xmlResponse = CallSoapFunction(GetTrizettoURL("service.asmx"),
				GetTrizettoURI(),
				"DoInquiryByX12DataWith271Response", 
				strXMLRequest,
				"", "", FALSE, strHeader, NULL, NULL, nTimeoutSeconds * 1000);

		}catch(CSOAPFaultException *e) {
			//if we receive a soap fault, give a clean message, and abort

			char errorMessage[4096];
			if (!e->GetErrorMessage(errorMessage, 4096)) {
				//throw a regular exception
				throw e;
			}

			CString strError;
			strError.Format("The Eligibility Real-Time Inquiry failed with the following error:\n\n"
				"%s\n"
				"Please ensure your login information is correct, and that you have access to the internet.\n\n"
				"If this message persists, please contact NexTech Technical Support.", errorMessage);
			AfxMessageBox(strError);

			e->Delete();
			return tirvCatastrophicFail;

		}
		catch (_com_error& e) {
			if (e.Error() == 0x80072EE2) {	//timeout
				//timeout, try again
				nTimeoutCount++;

				Log("Timeout connecting to TriZetto on attempt %li...", nTimeoutCount);

				// (j.jones 2015-11-12 11:13) - PLID 67578 - if we have timed out more than
				// our allotted retries, fail with a timeout exceeded status
				if (nTimeoutCount > nTimeoutRetryCount) {
					return tirvTimeoutExceeded;
				}

				bContinue = true;

				//wait two seconds before trying again
				Sleep(2000);
				continue;
			}
			else {
				HandleException(e, "Error submitting eligibility request to TriZetto.", __LINE__, __FILE__);
				return tirvCatastrophicFail;
			}
		}
		catch (...) {
			HandleException(NULL, "Error submitting eligibility request to TriZetto.", __LINE__, __FILE__);
			return tirvCatastrophicFail;
		}
	}

	if (xmlResponse != NULL) {

		//the response will have the ANSI 271 response, extra messages
		//(that SHOULD already be included in the response!), and the
		//original 270 file we just sent

/*
#ifdef _DEBUG
		CMsgBox dlg;
		CString strXML((LPCTSTR)xmlResponse->xml);
		dlg.msg = strXML;
		dlg.DoModal();
#endif
*/
		MSXML2::IXMLDOMNodePtr xmlResponseAsRawString = FindChildNode(xmlResponse, "ResponseAsRawString");
		if(xmlResponseAsRawString) {
			str271Response = (LPCTSTR)xmlResponseAsRawString->Gettext();
		}
		
		if(str271Response.IsEmpty()) {
			return tirvResponseEmpty;
		}
		else {
			return tirvSuccess;
		}
	}
	else {
		return tirvSendFail;
	}
}

};

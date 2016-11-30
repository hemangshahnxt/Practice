#pragma once

// (j.jones 2010-07-01 11:58) - PLID 39319 - created

namespace TrizettoEligibility
{

CString GetTrizettoURL(CString strServiceName);
CString GetTrizettoURI();

//an enum of possible return values
enum ETrizettoInquiryReturnValues {

	tirvSuccess = 0,
	tirvSendFail,
	tirvResponseEmpty,
	tirvCatastrophicFail,
	tirvTimeoutExceeded,	// (j.jones 2015-11-12 11:14) - PLID 67578 - returned if we timed out after several attempts
};

//sends the request to Trizetto, and retrieves the response in the same transaction
// (j.jones 2015-11-13 08:36) - PLID 67578 - added a timeout, in seconds, and a timeout retry count
ETrizettoInquiryReturnValues PerformEligibilityInquiry(CString strSiteID, CString strPassword, CString strPayerID,
							long nTimeoutSeconds, long nTimeoutRetryCount,
							IN CString str270Request, OUT CString &str271Response);

};

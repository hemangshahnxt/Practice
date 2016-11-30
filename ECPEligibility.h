#pragma once

// (j.jones 2010-11-08 11:27) - PLID 40914 - created

namespace ECPEligibility
{

//an enum of possible return values
enum EECPInquiryReturnValues {

	eirvSuccess = 0,
	eirvSendFail,
	eirvResponseEmptyOrInvalid,
	eirvCatastrophicFail,
	eirvTimeoutExceeded,	// (j.jones 2015-11-12 11:14) - PLID 67578 - returned if we timed out after several attempts
};

//sends the request to ECP, and retrieves the response in the same transaction
// (j.jones 2015-11-13 08:36) - PLID 67578 - added a timeout, in seconds, and a timeout retry count
EECPInquiryReturnValues PerformEligibilityInquiry(CString strLoginName, CString strPassword,
							long nTimeoutSeconds, long nTimeoutRetryCount,
							IN CString str270Request, OUT CString &str271Response);

};

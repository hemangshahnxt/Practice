#include "stdafx.h"
#include "ECPEligibility.h"
#include "InternetUtils.h"

// (j.jones 2010-11-08 11:27) - PLID 40914 - created

namespace ECPEligibility
{

//sends the request to ECP, and retrieves the response in the same transaction
// (j.jones 2015-11-13 08:36) - PLID 67578 - added a timeout, in seconds, and a timeout retry count
EECPInquiryReturnValues PerformEligibilityInquiry(CString strLoginName, CString strPassword,
							long nTimeoutSeconds, long nTimeoutRetryCount,
							IN CString str270Request, OUT CString &str271Response)
{
	str271Response = "";

	// (a.walling 2010-11-08 09:48) - PLID 40914 - I went ahead and updated this code to use the new file uploads
	InternetUtils::FormData formData;
	formData
		.Add("login", strLoginName)
		.Add("pw", strPassword)
		.Add("action", "UPE")
		.AddFile("file_name", "current_request.txt", str270Request);

	InternetUtils::Download download;

	// (j.jones 2015-11-13 08:36) - PLID 67578 - added a custom timeout, in seconds
	download
		.SetSource("https://www.4ecp.com/claims/cgi-bin/post.do")
		.SetFormData(formData)
		.SetSendTimeout(nTimeoutSeconds * 1000)
		.SetReceiveTimeout(nTimeoutSeconds * 1000);
	
	// (j.jones 2015-11-13 08:58) - PLID 67578 - handled retries after a timeout
	long nTimeoutCount = 0;
	bool bContinue = true;
	while (bContinue) {

		try {

			bContinue = false;

			str271Response = download.DownloadToString();

		}
		catch (_com_error& e) {
			if (e.Error() == 0x80072EE2) {	//timeout
				//timeout, try again
				nTimeoutCount++;

				Log("Timeout connecting to ECP on attempt %li...", nTimeoutCount);

				// (j.jones 2015-11-12 11:13) - PLID 67578 - if we have timed out more than
				// our allotted retries, fail with a timeout exceeded status
				if (nTimeoutCount > nTimeoutRetryCount) {
					return eirvTimeoutExceeded;
				}

				bContinue = true;

				//wait two seconds before trying again
				Sleep(2000);
				continue;
			}
			else {
				HandleException(e, "Error submitting eligibility request to ECP", __LINE__, __FILE__);
				return eirvCatastrophicFail;
			}
		}
	}

	if(str271Response.IsEmpty()) {
		//no response
		return eirvResponseEmptyOrInvalid;
	}
	else if(str271Response.Find("Authentication Failed") != -1 || str271Response.Find("Authentication Error") != -1) {
		AfxMessageBox("The Eligibility Real-Time Inquiry failed due to invalid login information.\n\n"
			"Please ensure your login information is correct. If this message persists, please contact NexTech Technical Support.");
		return eirvCatastrophicFail;
	}
	else if(str271Response.Find("ISA*") == -1 || str271Response.Find("ST*271") == -1) {
		//this response is invalid, log it, and track it as failed
		Log("Invalid 271 response received: %s", str271Response);						
		
		//we received something, but not a valid 271 file
		return eirvResponseEmptyOrInvalid;
	}
	else {
		//we received a response

		//strip out the encoding from the top
		long nISAStart = str271Response.Find("ISA*");
		if(nISAStart == -1) {
			//this response is invalid, log it, and track it as failed
			Log("Invalid 271 response received: %s", str271Response);						
			
			//we received something, but not a valid 271 file
			return eirvResponseEmptyOrInvalid;
		}
		else {

			//remove everything to the left of ISA*
			str271Response = str271Response.Right(str271Response.GetLength() - nISAStart);

			return eirvSuccess;
		}
	}
}

};

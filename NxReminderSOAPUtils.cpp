#include "stdafx.h"
#include "NxReminderSOAPUtils.h"

// (f.dinatale 2010-11-03) - PLID 40829 - Created the CNxRetrieveNxReminderClient object for communication to the NxReminder Server
// CNxRetrieveNxReminderClient Methods
CNxRetrieveNxReminderClient::CNxRetrieveNxReminderClient(const CString& strLicense) : m_strLicense(strLicense)
{
	m_strUsername = GetRemotePropertyText("NxReminderUsername");
	m_strPassword = GetRemotePropertyText("NxReminderPassword");
}

//CString CNxRetrieveNxReminderClient::GetNickname()
//{
//	return m_strNickname;
//}

CString CNxRetrieveNxReminderClient::GetLicenseKey()
{
	return m_strLicense;
}

int CNxRetrieveNxReminderClient::GetTierID()
{
	return m_nTierID;
}

int CNxRetrieveNxReminderClient::GetAllotedMessages()
{
	return m_nAllotedMessages;
}

int CNxRetrieveNxReminderClient::GetRemainingMessages()
{
	return m_nRemainingMessages;
}

bool CNxRetrieveNxReminderClient::IsActive()
{
	return m_bActive;
}

//void CNxRetrieveNxReminderClient::SetNickname(const CString& strNickname)
//{
//	m_strNickname = strNickname;
//}

void CNxRetrieveNxReminderClient::SetLicenseKey(const CString& strLicense)
{
	m_strLicense = strLicense;
}

void CNxRetrieveNxReminderClient::SetTierID(const int& nTier)
{
	m_nTierID = nTier;
}

void CNxRetrieveNxReminderClient::SetAllotedMessages(const int& nMessages)
{
	m_nAllotedMessages = nMessages;
}

void CNxRetrieveNxReminderClient::SetRemainingMessages(const int& nMessages)
{
	m_nRemainingMessages = nMessages;
}

void CNxRetrieveNxReminderClient::SetActive(const bool& bActive)
{
	m_bActive = bActive;
}

BOOL CNxRetrieveNxReminderClient::IsProductionAccount() {

	long nProduction = GetRemotePropertyInt("NxReminderProductionMode", 0, 0, "<None>", true);
	if (nProduction == 1) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}

int CNxRetrieveNxReminderClient::GetClientInfo()
{
	MSXML2::IXMLDOMNodePtr pXMLResponse;
	CString strXML = "";

	strXML += GetXMLElementValuePair("username", m_strUsername);
	strXML += GetXMLElementValuePair("password", m_strPassword);
	strXML += GetXMLElementValuePair("licenseID", m_strLicense);
	
	try {
		pXMLResponse = CallSoapFunction(GetNxReminderURL(), GetNxReminderURI(), "GetClientInfo", strXML);

		if(pXMLResponse != NULL) {
#ifdef _DEBUG
			CString strXMLResponse = (LPCTSTR)pXMLResponse->Getxml();
			strXMLResponse.Replace("><", ">\r\n<");
#endif
			CString strStatus = GetXMLNodeText(pXMLResponse, "ResponseCode");

			if(strStatus == "OK") {
				// (f.dinatale 2010-11-30) - PLID 40829 - Nickname no longer needed.
				//m_strNickname = GetXMLNodeText(pXMLResponse, "Nickname");
				m_nTierID = atoi(GetXMLNodeText(pXMLResponse, "LicenseType"));
				m_nAllotedMessages = atoi(GetXMLNodeText(pXMLResponse, "AllottedMessages"));
				m_nRemainingMessages = atoi(GetXMLNodeText(pXMLResponse, "RemainingMessages"));
				m_bActive = GetXMLNodeText(pXMLResponse, "Active") == "true" ? true : false;
				return 0;
			} else {
				//m_strNickname = "";
				m_nTierID = -1;
				m_nAllotedMessages = 0;
				m_nRemainingMessages = -1;
				m_bActive = false;
				
				// (f.dinatale 2011-01-13) - PLID 40829 - Added the case when the client doesn't exist.
				if(strStatus == "ClientDoesNotExist") {
					return 1;
				} else {
					return -2;
				}
			}
		} else {
			return -1;
		}
	} NxCatchAll("CNxRetrieveNxReminderClient::GetClientInfo : Invalid response from NxReminderServer");
	
	m_nTierID = -1;
	m_nAllotedMessages = 0;
	m_nRemainingMessages = -1;
	m_bActive = false;
	return -1;
}

// (f.dinatale 2010-11-30) - PLID 40829 - The nickname field is no longer needed so this whole message is not needed.
/*int CNxRetrieveNxReminderClient::ModifyClientNickname(const CString& strNickname)
{
	MSXML2::IXMLDOMNodePtr pXMLResponse;
	CString strXML = "";
	CString strUsername = GetRemotePropertyText("NxReminderUsername");
	CString strPassword = GetRemotePropertyText("NxReminderPassword");

	strXML += GetXMLElementValuePair("username", strUsername);
	strXML += GetXMLElementValuePair("password", strPassword);
	strXML += GetXMLElementValuePair("licenseID", m_strLicense);
	strXML += GetXMLElementValuePair("nickname", strNickname);
	pXMLResponse = CallSoapFunction(GetNxReminderURL(), GetNxReminderURI(), "ModifyClientNickname", strXML);

	if(pXMLResponse != NULL) {
#ifdef _DEBUG
		CString strXMLResponse = (LPCTSTR)pXMLResponse->Getxml();
		strXMLResponse.Replace("><", ">\r\n<");
#endif

		CString strStatus = GetXMLNodeText(pXMLResponse, "StatusCode");
		CString strStatusMessage = GetXMLNodeText(pXMLResponse, "StatusMessage");

		if(strStatus == "OK") {
			return 0;
		} else {
			if(strStatus == "CredentialFailure") {
				return -1;
			} else {
				return -2;
			}
		}
	} else {
		ThrowNxException("CNxRetrieveNxReminderClient::ModifyClientNickname : Invalid response from NxReminderServer");
		return -1;
	}
}*/

int CNxRetrieveNxReminderClient::ModifyClientLicense(long nLicenseType, long nAllotedMessages)
{
	MSXML2::IXMLDOMNodePtr pXMLResponse;
	CString strXML = "";

	CString strLicenseType;
	strLicenseType.Format("%li", nLicenseType);

	CString strAllotedMessages;
	strAllotedMessages.Format("%li", nAllotedMessages);

	strXML += GetXMLElementValuePair("username", m_strUsername);
	strXML += GetXMLElementValuePair("password", m_strPassword);
	strXML += GetXMLElementValuePair("licenseID", m_strLicense);
	strXML += GetXMLElementValuePair("licenseType", strLicenseType);
	strXML += GetXMLElementValuePair("allottedMessages", strAllotedMessages);

	try {
		pXMLResponse = CallSoapFunction(GetNxReminderURL(), GetNxReminderURI(), "ModifyClientLicense", strXML);

		if(pXMLResponse != NULL) {
#ifdef _DEBUG
			CString strXMLResponse = (LPCTSTR)pXMLResponse->Getxml();
			strXMLResponse.Replace("><", ">\r\n<");
#endif

			CString strStatus = GetXMLNodeText(pXMLResponse, "ResponseCode");

			if(strStatus == "OK") {
				return 0;
			} else {
				return -2;
			}
		} else {
			return -1;
		}
	} NxCatchAll("CNxRetrieveNxReminderClient::ModifyClientLicense : Invalid response from NxReminderServer");
	return -1;
}

int CNxRetrieveNxReminderClient::ModifyClientActiveStatus(const bool& bActive)
{
	MSXML2::IXMLDOMNodePtr pXMLResponse;
	CString strXML = "";

	strXML += GetXMLElementValuePair("username", m_strUsername);
	strXML += GetXMLElementValuePair("password", m_strPassword);
	strXML += GetXMLElementValuePair("licenseID", m_strLicense);
	strXML += GetXMLElementValuePair("active", bActive ? "true" : "false");

	try {
		pXMLResponse = CallSoapFunction(GetNxReminderURL(), GetNxReminderURI(), "ModifyClientActiveStatus", strXML);

		if(pXMLResponse != NULL) {
#ifdef _DEBUG
			CString strXMLResponse = (LPCTSTR)pXMLResponse->Getxml();
			strXMLResponse.Replace("><", ">\r\n<");
#endif

			CString strStatus = GetXMLNodeText(pXMLResponse, "ResponseCode");
			CString strStatusMessage = GetXMLNodeText(pXMLResponse, "ResponseCodeMessage");

			if(strStatus == "OK") {
				return 0;
			} else {
				return -2;
			}
		} else {
			return -1;
		}
	} NxCatchAll("CNxRetrieveNxReminderClient::ModifyClientActiveStatus : Invalid response from NxReminderServer");
	return -1;
}

int CNxRetrieveNxReminderClient::GetClientUsage(const CArray<long> & arrLicenses, std::vector<CNxRetrieveNxReminderClient::ClientInfo> & vClientUsageInfo)
{
	MSXML2::IXMLDOMNodePtr pXMLResponse;
	CString strXML = "";

	CString strLicenseType;
	int nNumClients = arrLicenses.GetCount();

	// Construct the list of client licenses
	CString strClients = "";
	CString strTemp;
	for(int i = 0; i < nNumClients; i++) {
		strTemp.Format("%li", arrLicenses.GetAt(i));
		strClients += GetXMLElementValuePair("long", strTemp);
	}

	// Construct the XML value pair
	strXML += GetXMLElementValuePair("username", m_strUsername);
	strXML += GetXMLElementValuePair("password", m_strPassword);

	if(strClients == ""){
		strXML += "<licenseIDs/>";
	} else {
		strXML += GetXMLElementValuePair("licenseIDs", strClients);
	}

	strXML += "<startBillingStartDate/><endBillingStartDate/>";

	try{
		// Try and get a response
		pXMLResponse = CallSoapFunction(GetNxReminderURL(), GetNxReminderURI(), "GetClientUsage", strXML);

		// If we get a valid response, try and parse it.
		if(pXMLResponse != NULL) {
#ifdef _DEBUG
			CString strXMLResponse = (LPCTSTR)pXMLResponse->Getxml();
			strXMLResponse.Replace("><", ">\r\n<");
#endif
			// We need to burrow down for each client to retrieve their tracking records.
			// The first child contains the status code.
			CString strStatus = GetXMLNodeText(pXMLResponse, "ResponseCode");
			// The first child's last child contains a list of InternalClientUsageResponses.
			MSXML2::IXMLDOMNodeListPtr pResponses = pXMLResponse->GetlastChild()->GetchildNodes();

			// DOM Node List Pointer for the Tracking records within the InternalClientUsageResponses.
			MSXML2::IXMLDOMNodeListPtr pTrackingRecords;

			// Temporary pointers
			MSXML2::IXMLDOMNodePtr pXMLNode, pXMLTrackingRecord;

			// If the response is okay, go ahead and parse it.
			if(strStatus == "OK") {
#ifdef _DEBUG
				int nNumResponse = pResponses->Getlength();
				int nNumRecords;
#endif

				// For each InternalClientUsageResponse, there is a set of TrackingRecords.
				int nResponseLength = pResponses->Getlength();
				for(int i = 0; i < pResponses->Getlength(); i++) {
					// Get the last child, which should be the TrackingRecord, and get a list of all its nodes.
					pXMLNode = pResponses->Getitem(i)->GetlastChild();
					pTrackingRecords = pXMLNode->GetchildNodes();
#ifdef _DEBUG
					nNumRecords = pTrackingRecords->Getlength();
#endif
					// For each of the tracking records, construct the client info struct and push it to the vector of results.
					for(int j = 0; j < pTrackingRecords->Getlength(); j++) {
						pXMLTrackingRecord = pTrackingRecords->Getitem(j);
						// Populate the struct and push it to the vector.
						CNxRetrieveNxReminderClient::ClientInfo NexRemClient(pResponses->Getitem(i), pXMLTrackingRecord);
						vClientUsageInfo.push_back(NexRemClient);
					}
				}
				return 0;
			} else {
				// Something went wrong, let the calling function know.
				return -1;
			}
		} 
	} NxCatchAll("CNxRetrieveNxReminderClient::GetClientUsage : Invalid response from NxReminderServer");
	return -1;
}

// (f.dinatale 2010-12-14) - PLID 41275 - Added the ability to parse the GetClientUsage SOAP message.
int CNxRetrieveNxReminderClient::GetClientUsage(const CArray<long> & arrLicenses, std::vector<CNxRetrieveNxReminderClient::ClientInfo> & vClientUsageInfo, COleDateTime dtStart, COleDateTime dtEnd)
{
	MSXML2::IXMLDOMNodePtr pXMLResponse;
	CString strXML = "";

	CString strLicenseType;
	int nNumClients = arrLicenses.GetCount();

	// Construct the list of client licenses
	CString strClients = "";
	CString strTemp;
	for(int i = 0; i < nNumClients; i++) {
		strTemp.Format("%li", arrLicenses.GetAt(i));
		strClients += GetXMLElementValuePair("long", strTemp);
	}

	// Construct the XML value pairs
	strXML += GetXMLElementValuePair("username", m_strUsername);
	strXML += GetXMLElementValuePair("password", m_strPassword);

	if(strClients == ""){
		strXML += "<licenseIDs />";
	} else {
		strXML += GetXMLElementValuePair("licenseIDs", strClients);
	}


	if(dtStart == NULL && dtEnd == NULL) {
		strXML += "<startBillingStartDate /><endBillingStartDate />";
	} else {
		strXML += GetXMLElementValuePair("startBillingStartDate ", dtStart.Format(VAR_DATEVALUEONLY));
		strXML += GetXMLElementValuePair("endBillingStartDate ", dtEnd.Format(VAR_DATEVALUEONLY));
	}

	try{
		// Try and get a response
		pXMLResponse = CallSoapFunction(GetNxReminderURL(), GetNxReminderURI(), "GetClientUsage", strXML);

		// If we get a valid response, try and parse it.
		if(pXMLResponse != NULL) {
#ifdef _DEBUG
			CString strXMLResponse = (LPCTSTR)pXMLResponse->Getxml();
			strXMLResponse.Replace("><", ">\r\n<");
#endif
			// We need to burrow down for each client to retrieve their tracking records.
			// The first child contains the status code.
			CString strStatus = GetXMLNodeText(pXMLResponse, "ResponseCode");
			// The first child's last child contains a list of InternalClientUsageResponses.
			MSXML2::IXMLDOMNodeListPtr pResponses = pXMLResponse->GetlastChild()->GetchildNodes();

			// DOM Node List Pointer for the Tracking records within the InternalClientUsageResponses.
			MSXML2::IXMLDOMNodeListPtr pTrackingRecords;

			// Temporary pointers
			MSXML2::IXMLDOMNodePtr pXMLNode, pXMLTrackingRecord;

			// If the response is okay, go ahead and parse it.
			if(strStatus == "OK") {
#ifdef _DEBUG
				int nNumResponse = pResponses->Getlength();
				int nNumRecords;
#endif

				// For each InternalClientUsageResponse, there is a set of TrackingRecords.
				int nResponseLength = pResponses->Getlength();
				for(int i = 0; i < pResponses->Getlength(); i++) {
					// Get the last child, which should be the TrackingRecord, and get a list of all its nodes.
					pXMLNode = pResponses->Getitem(i)->GetlastChild();
					pTrackingRecords = pXMLNode->GetchildNodes();
#ifdef _DEBUG
					nNumRecords = pTrackingRecords->Getlength();
#endif
					// For each of the tracking records, construct the client info struct and push it to the vector of results.
					for(int j = 0; j < pTrackingRecords->Getlength(); j++) {
						pXMLTrackingRecord = pTrackingRecords->Getitem(j);
						// Populate the struct and push it to the vector.
						CNxRetrieveNxReminderClient::ClientInfo NexRemClient(pResponses->Getitem(i), pXMLTrackingRecord);
						vClientUsageInfo.push_back(NexRemClient);
					}
				}
				return 0;
			} else {
				// Something went wrong, let the calling function know.
				return -1;
			}
		} 
	} NxCatchAll("CNxRetrieveNxReminderClient::GetClientUsage : Invalid response from NxReminderServer");
	return -1;
}

CString CNxRetrieveNxReminderClient::GetNxReminderURL()
{
	if (IsProductionAccount()) {
		// (r.gonet 09-08-2014) - PLID 63583 - We now get the URL from a property.
		CString strUrl = GetRemotePropertyText("NxReminderProductionURL", "https://nexreminder.nextech.com/NexReminder/InternalServices.asmx", 0, "<None>", true);
		// (r.gonet 09-08-2014) - PLID 63583 - Warn about using the real production URL if this isn't internal.
		if (VarString(GetAPISubkey()).CompareNoCase("Internal") != 0 &&
			(strUrl.CompareNoCase("http://nexreminder.nextech.com/NexReminder/InternalServices.asmx") == 0 ||
			strUrl.CompareNoCase("https://nexreminder.nextech.com/NexReminder/InternalServices.asmx") == 0)) {
			if (IDNO == MsgBox(MB_ICONWARNING | MB_YESNO, "!!! WARNING !!!: \r\n\r\nYou are about to connect to the LIVE NexReminder Server Internal Service from a database other than Internal. You "
				"could mess with live client licenses by doing this. Are you SURE want to continue? Otherwise, please look into the properties "
				"'NxReminderProductionMode' and 'NxReminderTestURL' as alternatives.")) {
				return "";
			}
		}
		return strUrl;
	} else {
		// (r.gonet 09-08-2014) - PLID 63583 - We now get the URL from a property.
		CString strUrl = GetRemotePropertyText("NxReminderTestURL", "", 0, "<None>", true);
		if (strUrl.IsEmpty()) {
			MsgBox(MB_ICONWARNING | MB_OK, "No value is configured for the NexReminder Internal Service URL. Please look into the properties "
				"'NxReminderProductionMode' and 'NxReminderTestURL'.");
		}
		return strUrl;
	}
}

CString CNxRetrieveNxReminderClient::GetNxReminderURI()
{
	// (r.gonet 09-08-2014) - PLID 63583 - We now get the URI from a property.
	return GetRemotePropertyText("NxReminderURI", "https://nexreminder.nextech.com/Internal", 0, "<None>", true);
}

// (z.manning 2010-11-23 11:29) - PLID 41560 - Gets the account nickname from client services instead
// of from internal services.
// (f.dinatale 2010-12-06) - PLID 40829 - Nickname is no longer needed
/*CString CNxRetrieveNxReminderClient::GetNicknameFromServer()
{
	MSXML2::IXMLDOMNodePtr pXMLResponse;
	CString strXML = "";

	strXML += GetXMLElementValuePair("licenseNumber", m_strLicense);
	pXMLResponse = CallSoapFunction(GetNxReminderClientURL(), GetNxReminderClientURI(), "GetNickname", strXML);

	if(pXMLResponse != NULL)
	{
#ifdef _DEBUG
		CString strXMLResponse = (LPCTSTR)pXMLResponse->Getxml();
		strXMLResponse.Replace("><", ">\r\n<");
#endif

		CString strNickname = GetXMLNodeText(pXMLResponse, "Value");
		return strNickname;
	}
	else {
		ThrowNxException("CNxRetrieveNxReminderClient::GetNicknameFromServer - Invalid response from NxReminderServer");
		return "";
	}
}*/
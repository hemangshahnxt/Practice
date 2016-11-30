#pragma once

#include "SOAPUtils.h"
#include "NxException.h"

// (f.dinatale 2010-10-26) - PLID 40827 - dialog created
// NxReminderSettingsDlg dialog
class CNxRetrieveNxReminderClient
{

public:
	// Constructor
	CNxRetrieveNxReminderClient(const CString& strLicense);

	// (f.dinatale 2010-12-10) - PLID 41275 - Client Usage structures to store data.
	struct ClientInfo {
		ClientInfo()
			: nClientID(-1)
			, bActive(false)
			, nAllottedMessages(0)
			, strStartDate("")
			, strEndDate("")
			, nAllottedCredits(0)
			, nRemainingCredits(0)
			, nCreditsUsedSending(0)
			, nCreditsUsedReceiving(0)
			, nMessagesSent(0)
			, nMessagesReceived(0)
		{};

		ClientInfo(MSXML2::IXMLDOMNodePtr pInternalClientUsageResponseNode, MSXML2::IXMLDOMNodePtr pTrackingRecord)
			: nClientID(atol(GetXMLNodeText(pInternalClientUsageResponseNode, "ClientID")))
			, bActive((GetXMLNodeText(pInternalClientUsageResponseNode, "Active") == "true" ? true : false))
			, nAllottedMessages(atol(GetXMLNodeText(pInternalClientUsageResponseNode, "AllottedMessages")))
			, strStartDate(GetXMLNodeText(pTrackingRecord, "StartTime"))
			, strEndDate(GetXMLNodeText(pTrackingRecord, "EndTime"))
			, nAllottedCredits(atol(GetXMLNodeText(pTrackingRecord, "AllottedCredits")))
			, nRemainingCredits(atol(GetXMLNodeText(pTrackingRecord, "RemainingCredits")))
			, nCreditsUsedSending(atol(GetXMLNodeText(pTrackingRecord, "CreditsUsedSending")))
			, nCreditsUsedReceiving(atol(GetXMLNodeText(pTrackingRecord, "CreditsUsedReceiving")))
			, nMessagesSent(atol(GetXMLNodeText(pTrackingRecord, "MessagesSent")))
			, nMessagesReceived(atol(GetXMLNodeText(pTrackingRecord, "MessagesReceived")))
		{};

		long nClientID;
		bool bActive;
		long nAllottedMessages;
		CString strStartDate;
		CString strEndDate;
		long nAllottedCredits;
		long nRemainingCredits;
		long nCreditsUsedSending;
		long nCreditsUsedReceiving;
		long nMessagesSent;
		long nMessagesReceived;
		};

	// Getters
	//CString GetNickname();
	CString GetLicenseKey();
	int GetTierID();
	int GetAllotedMessages();
	int GetRemainingMessages();
	// (f.dinatale 2010-11-30) - PLID 40829 - Needed a way to set and get if the client is active.
	bool IsActive();

	// Setters
	//void SetNickname(const CString& strNickname);
	void SetLicenseKey(const CString& strLicense);
	void SetTierID(const int& nTier);
	void SetAllotedMessages(const int& nMessages);
	void SetRemainingMessages(const int& nMessages);
	// (f.dinatale 2010-11-30) - PLID 40829 - Needed a way to set and get if the client is active.
	void SetActive(const bool& bActive);

	// Other
	int GetClientInfo();
	// (f.dinatale 2010-12-01) - PLID 41275 - Added the ability to retrieve reporting information.
	int GetClientUsage(const CArray<long> & arrLicenses, std::vector<CNxRetrieveNxReminderClient::ClientInfo> & vClientUsageInfo, COleDateTime dtStart, COleDateTime dtEnd);
	int GetClientUsage(const CArray<long> & arrLicenses, std::vector<CNxRetrieveNxReminderClient::ClientInfo> & vClientUsageInfo);
	int ModifyClientActiveStatus(const bool& bActive);
	int ModifyClientLicense(long nLicenseType, long nAllotedMessages);
	//int ModifyClientNickname(const CString& strNickname);
	
	// (z.manning 2010-11-23 11:29) - PLID 41560 - Gets the account nickname from client services instead
	// of from internal services.
	// (f.dinatale 2010-12-06) - PLID 40829 - Nickname is no longer needed.
	//CString GetNicknameFromServer();

protected:
	// Protected Methods
	BOOL IsProductionAccount();
	CString GetNxReminderURL();
	CString GetNxReminderURI();

	// Member Variables
	// (f.dinatale 2010-11-30) - PLID 40829 - Nickname no longer needed.
	//CString m_strNickname;
	CString m_strLicense;
	int m_nTierID;
	int m_nAllotedMessages;
	int m_nRemainingMessages;
	bool m_bActive;
	CString m_strUsername;
	CString m_strPassword;
};
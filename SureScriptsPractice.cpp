#include "stdafx.h"
#include "SureScriptsPractice.h"
#include <typeinfo>
#include "GlobalAuditUtils.h"
#include "AuditTrail.h"
#include "SOAPUtils.h"
#include "MiscSystemUtils.h"
#include "NxAutoQuantum.h"

// (a.walling 2009-10-13 10:01) - PLID 35930
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37026 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



namespace SureScripts {
	BOOL IsEnabled()
	{
		// (j.fouts 2012-12-31 11:14) - PLID 54400 - Check for the SureScripts license rather than any E-Prescribing license
		return (g_pLicense->HasEPrescribing(CLicense::cflrSilent) == CLicense::eptSureScripts);
	}

	// (a.walling 2009-04-09 12:32) - PLID 33895 - Insert message into outgoing queue
	// (a.walling 2009-04-21 13:37) - PLID 34032 - Allow updating existing
	void QueueOutgoingMessage(ADODB::_Connection *lpCon, SureScripts::Messages::MessageType& message, long nPatientID, BOOL bNotify, long nPendingSureScriptsMessageID)
	{
		SureScripts::MessageType messageType = SureScripts::mtInvalid;

		CString strPrescriberOrderNumber;

		// if a newrx message, put the prescription ID in
		if (messageType == SureScripts::mtInvalid) {
			try {
				SureScripts::Messages::NewRxMessage& NewRx = dynamic_cast<SureScripts::Messages::NewRxMessage&>(message);

				messageType = SureScripts::mtNewRx;

				strPrescriberOrderNumber = NewRx.NewRxBody.PrescriberOrderNumber;
			} catch (std::bad_cast) {
			}
		}

		if (messageType == SureScripts::mtInvalid) {
			try {
				SureScripts::Messages::RefillResponseMessage& RefillResponse = dynamic_cast<SureScripts::Messages::RefillResponseMessage&>(message);

				messageType = SureScripts::mtRefillResponse;

				strPrescriberOrderNumber = RefillResponse.RefillResponseBody.PrescriberOrderNumber;
			} catch (std::bad_cast) {
			}
		}

		
		if (messageType == SureScripts::mtInvalid) {
			ThrowNxException("QueueOutgoingMessage only supports NewRx, RefillResponses");
		}

		_variant_t varPatientID = g_cvarNull;
		if (nPatientID != -1) {
			varPatientID = nPatientID;
		}

		_variant_t varRelatesToMessageID = g_cvarNull;
		if (!message.Header.RelatesToMessageID.IsEmpty()) {
			varRelatesToMessageID = message.Header.RelatesToMessageID;
		}

		if (messageType == SureScripts::mtNewRx && nPendingSureScriptsMessageID == -2) {
			// (a.walling 2009-04-22 15:19) - PLID 33948 - We should look for it
			if (strPrescriberOrderNumber.IsEmpty()) {
				ThrowNxException("Cannot find prescriber order number for existing prescription!");
			}

			ADODB::_RecordsetPtr prsPendingID = CreateParamRecordset(lpCon,
				FormatString(
				"SELECT SureScriptsMessagesT.ID FROM SureScriptsMessagesT "
				"INNER JOIN PatientMedications ON SureScriptsMessagesT.PatientMedicationID = PatientMedications.ID "
				"WHERE PatientMedications.UID = {STRING} "
				"AND NeedsAttention = 1 "
				"AND MessageType = %li "
				"AND DateReceived IS NULL ", SureScripts::mtPendingRx),
				FormatGUIDAsStringForSql(strPrescriberOrderNumber));

			if (!prsPendingID->eof) {
				nPendingSureScriptsMessageID = AdoFldLong(prsPendingID, "ID");
			} else {
				nPendingSureScriptsMessageID = -1;
			}
		}
		
		if (nPendingSureScriptsMessageID != -1) {
			// (a.walling 2009-04-21 13:38) - PLID 34032 - Update an existing message (PrescriptionID and PatientID will already be set)
			ASSERT(messageType == SureScripts::mtNewRx);
			if (messageType != SureScripts::mtNewRx) {
				ThrowNxException("Can only update pending messages for NewRx");
			}

			// (a.walling 2009-04-23 15:24) - PLID 34032 - Need to ensure the dates are set to NULL.
			ExecuteParamSql(lpCon, 
				"UPDATE SureScriptsMessagesT "
				"SET MessageID = {STRING}, RelatesToMessageID = {VT_BSTR}, Message = {STRING}, MessageType = {INT}, DateSent = NULL, DateReceived = NULL, Sender = {STRING}, Recipient = {STRING}, NeedsAttention = 0 "
				"WHERE ID = {INT}",
				message.Header.MessageID, varRelatesToMessageID, message.ToString(), messageType, message.Header.From, message.Header.To,
				nPendingSureScriptsMessageID);
		} else {
			if (!strPrescriberOrderNumber.IsEmpty()) {
				ExecuteParamSql(lpCon, 
					"DECLARE @PatientMedicationID INT;\r\n"
					"SET @PatientMedicationID = (SELECT TOP 1 ID FROM PatientMedications WHERE UID = {STRING});\r\n"
					"INSERT INTO SureScriptsMessagesT (MessageID, RelatesToMessageID, Message, MessageType, PatientID, PatientMedicationID, Sender, Recipient) "
					"VALUES ({STRING}, {VT_BSTR}, {STRING}, {INT}, {VT_I4}, @PatientMedicationID, {STRING}, {STRING})\r\n",
					FormatGUIDAsStringForSql(strPrescriberOrderNumber), message.Header.MessageID, varRelatesToMessageID, message.ToString(), messageType, varPatientID, message.Header.From, message.Header.To);
			} else {
				ExecuteParamSql(lpCon, 
					"INSERT INTO SureScriptsMessagesT (MessageID, RelatesToMessageID, Message, MessageType, PatientID, Sender, Recipient) "
					"VALUES ({STRING}, {VT_BSTR}, {STRING}, {INT}, {VT_I4}, {STRING}, {STRING})\r\n",
					message.Header.MessageID, varRelatesToMessageID, message.ToString(), messageType, varPatientID, message.Header.From, message.Header.To);
			}
		}

		TRACE("Queued SureScripts message %s\n", message.Header.MessageID);
		LogDetail("Queued SureScripts message %s\n", message.Header.MessageID);

		if (bNotify) {
			NotifyNxServerOfNewMessages();
		}
	}

	// (a.walling 2009-04-21 12:54) - PLID 34032 - A message may have failed to generate, but is still wanted to be prescribed
	// (a.walling 2009-04-24 11:30) - PLID 34033 - Pass in the provider and a description
	void QueuePendingNewRx(ADODB::_Connection *lpCon, long nPatientID, long nPrescriptionID, long nProviderID, const CString& strMedicationName, const CString& strErrorDescription)
	{
		if (nPatientID == -1 || nPrescriptionID == -1) {
			ThrowNxException("QueuePendingNewRx requires patient and prescription");
		}

		CString strFakeMessage;
		if (strErrorDescription.IsEmpty()) {
			strFakeMessage.Format(
				"<Message xmlns=\"http://www.surescripts.com/messaging\">"
					"<Body>"
						"<NewRx>"
							"<MedicationPrescribed>"
								"<DrugDescription>%s</DrugDescription>"
							"</MedicationPrescribed>"
						"</NewRx>"
					"</Body>"
				"</Message>", XMLEncode(strMedicationName));
		} else {
			strFakeMessage.Format(
				"<Message xmlns=\"http://www.surescripts.com/messaging\">"
					"<Body>"
						"<NewRx>"
							"<MedicationPrescribed>"
								"<DrugDescription>%s</DrugDescription>"
							"</MedicationPrescribed>"
							"<ErrorDescription>%s</ErrorDescription>"
						"</NewRx>"
					"</Body>"
				"</Message>", XMLEncode(strMedicationName), XMLEncode(strErrorDescription));
		}

		if (nProviderID == -1) {
			// (a.walling 2009-04-22 15:30) - PLID 34032 - Try to update an existing one if possible
			ExecuteParamSql(lpCon, 
				"UPDATE SureScriptsMessagesT SET Message = {STRING}, Sender = '' WHERE NeedsAttention = 1 AND DateReceived IS NULL AND MessageType = {INT} AND PatientID = {INT} AND PatientMedicationID = {INT};\r\n"
				"IF @@ROWCOUNT = 0 INSERT INTO SureScriptsMessagesT (MessageID, RelatesToMessageID, Message, MessageType, PatientID, PatientMedicationID, DateSent, Sender, Recipient, NeedsAttention) "
				"VALUES ({STRING}, '', {STRING}, {INT}, {INT}, {INT}, GetDate(), '', '', 1);",
				strFakeMessage, (long)SureScripts::mtPendingRx, nPatientID, nPrescriptionID,
				NewPlainUUID(), strFakeMessage, (long)SureScripts::mtPendingRx, nPatientID, nPrescriptionID);
				// GetNewUniqueID now in NxSystemUtilitiesLib
		} else {
			// (a.walling 2009-04-22 15:30) - PLID 34032 - Try to update an existing one if possible
			ExecuteParamSql(lpCon, 
				"DECLARE @ProviderSPI NVARCHAR(50);\r\n"
				"DECLARE @Sender NVARCHAR(80);\r\n"

				"SET @ProviderSPI = (SELECT SPI FROM ProvidersT WHERE PersonID = {INT});\r\n"

				"IF @ProviderSPI IS NOT NULL AND @ProviderSPI <> '' SET @Sender = 'mailto:' + @ProviderSPI + '.spi@surescripts.com' ELSE SET @Sender = '';\r\n"

				"UPDATE SureScriptsMessagesT SET Message = {STRING}, Sender = @Sender WHERE NeedsAttention = 1 AND DateReceived IS NULL AND MessageType = {INT} AND PatientID = {INT} AND PatientMedicationID = {INT};\r\n"
				"IF @@ROWCOUNT = 0 INSERT INTO SureScriptsMessagesT (MessageID, RelatesToMessageID, Message, MessageType, PatientID, PatientMedicationID, DateSent, Sender, Recipient, NeedsAttention) "
				"VALUES ({STRING}, '', {STRING}, {INT}, {INT}, {INT}, GetDate(), @Sender, '', 1);",
				nProviderID,
				strFakeMessage, (long)SureScripts::mtPendingRx, nPatientID, nPrescriptionID,
				NewPlainUUID(), strFakeMessage, (long)SureScripts::mtPendingRx, nPatientID, nPrescriptionID);
				// GetNewUniqueID now in NxSystemUtilitiesLib
		}
		
		// (a.walling 2009-04-28 09:53) - PLID 34032 - Ensure we refresh
		CClient::RefreshTable(NetUtils::SureScriptsMessagesT);
	}
		
	// (a.walling 2009-04-21 13:15) - PLID 34032 - Will not be sent to surescripts after all
	void RemovePendingNewRx(ADODB::_Connection *lpCon, long nSureScriptsMessageID)
	{
		if (nSureScriptsMessageID == -1) {
			ThrowNxException("RemovePendingNewRx requires message id");
		}
		// set datereceived as the time it was 'removed'
		ExecuteParamSql(lpCon, 
			"UPDATE SureScriptsMessagesT SET NeedsAttention = 0, DateReceived = GetDate() WHERE MessageType = {INT} AND ID = {INT}",
			(long)SureScripts::mtPendingRx, nSureScriptsMessageID);
	}
	
	// (a.walling 2009-04-22 17:33) - PLID 34046
	// (a.walling 2014-01-30 00:00) - PLID 60546 - Quantize
	void DeletePendingNewRxInEMRBatch(Nx::Quantum::Batch& strSqlBatch, long nPrescriptionID)
	{
		if (nPrescriptionID == -1) {
			return;
		}

#pragma TODO("Is this dead code?")
		// this is EMR, will not be audited, although the prescription deletion itself would be audited
		AddStatementToSqlBatch(strSqlBatch,
			"EXEC ('"
			"DECLARE @PendingSureScriptsMessagesT TABLE (ID INT NOT NULL) \r\n" // (a.walling 2009-04-22 17:50) - PLID 33948
			"INSERT INTO @PendingSureScriptsMessagesT(ID) "
				"SELECT ID FROM SureScriptsMessagesT WHERE DateReceived IS NULL AND MessageType = %li AND NeedsAttention = 1 "
				"AND PatientMedicationID = %li;\r\n"
			"UPDATE SureScriptsMessagesT SET NeedsAttention = 0, DateReceived = GetDate() WHERE ID IN (SELECT ID FROM @PendingSureScriptsMessagesT);\r\n"
			"INSERT INTO SureScriptsActionsT (MessageID, ActionType, UserID, Date) "
				"SELECT ID, %li, %li, GetDate() FROM @PendingSureScriptsMessagesT "
			"');",
			(long)SureScripts::mtPendingRx, nPrescriptionID,
			SureScripts::atPendingPrescriptionRemoved, GetCurrentUserID()
		);
	}
	
	// (a.walling 2009-04-22 17:33) - PLID 34046
	void DeletePendingNewRx(ADODB::_Connection *lpCon, long nPrescriptionID)
	{
		if (nPrescriptionID == -1) {
			return;
		}

		// this is EMR, will not be audited, although the prescription deletion itself would be audited
		ExecuteParamSql(lpCon,
			"DECLARE @PendingSureScriptsMessagesT TABLE (ID INT NOT NULL);\r\n"
			"INSERT INTO @PendingSureScriptsMessagesT(ID) "
				"SELECT ID FROM SureScriptsMessagesT WHERE DateReceived IS NULL AND MessageType = {INT} AND NeedsAttention = 1 "
				"AND PatientMedicationID = {INT};\r\n"
			"UPDATE SureScriptsMessagesT SET NeedsAttention = 0, DateReceived = GetDate() WHERE ID IN (SELECT ID FROM @PendingSureScriptsMessagesT);\r\n"
			"INSERT INTO SureScriptsActionsT (MessageID, ActionType, UserID, Date) "
				"SELECT ID, {INT}, {INT}, GetDate() FROM @PendingSureScriptsMessagesT "
			"DELETE FROM @PendingSureScriptsMessagesT",
			(long)SureScripts::mtPendingRx, nPrescriptionID,
			SureScripts::atPendingPrescriptionRemoved, GetCurrentUserID()
		);
	}

	// (a.walling 2010-01-25 08:32) - PLID 37026 - Added nPatientID
	void CreateAction(long nID, SureScripts::ActionType actionType, const CString& strDescription, long nPatientID, const CString &strPatientName, BOOL bUnsetNeedsAction)
	{
		AuditEventItems aeiItem = SureScripts::GetAuditItemForAction(actionType);

		//TES 4/13/2009 - PLID 33890 - Update the data
		if (bUnsetNeedsAction) {
			ExecuteParamSql("UPDATE SureScriptsMessagesT SET NeedsAttention = 0 WHERE ID = {INT};"
				"INSERT INTO SureScriptsActionsT (MessageID, ActionType, UserID, Date) "
				"VALUES ({INT}, {INT}, {INT}, getdate())",
				nID, nID, (int)actionType, GetCurrentUserID());
		} else {
			ExecuteParamSql("INSERT INTO SureScriptsActionsT (MessageID, ActionType, UserID, Date) "
				"VALUES ({INT}, {INT}, {INT}, getdate())",
				nID, (int)actionType, GetCurrentUserID());
		}

		//TES 4/13/2009 - PLID 33890 - Audit
		// (a.walling 2010-01-25 10:02) - PLID 37026 - Audit the patient ID
		AuditEvent(nPatientID, strPatientName, BeginNewAuditEvent(), aeiItem, nID, "", 
			strDescription, aepMedium);

		// (a.walling 2009-04-28 09:53) - PLID 34032 - Ensure we refresh
		CClient::RefreshTable(NetUtils::SureScriptsMessagesT);
	}

	void NotifyNxServerOfNewMessages()
	{
		CClient::Send(PACKET_TYPE_SURESCRIPTS_MESSAGES_PENDING, NULL, 0);
	}

	CString FormatPhoneForImport(const CString& strPhone)
	{
		if (strPhone.IsEmpty())
			return strPhone;

		CString strValue;

		BOOL bFormatPhoneNums = GetRemotePropertyInt("FormatPhoneNums", 1, 0, "<None>", true); 
		if (bFormatPhoneNums) {
			CString strPhoneFormat = GetRemotePropertyText("PhoneFormatString", "(###) ###-####", 0, "<None>", true);
			strValue = FormatPhone(strPhone, strPhoneFormat);
			return strValue;
		} else {
			return strPhone;
		}
	}


	// (a.walling 2009-04-14 16:50) - PLID 33951 - Add the pharmacy to data
	long AddPharmacy(ADODB::_Connection* lpCon, MSXML2::IXMLDOMDocument2Ptr pDocument) {
		long nNewID = -1;

		pDocument->setProperty("SelectionNamespaces", "xmlns:ss='http://www.surescripts.com/messaging'");
		CString strName = GetTextFromXPath(pDocument, "//ss:Pharmacy/ss:StoreName");
		CString strOnlineAddress = GetTextFromXPath(pDocument, "//ss:Pharmacy/ss:Email");		
		CString strNCPDPID = GetTextFromXPath(pDocument, "//ss:Pharmacy/ss:Identification/ss:NCPDPID");
		CString strNPI = GetTextFromXPath(pDocument, "//ss:Pharmacy/ss:Identification/ss:NPI");

		CString strAddressLine1 = GetTextFromXPath(pDocument, "//ss:Pharmacy/ss:Address/ss:AddressLine1");
		CString strAddressLine2 = GetTextFromXPath(pDocument, "//ss:Pharmacy/ss:Address/ss:AddressLine2");
		CString strCity = GetTextFromXPath(pDocument, "//ss:Pharmacy/ss:Address/ss:City");
		CString strState = GetTextFromXPath(pDocument, "//ss:Pharmacy/ss:Address/ss:State");
		CString strZip = GetTextFromXPath(pDocument, "//ss:Pharmacy/ss:Address/ss:ZipCode");

		CString strWorkPhone;
		CString strAltPhone;
		CString strFax;


		MSXML2::IXMLDOMNodePtr pPhoneNumbers = pDocument->selectSingleNode("//ss:Pharmacy/ss:PhoneNumbers");
		if (pPhoneNumbers) {
			MSXML2::IXMLDOMNodeListPtr pNumbers = pPhoneNumbers->GetchildNodes();
			for (int i = 0; i < pNumbers->Getlength(); i++) {
				MSXML2::IXMLDOMNodePtr pNumber = pNumbers->Getitem(i);
				if (GetXMLNodeText(pNumber, "Qualifier").CompareNoCase("TE") == 0) {
					if (strWorkPhone.IsEmpty()) {
						strWorkPhone = GetXMLNodeText(pNumber, "Number");
					} else {
						strAltPhone = GetXMLNodeText(pNumber, "Number");
					}
				} else if (GetXMLNodeText(pNumber, "Qualifier").CompareNoCase("WP") == 0) {
					if (strWorkPhone.IsEmpty()) {
						strWorkPhone = GetXMLNodeText(pNumber, "Number");
					} else {
						strAltPhone = GetXMLNodeText(pNumber, "Number");
					}
				} else if (GetXMLNodeText(pNumber, "Qualifier").CompareNoCase("FX") == 0) {
					strFax = GetXMLNodeText(pNumber, "Number");
				} 
			}
		}

		// alright, let's go
		// (a.walling 2009-04-28 10:16) - PLID 33951 - Do not audit in a batch
		CString strBatch;
		strBatch.Format(
			"SET NOCOUNT ON;\r\n"
			"DECLARE @NewLocationID INT;\r\n"
			//"DECLARE @AuditEvent INT;\r\n"
			//"INSERT INTO AuditT (ChangedDate, ChangedByUserName, ChangedAtLocationName) VALUES (GetDate(),{STRING},{STRING});\r\n"
			//"SET @AuditEvent = (SELECT Convert(int, SCOPE_IDENTITY()));\r\n"
			"SET @NewLocationID = (SELECT COALESCE(MAX(LocationsT.ID), 0) + 1 FROM LocationsT);\r\n"

			"INSERT INTO LocationsT(ID, Name, Address1, Address2, City, State, Zip, Phone, Phone2, Fax, OnLineAddress, Notes, TypeID, NPI, LinkToDirectory) "
			"VALUES(@NewLocationID, {STRING}, {STRING}, {STRING}, {STRING}, {STRING}, {STRING}, {STRING}, {STRING}, {STRING}, {STRING}, '', 3, {STRING}, 1);\r\n"

			"INSERT INTO PharmacyIDT(LocationID, NCPDPID, FileID, StateLicNum, MedicareNum, MedicaidNum, PPONum, PayerID, BINLocNum, DEANum, HIN, NAICCode) "
			"VALUES(@NewLocationID, {STRING}, '', '', '', '', '', '', '', '', '', '');\r\n"
	
			//"INSERT INTO AuditDetailsT (AuditID, PersonName, ItemID, RecordID, OldValue, NewValue, Priority, Type) "
			//"VALUES (@AuditEvent, '', %li, CONVERT(NVARCHAR(20), @NewLocationID), '', {STRING}, %li, %li);\r\n"

			"SET NOCOUNT OFF\r\n"
			"SELECT @NewLocationID AS NewID"
		);

		ADODB::_RecordsetPtr prs = CreateParamRecordset(lpCon, strBatch,
			//GetCurrentUserName(), GetCurrentLocationName(),
			strName, strAddressLine1, strAddressLine2, strCity, strState, strZip, FormatPhoneForImport(strWorkPhone), FormatPhoneForImport(strAltPhone), FormatPhoneForImport(strFax), strOnlineAddress, strNPI,
			strNCPDPID);
			//strName);

		if (!prs->eof) {
			nNewID = AdoFldLong(prs, "NewID", -1);
			AuditEvent(-1, "", BeginNewAuditEvent(), aeiLocationCreate, nNewID, "", strName, aepMedium, aetCreated);
		}
		
		return nNewID;
	}

	// (a.walling 2009-04-16 17:07) - PLID 33951
	AuditEventItems GetAuditItemForAction(SureScripts::ActionType at)
	{
		switch(at) {
			case atErrorAcknowledged:
				return aeiSureScriptsErrorMessageAcknowledged;
			case atRefillRequestApproved:
				return aeiSureScriptsRefillRequestApproved;
			case atRefillRequestDenied:
				return aeiSureScriptsRefillRequestDenied;
			case atRefillRequestDeniedNewRx:
				return aeiSureScriptsRefillRequestDeniedNewRx;
			case atInvalidMessageAcknowledged:	//TES 4/17/2009 - PLID 33890
				return aeiSureScriptsInvalidMessageAcknowledged;
			case atPendingPrescriptionRemoved: // (a.walling 2009-04-21 13:31) - PLID 34032
				return aeiSureScriptsPendingPrescriptionRemoved;
			case atPendingPrescriptionEdited:  // (a.walling 2009-04-21 15:41) - PLID 34032
				return aeiSureScriptsPendingPrescriptionEdited;
		}
		
		ThrowNxException("Invalid action type %li for auditing", (int)at);
		return aeiSureScriptsErrorMessageAcknowledged;
	}

}
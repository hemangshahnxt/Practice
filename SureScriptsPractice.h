#pragma once

#include "SureScriptsUtils.h"
#include "SureScripts.h"

#include "GlobalAuditUtils.h"

#include "NxAutoQuantumFwd.h"

namespace SureScripts {
	BOOL IsEnabled();

	// (a.walling 2009-04-09 12:32) - PLID 33895 - Insert message into outgoing queue
	// (a.walling 2009-04-22 15:19) - PLID 33948 - If we get a special PendingSureScriptsMessageID of -2, then look for it.
	void QueueOutgoingMessage(ADODB::_Connection *lpCon, SureScripts::Messages::MessageType& message, long nPatientID, BOOL bNotify, long nPendingSureScriptsMessageID) throw(...);
	// (a.walling 2009-04-21 12:54) - PLID 34032 - A message may have failed to generate, but is still wanted to be prescribed
	// (a.walling 2009-04-24 11:30) - PLID 34033 - Pass in the provider and a description
	void QueuePendingNewRx(ADODB::_Connection *lpCon, long nPatientID, long nPrescriptionID, long nProviderID, const CString& strMedicationName, const CString& strErrorDescription) throw(...);
	// (a.walling 2009-04-21 13:15) - PLID 34032 - Will not be sent to surescripts after all
	void RemovePendingNewRx(ADODB::_Connection *lpCon, long nSureScriptsMessageID) throw(...);

	// (a.walling 2009-04-22 17:33) - PLID 34046
	void DeletePendingNewRxInEMRBatch(Nx::Quantum::Batch& strSqlBatch, long nPrescriptionID) throw(...);
	// (a.walling 2009-04-22 17:33) - PLID 34046
	void DeletePendingNewRx(ADODB::_Connection *lpCon, long nPrescriptionID) throw(...);
	
	// (a.walling 2009-04-16 16:43) - PLID 33951
	//TES 4/17/2009 - PLID 33890 - Added a strPatientName parameter, for auditing.
	// (a.walling 2010-01-25 08:32) - PLID 37026 - Added nPatientID
	void CreateAction(long nID, SureScripts::ActionType actionType, const CString& strDescription, long nPatientID, const CString &strPatientName, BOOL bUnsetNeedsAction);
	void NotifyNxServerOfNewMessages();

	CString FormatPhoneForImport(const CString& strPhone);

	// (a.walling 2009-04-16 17:07) - PLID 33951
	AuditEventItems GetAuditItemForAction(SureScripts::ActionType at) throw(...);

	// (a.walling 2009-04-14 16:50) - PLID 33951 - Add the pharmacy to data
	long AddPharmacy(ADODB::_Connection* lpCon, MSXML2::IXMLDOMDocument2Ptr pDocument) throw(...);
}
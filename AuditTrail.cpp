// AuditTrail.cpp: implementation of the AuditTrail class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "practice.h"
#include "AuditTrail.h"
#include "Base64.h"
#include "practiceRc.h"
#include "syslog.h"
#include "SoapUtils.h"
#include <boost/container/flat_map.hpp>


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// (a.walling 2013-04-05 12:28) - PLID 56091 - AuditXML stuff for syslog internal to AuditTrail.cpp

// (a.walling 2009-12-17 12:58) - PLID 35779 - Format RFC 3881-compliant audit messages
class AuditXML {
public:
	AuditXML()
	{

	};

	~AuditXML()
	{

	};

	void CreateDocument(CString PersonName, long AuditID, long Item, long RecordID, CString OldValue, CString NewValue, int Priority, int Type);

	CString GetXML();
	void Validate();

	static LPCTSTR GetCRUD(int Type);

protected:
	MSXML2::IXMLDOMNodePtr CreateNode(LPCTSTR szName) throw(...); // creates a AM element
	MSXML2::IXMLDOMNodePtr CreateNodeWithText(LPCTSTR szName, LPCTSTR szText) throw(...); // creates a AM element with text
	MSXML2::IXMLDOMNodePtr CreateAttribute(LPCTSTR szName, _variant_t varValue) throw(...); // creates a AM attribute with a value
	MSXML2::IXMLDOMNodePtr CreateAttribute(LPCTSTR szName) throw(...); // creates a AM attribute
	MSXML2::IXMLDOMNodePtr CreateCode(LPCTSTR szElementName, _variant_t varCode, _variant_t varDisplayName, _variant_t varCodeSystemName, _variant_t varCodeSystem) throw(...);
	MSXML2::IXMLDOMNodePtr CreateTypeValuePair(LPCTSTR szElementName, LPCTSTR szType, LPCTSTR szValue);

	// namespaces
	static const LPCTSTR am; //http://audit-message-uri
	static const LPCTSTR NexTechAuditOID;
	static const LPCTSTR NexTechAuditOID_Name;	

	MSXML2::IXMLDOMDocument2Ptr Doc;
};

using namespace ADODB;

// (a.walling 2010-06-08 10:02) - PLID 38558 - Audit flags
// Currently this is just used within Practice to differentiate between UB92 and UB04 audit item descriptions
void EnsureAuditFlags()
{
	DWORD dwFlags = 0;

	UBFormType formType = GetUBFormType();
	if (formType == eUB92) {
		dwFlags |= afUB92;
	}

	SetGlobalAuditFlags(dwFlags);
}

// (a.walling 2009-12-17 18:00) - PLID 36626 - Is syslog enabled? Only check once per session
bool IsSyslogEnabled()
{
	static bool bCheckedSyslog = false;
	static bool bSyslogEnabled = FALSE;

	if (!bCheckedSyslog) {
		bCheckedSyslog = true;

		bSyslogEnabled = GetRemotePropertyInt("RelayAuditingToSyslog", FALSE, 0, "<None>", true) ? true : false;
	}
	
	return bSyslogEnabled;
}



//Begin a new Audit Event. Adds a new entry to AuditT and returns the new AuditID
long BeginNewAuditEvent()
{
	// (a.walling 2010-11-01 11:07) - PLID 40965 - Get rid of duplicated code
	return BeginNewAuditEvent(GetCurrentUserName());
}


//Added for NexWeb use
// (a.walling 2010-11-01 11:07) - PLID 40965 - Also called by BeginNewAuditEvent as well
long BeginNewAuditEvent(CString strUserName) 
{
	// (s.dhole 2013-12-11 12:21) - PLID 58958 Move to shared classs
	return BeginNewAuditEventEx( strUserName,GetCurrentLocationName(),NULL);

}

long BeginAuditTransaction() // BeginNewAuditEvent counterpart
{
	//TODO - DRT - 1/19/2007 - PLID 24316 - If you get a memory leak here, it is probably because
	//	you hit cancel on the debug-only popup with the sql change for the itementrydlg.  This
	//	will be fixed in a future scope.
	// (s.dhole 2013-12-11 12:21) - PLID 58958 Move to shared classs
	return BeginAuditTransactionEx(GetCurrentUserName(), GetCurrentLocationName());
}



void CommitAuditTransaction(long AuditID)
{
	// (s.dhole 2013-12-11 12:21) - PLID 58958 Move to shared classs
	try{
		CommitAuditTransactionEx(AuditID);
	}NxCatchAll(__FUNCTION__);
}


void RollbackAuditTransaction(long AuditID)
{
	// (s.dhole 2013-12-11 12:21) - PLID 58958 Move to shared classs
	try{
		RollbackAuditTransactionEx(AuditID);
	}NxCatchAll(__FUNCTION__);
}



//Audit a change for a pre-created Audit Event. This instance takes in a PersonName.

//Type is optional: 1 is changed (default), 2 is deleted, 3 is created
// (a.walling 2010-01-21 12:36) - PLID 37018 - Require a patient ID, or -1 if not applicable

// (a.walling 2010-01-21 12:36) - PLID 37018 - Require a patient ID, or -1 if not applicable
void AuditEvent(long PatientID, CString PersonName, long AuditID, long Item, long RecordID, CString OldValue, CString NewValue, int Priority, int Type)
{
	AuditEvent(AuditDetail(PatientID, PersonName, AuditID, Item, RecordID, OldValue, NewValue, Priority, Type),GetCurrentUserName(), GetCurrentLocationName());
}



// (j.jones 2007-08-30 09:54) - PLID 27221 - Behaves like AuditEvent, in that it supports transactions,
// but takes in a CPendingAuditInfo item which contains all the audit information. Optionally a
// new record ID can be sent in to override pPendingAuditInfo->nRecordID, incase we didn't have a
// valid record ID when we creatd pPendingAuditInfo.
void AuditPendingEvent(long nAuditID, CPendingAuditInfo *pPendingAuditInfo, long nNewRecordID /*= -1*/)
{
	long nRecordIDToUse = pPendingAuditInfo->m_nRecordID;

	//should we override it?
	if(nNewRecordID != -1)
		nRecordIDToUse = nNewRecordID;

	// (a.walling 2010-01-21 15:35) - PLID 37018 - Require a patient ID, or -1 if not applicable
	AuditEvent(pPendingAuditInfo->m_nPatientID, pPendingAuditInfo->m_strPersonName, nAuditID, pPendingAuditInfo->m_aeiItem, nRecordIDToUse, pPendingAuditInfo->m_strOldValue, pPendingAuditInfo->m_strNewValue, pPendingAuditInfo->m_aepPriority, pPendingAuditInfo->m_aetType);
}

using namespace ADODB;

ADODB::_RecordsetPtr GetAuditTrailReportRecordset(CString strSQL) {

	_RecordsetPtr rs(__uuidof(Recordset));
	rs->CursorLocation = adUseClient;
	// (a.walling 2009-08-11 13:25) - PLID 35178 - Use the snapshot connection
	rs->Open((LPCTSTR)strSQL, (LPDISPATCH)GetRemoteDataSnapshot(), adOpenDynamic, adLockBatchOptimistic, adCmdText);

	//DISCONNECT THE RECORDSET NOW! We don't want to make changes to the data.
	rs->PutRefActiveConnection(NULL);

	while(!rs->eof) {

		long ItemID = rs->Fields->Item["ItemID"]->Value.lVal;
		
		_variant_t var = _bstr_t(GetAuditItemDescription(ItemID));

		rs->Fields->Item["ItemType"]->Value = var;

		rs->MoveNext();
	}

	if(!rs->bof)
		rs->MoveFirst();

	return rs;
}

//NOTE: JJ - 10/2/2002- For all these, you'll see that I assign OldValue to ItemType. This is because any value
	//you update in the recordset has to correspond to a legitimate database field. You can't say
	//'' AS ItemType - that will give errors. Sigh.

CString GetAuditTrailFinancialReportSql() {

	// (z.manning 2009-05-22 17:25) - PLID 34330 - Honor the permission to view old and new value
	CString strOld, strNew;
	if(CheckCurrentUserPermissions(bioAdminAuditTrail, sptDynamic0, FALSE, FALSE, TRUE)) {
		strOld = "AuditDetailsT.OldValue";
		strNew = "AuditDetailsT.NewValue";
	}
	else {
		strOld = strNew = "''";
	}

	// (a.walling 2010-01-25 09:42) - PLID 37056 - Honor the permission to view the patient name
	CString strPersonName;
	if(CheckCurrentUserPermissions(bioAdminAuditTrail, sptDynamic1, FALSE, FALSE, TRUE)) {
		strPersonName = "AuditDetailsT.PersonName";
	} else {
		strPersonName = "CASE WHEN AuditDetailsT.InternalPatientID IS NULL THEN PersonName ELSE '' END AS PersonName";
	}

	//TES 7/10/2008 - PLID 30671 - Went ahead and added in some new Item types for audits that I added. Nobody else seems
	// to have been updating this query, but that's no reason I shouldn't.
	//TES 7/10/2008 - PLID 30676 - Updated again for payment provider audits.
	// (a.walling 2010-01-22 16:39) - PLID 37043 - Include patient ID and internal ID
	// (a.walling 2010-01-25 09:38) - PLID 37056 - Blank out the PersonName when a patient record and lacking appropriate permissions
	CString strSQL = FormatString("SELECT AuditT.ChangedByUserName AS UserName, AuditT.ChangedAtLocationName, AuditT.ChangedDate AS TDate, (CASE WHEN AuditDetailsT.Priority = 1 THEN 'High' WHEN AuditDetailsT.Priority = 2 THEN 'Medium' WHEN AuditDetailsT.Priority = 3 THEN 'Low' ELSE '' End) AS PriorityDesc, "
		"AuditDetailsT.ID, AuditDetailsT.AuditID, AuditDetailsT.RecordID, %s AS OldValue, %s AS NewValue, AuditDetailsT.Priority, AuditDetailsT.Type, %s, AuditDetailsT.ItemID, "
		"AuditDetailsT.Priority AS Priority, AuditDetailsT.ID AS AuditDetailID, "
					 "       AuditDetailsT.InternalPatientID, "
					 "       COALESCE(PatientsT.UserDefinedID, PersonT.ArchivedUserDefinedID) AS PatientID, "	
					 "       (CONVERT(nvarchar(3), AuditT.IPOctet1) + '.' + "
					 "		  CONVERT(nvarchar(3), AuditT.IPOctet2) + '.' + "
					 "		  CONVERT(nvarchar(3), AuditT.IPOctet3) + '.' + "
					 "		  CONVERT(nvarchar(3), AuditT.IPOctet4)) AS IPAddress, "
				"CASE WHEN (AuditDetailsT.ItemID >= 2025 AND AuditDetailsT.ItemID <= 2041) THEN 'Bill - ' + BillsT.Description "
				"WHEN ((AuditDetailsT.ItemID >= 2021 AND AuditDetailsT.ItemID <= 2024) AND LineItemT.Type = 1) THEN 'Payment - ' + LineItemT.Description "
				"WHEN ((AuditDetailsT.ItemID >= 2021 AND AuditDetailsT.ItemID <= 2024) AND LineItemT.Type = 2) THEN 'Adjustment - ' + LineItemT.Description "
				"WHEN ((AuditDetailsT.ItemID >= 2021 AND AuditDetailsT.ItemID <= 2024) AND LineItemT.Type = 3) THEN 'Refund - ' + LineItemT.Description "
				"WHEN ((AuditDetailsT.ItemID >= 2001 AND AuditDetailsT.ItemID <= 2020) AND LineItemT.Type = 10) THEN 'Charge - ' + LineItemT.Description "
				"WHEN ((AuditDetailsT.ItemID >= 2001 AND AuditDetailsT.ItemID <= 2020) AND LineItemT.Type = 11) THEN 'Quote Charge - ' + LineItemT.Description "
				"WHEN (AuditDetailsT.ItemID IN (2146, 2150, 2153)) THEN 'Payment - ' + LineItemT.Description "
				"WHEN (AuditDetailsT.ItemID IN (2147, 2151, 2154)) THEN 'Adjustment - ' + LineItemT.Description "
				"WHEN (AuditDetailsT.ItemID IN (2148, 2152, 2155)) THEN 'Refund - ' + LineItemT.Description "
				"WHEN (AuditDetailsT.ItemID IN (2149)) THEN 'Charge - ' + LineItemT.Description "
				"ELSE '' END AS Item, AuditDetailsT.OldValue AS ItemType "
				"FROM AuditT INNER JOIN AuditDetailsT ON AuditT.ID = AuditDetailsT.AuditID "
				"LEFT JOIN LineItemT ON AuditDetailsT.RecordID = LineItemT.ID "
				"LEFT JOIN BillsT ON AuditDetailsT.RecordID = BillsT.ID "
				"LEFT JOIN PersonT ON AuditDetailsT.InternalPatientID = PersonT.ID "
				"LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
				"WHERE (AuditDetailsT.ItemID >= 2001 AND AuditDetailsT.ItemID <= 3000)"
				, strOld, strNew, strPersonName);

	return strSQL;
}

CString GetAuditTrailSchedulerReportSql(LPCTSTR strPatientName) {

	// (z.manning 2009-05-22 17:25) - PLID 34330 - Honor the permission to view old and new value
	CString strOld, strNew;
	if(CheckCurrentUserPermissions(bioAdminAuditTrail, sptDynamic0, FALSE, FALSE, TRUE)) {
		strOld = "AuditDetailsT.OldValue";
		strNew = "AuditDetailsT.NewValue";
	}
	else {
		strOld = strNew = "''";
	}

	// (a.walling 2010-01-25 09:42) - PLID 37056 - Honor the permission to view the patient name
	CString strPersonName;
	if(CheckCurrentUserPermissions(bioAdminAuditTrail, sptDynamic1, FALSE, FALSE, TRUE)) {
		strPersonName = "AuditDetailsT.PersonName";
	} else {
		strPersonName = "CASE WHEN AuditDetailsT.InternalPatientID IS NULL THEN PersonName ELSE '' END AS PersonName";
	}

	CString strSQL;
	if(strPatientName) {
		// (b.cardillo 2005-11-18 17:29) - PLID 18390 - Fixed a bug where it was referencing 
		// appointment info even when the RecordID pointed to templates.  It now pulls template 
		// info for such audit details.
		// (b.cardillo 2005-11-18 17:41) - PLID 18391 - Fixed a bug where if the appointment 
		// had no purpose, the Item field would be NULL, and therefore adding the appointment 
		// notes to NULL was still NULL.  It now COALESCES so that it says "{ No Purpose }".
		// (a.walling 2010-01-22 16:39) - PLID 37043 - Include patient ID and internal ID
		// (a.walling 2010-01-25 09:38) - PLID 37056 - Blank out the PersonName when a patient record and lacking appropriate permissions
		strSQL.Format("SELECT AuditT.ChangedByUserName AS UserName, AuditT.ChangedAtLocationName, AuditT.ChangedDate AS TDate, (CASE WHEN AuditDetailsT.Priority = 1 THEN 'High' WHEN AuditDetailsT.Priority = 2 THEN 'Medium' WHEN AuditDetailsT.Priority = 3 THEN 'Low' ELSE '' End) AS PriorityDesc, "
			"AuditDetailsT.ID, AuditDetailsT.AuditID, AuditDetailsT.RecordID, %s AS OldValue, %s AS NewValue, AuditDetailsT.Priority, AuditDetailsT.Type, %s, AuditDetailsT.ItemID, "
			"AuditDetailsT.Priority AS Priority, AuditDetailsT.ID AS AuditDetailID, "
					 "       AuditDetailsT.InternalPatientID, "
					 "       COALESCE(PatientsT.UserDefinedID, PersonT.ArchivedUserDefinedID) AS PatientID, "	
					 "       (CONVERT(nvarchar(3), AuditT.IPOctet1) + '.' + "
					 "		  CONVERT(nvarchar(3), AuditT.IPOctet2) + '.' + "
					 "		  CONVERT(nvarchar(3), AuditT.IPOctet3) + '.' + "
					 "		  CONVERT(nvarchar(3), AuditT.IPOctet4)) AS IPAddress, "
				"(CASE "
				" WHEN AuditDetailsT.ItemID = 3013 THEN 'UpdatePastPendingAppts' "
				" WHEN AuditDetailsT.ItemID IN (%li, %li) THEN COALESCE((SELECT A.Name FROM TemplateT A WHERE A.ID = AuditDetailsT.RecordID), '[Item no longer exists]') "
				" ELSE (CASE WHEN AppointmentsT.ID IS NULL THEN 'Deleted Item' ELSE COALESCE(dbo.GetPurposeString(AppointmentsT.ID), '{ No Purpose }') + (CASE WHEN AppointmentsT.Notes <> '' THEN ' - ' + AppointmentsT.Notes ELSE '' END) END) END) AS Item, "
				"AuditDetailsT.OldValue AS ItemType "
				"FROM AuditT INNER JOIN AuditDetailsT ON AuditT.ID = AuditDetailsT.AuditID "
				"LEFT JOIN AppointmentsT ON AuditDetailsT.RecordID = AppointmentsT.ID "
				"LEFT JOIN PersonT ON AuditDetailsT.InternalPatientID = PersonT.ID "
				"LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
				"WHERE (AuditDetailsT.ItemID >= 3001 AND AuditDetailsT.ItemID <= 4000 "
				"AND AuditDetailsT.PersonName = '%s')"
				, strOld, strNew, strPersonName, aeiTemplateCreate, aeiTemplateDelete, _Q(strPatientName));
	}
	else {
		// (b.cardillo 2005-11-18 17:29) - PLID 18390 - Fixed a bug where it was referencing 
		// appointment info even when the RecordID pointed to templates.  It now pulls template 
		// info for such audit details.
		// (b.cardillo 2005-11-18 17:41) - PLID 18391 - Fixed a bug where if the appointment 
		// had no purpose, the Item field would be NULL, and therefore adding the appointment 
		// notes to NULL was still NULL.  It now COALESCES so that it says "{ No Purpose }".
		
		// (a.walling 2010-01-22 16:39) - PLID 37043 - Include patient ID and internal ID
		// (a.walling 2010-01-25 09:38) - PLID 37056 - Blank out the PersonName when a patient record and lacking appropriate permissions
		strSQL.Format("SELECT AuditT.ChangedByUserName AS UserName, AuditT.ChangedAtLocationName, AuditT.ChangedDate AS TDate, (CASE WHEN AuditDetailsT.Priority = 1 THEN 'High' WHEN AuditDetailsT.Priority = 2 THEN 'Medium' WHEN AuditDetailsT.Priority = 3 THEN 'Low' ELSE '' End) AS PriorityDesc, "
			"AuditDetailsT.ID, AuditDetailsT.AuditID, AuditDetailsT.RecordID, %s AS OldValue, %s AS NewValue, AuditDetailsT.Priority, AuditDetailsT.Type, %s, AuditDetailsT.ItemID, "
			"AuditDetailsT.Priority AS Priority, AuditDetailsT.ID AS AuditDetailID, "
					 "       AuditDetailsT.InternalPatientID, "
					 "       COALESCE(PatientsT.UserDefinedID, PersonT.ArchivedUserDefinedID) AS PatientID, "	
					 "       (CONVERT(nvarchar(3), AuditT.IPOctet1) + '.' + "
					 "		  CONVERT(nvarchar(3), AuditT.IPOctet2) + '.' + "
					 "		  CONVERT(nvarchar(3), AuditT.IPOctet3) + '.' + "
					 "		  CONVERT(nvarchar(3), AuditT.IPOctet4)) AS IPAddress, "
				"(CASE "
				" WHEN AuditDetailsT.ItemID = 3013 THEN 'UpdatePastPendingAppts' "
				" WHEN AuditDetailsT.ItemID IN (%li, %li) THEN COALESCE((SELECT A.Name FROM TemplateT A WHERE A.ID = AuditDetailsT.RecordID), '[Item no longer exists]') "
				" ELSE (CASE WHEN AppointmentsT.ID IS NULL THEN 'Deleted Item' ELSE COALESCE(dbo.GetPurposeString(AppointmentsT.ID), '{ No Purpose }') + (CASE WHEN AppointmentsT.Notes <> '' THEN ' - ' + AppointmentsT.Notes ELSE '' END) END) END) AS Item, "
				"AuditDetailsT.OldValue AS ItemType "
				"FROM AuditT INNER JOIN AuditDetailsT ON AuditT.ID = AuditDetailsT.AuditID "
				"LEFT JOIN AppointmentsT ON AuditDetailsT.RecordID = AppointmentsT.ID "
				"LEFT JOIN PersonT ON AuditDetailsT.InternalPatientID = PersonT.ID "
				"LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
				"WHERE (AuditDetailsT.ItemID >= 3001 AND AuditDetailsT.ItemID <= 4000)"
				, strOld, strNew, strPersonName, aeiTemplateCreate, aeiTemplateDelete);
	}

	return strSQL;
}

CString GetAuditTrailInventoryReportSql() {

	// (z.manning 2009-05-22 17:25) - PLID 34330 - Honor the permission to view old and new value
	CString strOld, strNew;
	if(CheckCurrentUserPermissions(bioAdminAuditTrail, sptDynamic0, FALSE, FALSE, TRUE)) {
		strOld = "AuditDetailsT.OldValue";
		strNew = "AuditDetailsT.NewValue";
	}
	else {
		strOld = strNew = "''";
	}

	// (a.walling 2010-01-25 09:42) - PLID 37056 - Honor the permission to view the patient name
	CString strPersonName;
	if(CheckCurrentUserPermissions(bioAdminAuditTrail, sptDynamic1, FALSE, FALSE, TRUE)) {
		strPersonName = "AuditDetailsT.PersonName";
	} else {
		strPersonName = "CASE WHEN AuditDetailsT.InternalPatientID IS NULL THEN PersonName ELSE '' END AS PersonName";
	}

	CString strSQL;
	//(e.lally 2009-02-16) PLID 33078 - Added the inclusion of the ItemIDs for adding and removing allocation products to a bill (they will also display
	//	under the financial audit). I am using the enum names for searchability and clarity.	
	// (a.walling 2010-01-22 16:39) - PLID 37043 - Include patient ID and internal ID
	// (a.walling 2010-01-25 09:38) - PLID 37056 - Blank out the PersonName when a patient record and lacking appropriate permissions
	strSQL.Format("SELECT AuditT.ChangedByUserName AS UserName, AuditT.ChangedAtLocationName, AuditT.ChangedDate AS TDate, (CASE WHEN AuditDetailsT.Priority = 1 THEN 'High' WHEN AuditDetailsT.Priority = 2 THEN 'Medium' WHEN AuditDetailsT.Priority = 3 THEN 'Low' ELSE '' End) AS PriorityDesc, "
		"AuditDetailsT.ID, AuditDetailsT.AuditID, AuditDetailsT.RecordID, %s AS OldValue, %s AS NewValue, AuditDetailsT.Priority, AuditDetailsT.Type, %s, AuditDetailsT.ItemID, "
		"AuditDetailsT.Priority AS Priority, AuditDetailsT.ID AS AuditDetailID, "
					 "       AuditDetailsT.InternalPatientID, "
					 "       COALESCE(PatientsT.UserDefinedID, PersonT.ArchivedUserDefinedID) AS PatientID, "	
					 "       (CONVERT(nvarchar(3), AuditT.IPOctet1) + '.' + "
					 "		  CONVERT(nvarchar(3), AuditT.IPOctet2) + '.' + "
					 "		  CONVERT(nvarchar(3), AuditT.IPOctet3) + '.' + "
					 "		  CONVERT(nvarchar(3), AuditT.IPOctet4)) AS IPAddress, "
				"(CASE WHEN ((AuditDetailsT.ItemID >= 4001 AND AuditDetailsT.ItemID <= 4014) AND ServiceT.Name Is Not Null) THEN 'Product - ' + ServiceT.Name "
				"WHEN ((AuditDetailsT.ItemID = 4015) AND OrderT.Description Is Not Null) THEN 'Order - ' + OrderT.Description "
				"ELSE '' END) AS Item, "
				"AuditDetailsT.OldValue AS ItemType "
				"FROM AuditT INNER JOIN AuditDetailsT ON AuditT.ID = AuditDetailsT.AuditID "
				"LEFT JOIN ServiceT ON AuditDetailsT.RecordID = ServiceT.ID "
				"LEFT JOIN OrderT ON AuditDetailsT.RecordID = OrderT.ID "
				"LEFT JOIN PersonT ON AuditDetailsT.InternalPatientID = PersonT.ID "
				"LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
				"WHERE ((AuditDetailsT.ItemID >= 4001 AND AuditDetailsT.ItemID <= 5000) OR AuditDetailsT.ItemID IN(%li, %li)) "
				, strOld, strNew, strPersonName, aeiAddChargedAllocationDetail, aeiDeleteChargedAllocationDetail);

	return strSQL;
}

CString GetAuditTrailPatientsReportSql() {

	// (z.manning 2009-05-22 17:25) - PLID 34330 - Honor the permission to view old and new value
	CString strOld, strNew;
	if(CheckCurrentUserPermissions(bioAdminAuditTrail, sptDynamic0, FALSE, FALSE, TRUE)) {
		strOld = "AuditDetailsT.OldValue";
		strNew = "AuditDetailsT.NewValue";
	}
	else {
		strOld = strNew = "''";
	}

	// (a.walling 2010-01-25 09:42) - PLID 37056 - Honor the permission to view the patient name
	CString strPersonName;
	if(CheckCurrentUserPermissions(bioAdminAuditTrail, sptDynamic1, FALSE, FALSE, TRUE)) {
		strPersonName = "AuditDetailsT.PersonName";
	} else {
		strPersonName = "CASE WHEN AuditDetailsT.InternalPatientID IS NULL THEN PersonName ELSE '' END AS PersonName";
	}

	
	// (a.walling 2010-01-22 16:39) - PLID 37043 - Include patient ID and internal ID
	// (a.walling 2010-01-25 09:38) - PLID 37056 - Blank out the PersonName when a patient record and lacking appropriate permissions
	CString strSQL = FormatString("SELECT AuditT.ChangedByUserName AS UserName, AuditT.ChangedAtLocationName, AuditT.ChangedDate AS TDate, (CASE WHEN AuditDetailsT.Priority = 1 THEN 'High' WHEN AuditDetailsT.Priority = 2 THEN 'Medium' WHEN AuditDetailsT.Priority = 3 THEN 'Low' ELSE '' End) AS PriorityDesc, "
		"AuditDetailsT.ID, AuditDetailsT.AuditID, AuditDetailsT.RecordID, %s AS OldValue, %s AS NewValue, AuditDetailsT.Priority, AuditDetailsT.Type, %s, AuditDetailsT.ItemID, "
		"AuditDetailsT.Priority AS Priority, AuditDetailsT.ID AS AuditDetailID, "
					 "       AuditDetailsT.InternalPatientID, "
					 "       COALESCE(PatientsT.UserDefinedID, PersonT.ArchivedUserDefinedID) AS PatientID, "	
					 "       (CONVERT(nvarchar(3), AuditT.IPOctet1) + '.' + "
					 "		  CONVERT(nvarchar(3), AuditT.IPOctet2) + '.' + "
					 "		  CONVERT(nvarchar(3), AuditT.IPOctet3) + '.' + "
					 "		  CONVERT(nvarchar(3), AuditT.IPOctet4)) AS IPAddress, "
				"'' AS Item, "
				"AuditDetailsT.OldValue AS ItemType "
				"FROM AuditT INNER JOIN AuditDetailsT ON AuditT.ID = AuditDetailsT.AuditID "
				"LEFT JOIN PersonT ON AuditDetailsT.InternalPatientID = PersonT.ID "
				"LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
				"WHERE (AuditDetailsT.ItemID >= 1 AND AuditDetailsT.ItemID <= 1000)"
				, strOld, strNew, strPersonName);

	return strSQL;
}

CString GetAuditTrailContactsReportSql() {

	// (z.manning 2009-05-22 17:25) - PLID 34330 - Honor the permission to view old and new value
	CString strOld, strNew;
	if(CheckCurrentUserPermissions(bioAdminAuditTrail, sptDynamic0, FALSE, FALSE, TRUE)) {
		strOld = "AuditDetailsT.OldValue";
		strNew = "AuditDetailsT.NewValue";
	}
	else {
		strOld = strNew = "''";
	}

	// (a.walling 2010-01-25 09:42) - PLID 37056 - Honor the permission to view the patient name
	CString strPersonName;
	if(CheckCurrentUserPermissions(bioAdminAuditTrail, sptDynamic1, FALSE, FALSE, TRUE)) {
		strPersonName = "AuditDetailsT.PersonName";
	} else {
		strPersonName = "CASE WHEN AuditDetailsT.InternalPatientID IS NULL THEN PersonName ELSE '' END AS PersonName";
	}
	
	// (a.walling 2010-01-22 16:39) - PLID 37043 - Include patient ID and internal ID
	// (a.walling 2010-01-25 09:38) - PLID 37056 - Blank out the PersonName when a patient record and lacking appropriate permissions
	CString strSQL = FormatString("SELECT AuditT.ChangedByUserName AS UserName, AuditT.ChangedAtLocationName, AuditT.ChangedDate AS TDate, (CASE WHEN AuditDetailsT.Priority = 1 THEN 'High' WHEN AuditDetailsT.Priority = 2 THEN 'Medium' WHEN AuditDetailsT.Priority = 3 THEN 'Low' ELSE '' End) AS PriorityDesc, "
		"AuditDetailsT.ID, AuditDetailsT.AuditID, AuditDetailsT.RecordID, %s AS OldValue, %s AS NewValue, AuditDetailsT.Priority, AuditDetailsT.Type, %s, AuditDetailsT.ItemID, "
		"AuditDetailsT.Priority AS Priority, AuditDetailsT.ID AS AuditDetailID, "
					 "       AuditDetailsT.InternalPatientID, "
					 "       COALESCE(PatientsT.UserDefinedID, PersonT.ArchivedUserDefinedID) AS PatientID, "	
					 "       (CONVERT(nvarchar(3), AuditT.IPOctet1) + '.' + "
					 "		  CONVERT(nvarchar(3), AuditT.IPOctet2) + '.' + "
					 "		  CONVERT(nvarchar(3), AuditT.IPOctet3) + '.' + "
					 "		  CONVERT(nvarchar(3), AuditT.IPOctet4)) AS IPAddress, "
				"'' AS Item, "
				"AuditDetailsT.OldValue AS ItemType "
				"FROM AuditT INNER JOIN AuditDetailsT ON AuditT.ID = AuditDetailsT.AuditID "
				"LEFT JOIN PersonT ON AuditDetailsT.InternalPatientID = PersonT.ID "
				"LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
				"WHERE (AuditDetailsT.ItemID >= 1001 AND AuditDetailsT.ItemID <= 2000)"
				, strOld, strNew, strPersonName);

	return strSQL;
}

CString GetAuditTrailInsuranceReportSql() {

	// (z.manning 2009-05-22 17:25) - PLID 34330 - Honor the permission to view old and new value
	CString strOld, strNew;
	if(CheckCurrentUserPermissions(bioAdminAuditTrail, sptDynamic0, FALSE, FALSE, TRUE)) {
		strOld = "AuditDetailsT.OldValue";
		strNew = "AuditDetailsT.NewValue";
	}
	else {
		strOld = strNew = "''";
	}

	// (a.walling 2010-01-25 09:42) - PLID 37056 - Honor the permission to view the patient name
	CString strPersonName;
	if(CheckCurrentUserPermissions(bioAdminAuditTrail, sptDynamic1, FALSE, FALSE, TRUE)) {
		strPersonName = "AuditDetailsT.PersonName";
	} else {
		strPersonName = "CASE WHEN AuditDetailsT.InternalPatientID IS NULL THEN PersonName ELSE '' END AS PersonName";
	}
	
	// (a.walling 2010-01-22 16:39) - PLID 37043 - Include patient ID and internal ID
	// (a.walling 2010-01-25 09:38) - PLID 37056 - Blank out the PersonName when a patient record and lacking appropriate permissions
	CString strSQL = FormatString("SELECT AuditT.ChangedByUserName AS UserName, AuditT.ChangedAtLocationName, AuditT.ChangedDate AS TDate, (CASE WHEN AuditDetailsT.Priority = 1 THEN 'High' WHEN AuditDetailsT.Priority = 2 THEN 'Medium' WHEN AuditDetailsT.Priority = 3 THEN 'Low' ELSE '' End) AS PriorityDesc, "
		"AuditDetailsT.ID, AuditDetailsT.AuditID, AuditDetailsT.RecordID, %s AS OldValue, %s AS NewValue, AuditDetailsT.Priority, AuditDetailsT.Type, %s, AuditDetailsT.ItemID, "
		"AuditDetailsT.Priority AS Priority, AuditDetailsT.ID AS AuditDetailID, "
					 "       AuditDetailsT.InternalPatientID, "
					 "       COALESCE(PatientsT.UserDefinedID, PersonT.ArchivedUserDefinedID) AS PatientID, "	
					 "       (CONVERT(nvarchar(3), AuditT.IPOctet1) + '.' + "
					 "		  CONVERT(nvarchar(3), AuditT.IPOctet2) + '.' + "
					 "		  CONVERT(nvarchar(3), AuditT.IPOctet3) + '.' + "
					 "		  CONVERT(nvarchar(3), AuditT.IPOctet4)) AS IPAddress, "
				"'' AS Item, "
				"AuditDetailsT.OldValue AS ItemType "
				"FROM AuditT INNER JOIN AuditDetailsT ON AuditT.ID = AuditDetailsT.AuditID "
				"LEFT JOIN PersonT ON AuditDetailsT.InternalPatientID = PersonT.ID "
				"LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
				"WHERE (AuditDetailsT.ItemID >= 5001 AND AuditDetailsT.ItemID <= 6000)"
				, strOld, strNew, strPersonName);

	return strSQL;
}

CString GetAuditTrailPalmReportSql() {

	// (z.manning 2009-05-22 17:25) - PLID 34330 - Honor the permission to view old and new value
	CString strOld, strNew;
	if(CheckCurrentUserPermissions(bioAdminAuditTrail, sptDynamic0, FALSE, FALSE, TRUE)) {
		strOld = "AuditDetailsT.OldValue";
		strNew = "AuditDetailsT.NewValue";
	}
	else {
		strOld = strNew = "''";
	}

	// (a.walling 2010-01-25 09:42) - PLID 37056 - Honor the permission to view the patient name
	CString strPersonName;
	if(CheckCurrentUserPermissions(bioAdminAuditTrail, sptDynamic1, FALSE, FALSE, TRUE)) {
		strPersonName = "AuditDetailsT.PersonName";
	} else {
		strPersonName = "CASE WHEN AuditDetailsT.InternalPatientID IS NULL THEN PersonName ELSE '' END AS PersonName";
	}

	// (a.walling 2010-01-22 16:39) - PLID 37043 - Include patient ID and internal ID
	// (a.walling 2010-01-25 09:38) - PLID 37056 - Blank out the PersonName when a patient record and lacking appropriate permissions
	CString strSQL = FormatString("SELECT AuditT.ChangedByUserName AS UserName, AuditT.ChangedAtLocationName, AuditT.ChangedDate AS TDate, (CASE WHEN AuditDetailsT.Priority = 1 THEN 'High' WHEN AuditDetailsT.Priority = 2 THEN 'Medium' WHEN AuditDetailsT.Priority = 3 THEN 'Low' ELSE '' End) AS PriorityDesc, "
		"AuditDetailsT.ID, AuditDetailsT.AuditID, AuditDetailsT.RecordID, %s AS OldValue, %s AS NewValue, AuditDetailsT.Priority, AuditDetailsT.Type, %s, AuditDetailsT.ItemID, "
		"AuditDetailsT.Priority AS Priority, AuditDetailsT.ID AS AuditDetailID, "
					 "       AuditDetailsT.InternalPatientID, "
					 "       COALESCE(PatientsT.UserDefinedID, PersonT.ArchivedUserDefinedID) AS PatientID, "	
					 "       (CONVERT(nvarchar(3), AuditT.IPOctet1) + '.' + "
					 "		  CONVERT(nvarchar(3), AuditT.IPOctet2) + '.' + "
					 "		  CONVERT(nvarchar(3), AuditT.IPOctet3) + '.' + "
					 "		  CONVERT(nvarchar(3), AuditT.IPOctet4)) AS IPAddress, "
				"'' AS Item, "
				"AuditDetailsT.OldValue AS ItemType "
				"FROM AuditT INNER JOIN AuditDetailsT ON AuditT.ID = AuditDetailsT.AuditID "
				"LEFT JOIN PersonT ON AuditDetailsT.InternalPatientID = PersonT.ID "
				"LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
				"WHERE (AuditDetailsT.ItemID >= 6001 AND AuditDetailsT.ItemID <= 7000)"
				, strOld, strNew, strPersonName);

	return strSQL;
}

CString GetAuditTrailEMRReportSql() {

	// (z.manning 2009-05-22 17:25) - PLID 34330 - Honor the permission to view old and new value
	CString strOld, strNew;
	if(CheckCurrentUserPermissions(bioAdminAuditTrail, sptDynamic0, FALSE, FALSE, TRUE)) {
		strOld = "AuditDetailsT.OldValue";
		strNew = "AuditDetailsT.NewValue";
	}
	else {
		strOld = strNew = "''";
	}

	// (a.walling 2010-01-25 09:42) - PLID 37056 - Honor the permission to view the patient name
	CString strPersonName;
	if(CheckCurrentUserPermissions(bioAdminAuditTrail, sptDynamic1, FALSE, FALSE, TRUE)) {
		strPersonName = "AuditDetailsT.PersonName";
	} else {
		strPersonName = "CASE WHEN AuditDetailsT.InternalPatientID IS NULL THEN PersonName ELSE '' END AS PersonName";
	}

	// (a.walling 2010-01-22 16:39) - PLID 37043 - Include patient ID and internal ID
	// (a.walling 2010-01-25 09:38) - PLID 37056 - Blank out the PersonName when a patient record and lacking appropriate permissions
	CString strSQL = FormatString("SELECT AuditT.ChangedByUserName AS UserName, AuditT.ChangedAtLocationName, AuditT.ChangedDate AS TDate, (CASE WHEN AuditDetailsT.Priority = 1 THEN 'High' WHEN AuditDetailsT.Priority = 2 THEN 'Medium' WHEN AuditDetailsT.Priority = 3 THEN 'Low' ELSE '' End) AS PriorityDesc, "
		"AuditDetailsT.ID, AuditDetailsT.AuditID, AuditDetailsT.RecordID, %s AS OldValue, %s AS NewValue, AuditDetailsT.Priority, AuditDetailsT.Type, %s, AuditDetailsT.ItemID, "
		"AuditDetailsT.Priority AS Priority, AuditDetailsT.ID AS AuditDetailID, "
					 "       AuditDetailsT.InternalPatientID, "
					 "       COALESCE(PatientsT.UserDefinedID, PersonT.ArchivedUserDefinedID) AS PatientID, "	
					 "       (CONVERT(nvarchar(3), AuditT.IPOctet1) + '.' + "
					 "		  CONVERT(nvarchar(3), AuditT.IPOctet2) + '.' + "
					 "		  CONVERT(nvarchar(3), AuditT.IPOctet3) + '.' + "
					 "		  CONVERT(nvarchar(3), AuditT.IPOctet4)) AS IPAddress, "
				"'' AS Item, "
				"AuditDetailsT.OldValue AS ItemType "
				"FROM AuditT INNER JOIN AuditDetailsT ON AuditT.ID = AuditDetailsT.AuditID "
				"LEFT JOIN PersonT ON AuditDetailsT.InternalPatientID = PersonT.ID "
				"LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
				"WHERE (AuditDetailsT.ItemID >= 7001 AND AuditDetailsT.ItemID <= 8000)"
				, strOld, strNew, strPersonName);

	return strSQL;
}

CString GetAuditTrailMiscReportSql() {

	// (z.manning 2009-05-22 17:25) - PLID 34330 - Honor the permission to view old and new value
	CString strOld, strNew;
	if(CheckCurrentUserPermissions(bioAdminAuditTrail, sptDynamic0, FALSE, FALSE, TRUE)) {
		strOld = "AuditDetailsT.OldValue";
		strNew = "AuditDetailsT.NewValue";
	}
	else {
		strOld = strNew = "''";
	}

	// (a.walling 2010-01-25 09:42) - PLID 37056 - Honor the permission to view the patient name
	CString strPersonName;
	if(CheckCurrentUserPermissions(bioAdminAuditTrail, sptDynamic1, FALSE, FALSE, TRUE)) {
		strPersonName = "AuditDetailsT.PersonName";
	} else {
		strPersonName = "CASE WHEN AuditDetailsT.InternalPatientID IS NULL THEN PersonName ELSE '' END AS PersonName";
	}

	// (a.walling 2010-01-22 16:39) - PLID 37043 - Include patient ID and internal ID
	// (a.walling 2010-01-25 09:38) - PLID 37056 - Blank out the PersonName when a patient record and lacking appropriate permissions
	CString strSQL = FormatString("SELECT AuditT.ChangedByUserName AS UserName, AuditT.ChangedAtLocationName, AuditT.ChangedDate AS TDate, (CASE WHEN AuditDetailsT.Priority = 1 THEN 'High' WHEN AuditDetailsT.Priority = 2 THEN 'Medium' WHEN AuditDetailsT.Priority = 3 THEN 'Low' ELSE '' End) AS PriorityDesc, "
		"AuditDetailsT.ID, AuditDetailsT.AuditID, AuditDetailsT.RecordID, %s AS OldValue, %s AS NewValue, AuditDetailsT.Priority, AuditDetailsT.Type, %s, AuditDetailsT.ItemID, "
		"AuditDetailsT.Priority AS Priority, AuditDetailsT.ID AS AuditDetailID, "
					 "       AuditDetailsT.InternalPatientID, "
					 "       COALESCE(PatientsT.UserDefinedID, PersonT.ArchivedUserDefinedID) AS PatientID, "	
					 "       (CONVERT(nvarchar(3), AuditT.IPOctet1) + '.' + "
					 "		  CONVERT(nvarchar(3), AuditT.IPOctet2) + '.' + "
					 "		  CONVERT(nvarchar(3), AuditT.IPOctet3) + '.' + "
					 "		  CONVERT(nvarchar(3), AuditT.IPOctet4)) AS IPAddress, "
				"'' AS Item, "
				"AuditDetailsT.OldValue AS ItemType "
				"FROM AuditT INNER JOIN AuditDetailsT ON AuditT.ID = AuditDetailsT.AuditID "
				"LEFT JOIN PersonT ON AuditDetailsT.InternalPatientID = PersonT.ID "
				"LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
				"WHERE (AuditDetailsT.ItemID >= 9001)"
				, strOld, strNew, strPersonName);

	return strSQL;
}

//TES 7/11/2008 - PLID 30668 - New report, filters on just the financial audits that could throw off the AR
CString GetAuditTrailARIssuesReportSql(const COleDateTime &dtFrom)
{
	// (z.manning 2009-05-22 17:25) - PLID 34330 - Honor the permission to view old and new value
	CString strOld, strNew;
	if(CheckCurrentUserPermissions(bioAdminAuditTrail, sptDynamic0, FALSE, FALSE, TRUE)) {
		strOld = "AuditDetailsT.OldValue";
		strNew = "AuditDetailsT.NewValue";
	}
	else {
		strOld = strNew = "''";
	}

	// (a.walling 2010-01-25 09:42) - PLID 37056 - Honor the permission to view the patient name
	CString strPersonName;
	if(CheckCurrentUserPermissions(bioAdminAuditTrail, sptDynamic1, FALSE, FALSE, TRUE)) {
		strPersonName = "AuditTrailARIssuesSubQ.PersonName";
	} else {
		strPersonName = "CASE WHEN AuditTrailARIssuesSubQ.InternalPatientID IS NULL THEN AuditTrailARIssuesSubQ.PersonName ELSE '' END AS PersonName";
	}

	CString strSql;
	//TES 7/11/2008 - PLID 30668 - All the basic line item audits.
    // v.arth 05/19/09 - PLID 28569 - Added AuditT.IPAddress

	// (a.walling 2010-01-22 16:39) - PLID 37043 - Include patient ID and internal ID
	strSql += FormatString("SELECT AuditT.ChangedByUserName AS UserName, AuditT.ChangedAtLocationName, AuditT.ChangedDate AS "
		"TDate, (CASE WHEN AuditDetailsT.Priority = 1 THEN 'High' WHEN AuditDetailsT.Priority = 2 THEN 'Medium' WHEN "
		"AuditDetailsT.Priority = 3 THEN 'Low' ELSE '' End) AS PriorityDesc, "
		"AuditDetailsT.ID, AuditDetailsT.AuditID, AuditDetailsT.RecordID, %s AS OldValue, %s AS NewValue, AuditDetailsT.Priority, AuditDetailsT.Type, AuditDetailsT.PersonName, AuditDetailsT.ItemID, "
		"AuditDetailsT.ID AS AuditDetailID, "
					 "       AuditDetailsT.InternalPatientID, "
					 "       COALESCE(PatientsT.UserDefinedID, PersonT.ArchivedUserDefinedID) AS PatientID, "	
					 "       (CONVERT(nvarchar(3), AuditT.IPOctet1) + '.' + "
					 "		  CONVERT(nvarchar(3), AuditT.IPOctet2) + '.' + "
					 "		  CONVERT(nvarchar(3), AuditT.IPOctet3) + '.' + "
					 "		  CONVERT(nvarchar(3), AuditT.IPOctet4)) AS IPAddress, CASE WHEN LineItemT.Type = 10 THEN 'Charge - ' WHEN LineItemT.Type = 1 THEN 'Payment - ' WHEN "
		"LineItemT.Type = 2 THEN 'Adjustment - ' WHEN LineItemT.Type = 3 THEN 'Refund - ' END + "
		"LineItemT.Description AS Item, LineItemT.InputDate AS InputDate1, NULL AS InputDate2  "
		""
		"FROM AuditT INNER JOIN AuditDetailsT ON AuditT.ID = AuditDetailsT.AuditID LEFT JOIN LineItemT ON "
		"AuditDetailsT.RecordID = LineItemT.ID "
		"LEFT JOIN PersonT ON AuditDetailsT.InternalPatientID = PersonT.ID "
		"LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		"WHERE AuditDetailsT.ItemID IN (%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i)", 
		strOld, strNew, aeiChargeModifier, aeiChargeModifier2, aeiChargeQty, aeiChargeTax, aeiChargeTax2, aeiChargePercentOff, 
		aeiChargeDiscount, aeiChargeLineDeleted, aeiChargeLineAmount, aeiChargeInputDate, aeiChargeModifier3, 
		aeiChargeModifier4, aeiPaymentDate, aeiPaymentDeleted, aeiPaymentAmount, aeiAdjustmentDate, aeiAdjustmentDeleted, 
		aeiAdjustmentAmount, aeiRefundDate, aeiRefundDeleted, aeiRefundAmount, aeiPaymentType, aeiChargeProvider, 
		aeiPaymentProvider, aeiAdjustmentProvider, aeiRefundProvider, aeiPaymentLocation, aeiAdjustmentLocation, 
		aeiRefundLocation);

	//TES 7/11/2008 - PLID 30668 - All the basic bill audits (there's only one).
    // v.arth 05/19/09 - PLID 28569 - Added AuditT.IPAddress
	// (a.walling 2010-01-22 16:39) - PLID 37043 - Include patient ID and internal ID
	strSql += FormatString(" UNION ALL SELECT AuditT.ChangedByUserName AS UserName, AuditT.ChangedAtLocationName, AuditT.ChangedDate "
		"AS TDate, (CASE WHEN AuditDetailsT.Priority = 1 THEN 'High' WHEN AuditDetailsT.Priority = 2 THEN 'Medium' WHEN "
		"AuditDetailsT.Priority = 3 THEN 'Low' ELSE '' End) AS PriorityDesc, "
		"AuditDetailsT.ID, AuditDetailsT.AuditID, AuditDetailsT.RecordID, %s AS OldValue, %s AS NewValue, AuditDetailsT.Priority, AuditDetailsT.Type, AuditDetailsT.PersonName, AuditDetailsT.ItemID, "
		"AuditDetailsT.ID AS AuditDetailID, "
					 "       AuditDetailsT.InternalPatientID, "
					 "       COALESCE(PatientsT.UserDefinedID, PersonT.ArchivedUserDefinedID) AS PatientID, "	
					 "       (CONVERT(nvarchar(3), AuditT.IPOctet1) + '.' + "
					 "		  CONVERT(nvarchar(3), AuditT.IPOctet2) + '.' + "
					 "		  CONVERT(nvarchar(3), AuditT.IPOctet3) + '.' + "
					 "		  CONVERT(nvarchar(3), AuditT.IPOctet4)) AS IPAddress, 'Bill - ' + BillsT.Description AS Item, BillsT.InputDate AS InputDate1, NULL AS InputDate2 "
		"FROM AuditT INNER JOIN AuditDetailsT ON AuditT.ID = AuditDetailsT.AuditID LEFT JOIN BillsT ON AuditDetailsT.RecordID "
		"= BillsT.ID "
		"LEFT JOIN PersonT ON AuditDetailsT.InternalPatientID = PersonT.ID "
		"LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		"WHERE AuditDetailsT.ItemID IN (%i)", strOld, strNew, aeiBillDeleted);

	//TES 7/11/2008 - PLID 30668 - All the basic apply audits (there's only one, and it's obsolete).
    // v.arth 05/19/09 - PLID 28569 - Added AuditT.IPAddress
	// (a.walling 2010-01-22 16:39) - PLID 37043 - Include patient ID and internal ID
	strSql += FormatString(" UNION ALL SELECT AuditT.ChangedByUserName AS UserName, AuditT.ChangedAtLocationName, AuditT.ChangedDate "
		"AS TDate, (CASE WHEN AuditDetailsT.Priority = 1 THEN 'High' WHEN AuditDetailsT.Priority = 2 THEN 'Medium' WHEN "
		"AuditDetailsT.Priority = 3 THEN 'Low' ELSE '' End) AS PriorityDesc, "
		"AuditDetailsT.ID, AuditDetailsT.AuditID, AuditDetailsT.RecordID, %s AS OldValue, %s AS NewValue, AuditDetailsT.Priority, AuditDetailsT.Type, AuditDetailsT.PersonName, AuditDetailsT.ItemID, "
		"AuditDetailsT.ID AS AuditDetailID, "
					 "       AuditDetailsT.InternalPatientID, "
					 "       COALESCE(PatientsT.UserDefinedID, PersonT.ArchivedUserDefinedID) AS PatientID, "	
					 "       (CONVERT(nvarchar(3), AuditT.IPOctet1) + '.' + "
					 "		  CONVERT(nvarchar(3), AuditT.IPOctet2) + '.' + "
					 "		  CONVERT(nvarchar(3), AuditT.IPOctet3) + '.' + "
					 "		  CONVERT(nvarchar(3), AuditT.IPOctet4)) AS IPAddress, '' AS Item, NULL AS InputDate1, NULL AS InputDate2 "
		"FROM AuditT INNER JOIN AuditDetailsT ON AuditT.ID = AuditDetailsT.AuditID "
		"LEFT JOIN PersonT ON AuditDetailsT.InternalPatientID = PersonT.ID "
		"LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		"WHERE AuditDetailsT.ItemID IN (%i)", 
		strOld, strNew, aeiApplyDeleted_OBSOLETE);

	//TES 7/11/2008 - PLID 30668 - Special case for the bill location, which links via the charge ID, but actually "is" a bill.
    // v.arth 05/19/09 - PLID 28569 - Added AuditT.IPAddress
	// (a.walling 2010-01-22 16:39) - PLID 37043 - Include patient ID and internal ID
	strSql += FormatString(" UNION ALL SELECT AuditT.ChangedByUserName AS UserName, AuditT.ChangedAtLocationName, "
		"AuditT.ChangedDate AS TDate, (CASE WHEN AuditDetailsT.Priority = 1 THEN 'High' WHEN AuditDetailsT.Priority = 2 "
		"THEN 'Medium' WHEN AuditDetailsT.Priority = 3 THEN 'Low' ELSE '' End) AS PriorityDesc, "
		"AuditDetailsT.ID, AuditDetailsT.AuditID, AuditDetailsT.RecordID, %s AS OldValue, %s AS NewValue, AuditDetailsT.Priority, AuditDetailsT.Type, AuditDetailsT.PersonName, AuditDetailsT.ItemID, "
		"AuditDetailsT.ID AS AuditDetailID, "
					 "       AuditDetailsT.InternalPatientID, "
					 "       COALESCE(PatientsT.UserDefinedID, PersonT.ArchivedUserDefinedID) AS PatientID, "	
					 "       (CONVERT(nvarchar(3), AuditT.IPOctet1) + '.' + "
					 "		  CONVERT(nvarchar(3), AuditT.IPOctet2) + '.' + "
					 "		  CONVERT(nvarchar(3), AuditT.IPOctet3) + '.' + "
					 "		  CONVERT(nvarchar(3), AuditT.IPOctet4)) AS IPAddress, 'Bill - ' + BillsT.Description AS Item, BillsT.InputDate AS InputDate1, "
		"NULL AS InputDate2 "
		"FROM AuditT INNER JOIN AuditDetailsT ON AuditT.ID = AuditDetailsT.AuditID LEFT JOIN ChargesT ON "
		"AuditDetailsT.RecordID = ChargesT.ID LEFT JOIN BillsT ON ChargesT.BillID = BillsT.ID "
		"LEFT JOIN PersonT ON AuditDetailsT.InternalPatientID = PersonT.ID "
		"LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		"WHERE AuditDetailsT.ItemID IN (%i)", 
		strOld, strNew, aeiBillLocation);

	//TES 7/11/2008 - PLID 30668 - Unapplied Item audits (these are a bit more complicated).

	// (a.walling 2010-01-22 16:39) - PLID 37043 - Include patient ID and internal ID
	strSql += FormatString(" UNION ALL SELECT AuditT.ChangedByUserName AS UserName, AuditT.ChangedAtLocationName, "
		"AuditT.ChangedDate AS TDate, (CASE WHEN AuditDetailsT.Priority = 1 THEN 'High' WHEN AuditDetailsT.Priority = 2 THEN "
		"'Medium' WHEN AuditDetailsT.Priority = 3 THEN 'Low' ELSE '' End) AS PriorityDesc, "
		"AuditDetailsT.ID, AuditDetailsT.AuditID, AuditDetailsT.RecordID, %s AS OldValue, %s AS NewValue, AuditDetailsT.Priority, AuditDetailsT.Type, AuditDetailsT.PersonName, AuditDetailsT.ItemID, "
		"AuditDetailsT.ID AS AuditDetailID, "
					 "       AuditDetailsT.InternalPatientID, "
					 "       COALESCE(PatientsT.UserDefinedID, PersonT.ArchivedUserDefinedID) AS PatientID, "	
					 "       (CONVERT(nvarchar(3), AuditT.IPOctet1) + '.' + "
					 "		  CONVERT(nvarchar(3), AuditT.IPOctet2) + '.' + "
					 "		  CONVERT(nvarchar(3), AuditT.IPOctet3) + '.' + "
					 "		  CONVERT(nvarchar(3), AuditT.IPOctet4)) AS IPAddress, 'Source: ' + CASE WHEN LineItemT.Type = 1 THEN 'Payment - ' WHEN LineItemT.Type = "
		"2 THEN 'Adjustment - ' WHEN LineItemT.Type = 3 THEN 'Refund - ' END + LineItemT.Description + 'Dest: ' + CASE WHEN "
		"DestItem.Type = 10 THEN 'Charge - ' WHEN DestItem.Type = 1 THEN 'Payment - ' WHEN DestItem.Type = 2 THEN "
		"'Adjustment - ' WHEN DestItem.Type = 3 THEN 'Refund - ' END + DestItem.Description AS Item, LineItemT.InputDate AS "
		"InputDate1, DestItem.InputDate AS InputDate2 "
		"FROM AuditT INNER JOIN AuditDetailsT ON AuditT.ID = AuditDetailsT.AuditID LEFT JOIN LineItemT ON "
		"AuditDetailsT.RecordID = LineItemT.ID LEFT JOIN (SELECT AuditID, RecordID FROM AuditDetailsT "
		"WHERE AuditDetailsT.ItemID IN (%i,%i,%i,%i)) AS DestAudits ON AuditDetailsT.AuditID = DestAudits.AuditID LEFT JOIN "
		"LineItemT DestItem ON DestAudits.RecordID = DestItem.ID "
		"LEFT JOIN PersonT ON AuditDetailsT.InternalPatientID = PersonT.ID "
		"LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		"WHERE AuditDetailsT.ItemID IN (%i,%i,%i)", 
		strOld, strNew, aeiItemUnappliedFromCharge, aeiItemUnappliedFromPayment, aeiItemUnappliedFromAdjustment, aeiItemUnappliedFromRefund, aeiPaymentUnapplied, 
		aeiAdjustmentUnapplied, aeiRefundUnapplied);


	//TES 7/11/2008 - PLID 30668 - Apply the Input Date preference.
	// (a.walling 2010-01-22 16:39) - PLID 37043 - Include patient ID and internal ID
	// (a.walling 2010-01-25 09:38) - PLID 37056 - Blank out the PersonName when a patient record and lacking appropriate permissions
	CString strFinalSql;
	strFinalSql.Format("SELECT AuditTrailARIssuesSubQ.UserName AS UserName, AuditTrailARIssuesSubQ.ChangedAtLocationName, AuditTrailARIssuesSubQ.TDate AS TDate, AuditTrailARIssuesSubQ.PriorityDesc, AuditTrailARIssuesSubQ.ID, AuditTrailARIssuesSubQ.AuditID, "
		"AuditTrailARIssuesSubQ.RecordID, AuditTrailARIssuesSubQ.OldValue, AuditTrailARIssuesSubQ.NewValue, AuditTrailARIssuesSubQ.Priority AS Priority, AuditTrailARIssuesSubQ.Type, %s, AuditTrailARIssuesSubQ.ItemID, AuditTrailARIssuesSubQ.AuditDetailID, AuditTrailARIssuesSubQ.IPAddress, AuditTrailARIssuesSubQ.Item, "
		"AuditDetailsT.OldValue AS ItemType, AuditTrailARIssuesSubQ.InternalPatientID, AuditTrailARIssuesSubQ.PatientID "
		"FROM (%s) AS AuditTrailARIssuesSubQ INNER JOIN AuditDetailsT ON AuditTrailARIssuesSubQ.ID = AuditDetailsT.ID",
		strPersonName, strSql);
	if(GetRemotePropertyInt("AuditTrail_ARIssues_OnlyPastItems", 1, 0, "<None>", true)) {
		if(dtFrom.GetStatus() == COleDateTime::invalid) {
			//TES 7/14/2008 - PLID 30668 - If they run it for all dates, then there are no past items.  The report will have
			// a note explaining why it's empty.
			strFinalSql += FormatString(" WHERE (0=1)");
		}
		else {
			strFinalSql += FormatString(" WHERE (InputDate1 Is Null OR InputDate1 < '%s') AND "
				"(InputDate2 Is Null OR InputDate2 < '%s')", _Q(FormatDateTimeForSql(dtFrom)), _Q(FormatDateTimeForSql(dtFrom)));
		}
	}

	return strFinalSql;
}

//TES 6/11/2009 - PLID 34607 - New report just on General 1 Demographics
CString GetAuditTrailDemographicReportSql(long nPatientID /*= -1*/) {

	// (z.manning 2009-05-22 17:25) - PLID 34330 - Honor the permission to view old and new value
	CString strOld, strNew;
	if(CheckCurrentUserPermissions(bioAdminAuditTrail, sptDynamic0, FALSE, FALSE, TRUE)) {
		strOld = "AuditDetailsT.OldValue";
		strNew = "AuditDetailsT.NewValue";
	}
	else {
		strOld = strNew = "''";
	}

	// (a.walling 2010-01-25 09:42) - PLID 37056 - Honor the permission to view the patient name
	CString strPersonName;
	if(CheckCurrentUserPermissions(bioAdminAuditTrail, sptDynamic1, FALSE, FALSE, TRUE)) {
		strPersonName = "PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PersonName";
	} else {
		strPersonName = "CASE WHEN AuditDetailsT.InternalPatientID IS NULL THEN PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle ELSE '' END AS PersonName";
	}

	//TES 6/11/2009 - PLID 34607 - Gather all the audit items that are "demographics"
	// (j.jones 2009-10-14 14:35) - PLID 34327 - supported the ethnicity audit
	// (z.manning 2010-01-07 09:24) - PLID 36788 - Added language
	CString strItemFilter;
	strItemFilter.Format("AuditDetailsT.ItemID IN (%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,"
		"%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i)",
		aeiPatientPersonFirst,aeiPatientPersonMiddle,aeiPatientPersonLast,aeiPatientUserID,aeiPatientAddress,
		aeiPatientHPhone,aeiPatientWPhone,aeiPatientAddress2,aeiPatientCity,aeiPatientState,aeiPatientZip,
		aeiPatientBirthDate,aeiPatientSSN,aeiPatientEmail,aeiPatientMaritalStatus,aeiPatientSpouseStatus,
		aeiPatientEmergFirst,aeiPatientEmergLast,aeiPatientEmergHPhone,aeiPatientEmergWPhone,aeiPatientEmergRelation,
		aeiPatientPagerNumber,aeiPatientOtherPhone,aeiPatientMobilePhone,aeiPatientExtension,aeiPatientFaxNumber,
		aeiPatientGender,aeiPatientFirstContact,aeiPatientNickname,aeiPatientTitle,aeiPatientPrivHome,
		aeiPatientPrivWork,aeiPatientPrivCell,aeiPatientPrivPager,aeiPatientPrivOther,aeiPatientPrivFax,
		aeiPatientPrivEmail,aeiPatientRace,aeiPatientEthnicity,aeiPatientPrefContact,aeiPatientExcludeMailing,aeiPatientG1Custom1,
		aeiPatientG1Custom2,aeiPatientG1Custom3,aeiPatientG1Custom4,aeiPatientLanguage);
	CString strSQL;
	if(nPatientID != -1) {
		// (a.walling 2010-01-22 16:39) - PLID 37043 - Include patient ID and internal ID
		// (a.walling 2010-01-25 09:38) - PLID 37056 - Blank out the PersonName when a patient record and lacking appropriate permissions
		strSQL = FormatString("SELECT AuditT.ChangedByUserName AS UserName, AuditT.ChangedAtLocationName, AuditT.ChangedDate AS TDate, (CASE WHEN AuditDetailsT.Priority = 1 THEN 'High' WHEN AuditDetailsT.Priority = 2 THEN 'Medium' WHEN AuditDetailsT.Priority = 3 THEN 'Low' ELSE '' End) AS PriorityDesc, "
			"AuditDetailsT.ID, AuditDetailsT.AuditID, AuditDetailsT.RecordID, %s AS OldValue, %s AS NewValue, AuditDetailsT.Priority, AuditDetailsT.Type, %s, AuditDetailsT.ItemID, "
			"AuditDetailsT.Priority AS Priority, AuditDetailsT.ID AS AuditDetailID, "
						 "       AuditDetailsT.InternalPatientID, "
						 "       COALESCE(PatientsT.UserDefinedID, PatientPersonT.ArchivedUserDefinedID) AS PatientID, "	
						 "       (CONVERT(nvarchar(3), AuditT.IPOctet1) + '.' + "
						 "		  CONVERT(nvarchar(3), AuditT.IPOctet2) + '.' + "
						 "		  CONVERT(nvarchar(3), AuditT.IPOctet3) + '.' + "
						 "		  CONVERT(nvarchar(3), AuditT.IPOctet4)) AS IPAddress, "
					"'' AS Item, "
					"AuditDetailsT.OldValue AS ItemType "
					"FROM AuditT INNER JOIN AuditDetailsT ON AuditT.ID = AuditDetailsT.AuditID "
					"INNER JOIN PersonT ON AuditDetailsT.RecordID = PersonT.ID "
					"LEFT JOIN PersonT PatientPersonT ON AuditDetailsT.InternalPatientID = PatientPersonT.ID "
					"LEFT JOIN PatientsT ON PatientPersonT.ID = PatientsT.PersonID "
					"WHERE (%s) "
					"AND AuditDetailsT.RecordID = %li"
					, strOld, strNew, strPersonName, strItemFilter, nPatientID);
		
	}
	else {		
		// (a.walling 2010-01-22 16:39) - PLID 37043 - Include patient ID and internal ID
		// (a.walling 2010-01-25 09:38) - PLID 37056 - Blank out the PersonName when a patient record and lacking appropriate permissions
		strSQL = FormatString("SELECT AuditT.ChangedByUserName AS UserName, AuditT.ChangedAtLocationName, AuditT.ChangedDate AS TDate, (CASE WHEN AuditDetailsT.Priority = 1 THEN 'High' WHEN AuditDetailsT.Priority = 2 THEN 'Medium' WHEN AuditDetailsT.Priority = 3 THEN 'Low' ELSE '' End) AS PriorityDesc, "
			"AuditDetailsT.ID, AuditDetailsT.AuditID, AuditDetailsT.RecordID, %s AS OldValue, %s AS NewValue, AuditDetailsT.Priority, AuditDetailsT.Type, %s, AuditDetailsT.ItemID, "
			"AuditDetailsT.Priority AS Priority, AuditDetailsT.ID AS AuditDetailID, "
						 "       AuditDetailsT.InternalPatientID, "
						 "       COALESCE(PatientsT.UserDefinedID, PatientPersonT.ArchivedUserDefinedID) AS PatientID, "	
						 "       (CONVERT(nvarchar(3), AuditT.IPOctet1) + '.' + "
						 "		  CONVERT(nvarchar(3), AuditT.IPOctet2) + '.' + "
						 "		  CONVERT(nvarchar(3), AuditT.IPOctet3) + '.' + "
						 "		  CONVERT(nvarchar(3), AuditT.IPOctet4)) AS IPAddress, "
					"'' AS Item, "
					"AuditDetailsT.OldValue AS ItemType "
					"FROM AuditT INNER JOIN AuditDetailsT ON AuditT.ID = AuditDetailsT.AuditID "
					"INNER JOIN PersonT ON AuditDetailsT.RecordID = PersonT.ID "
					"LEFT JOIN PersonT PatientPersonT ON AuditDetailsT.InternalPatientID = PatientPersonT.ID "
					"LEFT JOIN PatientsT ON PatientPersonT.ID = PatientsT.PersonID "
					"WHERE (%s)"
					, strOld, strNew, strPersonName, strItemFilter);
	}

	return strSQL;
}


const LPCTSTR AuditXML::am = "http://audit-message-uri";
const LPCTSTR AuditXML::NexTechAuditOID = "2.25.79364944623376954839912467830817539355.5.1";
const LPCTSTR AuditXML::NexTechAuditOID_Name = "NexTech Auditing Event ID";

// creates a AM element
MSXML2::IXMLDOMNodePtr AuditXML::CreateNode(LPCTSTR szName)
{
	return Doc->createNode(NODE_ELEMENT, szName, am);
}

// creates a AM element with text
MSXML2::IXMLDOMNodePtr AuditXML::CreateNodeWithText(LPCTSTR szName, LPCTSTR szText)
{
	MSXML2::IXMLDOMNodePtr Node = Doc->createNode(NODE_ELEMENT, szName, am);
	Node->text = szText;
	return Node;
}

// creates a AM attribute with a value
MSXML2::IXMLDOMNodePtr AuditXML::CreateAttribute(LPCTSTR szName, _variant_t varValue)
{
	MSXML2::IXMLDOMNodePtr Attribute = Doc->createNode(NODE_ATTRIBUTE, szName, am);
	Attribute->nodeValue = varValue;

	return Attribute;
}

MSXML2::IXMLDOMNodePtr AuditXML::CreateCode(LPCTSTR szElementName, _variant_t varCode, _variant_t varDisplayName, _variant_t varCodeSystemName, _variant_t varCodeSystem)
{
	MSXML2::IXMLDOMNodePtr Node = CreateNode(szElementName);
	if (varCode.vt != VT_NULL && varCode.vt != VT_EMPTY) {
		Node->attributes->setNamedItem(CreateAttribute("code", varCode));
	}
	if (varDisplayName.vt != VT_NULL && varDisplayName.vt != VT_EMPTY) {
		Node->attributes->setNamedItem(CreateAttribute("displayName", varDisplayName));
	}
	if (varCodeSystemName.vt != VT_NULL && varCodeSystemName.vt != VT_EMPTY) {
		Node->attributes->setNamedItem(CreateAttribute("codeSystemName", varCodeSystemName));
	}
	if (varCodeSystem.vt != VT_NULL && varCodeSystem.vt != VT_EMPTY) {
		Node->attributes->setNamedItem(CreateAttribute("codeSystem", varCodeSystem));
	}

	return Node;
}

MSXML2::IXMLDOMNodePtr AuditXML::CreateTypeValuePair(LPCTSTR szElementName, LPCTSTR szType, LPCTSTR szValue)
{
	MSXML2::IXMLDOMNodePtr Node = CreateNode(szElementName);
	Node->attributes->setNamedItem(CreateAttribute("type", szType));
	Node->attributes->setNamedItem(CreateAttribute("value", (LPCTSTR)CBase64::Encode((unsigned char*)szValue, strlen(szValue))));

	return Node;
}

// creates a AM attribute
MSXML2::IXMLDOMNodePtr AuditXML::CreateAttribute(LPCTSTR szName)
{
	return Doc->createNode(NODE_ATTRIBUTE, szName, am);
}

LPCTSTR AuditXML::GetCRUD(int Type)
{
	switch(Type) {
		case aetChanged:
			return "U";
		case aetDeleted:
			return "D";
		case aetCreated:
			return "C";
		case aetOpened:
			return "R";
		// (a.walling 2010-01-11 16:04) - PLID 35779 - Certain events, such as logon, are technically 'Execute' and not CRUD, but not a big deal.
		default:
			ThrowNxException("Invalid audit event type %li", (long)Type);
	}
}

// (a.walling 2009-12-17 12:58) - PLID 35779 - Format RFC 3881-compliant audit messages
void AuditXML::CreateDocument(CString PersonName, long AuditID, long Item, long RecordID, CString OldValue, CString NewValue, int Priority, int Type)
{
/* http://tools.ietf.org/html/rfc3881

schema URI: http://audit-message-uri

AuditMessage
	EventIdentification
		EventActionCode (Optional, the usual CRUDE types)
		EventDateTime (ISO 8601)
		EventOutcomeIndicator (0 = Success, 4 = Minor failure, 8 = Serious failure, 12 = Major failure)
		EventID (Per-implementation provided CV vocabulary)
		EventTypeCode (Optional)
	ActiveParticipant
		UserID - Current user ID
		UserName (Optional) - Current username
	AuditSourceIdentification
		AuditSourceID (Unique identifier text string for audited domain) - Current user location
	ParticipantObjectIdentification (Optional)
		ParticipantObjectID (Unique identifier)
			ParticipantObjectIDTypeCode
		ParticipantObjectName (Optional)
		ParticipantObjectDetail (Optional)
			type
			value

*/
	HR(Doc.CreateInstance(__uuidof(MSXML2::DOMDocument60)));

	// (a.walling 2010-06-08 10:02) - PLID 38558 - Audit flags
	EnsureAuditFlags();
	
	
	MSXML2::IXMLDOMNodePtr Root = Doc->appendChild(CreateNode("AuditMessage"));

	// EventIdentification
	{
		MSXML2::IXMLDOMNodePtr Element = Root->appendChild(CreateNode("EventIdentification"));
		Element->attributes->setNamedItem(CreateAttribute("EventActionCode", GetCRUD(Type)));
		Element->attributes->setNamedItem(CreateAttribute("EventDateTime", (LPCTSTR)GetISO8601Timestamp()));

		// (a.walling 2010-01-11 16:05) - PLID 35779 - Marking everything as success, even failed login, however the 'failed login' event was a success, I suppose
		Element->attributes->setNamedItem(CreateAttribute("EventOutcomeIndicator", "0"));
		Element->appendChild(CreateCode("EventID", Item, (LPCTSTR)GetAuditItemDescription(Item), NexTechAuditOID_Name, NexTechAuditOID));
	}

	// ActiveParticipant
	{
		MSXML2::IXMLDOMNodePtr Element = Root->appendChild(CreateNode("ActiveParticipant"));
		Element->attributes->setNamedItem(CreateAttribute("UserID", GetCurrentUserID()));
		Element->attributes->setNamedItem(CreateAttribute("UserName", (LPCTSTR)GetCurrentUserName()));
	}

	// AuditSourceIdentification
	{
		MSXML2::IXMLDOMNodePtr Element = Root->appendChild(CreateNode("AuditSourceIdentification"));
		Element->attributes->setNamedItem(CreateAttribute("AuditSourceID", GetCurrentLocationName()));
	}
	
	// ParticipantObjectIdentification
	if (!OldValue.IsEmpty() || !NewValue.IsEmpty()) {
		MSXML2::IXMLDOMNodePtr Element = Root->appendChild(CreateNode("ParticipantObjectIdentification"));
		Element->attributes->setNamedItem(CreateAttribute("ParticipantObjectID", RecordID));
		// our 'types' are dependent on our event, so we can just re-use the same standard here. However I will not duplicate the description or name to save space.
		Element->appendChild(CreateCode("ParticipantObjectIDTypeCode", Item, g_cvarNull, g_cvarNull, NexTechAuditOID));

		if (!OldValue.IsEmpty()) {
			Element->appendChild(CreateTypeValuePair("ParticipantObjectDetail", "OldValue", OldValue));
		}
		if (!NewValue.IsEmpty()) {
			Element->appendChild(CreateTypeValuePair("ParticipantObjectDetail", "NewValue", NewValue));
		}
	}
}


void AuditXML::Validate() {
	// (a.walling 2010-01-11 16:05) - PLID 35579 - Comment this back in if you need to validate this stuff for whatever reason.
/*
#ifdef _DEBUG
	try {
		CString strXML = GetXML();

		//create our array
		CPtrArray pSchemaArray;

		SchemaType *pSchema = new SchemaType;
		
		pSchema->nResourceID = IDR_AUDITMESSAGE_XSD;
		pSchema->strNameSpace = am;
		pSchema->strResourceFolder = "XSD";

		pSchemaArray.Add(pSchema);

		//throws its own errors

		BOOL bReturn = FALSE;
		try {
			bReturn = ValidateXMLWithSchema(strXML, &pSchemaArray);
		} catch (...) {
			//delete the array
			for(int i = 0; i < pSchemaArray.GetSize(); i++) {
				SchemaType* pSchema = (SchemaType*)pSchemaArray.GetAt(i);
				if(pSchema)
					delete pSchema;
			}
			pSchemaArray.RemoveAll();

			throw;
		};

		//now delete the array
		for(int i = 0; i < pSchemaArray.GetSize(); i++) {
			SchemaType* pSchema = (SchemaType*)pSchemaArray.GetAt(i);
			if(pSchema)
				delete pSchema;
		}
		pSchemaArray.RemoveAll();
	} NxCatchAllThread("Error validating audit message XML");
#endif
*/
}

CString AuditXML::GetXML()
{
	CString strXML((LPCTSTR)Doc->xml);
	return strXML;
}



// AuditTrail.h: interface for the AuditTrail class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AUDITTRAIL_H__64B1D6C1_D029_11D4_A760_0001024317D6__INCLUDED_)
#define AFX_AUDITTRAIL_H__64B1D6C1_D029_11D4_A760_0001024317D6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//TES 8/26/2005 - Copied the enums into a sharable file, GlobalAuditUtils.h
#include "GlobalAuditUtils.h"
#include "NxPracticeSharedLib\AuditUtils.h"


ADODB::_RecordsetPtr GetAuditTrailReportRecordset(CString strSQL);
CString GetAuditTrailFinancialReportSql();
CString GetAuditTrailSchedulerReportSql(LPCTSTR strPatient);
CString GetAuditTrailInventoryReportSql();
CString GetAuditTrailPatientsReportSql();
CString GetAuditTrailContactsReportSql();
CString GetAuditTrailInsuranceReportSql();
CString GetAuditTrailPalmReportSql();
CString GetAuditTrailEMRReportSql();
CString GetAuditTrailMiscReportSql();
//TES 7/11/2008 - PLID 30668 - New report, filters on just the financial audits that could throw off the AR
CString GetAuditTrailARIssuesReportSql(const COleDateTime &dtFrom);
//TES 6/11/2009 - PLID 34607 - New report just on General 1 Demographics
CString GetAuditTrailDemographicReportSql(long nPatientID);

long BeginNewAuditEvent();
long BeginNewAuditEvent(CString strUserName);

// (a.walling 2013-04-05 12:28) - PLID 56091 - Sorely needed, basic structure for an AuditDetail entry
//struct AuditDetail
//{
//	long auditID;
//	AuditEventItems item;
//	long recordID;
//	long patientID;
//	AuditEventTypes type;
//	AuditEventPriorities priority;
//
//	CString personName;
//	CString oldValue;
//	CString newValue;
//
//	AuditDetail()
//		: auditID(0)
//		, item((AuditEventItems)-1)
//		, recordID(-1)
//		, patientID(-1)
//		, type(aetChanged)
//		, priority(aepMedium)
//	{}
//	
//	AuditDetail(long PatientID, CString PersonName, long AuditID, long Item, long RecordID, CString OldValue, CString NewValue, int Priority, int Type = 1)
//		: auditID(AuditID)
//		, item((AuditEventItems)Item)
//		, recordID(RecordID)
//		, patientID(PatientID)
//		, type((AuditEventTypes)Type)
//		, priority((AuditEventPriorities)Priority)
//		, personName(PersonName)
//		, oldValue(OldValue)
//		, newValue(NewValue)
//	{}
//};



// (a.walling 2010-01-21 12:36) - PLID 37018 - Require a patient ID, or -1 if not applicable
void AuditEvent(long PatientID, CString PersonName, long AuditID, long Item, long RecordID, CString OldValue, CString NewValue, int Priority, int Type = 1);

// (j.jones 2007-08-30 09:54) - PLID 27221 - Behaves like AuditEvent, in that it supports transactions,
// but takes in a CPendingAuditInfo item which contains all the audit information. Optionally a
// new record ID can be sent in to override pPendingAuditInfo->nRecordID, incase we didn't have a
// valid record ID when we creatd pPendingAuditInfo.
void AuditPendingEvent(long nAuditID, CPendingAuditInfo *pPendingAuditInfo, long nNewRecordID = -1);


void CommitAuditTransaction(long AuditID);
void RollbackAuditTransaction(long AuditID);
long BeginAuditTransaction();




// (a.walling 2013-04-05 12:28) - PLID 56091 - AuditXML stuff for syslog internal to AuditTrail.cpp


#endif // !defined(AFX_AUDITTRAIL_H__64B1D6C1_D029_11D4_A760_0001024317D6__INCLUDED_)

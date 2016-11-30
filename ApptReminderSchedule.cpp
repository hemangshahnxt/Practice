// (r.gonet 2010-06-23) - PLID 39012 - Created. All comments, unless marked otherwise, are for this PLID.

#include "stdafx.h"
#include "ApptReminderSchedule.h"

using namespace ADODB;


CApptReminderSchedule::CApptReminderSchedule()
{
	m_nNewRuleID = -1;
	m_nNewExceptionID = -1;
	// (r.gonet 2015-11-16 02:33) - PLID 67587 - Init the sending limit options.
	m_nSendingLimitOptions = ESendingLimitOptions::Invalid;
}

// Rules and exceptions are dynamically allocated so be sure to clean them up
CApptReminderSchedule::~CApptReminderSchedule()
{
	this->ClearRules();
	this->ClearExceptions();
}

// Adds a rule to the schedule
void CApptReminderSchedule::AddRule(CApptReminderScheduleRule *pRule)
{
	if(pRule == NULL){
		return;
	}

	// Set the id to something temporary
	pRule->SetID(m_nNewRuleID--);
	m_arRules.Add(pRule);
	pRule->SetIsNew(TRUE);
}

// Removes the first rule encountered that matches the ID passed in
void CApptReminderSchedule::RemoveRule(long nID)
{
	int nCount = m_arRules.GetCount();
	for(int i = 0; i < nCount; i++) {
		CApptReminderScheduleRule *pRule = m_arRules.GetAt(i);
		if(pRule && pRule->GetID() == nID) {
			pRule->SetIsDeleted(TRUE);
			return;
		}
	}
}

// Adds an exception to the schedule
void CApptReminderSchedule::AddException(CApptReminderException *pException)
{
	if(pException == NULL){
		return;
	}

	// Set the id to something temporary
	pException->SetID(m_nNewExceptionID--);
	m_arExceptions.Add(pException);
	pException->SetIsNew(TRUE);
}

// Removes the first exception encountered that matches the ID passed in
void CApptReminderSchedule::RemoveException(long nID)
{
	int nCount = m_arExceptions.GetCount();
	for(int i = 0; i < nCount; i++) {
		CApptReminderException *pException = m_arExceptions.GetAt(i);
		if(pException && pException->GetID() == nID) {
			pException->SetIsDeleted(TRUE);
			return;
		}
	}
}

// (r.gonet 2015-11-16 02:33) - PLID 67587 - Set the sending limit options bitmask, controlling how
// NexReminderClient considers an appointment reminder message already sent. 
void CApptReminderSchedule::SetSendingLimitOptions(long nSendingLimitOptions)
{
	m_nSendingLimitOptions = nSendingLimitOptions;
}

// (r.gonet 2015-11-16 02:33) - PLID 67587 - Gets the sending limit options bitmask, controlling how
// NexReminderClient considers an appointment reminder message already sent. 
long CApptReminderSchedule::GetSendingLimitOptions() const
{
	return m_nSendingLimitOptions;
}

// Gets the number of rules in the schedule
int CApptReminderSchedule::GetRuleCount()
{
	return m_arRules.GetCount();
}

// Gets the number of exceptions in the schedule
int CApptReminderSchedule::GetExceptionCount()
{
	return m_arExceptions.GetCount();
}

// Retrieves a rule based on its position in the rule list or NULL if the position is invalid
CApptReminderScheduleRule * CApptReminderSchedule::GetRule(int nIndex)
{
	if(nIndex >= m_arRules.GetCount()) {
		return NULL;
	}
	return m_arRules.GetAt(nIndex);
}

// Retrieves an exception based on its position in the exception list or NULL if the position is invalid
CApptReminderException * CApptReminderSchedule::GetException(int nIndex)
{
	if(nIndex >= m_arExceptions.GetCount()) {
		return NULL;
	}
	return m_arExceptions.GetAt(nIndex);
}

// Retrieves a rule based on its ID or NULL if it does not exist
CApptReminderScheduleRule * CApptReminderSchedule::GetRuleById(int nID)
{
	int nCount = m_arRules.GetCount();
	for(int i = 0; i < nCount; i++) {
		CApptReminderScheduleRule *pRule = m_arRules.GetAt(i);
		if(pRule && pRule->GetID() == nID) {
			return pRule;
		}
	}
	return NULL;
}

// Retrieves an exception based on its ID or NULL if it does not exist
CApptReminderException * CApptReminderSchedule::GetExceptionById(int nID)
{
	int nCount = m_arExceptions.GetCount();
	for(int i = 0; i < nCount; i++) {
		CApptReminderException *pException = m_arExceptions.GetAt(i);
		if(pException && pException->GetID() == nID) {
			return pException;
		}
	}
	return NULL;
}

// Gets the first valid rule position for use in iteration
long CApptReminderSchedule::GetRuleStartPosition()
{
	int nCount = m_arRules.GetCount();
	for(int i = 0; i < nCount; i++) {
		CApptReminderScheduleRule *pRule = m_arRules.GetAt(i);
		if(pRule && !pRule->GetIsDeleted()) {
			return i;
		}
	}
	return 0;
}

// Gets the first valid exception position for use in iteration
long CApptReminderSchedule::GetExceptionStartPosition()
{
	int nCount = m_arExceptions.GetCount();
	for(int i = 0; i < nCount; i++) {
		CApptReminderException *pException = m_arExceptions.GetAt(i);
		if(pException && !pException->GetIsDeleted()) {
			return i;
		}
	}
	return 0;
}

// Iterate through the non deleted rules. Returns true if there are more rules, false otherwise. 
// pRu/e gets the element. nPosition keeps track of the position in the rule list.
// Use GetRuleStartPosition() to get the first nPosition. It is changed after each iteration.
BOOL CApptReminderSchedule::GetNextRule(CApptReminderScheduleRule OUT * &pRule, long &nPosition)
{
	int nCount = m_arRules.GetCount();
	for(int i = nPosition; i < nCount; i++) {
		CApptReminderScheduleRule *pRuleTemp = m_arRules.GetAt(i);
		if(pRuleTemp && !pRuleTemp->GetIsDeleted()) {
			pRule = pRuleTemp;
			nPosition = i + 1;
			return TRUE;
		}
	}
	pRule = NULL;
	return FALSE;
}

// Iterate through the non deleted exceptions. Returns true if there are more exceptions, false otherwise. 
// pException gets the element. nPosition keeps track of the position in the exception list.
// Use GetExceptionStartPosition() to get the first nPosition. It is changed after each iteration.
BOOL CApptReminderSchedule::GetNextException(CApptReminderException OUT * &pException, long &nPosition)
{
	int nCount = m_arExceptions.GetCount();
	for(int i = nPosition; i < nCount; i++) {
		CApptReminderException *pExceptionTemp = m_arExceptions.GetAt(i);
		if(pExceptionTemp && !pExceptionTemp->GetIsDeleted()) {
			pException = pExceptionTemp;
			nPosition = i + 1;
			return TRUE;
		}
	}
	pException = NULL;
	return FALSE;
}

// Remove all rules
void CApptReminderSchedule::ClearRules()
{
	int nCount = m_arRules.GetCount();
	while(m_arRules.GetCount() > 0) {
		CApptReminderScheduleRule *pRule = m_arRules.GetAt(0);
		if(pRule) {
			m_arRules.RemoveAt(0);
			delete pRule;
			pRule = NULL;
		}
	}

}

// Remove all exceptions
void CApptReminderSchedule::ClearExceptions()
{
	int nCount = m_arExceptions.GetCount();
	while(m_arExceptions.GetCount() > 0) {
		CApptReminderException *pException = m_arExceptions.GetAt(0);
		if(pException) {
			m_arExceptions.RemoveAt(0);
			delete pException;
			pException = NULL;
		}
	}
}

// Return true if any rules or exceptions have been modified in the schedule
BOOL CApptReminderSchedule::GetModified()
{
	// Check the rules to see if they were modified
	int nRuleCount = m_arRules.GetCount();
	for(int i = 0; i < nRuleCount; i++) {
		CApptReminderScheduleRule *pRule = m_arRules.GetAt(i);
		if(pRule && pRule->GetIsModified()) {
			return TRUE;
		}
	}

	int nExceptionCount = m_arExceptions.GetCount();
	for(int i = 0; i < nExceptionCount; i++) {
		CApptReminderException *pException = m_arExceptions.GetAt(i);
		if(pException && pException->GetIsModified()) {
			return TRUE;
		}
	}

	// (r.gonet 2015-11-16 02:33) - PLID 67587 - Check if the sending limit options bitmask has been modified.
	long nOldSendingLimitOptions = GetRemotePropertyInt("ApptReminderSendingLimitOptions", ESendingLimitOptions::Invalid, 0, 0, false);
	if (nOldSendingLimitOptions != m_nSendingLimitOptions) {
		return TRUE;
	}

	return FALSE;
}

// Save every rule and exception to the database
void CApptReminderSchedule::Save()
{
	CString strBatch = "";
	CNxParamSqlArray aryParams;
	// Ask all rules to save themselves to the batch, if they are modified
	int nRuleCount = m_arRules.GetCount();
	for(int i = 0; i < nRuleCount; i++) {
		CApptReminderScheduleRule *pRule = m_arRules.GetAt(i);
		if(pRule && pRule->GetIsModified()) {
			pRule->AddToSqlBatch(strBatch, aryParams);
		}
	}

	// Ask all exceptions to save themselves to the batch, if they are modified
	int nExceptionCount = m_arExceptions.GetCount();
	for(int i = 0; i < nExceptionCount; i++) {
		CApptReminderException *pException = m_arExceptions.GetAt(i);
		if(pException && pException->GetIsModified()) {
			pException->AddToSqlBatch(strBatch, aryParams);
		}
	}

	// Do the save. Do not make this parameterized as there would probably be too many parameters
	//  for SQL Server to handle and it would give you an error.
	ExecuteParamSqlBatch(GetRemoteData(), strBatch, aryParams);

	// (r.gonet 2015-11-16 02:33) - PLID 67587 - Don't let an invalid sending limit options value be saved. 
	if (m_nSendingLimitOptions == ESendingLimitOptions::Invalid) {
		ThrowNxException("%s : Attempted to save ESendingLimitOptions::Invalid as the appointment reminder sending limit.", __FUNCTION__);
	}
	// (r.gonet 2015-11-16 02:33) - PLID 67587 - Save the sending limit options bitmask.
	SetRemotePropertyInt("ApptReminderSendingLimitOptions", m_nSendingLimitOptions);
}

// (r.gonet 2015-11-16 02:33) - PLID 67587 - Loads the appointment reminder schedule from data.
// Before we just used to call LoadRules and LoadExceptions individually.
void CApptReminderSchedule::Load()
{
	LoadRules();
	LoadExceptions();
	// (r.gonet 2015-11-16 02:33) - PLID 67587 - Load the sending limit options.
	// Default to the legacy behavior.
	m_nSendingLimitOptions = GetRemotePropertyInt("ApptReminderSendingLimitOptions", 
			ESendingLimitOptions::PerRule | ESendingLimitOptions::PerAppointment, 0, 0, false);
}

// Load all reminder rules into the schedule from the database
void CApptReminderSchedule::LoadRules()
{
	// Clear the rules and reload them from the database
	this->ClearRules();
	
	// (z.manning 2011-05-16 11:40) - PLID 43708 - Added more fields to support different send types
	_RecordsetPtr prs = CreateRecordset(GetRemoteData(), 
		"SELECT ID, AdvanceNotice, IgnoreExceptions, AllTypes, AllResources, SendType, SendTime, SendTimeDaysBefore \r\n"
		"FROM ApptReminderRulesT \r\n"
		"WHERE Deleted = 0 \r\n"
		);
	// Create an encapsulating object for each record we pull
	while(!prs->eof) {
		long nID = AdoFldLong(prs, "ID");
		long nAdvanceNotice = AdoFldLong(prs, "AdvanceNotice");
		BOOL bIgnoreExceptions = AdoFldBool(prs, "IgnoreExceptions");
		// (r.gonet 2010-06-23) - PLID 39282 - Load whether we are filtering on all types and resources
		BOOL bAllTypes = AdoFldBool(prs, "AllTypes");
		BOOL bAllResources = AdoFldBool(prs, "AllResources");
		CApptReminderScheduleRule::ESendType eSendType = (CApptReminderScheduleRule::ESendType)AdoFldByte(prs, "SendType");
		COleDateTime dtSendTime = AdoFldDateTime(prs, "SendTime");
		short nSendTimeDaysBefore = AdoFldShort(prs, "SendTimeDaysBefore");

		// Transformation
		COleDateTimeSpan tsAdvanceNotice(0, nAdvanceNotice, 0, 0);

		CArray<long, long> aryTypeIDs;
		CArray<long, long> aryResourceIDs;

		// (r.gonet 2010-09-02) - PLID 39282 - We also need the associated appointment types and resources
		_RecordsetPtr prsFilters = CreateParamRecordset(GetRemoteData(), 
			"SELECT AppointmentType, AppointmentResource "
			"FROM ApptReminderFiltersT "
			"WHERE RuleID = {INT}",
			nID);
		while(!prsFilters->eof) {
			long nAppointmentTypeID = AdoFldLong(prsFilters, "AppointmentType", -1);
			long nAppointmentResourceID = AdoFldLong(prsFilters, "AppointmentResource", -1);
			if(nAppointmentTypeID != -1) {
				// We have a type
				aryTypeIDs.Add(nAppointmentTypeID);
			} else if(nAppointmentResourceID != -1) {
				// We have a resource
				aryResourceIDs.Add(nAppointmentResourceID);
			}
			prsFilters->MoveNext();
		}

		// (r.gonet 2010-09-02) - PLID 39282 - Assign the type ids and resource ids in the constructor. Otherwise the modified bit will be set.
		CApptReminderScheduleRule *pRule = new CApptReminderScheduleRule(nID, tsAdvanceNotice, eSendType, dtSendTime, nSendTimeDaysBefore, bIgnoreExceptions,
			bAllTypes, bAllResources, aryTypeIDs, aryResourceIDs);

		// Now add the rule to the schedule
		this->m_arRules.Add(pRule);

		prs->MoveNext();
	}
}

// Load all exceptions into the schedule from the database
void CApptReminderSchedule::LoadExceptions()
{
	// Clear the exceptions and reload them from the database
	this->ClearExceptions();
	
	// (r.gonet 12/06/2010) - PLID 40575 - Added in an option for not filtering on days at all.
	_RecordsetPtr prs = CreateRecordset(GetRemoteData(), "SELECT ID, StartDate, StartTime, EndDate, EndTime, FilterOnWeekdays, WeekDays FROM ApptReminderExceptionsT");
	// Create an encapsulating object for each record we pull
	while(!prs->eof) {
		COleDateTime dtNull;
		dtNull.SetStatus(COleDateTime::null);
		long nID = AdoFldLong(prs, "ID");
		COleDateTime dtStartDate(AdoFldDateTime(prs, "StartDate", dtNull));
		COleDateTime dtStartTime(AdoFldDateTime(prs, "StartTime", dtNull));
		COleDateTime dtEndDate(AdoFldDateTime(prs, "EndDate", dtNull));
		COleDateTime dtEndTime(AdoFldDateTime(prs, "EndTime", dtNull));
		// (r.gonet 12/06/2010) - PLID 40575 - Added in an option for not filtering on days at all.
		BOOL bFilterOnWeekDays = AdoFldBool(prs, "FilterOnWeekDays", FALSE);
		long bmWeekdays = AdoFldLong(prs, "WeekDays", 0);

		CApptReminderException *pException = new CApptReminderException(nID, dtStartDate, dtStartTime, dtEndDate, dtEndTime, bFilterOnWeekDays, bmWeekdays);

		// Now add the exception to the schedule
		this->m_arExceptions.Add(pException);

		prs->MoveNext();
	}
}
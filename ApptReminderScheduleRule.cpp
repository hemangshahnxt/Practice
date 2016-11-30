#include "stdafx.h"
#include "ApptReminderScheduleRule.h"
#include "InternationalUtils.h"

using namespace ADODB;

/////////////////////////////////////
// Appointment Reminder Rules      //
/////////////////////////////////////

// (r.gonet 2010-06-23) - PLID 40574 - Construction and Destruction
CApptReminderScheduleRule::CApptReminderScheduleRule()
: m_nID(0), m_tsAdvanceNotice(), m_bIgnoreExceptions(FALSE), m_bFilterOnAllTypes(FALSE), m_bFilterOnAllResources(FALSE)
	, m_eSendType(stHoursBeforeAppt), m_nSendTimeDaysBefore(1)
{
	m_bAdded = FALSE;
	m_bDeleted = FALSE;
	m_bModified = FALSE;
	m_dtSendTime.SetTime(11, 0, 0);
}

// (r.gonet 2010-06-23) - PLID 40574
// (z.manning 2011-05-16 11:40) - PLID 43708 - Added more params to support different send types
CApptReminderScheduleRule::CApptReminderScheduleRule(long nID, COleDateTimeSpan tsAdvanceNotice, ESendType eSendType, COleDateTime dtSendTime, short nSendTimeDaysBefore, BOOL bIgnoreExceptions,
													 BOOL bAllTypes, BOOL bAllResources, CArray<long, long> &arynTypes, CArray<long, long> &arynResources)
: m_nID(nID), m_tsAdvanceNotice(tsAdvanceNotice), m_bIgnoreExceptions(bIgnoreExceptions), 
m_bFilterOnAllTypes(bAllTypes), m_bFilterOnAllResources(bAllResources), m_eSendType(eSendType), m_dtSendTime(dtSendTime), m_nSendTimeDaysBefore(nSendTimeDaysBefore)
{
	m_bAdded = FALSE;
	m_bDeleted = FALSE;
	m_bModified = FALSE;
	m_arynApptTypes.Copy(arynTypes);
	m_arynApptResources.Copy(arynResources);
}

// (r.gonet 2010-06-23) - PLID 40574 - Implementation
void CApptReminderScheduleRule::SetID(long nID)
{
	// (r.gonet 2010-06-23) - PLID 40574 - Don't track ID changes
	m_nID = nID;
}

// (r.gonet 2010-06-23) - PLID 40574 - Sets how long before the appointment to send a reminder
void CApptReminderScheduleRule::SetAdvanceNotice(COleDateTimeSpan tsAdvanceNotice)
{
	if(m_tsAdvanceNotice != tsAdvanceNotice) {
		m_bModified = TRUE;
	}
	m_tsAdvanceNotice = tsAdvanceNotice;
}

// (r.gonet 2010-06-23) - PLID 40574 - Sets whether to override all appointment reminder schedule exceptions
void CApptReminderScheduleRule::SetIgnoreExceptions(BOOL bIgnoreExceptions)
{
	if(m_bIgnoreExceptions != bIgnoreExceptions) {
		m_bModified = TRUE;
	}
	m_bIgnoreExceptions = bIgnoreExceptions;
}

// (r.gonet 2010-06-23) - PLID 39282 - Sets the appointment types, by id, to send a reminder for
void CApptReminderScheduleRule::SetAppointmentTypes(CArray<long, long> &arynTypes)
{
	BOOL bAppointmentTypesModified = FALSE;
	// Determine if this modifies our internal set of types
	// First compare the counts
	if(arynTypes.GetCount() != m_arynApptTypes.GetCount()) {
		bAppointmentTypesModified = TRUE;
	} else {
		// Unfortunately they are the same counts, now we have to make sure the two sets are the same.
		for(int i = 0; i < arynTypes.GetCount(); i++) {
			long nTypeID = arynTypes[i];
			BOOL bTypeFound = FALSE;
			for(int j = 0; j < m_arynApptTypes.GetCount(); j++) {
				if(nTypeID == m_arynApptTypes[j]) {
					bTypeFound = TRUE;
				}
			}
			if(!bTypeFound) {
				// The set differs
				bAppointmentTypesModified = TRUE;
			}
		}
	}

	// Now to verify there are no duplicate entries, but only if the set changed
	if(bAppointmentTypesModified) {
		for(int i = 0; i < arynTypes.GetCount(); i++) {
			for(int j = i + 1; j < arynTypes.GetCount(); j++) {
				if(arynTypes[i] == arynTypes[j]) { 
					// Duplicate found! This should never happen. The function may have been called incorrecly. 
					// Assert for developers.
					ASSERT(FALSE);
				}
			}
		}
	}

	if(bAppointmentTypesModified) {
		m_arynApptTypes.RemoveAll();
		m_arynApptTypes.Copy(arynTypes);
		m_bModified = TRUE;
	}
}

// (r.gonet 2010-06-23) - PLID 39282 - Sets the appointment resources, by id, to send a reminder for
void CApptReminderScheduleRule::SetAppointmentResources(CArray<long, long> &arynResources)
{
	BOOL bAppointmentResourcesModified = FALSE;
	// Determine if this modifies our internal set of Resources
	// First compare the counts
	if(arynResources.GetCount() != m_arynApptResources.GetCount()) {
		bAppointmentResourcesModified = TRUE;
	} else {
		// Unfortunately they are the same counts, now we have to make sure the two sets are the same.
		for(int i = 0; i < arynResources.GetCount(); i++) {
			long nTypeID = arynResources[i];
			BOOL bTypeFound = FALSE;
			for(int j = 0; j < m_arynApptResources.GetCount(); j++) {
				if(nTypeID == m_arynApptResources[j]) {
					bTypeFound = TRUE;
				}
			}
			if(!bTypeFound) {
				// The set differs
				bAppointmentResourcesModified = TRUE;
			}
		}
	}

	// Now to verify there are no duplicate entries, but only if the set changed
	if(bAppointmentResourcesModified) {
		for(int i = 0; i < arynResources.GetCount(); i++) {
			for(int j = i + 1; j < arynResources.GetCount(); j++) {
				if(arynResources[i] == arynResources[j]) { 
					// Duplicate found! This should never happen. The function may have been called incorrecly. 
					// Assert for developers.
					ASSERT(FALSE);
				}
			}
		}
	}

	if(bAppointmentResourcesModified) {
		m_arynApptResources.RemoveAll();
		m_arynApptResources.Copy(arynResources);
		m_bModified = TRUE;
	}
}

// (r.gonet 2010-06-23) - PLID 39282 - Adds an appointment type to filter for what appointments to send a reminder for
void CApptReminderScheduleRule::AddAppointmentType(long nTypeID)
{
	if(!ExistsInArray(nTypeID, m_arynApptTypes)) {
		m_arynApptTypes.Add(nTypeID);
		m_bModified = TRUE;
	}
}

// (r.gonet 2010-06-23) - PLID 40574 - Adds an appointment resource to filter for what appointments to send a reminder for
void CApptReminderScheduleRule::AddAppointmentResource(long nResourceID)
{
	if(!ExistsInArray(nResourceID, m_arynApptResources)) {
		m_arynApptResources.Add(nResourceID);
		m_bModified = TRUE;
	}
}

// (r.gonet 2010-06-23) - PLID 39282 - Removes an appointment type to filter for what appointments to send a reminder for
void CApptReminderScheduleRule::RemoveAppointmentType(long nTypeID)
{
	for(int i = 0; i < m_arynApptTypes.GetCount(); i++) {
		if(m_arynApptTypes[i] == nTypeID) {
			m_arynApptTypes.RemoveAt(i);
			m_bModified = TRUE;
			return;
		}
	}
}

// (r.gonet 2010-06-23) - PLID 39282 - Removes an appointment resource to filter for what appointments to send a reminder for
void CApptReminderScheduleRule::RemoveAppointmentResource(long nResourceID)
{
	for(int i = 0; i < m_arynApptResources.GetCount(); i++) {
		if(m_arynApptResources[i] == nResourceID) {
			m_arynApptResources.RemoveAt(i);
			m_bModified = TRUE;
			return;
		}
	}
}

// (r.gonet 2010-06-23) - PLID 39282 - Removes all appointment types to filter on
void CApptReminderScheduleRule::RemoveAllAppointmentTypes()
{
	if(m_arynApptTypes.GetCount() > 0) {
		m_bModified = TRUE;
	}
	m_arynApptTypes.RemoveAll();
}

// (r.gonet 2010-06-23) - PLID 39282 - Removes all appointment resources to filter on
void CApptReminderScheduleRule::RemoveAllAppointmentResources()
{
	if(m_arynApptResources.GetCount() > 0) {
		m_bModified = TRUE;
	}
	m_arynApptResources.RemoveAll();
}

void CApptReminderScheduleRule::SetFilterOnAllTypes(BOOL bAll)
{
	if(m_bFilterOnAllTypes != bAll) {
		m_bModified = TRUE;
	}
	m_bFilterOnAllTypes = bAll;
}

void CApptReminderScheduleRule::SetFilterOnAllResources(BOOL bAll)
{
	if(m_bFilterOnAllResources != bAll) {
		m_bModified = TRUE;
	}
	m_bFilterOnAllResources = bAll;
}

// (r.gonet 2010-06-23) - PLID 40574 - Gets the ID of the rule
long CApptReminderScheduleRule::GetID()
{
	return m_nID;
}

// (r.gonet 2010-06-23) - PLID 40574 - Gets the amount of time before an appointment to send a reminder
COleDateTimeSpan CApptReminderScheduleRule::GetAdvanceNotice()
{
	return m_tsAdvanceNotice;
}

// (r.gonet 2010-06-23) - PLID 40574 - Gets whether or not this rule overrides all exceptions
BOOL CApptReminderScheduleRule::GetIgnoreExceptions()
{
	return m_bIgnoreExceptions;
}

// (r.gonet 2010-06-23) - PLID 39282 - Gets a copy of all appointment types this rule is filtering on
void CApptReminderScheduleRule::GetAppointmentTypes(OUT CArray<long, long> &arynTypes)
{
	// Return a copy because we don't want the caller messing with our internal state
	arynTypes.Copy(m_arynApptTypes);
}

// (r.gonet 2010-06-23) - PLID 39282 - Gets a copy of all appointment resources this rule is filtering on
void CApptReminderScheduleRule::GetAppointmentResources(OUT CArray<long, long> &arynResources)
{
	// Return a copy because we don't want the caller messing with our internal state
	arynResources.Copy(m_arynApptResources);
}

// (r.gonet 2010-06-23) - PLID 39282 - Gets whether to use all appointment types as a filter
BOOL CApptReminderScheduleRule::GetFilterOnAllTypes()
{
	return m_bFilterOnAllTypes;
}

// (r.gonet 2010-06-23) - PLID 39282 - Gets whether to use all appointment resources as a filter
BOOL CApptReminderScheduleRule::GetFilterOnAllResources()
{
	return m_bFilterOnAllResources;
}

// (r.gonet 2010-06-23) - PLID 40574 - Sets the new flag so this rule gets inserted into the database
void CApptReminderScheduleRule::SetIsNew(BOOL bValue)
{
	if(m_bAdded != bValue) {
		m_bModified = TRUE;
	}
	m_bAdded = bValue;
}

// (r.gonet 2010-06-23) - PLID 40574 - Sets the deleted flag so this rule gets deleted from the database
void CApptReminderScheduleRule::SetIsDeleted(BOOL bValue)
{
	if(m_bDeleted != bValue) {
		m_bModified = TRUE;
	}
	m_bDeleted = bValue;
}

// (r.gonet 2010-06-23) - PLID 40574 - Gets whether this rule is marked for deletion
BOOL CApptReminderScheduleRule::GetIsDeleted()
{
	return m_bDeleted;
}

// (r.gonet 2010-06-23) - PLID 40574 - Gets whether this rule is marked as being modified and thus for updating of the database
BOOL CApptReminderScheduleRule::GetIsModified()
{
	return m_bModified;
}

// (z.manning 2011-05-16 12:08) - PLID 43708
void CApptReminderScheduleRule::SetSendType(const ESendType eSendType)
{
	if(m_eSendType != eSendType) {
		m_bModified = TRUE;
	}
	m_eSendType = eSendType;
}

// (z.manning 2011-05-16 12:08) - PLID 43708
CApptReminderScheduleRule::ESendType CApptReminderScheduleRule::GetSendType()
{
	return m_eSendType;
}

// (z.manning 2011-05-16 12:08) - PLID 43708
void CApptReminderScheduleRule::SetSendTime(const COleDateTime dtSendTime)
{
	if(dtSendTime != m_dtSendTime) {
		m_bModified = TRUE;
	}
	m_dtSendTime = dtSendTime;
}

// (z.manning 2011-05-16 12:08) - PLID 43708
COleDateTime CApptReminderScheduleRule::GetSendTime()
{
	return m_dtSendTime;
}

// (z.manning 2011-05-16 12:08) - PLID 43708
void CApptReminderScheduleRule::SetSendTimeDaysBefore(const short nSendTimeDaysBefore)
{
	if(m_nSendTimeDaysBefore != nSendTimeDaysBefore) {
		m_bModified = TRUE;
	}
	m_nSendTimeDaysBefore = nSendTimeDaysBefore;
}

// (z.manning 2011-05-16 12:08) - PLID 43708
short CApptReminderScheduleRule::GetSendTimeDaysBefore()
{
	return m_nSendTimeDaysBefore;
}

// (r.gonet 2010-06-23) - PLID 40574 - Adds to an SQL batch the save statements for this rule
// (r.gonet 2010-12-13) - PLID 40574 - Updated the queries to be parameterized.
void CApptReminderScheduleRule::AddToSqlBatch(CString &strSqlBatch, CNxParamSqlArray &aryParams)
{
	if(m_bAdded) {
		if(!m_bDeleted) {
			// We need to have an sql variable to store the new ids in, so check for it and add it if necessary
			if(strSqlBatch.Find("DECLARE @ApptReminderScheduleRuleID INT;") == -1) {
				// We need to declare it
				AddParamStatementToSqlBatch(strSqlBatch, aryParams, "DECLARE @ApptReminderScheduleRuleID INT; \r\n");
			}
			// (r.gonet 2010-06-23) - PLID 39282 - Add whether to filter on all types and resources
			// (z.manning 2011-05-16 11:40) - PLID 43708 - Added more fields to support different send types
			AddParamStatementToSqlBatch(strSqlBatch, aryParams,
				"INSERT INTO ApptReminderRulesT (AdvanceNotice, IgnoreExceptions, AllTypes, AllResources, SendType, SendTime, SendTimeDaysBefore) "
				"VALUES ({INT}, {BIT}, {BIT}, {BIT}, {INT}, {OLEDATETIME}, {INT}); \r\n",
				(long)m_tsAdvanceNotice.GetTotalHours(), m_bIgnoreExceptions, m_bFilterOnAllTypes, m_bFilterOnAllResources, m_eSendType, m_dtSendTime, m_nSendTimeDaysBefore);
			AddParamStatementToSqlBatch(strSqlBatch, aryParams, "SET @ApptReminderScheduleRuleID = (SELECT CONVERT(INT, SCOPE_IDENTITY()) AS NewID); \r\n");
			// (r.gonet 2010-06-23) - PLID 39282 - Now for the appointment types and resources
			if(!m_bFilterOnAllTypes) {
				for(int i = 0; i < m_arynApptTypes.GetCount(); i++) {
					AddParamStatementToSqlBatch(strSqlBatch, aryParams,
						"INSERT INTO ApptReminderFiltersT (RuleID, AppointmentType) "
						"VALUES (@ApptReminderScheduleRuleID, {INT}); \r\n", m_arynApptTypes[i]);
				}
			}
			// (r.gonet 2010-06-23) - PLID 39282
			if(!m_bFilterOnAllResources) {
				for(int i = 0; i < m_arynApptResources.GetCount(); i++) {
					AddParamStatementToSqlBatch(strSqlBatch, aryParams,
						"INSERT INTO ApptReminderFiltersT (RuleID, AppointmentResource) "
						"VALUES (@ApptReminderScheduleRuleID, {INT}); \r\n", m_arynApptResources[i]);
				}
			}
		}
	} else if(m_bDeleted) {
		if(!m_bAdded) {
			// (r.gonet 2010-06-23) - PLID 39282 - Delete the appointment reminder types and resources first.
			AddParamStatementToSqlBatch(strSqlBatch, aryParams,
				"DELETE FROM ApptReminderFiltersT WHERE RuleID = {INT}; \r\n",
				m_nID);
			// Now the rule
			AddParamStatementToSqlBatch(strSqlBatch, aryParams,
				"UPDATE ApptReminderRulesT SET Deleted = CONVERT(BIT, 1) WHERE ID = {INT}; \r\n",
				m_nID);
		}
	} else if(m_bModified) {
		// (z.manning 2011-05-16 11:40) - PLID 43708 - Added more fields to support different send types
		AddParamStatementToSqlBatch(strSqlBatch, aryParams,
			"UPDATE ApptReminderRulesT \r\n "
			"SET AdvanceNotice = {INT}, "
			"	IgnoreExceptions = {BIT}, "
			"	AllTypes = {BIT}, "
			"	AllResources = {BIT}, "
			"	SendType = {INT}, "
			"	SendTime = {OLEDATETIME}, "
			"	SendTimeDaysBefore = {INT} "
			"WHERE ID = {INT} ; \r\n",
			(long)m_tsAdvanceNotice.GetTotalHours(), m_bIgnoreExceptions, m_bFilterOnAllTypes, m_bFilterOnAllResources, m_eSendType, m_dtSendTime, m_nSendTimeDaysBefore, m_nID);
		// (r.gonet 2010-06-23) - PLID 39282 - But we also need to update the appointment types and resources
		//  Clear out all of the old filters
		AddParamStatementToSqlBatch(strSqlBatch, aryParams,
				"DELETE FROM ApptReminderFiltersT "
				"WHERE RuleID = {INT}; \r\n", m_nID);
		// (r.gonet 2010-06-23) - PLID 39282 - Then put in the appointment type filters
		if(!m_bFilterOnAllTypes) {
			for(int i = 0; i < m_arynApptTypes.GetCount(); i++) {
				AddParamStatementToSqlBatch(strSqlBatch, aryParams,
						"INSERT INTO ApptReminderFiltersT (RuleID, AppointmentType) "
						"VALUES ({INT}, {INT}); \r\n", m_nID, m_arynApptTypes[i]);
			}
		}
		// (r.gonet 2010-06-23) - PLID 39282 - Finally, put in the appointment resource filters
		if(!m_bFilterOnAllResources) {
			for(int i = 0; i < m_arynApptResources.GetCount(); i++) {
				AddParamStatementToSqlBatch(strSqlBatch, aryParams,
						"INSERT INTO ApptReminderFiltersT (RuleID, AppointmentResource) "
						"VALUES ({INT}, {INT}); \r\n", m_nID, m_arynApptResources[i]);
			}
		}
	}
}

// (r.gonet 2010-06-23) - PLID 40574 - Return a string representing the rule in a human readable format
CString CApptReminderScheduleRule::GetDescription()
{
	CString strDescription;

	// (z.manning 2011-05-17 09:41) - PLID 43708 - We now have 2 different methods for sending reminders
	if(m_eSendType == stSpecificTime)
	{
		strDescription += FormatString("Remind at %s %li day%s before appointment"
			, FormatDateTimeForInterface(m_dtSendTime, DTF_STRIP_SECONDS, dtoTime), m_nSendTimeDaysBefore
			, m_nSendTimeDaysBefore == 1 ? "" : "s");
	}
	else
	{
		// Days and hours
		if(m_tsAdvanceNotice.GetDays() > 0) {
			strDescription.Format("Remind %li day%s, %li hour%s before appointment", 
				m_tsAdvanceNotice.GetDays(),
				(m_tsAdvanceNotice.GetDays() == 1 ? "" : "s"), 
				m_tsAdvanceNotice.GetHours(),
				(m_tsAdvanceNotice.GetHours() == 1 ? "" : "s"));
		} else {
			strDescription = FormatString("Remind %li hour%s before the appointment",
				m_tsAdvanceNotice.GetHours(),
				(m_tsAdvanceNotice.GetHours() == 1 ? "" : "s"));
		}
	}

	// (r.gonet 2010-06-23) - PLID 39282 - Add in the types and resources to the description
	CString strTypeIDs, strResourceIDs;
	strTypeIDs = ArrayAsString(m_arynApptTypes, TRUE);
	strResourceIDs = ArrayAsString(m_arynApptResources, TRUE);
	
	// (r.gonet 2010-06-23) - PLID 39282 - Now the type names, which we'll have to query for since we only have the IDs
	if(!strTypeIDs.IsEmpty()) {
		CString strTypeNames;
		// (r.gonet 2010-06-23) - PLID 39282 - Find the actual names of these types, plus the No Type type if the user selected it
		CString strQuery = FormatString(
			"SELECT '{No Type}' AS Name WHERE 0 IN (%s) "
			"UNION "
			"SELECT Name FROM AptTypeT WHERE ID IN (%s) ORDER BY Name "
			, strTypeIDs, strTypeIDs);
		_RecordsetPtr prs = CreateRecordset(GetRemoteData(), strQuery);
		while(!prs->eof) {
			CString strName = AdoFldString(prs, "Name");
			strTypeNames += strName + ", ";
			prs->MoveNext();
		}
		prs->Close();
		if(!strTypeNames.IsEmpty()) {
			strTypeNames = strTypeNames.Left(strTypeNames.GetLength() - 2);
			
			// (r.gonet 2010-06-23) - PLID 39282 - Add an 'and'
			int nIndex = strTypeNames.ReverseFind(',');
			if(nIndex >= 0) {
				CString str1, str2;
				str1 = strTypeNames.Left(nIndex);
				str2 = strTypeNames.Right(strTypeNames.GetLength() - (nIndex + 1));
				if(str1.ReverseFind(',') >= 0) {
					strTypeNames = str1 + ", and" + str2;
				} else {
					strTypeNames = str1 + " and" + str2;
				}
			}
		}
		strDescription += ", for appointment types " + strTypeNames;
	}

	// (r.gonet 2010-06-23) - PLID 39282 - Finally the resource names, which we'll also have to query for since we only have the IDs
	if(!strResourceIDs.IsEmpty()) {
		CString strResourceNames;
		// (r.gonet 2010-06-23) - PLID 39282 - Find the actual names of these Resources
		CString strQuery = FormatString("SELECT Item FROM ResourceT WHERE ID IN (%s) ORDER BY Item", strResourceIDs);
		_RecordsetPtr prs = CreateRecordset(GetRemoteData(), strQuery);
		while(!prs->eof) {
			CString strName = AdoFldString(prs, "Item");
			strResourceNames += strName + ", ";
			prs->MoveNext();
		}
		prs->Close();
		if(!strResourceNames.IsEmpty()) {
			strResourceNames = strResourceNames.Left(strResourceNames.GetLength() - 2);

			// (r.gonet 2010-06-23) - PLID 39282 - Add an 'and'
			int nIndex = strResourceNames.ReverseFind(',');
			if(nIndex >= 0) {
				CString str1, str2;
				str1 = strResourceNames.Left(nIndex);
				str2 = strResourceNames.Right(strResourceNames.GetLength() - (nIndex + 1));
				if(str1.ReverseFind(',') >= 0) {
					strResourceNames = str1 + ", and" + str2;
				} else {
					strResourceNames = str1 + " and" + str2;
				}
			}
		}
		strDescription += ", for resources " + strResourceNames;
	}

	strDescription = strDescription.TrimRight();
	strDescription += ".";

	return strDescription;
}

/////////////////////////////////////
// Appointment Reminder Exceptions //
/////////////////////////////////////

// (r.gonet 2010-06-23) - PLID 40575 - Construction of an empty exception
CApptReminderException::CApptReminderException()
: m_nID(0), m_dtStartDate(), m_dtStartTime(), m_dtEndDate(), m_dtEndTime(), m_bFilterOnWeekDays(FALSE), m_bmWeekDays(0)
{
	m_bAdded = FALSE;
	m_bDeleted = FALSE;
	m_bModified = FALSE;
	m_dtStartDate.SetStatus(COleDateTime::null);
	m_dtEndDate.SetStatus(COleDateTime::null);
	m_dtStartTime.SetStatus(COleDateTime::null);
	m_dtEndTime.SetStatus(COleDateTime::null);
}

// (r.gonet 2010-06-23) - PLID 40575 - Construction of a full exception
CApptReminderException::CApptReminderException(long nID, COleDateTime dtStart, COleDateTime dtStartTime, 
	COleDateTime dtEnd, COleDateTime dtEndTime, BOOL bFilterOnWeekDays, long bmWeekDays)
: m_nID(nID), m_dtStartDate(dtStart), m_dtStartTime(dtStartTime), m_dtEndDate(dtEnd), m_dtEndTime(dtEndTime), m_bFilterOnWeekDays(bFilterOnWeekDays), m_bmWeekDays(bmWeekDays)
{
	m_bAdded = FALSE;
	m_bDeleted = FALSE;
	m_bModified = FALSE;
}

// (r.gonet 2010-06-23) - PLID 40575
void CApptReminderException::SetID(long nID)
{
	// Don't track id changes
	m_nID = nID;
}

// (r.gonet 2010-06-23) - PLID 40575
void CApptReminderException::SetStartTime(COleDateTime dtStartTime)
{
	if(m_dtStartTime != dtStartTime) {
		m_bModified = TRUE;
	}
	m_dtStartTime = dtStartTime;
}

// (r.gonet 2010-06-23) - PLID 40575
void CApptReminderException::SetEndTime(COleDateTime dtEndTime)
{
	if(m_dtEndTime != dtEndTime) {
		m_bModified = TRUE;
	}
	m_dtEndTime = dtEndTime;
}

// (r.gonet 2010-06-23) - PLID 40575
void CApptReminderException::SetStartDate(COleDateTime dtStartDate)
{
	if(m_dtStartDate != dtStartDate) {
		m_bModified = TRUE;
	}
	m_dtStartDate = dtStartDate;
}

// (r.gonet 2010-06-23) - PLID 40575
void CApptReminderException::SetEndDate(COleDateTime dtEndDate)
{
	if(m_dtEndDate != dtEndDate) {
		m_bModified = TRUE;
	}
	m_dtEndDate = dtEndDate;
}

// (r.gonet 2010-06-23) - PLID 40575
long CApptReminderException::GetID()
{
	return m_nID;
}

// (r.gonet 2010-06-23) - PLID 40575
COleDateTime CApptReminderException::GetStartTime()
{
	return m_dtStartTime;
}

// (r.gonet 2010-06-23) - PLID 40575
COleDateTime CApptReminderException::GetEndTime()
{
	return m_dtEndTime;
}

// (r.gonet 2010-06-23) - PLID 40575
COleDateTime CApptReminderException::GetStartDate()
{
	return m_dtStartDate;
}

// (r.gonet 2010-06-23) - PLID 40575
COleDateTime CApptReminderException::GetEndDate()
{
	return m_dtEndDate;
}

// (r.gonet 2010-06-23) - PLID 40575 - Mark the item for insertion when saving to the database
void CApptReminderException::SetIsNew(BOOL bValue)
{
	if(m_bAdded != bValue) {
		m_bModified = TRUE;
	}
	m_bAdded = bValue;
}

// (r.gonet 2010-06-23) - PLID 40575 - Mark the item for deletion when saving to the database
void CApptReminderException::SetIsDeleted(BOOL bValue)
{
	if(m_bDeleted != bValue) {
		m_bModified = TRUE;
	}
	m_bDeleted = bValue;
}

// (r.gonet 2010-06-23) - PLID 40575 - Get whether the item will be deleted upon saving
BOOL CApptReminderException::GetIsDeleted()
{
	return m_bDeleted;
}

// (r.gonet 2010-06-23) - PLID 40575 - Get whether the item will be updated upon saving
BOOL CApptReminderException::GetIsModified()
{
	return m_bModified;
}

// (r.gonet 2010-06-23) - PLID 40575 - This name can get a little confusing, but I couldn't think of another one. 
// This means, iff the day is included in the exception, return true. Iff it is not in the exception, return false.
BOOL CApptReminderException::GetWeekday(CApptReminderException::EDayOfWeek dow)
{
	long result = m_bmWeekDays & dow;
	// We must calculate whether we are including this day in the exception from the bitmask
	if(dow == 0) {
		return (m_bmWeekDays == 0 ? TRUE : FALSE);
	} else if((m_bmWeekDays & dow) == dow) {
		return TRUE;
	}
	return FALSE;
}

// (r.gonet 2010-06-23) - PLID 40575 - This name can get a little confusing, but I couldn't think of another one. 
// This means, iff the day is included in the exception, return true. Iff it is not in the exception, return false.
void CApptReminderException::SetWeekday(CApptReminderException::EDayOfWeek dow, BOOL bValue)
{
	m_bModified = TRUE;

	// We must calculate whether we are including this day in the exception from the bitmask
	if(bValue) {
		m_bmWeekDays = m_bmWeekDays | dow;
	} else {
		m_bmWeekDays = m_bmWeekDays & ~dow;
	}
}

// (r.gonet 2010-06-23) - PLID 40575 - Sets the weekdays bitmask which stores whether or not we will send reminders for any given
//  weekday. The CApptReminderException::EDayOfWeek enumeration gives the bitmask values for 
//  each day. To specify multiple days, Bitwise OR the EDayOfWeek values together.
void CApptReminderException::SetWeekdays(long bmWeekdays)
{
	if(m_bmWeekDays != bmWeekdays) {
		m_bModified = TRUE;
	}
	m_bmWeekDays = bmWeekdays;
}

// (r.gonet 2010-12-13) - PLID 40575 - Sets whether to filter on weekdays at all for exceptions.
void CApptReminderException::SetFilterOnWeekDays(BOOL bValue)
{
	m_bModified = TRUE;
	m_bFilterOnWeekDays = bValue;
	if(!m_bFilterOnWeekDays) {
		m_bmWeekDays = 0;
	}
}

// (r.gonet 2010-12-13) - PLID 40575 - Gets whether to filter on weekdays at all for exceptions.
BOOL CApptReminderException::GetFilterOnWeekDays()
{
	return m_bFilterOnWeekDays;
}

// (r.gonet 2010-06-23) - PLID 40575 - Adds to an sql batch the statements to save this reminder exception
// (r.gonet 2010-12-13) - PLID 40575 - Updated the queries to be parameterized and added in the FilterOnWeekdays bit that clears up the confusing
//  filtering on days.
void CApptReminderException::AddToSqlBatch(CString &strSqlBatch, CNxParamSqlArray &aryParams)
{
	COleDateTime dtZero = GetDateTimeZero();
	if(m_bAdded) {
		if(!m_bDeleted) {
			// Add the exception to the database
			AddParamStatementToSqlBatch(strSqlBatch, aryParams, 
				"INSERT INTO ApptReminderExceptionsT (StartDate, StartTime, EndDate, EndTime, FilterOnWeekDays, WeekDays) \r\n"
				"VALUES ({VT_DATE}, {VT_DATE}, {VT_DATE}, {VT_DATE}, {BIT}, {INT});\r\n",
				(m_dtStartDate.GetStatus() != COleDateTime::valid ? g_cvarNull : COleVariant(m_dtStartDate)),
				(m_dtStartTime.GetStatus() != COleDateTime::valid ? g_cvarNull : COleVariant(m_dtStartTime)),
				(m_dtEndDate.GetStatus() != COleDateTime::valid ? g_cvarNull : COleVariant(m_dtEndDate)), 
				(m_dtEndTime.GetStatus() != COleDateTime::valid ? g_cvarNull : COleVariant(m_dtEndTime)),
				m_bFilterOnWeekDays,
				m_bmWeekDays);
		}
	} else if(m_bDeleted) {
		if(!m_bAdded) {
			// Delete the exception from the database
			AddParamStatementToSqlBatch(strSqlBatch, aryParams,
				"DELETE FROM ApptReminderExceptionsT WHERE ID = {INT};\r\n",
				m_nID);
		}
	} else if(m_bModified) {
		// Update the exception in the database if it has been modified
		AddParamStatementToSqlBatch(strSqlBatch, aryParams,
			"UPDATE ApptReminderExceptionsT SET StartDate = {VT_DATE}, StartTime = {VT_DATE}, EndDate = {VT_DATE}, EndTime = {VT_DATE}, FilterOnWeekDays = {INT}, WeekDays = {INT} "
			"WHERE ID = {INT}\r\n ",
			(m_dtStartDate.GetStatus() != COleDateTime::valid ? g_cvarNull : COleVariant(m_dtStartDate)),
			(m_dtStartTime.GetStatus() != COleDateTime::valid ? g_cvarNull : COleVariant(m_dtStartTime)),
			(m_dtEndDate.GetStatus() != COleDateTime::valid ? g_cvarNull : COleVariant(m_dtEndDate)), 
			(m_dtEndTime.GetStatus() != COleDateTime::valid ? g_cvarNull : COleVariant(m_dtEndTime)),
			m_bFilterOnWeekDays,
			m_bmWeekDays, 
			m_nID);
	}
}

// (r.gonet 2010-06-23) - PLID 40575 - Return a string representing the exception in a human readable format
CString CApptReminderException::GetDescription()
{
	CString strDescriptionHeader = "Do not remind ";
	CString strDescriptionTrailer = ".";
	CString strDescription = "";
	CString strDateRange = "";

	// Add in the date range to the description
	if(m_dtStartDate.GetStatus() == COleDateTime::valid) {
		strDateRange.Format("%s to %s", 
			FormatDateTimeForInterface(m_dtStartDate, NULL, dtoDate),
			FormatDateTimeForInterface(m_dtEndDate, NULL, dtoDate));
	}

	// Add in the time range
	CString strTimeRange = "";
	if(m_dtStartTime.GetStatus() == COleDateTime::valid) {
		strTimeRange.Format("%s to %s", 
			FormatDateTimeForInterface(m_dtStartTime, NULL, dtoTime),
			FormatDateTimeForInterface(m_dtEndTime, NULL, dtoTime));
	}

	// Add in the days of the week
	CString strWeekDays = "";
	if(m_bFilterOnWeekDays)
	{
		if(GetWeekday(CApptReminderException::dowAllDays)) {
			strWeekDays += "Any Day";
		} else {
			if(GetWeekday(CApptReminderException::dowSunday)) {
				strWeekDays += "Sunday, ";
			}
			if(GetWeekday(CApptReminderException::dowMonday)) {
				strWeekDays += "Monday, ";
			}
			if(GetWeekday(CApptReminderException::dowTuesday)) {
				strWeekDays += "Tuesday, ";
			}
			if(GetWeekday(CApptReminderException::dowWednesday)) {
				strWeekDays += "Wednesday, ";
			}
			if(GetWeekday(CApptReminderException::dowThursday)) {
				strWeekDays += "Thursday, ";
			}
			if(GetWeekday(CApptReminderException::dowFriday)) {
				strWeekDays += "Friday, ";
			}
			if(GetWeekday(CApptReminderException::dowSaturday)) {
				strWeekDays += "Saturday, ";
			}
			
			// Delete the last comma
			if(!strWeekDays.IsEmpty()) {
				strWeekDays = strWeekDays.Left(strWeekDays.GetLength() - 2);
			}

			// Add an 'and'
			int nIndex = strWeekDays.ReverseFind(',');
			if(nIndex >= 0) {
				CString str1, str2;
				str1 = strWeekDays.Left(nIndex);
				str2 = strWeekDays.Right(strWeekDays.GetLength() - (nIndex + 1));
				if(str1.ReverseFind(',') >= 0) {
					strWeekDays = str1 + ", and" + str2;
				} else {
					strWeekDays = str1 + " and" + str2;
				}
			}
		}
	}

	// Attach each piece of the description together
	// I actually bumped the weekdays up in the chain because the sentence reads better.
	if(!strWeekDays.IsEmpty()) {
		strDescription += FormatString("on %s", strWeekDays);
	}
	if(!strDateRange.IsEmpty()) {
		if(!strDescription.IsEmpty()) {
			strDescription += ", ";
		}
		strDescription += FormatString("during %s", strDateRange);
	}
	if(!strTimeRange.IsEmpty()) {
		if(!strDescription.IsEmpty()) {
			strDescription += ", ";
		}
		strDescription += FormatString("from %s", strTimeRange);
	}
	return strDescriptionHeader + strDescription + strDescriptionTrailer;
}
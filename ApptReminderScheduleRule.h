#pragma once

// (r.gonet 2010-06-23) - PLID 40574 - Encapsulate a database appt reminder rule for information hiding, tracking, and auditing
class CApptReminderScheduleRule
{
public:
	// (z.manning 2011-05-16 11:25) - PLID 43708 - Added an enum as we now have different methods of sending
	// (This enum is also in NxReminderClient.)
	// SAVED TO DATA, DO NOT CHANGE
	enum ESendType
	{
		stHoursBeforeAppt = 1,
		stSpecificTime = 2,
	};

// Private, not protected to elimininate dependenencies with any descendent classes
private:
	// Fields
	long m_nID;
	COleDateTimeSpan m_tsAdvanceNotice;
	BOOL m_bIgnoreExceptions;
	// (r.gonet 2010-09-02) - PLID 39282 - Reminder rules can now be specific for an apt type and resource
	CArray<long,long> m_arynApptTypes;
	CArray<long,long> m_arynApptResources;
	// (r.gonet 2010-06-23) - PLID 39282
	BOOL m_bFilterOnAllTypes;
	BOOL m_bFilterOnAllResources;
	ESendType m_eSendType; // (z.manning 2011-05-16 11:30) - PLID 43708
	COleDateTime m_dtSendTime; // (z.manning 2011-05-16 11:31) - PLID 43708
	short m_nSendTimeDaysBefore; // (z.manning 2011-05-16 11:57) - PLID 43708

	// Modification status
	BOOL m_bModified;
	BOOL m_bDeleted;
	BOOL m_bAdded;
public:
	// Construction and Destruction
	CApptReminderScheduleRule();
	// (z.manning 2011-05-16 11:40) - PLID 43708 - Added more params to support different send types
	CApptReminderScheduleRule(long nID, COleDateTimeSpan tsAdvanceNotice, ESendType eSendType, COleDateTime dtSendTime, short nSendTimeDaysBefore, BOOL bIgnoreExceptions, 
		BOOL bAllTypes, BOOL bAllResources, CArray<long, long> &arynTypes, CArray<long, long> &arynResources);

	// Implementation
	void SetID(long nID);
	void SetAdvanceNotice(COleDateTimeSpan tsAdvanceNotice);
	void SetIgnoreExceptions(BOOL bIgnoreExceptions);
	// (r.gonet 2010-09-02) - PLID 39282 - Reminder rules can now be specific for types an resources
	void SetAppointmentTypes(CArray<long, long> &arynTypes);
	void SetAppointmentResources(CArray<long, long> &arynResources);
	void AddAppointmentType(long nTypeID);
	void AddAppointmentResource(long nResourceID);
	void RemoveAppointmentType(long nTypeID);
	void RemoveAppointmentResource(long nResourceID);
	void RemoveAllAppointmentTypes();
	void RemoveAllAppointmentResources();
	// (r.gonet 2010-06-23) - PLID 39282
	void SetFilterOnAllTypes(BOOL bAll);
	void SetFilterOnAllResources(BOOL bAll);
	void SetSendType(const ESendType eSendType);
	void SetSendTime(const COleDateTime dtSendTime);
	void SetSendTimeDaysBefore(const short nSendTimeDaysBefore);

	long GetID();
	COleDateTimeSpan GetAdvanceNotice();
	BOOL GetIgnoreExceptions();
	// (r.gonet 2010-09-02) - PLID 39282 - Reminder rules can now be specific for types and resources
	void GetAppointmentTypes(OUT CArray<long, long> &arynTypes);
	void GetAppointmentResources(OUT CArray<long, long> &arynResources);
	BOOL GetFilterOnAllTypes();
	BOOL GetFilterOnAllResources();
	CString GetDescription();
	ESendType GetSendType();
	COleDateTime GetSendTime();
	short GetSendTimeDaysBefore();

	// Saving	
	void SetIsNew(BOOL bValue);
	void SetIsDeleted(BOOL bValue);
	BOOL GetIsDeleted();
	BOOL GetIsModified();
	void AddAppointmentTypesInsertQuery(CString &strSqlBatch);
	void AddAppointmentResourcesInsertQuery(CString &strSqlBatch);
	void AddToSqlBatch(CString &strSQLBatch, CNxParamSqlArray &aryParams);
};

// (r.gonet 2010-06-23) - PLID 40575 - Encapsulates a database appt reminder schedule exception for information hiding, tracking, and auditing
class CApptReminderException
{
public:
	// (r.gonet 2010-06-23) - PLID 40575 - Provide an enumeration for setting the weekdays bitmask
	enum EDayOfWeek
	{
		dowNoDays = 0,
		dowSunday = 1,
		dowMonday = 1 << 1,
		dowTuesday = 1 << 2,
		dowWednesday = 1 << 3,
		dowThursday = 1 << 4,
		dowFriday = 1 << 5,
		dowSaturday = 1 << 6,
		dowWeekdays = (dowMonday | dowTuesday | dowWednesday | dowThursday | dowFriday),
		dowWeekends = (dowSaturday | dowSunday),
		dowAllDays = (dowWeekdays | dowWeekends)
	};

private:
	long m_nID;
	COleDateTime m_dtStartTime;
	COleDateTime m_dtEndTime;
	COleDateTime m_dtStartDate;
	COleDateTime m_dtEndDate;
	BOOL m_bTimesSet;
	BOOL m_bDatesSet;
	BOOL m_bFilterOnWeekDays;
	long m_bmWeekDays;

	// Modification status
	BOOL m_bModified;
	BOOL m_bDeleted;
	BOOL m_bAdded;
public:
	CApptReminderException();
	CApptReminderException(long nID, COleDateTime dtStart, COleDateTime dtStartTime, 
		COleDateTime dtEnd, COleDateTime dtEndTime, BOOL bFilterOnWeekDays, long bmWeekDays);

	void SetID(long nID);
	void SetStartTime(COleDateTime dtStartTime);
	void SetEndTime(COleDateTime dtEndTime);
	void SetStartDate(COleDateTime dtStartDate);
	void SetEndDate(COleDateTime dtEndDate);
	void SetFilterOnWeekDays(BOOL bValue);
	void SetWeekday(EDayOfWeek dow, BOOL bValue);
	void SetWeekdays(long bmWeekdays);

	long GetID();
	COleDateTime GetStartTime();
	COleDateTime GetEndTime();
	COleDateTime GetStartDate();
	COleDateTime GetEndDate();
	BOOL GetFilterOnWeekDays();
	BOOL GetWeekday(EDayOfWeek dow);
	CString GetDescription();

	// Saving
	void SetIsNew(BOOL bValue);
	void SetIsDeleted(BOOL bValue);
	BOOL GetIsDeleted();
	BOOL GetIsModified();
	void AddToSqlBatch(CString &strSqlBatch, CNxParamSqlArray &aryParams);
};

// (r.gonet 2010-06-23) - PLID 39012 - Created. All comments, unless marked otherwise, are for this PLID.

#pragma once

#include "ApptReminderScheduleRule.h"

class CApptReminderSchedule
{
public:
	// (r.gonet 2015-11-16 02:33) - PLID 67587 - Added enumeration for the sending limit options,
	// which control how NexReminderClient determines if a reminder has already been sent.
	// Saved to data.
	enum ESendingLimitOptions {
		Invalid = 0,
		PerAppointment = 1,
		PerRule = 2,
		PerDay = 4,
		PerPatient = 8,
		PerAppointmentDate = 16,
		PerResource = 32,
		PerLocation = 64,
		// Bitmask. Values need to be powers of 2.
	};
private:

	// All rules for when to send the appointment reminders
	CArray<CApptReminderScheduleRule *, CApptReminderScheduleRule *> m_arRules;
	// The next new rule ID is generated from this down-counter
	long m_nNewRuleID;
	
	// All rules for when NOT to send the appointment reminders
	CArray<CApptReminderException *, CApptReminderException *> m_arExceptions;
	// The next new exception ID is generated from this down counter
	long m_nNewExceptionID;

	// (r.gonet 2015-11-16 02:33) - PLID 67587 - Bitmask of ESendingLimitOptions, which control how 
	// NexReminderClient determines if a reminder has already been sent.
	long m_nSendingLimitOptions;
private:
	CApptReminderScheduleRule * GetRule(int nIndex);
	CApptReminderException * GetException(int nIndex);
	void LoadRules();
	void LoadExceptions();
public:
	// Constructors
	CApptReminderSchedule();
	virtual ~CApptReminderSchedule();
	
	// Implementation
	void AddRule(CApptReminderScheduleRule *arrRule);
	void RemoveRule(long nID);

	void AddException(CApptReminderException *arrException);
	void RemoveException(long nID);

	// (r.gonet 2015-11-16 02:33) - PLID 67587 - Sets the bitmask of ESendingLimitOptions, 
	// which control how NexReminderClient determines if a reminder has already been sent.
	void SetSendingLimitOptions(long nSendingLimitOptions);
	// (r.gonet 2015-11-16 02:33) - PLID 67587 - Gets the bitmask of ESendingLimitOptions, 
	// which control how NexReminderClient determines if a reminder has already been sent.
	long GetSendingLimitOptions() const;

	int GetRuleCount();
	int GetExceptionCount();
	CApptReminderScheduleRule * GetRuleById(int nID);
	CApptReminderException * GetExceptionById(int nID);
	long GetRuleStartPosition();
	long GetExceptionStartPosition();
	BOOL GetNextRule(CApptReminderScheduleRule OUT * &pRule, long &nPosition);
	BOOL GetNextException(CApptReminderException OUT * &pException, long &nPosition);
	void ClearRules();
	void ClearExceptions();

	BOOL GetModified();
	
	// Saving
	void Save();

	// Loading
	// (r.gonet 2015-11-16 02:33) - PLID 67587 - Loads the schedule.
	void Load();
};

#ifndef __COMMONSCHEDUTILS_H__
#define __COMMONSCHEDUTILS_H__

#pragma once

#include "CommonFFAUtils.h"

enum EProcessRuleResult {
	prAppointmentInvalid,
	prAppointmentValid
};

enum ERuleWarningsResult {
	rwNoRulesApplied = 0,
	rwHasWarnings = 1,
	rwIsPrevented = 2,
	rwNeededPermission = 4,
};

enum EProcessAlarmResult {
	paAppointmentInvalid,
	paAppointmentValid
};

long AppointmentReadRules(long nPatientID, long nResourceID, long nLocationID, const COleDateTime &dtDate, 
							  const COleDateTime &dtStartTime, const COleDateTime &dtEndTime, 
							  long nAptType, long nAptPurpose, long nIgnoreApptID, BOOL bIsFFA = FALSE, OUT CString *pstrFatalWarnings = NULL, OUT CString *pstrNonfatalWarnings = NULL,
							  ADODB::_RecordsetPtr prsRuleSet = NULL);

BOOL AppointmentValidateByRules(ADODB::_RecordsetPtr pAppt, long bIgnoreLinkedAppts);
BOOL AppointmentValidateByRules(ADODB::_RecordsetPtr pAppt, OUT CString *pstrFatalWarnings, OUT CString *pstrNonfatalWarnings, OUT long *pnRuleWarningResult);
BOOL AppointmentValidateByRules(long nPatientID, long nResourceID, long nLocationID, const COleDateTime &dtDate, 
							  const COleDateTime &dtStartTime, const COleDateTime &dtEndTime, 
							  long nAptType, long nAptPurpose, long nIgnoreApptID, BOOL bSilent = FALSE, OUT CString *pstrFatalWarnings = NULL, OUT CString *pstrNonfatalWarnings = NULL, OUT long *pnRuleWarningResult = NULL,
							  ADODB::_RecordsetPtr prsRuleSet = NULL);
// (a.walling 2010-06-14 16:50) - PLID 23560 - Resource Sets
//TES 2/25/2011 - PLID 42523 - Changed nLineItemOverrideID to default to -2.  -2 means that no precision template was calculated, whereas -1
// means that precision templates were checked, and the appointment isn't on one.  This is used when checking the "Schedule Outside Of Templates"
// permission; -2 means that the appointment might be on a precision template, -1 means it definitely isn't, and of course a positive
// value means that it definitely is.
BOOL AppointmentValidateByRules(long nPatientID, const CDWordArray& adwResourceID, long nLocationID, const COleDateTime &dtDate, 
							  const COleDateTime &dtStartTime, const COleDateTime &dtEndTime, 
							  long nAptType, const CDWordArray& adwAptPurpose, long nIgnoreApptID, BOOL bIgnoreLinkedAppts, BOOL bIgnoreNonLinkedAppts, long nLineItemOverrideID = -2, BOOL bSilent = FALSE, int nRuleTolerance = -1);

BOOL AppointmentValidateDoubleBooking(COleDateTime dtStart, COleDateTime dtEnd, CDWordArray& adwResources, long nIgnoreApptID = -1);

BOOL AppointmentValidateByPermissions(ADODB::_RecordsetPtr pAppt);
BOOL AppointmentValidateByPermissions(long nPatientID, const CDWordArray& adwResourceID, long nLocationID, const COleDateTime &dtDate, 
							  const COleDateTime &dtStartTime, const COleDateTime &dtEndTime, 
							  long nAptType, const CDWordArray& adwAptPurpose, long nIgnoreApptID);

BOOL AppointmentValidateByAlarms(ADODB::_RecordsetPtr pAppt, long nResIDToIgnore = -1);
BOOL AppointmentValidateByAlarms(ADODB::_RecordsetPtr pAppt, OUT CString *pstrFatalWarnings, OUT CString *pstrNonfatalWarnings, OUT long *pnRuleWarningResult, long nResIDToIgnore = -1);
BOOL AppointmentValidateByAlarms(long nPatientID, const COleDateTime &dtDate, const COleDateTime &dtTime, long nAptType, const CDWordArray& adwAptPurpose, const CDWordArray& adwAptResource, long nIgnoreApptID1, long nIgnoreApptID2 = -1);
// (j.jones 2009-08-12 12:52) - PLID 25230 - removed bSilent, because it was always true
BOOL AppointmentValidateByAlarms(long nPatientID, const COleDateTime &dtDate, long nAptType, long nAptPurpose, const CDWordArray& adwAptResource, long nIgnoreApptID1, OUT CString *pstrFatalWarnings = NULL, OUT CString *pstrNonfatalWarnings = NULL, OUT long *pnAlarmWarningResult = NULL, long nResIDToIgnore = -1);

BOOL AppointmentValidateByDuration(ADODB::_RecordsetPtr pAppt);
BOOL AppointmentValidateByDuration(const COleDateTime& dtStart, const COleDateTime& dtEnd, long nAptTypeID,
								   const CDWordArray& adwResource, const CDWordArray& adwPurpose);

// (j.jones 2014-12-04 15:01) - PLID 64179 - tracks a scheduler mix rule detail
class SchedulerMixRuleDetail {

public:
	long nID;
	long nMaxAppts;

	SchedulerMixRuleDetail() {
		nID = -1;
		nMaxAppts = -1;
	}
};

// (j.jones 2014-11-26 09:36) - PLID 64179 - tracks a scheduler mix rule ID and name
// (j.jones 2014-12-04 09:18) - PLID 64119 - added Color
class SchedulerMixRule {

public:
	long nID;
	CString strName;
	long nColor;	//-1 means no color is specified
	std::vector<SchedulerMixRuleDetail> aryRuleDetails;

	SchedulerMixRule() {
		nID = -1;
		strName = "";
		nColor = -1;
	}
};

// (j.jones 2014-11-25 08:55) - PLID 64178 - Added validation for scheduler mix rules.
// The fields are the slot the user is trying to schedule into, not the slot where the
// appointment currently exists.
// (j.jones 2014-11-26 16:42) - PLID 64272 - the callers have an optional overriddenMixRules returned,
// this is only filled if the appt. exceeded a rule and a user overrode it, the caller needs to save this info
bool AppointmentValidateByMixRules(CWnd *pParentWnd, ADODB::_RecordsetPtr pAppt, OUT std::vector<SchedulerMixRule> &overriddenMixRules, OUT SelectedFFASlotPtr &pSelectedFFASlot, OPTIONAL long nAppointmentIDToIgnore = -1, OPTIONAL long nAppointmentID2ToIgnore = -1);
bool AppointmentValidateByMixRules(CWnd *pParentWnd, long nPatientID, const COleDateTime& dtApptDate, long nLocationID, long nInsuredPartyID, long nAptTypeID,
	const CDWordArray& adwResource, const CDWordArray& adwPurpose, OUT std::vector<SchedulerMixRule> &overriddenMixRules, OUT SelectedFFASlotPtr &pSelectedFFASlot, OPTIONAL long nAppointmentIDToIgnore = -1, OPTIONAL long nAppointmentID2ToIgnore = -1);
bool AppointmentValidateByMixRules(const COleDateTime& dtApptDate, long nLocationID, long nInsuredPartyID, long nAptTypeID,
	const CDWordArray& adwResource, const CDWordArray& adwPurpose, OUT std::vector<SchedulerMixRule> &exceededMixRules, OPTIONAL long nAppointmentIDToIgnore = -1, OPTIONAL long nAppointmentID2ToIgnore = -1);

// (j.jones 2014-12-05 10:45) - PLID 64274 - Given an ID of an appt. known to have an entry in
// AppointmentMixRuleOverridesT, this function will re-check every overridden rule on the appt.
// If that rule is still exceeded on the appt. date, this returns true. If the rule is no longer
// exceeded, or if the appointment is cancelled, this returns false.
bool IsAppointmentMixRuleOverrideCurrent(long nAppointmentID);

// (j.jones 2014-12-05 13:58) - PLID 64274 - Finds all appointments in the provided date range
// that have entries in AppointmentMixRuleOverridesT, and revalidates the mix rules for that
// appointment date to confirm whether the rule is still exceeded. If it isn't, then the
// entry in AppointmentMixRuleOverridesT is meaningless. Cancelled appointments are ignored.
// Returns a list of Appointment IDs with overrides that still apply.
std::vector<long> GetOverrideAppointmentsInDateRange(COleDateTime dtStartDate, COleDateTime dtEndDate);

// (j.jones 2014-12-04 15:14) - PLID 64119 - Returns -1 if no mix rule color applies.
// Otherwise returns the color of the applicable mix rule. If multiple rules apply,
// this returns the highest color value, for consistency. This decision is arbitrary.
long GetAppointmentMixRuleColor(long nAppointmentID);
long GetAppointmentMixRuleColor(const COleDateTime& dtApptDate, long nLocationID, long nInsuredPartyID, long nAptTypeID,
	const CDWordArray& adwResource, const CDWordArray& adwPurpose);

long AppointmentReadAlarms(long nPatientID, const COleDateTime &dtDate, long nAptType, long nAptPurpose, const CDWordArray& adwAptResource, long nIgnoreApptID1, OUT CString *pstrFatalWarnings = NULL, OUT CString *pstrNonfatalWarnings = NULL, long nIgnoreApptID2 = -1);
EProcessAlarmResult ProcessAlarm(long nAlarmID, long nPatientID, const COleDateTime &dtDate, long nAptType, long nAptPurpose, const CDWordArray& adwAptResource, long nIgnoreApptID1, CString &strConflictingAppts, long nIgnoreApptID2 = -1);

BOOL MatchAppointmentToAlarmDetails(long nAptType, long nAptPurpose, long nAlarmID);
CString GetAppointmentAlarmConflictInfo(long nConflictingApptID);

BOOL CheckWarnProcedureCredentials(const CDWordArray& adwResourceID, const CDWordArray& adwAptPurpose);

// (j.dinatale 2012-11-21 10:46) - PLID 54162 - need to determine if info about our appt makes it an event
bool IsAppointmentEvent(const COleDateTime& dtDate, const COleDateTime& dtStartTime, const COleDateTime& dtEndTime);

// CAH 5/16/2003 - COleDateTimeSpan doesn't work right if you subtract a time in the future
// from a time in the past...at least right by my idea of how it should work, anyway. This
// function works around that defect to give us the total number of minutes that dt1 and dt2
// are different.
long GetSafeTotalMinuteDiff(const COleDateTime& dt1, const COleDateTime& dt2);

bool AttemptWarnForInsAuth(long nPatientID, COleDateTime dtApptDate);

void TruncateSchedulerWarning(CString& strText);

void LoadResourceIDStringIntoArray(CString strResourceIDs, CDWordArray& adwIDs);
void LoadResourceIDStringIntoMap(CString strResourceIDs, CArray<LPARAM, LPARAM>& naryResourceIDs ,CMap<long, long, long, long>& mapIDs); // (r.farnworth 2015-05-28 08:33)
void LoadPurposeIDStringIntoArray(CString strPurposeIDs, CDWordArray& adwIDs);

// (z.manning, 01/23/2007) - PLID 24392 - Moved these here from the CTemplateEntryDlg class.
void RemoveTemplate(long nTemplateID);
void DeleteTemplatePermissions(long nTemplateID);
// (z.manning 2011-12-09 09:49) - PLID 46906 - Added a function to delete permissions for given rule IDs
void DeleteRulePermissions(CArray<long,long> *parynRuleIDs);

//TES 6/19/2010 - PLID 5888 - Deletes a Resource Availability template
void RemoveResourceAvailTemplate(long nTemplateID);

// (a.walling 2010-06-22 09:23) - PLID 23560 - Check resource sets
bool CheckExistingApptResourceSetConflicts(int nAptTypeID, int nFromResID, const CDWordArray& dwaResourceIDs, OUT CString& strMessage);
bool CheckNewApptResourceSetConflicts(int nAptTypeID, const CDWordArray& dwaPurposeIDs, const CDWordArray& dwaResourceIDs, OUT CString& strMessage);
bool CheckResourceSetConflicts(ADODB::_RecordsetPtr prs, const CDWordArray& dwaResourceIDs, OUT CString& strMessage, bool bIncludeSaveText);

//TES 12/12/2014 - PLID 64120 - Determines whether this database has any ScheduleMixRulesT records in it
bool DoesDatabaseHaveMixRules();
//TES 2/11/2015 - PLID 64120 - Call this when first creating a mix rule
void SetDatabaseHasMixRules();
// (j.jones 2014-12-11 15:07) - PLID 64178 - moved CComplexTimeRange to NxSchedulerDlg.h

// (j.jones 2014-12-02 12:00) - PLID 64272 - tracks if a scheduler mix rule has been overridden for an appointment
void TrackAppointmentMixRuleOverride(long nAppointmentID, const IN std::vector<SchedulerMixRule> &overriddenMixRules);

namespace Nx
{
	namespace Scheduler
	{
		// (a.walling 2015-01-05 14:23) - PLID 64518 - Drop target support
		struct TargetInfo
		{
			CPoint ptScreen = CPoint(0, 0);
			COleDateTime time = g_cdtNull;
			COleDateTime date = g_cdtNull;			
			long resourceID = -1;
			long day = 0;
			bool isEvent = false;
		};
	}
}

#endif
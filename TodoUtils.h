// (c.haag 2008-06-09 08:38) - PLID 30321 - Initial implementation
//TES 10/15/2008 - PLID 31646 - Copied GlobalTodoUtils.h to TodoUtils.h, as many of the functions were not actually "Global"
// in the sense of being shareable with other projects.  Where applicable, the functions in here just call their 
// equivalents in GlobalTodoUtils.

#pragma once

#include "GlobalTodoUtils.h"

// (c.haag 2008-06-09 11:20) - PLID 30321 - Create a todo alarm for a single assignee
// (c.haag 2008-07-10 10:31) - PLID 30648 - Added EnteredBy
// (c.haag 2008-07-10 14:58) - PLID 30674 - Added nSourceActionID
// (z.manning 2009-02-26 15:07) - PLID 33141 - Added SourceDataGroupID
// (z.manning 2010-02-25 10:42) - PLID 37532 - SourceDetailImageStampID
long TodoCreate(const COleDateTime& dtRemind, COleDateTime dtDeadline, long nAssignTo, const CString& strNotes, const CString& strTask, long nRegardingID, TodoType RegardingType,
				long nPersonID = -1, long nLocationID = -1, TodoPriority Priority = ttpLow, long nCategoryID = -1, const COleDateTime& dtDone = (DATE)0, long nEnteredBy = -1,
				long nSourceActionID = -1, long nSourceDataGroupID = -1, long nSourceDetailImageStampID = -1);

// (c.haag 2008-06-09 11:20) - PLID 30321 - Create a todo alarm for multiple assignees
// (c.haag 2008-07-10 10:31) - PLID 30648 - Added EnteredBy
// (c.haag 2008-07-10 14:58) - PLID 30674 - Added nSourceActionID
// (z.manning 2009-02-26 15:07) - PLID 33141 - Added SourceDataGroupID
// (z.manning 2010-02-25 10:43) - PLID 37532 - SourceDetailImageStampID
long TodoCreate(const COleDateTime& dtRemind, COleDateTime dtDeadline, const CArray<long,long>& anAssignTo, const CString& strNotes, const CString& strTask, long nRegardingID, TodoType RegardingType,
				long nPersonID = -1, long nLocationID = -1, TodoPriority Priority = ttpLow, long nCategoryID = -1, const COleDateTime& dtDone = (DATE)0, long nEnteredBy = -1,
				long nSourceActionID = -1, long nSourceDataGroupID = -1, long nSourceDetailImageStampID = -1);

// (c.haag 2008-06-09 12:14) - PLID 30321 - Reassigns all todos from one user to another
void TodoTransferAssignTo(long nCurrentUserID, long nNewUserID, const CString& strTaskFilter = "");

// (c.haag 2008-06-09 17:08) - PLID 30328 - Deletes a single todo alarm
// (c.haag 2008-07-14 15:25) - PLID 30607 - Added bFromEMR
void TodoDelete(long nTaskID, BOOL bFromEMR = FALSE);

// (c.haag 2008-06-09 17:08) - PLID 30328 - Deletes a batch of todo alarms given a filter that
// can operate in the form SELECT ID FROM TodoList WHERE (strWhere)
void TodoDelete(const CString& strWhere);

// (c.haag 2008-06-23 13:02) - PLID 30471 - This is just like TodoTransferAssignTo except it operates
// on EMR todo actions (EMRActionsTodoAssignToT) 
void TodoTransferEmrActionAssignTo(long nCurrentUserID, long nNewUserID);

// (c.haag 2008-06-24 12:41) - PLID 17244 - This function returns an SQL query string that lets the caller
// update EMR detail-related todo alarm notes. These notes are formatted with the EMN description, then the
// detail name, followed by the actual note. The query will update those notes with the latest descriptions
// and names only if the note is properly formatted.
// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
CSqlFragment TodoGetEmrDetailAlarmUpdateQ(long nEMNID, long nEMNDetailID);

// (c.haag 2008-07-11 10:41) - PLID 30550 - This function returns an SQL query string that lets
// the caller update EMN-related todo alarm notes. These notes are formatted with the EMN description
// followed by the actual note. The query will update those notes with the latest descriptions
// and names only if the note is properly formatted.
// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
CSqlFragment TodoGetEmnAlarmUpdateQ(long nEMNID);

// (c.haag 2008-06-30 10:41) - PLID 11599 - Change the assign-to's of a todo alarm
void TodoChangeAssignTo(long nTaskID, CArray<long,long>& anAssignIDs);

// (c.haag 2008-07-03 11:52) - PLID 30615 - Called when the caller wants to change todo alarm assignees. Returns TRUE
// if write permissions check out
BOOL TodoCheckItemReassignmentPermissions(CArray<long,long>& anOldAssignTo, CArray<long,long>& anNewAssignTo);

//TES 10/10/2008 - PLID 31646 - Create a ToDo Alarm due to one or more changed lab results.
// Pass in the patient, an array of strings describing what changed, and whether the results were changed by the HL7
// Link (as opposed to just being manually changed).  Returns the ID of the created ToDo Alarm.
// (z.manning 2010-05-12 14:57) - PLID 37405 - Added nLabID
//TES 8/6/2013 - PLID 51147 - Added Priority
long TodoCreateForLab(long nPatientID, const long nLabID, const CStringArray &saChangedResults, bool bFromHL7, TodoPriority Priority, long nEnteredBy = -1);

// (c.haag 2008-07-03 12:25) - PLID 30615 - This function encapsulates security checking, and saves
// us from having to repeat a lot of code.
// (z.manning 2009-09-11 18:05) - PLID 32048 - Moved this here and made it global
BOOL CheckAssignToPermissions(CArray<long,long> &anAssignTo, ESecurityPermissionType type);
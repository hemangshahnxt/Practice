// (c.haag 2008-06-09 08:38) - PLID 30321 - Initial implementation
//TES 10/15/2008 - PLID 31646 - Copied GlobalTodoUtils.cpp to TodoUtils.cpp, as many of the functions were not actually "Global"
// in the sense of being shareable with other projects.  Where applicable, the functions in here just call their 
// equivalents in GlobalTodoUtils.

#include "stdafx.h"
#include "TodoUtils.h"
#include "DateTimeUtils.h"

using namespace ADODB;

// (c.haag 2008-06-09 11:20) - PLID 30321 - Create a todo alarm for a single assignee
// (c.haag 2008-07-10 10:31) - PLID 30648 - Added EnteredBy
// (c.haag 2008-07-10 14:58) - PLID 30674 - Added nSourceActionID
// (z.manning 2009-02-26 15:06) - PLID 33141 - Added SourceDataGroupID
// (z.manning 2010-02-25 10:49) - PLID 37532 - SourceDetailImageStampID
long TodoCreate(const COleDateTime& dtRemind, COleDateTime dtDeadline, long nAssignTo, const CString& strNotes, const CString& strTask, long nRegardingID, TodoType RegardingType,
				long nPersonID /* = -1*/, long nLocationID /* = -1 */, TodoPriority Priority /*= ttpLow */, long nCategoryID /*= -1*/, const COleDateTime& dtDone /*= (DATE)0 */,
				long nEnteredBy /* = -1*/, long nSourceActionID /* = -1*/, long nSourceDataGroupID /* = -1 */, long nSourceDetailImageStampID /* = -1 */)
{
	if(nEnteredBy == -1) {
		nEnteredBy = GetCurrentUserID();
	}
	return TodoCreate(GetRemoteData(), dtRemind, dtDeadline, nAssignTo, strNotes, strTask, nRegardingID, RegardingType, nPersonID,
		nLocationID, Priority, nCategoryID, dtDone, nEnteredBy, nSourceActionID, nSourceDataGroupID, nSourceDetailImageStampID);
}

// (c.haag 2008-06-09 11:20) - PLID 30321 - Create a todo alarm for multiple assignees
// (c.haag 2008-07-10 10:31) - PLID 30648 - Added EnteredBy
// (c.haag 2008-07-10 14:58) - PLID 30674 - Added nSourceActionID
// (z.manning 2009-02-26 15:06) - PLID 33141 - Added SourceDataGroupID
// (z.manning 2010-02-25 10:49) - PLID 37532 - SourceDetailImageStampID
long TodoCreate(const COleDateTime& dtRemind, COleDateTime dtDeadline, const CArray<long,long>& anAssignTo, const CString& strNotes, const CString& strTask, long nRegardingID, TodoType RegardingType,
				long nPersonID /* = -1*/, long nLocationID /* = -1 */, TodoPriority Priority /*= ttpLow */, long nCategoryID /*= -1*/, const COleDateTime& dtDone /*= (DATE)0 */,
				long nEnteredBy /* = -1*/, long nSourceActionID /* = -1 */, long nSourceDataGroupID /* = -1 */, long nSourceDetailImageStampID /* = -1 */)
{
	if(nEnteredBy == -1) {
		nEnteredBy = GetCurrentUserID();
	}
	return TodoCreate(GetRemoteData(), dtRemind, dtDeadline, anAssignTo, strNotes, strTask, nRegardingID, RegardingType, nPersonID,
		nLocationID, Priority, nCategoryID, dtDone, nEnteredBy, nSourceActionID, nSourceDataGroupID, nSourceDetailImageStampID);
}

// (c.haag 2008-06-09 12:14) - PLID 30321 - Reassigns all todos from one user to another
void TodoTransferAssignTo(long nCurrentUserID, long nNewUserID, const CString& strTaskFilter /*= ""*/)
{
	if (nCurrentUserID == nNewUserID) {
		ThrowNxException("Current and new user cannot be the same.");		
	}

	CString strFilter;
	if (!strTaskFilter.IsEmpty() && strTaskFilter != "(1=1)") {
		strFilter = FormatString("AND TaskID IN (SELECT TaskID FROM TodoList WHERE (%s))", strTaskFilter);
	}

	// First, account for tasks assigned to both nCurrentUserID and nNewUserID. For those
	// tasks, we delete records corresponding to nCurrentUserID.
	CString strSql = FormatString("DELETE FROM TodoAssignToT WHERE AssignTo = {INT} AND TaskID IN "
		"(SELECT TaskID FROM TodoAssignToT WHERE AssignTo = {INT}) %s\r\n "
	// Second, update all the instances of nCurrentUserID to nNewUserID
		"UPDATE TodoAssignToT SET AssignTo = {INT} WHERE AssignTo = {INT} %s"
		,strFilter, strFilter
		);
	ExecuteParamSql(strSql
		,nCurrentUserID, nNewUserID
		,nNewUserID, nCurrentUserID);
}

// (c.haag 2008-06-09 17:08) - PLID 30328 - Deletes a single todo alarm
// (c.haag 2008-07-14 15:25) - PLID 30607 - Added bFromEMR
void TodoDelete(long nTaskID, BOOL bFromEMR /*= FALSE */)
{
	// (c.haag 2008-06-24 12:00) - PLID 17244 - Don't let non-administrators delete
	// partially created spawned todo alarms
	// (c.haag 2008-07-14 15:26) - PLID 30607 - If bFromEMR is TRUE, it means we're deleting
	// the todo alarm from the More Info topic. That means it's perfectly fine that the RegardingID
	// is -1 because it corresponds to a currently open and unsaved EMN or EMN detail. Also optimized
	// the query to not use a UNION clause.
	if (!IsCurrentUserAdministrator() && !bFromEMR) {
		_RecordsetPtr prs = CreateParamRecordset(
			FormatString("SELECT TaskID FROM TodoList WHERE TaskID = {INT} AND RegardingType IN (%d,%d) AND RegardingID <= 0 "
			,(long)ttEMNDetail,(long)ttEMN)
			,nTaskID);
		if (!prs->eof) {
			MsgBox(MB_ICONERROR | MB_OK, "You may not delete a todo alarm that is linked to an unsaved EMR detail.");
			return;
		}
	}

	// (c.haag 2008-07-10 15:07) - PLID 30674 - Also delete from EMRTodosT
	ExecuteParamSql(
		"DELETE FROM TodoAssignToT WHERE TaskID = {INT};\r\n"
		"DELETE FROM EMRTodosT WHERE TaskID = {INT};\r\n"
		"DELETE FROM TodoList WHERE TaskID = {INT};\r\n"
		, nTaskID, nTaskID, nTaskID);
}

// (c.haag 2008-06-09 17:08) - PLID 30328 - Deletes a batch of todo alarms given a filter that
// can operate in the form SELECT ID FROM TodoList WHERE (strWhere)
void TodoDelete(const CString& strWhere)
{
	// (c.haag 2008-06-24 12:00) - PLID 17244 - Don't let non-administrators delete
	// partially created spawned todo alarms
	// (c.haag 2008-07-11 11:29) - PLID 30550 - Don't let non-administrators delete partially created
	// EMN todos
	CString strExWhere;
	if (!IsCurrentUserAdministrator()) {
		strExWhere.Format("(%s) AND TaskID NOT IN (SELECT TaskID FROM TodoList WHERE RegardingType IN (%d,%d) AND RegardingID <= 0)",
			strWhere, (long)ttEMNDetail, (long)ttEMN);
	} else {
		strExWhere = strWhere;
	}
	// Don't use _Q; formatting is the caller's responsibility.
	// (c.haag 2008-07-10 15:08) - PLID 30674 - Also delete from EMRTodosT
	ExecuteSql(
		"DELETE FROM TodoAssignToT WHERE TaskID IN (SELECT TaskID FROM TodoList WHERE (%s));\r\n"
		"DELETE FROM EMRTodosT WHERE TaskID IN (SELECT TaskID FROM TodoList WHERE (%s));\r\n"
		"DELETE FROM TodoList WHERE (%s);\r\n"
		, strExWhere, strExWhere, strExWhere);
}

// (c.haag 2008-06-23 13:02) - PLID 30471 - This is just like TodoTransferAssignTo except it operates
// on EMR todo actions (EMRActionsTodoAssignToT) 
void TodoTransferEmrActionAssignTo(long nCurrentUserID, long nNewUserID)
{
	// First, account for actions assigned to both nCurrentUserID and nNewUserID. For those
	// actions, we delete records corresponding to nCurrentUserID.
	CString strSql = FormatString("DELETE FROM EMRActionsTodoAssignToT WHERE AssignTo = {INT} AND ActionID IN "
		"(SELECT ActionID FROM EMRActionsTodoAssignToT WHERE AssignTo = {INT})\r\n "
	// Second, update all the instances of nCurrentUserID to nNewUserID
		"UPDATE EMRActionsTodoAssignToT SET AssignTo = {INT} WHERE AssignTo = {INT}");
	ExecuteParamSql(strSql
		,nCurrentUserID, nNewUserID
		,nNewUserID, nCurrentUserID);
}

// (c.haag 2008-06-24 12:41) - PLID 17244 - This function returns an SQL query string that lets the caller
// update EMR detail-related todo alarm notes. These notes are formatted with the EMN description, then the
// detail name, followed by the actual note. The query will update those notes with the latest descriptions
// and names only if the note is properly formatted.
// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
CSqlFragment TodoGetEmrDetailAlarmUpdateQ(long nEMNID, long nEMNDetailID)
{
	CSqlFragment sql(
		"UPDATE TodoList SET Notes =  "
		"(SELECT "
		"'EMN: ' + CASE WHEN EmrMasterT.Description IS NULL THEN '' ELSE EmrMasterT.Description END + char(13) + char(10) + "
		"'Detail: ' + CASE WHEN EmrDetailsT.EmrInfoID = -27 THEN "
		"   CASE WHEN MacroName IS NULL THEN '' ELSE MacroName END "
		"ELSE "
		"   CASE WHEN EmrInfoT.Name IS NULL THEN '' ELSE EmrInfoT.Name END "
		"END "
		"+ char(13) + char(10) + "
		"Right(Notes, Len(Notes) - (CHARINDEX(char(10), Notes, CHARINDEX(char(10), Notes) + 1))) "
		"AS NewTodoNote "
		"FROM TodoList T "
		"LEFT JOIN EmrDetailsT ON EmrDetailsT.ID = T.RegardingID "
		"LEFT JOIN EmrInfoT ON EmrInfoT.ID = EmrDetailsT.EmrInfoID "
		"LEFT JOIN EmrMasterT ON EmrMasterT.ID = EmrDetailsT.EMRID "
		"WHERE T.TaskID = TodoList.TaskID "
		") "
		"WHERE RegardingType = {CONST} " 
		"AND RegardingID > 0 "
		"AND CHARINDEX(char(10), Notes) > 0 "
		"AND CHARINDEX(char(10), Notes, CHARINDEX(char(10), Notes) + 1) > 0 "
		"AND Len(Notes) > 3 "
		"AND Left(Notes,4) = 'EMN:' "
		"AND SUBSTRING(Notes, CHARINDEX(char(10), Notes) + 1, 7) = 'Detail:' "
		,(long)ttEMNDetail);

	if (nEMNID > 0) {
		sql += CSqlFragment("AND RegardingID IN (SELECT ID FROM EmrDetailsT WHERE EMRID = {INT}) ", nEMNID);
	}
	if (nEMNDetailID > 0) {
		sql += CSqlFragment("AND RegardingID = {INT} ", nEMNDetailID);
	}
	return sql;
}

// (c.haag 2008-07-11 10:41) - PLID 30550 - This function returns an SQL query string that lets
// the caller update EMN-related todo alarm notes. These notes are formatted with the EMN description
// followed by the actual note. The query will update those notes with the latest descriptions
// and names only if the note is properly formatted.
// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
CSqlFragment TodoGetEmnAlarmUpdateQ(long nEMNID)
{
	return CSqlFragment(
		"UPDATE TodoList SET Notes =  "
		"(SELECT "
		"'EMN: ' + CASE WHEN EmrMasterT.Description IS NULL THEN '' ELSE EmrMasterT.Description END + char(13) + char(10) + "
		"Right(Notes, Len(Notes) - (CHARINDEX(char(10), Notes))) "
		"AS NewTodoNote "
		"FROM TodoList T "
		"LEFT JOIN EmrMasterT ON EmrMasterT.ID = TodoList.RegardingID "
		"WHERE T.TaskID = TodoList.TaskID "
		") "
		"WHERE RegardingType = {CONST} " 
		"AND RegardingID > 0 "
		"AND CHARINDEX(char(10), Notes) > 0 "
		"AND Len(Notes) > 3 "
		"AND Left(Notes,4) = 'EMN:' "
		"AND RegardingID = {INT} "
		,(long)ttEMN, nEMNID);
}

// (c.haag 2008-06-30 10:41) - PLID 11599 - Change the assign-to's of a todo alarm
void TodoChangeAssignTo(long nTaskID, CArray<long,long>& anAssignIDs)
{
	if (anAssignIDs.GetSize() == 0) {
		ThrowNxException("Attempted to remove all assignees from a todo alarm");
	}
	ExecuteSql("DELETE FROM TodoAssignToT WHERE TaskID = %d;\r\n"
		"INSERT INTO TodoAssignToT (TaskID, AssignTo) "
		"SELECT %d, PersonID FROM UsersT WHERE PersonID IN (%s)",
		nTaskID, nTaskID, ArrayAsString(anAssignIDs));
}

// (c.haag 2008-07-03 11:52) - PLID 30615 - Called when the caller wants to change todo alarm assignees. Returns TRUE
// if write permissions check out
BOOL TodoCheckItemReassignmentPermissions(CArray<long,long>& anOldAssignTo, CArray<long,long>& anNewAssignTo)
{
	// Figure out what has changed between the old and new assignment lists
	BOOL bRemovedOtherAssign = FALSE;
	BOOL bRemovedSelfAssign = FALSE;
	BOOL bAddedOtherAssign = FALSE;
	BOOL bAddedSelfAssign = FALSE;
	const long nCurrentUserID = GetCurrentUserID();
	int i;

	for (i=0; i < anOldAssignTo.GetSize() && !bRemovedOtherAssign; i++) {
		long nAssignID = anOldAssignTo[i];
		if (nAssignID != nCurrentUserID && !IsIDInArray(nAssignID, anNewAssignTo)) {
			bRemovedOtherAssign = TRUE;
		}
	}
	if (IsIDInArray(nCurrentUserID, anOldAssignTo) && !IsIDInArray(nCurrentUserID, anNewAssignTo)) {
		bRemovedSelfAssign = TRUE;
	}
	for (i=0; i < anNewAssignTo.GetSize() && !bAddedOtherAssign; i++) {
		long nAssignID = anNewAssignTo[i];
		if (nAssignID != nCurrentUserID && !IsIDInArray(nAssignID, anOldAssignTo)) {
			bAddedOtherAssign = TRUE;
		}
	}
	if (!IsIDInArray(nCurrentUserID, anOldAssignTo) && IsIDInArray(nCurrentUserID, anNewAssignTo)) {
		bAddedSelfAssign = TRUE;
	}

	// Now based on what we figured out, proceed to check permissions. We don't allow changing todo
	// assignments if they violate the permissions.
	if(bRemovedSelfAssign || bAddedSelfAssign) {
		if (!CheckCurrentUserPermissions(bioSelfFollowUps, sptWrite)) {
			return FALSE;
		}
	}
	if(bRemovedOtherAssign || bAddedOtherAssign) {
		if (!CheckCurrentUserPermissions(bioNonSelfFollowUps, sptWrite)) {
			return FALSE;
		}
	}
	return TRUE;
}

//TES 10/10/2008 - PLID 31646 - Create a ToDo Alarm due to one or more changed lab results.
// Pass in the patient, an array of strings describing what changed, and whether the results were changed by the HL7
// Link (as opposed to just being manually changed).  Returns the ID of the created ToDo Alarm.
// (z.manning 2010-05-12 14:57) - PLID 37405 - Added nLabID
//TES 8/6/2013 - PLID 51147 - Added Priority
long TodoCreateForLab(long nPatientID, const long nLabID, const CStringArray &saChangedResults, bool bFromHL7, TodoPriority Priority, long nEnteredBy /*= -1*/)
{
	//TES 10/10/2008 - PLID 31646 - Check the preference for who to assign it to.
	long nToDoUser = GetRemotePropertyInt("Lab_DefaultTodoUser", 0, 0, "<None>");
	if(nToDoUser == 0) {
		//TES 10/10/2008 - PLID 31646 - <Patient Coordinator>
		_RecordsetPtr rs = CreateParamRecordset("SELECT EmployeeID FROM PatientsT WHERE PersonID = {INT}", nPatientID);
		if(!rs->eof) {
			nToDoUser = AdoFldLong(rs, "EmployeeID", 0);
		}
	}
	if(nToDoUser <= 0) {
		//TES 10/10/2008 - PLID 31646 - It's got to be assigned to somebody, let's just use the current user.
		nToDoUser = GetCurrentUserID();
	}
	if(nEnteredBy == -1) {
		nEnteredBy = GetCurrentUserID();
	}
	//TES 8/6/2013 - PLID 51147 - Pass our Priority along
	return TodoCreateForLab(GetRemoteData(), nPatientID, nLabID, nToDoUser, nEnteredBy, saChangedResults, bFromHL7, Priority);
}

// (c.haag 2008-07-03 12:25) - PLID 30615 - This function encapsulates security checking, and saves
// us from having to repeat a lot of code.
// (z.manning 2009-09-11 18:05) - PLID 32048 - Moved this here and made it global
BOOL CheckAssignToPermissions(CArray<long,long> &anAssignTo, ESecurityPermissionType type)
{
	BOOL bSelfAssign = FALSE;
	BOOL bRemoteAssign = FALSE;

	// (a.walling 2006-10-31 17:24) - PLID 23299 - Still check the permissions even though the assign to is
	//		an inactive user.
	// (c.haag 2008-06-10 10:30) - I updated the logic to keep consistent with Adam's change
	// (c.haag 2008-07-03 10:38) - For editing or deleting todo alarms, the rules apply just as they
	// did before with one exception: that is when the todo is assigned to both one self and to another user.
	// Because the todo has joint ownership, an affirmative same-user or other-user permission trumps the 
	// restriction of the other permission. That is, of course, unless both fail the test (in which case, 
	// by definition, you can't edit or delete anything). Consider the possibilities with the alternative -- 
	// you can't edit a todo assigned to you because someone else owns it?
	if (0 == anAssignTo.GetSize()) {
		// The todo is assgined to nobody -- bad data! Return TRUE so that the user may have the chance
		// to fix it.
		return TRUE;
	}
	else {
		for (int i=0; i < anAssignTo.GetSize(); i++) {
			if (anAssignTo[i] == GetCurrentUserID()) {
				bSelfAssign = TRUE;
			} else {
				bRemoteAssign = TRUE;
			}
		}
	}

	if (bSelfAssign && bRemoteAssign) {
		// If we get here, the todo is joint owned. Allow the user to do whatever unless
		// they fail both permission tests
		if(!CheckCurrentUserPermissions(bioSelfFollowUps, type) &&
			!CheckCurrentUserPermissions(bioNonSelfFollowUps, type))
		{
			return FALSE;
		}
	}
	else {
		if (bSelfAssign)
		{
			if(!CheckCurrentUserPermissions(bioSelfFollowUps, type))
				return FALSE;
		}
		if (bRemoteAssign)
		{
			if(!CheckCurrentUserPermissions(bioNonSelfFollowUps, type))
				return FALSE;
		}
	}
	return TRUE;
}
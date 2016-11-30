// AttendanceUtils.cpp
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AttendanceUtils.h"
#include "DateTimeUtils.h"
#include "InternationalUtils.h"
#include "TodoUtils.h"
#include "AuditTrail.h" // (j.luckoski 2012-07-23 12:22) - PLID 29211 - Audit attendance

// (a.walling 2009-10-13 10:01) - PLID 35930
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (z.manning, 02/28/2008) - PLID 29139 - Utility classes and functions for the attendance sheet

//////////////////////////////////////////////////////////////////////////////////////////////
// Begin class AttendanceData

AttendanceData::AttendanceData()
{
	nID = -1;
	pUser = NULL;
	cyHoursWorked.SetCurrency(0, 0);
	cyVacation.SetCurrency(0, 0);
	cySick.SetCurrency(0, 0);
	cyPaid.SetCurrency(0, 0);
	cyUnpaid.SetCurrency(0, 0);
	cyOther.SetCurrency(0, 0);
	dtDate.SetStatus(COleDateTime::invalid);
	varApproverUserID.vt = VT_NULL;
	eType = atNormal;
	dtInputDate.SetStatus(COleDateTime::invalid);
	varParentID.vt = VT_NULL; // (z.manning, 05/19/2008) - PLID 30105
}

/*r.wilson PLID 45311 - made a function to reduce code redundancy. Uses code intially created by z.manning
		this function adds todos to the database
*/
void AttendanceData::AddTodosToDatabase()
{
	CString strTodoNote = FormatString("Approve time of for %s %s on %s", pUser->m_strFirstName, pUser->m_strLastName, FormatDateTimeForInterface(dtDate, 0, dtoDate));

			// (j.armen 2014-01-30 09:29) - PLID 18569 - Idenitate TodoList
			ExecuteParamSql(
				"DECLARE @nManagerID INT \r\n"
				"DECLARE @nTodoID INT \r\n"
				"DECLARE ManagerCursor CURSOR OPTIMISTIC for \r\n"
				"SELECT DepartmentManagersT.UserID \r\n"
				"FROM DepartmentManagersT \r\n"
				"WHERE DepartmentManagersT.DepartmentID IN (SELECT DepartmentID FROM UserDepartmentLinkT WHERE UserID = {INT}) \r\n"
				"OPEN ManagerCursor \r\n"
				"FETCH NEXT FROM ManagerCursor INTO @nManagerID \r\n"
				"WHILE @@FETCH_STATUS = 0 BEGIN \r\n"
				// (z.manning, 01/11/2008) - Create the to-do task
				"	INSERT INTO ToDoList (PersonID, CategoryID, Task, Priority, \r\n"
				"		Deadline, Remind, Notes, EnteredBy, RegardingID, RegardingType) \r\n"
				"	SELECT {INT}, (SELECT TOP 1 IntParam FROM ConfigRT WHERE Name = 'AttendanceToDoCategory' AND IntParam IN (SELECT ID FROM NoteCatsF)), \r\n"
				"		'Other', 1, DATEADD(d, 7, GetDate()), GetDate(), {STRING}, {INT}, {INT}, {INT} \r\n"
				"	SET @nTodoID = SCOPE_IDENTITY()\r\n"
				// (c.haag 2008-06-11 11:40) - PLID 30321 - Add records into TodoAssignToT as well
				"	INSERT INTO TodoAssignToT (TaskID, AssignTo) \r\n"
				"	VALUES (@nTodoID, @nManagerID) \r\n "
				// (z.manning, 01/11/2008) - Link it with the current attendance appointment.
				"	INSERT INTO AttendanceToDoLinkT (AttendanceAppointmentID, ToDoID) VALUES ({INT}, @nTodoID) \r\n"
				"FETCH NEXT FROM ManagerCursor INTO @nManagerID \r\n"
				"END \r\n"
				"CLOSE ManagerCursor \r\n"
				"DEALLOCATE ManagerCursor \r\n"
				, pUser->m_nUserID, pUser->m_nUserID, strTodoNote, GetCurrentUserID(), pUser->m_nUserID
				, ttPatientContact, nID);		
}

void AttendanceData::SaveToData()
{
	if(nID == -1)
	{
		// (z.manning, 04/11/2008) - PLID 29628 - Added input date
		// (z.manning, 05/19/2008) - PLID 30105 - Added parent ID
		// (z.manning 2008-11-13 12:35) - PLID 31831 - Added paid and unpaid
		ADODB::_RecordsetPtr prsSave = CreateParamRecordset(
			"SET NOCOUNT ON \r\n"
			"DECLARE @dtCurrent datetime \r\n"
			"SET @dtCurrent = GetDate() \r\n"
			"INSERT INTO AttendanceAppointmentsT (UserID, Date, HoursWorked, Vacation, ApprovedBy, Sick, Other, Notes, Type, AdminNotes, InputDate, ParentID, PaidTimeOff, UnpaidTimeOff) \r\n"
			"VALUES ({INT}, {STRING}, {STRING}, {STRING}, {VT_I4}, {STRING}, {STRING}, {STRING}, {INT}, {STRING}, @dtCurrent, {VT_I4}, {STRING}, {STRING}) \r\n"
			"SET NOCOUNT OFF \r\n"
			"SELECT CONVERT(int, SCOPE_IDENTITY()) AS ID, @dtCurrent AS InputDate "
			, pUser->m_nUserID, FormatDateTimeForSql(dtDate, dtoDate), FormatCurrencyForSql(cyHoursWorked), FormatCurrencyForSql(cyVacation)
			, varApproverUserID, FormatCurrencyForSql(cySick), FormatCurrencyForSql(cyOther), strNotes, eType, strAdminNotes, varParentID
			, FormatCurrencyForSql(cyPaid), FormatCurrencyForSql(cyUnpaid));

		nID = AdoFldLong(prsSave, "ID");
		dtInputDate = AdoFldDateTime(prsSave, "InputDate");

		// (j.luckoski 2012-07-24 10:18) - PLID 29122 - Audit new requests
		pUser->AuditAttendance(pUser, nID, aeiAttendanceRequest, FormatDateTimeForSql(dtDate), strNotes, FormatCurrencyForSql(cyVacation),FormatCurrencyForSql(cySick), FormatCurrencyForSql(cyUnpaid), FormatCurrencyForSql(cyOther), FormatCurrencyForSql(cyHoursWorked), FormatCurrencyForSql(cyPaid));

		// (z.manning, 01/11/2008) - PLID 28600 - This is a new request, so if there is any vacation, sick, or other
		// hours then let's go ahead and create a todo task for this to be approved (assuming it's not already).
		// (z.manning 2008-11-13 12:42) - PLID 31831 - For the time being at least, paid and unpaid can only
		// be entered by managers so there's no need to create a to-do for those.
		COleCurrency cyZero(0, 0);
		// (j.luckoski 2012-10-24 14:28) - PLID 53329 - Removed cyHoursWorked from the if statement preventing todo alarms
		if(varApproverUserID.vt == VT_NULL && (cyVacation != cyZero || cySick != cyZero || cyOther != cyZero || cyPaid != cyZero || cyUnpaid != cyZero))
		{
			AddTodosToDatabase();
		} else if(varApproverUserID.vt == VT_I4) { // (j.luckoski 2012-11-20 09:39) - PLID 53329 - Must have approver user ID to indicate manager approval
			// (j.luckoski 2012-07-24 10:59) - PLID 29122 - Manager created so approved
			pUser->AuditAttendance(pUser, nID, aeiAttendanceApproved, FormatDateTimeForSql(dtDate), strNotes, FormatCurrencyForSql(cyVacation),FormatCurrencyForSql(cySick), FormatCurrencyForSql(cyUnpaid), FormatCurrencyForSql(cyOther), FormatCurrencyForSql(cyHoursWorked), FormatCurrencyForSql(cyPaid));	
		}
	}
	else
	{
		// (z.manning, 05/19/2008) - PLID 30105 - Added parent ID
		// (z.manning 2008-11-13 12:43) - PLID 31831 - Added paid/unpaid
		ExecuteParamSql(
			"UPDATE AttendanceAppointmentsT \r\n"
			"SET UserID = {INT}, \r\n"
			"	Date = {STRING}, \r\n"
			"	Vacation = {STRING}, "
			"	ApprovedBy = {VT_I4}, \r\n"
			"	Sick = {STRING}, \r\n"
			"	Other = {STRING}, \r\n"
			"	Notes = {STRING}, \r\n"
			"	HoursWorked = {STRING}, \r\n"
			"	Type = {INT}, \r\n"
			"	AdminNotes = {STRING}, \r\n"
			"	ParentID = {VT_I4}, \r\n"
			"	PaidTimeOff = {STRING}, \r\n"
			"	UnpaidTimeOff = {STRING} \r\n"
			"WHERE ID = {INT} "
			, pUser->m_nUserID, FormatDateTimeForSql(dtDate, dtoDate), FormatCurrencyForSql(cyVacation)
			, varApproverUserID, FormatCurrencyForSql(cySick), FormatCurrencyForSql(cyOther), strNotes
			, FormatCurrencyForSql(cyHoursWorked), eType, strAdminNotes, varParentID
			, FormatCurrencyForSql(cyPaid), FormatCurrencyForSql(cyUnpaid)
			, nID);

		//(r.wilson 9/10/2011)PLID 45311 - Resused code from above
		COleCurrency cyZero(0, 0);
		
		// (j.luckoski 2012-07-24 10:18) - PLID 29122 - Audit updates
		pUser->AuditAttendance(pUser, nID, aeiAttendanceUpdate, FormatDateTimeForSql(dtDate), strNotes, FormatCurrencyForSql(cyVacation),FormatCurrencyForSql(cySick), FormatCurrencyForSql(cyUnpaid), FormatCurrencyForSql(cyOther), FormatCurrencyForSql(cyHoursWorked), FormatCurrencyForSql(cyPaid));
		
		// (j.luckoski 2012-10-24 14:28) - PLID 53329 - Removed cyHoursWorked from the if statement preventing todo alarms
		if(varApproverUserID.vt == VT_NULL && (cyVacation != cyZero || cySick != cyZero || cyOther != cyZero || cyPaid != cyZero || cyUnpaid != cyZero))
		{
			//r.wilson PLID 45311 - If there are no todos for this id in the database already then we need to create them
			if(!ReturnsRecordsParam(
				" SELECT ID "
				" FROM AttendanceAppointmentsT INNER JOIN AttendanceToDoLinkT "
				" ON AttendanceAppointmentsT.ID = AttendanceToDoLinkT.AttendanceAppointmentID "
				" WHERE "
				" AttendanceAppointmentsT.ID = {INT} ",
				nID))
			{
				AddTodosToDatabase();
			}

		} else if(varApproverUserID.vt == VT_I4) { // (j.luckoski 2012-11-20 09:39) - PLID 53329 - Must have approver user ID to indicate manager approval
			// (j.luckoski 2012-07-24 10:59) - PLID 29122 - Manager created so approved
			pUser->AuditAttendance(pUser, nID, aeiAttendanceApproved, FormatDateTimeForSql(dtDate), strNotes, FormatCurrencyForSql(cyVacation),FormatCurrencyForSql(cySick), FormatCurrencyForSql(cyUnpaid), FormatCurrencyForSql(cyOther), FormatCurrencyForSql(cyHoursWorked), FormatCurrencyForSql(cyPaid));	
		}

		// (z.manning 2009-08-04 09:34) - PLID 35098 - If this was an existing time off request that is now
		// approved, we need to ensure there are no outstanding to-do tasks for this appt.
		if(varApproverUserID.vt == VT_I4) {
			CArray<long,long> arynAttendanceDataIDs;
			arynAttendanceDataIDs.Add(nID);
			DeleteAssociatedTodos(arynAttendanceDataIDs);
		}
	}
}

void AttendanceData::DeleteFromData()
{
	if(nID == -1) {
		ASSERT(FALSE);
		return;
	}

	// (z.manning, 01/14/2008) - PLID 28600 - Need to first delete any assocaited to-do alarms.
	CArray<long,long> arynAttendanceDataIDs;
	arynAttendanceDataIDs.Add(nID);
	DeleteAssociatedTodos(arynAttendanceDataIDs);

	// (z.manning, 05/20/2008) - PLID 30105 - If this is part of a group of attendance appts, then
	// let's break up the group to avoid weird data as well as possible FK errors.
	long nParentID = VarLong(varParentID, -1);
	ExecuteParamSql(
		"UPDATE AttendanceAppointmentsT SET ParentID = NULL WHERE ParentID IN ({INT}, {INT}) \r\n"
		"DELETE FROM AttendanceAppointmentsT WHERE ID = {INT} "
		, nID, nParentID, nID);

	// (z.manning, 05/20/2008) - PLID 30105 - Now clear out the parent IDs in memory
	for(int nIndex = 0; nIndex < pUser->m_arypAttendanceData.GetSize(); nIndex++)
	{
		AttendanceData *pTemp = pUser->m_arypAttendanceData.GetAt(nIndex);
		long nTempParentID = VarLong(pTemp->varParentID, -1);
		if(nTempParentID > 0 && (nTempParentID == nID || nTempParentID == nParentID)) {
			pTemp->varParentID.vt = VT_NULL;
		}
	}
}

BOOL AttendanceData::IsApproved()
{
	if(varApproverUserID.vt == VT_I4) {
		ASSERT(VarLong(varApproverUserID) > 0);
		return TRUE;
	}
	else {
		ASSERT(varApproverUserID.vt == VT_NULL);
		return FALSE;
	}
}

AttendanceData& AttendanceData::operator=(const AttendanceData &source)
{
	nID = source.nID;
	pUser = source.pUser;
	dtDate = source.dtDate;
	cyHoursWorked = source.cyHoursWorked;
	cyVacation = source.cyVacation;
	cySick = source.cySick;
	// (z.manning 2008-11-13 12:44) - PLID 31831 - Paid/unpaid
	cyPaid = source.cyPaid;
	cyUnpaid = source.cyUnpaid;
	cyOther = source.cyOther;
	varApproverUserID = source.varApproverUserID;
	strNotes = source.strNotes;
	eType = source.eType;
	strAdminNotes = source.strAdminNotes;
	dtInputDate = source.dtInputDate;
	varParentID = source.varParentID; // (z.manning, 05/19/2008) - PLID 30105

	return *this;
}

// (z.manning, 01/11/2008) - PLID 28461 - Returns a text summary of the attendance data.
CString AttendanceData::GetText()
{
	CString str = FormatString("Date: %s", FormatDateTimeForInterface(dtDate, 0, dtoDate));
	COleCurrency cyZero(0, 0);

	COleDateTime dtStart, dtEnd;
	GetDateRange(dtStart, dtEnd);
	if(dtEnd > dtStart) {
		// (z.manning, 9/2/2008) - PLID 31162 - If we have a date range then make sure
		// to include that in the text.
		str += FormatString(" - %s", FormatDateTimeForInterface(dtEnd, 0, dtoDate));
	}
	str += "\r\n";

	if(cyHoursWorked != cyZero) {
		str += FormatString("Hours worked: %s\r\n", FormatAttendanceValue(cyHoursWorked));
	}
	if(cyVacation != cyZero) {
		CString strVacation;
		if(dtEnd > dtStart) {
			// (z.manning, 9/2/2008) - PLID 31162 - We have a date range, so account for that
			// (z.manning 2008-11-13 16:03) - PLID 31783 - Don't count holidays and weekends.
			long nVacation = 0;
			for(COleDateTime dtTemp = dtStart; dtTemp <= dtEnd; dtTemp += ONE_DAY) {
				if(IsValidVacationDay(dtTemp)) {
					nVacation += 8;
				}
			}
			strVacation = AsString(nVacation);
		}
		else {
			strVacation = FormatAttendanceValue(cyVacation);
		}
		// (d.thompson 2014-02-27) - PLID 61016 - Rename vacation to PTO
		str += FormatString("PTO: %s\r\n", strVacation);
	}
	if(cySick != cyZero) {
		str += FormatString("Sick: %s\r\n", FormatAttendanceValue(cySick));
	}
	// (z.manning 2008-11-13 12:44) - PLID 31831 - Added paid/unpaid
	if(cyPaid != cyZero) {
		str += FormatString("Paid time off: %s\r\n", FormatAttendanceValue(cyPaid));
	}
	if(cyUnpaid != cyZero) {
		str += FormatString("Unpaid time off: %s\r\n", FormatAttendanceValue(cyUnpaid));
	}
	if(cyOther != cyZero) {
		str += FormatString("Other: %s\r\n", FormatAttendanceValue(cyOther));
	}
	str += "\r\n" + strNotes + "\r\n";

	return str;
}

// (z.manning, 05/19/2008) - PLID 30105 - Returns the end date of a group of attendance appointments
void AttendanceData::GetDateRange(OUT COleDateTime &dtStart, OUT COleDateTime &dtEnd)
{
	dtStart = dtEnd = dtDate;
	for(int nIndex = 0; nIndex < pUser->m_arypAttendanceData.GetSize(); nIndex++)
	{
		AttendanceData *pTemp = pUser->m_arypAttendanceData.GetAt(nIndex);
		long nParentID = VarLong(varParentID, -1);
		if( (nParentID > 0 && (nParentID == pTemp->nID || nParentID == VarLong(pTemp->varParentID, -1)))
			 || (nID > 0 && nID == VarLong(pTemp->varParentID, -1)) )
		{
			if(pTemp->dtDate < dtStart) {
				dtStart = pTemp->dtDate;
			}
			if(pTemp->dtDate > dtEnd) {
				dtEnd = pTemp->dtDate;
			}
		}
	}
}

BOOL AttendanceData::IsPartOfGroup()
{
	for(int nIndex = 0; nIndex < pUser->m_arypAttendanceData.GetSize(); nIndex++)
	{
		AttendanceData *pTemp = pUser->m_arypAttendanceData.GetAt(nIndex);
		long nParentID = VarLong(varParentID, -1);
		if( (nParentID > 0 && (nParentID == pTemp->nID || nParentID == VarLong(pTemp->varParentID, -1)))
			 || (nID > 0 && nID == VarLong(pTemp->varParentID, -1)) && pTemp != this)
		{
			return TRUE;
		}
	}

	return FALSE;
}

// End class AttendanceData
//////////////////////////////////////////////////////////////////////////////////////////////
// Begin class AttendanceUser

AttendanceUser::AttendanceUser(AttendanceInfo *pParentInfo)
{
	m_nUserID = -1;
	m_bSorted = FALSE;
	m_nVacationAllowance = 0;
	m_nSickAllowance = 0;
	m_dtDateOfHire.SetStatus(COleDateTime::invalid);
	m_dtDateOfTermination.SetStatus(COleDateTime::invalid);
	m_eType = autSalary;
	m_pAttendanceInfo = pParentInfo;
	m_nVacationBonus = 0;
}

AttendanceUser::~AttendanceUser()
{
	for(int i = 0; i < m_arypAttendanceData.GetSize(); i++) {
		if(m_arypAttendanceData.GetAt(i) != NULL) {
			delete m_arypAttendanceData.GetAt(i);
		}
	}
	m_arypAttendanceData.RemoveAll();
}

void AttendanceUser::SortByDate()
{
	if(m_bSorted) {
		// (z.manning, 11/12/2007) - If we're already sorted, don't do it again.
		return;
	}

	qsort(m_arypAttendanceData.GetData(), m_arypAttendanceData.GetSize(), sizeof(AttendanceData*), CompareAttendanceDataByDate);
	m_bSorted = TRUE;
}

// (z.manning, 11/28/2007) - Returns true if the user is in the given department and false otherwise.
BOOL AttendanceUser::IsInDepartment(long nDepartmentID)
{
	for(int nDeptIndex = 0; nDeptIndex < m_arynDepartmentIDs.GetSize(); nDeptIndex++)
	{
		if(m_arynDepartmentIDs.GetAt(nDeptIndex) == nDepartmentID) {
			return TRUE;
		}
	}

	return FALSE;
}

AttendanceData* AttendanceUser::GetAttendanceDataForDate(COleDateTime dtDate)
{
	dtDate.SetDate(dtDate.GetYear(), dtDate.GetMonth(), dtDate.GetDay());
	for(int nDataIndex = 0; nDataIndex < m_arypAttendanceData.GetSize(); nDataIndex++)
	{
		COleDateTime dtTemp = m_arypAttendanceData.GetAt(nDataIndex)->dtDate;
		if(dtDate == m_arypAttendanceData.GetAt(nDataIndex)->dtDate) {
			return m_arypAttendanceData.GetAt(nDataIndex);
		}
	}

	return NULL;
}

void AttendanceUser::AddAttendanceData(AttendanceData *pData)
{
	if(m_bSorted)
	{
		// (z.manning, 12/03/2007) - Our data is sorted so let's keep it that way!
		BOOL bAdded = FALSE;
		for(int nDataIndex = 0; nDataIndex < m_arypAttendanceData.GetSize(); nDataIndex++)
		{
			if(pData->dtDate < m_arypAttendanceData.GetAt(nDataIndex)->dtDate) {
				m_arypAttendanceData.InsertAt(nDataIndex, pData);
				bAdded = TRUE;
				break;
			}
		}

		if(!bAdded) {
			m_arypAttendanceData.Add(pData);
		}
	}
	else
	{
		// (z.manning, 12/03/2007) - It's not sorted, so just add it at the end.
		m_arypAttendanceData.Add(pData);
	}
}

void AttendanceUser::RemoveAttendanceData(AttendanceData *pData)
{
	if(pData == NULL) {
		ASSERT(FALSE);
		return;
	}

	for(int nDataIndex = 0; nDataIndex < m_arypAttendanceData.GetSize(); nDataIndex++)
	{
		if(m_arypAttendanceData.GetAt(nDataIndex) == pData) {
			// (j.luckoski 2012-07-24 09:53) - PLID 29122 - Audit deletion of attendance appts
			AuditAttendance(pData->pUser, pData->nID, aeiAttendanceDelete, FormatDateTimeForSql(pData->dtDate), pData->strNotes, FormatCurrencyForSql(pData->cyVacation),FormatCurrencyForSql(pData->cySick), FormatCurrencyForSql(pData->cyUnpaid), FormatCurrencyForSql(pData->cyOther), FormatCurrencyForSql(pData->cyHoursWorked), FormatCurrencyForSql(pData->cyPaid));
			delete m_arypAttendanceData.GetAt(nDataIndex);
			m_arypAttendanceData.SetAt(nDataIndex, NULL);
			m_arypAttendanceData.RemoveAt(nDataIndex);
			break;
		}
	}
}

void AttendanceUser::ApproveAttendanceData(CArray<AttendanceData*,AttendanceData*> &arypDataToApprove, CWnd *pwndParent)
{
	if(arypDataToApprove.GetSize() == 0) {
		return;
	}

	CString strBody = "The following time off requests have been approved...\r\n\r\n";

	CArray<long,long> arynAttendanceDataIDs;
	for(int nDataIndex = 0; nDataIndex < arypDataToApprove.GetSize(); nDataIndex++)
	{
		AttendanceData *pData = arypDataToApprove.GetAt(nDataIndex);

		BOOL bApprove = TRUE;
		// (z.manning, 02/13/2008) - Warn if this will put someone over their allowed balance.
		// (d.thompson 2014-02-27) - PLID 61016 - Rename vacation to PTO
		if(pData->pUser->GetTotalUsedVacation() > COleCurrency(pData->pUser->GetAvailableVacation(),0)) {
			CString strMsg = FormatString("'%s' has scheduled more PTO than he or she is allotted.\r\n\r\n"
				"Are you sure you want to approve his or her PTO on %s", pData->pUser->GetFullName(), FormatDateTimeForInterface(pData->dtDate,0,dtoDate));
			if(MessageBox(pwndParent->GetSafeHwnd(), strMsg, "Approve PTO", MB_YESNO) != IDYES) {
				bApprove = FALSE;
			}
		}
		if(pData->pUser->GetTotalUsedSick() > COleCurrency(pData->pUser->GetAvailableSick(),0)) {
			CString strMsg = FormatString("%s has scheduled more sick time than he or she is allotted.\r\n\r\n"
				"Are you sure you want to approve his or her sick time on %s", pData->pUser->GetFullName(), FormatDateTimeForInterface(pData->dtDate,0,dtoDate));
			if(MessageBox(pwndParent->GetSafeHwnd(), strMsg, "Approve Sick", MB_YESNO) != IDYES) {
				bApprove = FALSE;
			}
		}

		if(bApprove) {
			// (j.luckoski 2012-07-24 09:53) - PLID 29122 - Audit approval of attendance appts
			AuditAttendance(pData->pUser, pData->nID, aeiAttendanceApproved, FormatDateTimeForSql(pData->dtDate), pData->strNotes, FormatCurrencyForSql(pData->cyVacation),FormatCurrencyForSql(pData->cySick), FormatCurrencyForSql(pData->cyUnpaid), FormatCurrencyForSql(pData->cyOther), FormatCurrencyForSql(pData->cyHoursWorked), FormatCurrencyForSql(pData->cyPaid));
			strBody += pData->GetText() + "\r\n\r\n";
			arynAttendanceDataIDs.Add(pData->nID);
			pData->varApproverUserID = GetCurrentUserID();
		}
		else {
			arypDataToApprove.RemoveAt(nDataIndex);
			nDataIndex--;
		}
	}

	if(arypDataToApprove.GetSize() == 0) {
		return;
	}

	ExecuteParamSql(FormatString(
		"UPDATE AttendanceAppointmentsT SET ApprovedBy = {INT} \r\n"
		"WHERE ID IN (%s) ", ArrayAsString(arynAttendanceDataIDs,false))
		, GetCurrentUserID()
		);

	// (z.manning, 01/14/2008) - PLID 28600 - Let's go ahead and just delete any assocaited to-do alarms.
	DeleteAssociatedTodos(arynAttendanceDataIDs);

	// (z.manning, 01/11/2008) - PLID 28461 - Email the user that this time of request has been approved.
	if(GetRemotePropertyInt("EmailOnTimeOffApproved", 1, 0, GetCurrentUserName(), true) == 1) {
		SendEmail(pwndParent, m_strEmail, "Time Off Approved", strBody);
	}
}

void AttendanceUser::DenyAttendanceData(const CArray<AttendanceData*,AttendanceData*> &arypDataToApprove, CWnd *pwndParent)
{
	if(arypDataToApprove.GetSize() == 0) {
		return;
	}

	CString strBody = "The following time off requests have been DENIED...\r\n\r\n";

	CArray<long,long> arynAttendanceDataIDs;
	for(int nDataIndex = 0; nDataIndex < arypDataToApprove.GetSize(); nDataIndex++)
	{
		// (j.luckoski 2012-07-24 09:54) - PLID 29122 - Audit denying attendance appts
		AttendanceData *pData = arypDataToApprove.GetAt(nDataIndex);
		AuditAttendance(pData->pUser, pData->nID, aeiAttendanceDenied, FormatDateTimeForSql(pData->dtDate), pData->strNotes, FormatCurrencyForSql(pData->cyVacation),FormatCurrencyForSql(pData->cySick), FormatCurrencyForSql(pData->cyUnpaid), FormatCurrencyForSql(pData->cyOther), FormatCurrencyForSql(pData->cyHoursWorked), FormatCurrencyForSql(pData->cyPaid));
		arynAttendanceDataIDs.Add(pData->nID);
		strBody += pData->GetText() + "\r\n\r\n";
		pData->DeleteFromData();
		RemoveAttendanceData(pData);
	}

	// (z.manning, 01/11/2008) - PLID 28461 - Email the user that this time of request has been denied.
	if(GetRemotePropertyInt("EmailOnTimeOffDenied", 1, 0, GetCurrentUserName(), true) == 1) {
		SendEmail(pwndParent, m_strEmail, "Time Off DENIED", strBody);
	}
}

CString AttendanceUser::GetFullName()
{
	return m_strLastName + ", " + m_strFirstName;
}

float AttendanceUser::GetPercentOfYearWorkedAsOf(COleDateTime dtDate)
{
	float fPercentOfYearWorked;
	int nYear = dtDate.GetYear();
	// (z.manning, 01/15/2008) - Per Najla, employees do not start accruing vacation until after 90 days, so
	// let's find that date and use it as the basis determing how much they've accrued.
	// (d.thompson 2014-02-25) - PLID 61017 - We have rescinded the 90 policy.  You now start accruing days immediately
	//	upon hire.
	COleDateTime dtStartAccrualDate;
	if(m_dtDateOfHire.GetStatus() == COleDateTime::valid) {
		dtStartAccrualDate = m_dtDateOfHire;
	}
	else {
		dtStartAccrualDate.SetDate(nYear, 1, 1);
	}

	if(m_pAttendanceInfo->m_nYear == nYear) {
		COleDateTime dtLastDayOfYear(nYear, 12, 31, 0, 0, 0);
		int nHireDateOffset = 0;
		if(dtStartAccrualDate > dtDate) {
			return 0.0;
		}
		else if(dtStartAccrualDate.GetYear() == nYear) {
			// (z.manning, 12/05/2007) - Was hired earlier this year, so let's adjust the amount accrued to that date.
			nHireDateOffset = dtStartAccrualDate.GetDayOfYear();
		}
		// (z.manning 2008-12-04 15:20) - PLID 29105 - Need to add one to nHireDateOffset below so that the first
		// day they work counts!
		fPercentOfYearWorked = (dtDate.GetDayOfYear() - nHireDateOffset + 1) / (float)dtLastDayOfYear.GetDayOfYear();
	}
	else if(m_pAttendanceInfo->m_nYear < nYear) {
		fPercentOfYearWorked = 1.0;
	}
	else {
		fPercentOfYearWorked = 0.0;
	}

	return fPercentOfYearWorked;
}

void AttendanceUser::GetCurrentUsedTotals(OUT COleCurrency &cyHoursWorked, OUT COleCurrency &cyVacation, OUT COleCurrency &cySick, OUT COleCurrency &cyPaid, OUT COleCurrency &cyUnpaid, OUT COleCurrency &cyOther, const IN COleDateTime dtRangeStart, const IN COleDateTime dtRangeEnd, BOOL bIgnoreFloatingHolidays /* = TRUE */, BOOL bUnapprovedOnly /* = FALSE */)
{
	cyHoursWorked = cyVacation = cySick = cyPaid = cyUnpaid = cyOther = COleCurrency(0, 0);
	for(int nDataIndex = 0; nDataIndex < m_arypAttendanceData.GetSize(); nDataIndex++)
	{
		AttendanceData *pData = m_arypAttendanceData.GetAt(nDataIndex);
		// (z.manning, 05/19/2008) - PLID 30102 - Handle the bUnapprovedOnly parameter
		if(pData->dtDate >= dtRangeStart && pData->dtDate <= dtRangeEnd && (!bUnapprovedOnly || !pData->IsApproved()))
		{
			cyHoursWorked += pData->cyHoursWorked;
			// (z.manning, 02/08/2008) - Floating holidays do not count toward usage totals
			if(pData->eType != atFloatingHoliday || !bIgnoreFloatingHolidays) {
				cyVacation += pData->cyVacation;
			}
			cySick += pData->cySick;
			cyOther += pData->cyOther;
			// (z.manning 2008-11-13 15:05) - PLID 31831 - Paid/Unpaid
			cyPaid += pData->cyPaid;
			cyUnpaid += pData->cyUnpaid;
		}
	}
}

COleCurrency AttendanceUser::GetTotalHoursWorked()
{
	// (z.manning 2008-11-13 13:59) - PLID 31831 - Added paid/unpaid
	COleCurrency cyHoursWorked, cyVacation, cySick, cyOther, cyPaid, cyUnpaid;
	GetCurrentUsedTotals(cyHoursWorked, cyVacation, cySick, cyPaid, cyUnpaid, cyOther, COleDateTime(m_pAttendanceInfo->m_nYear,1,1,0,0,0), COleDateTime(m_pAttendanceInfo->m_nYear,12,31,0,0,0));
	return cyHoursWorked;
}

COleCurrency AttendanceUser::GetTotalUsedVacation(BOOL bIgnoreFloatingHolidays /* = TRUE */)
{
	// (z.manning 2008-11-13 13:59) - PLID 31831 - Added paid/unpaid
	COleCurrency cyHoursWorked, cyVacation, cySick, cyOther, cyPaid, cyUnpaid;
	GetCurrentUsedTotals(cyHoursWorked, cyVacation, cySick, cyPaid, cyUnpaid, cyOther, COleDateTime(m_pAttendanceInfo->m_nYear,1,1,0,0,0), COleDateTime(m_pAttendanceInfo->m_nYear,12,31,0,0,0), bIgnoreFloatingHolidays);
	return cyVacation;
}

COleCurrency AttendanceUser::GetTotalUsedSick()
{
	// (z.manning 2008-11-13 13:59) - PLID 31831 - Added paid/unpaid
	COleCurrency cyHoursWorked, cyVacation, cySick, cyOther, cyPaid, cyUnpaid;
	GetCurrentUsedTotals(cyHoursWorked, cyVacation, cySick, cyPaid, cyUnpaid, cyOther, COleDateTime(m_pAttendanceInfo->m_nYear,1,1,0,0,0), COleDateTime(m_pAttendanceInfo->m_nYear,12,31,0,0,0));
	return cySick;
}

// (z.manning 2008-11-13 13:57) - PLID 31831
COleCurrency AttendanceUser::GetTotalUsedPaid()
{
	COleCurrency cyHoursWorked, cyVacation, cySick, cyOther, cyPaid, cyUnpaid;
	GetCurrentUsedTotals(cyHoursWorked, cyVacation, cySick, cyPaid, cyUnpaid, cyOther, COleDateTime(m_pAttendanceInfo->m_nYear,1,1,0,0,0), COleDateTime(m_pAttendanceInfo->m_nYear,12,31,0,0,0));
	return cyPaid;
}

// (z.manning 2008-11-13 13:57) - PLID 31831
COleCurrency AttendanceUser::GetTotalUsedUnpaid()
{
	COleCurrency cyHoursWorked, cyVacation, cySick, cyOther, cyPaid, cyUnpaid;
	GetCurrentUsedTotals(cyHoursWorked, cyVacation, cySick, cyPaid, cyUnpaid, cyOther, COleDateTime(m_pAttendanceInfo->m_nYear,1,1,0,0,0), COleDateTime(m_pAttendanceInfo->m_nYear,12,31,0,0,0));
	return cyUnpaid;
}

COleCurrency AttendanceUser::GetTotalUsedOther()
{
	// (z.manning 2008-11-13 13:59) - PLID 31831 - Added paid/unpaid
	COleCurrency cyHoursWorked, cyVacation, cySick, cyOther, cyPaid, cyUnpaid;
	GetCurrentUsedTotals(cyHoursWorked, cyVacation, cySick, cyPaid, cyUnpaid, cyOther, COleDateTime(m_pAttendanceInfo->m_nYear,1,1,0,0,0), COleDateTime(m_pAttendanceInfo->m_nYear,12,31,0,0,0));
	return cyOther;
}

void AttendanceUser::GetBalances(OUT COleCurrency &cyVacation, OUT COleCurrency &cySick)
{
	cyVacation = COleCurrency(GetAvailableVacation(), 0) - GetTotalUsedVacation();
	cySick = COleCurrency(GetAvailableSick(), 0) - GetTotalUsedSick();
}

COleCurrency AttendanceUser::GetTotalHoursWorkedByDate(COleDateTime dtStart, COleDateTime dtEnd)
{
	// (z.manning 2008-11-13 14:02) - PLID 31831 - Added paid/unpaid
	COleCurrency cyHoursWorked, cyVacation, cySick, cyOther, cyPaid, cyUnpaid;
	GetCurrentUsedTotals(cyHoursWorked, cyVacation, cySick, cyPaid, cyUnpaid, cyOther, dtStart, dtEnd);
	return cyHoursWorked;
}

// (z.manning, 05/19/2008) - PLID 30102 - Added parameter to ignore approved time off
COleCurrency AttendanceUser::GetTotalUsedVacationByDate(COleDateTime dtStart, COleDateTime dtEnd, BOOL bIgnoreFloatingHolidays /* = TRUE */, BOOL bUnapprovedOnly /* = FALSE */)
{
	// (z.manning 2008-11-13 14:02) - PLID 31831 - Added paid/unpaid
	COleCurrency cyHoursWorked, cyVacation, cySick, cyOther, cyPaid, cyUnpaid;
	GetCurrentUsedTotals(cyHoursWorked, cyVacation, cySick, cyPaid, cyUnpaid, cyOther, dtStart, dtEnd, bIgnoreFloatingHolidays, bUnapprovedOnly);
	return cyVacation;
}

// (z.manning, 05/19/2008) - PLID 30102 - Added parameter to ignore approved time off
COleCurrency AttendanceUser::GetTotalUsedSickByDate(COleDateTime dtStart, COleDateTime dtEnd, BOOL bUnapprovedOnly /* = FALSE */)
{
	// (z.manning 2008-11-13 14:02) - PLID 31831 - Added paid/unpaid
	COleCurrency cyHoursWorked, cyVacation, cySick, cyOther, cyPaid, cyUnpaid;
	GetCurrentUsedTotals(cyHoursWorked, cyVacation, cySick, cyPaid, cyUnpaid, cyOther, dtStart, dtEnd, TRUE, bUnapprovedOnly);
	return cySick;
}

COleCurrency AttendanceUser::GetTotalUsedPaidByDate(COleDateTime dtStart, COleDateTime dtEnd, BOOL bUnapprovedOnly /* = FALSE */)
{
	COleCurrency cyHoursWorked, cyVacation, cySick, cyOther, cyPaid, cyUnpaid;
	GetCurrentUsedTotals(cyHoursWorked, cyVacation, cySick, cyPaid, cyUnpaid, cyOther, dtStart, dtEnd, TRUE, bUnapprovedOnly);
	return cyPaid;
}

COleCurrency AttendanceUser::GetTotalUsedUnpaidByDate(COleDateTime dtStart, COleDateTime dtEnd, BOOL bUnapprovedOnly /* = FALSE */)
{
	COleCurrency cyHoursWorked, cyVacation, cySick, cyOther, cyPaid, cyUnpaid;
	GetCurrentUsedTotals(cyHoursWorked, cyVacation, cySick, cyPaid, cyUnpaid, cyOther, dtStart, dtEnd, TRUE, bUnapprovedOnly);
	return cyUnpaid;
}

// (z.manning, 05/19/2008) - PLID 30102 - Added parameter to ignore approved time off
COleCurrency AttendanceUser::GetTotalUsedOtherByDate(COleDateTime dtStart, COleDateTime dtEnd, BOOL bUnapprovedOnly /* = FALSE */)
{
	// (z.manning 2008-11-13 14:02) - PLID 31831 - Added paid/unpaid
	COleCurrency cyHoursWorked, cyVacation, cySick, cyOther, cyPaid, cyUnpaid;
	GetCurrentUsedTotals(cyHoursWorked, cyVacation, cySick, cyPaid, cyUnpaid, cyOther, dtStart, dtEnd, TRUE, bUnapprovedOnly);
	return cyOther;
}

COleCurrency AttendanceUser::GetBalanceVacation()
{
	COleCurrency cyVacation, cySick;
	GetBalances(cyVacation, cySick);
	return cyVacation;
}

COleCurrency AttendanceUser::GetBalanceSick()
{
	COleCurrency cyVacation, cySick;
	GetBalances(cyVacation, cySick);
	return cySick;
}

// (z.manning 2008-11-21 12:27) - PLID 32139
CString AttendanceUser::GetBalanceVacationText()
{
	CString strBalance = FormatAttendanceValue(GetBalanceVacation());
	// (z.manning 2009-08-03 09:50) - PLID 32907 - No more floating holiday per KM
	//if(GetFloatingHolidayCount() == 0) {
	//	strBalance += " + FH";
	//}
	return strBalance;
}

void AttendanceUser::GetAccruedTotals(OUT COleCurrency &cyVacation, OUT COleCurrency &cySick)
{
	float fPercentOfYearWorked = GetPercentOfYearWorkedAsOf(COleDateTime::GetCurrentTime());
	cyVacation = COleCurrency((int)(m_nVacationAllowance * fPercentOfYearWorked) + m_nVacationBonus, 0);
	cySick = COleCurrency((int)(m_nSickAllowance * fPercentOfYearWorked), 0);
}

COleCurrency AttendanceUser::GetAccruedVacation()
{
	COleCurrency cyVacation, cySick;
	GetAccruedTotals(cyVacation, cySick);
	return cyVacation;
}

COleCurrency AttendanceUser::GetAccruedSick()
{
	COleCurrency cyVacation, cySick;
	GetAccruedTotals(cyVacation, cySick);
	return cySick;
}

void AttendanceUser::GetAvailableTotals(OUT int &nVacation, OUT int &nSick)
{
	// (z.manning, 01/15/2008) - The total vacation and sick time available to an employee needs to reflect
	// his/her date of hire.
	// (z.manning 2008-11-18 11:45) - PLID 31782 - We need to use the year of the attendance info
	// here instead of just the current calendar year.
	float fPercentOfYearWorked = GetPercentOfYearWorkedAsOf(COleDateTime(m_pAttendanceInfo->m_nYear, 12, 31, 0, 0, 0));

	nVacation = (int)(m_nVacationAllowance * fPercentOfYearWorked) + m_nVacationBonus;
	nSick = (int)(m_nSickAllowance * fPercentOfYearWorked);
}

int AttendanceUser::GetAvailableVacation()
{
	int nSick, nVacation;
	GetAvailableTotals(nVacation, nSick);
	return nVacation;
}

int AttendanceUser::GetAvailableSick()
{
	int nSick, nVacation;
	GetAvailableTotals(nVacation, nSick);
	return nSick;
}

BOOL AttendanceUser::IsManager()
{
	return (m_arynManagedDepartmentIDs.GetSize() > 0);
}

int AttendanceUser::GetFloatingHolidayCount()
{
	int nCount = 0;
	for(int nDataIndex = 0; nDataIndex < m_arypAttendanceData.GetSize(); nDataIndex++)
	{
		AttendanceData *pData = m_arypAttendanceData.GetAt(nDataIndex);
		if(pData->eType == atFloatingHoliday) {
			nCount++;
		}
	}

	return nCount;
}

// End class AttendanceUser
//////////////////////////////////////////////////////////////////////////////////////////////
// Begin class AttendanceInfo

AttendanceInfo::AttendanceInfo()
{
	m_nYear = COleDateTime::GetCurrentTime().GetYear();
}

AttendanceInfo::~AttendanceInfo()
{
	Clear();
}

void AttendanceInfo::Clear()
{
	for(int i = 0; i < m_arypAttendanceUsers.GetSize(); i++) {
		if(m_arypAttendanceUsers.GetAt(i) != NULL) {
			delete m_arypAttendanceUsers.GetAt(i);
		}
	}
	m_arypAttendanceUsers.RemoveAll();
	m_arydtPayrollCloseDates.RemoveAll();
}

void AttendanceInfo::LoadAllByYear(const int nYear)
{
	Clear();
	m_nYear = nYear;

	COleDateTime dtRangeStart, dtRangeEnd;
	dtRangeStart.SetDate(nYear, 1, 1);
	dtRangeEnd.SetDate(nYear, 12, 31);

	// (z.manning, 11/28/2007) - Load ALL attendance info for every user for the given year so that we don't need
	// to access data again unless the caller wants a different year.
	// (z.manning, 01/03/2008) - PLID 28461 - Added email address.
	// (z.manning, 04/11/2008) - PLID 29628 - Added input date
	// (z.manning, 05/19/2008) - PLID 30105 - Added ParentID
	// (z.manning, 9/2/2008) - PLID 31162 - Make sure load inactive users if they are managers or if they
	// have attendance info for the current year.
	// (z.manning 2008-11-13 12:47) - PLID 31831 - Added PaidTimeOff and UnpaidTimeOff
	// (j.jones 2012-12-24 11:47) - PLID 54317 - ensured we sort by user and date
	ADODB::_RecordsetPtr prsAttendance = CreateParamRecordset(
		"SELECT UsersT.PersonID AS UserID, Date, Vacation, Sick, Other, AttendanceAppointmentsT.Notes \r\n"
		"	, dbo.GetUserDepartmentIDList(UsersT.PersonID) AS DepartmentIDList \r\n"
		"	, VacationAllowance, SickAllowance, AttendanceAppointmentsT.ID AS AppointmentID \r\n"
		"	, DateOfHire, DateOfTerm, UserType, HoursWorked, First, Last, VacationAdjustment, ApprovedBy \r\n"
		"	, dbo.GetUserDepartmentsManagedIDList(UsersT.PersonID) AS ManagedDepartmentsIDList, Email \r\n"
		"	, AttendanceAppointmentsT.Type, AdminNotes, AttendanceAppointmentsT.InputDate, ParentID \r\n"
		"	, PaidTimeOff, UnpaidTimeOff \r\n"
		"FROM UsersT \r\n"
		"INNER JOIN PersonT ON UsersT.PersonID = PersonT.ID \r\n"
		"LEFT JOIN AttendanceAppointmentsT ON UsersT.PersonID = AttendanceAppointmentsT.UserID AND Date BETWEEN {STRING} AND {STRING} \r\n"
		"LEFT JOIN AttendanceAllowanceHistoryT ON UsersT.PersonID = AttendanceAllowanceHistoryT.UserID \r\n"
		"	AND COALESCE(AttendanceAllowanceHistoryT.Year, 0) = \r\n"
		"		(SELECT MAX(Year) FROM AttendanceAllowanceHistoryT WHERE Year <= {INT} AND UserID = UsersT.PersonID) \r\n"
		"LEFT JOIN AttendanceAdjustmentsT ON UsersT.PersonID = AttendanceAdjustmentsT.UserID AND AttendanceAdjustmentsT.Year = {INT} \r\n"
		"WHERE PersonT.ID > 0 AND (PersonT.Archived = 0 OR AttendanceAppointmentsT.ID IS NOT NULL OR \r\n"
		"	UsersT.PersonID IN (SELECT UserID FROM DepartmentManagersT)) \r\n"
		"ORDER BY UsersT.PersonID, AttendanceAppointmentsT.Date; \r\n"
		"\r\n"
		// (z.manning, 02/11/2008) - PLID 28885 - 2nd query to load pay period dates.
		"SELECT EndDate \r\n"
		"FROM AttendancePayPeriodsT \r\n"
		"WHERE DATEPART(yyyy, EndDate) = {INT} \r\n"
		"ORDER BY EndDate; \r\n"
		, FormatDateTimeForSql(dtRangeStart,dtoDate), FormatDateTimeForSql(dtRangeEnd,dtoDate), nYear, nYear, nYear);

	while(!prsAttendance->eof)
	{
		ADODB::FieldsPtr pFields = prsAttendance->Fields;

		// (z.manning, 11/28/2007) - Load all the data for this user.
		long nCurrentUserID = AdoFldLong(pFields, "UserID");

		// (j.jones 2012-12-24 11:51) - PLID 54317 - see if the user already has a pointer in our array
		AttendanceUser *pUser = NULL;
		for(int u=0;u<m_arypAttendanceUsers.GetSize() && pUser == NULL; u++) {
			AttendanceUser *pCheckUser = m_arypAttendanceUsers.GetAt(u);
			if(pCheckUser->m_nUserID == nCurrentUserID) {
				//found the user
				pUser = pCheckUser;
			}
		}

		if(pUser == NULL) {
			//we didn't find a user, so create it
			pUser = new AttendanceUser(this);
			m_arypAttendanceUsers.Add(pUser);
		}

		pUser->m_nUserID = nCurrentUserID;
		pUser->m_strFirstName = AdoFldString(pFields, "First", "");
		pUser->m_strLastName = AdoFldString(pFields, "Last", "");
		if(pFields->GetItem("DateOfHire")->Value.vt == VT_DATE) {
			pUser->m_dtDateOfHire = AdoFldDateTime(pFields, "DateOfHire");
		}
		if(pFields->GetItem("DateOfTerm")->Value.vt == VT_DATE) {
			pUser->m_dtDateOfTermination = AdoFldDateTime(pFields, "DateOfTerm");
		}
		pUser->m_nVacationAllowance = AdoFldLong(pFields, "VacationAllowance", 0);
		pUser->m_nVacationBonus = AdoFldLong(pFields, "VacationAdjustment", 0);
		pUser->m_nSickAllowance = AdoFldLong(pFields, "SickAllowance", 0);
		pUser->m_eType = (EAttendanceUserTypes)AdoFldByte(pFields, "UserType", autSalary);
		// (z.manning, 01/03/2008) - PLID 28461 - Added email address.
		pUser->m_strEmail = AdoFldString(pFields, "Email", "");
		// (z.manning, 11/28/2007) - We have a SQL function to load a semicolon-delimited list of the departments
		// that a user belongs to so that we can easily get that information in the same general loading query.
		// So let's go ahead and parse that list here if we found anything.
		CString strSemiColonDelimitedDeptIDs = AdoFldString(pFields, "DepartmentIDList", "");
		while(strSemiColonDelimitedDeptIDs.GetLength() > 0)
		{
			int nSemiColon = strSemiColonDelimitedDeptIDs.Find(';');
			if(nSemiColon == -1) {
				nSemiColon = strSemiColonDelimitedDeptIDs.GetLength();
			}
			pUser->m_arynDepartmentIDs.Add(AsLong(_bstr_t(strSemiColonDelimitedDeptIDs.Left(nSemiColon))));
			strSemiColonDelimitedDeptIDs.Delete(0, nSemiColon + 1);
		}
		// (z.manning, 01/03/2008) - PLID 28461 - Same thing for departments managed.
		strSemiColonDelimitedDeptIDs = AdoFldString(pFields, "ManagedDepartmentsIDList", "");
		while(strSemiColonDelimitedDeptIDs.GetLength() > 0)
		{
			int nSemiColon = strSemiColonDelimitedDeptIDs.Find(';');
			if(nSemiColon == -1) {
				nSemiColon = strSemiColonDelimitedDeptIDs.GetLength();
			}
			pUser->m_arynManagedDepartmentIDs.Add(AsLong(_bstr_t(strSemiColonDelimitedDeptIDs.Left(nSemiColon))));
			strSemiColonDelimitedDeptIDs.Delete(0, nSemiColon + 1);
		}

		// (z.manning, 11/28/2007) - For as long as the user name stays the same, load attendance appointments
		// for that user.
		DECIMAL decZero;
		while(nCurrentUserID == AdoFldLong(pFields,"UserID"))
		{
			if(pFields->GetItem("Date")->Value.vt == VT_DATE)
			{
				AttendanceData *pData = new AttendanceData;
				pData->nID = AdoFldLong(pFields, "AppointmentID");
				pData->pUser = pUser;
				pData->dtDate = AdoFldDateTime(pFields, "Date");
				pData->cyHoursWorked = ConvertDecimalToCurrency(AdoFldDecimal(pFields, "HoursWorked", decZero));
				pData->cyVacation = ConvertDecimalToCurrency(AdoFldDecimal(pFields, "Vacation", decZero));
				pData->cySick = ConvertDecimalToCurrency(AdoFldDecimal(pFields, "Sick", decZero));
				// (z.manning 2008-11-13 12:48) - PLID 31831 - Paid/unpaid time off fields
				pData->cyPaid = ConvertDecimalToCurrency(AdoFldDecimal(pFields, "PaidTimeOff", decZero));
				pData->cyUnpaid = ConvertDecimalToCurrency(AdoFldDecimal(pFields, "UnpaidTimeOff", decZero));
				pData->cyOther = ConvertDecimalToCurrency(AdoFldDecimal(pFields, "Other", decZero));
				pData->strNotes = AdoFldString(pFields, "Notes", "");
				pData->strAdminNotes = AdoFldString(pFields, "AdminNotes", "");
				pData->varApproverUserID = pFields->GetItem("ApprovedBy")->GetValue();
				pData->eType = (EAttendanceType)AdoFldByte(pFields, "Type");
				// (z.manning, 04/11/2008) - PLID 29628 - Added input date
				_variant_t varInputDate = pFields->GetItem("InputDate")->Value;
				if(varInputDate.vt == VT_DATE) {
					pData->dtInputDate = VarDateTime(varInputDate);
				}
				// (z.manning, 05/19/2008) - PLID 30105 - Added parent ID
				pData->varParentID = pFields->GetItem("ParentID")->GetValue();

				pUser->m_arypAttendanceData.Add(pData);
			}

			prsAttendance->MoveNext();
			if(prsAttendance->eof) {
				break;
			}
		}
	}

	// (z.manning, 02/11/2008) - PLID 28885 - Now load the pay period info
	prsAttendance = prsAttendance->NextRecordset(NULL);
	while(!prsAttendance->eof)
	{
		ADODB::FieldsPtr pflds = prsAttendance->GetFields();
		m_arydtPayrollCloseDates.Add(AdoFldDateTime(pflds, "EndDate"));
		prsAttendance->MoveNext();
	}

	prsAttendance->Close();
}

// (z.manning, 11/28/2007) - This function returns an array of attendance users for the given user IDs.
void AttendanceInfo::GetAttendanceData(IN const CArray<long,long> &arynUserIDs, OUT CArray<AttendanceUser*,AttendanceUser*> &arypUsers)
{
	arypUsers.RemoveAll();
	for(int i = 0; i < m_arypAttendanceUsers.GetSize(); i++)
	{
		long nUserID = m_arypAttendanceUsers.GetAt(i)->m_nUserID;
		for(int j = 0; j < arynUserIDs.GetSize(); j++) 
		{
			if(nUserID == arynUserIDs.GetAt(j))
			{
				AttendanceUser *pUserToAdd = m_arypAttendanceUsers.GetAt(i);
				// (z.manning, 11/20/2007) - Sort the array entries by username.
				for(int nUserIndex = arypUsers.GetSize() - 1; nUserIndex >= 0; nUserIndex--)
				{
					if(pUserToAdd->m_strLastName.CompareNoCase(arypUsers.GetAt(nUserIndex)->m_strLastName) > 0) {
						arypUsers.InsertAt(nUserIndex + 1, pUserToAdd);
						break;
					}
				}
				if(nUserIndex < 0) {
					arypUsers.InsertAt(0, pUserToAdd);
				}
				break;
			}
		}
	}
}

// (z.manning, 11/28/2007) - Returns the AttendanceUser pointer for the the given user ID.
AttendanceUser* AttendanceInfo::GetAttendanceUserByID(long nUserID)
{
	for(int i = 0; i < m_arypAttendanceUsers.GetSize(); i++)
	{
		if(m_arypAttendanceUsers.GetAt(i)->m_nUserID == nUserID) {
			return m_arypAttendanceUsers.GetAt(i);
		}
	}

	ThrowNxException("AttendanceInfo::GetAttendanceUserByID - Failed to find UserID %li", nUserID);
}

// (z.manning, 01/11/2008) - PLID 28461 - This function will send an email to all of given data's user's managers.
void AttendanceInfo::EmailManagers(AttendanceData *pData)
{
	if(pData == NULL) {
		ThrowNxException("AttendanceInfo::EmailManagers - Expected non-null attendance data");
	}

	AttendanceUser *pUser = pData->pUser;
	CStringArray arystrTo;
	for(int nDeptIndex = 0; nDeptIndex < pUser->m_arynDepartmentIDs.GetSize(); nDeptIndex++)
	{
		long nDeptID = pUser->m_arynDepartmentIDs.GetAt(nDeptIndex);
		for(int nUserIndex = 0; nUserIndex < m_arypAttendanceUsers.GetSize(); nUserIndex++)
		{
			AttendanceUser *pTemp = m_arypAttendanceUsers.GetAt(nUserIndex);
			for(int nManagedDeptIndex = 0; nManagedDeptIndex < pTemp->m_arynManagedDepartmentIDs.GetSize(); nManagedDeptIndex++)
			{
				if(nDeptID == pTemp->m_arynManagedDepartmentIDs.GetAt(nManagedDeptIndex)) {
					BOOL bEmailFound = FALSE;
					for(int nEmailIndex = 0; nEmailIndex < arystrTo.GetSize(); nEmailIndex++) {
						if(pTemp->m_strEmail == arystrTo.GetAt(nEmailIndex)) {
							bEmailFound = TRUE;
							break;
						}
					}
					// (z.manning, 01/11/2008) - Only add the email address if it's not already in the list to avoid
					// duplication.
					if(!bEmailFound && !pTemp->m_strEmail.IsEmpty()) {
						arystrTo.Add(pTemp->m_strEmail);
					}
				}
			}
		}
	}

	CString strBody = FormatString("%s %s would like to request the following time off...\r\n\r\n%s", pUser->m_strFirstName, pUser->m_strLastName, pData->GetText());
	SendEmail(NULL, arystrTo, "Time Off Request", strBody);
}

// (z.manning, 02/11/2008) - PLID 28885 - Returns true if the given date is a payroll closing date, false otherwise
BOOL AttendanceInfo::IsPayrollCloseDate(const COleDateTime dtDate)
{
	// (z.manning, 02/12/2008) - Should not be calling this function for a date that's no the current one.
	ASSERT(dtDate.GetYear() == m_nYear);

	for(int nDateIndex = 0; nDateIndex < m_arydtPayrollCloseDates.GetSize(); nDateIndex++)
	{
		COleDateTime dtTemp = m_arydtPayrollCloseDates.GetAt(nDateIndex);
		if(dtTemp == dtDate) {
			return TRUE;
		}
	}

	return FALSE;
}

// (z.manning, 02/12/2008) - PLID 28885 - Returns the most recent payroll closing date before given pivot date.
COleDateTime AttendanceInfo::GetFirstPayrollCloseDateBefore(const COleDateTime dtPivot)
{
	if(dtPivot.GetYear() != m_nYear) {
		AfxThrowNxException("AttendanceInfo::GetFirstPayrollCloseDateBefore - Pivot year (%i) does not match currently loaded year (%i)", dtPivot.GetYear(), m_nYear);
	}

	// (z.manning, 02/12/2008) - PLID 28885 - We maintain the payroll dates in order so start at the end
	// and find the first date that comes before pivot date.
	for(int nDateIndex = m_arydtPayrollCloseDates.GetSize() - 1; nDateIndex >= 0; nDateIndex--)
	{
		COleDateTime dtTemp = m_arydtPayrollCloseDates.GetAt(nDateIndex);
		if(dtTemp < dtPivot) {
			return dtTemp;
		}
	}

	COleDateTime dtInvalid;
	dtInvalid.SetStatus(COleDateTime::invalid);
	return dtInvalid;
}

// (z.manning, 02/12/2008) - PLID 28885 - Returns true if the given date comes before the latest pay period, false otherwise.
BOOL AttendanceInfo::IsDateLocked(const COleDateTime dtDate)
{
	if(m_arydtPayrollCloseDates.GetSize() > 0) {
		// (z.manning, 02/12/2008) - The list is sorted, so pull the last date.
		if(dtDate <= m_arydtPayrollCloseDates.GetAt(m_arydtPayrollCloseDates.GetSize() - 1)) {
			return TRUE;
		}
	}

	// (z.manning, 02/12/2008) - We only have data for one year, so we need to check data if the previous
	// check didn't prove the date is locked if the date we're looking for is in a different year.
	ADODB::_RecordsetPtr prsMaxDate = CreateRecordset("SELECT MAX(EndDate) AS MaxDate FROM AttendancePayPeriodsT");
	if(!prsMaxDate->eof) {
		COleDateTime dtMaxDate = AdoFldDateTime(prsMaxDate, "MaxDate");
		if(dtDate <= dtMaxDate) {
			return TRUE;
		}
	}

	return FALSE;
}

// End class AttendanceInfo
/////////////////////////////////////////////////////////////////////////////////////////////


// (z.manning, 11/28/2007) - This function is so we can use qsort on an array of AttendanceData.
int CompareAttendanceDataByDate(const void *pDataA, const void *pDataB)
{
	AttendanceData **ppDataA = (AttendanceData**)pDataA;
	AttendanceData **ppDataB = (AttendanceData**)pDataB;

	if(*ppDataA != NULL && *ppDataB != NULL)
	{
		COleDateTime dtDateA = (*ppDataA)->dtDate;
		COleDateTime dtDateB = (*ppDataB)->dtDate;

		if(dtDateA > dtDateB) {
			return 1;
		}
		else if(dtDateA < dtDateB) {
			return -1;
		}
		else {
			ASSERT(dtDateA == dtDateB);
			return 0;
		}
	}
	else 
	{
		// (z.manning, 06/25/2007) - Uhh, we shouldn't have a null pointer.
		ASSERT(FALSE);
		return -1;
	}
}

// (z.manning, 11/28/2007) - This function returns the number of users we can show attendance info for
// wihout exceeding the datalist's max number of columns.
int GetMaxVisibleUsers()
{
	// (z.manning, 11/16/2007) - We have a limit of 255 columns in the datalist.
	return (255 - alcBeginDynamicColumns) / aldcDynamicColumnCount;
}

// (z.manning, 11/28/2007) - This function will return the info needed for the custom date range option.
void LoadAttendanceCustomDateRangeProperty(OUT ECustomDateRangeTypes &nType, OUT int &nFrequency)
{
	// (z.manning, 11/28/2007) - This property should be in the format: <type>:<frequency>
	CString strDateRangeProperty = GetRemotePropertyText("AttendanceCustomDateRange", FormatString("%i;2",cdrtMonthly), 0, "<None>", true);
	int nSemicolon = strDateRangeProperty.Find(';');
	if(nSemicolon == -1) {
		ThrowNxException("CAttendanceCustomDateRangeSetupDlg::Load - Invalid date range property");
	}
	nType = (ECustomDateRangeTypes)atoi(strDateRangeProperty.Left(nSemicolon));
	strDateRangeProperty.Delete(0, nSemicolon + 1);
	nFrequency = atoi(strDateRangeProperty);
}

CString FormatAttendanceValue(COleCurrency cy)
{
	CString str;
	if(cy == COleCurrency(0,0)) {
		str = "0";
	}
	else {
		str = FormatCurrencyForInterface(cy, FALSE, TRUE);
		BOOL bEndParenthesis = FALSE;
		if(str.GetAt(str.GetLength() - 1) == ')') {
			str.Delete(str.GetLength() - 1, 1);
			bEndParenthesis = TRUE;
		}
		str.TrimRight('0');
		str.TrimRight('.');
		if(bEndParenthesis) {
			str += ')';
		}
		ASSERT(str.GetLength() > 0);
	}

	return str;
}

void EnsureValidAttendanceString(IN OUT CString &strAttendanceValue, IN const int nMaxValue)
{
	// (z.manning, 12/03/2007) - For starters, make sure it's a valid currency value.
	COleCurrency cy;
	if(!cy.ParseCurrency(strAttendanceValue)) {
		strAttendanceValue = "0";
		return;
	}

	if(cy > COleCurrency(nMaxValue, 0) || cy < COleCurrency(nMaxValue * -1, 0)) {
		strAttendanceValue = "0";
		return;
	}

	// (z.manning, 12/03/2007) - We only store a maximum of 2 decimal places in data, so round if necessary.
	double dblLo = cy.m_cur.Lo / 100.;
	dblLo = atof(FormatString("%.0f", dblLo)) * 100;
	cy.m_cur.Lo = (unsigned long)dblLo;
	strAttendanceValue = FormatAttendanceValue(cy);
}

BOOL IsHoliday(IN const COleDateTime dt)
{
	CString strPlaceholder;
	return IsHoliday(dt, strPlaceholder);
}

BOOL IsHoliday(IN const COleDateTime dt, OUT CString &strHoliday)
{
	// (z.manning, 12/11/2007) - PLID 28218 - For now holidays are just hard-coded in. If we ever release
	// this to clients we would need a way to customize this or perhaps integrate it with a user-specified
	// schedule template.

	// (z.manning, 12/11/2007) - At NexTech, we get the following days off
	// 
	// - New Year's Day
	// - Memorial Day
	// - July 4th
	// - Labor Day
	// - Thanksgiving
	// - Day after Thanksgiving
	// - Christmas Eve				// (d.thompson 2014-02-25) - PLID 61014 - Added
	// - Christmas
	//
	// This function also account for holidays that occur on weekends and will return true if the nearest
	// weekday should be a holiday.

	// New Year's Day
	if( (dt.GetMonth() == 1 && dt.GetDay() == 1 && dt.GetDayOfWeek() >= 2 && dt.GetDayOfWeek() <= 6) ||
		(dt.GetMonth() == 1 && dt.GetDay() == 2 && dt.GetDayOfWeek() == 2) ||
		(dt.GetMonth() == 12 && dt.GetDay() == 31 && dt.GetDayOfWeek() == 6) )
	{
		// Happy new year!
		strHoliday = "New Year's Day";
		return TRUE;
	}

	// July 4th
	if( (dt.GetMonth() == 7 && dt.GetDay() == 4 && dt.GetDayOfWeek() >= 2 && dt.GetDayOfWeek() <= 6) ||
		(dt.GetMonth() == 7 && dt.GetDay() == 5 && dt.GetDayOfWeek() == 2) ||
		(dt.GetMonth() == 7 && dt.GetDay() == 3 && dt.GetDayOfWeek() == 6) )
	{
		strHoliday = "4th of July";
		return TRUE;
	}

	// (d.thompson 2014-02-25) - PLID 61014 - Christmas Eve.  NOTE:  Due to the impending removal of the attendance
	//	system, we are not bothering with the weekend problems here.  It gets weird (what if Christmas is a Monday and already a holiday?), 
	//	so we're just going to ignore it for now.  If for some reason this system is not removed, we'll need to
	//	fix this next year
	if(dt.GetMonth() == 12 && dt.GetDay() == 24 && dt.GetDayOfWeek() >= 2 && dt.GetDayOfWeek() <= 6)
	{
		strHoliday = "Christmas Eve";
		return TRUE;
	}

	// Christmas
	if( (dt.GetMonth() == 12 && dt.GetDay() == 25 && dt.GetDayOfWeek() >= 2 && dt.GetDayOfWeek() <= 6) ||
		(dt.GetMonth() == 12 && dt.GetDay() == 26 && dt.GetDayOfWeek() == 2) ||
		(dt.GetMonth() == 12 && dt.GetDay() == 24 && dt.GetDayOfWeek() == 6) )
	{

		strHoliday = "Christmas";
		return TRUE;
	}

	// Memorial Day (last Monday in May)
	COleDateTime dtMemorialDay;
	dtMemorialDay.SetDate(dt.GetYear(), 6, 1);
	dtMemorialDay = dtMemorialDay - ONE_DAY;
	while(dtMemorialDay.GetDayOfWeek() != 2) {
		dtMemorialDay = dtMemorialDay - ONE_DAY;
	}
	if(dtMemorialDay == dt) {
		strHoliday = "Memorial Day";
		return TRUE;
	}

	// Labor day (first Monday in September)
	COleDateTime dtLaborDay;
	dtLaborDay.SetDate(dt.GetYear(), 9, 1);
	while(dtLaborDay.GetDayOfWeek() != 2) {
		dtLaborDay = dtLaborDay + ONE_DAY;
	}
	if(dtLaborDay == dt) {
		strHoliday = "Labor Day";
		return TRUE;
	}

	// Thanksgiving (4th Thursday in November)
	COleDateTime dtThanksgiving;
	dtThanksgiving.SetDate(dt.GetYear(), 11, 1);
	short nThursdayCount = 0;
	while(nThursdayCount < 4) {
		if(dtThanksgiving.GetDayOfWeek() == 5) {
			nThursdayCount++;
		}

		if(nThursdayCount < 4) {
			dtThanksgiving = dtThanksgiving + ONE_DAY;
		}
	}
	if(dtThanksgiving == dt) {
		// Gobble gobble
		strHoliday = "Thanksgiving";
		return TRUE;
	}
	// (z.manning 2009-08-03 10:34) - PLID 32907 - We now get the Friday after Thanksgiving off 
	// every year instead of having a floating holiday per KM.
	if(dt == (dtThanksgiving + ONE_DAY)) {
		strHoliday.Empty();
		return TRUE;
	}

	return FALSE;
}

void DeleteAssociatedTodos(CArray<long,long> &arynAttendanceDataIDs)
{
	if(arynAttendanceDataIDs.GetSize() == 0) {
		return;
	}

	// (z.manning, 01/11/2008) - PLID 28600 - Need to delete any to-do tasks associated with these attendance
	// appts as completed.
	// (c.haag 2008-06-11 08:41) - PLID 30328 - Delete TodoAssignToT records
	ADODB::_RecordsetPtr prs = CreateParamRecordset(FormatString(
		"SET NOCOUNT ON \r\n"
		"DECLARE @TodoIDs TABLE (TodoID INT NOT NULL PRIMARY KEY) \r\n"
		"INSERT INTO @TodoIDs (TodoID) SELECT TodoID FROM AttendanceToDoLinkT WHERE AttendanceAppointmentID IN (%s) \r\n"
		"DELETE FROM AttendanceToDoLinkT WHERE TodoID IN (SELECT TodoID FROM @TodoIDs) \r\n"
		"DELETE FROM TodoAssignToT WHERE TaskID IN (SELECT TodoID FROM @TodoIDs) \r\n"
		// (c.haag 2008-07-10 15:30) - PLID 30674 - Delete EMR todo table records
		"DELETE FROM EMRTodosT WHERE TaskID IN (SELECT TodoID FROM @TodoIDs) \r\n"
		"DELETE FROM TodoList WHERE TaskID IN (SELECT TodoID FROM @TodoIDs) \r\n"
		"SET NOCOUNT OFF \r\n"
		"SELECT (SELECT COUNT(*) FROM @TodoIDs) AS TodoUpdateCount, (SELECT TOP 1 TodoID FROM @TodoIDs) AS TodoID "
		, ArrayAsString(arynAttendanceDataIDs,false))
		);

	long nTodoUpdateCount = AdoFldLong(prs, "TodoUpdateCount");
	if(nTodoUpdateCount == 1) {
		CClient::RefreshTable(NetUtils::TodoList, AdoFldLong(prs, "TodoID"));
	}
	else if(nTodoUpdateCount > 1) {
		CClient::RefreshTable(NetUtils::TodoList);
	}
}

// (z.manning, 02/13/2008) - PLID 28909 - Determines is a reservation object is an attendance appointment
BOOL IsResAttendanceAppointment(CReservation pRes)
{
	return (IsNexTechInternal() && pRes.GetReservationID() == -1 && pRes.GetTemplateItemID() == -1);
}

// (z.manning, 05/19/2008) - PLID 30105 - Returns true if the given date is a valid vacation day
// i.e. a non-holiday weekday
BOOL IsValidVacationDay(COleDateTime dt)
{
	if(dt.GetDayOfWeek() == 1 || dt.GetDayOfWeek() == 7 || IsHoliday(dt)) {
		return FALSE;
	}

	return TRUE;
}

// (z.manning 2012-03-27 12:00) - PLID 49227 - Row color options
OLE_COLOR GetAccruedRowColor()
{
	return GetRemotePropertyInt("AttendanceAccruedRowColor", RGB(255,220,180), 0, GetCurrentUserName());
}

// (z.manning 2012-03-27 12:00) - PLID 49227 - Row color options
OLE_COLOR GetAvailableRowColor()
{
	return GetRemotePropertyInt("AttendanceAvailableRowColor", RGB(255,220,160), 0, GetCurrentUserName());
}

// (z.manning 2012-03-27 12:00) - PLID 49227 - Row color options
OLE_COLOR GetTotalUsedRowColor()
{
	return GetRemotePropertyInt("AttendanceTotalUsedRowColor", RGB(235,200,250), 0, GetCurrentUserName());
}

// (z.manning 2012-03-27 12:00) - PLID 49227 - Row color options
OLE_COLOR GetBalanceRowColor()
{
	return GetRemotePropertyInt("AttendanceBalanceRowColor", RGB(255,200,120), 0, GetCurrentUserName());
}

// (j.luckoski 2012-07-24 11:34) - PLID 29122 - Audit attendnace using the pUser
// (j.luckoski 2012-09-04 16:57) - PLID 29122 - Added other to the output
// (j.luckoski 2012-09-05 15:20) - PLID 29122 - Added hours worked and hours paid to the output
void AttendanceUser::AuditAttendance(AttendanceUser *pUser, long nID, AuditEventItems aeiAttendanceChange, CString strDate, CString strNotes, CString strVacation, CString strSick, CString strUnpaid, CString strOther, CString strHoursWorked, CString strPaid) 
{
	// (j.luckoski 2012-08-29 15:12) - PLID 29122 - Added in a better old and new value system designed to handle more accurate descriptions of whats going on
	CString strOld = "";
	CString strNew = "";
	if(strVacation == "0") { strVacation = ""; } else {strVacation = "PTO:" + strVacation + " ";}
	if(strSick == "0") { strSick = ""; } else {strSick = "Sick:" + strSick + " ";}
	if(strUnpaid == "0") { strUnpaid = ""; } else {strUnpaid = "UnPaid:" + strUnpaid + " ";}
	if(strOther == "0") { strOther = ""; } else {strOther = "Other:" + strOther + " ";}
	if(strHoursWorked == "0") { strHoursWorked = ""; } else {strHoursWorked = "Worked:" + strHoursWorked + " ";}
	if(strPaid == "0") { strPaid = ""; } else {strPaid = "Paid:" + strPaid + " ";}

	if(aeiAttendanceChange == aeiAttendanceDelete) {
		strOld = "For " + strDate+ " " + strHoursWorked + strVacation + strSick + strPaid + strUnpaid + strOther + "- " + strNotes;
	} else {
		strNew = "For " + strDate + " " + strHoursWorked + strVacation + strSick + strPaid + strUnpaid + strOther + "- " + strNotes;
	}

	AuditEvent(pUser->m_nUserID, pUser->GetFullName(), 
		BeginNewAuditEvent(), aeiAttendanceChange, nID, 
		strOld, strNew,
		aepMedium, aetChanged); 	
}

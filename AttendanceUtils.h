// AttendanceUtils.h
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ATTENDANCEUTILS_H__31E2BB73_7D70_4F00_9FDB_B16BE2DA72D0__INCLUDED_)
#define AFX_ATTENDANCEUTILS_H__31E2BB73_7D70_4F00_9FDB_B16BE2DA72D0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// (c.haag 2010-02-04 12:03) - PLID 37221 - Accesses to reservation objects should be done through the
// CReservation class
//#import "SingleDay.tlb" rename("Delete", "DeleteRes")
#include "Reservation.h"

// (z.manning, 02/28/2008) - PLID 29139 - Utility classes and functions for the attendance sheet

/***********************************************************************************************

(z.manning, 11/28/2007) - PLID 28216 - For the time being the attendance feature is for
internal use only. So, we don't yet have mods for it, so here is all the SQL for the data
structure changes that were made on Internal.
(z.manning, 11/28/2007) - PLID 28218 - If we ever do release this to clients, we would need to
make mods for all of this.


CREATE TABLE AttendanceAppointmentsT
(
	ID int not NULL IDENTITY CONSTRAINT PK_AttendanceAppointmentsT PRIMARY KEY,
	UserID int NOT NULL CONSTRAINT FK_AttendanceAppointmentsT_UsersT FOREIGN KEY REFERENCES UsersT(PersonID),
	Date datetime NOT NULL,
	HoursWorked decimal(4, 2) NOT NULL DEFAULT(0),
	Vacation decimal(4, 2) NOT NULL DEFAULT(0),
	Sick decimal(4, 2) NOT NULL DEFAULT(0),
	PaidTimeOff decimal(4, 2) NOT NULL DEFAULT(0),
	UnpaidTimeOff decimal(4, 2) NOT NULL DEFAULT(0),
	Other decimal(4, 2) NOT NULL DEFAULT(0),
	Notes nvarchar(255) NOT NULL DEFAULT(''),
	ApprovedBy int NULL CONSTRAINT FK_AttendanceAppointmentsT_UsersT_ApprovedBy FOREIGN KEY REFERENCES UsersT(PersonID),
	Type tinyint NOT NULL DEFAULT(1),
	AdminNotes nvarchar(255) NOT NULL DEFAULT(''),
	ParentID INT NULL CONSTRAINT FK_AttendanceAppointmentsT_ParentID REFERENCES AttendanceAppointmentsT(ID)
	CONSTRAINT UQ_AttendanceAppointmentsT_UserID_Date UNIQUE(UserID, Date)
)

CREATE TABLE AttendanceAllowanceHistoryT
(
	UserID int NOT NULL CONSTRAINT FK_AttendanceAllowanceHistoryT_UsersT REFERENCES UsersT(PersonID),
	Year int NOT NULL,
	VacationAllowance int NOT NULL DEFAULT(0),
	SickAllowance int NOT NULL DEFAULT(0),
	UserType tinyint NOT NULL DEFAULT(1),
	CONSTRAINT PK_AttendanceAllowanceHistoryT PRIMARY KEY(UserID, Year)
)

CREATE TABLE AttendanceAdjustmentsT
(
	UserID int NOT NULL CONSTRAINT FK_AttendanceAdjustmentsT_UsersT REFERENCES UsersT(PersonID),
	Year int NOT NULL,
	VacationAdjustment int NOT NULL DEFAULT(0),
	Notes nvarchar(255) NOT NULL DEFAULT(''),
	CONSTRAINT PK_AttendanceAdjustmentsT PRIMARY KEY(UserID, Year)
)

CREATE TABLE AttendanceToDoLinkT
(
	AttendanceAppointmentID INT NOT NULL CONSTRAINT FK_AttendanceToDoLinkT_AttendanceAppointmentsT REFERENCES AttendanceAppointmentsT(ID),
	ToDoID INT NOT NULL CONSTRAINT FK_AttendanceToDoLinkT_ToDoList REFERENCES ToDoList(TaskID),
	CONSTRAINT PK_AttendanceToDoLinkT PRIMARY KEY(AttendanceAppointmentID, ToDoID)
)

CREATE TABLE AttendancePayPeriodsT
(
	ID int NOT NULL CONSTRAINT PK_AttendancePayPeriodsT PRIMARY KEY,
	EndDate datetime NOT NULL
)

CREATE TABLE DepartmentsT
(
	ID INT NOT NULL IDENTITY CONSTRAINT PK_DepartmentsT PRIMARY KEY,
	Name nvarchar(50) NOT NULL DEFAULT('') CONSTRAINT UQ_DepartmentsT_Name UNIQUE
)

CREATE TABLE UserDepartmentLinkT
(
	UserID INT NOT NULL CONSTRAINT FK_UserDepartmentLinkT_UsersT REFERENCES UsersT(PersonID),
	DepartmentID INT NOT NULL CONSTRAINT FK_UserDepartmentLinkT_DepartmentsT REFERENCES DepartmentsT(ID),
	CONSTRAINT PK_UserDepartmentLinkT PRIMARY KEY (UserID, DepartmentID)
)

CREATE TABLE DepartmentManagersT
(
	UserID INT NOT NULL CONSTRAINT FK_DepartmentManagersT_UsersT REFERENCES UsersT(PersonID),
	DepartmentID INT NOT NULL CONSTRAINT FK_DepartmentManagersT_DepartmentsT REFERENCES DepartmentsT(ID),
	CONSTRAINT PK_DepartmentManagersT PRIMARY KEY (UserID, DepartmentID)
)

CREATE TABLE DepartmentResourceLinkT
(
	DepartmentID INT NOT NULL CONSTRAINT FK_DepartmentResourceLinkT_DepartmentsT REFERENCES DepartmentsT(ID),
	ResourceID INT NOT NULL CONSTRAINT FK_DepartmentResourceLinkT_ResourceT REFERENCES ResourceT(ID),
	CONSTRAINT PK_DepartmentResourceLinkT PRIMARY KEY (DepartmentID, ResourceID)
)

CREATE FUNCTION GetUserDepartmentIDList (@nUserID int)
RETURNS nvarchar(4000)
BEGIN
DECLARE @strList nvarchar(4000)
SELECT @strList = COALESCE(@strList + ';', '') + CONVERT(nvarchar(100), UserDepartmentLinkT.DepartmentID)
FROM UserDepartmentLinkT
WHERE UserDepartmentLinkT.UserID = @nUserID
RETURN @strList
END 

CREATE FUNCTION GetUserDepartmentsManagedIDList (@nUserID int)
RETURNS nvarchar(4000)
BEGIN
DECLARE @strList nvarchar(4000)
SELECT @strList = COALESCE(@strList + ';', '') + CONVERT(nvarchar(100), DepartmentManagersT.DepartmentID)
FROM DepartmentManagersT
WHERE DepartmentManagersT.UserID = @nUserID
RETURN @strList
END 

CREATE FUNCTION GetDepartmentManagerIDList (@nDepartmentID int)
RETURNS nvarchar(4000)
BEGIN
DECLARE @strList nvarchar(4000)
SELECT @strList = COALESCE(@strList + ';', '') + CONVERT(nvarchar(100), UserID)
FROM DepartmentManagersT
WHERE DepartmentID = @nDepartmentID
RETURN @strList
END 

CREATE FUNCTION GetDepartmentResourceList (@nDepartmentID int)
RETURNS nvarchar(4000)
BEGIN
DECLARE @strList nvarchar(4000)
SELECT @strList = COALESCE(@strList + ', ', '') + CONVERT(nvarchar(100), ResourceT.Item)
FROM DepartmentResourceLinkT
LEFT JOIN ResourceT ON DepartmentResourceLinkT.ResourceID = ResourceT.ID
WHERE DepartmentResourceLinkT.DepartmentID = @nDepartmentID
RETURN @strList
END 

CREATE FUNCTION GetAttendanceAppointmentTypeID (@nAttendanceAppointmentID int)
RETURNS int
BEGIN
DECLARE @nTypeID int
SET @nTypeID = 
(SELECT CASE 
	WHEN Vacation > 0      THEN (SELECT TOP 1 IntParam FROM ConfigRT WHERE Name = 'AttendanceVacationTypeID')
	WHEN Sick > 0          THEN (SELECT TOP 1 IntParam FROM ConfigRT WHERE Name = 'AttendanceSickTypeID') 
	WHEN PaidTimeOff > 0   THEN (SELECT TOP 1 IntParam FROM ConfigRT WHERE Name = 'AttendancePaidTypeID') 
	WHEN UnpaidTimeOff > 0 THEN (SELECT TOP 1 IntParam FROM ConfigRT WHERE Name = 'AttendanceUnpaidTypeID') 
	WHEN Other > 0         THEN (SELECT TOP 1 IntParam FROM ConfigRT WHERE Name = 'AttendanceOtherTypeID') 
	END
 FROM AttendanceAppointmentsT WHERE ID = @nAttendanceAppointmentID
)
RETURN @nTypeID
END

*******************************************************************************************************************/


#define ONE_DAY		COleDateTimeSpan(1, 0, 0, 0)

#define MIN_YEAR	1753
#define MAX_YEAR	9999

// (z.manning, 12/04/2007) - These are stored as part of a remote property, so leave them alone.
enum ECustomDateRangeTypes
{
	cdrtMonthly = 1,
	cdrtWeekly = 2,
};

// (z.manning, 12/04/2007) - These are stored in data so don't change any existing values.
enum EAttendanceUserTypes
{
	autSalary = 1,
	autHourly,
};

// (z.manning, 02/08/2008) - A way to differentiate between normal vacation and floating holidays
// (and potential future different vacation types).
enum EAttendanceType
{
	atNormal = 1,
	atFloatingHoliday,
};

// (z.manning, 11/28/2007) - The following 2 enums are for the main attendance list in AttendanceDlg.
// Other classes access the datalist, however, which is why the enums are declared here.
enum EAttendanceListColumns
{
	alcDisplayDate,
	alcStartDate,
	alcEndDate,
	alcBeginDynamicColumns,
};
enum EAttendanceListDynamicColumns
{
	aldcPointer = 0,
	aldcHoursWorked,
	aldcPTO,			// (d.thompson 2014-02-27) - PLID 61016 - We named vacation to PTO, so for code clarity I updated the enum as well
	aldcSick,
	// (z.manning 2008-11-13 11:27) - PLID 31831 - Added columns for paid and unpaid time off
	aldcPaid,
	aldcUnpaid,
	aldcOther,
	aldcNotes,
	aldcAdminNotes,
	aldcDynamicColumnCount,
};

class AttendanceInfo;
class AttendanceUser;

// (z.manning, 11/28/2007) - This struct represents an entry in AttendanceAppointmentsT.
class AttendanceData
{
public:
	long nID;
	AttendanceUser *pUser;
	COleDateTime dtDate;
	COleCurrency cyHoursWorked;
	COleCurrency cyVacation;
	COleCurrency cySick;
	// (z.manning 2008-11-13 12:30) - PLID 31831 - Added paid and unpaid
	COleCurrency cyPaid;
	COleCurrency cyUnpaid;
	COleCurrency cyOther;
	_variant_t varApproverUserID;
	CString strNotes;
	CString strAdminNotes;
	EAttendanceType eType;
	// (z.manning, 04/11/2008) - PLID 29628 - Added input date
	COleDateTime dtInputDate;
	// (z.manning, 05/19/2008) - PLID 30105 - Added parent ID
	_variant_t varParentID;

	AttendanceData();

	AttendanceData& AttendanceData::operator=(const AttendanceData &source);

	void SaveToData();
	//(r.wilson - 9/9/2011) PLID 45311 
	void AddTodosToDatabase();
	void DeleteFromData();

	BOOL IsApproved();

	// (z.manning, 01/07/2008) - PLID 28461 - Added function to format the text of the attendance data.
	CString GetText();

	// (z.manning, 05/19/2008) - PLID 30105 - Returns the end date of a group of attendance appointments
	void GetDateRange(OUT COleDateTime &dtStart, OUT COleDateTime &dtEnd);

	// (z.manning, 05/20/2008) - PLID 30105 - Returns true if this attendance appt is part of a group
	BOOL IsPartOfGroup();
};

// (z.manning, 11/28/2007) - The AttendanceUser class stores all of the attendance information for a user.
class AttendanceUser
{
public:
	AttendanceUser(AttendanceInfo *pParentInfo);
	virtual ~AttendanceUser();

	AttendanceInfo *m_pAttendanceInfo;

	long m_nUserID;
	CString m_strFirstName;
	CString m_strLastName;
	COleDateTime m_dtDateOfHire;
	COleDateTime m_dtDateOfTermination;
	CArray<long,long> m_arynDepartmentIDs;
	CArray<long,long> m_arynManagedDepartmentIDs;
	EAttendanceUserTypes m_eType;
	// (z.manning, 01/03/2008) - PLID 28461 - Track the email address for users.
	CString m_strEmail;

	CArray<AttendanceData*,AttendanceData*> m_arypAttendanceData;

	int m_nVacationAllowance;
	int m_nVacationBonus;
	int m_nSickAllowance;

	BOOL m_bSorted;

	BOOL IsManager();

	void AddAttendanceData(AttendanceData *pData);
	void RemoveAttendanceData(AttendanceData *pData);

	// (j.luckoski 2012-07-23 12:14) - PLID 29122 - Audit attendance
	void AuditAttendance(AttendanceUser *pUser, long nID, AuditEventItems aeiAttendanceChange, CString strDate, CString strNotes, CString strVacation, CString strSick, CString strUnpaid, CString strOther, CString strHoursWorked, CString strPaid);

	void ApproveAttendanceData(CArray<AttendanceData*,AttendanceData*> &arypDataToApprove, CWnd *pwndParent);
	void DenyAttendanceData(const CArray<AttendanceData*,AttendanceData*> &arypDataToApprove, CWnd *pwndParent);

	void SortByDate();

	BOOL IsInDepartment(long nDepartmentID);

	AttendanceData* GetAttendanceDataForDate(COleDateTime dtDate);

	CString GetFullName();

	float GetPercentOfYearWorkedAsOf(COleDateTime dtDate);

	void GetCurrentUsedTotals(OUT COleCurrency &cyHoursWorked, OUT COleCurrency &cyVacation, OUT COleCurrency &cySick, OUT COleCurrency &cyPaid, OUT COleCurrency &cyUnpaid, OUT COleCurrency &cyOther, const IN COleDateTime dtRangeStart, const IN COleDateTime dtRangeEnd, BOOL bIgnoreFloatingHolidays = TRUE, BOOL bUnapprovedOnly = FALSE);
	COleCurrency GetTotalHoursWorked();
	COleCurrency GetTotalUsedVacation(BOOL bIgnoreFloatingHolidays = TRUE);
	COleCurrency GetTotalUsedSick();
	// (z.manning 2008-11-13 13:55) - PLID 31831 - Paid/Unpaid
	COleCurrency GetTotalUsedPaid();
	COleCurrency GetTotalUsedUnpaid();
	COleCurrency GetTotalUsedOther();
	COleCurrency GetTotalHoursWorkedByDate(COleDateTime dtStart, COleDateTime dtEnd);
	// (z.manning, 05/19/2008) - PLID 30102 - Added parameter to ignore approved time off
	COleCurrency GetTotalUsedVacationByDate(COleDateTime dtStart, COleDateTime dtEnd, BOOL bIgnoreFloatingHolidays = TRUE, BOOL bUnapprovedOnly = FALSE);
	COleCurrency GetTotalUsedSickByDate(COleDateTime dtStart, COleDateTime dtEnd, BOOL bUnapprovedOnly = FALSE);
	// (z.manning 2008-11-13 13:56) - PLID 31831 - Paid/Unpaid
	COleCurrency GetTotalUsedPaidByDate(COleDateTime dtStart, COleDateTime dtEnd, BOOL bUnapprovedOnly = FALSE);
	COleCurrency GetTotalUsedUnpaidByDate(COleDateTime dtStart, COleDateTime dtEnd, BOOL bUnapprovedOnly = FALSE);
	COleCurrency GetTotalUsedOtherByDate(COleDateTime dtStart, COleDateTime dtEnd, BOOL bUnapprovedOnly = FALSE);

	void GetAvailableTotals(OUT int &nVacation, OUT int &nSick);
	int GetAvailableVacation();
	int GetAvailableSick();

	void GetBalances(OUT COleCurrency &cyVacation, OUT COleCurrency &cySick);
	COleCurrency GetBalanceVacation();
	COleCurrency GetBalanceSick();
	// (z.manning 2008-11-21 12:26) - PLID 32139
	CString GetBalanceVacationText();

	void GetAccruedTotals(OUT COleCurrency &cyVacation, OUT COleCurrency &cySick);
	COleCurrency GetAccruedVacation();
	COleCurrency GetAccruedSick();

	int GetFloatingHolidayCount();
};

// (z.manning, 11/28/2007) - The AttendanceInfo class stores all attendance info for a given year.
class AttendanceInfo
{
public:
	AttendanceInfo();
	virtual ~AttendanceInfo();

	CArray<AttendanceUser*,AttendanceUser*> m_arypAttendanceUsers;

	long m_nYear;

	void LoadAllByYear(const int nYear);

	void GetAttendanceData(IN const CArray<long,long> &arynUserIDs, OUT CArray<AttendanceUser*,AttendanceUser*> &arypUsers);

	AttendanceUser* GetAttendanceUserByID(long nUserID);

	// (z.manning, 01/02/2008) - PLID 28461 - Function to email the current user's managers regarding given attendance data.
	void EmailManagers(AttendanceData *pData);

	// (z.manning, 02/11/2008) - PLID 28885 - Returns true if the given date is a payroll closing date, false otherwise
	BOOL IsPayrollCloseDate(const COleDateTime dtDate);

	// (z.manning, 02/12/2008) - PLID 28885 - Returns the most recent payroll closing date before given pivot date.
	COleDateTime GetFirstPayrollCloseDateBefore(const COleDateTime dtPivot);

	// (z.manning, 02/12/2008) - PLID 28885 - Returns true if the given date comes before the latest pay period, false otherwise.
	BOOL IsDateLocked(const COleDateTime dtDate);

protected:
	// (z.manning, 02/12/2008) - PLID 28885 - Stoer the dates the a pay period has ended on. Also, this
	// array should ALWAYS be sorted in ascending order.
	CArray<COleDateTime,COleDateTime> m_arydtPayrollCloseDates;

	void Clear();
};

struct AttendanceDepartment
{
	long nID;
	CString strName;
};

// (z.manning, 11/28/2007) - This function is so we can use qsort on an array of AttendanceData.
int CompareAttendanceDataByDate(const void *pDataA, const void *pDataB);

// (z.manning, 11/28/2007) - This function returns the number of users we can show attendance info for
// wihout exceeding the datalist's max number of columns.
int GetMaxVisibleUsers();

// (z.manning, 11/28/2007) - This function will return the info needed for the custom date range option.
void LoadAttendanceCustomDateRangeProperty(OUT ECustomDateRangeTypes &nType, OUT int &nFrequency);

CString FormatAttendanceValue(COleCurrency cy);

void EnsureValidAttendanceString(IN OUT CString &strAttendanceValue, IN const int nMaxValue);

BOOL IsHoliday(IN const COleDateTime dt);
BOOL IsHoliday(IN const COleDateTime dt, OUT CString &strHoliday);

// (z.manning, 01/11/2008) - PLID 28600 - Function to delete any to-do tasks associated with the given attendance
// appointment IDs
void DeleteAssociatedTodos(CArray<long,long> &arynAttendanceDataIDs);

// (z.manning, 02/13/2008) - PLID 28909 - Determines is a reservation object is an attendance appointment
BOOL IsResAttendanceAppointment(CReservation pRes);

// (z.manning, 05/19/2008) - PLID 30105 - Returns true if the given date is a valid vacation day
// i.e. a non-holiday weekday
BOOL IsValidVacationDay(COleDateTime dt);

// (z.manning 2012-03-27 12:00) - PLID 49227 - Row color options
OLE_COLOR GetAccruedRowColor();
OLE_COLOR GetAvailableRowColor();
OLE_COLOR GetTotalUsedRowColor();
OLE_COLOR GetBalanceRowColor();

#endif // !defined(AFX_ATTENDANCEUTILS_H__31E2BB73_7D70_4F00_9FDB_B16BE2DA72D0__INCLUDED_)

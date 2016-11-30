// FilterDetailDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FilterDetail.h"
#include "FilterFieldInfo.h"
//NOTE: This include, and the ADD_FILTER_FIELDS, really should be in "filterdetailcallback.cpp"
#include "Groups.h"
//NOTE: This include would not be part of filterdetailcallback, this dialog is part of the filtering structure.
#include "RemovedFilterDlg.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////////////
// Here is where we define all our valid field types

// First we need a helper macro
#define P(strliteral)		strliteral "\0"

//We need this class because we want some way of ASSERTing if the array is invalid, and since we're not in a function,
//we have to instantiate an object which will validate the array.
#ifdef _DEBUG
class CAssertValidArray {
public:
	CAssertValidArray(CFilterFieldInfo *ar, long nArrayCount) {CFilterDetail::AssertValidArray(ar, nArrayCount);}
};
#endif

// Then we need the macros to cleanly create our static field type list
#define BEGIN_ADD_FILTER_FIELDS(flds)				const CFilterFieldInfo CFilterDetail::flds[] = {
#define ADD_FILTER_FIELD							CFilterFieldInfo
#define ADD_FILTER_FIELD_OBSOLETE					CFilterFieldInfo
#define ADD_FILTER_FIELD_REMOVED					CFilterFieldInfo
#ifdef _DEBUG
#define END_ADD_FILTER_FIELDS(flds, fldcnt)			}; const long CFilterDetail::fldcnt = sizeof(CFilterDetail::flds) / sizeof(CFilterFieldInfo); CAssertValidArray ava##flds((CFilterFieldInfo*)CFilterDetail::flds, CFilterDetail::fldcnt)
#else
#define END_ADD_FILTER_FIELDS(flds, fldcnt)			}; const long CFilterDetail::fldcnt = sizeof(CFilterDetail::flds) / sizeof(CFilterFieldInfo)
#endif

/*
///////strParams////////////////////////////////////////////////////////////////
	ftComboSelect: the strParams parameter must be a double-null-terminated
		string of the form:
			P("SELECT {FieldXYZ} AS ID, {FieldABC} AS Name FROM {JoinClause}")
		The idea is that you must at least have one field "AS ID" and one 
		field "AS Name" so they can be referred to as ID and Name without 
		a prefix (such as SomeTableT.Name).  When the WHERE clause is 
		generated later, it will be something like 
			sprintf(buf, "WHERE %s = %s", strFieldInternal, paramsRS.GetFieldAsString("ID"));
		Be sure to use the P(str) macro because the string must be 
		terminated with two nulls

	ftComboValues: the strParams parameter must be a string of NULL-
		DELIMITED items terminated by two consecutive NULLs.  The items 
		must be given in pairs of apparent values and their corresponding 
		data representations, in that order.  For example, to make the 
		combo show True and False, with the corresponding integer values 
		of -1 and 0, the parameter string would need to be: "True" 
		followed by "-1" followed by "False" followed by "0".  A 
		convenient way to create a null-delimited list within a string is
		to use the P Macro like so:
			P("True") P("-1") P("False") P("0")
		It is important to note that any surrounding characters should be
		included within the string.  So, for example, if you want the 
		list to have two entries, "Bob's Birthday" and "Steve's Birthday", 
		you would do the following:
			P("Bob's Birthday") P("#10/20/1976#") P("Steve's Birthday") P("#05/31/1977#")
		Be careful to note that strings would be similar, i.e.
			P("Bob's Last Name") P("""Cardillo""") P("Steve's Last Name") P("""Koncal""")

	ftText, ftDate, and ftNumber do not take strParams right now
////////////////////////////////////////////////////////////////////////////////

////////strJoinClause///////////////////////////////////////////////////////////
	The strJoinClause can be NULL, or can be the actual join clause.  If 
	it is the actual join clause, make sure to exclude the left side of 
	it, with the assumption that all join dependencies (described later)
	are already on the left, like in the following example:
		use
			"CustomDataT ON Patients.ID = CustomDataT.PatientID"
		instead of
			"Patients LEFT JOIN CustomDataT ON Patients.ID = CustomDataT.PatientID"
	All joins are implicitly LEFT JOINs.
///////////////////////////////////////////////////////////////////////////////

////////nJoinDependsOn/////////////////////////////////////////////////////////
	If a join has a dependency besides the Patients table ("Patients" is 
	the table off of which all future joins are based), it may be 
	supplied in the nJoinDependsOn parameter.  Zero is the default and 
	indicates that the join simply depends on "Patients".  A number 
	greater than zero must correspond to the nInfoId parameter of the 
	filter field upon which this filter field depends.  So, to continue
	the above example, to include a field which joins to CustomDataT,
	such as [Contact Info], one might set the nJoinDependsOn parameter
	equal to the id of the filter field that used the above join clause
		"CustomDataT ON Patients.ID = CustomDataT.PatientID"
	Let's say it was filter field 23.  Our current field might use a 
	strJoinClause of 
		"[Contact Info] AS CustomContactInfo1 ON CustomDataT.CustomContact1 = CustomContactInfo1.ID"
	and a nJoinDepensOn of 
		23.
	This will force the CustomDataT join to be appended first, so that 
	CustomContactInfo1 may make use of it.

	The nJoinDependsOn field may also be used to avoid repeated 
	strJoinClauses.  So if many fields make use of the same join clause, 
	only one field needs to have the actual strJoinClause, while the 
	remaining fields may reference the nInfoID of the first.
///////////////////////////////////////////////////////////////////////////////

*/

// For convenience keep track of the highest info ID (keep adding to 
// this as you add filters) because all filter IDs MUST be unique
//(J.Camacho 2013-03-12 10:28) - PLID 53866 - 992 to 993. replace this when number is updated.
// (s.tullis 2014-06-25 13:56) - PLID 42233 - 2027 to 2028
// (v.maida 2014-08-08 15:06) - PLID 34948 - Changed from 2028 to 2030 (because two new filters were added). Increment this number when new filters are added.
unsigned long FILTER_FIELD_NEXT_INFO_ID = 2031;

// IMPORTANT NOTE:
//		Unless you REALLY know what you're doing, don't mess with the IDs 
//		here.  Not only must they be unique against each other, they must 
//		be unique against the path.  In other words, the ID 43 may be 
//		referenced in places other than within this cpp file.

// Create the field type list
//3-13-03 m.cable - filters that use dates now use the AsDateNoTime(datetime) function to get the date for comparison
// (z.manning, 05/25/2006) - Now there's a AsTimeNoDate(datetime) function for filter fields that use times.
// (a.wilson 2012-2-24) PLID 48398 - Removed ':' from filters to fix compatibility change errors.

BEGIN_ADD_FILTER_FIELDS(g_FilterFields)
	
	//ADD_FILTER_FIELD(1,   "Has Appointment w/ Date", "dbo.AsDateNoTime(AppointmentsT.StartTime)", fboPerson, foAll, foGreaterEqual, ftDate, NULL, NULL, FILTER_DEPENDS_ON_BASE, "AppointmentsT WITH(NOLOCK)", "AppointmentsT.PatientID = PersonT.ID AND Status <> 4"), //see note at the top about converting date
	ADD_FILTER_FIELD_OBSOLETE(1, "{277, 2048, \"0-{278, PRE_EXISTING_FIELD_OPERATOR, \"\"PRE_EXISTING_SELECTED_VALUE\"\"}\"}"),
	//ADD_FILTER_FIELD(3,   "Has Appointment w/ StartTime", "Convert(varchar,AppointmentsT.StartTime,8)", fboPerson, foAll, foEqual, ftTime, NULL, NULL, FILTER_DEPENDS_ON_BASE, "AppointmentsT WITH(NOLOCK)", "AppointmentsT.PatientID = PersonT.ID AND Status <> 4"),
	ADD_FILTER_FIELD_OBSOLETE(3, "{277, 2048, \"0-{279, PRE_EXISTING_FIELD_OPERATOR, \"\"PRE_EXISTING_SELECTED_VALUE\"\"}\"}"),
	//ADD_FILTER_FIELD(4,   "Has Appointment w/ EndTime", "Convert(varchar,AppointmentsT.EndTime,8)", fboPerson, foAll, foEqual, ftTime, NULL, NULL, FILTER_DEPENDS_ON_BASE, "AppointmentsT WITH(NOLOCK)", "AppointmentsT.PatientID = PersonT.ID AND Status <> 4"),
	ADD_FILTER_FIELD_OBSOLETE(4, "{277, 2048, \"0-{280, PRE_EXISTING_FIELD_OPERATOR, \"\"PRE_EXISTING_SELECTED_VALUE\"\"}\"}"),
	//ADD_FILTER_FIELD(183, "Has Appointment w/ Purpose", "AppointmentPurposeT.PurposeID", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID, Name FROM AptPurposeT ORDER BY Name"), NULL, FILTER_DEPENDS_ON_BASE, "AppointmentsT WITH(NOLOCK) INNER JOIN AppointmentPurposeT WITH(NOLOCK) ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID", "AppointmentsT.PatientID = PersonT.ID AND Status <> 4"),
	ADD_FILTER_FIELD_OBSOLETE(183, "{277, 2048, \"0-{281, PRE_EXISTING_FIELD_OPERATOR, \"\"PRE_EXISTING_SELECTED_VALUE\"\"}\"}"),
	//ADD_FILTER_FIELD(5,   "Has Appointment w/ Purpose Name", "AptPurposeT.Name", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, FILTER_DEPENDS_ON_BASE, "AppointmentsT WITH(NOLOCK) INNER JOIN AppointmentPurposeT WITH(NOLOCK) ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID INNER JOIN AptPurposeT WITH(NOLOCK) ON AppointmentPurposeT.PurposeID = AptPurposeT.ID", "AppointmentsT.PatientID = PersonT.ID AND Status <> 4"),
	ADD_FILTER_FIELD_OBSOLETE(5, "{277, 2048, \"0-{282, PRE_EXISTING_FIELD_OPERATOR, \"\"PRE_EXISTING_SELECTED_VALUE\"\"}\"}"),
	//ADD_FILTER_FIELD(184, "Has Appointment w/ Type", "AppointmentsT.AptTypeID", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID, Name FROM AptTypeT ORDER BY Name"), NULL, FILTER_DEPENDS_ON_BASE, "AppointmentsT WITH(NOLOCK)", "AppointmentsT.PatientID = PersonT.ID AND Status <> 4"),
	ADD_FILTER_FIELD_OBSOLETE(184, "{277, 2048, \"0-{283, PRE_EXISTING_FIELD_OPERATOR, \"\"PRE_EXISTING_SELECTED_VALUE\"\"}\"}"),
	//ADD_FILTER_FIELD(7,   "Has Appointment w/ Type Name", "AptTypeT.Name", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, FILTER_DEPENDS_ON_BASE, "AppointmentsT WITH(NOLOCK) INNER JOIN AptTypeT WITH(NOLOCK) ON AppointmentsT.AptTypeID = AptTypeT.ID", "AppointmentsT.PatientID = PersonT.ID AND Status <> 4"),
	ADD_FILTER_FIELD_OBSOLETE(7, "{277, 2048, \"0-{284, PRE_EXISTING_FIELD_OPERATOR, \"\"PRE_EXISTING_SELECTED_VALUE\"\"}\"}"),
	//ADD_FILTER_FIELD(2,   "Has Appointment w/ Notes", "AppointmentsT.Notes", fboPerson, foAll, foGreaterEqual, ftText, NULL, NULL, FILTER_DEPENDS_ON_BASE, "AppointmentsT WITH(NOLOCK)", "AppointmentsT.PatientID = PersonT.ID AND Status <> 4"),
	ADD_FILTER_FIELD_OBSOLETE(2, "{277, 2048, \"0-{285, PRE_EXISTING_FIELD_OPERATOR, \"\"PRE_EXISTING_SELECTED_VALUE\"\"}\"}"),
	//ADD_FILTER_FIELD(158, "Has Appointment w/ Resource", "AppointmentResourceT.ResourceID", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID AS ID, Item AS Name FROM ResourceT WHERE (SELECT MAX(Relevence) FROM UserResourcesT WHERE ResourceID = ResourceT.ID) >= 0 OR (SELECT MAX(Relevence) FROM ResourceViewDetailsT WHERE ResourceID = ResourceT.ID) >= 0 ORDER BY Item") , NULL, FILTER_DEPENDS_ON_BASE, "AppointmentsT INNER JOIN AppointmentResourceT ON AppointmentsT.ID = AppointmentResourceT.AppointmentID", "AppointmentsT.PatientID = PersonT.ID AND Status <> 4"),
	ADD_FILTER_FIELD_OBSOLETE(158, "{277, 2048, \"0-{286, PRE_EXISTING_FIELD_OPERATOR, \"\"PRE_EXISTING_SELECTED_VALUE\"\"}\"}"),
	//ADD_FILTER_FIELD(6,   "Has Appointment w/ Resource Name", "ResourceT.Item", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, FILTER_DEPENDS_ON_BASE, "AppointmentsT WITH(NOLOCK) INNER JOIN AppointmentResourceT WITH(NOLOCK) ON AppointmentsT.ID = AppointmentResourceT.AppointmentID INNER JOIN ResourceT WITH(NOLOCK) ON AppointmentResourceT.ResourceID = ResourceT.ID", "AppointmentsT.PatientID = PersonT.ID AND Status <> 4"),
	ADD_FILTER_FIELD_OBSOLETE(6, "{277, 2048, \"0-{287, PRE_EXISTING_FIELD_OPERATOR, \"\"PRE_EXISTING_SELECTED_VALUE\"\"}\"}"),
	//ADD_FILTER_FIELD(262, "Has Appointment w/ Show State", "AppointmentsT.ShowState", fboPerson, foEqual|foNotEqual, foEqual, ftComboValues, P("Pending") P("0") P("In") P("1") P("Out") P("2") P("No Show") P("3"), NULL, FILTER_DEPENDS_ON_BASE, "AppointmentsT WITH(NOLOCK)", "AppointmentsT.PatientID = PersonT.ID AND Status <> 4"),
	ADD_FILTER_FIELD_OBSOLETE(262, "{277, 2048, \"0-{288, PRE_EXISTING_FIELD_OPERATOR, \"\"PRE_EXISTING_SELECTED_VALUE\"\"}\"}"),
	//ADD_FILTER_FIELD(265, "Has Appointment w/  Location", "AppointmentsT.LocationID", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID, Name FROM LocationsT WHERE Managed = 1 ORDER BY Name"), NULL, FILTER_DEPENDS_ON_BASE, "AppointmentsT WITH(NOLOCK)", "AppointmentsT.PatientID = PersonT.ID AND Status <> 4"),
	ADD_FILTER_FIELD_OBSOLETE(265, "{277, 2048, \"0-{289, PRE_EXISTING_FIELD_OPERATOR, \"\"PRE_EXISTING_SELECTED_VALUE\"\"}\"}"),

	ADD_FILTER_FIELD(218, "Next Appointment Date", "dbo.AsDateNoTime(NextAppointmentQ.StartTime)", fboPerson, foAll, foGreaterEqual, ftDate, NULL, //see note at the top about converting date
				"(SELECT AppointmentsT.* FROM (SELECT AppointmentsT.PatientID AS PersonID, "
				"CONVERT(int, SUBSTRING(MIN(CONVERT(nvarchar, AppointmentsT.StartTime, 121) + ' ' + CONVERT(nvarchar, AppointmentsT.ID)), 25, 80)) AS ResID "
				"FROM AppointmentsT WITH(NOLOCK) WHERE Status <> 4 AND StartTime >= GETDATE() GROUP BY AppointmentsT.PatientID) NextAppointmentConnectQ "
				"LEFT JOIN AppointmentsT WITH(NOLOCK) ON NextAppointmentConnectQ.ResID = AppointmentsT.ID) NextAppointmentQ ON PersonT.ID = NextAppointmentQ.PatientID"),
	ADD_FILTER_FIELD(219, "Next Appointment Start Time", "dbo.AsTimeNoDate(NextAppointmentQ.StartTime)", fboPerson, foAll, foEqual, ftTime, NULL, NULL, 218),
	ADD_FILTER_FIELD(220, "Next Appointment End Time", "dbo.AsTimeNoDate(NextAppointmentQ.EndTime)", fboPerson, foAll, foEqual, ftTime, NULL, NULL, 218),
	ADD_FILTER_FIELD(393, "Next Appointment Arrival Time", "dbo.AsTimeNoDate(NextAppointmentQ.ArrivalTime)", fboPerson, foAll, foEqual, ftTime, NULL, NULL, 218),
	ADD_FILTER_FIELD(221, "Next Appointment Has Purpose", "AppointmentPurposeT.PurposeID", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID, Name FROM AptPurposeT ORDER BY Name"), "AppointmentPurposeT ON NextAppointmentQ.ID = AppointmentPurposeT.AppointmentID", 218, NULL, NULL, NULL, NULL, 0, P("ID") P("Name") P("AptPurposeT")),
	ADD_FILTER_FIELD(222, "Next Appointment Has Purpose Name", "AptPurposeT.Name", fboPerson, foAll, foBeginsWith, ftText, NULL, "AptPurposeT ON AppointmentPurposeT.PurposeID = AptPurposeT.ID", 221),
	ADD_FILTER_FIELD(223, "Next Appointment Type", "NextAppointmentQ.AptTypeID", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID, Name FROM AptTypeT ORDER BY Name"), NULL, 218, NULL, NULL, NULL, NULL, 0, P("ID") P("Name") P("AptTypeT")),
	ADD_FILTER_FIELD(224, "Next Appointment Type Name", "AptTypeT.Name", fboPerson, foAll, foBeginsWith, ftText, NULL, "AptTypeT ON NextAppointmentQ.AptTypeID = AptTypeT.ID", 218),
	ADD_FILTER_FIELD(225, "Next Appointment Notes", "NextAppointmentQ.Notes", fboPerson, foAll, foGreaterEqual, ftText, NULL, NULL, 218),
	ADD_FILTER_FIELD(226, "Next Appointment Has Resource", "AppointmentResourceT.ResourceID", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID AS ID, Item AS Name FROM ResourceT WHERE (SELECT MAX(Relevence) FROM UserResourcesT WHERE ResourceID = ResourceT.ID) >= 0 OR (SELECT MAX(Relevence) FROM ResourceViewDetailsT WHERE ResourceID = ResourceT.ID) >= 0 ORDER BY Item") , "AppointmentResourceT ON NextAppointmentQ.ID = AppointmentResourceT.AppointmentID", 218, NULL, NULL, NULL, NULL, 0, P("ID") P("Item") P("ResourceT") P("(SELECT MAX(Relevence) FROM UserResourcesT WHERE ResourceID = ResourceT.ID) >= 0 OR (SELECT MAX(Relevence) FROM ResourceViewDetailsT WHERE ResourceID = ResourceT.ID) >= 0")),
	ADD_FILTER_FIELD(227, "Next Appointment Has Resource Name", "ResourceT.Item", fboPerson, foAll, foBeginsWith, ftText, NULL, "ResourceT ON AppointmentResourceT.ResourceID = ResourceT.ID", 226),
	ADD_FILTER_FIELD(263, "Next Appointment Show State", "NextAppointmentQ.ShowState", fboPerson, foEqual|foNotEqual, foEqual, ftComboSelect, P("SELECT ID, Name FROM AptShowStateT"), NULL, 218, NULL, NULL, NULL, NULL, 0, P("ID") P("Name") P("AptShowStateT")),
	ADD_FILTER_FIELD(264, "Next Appointment Location", "NextAppointmentQ.LocationID", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID, Name FROM LocationsT WHERE Managed = 1 AND TypeID = 1 ORDER BY Name"), NULL, 218, NULL, NULL, NULL, NULL, 0, P("ID") P("Name") P("LocationsT") P("Managed = 1 AND TypeID = 1")),

	ADD_FILTER_FIELD(350, "Count Of No Show Appointments", "COALESCE(NoShowApptTotalsQ.NoShowCount,0)", fboPerson, foAllGreater|foAllLess|foEqual|foNotEqual, foEqual, ftNumber, NULL,
				"(SELECT PatientID, Count(ID) AS NoShowCount FROM AppointmentsT WITH(NOLOCK) WHERE Status <> 4 AND ShowState = 3 GROUP BY AppointmentsT.PatientID) NoShowApptTotalsQ ON PersonT.ID = NoShowApptTotalsQ.PatientID"),

	ADD_FILTER_FIELD(297, "Last Appointment Date", "dbo.AsDateNoTime(LastAppointmentQ.StartTime)", fboPerson, foAll, foGreaterEqual, ftDate, NULL, //see note at the top about converting date
				"(SELECT AppointmentsT.* FROM (SELECT AppointmentsT.PatientID AS PersonID, "
				"CONVERT(int, SUBSTRING(MAX(CONVERT(nvarchar, AppointmentsT.StartTime, 121) + ' ' + CONVERT(nvarchar, AppointmentsT.ID)), 25, 80)) AS ResID "
				"FROM AppointmentsT WITH(NOLOCK) WHERE Status <> 4 AND StartTime < GETDATE() GROUP BY AppointmentsT.PatientID) LastAppointmentConnectQ "
				"LEFT JOIN AppointmentsT WITH(NOLOCK) ON LastAppointmentConnectQ.ResID = AppointmentsT.ID) LastAppointmentQ ON PersonT.ID = LastAppointmentQ.PatientID"),
	ADD_FILTER_FIELD(298, "Last Appointment Start Time", "dbo.AsTimeNoDate(LastAppointmentQ.StartTime)", fboPerson, foAll, foEqual, ftTime, NULL, NULL, 297),
	ADD_FILTER_FIELD(299, "Last Appointment End Time", "dbo.AsTimeNoDate(LastAppointmentQ.EndTime)", fboPerson, foAll, foEqual, ftTime, NULL, NULL, 297),
	ADD_FILTER_FIELD(394, "Last Appointment Arrival Time", "dbo.AsTimeNoDate(LastAppointmentQ.ArrivalTime)", fboPerson, foAll, foEqual, ftTime, NULL, NULL, 297),
	ADD_FILTER_FIELD(300, "Last Appointment Has Purpose", "AppointmentPurposeT.PurposeID", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID, Name FROM AptPurposeT ORDER BY Name"), "AppointmentPurposeT ON LastAppointmentQ.ID = AppointmentPurposeT.AppointmentID", 297, NULL, NULL, NULL, NULL, 0, P("ID") P("Name") P("AptPurposeT")),
	ADD_FILTER_FIELD(301, "Last Appointment Has Purpose Name", "AptPurposeT.Name", fboPerson, foAll, foBeginsWith, ftText, NULL, "AptPurposeT ON AppointmentPurposeT.PurposeID = AptPurposeT.ID", 300),
	ADD_FILTER_FIELD(302, "Last Appointment Type", "LastAppointmentQ.AptTypeID", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID, Name FROM AptTypeT ORDER BY Name"), NULL, 297, NULL, NULL, NULL, NULL, 0, P("ID") P("Name") P("AptTypeT")),
	ADD_FILTER_FIELD(303, "Last Appointment Type Name", "AptTypeT.Name", fboPerson, foAll, foBeginsWith, ftText, NULL, "AptTypeT ON LastAppointmentQ.AptTypeID = AptTypeT.ID", 297),
	ADD_FILTER_FIELD(304, "Last Appointment Notes", "LastAppointmentQ.Notes", fboPerson, foAll, foGreaterEqual, ftText, NULL, NULL, 297),
	ADD_FILTER_FIELD(305, "Last Appointment Has Resource", "AppointmentResourceT.ResourceID", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID AS ID, Item AS Name FROM ResourceT WHERE (SELECT MAX(Relevence) FROM UserResourcesT WHERE ResourceID = ResourceT.ID) >= 0 OR (SELECT MAX(Relevence) FROM ResourceViewDetailsT WHERE ResourceID = ResourceT.ID) >= 0 ORDER BY Item") , "AppointmentResourceT ON LastAppointmentQ.ID = AppointmentResourceT.AppointmentID", 297, NULL, NULL, NULL, NULL, 0, P("ID") P("Item") P("ResourceT") P("(SELECT MAX(Relevence) FROM UserResourcesT WHERE ResourceID = ResourceT.ID) >= 0 OR (SELECT MAX(Relevence) FROM ResourceViewDetailsT WHERE ResourceID = ResourceT.ID) >= 0")),
	ADD_FILTER_FIELD(306, "Last Appointment Has Resource Name", "ResourceT.Item", fboPerson, foAll, foBeginsWith, ftText, NULL, "ResourceT ON AppointmentResourceT.ResourceID = ResourceT.ID", 305),
	ADD_FILTER_FIELD(307, "Last Appointment Show State", "LastAppointmentQ.ShowState", fboPerson, foEqual|foNotEqual, foEqual, ftComboSelect, P("SELECT ID, Name FROM AptShowStateT"), NULL, 297, NULL, NULL, NULL, NULL, 0, P("ID") P("Name") P("AptShowStateT")),
	ADD_FILTER_FIELD(308, "Last Appointment Location", "LastAppointmentQ.LocationID", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID, Name FROM LocationsT WHERE Managed = 1 AND TypeID = 1 ORDER BY Name"), NULL, 297, NULL, NULL, NULL, NULL, 0, P("ID") P("Name") P("LocationsT") P("Managed = 1 AND TypeID = 1")),

	// This provides a way to see if a given appointment has any or all of thechosen properties, in other words ALL of these fields are tested at once for each appointment, if a single appointment meets all of the fields in the filter then the patient is included in the filter
	//ADD_FILTER_FIELD(240, "APPOINTMENT: Date", "dbo.AsDateNoTime(AppointmentsT.StartTime)", fboPerson, foAll, foGreaterEqual, ftDate, NULL, "(SELECT * FROM AppointmentsT WITH(NOLOCK) WHERE Status <> 4) AS AppointmentsT ON PersonT.ID = AppointmentsT.PatientID"),  //see note at the top about converting date
	ADD_FILTER_FIELD_REMOVED("APPOINTMENT: Date", 240, "WARNING: The obsolete filter field \"APPOINTMENT: Date\" could not be loaded. "
				"Please modify this filter to use the \"Has Appointment\" filter field, with appropriate subfilters. "
				"You will not be able to save this filter until the obsolete field has been removed.", "NexTech_Practice_Manual.chm", "System_Setup/Letter_Writing_Setup/create_an_appointment_based_filter.htm"),
	//ADD_FILTER_FIELD(241, "APPOINTMENT: StartTime", "Convert(varchar,AppointmentsT.StartTime,8)", fboPerson, foAll, foEqual, ftTime, NULL, NULL, 240),
	ADD_FILTER_FIELD_REMOVED("APPOINTMENT: StartTime", 241, "WARNING: The obsolete filter field \"APPOINTMENT: StartTime\" could not be loaded. "
				"Please modify this filter to use the \"Has Appointment\" filter field, with appropriate subfilters. "
				"You will not be able to save this filter until the obsolete field has been removed.", "NexTech_Practice_Manual.chm", "System_Setup/Letter_Writing_Setup/create_an_appointment_based_filter.htm"),
	//ADD_FILTER_FIELD(242, "APPOINTMENT: EndTime", "Convert(varchar,AppointmentsT.EndTime,8)", fboPerson, foAll, foEqual, ftTime, NULL, NULL, 240),
	ADD_FILTER_FIELD_REMOVED("APPOINTMENT: EndTime", 242, "WARNING: The obsolete filter field \"APPOINTMENT: EndTime\" could not be loaded. "
				"Please modify this filter to use the \"Has Appointment\" filter field, with appropriate subfilters. "
				"You will not be able to save this filter until the obsolete field has been removed.", "NexTech_Practice_Manual.chm", "System_Setup/Letter_Writing_Setup/create_an_appointment_based_filter.htm"),
	//ADD_FILTER_FIELD(243, "APPOINTMENT: Has Purpose", "AppointmentPurposeT.PurposeID", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID, Name FROM AptPurposeT ORDER BY Name"), "AppointmentPurposeT ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID", 240),
	ADD_FILTER_FIELD_REMOVED("APPOINTMENT: Has Purpose", 243, "WARNING: The obsolete filter field \"APPOINTMENT: Has Purpose\" could not be loaded. "
				"Please modify this filter to use the \"Has Appointment\" filter field, with appropriate subfilters. "
				"You will not be able to save this filter until the obsolete field has been removed.", "NexTech_Practice_Manual.chm", "System_Setup/Letter_Writing_Setup/create_an_appointment_based_filter.htm"),
	//ADD_FILTER_FIELD(244, "APPOINTMENT: Has Purpose Name", "AptPurposeT.Name", fboPerson, foAll, foBeginsWith, ftText, NULL, "AptPurposeT ON AppointmentPurposeT.PurposeID = AptPurposeT.ID", 243),
	ADD_FILTER_FIELD_REMOVED("APPOINTMENT: Has Purpose Name", 244, "WARNING: The obsolete filter field \"APPOINTMENT: Has Purpose Name\" could not be loaded. "
				"Please modify this filter to use the \"Has Appointment\" filter field, with appropriate subfilters. "
				"You will not be able to save this filter until the obsolete field has been removed.", "NexTech_Practice_Manual.chm", "System_Setup/Letter_Writing_Setup/create_an_appointment_based_filter.htm"),
	//ADD_FILTER_FIELD(245, "APPOINTMENT: Type", "AppointmentsT.AptTypeID", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID, Name FROM AptTypeT ORDER BY Name"), NULL, 240),
	ADD_FILTER_FIELD_REMOVED("APPOINTMENT: Type", 245, "WARNING: The obsolete filter field \"APPOINTMENT: Type\" could not be loaded. "
				"Please modify this filter to use the \"Has Appointment\" filter field, with appropriate subfilters. "
				"You will not be able to save this filter until the obsolete field has been removed.", "NexTech_Practice_Manual.chm", "System_Setup/Letter_Writing_Setup/create_an_appointment_based_filter.htm"),
	//ADD_FILTER_FIELD(246, "APPOINTMENT: Type Name", "AptTypeT.Name", fboPerson, foAll, foBeginsWith, ftText, NULL, "AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID", 240),
	ADD_FILTER_FIELD_REMOVED("APPOINTMENT: Type Name", 246, "WARNING: The obsolete filter field \"APPOINTMENT: Type Name\" could not be loaded. "
				"Please modify this filter to use the \"Has Appointment\" filter field, with appropriate subfilters. "
				"You will not be able to save this filter until the obsolete field has been removed.", "NexTech_Practice_Manual.chm", "System_Setup/Letter_Writing_Setup/create_an_appointment_based_filter.htm"),
	//ADD_FILTER_FIELD(247, "APPOINTMENT: Notes", "AppointmentsT.Notes", fboPerson, foAll, foGreaterEqual, ftText, NULL, NULL, 240),
	ADD_FILTER_FIELD_REMOVED("APPOINTMENT: Notes", 247, "WARNING: The obsolete filter field \"APPOINTMENT: Notes\" could not be loaded. "
				"Please modify this filter to use the \"Has Appointment\" filter field, with appropriate subfilters. "
				"You will not be able to save this filter until the obsolete field has been removed.", "NexTech_Practice_Manual.chm", "System_Setup/Letter_Writing_Setup/create_an_appointment_based_filter.htm"),
	//ADD_FILTER_FIELD(248, "APPOINTMENT: Has Resource", "AppointmentResourceT.ResourceID", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID AS ID, Item AS Name FROM ResourceT WHERE (SELECT MAX(Relevence) FROM UserResourcesT WHERE ResourceID = ResourceT.ID) >= 0 OR (SELECT MAX(Relevence) FROM ResourceViewDetailsT WHERE ResourceID = ResourceT.ID) >= 0 ORDER BY Item") , "AppointmentResourceT ON AppointmentsT.ID = AppointmentResourceT.AppointmentID", 240),
	ADD_FILTER_FIELD_REMOVED("APPOINTMENT: Has Resource", 248, "WARNING: The obsolete filter field \"APPOINTMENT: Has Resource\" could not be loaded. "
				"Please modify this filter to use the \"Has Appointment\" filter field, with appropriate subfilters. "
				"You will not be able to save this filter until the obsolete field has been removed.", "NexTech_Practice_Manual.chm", "System_Setup/Letter_Writing_Setup/create_an_appointment_based_filter.htm"),
	//ADD_FILTER_FIELD(249, "APPOINTMENT: Has Resource Name", "ResourceT.Item", fboPerson, foAll, foBeginsWith, ftText, NULL, "ResourceT ON AppointmentResourceT.ResourceID = ResourceT.ID", 248),
	ADD_FILTER_FIELD_REMOVED("APPOINTMENT: Has Resource Name", 249, "WARNING: The obsolete filter field \"APPOINTMENT: Has Resource Name\" could not be loaded. "
				"Please modify this filter to use the \"Has Appointment\" filter field, with appropriate subfilters. "
				"You will not be able to save this filter until the obsolete field has been removed.", "NexTech_Practice_Manual.chm", "System_Setup/Letter_Writing_Setup/create_an_appointment_based_filter.htm"),
	//ADD_FILTER_FIELD(261, "APPOINTMENT: Show State", "AppointmentsT.ShowState", fboPerson, foEqual|foNotEqual, foEqual, ftComboValues, P("Pending") P("0") P("In") P("1") P("Out") P("2") P("No Show") P("3"), NULL, 240),
	ADD_FILTER_FIELD_OBSOLETE("APPOINTMENT: Show State", 261, "WARNING: The obsolete filter field \"APPOINTMENT: Show State\" could not be loaded. "
				"Please modify this filter to use the \"Has Appointment\" filter field, with appropriate subfilters. "
				"You will not be able to save this filter until the obsolete field has been removed.", "NexTech_Practice_Manual.chm", "System_Setup/Letter_Writing_Setup/create_an_appointment_based_filter.htm"),
	//ADD_FILTER_FIELD(266, "APPOINTMENT: Location", "AppointmentsT.LocationID", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID, Name FROM LocationsT WHERE Managed = 1 ORDER BY Name"), NULL, 240),
	ADD_FILTER_FIELD_REMOVED("APPOINTMENT: Location", 266, "WARNING: The obsolete filter field \"APPOINTMENT: Location\" could not be loaded. "
				"Please modify this filter to use the \"Has Appointment\" filter field, with appropriate subfilters. "
				"You will not be able to save this filter until the obsolete field has been removed.", "NexTech_Practice_Manual.chm", "System_Setup/Letter_Writing_Setup/create_an_appointment_based_filter.htm"),

	ADD_FILTER_FIELD(23, "REPLACE_FIELD_NAME(Custom CheckBox 1):SELECT Name FROM CustomFieldsT WHERE ID = 41", "CASE WHEN CustomCheck1.IntParam <> 0 THEN -1 ELSE 0 END", fboPerson, foEqual, foEqual, ftComboValues, P("True") P("-1") P("False") P("0"), "CustomFieldDataT CustomCheck1 ON PersonT.ID = CustomCheck1.PersonID AND CustomCheck1.FieldID = 41"),
	ADD_FILTER_FIELD(24, "REPLACE_FIELD_NAME(Custom CheckBox 2):SELECT Name FROM CustomFieldsT WHERE ID = 42", "CASE WHEN CustomCheck2.IntParam <> 0 THEN -1 ELSE 0 END", fboPerson, foEqual, foEqual, ftComboValues, P("True") P("-1") P("False") P("0"), "CustomFieldDataT CustomCheck2 ON PersonT.ID = CustomCheck2.PersonID AND CustomCheck2.FieldID = 42"),
	ADD_FILTER_FIELD(25, "REPLACE_FIELD_NAME(Custom CheckBox 3):SELECT Name FROM CustomFieldsT WHERE ID = 43", "CASE WHEN CustomCheck3.IntParam <> 0 THEN -1 ELSE 0 END", fboPerson, foEqual, foEqual, ftComboValues, P("True") P("-1") P("False") P("0"), "CustomFieldDataT CustomCheck3 ON PersonT.ID = CustomCheck3.PersonID AND CustomCheck3.FieldID = 43"),
	ADD_FILTER_FIELD(26, "REPLACE_FIELD_NAME(Custom CheckBox 4):SELECT Name FROM CustomFieldsT WHERE ID = 44", "CASE WHEN CustomCheck4.IntParam <> 0 THEN -1 ELSE 0 END", fboPerson, foEqual, foEqual, ftComboValues, P("True") P("-1") P("False") P("0"), "CustomFieldDataT CustomCheck4 ON PersonT.ID = CustomCheck4.PersonID AND CustomCheck4.FieldID = 44"),
	ADD_FILTER_FIELD(27, "REPLACE_FIELD_NAME(Custom CheckBox 5):SELECT Name FROM CustomFieldsT WHERE ID = 45", "CASE WHEN CustomCheck5.IntParam <> 0 THEN -1 ELSE 0 END", fboPerson, foEqual, foEqual, ftComboValues, P("True") P("-1") P("False") P("0"), "CustomFieldDataT CustomCheck5 ON PersonT.ID = CustomCheck5.PersonID AND CustomCheck5.FieldID = 45"),
	ADD_FILTER_FIELD(28, "REPLACE_FIELD_NAME(Custom CheckBox 6):SELECT Name FROM CustomFieldsT WHERE ID = 46", "CASE WHEN CustomCheck6.IntParam <> 0 THEN -1 ELSE 0 END", fboPerson, foEqual, foEqual, ftComboValues, P("True") P("-1") P("False") P("0"), "CustomFieldDataT CustomCheck6 ON PersonT.ID = CustomCheck6.PersonID AND CustomCheck6.FieldID = 46"),
	// (j.armen 2011-06-27 16:54) - PLID 44253 - Updated filters to use new Custom List structure
	ADD_FILTER_FIELD(29, "REPLACE_FIELD_NAME(Custom Combo 1):SELECT Name + ' (any)' AS Name FROM CustomFieldsT WHERE ID = 21", "CustomCombo1.CustomListItemsID", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID, [Text] AS NAME FROM CustomListItemsT WHERE CustomFieldID = 21 ORDER BY [Text]"), "CustomListDataT CustomCombo1 ON PersonT.ID = CustomCombo1.PersonID AND CustomCombo1.FieldID = 21", FILTER_DEPENDS_ON_BASE, NULL, NULL, NULL, NULL, 0, P("ID") P("Text") P("CustomListItemsT") P("CustomFieldID = 21")),
	ADD_FILTER_FIELD(30, "REPLACE_FIELD_NAME(Custom Combo 2):SELECT Name + ' (any)' AS Name FROM CustomFieldsT WHERE ID = 22", "CustomCombo2.CustomListItemsID", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID, [Text] AS NAME FROM CustomListItemsT WHERE CustomFieldID = 22 ORDER BY [Text]"), "CustomListDataT CustomCombo2 ON PersonT.ID = CustomCombo2.PersonID AND CustomCombo2.FieldID = 22", FILTER_DEPENDS_ON_BASE, NULL, NULL, NULL, NULL, 0, P("ID") P("Text") P("CustomListItemsT") P("CustomFieldID = 22")),
	ADD_FILTER_FIELD(31, "REPLACE_FIELD_NAME(Custom Combo 3):SELECT Name + ' (any)' AS Name FROM CustomFieldsT WHERE ID = 23", "CustomCombo3.CustomListItemsID", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID, [Text] AS NAME FROM CustomListItemsT WHERE CustomFieldID = 23 ORDER BY [Text]"), "CustomListDataT CustomCombo3 ON PersonT.ID = CustomCombo3.PersonID AND CustomCombo3.FieldID = 23", FILTER_DEPENDS_ON_BASE, NULL, NULL, NULL, NULL, 0, P("ID") P("Text") P("CustomListItemsT") P("CustomFieldID = 23")),
	ADD_FILTER_FIELD(32, "REPLACE_FIELD_NAME(Custom Combo 4):SELECT Name + ' (any)' AS Name FROM CustomFieldsT WHERE ID = 24", "CustomCombo4.CustomListItemsID", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID, [Text] AS NAME FROM CustomListItemsT WHERE CustomFieldID = 24 ORDER BY [Text]"), "CustomListDataT CustomCombo4 ON PersonT.ID = CustomCombo4.PersonID AND CustomCombo4.FieldID = 24", FILTER_DEPENDS_ON_BASE, NULL, NULL, NULL, NULL, 0, P("ID") P("Text") P("CustomListItemsT") P("CustomFieldID = 24")),
	ADD_FILTER_FIELD(33, "REPLACE_FIELD_NAME(Custom Combo 5):SELECT Name + ' (any)' AS Name FROM CustomFieldsT WHERE ID = 25", "CustomCombo5.CustomListItemsID", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID, [Text] AS NAME FROM CustomListItemsT WHERE CustomFieldID = 25 ORDER BY [Text]"), "CustomListDataT CustomCombo5 ON PersonT.ID = CustomCombo5.PersonID AND CustomCombo5.FieldID = 25", FILTER_DEPENDS_ON_BASE, NULL, NULL, NULL, NULL, 0, P("ID") P("Text") P("CustomListItemsT") P("CustomFieldID = 25")),
	ADD_FILTER_FIELD(34, "REPLACE_FIELD_NAME(Custom Combo 6):SELECT Name + ' (any)' AS Name FROM CustomFieldsT WHERE ID = 26", "CustomCombo6.CustomListItemsID", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID, [Text] AS NAME FROM CustomListItemsT WHERE CustomFieldID = 26 ORDER BY [Text]"), "CustomListDataT CustomCombo6 ON PersonT.ID = CustomCombo6.PersonID AND CustomCombo6.FieldID = 26", FILTER_DEPENDS_ON_BASE, NULL, NULL, NULL, NULL, 0, P("ID") P("Text") P("CustomListItemsT") P("CustomFieldID = 26")),
	ADD_FILTER_FIELD(35, "REPLACE_FIELD_NAME(Custom Date 1):SELECT Name FROM CustomFieldsT WHERE ID = 51", "dbo.AsDateNoTime(CustomDate1.DateParam)", fboPerson, foAll, foEqual, ftDate, NULL, "CustomFieldDataT CustomDate1 ON PersonT.ID = CustomDate1.PersonID AND CustomDate1.FieldID = 51"),	//see note at the top about converting date
	ADD_FILTER_FIELD(36, "REPLACE_FIELD_NAME(Custom Date 2):SELECT Name FROM CustomFieldsT WHERE ID = 52", "dbo.AsDateNoTime(CustomDate2.DateParam)", fboPerson, foAll, foEqual, ftDate, NULL, "CustomFieldDataT CustomDate2 ON PersonT.ID = CustomDate2.PersonID AND CustomDate2.FieldID = 52"),	//see note at the top about converting date
	ADD_FILTER_FIELD(37, "REPLACE_FIELD_NAME(Custom Date 3):SELECT Name FROM CustomFieldsT WHERE ID = 53", "dbo.AsDateNoTime(CustomDate3.DateParam)", fboPerson, foAll, foEqual, ftDate, NULL, "CustomFieldDataT CustomDate3 ON PersonT.ID = CustomDate3.PersonID AND CustomDate3.FieldID = 53"),	//see note at the top about converting date
	ADD_FILTER_FIELD(38, "REPLACE_FIELD_NAME(Custom Date 4):SELECT Name FROM CustomFieldsT WHERE ID = 54", "dbo.AsDateNoTime(CustomDate4.DateParam)", fboPerson, foAll, foEqual, ftDate, NULL, "CustomFieldDataT CustomDate4 ON PersonT.ID = CustomDate4.PersonID AND CustomDate4.FieldID = 54"),	//see note at the top about converting date
	ADD_FILTER_FIELD(39, "REPLACE_FIELD_NAME(Custom Memo 1):SELECT Name FROM CustomFieldsT WHERE ID = 17", "CustomMemo1.TextParam", fboPerson, foAll, foBeginsWith, ftText, NULL, "CustomFieldDataT CustomMemo1 ON PersonT.ID = CustomMemo1.PersonID AND CustomMemo1.FieldID = 17"),
	ADD_FILTER_FIELD(40, "REPLACE_FIELD_NAME(Custom Text 1):SELECT Name FROM CustomFieldsT WHERE ID = 11", "CustomText1.TextParam", fboPerson, foAll, foBeginsWith, ftText, NULL, "CustomFieldDataT CustomText1 ON PersonT.ID = CustomText1.PersonID AND CustomText1.FieldID = 11"),
	ADD_FILTER_FIELD(41, "REPLACE_FIELD_NAME(Custom Text 2):SELECT Name FROM CustomFieldsT WHERE ID = 12", "CustomText2.TextParam", fboPerson, foAll, foBeginsWith, ftText, NULL, "CustomFieldDataT CustomText2 ON PersonT.ID = CustomText2.PersonID AND CustomText2.FieldID = 12"),
	ADD_FILTER_FIELD(42, "REPLACE_FIELD_NAME(Custom Text 3):SELECT Name FROM CustomFieldsT WHERE ID = 13", "CustomText3.TextParam", fboPerson, foAll, foBeginsWith, ftText, NULL, "CustomFieldDataT CustomText3 ON PersonT.ID = CustomText3.PersonID AND CustomText3.FieldID = 13"),
	ADD_FILTER_FIELD(43, "REPLACE_FIELD_NAME(Custom Text 4):SELECT Name FROM CustomFieldsT WHERE ID = 14", "CustomText4.TextParam", fboPerson, foAll, foBeginsWith, ftText, NULL, "CustomFieldDataT CustomText4 ON PersonT.ID = CustomText4.PersonID AND CustomText4.FieldID = 14"),
	ADD_FILTER_FIELD(44, "REPLACE_FIELD_NAME(Custom Text 5):SELECT Name FROM CustomFieldsT WHERE ID = 15", "CustomText5.TextParam", fboPerson, foAll, foBeginsWith, ftText, NULL, "CustomFieldDataT CustomText5 ON PersonT.ID = CustomText5.PersonID AND CustomText5.FieldID = 15"),
	ADD_FILTER_FIELD(45, "REPLACE_FIELD_NAME(Custom Text 6):SELECT Name FROM CustomFieldsT WHERE ID = 16", "CustomText6.TextParam", fboPerson, foAll, foBeginsWith, ftText, NULL, "CustomFieldDataT CustomText6 ON PersonT.ID = CustomText6.PersonID AND CustomText6.FieldID = 16"),
	ADD_FILTER_FIELD(402, "REPLACE_FIELD_NAME(Custom Text 7):SELECT Name FROM CustomFieldsT WHERE ID = 90", "CustomText7.TextParam", fboPerson, foAll, foBeginsWith, ftText, NULL, "CustomFieldDataT CustomText7 ON PersonT.ID = CustomText7.PersonID AND CustomText7.FieldID = 90"), // (a.walling 2007-07-03 09:36) - PLID 15491 - Added more custom text fields
	ADD_FILTER_FIELD(403, "REPLACE_FIELD_NAME(Custom Text 8):SELECT Name FROM CustomFieldsT WHERE ID = 91", "CustomText8.TextParam", fboPerson, foAll, foBeginsWith, ftText, NULL, "CustomFieldDataT CustomText8 ON PersonT.ID = CustomText8.PersonID AND CustomText8.FieldID = 91"),
	ADD_FILTER_FIELD(404, "REPLACE_FIELD_NAME(Custom Text 9):SELECT Name FROM CustomFieldsT WHERE ID = 92", "CustomText9.TextParam", fboPerson, foAll, foBeginsWith, ftText, NULL, "CustomFieldDataT CustomText9 ON PersonT.ID = CustomText9.PersonID AND CustomText9.FieldID = 92"),
	ADD_FILTER_FIELD(405, "REPLACE_FIELD_NAME(Custom Text 10):SELECT Name FROM CustomFieldsT WHERE ID = 93", "CustomText10.TextParam", fboPerson, foAll, foBeginsWith, ftText, NULL, "CustomFieldDataT CustomText10 ON PersonT.ID = CustomText10.PersonID AND CustomText10.FieldID = 93"),
	ADD_FILTER_FIELD(406, "REPLACE_FIELD_NAME(Custom Text 11):SELECT Name FROM CustomFieldsT WHERE ID = 94", "CustomText11.TextParam", fboPerson, foAll, foBeginsWith, ftText, NULL, "CustomFieldDataT CustomText11 ON PersonT.ID = CustomText11.PersonID AND CustomText11.FieldID = 94"),
	ADD_FILTER_FIELD(407, "REPLACE_FIELD_NAME(Custom Text 12):SELECT Name FROM CustomFieldsT WHERE ID = 95", "CustomText12.TextParam", fboPerson, foAll, foBeginsWith, ftText, NULL, "CustomFieldDataT CustomText12 ON PersonT.ID = CustomText12.PersonID AND CustomText12.FieldID = 95"),
	
	ADD_FILTER_FIELD(56, "Custom Contact", "CustomContactQ.IntParam", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, 
		P("SELECT CustomContactsListingQ.ID AS ID, CustomContactsListingQ.[Last] + ', ' + CustomContactsListingQ.[First] + ' ' + CustomContactsListingQ.Middle AS Name FROM "
		"(SELECT PersonT.* FROM PersonT LEFT JOIN ContactsT ON PersonT.ID = ContactsT.PersonID LEFT JOIN SupplierT ON PersonT.ID = SupplierT.PersonID LEFT JOIN ReferringPhyST ON PersonT.ID = ReferringPhyST.PersonID LEFT JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID LEFT JOIN UsersT ON PersonT.ID = UsersT.PersonID AND UsersT.PersonID > 0 "
		"WHERE (ContactsT.PersonID Is Not Null OR SupplierT.PersonID Is Not Null OR ReferringPhyST.PersonID Is Not Null OR ProvidersT.PersonID Is Not Null OR UsersT.PersonID Is Not Null)) AS CustomContactsListingQ ORDER BY [Last] ASC, [First] ASC, [Middle] ASC"), "CustomFieldDataT AS CustomContactQ WITH(NOLOCK) ON PersonT.ID = CustomContactQ.PersonID AND CustomContactQ.FieldID = 31", FILTER_DEPENDS_ON_BASE, NULL, NULL, NULL, NULL, 0, 
		P("CustomContactsListingQ.ID") P("CustomContactsListingQ.[Last] + ', ' + CustomContactsListingQ.[First] + ' ' + CustomContactsListingQ.Middle") P("(SELECT PersonT.* FROM PersonT LEFT JOIN ContactsT ON PersonT.ID = ContactsT.PersonID LEFT JOIN SupplierT ON PersonT.ID = SupplierT.PersonID LEFT JOIN ReferringPhyST ON PersonT.ID = ReferringPhyST.PersonID LEFT JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID LEFT JOIN UsersT ON PersonT.ID = UsersT.PersonID AND UsersT.PersonID > 0") P("(ContactsT.PersonID Is Not Null OR SupplierT.PersonID Is Not Null OR ReferringPhyST.PersonID Is Not Null OR ProvidersT.PersonID Is Not Null OR UsersT.PersonID Is Not Null)) AS CustomContactsListingQ")),
	ADD_FILTER_FIELD(206, "Custom Contact Prefix", "CustomContactInfo1.Prefix", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 46),
	ADD_FILTER_FIELD(46, "Custom Contact First Name", "CustomContactInfo1.[First]", fboPerson, foAll, foBeginsWith, ftText, NULL, "(SELECT PersonT.*, PrefixT.Prefix FROM PersonT WITH(NOLOCK) LEFT JOIN PrefixT ON PersonT.PrefixID = PrefixT.ID) AS CustomContactInfo1 ON CustomContactQ.IntParam = CustomContactInfo1.ID", 56),
	ADD_FILTER_FIELD(47, "Custom Contact Middle Name", "CustomContactInfo1.Middle", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 46),
	ADD_FILTER_FIELD(48, "Custom Contact Last Name", "CustomContactInfo1.[Last]", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 46),
	ADD_FILTER_FIELD(207, "Custom Contact Title", "CustomContactInfo1.Title", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 46),
	ADD_FILTER_FIELD(49, "Custom Contact Address1", "CustomContactInfo1.Address1", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 46),
	ADD_FILTER_FIELD(50, "Custom Contact Address2", "CustomContactInfo1.Address2", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 46),
	ADD_FILTER_FIELD(51, "Custom Contact City", "CustomContactInfo1.City", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 46),
	ADD_FILTER_FIELD(52, "Custom Contact State", "CustomContactInfo1.State", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 46),
	ADD_FILTER_FIELD(53, "Custom Contact Zipcode", "CustomContactInfo1.Zip", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 46),
	ADD_FILTER_FIELD(54, "Custom Contact Phone", "(Replace(Replace(Replace(Replace(CustomContactInfo1.WorkPhone,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL, 46),
	ADD_FILTER_FIELD(55, "Custom Contact Ext", "CustomContactInfo1.Extension", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 46),
	
	ADD_FILTER_FIELD(159, "Doctor", "PatientsT.MainPhysician", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT PersonT.[Last] + ', ' + PersonT.[First] + ' ' + PersonT.Middle AS Name, PersonT.ID AS ID FROM PersonT WITH(NOLOCK) INNER JOIN ProvidersT WITH(NOLOCK) ON PersonT.ID = ProvidersT.PersonID ORDER BY PersonT.[Last] ASC, PersonT.[First], PersonT.Middle"), "PatientsT WITH(NOLOCK) ON PersonT.ID = PatientsT.PersonID", FILTER_DEPENDS_ON_BASE, NULL, NULL, NULL, NULL, 0, P("PersonT.ID") P("PersonT.[Last] + ', ' + PersonT.[First] + ' ' + PersonT.Middle") P("PersonT WITH(NOLOCK) INNER JOIN ProvidersT WITH(NOLOCK) ON PersonT.ID = ProvidersT.PersonID")),
	ADD_FILTER_FIELD(208, "Doctor Prefix", "ProviderPersonT.Prefix", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 57),
	ADD_FILTER_FIELD(58, "Doctor First Name", "ProviderPersonT.[First]", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 57),
	ADD_FILTER_FIELD(59, "Doctor Middle Name", "ProviderPersonT.Middle", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 57),
	ADD_FILTER_FIELD(60, "Doctor Last Name", "ProviderPersonT.[Last]", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 57),
	ADD_FILTER_FIELD(61, "Doctor Title", "ProviderPersonT.Title", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 57),
	ADD_FILTER_FIELD(199, "Doctor Address1", "ProviderPersonT.Address1", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 57),
	ADD_FILTER_FIELD(200, "Doctor Address2", "ProviderPersonT.Address2", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 57),
	ADD_FILTER_FIELD(201, "Doctor City", "ProviderPersonT.City", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 57),
	ADD_FILTER_FIELD(202, "Doctor State", "ProviderPersonT.State", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 57),
	ADD_FILTER_FIELD(203, "Doctor Zipcode", "ProviderPersonT.Zip", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 57),
	ADD_FILTER_FIELD(204, "Doctor Phone", "(Replace(Replace(Replace(Replace(ProviderPersonT.WorkPhone,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL, 57),
	ADD_FILTER_FIELD(205, "Doctor Ext", "ProviderPersonT.Extension", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 57),

	ADD_FILTER_FIELD(57, "Doctor Birth Date", "dbo.AsDateNoTime(ProviderPersonT.Birthdate)", fboPerson, foAll, foEqual, ftDate, NULL, "(SELECT PersonT.*, PrefixT.Prefix FROM PersonT WITH(NOLOCK) LEFT JOIN PrefixT ON PersonT.PrefixID = PrefixT.ID) AS ProviderPersonT ON PatientsT.MainPhysician = ProviderPersonT.ID", 159),	//see note at the top about converting date
	
	ADD_FILTER_FIELD(62, "Emergency Contact First Name", "PersonT.EmergFirst", fboPerson),
	ADD_FILTER_FIELD(63, "Emergency Contact Last Name", "PersonT.EmergLast", fboPerson),
	ADD_FILTER_FIELD(64, "Emergency Contact Home Phone", "(Replace(Replace(Replace(Replace(PersonT.EmergHPhone,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL),
	ADD_FILTER_FIELD(65, "Emergency Contact Work Phone", "(Replace(Replace(Replace(Replace(PersonT.EmergWPhone,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL),
	
	ADD_FILTER_FIELD(66, "Employer Company Name", "PersonT.Company", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 159),
	ADD_FILTER_FIELD(67, "Employer First Name", "PatientsT.EmployerFirst", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 159),
	ADD_FILTER_FIELD(68, "Employer Middle Name", "PatientsT.EmployerMiddle", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 159),
	ADD_FILTER_FIELD(69, "Employer Last Name", "PatientsT.EmployerLast", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 159),
	ADD_FILTER_FIELD(70, "Employer Address1", "PatientsT.EmployerAddress1", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 159),
	ADD_FILTER_FIELD(71, "Employer Address2", "PatientsT.EmployerAddress2", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 159),
	ADD_FILTER_FIELD(72, "Employer City", "PatientsT.EmployerCity", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 159),
	ADD_FILTER_FIELD(73, "Employer State", "PatientsT.EmployerState", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 159),
	ADD_FILTER_FIELD(74, "Employer ZipCode", "PatientsT.EmployerZip", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 159),

	ADD_FILTER_FIELD(162, "ID", "PatientsT.UserDefinedID", fboPerson, foAll, foEqual, ftNumber, NULL, NULL, 159),
	ADD_FILTER_FIELD(75, "Active", "CASE WHEN PersonT.Archived <> 0 THEN 1 ELSE 0 END", fboPerson, foEqual|foBlank|foNotBlank, foEqual, ftComboValues, P("Active") P("0") P("Inactive") P("1"), NULL),
	ADD_FILTER_FIELD(76, "Prefix", "PrefixT.Prefix", fboPerson, foAll, foBeginsWith, ftText, NULL, "PrefixT ON PersonT.PrefixID = PrefixT.ID"),
	ADD_FILTER_FIELD(77, "First Name", "PersonT.[First]", fboPerson),
	ADD_FILTER_FIELD(78, "Middle Name", "PersonT.Middle", fboPerson),
	ADD_FILTER_FIELD(79, "Last Name", "PersonT.[Last]", fboPerson),
	ADD_FILTER_FIELD(80, "Title", "PersonT.Title", fboPerson),
	ADD_FILTER_FIELD(81, "Nick Name", "PatientsT.Nickname", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 159),
	ADD_FILTER_FIELD(82, "Address1", "PersonT.Address1", fboPerson),
	ADD_FILTER_FIELD(83, "Address2", "PersonT.Address2", fboPerson),
	ADD_FILTER_FIELD(84, "City", "PersonT.City", fboPerson),
	ADD_FILTER_FIELD(85, "State", "PersonT.State", fboPerson),
	ADD_FILTER_FIELD(86, "Zipcode", "PersonT.Zip", fboPerson),
	// (s.tullis 2014-06-25 13:56) - PLID 42233 - Security groups filter
	ADD_FILTER_FIELD(2027,"Security Group","SecurityGroupsT.ID " ,fboPerson,  foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID, NAME FROM SecurityGroupsT ORDER BY NAME ASC "), "PatientsT ON PersonT.ID= PatientsT.PersonID Left Join SecurityGroupDetailsT ON SecurityGroupDetailsT.PatientID = PatientsT.PersonID Left Join SecurityGroupsT ON SecurityGroupsT.ID = SecurityGroupDetailsT.SecurityGroupID", FILTER_DEPENDS_ON_BASE),
	// (a.walling 2010-03-24 12:39) - PLID 36815 - Filter field for Country
	ADD_FILTER_FIELD(420, "Country", "CountriesT.ID", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID, CountryName AS Name FROM CountriesT ORDER BY CountryName ASC"), "CountriesT ON PersonT.Country = CountriesT.CountryName", FILTER_DEPENDS_ON_BASE),
	ADD_FILTER_FIELD(87, "Email", "PersonT.Email", fboPerson),
	ADD_FILTER_FIELD(88, "Home Phone", "(Replace(Replace(Replace(Replace(PersonT.HomePhone,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL),
	ADD_FILTER_FIELD(89, "Work Phone", "(Replace(Replace(Replace(Replace(PersonT.WorkPhone,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL),
	// (z.manning 2009-02-16 09:12) - PLID 32856 - Added filter fields for pager and other phone
	ADD_FILTER_FIELD(412, "Pager", "(Replace(Replace(Replace(Replace(PersonT.Pager,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL),
	ADD_FILTER_FIELD(413, "Other Phone", "(Replace(Replace(Replace(Replace(PersonT.OtherPhone,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL),
	ADD_FILTER_FIELD(90, "Fax", "(Replace(Replace(Replace(Replace(PersonT.Fax,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL),
	//m.hancock - 11/1/05 - PLID 18014 - Gender filter operator 'Is Blank' doesn't work.  Since the gender is always stored as either
	//0, 1, or 2, the option for Is Blank doesn't recognize 0 as the Null / blank selection.  To fix this, we're removing
	//the operators for Is Blank and Is Not Blank and adding a choice for selecting the blank choice for gender.
	ADD_FILTER_FIELD(91, "Gender", "PersonT.Gender", fboPerson, foEqual|foNotEqual, foEqual, ftComboValues, P("{ Undefined }") P("0") P("Male") P("1") P("Female") P("2"), NULL),
	ADD_FILTER_FIELD(92, "SS Number", "PersonT.SocialSecurity", fboPerson),
	ADD_FILTER_FIELD(93, "Birth Date", "dbo.AsDateNoTime(PersonT.Birthdate)", fboPerson, foAll, foEqual, ftDate, NULL, NULL),	//see note at the top about converting date
	ADD_FILTER_FIELD(189, "Birth Month", "MONTH(PersonT.BirthDate)", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboValues, CGroups::CalcInternationalMonthList(), NULL),
	ADD_FILTER_FIELD(190, "Birth Day-Of-Month", "DAY(PersonT.BirthDate)", fboPerson, foAll, foEqual, ftNumber, NULL, NULL),
	ADD_FILTER_FIELD(94, "Age", "(CASE WHEN PersonT.BirthDate IS NOT NULL THEN (CASE WHEN PersonT.BirthDate < GETDATE() THEN YEAR(GETDATE()-PersonT.BirthDate)-1900 ELSE 0 END) ELSE NULL END)", fboPerson, foAll, foEqual, ftNumber, NULL, NULL),
	//(e.lally 2010-01-28) PLID 31744 - Added filter for username that entered the patient into the system.
	ADD_FILTER_FIELD(419, "Entered By", "PersonT.UserID", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT UsersT.Username AS Name, UsersT.PersonID AS ID FROM PersonT WITH(NOLOCK) INNER JOIN UsersT WITH(NOLOCK) ON PersonT.ID = UsersT.PersonID ORDER BY UsersT.Username ASC"), NULL, FILTER_DEPENDS_ON_BASE, NULL, NULL, NULL, NULL, 0, P("PersonT.ID") P("UsersT.Username") P("PersonT WITH(NOLOCK) INNER JOIN UsersT WITH(NOLOCK) ON PersonT.ID = UsersT.PersonID")),
	ADD_FILTER_FIELD(173, "Employment", "PatientsT.Employment", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboValues, P("Full-Time") P("1") P("Part-Time") P("4") P("Full-Time Student") P("2") P("Part-Time Student") P("3") P("Retired") P("5") P("Other") P("6") P("Unknown") P("0"), NULL, 159),
	ADD_FILTER_FIELD(95, "Occupation", "PatientsT.Occupation", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 159),
	ADD_FILTER_FIELD(215, "Spouse", "PatientsT.SpouseName", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 159),	
	ADD_FILTER_FIELD(96, "Default ICD-9 1", "(SELECT DiagCodes.CodeNumber FROM DiagCodes WHERE DiagCodes.ID = PatientsT.DefaultDiagID1)", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 159),
	ADD_FILTER_FIELD(185, "Default ICD-9 2", "(SELECT DiagCodes.CodeNumber FROM DiagCodes WHERE DiagCodes.ID = PatientsT.DefaultDiagID2)", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 159),
	ADD_FILTER_FIELD(186, "Default ICD-9 3", "(SELECT DiagCodes.CodeNumber FROM DiagCodes WHERE DiagCodes.ID = PatientsT.DefaultDiagID3)", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 159),
	ADD_FILTER_FIELD(187, "Default ICD-9 4", "(SELECT DiagCodes.CodeNumber FROM DiagCodes WHERE DiagCodes.ID = PatientsT.DefaultDiagID4)", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 159),
	// (j.jones 2014-02-27 08:44) - PLID 60771 - added ICD-10 filters
	ADD_FILTER_FIELD(2022, "Default ICD-10 1", "(SELECT DiagCodes.CodeNumber FROM DiagCodes WHERE DiagCodes.ID = PatientsT.DefaultICD10DiagID1)", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 159),
	ADD_FILTER_FIELD(2023, "Default ICD-10 2", "(SELECT DiagCodes.CodeNumber FROM DiagCodes WHERE DiagCodes.ID = PatientsT.DefaultICD10DiagID2)", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 159),
	ADD_FILTER_FIELD(2024, "Default ICD-10 3", "(SELECT DiagCodes.CodeNumber FROM DiagCodes WHERE DiagCodes.ID = PatientsT.DefaultICD10DiagID3)", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 159),
	ADD_FILTER_FIELD(2025, "Default ICD-10 4", "(SELECT DiagCodes.CodeNumber FROM DiagCodes WHERE DiagCodes.ID = PatientsT.DefaultICD10DiagID4)", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 159),
	ADD_FILTER_FIELD(97, "Referral Source Name (Primary)", "ReferralSourceT.Name", fboPerson, foAll, foBeginsWith, ftText, NULL, "ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID", 159),
	ADD_FILTER_FIELD(230, "Referral Source Name (Any)", "MultiReferralsQ.Name", fboPerson, foAll, foBeginsWith, ftText, NULL, "(SELECT ReferralSourceT.Name, ReferralSourceT.PersonID, MultiReferralsT.PatientID FROM MultiReferralsT INNER JOIN ReferralSourceT ON MultiReferralsT.ReferralID = ReferralSourceT.PersonID) MultiReferralsQ ON PatientsT.PersonID = MultiReferralsQ.PatientID", 159),
	ADD_FILTER_FIELD(231, "Referral Source (Primary)", "ReferralSourceT.PersonID", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ReferralSourceT.PersonID AS ID, ReferralSourceT.Name AS Name FROM ReferralSourceT ORDER BY ReferralSourceT.Name"), NULL,  97, NULL, NULL, NULL, NULL, 0, P("PersonID") P("Name") P("ReferralSourceT")),
	ADD_FILTER_FIELD(232, "Referral Source (Any)", "MultiReferralsQ.PersonID", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ReferralSourceT.PersonID AS ID, ReferralSourceT.Name AS Name FROM ReferralSourceT ORDER BY ReferralSourceT.Name"), NULL, 230, NULL, NULL, NULL, NULL, 0, P("PersonID") P("Name") P("ReferralSourceT")),
	ADD_FILTER_FIELD(259, "Warning Message", "PersonT.WarningMessage", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank|foBeginsWith|foEndsWith|foContains|foLike|foNotLike, foBeginsWith, ftText, NULL, NULL),
	ADD_FILTER_FIELD(260, "Show Warning", "PersonT.DisplayWarning", fboPerson, foEqual|foNotEqual, foEqual, ftComboValues, P("True") P("1") P("False") P("0"), NULL),
	ADD_FILTER_FIELD(292, "Excluded From Mailings", "PersonT.ExcludeFromMailings", fboPerson, foEqual|foNotEqual, foEqual, ftComboValues, P("True") P("1") P("False") P("0"), NULL),
	ADD_FILTER_FIELD(296, "Has Responsibility Type", "RespTypeT.ID", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT RespTypeT.ID AS ID, RespTypeT.TypeName AS Name FROM RespTypeT ORDER BY RespTypeT.Priority"), "InsuredPartyT ON PersonT.ID = InsuredPartyT.PatientID LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID",  NULL, NULL, NULL, NULL, NULL, 0, P("ID") P("TypeName") P("RespTypeT")),
	ADD_FILTER_FIELD(309, "Marital Status", "PatientsT.MaritalStatus", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboValues, P("Single") P("1") P("Married") P("2") P("Other") P("3"), NULL, 159),
	// (d.moore 2007-05-02 13:59) - PLID 23602 - 'CurrentStatus = 3' was used to determine both the number of Patients and the number of Prospects. It is now only counted as a Patient.
	ADD_FILTER_FIELD(179, "Patients Referred", "(SELECT Count(ReferringPatientID) AS CountOfReferredPatients FROM PatientsT WITH(NOLOCK) WHERE ReferringPatientID = PersonT.ID AND (CurrentStatus = 1 OR CurrentStatus = 3))", fboPerson, foEquality, foGreater, ftNumber, NULL, NULL),
	ADD_FILTER_FIELD(180, "Prospects Referred", "(SELECT Count(ReferringPatientID) AS CountOfReferredProspects FROM PatientsT WITH(NOLOCK) WHERE ReferringPatientID = PersonT.ID AND CurrentStatus = 2)", fboPerson, foEquality, foGreater, ftNumber, NULL, NULL),

	ADD_FILTER_FIELD(98, "Type", "PatientsT.TypeOfPatient", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT TypeIndex AS ID, GroupName AS Name FROM GroupTypes ORDER BY GroupName"), NULL, 159, NULL, NULL, NULL, NULL, 0, P("TypeIndex") P("GroupName") P("GroupTypes")),
	ADD_FILTER_FIELD(99, "Type Text", "GroupTypes.GroupName", fboPerson, foAll, foBeginsWith, ftText, NULL, "GroupTypes ON PatientsT.TypeOfPatient = GroupTypes.TypeIndex", 159),
	ADD_FILTER_FIELD(174, "Location", "PersonT.Location", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID, Name FROM LocationsT WHERE Managed = 1 AND TypeID = 1 ORDER BY Name"), NULL, FILTER_DEPENDS_ON_BASE, NULL, NULL, NULL, NULL, 0, P("ID") P("Name") P("LocationsT") P("Managed = 1 AND TypeID = 1")),
	ADD_FILTER_FIELD(100, "Status", "PatientsT.CurrentStatus", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboValues, P("Patient") P("1") P("Prospect") P("2") P("Patient/Prospect") P("3") P("Unspecified") P("0"), NULL, 159, NULL, NULL, NULL, NULL, 0),
	ADD_FILTER_FIELD(156, "First Contact Date", "dbo.AsDateNoTime(PersonT.FirstContactDate)", fboPerson, foAll, foEqual, ftDate, NULL, NULL),	//see note at the top about converting date
	ADD_FILTER_FIELD(157, "Last Update Date", "dbo.AsDateNoTime(PatientsT.ModifiedDate)", fboPerson, foAll, foEqual, ftDate, NULL, NULL, 159),	//see note at the top about converting date
	ADD_FILTER_FIELD(217, "Last History or Note Date", "dbo.AsDateNoTime((SELECT Max(Date) AS Date FROM (SELECT Max(Date) AS Date FROM MailSent WHERE MailSent.PersonID = PersonT.ID UNION ALL SELECT Max(Date) FROM Notes WHERE Notes.PersonID = PersonT.ID) SubQ))", fboPerson, foAll, foEqual, ftDate, NULL, NULL),
	ADD_FILTER_FIELD(101, "REPLACE_FIELD_NAME(Custom 1):SELECT Name FROM CustomFieldsT WHERE ID = 1", "(SELECT TextParam FROM CustomFieldDataT WHERE PersonID = PersonT.ID AND FieldID = 1)", fboPerson),
	ADD_FILTER_FIELD(102, "REPLACE_FIELD_NAME(Custom 2):SELECT Name FROM CustomFieldsT WHERE ID = 2", "(SELECT TextParam FROM CustomFieldDataT WHERE PersonID = PersonT.ID AND FieldID = 2)", fboPerson),
	ADD_FILTER_FIELD(103, "REPLACE_FIELD_NAME(Custom 3):SELECT Name FROM CustomFieldsT WHERE ID = 3", "(SELECT TextParam FROM CustomFieldDataT WHERE PersonID = PersonT.ID AND FieldID = 3)", fboPerson),
	ADD_FILTER_FIELD(104, "REPLACE_FIELD_NAME(Custom 4):SELECT Name FROM CustomFieldsT WHERE ID = 4", "(SELECT TextParam FROM CustomFieldDataT WHERE PersonID = PersonT.ID AND FieldID = 4)", fboPerson),
	ADD_FILTER_FIELD(105, "REPLACE_FIELD_NAME(Custom 5):SELECT Name FROM CustomFieldsT WHERE ID = 5", "(SELECT TextParam FROM CustomFieldDataT WHERE PersonID = PersonT.ID AND FieldID = 5)", fboPerson),
	//DRT 9/11/2006 - PLID 15308 - Added Privacy fields
	ADD_FILTER_FIELD(395, "Privacy Home Phone", "CASE WHEN PersonT.PrivHome IS NULL THEN 0 ELSE PersonT.PrivHome END", fboPerson, foEqual|foNotEqual, foEqual, ftComboValues, P("True") P("1") P("False") P("0"), NULL, NULL, NULL, NULL, NULL, NULL, 0),
	ADD_FILTER_FIELD(396, "Privacy Work Phone", "CASE WHEN PersonT.PrivWork IS NULL THEN 0 ELSE PersonT.PrivWork END", fboPerson, foEqual|foNotEqual, foEqual, ftComboValues, P("True") P("1") P("False") P("0"), NULL, NULL, NULL, NULL, NULL, NULL, 0),
	ADD_FILTER_FIELD(397, "Privacy Mobile Phone", "CASE WHEN PersonT.PrivCell IS NULL THEN 0 ELSE PersonT.PrivCell END", fboPerson, foEqual|foNotEqual, foEqual, ftComboValues, P("True") P("1") P("False") P("0"), NULL, NULL, NULL, NULL, NULL, NULL, 0),
	ADD_FILTER_FIELD(398, "Privacy Pager", "PersonT.PrivPager", fboPerson, foEqual|foNotEqual, foEqual, ftComboValues, P("True") P("1") P("False") P("0"), NULL, NULL, NULL, NULL, NULL, NULL, 0),
	ADD_FILTER_FIELD(399, "Privacy Other Phone", "PersonT.PrivOther", fboPerson, foEqual|foNotEqual, foEqual, ftComboValues, P("True") P("1") P("False") P("0"), NULL, NULL, NULL, NULL, NULL, NULL, 0),
	ADD_FILTER_FIELD(400, "Privacy Fax", "PersonT.PrivFax", fboPerson, foEqual|foNotEqual, foEqual, ftComboValues, P("True") P("1") P("False") P("0"), NULL, NULL, NULL, NULL, NULL, NULL, 0),
	ADD_FILTER_FIELD(401, "Privacy Email", "PersonT.PrivEmail", fboPerson, foEqual|foNotEqual, foEqual, ftComboValues, P("True") P("1") P("False") P("0"), NULL, NULL, NULL, NULL, NULL, NULL, 0),
	// (z.manning 2008-07-11 09:25) - PLID 30678 - Added text message field
	ADD_FILTER_FIELD(408, "Privacy Text Message", "PersonT.TextMessage", fboPerson, foEqual|foNotEqual, foEqual, ftComboValues, P("True") P("1") P("False") P("0"), NULL, NULL, NULL, NULL, NULL, NULL, 0),
	// (r.gonet 10-05-2010 10:45) - PLID 14585 - Added a filter for the patient's preferred contact method
	ADD_FILTER_FIELD(465, "Preferred Contact", "PatientsT.PreferredContact", fboPerson, foEqual|foNotEqual, foEqual, ftComboSelect, 
		P("SELECT 0 AS ID, '{ No Preference }' AS Name "
		  "UNION "
		  "SELECT 1 AS ID, 'Home Phone' AS Name "
		  "UNION "
		  "SELECT 2 AS ID, 'Work Phone' AS Name "
		  "UNION "
		  "SELECT 3 AS ID, 'Mobile Phone' AS Name "
		  "UNION "
		  "SELECT 4 AS ID, 'Pager' AS Name "
		  "UNION "
		  "SELECT 5 AS ID, 'Other Phone' AS Name "
		  "UNION "
		  "SELECT 6 AS ID, 'Email' AS Name "
		  "UNION "
		  "SELECT 7 AS ID, 'Text Messaging' AS Name "), 
		  "PatientsT WITH(NOLOCK) ON PersonT.ID = PatientsT.PersonID",
		  FILTER_DEPENDS_ON_BASE, NULL, NULL, NULL, NULL, 0,
		  P("ID") P("Name") P("(SELECT 0 AS ID, ' { No Preference }' AS Name "
		  "UNION "
		  "SELECT 1 AS ID, 'Home Phone' AS Name "
		  "UNION "
		  "SELECT 2 AS ID, 'Work Phone' AS Name "
		  "UNION "
		  "SELECT 3 AS ID, 'Mobile Phone' AS Name "
		  "UNION "
		  "SELECT 4 AS ID, 'Pager' AS Name "
		  "UNION "
		  "SELECT 5 AS ID, 'Other Phone' AS Name "
		  "UNION "
		  "SELECT 6 AS ID, 'Email' AS Name "
		  "UNION "
		  "SELECT 7 AS ID, 'Text Messaging' AS Name) SubQ ")),
	
	//Contact Custom Fields
	ADD_FILTER_FIELD(233, "REPLACE_FIELD_NAME(Contact, Custom 1):SELECT Name FROM CustomFieldsT WHERE ID = 6", "(SELECT TextParam FROM CustomFieldDataT WHERE PersonID = PersonT.ID AND FieldID = 6)", fboPerson),
	ADD_FILTER_FIELD(234, "REPLACE_FIELD_NAME(Contact, Custom 2):SELECT Name FROM CustomFieldsT WHERE ID = 7", "(SELECT TextParam FROM CustomFieldDataT WHERE PersonID = PersonT.ID AND FieldID = 7)", fboPerson),
	ADD_FILTER_FIELD(235, "REPLACE_FIELD_NAME(Contact, Custom 3):SELECT Name FROM CustomFieldsT WHERE ID = 8", "(SELECT TextParam FROM CustomFieldDataT WHERE PersonID = PersonT.ID AND FieldID = 8)", fboPerson),
	ADD_FILTER_FIELD(236, "REPLACE_FIELD_NAME(Contact, Custom 4):SELECT Name FROM CustomFieldsT WHERE ID = 9", "(SELECT TextParam FROM CustomFieldDataT WHERE PersonID = PersonT.ID AND FieldID = 9)", fboPerson),
		
	ADD_FILTER_FIELD(237, "Insurance Company HCFA Group", "InsuranceCoT.HCFASetupGroupID", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID, Name FROM HCFASetupT ORDER BY Name"), "InsuranceCoT ON PersonT.ID = InsuranceCoT.PersonID", FILTER_DEPENDS_ON_BASE, NULL, NULL, NULL, NULL, 0, P("ID") P("Name") P("HCFASetupT")),
	ADD_FILTER_FIELD(238, "Patient HCFA Groups", "InsuranceCoT.HCFASetupGroupID", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID, Name FROM HCFASetupT ORDER BY Name"), "InsuredPartyT ON PersonT.ID = InsuredPartyT.PatientID LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID", FILTER_DEPENDS_ON_BASE, NULL, NULL, NULL, NULL, 0, P("ID") P("Name") P("HCFASetupT")),
	
	//Note: This uses the weird setup below because, back in the day, someone known only as "BVB" designed the filter to display all possible procedure-step combinations. 
	//Since the structure has changed, this would be unwieldy, but obviously we can't do anything about the format of the ids as they currently are.  So, well, you'll see what I mean.
	ADD_FILTER_FIELD(181, "Procedure Interest", "CONVERT(nvarchar, LaddersQ.ProcedureID) + ' - -1'", fboPerson, foEquality, foEqual, ftComboSelect, P("SELECT DISTINCT '''' + UniqueInterestIdent + '''' AS ID, InterestText AS Name FROM (SELECT CONVERT(nvarchar, LaddersQ.ProcedureID) + ' - -1' AS UniqueInterestIdent, (SELECT Name FROM ProcedureT WHERE ProcedureT.ID = LaddersQ.ProcedureID) AS InterestText FROM ProcedureT INNER JOIN (SELECT LaddersT.PersonID, ProcInfoDetailsT.ProcedureID FROM LaddersT INNER JOIN ProcInfoDetailsT ON LaddersT.ProcInfoID = ProcInfoDetailsT.ProcInfoID) AS LaddersQ ON ProcedureT.ID = LaddersQ.ProcedureID) PatientInterestInfoQ ORDER BY InterestText"), "(SELECT LaddersT.PersonID, ProcInfoDetailsT.ProcedureID FROM LaddersT INNER JOIN ProcInfoDetailsT ON LaddersT.ProcInfoID = ProcInfoDetailsT.ProcInfoID) AS LaddersQ ON PersonT.ID = LaddersQ.PersonID", FILTER_DEPENDS_ON_BASE, NULL, NULL, NULL, NULL, 0),

	ADD_FILTER_FIELD(182, "Patient Coordinator", "PatientsT.EmployeeID", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT PersonT.ID AS ID, PersonT.[Last] + ', ' + PersonT.[First] + ' ' + PersonT.Middle AS Name FROM PersonT WITH(NOLOCK) INNER JOIN UsersT WITH(NOLOCK) ON PersonT.ID = UsersT.PersonID AND UsersT.PersonID > 0 WHERE UsersT.PatientCoordinator = 1"),"PatientsT WITH(NOLOCK) ON PersonT.ID = PatientsT.PersonID", FILTER_DEPENDS_ON_BASE, NULL, NULL, NULL, NULL, 0, P("ID") P("PersonT.[Last] + ', ' + PersonT.[First] + ' ' + PersonT.Middle") P("PersonT WITH(NOLOCK) INNER JOIN UsersT WITH(NOLOCK) ON PersonT.ID = UsersT.PersonID AND UsersT.PersonID > 0") P("UsersT.PatientCoordinator = 1")),
	
/*
	// Todo: enable these fields.  This sql statement might be useful to you.
////ADD_FILTER_FIELD(106, "Prepays", "Sum(LineItemT.Amount)", foAll, foGreater, ftNumber, NULL, "LineItemT ON Patients.ID = LineItemT.PatientID"),
//SELECT Patients.ID AS ID
//FROM Patients LEFT JOIN LineItemT ON Patients.ID = LineItemT.PatientID
//WHERE (((Patients.ID)>0))
//GROUP BY Patients.ID
//HAVING (((Sum(LineItemT.Amount))>0));

	ADD_FILTER_FIELD(, "Balance", GetPatientBalanceSubQ() + ".AccountBal"),
	ADD_FILTER_FIELD(, "Last_Payment_Date", "Format(" + GetLastPaymentDateSubQ() + ".LastPaymentDate" + ",\"Short Date\")"),
	ADD_FILTER_FIELD(, "Last_Payment_Amount", GetLastPaymentAmountSubQ() + ".LastPaymentAmount"),
//*/

	ADD_FILTER_FIELD(164, "Last Payment Date", "dbo.AsDateNoTime(FilterLastPaymentSubQ.LastPaymentDate)", fboPerson, foEquality, foGreater, ftDate, NULL, //see note at the top about converting date
				"(SELECT LineItemT.PatientID, LineItemT.Amount AS LastPaymentAmount, LineItemT.Date AS LastPaymentDate FROM ( "
				" SELECT CONVERT(int, SUBSTRING(MAX(CONVERT(nvarchar, LineItemT.Date, 121) + ' ' + CONVERT(nvarchar, LineItemT.InputDate, 121) + ' ' + CONVERT(nvarchar, LineItemT.ID)), 49, 80)) AS PaymentID "
				" FROM LineItemT WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type = 1) GROUP BY LineItemT.PatientID) LastPaymentInternalQ "
				"INNER JOIN LineItemT ON LastPaymentInternalQ.PaymentID = LineItemT.ID) FilterLastPaymentSubQ ON PersonT.ID = FilterLastPaymentSubQ.PatientID"), 

	ADD_FILTER_FIELD(165, "Last Payment Amount", "FilterLastPaymentSubQ.LastPaymentAmount", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 164), 

	// (j.jones 2010-05-05 08:41) - PLID 37000 - added filters for last patient payment date & amount,
	// last insurance payment date & amount, primary, and secondary
	ADD_FILTER_FIELD(421, "Last Patient Payment Date", "dbo.AsDateNoTime(FilterLastPatientPaymentSubQ.LastPaymentDate)", fboPerson, foEquality, foGreater, ftDate, NULL, //see note at the top about converting date
				"(SELECT LineItemT.PatientID, LineItemT.Amount AS LastPaymentAmount, LineItemT.Date AS LastPaymentDate FROM ( "
				" SELECT CONVERT(int, SUBSTRING(MAX(CONVERT(nvarchar, LineItemT.Date, 121) + ' ' + CONVERT(nvarchar, LineItemT.InputDate, 121) + ' ' + CONVERT(nvarchar, LineItemT.ID)), 49, 80)) AS PaymentID "
				" FROM LineItemT "
				" INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
				" WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type = 1) AND (PaymentsT.InsuredPartyID Is Null OR PaymentsT.InsuredPartyID = -1) GROUP BY LineItemT.PatientID) LastPaymentInternalQ "
				"INNER JOIN LineItemT ON LastPaymentInternalQ.PaymentID = LineItemT.ID) FilterLastPatientPaymentSubQ ON PersonT.ID = FilterLastPatientPaymentSubQ.PatientID"), 

	ADD_FILTER_FIELD(422, "Last Patient Payment Amount", "FilterLastPatientPaymentSubQ.LastPaymentAmount", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 421), 

	ADD_FILTER_FIELD(423, "Last Insurance Payment Date", "dbo.AsDateNoTime(FilterLastInsurancePaymentSubQ.LastPaymentDate)", fboPerson, foEquality, foGreater, ftDate, NULL, //see note at the top about converting date
				"(SELECT LineItemT.PatientID, LineItemT.Amount AS LastPaymentAmount, LineItemT.Date AS LastPaymentDate FROM ( "
				" SELECT CONVERT(int, SUBSTRING(MAX(CONVERT(nvarchar, LineItemT.Date, 121) + ' ' + CONVERT(nvarchar, LineItemT.InputDate, 121) + ' ' + CONVERT(nvarchar, LineItemT.ID)), 49, 80)) AS PaymentID "
				" FROM LineItemT "
				" INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
				" WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type = 1) AND (PaymentsT.InsuredPartyID Is Not Null AND PaymentsT.InsuredPartyID > -1) GROUP BY LineItemT.PatientID) LastPaymentInternalQ "
				"INNER JOIN LineItemT ON LastPaymentInternalQ.PaymentID = LineItemT.ID) FilterLastInsurancePaymentSubQ ON PersonT.ID = FilterLastInsurancePaymentSubQ.PatientID"), 

	ADD_FILTER_FIELD(424, "Last Insurance Payment Amount", "FilterLastInsurancePaymentSubQ.LastPaymentAmount", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 423), 

	ADD_FILTER_FIELD(425, "Last Primary Insurance Payment Date", "dbo.AsDateNoTime(FilterLastPrimaryInsurancePaymentSubQ.LastPaymentDate)", fboPerson, foEquality, foGreater, ftDate, NULL, //see note at the top about converting date
				"(SELECT LineItemT.PatientID, LineItemT.Amount AS LastPaymentAmount, LineItemT.Date AS LastPaymentDate FROM ( "
				" SELECT CONVERT(int, SUBSTRING(MAX(CONVERT(nvarchar, LineItemT.Date, 121) + ' ' + CONVERT(nvarchar, LineItemT.InputDate, 121) + ' ' + CONVERT(nvarchar, LineItemT.ID)), 49, 80)) AS PaymentID "
				" FROM LineItemT "
				" INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
				" INNER JOIN InsuredPartyT ON PaymentsT.InsuredPartyID = InsuredPartyT.PersonID "
				" WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type = 1) AND (InsuredPartyT.RespTypeID = 1) GROUP BY LineItemT.PatientID) LastPaymentInternalQ "
				"INNER JOIN LineItemT ON LastPaymentInternalQ.PaymentID = LineItemT.ID) FilterLastPrimaryInsurancePaymentSubQ ON PersonT.ID = FilterLastPrimaryInsurancePaymentSubQ.PatientID"), 

	ADD_FILTER_FIELD(426, "Last Primary Insurance Payment Amount", "FilterLastPrimaryInsurancePaymentSubQ.LastPaymentAmount", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 425), 

	ADD_FILTER_FIELD(427, "Last Secondary Insurance Payment Date", "dbo.AsDateNoTime(FilterLastSecondaryInsurancePaymentSubQ.LastPaymentDate)", fboPerson, foEquality, foGreater, ftDate, NULL, //see note at the top about converting date
				"(SELECT LineItemT.PatientID, LineItemT.Amount AS LastPaymentAmount, LineItemT.Date AS LastPaymentDate FROM ( "
				" SELECT CONVERT(int, SUBSTRING(MAX(CONVERT(nvarchar, LineItemT.Date, 121) + ' ' + CONVERT(nvarchar, LineItemT.InputDate, 121) + ' ' + CONVERT(nvarchar, LineItemT.ID)), 49, 80)) AS PaymentID "
				" FROM LineItemT "
				" INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
				" INNER JOIN InsuredPartyT ON PaymentsT.InsuredPartyID = InsuredPartyT.PersonID "
				" WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type = 1) AND (InsuredPartyT.RespTypeID = 2) GROUP BY LineItemT.PatientID) LastPaymentInternalQ "
				"INNER JOIN LineItemT ON LastPaymentInternalQ.PaymentID = LineItemT.ID) FilterLastSecondaryInsurancePaymentSubQ ON PersonT.ID = FilterLastSecondaryInsurancePaymentSubQ.PatientID"), 

	ADD_FILTER_FIELD(428, "Last Secondary Insurance Payment Amount", "FilterLastSecondaryInsurancePaymentSubQ.LastPaymentAmount", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 427), 

	// (j.jones 2011-04-01 14:52) - PLID 43109 - added Vision Primary & Secondary payment filters
	ADD_FILTER_FIELD(471, "Last Vision Insurance (Primary) Payment Date", "dbo.AsDateNoTime(FilterLastVisionPrimaryInsurancePaymentSubQ.LastPaymentDate)", fboPerson, foEquality, foGreater, ftDate, NULL, //see note at the top about converting date
				"(SELECT LineItemT.PatientID, LineItemT.Amount AS LastPaymentAmount, LineItemT.Date AS LastPaymentDate FROM ( "
				" SELECT CONVERT(int, SUBSTRING(MAX(CONVERT(nvarchar, LineItemT.Date, 121) + ' ' + CONVERT(nvarchar, LineItemT.InputDate, 121) + ' ' + CONVERT(nvarchar, LineItemT.ID)), 49, 80)) AS PaymentID "
				" FROM LineItemT "
				" INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
				" INNER JOIN InsuredPartyT ON PaymentsT.InsuredPartyID = InsuredPartyT.PersonID "
				" INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				" WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type = 1) AND (RespTypeT.CategoryType = 2 AND RespTypeT.CategoryPlacement = 1) "
				" GROUP BY LineItemT.PatientID) LastPaymentInternalQ "
				"INNER JOIN LineItemT ON LastPaymentInternalQ.PaymentID = LineItemT.ID) FilterLastVisionPrimaryInsurancePaymentSubQ ON PersonT.ID = FilterLastVisionPrimaryInsurancePaymentSubQ.PatientID"), 

	ADD_FILTER_FIELD(472, "Last Vision Insurance (Primary) Payment Amount", "FilterLastVisionPrimaryInsurancePaymentSubQ.LastPaymentAmount", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 471), 

	ADD_FILTER_FIELD(473, "Last Vision Insurance (Secondary) Payment Date", "dbo.AsDateNoTime(FilterLastVisionSecondaryInsurancePaymentSubQ.LastPaymentDate)", fboPerson, foEquality, foGreater, ftDate, NULL, //see note at the top about converting date
				"(SELECT LineItemT.PatientID, LineItemT.Amount AS LastPaymentAmount, LineItemT.Date AS LastPaymentDate FROM ( "
				" SELECT CONVERT(int, SUBSTRING(MAX(CONVERT(nvarchar, LineItemT.Date, 121) + ' ' + CONVERT(nvarchar, LineItemT.InputDate, 121) + ' ' + CONVERT(nvarchar, LineItemT.ID)), 49, 80)) AS PaymentID "
				" FROM LineItemT "
				" INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
				" INNER JOIN InsuredPartyT ON PaymentsT.InsuredPartyID = InsuredPartyT.PersonID "
				" INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				" WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type = 1) AND (RespTypeT.CategoryType = 2 AND RespTypeT.CategoryPlacement = 2) "
				" GROUP BY LineItemT.PatientID) LastPaymentInternalQ "
				"INNER JOIN LineItemT ON LastPaymentInternalQ.PaymentID = LineItemT.ID) FilterLastVisionSecondaryInsurancePaymentSubQ ON PersonT.ID = FilterLastVisionSecondaryInsurancePaymentSubQ.PatientID"), 

	ADD_FILTER_FIELD(474, "Last Vision Insurance (Secondary) Payment Amount", "FilterLastVisionSecondaryInsurancePaymentSubQ.LastPaymentAmount", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 473), 

	// (d.singleton 2012-09-11 09:49) - PLID 51808 add auto filters
	ADD_FILTER_FIELD(550, "Last Auto Insurance (Primary) Payment Date", "dbo.AsDateNoTime(FilterLastAutoPrimaryInsurancePaymentSubQ.LastPaymentDate)", fboPerson, foEquality, foGreater, ftDate, NULL, //see note at the top about converting date
				"(SELECT LineItemT.PatientID, LineItemT.Amount AS LastPaymentAmount, LineItemT.Date AS LastPaymentDate FROM ( "
				" SELECT CONVERT(int, SUBSTRING(MAX(CONVERT(nvarchar, LineItemT.Date, 121) + ' ' + CONVERT(nvarchar, LineItemT.InputDate, 121) + ' ' + CONVERT(nvarchar, LineItemT.ID)), 49, 80)) AS PaymentID "
				" FROM LineItemT "
				" INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
				" INNER JOIN InsuredPartyT ON PaymentsT.InsuredPartyID = InsuredPartyT.PersonID "
				" INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				" WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type = 1) AND (RespTypeT.CategoryType = 3 AND RespTypeT.CategoryPlacement = 1) "
				" GROUP BY LineItemT.PatientID) LastPaymentInternalQ "
				"INNER JOIN LineItemT ON LastPaymentInternalQ.PaymentID = LineItemT.ID) FilterLastAutoPrimaryInsurancePaymentSubQ ON PersonT.ID = FilterLastAutoPrimaryInsurancePaymentSubQ.PatientID"), 

	ADD_FILTER_FIELD(551, "Last Auto Insurance (Primary) Payment Amount", "FilterLastAutoPrimaryInsurancePaymentSubQ.LastPaymentAmount", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 550), 
	
	ADD_FILTER_FIELD(552, "Last Auto Insurance (Secondary) Payment Date", "dbo.AsDateNoTime(FilterLastAutoSecondaryInsurancePaymentSubQ.LastPaymentDate)", fboPerson, foEquality, foGreater, ftDate, NULL, //see note at the top about converting date
				"(SELECT LineItemT.PatientID, LineItemT.Amount AS LastPaymentAmount, LineItemT.Date AS LastPaymentDate FROM ( "
				" SELECT CONVERT(int, SUBSTRING(MAX(CONVERT(nvarchar, LineItemT.Date, 121) + ' ' + CONVERT(nvarchar, LineItemT.InputDate, 121) + ' ' + CONVERT(nvarchar, LineItemT.ID)), 49, 80)) AS PaymentID "
				" FROM LineItemT "
				" INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
				" INNER JOIN InsuredPartyT ON PaymentsT.InsuredPartyID = InsuredPartyT.PersonID "
				" INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				" WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type = 1) AND (RespTypeT.CategoryType = 3 AND RespTypeT.CategoryPlacement = 2) "
				" GROUP BY LineItemT.PatientID) LastPaymentInternalQ "
				"INNER JOIN LineItemT ON LastPaymentInternalQ.PaymentID = LineItemT.ID) FilterLastAutoSecondaryInsurancePaymentSubQ ON PersonT.ID = FilterLastAutoSecondaryInsurancePaymentSubQ.PatientID"), 

	ADD_FILTER_FIELD(553, "Last Auto Insurance (Secondary) Payment Amount", "FilterLastAutoSecondaryInsurancePaymentSubQ.LastPaymentAmount", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 552), 

	// (d.singleton 2012-09-14 13:31) - PLID 52657 add workers comp filters
	ADD_FILTER_FIELD(554, "Last Workers Comp Insurance (Primary) Payment Date", "dbo.AsDateNoTime(FilterLastWorkersCompPrimaryInsurancePaymentSubQ.LastPaymentDate)", fboPerson, foEquality, foGreater, ftDate, NULL, //see note at the top about converting date
				"(SELECT LineItemT.PatientID, LineItemT.Amount AS LastPaymentAmount, LineItemT.Date AS LastPaymentDate FROM ( "
				" SELECT CONVERT(int, SUBSTRING(MAX(CONVERT(nvarchar, LineItemT.Date, 121) + ' ' + CONVERT(nvarchar, LineItemT.InputDate, 121) + ' ' + CONVERT(nvarchar, LineItemT.ID)), 49, 80)) AS PaymentID "
				" FROM LineItemT "
				" INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
				" INNER JOIN InsuredPartyT ON PaymentsT.InsuredPartyID = InsuredPartyT.PersonID "
				" INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				" WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type = 1) AND (RespTypeT.CategoryType = 4 AND RespTypeT.CategoryPlacement = 1) "
				" GROUP BY LineItemT.PatientID) LastPaymentInternalQ "
				"INNER JOIN LineItemT ON LastPaymentInternalQ.PaymentID = LineItemT.ID) FilterLastWorkersCompPrimaryInsurancePaymentSubQ ON PersonT.ID = FilterLastWorkersCompPrimaryInsurancePaymentSubQ.PatientID"), 

	ADD_FILTER_FIELD(555, "Last Workers Comp Insurance (Primary) Payment Amount", "FilterLastWorkersCompPrimaryInsurancePaymentSubQ.LastPaymentAmount", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 554), 

	
	ADD_FILTER_FIELD(556, "Last Workers Comp Insurance (Secondary) Payment Date", "dbo.AsDateNoTime(FilterLastWorkersCompSecondaryInsurancePaymentSubQ.LastPaymentDate)", fboPerson, foEquality, foGreater, ftDate, NULL, //see note at the top about converting date
				"(SELECT LineItemT.PatientID, LineItemT.Amount AS LastPaymentAmount, LineItemT.Date AS LastPaymentDate FROM ( "
				" SELECT CONVERT(int, SUBSTRING(MAX(CONVERT(nvarchar, LineItemT.Date, 121) + ' ' + CONVERT(nvarchar, LineItemT.InputDate, 121) + ' ' + CONVERT(nvarchar, LineItemT.ID)), 49, 80)) AS PaymentID "
				" FROM LineItemT "
				" INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
				" INNER JOIN InsuredPartyT ON PaymentsT.InsuredPartyID = InsuredPartyT.PersonID "
				" INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				" WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type = 1) AND (RespTypeT.CategoryType = 4 AND RespTypeT.CategoryPlacement = 2) "
				" GROUP BY LineItemT.PatientID) LastPaymentInternalQ "
				"INNER JOIN LineItemT ON LastPaymentInternalQ.PaymentID = LineItemT.ID) FilterLastWorkersCompSecondaryInsurancePaymentSubQ ON PersonT.ID = FilterLastWorkersCompSecondaryInsurancePaymentSubQ.PatientID"), 

	ADD_FILTER_FIELD(557, "Last Workers Comp Insurance (Secondary) Payment Amount", "FilterLastWorkersCompSecondaryInsurancePaymentSubQ.LastPaymentAmount", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 556), 

	// (d.singleton 2012-09-14 13:31) - PLID 52658 add dental filter
	ADD_FILTER_FIELD(558, "Last Dental Insurance (Primary) Payment Date", "dbo.AsDateNoTime(FilterLastDentalPrimaryInsurancePaymentSubQ.LastPaymentDate)", fboPerson, foEquality, foGreater, ftDate, NULL, //see note at the top about converting date
				"(SELECT LineItemT.PatientID, LineItemT.Amount AS LastPaymentAmount, LineItemT.Date AS LastPaymentDate FROM ( "
				" SELECT CONVERT(int, SUBSTRING(MAX(CONVERT(nvarchar, LineItemT.Date, 121) + ' ' + CONVERT(nvarchar, LineItemT.InputDate, 121) + ' ' + CONVERT(nvarchar, LineItemT.ID)), 49, 80)) AS PaymentID "
				" FROM LineItemT "
				" INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
				" INNER JOIN InsuredPartyT ON PaymentsT.InsuredPartyID = InsuredPartyT.PersonID "
				" INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				" WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type = 1) AND (RespTypeT.CategoryType = 5 AND RespTypeT.CategoryPlacement = 1) "
				" GROUP BY LineItemT.PatientID) LastPaymentInternalQ "
				"INNER JOIN LineItemT ON LastPaymentInternalQ.PaymentID = LineItemT.ID) FilterLastDentalPrimaryInsurancePaymentSubQ ON PersonT.ID = FilterLastDentalPrimaryInsurancePaymentSubQ.PatientID"), 

	ADD_FILTER_FIELD(559, "Last Dental Insurance (Primary) Payment Amount", "FilterLastDentalPrimaryInsurancePaymentSubQ.LastPaymentAmount", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 558), 
	
	ADD_FILTER_FIELD(560, "Last Dental Insurance (Secondary) Payment Date", "dbo.AsDateNoTime(FilterLastDentalSecondaryInsurancePaymentSubQ.LastPaymentDate)", fboPerson, foEquality, foGreater, ftDate, NULL, //see note at the top about converting date
				"(SELECT LineItemT.PatientID, LineItemT.Amount AS LastPaymentAmount, LineItemT.Date AS LastPaymentDate FROM ( "
				" SELECT CONVERT(int, SUBSTRING(MAX(CONVERT(nvarchar, LineItemT.Date, 121) + ' ' + CONVERT(nvarchar, LineItemT.InputDate, 121) + ' ' + CONVERT(nvarchar, LineItemT.ID)), 49, 80)) AS PaymentID "
				" FROM LineItemT "
				" INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
				" INNER JOIN InsuredPartyT ON PaymentsT.InsuredPartyID = InsuredPartyT.PersonID "
				" INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				" WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type = 1) AND (RespTypeT.CategoryType = 5 AND RespTypeT.CategoryPlacement = 2) "
				" GROUP BY LineItemT.PatientID) LastPaymentInternalQ "
				"INNER JOIN LineItemT ON LastPaymentInternalQ.PaymentID = LineItemT.ID) FilterLastDentalSecondaryInsurancePaymentSubQ ON PersonT.ID = FilterLastDentalSecondaryInsurancePaymentSubQ.PatientID"), 

	ADD_FILTER_FIELD(561, "Last Dental Insurance (Secondary) Payment Amount", "FilterLastDentalSecondaryInsurancePaymentSubQ.LastPaymentAmount", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 560), 

	// (d.singleton 2012-09-14 13:33) - PLID 52659 add study filter
	ADD_FILTER_FIELD(562, "Last Study Insurance (Primary) Payment Date", "dbo.AsDateNoTime(FilterLastStudyPrimaryInsurancePaymentSubQ.LastPaymentDate)", fboPerson, foEquality, foGreater, ftDate, NULL, //see note at the top about converting date
				"(SELECT LineItemT.PatientID, LineItemT.Amount AS LastPaymentAmount, LineItemT.Date AS LastPaymentDate FROM ( "
				" SELECT CONVERT(int, SUBSTRING(MAX(CONVERT(nvarchar, LineItemT.Date, 121) + ' ' + CONVERT(nvarchar, LineItemT.InputDate, 121) + ' ' + CONVERT(nvarchar, LineItemT.ID)), 49, 80)) AS PaymentID "
				" FROM LineItemT "
				" INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
				" INNER JOIN InsuredPartyT ON PaymentsT.InsuredPartyID = InsuredPartyT.PersonID "
				" INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				" WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type = 1) AND (RespTypeT.CategoryType = 6 AND RespTypeT.CategoryPlacement = 1) "
				" GROUP BY LineItemT.PatientID) LastPaymentInternalQ "
				"INNER JOIN LineItemT ON LastPaymentInternalQ.PaymentID = LineItemT.ID) FilterLastStudyPrimaryInsurancePaymentSubQ ON PersonT.ID = FilterLastStudyPrimaryInsurancePaymentSubQ.PatientID"), 

	ADD_FILTER_FIELD(563, "Last Study Insurance (Primary) Payment Amount", "FilterLastStudyPrimaryInsurancePaymentSubQ.LastPaymentAmount", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 562), 
	
	ADD_FILTER_FIELD(564, "Last Study Insurance (Secondary) Payment Date", "dbo.AsDateNoTime(FilterLastStudySecondaryInsurancePaymentSubQ.LastPaymentDate)", fboPerson, foEquality, foGreater, ftDate, NULL, //see note at the top about converting date
				"(SELECT LineItemT.PatientID, LineItemT.Amount AS LastPaymentAmount, LineItemT.Date AS LastPaymentDate FROM ( "
				" SELECT CONVERT(int, SUBSTRING(MAX(CONVERT(nvarchar, LineItemT.Date, 121) + ' ' + CONVERT(nvarchar, LineItemT.InputDate, 121) + ' ' + CONVERT(nvarchar, LineItemT.ID)), 49, 80)) AS PaymentID "
				" FROM LineItemT "
				" INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
				" INNER JOIN InsuredPartyT ON PaymentsT.InsuredPartyID = InsuredPartyT.PersonID "
				" INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				" WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type = 1) AND (RespTypeT.CategoryType = 6 AND RespTypeT.CategoryPlacement = 2) "
				" GROUP BY LineItemT.PatientID) LastPaymentInternalQ "
				"INNER JOIN LineItemT ON LastPaymentInternalQ.PaymentID = LineItemT.ID) FilterLastStudySecondaryInsurancePaymentSubQ ON PersonT.ID = FilterLastStudySecondaryInsurancePaymentSubQ.PatientID"), 

	ADD_FILTER_FIELD(565, "Last Study Insurance (Secondary) Payment Amount", "FilterLastStudySecondaryInsurancePaymentSubQ.LastPaymentAmount", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 564), 

	// (d.singleton 2012-09-14 13:33) - PLID 52260 add LOP filter
	ADD_FILTER_FIELD(566, "Last LOP Insurance (Primary) Payment Date", "dbo.AsDateNoTime(FilterLastLOPPrimaryInsurancePaymentSubQ.LastPaymentDate)", fboPerson, foEquality, foGreater, ftDate, NULL, //see note at the top about converting date
				"(SELECT LineItemT.PatientID, LineItemT.Amount AS LastPaymentAmount, LineItemT.Date AS LastPaymentDate FROM ( "
				" SELECT CONVERT(int, SUBSTRING(MAX(CONVERT(nvarchar, LineItemT.Date, 121) + ' ' + CONVERT(nvarchar, LineItemT.InputDate, 121) + ' ' + CONVERT(nvarchar, LineItemT.ID)), 49, 80)) AS PaymentID "
				" FROM LineItemT "
				" INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
				" INNER JOIN InsuredPartyT ON PaymentsT.InsuredPartyID = InsuredPartyT.PersonID "
				" INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				" WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type = 1) AND (RespTypeT.CategoryType = 7 AND RespTypeT.CategoryPlacement = 1) "
				" GROUP BY LineItemT.PatientID) LastPaymentInternalQ "
				"INNER JOIN LineItemT ON LastPaymentInternalQ.PaymentID = LineItemT.ID) FilterLastLOPPrimaryInsurancePaymentSubQ ON PersonT.ID = FilterLastLOPPrimaryInsurancePaymentSubQ.PatientID"), 

	ADD_FILTER_FIELD(567, "Last LOP Insurance (Primary) Payment Amount", "FilterLastLOPPrimaryInsurancePaymentSubQ.LastPaymentAmount", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 566), 
	
	ADD_FILTER_FIELD(568, "Last LOP Insurance (Secondary) Payment Date", "dbo.AsDateNoTime(FilterLastLOPSecondaryInsurancePaymentSubQ.LastPaymentDate)", fboPerson, foEquality, foGreater, ftDate, NULL, //see note at the top about converting date
				"(SELECT LineItemT.PatientID, LineItemT.Amount AS LastPaymentAmount, LineItemT.Date AS LastPaymentDate FROM ( "
				" SELECT CONVERT(int, SUBSTRING(MAX(CONVERT(nvarchar, LineItemT.Date, 121) + ' ' + CONVERT(nvarchar, LineItemT.InputDate, 121) + ' ' + CONVERT(nvarchar, LineItemT.ID)), 49, 80)) AS PaymentID "
				" FROM LineItemT "
				" INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
				" INNER JOIN InsuredPartyT ON PaymentsT.InsuredPartyID = InsuredPartyT.PersonID "
				" INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				" WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type = 1) AND (RespTypeT.CategoryType = 7 AND RespTypeT.CategoryPlacement = 2) "
				" GROUP BY LineItemT.PatientID) LastPaymentInternalQ "
				"INNER JOIN LineItemT ON LastPaymentInternalQ.PaymentID = LineItemT.ID) FilterLastLOPSecondaryInsurancePaymentSubQ ON PersonT.ID = FilterLastLOPSecondaryInsurancePaymentSubQ.PatientID"), 

	ADD_FILTER_FIELD(569, "Last LOP Insurance (Secondary) Payment Amount", "FilterLastLOPSecondaryInsurancePaymentSubQ.LastPaymentAmount", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 568), 

	// (d.singleton 2012-09-14 13:33) - PLID 52661 add LOA filter
	ADD_FILTER_FIELD(570, "Last LOA Insurance (Primary) Payment Date", "dbo.AsDateNoTime(FilterLastLOAPrimaryInsurancePaymentSubQ.LastPaymentDate)", fboPerson, foEquality, foGreater, ftDate, NULL, //see note at the top about converting date
				"(SELECT LineItemT.PatientID, LineItemT.Amount AS LastPaymentAmount, LineItemT.Date AS LastPaymentDate FROM ( "
				" SELECT CONVERT(int, SUBSTRING(MAX(CONVERT(nvarchar, LineItemT.Date, 121) + ' ' + CONVERT(nvarchar, LineItemT.InputDate, 121) + ' ' + CONVERT(nvarchar, LineItemT.ID)), 49, 80)) AS PaymentID "
				" FROM LineItemT "
				" INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
				" INNER JOIN InsuredPartyT ON PaymentsT.InsuredPartyID = InsuredPartyT.PersonID "
				" INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				" WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type = 1) AND (RespTypeT.CategoryType = 8 AND RespTypeT.CategoryPlacement = 1) "
				" GROUP BY LineItemT.PatientID) LastPaymentInternalQ "
				"INNER JOIN LineItemT ON LastPaymentInternalQ.PaymentID = LineItemT.ID) FilterLastLOAPrimaryInsurancePaymentSubQ ON PersonT.ID = FilterLastLOAPrimaryInsurancePaymentSubQ.PatientID"), 

	ADD_FILTER_FIELD(571, "Last LOA Insurance (Primary) Payment Amount", "FilterLastLOAPrimaryInsurancePaymentSubQ.LastPaymentAmount", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 570), 

	ADD_FILTER_FIELD(572, "Last LOA Insurance (Secondary) Payment Date", "dbo.AsDateNoTime(FilterLastLOASecondaryInsurancePaymentSubQ.LastPaymentDate)", fboPerson, foEquality, foGreater, ftDate, NULL, //see note at the top about converting date
				"(SELECT LineItemT.PatientID, LineItemT.Amount AS LastPaymentAmount, LineItemT.Date AS LastPaymentDate FROM ( "
				" SELECT CONVERT(int, SUBSTRING(MAX(CONVERT(nvarchar, LineItemT.Date, 121) + ' ' + CONVERT(nvarchar, LineItemT.InputDate, 121) + ' ' + CONVERT(nvarchar, LineItemT.ID)), 49, 80)) AS PaymentID "
				" FROM LineItemT "
				" INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
				" INNER JOIN InsuredPartyT ON PaymentsT.InsuredPartyID = InsuredPartyT.PersonID "
				" INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				" WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type = 1) AND (RespTypeT.CategoryType = 8 AND RespTypeT.CategoryPlacement = 2) "
				" GROUP BY LineItemT.PatientID) LastPaymentInternalQ "
				"INNER JOIN LineItemT ON LastPaymentInternalQ.PaymentID = LineItemT.ID) FilterLastLOASecondaryInsurancePaymentSubQ ON PersonT.ID = FilterLastLOASecondaryInsurancePaymentSubQ.PatientID"), 

	ADD_FILTER_FIELD(573, "Last LOA Insurance (Secondary) Payment Amount", "FilterLastLOASecondaryInsurancePaymentSubQ.LastPaymentAmount", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 572), 
	
	//these regular balance fields contain only patients who have financial activity ... if a patient has never had a charge/pay/etc, he will not show up in this list as 0 balance
	ADD_FILTER_FIELD(166, "Balance (Financial Patients)", "FilterFinBalanceInfoSubQ.AllTotResp", fboPerson, foEquality, foGreater, ftCurrency, NULL, 
				"(SELECT StatementPatBalanceSub.PatFixID AS PersonID, SUM(StatementPatBalanceSub.PatCharge - StatementPatBalanceSub.PatPay + StatementPatBalanceSub.TotInsCharge - StatementPatBalanceSub.TotInsPays) AS AllTotResp, SUM(StatementPatBalanceSub.PatCharge - StatementPatBalanceSub.PatPay) AS PatTotResp, "
				"SUM(StatementPatBalanceSub.TotInsCharge - StatementPatBalanceSub.TotInsPays) AS InsTotResp, "
				"SUM(StatementPatBalanceSub.PriInsCharge - StatementPatBalanceSub.PriInsPays) AS InsPriResp, "
				"SUM(StatementPatBalanceSub.SecInsCharge - StatementPatBalanceSub.SecInsPays) AS InsSecResp, "
				"SUM(StatementPatBalanceSub.OthInsCharge - StatementPatBalanceSub.OthInsPays) AS InsOthResp, "
				"SUM(StatementPatBalanceSub.VisionPriInsCharge - StatementPatBalanceSub.VisionPriInsPays) AS VisionInsPriResp, "
				"SUM(StatementPatBalanceSub.VisionSecInsCharge - StatementPatBalanceSub.VisionSecInsPays) AS VisionInsSecResp, "
				"SUM(StatementPatBalanceSub.AutoPriInsCharge - StatementPatBalanceSub.AutoPriInsPays) AS AutoInsPriResp, "
				"SUM(StatementPatBalanceSub.AutoSecInsCharge - StatementPatBalanceSub.AutoSecInsPays) AS AutoInsSecResp, "
				"SUM(StatementPatBalanceSub.WorkersCompPriInsCharge - StatementPatBalanceSub.WorkersCompPriInsPays) AS WorkersCompInsPriResp, "
				"SUM(StatementPatBalanceSub.WorkersCompSecInsCharge - StatementPatBalanceSub.WorkersCompSecInsPays) AS WorkersCompInsSecResp, "
				"SUM(StatementPatBalanceSub.DentalPriInsCharge - StatementPatBalanceSub.DentalPriInsPays) AS DentalInsPriResp, "
				"SUM(StatementPatBalanceSub.DentalSecInsCharge - StatementPatBalanceSub.DentalSecInsPays) AS DentalInsSecResp, "
				"SUM(StatementPatBalanceSub.StudyPriInsCharge - StatementPatBalanceSub.StudyPriInsPays) AS StudyInsPriResp, "
				"SUM(StatementPatBalanceSub.StudySecInsCharge - StatementPatBalanceSub.StudySecInsPays) AS StudyInsSecResp, "
				"SUM(StatementPatBalanceSub.LOPPriInsCharge - StatementPatBalanceSub.LOPPriInsPays) AS LOPInsPriResp, "
				"SUM(StatementPatBalanceSub.LOPSecInsCharge - StatementPatBalanceSub.LOPSecInsPays) AS LOPInsSecResp, "
				"SUM(StatementPatBalanceSub.LOAPriInsCharge - StatementPatBalanceSub.LOAPriInsPays) AS LOAInsPriResp, "
				"SUM(StatementPatBalanceSub.LOASecInsCharge - StatementPatBalanceSub.LOASecInsPays) AS LOAInsSecResp "
				"FROM (SELECT StatementAllData.PatFixID,  PatCharge = CASE WHEN StatementAllData.Type = 10 THEN (StatementAllData.Total - StatementAllData.TotalInsurance) ELSE 0 END, PatPay = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN CASE WHEN StatementAllData.TotalInsurance <> 0 THEN 0 ELSE StatementAllData.Total END ELSE CASE WHEN StatementAllData.Type = 3 THEN CASE WHEN StatementAllDAta.TotalInsurance <> 0 THEN 0 ELSE StatementAllData.Total * - 1 END ELSE 0 END END, TotInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.TotalInsurance ELSE 0 END, TotInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.TotalInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.TotalInsurance * - 1 ELSE 0 END END, "
				"PriInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.PriInsurance ELSE 0 END,  "
				"PriInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.PriInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.PriInsurance * - 1 ELSE 0 END END, "
				"SecInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.SecInsurance ELSE 0 END,  "
				"SecInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.SecInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.SecInsurance * - 1 ELSE 0 END END, "
				"OthInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.OthInsurance ELSE 0 END,  "
				"OthInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.OthInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.OthInsurance * - 1 ELSE 0 END END, "
				"VisionPriInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.VisionPriInsurance ELSE 0 END,  "
				"VisionPriInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.VisionPriInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.VisionPriInsurance * - 1 ELSE 0 END END, "
				"VisionSecInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.VisionSecInsurance ELSE 0 END,  "
				"VisionSecInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.VisionSecInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.VisionSecInsurance * - 1 ELSE 0 END END, "
				"AutoPriInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.AutoPriInsurance ELSE 0 END,  "
				"AutoPriInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.AutoPriInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.AutoPriInsurance * - 1 ELSE 0 END END, "
				"AutoSecInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.AutoSecInsurance ELSE 0 END,  "
				"AutoSecInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.AutoSecInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.AutoSecInsurance * - 1 ELSE 0 END END, "
				"WorkersCompPriInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.WorkersCompPriInsurance ELSE 0 END,  "
				"WorkersCompPriInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.WorkersCompPriInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.WorkersCompPriInsurance * - 1 ELSE 0 END END, "
				"WorkersCompSecInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.WorkersCompSecInsurance ELSE 0 END,  "
				"WorkersCompSecInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.WorkersCompSecInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.WorkersCompSecInsurance * - 1 ELSE 0 END END, "
				"DentalPriInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.DentalPriInsurance ELSE 0 END,  "
				"DentalPriInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.DentalPriInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.DentalPriInsurance * - 1 ELSE 0 END END, "
				"DentalSecInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.DentalSecInsurance ELSE 0 END,  "
				"DentalSecInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.DentalSecInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.DentalSecInsurance * - 1 ELSE 0 END END, "
				"StudyPriInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.StudyPriInsurance ELSE 0 END,  "
				"StudyPriInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.StudyPriInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.StudyPriInsurance * - 1 ELSE 0 END END, "
				"StudySecInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.StudySecInsurance ELSE 0 END,  "
				"StudySecInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.StudySecInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.StudySecInsurance * - 1 ELSE 0 END END, "
				"LOPPriInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.LOPPriInsurance ELSE 0 END,  "
				"LOPPriInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.LOPPriInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.LOPPriInsurance * - 1 ELSE 0 END END, "
				"LOPSecInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.LOPSecInsurance ELSE 0 END,  "
				"LOPSecInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.LOPSecInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.LOPSecInsurance * - 1 ELSE 0 END END, "
				"LOAPriInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.LOAPriInsurance ELSE 0 END,  "
				"LOAPriInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.LOAPriInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.LOAPriInsurance * - 1 ELSE 0 END END, "
				"LOASecInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.LOASecInsurance ELSE 0 END,  "
				"LOASecInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.LOASecInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.LOASecInsurance * - 1 ELSE 0 END END "
				"FROM (SELECT StmtCharges.ID, StmtCharges.PatFixID,  StmtCharges.Type, StmtCharges.Total,  StmtCharges.TotalInsurance,  "
				"StmtCharges.PriInsurance,  StmtCharges.SecInsurance,  StmtCharges.OthInsurance, StmtCharges.VisionPriInsurance,  StmtCharges.VisionSecInsurance, StmtCharges.AutoPriInsurance,  StmtCharges.AutoSecInsurance, StmtCharges.WorkersCompPriInsurance,  StmtCharges.WorkersCompSecInsurance, StmtCharges.DentalPriInsurance,  StmtCharges.DentalSecInsurance, StmtCharges.StudyPriInsurance,  StmtCharges.StudySecInsurance, StmtCharges.LOPPriInsurance,  StmtCharges.LOPSecInsurance, StmtCharges.LOAPriInsurance,  StmtCharges.LOASecInsurance "
				"FROM (SELECT LineItemT.ID,  LineItemT.PatientID AS PatFixID,  10 AS Type, "
				"TotalInsurance = SUM(CASE WHEN ChargeRespT.InsuredPartyID IS NOT NULL  THEN ChargeRespT.Amount ELSE 0 END), "
				"Total = CASE WHEN SUM(ChargeRespT.Amount) IS NULL  THEN 0 ELSE SUM(ChargeRespT.Amount) END,  "
				"PriInsurance = CASE WHEN InsuredPartyT.RespTypeID = 1 THEN SUM(ChargeRespT.Amount) ELSE 0 END, "
				"SecInsurance = CASE WHEN InsuredPartyT.RespTypeID = 2 THEN SUM(ChargeRespT.Amount)  ELSE 0 END,  "
				"OthInsurance = CASE WHEN InsuredPartyT.RespTypeID <> 1 AND  InsuredPartyT.RespTypeID <> 2 THEN SUM(ChargeRespT.Amount) ELSE 0 END, "
				"VisionPriInsurance = CASE WHEN RespTypeT.CategoryType = 2 AND RespTypeT.CategoryPlacement = 1 THEN SUM(ChargeRespT.Amount) ELSE 0 END, "
				"VisionSecInsurance = CASE WHEN RespTypeT.CategoryType = 2 AND RespTypeT.CategoryPlacement = 2 THEN SUM(ChargeRespT.Amount)  ELSE 0 END,  "
				"AutoPriInsurance = CASE WHEN RespTypeT.CategoryType = 3 AND RespTypeT.CategoryPlacement = 1 THEN SUM(ChargeRespT.Amount) ELSE 0 END, "
				"AutoSecInsurance = CASE WHEN RespTypeT.CategoryType = 3 AND RespTypeT.CategoryPlacement = 2 THEN SUM(ChargeRespT.Amount)  ELSE 0 END,  "
				"WorkersCompPriInsurance = CASE WHEN RespTypeT.CategoryType = 4 AND RespTypeT.CategoryPlacement = 1 THEN SUM(ChargeRespT.Amount) ELSE 0 END, "
				"WorkersCompSecInsurance = CASE WHEN RespTypeT.CategoryType = 4 AND RespTypeT.CategoryPlacement = 2 THEN SUM(ChargeRespT.Amount)  ELSE 0 END,  "
				"DentalPriInsurance = CASE WHEN RespTypeT.CategoryType = 5 AND RespTypeT.CategoryPlacement = 1 THEN SUM(ChargeRespT.Amount) ELSE 0 END, "
				"DentalSecInsurance = CASE WHEN RespTypeT.CategoryType = 5 AND RespTypeT.CategoryPlacement = 2 THEN SUM(ChargeRespT.Amount)  ELSE 0 END, "
				"StudyPriInsurance = CASE WHEN RespTypeT.CategoryType = 6 AND RespTypeT.CategoryPlacement = 1 THEN SUM(ChargeRespT.Amount) ELSE 0 END, "
				"StudySecInsurance = CASE WHEN RespTypeT.CategoryType = 6 AND RespTypeT.CategoryPlacement = 2 THEN SUM(ChargeRespT.Amount)  ELSE 0 END, "
				"LOPPriInsurance = CASE WHEN RespTypeT.CategoryType = 7 AND RespTypeT.CategoryPlacement = 1 THEN SUM(ChargeRespT.Amount) ELSE 0 END, "
				"LOPSecInsurance = CASE WHEN RespTypeT.CategoryType = 7 AND RespTypeT.CategoryPlacement = 2 THEN SUM(ChargeRespT.Amount)  ELSE 0 END, "
				"LOAPriInsurance = CASE WHEN RespTypeT.CategoryType = 8 AND RespTypeT.CategoryPlacement = 1 THEN SUM(ChargeRespT.Amount) ELSE 0 END, "
				"LOASecInsurance = CASE WHEN RespTypeT.CategoryType = 8 AND RespTypeT.CategoryPlacement = 2 THEN SUM(ChargeRespT.Amount)  ELSE 0 END  "
				"FROM InsuredPartyT "
				"LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				"RIGHT OUTER JOIN ChargeRespT ON  InsuredPartyT.PersonID = ChargeRespT.InsuredPartyID RIGHT OUTER JOIN LineItemT ON  ChargeRespT.ChargeID = LineItemT.ID "
				"WHERE (LineItemT.Deleted = 0) AND  (LineItemT.Type = 10) "
				"GROUP BY InsuredPartyT.RespTypeID, RespTypeT.CategoryType, RespTypeT.CategoryPlacement, LineItemT.ID, LineItemT.PatientID,  LineItemT.Type) AS StmtCharges "
				"UNION "
				"SELECT StmtPays.ID, StmtPays.PatFixID,  StmtPays.Type, StmtPays.UnAppliedAmount,  StmtPays.TotalInsurance,  "
				"StmtPays.PriInsurance, StmtPays.SecInsurance,  StmtPays.OthInsurance, StmtPays.VisionPriInsurance, StmtPays.VisionSecInsurance, StmtPays.AutoPriInsurance, StmtPays.AutoSecInsurance, StmtPays.WorkersCompPriInsurance, StmtPays.WorkersCompSecInsurance, StmtPays.DentalPriInsurance, StmtPays.DentalSecInsurance, StmtPays.StudyPriInsurance, StmtPays.StudySecInsurance, StmtPays.LOPPriInsurance, StmtPays.LOPSecInsurance, StmtPays.LOAPriInsurance, StmtPays.LOASecInsurance "
				"FROM (SELECT LineItemT.ID, LineItemT.Type,  LineItemT.PatientID AS PatFixID, "
				"UnAppliedAmount = CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL  THEN 0 ELSE [AppliesT].[Amount] END)  ELSE MIN([LineItemT].[Amount])  - SUM(CASE WHEN [AppliesT].[Amount] IS NULL  THEN 0 ELSE [AppliesT].[Amount] END) END, "
				"TotalInsurance = CASE WHEN MIN([PaymentsT].[InsuredPartyID]) > 0 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount])  + SUM(CASE WHEN [AppliesT].[Amount] IS NULL  THEN 0 ELSE [AppliesT].[Amount] END)  ELSE MIN([LineItemT].[Amount])  - SUM(CASE WHEN [AppliesT].[Amount] IS NULL  THEN 0 ELSE [AppliesT].[Amount] END)  END ELSE 0 END, "
				"PriInsurance = CASE WHEN [InsuredPartyT].[RespTypeID] = 1 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END ELSE 0 END, "
				"SecInsurance = CASE WHEN [InsuredPartyT].[RespTypeID] = 2 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END  ELSE 0 END, "
				"OthInsurance = CASE WHEN [InsuredPartyT].[RespTypeID] <> 1 AND InsuredPartyT.RespTypeID <> 2 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END ELSE 0 END, "
				"VisionPriInsurance = CASE WHEN RespTypeT.CategoryType = 2 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END ELSE 0 END, "
				"VisionSecInsurance = CASE WHEN RespTypeT.CategoryType = 2 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END  ELSE 0 END, "
				"AutoPriInsurance = CASE WHEN RespTypeT.CategoryType = 3 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END ELSE 0 END, "
				"AutoSecInsurance = CASE WHEN RespTypeT.CategoryType = 3 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END  ELSE 0 END, "
				"WorkersCompPriInsurance = CASE WHEN RespTypeT.CategoryType = 4 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END ELSE 0 END, "
				"WorkersCompSecInsurance = CASE WHEN RespTypeT.CategoryType = 4 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END  ELSE 0 END, "
				"DentalPriInsurance = CASE WHEN RespTypeT.CategoryType = 5 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END ELSE 0 END, "
				"DentalSecInsurance = CASE WHEN RespTypeT.CategoryType = 5 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END  ELSE 0 END, "
				"StudyPriInsurance = CASE WHEN RespTypeT.CategoryType = 6 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END ELSE 0 END, "
				"StudySecInsurance = CASE WHEN RespTypeT.CategoryType = 6 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END  ELSE 0 END, "
				"LOPPriInsurance = CASE WHEN RespTypeT.CategoryType = 7 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END ELSE 0 END, "
				"LOPSecInsurance = CASE WHEN RespTypeT.CategoryType = 7 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END  ELSE 0 END, "
				"LOAPriInsurance = CASE WHEN RespTypeT.CategoryType = 8 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END ELSE 0 END, "
				"LOASecInsurance = CASE WHEN RespTypeT.CategoryType = 8 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END  ELSE 0 END "
				"FROM InsuredPartyT "
				"LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				"RIGHT OUTER JOIN PaymentsT ON InsuredPartyT.PersonID = PaymentsT.InsuredPartyID LEFT OUTER JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID RIGHT OUTER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
				"WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type < 10) "
				"GROUP BY LineItemT.ID, LineItemT.Type, LineItemT.PatientID, [InsuredPartyT].[RespTypeID], RespTypeT.CategoryType, RespTypeT.CategoryPlacement "
				"HAVING (CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END <> 0)) AS StmtPays "
				"UNION ALL "
				"SELECT StmtApplies.ChargeID, StmtApplies.PatFixID, StmtApplies.Type, StmtApplies.ApplyAmount, StmtApplies.TotalInsurance, "
				"StmtApplies.PriInsurance, StmtApplies.SecInsurance, StmtApplies.OthInsurance, StmtApplies.VisionPriInsurance, StmtApplies.VisionSecInsurance, StmtApplies.AutoPriInsurance, StmtApplies.AutoSecInsurance, StmtApplies.WorkersCompPriInsurance, StmtApplies.WorkersCompSecInsurance, StmtApplies.DentalPriInsurance, StmtApplies.DentalSecInsurance, StmtApplies.StudyPriInsurance, StmtApplies.StudySecInsurance, StmtApplies.LOPPriInsurance, StmtApplies.LOPSecInsurance, StmtApplies.LOAPriInsurance, StmtApplies.LOASecInsurance "
				"FROM (SELECT * FROM (SELECT AppliesT.SourceID AS ChargeID, LineItemT1.PatientID AS PatFixID, LineItemT1.Type, "
				"ApplyAmount = CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END, "
				"TotalInsurance = CASE WHEN [PaymentsT].[InsuredPartyID] > 0 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"PriInsurance = CASE WHEN InsuredPartyT.RespTypeID = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"SecInsurance = CASE WHEN InsuredPartyT.RespTypeID = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"OthInsurance = CASE WHEN InsuredPartyT.RespTypeID <> 1 AND InsuredPartyT.RespTypeID <> 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"VisionPriInsurance = CASE WHEN RespTypeT.CategoryType = 2 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"VisionSecInsurance = CASE WHEN RespTypeT.CategoryType = 2 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"AutoPriInsurance = CASE WHEN RespTypeT.CategoryType = 3 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"AutoSecInsurance = CASE WHEN RespTypeT.CategoryType = 3 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"WorkersCompPriInsurance = CASE WHEN RespTypeT.CategoryType = 4 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"WorkersCompSecInsurance = CASE WHEN RespTypeT.CategoryType = 4 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"DentalPriInsurance = CASE WHEN RespTypeT.CategoryType = 5 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"DentalSecInsurance = CASE WHEN RespTypeT.CategoryType = 5 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"StudyPriInsurance = CASE WHEN RespTypeT.CategoryType = 6 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"StudySecInsurance = CASE WHEN RespTypeT.CategoryType = 6 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"LOPPriInsurance = CASE WHEN RespTypeT.CategoryType = 7 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"LOPSecInsurance = CASE WHEN RespTypeT.CategoryType = 7 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"LOAPriInsurance = CASE WHEN RespTypeT.CategoryType = 8 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"LOASecInsurance = CASE WHEN RespTypeT.CategoryType = 8 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END "
				"FROM InsuredPartyT "
				"LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				"RIGHT OUTER JOIN PaymentsT ON InsuredPartyT.PersonID = PaymentsT.InsuredPartyID LEFT OUTER JOIN LineItemT LineItemT1 ON PaymentsT.ID = LineItemT1.ID RIGHT OUTER JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID "
				"WHERE (LineItemT1.Deleted = 0) AND (LineItemT1.Deleted = 0) AND (AppliesT.PointsToPayments = 0)) AS StatementDataAppliesCharges "
				"UNION ALL "
				"SELECT * FROM (SELECT AppliesT.DestID AS ChargeID, LineItemT1.PatientID AS PatFixID, LineItemT1.Type, "
				"ApplyAmount = CASE WHEN LineItemT1.Type = 3 THEN -1 * AppliesT.Amount ELSE AppliesT.Amount END, "
				"TotalInsurance = CASE WHEN [PaymentsT].[InsuredPartyID] > 0 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"PriInsurance = CASE WHEN InsuredPartyT.RespTypeID = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"SecInsurance = CASE WHEN InsuredPartyT.RespTypeID = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"OthInsurance = CASE WHEN InsuredPartyT.RespTypeID <> 1 AND InsuredPartyT.RespTypeID <> 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END,  "
				"VisionPriInsurance = CASE WHEN RespTypeT.CategoryType = 2 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"VisionSecInsurance = CASE WHEN RespTypeT.CategoryType = 2 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"AutoPriInsurance = CASE WHEN RespTypeT.CategoryType = 3 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"AutoSecInsurance = CASE WHEN RespTypeT.CategoryType = 3 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"WorkersCompPriInsurance = CASE WHEN RespTypeT.CategoryType = 4 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"WorkersCompSecInsurance = CASE WHEN RespTypeT.CategoryType = 4 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"DentalPriInsurance = CASE WHEN RespTypeT.CategoryType = 5 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"DentalSecInsurance = CASE WHEN RespTypeT.CategoryType = 5 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"StudyPriInsurance = CASE WHEN RespTypeT.CategoryType = 6 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"StudySecInsurance = CASE WHEN RespTypeT.CategoryType = 6 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"LOPPriInsurance = CASE WHEN RespTypeT.CategoryType = 7 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"LOPSecInsurance = CASE WHEN RespTypeT.CategoryType = 7 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"LOAPriInsurance = CASE WHEN RespTypeT.CategoryType = 8 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"LOASecInsurance = CASE WHEN RespTypeT.CategoryType = 8 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END "
				"FROM PaymentsT "
				"LEFT OUTER JOIN InsuredPartyT ON PaymentsT.InsuredPartyID = InsuredPartyT.PersonID "
				"LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				"LEFT OUTER JOIN LineItemT LineItemT1 ON PaymentsT.ID = LineItemT1.ID RIGHT OUTER JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID "
				"WHERE (LineItemT1.Deleted = 0) AND (LineItemT1.Deleted = 0) AND (AppliesT.PointsToPayments = 1)) AS StatementDataAppliesPays) AS StmtApplies) AS StatementAllData) AS StatementPatBalanceSub "
				"GROUP BY StatementPatBalanceSub.PatFixID) FilterFinBalanceInfoSubQ ON PersonT.ID = FilterFinBalanceInfoSubQ.PersonID ", FILTER_DEPENDS_ON_BASE, "LineItemT", "LineItemT.Deleted = 0 AND LineItemT.PatientID = PersonT.ID AND LineItemT.Type <> 11"),
	ADD_FILTER_FIELD(167, "Balance (Fin Pts) Patient Total", "FilterFinBalanceInfoSubQ.PatTotResp", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 166, "LineItemT", "LineItemT.Deleted = 0 AND LineItemT.PatientID = PersonT.ID AND LineItemT.Type <> 11"), 
	ADD_FILTER_FIELD(168, "Balance (Fin Pts) Insurance Total", "FilterFinBalanceInfoSubQ.InsTotResp", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 166, "LineItemT", "LineItemT.Deleted = 0 AND LineItemT.PatientID = PersonT.ID AND LineItemT.Type <> 11"), 
	ADD_FILTER_FIELD(169, "Balance (Fin Pts) Insurance Primary", "FilterFinBalanceInfoSubQ.InsPriResp", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 166, "LineItemT", "LineItemT.Deleted = 0 AND LineItemT.PatientID = PersonT.ID AND LineItemT.Type <> 11"), 
	ADD_FILTER_FIELD(170, "Balance (Fin Pts) Insurance Secondary", "FilterFinBalanceInfoSubQ.InsSecResp", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 166, "LineItemT", "LineItemT.Deleted = 0 AND LineItemT.PatientID = PersonT.ID AND LineItemT.Type <> 11"), 
	ADD_FILTER_FIELD(171, "Balance (Fin Pts) Insurance Other", "FilterFinBalanceInfoSubQ.InsOthResp", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 166, "LineItemT", "LineItemT.Deleted = 0 AND LineItemT.PatientID = PersonT.ID AND LineItemT.Type <> 11"), 

	// (j.jones 2011-04-01 14:52) - PLID 43109 - added Vision Primary & Secondary balance filters
	ADD_FILTER_FIELD(475, "Balance (Fin Pts) Insurance Vision Primary", "FilterFinBalanceInfoSubQ.VisionInsPriResp", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 166, "LineItemT", "LineItemT.Deleted = 0 AND LineItemT.PatientID = PersonT.ID AND LineItemT.Type <> 11"), 
	ADD_FILTER_FIELD(476, "Balance (Fin Pts) Insurance Vision Secondary", "FilterFinBalanceInfoSubQ.VisionInsSecResp", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 166, "LineItemT", "LineItemT.Deleted = 0 AND LineItemT.PatientID = PersonT.ID AND LineItemT.Type <> 11"), 

	// (d.singleton 2012-09-11 12:22) - PLID 51808 add auto filter
	// (d.singleton 2012-09-14 13:34) - PLID 52657 add workers comp filter
	// (d.singleton 2012-09-14 13:34) - PLID 52658 add dental filter
	// (d.singleton 2012-09-14 13:35) - PLID 52659 add study filter
	// (d.singleton 2012-09-14 13:35) - PLID 52660 add LOP filter
	// (d.singleton 2012-09-14 13:35) - PLID 52661 add LOA filter
	ADD_FILTER_FIELD(574, "Balance (Fin Pts) Insurance Auto Primary", "FilterFinBalanceInfoSubQ.AutoInsPriResp", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 166, "LineItemT", "LineItemT.Deleted = 0 AND LineItemT.PatientID = PersonT.ID AND LineItemT.Type <> 11"), 
	ADD_FILTER_FIELD(575, "Balance (Fin Pts) Insurance Auto Secondary", "FilterFinBalanceInfoSubQ.AutoInsSecResp", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 166, "LineItemT", "LineItemT.Deleted = 0 AND LineItemT.PatientID = PersonT.ID AND LineItemT.Type <> 11"),
	ADD_FILTER_FIELD(576, "Balance (Fin Pts) Insurance Workers Comp Primary", "FilterFinBalanceInfoSubQ.WorkersCompInsPriResp", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 166, "LineItemT", "LineItemT.Deleted = 0 AND LineItemT.PatientID = PersonT.ID AND LineItemT.Type <> 11"), 
	ADD_FILTER_FIELD(577, "Balance (Fin Pts) Insurance Workers Comp Secondary", "FilterFinBalanceInfoSubQ.WorkersCompInsSecResp", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 166, "LineItemT", "LineItemT.Deleted = 0 AND LineItemT.PatientID = PersonT.ID AND LineItemT.Type <> 11"),
	ADD_FILTER_FIELD(578, "Balance (Fin Pts) Insurance Dental Primary", "FilterFinBalanceInfoSubQ.DentalInsPriResp", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 166, "LineItemT", "LineItemT.Deleted = 0 AND LineItemT.PatientID = PersonT.ID AND LineItemT.Type <> 11"), 
	ADD_FILTER_FIELD(579, "Balance (Fin Pts) Insurance Dental Secondary", "FilterFinBalanceInfoSubQ.DentalInsSecResp", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 166, "LineItemT", "LineItemT.Deleted = 0 AND LineItemT.PatientID = PersonT.ID AND LineItemT.Type <> 11"),
	ADD_FILTER_FIELD(580, "Balance (Fin Pts) Insurance Study Primary", "FilterFinBalanceInfoSubQ.StudyInsPriResp", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 166, "LineItemT", "LineItemT.Deleted = 0 AND LineItemT.PatientID = PersonT.ID AND LineItemT.Type <> 11"), 
	ADD_FILTER_FIELD(581, "Balance (Fin Pts) Insurance Study Secondary", "FilterFinBalanceInfoSubQ.StudyInsSecResp", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 166, "LineItemT", "LineItemT.Deleted = 0 AND LineItemT.PatientID = PersonT.ID AND LineItemT.Type <> 11"),
	ADD_FILTER_FIELD(582, "Balance (Fin Pts) Insurance LOP Primary", "FilterFinBalanceInfoSubQ.LOPInsPriResp", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 166, "LineItemT", "LineItemT.Deleted = 0 AND LineItemT.PatientID = PersonT.ID AND LineItemT.Type <> 11"), 
	ADD_FILTER_FIELD(583, "Balance (Fin Pts) Insurance LOP Secondary", "FilterFinBalanceInfoSubQ.LOPInsSecResp", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 166, "LineItemT", "LineItemT.Deleted = 0 AND LineItemT.PatientID = PersonT.ID AND LineItemT.Type <> 11"),
	ADD_FILTER_FIELD(584, "Balance (Fin Pts) Insurance LOA Primary", "FilterFinBalanceInfoSubQ.LOAInsPriResp", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 166, "LineItemT", "LineItemT.Deleted = 0 AND LineItemT.PatientID = PersonT.ID AND LineItemT.Type <> 11"), 
	ADD_FILTER_FIELD(585, "Balance (Fin Pts) Insurance LOA Secondary", "FilterFinBalanceInfoSubQ.LOAInsSecResp", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 166, "LineItemT", "LineItemT.Deleted = 0 AND LineItemT.PatientID = PersonT.ID AND LineItemT.Type <> 11"),

	//the _All_ fields contain all patients who have had financial activity, and all patients who have had none - this is really only relevant if you're filtering on = 0 balances
	ADD_FILTER_FIELD(193, "Balance (All Patients)", "FilterAllBalanceInfoSubQ.AllTotResp", fboPerson, foEquality, foGreater, ftCurrency, NULL, 
				"(SELECT ID AS PersonID, CASE WHEN AllTotResp IS NULL THEN 0 ELSE AllTotResp END AS AllTotResp, "
				"CASE WHEN SubQ.PatTotResp IS NULL THEN 0 ELSE SubQ.PatTotResp END AS PatTotResp, "
				"CASE WHEN SubQ.InsTotResp IS NULL THEN 0 ELSE SubQ.InsTotResp END AS InsTotResp, "
				"CASE WHEN SubQ.InsPriResp IS NULL THEN 0 ELSE SubQ.InsPriResp END AS InsPriResp, "
				"CASE WHEN SubQ.InsSecResp IS NULL THEN 0 ELSE SubQ.InsSecResp END AS InsSecResp, "
				"CASE WHEN SubQ.InsOthResp IS NULL THEN 0 ELSE SubQ.InsOthResp END AS InsOthResp, "
				"CASE WHEN SubQ.VisionInsPriResp IS NULL THEN 0 ELSE SubQ.VisionInsPriResp END AS VisionInsPriResp, "
				"CASE WHEN SubQ.VisionInsSecResp IS NULL THEN 0 ELSE SubQ.VisionInsSecResp END AS VisionInsSecResp, "
				"CASE WHEN SubQ.AutoInsPriResp IS NULL THEN 0 ELSE SubQ.AutoInsPriResp END AS AutoInsPriResp, "
				"CASE WHEN SubQ.AutoInsSecResp IS NULL THEN 0 ELSE SubQ.AutoInsSecResp END AS AutoInsSecResp, "
				"CASE WHEN SubQ.WorkersCompInsPriResp IS NULL THEN 0 ELSE SubQ.WorkersCompInsPriResp END AS WorkersCompInsPriResp, "
				"CASE WHEN SubQ.WorkersCompInsSecResp IS NULL THEN 0 ELSE SubQ.WorkersCompInsSecResp END AS WorkersCompInsSecResp, "
				"CASE WHEN SubQ.DentalInsPriResp IS NULL THEN 0 ELSE SubQ.DentalInsPriResp END AS DentalInsPriResp, "
				"CASE WHEN SubQ.DentalInsSecResp IS NULL THEN 0 ELSE SubQ.DentalInsSecResp END AS DentalInsSecResp, "
				"CASE WHEN SubQ.StudyInsPriResp IS NULL THEN 0 ELSE SubQ.StudyInsPriResp END AS StudyInsPriResp, "
				"CASE WHEN SubQ.StudyInsSecResp IS NULL THEN 0 ELSE SubQ.StudyInsSecResp END AS StudyInsSecResp, "
				"CASE WHEN SubQ.LOPInsPriResp IS NULL THEN 0 ELSE SubQ.LOPInsPriResp END AS LOPInsPriResp, "
				"CASE WHEN SubQ.LOPInsSecResp IS NULL THEN 0 ELSE SubQ.LOPInsSecResp END AS LOPInsSecResp, "
				"CASE WHEN SubQ.LOAInsPriResp IS NULL THEN 0 ELSE SubQ.LOAInsPriResp END AS LOAInsPriResp, "
				"CASE WHEN SubQ.LOAInsSecResp IS NULL THEN 0 ELSE SubQ.LOAInsSecResp END AS LOAInsSecResp "
				"FROM PersonT WITH(NOLOCK) LEFT JOIN "
				"(SELECT StatementPatBalanceSub.PatFixID AS PersonID, SUM(StatementPatBalanceSub.PatCharge - StatementPatBalanceSub.PatPay + StatementPatBalanceSub.TotInsCharge - StatementPatBalanceSub.TotInsPays) AS AllTotResp, SUM(StatementPatBalanceSub.PatCharge - StatementPatBalanceSub.PatPay) AS PatTotResp, "
				"SUM(StatementPatBalanceSub.TotInsCharge - StatementPatBalanceSub.TotInsPays) AS InsTotResp, "
				"SUM(StatementPatBalanceSub.PriInsCharge - StatementPatBalanceSub.PriInsPays) AS InsPriResp, "
				"SUM(StatementPatBalanceSub.SecInsCharge - StatementPatBalanceSub.SecInsPays) AS InsSecResp, "
				"SUM(StatementPatBalanceSub.OthInsCharge - StatementPatBalanceSub.OthInsPays) AS InsOthResp, "
				"SUM(StatementPatBalanceSub.VisionPriInsCharge - StatementPatBalanceSub.VisionPriInsPays) AS VisionInsPriResp, "
				"SUM(StatementPatBalanceSub.VisionSecInsCharge - StatementPatBalanceSub.VisionSecInsPays) AS VisionInsSecResp, "
				"SUM(StatementPatBalanceSub.AutoPriInsCharge - StatementPatBalanceSub.AutoPriInsPays) AS AutoInsPriResp, "
				"SUM(StatementPatBalanceSub.AutoSecInsCharge - StatementPatBalanceSub.AutoSecInsPays) AS AutoInsSecResp, "
				"SUM(StatementPatBalanceSub.WorkersCompPriInsCharge - StatementPatBalanceSub.WorkersCompPriInsPays) AS WorkersCompInsPriResp, "
				"SUM(StatementPatBalanceSub.WorkersCompSecInsCharge - StatementPatBalanceSub.WorkersCompSecInsPays) AS WorkersCompInsSecResp, "
				"SUM(StatementPatBalanceSub.DentalPriInsCharge - StatementPatBalanceSub.DentalPriInsPays) AS DentalInsPriResp, "
				"SUM(StatementPatBalanceSub.DentalSecInsCharge - StatementPatBalanceSub.DentalSecInsPays) AS DentalInsSecResp, "
				"SUM(StatementPatBalanceSub.StudyPriInsCharge - StatementPatBalanceSub.StudyPriInsPays) AS StudyInsPriResp, "
				"SUM(StatementPatBalanceSub.StudySecInsCharge - StatementPatBalanceSub.StudySecInsPays) AS StudyInsSecResp, "
				"SUM(StatementPatBalanceSub.LOPPriInsCharge - StatementPatBalanceSub.LOPPriInsPays) AS LOPInsPriResp, "
				"SUM(StatementPatBalanceSub.LOPSecInsCharge - StatementPatBalanceSub.LOPSecInsPays) AS LOPInsSecResp, "
				"SUM(StatementPatBalanceSub.LOAPriInsCharge - StatementPatBalanceSub.LOAPriInsPays) AS LOAInsPriResp, "
				"SUM(StatementPatBalanceSub.LOASecInsCharge - StatementPatBalanceSub.LOASecInsPays) AS LOAInsSecResp "
				"FROM (SELECT StatementAllData.PatFixID,  PatCharge = CASE WHEN StatementAllData.Type = 10 THEN (StatementAllData.Total - StatementAllData.TotalInsurance) ELSE 0 END, PatPay = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN CASE WHEN StatementAllData.TotalInsurance <> 0 THEN 0 ELSE StatementAllData.Total END ELSE CASE WHEN StatementAllData.Type = 3 THEN CASE WHEN StatementAllDAta.TotalInsurance <> 0 THEN 0 ELSE StatementAllData.Total * - 1 END ELSE 0 END END, TotInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.TotalInsurance ELSE 0 END, TotInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.TotalInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.TotalInsurance * - 1 ELSE 0 END END, "
				"PriInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.PriInsurance ELSE 0 END,  "
				"PriInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.PriInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.PriInsurance * - 1 ELSE 0 END END, "
				"SecInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.SecInsurance ELSE 0 END,  "
				"SecInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.SecInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.SecInsurance * - 1 ELSE 0 END END, "
				"OthInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.OthInsurance ELSE 0 END,  "
				"OthInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.OthInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.OthInsurance * - 1 ELSE 0 END END, "
				"VisionPriInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.VisionPriInsurance ELSE 0 END,  "
				"VisionPriInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.VisionPriInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.VisionPriInsurance * - 1 ELSE 0 END END, "
				"VisionSecInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.VisionSecInsurance ELSE 0 END,  "
				"VisionSecInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.VisionSecInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.VisionSecInsurance * - 1 ELSE 0 END END, "
				"AutoPriInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.AutoPriInsurance ELSE 0 END,  "
				"AutoPriInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.AutoPriInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.AutoPriInsurance * - 1 ELSE 0 END END, "
				"AutoSecInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.AutoSecInsurance ELSE 0 END,  "
				"AutoSecInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.AutoSecInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.AutoSecInsurance * - 1 ELSE 0 END END, "
				"WorkersCompPriInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.WorkersCompPriInsurance ELSE 0 END,  "
				"WorkersCompPriInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.WorkersCompPriInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.WorkersCompPriInsurance * - 1 ELSE 0 END END, "
				"WorkersCompSecInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.WorkersCompSecInsurance ELSE 0 END,  "
				"WorkersCompSecInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.WorkersCompSecInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.WorkersCompSecInsurance * - 1 ELSE 0 END END, "
				"DentalPriInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.DentalPriInsurance ELSE 0 END,  "
				"DentalPriInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.DentalPriInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.DentalPriInsurance * - 1 ELSE 0 END END, "
				"DentalSecInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.DentalSecInsurance ELSE 0 END,  "
				"DentalSecInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.DentalSecInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.DentalSecInsurance * - 1 ELSE 0 END END, "
				"StudyPriInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.StudyPriInsurance ELSE 0 END,  "
				"StudyPriInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.StudyPriInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.StudyPriInsurance * - 1 ELSE 0 END END, "
				"StudySecInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.StudySecInsurance ELSE 0 END,  "
				"StudySecInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.StudySecInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.StudySecInsurance * - 1 ELSE 0 END END, "
				"LOPPriInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.LOPPriInsurance ELSE 0 END,  "
				"LOPPriInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.LOPPriInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.LOPPriInsurance * - 1 ELSE 0 END END, "
				"LOPSecInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.LOPSecInsurance ELSE 0 END,  "
				"LOPSecInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.LOPSecInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.LOPSecInsurance * - 1 ELSE 0 END END, "
				"LOAPriInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.LOAPriInsurance ELSE 0 END,  "
				"LOAPriInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.LOAPriInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.LOAPriInsurance * - 1 ELSE 0 END END, "
				"LOASecInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.LOASecInsurance ELSE 0 END,  "
				"LOASecInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.LOASecInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.LOASecInsurance * - 1 ELSE 0 END END "
				"FROM (SELECT StmtCharges.ID, StmtCharges.PatFixID,  StmtCharges.Type, StmtCharges.Total, StmtCharges.TotalInsurance,  "
				"StmtCharges.PriInsurance, StmtCharges.SecInsurance, StmtCharges.OthInsurance, StmtCharges.VisionPriInsurance, StmtCharges.VisionSecInsurance, StmtCharges.AutoPriInsurance, StmtCharges.AutoSecInsurance, StmtCharges.WorkersCompPriInsurance, StmtCharges.WorkersCompSecInsurance, StmtCharges.DentalPriInsurance, StmtCharges.DentalSecInsurance, StmtCharges.StudyPriInsurance, StmtCharges.StudySecInsurance, StmtCharges.LOPPriInsurance, StmtCharges.LOPSecInsurance, StmtCharges.LOAPriInsurance, StmtCharges.LOASecInsurance "
				"FROM (SELECT LineItemT.ID,  LineItemT.PatientID AS PatFixID,  10 AS Type, TotalInsurance = SUM(CASE WHEN ChargeRespT.InsuredPartyID IS NOT NULL  THEN ChargeRespT.Amount ELSE 0 END), Total = CASE WHEN SUM(ChargeRespT.Amount) IS NULL  THEN 0 ELSE SUM(ChargeRespT.Amount) END,  "
				"PriInsurance = CASE WHEN InsuredPartyT.RespTypeID = 1 THEN SUM(ChargeRespT.Amount) ELSE 0 END, "
				"SecInsurance = CASE WHEN InsuredPartyT.RespTypeID = 2 THEN SUM(ChargeRespT.Amount) ELSE 0 END, "
				"OthInsurance = CASE WHEN InsuredPartyT.RespTypeID <> 1 AND  InsuredPartyT.RespTypeID <> 2 THEN SUM(ChargeRespT.Amount) ELSE 0 END, "
				"VisionPriInsurance = CASE WHEN RespTypeT.CategoryType = 2 AND RespTypeT.CategoryPlacement = 1 THEN SUM(ChargeRespT.Amount) ELSE 0 END, "
				"VisionSecInsurance = CASE WHEN RespTypeT.CategoryType = 2 AND RespTypeT.CategoryPlacement = 2 THEN SUM(ChargeRespT.Amount) ELSE 0 END, "
				"AutoPriInsurance = CASE WHEN RespTypeT.CategoryType = 3 AND RespTypeT.CategoryPlacement = 1 THEN SUM(ChargeRespT.Amount) ELSE 0 END, "
				"AutoSecInsurance = CASE WHEN RespTypeT.CategoryType = 3 AND RespTypeT.CategoryPlacement = 2 THEN SUM(ChargeRespT.Amount) ELSE 0 END, "
				"WorkersCompPriInsurance = CASE WHEN RespTypeT.CategoryType = 4 AND RespTypeT.CategoryPlacement = 1 THEN SUM(ChargeRespT.Amount) ELSE 0 END, "
				"WorkersCompSecInsurance = CASE WHEN RespTypeT.CategoryType = 4 AND RespTypeT.CategoryPlacement = 2 THEN SUM(ChargeRespT.Amount) ELSE 0 END, "
				"DentalPriInsurance = CASE WHEN RespTypeT.CategoryType = 5 AND RespTypeT.CategoryPlacement = 1 THEN SUM(ChargeRespT.Amount) ELSE 0 END, "
				"DentalSecInsurance = CASE WHEN RespTypeT.CategoryType = 5 AND RespTypeT.CategoryPlacement = 2 THEN SUM(ChargeRespT.Amount) ELSE 0 END, "
				"StudyPriInsurance = CASE WHEN RespTypeT.CategoryType = 6 AND RespTypeT.CategoryPlacement = 1 THEN SUM(ChargeRespT.Amount) ELSE 0 END, "
				"StudySecInsurance = CASE WHEN RespTypeT.CategoryType = 6 AND RespTypeT.CategoryPlacement = 2 THEN SUM(ChargeRespT.Amount) ELSE 0 END, "
				"LOPPriInsurance = CASE WHEN RespTypeT.CategoryType = 7 AND RespTypeT.CategoryPlacement = 1 THEN SUM(ChargeRespT.Amount) ELSE 0 END, "
				"LOPSecInsurance = CASE WHEN RespTypeT.CategoryType = 7 AND RespTypeT.CategoryPlacement = 2 THEN SUM(ChargeRespT.Amount) ELSE 0 END, "
				"LOAPriInsurance = CASE WHEN RespTypeT.CategoryType = 8 AND RespTypeT.CategoryPlacement = 1 THEN SUM(ChargeRespT.Amount) ELSE 0 END, "
				"LOASecInsurance = CASE WHEN RespTypeT.CategoryType = 8 AND RespTypeT.CategoryPlacement = 2 THEN SUM(ChargeRespT.Amount) ELSE 0 END "
				"FROM InsuredPartyT "
				"LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				"RIGHT OUTER JOIN ChargeRespT ON  InsuredPartyT.PersonID = ChargeRespT.InsuredPartyID RIGHT OUTER JOIN LineItemT ON  ChargeRespT.ChargeID = LineItemT.ID "
				"WHERE (LineItemT.Deleted = 0) AND  (LineItemT.Type = 10) "
				"GROUP BY InsuredPartyT.RespTypeID, RespTypeT.CategoryType, RespTypeT.CategoryPlacement, LineItemT.ID, LineItemT.PatientID,  LineItemT.Type) AS StmtCharges "
				"UNION "
				"SELECT StmtPays.ID, StmtPays.PatFixID,  StmtPays.Type, StmtPays.UnAppliedAmount,  StmtPays.TotalInsurance,  "
				"StmtPays.PriInsurance, StmtPays.SecInsurance,  StmtPays.OthInsurance, StmtPays.VisionPriInsurance, StmtPays.VisionSecInsurance, StmtPays.AutoPriInsurance, StmtPays.AutoSecInsurance, "
				"StmtPays.WorkersCompPriInsurance, StmtPays.WorkersCompSecInsurance, StmtPays.DentalPriInsurance, StmtPays.DentalSecInsurance, StmtPays.StudyPriInsurance, StmtPays.StudySecInsurance, "
				"StmtPays.LOPPriInsurance, StmtPays.LOPSecInsurance, StmtPays.LOAPriInsurance, StmtPays.LOASecInsurance "
				"FROM (SELECT LineItemT.ID, LineItemT.Type,  LineItemT.PatientID AS PatFixID, UnAppliedAmount = CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL  THEN 0 ELSE [AppliesT].[Amount] END)  ELSE MIN([LineItemT].[Amount])  - SUM(CASE WHEN [AppliesT].[Amount] IS NULL  THEN 0 ELSE [AppliesT].[Amount] END) END, TotalInsurance = CASE WHEN MIN([PaymentsT].[InsuredPartyID]) > 0 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount])  + SUM(CASE WHEN [AppliesT].[Amount] IS NULL  THEN 0 ELSE [AppliesT].[Amount] END)  ELSE MIN([LineItemT].[Amount])  - SUM(CASE WHEN [AppliesT].[Amount] IS NULL  THEN 0 ELSE [AppliesT].[Amount] END)  END ELSE 0 END, "
				"PriInsurance = CASE WHEN [InsuredPartyT].[RespTypeID] = 1 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END ELSE 0 END, "
				"SecInsurance = CASE WHEN [InsuredPartyT].[RespTypeID] = 2 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END  ELSE 0 END, "
				"OthInsurance = CASE WHEN [InsuredPartyT].[RespTypeID] <> 1 AND InsuredPartyT.RespTypeID <> 2 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END ELSE 0 END, "
				"VisionPriInsurance = CASE WHEN RespTypeT.CategoryType = 2 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END ELSE 0 END, "
				"VisionSecInsurance = CASE WHEN RespTypeT.CategoryType = 2 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END  ELSE 0 END, "
				"AutoPriInsurance = CASE WHEN RespTypeT.CategoryType = 3 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END ELSE 0 END, "
				"AutoSecInsurance = CASE WHEN RespTypeT.CategoryType = 3 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END  ELSE 0 END, "
				"WorkersCompPriInsurance = CASE WHEN RespTypeT.CategoryType = 4 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END ELSE 0 END, "
				"WorkersCompSecInsurance = CASE WHEN RespTypeT.CategoryType = 4 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END  ELSE 0 END, "
				"DentalPriInsurance = CASE WHEN RespTypeT.CategoryType = 5 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END ELSE 0 END, "
				"DentalSecInsurance = CASE WHEN RespTypeT.CategoryType = 5 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END  ELSE 0 END, "
				"StudyPriInsurance = CASE WHEN RespTypeT.CategoryType = 6 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END ELSE 0 END, "
				"StudySecInsurance = CASE WHEN RespTypeT.CategoryType = 6 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END  ELSE 0 END, "
				"LOPPriInsurance = CASE WHEN RespTypeT.CategoryType = 7 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END ELSE 0 END, "
				"LOPSecInsurance = CASE WHEN RespTypeT.CategoryType = 7 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END  ELSE 0 END, "
				"LOAPriInsurance = CASE WHEN RespTypeT.CategoryType = 8 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END ELSE 0 END, "
				"LOASecInsurance = CASE WHEN RespTypeT.CategoryType = 8 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END  ELSE 0 END "
				"FROM InsuredPartyT "
				"LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				"RIGHT OUTER JOIN PaymentsT ON InsuredPartyT.PersonID = PaymentsT.InsuredPartyID LEFT OUTER JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID RIGHT OUTER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
				"WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type < 10) "
				"GROUP BY LineItemT.ID, LineItemT.Type, LineItemT.PatientID, [InsuredPartyT].[RespTypeID], RespTypeT.CategoryType, RespTypeT.CategoryPlacement "
				"HAVING (CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END <> 0)) AS StmtPays "
				"UNION ALL "
				"SELECT StmtApplies.ChargeID, StmtApplies.PatFixID, StmtApplies.Type, StmtApplies.ApplyAmount, StmtApplies.TotalInsurance, "
				"StmtApplies.PriInsurance, StmtApplies.SecInsurance, StmtApplies.OthInsurance, StmtApplies.VisionPriInsurance, StmtApplies.VisionSecInsurance, StmtApplies.AutoPriInsurance, StmtApplies.AutoSecInsurance, "
				"StmtApplies.WorkersCompPriInsurance, StmtApplies.WorkersCompSecInsurance, StmtApplies.DentalPriInsurance, StmtApplies.DentalSecInsurance, StmtApplies.StudyPriInsurance, StmtApplies.StudySecInsurance, "
				"StmtApplies.LOPPriInsurance, StmtApplies.LOPSecInsurance, StmtApplies.LOAPriInsurance, StmtApplies.LOASecInsurance "
				"FROM (SELECT * FROM (SELECT AppliesT.SourceID AS ChargeID, LineItemT1.PatientID AS PatFixID, LineItemT1.Type, ApplyAmount = CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END, TotalInsurance = CASE WHEN [PaymentsT].[InsuredPartyID] > 0 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"PriInsurance = CASE WHEN InsuredPartyT.RespTypeID = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"SecInsurance = CASE WHEN InsuredPartyT.RespTypeID = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"OthInsurance = CASE WHEN InsuredPartyT.RespTypeID <> 1 AND InsuredPartyT.RespTypeID <> 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"VisionPriInsurance = CASE WHEN RespTypeT.CategoryType = 2 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"VisionSecInsurance = CASE WHEN RespTypeT.CategoryType = 2 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"AutoPriInsurance = CASE WHEN RespTypeT.CategoryType = 3 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"AutoSecInsurance = CASE WHEN RespTypeT.CategoryType = 3 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"WorkersCompPriInsurance = CASE WHEN RespTypeT.CategoryType = 4 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"WorkersCompSecInsurance = CASE WHEN RespTypeT.CategoryType = 4 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"DentalPriInsurance = CASE WHEN RespTypeT.CategoryType = 5 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"DentalSecInsurance = CASE WHEN RespTypeT.CategoryType = 5 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"StudyPriInsurance = CASE WHEN RespTypeT.CategoryType = 6 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"StudySecInsurance = CASE WHEN RespTypeT.CategoryType = 6 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"LOPPriInsurance = CASE WHEN RespTypeT.CategoryType = 7 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"LOPSecInsurance = CASE WHEN RespTypeT.CategoryType = 7 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"LOAPriInsurance = CASE WHEN RespTypeT.CategoryType = 8 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"LOASecInsurance = CASE WHEN RespTypeT.CategoryType = 8 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END "
				"FROM InsuredPartyT "
				"LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				"RIGHT OUTER JOIN PaymentsT ON InsuredPartyT.PersonID = PaymentsT.InsuredPartyID LEFT OUTER JOIN LineItemT LineItemT1 ON PaymentsT.ID = LineItemT1.ID RIGHT OUTER JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID "
				"WHERE (LineItemT1.Deleted = 0) AND (LineItemT1.Deleted = 0) AND (AppliesT.PointsToPayments = 0)) AS StatementDataAppliesCharges "
				"UNION ALL "
				"SELECT * FROM (SELECT AppliesT.DestID AS ChargeID, LineItemT1.PatientID AS PatFixID, LineItemT1.Type, ApplyAmount = CASE WHEN LineItemT1.Type = 3 THEN -1 * AppliesT.Amount ELSE AppliesT.Amount END, TotalInsurance = CASE WHEN [PaymentsT].[InsuredPartyID] > 0 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"PriInsurance = CASE WHEN InsuredPartyT.RespTypeID = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"SecInsurance = CASE WHEN InsuredPartyT.RespTypeID = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"OthInsurance = CASE WHEN InsuredPartyT.RespTypeID <> 1 AND InsuredPartyT.RespTypeID <> 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"VisionPriInsurance = CASE WHEN RespTypeT.CategoryType = 2 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"VisionSecInsurance = CASE WHEN RespTypeT.CategoryType = 2 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"AutoPriInsurance = CASE WHEN RespTypeT.CategoryType = 3 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"AutoSecInsurance = CASE WHEN RespTypeT.CategoryType = 3 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"WorkersCompPriInsurance = CASE WHEN RespTypeT.CategoryType = 4 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"WorkersCompSecInsurance = CASE WHEN RespTypeT.CategoryType = 4 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"DentalPriInsurance = CASE WHEN RespTypeT.CategoryType = 5 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"DentalSecInsurance = CASE WHEN RespTypeT.CategoryType = 5 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"StudyPriInsurance = CASE WHEN RespTypeT.CategoryType = 6 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"StudySecInsurance = CASE WHEN RespTypeT.CategoryType = 6 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"LOPPriInsurance = CASE WHEN RespTypeT.CategoryType = 7 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"LOPSecInsurance = CASE WHEN RespTypeT.CategoryType = 7 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"LOAPriInsurance = CASE WHEN RespTypeT.CategoryType = 8 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"LOASecInsurance = CASE WHEN RespTypeT.CategoryType = 8 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END "
				"FROM PaymentsT "
				"LEFT OUTER JOIN InsuredPartyT ON PaymentsT.InsuredPartyID = InsuredPartyT.PersonID "
				"LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				"LEFT OUTER JOIN LineItemT LineItemT1 ON PaymentsT.ID = LineItemT1.ID RIGHT OUTER JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID "
				"WHERE (LineItemT1.Deleted = 0) AND (LineItemT1.Deleted = 0) AND (AppliesT.PointsToPayments = 1)) AS StatementDataAppliesPays) AS StmtApplies) AS StatementAllData) AS StatementPatBalanceSub "
				"GROUP BY StatementPatBalanceSub.PatFixID) SubQ "
				"ON PersonT.ID = SubQ.PersonID INNER JOIN PatientsT WITH(NOLOCK) ON PersonT.ID = PatientsT.PersonID "
				") FilterAllBalanceInfoSubQ ON PersonT.ID = FilterAllBalanceInfoSubQ.PersonID "),

	ADD_FILTER_FIELD(194, "Balance (All Pts) Patient Total", "FilterAllBalanceInfoSubQ.PatTotResp", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 193), 
	ADD_FILTER_FIELD(195, "Balance (All Pts) Insurance Total", "FilterAllBalanceInfoSubQ.InsTotResp", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 193), 
	ADD_FILTER_FIELD(196, "Balance (All Pts) Insurance Primary", "FilterAllBalanceInfoSubQ.InsPriResp", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 193), 
	ADD_FILTER_FIELD(197, "Balance (All Pts) Insurance Secondary", "FilterAllBalanceInfoSubQ.InsSecResp", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 193), 
	ADD_FILTER_FIELD(198, "Balance (All Pts) Insurance Other", "FilterAllBalanceInfoSubQ.InsOthResp", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 193), 

	// (j.jones 2011-04-04 09:14) - PLID 43109 - added Vision Primary & Secondary balance filters
	ADD_FILTER_FIELD(477, "Balance (All Pts) Insurance Vision Primary", "FilterAllBalanceInfoSubQ.VisionInsPriResp", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 193), 
	ADD_FILTER_FIELD(478, "Balance (All Pts) Insurance Vision Secondary", "FilterAllBalanceInfoSubQ.VisionInsSecResp", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 193),

	// (d.singleton 2012-09-11 12:22) - PLID 51808 add auto filter
	// (d.singleton 2012-09-14 13:34) - PLID 52657 add workers comp filter
	// (d.singleton 2012-09-14 13:34) - PLID 52658 add dental filter
	// (d.singleton 2012-09-14 13:35) - PLID 52659 add study filter
	// (d.singleton 2012-09-14 13:35) - PLID 52660 add LOP filter
	// (d.singleton 2012-09-14 13:35) - PLID 52661 add LOA filter
	ADD_FILTER_FIELD(586, "Balance (All Pts) Insurance Auto Primary", "FilterAllBalanceInfoSubQ.AutoInsPriResp", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 193), 
	ADD_FILTER_FIELD(587, "Balance (All Pts) Insurance Auto Secondary", "FilterAllBalanceInfoSubQ.AutoInsSecResp", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 193),
	ADD_FILTER_FIELD(588, "Balance (All Pts) Insurance Workers Comp Primary", "FilterAllBalanceInfoSubQ.WorkersCompInsPriResp", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 193), 
	ADD_FILTER_FIELD(589, "Balance (All Pts) Insurance Workers Comp Secondary", "FilterAllBalanceInfoSubQ.WorkersCompInsSecResp", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 193),
	ADD_FILTER_FIELD(590, "Balance (All Pts) Insurance Dental Primary", "FilterAllBalanceInfoSubQ.DentalInsPriResp", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 193), 
	ADD_FILTER_FIELD(591, "Balance (All Pts) Insurance Dental Secondary", "FilterAllBalanceInfoSubQ.DentalInsSecResp", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 193),
	ADD_FILTER_FIELD(592, "Balance (All Pts) Insurance Study Primary", "FilterAllBalanceInfoSubQ.StudyInsPriResp", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 193), 
	ADD_FILTER_FIELD(593, "Balance (All Pts) Insurance Study Secondary", "FilterAllBalanceInfoSubQ.StudyInsSecResp", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 193),
	ADD_FILTER_FIELD(594, "Balance (All Pts) Insurance LOP Primary", "FilterAllBalanceInfoSubQ.LOPInsPriResp", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 193), 
	ADD_FILTER_FIELD(595, "Balance (All Pts) Insurance LOP Secondary", "FilterAllBalanceInfoSubQ.LOPInsSecResp", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 193),
	ADD_FILTER_FIELD(596, "Balance (All Pts) Insurance LOA Primary", "FilterAllBalanceInfoSubQ.LOAInsPriResp", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 193), 
	ADD_FILTER_FIELD(597, "Balance (All Pts) Insurance LOA Secondary", "FilterAllBalanceInfoSubQ.LOAInsSecResp", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 193),

	// (j.jones 2008-05-02 13:10) - PLID 28297 - fixed the last UNION such that when looking at applies to payments,
	// we confirm the destination of the apply is actually a payment, not an adjustment or refund

	//these regular net payments fields contain only patients who have financial activity ... if a patient has never had a charge/pay/etc, he will not show up in this list as 0 balance
	ADD_FILTER_FIELD(310, "Net Payments (Financial Patients)", "FilterFinNetPayInfoSubQ.AllTotPays", fboPerson, foEquality, foGreater, ftCurrency, NULL, 
				"(SELECT StatementPatBalanceSub.PatFixID AS PersonID, SUM(StatementPatBalanceSub.PatPay + StatementPatBalanceSub.TotInsPays) AS AllTotPays, SUM(StatementPatBalanceSub.PatPay) AS PatTotPays, SUM(StatementPatBalanceSub.TotInsPays) AS InsTotPays, "
				"SUM(StatementPatBalanceSub.PriInsPays) AS InsPriPays, "
				"SUM(StatementPatBalanceSub.SecInsPays) AS InsSecPays, "
				"SUM(StatementPatBalanceSub.OthInsPays) AS InsOthPays, "
				"SUM(StatementPatBalanceSub.VisionPriInsPays) AS VisionInsPriPays, "
				"SUM(StatementPatBalanceSub.VisionSecInsPays) AS VisionInsSecPays, "
				"SUM(StatementPatBalanceSub.AutoPriInsPays) AS AutoInsPriPays, "
				"SUM(StatementPatBalanceSub.AutoSecInsPays) AS AutoInsSecPays, "
				"SUM(StatementPatBalanceSub.WorkersCompPriInsPays) AS WorkersCompInsPriPays, "
				"SUM(StatementPatBalanceSub.WorkersCompSecInsPays) AS WorkersCompInsSecPays, "
				"SUM(StatementPatBalanceSub.DentalPriInsPays) AS DentalInsPriPays, "
				"SUM(StatementPatBalanceSub.DentalSecInsPays) AS DentalInsSecPays, "
				"SUM(StatementPatBalanceSub.StudyPriInsPays) AS StudyInsPriPays, "
				"SUM(StatementPatBalanceSub.StudySecInsPays) AS StudyInsSecPays, "
				"SUM(StatementPatBalanceSub.LOPPriInsPays) AS LOPInsPriPays, "
				"SUM(StatementPatBalanceSub.LOPSecInsPays) AS LOPInsSecPays, "
				"SUM(StatementPatBalanceSub.LOAPriInsPays) AS LOAInsPriPays, "
				"SUM(StatementPatBalanceSub.LOASecInsPays) AS LOAInsSecPays "
				"FROM (SELECT StatementAllData.PatFixID,  PatCharge = CASE WHEN StatementAllData.Type = 10 THEN (StatementAllData.Total - StatementAllData.TotalInsurance) ELSE 0 END, PatPay = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN CASE WHEN StatementAllData.TotalInsurance <> 0 THEN 0 ELSE StatementAllData.Total END ELSE CASE WHEN StatementAllData.Type = 3 THEN CASE WHEN StatementAllDAta.TotalInsurance <> 0 THEN 0 ELSE StatementAllData.Total * - 1 END ELSE 0 END END, TotInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.TotalInsurance ELSE 0 END, TotInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.TotalInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.TotalInsurance * - 1 ELSE 0 END END, "
				"PriInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.PriInsurance ELSE 0 END, "
				"PriInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.PriInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.PriInsurance * - 1 ELSE 0 END END, "
				"SecInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.SecInsurance ELSE 0 END, "
				"SecInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.SecInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.SecInsurance * - 1 ELSE 0 END END, "
				"OthInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.OthInsurance ELSE 0 END, "
				"OthInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.OthInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.OthInsurance * - 1 ELSE 0 END END, "
				"VisionPriInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.VisionPriInsurance ELSE 0 END, "
				"VisionPriInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.VisionPriInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.VisionPriInsurance * - 1 ELSE 0 END END, "
				"VisionSecInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.VisionSecInsurance ELSE 0 END, "
				"VisionSecInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.VisionSecInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.VisionSecInsurance * - 1 ELSE 0 END END, "
				"AutoPriInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.AutoPriInsurance ELSE 0 END, "
				"AutoPriInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.AutoPriInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.AutoPriInsurance * - 1 ELSE 0 END END, "
				"AutoSecInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.AutoSecInsurance ELSE 0 END, "
				"AutoSecInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.AutoSecInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.AutoSecInsurance * - 1 ELSE 0 END END, "
				"WorkersCompPriInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.WorkersCompPriInsurance ELSE 0 END, "
				"WorkersCompPriInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.WorkersCompPriInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.WorkersCompPriInsurance * - 1 ELSE 0 END END, "
				"WorkersCompSecInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.WorkersCompSecInsurance ELSE 0 END, "
				"WorkersCompSecInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.WorkersCompSecInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.WorkersCompSecInsurance * - 1 ELSE 0 END END, "
				"DentalPriInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.DentalPriInsurance ELSE 0 END, "
				"DentalPriInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.DentalPriInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.DentalPriInsurance * - 1 ELSE 0 END END, "
				"DentalSecInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.DentalSecInsurance ELSE 0 END, "
				"DentalSecInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.DentalSecInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.DentalSecInsurance * - 1 ELSE 0 END END, "
				"StudyPriInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.StudyPriInsurance ELSE 0 END, "
				"StudyPriInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.StudyPriInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.StudyPriInsurance * - 1 ELSE 0 END END, "
				"StudySecInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.StudySecInsurance ELSE 0 END, "
				"StudySecInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.StudySecInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.StudySecInsurance * - 1 ELSE 0 END END, "
				"LOPPriInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.LOPPriInsurance ELSE 0 END, "
				"LOPPriInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.LOPPriInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.LOPPriInsurance * - 1 ELSE 0 END END, "
				"LOPSecInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.LOPSecInsurance ELSE 0 END, "
				"LOPSecInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.LOPSecInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.LOPSecInsurance * - 1 ELSE 0 END END, "
				"LOAPriInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.LOAPriInsurance ELSE 0 END, "
				"LOAPriInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.LOAPriInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.LOAPriInsurance * - 1 ELSE 0 END END, "
				"LOASecInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.LOASecInsurance ELSE 0 END, "
				"LOASecInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.LOASecInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.LOASecInsurance * - 1 ELSE 0 END END "
				"FROM (SELECT StmtCharges.ID, StmtCharges.PatFixID, StmtCharges.Type, StmtCharges.Total,  StmtCharges.TotalInsurance, "
				"StmtCharges.PriInsurance, StmtCharges.SecInsurance, StmtCharges.OthInsurance, "
				"StmtCharges.VisionPriInsurance, StmtCharges.VisionSecInsurance, StmtCharges.AutoPriInsurance, StmtCharges.AutoSecInsurance, StmtCharges.WorkersCompPriInsurance, StmtCharges.WorkersCompSecInsurance, StmtCharges.DentalPriInsurance, StmtCharges.DentalSecInsurance, StmtCharges.StudyPriInsurance, StmtCharges.StudySecInsurance, "
				"StmtCharges.LOPPriInsurance, StmtCharges.LOPSecInsurance, StmtCharges.LOAPriInsurance, StmtCharges.LOASecInsurance "
				"FROM (SELECT LineItemT.ID,  LineItemT.PatientID AS PatFixID,  10 AS Type, TotalInsurance = SUM(CASE WHEN ChargeRespT.InsuredPartyID IS NOT NULL  THEN ChargeRespT.Amount ELSE 0 END), Total = CASE WHEN SUM(ChargeRespT.Amount) IS NULL  THEN 0 ELSE SUM(ChargeRespT.Amount) END, "
				"PriInsurance = CASE WHEN InsuredPartyT.RespTypeID = 1 THEN SUM(ChargeRespT.Amount) ELSE 0 END, "
				"SecInsurance = CASE WHEN InsuredPartyT.RespTypeID = 2 THEN SUM(ChargeRespT.Amount) ELSE 0 END, "
				"OthInsurance = CASE WHEN InsuredPartyT.RespTypeID <> 1 AND InsuredPartyT.RespTypeID <> 2 THEN SUM(ChargeRespT.Amount) ELSE 0 END, "
				"VisionPriInsurance = CASE WHEN RespTypeT.CategoryType = 2 AND RespTypeT.CategoryPlacement = 1 THEN SUM(ChargeRespT.Amount) ELSE 0 END, "
				"VisionSecInsurance = CASE WHEN RespTypeT.CategoryType = 2 AND RespTypeT.CategoryPlacement = 2 THEN SUM(ChargeRespT.Amount) ELSE 0 END, "
				"AutoPriInsurance = CASE WHEN RespTypeT.CategoryType = 3 AND RespTypeT.CategoryPlacement = 1 THEN SUM(ChargeRespT.Amount) ELSE 0 END, "
				"AutoSecInsurance = CASE WHEN RespTypeT.CategoryType = 3 AND RespTypeT.CategoryPlacement = 2 THEN SUM(ChargeRespT.Amount) ELSE 0 END, "
				"WorkersCompPriInsurance = CASE WHEN RespTypeT.CategoryType = 4 AND RespTypeT.CategoryPlacement = 1 THEN SUM(ChargeRespT.Amount) ELSE 0 END, "
				"WorkersCompSecInsurance = CASE WHEN RespTypeT.CategoryType = 4 AND RespTypeT.CategoryPlacement = 2 THEN SUM(ChargeRespT.Amount) ELSE 0 END, "
				"DentalPriInsurance = CASE WHEN RespTypeT.CategoryType = 5 AND RespTypeT.CategoryPlacement = 1 THEN SUM(ChargeRespT.Amount) ELSE 0 END, "
				"DentalSecInsurance = CASE WHEN RespTypeT.CategoryType = 5 AND RespTypeT.CategoryPlacement = 2 THEN SUM(ChargeRespT.Amount) ELSE 0 END, "
				"StudyPriInsurance = CASE WHEN RespTypeT.CategoryType = 6 AND RespTypeT.CategoryPlacement = 1 THEN SUM(ChargeRespT.Amount) ELSE 0 END, "
				"StudySecInsurance = CASE WHEN RespTypeT.CategoryType = 6 AND RespTypeT.CategoryPlacement = 2 THEN SUM(ChargeRespT.Amount) ELSE 0 END, "
				"LOPPriInsurance = CASE WHEN RespTypeT.CategoryType = 7 AND RespTypeT.CategoryPlacement = 1 THEN SUM(ChargeRespT.Amount) ELSE 0 END, "
				"LOPSecInsurance = CASE WHEN RespTypeT.CategoryType = 7 AND RespTypeT.CategoryPlacement = 2 THEN SUM(ChargeRespT.Amount) ELSE 0 END, "
				"LOAPriInsurance = CASE WHEN RespTypeT.CategoryType = 8 AND RespTypeT.CategoryPlacement = 1 THEN SUM(ChargeRespT.Amount) ELSE 0 END, "
				"LOASecInsurance = CASE WHEN RespTypeT.CategoryType = 8 AND RespTypeT.CategoryPlacement = 2 THEN SUM(ChargeRespT.Amount) ELSE 0 END "
				"FROM InsuredPartyT "
				"LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				"RIGHT OUTER JOIN ChargeRespT ON  InsuredPartyT.PersonID = ChargeRespT.InsuredPartyID RIGHT OUTER JOIN LineItemT ON  ChargeRespT.ChargeID = LineItemT.ID "
				"WHERE (LineItemT.Deleted = 0) AND  (LineItemT.Type = 10) "
				"GROUP BY InsuredPartyT.RespTypeID, RespTypeT.CategoryType, RespTypeT.CategoryPlacement, LineItemT.ID, LineItemT.PatientID,  LineItemT.Type) AS StmtCharges "
				"UNION "
				"SELECT StmtPays.ID, StmtPays.PatFixID,  StmtPays.Type, StmtPays.UnAppliedAmount,  StmtPays.TotalInsurance,  "
				"StmtPays.PriInsurance, StmtPays.SecInsurance,  StmtPays.OthInsurance, "
				"StmtPays.VisionPriInsurance, StmtPays.VisionSecInsurance, StmtPays.AutoPriInsurance, StmtPays.AutoSecInsurance, StmtPays.WorkersCompPriInsurance, StmtPays.WorkersCompSecInsurance, StmtPays.DentalPriInsurance, StmtPays.DentalSecInsurance, StmtPays.StudyPriInsurance, StmtPays.StudySecInsurance, StmtPays.LOPPriInsurance, StmtPays.LOPSecInsurance, StmtPays.LOAPriInsurance, StmtPays.LOASecInsurance "
				"FROM (SELECT LineItemT.ID, LineItemT.Type,  LineItemT.PatientID AS PatFixID, UnAppliedAmount = CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL  THEN 0 ELSE [AppliesT].[Amount] END)  ELSE MIN([LineItemT].[Amount])  - SUM(CASE WHEN [AppliesT].[Amount] IS NULL  THEN 0 ELSE [AppliesT].[Amount] END) END, TotalInsurance = CASE WHEN MIN([PaymentsT].[InsuredPartyID]) > 0 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount])  + SUM(CASE WHEN [AppliesT].[Amount] IS NULL  THEN 0 ELSE [AppliesT].[Amount] END)  ELSE MIN([LineItemT].[Amount])  - SUM(CASE WHEN [AppliesT].[Amount] IS NULL  THEN 0 ELSE [AppliesT].[Amount] END)  END ELSE 0 END, "
				"PriInsurance = CASE WHEN [InsuredPartyT].[RespTypeID] = 1 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END ELSE 0 END, "
				"SecInsurance = CASE WHEN [InsuredPartyT].[RespTypeID] = 2 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END  ELSE 0 END, "
				"OthInsurance = CASE WHEN [InsuredPartyT].[RespTypeID] <> 1 AND InsuredPartyT.RespTypeID <> 2 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END ELSE 0 END, "
				"VisionPriInsurance = CASE WHEN RespTypeT.CategoryType = 2 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END ELSE 0 END, "
				"VisionSecInsurance = CASE WHEN RespTypeT.CategoryType = 2 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END  ELSE 0 END, "
				"AutoPriInsurance = CASE WHEN RespTypeT.CategoryType = 3 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END ELSE 0 END, "
				"AutoSecInsurance = CASE WHEN RespTypeT.CategoryType = 3 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END  ELSE 0 END, "
				"WorkersCompPriInsurance = CASE WHEN RespTypeT.CategoryType = 4 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END ELSE 0 END, "
				"WorkersCompSecInsurance = CASE WHEN RespTypeT.CategoryType = 4 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END  ELSE 0 END, "
				"DentalPriInsurance = CASE WHEN RespTypeT.CategoryType = 5 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END ELSE 0 END, "
				"DentalSecInsurance = CASE WHEN RespTypeT.CategoryType = 5 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END  ELSE 0 END, "
				"StudyPriInsurance = CASE WHEN RespTypeT.CategoryType = 6 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END ELSE 0 END, "
				"StudySecInsurance = CASE WHEN RespTypeT.CategoryType = 6 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END  ELSE 0 END, "
				"LOPPriInsurance = CASE WHEN RespTypeT.CategoryType = 7 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END ELSE 0 END, "
				"LOPSecInsurance = CASE WHEN RespTypeT.CategoryType = 7 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END  ELSE 0 END, "
				"LOAPriInsurance = CASE WHEN RespTypeT.CategoryType = 8 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END ELSE 0 END, "
				"LOASecInsurance = CASE WHEN RespTypeT.CategoryType = 8 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END  ELSE 0 END "
				"FROM InsuredPartyT "
				"LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				"RIGHT OUTER JOIN PaymentsT ON InsuredPartyT.PersonID = PaymentsT.InsuredPartyID LEFT OUTER JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID RIGHT OUTER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
				"WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type < 10) "
				"GROUP BY LineItemT.ID, LineItemT.Type, LineItemT.PatientID, [InsuredPartyT].[RespTypeID], RespTypeT.CategoryType, RespTypeT.CategoryPlacement "
				"HAVING (CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END <> 0) AND LineItemT.Type = 1) AS StmtPays "
				"UNION ALL "
				"SELECT StmtApplies.ChargeID, StmtApplies.PatFixID, StmtApplies.Type, StmtApplies.ApplyAmount, StmtApplies.TotalInsurance, "
				"StmtApplies.PriInsurance, StmtApplies.SecInsurance, StmtApplies.OthInsurance, "
				"StmtApplies.VisionPriInsurance, StmtApplies.VisionSecInsurance, StmtApplies.AutoPriInsurance, StmtApplies.AutoSecInsurance, StmtApplies.WorkersCompPriInsurance, StmtApplies.WorkersCompSecInsurance, StmtApplies.DentalPriInsurance, StmtApplies.DentalSecInsurance, StmtApplies.StudyPriInsurance, StmtApplies.StudySecInsurance, StmtApplies.LOPPriInsurance, StmtApplies.LOPSecInsurance, StmtApplies.LOAPriInsurance, StmtApplies.LOASecInsurance "
				"FROM (SELECT * FROM (SELECT AppliesT.SourceID AS ChargeID, LineItemT1.PatientID AS PatFixID, LineItemT1.Type, ApplyAmount = CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END, TotalInsurance = CASE WHEN [PaymentsT].[InsuredPartyID] > 0 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"PriInsurance = CASE WHEN InsuredPartyT.RespTypeID = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"SecInsurance = CASE WHEN InsuredPartyT.RespTypeID = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"OthInsurance = CASE WHEN InsuredPartyT.RespTypeID <> 1 AND InsuredPartyT.RespTypeID <> 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"VisionPriInsurance = CASE WHEN RespTypeT.CategoryType = 2 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"VisionSecInsurance = CASE WHEN RespTypeT.CategoryType = 2 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"AutoPriInsurance = CASE WHEN RespTypeT.CategoryType = 3 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"AutoSecInsurance = CASE WHEN RespTypeT.CategoryType = 3 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"WorkersCompPriInsurance = CASE WHEN RespTypeT.CategoryType = 4 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"WorkersCompSecInsurance = CASE WHEN RespTypeT.CategoryType = 4 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"DentalPriInsurance = CASE WHEN RespTypeT.CategoryType = 5 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"DentalSecInsurance = CASE WHEN RespTypeT.CategoryType = 5 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"StudyPriInsurance = CASE WHEN RespTypeT.CategoryType = 6 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"StudySecInsurance = CASE WHEN RespTypeT.CategoryType = 6 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"LOPPriInsurance = CASE WHEN RespTypeT.CategoryType = 7 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"LOPSecInsurance = CASE WHEN RespTypeT.CategoryType = 7 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"LOAPriInsurance = CASE WHEN RespTypeT.CategoryType = 8 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"LOASecInsurance = CASE WHEN RespTypeT.CategoryType = 8 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END "
				"FROM InsuredPartyT "
				"LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				"RIGHT OUTER JOIN PaymentsT ON InsuredPartyT.PersonID = PaymentsT.InsuredPartyID LEFT OUTER JOIN LineItemT LineItemT1 ON PaymentsT.ID = LineItemT1.ID RIGHT OUTER JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID "
				"WHERE (LineItemT1.Deleted = 0) AND (LineItemT1.Deleted = 0) AND (AppliesT.PointsToPayments = 0)) AS StatementDataAppliesCharges "
				"UNION ALL "
				"SELECT * FROM (SELECT AppliesT.DestID AS ChargeID, LineItemT1.PatientID AS PatFixID, LineItemT1.Type, ApplyAmount = CASE WHEN LineItemT1.Type = 3 THEN -1 * AppliesT.Amount ELSE AppliesT.Amount END, TotalInsurance = CASE WHEN [PaymentsT].[InsuredPartyID] > 0 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"PriInsurance = CASE WHEN InsuredPartyT.RespTypeID = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"SecInsurance = CASE WHEN InsuredPartyT.RespTypeID = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"OthInsurance = CASE WHEN InsuredPartyT.RespTypeID <> 1 AND InsuredPartyT.RespTypeID <> 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"VisionPriInsurance = CASE WHEN RespTypeT.CategoryType = 2 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"VisionSecInsurance = CASE WHEN RespTypeT.CategoryType = 2 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"AutoPriInsurance = CASE WHEN RespTypeT.CategoryType = 3 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"AutoSecInsurance = CASE WHEN RespTypeT.CategoryType = 3 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"WorkersCompPriInsurance = CASE WHEN RespTypeT.CategoryType = 4 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"WorkersCompSecInsurance = CASE WHEN RespTypeT.CategoryType = 4 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"DentalPriInsurance = CASE WHEN RespTypeT.CategoryType = 5 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"DentalSecInsurance = CASE WHEN RespTypeT.CategoryType = 5 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"StudyPriInsurance = CASE WHEN RespTypeT.CategoryType = 6 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"StudySecInsurance = CASE WHEN RespTypeT.CategoryType = 6 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"LOPPriInsurance = CASE WHEN RespTypeT.CategoryType = 7 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"LOPSecInsurance = CASE WHEN RespTypeT.CategoryType = 7 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"LOAPriInsurance = CASE WHEN RespTypeT.CategoryType = 8 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"LOASecInsurance = CASE WHEN RespTypeT.CategoryType = 8 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END "
				"FROM PaymentsT "
				"LEFT OUTER JOIN InsuredPartyT ON PaymentsT.InsuredPartyID = InsuredPartyT.PersonID "
				"LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				"LEFT OUTER JOIN LineItemT LineItemT1 ON PaymentsT.ID = LineItemT1.ID "
				"RIGHT OUTER JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID LEFT JOIN LineItemT LineItemT2 ON AppliesT.DestID = LineItemT2.ID "
				"WHERE (LineItemT1.Deleted = 0) AND (LineItemT1.Deleted = 0) AND (AppliesT.PointsToPayments = 1) AND LineItemT2.Type = 1) AS StatementDataAppliesPays) AS StmtApplies) AS StatementAllData) AS StatementPatBalanceSub "
				"GROUP BY StatementPatBalanceSub.PatFixID) FilterFinNetPayInfoSubQ ON PersonT.ID = FilterFinNetPayInfoSubQ.PersonID ", FILTER_DEPENDS_ON_BASE, "LineItemT", "LineItemT.Deleted = 0 AND LineItemT.PatientID = PersonT.ID AND LineItemT.Type <> 11"),
	ADD_FILTER_FIELD(311, "Net Payments (Fin Pts) Patient Total", "FilterFinNetPayInfoSubQ.PatTotPays", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 310, "LineItemT", "LineItemT.Deleted = 0 AND LineItemT.PatientID = PersonT.ID AND LineItemT.Type <> 11"), 
	ADD_FILTER_FIELD(312, "Net Payments (Fin Pts) Insurance Total", "FilterFinNetPayInfoSubQ.InsTotPays", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 310, "LineItemT", "LineItemT.Deleted = 0 AND LineItemT.PatientID = PersonT.ID AND LineItemT.Type <> 11"), 
	ADD_FILTER_FIELD(313, "Net Payments (Fin Pts) Insurance Primary", "FilterFinNetPayInfoSubQ.InsPriPays", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 310, "LineItemT", "LineItemT.Deleted = 0 AND LineItemT.PatientID = PersonT.ID AND LineItemT.Type <> 11"), 
	ADD_FILTER_FIELD(314, "Net Payments (Fin Pts) Insurance Secondary", "FilterFinNetPayInfoSubQ.InsSecPays", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 310, "LineItemT", "LineItemT.Deleted = 0 AND LineItemT.PatientID = PersonT.ID AND LineItemT.Type <> 11"), 
	ADD_FILTER_FIELD(315, "Net Payments (Fin Pts) Insurance Other", "FilterFinNetPayInfoSubQ.InsOthPays", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 310, "LineItemT", "LineItemT.Deleted = 0 AND LineItemT.PatientID = PersonT.ID AND LineItemT.Type <> 11"), 

	// (j.jones 2011-04-01 14:52) - PLID 43109 - added Vision Primary & Secondary payment filters
	ADD_FILTER_FIELD(479, "Net Payments (Fin Pts) Insurance Vision Primary", "FilterFinNetPayInfoSubQ.VisionInsPriPays", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 310, "LineItemT", "LineItemT.Deleted = 0 AND LineItemT.PatientID = PersonT.ID AND LineItemT.Type <> 11"), 
	ADD_FILTER_FIELD(480, "Net Payments (Fin Pts) Insurance Vision Secondary", "FilterFinNetPayInfoSubQ.VisionInsSecPays", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 310, "LineItemT", "LineItemT.Deleted = 0 AND LineItemT.PatientID = PersonT.ID AND LineItemT.Type <> 11"),

	// (d.singleton 2012-09-11 12:22) - PLID 51808 add auto filter
	// (d.singleton 2012-09-14 13:34) - PLID 52657 add workers comp filter
	// (d.singleton 2012-09-14 13:34) - PLID 52658 add dental filter
	// (d.singleton 2012-09-14 13:35) - PLID 52659 add study filter
	// (d.singleton 2012-09-14 13:35) - PLID 52660 add LOP filter
	// (d.singleton 2012-09-14 13:35) - PLID 52661 add LOA filter
	ADD_FILTER_FIELD(598, "Net Payments (Fin Pts) Insurance Auto Primary", "FilterFinNetPayInfoSubQ.AutoInsPriPays", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 310, "LineItemT", "LineItemT.Deleted = 0 AND LineItemT.PatientID = PersonT.ID AND LineItemT.Type <> 11"), 
	ADD_FILTER_FIELD(599, "Net Payments (Fin Pts) Insurance Auto Secondary", "FilterFinNetPayInfoSubQ.AutoInsSecPays", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 310, "LineItemT", "LineItemT.Deleted = 0 AND LineItemT.PatientID = PersonT.ID AND LineItemT.Type <> 11"),
	ADD_FILTER_FIELD(600, "Net Payments (Fin Pts) Insurance Workers Comp Primary", "FilterFinNetPayInfoSubQ.WorkersCompInsPriPays", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 310, "LineItemT", "LineItemT.Deleted = 0 AND LineItemT.PatientID = PersonT.ID AND LineItemT.Type <> 11"), 
	ADD_FILTER_FIELD(601, "Net Payments (Fin Pts) Insurance Workers Comp Secondary", "FilterFinNetPayInfoSubQ.WorkersCompInsSecPays", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 310, "LineItemT", "LineItemT.Deleted = 0 AND LineItemT.PatientID = PersonT.ID AND LineItemT.Type <> 11"),
	ADD_FILTER_FIELD(602, "Net Payments (Fin Pts) Insurance Dental Primary", "FilterFinNetPayInfoSubQ.DentalInsPriPays", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 310, "LineItemT", "LineItemT.Deleted = 0 AND LineItemT.PatientID = PersonT.ID AND LineItemT.Type <> 11"), 
	ADD_FILTER_FIELD(603, "Net Payments (Fin Pts) Insurance Dental Secondary", "FilterFinNetPayInfoSubQ.DentalInsSecPays", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 310, "LineItemT", "LineItemT.Deleted = 0 AND LineItemT.PatientID = PersonT.ID AND LineItemT.Type <> 11"),
	ADD_FILTER_FIELD(604, "Net Payments (Fin Pts) Insurance Study Primary", "FilterFinNetPayInfoSubQ.StudyInsPriPays", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 310, "LineItemT", "LineItemT.Deleted = 0 AND LineItemT.PatientID = PersonT.ID AND LineItemT.Type <> 11"), 
	ADD_FILTER_FIELD(605, "Net Payments (Fin Pts) Insurance Study Secondary", "FilterFinNetPayInfoSubQ.StudyInsSecPays", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 310, "LineItemT", "LineItemT.Deleted = 0 AND LineItemT.PatientID = PersonT.ID AND LineItemT.Type <> 11"),
	ADD_FILTER_FIELD(606, "Net Payments (Fin Pts) Insurance LOP Primary", "FilterFinNetPayInfoSubQ.LOPInsPriPays", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 310, "LineItemT", "LineItemT.Deleted = 0 AND LineItemT.PatientID = PersonT.ID AND LineItemT.Type <> 11"), 
	ADD_FILTER_FIELD(607, "Net Payments (Fin Pts) Insurance LOP Secondary", "FilterFinNetPayInfoSubQ.LOPInsSecPays", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 310, "LineItemT", "LineItemT.Deleted = 0 AND LineItemT.PatientID = PersonT.ID AND LineItemT.Type <> 11"),
	ADD_FILTER_FIELD(608, "Net Payments (Fin Pts) Insurance LOA Primary", "FilterFinNetPayInfoSubQ.LOAInsPriPays", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 310, "LineItemT", "LineItemT.Deleted = 0 AND LineItemT.PatientID = PersonT.ID AND LineItemT.Type <> 11"), 
	ADD_FILTER_FIELD(609, "Net Payments (Fin Pts) Insurance LOA Secondary", "FilterFinNetPayInfoSubQ.LOAInsSecPays", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 310, "LineItemT", "LineItemT.Deleted = 0 AND LineItemT.PatientID = PersonT.ID AND LineItemT.Type <> 11"),

	// (j.jones 2008-05-02 13:10) - PLID 28297 - fixed the last UNION such that when looking at applies to payments,
	// we confirm the destination of the apply is actually a payment, not an adjustment or refund

	//the _All_ fields contain all patients who have had financial activity, and all patients who have had none - this is really only relevant if you're filtering on = 0 balances
	ADD_FILTER_FIELD(316, "Net Payments (All Patients)", "FilterAllNetPayInfoSubQ.AllTotPays", fboPerson, foEquality, foGreater, ftCurrency, NULL, 
				"(SELECT ID AS PersonID, CASE WHEN AllTotPays IS NULL THEN 0 ELSE AllTotPays END AS AllTotPays, CASE WHEN SubQ.PatTotPays IS NULL THEN 0 ELSE SubQ.PatTotPays END AS PatTotPays, CASE WHEN SubQ.InsTotPays IS NULL THEN 0 ELSE SubQ.InsTotPays END AS InsTotPays, "
				"CASE WHEN SubQ.InsPriPays IS NULL THEN 0 ELSE SubQ.InsPriPays END AS InsPriPays, "
				"CASE WHEN SubQ.InsSecPays IS NULL THEN 0 ELSE SubQ.InsSecPays END AS InsSecPays, "
				"CASE WHEN SubQ.InsOthPays IS NULL THEN 0 ELSE SubQ.InsOthPays END AS InsOthPays, "
				"CASE WHEN SubQ.VisionInsPriPays IS NULL THEN 0 ELSE SubQ.VisionInsPriPays END AS VisionInsPriPays, "
				"CASE WHEN SubQ.VisionInsSecPays IS NULL THEN 0 ELSE SubQ.VisionInsSecPays END AS VisionInsSecPays, "
				"CASE WHEN SubQ.AutoInsPriPays IS NULL THEN 0 ELSE SubQ.AutoInsPriPays END AS AutoInsPriPays, "
				"CASE WHEN SubQ.AutoInsSecPays IS NULL THEN 0 ELSE SubQ.AutoInsSecPays END AS AutoInsSecPays, "
				"CASE WHEN SubQ.WorkersCompInsPriPays IS NULL THEN 0 ELSE SubQ.WorkersCompInsPriPays END AS WorkersCompInsPriPays, "
				"CASE WHEN SubQ.WorkersCompInsSecPays IS NULL THEN 0 ELSE SubQ.WorkersCompInsSecPays END AS WorkersCompInsSecPays, "
				"CASE WHEN SubQ.DentalInsPriPays IS NULL THEN 0 ELSE SubQ.DentalInsPriPays END AS DentalInsPriPays, "
				"CASE WHEN SubQ.DentalInsSecPays IS NULL THEN 0 ELSE SubQ.DentalInsSecPays END AS DentalInsSecPays, "
				"CASE WHEN SubQ.StudyInsPriPays IS NULL THEN 0 ELSE SubQ.StudyInsPriPays END AS StudyInsPriPays, "
				"CASE WHEN SubQ.StudyInsSecPays IS NULL THEN 0 ELSE SubQ.StudyInsSecPays END AS StudyInsSecPays, "
				"CASE WHEN SubQ.LOPInsPriPays IS NULL THEN 0 ELSE SubQ.LOPInsPriPays END AS LOPInsPriPays, "
				"CASE WHEN SubQ.LOPInsSecPays IS NULL THEN 0 ELSE SubQ.LOPInsSecPays END AS LOPInsSecPays, "
				"CASE WHEN SubQ.LOAInsPriPays IS NULL THEN 0 ELSE SubQ.LOAInsPriPays END AS LOAInsPriPays, "
				"CASE WHEN SubQ.LOAInsSecPays IS NULL THEN 0 ELSE SubQ.LOAInsSecPays END AS LOAInsSecPays "
				"FROM PersonT WITH(NOLOCK) LEFT JOIN "
				"(SELECT StatementPatBalanceSub.PatFixID AS PersonID, SUM(StatementPatBalanceSub.PatPay + StatementPatBalanceSub.TotInsPays) AS AllTotPays, SUM(StatementPatBalanceSub.PatPay) AS PatTotPays, SUM(StatementPatBalanceSub.TotInsPays) AS InsTotPays, "
				"SUM(StatementPatBalanceSub.PriInsPays) AS InsPriPays, "
				"SUM(StatementPatBalanceSub.SecInsPays) AS InsSecPays, "
				"SUM(StatementPatBalanceSub.OthInsPays) AS InsOthPays, "
				"SUM(StatementPatBalanceSub.VisionPriInsPays) AS VisionInsPriPays, "
				"SUM(StatementPatBalanceSub.VisionSecInsPays) AS VisionInsSecPays, "
				"SUM(StatementPatBalanceSub.AutoPriInsPays) AS AutoInsPriPays, "
				"SUM(StatementPatBalanceSub.AutoSecInsPays) AS AutoInsSecPays, "
				"SUM(StatementPatBalanceSub.WorkersCompPriInsPays) AS WorkersCompInsPriPays, "
				"SUM(StatementPatBalanceSub.WorkersCompSecInsPays) AS WorkersCompInsSecPays, "
				"SUM(StatementPatBalanceSub.DentalPriInsPays) AS DentalInsPriPays, "
				"SUM(StatementPatBalanceSub.DentalSecInsPays) AS DentalInsSecPays, "
				"SUM(StatementPatBalanceSub.StudyPriInsPays) AS StudyInsPriPays, "
				"SUM(StatementPatBalanceSub.StudySecInsPays) AS StudyInsSecPays, "
				"SUM(StatementPatBalanceSub.LOPPriInsPays) AS LOPInsPriPays, "
				"SUM(StatementPatBalanceSub.LOPSecInsPays) AS LOPInsSecPays, "
				"SUM(StatementPatBalanceSub.LOAPriInsPays) AS LOAInsPriPays, "
				"SUM(StatementPatBalanceSub.LOASecInsPays) AS LOAInsSecPays "
				"FROM (SELECT StatementAllData.PatFixID,  PatCharge = CASE WHEN StatementAllData.Type = 10 THEN (StatementAllData.Total - StatementAllData.TotalInsurance) ELSE 0 END, PatPay = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN CASE WHEN StatementAllData.TotalInsurance <> 0 THEN 0 ELSE StatementAllData.Total END ELSE CASE WHEN StatementAllData.Type = 3 THEN CASE WHEN StatementAllDAta.TotalInsurance <> 0 THEN 0 ELSE StatementAllData.Total * - 1 END ELSE 0 END END, TotInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.TotalInsurance ELSE 0 END, TotInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.TotalInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.TotalInsurance * - 1 ELSE 0 END END, "
				"PriInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.PriInsurance ELSE 0 END, "
				"PriInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.PriInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.PriInsurance * - 1 ELSE 0 END END, "
				"SecInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.SecInsurance ELSE 0 END, "
				"SecInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.SecInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.SecInsurance * - 1 ELSE 0 END END, "
				"OthInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.OthInsurance ELSE 0 END, "
				"OthInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.OthInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.OthInsurance * - 1 ELSE 0 END END, "
				"VisionPriInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.VisionPriInsurance ELSE 0 END, "
				"VisionPriInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.VisionPriInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.VisionPriInsurance * - 1 ELSE 0 END END, "
				"VisionSecInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.VisionSecInsurance ELSE 0 END, "
				"VisionSecInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.VisionSecInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.VisionSecInsurance * - 1 ELSE 0 END END, "
				"AutoPriInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.AutoPriInsurance ELSE 0 END, "
				"AutoPriInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.AutoPriInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.AutoPriInsurance * - 1 ELSE 0 END END, "
				"AutoSecInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.AutoSecInsurance ELSE 0 END, "
				"AutoSecInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.AutoSecInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.AutoSecInsurance * - 1 ELSE 0 END END, "
				"WorkersCompPriInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.WorkersCompPriInsurance ELSE 0 END, "
				"WorkersCompPriInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.WorkersCompPriInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.WorkersCompPriInsurance * - 1 ELSE 0 END END, "
				"WorkersCompSecInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.WorkersCompSecInsurance ELSE 0 END, "
				"WorkersCompSecInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.WorkersCompSecInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.WorkersCompSecInsurance * - 1 ELSE 0 END END, "
				"DentalPriInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.DentalPriInsurance ELSE 0 END, "
				"DentalPriInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.DentalPriInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.DentalPriInsurance * - 1 ELSE 0 END END, "
				"DentalSecInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.DentalSecInsurance ELSE 0 END, "
				"DentalSecInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.DentalSecInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.DentalSecInsurance * - 1 ELSE 0 END END, "
				"StudyPriInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.StudyPriInsurance ELSE 0 END, "
				"StudyPriInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.StudyPriInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.StudyPriInsurance * - 1 ELSE 0 END END, "
				"StudySecInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.StudySecInsurance ELSE 0 END, "
				"StudySecInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.StudySecInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.StudySecInsurance * - 1 ELSE 0 END END, "
				"LOPPriInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.LOPPriInsurance ELSE 0 END, "
				"LOPPriInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.LOPPriInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.LOPPriInsurance * - 1 ELSE 0 END END, "
				"LOPSecInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.LOPSecInsurance ELSE 0 END, "
				"LOPSecInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.LOPSecInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.LOPSecInsurance * - 1 ELSE 0 END END, "
				"LOAPriInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.LOAPriInsurance ELSE 0 END, "
				"LOAPriInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.LOAPriInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.LOAPriInsurance * - 1 ELSE 0 END END, "
				"LOASecInsCharge = CASE WHEN StatementAllData.Type = 10 THEN StatementAllData.LOASecInsurance ELSE 0 END, "
				"LOASecInsPays = CASE WHEN StatementAllData.Type = 1 OR StatementAllData.Type = 2 THEN StatementAllData.LOASecInsurance ELSE CASE WHEN StatementAllData.Type = 3 THEN StatementAllData.LOASecInsurance * - 1 ELSE 0 END END "
				"FROM (SELECT StmtCharges.ID, StmtCharges.PatFixID,  StmtCharges.Type, StmtCharges.Total,  StmtCharges.TotalInsurance,  "
				"StmtCharges.PriInsurance,  StmtCharges.SecInsurance,  StmtCharges.OthInsurance, "
				"StmtCharges.VisionPriInsurance,  StmtCharges.VisionSecInsurance, StmtCharges.AutoPriInsurance,  StmtCharges.AutoSecInsurance, StmtCharges.WorkersCompPriInsurance,  StmtCharges.WorkersCompSecInsurance, StmtCharges.DentalPriInsurance,  StmtCharges.DentalSecInsurance, StmtCharges.StudyPriInsurance,  StmtCharges.StudySecInsurance, StmtCharges.LOPPriInsurance,  StmtCharges.LOPSecInsurance, StmtCharges.LOAPriInsurance,  StmtCharges.LOASecInsurance "
				"FROM (SELECT LineItemT.ID,  LineItemT.PatientID AS PatFixID,  10 AS Type, TotalInsurance = SUM(CASE WHEN ChargeRespT.InsuredPartyID IS NOT NULL  THEN ChargeRespT.Amount ELSE 0 END), Total = CASE WHEN SUM(ChargeRespT.Amount) IS NULL  THEN 0 ELSE SUM(ChargeRespT.Amount) END,  "
				"PriInsurance = CASE WHEN InsuredPartyT.RespTypeID = 1 THEN SUM(ChargeRespT.Amount) ELSE 0 END, "
				"SecInsurance = CASE WHEN InsuredPartyT.RespTypeID = 2 THEN SUM(ChargeRespT.Amount) ELSE 0 END, "
				"OthInsurance = CASE WHEN InsuredPartyT.RespTypeID <> 1 AND InsuredPartyT.RespTypeID <> 2 THEN SUM(ChargeRespT.Amount) ELSE 0 END, "
				"VisionPriInsurance = CASE WHEN RespTypeT.CategoryType = 2 AND RespTypeT.CategoryPlacement = 1 THEN SUM(ChargeRespT.Amount) ELSE 0 END, "
				"VisionSecInsurance = CASE WHEN RespTypeT.CategoryType = 2 AND RespTypeT.CategoryPlacement = 2 THEN SUM(ChargeRespT.Amount) ELSE 0 END, "
				"AutoPriInsurance = CASE WHEN RespTypeT.CategoryType = 3 AND RespTypeT.CategoryPlacement = 1 THEN SUM(ChargeRespT.Amount) ELSE 0 END, "
				"AutoSecInsurance = CASE WHEN RespTypeT.CategoryType = 3 AND RespTypeT.CategoryPlacement = 2 THEN SUM(ChargeRespT.Amount) ELSE 0 END, "
				"WorkersCompPriInsurance = CASE WHEN RespTypeT.CategoryType = 4 AND RespTypeT.CategoryPlacement = 1 THEN SUM(ChargeRespT.Amount) ELSE 0 END, "
				"WorkersCompSecInsurance = CASE WHEN RespTypeT.CategoryType = 4 AND RespTypeT.CategoryPlacement = 2 THEN SUM(ChargeRespT.Amount) ELSE 0 END, "
				"DentalPriInsurance = CASE WHEN RespTypeT.CategoryType = 5 AND RespTypeT.CategoryPlacement = 1 THEN SUM(ChargeRespT.Amount) ELSE 0 END, "
				"DentalSecInsurance = CASE WHEN RespTypeT.CategoryType = 5 AND RespTypeT.CategoryPlacement = 2 THEN SUM(ChargeRespT.Amount) ELSE 0 END, "
				"StudyPriInsurance = CASE WHEN RespTypeT.CategoryType = 6 AND RespTypeT.CategoryPlacement = 1 THEN SUM(ChargeRespT.Amount) ELSE 0 END, "
				"StudySecInsurance = CASE WHEN RespTypeT.CategoryType = 6 AND RespTypeT.CategoryPlacement = 2 THEN SUM(ChargeRespT.Amount) ELSE 0 END, "
				"LOPPriInsurance = CASE WHEN RespTypeT.CategoryType = 7 AND RespTypeT.CategoryPlacement = 1 THEN SUM(ChargeRespT.Amount) ELSE 0 END, "
				"LOPSecInsurance = CASE WHEN RespTypeT.CategoryType = 7 AND RespTypeT.CategoryPlacement = 2 THEN SUM(ChargeRespT.Amount) ELSE 0 END, "
				"LOAPriInsurance = CASE WHEN RespTypeT.CategoryType = 8 AND RespTypeT.CategoryPlacement = 1 THEN SUM(ChargeRespT.Amount) ELSE 0 END, "
				"LOASecInsurance = CASE WHEN RespTypeT.CategoryType = 8 AND RespTypeT.CategoryPlacement = 2 THEN SUM(ChargeRespT.Amount) ELSE 0 END "
				"FROM InsuredPartyT "
				"LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				"RIGHT OUTER JOIN ChargeRespT ON  InsuredPartyT.PersonID = ChargeRespT.InsuredPartyID RIGHT OUTER JOIN LineItemT ON  ChargeRespT.ChargeID = LineItemT.ID "
				"WHERE (LineItemT.Deleted = 0) AND  (LineItemT.Type = 10) "
				"GROUP BY InsuredPartyT.RespTypeID, RespTypeT.CategoryType, RespTypeT.CategoryPlacement, LineItemT.ID, LineItemT.PatientID,  LineItemT.Type) AS StmtCharges "
				"UNION "
				"SELECT StmtPays.ID, StmtPays.PatFixID,  StmtPays.Type, StmtPays.UnAppliedAmount,  StmtPays.TotalInsurance,  "
				"StmtPays.PriInsurance, StmtPays.SecInsurance,  StmtPays.OthInsurance, "
				"StmtPays.VisionPriInsurance, StmtPays.VisionSecInsurance, StmtPays.AutoPriInsurance,  StmtPays.AutoSecInsurance, StmtPays.WorkersCompPriInsurance,  StmtPays.WorkersCompSecInsurance, StmtPays.DentalPriInsurance,  StmtPays.DentalSecInsurance, StmtPays.StudyPriInsurance,  StmtPays.StudySecInsurance, StmtPays.LOPPriInsurance,  StmtPays.LOPSecInsurance, StmtPays.LOAPriInsurance,  StmtPays.LOASecInsurance "
				"FROM (SELECT LineItemT.ID, LineItemT.Type,  LineItemT.PatientID AS PatFixID, UnAppliedAmount = CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL  THEN 0 ELSE [AppliesT].[Amount] END)  ELSE MIN([LineItemT].[Amount])  - SUM(CASE WHEN [AppliesT].[Amount] IS NULL  THEN 0 ELSE [AppliesT].[Amount] END) END, TotalInsurance = CASE WHEN MIN([PaymentsT].[InsuredPartyID]) > 0 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount])  + SUM(CASE WHEN [AppliesT].[Amount] IS NULL  THEN 0 ELSE [AppliesT].[Amount] END)  ELSE MIN([LineItemT].[Amount])  - SUM(CASE WHEN [AppliesT].[Amount] IS NULL  THEN 0 ELSE [AppliesT].[Amount] END)  END ELSE 0 END, "
				"PriInsurance = CASE WHEN [InsuredPartyT].[RespTypeID] = 1 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END ELSE 0 END, "
				"SecInsurance = CASE WHEN [InsuredPartyT].[RespTypeID] = 2 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END  ELSE 0 END, "
				"OthInsurance = CASE WHEN [InsuredPartyT].[RespTypeID] <> 1 AND InsuredPartyT.RespTypeID <> 2 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END ELSE 0 END, "
				"VisionPriInsurance = CASE WHEN RespTypeT.CategoryType = 2 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END ELSE 0 END, "
				"VisionSecInsurance = CASE WHEN RespTypeT.CategoryType = 2 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END  ELSE 0 END, "
				"AutoPriInsurance = CASE WHEN RespTypeT.CategoryType = 3 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END ELSE 0 END, "
				"AutoSecInsurance = CASE WHEN RespTypeT.CategoryType = 3 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END  ELSE 0 END, "
				"WorkersCompPriInsurance = CASE WHEN RespTypeT.CategoryType = 4 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END ELSE 0 END, "
				"WorkersCompSecInsurance = CASE WHEN RespTypeT.CategoryType = 4 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END  ELSE 0 END, "
				"DentalPriInsurance = CASE WHEN RespTypeT.CategoryType = 5 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END ELSE 0 END, "
				"DentalSecInsurance = CASE WHEN RespTypeT.CategoryType = 5 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END  ELSE 0 END, "
				"StudyPriInsurance = CASE WHEN RespTypeT.CategoryType = 6 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END ELSE 0 END, "
				"StudySecInsurance = CASE WHEN RespTypeT.CategoryType = 6 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END  ELSE 0 END, "
				"LOPPriInsurance = CASE WHEN RespTypeT.CategoryType = 7 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END ELSE 0 END, "
				"LOPSecInsurance = CASE WHEN RespTypeT.CategoryType = 7 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END  ELSE 0 END, "
				"LOAPriInsurance = CASE WHEN RespTypeT.CategoryType = 8 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END ELSE 0 END, "
				"LOASecInsurance = CASE WHEN RespTypeT.CategoryType = 8 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END  ELSE 0 END "
				"FROM InsuredPartyT "
				"LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				"RIGHT OUTER JOIN PaymentsT ON InsuredPartyT.PersonID = PaymentsT.InsuredPartyID LEFT OUTER JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID RIGHT OUTER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
				"WHERE (LineItemT.Deleted = 0) AND (LineItemT.Type < 10) "
				"GROUP BY LineItemT.ID, LineItemT.Type, LineItemT.PatientID, [InsuredPartyT].[RespTypeID], RespTypeT.CategoryType, RespTypeT.CategoryPlacement "
				"HAVING (CASE WHEN [LineItemT].[Type] = 3 THEN - 1 * MIN([LineItemT].[Amount]) + SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) ELSE MIN([LineItemT].[Amount]) - SUM(CASE WHEN [AppliesT].[Amount] IS NULL THEN 0 ELSE [AppliesT].[Amount] END) END <> 0) AND LineItemT.Type = 1) AS StmtPays "
				"UNION ALL "
				"SELECT StmtApplies.ChargeID, StmtApplies.PatFixID, StmtApplies.Type, StmtApplies.ApplyAmount, StmtApplies.TotalInsurance, "
				"StmtApplies.PriInsurance, StmtApplies.SecInsurance, StmtApplies.OthInsurance, "
				"StmtApplies.VisionPriInsurance, StmtApplies.VisionSecInsurance, StmtApplies.AutoPriInsurance,  StmtApplies.AutoSecInsurance, StmtApplies.WorkersCompPriInsurance,  StmtApplies.WorkersCompSecInsurance, StmtApplies.DentalPriInsurance,  StmtApplies.DentalSecInsurance, StmtApplies.StudyPriInsurance,  StmtApplies.StudySecInsurance, StmtApplies.LOPPriInsurance,  StmtApplies.LOPSecInsurance, StmtApplies.LOAPriInsurance,  StmtApplies.LOASecInsurance "
				"FROM (SELECT * FROM (SELECT AppliesT.SourceID AS ChargeID, LineItemT1.PatientID AS PatFixID, LineItemT1.Type, ApplyAmount = CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END, TotalInsurance = CASE WHEN [PaymentsT].[InsuredPartyID] > 0 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"PriInsurance = CASE WHEN InsuredPartyT.RespTypeID = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"SecInsurance = CASE WHEN InsuredPartyT.RespTypeID = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"OthInsurance = CASE WHEN InsuredPartyT.RespTypeID <> 1 AND InsuredPartyT.RespTypeID <> 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"VisionPriInsurance = CASE WHEN RespTypeT.CategoryType = 2 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"VisionSecInsurance = CASE WHEN RespTypeT.CategoryType = 2 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"AutoPriInsurance = CASE WHEN RespTypeT.CategoryType = 3 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"AutoSecInsurance = CASE WHEN RespTypeT.CategoryType = 3 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"WorkersCompPriInsurance = CASE WHEN RespTypeT.CategoryType = 4 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"WorkersCompSecInsurance = CASE WHEN RespTypeT.CategoryType = 4 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"DentalPriInsurance = CASE WHEN RespTypeT.CategoryType = 5 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"DentalSecInsurance = CASE WHEN RespTypeT.CategoryType = 5 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"StudyPriInsurance = CASE WHEN RespTypeT.CategoryType = 6 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"StudySecInsurance = CASE WHEN RespTypeT.CategoryType = 6 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"LOPPriInsurance = CASE WHEN RespTypeT.CategoryType = 7 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"LOPSecInsurance = CASE WHEN RespTypeT.CategoryType = 7 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"LOAPriInsurance = CASE WHEN RespTypeT.CategoryType = 8 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"LOASecInsurance = CASE WHEN RespTypeT.CategoryType = 8 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END "
				"FROM InsuredPartyT "
				"LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				"RIGHT OUTER JOIN PaymentsT ON InsuredPartyT.PersonID = PaymentsT.InsuredPartyID LEFT OUTER JOIN LineItemT LineItemT1 ON PaymentsT.ID = LineItemT1.ID RIGHT OUTER JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID "
				"WHERE (LineItemT1.Deleted = 0) AND (LineItemT1.Deleted = 0) AND (AppliesT.PointsToPayments = 0)) AS StatementDataAppliesCharges "
				"UNION ALL "
				"SELECT * FROM (SELECT AppliesT.DestID AS ChargeID, LineItemT1.PatientID AS PatFixID, LineItemT1.Type, ApplyAmount = CASE WHEN LineItemT1.Type = 3 THEN -1 * AppliesT.Amount ELSE AppliesT.Amount END, TotalInsurance = CASE WHEN [PaymentsT].[InsuredPartyID] > 0 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"PriInsurance = CASE WHEN InsuredPartyT.RespTypeID = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"SecInsurance = CASE WHEN InsuredPartyT.RespTypeID = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"OthInsurance = CASE WHEN InsuredPartyT.RespTypeID <> 1 AND InsuredPartyT.RespTypeID <> 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"VisionPriInsurance = CASE WHEN RespTypeT.CategoryType = 2 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"VisionSecInsurance = CASE WHEN RespTypeT.CategoryType = 2 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"AutoPriInsurance = CASE WHEN RespTypeT.CategoryType = 3 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"AutoSecInsurance = CASE WHEN RespTypeT.CategoryType = 3 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"WorkersCompPriInsurance = CASE WHEN RespTypeT.CategoryType = 4 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"WorkersCompSecInsurance = CASE WHEN RespTypeT.CategoryType = 4 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"DentalPriInsurance = CASE WHEN RespTypeT.CategoryType = 5 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"DentalSecInsurance = CASE WHEN RespTypeT.CategoryType = 5 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"StudyPriInsurance = CASE WHEN RespTypeT.CategoryType = 6 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"StudySecInsurance = CASE WHEN RespTypeT.CategoryType = 6 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"LOPPriInsurance = CASE WHEN RespTypeT.CategoryType = 7 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"LOPSecInsurance = CASE WHEN RespTypeT.CategoryType = 7 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"LOAPriInsurance = CASE WHEN RespTypeT.CategoryType = 8 AND RespTypeT.CategoryPlacement = 1 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END, "
				"LOASecInsurance = CASE WHEN RespTypeT.CategoryType = 8 AND RespTypeT.CategoryPlacement = 2 THEN CASE WHEN LineItemT1.Type = 3 THEN -1 * [AppliesT].[Amount] ELSE AppliesT.Amount END ELSE 0 END "
				"FROM PaymentsT "
				"LEFT OUTER JOIN InsuredPartyT ON PaymentsT.InsuredPartyID = InsuredPartyT.PersonID "
				"LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				"LEFT OUTER JOIN LineItemT LineItemT1 ON PaymentsT.ID = LineItemT1.ID "
				"RIGHT OUTER JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID LEFT JOIN LineItemT LineItemT2 ON AppliesT.DestID = LineItemT2.ID "
				"WHERE (LineItemT1.Deleted = 0) AND (LineItemT1.Deleted = 0) AND (AppliesT.PointsToPayments = 1) AND LineItemT2.Type = 1) AS StatementDataAppliesPays) AS StmtApplies) AS StatementAllData) AS StatementPatBalanceSub "
				"GROUP BY StatementPatBalanceSub.PatFixID) SubQ "
				"ON PersonT.ID = SubQ.PersonID INNER JOIN PatientsT WITH(NOLOCK) ON PersonT.ID = PatientsT.PersonID "
				") FilterAllNetPayInfoSubQ ON PersonT.ID = FilterAllNetPayInfoSubQ.PersonID "),

	ADD_FILTER_FIELD(317, "Net Payments (All Pts) Patient Total", "FilterAllNetPayInfoSubQ.PatTotPays", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 316), 
	ADD_FILTER_FIELD(318, "Net Payments (All Pts) Insurance Total", "FilterAllNetPayInfoSubQ.InsTotPays", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 316), 
	ADD_FILTER_FIELD(319, "Net Payments (All Pts) Insurance Primary", "FilterAllNetPayInfoSubQ.InsPriPays", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 316), 
	ADD_FILTER_FIELD(320, "Net Payments (All Pts) Insurance Secondary", "FilterAllNetPayInfoSubQ.InsSecPays", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 316), 
	ADD_FILTER_FIELD(321, "Net Payments (All Pts) Insurance Other", "FilterAllNetPayInfoSubQ.InsOthPays", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 316), 

	// (j.jones 2011-04-01 14:52) - PLID 43109 - added Vision Primary & Secondary payment filters
	ADD_FILTER_FIELD(481, "Net Payments (All Pts) Insurance Vision Primary", "FilterAllNetPayInfoSubQ.VisionInsPriPays", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 316), 
	ADD_FILTER_FIELD(482, "Net Payments (All Pts) Insurance Vision Secondary", "FilterAllNetPayInfoSubQ.VisionInsSecPays", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 316), 

	// (d.singleton 2012-09-11 12:22) - PLID 51808 add auto filter
	// (d.singleton 2012-09-14 13:34) - PLID 52657 add workers comp filter
	// (d.singleton 2012-09-14 13:34) - PLID 52658 add dental filter
	// (d.singleton 2012-09-14 13:35) - PLID 52659 add study filter
	// (d.singleton 2012-09-14 13:35) - PLID 52660 add LOP filter
	// (d.singleton 2012-09-14 13:35) - PLID 52661 add LOA filter
	ADD_FILTER_FIELD(610, "Net Payments (All Pts) Insurance Auto Primary", "FilterAllNetPayInfoSubQ.AutoInsPriPays", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 316), 
	ADD_FILTER_FIELD(611, "Net Payments (All Pts) Insurance Auto Secondary", "FilterAllNetPayInfoSubQ.AutoInsSecPays", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 316),
	ADD_FILTER_FIELD(612, "Net Payments (All Pts) Insurance Workers Comp Primary", "FilterAllNetPayInfoSubQ.WorkersCompInsPriPays", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 316), 
	ADD_FILTER_FIELD(613, "Net Payments (All Pts) Insurance Workers Comp Secondary", "FilterAllNetPayInfoSubQ.WorkersCompInsSecPays", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 316),
	ADD_FILTER_FIELD(614, "Net Payments (All Pts) Insurance Dental Primary", "FilterAllNetPayInfoSubQ.DentalInsPriPays", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 316), 
	ADD_FILTER_FIELD(615, "Net Payments (All Pts) Insurance Dental Secondary", "FilterAllNetPayInfoSubQ.DentalInsSecPays", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 316),
	ADD_FILTER_FIELD(616, "Net Payments (All Pts) Insurance Study Primary", "FilterAllNetPayInfoSubQ.StudyInsPriPays", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 316), 
	ADD_FILTER_FIELD(617, "Net Payments (All Pts) Insurance Study Secondary", "FilterAllNetPayInfoSubQ.StudyInsSecPays", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 316),
	ADD_FILTER_FIELD(618, "Net Payments (All Pts) Insurance LOP Primary", "FilterAllNetPayInfoSubQ.LOPInsPriPays", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 316), 
	ADD_FILTER_FIELD(619, "Net Payments (All Pts) Insurance LOP Secondary", "FilterAllNetPayInfoSubQ.LOPInsSecPays", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 316),
	ADD_FILTER_FIELD(620, "Net Payments (All Pts) Insurance LOA Primary", "FilterAllNetPayInfoSubQ.LOAInsPriPays", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 316), 
	ADD_FILTER_FIELD(621, "Net Payments (All Pts) Insurance LOA Secondary", "FilterAllNetPayInfoSubQ.LOAInsSecPays", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 316),

	ADD_FILTER_FIELD(172, "Suppress Statement", "PatientsT.SuppressStatement", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboValues, P("Suppressed") P("1") P("Not Suppressed") P("0"), NULL, 159),

	// (j.jones 2009-11-03 13:18) - PLID 36181 - added Statement Last Sent Date
	ADD_FILTER_FIELD(417, "Statement Last Sent Date", "dbo.AsDateNoTime(StatementLastSentQ.LastStatementDate)", fboPerson, foEquality, foGreater, ftDate, NULL,
				"(SELECT MailSent.PersonID, Max(Date) AS LastStatementDate FROM MailSent INNER JOIN MailSentNotesT ON MailSent.MailID = MailSentNotesT.MailID "
				"WHERE (MailSentNotesT.Note Like '%%Patient%%Statement%%Printed%%') OR (MailSentNotesT.Note Like '%%Patient%%Statement%%Run%%') OR (MailSentNotesT.Note Like '%%E-Statement%%Exported%%') "
				"GROUP BY MailSent.PersonID) AS StatementLastSentQ ON PersonT.ID = StatementLastSentQ.PersonID"), 

	ADD_FILTER_FIELD(106, "Referring Physician", "PatientsT.DefaultReferringPhyID", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ReferringPhysT.PersonID AS ID, PersonT.[Last] + ', ' + PersonT.[First] + ' ' + PersonT.Middle AS Name FROM ReferringPhysT WITH(NOLOCK) INNER JOIN PersonT WITH(NOLOCK) ON ReferringPhysT.PersonID = PersonT.ID ORDER BY PersonT.[Last] ASC, PersonT.[First] ASC, PersonT.[Middle] ASC"), NULL, 159, NULL, NULL, NULL, NULL, 0, P("ReferringPhysT.PersonID") P("PersonT.[Last] + ', ' + PersonT.[First] + ' ' + PersonT.Middle") P("ReferringPhysT WITH(NOLOCK) INNER JOIN PersonT WITH(NOLOCK) ON ReferringPhysT.PersonID = PersonT.ID")),
	ADD_FILTER_FIELD(209, "Referring Phy Prefix", "RefPhysContactInfo.Prefix", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 107),
	ADD_FILTER_FIELD(107, "Referring Phy First Name", "RefPhysContactInfo.[First]", fboPerson, foAll, foBeginsWith, ftText, NULL, "(SELECT PersonT.*, PrefixT.Prefix FROM PersonT WITH(NOLOCK) LEFT JOIN PrefixT ON PersonT.PrefixID = PrefixT.ID) AS RefPhysContactInfo ON ReferringPhysT.PersonID = RefPhysContactInfo.ID", 117),
	ADD_FILTER_FIELD(108, "Referring Phy Middle Name", "RefPhysContactInfo.Middle", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 107),
	ADD_FILTER_FIELD(109, "Referring Phy Last Name", "RefPhysContactInfo.[Last]", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 107),
	ADD_FILTER_FIELD(210, "Referring Phy Title", "RefPhysContactInfo.Title", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 107),
	ADD_FILTER_FIELD(110, "Referring Phy Address1", "RefPhysContactInfo.Address1", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 107),
	ADD_FILTER_FIELD(111, "Referring Phy Address2", "RefPhysContactInfo.Address2", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 107),
	ADD_FILTER_FIELD(112, "Referring Phy City", "RefPhysContactInfo.City", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 107),
	ADD_FILTER_FIELD(113, "Referring Phy State", "RefPhysContactInfo.State", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 107),
	ADD_FILTER_FIELD(114, "Referring Phy Zipcode", "RefPhysContactInfo.Zip", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 107),
	ADD_FILTER_FIELD(115, "Referring Phy Phone", "(Replace(Replace(Replace(Replace(RefPhysContactInfo.WorkPhone,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL, 107),
	ADD_FILTER_FIELD(116, "Referring Phy Ext", "RefPhysContactInfo.Extension", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 107),
	ADD_FILTER_FIELD(117, "Referring Phy UPIN", "ReferringPhyST.UPIN", fboPerson, foAll, foBeginsWith, ftText, NULL, "ReferringPhysT ON PatientsT.DefaultReferringPhyID = ReferringPhysT.PersonID", 159),


	// (j.gruber 2011-09-30 14:33) - PLID 45355 - Affiliate Physician Fields
	ADD_FILTER_FIELD(546, "Affiliate Physician", "PatientsT.AffiliatePhysID", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ReferringPhysT.PersonID AS ID, PersonT.[Last] + ', ' + PersonT.[First] + ' ' + PersonT.Middle AS Name FROM ReferringPhysT WITH(NOLOCK) INNER JOIN PersonT WITH(NOLOCK) ON ReferringPhysT.PersonID = PersonT.ID WHERE ReferringPhysT.AffiliatePhysician = 1 ORDER BY PersonT.[Last] ASC, PersonT.[First] ASC, PersonT.[Middle] ASC"), NULL, 159, NULL, NULL, NULL, NULL, 0, P("ReferringPhysT.PersonID") P("PersonT.[Last] + ', ' + PersonT.[First] + ' ' + PersonT.Middle") P("ReferringPhysT WITH(NOLOCK) INNER JOIN PersonT WITH(NOLOCK) ON ReferringPhysT.PersonID = PersonT.ID") P("ReferringPhysT.AffiliatePhysician = 1")),
	/*ADD_FILTER_FIELD(209, "Affiliate Phy Prefix", "RefPhysContactInfo.Prefix", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 107),
	ADD_FILTER_FIELD(107, "Affiliate Phy First Name", "RefPhysContactInfo.[First]", fboPerson, foAll, foBeginsWith, ftText, NULL, "(SELECT PersonT.*, PrefixT.Prefix FROM PersonT WITH(NOLOCK) LEFT JOIN PrefixT ON PersonT.PrefixID = PrefixT.ID) AS RefPhysContactInfo ON ReferringPhysT.PersonID = RefPhysContactInfo.ID", 117),
	ADD_FILTER_FIELD(108, "Affiliate Phy Middle Name", "RefPhysContactInfo.Middle", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 107),
	ADD_FILTER_FIELD(109, "Affiliate Phy Last Name", "RefPhysContactInfo.[Last]", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 107),
	ADD_FILTER_FIELD(210, "Affiliate Phy Title", "RefPhysContactInfo.Title", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 107),
	ADD_FILTER_FIELD(110, "Affiliate Phy Address1", "RefPhysContactInfo.Address1", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 107),
	ADD_FILTER_FIELD(111, "Affiliate Phy Address2", "RefPhysContactInfo.Address2", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 107),
	ADD_FILTER_FIELD(112, "Affiliate Phy City", "RefPhysContactInfo.City", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 107),
	ADD_FILTER_FIELD(113, "Affiliate Phy State", "RefPhysContactInfo.State", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 107),
	ADD_FILTER_FIELD(114, "Affiliate Phy Zipcode", "RefPhysContactInfo.Zip", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 107),
	ADD_FILTER_FIELD(115, "Affiliate Phy Phone", "(Replace(Replace(Replace(Replace(RefPhysContactInfo.WorkPhone,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL, 107),
	ADD_FILTER_FIELD(116, "Affiliate Phy Ext", "RefPhysContactInfo.Extension", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 107),
	ADD_FILTER_FIELD(117, "Affiliate Phy UPIN", "ReferringPhyST.UPIN", fboPerson, foAll, foBeginsWith, ftText, NULL, "ReferringPhysT ON PatientsT.DefaultReferringPhyID = ReferringPhysT.PersonID", 159),*/

	
	ADD_FILTER_FIELD(191, "Primary Physician", "PatientsT.PCP", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ReferringPhysT.PersonID AS ID, PersonT.[Last] + ', ' + PersonT.[First] + ' ' + PersonT.[Middle] AS Name FROM ReferringPhysT INNER JOIN PersonT WITH(NOLOCK) ON ReferringPhysT.PersonID = PersonT.ID ORDER BY PersonT.[Last] ASC, PersonT.[First] ASC, PersonT.[Middle] ASC"), NULL, 159, NULL, NULL, NULL, NULL, 0, P("ReferringPhysT.PersonID") P("PersonT.[Last] + ', ' + PersonT.[First] + ' ' + PersonT.[Middle]") P("ReferringPhysT INNER JOIN PersonT WITH(NOLOCK) ON ReferringPhysT.PersonID = PersonT.ID")),
	ADD_FILTER_FIELD(192, "Referring Patient", "PatientsT.ReferringPatientID", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT RefPatPatientsT.PersonID AS ID, PersonT.[Last] + ', ' + PersonT.[First] + ' ' + PersonT.[Middle] AS Name FROM PatientsT AS RefPatPatientsT WITH(NOLOCK) INNER JOIN PersonT WITH(NOLOCK) ON RefPatPatientsT.PersonID = PersonT.ID ORDER BY PersonT.[Last] ASC, PersonT.[First] ASC, PersonT.[Middle] ASC"), NULL, 159, NULL, NULL, NULL, NULL, 0, P("RefPatPatientsT.PersonID") P("PersonT.[Last] + ', ' + PersonT.[First] + ' ' + PersonT.[Middle]") P("PatientsT AS RefPatPatientsT WITH(NOLOCK) INNER JOIN PersonT WITH(NOLOCK) ON RefPatPatientsT.PersonID = PersonT.ID")),

	ADD_FILTER_FIELD(340, "Responsible Party First Name", "ResponsiblePartyT.[First]", fboPerson, foAll, foBeginsWith, ftText, NULL, "(SELECT ResponsiblePartyT.*, PersonT.* FROM ResponsiblePartyT LEFT JOIN PersonT WITH(NOLOCK) ON ResponsiblePartyT.PersonID = PersonT.ID) ResponsiblePartyT ON PersonT.ID = ResponsiblePartyT.PatientID"),
	ADD_FILTER_FIELD(341, "Responsible Party Middle Name", "ResponsiblePartyT.Middle", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 340),
	ADD_FILTER_FIELD(342, "Responsible Party Last Name", "ResponsiblePartyT.[Last]", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 340),
	ADD_FILTER_FIELD(343, "Responsible Party Address1", "ResponsiblePartyT.Address1", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 340),
	ADD_FILTER_FIELD(344, "Responsible Party Address2", "ResponsiblePartyT.Address2", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 340),
	ADD_FILTER_FIELD(345, "Responsible Party City", "ResponsiblePartyT.City", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 340),
	ADD_FILTER_FIELD(346, "Responsible Party State", "ResponsiblePartyT.State", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 340),
	ADD_FILTER_FIELD(347, "Responsible Party Zipcode", "ResponsiblePartyT.Zip", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 340),
	ADD_FILTER_FIELD(348, "Responsible Party Phone", "(Replace(Replace(Replace(Replace(ResponsiblePartyT.HomePhone,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL, 340),
	ADD_FILTER_FIELD(349, "Responsible Party Employer", "ResponsiblePartyT.Employer", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 340),

	ADD_FILTER_FIELD(211, "Primary Prefix", "InsuredPartyT1.Prefix", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 118),
	// (j.gruber 2010-08-04 09:23) - PLID 39948 - change copay structure
	ADD_FILTER_FIELD(118, "Primary First Name", "InsuredPartyT1.[First]", fboPerson, foAll, foBeginsWith, ftText, NULL, "(SELECT InsuredPartyT.*, PersonT.*, PrefixT.Prefix FROM (SELECT InsuredPartyT.*, InsPartyPayGroupsT.CopayPercentage as CopayPercent, InsPartyPayGroupsT.CopayMoney as Copay FROM InsuredPartyT LEFT JOIN (SELECT InsuredPartyID, CopayMoney, CopayPercentage FROM "
				"(SELECT * FROM ServicePayGroupsT WHERE Name = 'Copay') ServicePayGroupsT "
				" LEFT JOIN InsuredPartyPayGroupsT ON ServicePayGroupsT.ID = InsuredPartyPayGroupsT.PayGroupID) InsPartyPayGroupsT "
				" ON InsuredPartyT.PersonID = InsPartyPayGroupsT.InsuredPartyID WHERE RespTypeID = 1) InsuredPartyT LEFT JOIN (PersonT WITH(NOLOCK) LEFT JOIN PrefixT ON PersonT.PrefixID = PrefixT.ID) ON InsuredPartyT.PersonID = PersonT.ID WHERE InsuredPartyT.RespTypeID = 1) InsuredPartyT1 ON PersonT.ID = InsuredPartyT1.PatientID"),
	ADD_FILTER_FIELD(119, "Primary Middle Name", "InsuredPartyT1.Middle", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 118),
	ADD_FILTER_FIELD(120, "Primary Last Name", "InsuredPartyT1.[Last]", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 118),
	ADD_FILTER_FIELD(212, "Primary Title", "InsuredPartyT1.Title", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 118),
	ADD_FILTER_FIELD(121, "Primary Address1", "InsuredPartyT1.Address1", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 118),
	ADD_FILTER_FIELD(122, "Primary Address2", "InsuredPartyT1.Address2", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 118),
	ADD_FILTER_FIELD(123, "Primary City", "InsuredPartyT1.City", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 118),
	ADD_FILTER_FIELD(124, "Primary State", "InsuredPartyT1.State", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 118),
	ADD_FILTER_FIELD(125, "Primary Zipcode", "InsuredPartyT1.Zip", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 118),
	ADD_FILTER_FIELD(126, "Primary Phone", "(Replace(Replace(Replace(Replace(InsuredPartyT1.HomePhone,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL, 118),
	ADD_FILTER_FIELD(127, "Primary Ins Group Number", "InsuredPartyT1.PolicyGroupNum", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 118),
	ADD_FILTER_FIELD(128, "Primary Ins ID", "InsuredPartyT1.IDForInsurance", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 118),
	ADD_FILTER_FIELD(175, "Primary Effective Date", "dbo.AsDateNoTime(InsuredPartyT1.EffectiveDate)", fboPerson, foAll, foEqual, ftDate, NULL, NULL, 118),	//see note at the top about converting date
	ADD_FILTER_FIELD(176, "Primary CoPay Amount", "InsuredPartyT1.CoPay", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 118),
	ADD_FILTER_FIELD(390, "Primary CoPay Percent", "InsuredPartyT1.CopayPercent", fboPerson, foEquality, foGreater, ftNumber, NULL, NULL, 118),
	ADD_FILTER_FIELD(338, "Primary Employer", "InsuredPartyT1.Employer", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 118),
	ADD_FILTER_FIELD(129, "Primary Insurance", "InsuranceCoT1.Name", fboPerson, foAll, foBeginsWith, ftText, NULL, "InsuranceCoT AS InsuranceCoT1 ON InsuredPartyT1.InsuranceCoID = InsuranceCoT1.PersonID", 118),

	ADD_FILTER_FIELD(213, "Secondary Prefix", "InsuredPartyT2.Prefix", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 130),
	// (j.gruber 2010-08-04 09:23) - PLID 39948 - change copay structure
	ADD_FILTER_FIELD(130, "Secondary First Name", "InsuredPartyT2.[First]", fboPerson, foAll, foBeginsWith, ftText, NULL, "(SELECT InsuredPartyT.*, PersonT.*, PrefixT.Prefix FROM (SELECT InsuredPartyT.*, InsPartyPayGroupsT.CopayPercentage as CopayPercent, InsPartyPayGroupsT.CopayMoney as Copay FROM InsuredPartyT LEFT JOIN (SELECT InsuredPartyID, CopayMoney, CopayPercentage FROM "
				"(SELECT * FROM ServicePayGroupsT WHERE Name = 'Copay') ServicePayGroupsT "
				" LEFT JOIN InsuredPartyPayGroupsT ON ServicePayGroupsT.ID = InsuredPartyPayGroupsT.PayGroupID) InsPartyPayGroupsT "
				" ON InsuredPartyT.PersonID = InsPartyPayGroupsT.InsuredPartyID WHERE RespTypeID = 2) InsuredPartyT LEFT JOIN (PersonT WITH(NOLOCK) LEFT JOIN PrefixT ON PersonT.PrefixID = PrefixT.ID) ON InsuredPartyT.PersonID = PersonT.ID WHERE InsuredPartyT.RespTypeID = 2) InsuredPartyT2 ON PersonT.ID = InsuredPartyT2.PatientID"),
	ADD_FILTER_FIELD(131, "Secondary Middle Name", "InsuredPartyT2.Middle", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 130),
	ADD_FILTER_FIELD(132, "Secondary Last Name", "InsuredPartyT2.[Last]", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 130),
	ADD_FILTER_FIELD(214, "Secondary Title", "InsuredPartyT2.Title", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 130),
	ADD_FILTER_FIELD(133, "Secondary Address1", "InsuredPartyT2.Address1", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 130),
	ADD_FILTER_FIELD(134, "Secondary Address2", "InsuredPartyT2.Address2", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 130),
	ADD_FILTER_FIELD(135, "Secondary City", "InsuredPartyT2.City", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 130),
	ADD_FILTER_FIELD(136, "Secondary State", "InsuredPartyT2.State", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 130),
	ADD_FILTER_FIELD(137, "Secondary Zipcode", "InsuredPartyT2.Zip", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 130),
	ADD_FILTER_FIELD(138, "Secondary Phone", "(Replace(Replace(Replace(Replace(InsuredPartyT2.HomePhone,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL, 130),
	ADD_FILTER_FIELD(139, "Secondary Ins Group Number", "InsuredPartyT2.PolicyGroupNum", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 130),
	ADD_FILTER_FIELD(140, "Secondary Ins ID", "InsuredPartyT2.IDForInsurance", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 130),
	ADD_FILTER_FIELD(177, "Secondary Effective Date", "dbo.AsDateNoTime(InsuredPartyT2.EffectiveDate)", fboPerson, foAll, foEqual, ftDate, NULL, NULL, 130),	//see note at the top about converting date
	ADD_FILTER_FIELD(178, "Secondary CoPay Amount", "InsuredPartyT2.CoPay", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 130),
	ADD_FILTER_FIELD(391, "Secondary CoPay Percent", "InsuredPartyT2.CopayPercent", fboPerson, foEquality, foGreater, ftNumber, NULL, NULL, 130),
	ADD_FILTER_FIELD(339, "Secondary Employer", "InsuredPartyT2.Employer", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 130),
	ADD_FILTER_FIELD(141, "Secondary Insurance", "InsuranceCoT2.Name", fboPerson, foAll, foBeginsWith, ftText, NULL, "InsuranceCoT AS InsuranceCoT2 ON InsuredPartyT2.InsuranceCoID = InsuranceCoT2.PersonID", 130),

	// (j.jones 2011-04-01 14:52) - PLID 43109 - added Vision Primary & Secondary filters
	ADD_FILTER_FIELD(484, "Vision (Primary) Prefix", "InsuredPartyTVision1.Prefix", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 483),
	ADD_FILTER_FIELD(483, "Vision (Primary) First Name", "InsuredPartyTVision1.[First]", fboPerson, foAll, foBeginsWith, ftText, NULL, "(SELECT InsuredPartyT.*, PersonT.*, PrefixT.Prefix FROM (SELECT InsuredPartyT.*, InsPartyPayGroupsT.CopayPercentage as CopayPercent, InsPartyPayGroupsT.CopayMoney as Copay "
				"FROM InsuredPartyT LEFT JOIN (SELECT InsuredPartyID, CopayMoney, CopayPercentage FROM "
				"(SELECT * FROM ServicePayGroupsT WHERE Name = 'Copay') ServicePayGroupsT "
				" LEFT JOIN InsuredPartyPayGroupsT ON ServicePayGroupsT.ID = InsuredPartyPayGroupsT.PayGroupID) InsPartyPayGroupsT "
				" ON InsuredPartyT.PersonID = InsPartyPayGroupsT.InsuredPartyID "
				" LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				" WHERE RespTypeT.CategoryType = 2 AND RespTypeT.CategoryPlacement = 1) InsuredPartyT "
				" LEFT JOIN (PersonT WITH(NOLOCK) LEFT JOIN PrefixT ON PersonT.PrefixID = PrefixT.ID) ON InsuredPartyT.PersonID = PersonT.ID "
				" LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				" WHERE RespTypeT.CategoryType = 2 AND RespTypeT.CategoryPlacement = 1) InsuredPartyTVision1 ON PersonT.ID = InsuredPartyTVision1.PatientID"),
	ADD_FILTER_FIELD(485, "Vision (Primary) Middle Name", "InsuredPartyTVision1.Middle", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 483),
	ADD_FILTER_FIELD(486, "Vision (Primary) Last Name", "InsuredPartyTVision1.[Last]", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 483),
	ADD_FILTER_FIELD(487, "Vision (Primary) Title", "InsuredPartyTVision1.Title", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 483),
	ADD_FILTER_FIELD(488, "Vision (Primary) Address1", "InsuredPartyTVision1.Address1", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 483),
	ADD_FILTER_FIELD(489, "Vision (Primary) Address2", "InsuredPartyTVision1.Address2", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 483),
	ADD_FILTER_FIELD(490, "Vision (Primary) City", "InsuredPartyTVision1.City", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 483),
	ADD_FILTER_FIELD(491, "Vision (Primary) State", "InsuredPartyTVision1.State", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 483),
	ADD_FILTER_FIELD(492, "Vision (Primary) Zipcode", "InsuredPartyTVision1.Zip", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 483),
	ADD_FILTER_FIELD(493, "Vision (Primary) Phone", "(Replace(Replace(Replace(Replace(InsuredPartyTVision1.HomePhone,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL, 483),
	ADD_FILTER_FIELD(494, "Vision (Primary) Ins Group Number", "InsuredPartyTVision1.PolicyGroupNum", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 483),
	ADD_FILTER_FIELD(495, "Vision (Primary) Ins ID", "InsuredPartyTVision1.IDForInsurance", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 483),
	ADD_FILTER_FIELD(496, "Vision (Primary) Effective Date", "dbo.AsDateNoTime(InsuredPartyTVision1.EffectiveDate)", fboPerson, foAll, foEqual, ftDate, NULL, NULL, 483),	//see note at the top about converting date
	ADD_FILTER_FIELD(497, "Vision (Primary) CoPay Amount", "InsuredPartyTVision1.CoPay", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 483),
	ADD_FILTER_FIELD(498, "Vision (Primary) CoPay Percent", "InsuredPartyTVision1.CopayPercent", fboPerson, foEquality, foGreater, ftNumber, NULL, NULL, 483),
	ADD_FILTER_FIELD(499, "Vision (Primary) Employer", "InsuredPartyTVision1.Employer", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 483),
	ADD_FILTER_FIELD(500, "Vision (Primary) Insurance", "InsuranceCoTVision1.Name", fboPerson, foAll, foBeginsWith, ftText, NULL, "InsuranceCoT AS InsuranceCoTVision1 ON InsuredPartyTVision1.InsuranceCoID = InsuranceCoTVision1.PersonID", 483),

	ADD_FILTER_FIELD(502, "Vision (Secondary) Prefix", "InsuredPartyTVision2.Prefix", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 501),
	ADD_FILTER_FIELD(501, "Vision (Secondary) First Name", "InsuredPartyTVision2.[First]", fboPerson, foAll, foBeginsWith, ftText, NULL, "(SELECT InsuredPartyT.*, PersonT.*, PrefixT.Prefix FROM (SELECT InsuredPartyT.*, InsPartyPayGroupsT.CopayPercentage as CopayPercent, InsPartyPayGroupsT.CopayMoney as Copay FROM InsuredPartyT LEFT JOIN (SELECT InsuredPartyID, CopayMoney, CopayPercentage FROM "
				"(SELECT * FROM ServicePayGroupsT WHERE Name = 'Copay') ServicePayGroupsT "
				" LEFT JOIN InsuredPartyPayGroupsT ON ServicePayGroupsT.ID = InsuredPartyPayGroupsT.PayGroupID) InsPartyPayGroupsT "
				" ON InsuredPartyT.PersonID = InsPartyPayGroupsT.InsuredPartyID "
				" LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				" WHERE RespTypeT.CategoryType = 2 AND RespTypeT.CategoryPlacement = 2) InsuredPartyT "
				" LEFT JOIN (PersonT WITH(NOLOCK) LEFT JOIN PrefixT ON PersonT.PrefixID = PrefixT.ID) ON InsuredPartyT.PersonID = PersonT.ID "
				" LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				" WHERE RespTypeT.CategoryType = 2 AND RespTypeT.CategoryPlacement = 2) InsuredPartyTVision2 ON PersonT.ID = InsuredPartyTVision2.PatientID"),
	ADD_FILTER_FIELD(503, "Vision (Secondary) Middle Name", "InsuredPartyTVision2.Middle", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 501),
	ADD_FILTER_FIELD(504, "Vision (Secondary) Last Name", "InsuredPartyTVision2.[Last]", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 501),
	ADD_FILTER_FIELD(505, "Vision (Secondary) Title", "InsuredPartyTVision2.Title", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 501),
	ADD_FILTER_FIELD(506, "Vision (Secondary) Address1", "InsuredPartyTVision2.Address1", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 501),
	ADD_FILTER_FIELD(507, "Vision (Secondary) Address2", "InsuredPartyTVision2.Address2", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 501),
	ADD_FILTER_FIELD(508, "Vision (Secondary) City", "InsuredPartyTVision2.City", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 501),
	ADD_FILTER_FIELD(509, "Vision (Secondary) State", "InsuredPartyTVision2.State", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 501),
	ADD_FILTER_FIELD(510, "Vision (Secondary) Zipcode", "InsuredPartyTVision2.Zip", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 501),
	ADD_FILTER_FIELD(511, "Vision (Secondary) Phone", "(Replace(Replace(Replace(Replace(InsuredPartyTVision2.HomePhone,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL, 501),
	ADD_FILTER_FIELD(512, "Vision (Secondary) Ins Group Number", "InsuredPartyTVision2.PolicyGroupNum", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 501),
	ADD_FILTER_FIELD(513, "Vision (Secondary) Ins ID", "InsuredPartyTVision2.IDForInsurance", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 501),
	ADD_FILTER_FIELD(514, "Vision (Secondary) Effective Date", "dbo.AsDateNoTime(InsuredPartyTVision2.EffectiveDate)", fboPerson, foAll, foEqual, ftDate, NULL, NULL, 501),	//see note at the top about converting date
	ADD_FILTER_FIELD(515, "Vision (Secondary) CoPay Amount", "InsuredPartyTVision2.CoPay", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 501),
	ADD_FILTER_FIELD(516, "Vision (Secondary) CoPay Percent", "InsuredPartyTVision2.CopayPercent", fboPerson, foEquality, foGreater, ftNumber, NULL, NULL, 501),
	ADD_FILTER_FIELD(517, "Vision (Secondary) Employer", "InsuredPartyTVision2.Employer", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 501),
	ADD_FILTER_FIELD(518, "Vision (Secondary) Insurance", "InsuranceCoTVision2.Name", fboPerson, foAll, foBeginsWith, ftText, NULL, "InsuranceCoT AS InsuranceCoTVision2 ON InsuredPartyTVision2.InsuranceCoID = InsuranceCoTVision2.PersonID", 501),

	// (d.singleton 2012-09-12 15:57) - PLID 51808 added auto filters
	ADD_FILTER_FIELD(623, "Auto (Primary) Prefix", "InsuredPartyTAuto1.Prefix", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 622),
	ADD_FILTER_FIELD(622, "Auto (Primary) First Name", "InsuredPartyTAuto1.[First]", fboPerson, foAll, foBeginsWith, ftText, NULL, "(SELECT InsuredPartyT.*, PersonT.*, PrefixT.Prefix FROM (SELECT InsuredPartyT.*, InsPartyPayGroupsT.CopayPercentage as CopayPercent, InsPartyPayGroupsT.CopayMoney as Copay "
				"FROM InsuredPartyT LEFT JOIN (SELECT InsuredPartyID, CopayMoney, CopayPercentage FROM "
				"(SELECT * FROM ServicePayGroupsT WHERE Name = 'Copay') ServicePayGroupsT "
				" LEFT JOIN InsuredPartyPayGroupsT ON ServicePayGroupsT.ID = InsuredPartyPayGroupsT.PayGroupID) InsPartyPayGroupsT "
				" ON InsuredPartyT.PersonID = InsPartyPayGroupsT.InsuredPartyID "
				" LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				" WHERE RespTypeT.CategoryType = 3 AND RespTypeT.CategoryPlacement = 1) InsuredPartyT "
				" LEFT JOIN (PersonT WITH(NOLOCK) LEFT JOIN PrefixT ON PersonT.PrefixID = PrefixT.ID) ON InsuredPartyT.PersonID = PersonT.ID "
				" LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				" WHERE RespTypeT.CategoryType = 3 AND RespTypeT.CategoryPlacement = 1) InsuredPartyTAuto1 ON PersonT.ID = InsuredPartyTAuto1.PatientID"),
	ADD_FILTER_FIELD(624, "Auto (Primary) Middle Name", "InsuredPartyTAuto1.Middle", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 622),
	ADD_FILTER_FIELD(625, "Auto (Primary) Last Name", "InsuredPartyTAuto1.[Last]", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 622),
	ADD_FILTER_FIELD(626, "Auto (Primary) Title", "InsuredPartyTAuto1.Title", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 622),
	ADD_FILTER_FIELD(627, "Auto (Primary) Address1", "InsuredPartyTAuto1.Address1", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 622),
	ADD_FILTER_FIELD(628, "Auto (Primary) Address2", "InsuredPartyTAuto1.Address2", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 622),
	ADD_FILTER_FIELD(629, "Auto (Primary) City", "InsuredPartyTAuto1.City", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 622),
	ADD_FILTER_FIELD(630, "Auto (Primary) State", "InsuredPartyTAuto1.State", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 622),
	ADD_FILTER_FIELD(641, "Auto (Primary) Zipcode", "InsuredPartyTAuto1.Zip", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 622),
	ADD_FILTER_FIELD(642, "Auto (Primary) Phone", "(Replace(Replace(Replace(Replace(InsuredPartyTAuto1.HomePhone,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL, 622),
	ADD_FILTER_FIELD(643, "Auto (Primary) Ins Group Number", "InsuredPartyTAuto1.PolicyGroupNum", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 622),
	ADD_FILTER_FIELD(644, "Auto (Primary) Ins ID", "InsuredPartyTAuto1.IDForInsurance", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 622),
	ADD_FILTER_FIELD(645, "Auto (Primary) Effective Date", "dbo.AsDateNoTime(InsuredPartyTAuto1.EffectiveDate)", fboPerson, foAll, foEqual, ftDate, NULL, NULL, 622),	//see note at the top about converting date
	ADD_FILTER_FIELD(646, "Auto (Primary) CoPay Amount", "InsuredPartyTAuto1.CoPay", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 622),
	ADD_FILTER_FIELD(647, "Auto (Primary) CoPay Percent", "InsuredPartyTAuto1.CopayPercent", fboPerson, foEquality, foGreater, ftNumber, NULL, NULL, 622),
	ADD_FILTER_FIELD(648, "Auto (Primary) Employer", "InsuredPartyTAuto1.Employer", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 622),
	ADD_FILTER_FIELD(649, "Auto (Primary) Insurance", "InsuranceCoTAuto1.Name", fboPerson, foAll, foBeginsWith, ftText, NULL, "InsuranceCoT AS InsuranceCoTAuto1 ON InsuredPartyTAuto1.InsuranceCoID = InsuranceCoTAuto1.PersonID", 622),

	ADD_FILTER_FIELD(651, "Auto (Secondary) Prefix", "InsuredPartyTAuto2.Prefix", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 650),
	ADD_FILTER_FIELD(650, "Auto (Secondary) First Name", "InsuredPartyTAuto2.[First]", fboPerson, foAll, foBeginsWith, ftText, NULL, "(SELECT InsuredPartyT.*, PersonT.*, PrefixT.Prefix FROM (SELECT InsuredPartyT.*, InsPartyPayGroupsT.CopayPercentage as CopayPercent, InsPartyPayGroupsT.CopayMoney as Copay FROM InsuredPartyT LEFT JOIN (SELECT InsuredPartyID, CopayMoney, CopayPercentage FROM "
				"(SELECT * FROM ServicePayGroupsT WHERE Name = 'Copay') ServicePayGroupsT "
				" LEFT JOIN InsuredPartyPayGroupsT ON ServicePayGroupsT.ID = InsuredPartyPayGroupsT.PayGroupID) InsPartyPayGroupsT "
				" ON InsuredPartyT.PersonID = InsPartyPayGroupsT.InsuredPartyID "
				" LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				" WHERE RespTypeT.CategoryType = 3 AND RespTypeT.CategoryPlacement = 2) InsuredPartyT "
				" LEFT JOIN (PersonT WITH(NOLOCK) LEFT JOIN PrefixT ON PersonT.PrefixID = PrefixT.ID) ON InsuredPartyT.PersonID = PersonT.ID "
				" LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				" WHERE RespTypeT.CategoryType = 3 AND RespTypeT.CategoryPlacement = 2) InsuredPartyTAuto2 ON PersonT.ID = InsuredPartyTAuto2.PatientID"),
	ADD_FILTER_FIELD(652, "Auto (Secondary) Middle Name", "InsuredPartyTAuto2.Middle", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 650),
	ADD_FILTER_FIELD(653, "Auto (Secondary) Last Name", "InsuredPartyTAuto2.[Last]", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 650),
	ADD_FILTER_FIELD(654, "Auto (Secondary) Title", "InsuredPartyTAuto2.Title", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 650),
	ADD_FILTER_FIELD(655, "Auto (Secondary) Address1", "InsuredPartyTAuto2.Address1", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 650),
	ADD_FILTER_FIELD(656, "Auto (Secondary) Address2", "InsuredPartyTAuto2.Address2", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 650),
	ADD_FILTER_FIELD(657, "Auto (Secondary) City", "InsuredPartyTAuto2.City", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 650),
	ADD_FILTER_FIELD(658, "Auto (Secondary) State", "InsuredPartyTAuto2.State", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 650),
	ADD_FILTER_FIELD(659, "Auto (Secondary) Zipcode", "InsuredPartyTAuto2.Zip", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 650),
	ADD_FILTER_FIELD(660, "Auto (Secondary) Phone", "(Replace(Replace(Replace(Replace(InsuredPartyTAuto2.HomePhone,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL, 650),
	ADD_FILTER_FIELD(661, "Auto (Secondary) Ins Group Number", "InsuredPartyTAuto2.PolicyGroupNum", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 650),
	ADD_FILTER_FIELD(662, "Auto (Secondary) Ins ID", "InsuredPartyTAuto2.IDForInsurance", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 650),
	ADD_FILTER_FIELD(663, "Auto (Secondary) Effective Date", "dbo.AsDateNoTime(InsuredPartyTAuto2.EffectiveDate)", fboPerson, foAll, foEqual, ftDate, NULL, NULL, 650),	//see note at the top about converting date
	ADD_FILTER_FIELD(664, "Auto (Secondary) CoPay Amount", "InsuredPartyTAuto2.CoPay", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 650),
	ADD_FILTER_FIELD(665, "Auto (Secondary) CoPay Percent", "InsuredPartyTAuto2.CopayPercent", fboPerson, foEquality, foGreater, ftNumber, NULL, NULL, 650),
	ADD_FILTER_FIELD(666, "Auto (Secondary) Employer", "InsuredPartyTAuto2.Employer", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 650),
	ADD_FILTER_FIELD(667, "Auto (Secondary) Insurance", "InsuranceCoTAuto2.Name", fboPerson, foAll, foBeginsWith, ftText, NULL, "InsuranceCoT AS InsuranceCoTAuto2 ON InsuredPartyTAuto2.InsuranceCoID = InsuranceCoTAuto2.PersonID", 650),
	
	// (d.singleton 2012-09-14 13:36) - PLID 52657 added workers comp filter
	ADD_FILTER_FIELD(669, "Workers Comp (Primary) Prefix", "InsuredPartyTWorkersComp1.Prefix", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 668),
	ADD_FILTER_FIELD(668, "Workers Comp (Primary) First Name", "InsuredPartyTWorkersComp1.[First]", fboPerson, foAll, foBeginsWith, ftText, NULL, "(SELECT InsuredPartyT.*, PersonT.*, PrefixT.Prefix FROM (SELECT InsuredPartyT.*, InsPartyPayGroupsT.CopayPercentage as CopayPercent, InsPartyPayGroupsT.CopayMoney as Copay "
				"FROM InsuredPartyT LEFT JOIN (SELECT InsuredPartyID, CopayMoney, CopayPercentage FROM "
				"(SELECT * FROM ServicePayGroupsT WHERE Name = 'Copay') ServicePayGroupsT "
				" LEFT JOIN InsuredPartyPayGroupsT ON ServicePayGroupsT.ID = InsuredPartyPayGroupsT.PayGroupID) InsPartyPayGroupsT "
				" ON InsuredPartyT.PersonID = InsPartyPayGroupsT.InsuredPartyID "
				" LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				" WHERE RespTypeT.CategoryType = 4 AND RespTypeT.CategoryPlacement = 1) InsuredPartyT "
				" LEFT JOIN (PersonT WITH(NOLOCK) LEFT JOIN PrefixT ON PersonT.PrefixID = PrefixT.ID) ON InsuredPartyT.PersonID = PersonT.ID "
				" LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				" WHERE RespTypeT.CategoryType = 4 AND RespTypeT.CategoryPlacement = 1) InsuredPartyTWorkersComp1 ON PersonT.ID = InsuredPartyTWorkersComp1.PatientID"),
	ADD_FILTER_FIELD(670, "Workers Comp (Primary) Middle Name", "InsuredPartyTWorkersComp1.Middle", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 668),
	ADD_FILTER_FIELD(671, "Workers Comp (Primary) Last Name", "InsuredPartyTWorkersComp1.[Last]", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 668),
	ADD_FILTER_FIELD(672, "Workers Comp (Primary) Title", "InsuredPartyTWorkersComp1.Title", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 668),
	ADD_FILTER_FIELD(673, "Workers Comp (Primary) Address1", "InsuredPartyTWorkersComp1.Address1", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 668),
	ADD_FILTER_FIELD(674, "Workers Comp (Primary) Address2", "InsuredPartyTWorkersComp1.Address2", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 668),
	ADD_FILTER_FIELD(675, "Workers Comp (Primary) City", "InsuredPartyTWorkersComp1.City", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 668),
	ADD_FILTER_FIELD(676, "Workers Comp (Primary) State", "InsuredPartyTWorkersComp1.State", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 668),
	ADD_FILTER_FIELD(677, "Workers Comp (Primary) Zipcode", "InsuredPartyTWorkersComp1.Zip", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 668),
	ADD_FILTER_FIELD(678, "Workers Comp (Primary) Phone", "(Replace(Replace(Replace(Replace(InsuredPartyTWorkersComp1.HomePhone,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL, 668),
	ADD_FILTER_FIELD(679, "Workers Comp (Primary) Ins Group Number", "InsuredPartyTWorkersComp1.PolicyGroupNum", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 668),
	ADD_FILTER_FIELD(680, "Workers Comp (Primary) Ins ID", "InsuredPartyTWorkersComp1.IDForInsurance", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 668),
	ADD_FILTER_FIELD(681, "Workers Comp (Primary) Effective Date", "dbo.AsDateNoTime(InsuredPartyTWorkersComp1.EffectiveDate)", fboPerson, foAll, foEqual, ftDate, NULL, NULL, 668),	//see note at the top about converting date
	ADD_FILTER_FIELD(682, "Workers Comp (Primary) CoPay Amount", "InsuredPartyTWorkersComp1.CoPay", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 668),
	ADD_FILTER_FIELD(683, "Workers Comp (Primary) CoPay Percent", "InsuredPartyTWorkersComp1.CopayPercent", fboPerson, foEquality, foGreater, ftNumber, NULL, NULL, 668),
	ADD_FILTER_FIELD(684, "Workers Comp (Primary) Employer", "InsuredPartyTWorkersComp1.Employer", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 668),
	ADD_FILTER_FIELD(685, "Workers Comp (Primary) Insurance", "InsuranceCoTWorkersComp1.Name", fboPerson, foAll, foBeginsWith, ftText, NULL, "InsuranceCoT AS InsuranceCoTWorkersComp1 ON InsuredPartyTWorkersComp1.InsuranceCoID = InsuranceCoTWorkersComp1.PersonID", 668),

	ADD_FILTER_FIELD(687, "Workers Comp (Secondary) Prefix", "InsuredPartyTWorkersComp2.Prefix", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 686),
	ADD_FILTER_FIELD(686, "Workers Comp (Secondary) First Name", "InsuredPartyTWorkersComp2.[First]", fboPerson, foAll, foBeginsWith, ftText, NULL, "(SELECT InsuredPartyT.*, PersonT.*, PrefixT.Prefix FROM (SELECT InsuredPartyT.*, InsPartyPayGroupsT.CopayPercentage as CopayPercent, InsPartyPayGroupsT.CopayMoney as Copay FROM InsuredPartyT LEFT JOIN (SELECT InsuredPartyID, CopayMoney, CopayPercentage FROM "
				"(SELECT * FROM ServicePayGroupsT WHERE Name = 'Copay') ServicePayGroupsT "
				" LEFT JOIN InsuredPartyPayGroupsT ON ServicePayGroupsT.ID = InsuredPartyPayGroupsT.PayGroupID) InsPartyPayGroupsT "
				" ON InsuredPartyT.PersonID = InsPartyPayGroupsT.InsuredPartyID "
				" LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				" WHERE RespTypeT.CategoryType = 4 AND RespTypeT.CategoryPlacement = 2) InsuredPartyT "
				" LEFT JOIN (PersonT WITH(NOLOCK) LEFT JOIN PrefixT ON PersonT.PrefixID = PrefixT.ID) ON InsuredPartyT.PersonID = PersonT.ID "
				" LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				" WHERE RespTypeT.CategoryType = 4 AND RespTypeT.CategoryPlacement = 2) InsuredPartyTWorkersComp2 ON PersonT.ID = InsuredPartyTWorkersComp2.PatientID"),
	ADD_FILTER_FIELD(688, "Workers Comp (Secondary) Middle Name", "InsuredPartyTWorkersComp2.Middle", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 686),
	ADD_FILTER_FIELD(689, "Workers Comp (Secondary) Last Name", "InsuredPartyTWorkersComp2.[Last]", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 686),
	ADD_FILTER_FIELD(690, "Workers Comp (Secondary) Title", "InsuredPartyTWorkersComp2.Title", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 686),
	ADD_FILTER_FIELD(691, "Workers Comp (Secondary) Address1", "InsuredPartyTWorkersComp2.Address1", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 686),
	ADD_FILTER_FIELD(692, "Workers Comp (Secondary) Address2", "InsuredPartyTWorkersComp2.Address2", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 686),
	ADD_FILTER_FIELD(693, "Workers Comp (Secondary) City", "InsuredPartyTWorkersComp2.City", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 686),
	ADD_FILTER_FIELD(694, "Workers Comp (Secondary) State", "InsuredPartyTWorkersComp2.State", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 686),
	ADD_FILTER_FIELD(695, "Workers Comp (Secondary) Zipcode", "InsuredPartyTWorkersComp2.Zip", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 686),
	ADD_FILTER_FIELD(696, "Workers Comp (Secondary) Phone", "(Replace(Replace(Replace(Replace(InsuredPartyTWorkersComp2.HomePhone,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL, 686),
	ADD_FILTER_FIELD(697, "Workers Comp (Secondary) Ins Group Number", "InsuredPartyTWorkersComp2.PolicyGroupNum", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 686),
	ADD_FILTER_FIELD(698, "Workers Comp (Secondary) Ins ID", "InsuredPartyTWorkersComp2.IDForInsurance", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 686),
	ADD_FILTER_FIELD(699, "Workers Comp (Secondary) Effective Date", "dbo.AsDateNoTime(InsuredPartyTWorkersComp2.EffectiveDate)", fboPerson, foAll, foEqual, ftDate, NULL, NULL, 686),	//see note at the top about converting date
	ADD_FILTER_FIELD(700, "Workers Comp (Secondary) CoPay Amount", "InsuredPartyTWorkersComp2.CoPay", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 686),
	ADD_FILTER_FIELD(701, "Workers Comp (Secondary) CoPay Percent", "InsuredPartyTWorkersComp2.CopayPercent", fboPerson, foEquality, foGreater, ftNumber, NULL, NULL, 686),
	ADD_FILTER_FIELD(702, "Workers Comp (Secondary) Employer", "InsuredPartyTWorkersComp2.Employer", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 686),
	ADD_FILTER_FIELD(703, "Workers Comp (Secondary) Insurance", "InsuranceCoTWorkersComp2.Name", fboPerson, foAll, foBeginsWith, ftText, NULL, "InsuranceCoT AS InsuranceCoTWorkersComp2 ON InsuredPartyTWorkersComp2.InsuranceCoID = InsuranceCoTWorkersComp2.PersonID", 686),


	// (d.singleton 2012-09-14 13:37) - PLID 52658 added dental filter
	ADD_FILTER_FIELD(705, "Dental (Primary) Prefix", "InsuredPartyTDental1.Prefix", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 704),
	ADD_FILTER_FIELD(704, "Dental (Primary) First Name", "InsuredPartyTDental1.[First]", fboPerson, foAll, foBeginsWith, ftText, NULL, "(SELECT InsuredPartyT.*, PersonT.*, PrefixT.Prefix FROM (SELECT InsuredPartyT.*, InsPartyPayGroupsT.CopayPercentage as CopayPercent, InsPartyPayGroupsT.CopayMoney as Copay "
				"FROM InsuredPartyT LEFT JOIN (SELECT InsuredPartyID, CopayMoney, CopayPercentage FROM "
				"(SELECT * FROM ServicePayGroupsT WHERE Name = 'Copay') ServicePayGroupsT "
				" LEFT JOIN InsuredPartyPayGroupsT ON ServicePayGroupsT.ID = InsuredPartyPayGroupsT.PayGroupID) InsPartyPayGroupsT "
				" ON InsuredPartyT.PersonID = InsPartyPayGroupsT.InsuredPartyID "
				" LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				" WHERE RespTypeT.CategoryType = 5 AND RespTypeT.CategoryPlacement = 1) InsuredPartyT "
				" LEFT JOIN (PersonT WITH(NOLOCK) LEFT JOIN PrefixT ON PersonT.PrefixID = PrefixT.ID) ON InsuredPartyT.PersonID = PersonT.ID "
				" LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				" WHERE RespTypeT.CategoryType = 5 AND RespTypeT.CategoryPlacement = 1) InsuredPartyTDental1 ON PersonT.ID = InsuredPartyTDental1.PatientID"),
	ADD_FILTER_FIELD(706, "Dental (Primary) Middle Name", "InsuredPartyTDental1.Middle", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 704),
	ADD_FILTER_FIELD(707, "Dental (Primary) Last Name", "InsuredPartyTDental1.[Last]", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 704),
	ADD_FILTER_FIELD(708, "Dental (Primary) Title", "InsuredPartyTDental1.Title", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 704),
	ADD_FILTER_FIELD(709, "Dental (Primary) Address1", "InsuredPartyTDental1.Address1", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 704),
	ADD_FILTER_FIELD(710, "Dental (Primary) Address2", "InsuredPartyTDental1.Address2", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 704),
	ADD_FILTER_FIELD(711, "Dental (Primary) City", "InsuredPartyTDental1.City", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 704),
	ADD_FILTER_FIELD(712, "Dental (Primary) State", "InsuredPartyTDental1.State", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 704),
	ADD_FILTER_FIELD(713, "Dental (Primary) Zipcode", "InsuredPartyTDental1.Zip", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 704),
	ADD_FILTER_FIELD(714, "Dental (Primary) Phone", "(Replace(Replace(Replace(Replace(InsuredPartyTDental1.HomePhone,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL, 704),
	ADD_FILTER_FIELD(715, "Dental (Primary) Ins Group Number", "InsuredPartyTDental1.PolicyGroupNum", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 704),
	ADD_FILTER_FIELD(716, "Dental (Primary) Ins ID", "InsuredPartyTDental1.IDForInsurance", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 704),
	ADD_FILTER_FIELD(717, "Dental (Primary) Effective Date", "dbo.AsDateNoTime(InsuredPartyTDental1.EffectiveDate)", fboPerson, foAll, foEqual, ftDate, NULL, NULL, 704),	//see note at the top about converting date
	ADD_FILTER_FIELD(718, "Dental (Primary) CoPay Amount", "InsuredPartyTDental1.CoPay", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 704),
	ADD_FILTER_FIELD(719, "Dental (Primary) CoPay Percent", "InsuredPartyTDental1.CopayPercent", fboPerson, foEquality, foGreater, ftNumber, NULL, NULL, 704),
	ADD_FILTER_FIELD(720, "Dental (Primary) Employer", "InsuredPartyTDental1.Employer", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 704),
	ADD_FILTER_FIELD(721, "Dental (Primary) Insurance", "InsuranceCoTDental1.Name", fboPerson, foAll, foBeginsWith, ftText, NULL, "InsuranceCoT AS InsuranceCoTDental1 ON InsuredPartyTDental1.InsuranceCoID = InsuranceCoTDental1.PersonID", 704),

	ADD_FILTER_FIELD(723, "Dental (Secondary) Prefix", "InsuredPartyTDental2.Prefix", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 722),
	ADD_FILTER_FIELD(722, "Dental (Secondary) First Name", "InsuredPartyTDental2.[First]", fboPerson, foAll, foBeginsWith, ftText, NULL, "(SELECT InsuredPartyT.*, PersonT.*, PrefixT.Prefix FROM (SELECT InsuredPartyT.*, InsPartyPayGroupsT.CopayPercentage as CopayPercent, InsPartyPayGroupsT.CopayMoney as Copay FROM InsuredPartyT LEFT JOIN (SELECT InsuredPartyID, CopayMoney, CopayPercentage FROM "
				"(SELECT * FROM ServicePayGroupsT WHERE Name = 'Copay') ServicePayGroupsT "
				" LEFT JOIN InsuredPartyPayGroupsT ON ServicePayGroupsT.ID = InsuredPartyPayGroupsT.PayGroupID) InsPartyPayGroupsT "
				" ON InsuredPartyT.PersonID = InsPartyPayGroupsT.InsuredPartyID "
				" LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				" WHERE RespTypeT.CategoryType = 5 AND RespTypeT.CategoryPlacement = 2) InsuredPartyT "
				" LEFT JOIN (PersonT WITH(NOLOCK) LEFT JOIN PrefixT ON PersonT.PrefixID = PrefixT.ID) ON InsuredPartyT.PersonID = PersonT.ID "
				" LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				" WHERE RespTypeT.CategoryType = 5 AND RespTypeT.CategoryPlacement = 2) InsuredPartyTDental2 ON PersonT.ID = InsuredPartyTDental2.PatientID"),
	ADD_FILTER_FIELD(990, "Dental (Secondary) Middle Name", "InsuredPartyTDental2.Middle", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 722),
	ADD_FILTER_FIELD(724, "Dental (Secondary) Last Name", "InsuredPartyTDental2.[Last]", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 722),
	ADD_FILTER_FIELD(725, "Dental (Secondary) Title", "InsuredPartyTDental2.Title", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 722),
	ADD_FILTER_FIELD(726, "Dental (Secondary) Address1", "InsuredPartyTDental2.Address1", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 722),
	ADD_FILTER_FIELD(727, "Dental (Secondary) Address2", "InsuredPartyTDental2.Address2", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 722),
	ADD_FILTER_FIELD(728, "Dental (Secondary) City", "InsuredPartyTDental2.City", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 722),
	ADD_FILTER_FIELD(729, "Dental (Secondary) State", "InsuredPartyTDental2.State", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 722),
	ADD_FILTER_FIELD(730, "Dental (Secondary) Zipcode", "InsuredPartyTDental2.Zip", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 722),
	ADD_FILTER_FIELD(731, "Dental (Secondary) Phone", "(Replace(Replace(Replace(Replace(InsuredPartyTDental2.HomePhone,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL, 722),
	ADD_FILTER_FIELD(732, "Dental (Secondary) Ins Group Number", "InsuredPartyTDental2.PolicyGroupNum", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 722),
	ADD_FILTER_FIELD(733, "Dental (Secondary) Ins ID", "InsuredPartyTDental2.IDForInsurance", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 722),
	ADD_FILTER_FIELD(734, "Dental (Secondary) Effective Date", "dbo.AsDateNoTime(InsuredPartyTDental2.EffectiveDate)", fboPerson, foAll, foEqual, ftDate, NULL, NULL, 722),	//see note at the top about converting date
	ADD_FILTER_FIELD(735, "Dental (Secondary) CoPay Amount", "InsuredPartyTDental2.CoPay", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 722),
	ADD_FILTER_FIELD(736, "Dental (Secondary) CoPay Percent", "InsuredPartyTDental2.CopayPercent", fboPerson, foEquality, foGreater, ftNumber, NULL, NULL, 722),
	ADD_FILTER_FIELD(737, "Dental (Secondary) Employer", "InsuredPartyTDental2.Employer", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 722),
	ADD_FILTER_FIELD(738, "Dental (Secondary) Insurance", "InsuranceCoTDental2.Name", fboPerson, foAll, foBeginsWith, ftText, NULL, "InsuranceCoT AS InsuranceCoTDental2 ON InsuredPartyTDental2.InsuranceCoID = InsuranceCoTDental2.PersonID", 722),

	// (d.singleton 2012-09-14 13:37) - PLID 52659 added study filter
	ADD_FILTER_FIELD(740, "Study (Primary) Prefix", "InsuredPartyTStudy1.Prefix", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 739),
	ADD_FILTER_FIELD(739, "Study (Primary) First Name", "InsuredPartyTStudy1.[First]", fboPerson, foAll, foBeginsWith, ftText, NULL, "(SELECT InsuredPartyT.*, PersonT.*, PrefixT.Prefix FROM (SELECT InsuredPartyT.*, InsPartyPayGroupsT.CopayPercentage as CopayPercent, InsPartyPayGroupsT.CopayMoney as Copay "
				"FROM InsuredPartyT LEFT JOIN (SELECT InsuredPartyID, CopayMoney, CopayPercentage FROM "
				"(SELECT * FROM ServicePayGroupsT WHERE Name = 'Copay') ServicePayGroupsT "
				" LEFT JOIN InsuredPartyPayGroupsT ON ServicePayGroupsT.ID = InsuredPartyPayGroupsT.PayGroupID) InsPartyPayGroupsT "
				" ON InsuredPartyT.PersonID = InsPartyPayGroupsT.InsuredPartyID "
				" LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				" WHERE RespTypeT.CategoryType = 6 AND RespTypeT.CategoryPlacement = 1) InsuredPartyT "
				" LEFT JOIN (PersonT WITH(NOLOCK) LEFT JOIN PrefixT ON PersonT.PrefixID = PrefixT.ID) ON InsuredPartyT.PersonID = PersonT.ID "
				" LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				" WHERE RespTypeT.CategoryType = 6 AND RespTypeT.CategoryPlacement = 1) InsuredPartyTStudy1 ON PersonT.ID = InsuredPartyTStudy1.PatientID"),
	ADD_FILTER_FIELD(741, "Study (Primary) Middle Name", "InsuredPartyTStudy1.Middle", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 739),
	ADD_FILTER_FIELD(742, "Study (Primary) Last Name", "InsuredPartyTStudy1.[Last]", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 739),
	ADD_FILTER_FIELD(743, "Study (Primary) Title", "InsuredPartyTStudy1.Title", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 739),
	ADD_FILTER_FIELD(744, "Study (Primary) Address1", "InsuredPartyTStudy1.Address1", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 739),
	ADD_FILTER_FIELD(745, "Study (Primary) Address2", "InsuredPartyTStudy1.Address2", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 739),
	ADD_FILTER_FIELD(746, "Study (Primary) City", "InsuredPartyTStudy1.City", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 739),
	ADD_FILTER_FIELD(747, "Study (Primary) State", "InsuredPartyTStudy1.State", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 739),
	ADD_FILTER_FIELD(748, "Study (Primary) Zipcode", "InsuredPartyTStudy1.Zip", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 739),
	ADD_FILTER_FIELD(749, "Study (Primary) Phone", "(Replace(Replace(Replace(Replace(InsuredPartyTStudy1.HomePhone,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL, 739),
	ADD_FILTER_FIELD(750, "Study (Primary) Ins Group Number", "InsuredPartyTStudy1.PolicyGroupNum", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 739),
	ADD_FILTER_FIELD(751, "Study (Primary) Ins ID", "InsuredPartyTStudy1.IDForInsurance", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 739),
	ADD_FILTER_FIELD(752, "Study (Primary) Effective Date", "dbo.AsDateNoTime(InsuredPartyTStudy1.EffectiveDate)", fboPerson, foAll, foEqual, ftDate, NULL, NULL, 739),	//see note at the top about converting date
	ADD_FILTER_FIELD(753, "Study (Primary) CoPay Amount", "InsuredPartyTStudy1.CoPay", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 739),
	ADD_FILTER_FIELD(754, "Study (Primary) CoPay Percent", "InsuredPartyTStudy1.CopayPercent", fboPerson, foEquality, foGreater, ftNumber, NULL, NULL, 739),
	ADD_FILTER_FIELD(755, "Study (Primary) Employer", "InsuredPartyTStudy1.Employer", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 739),
	ADD_FILTER_FIELD(756, "Study (Primary) Insurance", "InsuranceCoTStudy1.Name", fboPerson, foAll, foBeginsWith, ftText, NULL, "InsuranceCoT AS InsuranceCoTStudy1 ON InsuredPartyTStudy1.InsuranceCoID = InsuranceCoTStudy1.PersonID", 739),

	ADD_FILTER_FIELD(758, "Study (Secondary) Prefix", "InsuredPartyTStudy2.Prefix", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 757),
	ADD_FILTER_FIELD(757, "Study (Secondary) First Name", "InsuredPartyTStudy2.[First]", fboPerson, foAll, foBeginsWith, ftText, NULL, "(SELECT InsuredPartyT.*, PersonT.*, PrefixT.Prefix FROM (SELECT InsuredPartyT.*, InsPartyPayGroupsT.CopayPercentage as CopayPercent, InsPartyPayGroupsT.CopayMoney as Copay FROM InsuredPartyT LEFT JOIN (SELECT InsuredPartyID, CopayMoney, CopayPercentage FROM "
				"(SELECT * FROM ServicePayGroupsT WHERE Name = 'Copay') ServicePayGroupsT "
				" LEFT JOIN InsuredPartyPayGroupsT ON ServicePayGroupsT.ID = InsuredPartyPayGroupsT.PayGroupID) InsPartyPayGroupsT "
				" ON InsuredPartyT.PersonID = InsPartyPayGroupsT.InsuredPartyID "
				" LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				" WHERE RespTypeT.CategoryType = 6 AND RespTypeT.CategoryPlacement = 2) InsuredPartyT "
				" LEFT JOIN (PersonT WITH(NOLOCK) LEFT JOIN PrefixT ON PersonT.PrefixID = PrefixT.ID) ON InsuredPartyT.PersonID = PersonT.ID "
				" LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				" WHERE RespTypeT.CategoryType = 6 AND RespTypeT.CategoryPlacement = 2) InsuredPartyTStudy2 ON PersonT.ID = InsuredPartyTStudy2.PatientID"),
	ADD_FILTER_FIELD(991, "Study (Secondary) Middle Name", "InsuredPartyTStudy2.Middle", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 757),
	ADD_FILTER_FIELD(759, "Study (Secondary) Last Name", "InsuredPartyTStudy2.[Last]", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 757),
	ADD_FILTER_FIELD(760, "Study (Secondary) Title", "InsuredPartyTStudy2.Title", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 757),
	ADD_FILTER_FIELD(761, "Study (Secondary) Address1", "InsuredPartyTStudy2.Address1", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 757),
	ADD_FILTER_FIELD(762, "Study (Secondary) Address2", "InsuredPartyTStudy2.Address2", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 757),
	ADD_FILTER_FIELD(763, "Study (Secondary) City", "InsuredPartyTStudy2.City", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 757),
	ADD_FILTER_FIELD(764, "Study (Secondary) State", "InsuredPartyTStudy2.State", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 757),
	ADD_FILTER_FIELD(765, "Study (Secondary) Zipcode", "InsuredPartyTStudy2.Zip", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 757),
	ADD_FILTER_FIELD(766, "Study (Secondary) Phone", "(Replace(Replace(Replace(Replace(InsuredPartyTStudy2.HomePhone,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL, 757),
	ADD_FILTER_FIELD(767, "Study (Secondary) Ins Group Number", "InsuredPartyTStudy2.PolicyGroupNum", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 757),
	ADD_FILTER_FIELD(768, "Study (Secondary) Ins ID", "InsuredPartyTStudy2.IDForInsurance", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 757),
	ADD_FILTER_FIELD(769, "Study (Secondary) Effective Date", "dbo.AsDateNoTime(InsuredPartyTStudy2.EffectiveDate)", fboPerson, foAll, foEqual, ftDate, NULL, NULL, 757),	//see note at the top about converting date
	ADD_FILTER_FIELD(770, "Study (Secondary) CoPay Amount", "InsuredPartyTStudy2.CoPay", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 757),
	ADD_FILTER_FIELD(771, "Study (Secondary) CoPay Percent", "InsuredPartyTStudy2.CopayPercent", fboPerson, foEquality, foGreater, ftNumber, NULL, NULL, 757),
	ADD_FILTER_FIELD(772, "Study (Secondary) Employer", "InsuredPartyTStudy2.Employer", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 757),
	ADD_FILTER_FIELD(773, "Study (Secondary) Insurance", "InsuranceCoTStudy2.Name", fboPerson, foAll, foBeginsWith, ftText, NULL, "InsuranceCoT AS InsuranceCoTStudy2 ON InsuredPartyTStudy2.InsuranceCoID = InsuranceCoTStudy2.PersonID", 757),

	// (d.singleton 2012-09-14 13:37) - PLID 52660 added LOP filter
	ADD_FILTER_FIELD(775, "LOP (Primary) Prefix", "InsuredPartyTLOP1.Prefix", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 774),
	ADD_FILTER_FIELD(774, "LOP (Primary) First Name", "InsuredPartyTLOP1.[First]", fboPerson, foAll, foBeginsWith, ftText, NULL, "(SELECT InsuredPartyT.*, PersonT.*, PrefixT.Prefix FROM (SELECT InsuredPartyT.*, InsPartyPayGroupsT.CopayPercentage as CopayPercent, InsPartyPayGroupsT.CopayMoney as Copay "
				"FROM InsuredPartyT LEFT JOIN (SELECT InsuredPartyID, CopayMoney, CopayPercentage FROM "
				"(SELECT * FROM ServicePayGroupsT WHERE Name = 'Copay') ServicePayGroupsT "
				" LEFT JOIN InsuredPartyPayGroupsT ON ServicePayGroupsT.ID = InsuredPartyPayGroupsT.PayGroupID) InsPartyPayGroupsT "
				" ON InsuredPartyT.PersonID = InsPartyPayGroupsT.InsuredPartyID "
				" LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				" WHERE RespTypeT.CategoryType = 7 AND RespTypeT.CategoryPlacement = 1) InsuredPartyT "
				" LEFT JOIN (PersonT WITH(NOLOCK) LEFT JOIN PrefixT ON PersonT.PrefixID = PrefixT.ID) ON InsuredPartyT.PersonID = PersonT.ID "
				" LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				" WHERE RespTypeT.CategoryType = 7 AND RespTypeT.CategoryPlacement = 1) InsuredPartyTLOP1 ON PersonT.ID = InsuredPartyTLOP1.PatientID"),
	ADD_FILTER_FIELD(776, "LOP (Primary) Middle Name", "InsuredPartyTLOP1.Middle", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 774),
	ADD_FILTER_FIELD(777, "LOP (Primary) Last Name", "InsuredPartyTLOP1.[Last]", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 774),
	ADD_FILTER_FIELD(778, "LOP (Primary) Title", "InsuredPartyTLOP1.Title", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 774),
	ADD_FILTER_FIELD(779, "LOP (Primary) Address1", "InsuredPartyTLOP1.Address1", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 774),
	ADD_FILTER_FIELD(780, "LOP (Primary) Address2", "InsuredPartyTLOP1.Address2", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 774),
	ADD_FILTER_FIELD(781, "LOP (Primary) City", "InsuredPartyTLOP1.City", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 774),
	ADD_FILTER_FIELD(782, "LOP (Primary) State", "InsuredPartyTLOP1.State", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 774),
	ADD_FILTER_FIELD(783, "LOP (Primary) Zipcode", "InsuredPartyTLOP1.Zip", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 774),
	ADD_FILTER_FIELD(784, "LOP (Primary) Phone", "(Replace(Replace(Replace(Replace(InsuredPartyTLOP1.HomePhone,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL, 774),
	ADD_FILTER_FIELD(785, "LOP (Primary) Ins Group Number", "InsuredPartyTLOP1.PolicyGroupNum", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 774),
	ADD_FILTER_FIELD(786, "LOP (Primary) Ins ID", "InsuredPartyTLOP1.IDForInsurance", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 774),
	ADD_FILTER_FIELD(787, "LOP (Primary) Effective Date", "dbo.AsDateNoTime(InsuredPartyTLOP1.EffectiveDate)", fboPerson, foAll, foEqual, ftDate, NULL, NULL, 774),	//see note at the top about converting date
	ADD_FILTER_FIELD(788, "LOP (Primary) CoPay Amount", "InsuredPartyTLOP1.CoPay", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 774),
	ADD_FILTER_FIELD(789, "LOP (Primary) CoPay Percent", "InsuredPartyTLOP1.CopayPercent", fboPerson, foEquality, foGreater, ftNumber, NULL, NULL, 774),
	ADD_FILTER_FIELD(790, "LOP (Primary) Employer", "InsuredPartyTLOP1.Employer", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 774),
	ADD_FILTER_FIELD(791, "LOP (Primary) Insurance", "InsuranceCoTLOP1.Name", fboPerson, foAll, foBeginsWith, ftText, NULL, "InsuranceCoT AS InsuranceCoTLOP1 ON InsuredPartyTLOP1.InsuranceCoID = InsuranceCoTLOP1.PersonID", 774),

	ADD_FILTER_FIELD(793, "LOP (Secondary) Prefix", "InsuredPartyTLOP2.Prefix", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 792),
	ADD_FILTER_FIELD(792, "LOP (Secondary) First Name", "InsuredPartyTLOP2.[First]", fboPerson, foAll, foBeginsWith, ftText, NULL, "(SELECT InsuredPartyT.*, PersonT.*, PrefixT.Prefix FROM (SELECT InsuredPartyT.*, InsPartyPayGroupsT.CopayPercentage as CopayPercent, InsPartyPayGroupsT.CopayMoney as Copay FROM InsuredPartyT LEFT JOIN (SELECT InsuredPartyID, CopayMoney, CopayPercentage FROM "
				"(SELECT * FROM ServicePayGroupsT WHERE Name = 'Copay') ServicePayGroupsT "
				" LEFT JOIN InsuredPartyPayGroupsT ON ServicePayGroupsT.ID = InsuredPartyPayGroupsT.PayGroupID) InsPartyPayGroupsT "
				" ON InsuredPartyT.PersonID = InsPartyPayGroupsT.InsuredPartyID "
				" LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				" WHERE RespTypeT.CategoryType = 7 AND RespTypeT.CategoryPlacement = 2) InsuredPartyT "
				" LEFT JOIN (PersonT WITH(NOLOCK) LEFT JOIN PrefixT ON PersonT.PrefixID = PrefixT.ID) ON InsuredPartyT.PersonID = PersonT.ID "
				" LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				" WHERE RespTypeT.CategoryType = 7 AND RespTypeT.CategoryPlacement = 2) InsuredPartyTLOP2 ON PersonT.ID = InsuredPartyTLOP2.PatientID"),
	ADD_FILTER_FIELD(794, "LOP (Secondary) Middle Name", "InsuredPartyTLOP2.Middle", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 792),
	ADD_FILTER_FIELD(795, "LOP (Secondary) Last Name", "InsuredPartyTLOP2.[Last]", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 792),
	ADD_FILTER_FIELD(796, "LOP (Secondary) Title", "InsuredPartyTLOP2.Title", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 792),
	ADD_FILTER_FIELD(797, "LOP (Secondary) Address1", "InsuredPartyTLOP2.Address1", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 792),
	ADD_FILTER_FIELD(798, "LOP (Secondary) Address2", "InsuredPartyTLOP2.Address2", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 792),
	ADD_FILTER_FIELD(799, "LOP (Secondary) City", "InsuredPartyTLOP2.City", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 792),
	ADD_FILTER_FIELD(800, "LOP (Secondary) State", "InsuredPartyTLOP2.State", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 792),
	ADD_FILTER_FIELD(801, "LOP (Secondary) Zipcode", "InsuredPartyTLOP2.Zip", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 792),
	ADD_FILTER_FIELD(802, "LOP (Secondary) Phone", "(Replace(Replace(Replace(Replace(InsuredPartyTLOP2.HomePhone,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL, 792),
	ADD_FILTER_FIELD(803, "LOP (Secondary) Ins Group Number", "InsuredPartyTLOP2.PolicyGroupNum", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 792),
	ADD_FILTER_FIELD(804, "LOP (Secondary) Ins ID", "InsuredPartyTLOP2.IDForInsurance", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 792),
	ADD_FILTER_FIELD(805, "LOP (Secondary) Effective Date", "dbo.AsDateNoTime(InsuredPartyTLOP2.EffectiveDate)", fboPerson, foAll, foEqual, ftDate, NULL, NULL, 792),	//see note at the top about converting date
	ADD_FILTER_FIELD(806, "LOP (Secondary) CoPay Amount", "InsuredPartyTLOP2.CoPay", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 792),
	ADD_FILTER_FIELD(807, "LOP (Secondary) CoPay Percent", "InsuredPartyTLOP2.CopayPercent", fboPerson, foEquality, foGreater, ftNumber, NULL, NULL, 792),
	ADD_FILTER_FIELD(808, "LOP (Secondary) Employer", "InsuredPartyTLOP2.Employer", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 792),
	ADD_FILTER_FIELD(809, "LOP (Secondary) Insurance", "InsuranceCoTLOP2.Name", fboPerson, foAll, foBeginsWith, ftText, NULL, "InsuranceCoT AS InsuranceCoTLOP2 ON InsuredPartyTLOP2.InsuranceCoID = InsuranceCoTLOP2.PersonID", 792),

	// (d.singleton 2012-09-14 13:37) - PLID 52661 added LOA filter
	ADD_FILTER_FIELD(811, "LOA (Primary) Prefix", "InsuredPartyTLOA1.Prefix", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 810),
	ADD_FILTER_FIELD(810, "LOA (Primary) First Name", "InsuredPartyTLOA1.[First]", fboPerson, foAll, foBeginsWith, ftText, NULL, "(SELECT InsuredPartyT.*, PersonT.*, PrefixT.Prefix FROM (SELECT InsuredPartyT.*, InsPartyPayGroupsT.CopayPercentage as CopayPercent, InsPartyPayGroupsT.CopayMoney as Copay "
				"FROM InsuredPartyT LEFT JOIN (SELECT InsuredPartyID, CopayMoney, CopayPercentage FROM "
				"(SELECT * FROM ServicePayGroupsT WHERE Name = 'Copay') ServicePayGroupsT "
				" LEFT JOIN InsuredPartyPayGroupsT ON ServicePayGroupsT.ID = InsuredPartyPayGroupsT.PayGroupID) InsPartyPayGroupsT "
				" ON InsuredPartyT.PersonID = InsPartyPayGroupsT.InsuredPartyID "
				" LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				" WHERE RespTypeT.CategoryType = 8 AND RespTypeT.CategoryPlacement = 1) InsuredPartyT "
				" LEFT JOIN (PersonT WITH(NOLOCK) LEFT JOIN PrefixT ON PersonT.PrefixID = PrefixT.ID) ON InsuredPartyT.PersonID = PersonT.ID "
				" LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				" WHERE RespTypeT.CategoryType = 8 AND RespTypeT.CategoryPlacement = 1) InsuredPartyTLOA1 ON PersonT.ID = InsuredPartyTLOA1.PatientID"),
	ADD_FILTER_FIELD(812, "LOA (Primary) Middle Name", "InsuredPartyTLOA1.Middle", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 810),
	ADD_FILTER_FIELD(813, "LOA (Primary) Last Name", "InsuredPartyTLOA1.[Last]", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 810),
	ADD_FILTER_FIELD(814, "LOA (Primary) Title", "InsuredPartyTLOA1.Title", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 810),
	ADD_FILTER_FIELD(815, "LOA (Primary) Address1", "InsuredPartyTLOA1.Address1", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 810),
	ADD_FILTER_FIELD(816, "LOA (Primary) Address2", "InsuredPartyTLOA1.Address2", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 810),
	ADD_FILTER_FIELD(817, "LOA (Primary) City", "InsuredPartyTLOA1.City", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 810),
	ADD_FILTER_FIELD(818, "LOA (Primary) State", "InsuredPartyTLOA1.State", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 810),
	ADD_FILTER_FIELD(819, "LOA (Primary) Zipcode", "InsuredPartyTLOA1.Zip", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 810),
	ADD_FILTER_FIELD(820, "LOA (Primary) Phone", "(Replace(Replace(Replace(Replace(InsuredPartyTLOA1.HomePhone,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL, 810),
	ADD_FILTER_FIELD(821, "LOA (Primary) Ins Group Number", "InsuredPartyTLOA1.PolicyGroupNum", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 810),
	ADD_FILTER_FIELD(822, "LOA (Primary) Ins ID", "InsuredPartyTLOA1.IDForInsurance", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 810),
	ADD_FILTER_FIELD(823, "LOA (Primary) Effective Date", "dbo.AsDateNoTime(InsuredPartyTLOA1.EffectiveDate)", fboPerson, foAll, foEqual, ftDate, NULL, NULL, 810),	//see note at the top about converting date
	ADD_FILTER_FIELD(824, "LOA (Primary) CoPay Amount", "InsuredPartyTLOA1.CoPay", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 810),
	ADD_FILTER_FIELD(825, "LOA (Primary) CoPay Percent", "InsuredPartyTLOA1.CopayPercent", fboPerson, foEquality, foGreater, ftNumber, NULL, NULL, 810),
	ADD_FILTER_FIELD(826, "LOA (Primary) Employer", "InsuredPartyTLOA1.Employer", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 810),
	ADD_FILTER_FIELD(827, "LOA (Primary) Insurance", "InsuranceCoTLOA1.Name", fboPerson, foAll, foBeginsWith, ftText, NULL, "InsuranceCoT AS InsuranceCoTLOA1 ON InsuredPartyTLOA1.InsuranceCoID = InsuranceCoTLOA1.PersonID", 810),

	ADD_FILTER_FIELD(829, "LOA (Secondary) Prefix", "InsuredPartyTLOA2.Prefix", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 828),
	ADD_FILTER_FIELD(828, "LOA (Secondary) First Name", "InsuredPartyTLOA2.[First]", fboPerson, foAll, foBeginsWith, ftText, NULL, "(SELECT InsuredPartyT.*, PersonT.*, PrefixT.Prefix FROM (SELECT InsuredPartyT.*, InsPartyPayGroupsT.CopayPercentage as CopayPercent, InsPartyPayGroupsT.CopayMoney as Copay FROM InsuredPartyT LEFT JOIN (SELECT InsuredPartyID, CopayMoney, CopayPercentage FROM "
				"(SELECT * FROM ServicePayGroupsT WHERE Name = 'Copay') ServicePayGroupsT "
				" LEFT JOIN InsuredPartyPayGroupsT ON ServicePayGroupsT.ID = InsuredPartyPayGroupsT.PayGroupID) InsPartyPayGroupsT "
				" ON InsuredPartyT.PersonID = InsPartyPayGroupsT.InsuredPartyID "
				" LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				" WHERE RespTypeT.CategoryType = 8 AND RespTypeT.CategoryPlacement = 2) InsuredPartyT "
				" LEFT JOIN (PersonT WITH(NOLOCK) LEFT JOIN PrefixT ON PersonT.PrefixID = PrefixT.ID) ON InsuredPartyT.PersonID = PersonT.ID "
				" LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				" WHERE RespTypeT.CategoryType = 8 AND RespTypeT.CategoryPlacement = 2) InsuredPartyTLOA2 ON PersonT.ID = InsuredPartyTLOA2.PatientID"),
	ADD_FILTER_FIELD(830, "LOA (Secondary) Middle Name", "InsuredPartyTLOA2.Middle", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 828),
	ADD_FILTER_FIELD(831, "LOA (Secondary) Last Name", "InsuredPartyTLOA2.[Last]", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 828),
	ADD_FILTER_FIELD(832, "LOA (Secondary) Title", "InsuredPartyTLOA2.Title", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 828),
	ADD_FILTER_FIELD(833, "LOA (Secondary) Address1", "InsuredPartyTLOA2.Address1", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 828),
	ADD_FILTER_FIELD(834, "LOA (Secondary) Address2", "InsuredPartyTLOA2.Address2", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 828),
	ADD_FILTER_FIELD(835, "LOA (Secondary) City", "InsuredPartyTLOA2.City", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 828),
	ADD_FILTER_FIELD(836, "LOA (Secondary) State", "InsuredPartyTLOA2.State", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 828),
	ADD_FILTER_FIELD(837, "LOA (Secondary) Zipcode", "InsuredPartyTLOA2.Zip", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 828),
	ADD_FILTER_FIELD(838, "LOA (Secondary) Phone", "(Replace(Replace(Replace(Replace(InsuredPartyTLOA2.HomePhone,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL, 828),
	ADD_FILTER_FIELD(839, "LOA (Secondary) Ins Group Number", "InsuredPartyTLOA2.PolicyGroupNum", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 828),
	ADD_FILTER_FIELD(840, "LOA (Secondary) Ins ID", "InsuredPartyTLOA2.IDForInsurance", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 828),
	ADD_FILTER_FIELD(841, "LOA (Secondary) Effective Date", "dbo.AsDateNoTime(InsuredPartyTLOA2.EffectiveDate)", fboPerson, foAll, foEqual, ftDate, NULL, NULL, 828),	//see note at the top about converting date
	ADD_FILTER_FIELD(842, "LOA (Secondary) CoPay Amount", "InsuredPartyTLOA2.CoPay", fboPerson, foEquality, foGreater, ftCurrency, NULL, NULL, 828),
	ADD_FILTER_FIELD(843, "LOA (Secondary) CoPay Percent", "InsuredPartyTLOA2.CopayPercent", fboPerson, foEquality, foGreater, ftNumber, NULL, NULL, 828),
	ADD_FILTER_FIELD(844, "LOA (Secondary) Employer", "InsuredPartyTLOA2.Employer", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 828),
	ADD_FILTER_FIELD(845, "LOA (Secondary) Insurance", "InsuranceCoTLOA2.Name", fboPerson, foAll, foBeginsWith, ftText, NULL, "InsuranceCoT AS InsuranceCoTLOA2 ON InsuredPartyTLOA2.InsuranceCoID = InsuranceCoTLOA2.PersonID", 828),

	ADD_FILTER_FIELD(160, "Primary Ins Co", "InsuranceCoT1.PersonID", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT InsuranceCoT.Name AS Name, InsuranceCoT.PersonID AS ID FROM InsuranceCoT ORDER BY InsuranceCoT.Name"), NULL, 129, NULL, NULL, NULL, NULL, 0, P("InsuranceCoT.PersonID") P("InsuranceCoT.Name") P("InsuranceCoT")),
	ADD_FILTER_FIELD(142, "Primary Ins Co Name", "InsuranceCoT1.Name", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 129),
	ADD_FILTER_FIELD(143, "Primary Ins Co Address1", "InsuranceCoPersonT1.Address1", fboPerson, foAll, foBeginsWith, ftText, NULL, "PersonT AS InsuranceCoPersonT1 WITH(NOLOCK) ON InsuranceCoT1.PersonID = InsuranceCoPersonT1.ID", 129),
	ADD_FILTER_FIELD(144, "Primary Ins Co Address2", "InsuranceCoPersonT1.Address2", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 143),
	ADD_FILTER_FIELD(145, "Primary Ins Co City", "InsuranceCoPersonT1.City", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 143),
	ADD_FILTER_FIELD(146, "Primary Ins Co State", "InsuranceCoPersonT1.State", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 143),
	ADD_FILTER_FIELD(147, "Primary Ins Co Zipcode", "InsuranceCoPersonT1.Zip", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 143),
	ADD_FILTER_FIELD(253, "Primary Ins Co First Name", "InsuranceContactsPersonT1.First", fboPerson, foAll, foBeginsWith, ftText, NULL, "PersonT AS InsuranceContactsPersonT1 WITH(NOLOCK) ON InsuredPartyT1.InsuranceContactID = InsuranceContactsPersonT1.ID", 118),
	ADD_FILTER_FIELD(254, "Primary Ins Co Last Name", "InsuranceContactsPersonT1.Last", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 253),
	ADD_FILTER_FIELD(148, "Primary Ins Co Phone", "(Replace(Replace(Replace(Replace(InsuranceContactsPersonT1.WorkPhone,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL, 253),
	ADD_FILTER_FIELD(255, "Primary Ins Co Fax", "(Replace(Replace(Replace(Replace(InsuranceContactsPersonT1.Fax,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL, 253),
	//(e.lally 2010-05-05) PLID 35792 - Added Financial Class of primary ins co
	ADD_FILTER_FIELD(429, "Primary Ins Co Financial Class", "FinancialClassT1.Name", fboPerson, foSimilarity|foBlank|foNotBlank|foEqual|foNotEqual, foBeginsWith, ftText, NULL, "FinancialClassT FinancialClassT1 ON InsuranceCoT1.FinancialClassID = FinancialClassT1.ID ", 129),

	ADD_FILTER_FIELD(161, "Secondary Ins Co", "InsuranceCoT2.PersonID", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT InsuranceCoT.Name AS Name, InsuranceCoT.PersonID AS ID FROM InsuranceCoT ORDER BY InsuranceCoT.Name"), NULL, 141, NULL, NULL, NULL, NULL, 0, P("InsuranceCoT.PersonID") P("InsuranceCoT.Name") P("InsuranceCoT")),
	ADD_FILTER_FIELD(149, "Secondary Ins Co Name", "InsuranceCoT2.Name", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 141),
	ADD_FILTER_FIELD(150, "Secondary Ins Co Address1", "InsuranceCoPersonT2.Address1", fboPerson, foAll, foBeginsWith, ftText, NULL, "PersonT AS InsuranceCoPersonT2 WITH(NOLOCK) ON InsuranceCoT2.PersonID = InsuranceCoPersonT2.ID", 141),
	ADD_FILTER_FIELD(151, "Secondary Ins Co Address2", "InsuranceCoPersonT2.Address2", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 150),
	ADD_FILTER_FIELD(152, "Secondary Ins Co City", "InsuranceCoPersonT2.City", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 150),
	ADD_FILTER_FIELD(153, "Secondary Ins Co State", "InsuranceCoPersonT2.State", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 150),
	ADD_FILTER_FIELD(154, "Secondary Ins Co Zipcode", "InsuranceCoPersonT2.Zip", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 150),
	ADD_FILTER_FIELD(256, "Secondary Ins Co First Name", "InsuranceContactsPersonT2.First", fboPerson, foAll, foBeginsWith, ftText, NULL, "PersonT AS InsuranceContactsPersonT2 WITH(NOLOCK) ON InsuredPartyT2.InsuranceContactID = InsuranceContactsPersonT2.ID", 130),
	ADD_FILTER_FIELD(257, "Secondary Ins Co Last Name", "InsuranceContactsPersonT2.Last", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 256),
	ADD_FILTER_FIELD(155, "Secondary Ins Co Phone", "(Replace(Replace(Replace(Replace(InsuranceContactsPersonT2.WorkPhone,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL, 256),
	ADD_FILTER_FIELD(258, "Secondary Ins Co Fax", "(Replace(Replace(Replace(Replace(InsuranceContactsPersonT2.Fax,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL, 256),
	//(e.lally 2010-05-05) PLID 35792 - Added Financial Class of secondary ins co
	ADD_FILTER_FIELD(430, "Secondary Ins Co Financial Class", "FinancialClassT2.Name", fboPerson, foSimilarity|foBlank|foNotBlank|foEqual|foNotEqual, foBeginsWith, ftText, NULL, "FinancialClassT FinancialClassT2 ON InsuranceCoT2.FinancialClassID = FinancialClassT2.ID ", 141),

	// (j.jones 2011-04-01 14:52) - PLID 43109 - added Vision Primary & Secondary filters
	ADD_FILTER_FIELD(519, "Vision (Primary) Ins Co", "InsuranceCoTVision1.PersonID", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT InsuranceCoT.Name AS Name, InsuranceCoT.PersonID AS ID FROM InsuranceCoT ORDER BY InsuranceCoT.Name"), NULL, 500, NULL, NULL, NULL, NULL, 0, P("InsuranceCoT.PersonID") P("InsuranceCoT.Name") P("InsuranceCoT")),
	ADD_FILTER_FIELD(520, "Vision (Primary) Ins Co Name", "InsuranceCoTVision1.Name", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 500),
	ADD_FILTER_FIELD(521, "Vision (Primary) Ins Co Address1", "InsuranceCoPersonTVision1.Address1", fboPerson, foAll, foBeginsWith, ftText, NULL, "PersonT AS InsuranceCoPersonTVision1 WITH(NOLOCK) ON InsuranceCoTVision1.PersonID = InsuranceCoPersonTVision1.ID", 500),
	ADD_FILTER_FIELD(522, "Vision (Primary) Ins Co Address2", "InsuranceCoPersonTVision1.Address2", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 521),
	ADD_FILTER_FIELD(523, "Vision (Primary) Ins Co City", "InsuranceCoPersonTVision1.City", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 521),
	ADD_FILTER_FIELD(524, "Vision (Primary) Ins Co State", "InsuranceCoPersonTVision1.State", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 521),
	ADD_FILTER_FIELD(525, "Vision (Primary) Ins Co Zipcode", "InsuranceCoPersonTVision1.Zip", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 521),
	ADD_FILTER_FIELD(526, "Vision (Primary) Ins Co First Name", "InsuranceContactsPersonTVision1.First", fboPerson, foAll, foBeginsWith, ftText, NULL, "PersonT AS InsuranceContactsPersonTVision1 WITH(NOLOCK) ON InsuredPartyTVision1.InsuranceContactID = InsuranceContactsPersonTVision1.ID", 483),
	ADD_FILTER_FIELD(527, "Vision (Primary) Ins Co Last Name", "InsuranceContactsPersonTVision1.Last", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 526),
	ADD_FILTER_FIELD(528, "Vision (Primary) Ins Co Phone", "(Replace(Replace(Replace(Replace(InsuranceContactsPersonTVision1.WorkPhone,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL, 526),
	ADD_FILTER_FIELD(529, "Vision (Primary) Ins Co Fax", "(Replace(Replace(Replace(Replace(InsuranceContactsPersonTVision1.Fax,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL, 526),
	ADD_FILTER_FIELD(530, "Vision (Primary) Ins Co Financial Class", "FinancialClassTVision1.Name", fboPerson, foSimilarity|foBlank|foNotBlank|foEqual|foNotEqual, foBeginsWith, ftText, NULL, "FinancialClassT FinancialClassTVision1 ON InsuranceCoTVision1.FinancialClassID = FinancialClassTVision1.ID ", 500),

	ADD_FILTER_FIELD(531, "Vision (Secondary) Ins Co", "InsuranceCoTVision2.PersonID", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT InsuranceCoT.Name AS Name, InsuranceCoT.PersonID AS ID FROM InsuranceCoT ORDER BY InsuranceCoT.Name"), NULL, 518, NULL, NULL, NULL, NULL, 0, P("InsuranceCoT.PersonID") P("InsuranceCoT.Name") P("InsuranceCoT")),
	ADD_FILTER_FIELD(532, "Vision (Secondary) Ins Co Name", "InsuranceCoTVision2.Name", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 518),
	ADD_FILTER_FIELD(533, "Vision (Secondary) Ins Co Address1", "InsuranceCoPersonTVision2.Address1", fboPerson, foAll, foBeginsWith, ftText, NULL, "PersonT AS InsuranceCoPersonTVision2 WITH(NOLOCK) ON InsuranceCoTVision2.PersonID = InsuranceCoPersonTVision2.ID", 518),
	ADD_FILTER_FIELD(534, "Vision (Secondary) Ins Co Address2", "InsuranceCoPersonTVision2.Address2", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 533),
	ADD_FILTER_FIELD(535, "Vision (Secondary) Ins Co City", "InsuranceCoPersonTVision2.City", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 533),
	ADD_FILTER_FIELD(536, "Vision (Secondary) Ins Co State", "InsuranceCoPersonTVision2.State", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 533),
	ADD_FILTER_FIELD(537, "Vision (Secondary) Ins Co Zipcode", "InsuranceCoPersonTVision2.Zip", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 533),
	ADD_FILTER_FIELD(538, "Vision (Secondary) Ins Co First Name", "InsuranceContactsPersonTVision2.First", fboPerson, foAll, foBeginsWith, ftText, NULL, "PersonT AS InsuranceContactsPersonTVision2 WITH(NOLOCK) ON InsuredPartyTVision2.InsuranceContactID = InsuranceContactsPersonTVision2.ID", 501),
	ADD_FILTER_FIELD(539, "Vision (Secondary) Ins Co Last Name", "InsuranceContactsPersonTVision2.Last", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 538),
	ADD_FILTER_FIELD(540, "Vision (Secondary) Ins Co Phone", "(Replace(Replace(Replace(Replace(InsuranceContactsPersonTVision2.WorkPhone,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL, 538),
	ADD_FILTER_FIELD(541, "Vision (Secondary) Ins Co Fax", "(Replace(Replace(Replace(Replace(InsuranceContactsPersonTVision2.Fax,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL, 538),
	ADD_FILTER_FIELD(542, "Vision (Secondary) Ins Co Financial Class", "FinancialClassTVision2.Name", fboPerson, foSimilarity|foBlank|foNotBlank|foEqual|foNotEqual, foBeginsWith, ftText, NULL, "FinancialClassT FinancialClassTVision2 ON InsuranceCoTVision2.FinancialClassID = FinancialClassTVision2.ID ", 518),

	// (d.singleton 2012-09-12 17:22) - PLID 51808 added auto filters 
	ADD_FILTER_FIELD(846, "Auto (Primary) Ins Co", "InsuranceCoTAuto1.PersonID", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT InsuranceCoT.Name AS Name, InsuranceCoT.PersonID AS ID FROM InsuranceCoT ORDER BY InsuranceCoT.Name"), NULL, 649, NULL, NULL, NULL, NULL, 0, P("InsuranceCoT.PersonID") P("InsuranceCoT.Name") P("InsuranceCoT")),
	ADD_FILTER_FIELD(847, "Auto (Primary) Ins Co Name", "InsuranceCoTAuto1.Name", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 649),
	ADD_FILTER_FIELD(848, "Auto (Primary) Ins Co Address1", "InsuranceCoPersonTAuto1.Address1", fboPerson, foAll, foBeginsWith, ftText, NULL, "PersonT AS InsuranceCoPersonTAuto1 WITH(NOLOCK) ON InsuranceCoTAuto1.PersonID = InsuranceCoPersonTAuto1.ID", 649),
	ADD_FILTER_FIELD(849, "Auto (Primary) Ins Co Address2", "InsuranceCoPersonTAuto1.Address2", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 848),
	ADD_FILTER_FIELD(850, "Auto (Primary) Ins Co City", "InsuranceCoPersonTAuto1.City", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 848),
	ADD_FILTER_FIELD(851, "Auto (Primary) Ins Co State", "InsuranceCoPersonTAuto1.State", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 848),
	ADD_FILTER_FIELD(852, "Auto (Primary) Ins Co Zipcode", "InsuranceCoPersonTAuto1.Zip", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 848),
	ADD_FILTER_FIELD(853, "Auto (Primary) Ins Co First Name", "InsuranceContactsPersonTAuto1.First", fboPerson, foAll, foBeginsWith, ftText, NULL, "PersonT AS InsuranceContactsPersonTAuto1 WITH(NOLOCK) ON InsuredPartyTAuto1.InsuranceContactID = InsuranceContactsPersonTAuto1.ID", 622),
	ADD_FILTER_FIELD(854, "Auto (Primary) Ins Co Last Name", "InsuranceContactsPersonTAuto1.Last", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 853),
	ADD_FILTER_FIELD(855, "Auto (Primary) Ins Co Phone", "(Replace(Replace(Replace(Replace(InsuranceContactsPersonTAuto1.WorkPhone,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL, 853),
	ADD_FILTER_FIELD(856, "Auto (Primary) Ins Co Fax", "(Replace(Replace(Replace(Replace(InsuranceContactsPersonTAuto1.Fax,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL, 853),
	ADD_FILTER_FIELD(857, "Auto (Primary) Ins Co Financial Class", "FinancialClassTAuto1.Name", fboPerson, foSimilarity|foBlank|foNotBlank|foEqual|foNotEqual, foBeginsWith, ftText, NULL, "FinancialClassT FinancialClassTAuto1 ON InsuranceCoTAuto1.FinancialClassID = FinancialClassTAuto1.ID ", 649),

	ADD_FILTER_FIELD(858, "Auto (Secondary) Ins Co", "InsuranceCoTAuto2.PersonID", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT InsuranceCoT.Name AS Name, InsuranceCoT.PersonID AS ID FROM InsuranceCoT ORDER BY InsuranceCoT.Name"), NULL, 667, NULL, NULL, NULL, NULL, 0, P("InsuranceCoT.PersonID") P("InsuranceCoT.Name") P("InsuranceCoT")),
	ADD_FILTER_FIELD(859, "Auto (Secondary) Ins Co Name", "InsuranceCoTAuto2.Name", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 667),
	ADD_FILTER_FIELD(860, "Auto (Secondary) Ins Co Address1", "InsuranceCoPersonTAuto2.Address1", fboPerson, foAll, foBeginsWith, ftText, NULL, "PersonT AS InsuranceCoPersonTAuto2 WITH(NOLOCK) ON InsuranceCoTAuto2.PersonID = InsuranceCoPersonTAuto2.ID", 667),
	ADD_FILTER_FIELD(861, "Auto (Secondary) Ins Co Address2", "InsuranceCoPersonTAuto2.Address2", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 860),
	ADD_FILTER_FIELD(862, "Auto (Secondary) Ins Co City", "InsuranceCoPersonTAuto2.City", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 860),
	ADD_FILTER_FIELD(863, "Auto (Secondary) Ins Co State", "InsuranceCoPersonTAuto2.State", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 860),
	ADD_FILTER_FIELD(864, "Auto (Secondary) Ins Co Zipcode", "InsuranceCoPersonTAuto2.Zip", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 860),
	ADD_FILTER_FIELD(865, "Auto (Secondary) Ins Co First Name", "InsuranceContactsPersonTAuto2.First", fboPerson, foAll, foBeginsWith, ftText, NULL, "PersonT AS InsuranceContactsPersonTAuto2 WITH(NOLOCK) ON InsuredPartyTAuto2.InsuranceContactID = InsuranceContactsPersonTAuto2.ID", 650),
	ADD_FILTER_FIELD(866, "Auto (Secondary) Ins Co Last Name", "InsuranceContactsPersonTAuto2.Last", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 865),
	ADD_FILTER_FIELD(867, "Auto (Secondary) Ins Co Phone", "(Replace(Replace(Replace(Replace(InsuranceContactsPersonTAuto2.WorkPhone,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL, 865),
	ADD_FILTER_FIELD(868, "Auto (Secondary) Ins Co Fax", "(Replace(Replace(Replace(Replace(InsuranceContactsPersonTAuto2.Fax,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL, 865),
	ADD_FILTER_FIELD(869, "Auto (Secondary) Ins Co Financial Class", "FinancialClassTAuto2.Name", fboPerson, foSimilarity|foBlank|foNotBlank|foEqual|foNotEqual, foBeginsWith, ftText, NULL, "FinancialClassT FinancialClassTAuto2 ON InsuranceCoTAuto2.FinancialClassID = FinancialClassTAuto2.ID ", 667),

	// (d.singleton 2012-09-14 13:38) - PLID 52657 added workers comp filter
	ADD_FILTER_FIELD(870, "Workers Comp (Primary) Ins Co", "InsuranceCoTWorkersComp1.PersonID", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT InsuranceCoT.Name AS Name, InsuranceCoT.PersonID AS ID FROM InsuranceCoT ORDER BY InsuranceCoT.Name"), NULL, 685, NULL, NULL, NULL, NULL, 0, P("InsuranceCoT.PersonID") P("InsuranceCoT.Name") P("InsuranceCoT")),
	ADD_FILTER_FIELD(871, "Workers Comp (Primary) Ins Co Name", "InsuranceCoTWorkersComp1.Name", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 685),
	ADD_FILTER_FIELD(872, "Workers Comp (Primary) Ins Co Address1", "InsuranceCoPersonTWorkersComp1.Address1", fboPerson, foAll, foBeginsWith, ftText, NULL, "PersonT AS InsuranceCoPersonTWorkersComp1 WITH(NOLOCK) ON InsuranceCoTWorkersComp1.PersonID = InsuranceCoPersonTWorkersComp1.ID", 685),
	ADD_FILTER_FIELD(873, "Workers Comp (Primary) Ins Co Address2", "InsuranceCoPersonTWorkersComp1.Address2", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 872),
	ADD_FILTER_FIELD(874, "Workers Comp (Primary) Ins Co City", "InsuranceCoPersonTWorkersComp1.City", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 872),
	ADD_FILTER_FIELD(875, "Workers Comp (Primary) Ins Co State", "InsuranceCoPersonTWorkersComp1.State", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 872),
	ADD_FILTER_FIELD(876, "Workers Comp (Primary) Ins Co Zipcode", "InsuranceCoPersonTWorkersComp1.Zip", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 872),
	ADD_FILTER_FIELD(877, "Workers Comp (Primary) Ins Co First Name", "InsuranceContactsPersonTWorkersComp1.First", fboPerson, foAll, foBeginsWith, ftText, NULL, "PersonT AS InsuranceContactsPersonTWorkersComp1 WITH(NOLOCK) ON InsuredPartyTWorkersComp1.InsuranceContactID = InsuranceContactsPersonTWorkersComp1.ID", 668),
	ADD_FILTER_FIELD(878, "Workers Comp (Primary) Ins Co Last Name", "InsuranceContactsPersonTWorkersComp1.Last", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 877),
	ADD_FILTER_FIELD(879, "Workers Comp (Primary) Ins Co Phone", "(Replace(Replace(Replace(Replace(InsuranceContactsPersonTWorkersComp1.WorkPhone,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL, 877),
	ADD_FILTER_FIELD(880, "Workers Comp (Primary) Ins Co Fax", "(Replace(Replace(Replace(Replace(InsuranceContactsPersonTWorkersComp1.Fax,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL, 877 ),
	ADD_FILTER_FIELD(881, "Workers Comp (Primary) Ins Co Financial Class", "FinancialClassTWorkersComp1.Name", fboPerson, foSimilarity|foBlank|foNotBlank|foEqual|foNotEqual, foBeginsWith, ftText, NULL, "FinancialClassT FinancialClassTWorkersComp1 ON InsuranceCoTWorkersComp1.FinancialClassID = FinancialClassTWorkersComp1.ID ", 685),

	ADD_FILTER_FIELD(882, "Workers Comp (Secondary) Ins Co", "InsuranceCoTWorkersComp2.PersonID", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT InsuranceCoT.Name AS Name, InsuranceCoT.PersonID AS ID FROM InsuranceCoT ORDER BY InsuranceCoT.Name"), NULL, 703, NULL, NULL, NULL, NULL, 0, P("InsuranceCoT.PersonID") P("InsuranceCoT.Name") P("InsuranceCoT")),
	ADD_FILTER_FIELD(883, "Workers Comp (Secondary) Ins Co Name", "InsuranceCoTWorkersComp2.Name", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 703),
	ADD_FILTER_FIELD(884, "Workers Comp (Secondary) Ins Co Address1", "InsuranceCoPersonTWorkersComp2.Address1", fboPerson, foAll, foBeginsWith, ftText, NULL, "PersonT AS InsuranceCoPersonTWorkersComp2 WITH(NOLOCK) ON InsuranceCoTWorkersComp2.PersonID = InsuranceCoPersonTWorkersComp2.ID", 703),
	ADD_FILTER_FIELD(885, "Workers Comp (Secondary) Ins Co Address2", "InsuranceCoPersonTWorkersComp2.Address2", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 884),
	ADD_FILTER_FIELD(886, "Workers Comp (Secondary) Ins Co City", "InsuranceCoPersonTWorkersComp2.City", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 884),
	ADD_FILTER_FIELD(887, "Workers Comp (Secondary) Ins Co State", "InsuranceCoPersonTWorkersComp2.State", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 884),
	ADD_FILTER_FIELD(888, "Workers Comp (Secondary) Ins Co Zipcode", "InsuranceCoPersonTWorkersComp2.Zip", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 884),
	ADD_FILTER_FIELD(889, "Workers Comp (Secondary) Ins Co First Name", "InsuranceContactsPersonTWorkersComp2.First", fboPerson, foAll, foBeginsWith, ftText, NULL, "PersonT AS InsuranceContactsPersonTWorkersComp2 WITH(NOLOCK) ON InsuredPartyTWorkersComp2.InsuranceContactID = InsuranceContactsPersonTWorkersComp2.ID", 686),
	ADD_FILTER_FIELD(890, "Workers Comp (Secondary) Ins Co Last Name", "InsuranceContactsPersonTWorkersComp2.Last", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 889),
	ADD_FILTER_FIELD(891, "Workers Comp (Secondary) Ins Co Phone", "(Replace(Replace(Replace(Replace(InsuranceContactsPersonTWorkersComp2.WorkPhone,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL, 889),
	ADD_FILTER_FIELD(892, "Workers Comp (Secondary) Ins Co Fax", "(Replace(Replace(Replace(Replace(InsuranceContactsPersonTWorkersComp2.Fax,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL, 889),
	ADD_FILTER_FIELD(893, "Workers Comp (Secondary) Ins Co Financial Class", "FinancialClassTWorkersComp2.Name", fboPerson, foSimilarity|foBlank|foNotBlank|foEqual|foNotEqual, foBeginsWith, ftText, NULL, "FinancialClassT FinancialClassTWorkersComp2 ON InsuranceCoTWorkersComp2.FinancialClassID = FinancialClassTWorkersComp2.ID ", 703),

	// (d.singleton 2012-09-14 13:38) - PLID 52658 added dental filter
	ADD_FILTER_FIELD(894, "Dental (Primary) Ins Co", "InsuranceCoTDental1.PersonID", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT InsuranceCoT.Name AS Name, InsuranceCoT.PersonID AS ID FROM InsuranceCoT ORDER BY InsuranceCoT.Name"), NULL, 721, NULL, NULL, NULL, NULL, 0, P("InsuranceCoT.PersonID") P("InsuranceCoT.Name") P("InsuranceCoT")),
	ADD_FILTER_FIELD(895, "Dental (Primary) Ins Co Name", "InsuranceCoTDental1.Name", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 721),
	ADD_FILTER_FIELD(896, "Dental (Primary) Ins Co Address1", "InsuranceCoPersonTDental1.Address1", fboPerson, foAll, foBeginsWith, ftText, NULL, "PersonT AS InsuranceCoPersonTDental1 WITH(NOLOCK) ON InsuranceCoTDental1.PersonID = InsuranceCoPersonTDental1.ID", 721),
	ADD_FILTER_FIELD(897, "Dental (Primary) Ins Co Address2", "InsuranceCoPersonTDental1.Address2", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 896),
	ADD_FILTER_FIELD(898, "Dental (Primary) Ins Co City", "InsuranceCoPersonTDental1.City", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 896),
	ADD_FILTER_FIELD(899, "Dental (Primary) Ins Co State", "InsuranceCoPersonTDental1.State", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 896),
	ADD_FILTER_FIELD(900, "Dental (Primary) Ins Co Zipcode", "InsuranceCoPersonTDental1.Zip", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 896),
	ADD_FILTER_FIELD(901, "Dental (Primary) Ins Co First Name", "InsuranceContactsPersonTDental1.First", fboPerson, foAll, foBeginsWith, ftText, NULL, "PersonT AS InsuranceContactsPersonTDental1 WITH(NOLOCK) ON InsuredPartyTDental1.InsuranceContactID = InsuranceContactsPersonTDental1.ID", 704),
	ADD_FILTER_FIELD(902, "Dental (Primary) Ins Co Last Name", "InsuranceContactsPersonTDental1.Last", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 901),
	ADD_FILTER_FIELD(903, "Dental (Primary) Ins Co Phone", "(Replace(Replace(Replace(Replace(InsuranceContactsPersonTDental1.WorkPhone,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL, 901),
	ADD_FILTER_FIELD(904, "Dental (Primary) Ins Co Fax", "(Replace(Replace(Replace(Replace(InsuranceContactsPersonTDental1.Fax,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL, 901),
	ADD_FILTER_FIELD(905, "Dental (Primary) Ins Co Financial Class", "FinancialClassTDental1.Name", fboPerson, foSimilarity|foBlank|foNotBlank|foEqual|foNotEqual, foBeginsWith, ftText, NULL, "FinancialClassT FinancialClassTDental1 ON InsuranceCoTDental1.FinancialClassID = FinancialClassTDental1.ID ", 721),

	ADD_FILTER_FIELD(906, "Dental (Secondary) Ins Co", "InsuranceCoTDental2.PersonID", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT InsuranceCoT.Name AS Name, InsuranceCoT.PersonID AS ID FROM InsuranceCoT ORDER BY InsuranceCoT.Name"), NULL, 738, NULL, NULL, NULL, NULL, 0, P("InsuranceCoT.PersonID") P("InsuranceCoT.Name") P("InsuranceCoT")),
	ADD_FILTER_FIELD(907, "Dental (Secondary) Ins Co Name", "InsuranceCoTDental2.Name", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 738),
	ADD_FILTER_FIELD(908, "Dental (Secondary) Ins Co Address1", "InsuranceCoPersonTDental2.Address1", fboPerson, foAll, foBeginsWith, ftText, NULL, "PersonT AS InsuranceCoPersonTDental2 WITH(NOLOCK) ON InsuranceCoTDental2.PersonID = InsuranceCoPersonTDental2.ID", 738),
	ADD_FILTER_FIELD(909, "Dental (Secondary) Ins Co Address2", "InsuranceCoPersonTDental2.Address2", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 908),
	ADD_FILTER_FIELD(910, "Dental (Secondary) Ins Co City", "InsuranceCoPersonTDental2.City", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 908),
	ADD_FILTER_FIELD(911, "Dental (Secondary) Ins Co State", "InsuranceCoPersonTDental2.State", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 908),
	ADD_FILTER_FIELD(912, "Dental (Secondary) Ins Co Zipcode", "InsuranceCoPersonTDental2.Zip", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 908),
	ADD_FILTER_FIELD(913, "Dental (Secondary) Ins Co First Name", "InsuranceContactsPersonTDental2.First", fboPerson, foAll, foBeginsWith, ftText, NULL, "PersonT AS InsuranceContactsPersonTDental2 WITH(NOLOCK) ON InsuredPartyTDental2.InsuranceContactID = InsuranceContactsPersonTDental2.ID", 722),
	ADD_FILTER_FIELD(914, "Dental (Secondary) Ins Co Last Name", "InsuranceContactsPersonTDental2.Last", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 913),
	ADD_FILTER_FIELD(915, "Dental (Secondary) Ins Co Phone", "(Replace(Replace(Replace(Replace(InsuranceContactsPersonTDental2.WorkPhone,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL, 913),
	ADD_FILTER_FIELD(916, "Dental (Secondary) Ins Co Fax", "(Replace(Replace(Replace(Replace(InsuranceContactsPersonTDental2.Fax,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL, 913),
	ADD_FILTER_FIELD(917, "Dental (Secondary) Ins Co Financial Class", "FinancialClassTDental2.Name", fboPerson, foSimilarity|foBlank|foNotBlank|foEqual|foNotEqual, foBeginsWith, ftText, NULL, "FinancialClassT FinancialClassTDental2 ON InsuranceCoTDental2.FinancialClassID = FinancialClassTDental2.ID ", 738),

	// (d.singleton 2012-09-14 13:38) - PLID 52659 added study filter
	ADD_FILTER_FIELD(918, "Study (Primary) Ins Co", "InsuranceCoTStudy1.PersonID", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT InsuranceCoT.Name AS Name, InsuranceCoT.PersonID AS ID FROM InsuranceCoT ORDER BY InsuranceCoT.Name"), NULL, 756, NULL, NULL, NULL, NULL, 0, P("InsuranceCoT.PersonID") P("InsuranceCoT.Name") P("InsuranceCoT")),
	ADD_FILTER_FIELD(919, "Study (Primary) Ins Co Name", "InsuranceCoTStudy1.Name", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 756),
	ADD_FILTER_FIELD(920, "Study (Primary) Ins Co Address1", "InsuranceCoPersonTStudy1.Address1", fboPerson, foAll, foBeginsWith, ftText, NULL, "PersonT AS InsuranceCoPersonTStudy1 WITH(NOLOCK) ON InsuranceCoTStudy1.PersonID = InsuranceCoPersonTStudy1.ID", 756),
	ADD_FILTER_FIELD(921, "Study (Primary) Ins Co Address2", "InsuranceCoPersonTStudy1.Address2", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 920),
	ADD_FILTER_FIELD(922, "Study (Primary) Ins Co City", "InsuranceCoPersonTStudy1.City", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 920),
	ADD_FILTER_FIELD(923, "Study (Primary) Ins Co State", "InsuranceCoPersonTStudy1.State", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 920),
	ADD_FILTER_FIELD(924, "Study (Primary) Ins Co Zipcode", "InsuranceCoPersonTStudy1.Zip", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 920),
	ADD_FILTER_FIELD(925, "Study (Primary) Ins Co First Name", "InsuranceContactsPersonTStudy1.First", fboPerson, foAll, foBeginsWith, ftText, NULL, "PersonT AS InsuranceContactsPersonTStudy1 WITH(NOLOCK) ON InsuredPartyTStudy1.InsuranceContactID = InsuranceContactsPersonTStudy1.ID", 739),
	ADD_FILTER_FIELD(926, "Study (Primary) Ins Co Last Name", "InsuranceContactsPersonTStudy1.Last", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 925),
	ADD_FILTER_FIELD(927, "Study (Primary) Ins Co Phone", "(Replace(Replace(Replace(Replace(InsuranceContactsPersonTStudy1.WorkPhone,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL, 925),
	ADD_FILTER_FIELD(928, "Study (Primary) Ins Co Fax", "(Replace(Replace(Replace(Replace(InsuranceContactsPersonTStudy1.Fax,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL, 925),
	ADD_FILTER_FIELD(929, "Study (Primary) Ins Co Financial Class", "FinancialClassTStudy1.Name", fboPerson, foSimilarity|foBlank|foNotBlank|foEqual|foNotEqual, foBeginsWith, ftText, NULL, "FinancialClassT FinancialClassTStudy1 ON InsuranceCoTStudy1.FinancialClassID = FinancialClassTStudy1.ID ", 756),

	ADD_FILTER_FIELD(930, "Study (Secondary) Ins Co", "InsuranceCoTStudy2.PersonID", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT InsuranceCoT.Name AS Name, InsuranceCoT.PersonID AS ID FROM InsuranceCoT ORDER BY InsuranceCoT.Name"), NULL, 773, NULL, NULL, NULL, NULL, 0, P("InsuranceCoT.PersonID") P("InsuranceCoT.Name") P("InsuranceCoT")),
	ADD_FILTER_FIELD(931, "Study (Secondary) Ins Co Name", "InsuranceCoTStudy2.Name", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 773),
	ADD_FILTER_FIELD(932, "Study (Secondary) Ins Co Address1", "InsuranceCoPersonTStudy2.Address1", fboPerson, foAll, foBeginsWith, ftText, NULL, "PersonT AS InsuranceCoPersonTStudy2 WITH(NOLOCK) ON InsuranceCoTStudy2.PersonID = InsuranceCoPersonTStudy2.ID", 773),
	ADD_FILTER_FIELD(933, "Study (Secondary) Ins Co Address2", "InsuranceCoPersonTStudy2.Address2", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 932),
	ADD_FILTER_FIELD(934, "Study (Secondary) Ins Co City", "InsuranceCoPersonTStudy2.City", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 932),
	ADD_FILTER_FIELD(935, "Study (Secondary) Ins Co State", "InsuranceCoPersonTStudy2.State", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 932),
	ADD_FILTER_FIELD(936, "Study (Secondary) Ins Co Zipcode", "InsuranceCoPersonTStudy2.Zip", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 932),
	ADD_FILTER_FIELD(937, "Study (Secondary) Ins Co First Name", "InsuranceContactsPersonTStudy2.First", fboPerson, foAll, foBeginsWith, ftText, NULL, "PersonT AS InsuranceContactsPersonTStudy2 WITH(NOLOCK) ON InsuredPartyTStudy2.InsuranceContactID = InsuranceContactsPersonTStudy2.ID", 757),
	ADD_FILTER_FIELD(938, "Study (Secondary) Ins Co Last Name", "InsuranceContactsPersonTStudy2.Last", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 937),
	ADD_FILTER_FIELD(939, "Study (Secondary) Ins Co Phone", "(Replace(Replace(Replace(Replace(InsuranceContactsPersonTStudy2.WorkPhone,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL, 937),
	ADD_FILTER_FIELD(940, "Study (Secondary) Ins Co Fax", "(Replace(Replace(Replace(Replace(InsuranceContactsPersonTStudy2.Fax,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL, 937),
	ADD_FILTER_FIELD(941, "Study (Secondary) Ins Co Financial Class", "FinancialClassTStudy2.Name", fboPerson, foSimilarity|foBlank|foNotBlank|foEqual|foNotEqual, foBeginsWith, ftText, NULL, "FinancialClassT FinancialClassTStudy2 ON InsuranceCoTStudy2.FinancialClassID = FinancialClassTStudy2.ID ", 773),

	// (d.singleton 2012-09-14 13:38) - PLID 52660 added LOP filter
	ADD_FILTER_FIELD(942, "LOP (Primary) Ins Co", "InsuranceCoTLOP1.PersonID", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT InsuranceCoT.Name AS Name, InsuranceCoT.PersonID AS ID FROM InsuranceCoT ORDER BY InsuranceCoT.Name"), NULL, 791, NULL, NULL, NULL, NULL, 0, P("InsuranceCoT.PersonID") P("InsuranceCoT.Name") P("InsuranceCoT")),
	ADD_FILTER_FIELD(943, "LOP (Primary) Ins Co Name", "InsuranceCoTLOP1.Name", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 791),
	ADD_FILTER_FIELD(944, "LOP (Primary) Ins Co Address1", "InsuranceCoPersonTLOP1.Address1", fboPerson, foAll, foBeginsWith, ftText, NULL, "PersonT AS InsuranceCoPersonTLOP1 WITH(NOLOCK) ON InsuranceCoTLOP1.PersonID = InsuranceCoPersonTLOP1.ID", 791),
	ADD_FILTER_FIELD(945, "LOP (Primary) Ins Co Address2", "InsuranceCoPersonTLOP1.Address2", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 944),
	ADD_FILTER_FIELD(946, "LOP (Primary) Ins Co City", "InsuranceCoPersonTLOP1.City", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 944),
	ADD_FILTER_FIELD(947, "LOP (Primary) Ins Co State", "InsuranceCoPersonTLOP1.State", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 944),
	ADD_FILTER_FIELD(948, "LOP (Primary) Ins Co Zipcode", "InsuranceCoPersonTLOP1.Zip", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 944),
	ADD_FILTER_FIELD(949, "LOP (Primary) Ins Co First Name", "InsuranceContactsPersonTLOP1.First", fboPerson, foAll, foBeginsWith, ftText, NULL, "PersonT AS InsuranceContactsPersonTLOP1 WITH(NOLOCK) ON InsuredPartyTLOP1.InsuranceContactID = InsuranceContactsPersonTLOP1.ID", 774),
	ADD_FILTER_FIELD(950, "LOP (Primary) Ins Co Last Name", "InsuranceContactsPersonTLOP1.Last", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 949),
	ADD_FILTER_FIELD(951, "LOP (Primary) Ins Co Phone", "(Replace(Replace(Replace(Replace(InsuranceContactsPersonTLOP1.WorkPhone,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL, 949),
	ADD_FILTER_FIELD(952, "LOP (Primary) Ins Co Fax", "(Replace(Replace(Replace(Replace(InsuranceContactsPersonTLOP1.Fax,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL, 949),
	ADD_FILTER_FIELD(953, "LOP (Primary) Ins Co Financial Class", "FinancialClassTLOP1.Name", fboPerson, foSimilarity|foBlank|foNotBlank|foEqual|foNotEqual, foBeginsWith, ftText, NULL, "FinancialClassT FinancialClassTLOP1 ON InsuranceCoTLOP1.FinancialClassID = FinancialClassTLOP1.ID ", 791),

	ADD_FILTER_FIELD(954, "LOP (Secondary) Ins Co", "InsuranceCoTLOP2.PersonID", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT InsuranceCoT.Name AS Name, InsuranceCoT.PersonID AS ID FROM InsuranceCoT ORDER BY InsuranceCoT.Name"), NULL, 809, NULL, NULL, NULL, NULL, 0, P("InsuranceCoT.PersonID") P("InsuranceCoT.Name") P("InsuranceCoT")),
	ADD_FILTER_FIELD(955, "LOP (Secondary) Ins Co Name", "InsuranceCoTLOP2.Name", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 809),
	ADD_FILTER_FIELD(956, "LOP (Secondary) Ins Co Address1", "InsuranceCoPersonTLOP2.Address1", fboPerson, foAll, foBeginsWith, ftText, NULL, "PersonT AS InsuranceCoPersonTLOP2 WITH(NOLOCK) ON InsuranceCoTLOP2.PersonID = InsuranceCoPersonTLOP2.ID", 809),
	ADD_FILTER_FIELD(957, "LOP (Secondary) Ins Co Address2", "InsuranceCoPersonTLOP2.Address2", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 956),
	ADD_FILTER_FIELD(958, "LOP (Secondary) Ins Co City", "InsuranceCoPersonTLOP2.City", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 956),
	ADD_FILTER_FIELD(959, "LOP (Secondary) Ins Co State", "InsuranceCoPersonTLOP2.State", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 956),
	ADD_FILTER_FIELD(960, "LOP (Secondary) Ins Co Zipcode", "InsuranceCoPersonTLOP2.Zip", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 956),
	ADD_FILTER_FIELD(961, "LOP (Secondary) Ins Co First Name", "InsuranceContactsPersonTLOP2.First", fboPerson, foAll, foBeginsWith, ftText, NULL, "PersonT AS InsuranceContactsPersonTLOP2 WITH(NOLOCK) ON InsuredPartyTLOP2.InsuranceContactID = InsuranceContactsPersonTLOP2.ID", 792),
	ADD_FILTER_FIELD(962, "LOP (Secondary) Ins Co Last Name", "InsuranceContactsPersonTLOP2.Last", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 961),
	ADD_FILTER_FIELD(963, "LOP (Secondary) Ins Co Phone", "(Replace(Replace(Replace(Replace(InsuranceContactsPersonTLOP2.WorkPhone,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL, 961),
	ADD_FILTER_FIELD(964, "LOP (Secondary) Ins Co Fax", "(Replace(Replace(Replace(Replace(InsuranceContactsPersonTLOP2.Fax,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL, 961),
	ADD_FILTER_FIELD(965, "LOP (Secondary) Ins Co Financial Class", "FinancialClassTLOP2.Name", fboPerson, foSimilarity|foBlank|foNotBlank|foEqual|foNotEqual, foBeginsWith, ftText, NULL, "FinancialClassT FinancialClassTLOP2 ON InsuranceCoTLOP2.FinancialClassID = FinancialClassTLOP2.ID ", 809),

	// (d.singleton 2012-09-14 13:39) - PLID 52661 added LOA filter
	// (s.tullis 2014-04-22 12:53) - PLID 59131 
	ADD_FILTER_FIELD(966, "LOA (Primary) Ins Co", "InsuranceCoTLOA1.PersonID", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT InsuranceCoT.Name AS Name, InsuranceCoT.PersonID AS ID FROM InsuranceCoT ORDER BY InsuranceCoT.Name"), NULL, 827, NULL, NULL, NULL, NULL, 0, P("InsuranceCoT.PersonID") P("InsuranceCoT.Name") P("InsuranceCoT")),
	ADD_FILTER_FIELD(967, "LOA (Primary) Ins Co Name", "InsuranceCoTLOA1.Name", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 827),
	ADD_FILTER_FIELD(968, "LOA (Primary) Ins Co Address1", "InsuranceCoPersonTLOA1.Address1", fboPerson, foAll, foBeginsWith, ftText, NULL, "PersonT AS InsuranceCoPersonTLOA1 WITH(NOLOCK) ON InsuranceCoTLOA1.PersonID = InsuranceCoPersonTLOA1.ID", 827),
	ADD_FILTER_FIELD(969, "LOA (Primary) Ins Co Address2", "InsuranceCoPersonTLOA1.Address2", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 968),
	ADD_FILTER_FIELD(970, "LOA (Primary) Ins Co City", "InsuranceCoPersonTLOA1.City", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 968),
	ADD_FILTER_FIELD(971, "LOA (Primary) Ins Co State", "InsuranceCoPersonTLOA1.State", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 968),
	ADD_FILTER_FIELD(972, "LOA (Primary) Ins Co Zipcode", "InsuranceCoPersonTLOA1.Zip", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 968),
	ADD_FILTER_FIELD(973, "LOA (Primary) Ins Co First Name", "InsuranceContactsPersonTLOA1.First", fboPerson, foAll, foBeginsWith, ftText, NULL, "PersonT AS InsuranceContactsPersonTLOA1 WITH(NOLOCK) ON InsuredPartyTLOA1.InsuranceContactID = InsuranceContactsPersonTLOA1.ID", 810),
	ADD_FILTER_FIELD(974, "LOA (Primary) Ins Co Last Name", "InsuranceContactsPersonTLOA1.Last", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 973),
	ADD_FILTER_FIELD(975, "LOA (Primary) Ins Co Phone", "(Replace(Replace(Replace(Replace(InsuranceContactsPersonTLOA1.WorkPhone,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL, 973),
	ADD_FILTER_FIELD(976, "LOA (Primary) Ins Co Fax", "(Replace(Replace(Replace(Replace(InsuranceContactsPersonTLOA1.Fax,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL, 973),
	ADD_FILTER_FIELD(977, "LOA (Primary) Ins Co Financial Class", "FinancialClassTLOA1.Name", fboPerson, foSimilarity|foBlank|foNotBlank|foEqual|foNotEqual, foBeginsWith, ftText, NULL, "FinancialClassT FinancialClassTLOA1 ON InsuranceCoTLOA1.FinancialClassID = FinancialClassTLOA1.ID ", 827),

	ADD_FILTER_FIELD(978, "LOA (Secondary) Ins Co", "InsuranceCoTLOA2.PersonID", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT InsuranceCoT.Name AS Name, InsuranceCoT.PersonID AS ID FROM InsuranceCoT ORDER BY InsuranceCoT.Name"), NULL, 845, NULL, NULL, NULL, NULL, 0, P("InsuranceCoT.PersonID") P("InsuranceCoT.Name") P("InsuranceCoT")),
	ADD_FILTER_FIELD(979, "LOA (Secondary) Ins Co Name", "InsuranceCoTLOA2.Name", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 845),
	ADD_FILTER_FIELD(980, "LOA (Secondary) Ins Co Address1", "InsuranceCoPersonTLOA2.Address1", fboPerson, foAll, foBeginsWith, ftText, NULL, "PersonT AS InsuranceCoPersonTLOA2 WITH(NOLOCK) ON InsuranceCoTLOA2.PersonID = InsuranceCoPersonTLOA2.ID", 845),
	ADD_FILTER_FIELD(981, "LOA (Secondary) Ins Co Address2", "InsuranceCoPersonTLOA2.Address2", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 980),
	ADD_FILTER_FIELD(982, "LOA (Secondary) Ins Co City", "InsuranceCoPersonTLOA2.City", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 980),
	ADD_FILTER_FIELD(983, "LOA (Secondary) Ins Co State", "InsuranceCoPersonTLOA2.State", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 980),
	ADD_FILTER_FIELD(984, "LOA (Secondary) Ins Co Zipcode", "InsuranceCoPersonTLOA2.Zip", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 980),
	ADD_FILTER_FIELD(985, "LOA (Secondary) Ins Co First Name", "InsuranceContactsPersonTLOA2.First", fboPerson, foAll, foBeginsWith, ftText, NULL, "PersonT AS InsuranceContactsPersonTLOA2 WITH(NOLOCK) ON InsuredPartyTLOA2.InsuranceContactID = InsuranceContactsPersonTLOA2.ID", 828),
	ADD_FILTER_FIELD(986, "LOA (Secondary) Ins Co Last Name", "InsuranceContactsPersonTLOA2.Last", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL, 985),
	ADD_FILTER_FIELD(987, "LOA (Secondary) Ins Co Phone", "(Replace(Replace(Replace(Replace(InsuranceContactsPersonTLOA2.WorkPhone,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL, 985),
	ADD_FILTER_FIELD(988, "LOA (Secondary) Ins Co Fax", "(Replace(Replace(Replace(Replace(InsuranceContactsPersonTLOA2.Fax,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL, 985),
	ADD_FILTER_FIELD(989, "LOA (Secondary) Ins Co Financial Class", "FinancialClassTLOA2.Name", fboPerson, foSimilarity|foBlank|foNotBlank|foEqual|foNotEqual, foBeginsWith, ftText, NULL, "FinancialClassT FinancialClassTLOA2 ON InsuranceCoTLOA2.FinancialClassID = FinancialClassTLOA2.ID ", 845),

	ADD_FILTER_FIELD(291, "Inventory Serial Number", "ProductItemsQ.ProductItemID", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ProductItemsT.SerialNum AS Name, ProductItemsT.ID AS ID FROM ProductItemsT WHERE SerialNum Is Not Null AND SerialNum <> '' AND ProductItemsT.Deleted = 0 ORDER BY ProductItemsT.SerialNum"),  
	"(SELECT ProductItemID, PatientID FROM ChargedProductItemsT INNER JOIN LineItemT ON ChargedProductItemsT.ChargeID = LineItemT.ID WHERE LineItemT.Deleted = 0 "
	"UNION SELECT ProductItemID, PersonID AS PatientID FROM ChargedProductItemsT INNER JOIN CaseHistoryDetailsT ON ChargedProductItemsT.CaseHistoryDetailID = CaseHistoryDetailsT.ID INNER JOIN CaseHistoryT ON CaseHistoryDetailsT.CaseHistoryID = CaseHistoryT.ID) "
	"ProductItemsQ ON PersonT.ID = ProductItemsQ.PatientID", FILTER_DEPENDS_ON_BASE, NULL, NULL, NULL, NULL, 0, P("ProductItemsT.ID") P("ProductItemsT.SerialNum") P("ProductItemsT") P("SerialNum Is Not Null AND SerialNum <> '' AND ProductItemsT.Deleted = 0")),

	//TES 5/29/2009 - PLID 34364 - Added a filter field to pull patients qualifying for a given Wellness Template
	//TES 6/2/2009 - PLID 34364 - This was using a stored function, but it didn't work in SQL 2K.  This is pretty fast, though.
	//TES 6/2/2009 - PLID 34446 - Filter out deleted templates
	ADD_FILTER_FIELD(414, "Qualifies For Wellness Template"	, "QualificationsT.WellnessTemplateID", fboPerson, foEqual|foNotEqual, foEqual, ftComboSelect, P("SELECT ID, Name FROM WellnessTemplateT WHERE Deleted = 0 AND SpecificToPatientID Is Null ORDER BY Name ASC"), 
	"(SELECT WellnessPatientQualificationT.PatientID,  WellnessTemplateCriterionT.WellnessTemplateID "
	"FROM WellnessPatientQualificationT INNER JOIN WellnessTemplateCriterionT "
	" ON WellnessPatientQualificationT.WellnessTemplateCriterionID = WellnessTemplateCriterionT.ID "
	"WHERE (COALESCE(dbo.AsDateNoTime(WellnessPatientQualificationT.EndDate), '9999-12-31') >= dbo.AsDateNoTime(GETDATE())) AND "
	" (WellnessPatientQualificationT.WellnessTemplateCriterionID NOT IN ( "
	"   SELECT ID FROM WellnessTemplateCriterionT WHERE WellnessTemplateID IN ( "
    "    SELECT WellnessTemplateID FROM PatientWellnessT "
	"     WHERE PatientID = WellnessPatientQualificationT.PatientID AND CompletedDate IS Not NULL)) "
	" ) "
	"GROUP BY WellnessPatientQualificationT.PatientID,  WellnessTemplateCriterionT.WellnessTemplateID "
	"HAVING COUNT(DISTINCT WellnessPatientQualificationT.WellnessTemplateCriterionID) /*CountQualifiedCriteria*/ "
	" = (SELECT COUNT(*) FROM WellnessTemplateCriterionT AllCriteria WHERE AllCriteria.WellnessTemplateID = WellnessTemplateCriterionT.WellnessTemplateID) /*CountTotalCriteria*/ "
	"AND MAX(COALESCE(dbo.AsDateNoTime(StartDate), dbo.AsDateNoTime(getdate()))) /*WillQualifyDate*/ <= dbo.AsDateNoTime(getDate()) "
	""
	"UNION SELECT PatientID, COALESCE(WellnessTemplateT.OriginalWellnessTemplateID,PatientWellnessT.WellnessTemplateID) AS WellnessTemplateID "
	"FROM PatientWellnessT INNER JOIN WellnessTemplateT ON PatientWellnessT.WellnessTemplateID = WellnessTemplateT.ID "
	"WHERE PatientWellnessT.ID NOT IN (SELECT ID FROM PatientWellnessCompletionItemT WHERE CompletionDate Is Not Null) "
	"GROUP BY PatientID, COALESCE(WellnessTemplateT.OriginalWellnessTemplateID,PatientWellnessT.WellnessTemplateID)) "
	"AS QualificationsT ON PersonT.ID = QualificationsT.PatientID", FILTER_DEPENDS_ON_BASE, NULL, NULL, NULL, NULL, 0, P("WellnessTemplateT.ID") P("WellnessTemplateT.Name") P("WellnessTemplateT") P("")),

	//TODO: This is not ideal. We need a better way for merging contacts. The most requested merge field is the Insurance
	//Company name, the remaining information can be pulled from PersonT. This field will always be blank if merged with a patient.
	ADD_FILTER_FIELD(216, "Insurance Company Name", "InsuranceCoT.Name", fboPerson, foAll, foBeginsWith, ftText, NULL, "InsuranceCoT ON PersonT.ID = InsuranceCoT.PersonID"),
	//Same problem with this field as well (though we combined them into one field for the time being)
	ADD_FILTER_FIELD(352, "Taxonomy Code", "(CASE WHEN ProvidersT.TaxonomyCode Is Null THEN ReferringPhysT.TaxonomyCode ELSE ProvidersT.TaxonomyCode END)", fboPerson, foAll, foBeginsWith, ftText, NULL, "ProvidersT ON PersonT.ID = ProvidersT.PersonID LEFT JOIN ReferringPhysT ON PersonT.ID = ReferringPhysT.PersonID"),
	//ADD_FILTER_FIELD(353, "Ref. Phy. Taxonomy Code", "ReferringPhysT.TaxonomyCode", fboPerson, foAll, foBeginsWith, ftText, NULL, "ReferringPhysT ON PersonT.ID = ReferringPhysT.PersonID"),
	ADD_FILTER_FIELD_OBSOLETE(353,"{352, PRE_EXISTING_FIELD_OPERATOR, \"PRE_EXISTING_SELECTED_VALUE\"}"),

	//ADD_FILTER_FIELD(11 , "Simple Last Name", "PersonT.Last", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL),
	ADD_FILTER_FIELD_OBSOLETE(11, "{79, PRE_EXISTING_FIELD_OPERATOR, \"PRE_EXISTING_SELECTED_VALUE\"}"),
	//ADD_FILTER_FIELD(9  , "Simple First Name", "PersonT.First", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL),
	ADD_FILTER_FIELD_OBSOLETE(9, "{77, PRE_EXISTING_FIELD_OPERATOR, \"PRE_EXISTING_SELECTED_VALUE\"}"),
	//ADD_FILTER_FIELD(10 , "Simple Middle Name", "PersonT.Middle", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL),
	ADD_FILTER_FIELD_OBSOLETE(10, "{78, PRE_EXISTING_FIELD_OPERATOR, \"PRE_EXISTING_SELECTED_VALUE\"}"),
	//ADD_FILTER_FIELD(12 , "Simple Address", "PersonT.Address1", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL),
	ADD_FILTER_FIELD_OBSOLETE(12, "{82, PRE_EXISTING_FIELD_OPERATOR, \"PRE_EXISTING_SELECTED_VALUE\"}"),
	//ADD_FILTER_FIELD(13 , "Simple City", "PersonT.City", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL),
	ADD_FILTER_FIELD_OBSOLETE(13, "{84, PRE_EXISTING_FIELD_OPERATOR, \"PRE_EXISTING_SELECTED_VALUE\"}"),
	//ADD_FILTER_FIELD(14 , "Simple State", "PersonT.State", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL),
	ADD_FILTER_FIELD_OBSOLETE(14, "{85, PRE_EXISTING_FIELD_OPERATOR, \"PRE_EXISTING_SELECTED_VALUE\"}"),
	//ADD_FILTER_FIELD(15 , "Simple Zip", "PersonT.Zip", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL),
	ADD_FILTER_FIELD_OBSOLETE(15, "{86, PRE_EXISTING_FIELD_OPERATOR, \"PRE_EXISTING_SELECTED_VALUE\"}"),
	//ADD_FILTER_FIELD(16 , "Simple SS Number", "PersonT.SocialSecurity", fboPerson, foAll, foBeginsWith, ftText, NULL, NULL),
	ADD_FILTER_FIELD_OBSOLETE(16, "{92, PRE_EXISTING_FIELD_OPERATOR, \"PRE_EXISTING_SELECTED_VALUE\"}"),
	//ADD_FILTER_FIELD(17 , "Simple Birth Date", "dbo.AsDateNoTime(PersonT.BirthDate)", fboPerson, foAll, foEqual, ftDate, NULL, NULL),	//see note at the top about conve
	ADD_FILTER_FIELD_OBSOLETE(17, "{93, PRE_EXISTING_FIELD_OPERATOR, \"PRE_EXISTING_SELECTED_VALUE\"}"),
	//ADD_FILTER_FIELD(22 , "Simple Birth Month", "MONTH(PersonT.BirthDate)", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboValues, CGroups::CalcInternationalMonthList(), NULL),
	ADD_FILTER_FIELD_OBSOLETE(22, "{189, PRE_EXISTING_FIELD_OPERATOR, \"PRE_EXISTING_SELECTED_VALUE\"}"),
	//ADD_FILTER_FIELD(188, "Simple Birth Day", "DAY(PersonT.BirthDate)", fboPerson, foAll, foEqual, ftNumber, NULL, NULL),
	ADD_FILTER_FIELD_OBSOLETE(188, "{190, PRE_EXISTING_FIELD_OPERATOR, \"PRE_EXISTING_SELECTED_VALUE\"}"),
	//ADD_FILTER_FIELD(18 , "Simple Age", "(CASE WHEN PersonT.BirthDate IS NOT NULL THEN (CASE WHEN PersonT.BirthDate < GETDATE() THEN YEAR(GETDATE()-PersonT.BirthDate)-1900 ELSE 0 END) ELSE NULL END)", fboPerson, foAll, foEqual, ftNumber, NULL, NULL),
	ADD_FILTER_FIELD_OBSOLETE(18, "{94, PRE_EXISTING_FIELD_OPERATOR, \"PRE_EXISTING_SELECTED_VALUE\"}"),
	//ADD_FILTER_FIELD(19 , "Simple Gender", "PersonT.Gender", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboValues, P("Male") P("1") P("Female") P("2")),
	ADD_FILTER_FIELD_OBSOLETE(19, "{91, PRE_EXISTING_FIELD_OPERATOR, \"PRE_EXISTING_SELECTED_VALUE\"}"),
	//ADD_FILTER_FIELD(20 , "Simple Status", "PatientsT.CurrentStatus", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboValues, P("Patient") P("1") P("Prospect") P("2") P("Patient/Prospect") P("3") P("Unspecified") P("0"), NULL, 159),
	ADD_FILTER_FIELD_OBSOLETE(20, "{100, PRE_EXISTING_FIELD_OPERATOR, \"PRE_EXISTING_SELECTED_VALUE\"}"),
	//ADD_FILTER_FIELD(21 , "Simple ID", "PatientsT.UserDefinedID", fboPerson, foAll, foEqual, ftNumber, NULL, NULL, 159),
	ADD_FILTER_FIELD_OBSOLETE(21, "{162, PRE_EXISTING_FIELD_OPERATOR, \"PRE_EXISTING_SELECTED_VALUE\"}"),

/* example:
	ADD_FILTER_FIELD(FILTER_FIELD_NEXT_INFO_ID, "ApparentField_LevelsSeperated_ByUnderscores", "InternalTable.InternalField", foOperator1|foOperator2|etc, foDefaultOperator, ftFieldType, dnstrParameters, strJoinClause, nJoinDependsOnInfoId), 
//*/

	//nextech only filter fields!
	ADD_FILTER_FIELD(228 , "Allow Support", "NxClientsT.AllowExpiredSupport", fboPerson, foAll, foEqual, ftComboValues, P("True") P("1") P("False") P("0"), "NxClientsT WITH(NOLOCK) ON PersonT.ID = NxClientsT.PersonID"),
	ADD_FILTER_FIELD(229 , "Support Expires", "dbo.AsDateNoTime(CASE WHEN IsDate(NxClientsT.SupportExpires) = 1 THEN SupportExpirES ELSE NULL END)", fboPerson, foAll, foEqual, ftDate, NULL, "NxClientsT WITH(NOLOCK) ON PersonT.ID = NxClientsT.PersonID"),	//see note at the top about converting date
	ADD_FILTER_FIELD(250 , "Support Rating", "NxClientsT.Rating", fboPerson, foAll, foEqual, ftComboValues, P("10") P("'10'") P("9") P("'9'") P("8") P("'8'") P("7") P("'7'") P("6") P("'6'") P("5") P("'5'") P("4") P("'4'") P("3") P("'3'") P("2") P("'2'") P("1") P("'1'"), "NxClientsT WITH(NOLOCK) ON PersonT.ID = NxClientsT.PersonID"), 
	ADD_FILTER_FIELD(251 , "Reference Rating", "NxClientsT.RefRating", fboPerson, foAll, foEqual, ftComboValues, P("A") P("'A'") P("B") P("'B'") P("C") P("'C'") P("D") P("'D'") P("F") P("'F'"), "NxClientsT WITH(NOLOCK) ON PersonT.ID = NxClientsT.PersonID"), 
	ADD_FILTER_FIELD(252 , "Reference Status", "NxClientsT.RefStatus", fboPerson, foAll, foEqual, ftComboValues, P("Power User") P("'Power User'") P("Normal User") P("'Normal User'"), "NxClientsT WITH(NOLOCK) ON PersonT.ID = NxClientsT.PersonID"), 
	//(J.Camacho 2013-03-12 10:28) - PLID 53866 - Added LicenseKey as a filterable option for internal NexTech use only.
	ADD_FILTER_FIELD(992 , "License Key", "NxClientsT.LicenseKey", fboPerson, foEqual|foNotEqual|foLess|foLessEqual|foGreater|foGreaterEqual|foBeginsWith|foEndsWith|foContains , foEqual, ftNumber, NULL, "NxClientsT WITH(NOLOCK) ON PersonT.ID = NxClientsT.PersonID"),
	//end nextech-only filter fields

	ADD_FILTER_FIELD(239 , "Mobile Phone", "(Replace(Replace(Replace(Replace(PersonT.CellPhone,'(',''),')',''),'-',''),' ',''))", fboPerson, foAll, foBeginsWith, ftPhoneNumber, NULL, NULL),

	//TES 3/26/2007 - PLID 20528 - Updated all ftSubFilterEditable fields to include the CreatedDate and ModifiedDate fields.
	ADD_FILTER_FIELD(267 , "Has EMN", "EMRMasterT.ID", fboPerson, foIn|foNotIn, foIn, ftSubFilterEditable, P("SELECT ID, Name, CreatedDate, ModifiedDate, Filter FROM FiltersT WHERE Type = 2 ORDER BY Name"), NULL, FILTER_DEPENDS_ON_BASE, "(SELECT * FROM EMRMasterT WHERE Deleted = 0) AS EMRMasterT", "PersonT.ID = EMRMasterT.PatientID", NULL, "EMN Filter", fboEMN),
	ADD_FILTER_FIELD(356 , "Has EMR", "EMRGroupsT.ID", fboPerson, foIn|foNotIn, foIn, ftSubFilterEditable, P("SELECT ID, Name, CreatedDate, ModifiedDate, Filter FROM FiltersT WHERE Type = 5 ORDER BY Name"), NULL, FILTER_DEPENDS_ON_BASE, "(SELECT * FROM EMRGroupsT WHERE Deleted = 0) AS EMRGroupsT", "PersonT.ID = EMRGroupsT.PatientID", NULL, "EMR Filter", fboEMR),

	//EMN filters
	ADD_FILTER_FIELD(268 , "Has Procedure", "EMNProcQ.ProcedureID", fboEMN, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID, Name FROM ProcedureT ORDER BY Name"), NULL, FILTER_DEPENDS_ON_BASE, "(SELECT EMRMasterT.ID AS EMRID, ProcedureID FROM (SELECT * FROM EmrMasterT WHERE Deleted = 0) AS EMRMasterT LEFT JOIN (SELECT * FROM EmrProcedureT WHERE Deleted = 0) AS EMRProcedureT ON EMRMasterT.ID = EMRProcedureT.EMRID) EMNProcQ", "EMRMasterT.ID = EMNProcQ.EMRID", NULL, NULL, 0, P("ID") P("Name") P("ProcedureT")),
	ADD_FILTER_FIELD(269 , "Has Provider", "EmnProvQ.ProviderID", fboEMN, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID, Last + ', ' + First + ' ' + Middle AS Name FROM PersonT WHERE ID IN (SELECT PersonID FROM ProvidersT) ORDER BY Last, First, Middle"), NULL, FILTER_DEPENDS_ON_BASE, "(SELECT EMRMasterT.ID AS EMRID, ProviderID FROM (SELECT * FROM EmrMasterT WHERE Deleted = 0) AS EMRMasterT LEFT JOIN (SELECT * FROM EmrProvidersT WHERE Deleted = 0) AS EmrProvidersT ON EMRMasterT.ID = EmrProvidersT.EMRID) EMNProvQ", "EMRMasterT.ID = EMNProvQ.EMRID", NULL, NULL, 0, P("ID") P("Last + ', ' + First + ' ' + Middle") P("PersonT") P("ID IN (SELECT PersonID FROM ProvidersT)")),
	ADD_FILTER_FIELD(270 , "Location", "EMRMasterT.LocationID", fboEMN, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID, Name FROM LocationsT WHERE TypeID = 1 ORDER BY Name"), NULL, FILTER_DEPENDS_ON_BASE, NULL, NULL, NULL, NULL, 0, P("ID") P("Name") P("LocationsT") P("TypeID = 1")),
	ADD_FILTER_FIELD(271 , "Date", "dbo.AsDateNoTime(EMRMasterT.Date)", fboEMN, foAll, foEqual, ftDate, NULL, NULL),
	// (j.jones 2010-12-08 09:57) - PLID 41757 - this filter is integer only, but the EMN's patient age can be a string,
	// the only time it is a non-integer string is if it has the word "month" in it, so assume that is < 1,
	// it must be a whole number or else the filter will fail
	ADD_FILTER_FIELD(293 , "Age", "CASE WHEN EmrMasterT.PatientAge LIKE '%month%' THEN 0 ELSE EmrMasterT.PatientAge END", fboEMN, foEquality, foEqual, ftNumber), 
	ADD_FILTER_FIELD(294 , "Has Procedure Name", "EMNProcQ.ProcName", fboEMN, foAll, foBeginsWith, ftText, NULL, NULL, FILTER_DEPENDS_ON_BASE, "(SELECT EMRMasterT.ID AS EMRID, ProcedureT.Name AS ProcName FROM (SELECT * FROM EmrMasterT WHERE Deleted = 0) AS EMRMasterT LEFT JOIN (SELECT * FROM EmrProcedureT WHERE Deleted = 0) AS EMRProcedureT ON EMRMasterT.ID = EMRProcedureT.EMRID LEFT JOIN ProcedureT ON EMRProcedureT.ProcedureID = ProcedureT.ID) EMNProcQ", "EMRMasterT.ID = EMNProcQ.EMRID"),

	// (c.haag 2008-06-27 15:20) - PLID 30319 - Filter out -27 (EMR text macro) sentinel items
	// (j.jones 2010-06-04 15:40) - PLID 39029 - also filter out Generic Table items, DataSubType of 3
	// (r.goldschmidt 2014-02-27 18:32) - PLID 59135 - Running an EMN letter writing filter with the field "Has Item" gets a SQL syntax error.
	ADD_FILTER_FIELD(273 , "Has Item", "EmrDetailsT.EmrInfoID", fboEMN, foEqual|foNotEqual, foEqual, ftComboSelect, P("SELECT ID, Name FROM EMRInfoT WHERE EmrInfoT.ID <> -27 AND DataSubType <> 3 ORDER BY Name"), NULL, FILTER_DEPENDS_ON_BASE, "(SELECT * FROM EmrDetailsT WHERE Deleted = 0 AND EmrInfoID <> -27 AND EmrInfoID NOT IN (SELECT ID FROM EMRInfoT WHERE DataSubType = 3)) AS EMRDetailsT", "EMRMasterT.ID = EMRDetailsT.EMRID", NULL, NULL, 0, P("ID") P("Name") P("EMRInfoT")),
	ADD_FILTER_FIELD(272 , "DYNAMIC_FIELD:SELECT ID, Name FROM EMRInfoT WHERE DataType = 1 AND ID <> -27 AND DataSubType <> 3", "EMRDetailsT.Text", fboEMN, foBlank | foNotBlank | foLike | foNotLike | foBeginsWith | foEndsWith | foContains, foBeginsWith, ftText, NULL, NULL, FILTER_DEPENDS_ON_BASE, "(SELECT * FROM EmrDetailsT WHERE Deleted = 0) AS EmrDetailsT LEFT JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID", "EMRMasterT.ID = EMRDetailsT.EMRID AND EMRInfoT.ID = {DYNAMIC_ID} AND EMRDetailsT.EMRID = EMRMasterT.ID"),
	ADD_FILTER_FIELD(274 , "DYNAMIC_FIELD:SELECT ID, Name FROM EMRInfoT WHERE DataType = 2", "EmrSelectT.EMRDataID", fboEMN, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID, Data AS Name FROM EmrDataT WHERE EmrInfoID = {DYNAMIC_ID}"), NULL, FILTER_DEPENDS_ON_BASE, "(SELECT * FROM EMRInfoT WHERE ID = {DYNAMIC_ID}) AS EMRInfoT LEFT JOIN EMRDataT ON EMRInfoT.ID = EMRDataT.EMRInfoID LEFT JOIN (SELECT * FROM EmrDetailsT WHERE Deleted = 0) AS EMRDetailsT ON EMRInfoT.ID = EMRDetailsT.EMRInfoID LEFT JOIN EMRSelectT ON EMRDetailsT.ID = EMRSelectT.EMRDetailID ", "EMRMasterT.ID = EMRDetailsT.EMRID"),
	ADD_FILTER_FIELD(355 , "DYNAMIC_FIELD:SELECT ID, Name FROM EMRInfoT WHERE DataType = 3", "EmrSelectT.EMRDataID", fboEMN, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID, Data AS Name FROM EmrDataT WHERE EmrInfoID = {DYNAMIC_ID}"), NULL, FILTER_DEPENDS_ON_BASE, "(SELECT * FROM EMRInfoT WHERE ID = {DYNAMIC_ID}) AS EMRInfoT LEFT JOIN EMRDataT ON EMRInfoT.ID = EMRDataT.EMRInfoID LEFT JOIN (SELECT * FROM EmrDetailsT WHERE Deleted = 0) AS EMRDetailsT ON EMRInfoT.ID = EMRDetailsT.EMRInfoID LEFT JOIN EMRSelectT ON EMRDetailsT.ID = EMRSelectT.EMRDetailID ", "EMRMasterT.ID = EMRDetailsT.EMRID"),
	ADD_FILTER_FIELD(275 , "< Advanced Filter >", "", fboEMN, foInvalidOperator, foInvalidOperator, ftAdvanced, NULL, NULL),
	ADD_FILTER_FIELD(276 , "< Subfilter >", "EMRMasterT.ID", fboEMN, foIn|foNotIn, foIn, ftSubFilterEditable, P("SELECT ID, Name, CreatedDate, ModifiedDate, Filter FROM FiltersT WHERE Type = 2 ORDER BY Name"), NULL, FILTER_DEPENDS_ON_BASE, NULL, NULL, NULL, "Emr Filter", fboEMN),

	//EMR filters
	ADD_FILTER_FIELD(357 , "Has Procedure", "EMRProcQ.ProcedureID", fboEMR, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID, Name FROM ProcedureT ORDER BY Name"), NULL, FILTER_DEPENDS_ON_BASE, "(SELECT EMRMasterT.ID AS EMNID, EMRMasterT.EMRGroupID AS EMRID, ProcedureID FROM (SELECT * FROM EmrMasterT WHERE Deleted = 0) AS EMRMasterT LEFT JOIN (SELECT * FROM EmrProcedureT WHERE Deleted = 0) AS EMRProcedureT ON EMRMasterT.ID = EMRProcedureT.EMRID WHERE EMRProcedureT.Deleted = 0) EMRProcQ", "EMRGroupsT.ID = EMRProcQ.EMRID", NULL, NULL, 0, P("ID") P("Name") P("ProcedureT")),
	ADD_FILTER_FIELD(358 , "Has Provider", "EmrProvQ.ProviderID", fboEMR, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID, Last + ', ' + First + ' ' + Middle AS Name FROM PersonT WHERE ID IN (SELECT PersonID FROM ProvidersT) ORDER BY Last, First, Middle"), NULL, FILTER_DEPENDS_ON_BASE, "(SELECT EMRMasterT.ID AS EMNID, EMRMasterT.EMRGroupID AS EMRID, ProviderID FROM (SELECT * FROM EmrMasterT WHERE Deleted = 0) AS EMRMasterT LEFT JOIN (SELECT * FROM EmrProvidersT WHERE Deleted = 0) AS EmrProvidersT ON EMRMasterT.ID = EmrProvidersT.EMRID) EMRProvQ", "EMRGroupsT.ID = EmrProvQ.EMRID", NULL, NULL, 0, P("ID") P("Last + ', ' + First + ' ' + Middle") P("PersonT") P("ID IN (SELECT PersonID FROM ProvidersT)")),
	ADD_FILTER_FIELD(359 , "Location", "EMRMasterT.LocationID", fboEMR, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID, Name FROM LocationsT WHERE TypeID = 1 ORDER BY Name"), NULL, FILTER_DEPENDS_ON_BASE, "(SELECT * FROM EmrMasterT WHERE Deleted = 0) AS EMRMasterT", "EMRGroupsT.ID = EMRMasterT.EMRGroupID", NULL, NULL, 0, P("ID") P("Name") P("LocationsT") P("TypeID = 1")),
	ADD_FILTER_FIELD(360 , "Date", "dbo.AsDateNoTime(EMRMasterT.Date)", fboEMR, foAll, foEqual, ftDate, NULL, NULL, FILTER_DEPENDS_ON_BASE, "(SELECT * FROM EmrMasterT WHERE Deleted = 0) AS EMRMasterT", "EMRGroupsT.ID = EMRMasterT.EMRGroupID"),
	// (j.jones 2010-12-08 09:57) - PLID 41757 - this filter is integer only, but the EMN's patient age can be a string,
	// the only time it is a non-integer string is if it has the word "month" in it, so assume that is < 1,
	// it must be a whole number or else the filter will fail
	ADD_FILTER_FIELD(361 , "Age", "CASE WHEN EmrMasterT.PatientAge LIKE '%month%' THEN 0 ELSE EmrMasterT.PatientAge END", fboEMR, foEquality, foEqual, ftNumber, NULL, NULL, FILTER_DEPENDS_ON_BASE, "(SELECT * FROM EmrMasterT WHERE Deleted = 0) AS EMRMasterT", "EMRGroupsT.ID = EMRMasterT.EMRGroupID"),
	ADD_FILTER_FIELD(362 , "Has Procedure Name", "EMRProcQ.ProcName", fboEMR, foAll, foBeginsWith, ftText, NULL, NULL, FILTER_DEPENDS_ON_BASE, "(SELECT EMRMasterT.EMRGroupID AS EMRID, ProcedureT.Name AS ProcName FROM EMRMasterT LEFT JOIN (SELECT * FROM EmrProcedureT WHERE Deleted = 0) AS EMRProcedureT ON EMRMasterT.EMRGroupID = EMRProcedureT.EMRID LEFT JOIN ProcedureT ON EMRProcedureT.ProcedureID = ProcedureT.ID) EMRProcQ", "EMRGroupsT.ID = EMRProcQ.EMRID"),

	ADD_FILTER_FIELD(363 , "Has Item", "EMRDetailsQ.EMRInfoID", fboEMR, foEqual|foNotEqual, foEqual, ftComboSelect, P("SELECT ID, Name FROM EMRInfoT WHERE EmrInfoT.ID <> -27 AND EmrInfoT.DataSubType <> 3 ORDER BY Name"), NULL, FILTER_DEPENDS_ON_BASE, "(SELECT EMRDetailsT.*, EMRDetailsT.ID AS EMRDetailID, EMRMasterT.EMRGroupID FROM (SELECT * FROM EmrDetailsT WHERE Deleted = 0) AS EMRDetailsT INNER JOIN EMRMasterT ON EMRDetailsT.EMRID = EMRMasterT.ID) AS EMRDetailsQ", "EMRGroupsT.ID = EMRDetailsQ.EMRGroupID", NULL, NULL, 0, P("ID") P("Name") P("EMRInfoT")),
	ADD_FILTER_FIELD(364 , "DYNAMIC_FIELD:SELECT ID, Name FROM EMRInfoT WHERE DataType = 1 AND ID <> -27 AND DataSubType <> 3", "EMRDetailsQ.Text", fboEMR, foBlank | foNotBlank | foLike | foNotLike | foBeginsWith | foEndsWith | foContains, foBeginsWith, ftText, NULL, NULL, FILTER_DEPENDS_ON_BASE, "(SELECT EMRDetailsT.*, EMRDetailsT.ID AS EMRDetailID, EMRMasterT.EMRGroupID FROM (SELECT * FROM EmrDetailsT WHERE Deleted = 0) AS EMRDetailsT INNER JOIN EMRMasterT ON EMRDetailsT.EMRID = EMRMasterT.ID) AS EMRDetailsQ LEFT JOIN EMRInfoT ON EMRDetailsQ.EMRInfoID = EMRInfoT.ID", "EMRGroupsT.ID = EMRDetailsQ.EMRGroupID AND EMRInfoT.ID = {DYNAMIC_ID} AND EMRDetailsQ.EMRGroupID = EMRGroupsT.ID"),
	ADD_FILTER_FIELD(365 , "DYNAMIC_FIELD:SELECT ID, Name FROM EMRInfoT WHERE DataType = 2", "EmrSelectT.EMRDataID", fboEMR, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID, Data AS Name FROM EmrDataT WHERE EmrInfoID = {DYNAMIC_ID}"), NULL, FILTER_DEPENDS_ON_BASE, "(SELECT * FROM EMRInfoT WHERE ID = {DYNAMIC_ID}) AS EMRInfoT LEFT JOIN EMRDataT ON EMRInfoT.ID = EMRDataT.EMRInfoID LEFT JOIN (SELECT EMRDetailsT.*, EMRDetailsT.ID AS EMRDetailID, EMRMasterT.EMRGroupID FROM (SELECT * FROM EmrDetailsT WHERE Deleted = 0) AS EMRDetailsT INNER JOIN EMRMasterT ON EMRDetailsT.EMRID = EMRMasterT.ID) AS EMRDetailsQ ON EMRInfoT.ID = EMRDetailsQ.EMRInfoID LEFT JOIN EMRSelectT ON EMRDetailsQ.ID = EMRSelectT.EMRDetailID", "EMRGroupsT.ID = EMRDetailsQ.EMRGroupID"),
	ADD_FILTER_FIELD(366 , "DYNAMIC_FIELD:SELECT ID, Name FROM EMRInfoT WHERE DataType = 3", "EmrSelectT.EMRDataID", fboEMR, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID, Data AS Name FROM EmrDataT WHERE EmrInfoID = {DYNAMIC_ID}"), NULL, FILTER_DEPENDS_ON_BASE, "(SELECT * FROM EMRInfoT WHERE ID = {DYNAMIC_ID}) AS EMRInfoT LEFT JOIN EMRDataT ON EMRInfoT.ID = EMRDataT.EMRInfoID LEFT JOIN (SELECT EMRDetailsT.*, EMRDetailsT.ID AS EMRDetailID, EMRMasterT.EMRGroupID FROM (SELECT * FROM EmrDetailsT WHERE Deleted = 0) AS EMRDetailsT INNER JOIN EMRMasterT ON EMRDetailsT.EMRID = EMRMasterT.ID) AS EMRDetailsQ ON EMRInfoT.ID = EMRDetailsQ.EMRInfoID LEFT JOIN EMRSelectT ON EMRDetailsQ.ID = EMRSelectT.EMRDetailID", "EMRGroupsT.ID = EMRDetailsQ.EMRGroupID"),
	ADD_FILTER_FIELD(367 , "< Advanced Filter >", "", fboEMR, foInvalidOperator, foInvalidOperator, ftAdvanced, NULL, NULL),
	ADD_FILTER_FIELD(368 , "< Subfilter >", "EMRGroupsT.ID", fboEMR, foIn|foNotIn, foIn, ftSubFilterEditable, P("SELECT ID, Name, CreatedDate, ModifiedDate, Filter FROM FiltersT WHERE Type = 2 ORDER BY Name"), NULL, FILTER_DEPENDS_ON_BASE, NULL, NULL, NULL, "EMR Filter", fboEMR),

	//Todo filters
	// (c.haag 2008-06-30 11:44) - PLID 30565 - Updated "Assigned To" to use the new multi-assignee todo structure
	ADD_FILTER_FIELD(325 , "Has Todo", "ToDoList.TaskID", fboPerson, foIn|foNotIn, foIn, ftSubFilterEditable, P("SELECT ID, Name, CreatedDate, ModifiedDate, Filter FROM FiltersT WHERE Type = 4 ORDER BY Name"), NULL, FILTER_DEPENDS_ON_BASE, "ToDoList", "PersonT.ID = ToDoList.PersonID", NULL, "Todo Filter", fboTodo),
	ADD_FILTER_FIELD(326 , "Assigned To (Any)", "TodoAssignToT.AssignTo", fboTodo, foEqual|foNotEqual, foEqual, ftComboSelect, P("SELECT PersonID AS ID, UserName AS Name FROM UsersT WHERE UsersT.PersonID > 0 ORDER BY UserName"), "TodoAssignToT ON TodoAssignToT.TaskID = TodoList.TaskID", FILTER_DEPENDS_ON_BASE, NULL, NULL, NULL, NULL, 0, P("PersonID") P("UserName") P("UsersT") P("UsersT.PersonID > 0")),
	ADD_FILTER_FIELD(327 , "Entered By", "ToDoList.EnteredBy", fboTodo, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT PersonID AS ID, UserName AS Name FROM UsersT WHERE UsersT.PersonID > 0 ORDER BY UserName"), NULL, FILTER_DEPENDS_ON_BASE, NULL, NULL, NULL, NULL, 0, P("PersonID") P("UserName") P("UsersT") P("UsersT.PersonID > 0")),
	ADD_FILTER_FIELD(328 , "Remind Date", "dbo.AsDateNoTime(ToDoList.Remind)", fboTodo, foAll, foEqual, ftDate, NULL, NULL),
	ADD_FILTER_FIELD(329 , "Notes", "ToDoList.Notes", fboTodo, foAll, foBeginsWith, ftText, NULL, NULL),
	ADD_FILTER_FIELD(330 , "Method", "ToDoList.Task", fboTodo, foAll, foBeginsWith, ftText, NULL, NULL),
	ADD_FILTER_FIELD(331 , "Priority", "ToDoList.Priority", fboTodo, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT 1 AS ID, 'High' AS Name UNION SELECT 2 AS ID, 'Medium' AS Name UNION SELECT 3 AS ID, 'Low' AS Name"), NULL),
	ADD_FILTER_FIELD(332 , "Deadline", "dbo.AsDateNoTime(ToDoList.Deadline)", fboTodo, foAll, foEqual, ftDate, NULL, NULL),
	ADD_FILTER_FIELD(333 , "Completed Date", "dbo.AsDateNoTime(ToDoList.Done)", fboTodo, foAll, foEqual, ftDate, NULL, NULL),
	ADD_FILTER_FIELD(334 , "Category", "ToDoList.CategoryID", fboTodo, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID, Description AS Name FROM NoteCatsF ORDER BY Description"), NULL, FILTER_DEPENDS_ON_BASE, NULL, NULL, NULL, NULL, 0, P("ID") P("Description") P("NoteCatsF")),

	ADD_FILTER_FIELD(322 , "Had Procedure Quoted", "QuotedProceduresQ.ProcedureID", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID, Name FROM ProcedureT ORDER BY Name"), NULL, FILTER_DEPENDS_ON_BASE, "(SELECT ServiceT.ProcedureID, LineItemT.PatientID FROM ServiceT INNER JOIN ChargesT ON ServiceT.ID = ChargesT.ServiceID INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID WHERE LineItemT.Type = 11 AND LineItemT.Deleted = 0 AND ServiceT.ProcedureID Is Not Null AND BillsT.Deleted = 0 GROUP BY ServiceT.ProcedureID, LineItemT.PatientID) AS QuotedProceduresQ", "PersonT.ID = QuotedProceduresQ.PatientID"),
	ADD_FILTER_FIELD(323 , "Had Procedure Billed", "BilledProceduresQ.ProcedureID", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID, Name FROM ProcedureT ORDER BY Name"), NULL, FILTER_DEPENDS_ON_BASE, "(SELECT ServiceT.ProcedureID, LineItemT.PatientID FROM ServiceT INNER JOIN ChargesT ON ServiceT.ID = ChargesT.ServiceID INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID WHERE LineItemT.Type = 10 AND LineItemT.Deleted = 0 AND ServiceT.ProcedureID Is Not Null AND BillsT.Deleted = 0 GROUP BY ServiceT.ProcedureID, LineItemT.PatientID) AS BilledProceduresQ", "PersonT.ID = BilledProceduresQ.PatientID"),	
	// (j.jones 2009-10-12 16:10) - PLID 35894 - supported MailSentProcedureT
	ADD_FILTER_FIELD(371 , "Has Photo for Procedure" , "PhotoProceduresQ.ProcedureID", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID, Name FROM ProcedureT ORDER BY Name"), NULL, FILTER_DEPENDS_ON_BASE, "(SELECT MailSentProcedureT.ProcedureID, MailSent.PersonID FROM MailSentProcedureT "
																																																													"INNER JOIN MailSent ON MailSentProcedureT.MailSentID = MailSent.MailID "
																																																													"INNER JOIN PersonT ON MailSent.PersonID = PersonT.ID "
																																																													"INNER JOIN ProcedureT ON MailSentProcedureT.ProcedureID = ProcedureT.ID "
																																																													"GROUP BY PersonID, ProcedureID) AS PhotoProceduresQ", "PersonT.ID = PhotoProceduresQ.PersonID"),

	ADD_FILTER_FIELD(277 , "Has Appointment", "AppointmentsT.ID", fboPerson, foIn|foNotIn, foIn, ftSubFilterEditable, P("SELECT ID, Name, CreatedDate, ModifiedDate, Filter FROM FiltersT WHERE Type = 3 ORDER BY Name"), NULL, FILTER_DEPENDS_ON_BASE, "(SELECT * FROM AppointmentsT WHERE Status <> 4) AppointmentsT ", "PersonT.ID = AppointmentsT.PatientID", NULL, "Appointment Filter", fboAppointment),	

	// (j.jones 2010-08-27 13:42) - PLID 39855 - Added 'Has No Appointment', which sets the new bCheckExists parameter to false
	// so we are saying we want to find patients who do NOT have an appointment in the subfilter. This is different than setting
	// "Has Appointment" to "Not In", and closes the loop on appointment filtering.
	// This filter also intentionally does not have the Not In operator. We can't think of a filter that can be accomplished with
	// Not In that cannot be accomplished with just In, and leaving Not In as a value just adds to the inherent confusion in this filter.
	ADD_FILTER_FIELD(431 , "Has No Appointment", "AppointmentsT.ID", fboPerson, foIn, foIn, ftSubFilterEditable, P("SELECT ID, Name, CreatedDate, ModifiedDate, Filter FROM FiltersT WHERE Type = 3 ORDER BY Name"), NULL, FILTER_DEPENDS_ON_BASE, "(SELECT * FROM AppointmentsT WHERE Status <> 4) AppointmentsT ", "PersonT.ID = AppointmentsT.PatientID", NULL, "Appointment Filter", fboAppointment, NULL, false),	

	ADD_FILTER_FIELD(278 , "Date", "dbo.AsDateNoTime(AppointmentsT.StartTime)", fboAppointment, foAll, foGreaterEqual, ftDate, NULL, NULL, FILTER_DEPENDS_ON_BASE), //see note at the top about converting date
	ADD_FILTER_FIELD(370 , "Input Date", "dbo.AsDateNoTime(AppointmentsT.CreatedDate)", fboAppointment, foAll, foGreaterEqual, ftDate, NULL, NULL, FILTER_DEPENDS_ON_BASE), //see note at the top about converting date
	ADD_FILTER_FIELD(279 , "Start Time", "dbo.AsTimeNoDate(AppointmentsT.StartTime)", fboAppointment, foAll, foEqual, ftTime, NULL, NULL, FILTER_DEPENDS_ON_BASE),
	ADD_FILTER_FIELD(280 , "End Time", "dbo.AsTimeNoDate(AppointmentsT.EndTime)", fboAppointment, foAll, foEqual, ftTime, NULL, NULL, FILTER_DEPENDS_ON_BASE),
	ADD_FILTER_FIELD(392 , "Arrival Time", "dbo.AsTimeNoDate(AppointmentsT.ArrivalTime)", fboAppointment, foAll, foEqual, ftTime, NULL, NULL, FILTER_DEPENDS_ON_BASE),
	ADD_FILTER_FIELD(281 , "Has Purpose", "AppointmentPurposeQ.PurposeID", fboAppointment, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID, Name FROM AptPurposeT ORDER BY Name"), NULL, FILTER_DEPENDS_ON_BASE, "(SELECT AppointmentsT.ID AS AppointmentID, AppointmentPurposeT.PurposeID FROM AppointmentsT LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID) AppointmentPurposeQ", "AppointmentsT.ID = AppointmentPurposeQ.AppointmentID", NULL, NULL, 0, P("ID") P("Name") P("AptPurposeT")),
	ADD_FILTER_FIELD(282 , "Has Purpose Name", "AptPurposeQ.Name", fboAppointment, foAll, foBeginsWith, ftText, NULL, NULL, FILTER_DEPENDS_ON_BASE, "(SELECT AppointmentsT.ID AS AppointmentID, AptPurposeT.Name FROM AppointmentsT LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID LEFT JOIN AptPurposeT ON AppointmentPurposeT.PurposeID = AptPurposeT.ID) AptPurposeQ", "AppointmentsT.ID = AptPurposeQ.AppointmentID"),
	ADD_FILTER_FIELD(283 , "Type", "AppointmentsT.AptTypeID", fboAppointment, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID, Name FROM AptTypeT ORDER BY Name"), NULL, FILTER_DEPENDS_ON_BASE, NULL, NULL, NULL, NULL, 0, P("ID") P("Name") P("AptTypeT")),
	ADD_FILTER_FIELD(284 , "Type Name", "AptTypeT.Name", fboAppointment, foAll, foBeginsWith, ftText, NULL, "AptTypeT WITH(NOLOCK) ON AppointmentsT.AptTypeID = AptTypeT.ID", FILTER_DEPENDS_ON_BASE),
	ADD_FILTER_FIELD(285 , "Notes", "AppointmentsT.Notes", fboAppointment, foAll, foBeginsWith, ftText, NULL, NULL, FILTER_DEPENDS_ON_BASE),
	ADD_FILTER_FIELD(286 , "Has Resource", "AppointmentResourceQ.ResourceID", fboAppointment, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID AS ID, Item AS Name FROM ResourceT WHERE (SELECT MAX(Relevence) FROM UserResourcesT WHERE ResourceID = ResourceT.ID) >= 0 OR (SELECT MAX(Relevence) FROM ResourceViewDetailsT WHERE ResourceID = ResourceT.ID) >= 0 ORDER BY Item"), NULL, FILTER_DEPENDS_ON_BASE, "(SELECT AppointmentsT.ID AS AppointmentID, AppointmentResourceT.ResourceID FROM AppointmentsT LEFT JOIN AppointmentResourceT ON AppointmentsT.ID = AppointmentResourceT.AppointmentID) AppointmentResourceQ", "AppointmentsT.ID = AppointmentResourceQ.AppointmentID", NULL, NULL, 0, P("ID") P("Item") P("ResourceT") P("(SELECT MAX(Relevence) FROM UserResourcesT WHERE ResourceID = ResourceT.ID) >= 0 OR (SELECT MAX(Relevence) FROM ResourceViewDetailsT WHERE ResourceID = ResourceT.ID) >= 0")),
	ADD_FILTER_FIELD(287 , "Has Resource Name", "AptResourceQ.Item", fboAppointment, foAll, foBeginsWith, ftText, NULL,NULL, 286, "(SELECT AppointmentsT.ID AS AppointmentID, AppointmentResourceT.ResourceID, ResourceT.Item FROM AppointmentsT LEFT JOIN AppointmentResourceT ON AppointmentsT.ID = AppointmentResourceT.AppointmentID LEFT JOIN ResourceT ON AppointmentResourceT.ResourceID = ResourceT.ID) AptResourceQ", "AppointmentsT.ID = AptResourceQ.AppointmentID"),
	ADD_FILTER_FIELD(288 , "Appointment Status", "AppointmentsT.ShowState", fboAppointment, foEqual|foNotEqual, foEqual, ftComboSelect, P("SELECT ID, Name FROM AptShowStateT"), NULL, FILTER_DEPENDS_ON_BASE, NULL, NULL, NULL, NULL, 0, P("ID") P("Name") P("AptShowStateT")),
	ADD_FILTER_FIELD(289 , "Location", "AppointmentsT.LocationID", fboAppointment, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID, Name FROM LocationsT WHERE TypeID = 1 ORDER BY Name"), NULL, FILTER_DEPENDS_ON_BASE, NULL, NULL, NULL, NULL, 0, P("ID") P("Name") P("LocationsT") P("TypeID = 1")),
	ADD_FILTER_FIELD(324 , "Age At Appointment", "(CASE WHEN PersonT.BirthDate IS NOT NULL THEN (CASE WHEN PersonT.BirthDate < AppointmentsT.Date THEN YEAR(AppointmentsT.Date-PersonT.BirthDate)-1900 ELSE 0 END) ELSE NULL END)", fboAppointment, foAll, foEqual, ftNumber, NULL, "PersonT ON AppointmentsT.PatientID = PersonT.ID ", FILTER_DEPENDS_ON_BASE),
	ADD_FILTER_FIELD(295 , "Is Event", 
		"CASE WHEN StartTime = EndTime AND DATEPART(Hh, StartTime) = 0 AND DATEPART(Mi, StartTime) = 0 AND DATEPART(Ss, StartTime) = 0 THEN 1 ELSE 0 END", 
		fboAppointment, foEqual, foEqual, ftComboValues, P("Yes") P("1") P("No") P("0") , NULL, FILTER_DEPENDS_ON_BASE),
	ADD_FILTER_FIELD(351 , "Category", "AptTypeT.Category", fboAppointment, foEqual|foNotEqual, foEqual, ftComboValues, P("Non Procedural") P("0") P("Consult") P("1") P("PreOp") P("2") P("Minor Procedure") P("3") P("Surgery") P("4") P("Follow-Up") P("5") P("Other Procedural") P("6") P("Block Time") P("7"), NULL, 284),
	ADD_FILTER_FIELD(369 , "Confirmed", "AppointmentsT.Confirmed", fboAppointment, foEqual|foNotEqual, foEqual, ftComboValues, P("No") P("0") P("Yes") P("1") P("LM") P("2"), NULL, FILTER_DEPENDS_ON_BASE),
	// (j.jones 2009-01-02 13:27) - PLID 32558 - removed the "Cancelled" option from normal usage because it is meaningless,
	// as the Has Appointment filter always excludes Cancelled appts.
	// (d.thompson 2009-03-18) - PLID 33452 - That was an incorrect assessment of the problem.  It is meaningless only for per-patient filters, 
	//	but it is still used in per-appointment filters, and in the appointment selector dialog.  I brought it back to life.
	ADD_FILTER_FIELD(389 , "Cancelled", "CASE WHEN AppointmentsT.Status = 4 THEN 1 ELSE 0 END", fboAppointment, foEqual, foEqual, ftComboValues, P("Yes") P("1") P("No") P("0") , NULL, FILTER_DEPENDS_ON_BASE),
	// (j.gruber 2012-08-07 15:56) - PLID 51925
	ADD_FILTER_FIELD(547 , "Has Insurance Company", "AppointmentInsuranceQ.InsuranceCoID", fboAppointment, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT PersonID as ID, Name FROM InsuranceCoT ORDER BY Name"), NULL, FILTER_DEPENDS_ON_BASE, "(SELECT AppointmentsT.ID AS AppointmentID, InsuredPartyT.InsuranceCoID FROM AppointmentsT LEFT JOIN AppointmentInsuredPartyT ON AppointmentsT.ID = AppointmentInsuredPartyT.AppointmentID LEFT JOIN InsuredPartyT ON AppointmentInsuredPartyT.InsuredPartyID = InsuredPartyT.PersonID) AppointmentInsuranceQ", "AppointmentsT.ID = AppointmentInsuranceQ.AppointmentID", NULL, NULL, 0, P("PersonID") P("Name") P("InsuranceCoT")),
	ADD_FILTER_FIELD(548 , "Has Insurance Responsibility", "AppointmentInsuranceQ.RespTypeID", fboAppointment, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID as ID, TypeName as Name FROM RespTypeT ORDER BY Priority"), NULL, FILTER_DEPENDS_ON_BASE, "(SELECT AppointmentsT.ID AS AppointmentID, InsuredPartyT.RespTypeID FROM AppointmentsT LEFT JOIN AppointmentInsuredPartyT ON AppointmentsT.ID = AppointmentInsuredPartyT.AppointmentID LEFT JOIN InsuredPartyT ON AppointmentInsuredPartyT.InsuredPartyID = InsuredPartyT.PersonID) AppointmentInsuranceQ", "AppointmentsT.ID = AppointmentInsuranceQ.AppointmentID", NULL, NULL, 0, P("ID") P("TypeName") P("RespTypeT")),
	ADD_FILTER_FIELD(549 , "Has Insurance Category", "AppointmentInsuranceQ.CategoryType", fboAppointment, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboValues, P("Medical") P("1") P("Vision") P("2") P("Auto") P("3") P("Workers' Comp") P("4") P("Dental") P("5") P("Study") P("6") P("Letter of Protection") P("7")P("Letter of Agreement") P("8") , NULL, FILTER_DEPENDS_ON_BASE, "(SELECT AppointmentsT.ID AS AppointmentID, RespTypeT.CategoryType FROM AppointmentsT LEFT JOIN AppointmentInsuredPartyT ON AppointmentsT.ID = AppointmentInsuredPartyT.AppointmentID LEFT JOIN InsuredPartyT ON AppointmentInsuredPartyT.InsuredPartyID = InsuredPartyT.PersonID LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID) AppointmentInsuranceQ", "AppointmentsT.ID = AppointmentInsuranceQ.AppointmentID", NULL, NULL, 0),

	ADD_FILTER_FIELD(335 , "< Advanced Filter >", "", fboAppointment, foInvalidOperator, foInvalidOperator, ftAdvanced, NULL, NULL),
	ADD_FILTER_FIELD(336 , "< Subfilter >", "AppointmentsT.ID", fboAppointment, foIn|foNotIn, foIn, ftSubFilterEditable, P("SELECT ID, Name, CreatedDate, ModifiedDate, Filter FROM FiltersT WHERE Type = 3 ORDER BY Name"), NULL, FILTER_DEPENDS_ON_BASE, NULL, NULL, NULL, "Appointment Filter", fboAppointment),

	ADD_FILTER_FIELD(388 , "Has Patient", "PersonT.ID", fboAppointment, foIn|foNotIn, foIn, ftSubFilterEditable, P("SELECT ID, Name, CreatedDate, ModifiedDate, Filter FROM FiltersT WHERE Type = 1 ORDER BY Name"), NULL, FILTER_DEPENDS_ON_BASE, "(SELECT PersonT.* FROM PersonT LEFT JOIN PatientsT ON PatientsT.PersonID = PersonT.ID WHERE PatientsT.CurrentStatus <> 4 AND PersonT.Archived = 0) PersonT ", "PersonT.ID = AppointmentsT.PatientID", NULL, "Patient Filter", fboPerson),

	// (j.armen 2011-07-05 17:01) - PLID 44205 - Added Confirmed By Field
	ADD_FILTER_FIELD(544 , "Confirmed By", "AppointmentsT.ConfirmedBy", fboAppointment, foEqual|foNotEqual|foLike|foBeginsWith|foEndsWith|foContains|foBlank|foNotBlank, foLike, ftText, NULL, NULL, FILTER_DEPENDS_ON_BASE),

	//(c.copits 2011-07-07) PLID 42631 - Need a LW filter for NexWeb EMN completion status.
	ADD_FILTER_FIELD(545, "Has NexWeb EMN", "EMRMasterT.Deleted = 0 AND NexWebDisplayT.Visible = 1 AND (EMRMasterT.PatientCreatedStatus", fboPerson, foEqual, foEqual, ftComboValues,
		P("Any") P("-1 OR EMRMasterT.PatientCreatedStatus IN (1,2)) ") 
		P("Complete") P("2)") 
		P("Incomplete") P("1)"),
		"EMRMasterT ON PersonT.ID = EMRMasterT.PatientID "
		"INNER JOIN EmrTemplateT ON EmrMasterT.TemplateID = EmrTemplateT.ID "
		"INNER JOIN NexWebDisplayT ON EMRTemplateT.ID = NexWebDisplayT.EmrTemplateID",
		NULL),

	ADD_FILTER_FIELD(290 , "Referred Patient", "ReferredPatsQ.ReferredID", fboPerson, foIn|foNotIn, foIn, ftSubFilterEditable, P("SELECT ID, Name, CreatedDate, ModifiedDate, Filter FROM FilterST WHERE Type = 1 ORDER BY Name"), "(SELECT PersonReferring.ID AS ReferringID, PersonReferred.PersonID AS ReferredID FROM PersonT PersonReferring INNER JOIN PatientsT PersonReferred ON PersonReferring.ID = PersonReferred.ReferringPatientID  "
	"UNION SELECT ReferringPhysT.PersonID AS ReferringID, PatientsT.PersonID AS ReferredID FROM ReferringPhysT INNER JOIN PatientsT ON ReferringPhysT.PersonID = PatientsT.DefaultReferringPhyID "
	"UNION SELECT ReferralSourceT.PersonID AS ReferringID, PatientsT.PersonID AS ReferredID FROM ReferralSourceT INNER JOIN PatientsT ON ReferralSourceT.PersonID = PatientsT.ReferralID) ReferredPatsQ ON PersonT.ID = ReferredPatsQ.ReferringID", FILTER_DEPENDS_ON_BASE, NULL, NULL, NULL, "Person Filter", fboPerson),

	// (b.cardillo 2005-07-22 15:41) - PLID 17090 - We used to use a CASE statement to replace a 
	// null age with 0, but that resulted in everyone who had never had ANY balance to behave as 
	// if they had a 0-day old balance, so filtering to get a list of anyone whose oldest balance 
	// was LESS than some number of days was resulting in giving you everyone whose oldest 
	// balance was less than that number of days AND everyone who had never had a balance.  I 
	// made it just use the Age value as is, so that any null values would fail to return that 
	// patient.
	ADD_FILTER_FIELD(337 , "Age Of Oldest Balance (Days)", "BalanceAgeQ.Age", fboPerson, foEquality, foGreater, ftNumber, NULL, 
	"(SELECT BalanceAgeSubQ.PatientID, Max(BalanceAgeSubQ.Age) AS Age FROM (SELECT LineItemT.PatientID, DATEDIFF(dd, BillsT.Date, getdate()) AS Age "
	// (b.cardillo 2005-10-04 15:52) - PLID 16861 - It was joining on ChargeRespT for no apparent 
	// reason, and the join was resulting in duplication of the apply amounts.  I took that join 
	// out, which is something I really should have on my previous check-in because it was partially 
	// to blame for the main problem to begin with.  By leaving it in, only part of the problem was 
	// resolved: it was now getting the correct charge amount in the HAVING clause, but it was still 
	// getting the wrong APPLY amount because the apply records were being outer-joined to the charge 
	// resps.  So by taking that outer join away, it becomes a simple list of charges.  The join to 
	// applies seems worrisome, but since our GROUP BY doesn't include the applies, we end up with 
	// the sum of all applies for each charge, which is exactly what we want.  The query could be 
	// simpler by eliminating the group by, just selecting the patient ID and bill date from 
	// LineItemT where the line item is not deleted, its type is 10 (it's a charge), and the sum of 
	// its ChargeRespT.Amounts is not equal to the sum of its AppliesT.Amounts.  That's exactly what 
	// we're doing, and according to my tests our query perform identically to the simpler one, so 
	// I'm leaving it as is.
	"FROM (ChargesT INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID) LEFT JOIN AppliesT ON ChargesT.ID = AppliesT.DestID "
	// (b.cardillo 2005-07-22 15:26) - PLID 16861 - Changed the GROUP BY clause to go by LineItemT.ID 
	// instead of LineItemT.Delete and LineItemT.Type, since that appears to be the goal and result, 
	// and by explicitly grouping by LineItemT.ID it allows me to use that value in the HAVING clause, 
	// which is really the key change I'm making for this pl item.  I changed the HAVING clause to look 
	// up the ChargeRespT.Amount sum instead of summing the outer ChargeRespT.Amount.  That's because 
	// the outer ChargeRespT is actually joined with the rest of the FROM clause, which means if there 
	// are multiple applies the ChargeRespT amount was getting duplicated for each additional apply, 
	// thus resulting in an over-calculation of the bill amount, which would never be balanced by the 
	// applies.  Referencing an externally (and singly) calculated sum of ChargeRespT.Amount for each 
	// line item, and comparing that to the apply sum makes the calculation correct.
	"WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 10 GROUP BY LineItemT.PatientID, BillsT.Date, LineItemT.ID "
	"HAVING (SELECT Sum(A.Amount) FROM ChargeRespT A WHERE A.ChargeID = LineItemT.ID) <> CASE WHEN Sum(AppliesT.Amount) Is Null THEN 0 ELSE Sum(AppliesT.Amount) END "
	"UNION SELECT LineItemT.PatientID, DATEDIFF(dd, LineItemT.Date, getdate()) "
	//TES 9/27/2005 - PLID 17682 - Added LineItemT.ID to the GROUP BY, otherwise payments for the same patient, date, and amount were getting grouped together.
	//Also, made it so that prepayments don't show as having balances.
	"FROM LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID LEFT JOIN AppliesT AppliesFrom ON LineItemT.ID = AppliesFrom.SourceID LEFT JOIN AppliesT AppliesT ON LineItemT.ID = AppliesT.DestID "
	"WHERE LineItemT.Deleted = 0 AND LineItemT.Type < 10 AND PaymentsT.PrePayment = 0 GROUP BY LineItemT.PatientID, LineItemT.ID, LineItemT.Date, LineItemT.Deleted, LineItemT.Type, LineItemT.Amount "
	// (j.jones 2006-12-12 09:05) - PLID 23832 - the payment totals were never right here because we added
	// the "applied from" amount to the "applied to" amount, when really anything applied to the payment
	// would be a negative. So the payment balance is in face the "applied from" amount MINUS the "applied to" amount
	"HAVING LineItemT.Amount <> CASE WHEN Sum(AppliesFrom.Amount) Is Null THEN 0 ELSE Sum(AppliesFrom.Amount) END - CASE WHEN Sum(AppliesT.Amount) Is Null THEN 0 ELSE Sum(AppliesT.Amount) END) BalanceAgeSubQ GROUP BY BalanceAgeSubQ.PatientID) BalanceAgeQ ON PersonT.ID = BalanceAgeQ.PatientID "
	""),

	// (j.jones 2011-03-08 17:23) - PLID 41847 - added Age Of Oldest Balance (days) for patient and for insurance,
	// these calculate using charge resp dates
	ADD_FILTER_FIELD(469 , "Age Of Oldest Patient Balance (Days)", "OldPatBalanceAgeQ.Age", fboPerson, foEquality, foGreater, ftNumber, NULL, 
	"(SELECT BalanceAgeSubQ.PatientID, Max(BalanceAgeSubQ.Age) AS Age FROM (SELECT LineItemT.PatientID, DATEDIFF(dd, Max(ChargeRespDetailT.Date), getdate()) AS Age "
	"FROM ChargesT "
	"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
	"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
	"INNER JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
	"INNER JOIN ChargeRespDetailT ON ChargeRespT.ID = ChargeRespDetailT.ChargeRespID "
	"LEFT JOIN (SELECT Sum(Amount) AS TotalApplied, DetailID FROM ApplyDetailsT GROUP BY DetailID) AS ApplyDetailsQ ON ChargeRespDetailT.ID = ApplyDetailsQ.DetailID "
	"WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 10 "
	"AND (ChargeRespT.InsuredPartyID Is Null OR ChargeRespT.InsuredPartyID = -1) "
	"AND ChargeRespDetailT.Amount > (CASE WHEN ApplyDetailsQ.TotalApplied Is Null THEN 0 ELSE ApplyDetailsQ.TotalApplied END) "
	"GROUP BY LineItemT.PatientID, BillsT.Date, LineItemT.ID "
	"UNION SELECT LineItemT.PatientID, DATEDIFF(dd, LineItemT.Date, getdate()) AS Age "
	"FROM LineItemT "
	"INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
	"LEFT JOIN AppliesT AppliesFrom ON LineItemT.ID = AppliesFrom.SourceID "
	"LEFT JOIN AppliesT AppliesT ON LineItemT.ID = AppliesT.DestID "
	"WHERE LineItemT.Deleted = 0 AND LineItemT.Type < 10 AND PaymentsT.PrePayment = 0 "
	"AND (PaymentsT.InsuredPartyID Is Null OR PaymentsT.InsuredPartyID = -1) "
	"GROUP BY LineItemT.PatientID, LineItemT.ID, LineItemT.Date, LineItemT.Deleted, LineItemT.Type, LineItemT.Amount "
	"HAVING LineItemT.Amount <> CASE WHEN Sum(AppliesFrom.Amount) Is Null THEN 0 ELSE Sum(AppliesFrom.Amount) END - CASE WHEN Sum(AppliesT.Amount) Is Null THEN 0 ELSE Sum(AppliesT.Amount) END) BalanceAgeSubQ GROUP BY BalanceAgeSubQ.PatientID) OldPatBalanceAgeQ ON PersonT.ID = OldPatBalanceAgeQ.PatientID"),

	ADD_FILTER_FIELD(470 , "Age Of Oldest Insurance Balance (Days)", "OldInsBalanceAgeQ.Age", fboPerson, foEquality, foGreater, ftNumber, NULL, 
	"(SELECT BalanceAgeSubQ.PatientID, Max(BalanceAgeSubQ.Age) AS Age FROM (SELECT LineItemT.PatientID, DATEDIFF(dd, Max(ChargeRespDetailT.Date), getdate()) AS Age "
	"FROM ChargesT "
	"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
	"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
	"INNER JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
	"INNER JOIN ChargeRespDetailT ON ChargeRespT.ID = ChargeRespDetailT.ChargeRespID "
	"LEFT JOIN (SELECT Sum(Amount) AS TotalApplied, DetailID FROM ApplyDetailsT GROUP BY DetailID) AS ApplyDetailsQ ON ChargeRespDetailT.ID = ApplyDetailsQ.DetailID "
	"WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 10 "
	"AND ChargeRespT.InsuredPartyID Is Not Null AND ChargeRespT.InsuredPartyID > 0 "
	"AND ChargeRespDetailT.Amount > (CASE WHEN ApplyDetailsQ.TotalApplied Is Null THEN 0 ELSE ApplyDetailsQ.TotalApplied END) "
	"GROUP BY LineItemT.PatientID, BillsT.Date, LineItemT.ID "
	"UNION SELECT LineItemT.PatientID, DATEDIFF(dd, LineItemT.Date, getdate()) AS Age "
	"FROM LineItemT "
	"INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
	"LEFT JOIN AppliesT AppliesFrom ON LineItemT.ID = AppliesFrom.SourceID "
	"LEFT JOIN AppliesT AppliesT ON LineItemT.ID = AppliesT.DestID "
	"WHERE LineItemT.Deleted = 0 AND LineItemT.Type < 10 AND PaymentsT.PrePayment = 0 "
	"AND PaymentsT.InsuredPartyID Is Not Null AND PaymentsT.InsuredPartyID > 0 "
	"GROUP BY LineItemT.PatientID, LineItemT.ID, LineItemT.Date, LineItemT.Deleted, LineItemT.Type, LineItemT.Amount "
	"HAVING LineItemT.Amount <> CASE WHEN Sum(AppliesFrom.Amount) Is Null THEN 0 ELSE Sum(AppliesFrom.Amount) END - CASE WHEN Sum(AppliesT.Amount) Is Null THEN 0 ELSE Sum(AppliesT.Amount) END) BalanceAgeSubQ GROUP BY BalanceAgeSubQ.PatientID) OldInsBalanceAgeQ ON PersonT.ID = OldInsBalanceAgeQ.PatientID"),

	// (j.jones 2011-03-08 17:23) - PLID 41847 - Added Age Of Most Recent Balance (days) for all, for patient, and for insurance.
	// The "all" filter only looks at the bill date, like the Age Of Oldest Balance filter, while the patient & insurance filters
	// use charge resp. dates.
	ADD_FILTER_FIELD(466 , "Age Of Most Recent Balance (Days)", "RecentBalanceAgeQ.Age", fboPerson, foEquality, foGreater, ftNumber, NULL, 
	"(SELECT BalanceAgeSubQ.PatientID, Min(BalanceAgeSubQ.Age) AS Age FROM (SELECT LineItemT.PatientID, DATEDIFF(dd, BillsT.Date, getdate()) AS Age "
	"FROM (ChargesT INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID) LEFT JOIN AppliesT ON ChargesT.ID = AppliesT.DestID "
	"WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 10 GROUP BY LineItemT.PatientID, BillsT.Date, LineItemT.ID "
	"HAVING (SELECT Sum(A.Amount) FROM ChargeRespT A WHERE A.ChargeID = LineItemT.ID) <> CASE WHEN Sum(AppliesT.Amount) Is Null THEN 0 ELSE Sum(AppliesT.Amount) END "
	"UNION SELECT LineItemT.PatientID, DATEDIFF(dd, LineItemT.Date, getdate()) AS Age "
	"FROM LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID LEFT JOIN AppliesT AppliesFrom ON LineItemT.ID = AppliesFrom.SourceID LEFT JOIN AppliesT AppliesT ON LineItemT.ID = AppliesT.DestID "
	"WHERE LineItemT.Deleted = 0 AND LineItemT.Type < 10 AND PaymentsT.PrePayment = 0 GROUP BY LineItemT.PatientID, LineItemT.ID, LineItemT.Date, LineItemT.Deleted, LineItemT.Type, LineItemT.Amount "
	"HAVING LineItemT.Amount <> CASE WHEN Sum(AppliesFrom.Amount) Is Null THEN 0 ELSE Sum(AppliesFrom.Amount) END - CASE WHEN Sum(AppliesT.Amount) Is Null THEN 0 ELSE Sum(AppliesT.Amount) END) BalanceAgeSubQ GROUP BY BalanceAgeSubQ.PatientID) RecentBalanceAgeQ ON PersonT.ID = RecentBalanceAgeQ.PatientID "
	""),

	ADD_FILTER_FIELD(467 , "Age Of Most Recent Patient Balance (Days)", "RecentPatBalanceAgeQ.Age", fboPerson, foEquality, foGreater, ftNumber, NULL, 
	"(SELECT BalanceAgeSubQ.PatientID, Min(BalanceAgeSubQ.Age) AS Age FROM (SELECT LineItemT.PatientID, DATEDIFF(dd, Min(ChargeRespDetailT.Date), getdate()) AS Age "
	"FROM ChargesT "
	"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
	"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
	"INNER JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
	"INNER JOIN ChargeRespDetailT ON ChargeRespT.ID = ChargeRespDetailT.ChargeRespID "
	"LEFT JOIN (SELECT Sum(Amount) AS TotalApplied, DetailID FROM ApplyDetailsT GROUP BY DetailID) AS ApplyDetailsQ ON ChargeRespDetailT.ID = ApplyDetailsQ.DetailID "
	"WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 10 "
	"AND (ChargeRespT.InsuredPartyID Is Null OR ChargeRespT.InsuredPartyID = -1) "
	"AND ChargeRespDetailT.Amount > (CASE WHEN ApplyDetailsQ.TotalApplied Is Null THEN 0 ELSE ApplyDetailsQ.TotalApplied END) "
	"GROUP BY LineItemT.PatientID, BillsT.Date, LineItemT.ID "
	"UNION SELECT LineItemT.PatientID, DATEDIFF(dd, LineItemT.Date, getdate()) AS Age "
	"FROM LineItemT "
	"INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
	"LEFT JOIN AppliesT AppliesFrom ON LineItemT.ID = AppliesFrom.SourceID "
	"LEFT JOIN AppliesT AppliesT ON LineItemT.ID = AppliesT.DestID "
	"WHERE LineItemT.Deleted = 0 AND LineItemT.Type < 10 AND PaymentsT.PrePayment = 0 "
	"AND (PaymentsT.InsuredPartyID Is Null OR PaymentsT.InsuredPartyID = -1) "
	"GROUP BY LineItemT.PatientID, LineItemT.ID, LineItemT.Date, LineItemT.Deleted, LineItemT.Type, LineItemT.Amount "
	"HAVING LineItemT.Amount <> CASE WHEN Sum(AppliesFrom.Amount) Is Null THEN 0 ELSE Sum(AppliesFrom.Amount) END - CASE WHEN Sum(AppliesT.Amount) Is Null THEN 0 ELSE Sum(AppliesT.Amount) END) BalanceAgeSubQ GROUP BY BalanceAgeSubQ.PatientID) RecentPatBalanceAgeQ ON PersonT.ID = RecentPatBalanceAgeQ.PatientID"),

	ADD_FILTER_FIELD(468 , "Age Of Most Recent Insurance Balance (Days)", "RecentInsBalanceAgeQ.Age", fboPerson, foEquality, foGreater, ftNumber, NULL, 
	"(SELECT BalanceAgeSubQ.PatientID, Min(BalanceAgeSubQ.Age) AS Age FROM (SELECT LineItemT.PatientID, DATEDIFF(dd, Min(ChargeRespDetailT.Date), getdate()) AS Age "
	"FROM ChargesT "
	"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
	"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
	"INNER JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
	"INNER JOIN ChargeRespDetailT ON ChargeRespT.ID = ChargeRespDetailT.ChargeRespID "
	"LEFT JOIN (SELECT Sum(Amount) AS TotalApplied, DetailID FROM ApplyDetailsT GROUP BY DetailID) AS ApplyDetailsQ ON ChargeRespDetailT.ID = ApplyDetailsQ.DetailID "
	"WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 10 "
	"AND ChargeRespT.InsuredPartyID Is Not Null AND ChargeRespT.InsuredPartyID > 0 "
	"AND ChargeRespDetailT.Amount > (CASE WHEN ApplyDetailsQ.TotalApplied Is Null THEN 0 ELSE ApplyDetailsQ.TotalApplied END) "
	"GROUP BY LineItemT.PatientID, BillsT.Date, LineItemT.ID "
	"UNION SELECT LineItemT.PatientID, DATEDIFF(dd, LineItemT.Date, getdate()) AS Age "
	"FROM LineItemT "
	"INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
	"LEFT JOIN AppliesT AppliesFrom ON LineItemT.ID = AppliesFrom.SourceID "
	"LEFT JOIN AppliesT AppliesT ON LineItemT.ID = AppliesT.DestID "
	"WHERE LineItemT.Deleted = 0 AND LineItemT.Type < 10 AND PaymentsT.PrePayment = 0 "
	"AND PaymentsT.InsuredPartyID Is Not Null AND PaymentsT.InsuredPartyID > 0 "
	"GROUP BY LineItemT.PatientID, LineItemT.ID, LineItemT.Date, LineItemT.Deleted, LineItemT.Type, LineItemT.Amount "
	"HAVING LineItemT.Amount <> CASE WHEN Sum(AppliesFrom.Amount) Is Null THEN 0 ELSE Sum(AppliesFrom.Amount) END - CASE WHEN Sum(AppliesT.Amount) Is Null THEN 0 ELSE Sum(AppliesT.Amount) END) BalanceAgeSubQ GROUP BY BalanceAgeSubQ.PatientID) RecentInsBalanceAgeQ ON PersonT.ID = RecentInsBalanceAgeQ.PatientID"),

	// (j.jones 2009-10-19 10:58) - PLID 35994 - race and ethnicity are now separate fields
	// (b.spivey, May 21, 2013) - PLID 56868 - Changed the filter fields for the letter writing module.
	// (b.spivey, June 13, 2013) - PLID 56868 - Didn't properly handle "is blank" or "is not blank". 
	ADD_FILTER_FIELD(354, "Race", "PersonRaceT.RaceID", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID, Name FROM RaceT ORDER BY Name ASC"), NULL, FILTER_DEPENDS_ON_BASE,  "PatientsT LEFT JOIN PersonRaceT ON PatientsT.PersonID = PersonRaceT.PersonID ", "PersonT.ID  = PatientsT.PersonID ", NULL, NULL, 0, P("ID") P("Name") P("RaceT")),
	ADD_FILTER_FIELD(415, "CDC Race", "CASE WHEN RaceT.RaceCodeID IS NULL THEN '' ELSE RaceT.RaceCodeID END ", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID, OfficialRaceName AS Name FROM RaceCodesT ORDER BY OfficialRaceName ASC"), NULL, FILTER_DEPENDS_ON_BASE, "PatientsT LEFT JOIN PersonRaceT ON PatientsT.PersonID = PersonRaceT.PersonID LEFT JOIN RaceT ON PersonRaceT.RaceID = RaceT.ID ", "PersonT.ID = PatientsT.PersonID ", NULL, NULL, 0, P("ID") P("Name") P("RaceCodesT")),	
	// (d.thompson 2012-08-09) - PLID 52045 - Reworked table structure
	ADD_FILTER_FIELD(416, "CDC Ethnicity", "PersonT.Ethnicity", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID, Name FROM EthnicityT ORDER BY Name ASC"), NULL, FILTER_DEPENDS_ON_BASE, NULL, NULL, NULL, NULL, 0, P("ID") P("Name") P("EthnicityT")),
	//(e.lally 2011-06-17) PLID 43992 - Language filter
	// (d.thompson 2012-08-14) - PLID 52046 - Reworked language structure
	ADD_FILTER_FIELD(543, "Language", "LanguageT.ID", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID, Name FROM LanguageT ORDER BY Name ASC"), "LanguageT ON PersonT.LanguageID = LanguageT.ID", FILTER_DEPENDS_ON_BASE),

	//Payment filters.
	ADD_FILTER_FIELD(372, "Has Payment", "PaymentsT.ID", fboPerson, foIn|foNotIn, foIn, ftSubFilterEditable, P("SELECT ID, Name, CreatedDate, ModifiedDate, Filter FROM FiltersT WHERE Type = 6 ORDER BY Name"), NULL, FILTER_DEPENDS_ON_BASE, "PaymentsT INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID", "PersonT.ID = LineItemT.PatientID", NULL, "Payment Filter", fboPayment),
	ADD_FILTER_FIELD(373, "Type", "LineItemT.Type", fboPayment, foEqual|foNotEqual, foEqual, ftComboValues, P("Payment") P("1") P("Adjustment") P("2") P("Refund") P("3"), NULL, FILTER_DEPENDS_ON_BASE),
	ADD_FILTER_FIELD(374, "Amount", "LineItemT.Amount", fboPayment, foEquality, foGreater, ftCurrency, NULL, NULL, FILTER_DEPENDS_ON_BASE),
	ADD_FILTER_FIELD(375, "Description", "LineItemT.Description", fboPayment, foAll, foBeginsWith, ftText, NULL, NULL, FILTER_DEPENDS_ON_BASE),
	ADD_FILTER_FIELD(376, "Date", "LineItemT.Date", fboPayment, foAll, foEqual, ftDate, NULL, NULL, FILTER_DEPENDS_ON_BASE),
	ADD_FILTER_FIELD(377, "Input Date", "LineItemT.InputDate", fboPayment, foAll, foEqual, ftDate, NULL, NULL, FILTER_DEPENDS_ON_BASE),
	ADD_FILTER_FIELD(378, "Entered By", "LineItemT.InputName", fboPayment, foAll, foBeginsWith, ftText, NULL, NULL, FILTER_DEPENDS_ON_BASE),
	ADD_FILTER_FIELD(379, "Location", "LineItemT.LocationID", fboPayment, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID, Name From LocationsT WHERE TypeID = 1 ORDER BY Name ASC"), NULL, FILTER_DEPENDS_ON_BASE, NULL, NULL, NULL, NULL, 0, P("ID") P("Name") P("LocationsT") P("TypeID = 1")),
	ADD_FILTER_FIELD(380, "Provider", "CASE WHEN PaymentsT.ProviderID = -1 THEN Null ELSE PaymentsT.ProviderID END", fboPayment, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID, Last + ', ' + First + ' ' + Middle AS Name FROM PersonT WHERE ID IN (SELECT PersonID FROM ProvidersT) ORDER BY Last, First, Middle ASC"), NULL, FILTER_DEPENDS_ON_BASE, NULL, NULL, NULL, NULL, 0, P("ID") P("Last + ', ' + First + ' ' + Middle") P("PersonT") P("ID IN (SELECT PersonID FROM ProvidersT)")),
	ADD_FILTER_FIELD(381, "Category", "PaymentsT.PaymentGroupID", fboPayment, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID, GroupName AS Name FROM PaymentGroupsT ORDER BY GroupName ASC"), NULL, FILTER_DEPENDS_ON_BASE, NULL, NULL, NULL, NULL, 0, P("ID") P("GroupName") P("PaymentGroupsT")),
	ADD_FILTER_FIELD(382, "Payor", "CASE WHEN PaymentsT.InsuredPartyID = -1 THEN -1 ELSE (SELECT InsuranceCoID FROM InsuredPartyT WHERE InsuredPartyT.PersonID = PaymentsT.InsuredPartyID) END", fboPayment, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT -1 AS ID, '<Patient>' AS Name UNION SELECT PersonID, Name FROM InsuranceCoT"), NULL, FILTER_DEPENDS_ON_BASE),
	ADD_FILTER_FIELD(383, "Deposited On", "PaymentsT.DepositDate", fboPayment, foAll, foGreaterEqual, ftDate, NULL, NULL, FILTER_DEPENDS_ON_BASE),
	ADD_FILTER_FIELD(384, "Is PrePayment", "PaymentsT.PrePayment", fboPayment, foEqual|foNotEqual, foEqual, ftComboValues, P("True") P("1") P("False") P("0"), NULL, FILTER_DEPENDS_ON_BASE),
	// (r.gonet 2015-09-18) - PLID 65870 - Added PayMethod 10 (Gift Certificate Refunds)
	ADD_FILTER_FIELD(385, "Method of Payment", "CASE WHEN PaymentsT.PayMethod = 7 THEN 1 WHEN PaymentsT.PayMethod = 8 THEN 2 WHEN PaymentsT.PayMethod = 9 THEN 3 WHEN PaymentsT.PayMethod = 10 THEN 4 ELSE PaymentsT.PayMethod END", fboPayment, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboValues, P("Cash") P("1") P("Check") P("2") P("Charge") P("3") P("Gift Certificate") P("4"), NULL, FILTER_DEPENDS_ON_BASE),
	ADD_FILTER_FIELD(386 , "< Advanced Filter >", "", fboPayment, foInvalidOperator, foInvalidOperator, ftAdvanced, NULL, NULL),
	//(e.lally 2008-06-04) PLID 27908 - Fixed the select statement to have valid syntax
	ADD_FILTER_FIELD(387 , "< Subfilter >", "PaymentsT.ID", fboPayment, foIn|foNotIn, foIn, ftSubFilterEditable, P("SELECT ID, Name, CreatedDate, ModifiedDate, Filter FROM FiltersT WHERE Type = 6 ORDER BY Name"), NULL, FILTER_DEPENDS_ON_BASE, NULL, NULL, NULL, "Payment Filter", fboPayment),

	//Medications and Allergies
	//(e.lally 2008-09-16) PLID 19569 - Added filter for patients with allergies and patients with current medications
	// (j.jones 2011-07-28 09:25) - PLID 44718 - fixed inactive filter, it had broken SQL syntax
	ADD_FILTER_FIELD(409, "Allergy"	, "PatientAllergyT.AllergyID"  , fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT AllergyT.ID AS ID, EmrDataT.Data AS Name FROM AllergyT INNER JOIN EmrDataT ON EmrDataT.ID = AllergyT.EmrDataID WHERE EmrDataT.Inactive = 0 ORDER BY EmrDataT.Data ASC"), "PatientAllergyT WITH(NOLOCK) ON PersonT.ID = PatientAllergyT.PersonID", FILTER_DEPENDS_ON_BASE, NULL, NULL, NULL, NULL, 0, P("AllergyT.ID") P("Data") P("AllergyT INNER JOIN EmrDataT ON EmrDataT.ID = AllergyT.EmrDataID") P("EmrDataT.Inactive =0")),
	// (j.jones 2010-01-13 11:39) - PLID 36851 - added filter for allergy notes
	ADD_FILTER_FIELD(418, "Has Allergy Note", "PatientAllergyT.Description", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank|foBeginsWith|foEndsWith|foContains, foContains, ftText, NULL, "PatientAllergyT WITH(NOLOCK) ON PersonT.ID = PatientAllergyT.PersonID"),
	// (r.gonet 2010-09-09 13:42) - PLID 40679 - Changed the Current Medication query to check for Discontinued status
	ADD_FILTER_FIELD(410, "Current Medication"	, "CurrentPatientMedsT.MedicationID"  , fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT DrugList.ID AS ID, EmrDataT.Data AS Name FROM DrugList INNER JOIN EmrDataT ON EmrDataT.ID = DrugList.EmrDataID WHERE EmrDataT.Inactive =0 ORDER BY EmrDataT.Data ASC"), "CurrentPatientMedsT WITH(NOLOCK) ON PersonT.ID = CurrentPatientMedsT.PatientID AND CurrentPatientMedsT.Discontinued <> 1", FILTER_DEPENDS_ON_BASE, NULL, NULL, NULL, NULL, 0, P("DrugList.ID") P("Data") P("DrugList INNER JOIN EmrDataT ON EmrDataT.ID = DrugList.EmrDataID") P("EmrDataT.Inactive =0")),
	// (j.jones 2008-11-18 10:55) - PLID 28543 - added a filter for prescribed medications
	ADD_FILTER_FIELD(411, "Prescribed Medication", "PatientMedicationsQ.MedicationID", fboPerson, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT DrugList.ID AS ID, EmrDataT.Data AS Name FROM DrugList INNER JOIN EmrDataT ON EmrDataT.ID = DrugList.EmrDataID WHERE EmrDataT.Inactive =0 ORDER BY EmrDataT.Data ASC"), "(SELECT PatientID, MedicationID FROM PatientMedications WITH(NOLOCK) WHERE Deleted = 0) AS PatientMedicationsQ ON PersonT.ID = PatientMedicationsQ.PatientID", FILTER_DEPENDS_ON_BASE, NULL, NULL, NULL, NULL, 0, P("DrugList.ID") P("Data") P("DrugList INNER JOIN EmrDataT ON EmrDataT.ID = DrugList.EmrDataID") P("EmrDataT.Inactive =0")),

	//TES 9/9/2010 - PLID 40457 - Lab Result filters.
	ADD_FILTER_FIELD(432, "Has Lab Result", "LabResultsT.ResultID", fboPerson, foIn|foNotIn, foIn, ftSubFilterEditable, P("SELECT ID, Name, CreatedDate, ModifiedDate, Filter FROM FiltersT WHERE Type = 7 ORDER BY Name"), NULL, FILTER_DEPENDS_ON_BASE, "LabResultsT INNER JOIN LabsT ON LabResultsT.LabID = LabsT.ID", "PersonT.ID = LabsT.PatientID", NULL, "Lab Result Filter", fboLabResult),
	ADD_FILTER_FIELD(433, "Name", "LabResultsT.Name", fboLabResult, foAll, foBeginsWith, ftText, NULL, NULL, FILTER_DEPENDS_ON_BASE),
	ADD_FILTER_FIELD(434, "Date Received", "dbo.AsDateNoTime(LabResultsT.DateReceived)", fboLabResult, foAll, foEqual, ftDate, NULL, NULL, FILTER_DEPENDS_ON_BASE),
	ADD_FILTER_FIELD(435, "Slide #", "LabResultsT.SlideTextID", fboLabResult, foAll, foBeginsWith, ftText, NULL, NULL, FILTER_DEPENDS_ON_BASE),
	ADD_FILTER_FIELD(436, "Diagnosis", "LabResultsT.DiagnosisDesc", fboLabResult, foBlank | foNotBlank | foLike | foNotLike | foBeginsWith | foEndsWith | foContains, foBeginsWith, ftText, NULL, NULL, FILTER_DEPENDS_ON_BASE),
	ADD_FILTER_FIELD(437, "Microscopic Description", "LabResultsT.ClinicalDiagnosisDesc", fboLabResult, foBlank | foNotBlank | foLike | foNotLike | foBeginsWith | foEndsWith | foContains, foBeginsWith, ftText, NULL, NULL, FILTER_DEPENDS_ON_BASE),
	ADD_FILTER_FIELD(438, "Flag", "LabResultsT.FlagID", fboLabResult, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID, Name FROM LabResultFlagsT ORDER BY Name ASC"), NULL, FILTER_DEPENDS_ON_BASE, NULL, NULL, NULL, NULL, 0, P("ID") P("Name") P("LabResultFlagsT")),
	ADD_FILTER_FIELD(439, "Value", "LabResultsT.Value", fboLabResult, foBlank | foNotBlank | foLike | foNotLike | foBeginsWith | foEndsWith | foContains, foBeginsWith, ftText, NULL, NULL, FILTER_DEPENDS_ON_BASE),
	ADD_FILTER_FIELD(440, "Reference", "LabResultsT.Reference", fboLabResult, foAll, foBeginsWith, ftText, NULL, NULL, FILTER_DEPENDS_ON_BASE),
	ADD_FILTER_FIELD(441, "Status", "LabResultsT.StatusID", fboLabResult, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID, Description AS Name FROM LabResultStatusT ORDER BY Description ASC"), NULL, FILTER_DEPENDS_ON_BASE, NULL, NULL, NULL, NULL, 0, P("ID") P("Description") P("LabResultStatusT")),
	ADD_FILTER_FIELD(442, "Comments", "LabResultsT.Comments", fboLabResult, foAll, foBeginsWith, ftText, NULL, NULL, FILTER_DEPENDS_ON_BASE),
	ADD_FILTER_FIELD(443, "Units", "LabResultsT.Units", fboLabResult, foAll, foBeginsWith, ftText, NULL, NULL, FILTER_DEPENDS_ON_BASE),
	ADD_FILTER_FIELD(444, "Acknowledged By", "LabResultsT.AcknowledgedUserID", fboLabResult, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT PersonID AS ID, UserName AS Name FROM UsersT ORDER BY UserName ASC"), NULL, FILTER_DEPENDS_ON_BASE, NULL, NULL, NULL, NULL, 0, P("PersonID") P("UserName") P("UsersT")),
	ADD_FILTER_FIELD(445, "Acknowledged On", "dbo.AsDateNoTime(LabResultsT.AcknowledgedDate)", fboLabResult, foAll, foEqual, ftDate, NULL, NULL, FILTER_DEPENDS_ON_BASE),
	ADD_FILTER_FIELD(446, "LOINC", "LabResultsT.LOINC", fboLabResult, foAll, foBeginsWith, ftText, NULL, NULL, FILTER_DEPENDS_ON_BASE),
	//TES 9/9/2010 - PLID 40457 - We put the "+'e0'" in the isnumeric() function to workaround the fact that it's imperfect,
	// see http://msdn.microsoft.com/en-us/library/ms186272.aspx
	ADD_FILTER_FIELD(447, "Value (Numeric)", "convert(float,CASE WHEN isnumeric(convert(nvarchar(1000), LabResultsT.Value)+'e0')=1 THEN convert(nvarchar(1000),  LabResultsT.Value) ELSE NULL END)", fboLabResult, foAllGreater|foAllLess|foEqual|foNotEqual, foEqual, ftNumber, NULL, NULL, FILTER_DEPENDS_ON_BASE),

	//TES 9/9/2010 - PLID 40470 - Has Immunization filters
	ADD_FILTER_FIELD(448, "Has Immunization", "PatientImmunizationsT.ID", fboPerson, foIn|foNotIn, foIn, ftSubFilterEditable, P("SELECT ID, Name, CreatedDate, ModifiedDate, Filter FROM FiltersT WHERE Type = 8 ORDER BY Name"), NULL, FILTER_DEPENDS_ON_BASE, "PatientImmunizationsT", "PersonT.ID = PatientImmunizationsT.PersonID", NULL, "Immunization Filter", fboImmunization),
	ADD_FILTER_FIELD(449, "Type", "PatientImmunizationsT.ImmunizationID", fboImmunization, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID, Type AS Name FROM ImmunizationsT ORDER BY Type ASC"), NULL, FILTER_DEPENDS_ON_BASE, NULL, NULL, NULL, NULL, 0, P("ID") P("Type") P("ImmunizationsT")),
	ADD_FILTER_FIELD(450, "Dosage", "PatientImmunizationsT.Dosage", fboImmunization, foAll, foBeginsWith, ftText, NULL, NULL, FILTER_DEPENDS_ON_BASE),
	ADD_FILTER_FIELD(451, "Date Administered", "dbo.AsDateNoTime(PatientImmunizationsT.DateAdministered)", fboImmunization, foAll, foGreaterEqual, ftDate, NULL, NULL, FILTER_DEPENDS_ON_BASE),
	ADD_FILTER_FIELD(452, "Lot Number", "PatientImmunizationsT.LotNumber", fboImmunization, foAll, foBeginsWith, ftText, NULL, NULL, FILTER_DEPENDS_ON_BASE),
	ADD_FILTER_FIELD(453, "Expiration Date", "dbo.AsDateNoTime(PatientImmunizationsT.ExpirationDate)", fboImmunization, foAll, foGreaterEqual, ftDate, NULL, NULL, FILTER_DEPENDS_ON_BASE),
	ADD_FILTER_FIELD(454, "Manufacturer", "PatientImmunizationsT.Manufacturer", fboImmunization, foAll, foBeginsWith, ftText, NULL, NULL, FILTER_DEPENDS_ON_BASE),
	ADD_FILTER_FIELD(455, "Route", "PatientImmunizationsT.RouteID", fboImmunization, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID, Name FROM ImmunizationRoutesT ORDER BY Name ASC"), NULL, FILTER_DEPENDS_ON_BASE, NULL, NULL, NULL, NULL, 0, P("ID") P("Name") P("ImmunizationRoutesT")),
	ADD_FILTER_FIELD(456, "Site", "PatientImmunizationsT.SiteID", fboImmunization, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID, Name FROM ImmunizationSitesT ORDER BY Name ASC"), NULL, FILTER_DEPENDS_ON_BASE, NULL, NULL, NULL, NULL, 0, P("ID") P("Name") P("ImmunizationSitesT")),
	ADD_FILTER_FIELD(457, "Allergy/Adverse Reaction", "PatientImmunizationsT.Reaction", fboImmunization, foAll, foBeginsWith, ftText, NULL, NULL, FILTER_DEPENDS_ON_BASE),

	//TES 9/9/2010 - PLID 40471 - EMR Problem filters.
	ADD_FILTER_FIELD(458, "Has EMR Problem", "EmrProblemsT.ID", fboPerson, foIn|foNotIn, foIn, ftSubFilterEditable, P("SELECT ID, Name, CreatedDate, ModifiedDate, Filter FROM FiltersT WHERE Type = 9 ORDER BY Name"), NULL, FILTER_DEPENDS_ON_BASE, "EmrProblemsT", "PersonT.ID = EmrProblemsT.PatientID", NULL, "EMR Problem Filter", fboEmrProblem),
	ADD_FILTER_FIELD(459, "Description", "EmrProblemsT.Description", fboEmrProblem, foAll, foBeginsWith, ftText, NULL, NULL, FILTER_DEPENDS_ON_BASE),
	ADD_FILTER_FIELD(460, "Input Date", "dbo.AsDateNoTime(EmrProblemsT.EnteredDate)", fboEmrProblem, foAll, foGreaterEqual, ftDate, NULL, NULL, FILTER_DEPENDS_ON_BASE),
	ADD_FILTER_FIELD(461, "Status", "EmrProblemsT.StatusID", fboEmrProblem, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID, Name FROM EmrProblemStatusT ORDER BY Name ASC"), NULL, FILTER_DEPENDS_ON_BASE, NULL, NULL, NULL, NULL, 0, P("ID") P("Name") P("EmrProblemStatusT")),
	ADD_FILTER_FIELD(462, "Onset Date", "dbo.AsDateNoTime(EmrProblemsT.OnsetDate)", fboEmrProblem, foAll, foGreaterEqual, ftDate, NULL, NULL, FILTER_DEPENDS_ON_BASE),
	ADD_FILTER_FIELD(463, "Chronicity", "EmrProblemsT.ChronicityID", fboEmrProblem, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID, Name FROM EmrProblemChronicityT ORDER BY Name ASC"), NULL, FILTER_DEPENDS_ON_BASE, NULL, NULL, NULL, NULL, 0, P("ID") P("Name") P("EmrProblemChronicityT")),
	// (j.jones 2014-02-27 08:52) - PLID 60771 - this filters only on ICD-9s in the managed list
	ADD_FILTER_FIELD(464, "ICD-9 Code", "EmrProblemsT.DiagCodeID", fboEmrProblem, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID, CodeNumber + ' - ' + CodeDesc AS Name FROM DiagCodes WHERE ICD10 = 0 ORDER BY CodeNumber + ' - ' + CodeDesc ASC"), NULL, FILTER_DEPENDS_ON_BASE, NULL, NULL, NULL, NULL, 0, P("ID") P("CodeNumber + ' - ' + CodeDesc") P("DiagCodes") P("ICD10 = 0")),
	// (j.jones 2014-02-27 08:52) - PLID 60771 - added ICD-10 code, which filters only on ICD-10s in the managed list
	ADD_FILTER_FIELD(2026, "ICD-10 Code", "EmrProblemsT.DiagCodeID_ICD10", fboEmrProblem, foEqual|foNotEqual|foBlank|foNotBlank, foEqual, ftComboSelect, P("SELECT ID, CodeNumber + ' - ' + CodeDesc AS Name FROM DiagCodes WHERE ICD10 = 1 ORDER BY CodeNumber + ' - ' + CodeDesc ASC"), NULL, FILTER_DEPENDS_ON_BASE, NULL, NULL, NULL, NULL, 0, P("ID") P("CodeNumber + ' - ' + CodeDesc") P("DiagCodes") P("ICD10 = 1")),
	
	//(r.wilson 10/21/2013) PLID 59121 - PatientLists
	ADD_FILTER_FIELD(1001, "Has Medication", "CurrentPatientMedsT.MedicationID", fboPerson, foIn|foNotIn, foIn, ftSubFilterEditable, P(" SELECT ID, Name, CreatedDate, ModifiedDate, Filter FROM FiltersT WHERE Type = 10 ORDER BY Name "), NULL, FILTER_DEPENDS_ON_BASE, "CurrentPatientMedsT INNER JOIN DrugList ON CurrentPatientMedsT.MedicationID = DrugList.ID INNER JOIN EmrDataT ON DrugList.EmrDataID = EmrDataT.ID" ,"PersonT.ID = CurrentPatientMedsT.PatientID", NULL, "Medication Filter", fboMedication),	
	ADD_FILTER_FIELD(1002, "Name", "CurrentPatientMedsT.Data", fboMedication, foAll, foEqual, ftText, NULL , NULL, FILTER_DEPENDS_ON_BASE),			
	//ADD_FILTER_FIELD(1003, "Prescription Date", "dbo.AsDateNoTime(DrugList.PrescriptionDate)", fboMedication, foAll, foEqual, ftDate, NULL, NULL, FILTER_DEPENDS_ON_BASE),
	ADD_FILTER_FIELD(1003, "Start Date", "dbo.AsDateNoTime(CurrentPatientMedsT.StartDate)", fboMedication, foAll, foEqual, ftDate, NULL, NULL, FILTER_DEPENDS_ON_BASE),
	ADD_FILTER_FIELD(1004, "SIG", "CurrentPatientMedsT.PatientInstructions", fboMedication, foAll, foContains, ftText, NULL , NULL, FILTER_DEPENDS_ON_BASE),			
	ADD_FILTER_FIELD(1005, "NDC", "CurrentPatientMedsT.NDCNumber", fboMedication, foAll, foContains, ftText, NULL , NULL, FILTER_DEPENDS_ON_BASE),	
	ADD_FILTER_FIELD(1006, "Discontinued", "CurrentPatientMedsT.Discontinued", fboMedication, foEqual|foNotEqual, foEqual, ftComboValues, P("Not Discontinued")P("0")P("Discontinued")P("1"), NULL, FILTER_DEPENDS_ON_BASE, NULL, NULL, NULL, NULL, fboLab, NULL),

	//(r.wilson 10/21/2013) PLID 59121 - PatientLists
	ADD_FILTER_FIELD(1501, "Has Allergy", "PatientAllergyT.ID", fboPerson, foIn|foNotIn, foIn, ftSubFilterEditable, P("SELECT ID, Name, CreatedDate, ModifiedDate, Filter FROM FiltersT WHERE Type = 11 ORDER BY Name "), NULL, FILTER_DEPENDS_ON_BASE, "PatientAllergyT INNER JOIN AllergyT ON PatientAllergyT.AllergyID = AllergyT.ID INNER JOIN EmrDataT ON AllergyT.EmrDataID = EmrDataT.ID" , "PersonT.ID = PatientAllergyT.PersonID", NULL, "Allergy Filter", fboAllergy),
	ADD_FILTER_FIELD(1502, "Entered Date", "dbo.AsDateNoTime(PatientAllergyT.EnteredDate)", fboAllergy, foAll, foEqual, ftDate, NULL, NULL, FILTER_DEPENDS_ON_BASE),
	ADD_FILTER_FIELD(1503, "Name", "PatientAllergyT.Data", fboAllergy, foAll, foEqual, ftText, NULL, NULL, FILTER_DEPENDS_ON_BASE),
	ADD_FILTER_FIELD(1504, "Notes", "PatientAllergyT.Description", fboAllergy, foAll, foBeginsWith, ftText, NULL, NULL, FILTER_DEPENDS_ON_BASE),

	//(r.wilson 10/21/2013) PLID 59121 - PatientLists
	ADD_FILTER_FIELD(2001, "Has Lab", "LabsT.ID", fboPerson, foIn|foNotIn, foIn, ftSubFilterEditable, P(" SELECT ID, Name, CreatedDate, ModifiedDate, Filter FROM FiltersT WHERE Type = 12 ORDER BY Name "), NULL , FILTER_DEPENDS_ON_BASE, "LabsT", "PersonT.ID = LabsT.PatientID", NULL, "Lab Filter", fboLab),
	ADD_FILTER_FIELD(2002, "Biopsy Date", "dbo.AsDateNoTime(LabsT.BiopsyDate)", fboLab, foAll, foEqual, ftDate, NULL, NULL, FILTER_DEPENDS_ON_BASE),
	ADD_FILTER_FIELD(2003, "Input Date", "dbo.AsDateNoTime(LabsT.InputDate)", fboLab, foAll, foEqual, ftDate, NULL, NULL, FILTER_DEPENDS_ON_BASE),
	ADD_FILTER_FIELD(2004, "Lab Procedure", "LabsT.LabProcedureID ", fboLab, foEqual|foNotEqual, foEqual, ftComboSelect, P("SELECT ID, Name FROM LabProceduresT ORDER BY Name ASC"), NULL, FILTER_DEPENDS_ON_BASE, NULL, NULL, NULL, NULL, 0, P("ID") P("Name") P("LabProceduresT")),
	ADD_FILTER_FIELD(2005, "Form Number", "LabsT.FormNumberTextID", fboLab, foAll, foEqual, ftText, NULL, NULL, FILTER_DEPENDS_ON_BASE),
	ADD_FILTER_FIELD(2006, "Specimen Label", "LabsT.Specimen", fboLab, foAll, foEqual, ftText, NULL, NULL, FILTER_DEPENDS_ON_BASE),
	ADD_FILTER_FIELD(2007, "Location", "LabsT.LocationID ", fboLab, foEqual|foNotEqual, foEqual, ftComboSelect, P("SELECT ID, Name FROM LocationsT WHERE TypeID = 1 ORDER BY Name ASC"), NULL, FILTER_DEPENDS_ON_BASE, NULL, NULL, NULL, NULL, 0, P("ID") P("Name") P("LocationsT")),
	ADD_FILTER_FIELD(2008, "Lab Location", "LabsT.LabLocationID ", fboLab, foEqual|foNotEqual, foEqual, ftComboSelect, P("SELECT ID, Name FROM LocationsT WHERE TypeID = 2 ORDER BY Name ASC"), NULL, FILTER_DEPENDS_ON_BASE, NULL, NULL, NULL, NULL, 0, P("ID") P("Name") P("LocationsT")),	
	ADD_FILTER_FIELD(2009, "Anatomic Side", "LabsT.AnatomySide", fboLab, foEqual|foNotEqual, foEqual, ftComboValues, P("No Side Chosen")P("0")P("Left")P("1")P("Right")P("2"), NULL, FILTER_DEPENDS_ON_BASE, NULL, NULL, NULL, NULL, NULL, NULL),
	ADD_FILTER_FIELD(2010, "Anatomic Location", "LabsT.AnatomyID", fboLab, foEqual|foNotEqual, foEqual, ftComboSelect, P("SELECT ID, Description AS Name FROM LabAnatomyT ORDER BY Description ASC"), /*P("LabAnatomyT.ID = LabsT.AnatomyID"),*/ NULL, FILTER_DEPENDS_ON_BASE, NULL, NULL, NULL, NULL, 0, P("ID") P("Description") P("LabAnatomyT")),
	ADD_FILTER_FIELD(2011, "Anatomic Qualifier", "LabsT.AnatomyQualifierID", fboLab, foEqual|foNotEqual, foEqual, ftComboSelect, P("SELECT ID, Name FROM AnatomyQualifiersT ORDER BY Name ASC"), NULL, FILTER_DEPENDS_ON_BASE, NULL, NULL, NULL, NULL, 0, P("ID") P("Name") P("AnatomyQualifiersT")),
	ADD_FILTER_FIELD(2012, "To Be Ordered", "LabsT.ToBeOrdered", fboLab, foAll, foEqual, ftText, NULL, NULL, FILTER_DEPENDS_ON_BASE),
	ADD_FILTER_FIELD(2013, "Initial Diagnosis", "LabsT.InitialDiagnosis", fboLab, foAll, foEqual, ftText, NULL, NULL, FILTER_DEPENDS_ON_BASE),
	ADD_FILTER_FIELD(2014, "Instructions", "LabsT.Instructions", fboLab, foAll, foEqual, ftText, NULL, NULL, FILTER_DEPENDS_ON_BASE),
	ADD_FILTER_FIELD(2015, "Comments", "LabsT.ClinicalData", fboLab, foAll, foEqual, ftText, NULL, NULL, FILTER_DEPENDS_ON_BASE), // AKA Clinical Data
	ADD_FILTER_FIELD(2016, "Biopsy Type", "LabsT.BiopsyTypeID", fboLab, foEqual|foNotEqual, foEqual, ftComboSelect, P("SELECT ID, Description AS Name FROM LabBiopsyTypeT ORDER BY Description ASC"), NULL, FILTER_DEPENDS_ON_BASE, NULL, NULL, NULL, NULL, 0, P("ID") P("Description") P("LabBiopsyTypeT")),
	ADD_FILTER_FIELD(2017, "Order Code", "LabsT.LOINC_Code", fboLab, foAll, foEqual, ftText, NULL, NULL, FILTER_DEPENDS_ON_BASE),
	ADD_FILTER_FIELD(2018, "Order Description", "LabsT.LOINC_Description", fboLab, foAll, foEqual, ftText, NULL, NULL, FILTER_DEPENDS_ON_BASE),
	// (z.manning 2014-02-17 17:34) - PLID 60521 - This didn't work before because it wasn't setting the exists stuff so I fixed it.
	ADD_FILTER_FIELD(2019, "Has Lab Result", "LabResultsT.ResultID", fboLab, foIn|foNotIn, foIn, ftSubFilterEditable, P("SELECT ID, Name, CreatedDate, ModifiedDate, Filter FROM FiltersT WHERE Type = 7 ORDER BY Name"), NULL, FILTER_DEPENDS_ON_BASE, "LabResultsT INNER JOIN LabsT ON LabResultsT.LabID = LabsT.ID", "PersonT.ID = LabsT.PatientID", NULL, "Lab Result Filter", fboLabResult, NULL ),	
	ADD_FILTER_FIELD(2020, "Completed", "CASE WHEN dbo.GetLabCompletedDate(LabsT.ID) IS NULL THEN 1 ELSE 0 END", fboLab, foEqual|foNotEqual, foEqual, ftComboValues, P("Completed")P("0")P("Not Completed")P("1"), NULL, FILTER_DEPENDS_ON_BASE, NULL, NULL, NULL, NULL, NULL, NULL),
	ADD_FILTER_FIELD(2021, "Completed Date", "dbo.GetLabCompletedDate(LabsT.ID)", fboLab, foAll, foEqual, ftDate, NULL,  NULL, FILTER_DEPENDS_ON_BASE, NULL, NULL, NULL, NULL, NULL, NULL),			

	// (v.maida - 2014-08-05 10:08) - PLID 34948 - Add filter for provider and referring physician specialty
	ADD_FILTER_FIELD(2028, "Referring Phys Specialty", "ReferringSpecialty.Description", fboPerson, foEqual|foNotEqual|foLike|foNotLike|foBeginsWith|foEndsWith|foContains|foIn|foNotIn|foBlank|foNotBlank, foEqual, ftText, NULL, "(SELECT ReferringPhysT.PersonID, ReferringSpecialty.Description from ReferringPhysT INNER JOIN AMASpecialtyListT AS ReferringSpecialty ON ReferringPhysT.AMASpecialtyID = ReferringSpecialty.ID)ReferringSpecialty ON PersonT.ID = ReferringSpecialty.PersonID"),
	ADD_FILTER_FIELD(2029, "Doctor Specialty", "ProviderSpecialty.Description", fboPerson, foEqual|foNotEqual|foLike|foNotLike|foBeginsWith|foEndsWith|foContains|foIn|foNotIn|foBlank|foNotBlank, foEqual, ftText, NULL, "(SELECT ProvidersT.PersonID, ProviderSpecialty.Description from ProvidersT INNER JOIN AMASpecialtyListT AS ProviderSpecialty ON ProvidersT.AMASpecialtyID = ProviderSpecialty.ID)ProviderSpecialty ON PersonT.ID = ProviderSpecialty.PersonID "),
	
	// (b.eyers 2015-06-24) - PLID 16785 - Filter telling if they have a photo attached or not
	ADD_FILTER_FIELD(2030, "Has Photo Attached", "CASE WHEN HasPhotoQ.PersonID IS NOT NULL THEN 1 ELSE 0 END", fboPerson, foEqual, foEqual, ftComboValues, P("Yes") P("1") P("No") P("0"), "(SELECT PersonID FROM MailSent WHERE dbo.IsImageFile(PathName) = 1 AND (MailSent.IsPhoto IS NULL OR MailSent.IsPhoto = 1)) AS HasPhotoQ ON PersonT.ID = HasPhotoQ.PersonID", NULL),

	ADD_FILTER_FIELD(FILTER_FIELD_SPECIAL_GROUP, "< Existing Group >", "PersonT.ID", fboPerson, foIn|foNotIn, foIn, ftComboSelect, P("SELECT ID, Name FROM GroupsT ORDER BY Name"), NULL, FILTER_DEPENDS_ON_BASE, NULL, NULL, "SELECT GroupDetailsT.PersonID FROM GroupDetailsT WHERE GroupID = {UserEnteredValue}"), 

	ADD_FILTER_FIELD(FILTER_FIELD_SPECIAL_ADVANCED, "< Advanced Filter >", "", fboPerson, foInvalidOperator, foInvalidOperator, ftAdvanced, NULL, NULL),
	//FiltersT.Type = 1 => Person-based filter
	ADD_FILTER_FIELD(FILTER_FIELD_SPECIAL_SUBFILTER, "< Subfilter >", "PersonT.ID", fboPerson, foIn|foNotIn, foIn, ftSubFilterEditable, P("SELECT ID, Name, CreatedDate, ModifiedDate, Filter FROM FiltersT WHERE Type = 1 ORDER BY Name"), NULL, FILTER_DEPENDS_ON_BASE, NULL, NULL, NULL, "Person Filter", fboPerson),
END_ADD_FILTER_FIELDS(g_FilterFields, g_nFilterFieldCount);

//Now,add the "base" filters
BEGIN_ADD_FILTER_FIELDS(g_BaseDependencies)
	ADD_FILTER_FIELD(fboPerson, fboPerson, "PersonT.ID", "PersonT"),
	ADD_FILTER_FIELD(fboEMN, fboEMN, "EmrMasterT.ID", "EmrMasterT"),
	ADD_FILTER_FIELD(fboAppointment, fboAppointment, "AppointmentsT.ID", "AppointmentsT WITH(NOLOCK)"),
	ADD_FILTER_FIELD(fboTodo, fboTodo, "ToDoList.TaskID", "ToDoList WITH(NOLOCK)"),
	ADD_FILTER_FIELD(fboEMR, fboEMR, "EMRGroupsT.ID", "EMRGroupsT"),
	ADD_FILTER_FIELD(fboPayment, fboPayment, "PaymentsT.ID", "PaymentsT INNER JOIN (SELECT * FROM LineItemT WHERE LineItemT.Deleted = 0) AS LineItemT ON PaymentsT.ID = LineItemT.ID"),
	ADD_FILTER_FIELD(fboLabResult, fboLabResult, "LabResultsT.ResultID", "(SELECT * FROM LabsT WHERE LabsT.Deleted = 0) LabsT INNER JOIN (SELECT * FROM LabResultsT WHERE LabResultsT.Deleted = 0) LabResultsT ON LabsT.ID = LabResultsT.LabID"),//TES 9/9/2010 - PLID 40457
	ADD_FILTER_FIELD(fboImmunization, fboImmunization, "PatientImmunizationsT.ID", "PatientImmunizationsT"), //TES 9/9/2010 - PLID 40470
	ADD_FILTER_FIELD(fboEmrProblem, fboEmrProblem, "EmrProblemsT.ID", "(SELECT * FROM EmrProblemsT WHERE Deleted = 0) EmrProblemsT"), //TES 9/9/2010 - PLID 40471
	ADD_FILTER_FIELD(fboMedication, fboMedication, "CurrentPatientMedsT.MedicationID", "(SELECT DrugList.ID AS MedicationID, PatientId,Data, PatientInstructions, Sig,NDCNumber, StartDate,Discontinued FROM CurrentPatientMedsT INNER JOIN DrugList ON CurrentPatientMedsT.MedicationID = DrugList.ID INNER JOIN EmrDataT ON DrugList.EmrDataID = EmrDataT.ID) CurrentPatientMedsT"), //(r.wilson 10/21/2013) PLID 59121 - PatientLists
	ADD_FILTER_FIELD(fboAllergy, fboAllergy, "PatientAllergyT.ID", "(SELECT PatientAllergyT.ID AS ID, AllergyID, PersonID, EmrDataT.Data, EnteredDate, Description, Discontinued FROM PatientAllergyT INNER JOIN AllergyT ON PatientAllergyT.AllergyID = AllergyT.ID INNER JOIN EmrDataT ON AllergyT.EmrDataID = EmrDataT.ID) PatientAllergyT"), //(r.wilson 10/21/2013) PLID 59121 - PatientLists
	ADD_FILTER_FIELD(fboLab, fboLab, "LabsT.ID", "(SELECT * FROM LabsT WHERE Deleted = 0)LabsT"), //(r.wilson 10/21/2013) PLID 59121 - PatientLists
END_ADD_FILTER_FIELDS(g_BaseDependencies, g_nBaseDependencyCount);

/////////////////////////////////////////////////////////////////////////////
// CFilterDetail 
// (c.haag 2010-12-07 17:37) - PLID 40640 - We now take in a connection object
CFilterDetail::CFilterDetail(long nFilterType, BOOL (WINAPI* pfnGetNewFilterString)(long, long, CString&, LPCTSTR, CString&), ADODB::_ConnectionPtr pCon /* = NULL */)
{
	// Init
	m_bUseOr = false;
	m_strValue = "";
	m_nFieldIndex = -1;
	m_foOperator = foInvalidOperator;
	m_nFilterType = nFilterType;
	m_nDynamicRecordID = INVALID_DYNAMIC_ID;
	m_pfnGetNewFilterString = pfnGetNewFilterString;
	m_pCon = pCon;
}

CFilterDetail::~CFilterDetail()
{
	// Do nothing
}

// Given a certain field (in g_FilterFields, specified by index), is the operator efo available?
#define HAS_OP(index, efo)					(((g_FilterFields[index].m_nAvailableOperators) & (efo)) != 0)

void CFilterDetail::SetOperator(long nOperator)
{
	if (m_nFieldIndex == -1) {
		m_foOperator = foInvalidOperator;
		return;
	} else {
		if (HAS_OP(m_nFieldIndex, nOperator)) {
			// This operator is supported by the current field type
			m_foOperator = (FieldOperatorEnum)nOperator;
		} else {
			// nOperator wasn't supported, or more likely, nOperator wasn't specified so 
			// see if this field type allows exacltly ONE operator and if so, use that
			switch (g_FilterFields[m_nFieldIndex].m_nAvailableOperators) {
			case foEqual:
			case foGreater:
			case foGreaterEqual:
			case foLess:
			case foLessEqual:
			case foNotEqual:
			case foLike:
			case foBeginsWith:
			case foEndsWith:
			case foContains:
				// It supports exactly one operator so use it
				m_foOperator = (FieldOperatorEnum)g_FilterFields[m_nFieldIndex].m_nAvailableOperators;
				break;
			default:
				// It supports either zero or more than one op, so we can't decide automatically
				m_foOperator = foInvalidOperator;
				break;
			}
		}
	}
}

void CFilterDetail::SetValue(LPCTSTR strValue)
{
	if (m_nFieldIndex == -1) {
		// Default to blank
		m_strValue = "";
	} else {
		// Get the default value string if it was passed
		if (strValue) {
			m_strValue = strValue;
		} else {
			m_strValue = "";
		}
	}
}

void CFilterDetail::SetUseOr(bool bUseOr)
{
	m_bUseOr = bUseOr;
}


BOOL CFilterDetail::HasAdvancedFilter() {

	if (m_nFieldIndex >= 0 ) {
		if (g_FilterFields[m_nFieldIndex].HasAdvancedFilter()) {
			return TRUE;
		}
		else {
			return FALSE;
		}
	}
	else {
		return FALSE;
	}
}

// (c.haag 2010-12-07 17:20) - PLID 40640 - We now take in an optional connection pointer
bool CFilterDetail::GetWhereClause(CString &strOutClause, CMapStringToString &mapUsedFilters, bool bIncludePrefix, BOOL bSilent /*=FALSE*/)
{
	if (Store()) {
		if (m_nFieldIndex >= 0) {
			// If we made it to here, we're okay so return the clause result
			// (c.haag 2010-12-07 17:20) - PLID 40640 - Pass in a connection pointer
			bool bSuccess = g_FilterFields[m_nFieldIndex].GetWhereClause(m_foOperator, m_strValue, m_bUseOr, mapUsedFilters, strOutClause, bIncludePrefix, bSilent, m_pCon);

			//DRT 7/21/2004 - PLID 13238 - We need to be able to filter on the dynamically selected item in the "Filter on Field" criteria.
			//	To do that, we allow the user to put {DYNAMIC_ID} in the exists where clause, and then here we replace it with the 
			//	actual ID that is selected.  This is used only if you have a field which uses "DYNAMIC_FIELD:SELECT ID, NAME..." to fill
			//	in the list of available fields (see Has EMR subfilter items).
			CString strDynamicID;
			strDynamicID.Format("%li", m_nDynamicRecordID);
			strOutClause.Replace("{DYNAMIC_ID}", strDynamicID);
			return bSuccess;
		} else {
			// nothing selected, so we still return success but empty the string
			strOutClause.Empty();
			return true;
		}
	} else {
		return false;
	}
}

bool CFilterDetail::IsEmpty()
{
	if (m_nFieldIndex >= 0) {
		// Something is selected so this is not empty
		return false;
	} else {
		// Nothing is selected so this is empty
		return true;
	}
}

// (a.walling 2008-10-03 17:29) - PLID 31589 - ASSERTion if you try to pass a signed char to any ::is* functions.
#define SKIP_WS(pstr)		{ while (isspace(unsigned char(*pstr))) pstr++; if (*pstr == '\0') AfxThrowNxException("Unexpected end of detail string"); }

// Made for our strings, not for SQL
long GetQuotedString(LPCTSTR str, OUT CString &strOut)
{
	LPCTSTR pstr = str;
	SKIP_WS(pstr);

	if (*pstr != '\"') {
		AfxThrowNxException("Expected opening quotation marks");
		return 0;
	}
	pstr++;

	LPCTSTR pstrBegin = pstr;

	// We're now inside the quotes
	for (pstrBegin = pstr; ; pstr++) {
		// The code below will break out when necessary
		if (*pstr == '\"') {
			if (*(pstr+1) == '\"') {
				// Skip the extra quotation mark
				pstr++;
			} else {
				// The quoted text is finished
				strOut = CString(pstrBegin, pstr-pstrBegin);
				// Unformat the text
				//NOTE: This at one time replaced "" with ', apparently because someone was overzealous in converting
				//to T-SQL syntax.  This should indeed be replacing "" with ", because if you look at MakeQuotedString,
				//it replaces " with "".
				strOut.Replace("\"\"", "\"");
				strOut.Replace("[[]", "[");
				strOut.Replace("[?]", "?");
				strOut.Replace("[*]", "*");
				// Return the number of characters processed (including the beginning and ending quotes)
				return pstr-pstrBegin+2;
			}
		} else if (*pstr == '\0') {
			AfxThrowNxException("Expected closing quotation marks");
			return 0;
		}
	}

}

// Made for our strings, not for SQL
bool MakeQuotedString(const CString &strIn, CString &strOut)
{
	strOut = strIn;
	strOut.Replace("\"", "\"\"");
	strOut.Replace("[", "[[]");
	strOut.Replace("?", "[?]");
	strOut.Replace("*", "[*]");
	return true;
}

bool CFilterDetail::GetDetailString(CString &strOut, bool bIncludePrefix /*= false*/)
{
	if (Store()) {
		if (m_nFieldIndex >= 0) {
			// Get the strings as quotable text
			CString strValue/*, strApparentField*/;
//			FormatValueText(g_FilterFields[m_nFieldIndex].m_strFieldNameApparent, strApparentField);
			MakeQuotedString(m_strValue, strValue);

			// Now return the result (with or without the initial + or * depending on whether it is requested)
			if(m_nDynamicRecordID != INVALID_DYNAMIC_ID) {
				 if (bIncludePrefix) {
					strOut.Format("%s{%li, %li, \"%s\", %li}", m_bUseOr ? " + " : " * ", g_FilterFields[m_nFieldIndex].m_nInfoId, m_foOperator, strValue, m_nDynamicRecordID);
				} else {
					strOut.Format("{%li, %li, \"%s\", %li}", g_FilterFields[m_nFieldIndex].m_nInfoId, m_foOperator, strValue, m_nDynamicRecordID);
				}
			}
			else {
				if (bIncludePrefix) {
					strOut.Format("%s{%li, %li, \"%s\"}", m_bUseOr ? " + " : " * ", g_FilterFields[m_nFieldIndex].m_nInfoId, m_foOperator, strValue);
				} else {
					strOut.Format("{%li, %li, \"%s\"}", g_FilterFields[m_nFieldIndex].m_nInfoId, m_foOperator, strValue);
				}
			}
			return true;
		} else {
			// nothing selected, so we still return success but empty the string
			strOut.Empty();
			return true;
		}
	} else {
		return false;
	}
}

void CFilterDetail::SetDetailString(LPCTSTR strDetail)
{
	if (strDetail) {
		LPCTSTR pstr = strDetail;
		
		// Get the opening "+" or "*" if there is one, otherwise there better be a "{"
		bool bUseOr = true;
		SKIP_WS(pstr);
		switch (*pstr) {
		case '+':
			// Plus indicates an OR operation; move to the next character
			bUseOr = true;
			pstr++;
			break;
		case '{':
			// Open brace implies an OR operation; DO NOT move to the next character
			bUseOr = true;
			break;
		case '*':
			// Asterisk indicates an AND operation; move to the next character
			bUseOr = false;
			pstr++;
			break;
		default:
			AfxThrowNxException("Expected '{', '+', or '*' while parsing filter detail string"); 
			return;
		}
		
		// Move past the required opening brace
		SKIP_WS(pstr);
		if (*pstr != '{') {
			AfxThrowNxException("Expected '{' while parsing filter detail string"); return;
		}
		pstr++;
		
		// Get the apparent field name
		SKIP_WS(pstr);
		long nFieldIndex = -1;
		if (*pstr == '\"') {
			// The apparent field name is in quotes
			CString strApparentField;
			pstr += GetQuotedString(pstr, strApparentField);
			nFieldIndex = CFilterFieldInfo::GetInfoIndex(strApparentField, (CFilterFieldInfo*)g_FilterFields, g_nFilterFieldCount);
			// Make sure the index we got was valid
			if (nFieldIndex < 0 || nFieldIndex >= g_nFilterFieldCount) {
				AfxThrowNxException("Field \"%s\" refers to invalid index %li while parsing filter detail string", strApparentField, nFieldIndex); return;
			}
		// (a.walling 2008-10-03 17:29) - PLID 31589 - ASSERTion if you try to pass a signed char to any ::is* functions.
		} else if (isdigit(unsigned char(*pstr))) {
			// The field info ID is specified
			long nFieldInfoId = 0;
			if (sscanf(pstr, "%li", &nFieldInfoId) != 1) {
				AfxThrowNxException("Expected numeric field id while parsing filter detail string"); return;
			}
			while (*pstr && (*pstr != ',')) pstr++;
			if (*pstr == NULL) AfxThrowNxException("Unexpected end of detail string while parsing field info id");
			// Convert the info id into a field index
			nFieldIndex = CFilterFieldInfo::GetInfoIndex(nFieldInfoId, (CFilterFieldInfo*)g_FilterFields, g_nFilterFieldCount);
			// Make sure the index we got was valid
			if (nFieldIndex < 0 || nFieldIndex >= g_nFilterFieldCount) {
				AfxThrowNxException("Field info ID %li refers to invalid index %li while parsing filter detail string", nFieldInfoId, nFieldIndex); return;
			}
		} else {
			AfxThrowNxException("Expected quoted field name or numeric field id while parsing filter detail string"); return;
		}

		// Get a comma
		SKIP_WS(pstr);
		if (*pstr != ',') {
			AfxThrowNxException("Expected ',' while parsing filter detail string"); return;
		}
		pstr++;

		// Get a number indicating the operator
		long nOperator;
		SKIP_WS(pstr);
		if (sscanf(pstr, "%li", &nOperator) != 1) {
			AfxThrowNxException("Expected operator code while parsing filter detail string"); return;
		}
		while (*pstr && (*pstr != ',')) pstr++;
		if (*pstr == NULL) AfxThrowNxException("Unexpected end of detail string while parsing operator");
		pstr++;

		// Get the text value
		CString strValue;
		SKIP_WS(pstr);
		if (*pstr != '\"') {
			AfxThrowNxException("Expected quoted value while parsing filter detail string"); return;
		}
		pstr += GetQuotedString(pstr, strValue);

		SKIP_WS(pstr);
		long nDynamicID = INVALID_DYNAMIC_ID;
		if(*pstr == ',') {
			//OK, we've got one more argument
			pstr++;
			SKIP_WS(pstr);
			if (sscanf(pstr, "%li", &nDynamicID) != 1) {
				AfxThrowNxException("Expected dynamic field id while parsing filter detail string"); return;
			}
			//Now advance to the closing bracket.
			while (*pstr && (*pstr != '}')) pstr++;
			if (*pstr == NULL) AfxThrowNxException("Unexpected end of detail string while parsing dynamic field id");
		}

		// Get the closing "}"
		if (*pstr != '}') {
			AfxThrowNxException("Expected ',' or '}' while parsing filter detail string"); return;
		}
		pstr++;

		// Success!!  Reflect the new settings on screen
		SetDetail(nFieldIndex, nOperator, strValue, bUseOr, nDynamicID);
	} else {
		AfxThrowNxException("Invalid filter detail string");
	}
}

void CFilterDetail::SetDetail(const CString &strApparentField, long nOperator, LPCTSTR strValue, bool bUseOr, long nDynamicID)
{
	// If we found it, we are using the i that corresponds, otherwise we are using -1
	SetDetail(CFilterFieldInfo::GetInfoIndex(strApparentField, (CFilterFieldInfo*)g_FilterFields, g_nFilterFieldCount), nOperator, strValue, bUseOr, nDynamicID);
}

void CFilterDetail::SetDetail(long nFieldIndex, long nOperator, LPCTSTR strValue, bool bUseOr, long nDynamicID)
{
	// If we don't find it, default to the nothing index
	m_nFieldIndex = nFieldIndex;

	// (a.walling 2007-11-09 17:04) - PLID 28065 - VS2008 - This is accessing an extremely out of bounds index. Ensure it >= 0.
	if (nFieldIndex >= 0) {

		//Is this detail obsolete?
		if(g_FilterFields[nFieldIndex].IsObsolete() ) {
			//Let's find the new filter string for it.
			CString strNewString;
			if(m_pfnGetNewFilterString != 0) {//If they gave us a callback.
				if( m_pfnGetNewFilterString(g_FilterFields[nFieldIndex].m_nInfoId, nOperator, g_FilterFields[nFieldIndex].GetFieldNameApparent(), strValue, strNewString)) {
					SetDetailString(strNewString);//This will eventually call SetDetail(), so we're done.
					return;
				}
			}
			//If they didn't give us a function, they must have
			strNewString = g_FilterFields[nFieldIndex].m_strDefaultReplacement;
			CString strOp;
			strOp.Format("%li", nOperator);
			strNewString.Replace("PRE_EXISTING_FIELD_OPERATOR", strOp);
			strNewString.Replace("PRE_EXISTING_SELECTED_VALUE", strValue);
			if(bUseOr) strNewString = "+ " + strNewString;
			else strNewString = "* " + strNewString;
			SetDetailString(strNewString);//This will eventually call SetDetail(), so we're done.
			return;
		}
		else if(g_FilterFields[nFieldIndex].IsRemoved() ) {
			//That's trouble.  Tell the user what happened.
			CRemovedFilterDlg dlg(NULL);
			dlg.m_strCaption = g_FilterFields[nFieldIndex].m_strRemovalExplanation;
			dlg.m_strHelpLocation = g_FilterFields[nFieldIndex].m_strRemovalHelpLocation;
			dlg.m_strHelpBookmark = g_FilterFields[nFieldIndex].m_strRemovalHelpBookmark;
			dlg.DoModal();
			//Now, we DON'T want to return, because we still want to show the user what their filter used
			//to look like.
		}
	}

	// Enable and fill the operators combo appropriately
	SetOperator(nOperator);
	
	// Enable and default the value edit box
	SetValue(strValue);
	
	// Decide whether this is an AND or an OR
	SetUseOr(bUseOr);

	// Sets the optional dynamic record id.
	SetDynamicID(nDynamicID);

	// This only does anything in a derived class
	Refresh();
}

void CFilterDetail::SetPosition(long nIndexPosition)
{
	// Derived class implements this usually
}

void CFilterDetail::Refresh()
{
	// Derived class implements this usually
}

bool CFilterDetail::Store()
{
	// Derived class implements this usually
	return true;
}

void CFilterDetail::AssertValidArray(CFilterFieldInfo *arFields, long nFieldCount)
{
	//We need to maintain a list of the ids we've come across.
	CArray<unsigned long, unsigned long> arIds;
	for(int i = 0; i < nFieldCount; i++) {
		//An assertion on this line means we have an out-of-range id
		ASSERT((arFields[i].m_nInfoId >= 0 && arFields[i].m_nInfoId < FILTER_FIELD_NEXT_INFO_ID) || 
		(arFields[i].m_nInfoId == FILTER_FIELD_SPECIAL_ADVANCED) || 
		(arFields[i].m_nInfoId == FILTER_FIELD_SPECIAL_SUBFILTER) || 
		(arFields[i].m_nInfoId == FILTER_FIELD_SPECIAL_GROUP));

		if(! ((arFields[i].m_nInfoId >= 0 && arFields[i].m_nInfoId < FILTER_FIELD_NEXT_INFO_ID) || 
		(arFields[i].m_nInfoId == FILTER_FIELD_SPECIAL_ADVANCED) || 
		(arFields[i].m_nInfoId == FILTER_FIELD_SPECIAL_SUBFILTER) || 
		(arFields[i].m_nInfoId == FILTER_FIELD_SPECIAL_GROUP))) return;

		for(int j = 0; j < arIds.GetSize(); j++) {
			//An assertion on this lines means we have a duplicate id
			ASSERT(arIds.GetAt(j) != arFields[i].m_nInfoId);
			if(arIds.GetAt(j) == arFields[i].m_nInfoId) return;
		}
		arIds.Add(arFields[i].m_nInfoId);
	}
}

void CFilterDetail::SetDynamicID(long nDynamicID)
{
	m_nDynamicRecordID = nDynamicID;
}

long CFilterDetail::GetDetailInfoId()
{	
	// (a.walling 2007-11-09 17:05) - PLID 28065 - For safety's sake, throw an exception if the index < 0.
	if (m_nFieldIndex < 0) {
		ThrowNxException("Invalid field index stored during call to CFilterDetail::GetDetailInfoId");
	}
	return g_FilterFields[m_nFieldIndex].m_nInfoId;
}

CString CFilterDetail::GetDetailFieldNameApparent()
{
	// (a.walling 2007-11-09 17:05) - PLID 28065 - For safety's sake, throw an exception if the index < 0.
	if (m_nFieldIndex < 0) {
		ThrowNxException("Invalid field index stored during call to CFilterDetail::GetDetailFieldNameApparent");
	}
	return g_FilterFields[m_nFieldIndex].GetFieldNameApparent();
}

FieldOperatorEnum CFilterDetail::GetDetailOperator()
{
	return m_foOperator;
}

CString CFilterDetail::GetDetailValue()
{
	return m_strValue;
}

bool CFilterDetail::GetDetailUseOr()
{
	return m_bUseOr;
}

long CFilterDetail::GetDetailInfoIndex()
{
	return m_nFieldIndex;
}

// (r.gonet 10/09/2013) - PLID 56236 - Gets the dynamic record ID.
long CFilterDetail::GetDynamicRecordID()
{
	return m_nDynamicRecordID;
}



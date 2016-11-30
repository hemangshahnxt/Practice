#include "stdafx.h"
#include "WaitingListUtils.h"
#include "GlobalUtils.h"
#include "GlobalDataUtils.h"
#include "InternationalUtils.h"

using namespace ADODB;

// (d.moore 2007-05-23 11:31) - PLID 4013

CString CWaitingListUtils::FormatLineItem(const WaitListLineItem &wlItem)
{
	// Formats the contents of the WaitListLineItem structure as text.
	
	CString strLine;
	// Format the date and time ranges.
	strLine.Format("%s to %s from %s to %s", 
		FormatDateTimeForInterface(wlItem.dtStartDate, NULL, dtoDate), 
		FormatDateTimeForInterface(wlItem.dtEndDate, NULL, dtoDate), 
		FormatDateTimeForInterface(wlItem.dtStartTime, DTF_STRIP_SECONDS, dtoTime), 
		FormatDateTimeForInterface(wlItem.dtEndTime, DTF_STRIP_SECONDS, dtoTime)); 
		
	// Format the resource list, if any.
	if (wlItem.bAllResources) {
		strLine += " for all resources";
	} else {
		CString strResourcList;
		int nResCount = wlItem.arResourceNames.GetSize();
		for (long i = 0; i < nResCount; i++) {
			if (i > 0) {
				strResourcList += ", ";
			}
			strResourcList += wlItem.arResourceNames[i];
		}
		if (nResCount > 0) {
			strLine += " for the " + strResourcList; 
			strLine += (nResCount > 1)?" resources":" resource";
		}
	}
	//Format the days associated with the resources.
	CString strDayNames = FormatDayValues(wlItem);
	if (!strDayNames.IsEmpty()) {
		strLine += strDayNames;
	}

	return strLine;
}


CString CWaitingListUtils::FormatDayValues(const WaitListLineItem &wlItem)
{
	// Returns the day values for the WaitListLineItem as a comma seperated string prefixed with the word 'on'.
	
	CString strDayList;
	
	// First check to see if the names have already been provided.
	long nDayCount = wlItem.arDayNames.GetSize();
	if (nDayCount > 0) {
		for (long i = 0; i < nDayCount; i++) {
			if (i > 0) {
				strDayList += ", ";
			}
			strDayList += wlItem.arDayNames[i];
		}
		strDayList = " on " + strDayList;
		return strDayList;
	} 
	
	// Names weren't provided. So check for anything in the ID array.
	nDayCount = wlItem.arDayIDs.GetSize();
	if (nDayCount > 0) {
		for (long i = 0; i < nDayCount; i++) {
			if (i > 0) {
				strDayList += ", ";
			}
			strDayList += GetDayName(wlItem.arDayIDs[i]);
		}
		strDayList = " on " + strDayList;
	}

	return strDayList;
}


CString CWaitingListUtils::GetDayName(long nDayID)
{
	// Returns the abbreviated name of a week day.
	
	COleDateTime dt;

	switch (nDayID) {
		case 1: //Sunday
			dt.SetDate(2007,7,1);		
			break;	
		case 2: //Monday
			dt.SetDate(2007,7,2);
			break;
		case 3: //Tuesday
			dt.SetDate(2007,7,3);
			break;
		case 4: //Wednesday
			dt.SetDate(2007,7,4);
			break;
		case 5: //Thursday
			dt.SetDate(2007,7,5);
			break;
		case 6: //Friday
			dt.SetDate(2007,7,6);
			break;
		case 7: //Saturday
			dt.SetDate(2007,7,7);
			break;
		default: //Sunday
			dt.SetDate(2007,7,1);
			break;
	}

	return FormatDateTimeForInterface(dt,"%a");
}


// Returns an SQL Query string for adding an appointment to the waiting list. If the 
//  appointment is already in the waiting list, then it updates the entry. Otherwise,
//  it addes a new entry.
CString CWaitingListUtils::BuildWaitingListUpdateQuery(
	const long nAppointmentID, const long nPatientID, const COleDateTime &dtDate, 
	const CDWordArray &adwPurposeIDs, const CDWordArray &adwResourceIDs, 
	const long nAptTypeID, const CString &strNotes)
{
	// Build a list of all of the purpose IDs for the appointment.
	CString strPurposeIdList, strID;
	long nNumPurposes = adwPurposeIDs.GetSize();
	for (long i = 0; i < nNumPurposes; i++) {
		strID.Format("%li, ", adwPurposeIDs[i]);
		strPurposeIdList += strID;
	}
	strPurposeIdList = strPurposeIdList.Left(strPurposeIdList.GetLength()-2);
	// Build a list of all of the resource IDs for the appointment.
	CString strResourceIdList;
	long nNumResources = adwResourceIDs.GetSize();
	for (i = 0; i < nNumResources; i++) {
		strID.Format("%li, ", adwResourceIDs[i]);
		strResourceIdList += strID;
	}
	strResourceIdList = strResourceIdList.Left(strResourceIdList.GetLength()-2);
	
	// ADD QUERY
	// Build the query parts for adding a new item to the waiting list.
	
	// Convert the Type and Appointment ID values into strings to help handling Null cases.
	CString strApptIdField, strApptIdValue, strTypeIdField, strTypeIdValue;
	if (nAppointmentID > 0) {
		strApptIdField = "AppointmentID, ";
		strApptIdValue.Format("%li, ", nAppointmentID);
	}
	if (nAptTypeID > 0) {
		strTypeIdField = "TypeID, ";
		strTypeIdValue.Format("%li, ", nAptTypeID);
	}

	// Copy the basic appointment data into the WaitingList table.
	CString strAddWaitListQ;
	strAddWaitListQ.Format(
		"INSERT INTO WaitingListT (%s%s PatientID, CreatedDate, Notes) "
		"VALUES (%s%s %li, @TodaysDate, '%s');\r\n"
		"SET @Wait_List_ID = @@IDENTITY; \r\n", 
		strApptIdField, strTypeIdField, 
		strApptIdValue, strTypeIdValue, 
		nPatientID, _Q(strNotes));

	// Set entries in the purpose table.
	CString strAddWaitingListPurposeQ;
	if (nNumPurposes > 0) {
		strAddWaitingListPurposeQ.Format(
			"INSERT INTO WaitingListPurposeT(WaitingListID, PurposeID) "
			"SELECT @Wait_List_ID, ID FROM AptPurposeT WHERE ID IN (%s);\r\n", 
			strPurposeIdList);
	}
	
	// Set entries in the line item table.
	CString strAddWaitingListItemQ;
	strAddWaitingListItemQ.Format(
		"DECLARE @Wait_List_Item_ID INT;\r\n"
		"INSERT INTO WaitingListItemT "
			"(WaitingListID, StartDate, EndDate, "
			"StartTime, EndTime, AllResources) "
		"VALUES ("
			"@Wait_List_ID, "
			"@TodaysDate, '%s', "
			"'5:00 AM', '10:00 PM', 0);\r\n"
		"SET @Wait_List_Item_ID = @@IDENTITY;\r\n", 
		// (c.haag 2008-10-31 09:12) - PLID 31856 - Use date formatting utilities
		FormatDateTimeForSql(dtDate, dtoDate));

	// Set resource entry.
	CString strAddWaitingListItemResourceQ;
	if (nNumResources > 0) {
		strAddWaitingListItemResourceQ.Format(
			"INSERT INTO WaitingListItemResourceT "
				"(ItemID, ResourceID) "
			"SELECT @Wait_List_Item_ID, ID "
			"FROM ResourceT WHERE ID IN (%s);\r\n", 
			strResourceIdList);
	}

	// UPDATE QUERY
	// Build the query parts for updating an existing item in the waiting list.
	
	// Get the ID for the waitlist using the appointment ID.
	CString strUpdateWaitListIdQ;
	strUpdateWaitListIdQ.Format(
		"SELECT @Wait_List_ID = ID FROM WaitingListT WHERE AppointmentID=%li;\r\n", 
		nAppointmentID);

	// Check to see if the appointment type ID should be modified.
	CString strAptTypeInsert;
	if (nAptTypeID > 0) {
		strAptTypeInsert.Format(", TypeID = %li ", nAptTypeID);
	}

	// Upate basic patient and appointment data.
	CString strUpdateWaitListQ;
		strUpdateWaitListQ.Format(
			"UPDATE WaitingListT "
			"SET PatientID = %li %s"
			"WHERE ID = @Wait_List_ID;\r\n", 
			nPatientID, strAptTypeInsert);
	
	// Update the list of purposes attached to the item.
	CString strDeleteWaitingListPurposeQ = 
		"DELETE FROM WaitingListPurposeT WHERE WaitingListID = @Wait_List_ID;\r\n";
	
	// Conditional Query for deciding if we need to add or update.
	CString strIfQ;
	strIfQ.Format("SELECT ID FROM WaitingListT WHERE AppointmentID = %li", nAppointmentID);

	// Put together the body of the final query.
	CString strQuery = 
		"DECLARE @Wait_List_ID INT; \r\n"
		"IF NOT EXISTS (" + strIfQ + ") \r\n"
			"BEGIN \r\n"
				"DECLARE @TodaysDate nvarchar(10); "
				"SET @TodaysDate = Convert(nvarchar(10), GetDate(), 101); "
				"\r\n" +
				strAddWaitListQ +
				strAddWaitingListPurposeQ +
				strAddWaitingListItemQ +
				strAddWaitingListItemResourceQ +
			"END \r\n"
		"ELSE \r\n"
			"BEGIN \r\n" +
				strUpdateWaitListIdQ +
				strUpdateWaitListQ +
				strDeleteWaitingListPurposeQ +
				strAddWaitingListPurposeQ +
			"END \r\n";

	return strQuery;
}

// Returns an SQL query string for deleting an item from the waiting list. This
//  function is designed to be called from the scheduler, so it uses the appointment
//  ID instead of a waiting list ID.
CString CWaitingListUtils::BuildWaitingListDeleteQuery(long nAppointmentID)
{
	CString strQuery;

	strQuery.Format(
		"DECLARE @Wait_List_ID INT; \r\n"
		"SELECT @Wait_List_ID = ID FROM WaitingListT WHERE AppointmentID=%li; \r\n"
		// Delete associated resources.
		"DELETE FROM WaitingListItemResourceT "
		"WHERE ItemID IN "
			"(SELECT ID FROM WaitingListItemT "
			"WHERE WaitingListID=@Wait_List_ID); "
		"\r\n"
		// Delete Days Query.
		"DELETE FROM WaitingListItemDaysT "
		"WHERE ItemID IN "
			"(SELECT ID FROM WaitingListItemT "
			"WHERE WaitingListID = @Wait_List_ID); "
		"\r\n"
		// Delete Line Items Query.
		"DELETE FROM WaitingListItemT "
		"WHERE WaitingListID = @Wait_List_ID; "
		"\r\n"
		// Delete Purpose Query.
		"DELETE FROM WaitingListPurposeT "
		"WHERE WaitingListID = @Wait_List_ID; "
		"\r\n"
		// Delete Waiting List Entry Query.
		"DELETE FROM WaitingListT WHERE ID = @Wait_List_ID; \r\n", 
		nAppointmentID);

	return strQuery;
}
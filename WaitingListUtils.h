#ifndef NEXTECH_PRACTICE_WAITING_LIST_UTILS_H
#define NEXTECH_PRACTICE_WAITING_LIST_UTILS_H

#pragma once



// (d.moore 2007-05-23 11:31) - PLID 4013

// (a.walling 2007-11-07 10:55) - PLID 27998 - VS2008 - Updated to work with VS2008
struct WaitListLineItem{
	long nLineItemID;
	long nTempID;
	long nWaitListID;
	bool bAllResources;
	COleDateTime dtStartDate;
	COleDateTime dtEndDate;
	COleDateTime dtStartTime;
	COleDateTime dtEndTime;
	CArray <long, long> arResourceIDs;
	CStringArray arResourceNames;
	CArray <long, long> arDayIDs;
	CStringArray arDayNames;
	bool bModified;

	WaitListLineItem(const WaitListLineItem &wlItem) : 
		nLineItemID(wlItem.nLineItemID), 
		nTempID(wlItem.nTempID), 
		nWaitListID(wlItem.nWaitListID), 
		bAllResources(wlItem.bAllResources), 
		dtStartDate(wlItem.dtStartDate), 
		dtEndDate(wlItem.dtEndDate), 
		dtStartTime(wlItem.dtStartTime), 
		dtEndTime(wlItem.dtEndTime), 
		bModified(wlItem.bModified)
	{
		arResourceIDs.Copy(wlItem.arResourceIDs);
		arResourceNames.Copy(wlItem.arResourceNames);
		arDayIDs.Copy(wlItem.arDayIDs);
		arDayNames.Copy(wlItem.arDayNames);
	};
	
	WaitListLineItem() : 
		nLineItemID(-1), 
		nTempID(-1), 
		nWaitListID(-1), 
		bAllResources(true), 
		bModified(false)
	{
		COleDateTime dt = COleDateTime::GetCurrentTime();
		dtStartDate = dt;
		dtEndDate = dt;
		dt.SetTime(9, 0, 0);
		dtStartTime = dt;
		dt.SetTime(18, 0, 0);
		dtEndTime = dt;
	};

	WaitListLineItem& operator=(const WaitListLineItem &wlItem) {
		if (this == &wlItem)
			return *this;

		nLineItemID = wlItem.nLineItemID;
		nTempID = wlItem.nTempID;
		nWaitListID = wlItem.nWaitListID;
		bAllResources = wlItem.bAllResources;
		dtStartDate = wlItem.dtStartDate;
		dtEndDate = wlItem.dtEndDate;
		dtStartTime = wlItem.dtStartTime;
		dtEndTime = wlItem.dtEndTime;
		arResourceIDs.Copy(wlItem.arResourceIDs);
		arResourceNames.Copy(wlItem.arResourceNames);
		arDayIDs.Copy(wlItem.arDayIDs);
		arDayNames.Copy(wlItem.arDayNames);
		bModified = wlItem.bModified;

		return *this;
	};
	
};

struct WaitListEntry {
	long ID;
	long nPatientID;
	long nApptType;
	CString strCreatedDate;
	CString strNotes;
};


class CWaitingListUtils
{
public:
	
	// Formats the contents of the WaitListLineItem structure as text.
	static CString FormatLineItem(const WaitListLineItem &wlItem);
	
	// Returns the day values for the WaitListLineItem as a comma seperated string prefixed with the word 'on'.
	static CString FormatDayValues(const WaitListLineItem &wlItem);
	
	// Returns the abbreviated name of a week day.
	static CString GetDayName(long nDayID);

	// Returns an SQL Query string for adding an appointment to the waiting list. If the 
	//  appointment is already in the waiting list, then it updates the entry. Otherwise,
	//  it addes a new entry.
	static CString BuildWaitingListUpdateQuery(
		const long nAppointmentID, const long nPatientID, const COleDateTime &dtDate, 
		const CDWordArray& adwPurposeIDs, const CDWordArray& adwResourceIDs, 
		const long nAptTypeID, const CString &strNotes);
	
	// Returns an SQL query string for deleting an item from the waiting list. This
	//  function is designed to be called from the scheduler, so it uses the appointment
	//  ID instead of a waiting list ID.
	static CString BuildWaitingListDeleteQuery(long nAppointmentID);
};

#endif


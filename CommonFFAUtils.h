#pragma once
// (r.gonet 2014-12-17) - PLID 64464 - Created this file to store some common classes and defines that are used in
// both the text-list mode and non-text list mode of the FFA.

#include "NxThread.h"

// (r.gonet 2014-12-17) - PLID 64464 - Moved from FirstAvailAppt.h
#define FFA_SECONDS_PER_SLOT	30
#define FFA_SLOTS_PER_HOUR		(60*60/FFA_SECONDS_PER_SLOT)
#define FFA_SLOTS_PER_MINUTE	(60/FFA_SECONDS_PER_SLOT)
#define FFA_TIME_LIST_SIZE		(24*60*60/FFA_SECONDS_PER_SLOT)

typedef CMap<COleDateTime, COleDateTime&, BOOL, BOOL> CMapDateTimeToBool;
typedef long TimeList[FFA_TIME_LIST_SIZE];
typedef CMap<COleDateTime, COleDateTime&, long*, long*> CMapDateTimeToTimeList;
typedef CMap<CString, LPCTSTR, CMapDateTimeToTimeList*, CMapDateTimeToTimeList*> CMapResourceToDateMap;
typedef CMap<CString, LPCTSTR, DWORD, DWORD> CMapResourceToID;

// (r.gonet 2014-11-19) - PLID 64174 - Added some stored insurance variables so we can reference what the
// user selected from the insured party dropdown when we go and create the appointment from the FFA results list.
class CFFAInsurance {
public:
	// (r.gonet 2014-11-19) - PLID 64174 - PersonID of the insured party.
	long m_nInsuredPartyID;
	// (r.gonet 2014-11-19) - PLID 64174 - PersonID of the insurance company associated with the insured party.
	long m_nInsuranceCoID;
	// (r.gonet 2014-11-19) - PLID 64174 - Name of the insurance company associated with the insured party.
	CString m_strInsuranceCoName;
	// (r.gonet 2014-11-19) - PLID 64174 - Responsibility type name of the insured party.
	CString m_strRespType;
	// (r.gonet 2014-11-19) - PLID 64174 - Category ID of the insured party. e.g. Medical, Vision, etc.
	long m_nCategoryID;

	// (r.gonet 2014-11-19) - PLID 64174 - Constructs a new FFA insurance object with default values.
	CFFAInsurance();
	// (r.gonet 2014-12-17) - PLID 64464 - Copy constructor
	CFFAInsurance(const CFFAInsurance &other);
};
typedef shared_ptr<CFFAInsurance> CFFAInsurancePtr;

// (r.gonet 2014-12-17) - PLID 64464 - Encapsulates the search settings used by the FFA so they can be passed around
// more easily.
class CFFASearchSettings {
public:
	CMapResourceToDateMap m_mapResToDate;
	CMapResourceToID m_mapResItemToID;
	COleDateTime m_dtStartDate;
	int m_nHours;			// Number of hours the appointment spans
	int m_nMinutes;			// Number of minutes the appointment spans
	int m_nArrivalMins;     // Number of minutes the arrival time should be before the start time
	int m_nSearchIntervalMinutes;
	long m_nAptType;
	long m_nAptPurpose;
	DWORD m_dwPatientID;
	BOOL m_bAnyOpenResource;
	//(e.lally 2011-05-16) PLID 41013 - Variables for preselecting values on a revised search 
	BOOL m_bAnyTimeChecked;
	BOOL m_bStartDateRadioChecked;
	BOOL m_bUserEditedDuration;
	BOOL m_bUseTemplateRules;
	long m_nOverBookingAllowed; //Number of conflicting appointments allowed
	BOOL m_bExcludeTemplatesWithWarnings;
	BOOL m_aResWeekdays[7];
	// (z.manning 2010-10-28 15:51) - PLID 41177 - These are now arrays
	CArray<LPARAM, LPARAM> m_arynWeekDayPrefList;
	CArray<LPARAM, LPARAM> m_arynResourceIDs;
	CArray<CString, CString&> m_aryResourceNames;
	long m_nResourceTypeSelection;//(s.dhole 5 / 22 / 2015 12:51 PM) - PLID 65621
	BOOL m_bSearchOfficeHoursOnly;

	long m_nDaysToSearch;
	// (r.gonet 2014-11-19) - PLID 64174 - Added some stored insurance variables so we can reference what the
	// user selected from the insured party dropdown when we go and create the appointment from the FFA results list.
	CFFAInsurancePtr m_pInsurance;
	// (a.wilson 2014-02-04 16:00) - PLID 15410 - updated to handle all hour selections.
	long m_anTimePrefList[24];
	CDWordArray m_adwAptResources;
	TimeList m_abAvailTimes;
	CString m_strResourceNameList;
	long m_nNumResources;
	// (r.farnworth 2015-06-08 10:26) - PLID 65635 - Need to track what location the user selected on the FFA search
	CArray<long, long> m_arynAptLocations;

	// (r.gonet 2014-12-17) - PLID 64464 - Constructs the set of FFA settings with default values.
	CFFASearchSettings();
	// (r.gonet 2014-12-17) - PLID 64464 - Copies FFA settings from an existing settings object. Performs a deep copy.
	CFFASearchSettings(const CFFASearchSettings &other);
	// (r.gonet 2014-12-17) - PLID 64464 - Frees up memory allocated to the search settings.
	~CFFASearchSettings();

	// (r.gonet 2014-12-17) - PLID 64464 - Clears the search setting resource maps. Frees memory.
	void ClearResourceMaps();
};
typedef shared_ptr<CFFASearchSettings> CFFASearchSettingsPtr;

// (z.manning 2010-11-01 14:11) - PLID 41272 - Data needed for a find first available opening
// (r.gonet 2014-12-17) - PLID 64464 - Moved to the CommonFFAUtils.h file from FirstAvailList.cpp
struct FFAOpening {
	COleDateTime dt;
	CString strResourceIDs;
	CString strResourceNames;
	// (j.politis 2015-07-09 16:15) - PLID 65630 - Practice-side read the new location info from the API response into structures in memory
	CString strLocationName;
	long nLocationID;
	CString strTemplateNames;

	// (b.cardillo 2016-01-31 14:08) - PLID 65630 - We don't try to use location color anymore.
	FFAOpening(const COleDateTime dtDate, const CString &strResIDs, const CString &strResNames, const CString &strLocName, long nLocID, const CString &strTempNames)
	{
		dt = dtDate;
		strResourceIDs = strResIDs;
		strResourceNames = strResNames;
		strLocationName = strLocName;
		nLocationID = nLocID;
		strTemplateNames = strTempNames;
	}

protected:
	FFAOpening();
};
typedef shared_ptr<FFAOpening> FFAOpeningPtr;

// (z.manning 2010-11-01 13:58) - PLID 41272 - This data is passed from the FFA query thread back to the
// main thread which then processes it and updates the UI.
// (r.gonet 2014-12-17) - PLID 64464 - Moved to the CommonFFAUtils.h file from FirstAvailList.cpp
struct FirstAvailListThreadData {
	CArray<FFAOpening*, FFAOpening*> arypOpenings;

	void CleanUp()
	{
		while (arypOpenings.GetSize() > 0) {
			// (r.gonet 2014-12-17) - PLID 64464 - Just remove. Don't delete.
			arypOpenings.RemoveAt(0);
		}
	}
};

class CFFASearchThread {
private:
	// (d.thompson 2010-11-01) - PLID 41274 - Thread data for launching the PopulateList thread
	// (r.gonet 2014-12-17) - PLID 64464 - Moved to the CommonFFAUtils.h file, CFFASearchThread, from FirstAvailList.cpp, global scope
	class CFFAThreadData {
	public:
		HWND m_hWnd;
		int m_nHours;
		int m_nMinutes;
		int m_nSearchIntervalMinutes;
		long m_nDaysToSearch;
		int m_nOverBookingAllowed;
		int m_nAptType;
		int m_nAptPurpose;
		BOOL m_bExcludeTemplatesWithWarnings;
		CArray<long, long> m_arynWeekDayPrefList;
		BOOL m_bAnyOpenResource;
		BOOL m_bSearchOfficeHoursOnly;
		COleDateTime m_dtStartDate;
		CArray<long, long> m_arynResourceIDs;
		long m_nNumResources;
		// (a.wilson 2014-02-04 16:01) - PLID 15410 - updated to handle all hour selections.
		long m_anTimePrefList[24];
		// (r.gonet 2014-11-19) - PLID 64173 - We need the insurance company that was selceted in the
		// insured party dropdown in order to pass it to the API and filter on the insurance when doing
		// scheduling mix rule filtering.
		long m_nInsuranceCoID;
		// (r.farnworth 2015-06-10 09:44) - PLID 65640 - We're adding LocationID to this search
		CArray<long, long> m_arynLocationIDs;
	};

public:
	// (r.gonet 2014-12-17) - PLID 64464 - Contains the time slot openings found by the FFA search.
	// This class frees the FFAOpening objects when the destructor is called.
	std::vector<FFAOpening *> m_vecFFAResultSet;
private:
	// (r.gonet 2014-12-17) - PLID 64464 - Thread to perform the FFA Search.
	NxThread m_thread;
	// (r.gonet 2014-12-17) - PLID 64464 - Handle to the finished event. Defaults to not set.
	// Set when the FFA thread finished gathering FFA results.
	HANDLE m_hFinishedEvent = NULL;

public:
	// (r.gonet 2014-12-17) - PLID 64464 - Constructs a new FFA Search Thread
	CFFASearchThread();
	// (r.gonet 2014-12-17) - PLID 64464 - Destructor
	~CFFASearchThread();

	// (r.gonet 2014-12-17) - PLID 64464 - Gets whether the thread is running or not.
	bool IsRunning();
	// (r.gonet 2014-12-17) - PLID 64464 - Gets whether the thread has finished the FFA search.
	bool IsFinished();
	// (r.gonet 2014-12-17) - PLID 64464 - Blocks until the thread has finished with the FFA search.
	// Returns true if the FFA search finished and false otherwise.
	bool WaitUntilFinished();
	// (r.gonet 2014-12-17) - PLID 64464 - Prematurely ends the FFA search thread.
	void Interrupt();
	// (r.gonet 2014-12-17) - PLID 64464 - Waits until the FFA search thread ends.
	void Join();
	// (r.gonet 2014-12-17) - PLID 64464 - Runs the FFA search with given settings.
	// The thread will post progress messages to hwndNotify. hwndNotify may be NULL 
	// to avoid having messages be posted.
	void Run(HWND hwndNotify, CFFASearchSettingsPtr pSettings);
private:
	// (r.gonet 2014-12-17) - PLID 64464 - Creates a CFFAThreadData object from FFA search settings and the handle
	// to the window that the FFA thread will notify with messages. hNotifyWindow may be NULL, in which 
	// case, no messages are sent.
	CFFAThreadData * CreateFFAThreadData(HWND hwndNotify, CFFASearchSettingsPtr pFFASearchSettings);
	// (r.gonet 2014-12-17) - PLID 64464 - Thread start function to run FFA.
	LRESULT PerformFFASearch(CFFAThreadData *pThreadData);
};

//TES 12/18/2014 - PLID 64466 - Used when FFA returns slot information rather than creating an appointment
class SelectedFFASlot
{
public:
	COleDateTime dtStart;
	COleDateTime dtEnd;
	COleDateTime dtArrival;
	CDWordArray dwaResourceIDs;
	long nLocationID; // (r.farnworth 2016-02-02 08:00) - PLID 68116

	SelectedFFASlot() {
		dtStart = g_cdtNull;
		dtEnd = g_cdtNull;
		dtArrival = g_cdtNull;
		nLocationID = -1;
	}

	bool IsValid() {
		if (dtStart.GetStatus() == COleDateTime::valid
			&& dtEnd.GetStatus() == COleDateTime::valid
			&& dtArrival.GetStatus() == COleDateTime::valid
			&& dwaResourceIDs.GetSize() > 0
			&& nLocationID > 0) {
			
			return true;
		}
		else {
			return false;
		}
	}
};
typedef shared_ptr<SelectedFFASlot> SelectedFFASlotPtr;
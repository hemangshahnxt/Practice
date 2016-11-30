#include "stdafx.h"
#include "NxAPI.h"
#include <NxDataUtilitiesLib/NxSafeArray.h>
#include "CommonFFAUtils.h"
#include <NxAPILib/NxAPIUtils.h>

// (r.gonet 2014-12-17) - PLID 64464 - Created this file to store some common classes and defines that are used in
// both the text-list mode and non-text list mode of the FFA.

// (r.gonet 2014-11-19) - PLID 64174 - Constructs a new FFA insurance object with default values.
CFFAInsurance::CFFAInsurance()
{
	m_nInsuredPartyID = -1;
	m_nInsuranceCoID = -1;
	m_strInsuranceCoName = "";
	m_strRespType = "";
	m_nCategoryID = -1;
}

// (r.gonet 2014-12-17) - PLID 64464 - Copy constructor
CFFAInsurance::CFFAInsurance(const CFFAInsurance &other)
{
	m_nInsuredPartyID = other.m_nInsuredPartyID;
	m_nInsuranceCoID = other.m_nInsuranceCoID;
	m_strInsuranceCoName = other.m_strInsuranceCoName;
	m_strRespType = other.m_strRespType;
	m_nCategoryID = other.m_nCategoryID;
}

// (r.gonet 2014-12-17) - PLID 64464 - Constructs the set of FFA settings with default values.
CFFASearchSettings::CFFASearchSettings()
{
	//
	// (c.haag 2006-11-27 11:23) - PLID 20772 - Now clear everything else.
	// Most of these will be overwritten by a successive search, but it will
	// help us find out what information is not filled in that should be when
	// debugging
	//
	m_dtStartDate.SetStatus(COleDateTime::invalid);
	m_nHours = 0;
	m_nMinutes = 0;
	m_nArrivalMins = 0;
	m_nSearchIntervalMinutes = 0;
	m_nDaysToSearch = 0;
	m_nAptType = -1;
	m_nAptPurpose = -1;
	m_nOverBookingAllowed = 0;
	m_nNumResources = 0;
	m_dwPatientID = -25;
	m_bAnyOpenResource = FALSE;
	m_bUseTemplateRules = FALSE;
	m_bSearchOfficeHoursOnly = FALSE;
	m_bExcludeTemplatesWithWarnings = FALSE;
	// (r.gonet 2014-11-19) - PLID 64173 - No insurance set by default.
	m_pInsurance.reset();
	//(e.lally 2011-05-16) PLID 41013
	m_bAnyTimeChecked = FALSE;
	m_bStartDateRadioChecked = TRUE;
	m_bUserEditedDuration = FALSE;
}

// (r.gonet 2014-12-17) - PLID 64464 - Copies FFA settings from an existing settings object. Performs a deep copy.
CFFASearchSettings::CFFASearchSettings(const CFFASearchSettings &other)
{
	POSITION pos = other.m_mapResToDate.GetStartPosition();
	while (pos) {
		CString strOtherKey;
		CMapDateTimeToTimeList *pOtherValue;
		other.m_mapResToDate.GetNextAssoc(pos, strOtherKey, pOtherValue);
		CMapDateTimeToTimeList *pMyValue = new CMapDateTimeToTimeList();
		POSITION pos2 = pOtherValue->GetStartPosition();
		while (pos2) {
			COleDateTime dtOtherKey2;
			long *pOtherValue2;
			pOtherValue->GetNextAssoc(pos2, dtOtherKey2, pOtherValue2);

			long *pMyValue2;
			if (pOtherValue2 != NULL) {
				pMyValue2 = new long(*pOtherValue2);
			} else {
				pMyValue2 = NULL;
			}
			(*pMyValue)[dtOtherKey2] = pMyValue2;
		}

		this->m_mapResToDate[strOtherKey] = pMyValue;
	}

	pos = other.m_mapResItemToID.GetStartPosition();
	while (pos) {
		CString strOtherKey;
		DWORD dwOtherValue;
		other.m_mapResItemToID.GetNextAssoc(pos, strOtherKey, dwOtherValue);
		this->m_mapResItemToID[strOtherKey] = dwOtherValue;
	}

	m_dtStartDate = other.m_dtStartDate;
	m_nHours = other.m_nHours;
	m_nMinutes = other.m_nMinutes;
	m_nArrivalMins = other.m_nArrivalMins;
	m_nSearchIntervalMinutes = other.m_nSearchIntervalMinutes;
	m_nAptType = other.m_nAptType;
	m_nAptPurpose = other.m_nAptPurpose;
	m_dwPatientID = other.m_dwPatientID;
	m_bAnyOpenResource = other.m_bAnyOpenResource;
	m_bAnyTimeChecked = other.m_bAnyTimeChecked;
	m_bStartDateRadioChecked = other.m_bStartDateRadioChecked;
	m_bUserEditedDuration = other.m_bUserEditedDuration;
	m_bUseTemplateRules = other.m_bUseTemplateRules;
	m_nOverBookingAllowed = other.m_nOverBookingAllowed;
	m_bExcludeTemplatesWithWarnings = other.m_bExcludeTemplatesWithWarnings;
	for (int i = 0; i < 7; i++) {
		m_aResWeekdays[i] = other.m_aResWeekdays[i];
	}
	m_arynWeekDayPrefList.Copy(other.m_arynWeekDayPrefList);
	m_arynResourceIDs.Copy(other.m_arynResourceIDs);
	m_aryResourceNames.Copy(other.m_aryResourceNames);
	m_bSearchOfficeHoursOnly = other.m_bSearchOfficeHoursOnly;

	m_nDaysToSearch = other.m_nDaysToSearch;
	if (other.m_pInsurance != NULL) {
		m_pInsurance.reset(new CFFAInsurance(const_cast<const CFFAInsurance&>(*other.m_pInsurance.get())));
	}
	for (int i = 0; i < 24; i++) {
		m_anTimePrefList[i] = other.m_anTimePrefList[i];
	}
	m_adwAptResources.Copy(other.m_adwAptResources);
	for (int i = 0; i < FFA_TIME_LIST_SIZE; i++) {
		m_abAvailTimes[i] = other.m_abAvailTimes[i];
	}
	m_strResourceNameList = other.m_strResourceNameList;
	m_nNumResources = other.m_nNumResources;

	// (r.farnworth 2015-06-08 10:26) - PLID 65635 - Need to track what location the user selected on the FFA search
	m_arynAptLocations.Copy(other.m_arynAptLocations);
}

// (r.gonet 2014-12-17) - PLID 64464 - Frees up memory allocated to the search settings.
CFFASearchSettings::~CFFASearchSettings()
{
	ClearResourceMaps();
}

// (r.gonet 2014-12-17) - PLID 64464 - Clears the search setting resource maps. Frees memory.
void CFFASearchSettings::ClearResourceMaps()
{
	POSITION posRes, posDate;
	CString strResource;
	CMapDateTimeToTimeList* pTimeListMap;
	COleDateTime dt;
	long* pTimeList;

	//////////////////////////////////////////////
	// Free all the memory objects we made
	for (posRes = m_mapResToDate.GetStartPosition(); posRes;) {
		m_mapResToDate.GetNextAssoc(posRes, strResource, pTimeListMap);

		for (posDate = pTimeListMap->GetStartPosition(); posDate;) {
			pTimeListMap->GetNextAssoc(posDate, dt, pTimeList);

			delete pTimeList;
		}
		pTimeListMap->RemoveAll();
		delete pTimeListMap;
	}

	m_mapResToDate.RemoveAll();
	m_mapResItemToID.RemoveAll();
}

// (r.gonet 2014-12-17) - PLID 64464 - Constructs a new FFA Search Thread
CFFASearchThread::CFFASearchThread()
{
	// (r.gonet 2014-12-17) - PLID 64464 - Initialize the finished event.
	m_hFinishedEvent = CreateEvent(NULL, 0, FALSE, NULL);
}

// (r.gonet 2014-12-17) - PLID 64464 - Destructor
CFFASearchThread::~CFFASearchThread()
{
	CWaitCursor pWait;
	// (r.gonet 2014-12-17) - PLID 64464 - End the thread, possibly early
	if(IsRunning()) {
		Interrupt();
		Join();
	}
	m_thread.Reset();
	// (r.gonet 2014-12-17) - PLID 64464 - Free all the data that we got back from the FFA.
	for (size_t i = 0; i < m_vecFFAResultSet.size(); i++) {
		delete m_vecFFAResultSet[i];
		m_vecFFAResultSet[i] = NULL;
	}
	CloseHandle(m_hFinishedEvent);
	m_hFinishedEvent = NULL;
}

// (r.gonet 2014-12-17) - PLID 64464 - Gets whether the thread is running or not.
bool CFFASearchThread::IsRunning()
{
	return m_thread;
}

// (r.gonet 2014-12-17) - PLID 64464 - Gets whether the thread has finished the FFA search.
bool CFFASearchThread::IsFinished()
{
	if (m_hFinishedEvent == NULL) {
		ThrowNxException("%s : m_hFinishedEvent is NULL", __FUNCTION__);
	}

	return WAIT_TIMEOUT != ::WaitForSingleObject(m_hFinishedEvent, 0);
}

// (r.gonet 2014-12-17) - PLID 64464 - Blocks until the thread has finished with the FFA search.
// Returns true if the FFA search finished successfully and false otherwise.
bool CFFASearchThread::WaitUntilFinished()
{
	if (m_thread == NULL || m_thread->m_hThread == NULL) {
		return false;
	}

	if (m_hFinishedEvent == NULL) {
		ThrowNxException("%s : m_hFinishedEvent is NULL", __FUNCTION__);
	}

	const HANDLE aryWaitHandles[2] = { m_hFinishedEvent, m_thread->m_hThread };

	// Waits until either the finished event is set or the thread has terminated.
	// If the thread has terminated, then the FFA didn't finish successfully.
	DWORD ret = ::WaitForMultipleObjects(2, aryWaitHandles, FALSE, INFINITE);
	if ((ret - WAIT_OBJECT_0) == 0) {
		return true;
	}

	return false;
}

// (r.gonet 2014-12-17) - PLID 64464 - Prematurely ends the FFA search thread.
void CFFASearchThread::Interrupt()
{
	if (IsRunning()) {
		m_thread->Interrupt();
	}
}

// (r.gonet 2014-12-17) - PLID 64464 - Waits until the FFA search thread ends.
void CFFASearchThread::Join()
{
	if (IsRunning()) {
		m_thread->Join();
	}
}

// (r.gonet 2014-12-17) - PLID 64464 - Runs the FFA search with given settings.
// The thread will post progress messages to hwndNotify. hwndNotify may be NULL 
// to avoid having messages be posted.
void CFFASearchThread::Run(HWND hwndNotify, CFFASearchSettingsPtr pSettings)
{
	if (pSettings == NULL) {
		ThrowNxException("%s : pSettings is NULL", __FUNCTION__);
	}
	if (!m_hFinishedEvent) {
		m_hFinishedEvent = CreateEvent(NULL, 0, FALSE, NULL);
	}

	if (IsRunning()) {
		Interrupt();
		Join();
	}

	// (r.gonet 2014-12-17) - PLID 64464 - Create a container object to pass arguments to the FFA thread.
	CFFAThreadData *pThreadData = CreateFFAThreadData(hwndNotify, pSettings);

	m_thread = NxThread(boost::bind(&CFFASearchThread::PerformFFASearch, this, pThreadData));
}

// (r.gonet 2014-12-17) - PLID 64464 - Creates a CFFAThreadData object from FFA search settings and the handle
// to the window that the FFA thread will notify with messages. hNotifyWindow may be NULL, in which 
// case, no messages are sent.
CFFASearchThread::CFFAThreadData * CFFASearchThread::CreateFFAThreadData(HWND hNotifyWindow, CFFASearchSettingsPtr pFFASearchSettings)
{
	if (pFFASearchSettings == NULL) {
		ThrowNxException("%s : pFFASearchSettings is NULL", __FUNCTION__);
	}

	CFFAThreadData *pData = new CFFAThreadData;
	pData->m_hWnd = hNotifyWindow;
	pData->m_nHours = pFFASearchSettings->m_nHours;
	pData->m_nMinutes = pFFASearchSettings->m_nMinutes;
	pData->m_nSearchIntervalMinutes = pFFASearchSettings->m_nSearchIntervalMinutes;
	pData->m_nDaysToSearch = pFFASearchSettings->m_nDaysToSearch;
	pData->m_nOverBookingAllowed = pFFASearchSettings->m_nOverBookingAllowed;
	pData->m_nAptType = pFFASearchSettings->m_nAptType;
	pData->m_nAptPurpose = pFFASearchSettings->m_nAptPurpose;
	pData->m_bExcludeTemplatesWithWarnings = pFFASearchSettings->m_bExcludeTemplatesWithWarnings;
	pData->m_arynWeekDayPrefList.Copy(pFFASearchSettings->m_arynWeekDayPrefList);
	pData->m_bAnyOpenResource = pFFASearchSettings->m_bAnyOpenResource;
	pData->m_bSearchOfficeHoursOnly = pFFASearchSettings->m_bSearchOfficeHoursOnly;
	pData->m_dtStartDate = pFFASearchSettings->m_dtStartDate;
	pData->m_arynResourceIDs.Copy(pFFASearchSettings->m_arynResourceIDs);
	pData->m_nNumResources = pFFASearchSettings->m_nNumResources;
	// (r.farnworth 2015-06-10 09:51) - PLID 65640
	pData->m_arynLocationIDs.Copy(pFFASearchSettings->m_arynAptLocations);
	// (r.gonet 2014-11-19) - PLID 64174 - Pass the insurance company ID to the thread so it can pass it to the API.
	if (pFFASearchSettings->m_pInsurance != NULL) {
		pData->m_nInsuranceCoID = pFFASearchSettings->m_pInsurance->m_nInsuranceCoID;
	} else {
		pData->m_nInsuranceCoID = -1;
	}
	// (a.wilson 2014-02-04 16:01) - PLID 15410 - update to handle all hour selections.
	memcpy(pData->m_anTimePrefList, pFFASearchSettings->m_anTimePrefList, 24 * sizeof(int));	//harcoded to 24 elements

	return pData;
}

// (r.gonet 2014-12-17) - PLID 64464 - Thread start function to run FFA. Moved from CFirstAvailList.cpp's global PopulateList() function.
LRESULT CFFASearchThread::PerformFFASearch(CFFASearchThread::CFFAThreadData *pThreadData)
{
	try {
#ifdef _DEBUG
		dbg_SetThreadName(GetCurrentThreadId(), "FFASearch");
#endif

		// (z.manning 2013-11-15 15:11) - PLID 58756 - Completely reworked this to use the API

		CoInitialize(NULL);

		NexTech_Accessor::_FFARequestInfoPtr pFFARequest(__uuidof(NexTech_Accessor::FFARequestInfo));

		COleDateTime dtStartDate = pThreadData->m_dtStartDate;
		dtStartDate.SetDateTime(dtStartDate.GetYear(), dtStartDate.GetMonth(), dtStartDate.GetDay(), 0, 0, 0);

		pFFARequest->AppointmentLengthMinutes = pThreadData->m_nHours * 60 + pThreadData->m_nMinutes;
		pFFARequest->SearchIntervalMinutes = pThreadData->m_nSearchIntervalMinutes;
		long nDaysToSearch = pThreadData->m_nDaysToSearch;
		pFFARequest->MaxConflicts = pThreadData->m_nOverBookingAllowed;
		pFFARequest->TypeID = _bstr_t(pThreadData->m_nAptType);
		pFFARequest->PurposeID = _bstr_t(pThreadData->m_nAptPurpose);
		pFFARequest->ExcludeWarnings = pThreadData->m_bExcludeTemplatesWithWarnings ? VARIANT_TRUE : VARIANT_FALSE;
		pFFARequest->AnyResource = pThreadData->m_bAnyOpenResource ? VARIANT_TRUE : VARIANT_FALSE;
		pFFARequest->DaysOfWeek = Nx::SafeArray<long>::From(pThreadData->m_arynWeekDayPrefList);
		// (z.manning 2013-11-19 16:47) - PLID 59638 - Set the office hours option
		pFFARequest->OfficeHoursOnly = pThreadData->m_bSearchOfficeHoursOnly ? VARIANT_TRUE : VARIANT_FALSE;
		// (r.gonet 2014-11-19) - PLID 64174 - Pass in the optional insurance company to the FFA slot calculation.
		// This will be used in scheduling mix rule filtering, which will remove slots that cannot be scheduled in
		// due to it not having any scheduling mix rule details with remaining appointments.
		if (pThreadData->m_nInsuranceCoID != -1) {
			pFFARequest->InsuranceCoID = _bstr_t(FormatString("%li", pThreadData->m_nInsuranceCoID));
		} else {
			// Leave it unspecified.
		}
		CArray<CString, LPCTSTR> aryResourceIDs;
		for (int nResourceIndex = 0; nResourceIndex < pThreadData->m_arynResourceIDs.GetCount(); nResourceIndex++) {
			aryResourceIDs.Add(AsString(pThreadData->m_arynResourceIDs.GetAt(nResourceIndex)));

		}
		pFFARequest->ResourceIDs = Nx::SafeArray<BSTR>::From(aryResourceIDs);
		// (b.cardillo 2016-01-31 15:22) - PLID 65649 - Set the new parameter to run ffa by location
		pFFARequest->IncludeLocationSpecificResults = NexTech_Accessor::GetNullableBool(TRUE);

		

		// (z.manning 2013-11-19 16:50) - PLID 59638 - No need to do this if they have the office hours options checked
		// (j.politis 2015-06-22 16:09) - PLID 66414 - Change the FFA UI in Practice when the user checks the “Only Include Office Hours” checkbox to no longer disable the time preference UI elements, and still pass both the checkbox state and the time preference to the API method call.
		CArray<NexTech_Accessor::_FFATimeRangePtr, NexTech_Accessor::_FFATimeRangePtr> aryTimeRanges;
		//For each block of time preferences, we need a separate entry in the ad-hoc procedure so generate 
		//the dynamic number of inserts needed, taking into account the Any or Every resource preference
		// (a.wilson 2014-02-04 16:01) - PLID 15410 - update to handle all hour selections.
		// (j.politis 2015-06-25 11:22) - PLID 65642 - Extend the time preferences listbox to go from midnight to midnight.
		for (short nTimePrefIndex = 0; nTimePrefIndex < 24; nTimePrefIndex++) {
			if (pThreadData->m_anTimePrefList[nTimePrefIndex]) {
				long nStart = nTimePrefIndex;
				//if there are back to back selected time preferences selected, put them together as one entry
				nTimePrefIndex++;
				while (nTimePrefIndex < 24 && pThreadData->m_anTimePrefList[nTimePrefIndex]) {
					nTimePrefIndex++;
				}

				NexTech_Accessor::_FFATimeRangePtr pTimeRange(__uuidof(NexTech_Accessor::FFATimeRange));
				pTimeRange->StartHour = (BYTE)nStart;
				pTimeRange->EndHour = (BYTE)nTimePrefIndex;
				aryTimeRanges.Add(pTimeRange);
			}
		}
		pFFARequest->TimeRanges = Nx::SafeArray<IUnknown*>::From(aryTimeRanges);

		if (pThreadData->m_hWnd) {
			// (d.thompson 2010-11-01) - PLID 41274 - Start the progress bar.  We'll just use 100% as the guide.
			PostMessage(pThreadData->m_hWnd, NXM_FFA_SET_PROGRESS_MIN_MAX, (WPARAM)0, (LPARAM)100);
		} else {
			// (r.gonet 2014-12-17) - PLID 64464 - Don't post a message if no notify window.
		}

		//set the timeout for 3 minutes
		CIncreaseAPIHttpTimeout(GetAPI(), 180000);

		// (d.thompson 2010-11-01) - PLID 27107 - Major overhaul of the FFA process.  Previously, we executed 1 ginormous query.  The query itself looped from
		//	the start day for x number of days (search parameter defined).  This, however, can be quite slow, and take minutes to return on a large search.
		//Instead, the query is now designed to operate on just a single day.  If they're searching for 45 days, we'll issue 45 queries to get available
		//	open slot information about each day, and put them into the list.  Since this function is run in a thread, we'll send the results back to 
		//	put in the datalist as each day completes, resulting in a very big apparent speed improvement.
		//The general approach of each day remains that same as it was when the query did everything.
		BOOL bContinue = TRUE;
		for (int nDay = 0; nDay < nDaysToSearch && bContinue; nDay++) {
			// (d.thompson 2010-11-01) - PLID 41274 - Set the progress text to state what day we're evaluating
			if (pThreadData->m_hWnd) {
				//Memory allocation:  We call AllocSysString to get a BSTR here, then send it to the main thread.  That thread
				//	must free the string.
				CString strDay;
				strDay.Format("Examining day:  %s...", FormatDateTimeForInterface(dtStartDate));
				PostMessage(pThreadData->m_hWnd, NXM_FFA_SET_PROGRESS_TEXT, (WPARAM)strDay.AllocSysString(), 0);
			} else {
				// (r.gonet 2014-12-17) - PLID 64464 - Don't post a message if no notify window.
			}

			//First code check.  The sql query used to iterate from start date range, adding 1 day each time.  Each time it incremented the day, 
			//	it did a verification that the day was in the valid search arguments (i.e., that they want to search on MW, not TTH, etc).  We 
			//	now need to run that check first thing here, and skip the whole attempt on this day if it's not included.
			//strWeekDayPrefList is a comma delimited list that indicates the allowed days of week.  For example, "1,3,5" would be Sunday, Tuesday, Thursday.
			// (z.manning 2013-11-19 10:19) - PLID 58756 - Day of week is now zero-based, so subtract 1 here
			if (!IsIDInArray(dtStartDate.GetDayOfWeek() - 1, pThreadData->m_arynWeekDayPrefList)) {
				//The day of the week we're on is completely skipped in the search parameters, so there's no point executing any queries here.
				//Increment to the next day and move on
				dtStartDate += COleDateTimeSpan(1, 0, 0, 0);

				if (pThreadData->m_hWnd) {
					// (d.thompson 2010-11-01) - PLID 41274 - Also step the progress bar appropriately, this counts as a day that we checked.  Send as a 
					//	percentage of 100.
					PostMessage(pThreadData->m_hWnd, NXM_FFA_SET_PROGRESS_POSITION, (WPARAM)(short)(((double)(nDay + 1) / (double)nDaysToSearch)*100.0), 0);
				} else {
					// (r.gonet 2014-12-17) - PLID 64464 - Don't post a message if no notify window.
				}
				continue;
			}

			pFFARequest->StartDate = dtStartDate;
			pFFARequest->DaysToSearch = 1;

			// (z.manning 2013-11-19 10:28) - PLID 58756 - Call the API to do the FFA search
			NexTech_Accessor::_FFAResultsPtr pResults =
				GetAPI()->FindFirstAvailableAppointment(GetAPISubkey(), GetAPILoginToken(), pFFARequest);

			FirstAvailListThreadData *pFFAThreadData = new FirstAvailListThreadData();
			Nx::SafeArray<IUnknown*> saryResults(pResults->Results);
			foreach(NexTech_Accessor::_FFAResultPtr pResult, saryResults)
			{
				CString strResourceIDs, strResourceNames;
				Nx::SafeArray<IUnknown*> saryResources(pResult->Resources);
				foreach(NexTech_Accessor::_FFAResourcePtr pResource, saryResources)
				{
					strResourceIDs += CString((LPCTSTR)pResource->ID) + " ";
					if (!strResourceNames.IsEmpty()) {
						strResourceNames += ", ";
					}
					strResourceNames += (LPCTSTR)pResource->Name;
				}
				strResourceIDs.TrimRight();

				CString strTemplateNames;
				Nx::SafeArray<BSTR> saryTemplateNames(pResult->TemplateNames);
				foreach(BSTR bstrTemplateName, saryTemplateNames)
				{
					if (!strTemplateNames.IsEmpty()) {
						strTemplateNames += ", ";
					}
					strTemplateNames += CString(bstrTemplateName);
				}

				// (j.politis 2015-07-09 16:15) - PLID 65630 - Practice-side read the new location info from the API response into structures in memory
				// (b.cardillo 2016-01-31 14:08) - PLID 65630 - API only returns locationID and locationName. Empty ID for old still (no-location) results.
				CString strLocationName;
				long nLocationID;
				if (pResult->locationID.length() == 0)
				{
					strLocationName = "";
					nLocationID = -1;
				}
				else
				{
					strLocationName = (LPCTSTR)pResult->LocationName;
					nLocationID = atol(CString((LPCTSTR)pResult->locationID));
				}

				// (b.cardillo 2016-01-31 14:08) - PLID 65630 - We don't try to use location color anymore.
				FFAOpening *pOpening = new FFAOpening(pResult->date, strResourceIDs, strResourceNames, strLocationName, nLocationID, strTemplateNames);
				pFFAThreadData->arypOpenings.Add(pOpening);
				// (r.gonet 2014-12-17) - PLID 64464 - Add the opening to our result set
				m_vecFFAResultSet.push_back(pOpening);
			}

			if (pThreadData->m_hWnd) {
				// (z.manning 2010-11-01 15:55) - PLID 41272 - Post a message to the main thread to process this data
				// and update the UI.
				PostMessage(pThreadData->m_hWnd, NXM_FFA_LIST_PROCESS_DATA, (WPARAM)pFFAThreadData, 0);
			} else {
				// (r.gonet 2014-12-17) - PLID 64464 - Don't post a message if no notify window. Cleanup the thread data. We don't need it afterall.
				delete pFFAThreadData;
			}

			// (d.thompson 2010-11-01) - PLID 27107 - At the end of the code loop, we need to increment the day.  The top of the loop
			//	will check to ensure this day is within applicable search parameters.
			dtStartDate += COleDateTimeSpan(1, 0, 0, 0);

			if (pThreadData->m_hWnd) {
				// (d.thompson 2010-11-01) - PLID 41274 - Also step the progress bar appropriately.  Send as a percentage of 100.
				PostMessage(pThreadData->m_hWnd, NXM_FFA_SET_PROGRESS_POSITION, (WPARAM)(short)(((double)(nDay + 1) / (double)nDaysToSearch)*100.0), 0);
			} else {
				// (r.gonet 2014-12-17) - PLID 64464 - Don't post a message if no notify window.
			}

			// (z.manning 2010-11-02 09:54) - PLID 41272 - Check and see if we should stop running this thread.
			// (r.gonet 2014-12-17) - PLID 64464 - Changed to use IsInterrupted() instead of a global event handle.
			if (m_thread->IsInterrupted()) {
				bContinue = FALSE;
			}
		}

		//Final status message
		// (d.thompson 2010-11-01) - PLID 41274 - Set the progress text to state what day we're evaluating
		if (pThreadData->m_hWnd)
		{
			//Memory allocation:  We call AllocSysString to get a BSTR here, then send it to the main thread.  That thread
			//	must free the string.
			CString strMsg;
			strMsg.Format("Completed Search!");
			PostMessage(pThreadData->m_hWnd, NXM_FFA_SET_PROGRESS_TEXT, (WPARAM)strMsg.AllocSysString(), 0);
		} else {
			// (r.gonet 2014-12-17) - PLID 64464 - Don't post a message if no notify window.
		}
		// (r.gonet 2014-12-17) - PLID 64464 - Set the event that we have finished the FFA search.
		SetEvent(m_hFinishedEvent);
	}
	NxCatchAllThread(__FUNCTION__);

	if (pThreadData->m_hWnd) {
		// (c.haag 2006-11-27 13:45) - PLID 20772 - Here we normally do some direct window
		// accesses. We should not do them here; we should do them from the main thread.
		PostMessage(pThreadData->m_hWnd, NXM_FFA_LIST_REQUERY_FINISHED, 0, 0);
	} else {
		// (r.gonet 2014-12-17) - PLID 64464 - Don't post a message if no notify window.
	}

	//We're done with our thread data, destroy it
	delete pThreadData;

	CoUninitialize();

	return 0;
}
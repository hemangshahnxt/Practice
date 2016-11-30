#include "stdafx.h"
#include "MedicationHistoryUtils.h"
#include <NxSystemUtilitiesLib/NxThread.h>
using namespace ADODB;

// (r.gonet 09/20/2013) - PLID 58396 - Starts a thread to request medication history for a patient.
// - hwndNotify: If it is determined that the patient has medication history and no request needs
//   be made, then the thread will send a NXM_BACKGROUND_MED_HISTORY_REQUEST_COMPLETE message to hwndNotify.
// - nPatientID: The person ID of the patient for which to request medication history.
// Note: Since medication history requests are asynchronous, callers should also listen for the 
//   table checker on MedicationHistoryResponseT.
void MedicationHistoryUtils::BeginBackgroundRequestMedHistory(HWND hwndNotify, long nPatientID)
{
	try {
		return;
		// Create a thread to request the history.
		//NxThread(boost::bind(&MedicationHistoryUtils::BackgroundRequestMedHistory, hwndNotify, nPatientID));
	} NxCatchAllThread(__FUNCTION__);
}

// (r.gonet 09/20/2013) - PLID 58396 - Requests medication history.
// - hwndNotify: If it is determined that the patient has medication history and no request needs
//   be made, then the thread will send a NXM_BACKGROUND_MED_HISTORY_REQUEST_COMPLETE message to hwndNotify.
// - nPatientID: The patient to request medication history for.
// Note: Since medication history requests are asynchronous, callers should also listen for the 
//   table checker on MedicationHistoryResponseT.
void MedicationHistoryUtils::BackgroundRequestMedHistory(HWND hwndNotify, long nPatientID)
{
#ifdef _DEBUG
	dbg_SetThreadName(GetCurrentThreadId(), "RxHistory");
#endif

	if(!(GetCurrentUserPermissions(bioPatientMedicationHistory) & sptRead)) {
		// We need to be able to determine if there is history pre-existing.
		return;
	}
	if(nPatientID <= 0) {
		// Invalid patient ID.
		return;
	}

	// Check to see if we have recently made a request. If so, then we don't need to make another.
	NexTech_Accessor::_RxHistoryMasterRequestPtr pMasterRequest = MedicationHistoryUtils::GetLastRxHistoryMasterRequest(nPatientID);
	if(pMasterRequest) {
		COleDateTime dtNow = COleDateTime::GetCurrentTime();
		COleDateTime dtToday;
		dtToday.SetDate(dtNow.GetYear(), dtNow.GetMonth(), dtNow.GetDay());
		if(pMasterRequest->RequestedDate >= dtToday) {
			// (r.gonet 12/16/2013) - PLID 58199 - Changed public API arguments from integer to string.
			long nMostRecentMasterRequestID = atol((LPCTSTR)pMasterRequest->ID);
			// We already have gotten history for this patient today. Don't get it again. But load up the history.
			BOOL bHasMedicationHistory = MedicationHistoryUtils::HasMedicationHistory(nPatientID, nMostRecentMasterRequestID) ? TRUE : FALSE;
			::PostMessage(hwndNotify, NXM_BACKGROUND_MED_HISTORY_REQUEST_COMPLETE, (WPARAM)nPatientID, (LPARAM)bHasMedicationHistory);
			return;
		} else {
			// The information is potentially out of date. Re-request it.
			MedicationHistoryUtils::RequestMedicationHistory(nPatientID);
		}
	} else {
		// We have never requested history for this patient. Request it.
		MedicationHistoryUtils::RequestMedicationHistory(nPatientID);
	}
}

// (r.gonet 09/20/2013) - PLID 57978 - Gets the patient's most recent (master) request for medication history.
// - nPatientID: The person ID of the patient for which to check for the medication history request.
// Returns: The master request of the patient's most recent medication history request. NULL if no prior
//   request was found.
NexTech_Accessor::_RxHistoryMasterRequestPtr MedicationHistoryUtils::GetLastRxHistoryMasterRequest(long nPatientID)
{
	// (a.walling 2014-01-06 12:43) - PLID 59996 - No connection pointers for these existing patient functions
	long nUserDefinedID = GetExistingPatientUserDefinedID(nPatientID);
	// Get the last medication history request
	NexTech_Accessor::_NullableRxHistoryMasterRequestPtr pNullableMasterRequest = GetAPI()->GetLastRxHistoryMasterRequest(GetAPISubkey(), GetAPILoginToken(), _bstr_t(FormatString("%li", nUserDefinedID)));	
	if(pNullableMasterRequest != NULL && pNullableMasterRequest->value != NULL) {
		// We have requested for this patient before.
		return pNullableMasterRequest->value;
	} else {
		// We have never requested for this patient before.
		return NULL;
	}
}

// (r.gonet 09/20/2013) - PLID 58396 - Gets whether or not the patient has medication history.
// - nPatientID: The person ID of the patient for which to check for the medication history.
// - nMasterRequestID: The medication history master request the responses must be associated with.
//   Pass -1 for any medication history master request.
// Returns: true if the patient has medication history. false otherwise.
bool MedicationHistoryUtils::HasMedicationHistory(long nPatientID, long nMasterRequestID/*=-1*/)
{
	// (r.gonet 12/20/2013) - PLID 57957 - Had to change the master request ID public API argument from int to string.
	CString strRxHistoryMasterRequestID;
	if(nMasterRequestID != -1) {
		// Only get history associated with this master request.
		strRxHistoryMasterRequestID.Format("%li", nMasterRequestID);
	} else {
		// Get history associated with any request.
	}

	// (r.gonet 12/20/2013) - PLID 57957 - Had to change the master request ID public API argument from int to string.
	// Get the patient's medication history results
	NexTech_Accessor::_PatientRxHistoryArrayPtr pRxHistoryResults = GetAPI()->GetPatientRxHistory(GetAPISubkey(), GetAPILoginToken(), bstr_t(nPatientID), _bstr_t(strRxHistoryMasterRequestID));
	if(pRxHistoryResults->PatientRxHistories != NULL) {
		Nx::SafeArray<IUnknown*> saryRxHistoryResponses(pRxHistoryResults->PatientRxHistories);
		foreach(NexTech_Accessor::_PatientRxHistoryPtr pRxHistoryResponse, saryRxHistoryResponses) {
			Nx::SafeArray<IUnknown*> saryDispensedMedications(pRxHistoryResponse->DispensedMedicationInfos);
			Nx::SafeArray<IUnknown*> saryPrescribedMedications(pRxHistoryResponse->PrescribedMedicationInfos);
			if(saryDispensedMedications.GetSize() > 0 || saryPrescribedMedications.GetSize() > 0) {
				// The patient has some medications in their history. That's all we need to know.
				return true;
			} else {
				// Continue checking for medications.
			}
		}
	} else {
		// The patient has no history.
	}
	// The patient has no medications from medication history.
	return false;
}

// (r.gonet 09/20/2013) - PLID 58396 - Starts a new request for patient medication history.
// - nPatientID: The person ID of the patient for which to check for the medication history.
// Returns: The master request that was represents this medication history request. NULL if the request failed.
// Notes: Callers should listen for the table checker MedicationHistoryResponseT for the responses.
NexTech_Accessor::_RxHistoryMasterRequestPtr MedicationHistoryUtils::RequestMedicationHistory(long nPatientID)
{
	if(!(GetCurrentUserPermissions(bioPatientMedicationHistory) & (sptDynamic0|sptDynamic0WithPass))) {
		// Don't request if we don't have permission to request.
		return NULL;
	}

	long nLocationID = GetCurrentLocationID();
	// (r.gonet 12/16/2013) - PLID 58199 - Changed public API arguments from integer to string.
	CString strLocationID = FormatString("%li", nLocationID);
	CString strCurrentUserID = FormatString("%li", GetCurrentUserID());
	// Get the registered provider Ids that this user is configured to be able to precribe under.
	NexTech_Accessor::_RegisteredUserPrescriberArrayPtr pPrescriberArray = GetAPI()->GetRegisteredUserPrescribers(GetAPISubkey(), GetAPILoginToken(), _bstr_t(strCurrentUserID), _bstr_t(strLocationID));
	Nx::SafeArray<IUnknown*> saryPrescribers(pPrescriberArray->Prescribers);
	if(saryPrescribers.GetSize() > 0) {
		// Request using the first prescriber returned.
		NexTech_Accessor::_RegisteredUserPrescriberPtr pPrescriber = saryPrescribers.GetAt(0);
		// (r.gonet 12/16/2013) - PLID 58199 - Changed public API properties from integer to string.
		long nProviderID = atol((LPCTSTR)pPrescriber->PrescriberID);
		
		COleDateTime dtFrom = GetDateTimeNull(); 
		COleDateTime dtTo = GetDateTimeNull();
		if(GetRemotePropertyInt("MedHistory_UseLastFillDateRange", FALSE, 0, GetCurrentUserName(), true)) {
			// (r.gonet 09/21/2013) - PLID 58397 - We should use the last fill date filters when requesting history. Find what they should be.
			long nDefaultFromNumMonthsInPast = GetRemotePropertyInt("MedHistory_DefaultFromNumMonthsInPast", 24, 0, GetCurrentUserName(), true);
			if(nDefaultFromNumMonthsInPast >= 0) {
				// Withous specifying a range, we should always go up to today.
				dtTo = COleDateTime::GetCurrentTime();
				// Break the month count down to number of years and number of months
				long nYears = nDefaultFromNumMonthsInPast / 12;
				long nMonths = nDefaultFromNumMonthsInPast % 12;
				// As the from date filter, subtract those years and months from the to date filter, and use the first of the month.
				dtFrom.SetDate(dtTo.GetYear() - nYears, dtTo.GetMonth() - nMonths, 1);
			}
		} else {
			// We should not use the last fill date range filter. Leave the date filters null.
		}
		// Go make the request. Pass NULL as the hwndForMessageBoxes so the function is silent on errors. 
		RequestEligibility(NULL, nPatientID, nProviderID, nLocationID, false);
		return MedicationHistoryUtils::RequestMedicationHistory(NULL, nPatientID, nProviderID, nLocationID, dtFrom, dtTo);
	} else {
		// They have no providers they can prescribe under, so they can't get the medication history either.
		return NULL;
	}
}

// (r.gonet 10/24/2013) - PLID 59104 - Checks if the patient has an appointment within three days.
bool MedicationHistoryUtils::HasAppointment(long nPatientID)
{
	// See if we can make an eligibility request. Per SureScripts requirements, the Practice must be seeing the patient within 3 days.
	// We have confirmed with SureScripts we can use 72 hours rather than 3 days.
	_RecordsetPtr prsAppointments = CreateParamRecordset(
		"SELECT AppointmentsT.ID "
		"FROM AppointmentsT "
		"WHERE AppointmentsT.PatientID = {INT} "
		"	AND AppointmentsT.Status <> 4 "
		"	AND AppointmentsT.StartTime >= DATEADD(hh, -72, GETDATE()) "
		"	AND AppointmentsT.StartTime <= DATEADD(hh, 72, GETDATE()) ",
		nPatientID);
	if(!prsAppointments->eof) {
		// Approval to request eligibility granted.
		return true;
	} else {
		// They have no appointment coming up or just past. We cannot request eligibilty then.
		return false;
	}
}

// (r.gonet 10/24/2013) - PLID 59104 - Requests eligibility for a patient
void MedicationHistoryUtils::RequestEligibility(HWND hwndForMessageBoxes, long nPatientID, long nProviderID, long nLocationID, bool bForceEligibilityRequest)
{
	try {
		if(bForceEligibilityRequest || HasAppointment(nPatientID)) {
			// Make the eligibility request.
			NexTech_Accessor::_SureScriptsEEligibilityRequestResponsePtr pEligibilityResponse = GetAPI()->RequestSureScriptsEEligibility(GetAPISubkey(), GetAPILoginToken(), 
				_bstr_t(FormatString("%li", nPatientID)), 
				_bstr_t(FormatString("%li", nProviderID)),
				_bstr_t(FormatString("%li", nLocationID)));
			if(!pEligibilityResponse) {
				// Failed!
				if(hwndForMessageBoxes != NULL && IsWindow(hwndForMessageBoxes)) {
					::MessageBox(hwndForMessageBoxes, "Error while requesting eligibility from SureScripts. Could not obtain an eligibility response. "
						"Practice will attempt to request medication history non-coverage related medication history.", "Error Requesting Eligibility", MB_ICONWARNING);
				} else {
					// Be silent.
				}
				// We got null for the eligibility request. This shouldn't happen. I'm tempted to return NULL, but we can still technically make the request.
				ASSERT(FALSE);
			}
		} else {
			// We cannot request eligibility. Meaning that we won't be getting claim data back.
		}
	}catch(_com_error e) {
		// Failed! Got an exception. Report it to the user.
		if(hwndForMessageBoxes != NULL && IsWindow(hwndForMessageBoxes)) {
			::MessageBox(hwndForMessageBoxes, FormatString("Error while requesting eligibility from SureScripts. %s\r\n"
				"Practice will attempt to request medication history non-coverage related medication history.", (LPCTSTR)e.Description()), "Error Requesting Eligibility", MB_ICONERROR);
		} else {
			// Be silent.
		}
	}
}

// (r.gonet 09/20/2013) - PLID 57978 - Starts a new request for patient medication history. The caller
//   further specifies parameters of the request.
// - hwndForMessageBoxes: If non-NULL, then if the request fails, the error message will be displayed
//   in a message box using this window handle. If NULL, the request will be silent.
// - nPatientID: The person ID of the patient for which to check for the medication history.
// - nProviderID: The person ID of the requesting provider. The provider must be registered for the
//   location with the ID of the nLocation argument and the current user must be configured to be able
//   to prescribe under this provider.
// - nLocationID: The ID of the location at which the request is being made. This needs not be the
//   current location.
// - dtFromDate: If the datetime status is valid, then the history returned will be filtered on the last fill date
//   by this value. Only medications with the last fill date equal or greater than this date time will be returned.
//   If the status is not valid, then SureScripts will use by default 1 year in the past for Fill Data and 2 years
//   for Claim Data. If valid, then dtToDate must also be valid.
// - dtToDate: If the datetime status is valid, then the history returned will be filtered on the last fill date
//   by this value. Only medications with the last fill date equal or less than this date time will be returned.
//   If the status is not valid, then SureScripts will use by default to the current date and time. If valid, 
//   then dtFromDate must also be valid.
// - bForceEligibilityRequest: In order to get the claim related medication history, an eligibility request must be made or retrieved
//   for the patient. The eligibility request cannot be made without an appointment within three days of the current date time.
//   If this argument is true, then the appointment requirement is ignored and the eligibility request is made anyway. If false,
//   then the appointemnt requirement is obeyed.
// Returns: The master request that was represents this medication history request. NULL if the request failed.
// Notes: Callers should listen for the table checker MedicationHistoryResponseT for the responses.
NexTech_Accessor::_RxHistoryMasterRequestPtr MedicationHistoryUtils::RequestMedicationHistory(HWND hwndForMessageBoxes, long nPatientID, long nProviderID, long nLocationID, COleDateTime dtFrom, COleDateTime dtTo)
{
	if(!(GetCurrentUserPermissions(bioPatientMedicationHistory) & (sptDynamic0|sptDynamic0WithPass))) {
		// We can't make requests. Permission denied.
		return NULL;
	}

	// (a.walling 2014-01-06 12:43) - PLID 59996 - No connection pointers for these existing patient functions
	long nUserDefinedID = GetExistingPatientUserDefinedID(nPatientID);	
	NexTech_Accessor::_NullableDateTimePtr pNullableFromDate(__uuidof(NexTech_Accessor::NullableDateTime));
	NexTech_Accessor::_NullableDateTimePtr pNullableToDate(__uuidof(NexTech_Accessor::NullableDateTime));
	if(dtFrom.GetStatus() == COleDateTime::valid && dtTo.GetStatus() == COleDateTime::valid) {
		// Both the from date and to date have to be valid in order to filter on the last fill date.
		pNullableFromDate->SetDateTime(dtFrom);
		pNullableToDate->SetDateTime(dtTo);
	}

	// (r.gonet 12/16/2013) - PLID 58199 - Changed public API arguments from integer to string.
	CString strProviderID = FormatString("%li", nProviderID);
	CString strLocationID = FormatString("%li", nLocationID);
	// (r.gonet 08/08/2013) - PLID 57978 - Send the request
	NexTech_Accessor::_RequestMedicationHistoryResultPtr pResult = GetAPI()->RequestMedicationHistory(GetAPISubkey(), GetAPILoginToken(), _bstr_t(FormatString("%li", nUserDefinedID)), _bstr_t(strProviderID), _bstr_t(strLocationID), pNullableFromDate, pNullableToDate);
	if(pResult == NULL) {
		ThrowNxException("%s : RequestMedicationHistory result was NULL.", __FUNCTION__);
	}
	if(!pResult->RequestFailed && pResult->RxHistoryMasterRequest != NULL) {
		// We got back a response and it was a success. Return the master request.
		return pResult->RxHistoryMasterRequest;
	} else if(pResult->RequestFailed) {
		// The request failed. Report it to the user if we can.
		CString strFailureMessage((LPCTSTR)pResult->RequestFailureMessage);
		if(strFailureMessage.IsEmpty()) {
			strFailureMessage = "Unknown error encountered while requesting medication history.";
		}
		if(hwndForMessageBoxes != NULL && IsWindow(hwndForMessageBoxes)) {
			::MessageBox(hwndForMessageBoxes, strFailureMessage, "Error", MB_ICONERROR|MB_OK);
		}
		return NULL;
	} else {
		// Such a state shouldn't occur.
		ASSERT(FALSE); 
		return NULL;
	}
}
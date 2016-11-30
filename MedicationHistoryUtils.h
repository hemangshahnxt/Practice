#include "NxAPI.h"

namespace MedicationHistoryUtils
{
	// (r.gonet 09/20/2013) - PLID 58396 - Starts a thread to request medication history for a patient.
	// - hwndNotify: If it is determined that the patient has medication history and no request needs
	//   be made, then the thread will send a NXM_BACKGROUND_MED_HISTORY_REQUEST_COMPLETE message to hwndNotify.
	// - nPatientID: The person ID of the patient for which to request medication history.
	// Note: Since medication history requests are asynchronous, callers should also listen for the 
	//   table checker on MedicationHistoryResponseT.
	void BeginBackgroundRequestMedHistory(HWND hwndNotify, long nPatientID);
	// (r.gonet 09/20/2013) - PLID 58396 - Requests medication history.
	// - hwndNotify: If it is determined that the patient has medication history and no request needs
	//   be made, then the thread will send a NXM_BACKGROUND_MED_HISTORY_REQUEST_COMPLETE message to hwndNotify.
	// - nPatientID: The patient to request medication history for.
	// Note: Since medication history requests are asynchronous, callers should also listen for the 
	//   table checker on MedicationHistoryResponseT.
	void BackgroundRequestMedHistory(HWND hwndNotify, long nPatientID);
	// (r.gonet 09/20/2013) - PLID 57978 - Gets the patient's most recent (master) request for medication history.
	// - nPatientID: The person ID of the patient for which to check for the medication history request.
	// Returns: The master request of the patient's most recent medication history request. NULL if no prior
	//   request was found.
	NexTech_Accessor::_RxHistoryMasterRequestPtr GetLastRxHistoryMasterRequest(long nPatientID);
	// (r.gonet 09/20/2013) - PLID 58396 - Gets whether or not the patient has medication history.
	// - nPatientID: The person ID of the patient for which to check for the medication history.
	// - nMasterRequestID: The medication history master request the responses must be associated with.
	//   Pass -1 for any medication history master request.
	// Returns: true if the patient has medication history. false otherwise.
	bool HasMedicationHistory(long nPatientID, long nMasterRequestID = -1);
	// (r.gonet 09/20/2013) - PLID 58396 - Starts a new request for patient medication history.
	// - nPatientID: The person ID of the patient for which to check for the medication history.
	// Returns: The master request that was represents this medication history request. NULL if the request failed.
	// Notes: Callers should listen for the table checker MedicationHistoryResponseT for the responses.
	NexTech_Accessor::_RxHistoryMasterRequestPtr RequestMedicationHistory(long nPatientID);
	// (r.gonet 10/24/2013) - PLID 59104 - Checks if the patient has an appointment within three days.
	// - nPatientID: The person ID of the patient for which to request eligibility.
	// Returns: true if they have an appointment. false otherwise.
	bool HasAppointment(long nPatient);
	// (r.gonet 10/24/2013) - PLID 59104 - Requests eligibility for a patient
	// - hwndForMessageBoxes: If non-NULL, then if the request fails, the error message will be displayed
	//   in a message box using this window handle. If NULL, the request will be silent.
	// - nPatientID: The person ID of the patient for which to request eligibility.
	// - nProviderID: The person ID of the requesting provider. The provider must be registered for the
	//   location with the ID of the nLocation argument and the current user must be configured to be able
	//   to prescribe under this provider.
	// - nLocationID: The ID of the location at which the request is being made. This needs not be the
	//   current location.
	// - bForceEligibilityRequest: In order to get the claim related medication history, an eligibility request must be made or retrieved
	//   for the patient. The eligibility request cannot be made without an appointment within three days of the current date time.
	//   If this argument is true, then the appointment requirement is ignored and the eligibility request is made anyway. If false,
	//   then the appointment requirement is obeyed.
	void RequestEligibility(HWND hwndForMessageBoxes, long nPatientID, long nProviderID, long nLocationID, bool bForceEligibilityRequest);
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
	// Returns: The master request that was represents this medication history request. NULL if the request failed.
	// Notes: Callers should listen for the table checker MedicationHistoryResponseT for the responses.
	NexTech_Accessor::_RxHistoryMasterRequestPtr RequestMedicationHistory(HWND hwndForMessageBoxes, long nPatientID, long nProviderID, long nLocationID, COleDateTime dtFromDate, COleDateTime dtToDate);
}
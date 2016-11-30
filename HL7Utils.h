#if !defined HL7_UTILS_H
#define HL7_UTILS_H

#pragma once

#include <NxHL7Lib\HL7CommonTypes.h>
#include "GlobalLabUtils.h"
#include "BillingModuleDlg.h"

struct HL7Message;
enum HL7CodeLink_RecordType;

// (z.manning 2009-01-08 15:12) - PLID 32663 - Added a function to handle the entire process of 
// updating a patient in HL7. Returns true if export is successful and false if it failed.
// (z.manning 2011-08-05 12:14) - PLID 40872 - Added param to send a new patient message instead
// (r.gonet 12/03/2012) - PLID 54105 - Retained for legacy purposes but this now uses SendUpdatePatientMessage() underneath.
//TES 6/7/2013 - PLID 55732 - Added strRequestGroupGUID to these three functions.  
// If it is non-empty, and if an error occurs for this message, only one error will be reported to the user per RequestGroupGUID.
// (b.eyers 2015-06-22) - PLID 66213 - extra field to send ROL in but can be used for other items later on
//TES 9/28/2015 - PLID 66192 - Added bSendPrimaryImage
// (r.goldschmidt 2015-11-09 10:57) - PLID 67517 - New HL7 setting, so updating needs to know if this is an attempted mass insurance update; added bMassInsuranceUpdate
// (j.jones 2016-04-06 13:48) - NX-100095 - added bOnlyUpdateIntellechart, will not update any HL7 link except Intellechart
BOOL UpdateExistingPatientInHL7(const long nPatientID, bool bOnlyUpdateInsuranceHL7Groups = false, bool bNewPatient = false, const CString &strRequestGroupGUID = "", HL7ROLActionCodes eROLCode = racUnchanged, bool bSendPrimaryImage = false, bool bMassInsuranceUpdate = false, bool bOnlyUpdateIntellechart = false);
//TES 11/12/2015 - PLID 67500 - New version for mass exports
// (r.goldschmidt 2016-02-04 16:10) - PLID 68161 -  add bool for mass insurance update
// (j.jones 2016-04-06 13:48) - NX-100095 - added bOnlyUpdateIntellechart, will not update any HL7 link except Intellechart
BOOL UpdateMultipleExistingPatientsInHL7(const CArray<long, long> &arPatientIDs, bool bOnlyUpdateInsuranceHL7Groups = false, bool bNewPatient = false, const CString &strRequestGroupGUID = "", HL7ROLActionCodes eROLCode = racUnchanged, bool bSendPrimaryImage = false, bool bMassInsuranceUpdate = false, bool bOnlyUpdateIntellechart = false);
// (z.manning 2009-01-08 17:03) - PLID 32663 - Update multiple patients at once for a given insurance
// company ID
// (d.thompson 2012-08-28) - PLID 52129 - Merged with above UpdateExistingHL7PatientsByRecordset()
void UpdateExistingHL7PatientsByInsCoID(const long nInsCoID);

// (v.maida 2014-12-23 12:19) - PLID 64472 - Add or update referring physicians in HL7.
BOOL AddOrUpdateRefPhysInHL7(const long nRefPhysID, bool bNewRefPhys = false, const CString &strRequestGroupGUID = "");

// (s.tullis 2015-06-23 14:17) - PLID 66197 -  Send Patient Reminder message on add or delete
BOOL SendPatientReminderHL7Message(const long nPatientID, const long nReminderID);

// (j.jones 2016-04-07 08:35) - NX-100095 - Added a function that sends HL7 messages, for IntelleChart links only,
// which include the patient's primary image. Only called when this image has changed.
BOOL SendPatientPrimaryPhotoHL7Message(const long nPatientID);

// (r.gonet 12/11/2012) - PLID 54117 - Get a description of message types.
CString GetHL7ExportMessageTypeDescription(HL7ExportMessageType hemt);

//TES 6/7/2013 - PLID 55732 - Added strRequestGroupGUID to these three functions.  
// If it is non-empty, and if an error occurs for this message, only one error will be reported to the user per RequestGroupGUID.

// (r.gonet 12/03/2012) - PLID 54106 - Sends a new appointment HL7 message to all HL7 links that support
//  automatic exporting to HL7 for appointments.
void SendNewAppointmentHL7Message(long nAppointmentID, bool bSendTableChecker = true, const CString &strRequestGroupGUID = "");
// (r.gonet 12/03/2012) - PLID 54107 - Sends an update appointment HL7 message to all HL7 links that support
//  automatic exporting to HL7 for appointments.
void SendUpdateAppointmentHL7Message(long nAppointmentID, bool bSendTableChecker = true, const CString &strRequestGroupGUID = "");
// (r.gonet 12/03/2012) - PLID 54108 - Sends a cancel appointment HL7 message to all HL7 links that support
//  automatic exporting to HL7 for appointments.
void SendCancelAppointmentHL7Message(long nAppointmentID, bool bSendTableChecker = true, const CString &strRequestGroupGUID = "");
/// <summary>
/// Sends a cancel appointment HL7 message to all HL7 links that support automatic exporting of appointments
/// </summary>
/// <param name="pClient">The HL7 client to use</param>
/// <param name="nApptID">The ID of the appointment being cancelled or deleted</param>
/// <param name="bSendTableChecker">True if we should send a table checker; otherwise false</param>
/// <param name="strRequestGroupGUID">The HL7 request group GUID, or an empty string if not applicable</param>
/// <returns>A collection of responses from each HL7 link which must be checked for errors</returns>
std::vector<HL7ResponsePtr> SendCancelAppointmentHL7Message(CHL7Client* pClient, long nApptID, bool bSendTableChecker, const CString &strRequestGroupGUID);

// (d.thompson 2012-08-28) - PLID 52129
void UpdateExistingHL7PatientsByInsCoIDAndPlan(const long nInsCoID, const long nInsPlanID);
// (d.thompson 2012-08-28) - PLID 52129
// (r.goldschmidt 2015-11-09 10:57) - PLID 67517 - New HL7 setting, so updating needs to know if this is an attempted mass insurance update
void UpdateExistingHL7PatientsByRecordset(ADODB::_RecordsetPtr prs, bool bMassInsuranceUpdate = false);

// (b.eyers 2015-11-12) - PLID 66977
void UpdateExistingHL7PatientsByInsContact(const long nContactID);

// (j.jones 2008-04-18 11:38) - PLID 21675 - given a message, will return the message date
// (z.manning 2011-06-15 15:51) - PLID 40903 - This no longer takes in a message, but rather a string representing MSH-7
COleDateTime GetHL7DateFromStringField(const CString &strDateField, long nHL7GroupID);

//Takes the message, sends an acknowledgement, if the message is an ACK, updates HL7MessageLogT, 
//otherwise, adds message to HL7MessageQueueT
//TES 4/21/2008 - PLID 29721 - Added parameters to help with auditing.
// (j.jones 2008-05-05 10:09) - PLID 29600 - added bBatchImports parameter
// (j.jones 2008-05-19 15:35) - PLID 30110 - added a parameter to disable sending tablecheckers for HL7 events
// (z.manning 2010-06-28 15:15) - PLID 38896 - Added message parameter
// (a.wilson 2013-05-09 11:13) - PLID 41682 - new return value of enum
HL7AddMessageToQueueResult AddMessageToQueue(const CString &strMessage, long nHL7GroupID, const CString &strHL7GroupName, BOOL bSendHL7Tablechecker, OUT HL7Message &message);

//TES 4/18/2008 - PLID 29721 - Moved GetActionDescriptionForEvent() and GetSpecificDescriptionForEvent() to HL7ParseUtils.

//Takes an incoming HL7 Message and performs the appropriate action (adds new patient, updates patient, etc.)
//Returns success or failure.
//TES 4/16/2008 - PLID 29595 - Note that any changes to this function should be kept in sync with the corresponding
// function in NxServer, HL7Support::TryToCommitEvent()
//TES 4/18/2008 - PLID 29657 - This function now also updates HL7MessageQueueT if the event was committed successfully.
//TES 4/21/2008 - PLID 29721 - Added parameters to help with auditing.
// (j.jones 2008-05-19 15:35) - PLID 30110 - added a parameter to disable sending tablecheckers for HL7 events
//TES 10/31/2013 - PLID 59251 - Added arNewCDSInterventions, any interventions triggered in this function will be added to it
BOOL CommitHL7Event(const HL7Message &Message, CWnd *pMessageParent, BOOL bSendHL7Tablechecker, IN OUT CDWordArray &arNewCDSInterventions);

//Functions called from CommitHL7Event
//TES 5/8/2008 - PLID 29685 - All these functions now call FinalizeHL7Event once they're done, which updates HL7MessageQueueT.
// Therefore, if you call any of them, and they return TRUE, that means that HL7MessageQueueT has already been updated with
// the appropriate action, the caller doesn't need to do anything more.  Also, they use the new HL7Message structure.
//ADT^A04
//TES 11/1/2007 - PLID 26892 - Added an optional output parameter to return the PersonID of the created record.
//TES 4/16/2008 - PLID 29595 - Note that any changes to this function should be kept in sync with the corresponding
// function in NxServer, HL7Support::TryAddPatientToData()
// (j.jones 2008-05-19 15:35) - PLID 30110 - added a parameter to disable sending tablecheckers for HL7 events
BOOL AddHL7PatientToData(const HL7Message &Message, BOOL bSendHL7Tablechecker, OPTIONAL OUT long *pnNewPersonID = NULL);
//ADT^A08
//TES 4/16/2008 - PLID 29595 - Note that any changes to this function should be kept in sync with the corresponding
// function in NxServer, HL7Support::TryUpdatePatient()
// (j.jones 2008-05-19 15:35) - PLID 30110 - added a parameter to disable sending tablecheckers for HL7 events
BOOL UpdateHL7Patient(const HL7Message &Message, CWnd *pMessageParent, BOOL bSendHL7Tablechecker);
//DFT^P03
// (j.jones 2008-05-19 15:35) - PLID 30110 - added a parameter to disable sending tablecheckers for HL7 events
// (j.gruber 2016-01-29 12:27) - PLID 68006 - added insured party
// (j.gruber 2016-03-22 10:12) - PLID 68006 - added BillFromType to distinguish HL7 from visitsToBeBilled
BOOL CreateHL7Bill(const HL7Message &Message, CWnd *pMessageParent, const CString &strVersion, BOOL bSendHL7Tablechecker, long nInsuredPartyID = -1, BillFromType eFromType = BillFromType::HL7Batch);
/// <summary>
/// Gets the earliest charge date from the provided HL7 bill message.
/// </summary>
/// <param name="strMessage">The HL7 bill message to get the earliest charge date from.</param>
/// <param name="nHL7GroupID">The HL7 Group ID for the group that the bill belongs to.</param>
/// <returns>The earliest charge within the provided HL7 bill message. The status of the returned datetime will be invalid if no charges could be found.</returns>
COleDateTime GetEarliestChargeDateFromHL7Bill(const CString &strMessage, const long &nHL7GroupID);

//TES 5/23/2011 - PLID 41353 - Added support for referring physician messages (MFN^M02)
BOOL HandleHL7RefPhys(const HL7Message &Message, BOOL bSendHL7Tablechecker);

//TES 9/18/2008 - PLID 21093 - Processes an ORU^R01 (Unsolicited Lab Result) message, creates a new Lab record if needed,
// and appends the results to the lab.
//TES 12/2/2008 - PLID 32297 - NOTE: This function must be kept in sync with the TryImportLabResult() function 
// in NxServer's HL7Support.cpp.
//TES 10/31/2013 - PLID 59251 - Added arNewCDSInterventions, any interventions triggered in this function will be added to it
BOOL ImportHL7LabResult(const HL7Message &Message, CWnd *pMessageParent, BOOL bSendHL7Tablechecker, IN OUT CDWordArray &arNewCDSInterventions);

// (z.manning 2010-07-01 13:44) - PLID 39422
BOOL HandleHL7SchedulerMessage(const HL7Message &message, CWnd *pwndParent);

//TES 5/8/2008 - PLID 29685 - This function takes in parameters representing batched SQL statements (possibly parameterized),
// and audits.  This will add to that batch in order to update HL7MessageQueueT, then commit the batch.
// (j.jones 2008-05-19 15:35) - PLID 30110 - added a parameter to disable sending tablecheckers for HL7 events
// (a.walling 2010-01-21 15:57) - PLID 37023
//TES 7/12/2010 - PLID 39518 - nPatientID is now passed by reference; if you pass in -1, then strSqlBatch must create a new patient,
// and put its ID in a variable named @nPatientID.  After this function is run, nPatientID will then have the value pulled from @nPatientID
//TES 7/17/2010 - PLID 39518 - nAuditTransactionID is now a pointer.  If you want the function to do all auditing itself, leave it null.
// If you have an AuditID already, pass it in, otherwise pass in a pointer to a variable set to -1, and it will begin the auditing and leave
// it up to the caller to commit it.
//TES 5/23/2011 - PLID 41353 - nPatientID can now be set to -2, meaning that this message doesn't have any patient information
//TES 6/10/2011 - PLID 41353 - Changed nPatientID to nPatientIDToAudit, and added an input parameter strNewIDVariable, representing the variable
// in the query that has been set to the new ID being created by this message, which will then be output in pnNewID.  
// If strNewIDVariable is non-empty, pnNewID must be non-NULL
BOOL FinalizeHL7Event(long &nPatientIDToAudit, const HL7Message &Message, CString &strSqlBatch, BOOL bSendHL7Tablechecker, CNxParamSqlArray *paryParams = NULL, long *pnAuditTransactionID = NULL, OPTIONAL IN const CString &strNewIDVariable = "", OPTIONAL OUT long *pnNewID = NULL);
//
//This returns either "NULL" or a string with the id of a referring physician that matches the given criteria.
//TES 5/7/2008 - PLID 29685 - This now takes in a parameterized SQL batch.  If a new referring physician needs to be created,
// this will now pass that in as 
//TES 7/15/2010 - PLID 39518 - The passed-in batch is now required to declare a variable @nNextPersonID, which must be, as the name
// implies, set to the equivalent of NewNumber("PersonT","ID").  If this function creates a new referring physician, it will
// increment @nNextPersonID.
//TES 5/23/2011 - PLID 41353 - Changed this to take in the new HL7ReferringPhysician struct, as well as nHL7GroupID.
//TES 5/23/2011 - PLID 41353 - Also added auditing, and replaced nHL7GroupID with Message
struct HL7ReferringPhysician;
//TES 6/9/2011 - PLID 41353 - Added bForPatient, used just to give more informative messages to the user.
CString GetNextechRefPhys(const HL7ReferringPhysician &hrp, bool bForPatient, const HL7Message &Message, IN OUT long &nAuditTransactionID, IN OUT CString &strSqlBatch, OPTIONAL IN OUT CNxParamSqlArray *paryParams = NULL);

//TES 9/18/2008 - PLID 31414 - Takes the given information pulled from an HL7 message, and matches it to a ProvidersT record
// in Nextech.  Interacts with the user.  Pass in strRecord and strField with user-meaningful names describing what field
// in the HL7 message this provider is from.
//TES 10/21/2008 - PLID 21432 - Made the strSqlBatch parameter optional (if it's not filled, then it will go ahead and
// just execute any needed queries), and also added an optional output parameter for whether the function in fact created
// a new provider record.
// (j.jones 2010-05-13 10:34) - PLID 36527 - now this needs to report whether the HL7IDLinkT needs audited
CString GetNextechProvider(const CString &strProvThirdPartyID, const CString &strProvFirst, const CString &strProvMiddle, const CString &strProvLast, long nHL7GroupID, OPTIONAL IN OUT CString *pstrSqlBatch = NULL, OPTIONAL IN OUT CNxParamSqlArray *paryParams = NULL, const CString &strRecord = "Lab Result", const CString &strField = "Ordering Provider", OPTIONAL OUT bool *pbRecordCreated = NULL, OPTIONAL OUT bool *pbHL7IDLinkT_NeedAudit = NULL);

// (z.manning 2010-07-01 10:16) - PLID 39422
long GetNextechResource(HL7Message message, const CString &strResourceThirdPartyID, const CString &strResourceName);

//TES 10/8/2008 - PLID 21093 - Needed to get locations for Labs, so I split GetHL7Location() out into its own function.  Once
// I'd done that, I realized that the lab needed its own function, because it pulls different records and has a different
// prompt, so I added a "copy" called GetHL7LabLocation(), but I'm keeping them as separate functions, both to make
// the functions that call them more readable, and also because changes to the logic of one will likely require similar
// changes in the logic of the other, and this way they're right next to each other.
BOOL GetHL7Location(const HL7Message &Message, const CString &strThirdPartyID, OUT long &nLocationID);
BOOL GetHL7LabLocationID(const HL7Message &Message, const CString &strThirdPartyID, OUT long &nLocationID);

// (j.dinatale 2011-10-19 15:30) - PLID 44823 - had to break this out into its own function, we want to handle it slightly differently
BOOL GetHL7ApptLocation(const HL7Message &Message, const CString &strThirdPartyID, OUT long &nLocationID);

// (d.singleton 2012-08-08 11:16) - PLID 51938 need to assign insured parties to appts when importing an appt
// (r.gonet 05/14/2013) - PLID 56675 - Added an output variable to tell us if the appointment insurance has changed.
BOOL HandleInsuredParty(const HL7Message &Message, CString strPersonName, long nPersonID, long nApptID, long nAuditID, OUT bool &bInsuranceChanged, CString &strSql,  CString &strError, BOOL bIsAppt, IN OUT CNxParamSqlArray &aryParams);

//TES 2/23/2010 - PLID 37501 - Attempts to match the existing information to an existing lab.  Will either return a valid LocationsT.ID,
// or -1, in which case the user will have been notified that the message will not be imported.
//TES 4/29/2011 - PLID 43424 - Renamed to GetPracticeLabLocation, this might be coming from OBX-23 or OBR-21
//TES 2/25/2013 - PLID 54876 - Added bSilent and strLabType.  bSilent controls whether it will prompt the user if no existing mapping is found,
// and strLabType will be used in any messages ("Receiving Lab" and "Performing Lab" are the two current options).  If bSilent is FALSE, then it is possible
// to return -1 without the user having been notified.
// (d.singleton 2013-11-05 17:10) - PLID 59337 - need the parish code and country added to preforming lab for hl7 labs
long GetPracticeLabLocation(long nHL7GroupID, const CString &strThirdPartyID, const CString &strName, const CString &strAddress1, const CString &strAddress2, const CString &strCity, const CString &strState, const CString &strZip,
							BOOL bSilent, const CString &strLabType, const CString &strCountry = "", const CString &strParish = "");

//TES 4/17/2008 - PLID 29595 - Moved some functions from here to HL7ParseUtils.


//TES 8/8/2007 - PLID 26892 - Revamped these functions to use some enums, rewrote the comment below to accurately
// describe the functions' behavior.
//TES 7/12/2007 - PLID 26642 - Call one of these functions (use the one with the HL7_PIDFields if you've already parsed
// them out, otherwise call the strMessage override) in order to find an internal PersonT.ID that matches the patient 
// identified in the HL7 Message.  This will return either a valid ID or -1.  Use the ENotFoundBehavior to specify what
// to do if the patient isn't already linked: you can either prompt the user to link the patient to an existing 
// Nextech patient, prompt the user to either link the patient to an existing one or create a new record for it, or you
// can tell it to just skip the record.
//If the function returns -1, and the *pNfr parameter was passed in, then it will be filled with either nfrFailure, meaning
// that the message couldn't be parsed or the user cancelled a dialog, nfrCreateNew, meaning that the user asked to
// create a new record in Nextech for this patient (NOTE: GetPersonFromHL7Message() will NOT actually create the new 
// patient, that's up to the calling function), or nfrSkipped, meaning that nfbSkip was passed in, and the message
// was properly parsed but didn't already exist in Nextech.

enum ENotFoundBehavior	{
	nfbPromptToLink = 0,
	nfbPromptToLinkAndCreate = 1,
	nfbSkip = 2,
};
enum ENotFoundResult {
	nfrFailure = 0,
	nfrCreateNew = 1,
	nfrSkipped = 2,
};

struct HL7_PIDFields;
struct HL7_InsuranceFields;

//TES 9/18/2008 - PLID 31414 - Renamed to make it clear that this is only for extracting patient information, not other
// types of PersonT records.
//TES 10/5/2009 - PLID 35695 - Added an optional output parameter for the patient name (to the first overload only, 
// the second one already has it being passed in via the HL7_PIDFields struct).
// (z.manning 2010-05-21 14:59) - PLID 38831 - Added parameter for lab import matching field flags
long GetPatientFromHL7Message(const CString &strMessage, long nHL7GroupID, CWnd *pMessageParent, ENotFoundBehavior nfb = nfbPromptToLink, OPTIONAL OUT ENotFoundResult *pNfr = NULL, bool bWillOverwrite = false, OPTIONAL OUT CString *pstrPatientName = NULL, OPTIONAL IN DWORD dwLabImportMatchingFieldFlags = 0);
long GetPatientFromHL7Message(const HL7_PIDFields &PID, long nHL7GroupID, CWnd *pMessageParent, ENotFoundBehavior nfb, OPTIONAL OUT ENotFoundResult *pNfr, bool bWillOverwrite = false, OPTIONAL IN DWORD dwLabImportMatchingField = 0);

//TES 7/12/2010 - PLID 39518 - Goes through all the entries in arInsuranceSegments, and ensures that strNextechInsCoID, 
// strNextechInsContactID, and strNextechInsPlanID are all filled with valid values.
//TES 7/14/2010 - PLID 39635 - This can now return FALSE, meaning that the user was prompted, and they cancelled, so abort the import.
//TES 7/19/2010 - PLID 39518 - Keep in sync with HL7Support::AssignInsCoIDs() in NxServer
BOOL AssignInsCoIDs(const HL7_PIDFields &PID, long nPatientID, CArray<HL7_InsuranceFields,HL7_InsuranceFields&> &arInsuranceSegments, long nHL7GroupID, IN OUT long &nAuditTransactionID, IN OUT CString &strSqlBatch, OPTIONAL IN OUT CNxParamSqlArray *paryParams = NULL);

//TES 7/15/2010 - PLID 39518 - Prompts the user to select a Relation To Patient to map to the given code.  If they choose one, it will
// be mapped in data (via adding statements to the passed-in batch), output in strRelation, and it will return TRUE.
// Otherwise, it will return FALSE, meaning processing should be aborted.
BOOL PromptForHL7Relation(const CString &strHL7RelationCode, long nHL7GroupID, const CString &strHL7GroupName, OUT CString &strRelation, IN OUT long &nAuditTransactionID, IN OUT CString &strSqlBatch, OPTIONAL IN OUT CNxParamSqlArray *aryParams = NULL);

//(e.lally 2007-08-01) PLID 26326 - This utility is to be used to tell if we support the commit all feature for
//the event type sent in
BOOL IsCommitAllEventSupported(const CString& strEventType);

// (j.jones 2008-04-21 14:53) - PLID 29597 - determine if we need to
// notify the user that there are pending messages, and do so
void TryNotifyUserPendingHL7Messages();

void HL7Init();
void HL7Destroy();

//TES 7/16/2009 - PLID 25154 - This was used in various places to iterate through the HL7 Settings, I moved it here.
struct HL7SettingsGroup
{
	long nID;
	long nExportType;
	BOOL bExpectAck;
	CString strName;
};

// (z.manning 2010-05-21 13:16) - PLID 38638 - Function to get the audit text when auditing changes
// in the required fields for auto importing lab results
CString GetLabImportMatchingFieldAuditText(const DWORD dwLabImportMatchingFieldFlags);

// (z.manning 2011-06-15 14:45) - PLID 40903
void GetHL7MessageQueueMap(IN const CArray<long,long> &arynMessageIDs, OUT CMap<long,long,CString,LPCTSTR> &mapMessageIDToMessage);
void GetHL7MessageLogMap(IN const CArray<long,long> &arynMessageIDs, OUT CMap<long,long,CString,LPCTSTR> &mapMessageIDToMessage);
	
//TES 6/22/2011 - PLID 44261 - Functions for interacting with the global CHL7SettingsCache
void EnsureHL7SettingsCache();
void DestroyHL7SettingsCache();
//TES 8/16/2011 - PLID 44262 - Added an accessor for the cache
class CHL7SettingsCache;
CHL7SettingsCache* GetHL7SettingsCache();

//TES 6/22/2011 - PLID 44261 - New functions for accessing HL7 Settings
CString GetHL7SettingText(long nHL7GroupID, const CString &strSetting);
long GetHL7SettingInt(long nHL7GroupID, const CString &strSetting);
BOOL GetHL7SettingBit(long nHL7GroupID, const CString &strSetting);
//TES 6/23/2011 - PLID 44261 - Functions to set HL7 settings.  You can pass in SQL param batch information, in which case the queries
// will just be appended, otherwise the database will be updated.
void SetHL7SettingText(long nHL7GroupID, const CString &strSetting, const CString &strValue, OPTIONAL IN OUT CString *pstrSql = NULL, OPTIONAL IN OUT CNxParamSqlArray *paryParams = NULL);
void SetHL7SettingInt(long nHL7GroupID, const CString &strSetting, long nValue, OPTIONAL IN OUT CString *pstrSql = NULL, OPTIONAL IN OUT CNxParamSqlArray *paryParams = NULL);
void SetHL7SettingBit(long nHL7GroupID, const CString &strSetting, BOOL bValue, OPTIONAL IN OUT CString *pstrSql = NULL, OPTIONAL IN OUT CNxParamSqlArray *paryParams = NULL);

//TES 6/22/2011 - PLID 44261 - Some functions for non-standard HL7 Settings
CString GetHL7GroupName(long nHL7GroupID);
LabType GetHL7LabProcedureType(long nHL7GroupID);
void GetOBR24Values(long nHL7GroupID, CStringArray &saOBR24Values);

//TES 6/22/2011 - PLID 44261 - Flag an HL7Settings group as needing to be reloaded from data
void RefreshHL7Group(long nHL7GroupID);

//TES 6/22/2011 - PLID 44261 - Loads all settings groups that have the specified value for the specified setting
void GetHL7SettingsGroupsBySetting(const CString &strSetting, const CString &strValue, OUT CArray<long,long> &arGroups);
void GetHL7SettingsGroupsBySetting(const CString &strSetting, long nValue, OUT CArray<long,long> &arGroups);
void GetHL7SettingsGroupsBySetting(const CString &strSetting, BOOL bValue, OUT CArray<long,long> &arGroups);

//TES 6/22/2011 - PLID 44261 - Get all settings groups
void GetAllHL7SettingsGroups(OUT CArray<long,long> &arGroups);

// (z.manning 2011-07-08 16:22) - PLID 38753
void ViewHL7ImportMessage(const long nMessageID);
void ViewHL7ExportMessage(const long nMessageID);
void DisplayHL7Message(CString strMessage);
// (r.gonet 02/26/2013) - PLID 48419 - Dismisses a message in HL7MessageQueueT
bool DismissImportedHL7Message(long nMessageID);
// (r.gonet 02/26/2013) - PLID 47534 - Dismisses a message in HL7MessageLogT
bool DismissExportedHL7Message(long nMessageID);

//r.wilson (8/23/2012) PLID 52222
CString	DeclareDefaultDeductibleVars();

// (d.singleton 2012-10-08 17:47) - PLID 53097 import MDM messages
BOOL HandleHL7Notification(const HL7Message &Message, CWnd *pwndParent);

// (d.singleton 2013-01-25 15:30) - PLID 54781 function for importing images through ORU messages
//TES 10/31/2013 - PLID 59251 - Added arNewCDSInterventions, any interventions triggered in this function will be added to it
BOOL HandleORUMessage(const HL7Message &Message, CWnd *pMessageParent, BOOL bSendHL7Tablechecker, IN OUT CDWordArray &arNewCDSInterventions);
BOOL ImportHL7Image(const HL7Message &Message, CWnd *pMessageParent, BOOL bSendHL7Tablechecker);
void DecodeEncapsulatedData(const CString &strFileData, const CString &strEncodingType, const CString &strFullFileName, CFile &ImageFile);

//TES 10/16/2015 - PLID 66204 - Custom type for MDI optical prescriptions
BOOL ImportOpticalPrescription(const HL7Message &Message, CWnd *pMessageParent, BOOL bSendHL7Tablechecker);

// (a.wilson 2013-06-11 15:41) - PLID 57117 - a functino to save emn bills to data as success.
void MarkHL7EMNBillsAsSent(const CArray<long>& aryEMNIDs);

//TES 1/23/2015 - PLID 55674 - Copied from NxServer's HL7Support, used to prevent logging queries from getting rolled back with failed transactions.
CNxAdoConnection GetNewAdoConnection(const CString strDatabase);

// (j.jones 2015-11-16 11:01) - PLID 67491 - Modular function to create a SQL fragment that adds a new entry in HL7CodeLinkT,
// intended for use when we think no link exists, but will handle cases where a NULL practice ID exists.
// It's the caller's responsibility to audit the creation. It will still be a creation audit even if we replace a NULL entry.
CSqlFragment CreateNewHL7CodeLinkT(long nHL7GroupID, HL7CodeLink_RecordType eType, CString strThirdPartyCode, long nPracticeID);
// (j.jones 2015-11-16 11:01) - PLID 67491 - version that takes in a variable name that is an INT, like @PracticeID,
// strictly for use in batches
void CreateNewHL7CodeLinkT_WithVariableName(long nHL7GroupID, HL7CodeLink_RecordType eType, CString strThirdPartyCode, CString strPracticeIDVariableName,
	IN OUT CString &strSqlBatch, OPTIONAL IN OUT CNxParamSqlArray *paryParams = NULL);

// (r.goldschmidt 2016-02-04 16:10) - PLID 68161 - split out logic for selecting the HL7SettingsGroups due for updating
// (j.jones 2016-04-06 13:48) - NX-100095 - added bOnlyUpdateIntellechart, will not return links to anybody but Intellechart
void SelectHL7SettingsGroups(OUT CArray<HL7SettingsGroup, HL7SettingsGroup&> &arDefaultGroups, bool bOnlyUpdateInsuranceHL7Groups, bool bNewPatient, bool bMassInsuranceUpdate, bool bOnlyUpdateIntellechart);

#endif
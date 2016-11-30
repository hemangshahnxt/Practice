#ifndef INCLUDE_PHASE_TRACKING
#define INCLUDE_PHASE_TRACKING

#pragma once



#include <SharedTrackingUtils.h>

//NOTE:  This file is shared with NexFormsExporter!!

class CNxParamSql;
typedef CArray<CNxParamSql, CNxParamSql> CNxParamSqlArray;
/*
Event Type				Corresponding Actions														nItemID					nItemID2			Called when																			Link Action																										Unapplied when
ET_BillCreated			PA_Bill, PA_BillSurgery, PA_BillCPT, PA_BillInventory, PA_BillQuote			BillsT.ID				n/a					A bill is created.																	Go to Billing tab, open up corresponding bill.																	Bill is deleted.
ET_MarkedDone			<all actions>																NULL					PhaseDetailsT.ID	A step is marked done.																Go to Tracking tab																								Step is marked not done.
ET_QuoteCreated			PA_Quote, PA_QuoteSurgery, PA_QuoteCPT, PA_QuoteInventory					BillsT.ID				n/a					A quote is created																	Go to Quotes tab, open up corresponding quote.																	Quote is deleted.
ET_AppointmentCreated	PA_ScheduleAptCategory, PA_ScheduleAptType, PA_ScheduleAptPurpose			AppointmentsT.ID		n/a					An appointment is created, or an appointment is marked as not cancelled anymore.	Go to Scheduler module, find appointment and (when I implement this) open the resentrydlg for this appointment.	Appointment is cancelled or deleted.
ET_TemplateSent			PA_WriteTemplate											MailSent.MailID			n/a					A template is merged (assuming that template has ever been tracked.					Go to History tab, open up document.																			Document is detached.
ET_ActualAppointment	PA_ActualAptCategory, PA_ActualAptType, PA_ActualAptPurpoxe					AppointmentsT.ID		n/a					An appointment is created or modified so as to be either "In" or "Out".				Go to Scheduler module, find appointment and (when I implement this) open the resentrydlg for this appointment.	Appointment is cancelled or deleted. (Not unapplied when it is marked pending or no-show, because a.) that would be hard, and b.), you can still link to it.)
ET_PacketSent			PA_SendPacket																MergedPacketsT.ID		n/a					A packet is merged																	Go to History tab (may open packet in future).																	Packet is detached.
*/
namespace PhaseTracking
{


//referenced in the data - very important this is the same for all clients, and it never changes
typedef enum AptCategory
{
	AC_NON_PROCEDURAL	= 0,
	AC_CONSULT			= 1,
	AC_PREOP			= 2,
	AC_MINOR			= 3,
	AC_SURGERY			= 4,
	AC_FOLLOW_UP		= 5,
	AC_OTHER			= 6,
	AC_BLOCK_TIME		= 7
} AptCategory;


// (z.manning 2011-06-24 12:28) - PLID 42916 - Moved PhaseAction enum to SharedTrackingUtils.h


//*************************************************************************************************************
//WARNING!!!!!  These enumerations are stored to the database.  So, from the point when this is released to a 
//client onward, they _cannot_ be changed!!  If you change the value of any existing enum, we're all screwed!
//*************************************************************************************************************
typedef enum EventType
{
	ET_Invalid = -1,	// (j.jones 2009-11-02 10:08) - PLID 36082 - added ET_Invalid

	ET_BillCreated = 1,
	ET_MarkedDone = 2,
	ET_QuoteCreated = 3,
	ET_AppointmentCreated = 4,
	ET_TemplateSent = 5,
	ET_ActualAppointment = 6,
	ET_PacketSent = 7,
	ET_MarkedDoneFromTodo = 8,
	ET_Skipped = 9,
	ET_LadderCreated = 10,
	ET_PaymentApplied = 11,//Payment applied to charge OR PIC!
	ET_EMRCreated = 12,
} EventType;

// (z.manning 2008-07-14 13:00) - PLID 14214 - Values for special step assignments
// NEVER CHANGE EXISTING VALUES as these are stored in data
enum TrackingStepSpecialAssignees
{
	tssaPatientCoordinator = -10,
	tssaCreatingUser = -11,
};

//TES 7/16/2010 - PLID 39400 - The different "scopes" for merging.
// THIS ENUM IS STORED TO DATA, DO NOT CHANGE!
enum MergeTemplateScope
{
	mtsPic = 0,
	mtsMasterProcedure = 1,
	mtsProcedure = 2,
	mtsPrescription = 3,
	mtsDetailProcedure = 4,
	mtsPatient = 5,
};

//Get the description, like "Quote a Procedure"
CString GetPhaseActionDescription(PhaseTracking::PhaseAction nAction);

//called to add a new patient procedure
// (j.jones 2009-11-02 10:06) - PLID 36082 - if an event auto-launched a ladder, it will pass in the event type and ID that did so
void AddPtntProcedures(CArray<int, int> &ary, long personID, bool bIsActive = true, long nOverrideLadderTemplateID = -1,
					   PhaseTracking::EventType nLaunchedByType = ET_Invalid, long nLaunchedByItemID = -1);

// (z.manning 2008-10-28 09:39) - PLID 31371 - Returns true if we added a tracking ladder and false if not
BOOL PromptToAddPtnProcedures(const long nPatientID);

//Creates a ProcInfoT record, fills in whatever appropriate things it should.
//Returns the id of the created procinfo
long CreateProcInfo(long nPatientID, CArray<int, int> &arProcIDs, bool bCreatePic = true);

BOOL CheckWarnPersonLicenses(long nPersonID, CString strPersonType);

// (c.haag 2009-02-11 12:33) - PLID 33008 - Auto-complete as many ladder steps as possible
// (c.haag 2013-11-26) - PLID 59822 - We no longer pass in the patient ID
void TryToCompleteAllLadderSteps(long nLadderID);

//Adds a ladder to the given ProcInfoT record, and starts tracking and whatnot.  If nLadderTemplateID is -1, it will 
//figure out which ladder to use based on the procedures on the ProcInfo.
//Returns the id of the created ladder, or -1 if none was created, which could happen if the procinfo already has a ladder
//or if the user cancels when asked to select a template to use.
// (j.jones 2009-11-02 10:50) - PLID 36082 - if given an event that launched the ladder,
// we will need to set it as the first step's completed event
long AddLadderToProcInfo(long nProcInfoID, long nPatientID, long nLadderTemplateID = -1,
						 PhaseTracking::EventType nLaunchedByType = ET_Invalid, long nLaunchedByItemID = -1);

void MoveStep (long interest, long newStep, long oldStep, bool bIsTracked);
//moves a step in the order

// (j.jones 2008-11-26 13:43) - PLID 30830 - this is dead code that is not being kept up to date, so I commented it out
//void DeleteStep (long interest, long step);
//deletes a step from a pipeline

//OK, here's how this is going to work.  If you want to track something you must:
//1: Add it to EventType
//1(a): Add a description to GetEventDescription.
//2: Add code to GetActionsForEvent to specify which actions are satisfied by this event.
//2(a): Make sure that any actions you track are appropriately handled in the Admin Tracking.
//3: Add code to MatchEventWithStep to, well, match the event with a step.
//NOTE:  If you need more than one int to identify the item you are tracking, don't change this function.
//Honestly, I did it once, and it was a bad idea.  Take my word for it, you're going to end up changing
//the data in the end, so why not do it now?
//3(a): Also add to MatchEventWithStepTemplate.
//4: Add a call to CreateAndApplyEvent in the appropriate location.
//5: If you want to be able to link to this event from the tracking tab (i.e., go to a specific bill for ET_BillCreated), then
//5(a): Add a function call to the switch statement in CMainFrame::LinkToEvent()
//5(b): Add the function you called to the appropriate dialog (i.e., OpenBill(int nBillID) in financialdlg.cpp)
//5(c): If there's any possibility that the event you linked to will be deleted someday, add a call to UnapplyEvent() where it might be deleted.
//5(d): Add a case to GetItemDescription
//5(e): Add a case to IsEventLinkable
//6: Add a case to the CReportInfo::GetRecordset() case 392 (Ladders) section.
//7: Document everything in \\Luke\Shared\Development\Documentation\p2002\EventTypes.xls and at the top of this file

//Creates an EventsT record, and, if it makes sense, one or more EventAppliesT records.
//Will only apply Events to steps that are currently pending, meaning it won't apply events out of order.
//TS 11/21/01: bFancyDateStuff tells you whether to do all the crazy ActiveDate stuff.
//TS 11/28/01: nStepID allows you to specify a specific step, even if it isn't the currently pending step.
//TS 10/29/02: returns true if at least one event was created, otherwise returns false.
//TS 02/24/03: if you specify -1 for nItemID, it will save NULL to that field.
//TS 04/10/03: also fills in appropriate ProcInfo records.  I think it did this before, but anyway it definitely does now.
//TS 12/29/03: If you specify a StepID, it will forcibly apply the event, without checking whether it actually matches the step.
//TES 12/7/2007 - PLID 28313 - Added a parameter, bAllowCreateLadder, so that this can be called in a way that will 
// not automatically create a new ladder, even if this event would otherwise meet the appropriate criteria to do so.
bool CreateAndApplyEvent(PhaseTracking::EventType nType, int nPatientID, COleDateTime &dt, int nItemID, bool bFancyDateStuff = true, long nStepID = -1, bool bAllowCreateLadder = true);

//Returns a user-friendly description of the event type.
CString GetEventDescription(PhaseTracking::EventType nType);

//Returns a description of the item that triggered the event.
CString GetItemDescription(PhaseTracking::EventType nType, int nItemID);

//Deletes any EventApplies which are of this event type, for this patient, and have this item ID.
//TS 02/24/03: If you set nItemID to -1, it will ignore that and delete all event applies for this type patient and step.
//             Otherwise, nStepID is ignored.
//TES 4/19/2006 - bool bUpdateLadder should be set to false only in the recursive call within UnapplyEvent, that way the 
//code that updates the ladder's status, todo alarms, and active date will only be called once.
// (c.haag 2014-10-13) - PLID 63862 - Call from the tracking API. We no longer need bUpdateLadder.
void UnapplyEvent(int nPatientID, PhaseTracking::EventType nType, int nItemID, int nStepID = -1);

//Returns the first step in the ladder that does _not_ have an event associated with it. (StepsT.ID).
long GetNextStepID(int nLadderID);

//Sets the ActiveDate appropriately for the given ladder
void SetActiveDate(long nLadderID, long nPreviousActiveStepID, long nPreviousActiveStepOrder, bool bFancyDateStuff = true, bool bAllowLadderCreation = true);

//Adds one or more ladders based on "Procedure Groups"
void AddProcedureGroups(CArray<int, int> &arProcGroupIDs, long personID, bool bIsActive = true);

//This function will try to find an event that matches the given step.  If it finds exactly one, it will
//apply it.  If it finds more than one, it will either apply the first one or return, based on bApplyIfMultiple
//Returns true if an event was successfully created, otherwise returns false.
// (c.haag 2013-11-26) - PLID 59822 - We no longer need the ladder, patient, step template or actions. Those are redundant
// fields that can be (and have been) calculated from the step.
bool TryToCompleteStep(long nStepID, bool bApplyIfMultiple);

//This function will complete a step; that is, create appropriate EventsT and EventAppliesT records, mark it done,
//set the status of the ladder, and return the ID of the event.  If nEventID is -1, it will create one.
//TS 02/24/03: if you specify -1 for nItemID, it will save NULL to that field.
// (c.haag 2013-11-26) - PLID 59822 - No function ever checked the return value, so I'm making this void.
void CompleteStep(PhaseTracking::EventType nType, int nPatientID, COleDateTime &dt, int nItemID, long nStep, COleDateTime dtEventDate, long nLadderID, long nEventID = -1);

//(e.lally 2007-05-22) PLID 25112 - This function adds the sql statement to the strSql parameter for taking the
//item out of any PICs so the caller can commit the change when they choose to.
//TES 11/28/2007 - PLID 28210 - Need to pass strSql by reference!
// (d.thompson 2010-01-13) - PLID 36865 - This now returns a parameterized query batch
void GenerateDetachFromProcInfoStatement(PhaseTracking::EventType nType, int nItemID, IN OUT CString &strSqlBatch, IN OUT CNxParamSqlArray &args);

//This function makes sure that any and all todo alarms that are associated with steps in the given ladder are
//appropriately synchronized with the actual state of the ladder.  The function will ensure there is exactly
//one active alarm, that it is associated with the active step, and that it has the same user as the step.
//If we're not doing todos, this function will do nothing.
// (a.walling 2006-10-23 17:50) - PLID 20421 - Pass in a relevant AppointmentID to include cancellation info in the task.
void SyncTodoWithLadder(IN long nLadderID, OPTIONAL IN long nAppointmentIDForReason = -1);

//This function will ensure that the step associated with this ladder is done if the todo is completed, and that
//they have the same reponsible user.  It should be noted that if a step gets marked done, a new step will become
//active, which will fire SyncTodoWithLadder.
//If we're not doing todos, this function will do nothing.
void SyncLadderWithTodo(long nTodoID);

//Fills arEvents with all events that match the given StepsT.ID.
struct TrackingEvent {
	long nID;
	COleDateTime dtDate;
	CString strDescription; //Empty unless you pass in TRUE for bIncludeDescription, because the description often 
							//uses intensive functions like dbo.GetProcInfoName that we don't want to execute unless necessary.
};
/*//bAllowPreviouslyApplied will return events even if they've already been applied to other steps.
//bAllowHistoricalTemplateEvents will ignore any time-based criteria for matching up templates (a.walling PLID 28276)*/
//TES 4/11/2008 - PLID 29510 - Combined bAllowPreviouslyApplied, bAllowHistoricalTemplateEvents (and a new concept I'm
// adding) into a single bManuallyApplied.  If the user is manually applying events, we want to apply looser standards
// then if it's attempting to automatically match up the event.
void GetMatchingEvents(long nStepTemplateID, long nLadderID, long nPatientID, PhaseAction nAction, OUT CArray<TrackingEvent,TrackingEvent&> &arEvents, BOOL bIncludeDescription = FALSE, BOOL bManuallyApplied = FALSE);

//Does a step with this action take you somewhere when you click on it?
bool IsActionLinkable(PhaseTracking::PhaseAction nAction);

//Does a step completed by this event take you somewhere when you click on it?
bool IsEventLinkable(PhaseTracking::EventType nType);

// (j.jones 2009-10-28 15:05) - PLID 35544 - doesn't do any processing,
// just confirms whether a given event can or cannot launch multiple ladders
bool CanEventLaunchMultiple(PhaseTracking::EventType nType);

//This returns the "primary" event type for the given action (i.e., ET_Skipped is valid for every action, but won't be 
//returned by this function for any of them).
EventType GetEventType(PhaseAction Action);

//This returns the ID field of the "primary" database table associated with a given action.
CString GetIDFieldNameFromAction(PhaseAction eAction);

BOOL MergeLadders(long nLadderIDToBeMerged, long nLadderIDToMergeInto, long nPatientID);

// (z.manning, 07/27/2007) - PLID 26846 - Determines if a given action is licensed or not.
BOOL IsTrackingActionLicensed(long nActionID);

// (z.manning, 08/30/2007) - PLID 18359 - Returns true if the give ladder template ID can be safely
// deleted, false otherwise.
BOOL CanDeleteLadderTemplate(long nLadderTemplateID, BOOL bSilent, CWnd *pwndParent);

//TES 11/25/2008 - PLID 32066 - Call this function to update a ProcInfo record's Surgeon to match the ProcInfo's 
// Surgery appointment (tied through the resource).  You may pass in -2 to for EITHER nProcInfoID OR nSurgeryApptID;
// if you do, you must pass in a valid ID for the other one, and the code will find all associated records and update
// them appropriately.  This code also checks whether the preference for this is even on, and does nothing if it isn't.
// You may pass in a pointer to a SQL batch, if you do then this function will add to the batch, but will not actually
// change any data.
// (d.thompson 2009-12-31) - PLID 36740 - Parameterized the batch, added arguments.  These are required if pstrBatch is
//	non-NULL
// (c.haag 2013-11-27) - PLID 59831 - We no longer allow a batch mode that returns SQL. API functions must not reveal
// database structure information.
void UpdateProcInfoSurgeon(long nProcInfoID, long nSurgeryApptID);

// (c.haag 2013-12-17) - PLID 60018 - This should be called when procedures are added to an EMR outside 
// of built-in EMR functionality. This function will update related tracking data
void HandleNewEmrProcedures(int nProcInfoID, CArray<long, long> &arProcIDs);

}


#endif //INCLUDE_PHASE_TRACKING
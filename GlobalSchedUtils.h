#ifndef GLOBAL_SCHEDULER_UTILITIES_H
#define GLOBAL_SCHEDULER_UTILITIES_H

#pragma once

class CNewPatientInsuredParty;
struct BlockReservation;
struct VisibleAppointment;
struct VisibleTemplate;
struct VisibleTemplateRule;
class CReservationReadSet;
class SchedulerMixRule;
class SelectedFFASlot;

#define DEFAULT_APPT_BORDER_COLOR				(RGB(0,0,0))
#define DEFAULT_APPT_BACKGROUND_COLOR			(RGB(255,255,236))
#define DEFAULT_OFFICE_HOURS_TEMPLATE_COLOR		(RGB(255,255,255))

enum EScheduleVisibleTimeRangeOptions
{
	svtroAllTimes = 0,
	svtroOfficeHours = 1,
	svtroCustom = 2,
};

// (z.manning, 07/26/2007) - PLID 14579 - The standard show state values are hardcoded a lot throughout the program
// but better late then never. It's possible to have user-definied ones, of course, but these values can't
// ever change. (BTW, I named this enum before I realized what its initials were.)
enum EAppointmentShowStates
{
	assPending = 0,
	assIn,
	assOut,
	assNoShow,
	assReceived,
};

class CSchedulerView;

struct SchedAuditItems {
	CStringArray aryOldPurpose;
	CStringArray aryNewPurpose;
	CStringArray aryOldResource;
	CStringArray aryNewResource;	
};

// (j.gruber 2012-07-26 13:28) - PLID 51830 - insurance company placements
//just in case we add more in the future
// (j.jones 2014-11-14 11:00) - PLID 64169 - moved the InsurancePlacements enum
// from ResEntryDlg, and renamed the enum
enum SchedulerInsurancePlacements
{
	priIns = 1,
	secIns = 2,
};

// (j.gruber 2012-07-30 11:37) - PLID 51830
struct InsuranceInfo
{
	long nInsuredPartyID;
	CString strInsCoName;
	CString strRespType;
};

// (j.jones 2014-11-14 11:11) - PLID 64169 - turned this map into a typedef
typedef CMap<long, long, InsuranceInfo*, InsuranceInfo*> AppointmentInsuranceMap;

// (j.jones 2014-11-14 10:58) - PLID 64169 - added a global function to fill an InsuranceInfo map
void SetAppointmentInsuranceMap(AppointmentInsuranceMap &mapInsPlacements, long nPlacement,
							long nInsuredPartyID, CString strInsCoName, CString strRespType, bool bErrorOnExistence = false);

// (j.jones 2014-11-14 13:41) - PLID 64169 - added a global function to clear an InsuranceInfo map
void ClearAppointmentInsuranceMap(AppointmentInsuranceMap &mapInsPlacements);

// (j.jones 2014-11-14 13:30) - PLID 64169 - The ability to load the appointment insurance map
// from a patient's insurance is now a global function.
// Returns true if we actually made changes to the insurance map, false if we did not.
bool TryAutoFillAppointmentInsurance(long nPatientID, IN OUT AppointmentInsuranceMap &mapInsPlacements);

// (s.tullis 2015-07-08 11:17) - PLID 63851 
// Need to check and warn Template Rules when deleting Appointment types and purposes
BOOL CheckWarnTemplateRuleDetails(BOOL bIsApptTypeOrPurpose, long nObjectID);

struct APPT_INFO {
	CDWordArray *dwPurposeList;
	CDWordArray *dwResourceList;
	long nApptID;
	long nAptTypeID;
	long nPatientID;
	COleDateTime dtStartTime;
	COleDateTime dtEndTime;
	COleDateTime dtArrivalTime;
	COleDateTime dtApptDate;
	long nLocationID;
	long nShowStateID;
	long nAptInitStatus;
	// (j.gruber 2012-07-31 12:22) - PLID 51830 - added insurance
	// (j.jones 2014-11-26 15:18) - PLID 64169 - removed an unnecessary const
	AppointmentInsuranceMap *pmapInsInfo;
};

// (j.jones 2013-05-07 14:08) - PLID 53969 - moved CLocationOfficeHours to globalschedutls
// Helper class to store office hours for a given location
class CLocationOfficeHours
{
public:
	CLocationOfficeHours();

public:
	void Init(IN int nLocationID);
	BOOL GetOfficeHours(IN int nDayIndex, OUT COleDateTime &dtOpen, OUT COleDateTime &dtClose);
	BOOL GetMinOfficeHours(OUT COleDateTime &dtOpen, OUT COleDateTime &dtClose);
	long GetLocationID() const;

public:
	void EnsureAllOfficeHours();

protected:
	COleDateTime m_dtDayOpen[7];
	COleDateTime m_dtDayClose[7];
	BOOL m_bDayValid[7];

protected:
	BOOL m_bGotOfficeHours;

protected:
	long m_nLocationID;
};

class CDurationGroup
{
protected:
	long m_lAptTypeID;
	long m_lDuration;
	CDWordArray m_adwPurposes;
public:
	CDurationGroup(long lAptTypeID, long lDuration)
	{
		m_lAptTypeID = lAptTypeID;
		m_lDuration = lDuration;
	}
	void AddPurpose(long lPurposeID)
	{
		m_adwPurposes.Add(lPurposeID);
	}
	BOOL InGroup(long lPurposeID)
	{
		for (int i=0; i < m_adwPurposes.GetSize(); i++)
		{
			if (m_adwPurposes[i] == (DWORD)lPurposeID)
				return TRUE;
		}
		return FALSE;
	}
	long GetAptTypeID() { return m_lAptTypeID; }
	long GetDuration() { return m_lDuration; }
	// (j.jones 2010-10-22 16:13) - PLID 36473 - added SetDuration
	void SetDuration(long nNewDuration) { m_lDuration = nNewDuration;}
	int GetPurposeCount() { return m_adwPurposes.GetSize(); }
	CString GetPurposeString() { return ArrayAsString(m_adwPurposes, true); }
};

// (z.manning 2011-05-06 15:33) - PLID 43601
void FillDefaultDurationGroups(CArray<CDurationGroup*,CDurationGroup*> *paryDurationGroups, const CArray<long,long> *paryResourceIDs);

namespace Nx
{
	namespace Scheduler
	{		
		// (a.walling 2015-01-08 11:47) - PLID 64381 - Cancel/noshow reason which can be either an ID or custom text.
		struct Reason
		{
			long id = -1;
			CString text;

			Reason()
			{}

			Reason(long id, CString text = "")
				: id(id)
				, text(text)
			{}

			explicit operator bool() const
			{
				return id != -1 || !text.IsEmpty();
			}

			bool IsCustom() const
			{
				return id == -1 && !text.IsEmpty();
			}
		};

		// (a.walling 2015-01-05 14:30) - PLID 64518 - Drop target support for CNxSchedulerDlg
		extern const CLIPFORMAT cfRes; // = ::RegisterClipboardFormat("Nextech Scheduler Reservation Format");
		// {
		//	DWORD appointmentID
		// }
	}
}

using CancelReason = Nx::Scheduler::Reason;
using NoShowReason = Nx::Scheduler::Reason;

// (j.jones 2007-11-19 10:11) - PLID 28043 - added bCutPaste so the AppointmentCancel functionality
// knows that the appt. is actually being cut and pasted and not flat out cancelled
// (a.walling 2015-01-08 11:47) - PLID 64381 - Allow passing a cancel reason to AppointmentCancel, and a bNeedReschedule flag
// (a.walling 2015-01-30 08:22) - PLID 64542 - bUsePromptPrefix will prepend the description of the appointment and patient with any prompts
bool AppointmentCancel(long nReservationID, bool bNoReasonPrompt = false, bool bNoOtherPrompts = false, bool bWarnOnLink = true, CancelReason cancelReason = {},
					   bool bGenerateSqlOnly = false, CString* pstrSQL = NULL, OUT long* pAuditID = NULL,
					   bool bHandleLinking = true, bool bCutPaste = false, bool bNeedReschedule = false, bool bUsePromptPrefix = false);

// (j.jones 2014-12-02 11:36) - PLID 64183 - added pParentWnd
bool AppointmentUncancel(CWnd *pParentWnd, long nReservationID);

// (j.jones 2007-11-09 16:55) - PLID 27987 - added bReturnAllocations
// (j.jones 2007-11-19 10:11) - PLID 28043 - added bCutPaste so the AppointmentCancel functionality
// knows that the appt. is actually being cut and pasted and not flat out cancelled
bool AppointmentDeleteNoHistory(long nReservationID, bool bNoPrompt = false,
								bool bGenerateSqlOnly = false, CString* pstrSQL = NULL, OUT long* pAuditID = NULL, bool bVoidSuperbills = false, bool bReturnAllocations = false, bool bCutPaste = false);

// (a.walling 2013-01-21 16:48) - PLID 54745 - Appointment count calculation in scheduler reverted to stop using recordsets and instead use Reservation objects' Data[] property map
//long GetCountOpen(CString strIDs);// Returns new Reservation ID (-1 for failure)

// (j.jones 2010-01-04 11:01) - PLID 32935 - added parameter to disable checking for required allocations
// (j.gruber 2012-07-30 13:26) - PLID 51869 - add insurance info
// (j.gruber 2013-01-08 09:09) - PLID 54483 - ReferringPhysID
long AppointmentCreate(long nPatientID, const CDWordArray& adwResourceID, long nLocationID, const COleDateTime &dtDate, 
							  const COleDateTime &dtStartTime, const COleDateTime &dtEndTime, const COleDateTime &dtArrivalTime,
							  long nConfirmed, bool bMoveUp, long lNoShow, const CString &strNotes, 
							  long nAptType, const CDWordArray& adwAptPurpose, bool bReady, long nRequestedResourceID = -1, 
							  BOOL bSendTableCheckerMsg = TRUE, bool bCheckForAllocations = true, const AppointmentInsuranceMap *mapInsInfo = NULL,
							  long nReferringPhysID = -1);

// (j.jones 2010-01-04 11:01) - PLID 32935 - added parameter to disable checking for required allocations
// (j.gruber 2013-01-08 09:09) - PLID 54483 - added referringphysID
bool AppointmentModify(long nReservationID, long nPatientID, const CDWordArray& adwResourceID, long nLocationID, 
							  const COleDateTime &dtDate, const COleDateTime &dtStartTime, const COleDateTime &dtEndTime, const COleDateTime &dtArrivalTime, 
							  long nConfirmed, bool bMoveUp, long lNoShow, const CString &strNotes, 
							  long nAptType, const CDWordArray& adwAptPurpose, SchedAuditItems* saiAuditItems, BOOL bReady,
							  long nRequestedResourceID, bool bCheckForAllocations = true,
							  const AppointmentInsuranceMap *pmapOrigInsInfo = NULL,
							  const AppointmentInsuranceMap *pmapInsInfo = NULL,
							  long nReferringPhysID = -1);

// (j.jones 2014-12-19 09:57) - PLID 64480 - given a SelectedFFASlot object and an existing appointment ID,
// change the appointment to use the new slot information, auditing accordingly
bool AppointmentModifyFromFFASlot(long nAppointmentID, shared_ptr<SelectedFFASlot> newSlot);

BOOL IsSchedulerDirty();
void SchedulerClean();

ADODB::_RecordsetPtr AppointmentGrab(long nReservationID, BOOL bIncludeResourceIDs = FALSE, BOOL bIncludePurposeIDs = FALSE);
// (j.jones 2014-12-02 10:00) - PLID 64182 - added pParentWnd
// (j.jones 2014-12-19 12:50) - PLID 64182 - bAuditDates is set to false if the caller does not need to audit
BOOL AppointmentUpdate(CWnd *pParentWnd, ADODB::_RecordsetPtr pAppt, bool &bAuditDates, long nResIDToIgnore = -1); //rules will ignore a passed in ID

OLE_COLOR AppointmentGetBorderColor(long nAptTypeID);
// (j.jones 2014-12-05 09:10) - PLID 64119 - this now optionally returns the status ID
OLE_COLOR AppointmentGetStatusColor(long nResID, OUT long *pnStatusID = NULL);
// (j.jones 2011-02-11 12:01) - PLID 35180 - required dates and a string of resource IDs
const CString &AppointmentGetText(long nReservationID, COleDateTime dtFromDate, COleDateTime dtToDate, CString strResourceIDs, CSchedulerView *pSchedView = NULL, CString strMultiPurpose = "");
long AppointmentGetPatientID(long nResID);

void UpdatePalmSyncT(long nNewResID);
void UpdatePalmSyncTByPerson(long nPersonID);

BOOL CheckOverwriteMarkIn(DWORD dwReservationID);

// (z.manning, 07/26/2007) - PLID 14579 - Added a parameter that tells the function whether or not we're updaing 
// an appt that's linked to an appt's whose state we just changed.
class CReasonDlg;
BOOL AppointmentMarkNoShow(DWORD dwReservationID, BOOL bUpdatingLinkedAppt = FALSE, CReasonDlg *pdlgReason = NULL);
BOOL AppointmentMarkIn(DWORD dwReservationID, BOOL bUpdatingLinkedAppt = FALSE);
BOOL AppointmentMarkOut(DWORD dwReservationID, BOOL bUpdatingLinkedAppt = FALSE);
BOOL AppointmentMarkPending(DWORD dwReservationID, BOOL bUpdatingLinkedAppt = FALSE);
BOOL AppointmentMarkReceived(DWORD dwReservationID, BOOL bUpdatingLinkedAppt = FALSE);
BOOL AppointmentMarkUserDefined(DWORD dwReservationID, const CString& strShowState, BOOL bUpdatingLinkedAppt = FALSE);
BOOL AppointmentMarkMoveUp(DWORD dwReservationID, BOOL bSetMoveUp);
//(e.lally 2009-09-28) PLID 18809 - Switched the confirm parameter into an enum instead of boolean
BOOL AppointmentMarkConfirmed(DWORD dwReservationID, enum EAppointmentConfirmState acsNewConfirmState);

BOOL AppointmentLink(DWORD nSrcResID, DWORD nDstResID, BOOL bQuietMode);
void AttemptAppointmentLinkAction(long nResID, long nDurationChange = 0);
// (z.manning, 07/25/2007) - PLID 14579 - Prompts to update the show state for any appts linked to nResID.
void UpdateLinkedAppointmentShowState(long nResID, long nShowStateID, CString strShowState, CReasonDlg *pdlgReason = NULL);

void ShowPackageInfo(const IN long nPatientID);

void ColorReservationBackground(LPDISPATCH lpDisp, OLE_COLOR clr);

//TES 8/10/2010 - PLID 39264 - Added dtDate and dtStartTime for use in looking up Location Templates (the function will just look at 
// the date/time portions respectively, so if you have one variable with both you can pass it in as each parameter).
BOOL GetValidAppointmentLocation(long nCurrentLocationID, long& nNewLocationID, long nResID, const COleDateTime &dtDate, const COleDateTime &dtStartTime, const CDWordArray* ResourceIDs = NULL);

void AppointmentCheckPatientProvider(long nPatientID, long nResID);
void AttemptFindAvailableMoveUp(const COleDateTime& dtOldStart, const COleDateTime& dtOldEnd, const COleDateTime& dtNewStart, const COleDateTime& dtNewEnd, long nResID,
								BOOL bResourcesChanged, const CDWordArray& adwChangedResources);

void OpenAppointment(long nResID);

void LogShowStateTime(long nApptID, long nStatus);

// (c.haag 2010-03-08 16:33) - PLID 37326 - Added bValidateTemplates. When TRUE, the appt is validated against scheduler templates.
// nLineItemOverrideID is no longer optional.
// (j.jones 2014-12-02 09:08) - PLID 64178 - added bValidateMixRules
// (j.jones 2014-11-26 10:39) - PLID 64179 - added pParentWnd
// (j.jones 2014-11-26 16:42) - PLID 64178 - Added an optional overriddenMixRules returned,
// this is only filled if the appt. exceeded a scheduler mix rule and a user overrode it. The caller needs to save this info.
BOOL ValidateAppt(CWnd* pParentWnd, APPT_INFO *pApptInfo, CSchedulerView * pView, BOOL bCheckRules, BOOL bSilent, BOOL bIsEvent, BOOL bCheckSurgeryCenter, BOOL bCheckOverWriteIn,
	long nLineItemOverrideID, BOOL bValidateTemplates, BOOL bValidateMixRules, OUT std::vector<SchedulerMixRule> &overriddenMixRules, OUT shared_ptr<SelectedFFASlot> &pSelectedFFASlot);

BOOL IsAptTypeCategorySurgical(short nAptTypeCategory);
BOOL IsAptTypeCategoryNonProcedural(short nAptTypeCategory);

BOOL HasPermissionForResource(long nAppointmentID, ESecurityPermissionType bioAptPerm, ESecurityPermissionType bioAptPermWithPass);

// (z.manning, 11/10/2006) - Moved here from SchedulerView.cpp.
CString CalcExcludedResourceIDs();

void MakeResLookLikeTemplateItem(LPDISPATCH lpRes, const CString& strTemplateName, COLORREF clrTemplate);

// (z.manning, 03/13/2007) - PLID 23635 - Returns the start and end times for the visible range of the schedule based on the preference.
void GetScheduleVisibleTimeRange(IN CLocationOfficeHours lohOfficeHours, IN int nDayOfWeek, OUT COleDateTime &dtStart, OUT COleDateTime &dtEnd);

// (z.manning, 05/10/2007) - PLID 11593 - This function will return the multi purpose string for the current
// reservation in the given read set.
CString GetMultiPurposeStringFromCurrentReservationReadSetPosition(CReservationReadSet &rs);

// (d.moore 2007-07-03 09:14) - PLID 4013 - If the date, time, or resources changed for the
//  appointment, then check to see if it is on the move-up/waiting list and prompt to see if
//  it should be removed.
void CheckAppointmentAgainstWaitingList (long nAppointmentID, long nPatientID, 
	const CDWordArray& adwResourceID, const COleDateTime &dtDate, 
	const COleDateTime &dtStartTime);

// (c.haag 2008-03-24 09:30) - PLID 29328 - Returns TRUE if we can show open allocation labels
// on a schedule list.
BOOL CanShowOpenAllocationsInSchedule();

// (c.haag 2008-03-24 09:30) - PLID 29328 - Returns TRUE if we can show open order labels
// on a schedule list.
BOOL CanShowOpenOrdersInSchedule();

// (j.jones 2008-03-24 14:59) - PLID 29388 - TrySendAppointmentTablecheckerForInventory takes in an appt. ID,
// and a boolean for if it is called from an allocation (FALSE means it is an order), checks the
// related scheduler preference, then sends the tablechecker
void TrySendAppointmentTablecheckerForInventory(long nApptID, BOOL bIsAllocation);

//DRT 6/12/2008 - PLID 9679 - Gets the default superbill templates for a given appointment.
//	This enumeration is for the output format of the paths in the CStringArray.
enum eDefaultSuperbillPathFormat {
	eFmtSuperbillPath = 0,				//The path after \Templates\Forms
	eFmtFullPath,						//The full path, including shared path.
};
void GetDefaultSuperbillTemplates(IN long nApptID, OUT CStringArray *pary, OUT bool &bPromptIfEmpty, IN eDefaultSuperbillPathFormat eOutputFormat);

//TES 6/12/2008 - PLID 28078 - Check whether this appointment requires creating an allocation, and prompt the user to create
// one if so. 
void CheckCreateAllocation(long nAppointmentID, long nPatientID, long nAptTypeID, const CDWordArray &dwaPurposes, long nLocationID);

// (z.manning 2008-07-23 14:15) - PLID 30804 - Moved these 10 functions here from CNxSchedulerDlg
// Also added parameters as necessary so they were no longer specific to that class
// (c.haag 2006-12-04 10:22) - PLID 23666 - Template line item block appearance calculation functions
// (more comments are in the CPP file)
void CheckRule_OfAnyKind(const VisibleTemplateRule* pRule, BOOL& bSelected);
void CheckRule_Type(const VisibleAppointment* pAppt, const VisibleTemplateRule* pRule, BOOL& bSelected, BOOL& bSatisfied);
void CheckRule_Purpose(const VisibleAppointment* pAppt, const VisibleTemplateRule* pRule, BOOL& bSelected, BOOL& bSatisfied);
BOOL AppointmentSatisfiesRule(const VisibleAppointment* pAppt, const VisibleTemplateRule* pRule);
// (z.manning 2011-06-14 16:44) - PLID 41131 - Added bIgnoreWarnings
BOOL AppointmentCanBeMadeOverBlockTemplate(const VisibleAppointment* pAppt, const BlockReservation& br, const COleDateTime& dtWorkingDate, BOOL bIgnoreWarnings = FALSE);
// (c.haag 2009-01-19 10:27) - PLID 25578 - Given an array of procedures, this function will
// look for all the detail procedures, and ensure their masters don't exist in the array.
void PruneMasterIDsOfDetailProcedures(CArray<long,long>& anProcedures,
									  const CMap<long,long,long,long>& mapProcedureMasterIDs);
// (c.haag 2009-01-19 09:54) - PLID 25578 - Given an array of procedures from a source, and array of procedures
// for an existing consult, this function calculate what procedures should be assigned to that existing consult.
void CalculateIdealConsultProcedures(const CArray<long,long>& anSourceProcedures,
									 const CArray<long,long>& anConsultProcedures,
									 const CMap<long,long,long,long>& mapProcedureMasterIDs,
									 CArray<long,long>& anResults);
// (c.haag 2009-01-19 10:41) - PLID 25578 - This function is a utility function for SyncSurgeryPurposesWithConsult
// and UpdateLinkedPICPurposes. Procedure-related data is read in from a recordset and stored in an array and various maps.
void FillProcedureData(ADODB::_RecordsetPtr& prs,
						CArray<long,long>& anProcedureIDs,
						CMap<long,long,CString,LPCTSTR>& mapProcedureIDToName,
						CMap<long,long,long,long>& mapProcedureIDToMasterID
						);
// (c.haag 2009-01-14 15:31) - PLID 25578 - Given a ProcInfoID, this function will invoke a dialog to update all
// the purposes of appointments linked with the record to match the purposes of those on the procedure info record
void UpdateLinkedPICPurposes(long nProcInfoID);
// (c.haag 2007-02-28 14:57) - PLID 24191 - If an appointment has more than one block it
// can suppress, then we need to have a competition between the blocks to see which one 
// is a better fit.
BOOL IsCurrentBlockBetterFit(BlockReservation& brVictim, BlockReservation& br, VisibleAppointment* pAppt, const COleDateTime& dtStart, const COleDateTime& dtEnd);
//TES 10/27/2010 - PLID 40868 - You can now use this function to calculate the TemplateItemID associated with an appointment.  To do
// so, pass in pApptToCheck and pnTemplateItemID. If you do this, the function will fill pnTemplateItemID with the precision template
// blocked by pApptToCheck, or -1 if there is no such precision template, or if pApptToCheck is not in arypAppts.  I admit it's a bit of
// a strange overload, but it ensures that the calculation will always match the display in the scheduler.
void RepopulateBlockResArray_ByAppointments(CArray<BlockReservation, BlockReservation>& aCurBlockRes, CArray<VisibleAppointment*,VisibleAppointment*> &arypAppts, short nOffsetDay, COleDateTime dtWorkingDate, int nInterval, LPDISPATCH lpdispSingleDay, OPTIONAL IN VisibleAppointment* pApptToCheck = NULL, OPTIONAL OUT long *pnTemplateItemID = NULL);
void RepopulateBlockResArray_ByTemplates(CArray<BlockReservation, BlockReservation>& aCurBlockRes, CArray<VisibleTemplate*,VisibleTemplate*> &arypTemplates, short nOffsetDay, COleDateTime dtWorking, BOOL bEnsureMatchingResource);
// (c.haag 2007-02-28 15:57) - PLID 24191 - This function will return the smallest
// number of appointment types contained in a rule which states "If the appointment
// is not one of the following types..." which also includes nTargetAptTypeID
int GetBestFittingAptTypeCountForAnyRule(const VisibleTemplate* pTemplate, long nTargetAptTypeID);
// (c.haag 2007-02-28 16:20) - PLID 24191 - This function will find which 
// "If the appointment is not one of the following purposes..." rule contains the
// most number of purposes included in anApptPurposes.
void GetBestFittingAptPurposeCountForAnyRule(const VisibleTemplate* pTemplate, const CDWordArray& anApptPurposes, OUT int& nMatchingPurposes, OUT int& nNonMatchingPurposes);
// (c.haag 2009-08-06 10:20) - PLID 25943 - This function detects whether appointments
// that are about to be marked as No Show are tied to superbills, and asks the user whether
// they want to void them or not. Returns a message box value: IDYES to void the superbills,
// IDNO to not void them, and IDCANCEL to abort the operation entirely.
// (z.manning 2011-04-01 15:12) - PLID 42973 - Added optional param for whether or not to have a cancel option.
UINT NeedsToVoidNoShowSuperbills(CSqlFragment sqlfragmentApptWhere, bool bCancelOption = true);
// (c.haag 2010-11-16 13:49) - PLID 39444 - Given a location name, this will return
// the location in abbreviated form.
CString GetLocationAbbreviation(CString strLocation);
// (c.haag 2011-06-23) - PLID 44287 - Shows a patient warning in a scheduler-favorable layout
void ShowSchedulerPatientWarning(CWnd* pParentWnd,
		long nPatientID,
		BOOL bSuppressPtWarning = FALSE,
		BOOL bSuppressInsReferralWarning = FALSE,
		BOOL bSuppressCoPayWarning = FALSE,
		BOOL bSuppressAllergyWarning = FALSE,
		BOOL bSuppressEMRProblemWarning = FALSE,
		BOOL bSuppressRewardsWarning = FALSE); // (j.luckoski 2013-03-04 13:16) - PLID 33548
// (c.haag 2011-06-23) - PLID 44287 - This function is used to edit an inventory allocation for an appointment
void AppointmentEditInvAllocation(CWnd* pParentWnd, long nApptID, BOOL bInSchedulerView,
								  long nPatientID = -1, long nLocationID = -1);
// (c.haag 2011-06-23) - PLID 44287 - This function is used to create an inventory allocation for an appointment
void AppointmentCreateInvAllocation(CWnd* pParentWnd, long nApptID, BOOL bInSchedulerView,
								  long nPatientID = -1, long nLocationID = -1);
// (c.haag 2011-06-23) - PLID 44287 - This function is used to create an inventory order for an appointment
void AppointmentCreateInvOrder(CWnd* pParentWnd, long nApptID, BOOL bInSchedulerView,
								  long nPatientID = -1, long nLocationID = -1);
// (c.haag 2011-06-23) - PLID 44287 - This function is used to create a new bill for a patient given an appointment ID
void AppointmentCreateNewBill(CWnd* pParentWnd, long nApptID, BOOL bInSchedulerView,
								  long nPatientID = -1);
// (c.haag 2011-06-23) - PLID 44319 - This function is used to create a new case history for a patient given an appointment ID
void AppointmentCreateCaseHistory(long nApptID, long nPatientID);
// (c.haag 2011-06-23) - PLID 44319 - This function is used to edit an existing case history for a patient given an appointment ID
void AppointmentEditCaseHistory(long nApptID, long nPatientID);

// (z.manning 2011-12-07 12:13) - PLID 46910 - Function to load resources from data into a map
void FillResourceMap(CMap<long,long,CString,LPCTSTR> *pmapResources);
void FillResourceMap(CMap<long,long,CString,LPCTSTR> *pmapResources, CArray<long> *parynResourceIDs);

// (j.jones 2012-04-11 09:48) - PLID 44174 - added global function for deleting resources
void DeleteSchedulerResource(CString &strSqlBatch, long nResourceIDToDelete);

// (j.gruber 2012-07-31 12:14) - PLID 51869
CString GetApptInsAuditString(const AppointmentInsuranceMap *pmapInsInfo);

// (j.gruber 2012-08-02 10:43) - PLID 51885 - put here since used in multiple scheduler dialogs
BOOL SaveInsuredParty(OUT long &nInsPartyID, CNewPatientInsuredParty party, long nPatientID, CString strPatientName);
BOOL AddNewInsuredParty(IN CWnd *pParent, IN CNewPatientInsuredParty patientInsInfo, IN long nPatientID, IN CString strPatientName, OUT long &nInsPartyID, OUT CString &strInsName, OUT CString &strInsCategory);

#endif
#include "stdafx.h"
#include "GlobalSchedUtils.h"
#include "CommonSchedUtils.h"
#include "GlobalUtils.h"
#include "PracProps.h"
#include "ReservationReadSet.h"
#include "AuditTrail.h"
#include "PhaseTracking.h"
#include "client.h"
#include "Msgbox.h"
#include "ResLinkActionDlg.h"
#include "PPCLink.h"
#include "GlobalDataUtils.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "MultiSelectDlg.h"
#include "CaseHistoryDlg.h"
#include "ReasonDlg.h"
#include "PhaseTracking.h"
#include "RoomSelectorDlg.h"
#include "WaitingListUtils.h"
#include "WaitingListMatches.h"
#include "GlobalFinancialUtils.h"
#include "InvUtils.h"
#include "InvPatientAllocationDlg.h"
#include "InvEditOrderDlg.h"
#include "PromptForAllocationDlg.h"
#include "HL7Utils.h"
#include "AppointmentsDlg.h"
#include <HL7ParseUtils.h>
#include "RemoteDataCache.h"
#include "UserWarningDlg.h"
#include "InvOrderDlg.h"
#include "PatientView.h"
#include "GlobalInsuredPartyUtils.h"
#include "Rewards.h" // (j.luckoski 2013-03-04 11:51) - PLID 33548
#include "NewPatientAddInsuredDlg.h"
#include "SchedulerView.h"
#include "BillingModuleDlg.h"
#include "FinancialDlg.h"
#include "NxSchedulerDlg.h"
#include "NexWebLoginInfoDlg.h"
#include <NxPracticeSharedLib\SharedScheduleUtils.h>
#include "ReschedulingUtils.h"
#include "HL7Client_Practice.h"

// (a.walling 2010-11-26 13:08) - PLID 40444 - Updated module tab enums and related code

using namespace ADODB;
// (c.haag 2010-02-04 12:03) - PLID 37221 - Accesses to reservation objects should be done through the
// CReservation class
//#import "SingleDay.tlb" rename("Delete", "DeleteRes")
#include "Reservation.h"
using namespace SINGLEDAYLib;

// (a.walling 2010-01-21 16:43) - PLID 37023 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



// (a.walling 2009-10-13 10:01) - PLID 35930
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CPracticeApp theApp;

namespace Nx
{
	namespace Scheduler
	{		
		// (a.walling 2015-01-05 14:30) - PLID 64518 - Drop target
		const CLIPFORMAT cfRes = (CLIPFORMAT)::RegisterClipboardFormat("Nextech Scheduler Reservation Format");
		// {
		//	DWORD appointmentID
		// }
	}
}

// ToDo: make this cpp file into a class of its own! (Bob)

//TES 6/9/2008 - PLID 23243 - Added parameters for the appointment ID, and its resource IDs, which this packet now sends.
void SendInPatientNotification(long nPatientID, long nAppointmentID, const CDWordArray &dwaResourceIDs)
{
	// (c.haag 2004-04-01 17:41) - Don't send In notifications for non-patients
	if (nPatientID < 0)
		return;

	try {
		// (d.thompson 2010-05-11) - PLID 38581 - Parameterized
		_RecordsetPtr rsPat = CreateParamRecordset("SELECT First, Last, DisplayWarning, WarningMessage FROM PersonT WHERE ID = {INT}", nPatientID);
		FieldsPtr flds = rsPat->Fields;

		CString str, strFirst, strLast;
		if (flds->Item["First"]->Value.vt != VT_NULL)
			strFirst = VarString(flds->Item["First"]->Value);
		if (flds->Item["Last"]->Value.vt != VT_NULL)
			strLast = VarString(flds->Item["Last"]->Value);

		str.Format("The patient %s %s is now in the office.", strFirst, strLast);

		// See if there is a patient warning
		if (AdoFldBool(flds, "DisplayWarning"))
		{
			str += CString("\r\n\r\nWarning: ") + AdoFldString(flds, "WarningMessage", "(Unspecified Warning)");
		}
		//DRT 6/15/2007 - PLID 25531 - Packets are no longer part of NetUtils namespace
		//TES 6/9/2008 - PLID 23243 - Send on the appointment ID and resource IDs we were passed in.
		CClient::SendAlert(str, nAppointmentID, dwaResourceIDs, PatientIsIn);
	}
	NxCatchAll("Error sending in patient notification");
}

void UpdatePalmSyncT(long nNewResID)
{
	// (z.manning, 5/1/2006, PLID 20373) - Don't need to do this if they don't have a palm license.
	if(g_pLicense->GetPalmCountAllowed() == 0) {
		return;
	}

	//
	// (c.haag 2006-05-10 14:18) - PLID 20542 - Merged the two queries into one
	//
	// (d.thompson 2010-05-11) - PLID 38581 - Parameterized and made a batch so failures will rollback properly.
	CParamSqlBatch sql;
	sql.Add("DELETE FROM PalmSyncT WHERE PalmSettingsTID IN "
		"(SELECT PalmSettingsTID FROM PalmSettingsResourceT "
		"INNER JOIN AppointmentResourceT ON AppointmentResourceT.ResourceID = PalmSettingsResourceT.ResourceID "
		"WHERE AppointmentID = {INT}) AND AppointmentID = {INT};", nNewResID, nNewResID);
	sql.Add("INSERT INTO PalmSyncT SELECT PalmSettingsResourceT.PalmSettingsTID, AppointmentResourceT.AppointmentID, 1 "
		"FROM AppointmentResourceT INNER JOIN PalmSettingsResourceT ON "
		"AppointmentResourceT.ResourceID = PalmSettingsResourceT.ResourceID "
		"GROUP BY dbo.PalmSettingsResourceT.PalmSettingsTID, "
		"AppointmentResourceT.AppointmentID HAVING (dbo.AppointmentResourceT.AppointmentID = {INT}) ", nNewResID);
	sql.Execute(GetRemoteData());

	/*
	// Remove all records of this appointment in PalmSyncT unless the following statement is true for
	// that specific record:
	//
	// The user of the PalmSettingsT record associated with the PalmSyncT record synchronizes at least one resource that
	// the appointment is not for.
	//
	// By doing this, we can't fool the program into thinking we should not sync an appointment whose resources were changed.
	//
	ExecuteSql("DELETE FROM PalmSyncT WHERE PalmSettingsTID IN "
		"(SELECT PalmSettingsTID FROM PalmSettingsResourceT "
		"INNER JOIN AppointmentResourceT ON AppointmentResourceT.ResourceID = PalmSettingsResourceT.ResourceID "
		"WHERE AppointmentID = %d) AND AppointmentID = %d",
		nNewResID, nNewResID);


	// Add to PalmSyncT for every palm user that uses this resource. We need the GROUP BY
	// because an appointment can be for two resources and the palm user may sync those
	// same two resources.
	//DRT 1/30/2006 - PLID 19055 - Added argument for the 'Audit' field, which is always 1.
	ExecuteSql("INSERT INTO PalmSyncT SELECT PalmSettingsResourceT.PalmSettingsTID, AppointmentResourceT.AppointmentID, 1 "
		"FROM AppointmentResourceT INNER JOIN PalmSettingsResourceT ON "
		"AppointmentResourceT.ResourceID = PalmSettingsResourceT.ResourceID "
		"GROUP BY dbo.PalmSettingsResourceT.PalmSettingsTID, "
		"AppointmentResourceT.AppointmentID HAVING (dbo.AppointmentResourceT.AppointmentID = %d)", nNewResID);*/
}

void UpdatePalmSyncTByPerson(long nPersonID)
{
	// (z.manning, 5/1/2006, PLID 20373) - Don't need to do this if they don't have a palm license.
	if(g_pLicense->GetPalmCountAllowed() == 0) {
		return;
	}

	// (d.thompson 2010-05-12) - PLID 38581 - Parameterized and added to a batch so errors will report properly.
	CParamSqlBatch sql;
	sql.Add("DELETE FROM PalmSyncT WHERE PalmSettingsTID IN (SELECT PalmSettingsTID FROM PalmSettingsResourceT "
		"INNER JOIN AppointmentResourceT ON AppointmentResourceT.ResourceID = PalmSettingsResourceT.ResourceID "
		"WHERE AppointmentID IN (SELECT ID FROM AppointmentsT WHERE PatientID = {INT})) "
		"AND AppointmentID IN (SELECT ID FROM AppointmentsT WHERE PatientID = {INT});\r\n",
		nPersonID, nPersonID);
	//DRT 1/30/2006 - PLID 19055 - Added argument for the 'Audit' field, which is always 1.
	sql.Add("INSERT INTO PalmSyncT SELECT PalmSettingsResourceT.PalmSettingsTID, AppointmentResourceT.AppointmentID, 1 "
		"FROM AppointmentResourceT INNER JOIN PalmSettingsResourceT ON "
		"AppointmentResourceT.ResourceID = PalmSettingsResourceT.ResourceID "
		"GROUP BY dbo.PalmSettingsResourceT.PalmSettingsTID, "
		"AppointmentResourceT.AppointmentID HAVING (dbo.AppointmentResourceT.AppointmentID IN (SELECT ID FROM AppointmentsT WHERE PatientID = {INT}));\r\n", nPersonID);
	sql.Execute(GetRemoteData());
}

// Return success boolean
// (j.jones 2007-11-19 10:11) - PLID 28043 - added bCutPaste so the AppointmentCancel functionality
// knows that the appt. is actually being cut and pasted and not flat out cancelled
// (d.thompson 2010-06-04) - PLID 39020 - Parameterized much of the "lookup" queries.  Did not touch 
//	the output in pstrSQL.
// (a.walling 2015-01-08 11:47) - PLID 64381 - Allow passing a cancel reason to AppointmentCancel, and a bNeedReschedule flag. Previously strCancelReason was ignored.
// (a.walling 2015-01-30 08:22) - PLID 64542 - bUsePromptPrefix will prepend the description of the appointment and patient with any prompts
bool AppointmentCancel(long nReservationID, bool bNoReasonPrompt /* = false*/, bool bNoOtherPrompts /* = false*/, bool bWarnOnLink /*= true*/, CancelReason cancelReason /*= {}*/, 
					   bool bGenerateSqlOnly /* = false */, CString* pstrSQL /* = NULL */, OUT long* pAuditID /* = NULL */,
					   bool bHandleLinking /* = true */, bool bCutPaste /* = false */, bool bNeedReschedule /* = false */, bool bUsePromptPrefix /* = false */)
{
	bool bVoidSuperbills = false;
	BOOL bReturnAllocations = FALSE;

	long nAuditTransactionID = -1;

	

	long lPatientID;
	BOOL bIsEvent;
	_RecordsetPtr prs;

	// Find the patient ID and event status.
	try {
		// (d.thompson 2010-06-04) - PLID 39020 - Parameterized
		prs = CreateParamRecordset("SELECT PatientID, Date, StartTime, EndTime, ShowState, LocationID, "
			"CASE WHEN StartTime = EndTime AND DATEPART(Hh, StartTime) = 0 AND DATEPART(Mi, StartTime) = 0 AND DATEPART(Ss, StartTime) = 0 THEN 1 ELSE 0 END AS IsEvent, "
			"AptTypeT.Name AS Type, dbo.GetPurposeString(AppointmentsT.ID) AS Purpose, dbo.GetResourceString(AppointmentsT.ID) AS Resource, dbo.GetResourceIDString(AppointmentsT.ID) AS ResourceIDs "
			"FROM AppointmentsT LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
			"WHERE AppointmentsT.ID = {INT}", nReservationID);
		//DRT 3/2/2006 - PLID 13631 - There have been some odd cases in the past where this is indeed not a real appointment.  To
		//	be safe, we really should check EOF before we do anything with the recordset.
		if(prs->eof) {
			if(!bNoOtherPrompts)
				//Follow our prompting rules - we're allowed
				AfxMessageBox("Practice was unable to cancel the appointment, it may have already been removed.  Please refresh the schedule and try again.");

			//we failed to cancel
			return false;
		}

		lPatientID = AdoFldLong(prs, "PatientID");
		bIsEvent = AdoFldLong(prs, "IsEvent");
	}
	NxCatchAllCall("::AppointmentCancel Error", return false);

	CString strPatientName = GetExistingPatientName(lPatientID);
	// (a.walling 2008-06-04 15:39) - PLID 29900
	CString strPatientNameForAudit = (lPatientID == -25) ? "{No Patient Selected}" : strPatientName;
	long nPatientIDForAudit = (lPatientID == -25) ? -1 : lPatientID;

	CString strPrefix;

	if(!bNoOtherPrompts) {

		if (bUsePromptPrefix) {
			// (a.walling 2015-01-30 08:22) - PLID 64542
			CString strResources = AdoFldString(prs, "Resource", "no resource");
			COleDateTime dtStart = AdoFldDateTime(prs, "StartTime");
			strPrefix.Format("Appointment for %s with %s on %s:\r\n\r\n", strPatientNameForAudit, strResources, FormatDateTimeForInterface(dtStart));
		}

		// (j.jones 2006-10-09 15:04) - PLID 22887 - if currently tied to a room, disallow cancelling
		// (d.thompson 2010-06-04) - PLID 39020 - Parameterized
		_RecordsetPtr prsTestRoom = CreateParamRecordset("SELECT ID FROM RoomAppointmentsT WHERE AppointmentID = {INT} AND StatusID <> -1", nReservationID);
		if(!prsTestRoom->eof) {
			AfxMessageBox(strPrefix + "This appointment is currently assigned to a room. You may not cancel an appointment while it is assigned to a room.");
			return false;
		}
		prsTestRoom->Close();

		// (c.haag 2009-10-12 13:14) - PLID 35722 - If currently tied to a history item, disallow deletion. Currently only applies to photos.
		// (d.thompson 2010-06-04) - PLID 39020 - Parameterized
		_RecordsetPtr prsTestMail = CreateParamRecordset("SELECT ID FROM MailSentProcedureT WHERE AppointmentsID = {INT}", nReservationID);
		if(!prsTestMail->eof) {
			AfxMessageBox(strPrefix + "This appointment is currently assigned to a patient photo.\n\nYou may not cancel an appointment while it is assigned to a photo.");
			return false;
		}
		prsTestMail->Close();

		//DRT 7/7/2005 - PLID 16664 - If this appointment is tied to a superbill, warn the user that it will be marked as VOID if they continue to cancel.
		//	Only warn them about ones that are not void already.
		if((GetRemotePropertyInt("ShowVoidSuperbillPrompt", 1, 0, "<None>", false) == 1) && (GetCurrentUserPermissions(bioVoidSuperbills) & sptWrite)) {
			// (d.thompson 2010-06-04) - PLID 39020 - Parameterized
			_RecordsetPtr prsSuperbill = CreateParamRecordset("SELECT SavedID FROM PrintedSuperbillsT WHERE ReservationID = {INT} AND Void = 0", nReservationID);
			CString strSuperbillIDs = "";
			long nSuperbillIDCnt = 0;
			while(!prsSuperbill->eof) {
				long nID = AdoFldLong(prsSuperbill, "SavedID", -1);
				if(nID > -1) {
					CString str;	str.Format("%li, ", nID);
					strSuperbillIDs += str;
					nSuperbillIDCnt++;
				}

				prsSuperbill->MoveNext();
			}
			strSuperbillIDs.TrimRight(", ");
			prsSuperbill->Close();

			if(nSuperbillIDCnt > 0) {
				//They are tied to superbills, we will warn the user and give them an opportunity to give up
				CString strFmt;
				strFmt.Format("This appointment is tied to %li superbill%s (ID%s:  %s)."
					"Would you like to mark these superbills VOID?\r\n\r\n"
					" - If you choose YES, the appointment will be canceled and all related superbill IDs will be marked VOID.\r\n"
					" - If you choose NO, the appointment will be canceled, but all related superbill IDs remain.\r\n"
					" - If you choose CANCEL, the appointment will remain.", nSuperbillIDCnt, nSuperbillIDCnt == 1 ? "" : "s", nSuperbillIDCnt == 1 ? "" : "s", strSuperbillIDs);

				int nRes = AfxMessageBox(strPrefix + strFmt, MB_YESNOCANCEL);
				if(nRes == IDCANCEL)
					return false;
				else if(nRes == IDYES)
					bVoidSuperbills = true;
			}
		}

		// (j.jones 2007-11-07 12:10) - PLID 27987 - handle inventory allocations linked to appts.
		// Check to see if there are any open allocations linked to this appt.		
		// We check the active status only, as opposed to completed or deleted
		// (d.thompson 2010-06-04) - PLID 39020 - parameterized
		_RecordsetPtr prsTestAlloc = CreateParamRecordset("SELECT TOP 1 ID FROM PatientInvAllocationsT WHERE AppointmentID = {INT} AND Status = {INT}", nReservationID, InvUtils::iasActive);
		if(!prsTestAlloc->eof) {
			//there are undeleted allocations, so warn the user
			//(yes, the data structure lets them have more than one, but that's rare - and we may forbid it -
			//so make the message make sense in the 99% of cases where they have only one allocation)
			// (j.jones 2007-11-29 09:57) - PLID 28196 - reworded this to properly explain that only
			// unused products can be returned to stock
			CString strFmt = "This patient has an inventory allocation tied to this appointment.\n"
				"Would you like to return any unused products in this allocation back to purchased stock?\r\n\r\n"

				" - If you choose YES, the appointment will be cancelled and any unused allocated products will be returned to stock.\r\n"
				" - If you choose NO, the appointment will be cancelled and all allocated products will remain allocated to this patient.\r\n"
				" - If you choose CANCEL, the appointment and allocated products will remain.";

			int nRes = AfxMessageBox(strPrefix + strFmt, MB_YESNOCANCEL);
			if(nRes == IDCANCEL)
				return false;
			else if(nRes == IDYES)
				bReturnAllocations = TRUE;
		}
		prsTestAlloc->Close();

		// (j.jones 2008-03-18 15:48) - PLID 29309 - warn if the appt. is linked to an order
		// (j.jones 2008-09-24 16:37) - PLID 31493 - changed to check if multiple orders were linked
		_RecordsetPtr rsOrders = CreateParamRecordset("SELECT ID FROM OrderAppointmentsT "
			"WHERE AppointmentID = {INT}", nReservationID);
		if(!rsOrders->eof) {
			CString strWarning = "This appointment is linked to an inventory order, are you sure you wish to cancel it?";
			if(rsOrders->GetRecordCount() > 1) {
				strWarning = "This appointment is linked to multiple inventory orders, are you sure you wish to cancel it?";
			}

			int nRes = MessageBox(GetActiveWindow(), strPrefix + strWarning, "Practice", MB_ICONQUESTION|MB_YESNO);
			if(nRes != IDYES) {
				return false;
			}
		}
		rsOrders->Close();
	}

	CString str = "";

	BOOL bTracking = FALSE, bCaseHistory = FALSE;

	// (d.thompson 2010-06-04) - PLID 39020 - Parameterized and combined 2 trips into 1 - we only care if there's a result in either.
	_RecordsetPtr prsTestProc = CreateParamRecordset("SELECT ID FROM ProcInfoT WHERE SurgeryApptID = {INT} "
		"UNION SELECT ProcInfoID FROM ProcInfoAppointmentsT WHERE AppointmentID = {INT}", nReservationID, nReservationID);
	if(!prsTestProc->eof) {
		bTracking = TRUE;
	}
	prsTestProc->Close();

	// (d.thompson 2010-06-04) - PLID 39020 - Parameterized
	_RecordsetPtr prsTestCase = CreateParamRecordset("SELECT ID FROM CaseHistoryT WHERE AppointmentID = {INT}", nReservationID);
	if(IsSurgeryCenter(false) && !prsTestCase->eof) {
		bCaseHistory = TRUE;
	}
	prsTestCase->Close();

	if(bTracking && bCaseHistory)
		str = "This appointment is attached to both a tracked procedure and a case history. Are you sure you wish to cancel it?";
	else if(bTracking)
		str = "This appointment is attached to a tracked procedure. Are you sure you wish to cancel it?";
	else if(bCaseHistory) {

		if(theApp.m_arypCaseHistories.GetSize() > 0) {
			long nSize = theApp.m_arypCaseHistories.GetSize();
			for (long i=0; i<nSize; i++) {
				CWnd *pWnd = (CWnd *)theApp.m_arypCaseHistories[i];
				if (pWnd->GetSafeHwnd() && pWnd->IsWindowVisible()) {
					MessageBox(GetActiveWindow(),strPrefix + "Please close all open Case Histories before continuing.",
						"Practice",MB_OK|MB_ICONEXCLAMATION);
					return false;
				}
			}
		}

		str = "This appointment is attached to a case history. Are you sure you wish to cancel it?";
	}

	if(str == "") {
		if(lPatientID == -25) { // (r.goldschmidt 2014-01-22 12:45) - PLID 58637 - Warned about cancelling an appointment for a patient with Person ID of 25 that the appointment is not for a specific patient; changed to -25
			str = "This appointment is not for a specific patient, and therefore can't be recovered.\nAre you absolutely sure you wish to cancel it?";
		}
		else
			str = CString(RCS(IDS_QUESTION_CANCEL_APPOINTMENT));
	}

	//Dialog to get reason for cancelling appointment
	CReasonDlg dlgCancel(NULL);
	dlgCancel.m_bNoShow = false;
	dlgCancel.m_nApptID = nReservationID;
	dlgCancel.m_strText = str;

	if (!bNoReasonPrompt) {
		// (a.walling 2015-02-04 09:12) - PLID 64412 - hide checkbox to add to rescheduling queue
		if (Nx::Scheduler::IsApptInReschedulingQueue(nReservationID)) {
			dlgCancel.m_bDisableReschedulingQueue = true;
		}
	}

	if (bNoReasonPrompt || (dlgCancel.DoModal() == IDOK)) {
		if(!bNoOtherPrompts && bWarnOnLink && bHandleLinking) {
			// Show the user which appointments are linked to this one
			try {
				// Get a list of all the linked appointments
				// (d.thompson 2010-06-04) - PLID 39020 - Parameterized
				_RecordsetPtr prs = CreateParamRecordset("SELECT GroupID FROM AptLinkT WHERE AptLinkT.AppointmentID = {INT}", nReservationID);

				if (!prs->eof)
				{
					// (d.thompson 2010-06-04) - PLID 39020 - Parameterized
					_RecordsetPtr prsAppts = CreateParamRecordset("SELECT AppointmentsT.ID, StartTime, EndTime, AptTypeT.Name, "
						"dbo.GetPurposeString(AppointmentID) AS Purposes FROM AptLinkT "
						"LEFT JOIN AppointmentsT ON AppointmentsT.ID = AptLinkT.AppointmentID "
						"LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
						"WHERE AptLinkT.GroupID = {INT} AND AptLinkT.AppointmentID <> {INT} AND AppointmentsT.Status <> 4",
						AdoFldLong(prs, "GroupID"), nReservationID);
					if (!prsAppts->eof)
					{
						CString strPrompt = "The following appointments are linked to this appointment:\n\n";
						while (!prsAppts->eof)
						{
							CString strAdd;
							COleDateTime dtOldStart = AdoFldDateTime(prsAppts, "StartTime");
							COleDateTime dtOldEnd = AdoFldDateTime(prsAppts, "EndTime");
							CString strType = AdoFldString(prsAppts, "Name", "");
							CString strPurpose = AdoFldString(prsAppts, "Purposes", "");

							// Build our message prompt further
							strAdd.Format("%s-%s %s %s", FormatDateTimeForInterface(dtOldStart,DTF_STRIP_SECONDS), FormatDateTimeForInterface(dtOldEnd, DTF_STRIP_SECONDS, dtoTime), strType, strPurpose);
							strPrompt += strAdd + "\r\n";
							prsAppts->MoveNext();
						}

						strPrompt += "\r\nWould you like to also cancel any of these appointments?";
						strPrompt += "\r\nIf \"Yes,\" you will be prompted to cancel linked appointments.";
						strPrompt += "\r\nIf \"No,\" only this appointment will be cancelled.";
						strPrompt += "\r\nIf \"Cancel,\" no appointments will be cancelled.";
						//TES 8/25/2009 - PLID 35112 - Don't pass string literal into MsgBox
						int nReturn = MsgBox(MB_YESNOCANCEL, "%s", strPrompt);
						if (nReturn == IDCANCEL) {
							return false;
						}
						else if (nReturn == IDYES) {
							//Checks to see if the user wants to input a cancel reason for all the linked appointments
							CString strMsg = "\r\nShould all the linked appointments be cancelled with the"; 
							strMsg += "\r\nsame reason as the first appointment? If not you will be";
							strMsg += "\r\nprompted for the cancel reason for all the linked appointments.";
							//TES 8/25/2009 - PLID 35112 - Don't pass string literal into MsgBox
							int nYesReturn = MsgBox(MB_YESNO, "%s", strMsg);
							bool dontGetReason = true;
							if (nYesReturn == IDNO) dontGetReason = false;
							prsAppts->MoveFirst();
							while(!prsAppts->eof) {
								//No infinite loop here, because we're passing in false for bWarnOnLink,
								//and this line can only be reached if bWarnOnLink is true.
								//TES 3/29/2012 - PLID 48852 - dontGetReason (which is their response to the dialog just above about the cancel
								// reason) was being passed in for bNoReasonPrompt AND bNoOtherPrompts.  But the fact that they don't want to
								// get prompted about the reason does not mean that they don't want to get prompted about all the other things
								// (such as voiding superbills) that they might want to get prompted about when cancelling an appointment.

								//a.walling 2015-01-07 00:00 - PLID 64381 - Previously this reason parameter was ignored in AppointmentCancel")
								AppointmentCancel(AdoFldLong(prsAppts, "ID"), dontGetReason, false, false, dlgCancel.GetReason());
								prsAppts->MoveNext();
								
							}
						}
					}
				}
			}
			NxCatchAll("Error getting linked appointments");
		}

		if (!bNoReasonPrompt) {
			// (a.walling 2015-01-08 12:01) - PLID 64381 - if no reason prompt, use the passed in reason
			cancelReason = dlgCancel.GetReason();
		}
		
		int nInvTodoTransactionID = -1; // (c.haag 2008-02-29 13:01) - PLID 29115 - Support for inventory todo alarm transactions
		try {
			CString strSQLFinal;
			CString strSQLTmp;
			
			// Mark the appointment as cancelled
			if(cancelReason.IsCustom()) {
				// if it is a custom reason store the reason
				strSQLFinal.Format(
					"UPDATE AppointmentsT "
					"SET Status = 4, "
					"ModifiedDate = getdate(), ModifiedLogin = '%s', "
					"CancelledDate = getdate(), CancelledBy = '%s', CancelledReason = '%s' "
					"WHERE ID = %li", _Q(GetCurrentUserName()), _Q(GetCurrentUserName()), _Q(cancelReason.text), nReservationID);
			}
			else {
				// if it is a reason from the drop down box, store the reason ID
				CString strReasonID;
				if(cancelReason.id == -1)
					strReasonID = "NULL";
				else
					strReasonID.Format("%li", cancelReason.id);
				strSQLFinal.Format(
					"UPDATE AppointmentsT "
					"SET Status = 4, "
					"ModifiedDate = getdate(), ModifiedLogin = '%s', "
					"CancelledDate = getdate(), CancelledBy = '%s', CancelReasonID = %s "
					"WHERE ID = %li", _Q(GetCurrentUserName()), _Q(GetCurrentUserName()), strReasonID, nReservationID);
			}

			if (bNeedReschedule || dlgCancel.m_bReschedule) {
				// (a.walling 2014-12-22 12:00) - PLID 64367 - Reschedule when cancelling
				Nx::Scheduler::AddToReschedulingQueue(nReservationID, lPatientID);
			}

			//DRT 7/7/2005 - PLID 16664 - Need to void any superbills which are linked to this appointment.
			if(bVoidSuperbills) {
				// (a.walling 2008-05-06 09:10) - PLID 28063 - Add this to the batch rather than run by itself
				strSQLTmp.Format("UPDATE PrintedSuperbillsT SET Void = 1, VoidDate = GetDate(), VoidUser = '%s' WHERE PrintedSuperbillsT.ReservationID = %li AND Void = 0", _Q(GetCurrentUserName()), nReservationID);
				strSQLFinal = strSQLFinal + ";" + strSQLTmp;
			}

			// (j.jones 2008-03-18 15:57) - PLID 29309 - detach from orders
			strSQLTmp.Format("DELETE FROM OrderAppointmentsT WHERE AppointmentID = %li", nReservationID);
			strSQLFinal = strSQLFinal + ";" + strSQLTmp;

			// (j.jones 2008-03-19 11:38) - PLID 29316 - audit those detachments
			_RecordsetPtr rsOrderInfo = CreateParamRecordset("SELECT OrderT.ID, OrderT.Description, "
				"Last + ', ' + First + ' ' + Middle AS Name, AppointmentsT.Date, AppointmentsT.StartTime "
				"FROM AppointmentsT "
				"INNER JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID "
				"INNER JOIN OrderAppointmentsT ON AppointmentsT.ID = OrderAppointmentsT.AppointmentID "
				"INNER JOIN OrderT ON OrderAppointmentsT.OrderID = OrderT.ID "
				"WHERE AppointmentsT.ID = {INT}", nReservationID);
			while(!rsOrderInfo->eof) {
				long nOrderID = AdoFldLong(rsOrderInfo, "ID");
				CString strOrderDesc = AdoFldString(rsOrderInfo, "Description","");
				CString strPatientName = AdoFldString(rsOrderInfo, "Name","");
				strPatientName.TrimRight();
				COleDateTime dtApptDate = AdoFldDateTime(rsOrderInfo, "Date");
				COleDateTime dtApptTime = AdoFldDateTime(rsOrderInfo, "StartTime");
				CString strOldApptLinkText;
				strOldApptLinkText.Format("Linked to patient '%s', Appointment Date: %s %s", strPatientName, FormatDateTimeForInterface(dtApptDate, NULL, dtoDate), FormatDateTimeForInterface(dtApptTime, DTF_STRIP_SECONDS, dtoTime));

				CString strOld;
				strOld.Format("Order: %s, %s", strOrderDesc, strOldApptLinkText);

				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}
				AuditEvent(-1, "", nAuditTransactionID, aeiInvOrderApptLinkDeleted, nOrderID, strOld, "<No Linked Appointment>", aepMedium, aetDeleted);

				rsOrderInfo->MoveNext();
			}
			rsOrderInfo->Close();		

			// (j.jones 2007-11-07 12:15) - PLID 27987 - if they chose to return allocations, mark only the "active"
			// allocations as deleted (because the details' "returned" status is only used when the allocation is 
			// actually completed, and we shouldn't modify the status of "completed" allocations
			if(bReturnAllocations) {
				// (c.haag 2008-02-29 12:58) - PLID 29115 - Begin an inventory todo transaction
				nInvTodoTransactionID = InvUtils::BeginInventoryTodoAlarmsTransaction();

				// (j.jones 2008-02-27 14:27) - PLID 29102 - remove links from case histories
				strSQLTmp.Format("DELETE FROM CaseHistoryAllocationLinkT WHERE AllocationID IN (SELECT ID FROM PatientInvAllocationsT WHERE AppointmentID = %li)", nReservationID);
				strSQLFinal = strSQLFinal + ";" + strSQLTmp;

				// (j.jones 2008-02-27 14:37) - PLID 29104 - audit those link deletions
				_RecordsetPtr rsCaseHistInfo = CreateParamRecordset("SELECT CaseHistoryT.ID AS CaseHistoryID, CaseHistoryT.Name, "
					"PatientInvAllocationsT.ID AS AllocationID, PatientInvAllocationsT.InputDate "
					"FROM CaseHistoryT "
					"INNER JOIN CaseHistoryAllocationLinkT ON CaseHistoryT.ID = CaseHistoryAllocationLinkT.CaseHistoryID "
					"INNER JOIN PatientInvAllocationsT ON CaseHistoryAllocationLinkT.AllocationID = PatientInvAllocationsT.ID "
					"WHERE PatientInvAllocationsT.AppointmentID = {INT}", nReservationID);
				//with the current design, there should never be more than one linked case history, but the 
				//structure allows it, so let's make it a while loop for accuracy
				while(!rsCaseHistInfo->eof) {
					long nCaseHistoryID = AdoFldLong(rsCaseHistInfo, "CaseHistoryID");
					CString strCaseHistoryName = AdoFldString(rsCaseHistInfo, "Name");
					long nAllocationID = AdoFldLong(rsCaseHistInfo, "AllocationID");
					COleDateTime dtInput = AdoFldDateTime(rsCaseHistInfo, "InputDate");

					CString strAllocationDescription;
					strAllocationDescription.Format("Allocation From %s", FormatDateTimeForInterface(dtInput, NULL, dtoDate));

					//need to audit that we removed the link from both the case history and the allocation
					if(nAuditTransactionID == -1) {
						nAuditTransactionID = BeginAuditTransaction();
					}
					AuditEvent(nPatientIDForAudit, strPatientName, nAuditTransactionID, aeiCaseHistoryInvAllocationLinkDeleted, nCaseHistoryID, strCaseHistoryName + ", " + strAllocationDescription, "<No Linked Allocation>", aepMedium, aetDeleted);
					AuditEvent(nPatientIDForAudit, strPatientName, nAuditTransactionID, aeiInvAllocationCaseHistoryLinkDeleted, nAllocationID, strAllocationDescription + ", Case History: " + strCaseHistoryName, "<No Linked Case History>", aepMedium, aetDeleted);
					rsCaseHistInfo->MoveNext();
				}
				rsCaseHistInfo->Close();

				// (c.haag 2008-02-29 13:04) - PLID 29115 - Update the inventory todo transaction
				_RecordsetPtr prsInvTodo = CreateParamRecordset("SELECT ID FROM PatientInvAllocationsT WHERE AppointmentID = {INT}", nReservationID);
				while (!prsInvTodo->eof) {
					InvUtils::AddToInventoryTodoAlarmsTransaction(nInvTodoTransactionID, InvUtils::eInvTrans_AllocationID, AdoFldLong(prsInvTodo, "ID"));
					prsInvTodo->MoveNext();
				} 
				prsInvTodo->Close();

				strSQLTmp.Format("UPDATE PatientInvAllocationDetailsT SET Status = %li WHERE AllocationID IN (SELECT ID FROM PatientInvAllocationsT WHERE Status = %li AND AppointmentID = %li)", InvUtils::iadsDeleted, InvUtils::iasActive, nReservationID);
				strSQLFinal = strSQLFinal + ";" + strSQLTmp;
				strSQLTmp.Format("UPDATE PatientInvAllocationsT SET Status = %li WHERE Status = %li AND AppointmentID = %li", InvUtils::iasDeleted, InvUtils::iasActive, nReservationID);
				strSQLFinal = strSQLFinal + ";" + strSQLTmp;
			}
			// (j.jones 2007-11-07 12:15) - PLID 27987 - always disconnect all allocations from this appt			
			strSQLTmp.Format("UPDATE PatientInvAllocationsT SET AppointmentID = NULL WHERE AppointmentID = %li", nReservationID);
			strSQLFinal = strSQLFinal + ";" + strSQLTmp;

			// (c.haag 2009-10-14 10:15) - PLID 35722 - Disconnect the cancelled appointment from photos
			// (c.haag 2009-12-15 10:21) - Reversing that decision. If an appt is uncancelled, we still want it bound to the photo
			//strSQLTmp.Format("UPDATE MailSentProcedureT SET AppointmentsID = NULL WHERE AppointmentsID = %li", nReservationID);
			//strSQLFinal = strSQLFinal + ";" + strSQLTmp;

			// (j.jones 2007-11-19 10:15) - PLID 28043 - if we cut and pasted,
			// then we're reassigning the allocation, so we don't need to audit here
			if(!bCutPaste) {
				// (j.jones 2007-11-19 09:39) - PLID 28043 - added allocation auditing
				// we have to audit the appt change for any linked allocations,
				// and then optionally audit the deletion for any linked allocations

				//for auditing, we need to look up the information from the allocation in data
				long nAllocationID = -1;
				_RecordsetPtr rs = CreateParamRecordset("SELECT PatientInvAllocationsT.ID, "
					"CONVERT(datetime, CONVERT(varchar, AppointmentsT.StartTime, 23)) + convert(datetime, RIGHT(CONVERT(varchar, AppointmentsT.StartTime), 7)) AS ApptDateTime "
					"FROM PatientInvAllocationsT "
					"LEFT JOIN AppointmentsT ON PatientInvAllocationsT.AppointmentID = AppointmentsT.ID "
					"WHERE PatientInvAllocationsT.AppointmentID = {INT} AND PatientInvAllocationsT.Status <> {INT}\r\n"
					""
					"SELECT PatientInvAllocationDetailsT.AllocationID, ServiceT.Name AS ProductName, "
					"SerialNum, ExpDate, Quantity "
					"FROM PatientInvAllocationDetailsT "
					"INNER JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID = PatientInvAllocationsT.ID "
					"LEFT JOIN ServiceT ON PatientInvAllocationDetailsT.ProductID = ServiceT.ID "
					"LEFT JOIN ProductItemsT ON PatientInvAllocationDetailsT.ProductItemID = ProductItemsT.ID "
					"WHERE PatientInvAllocationsT.AppointmentID = {INT} "
					"AND PatientInvAllocationsT.Status <> {INT} AND PatientInvAllocationDetailsT.Status <> {INT}", nReservationID, InvUtils::iasDeleted, nReservationID, InvUtils::iasDeleted, InvUtils::iadsDeleted);
				if(!rs->eof) {
					long nAllocationID = AdoFldLong(rs, "ID");
					_variant_t varApptDate = rs->Fields->Item["ApptDateTime"]->Value;					
					if(!bReturnAllocations && varApptDate.vt == VT_DATE) {
						//if not deleting the allocation, audit that the appt. changed
						CString strOldApptDate = FormatDateTimeForInterface(VarDateTime(varApptDate), NULL, dtoDateTime);
						CString strOldValue;
						strOldValue.Format("Appointment: %s", strOldApptDate);

						if(nAuditTransactionID == -1) {
							nAuditTransactionID = BeginAuditTransaction();
						}
						AuditEvent(nPatientIDForAudit, strPatientName, nAuditTransactionID, aeiInvAllocationAppointment, nAllocationID, strOldValue, "<No Appointment>", aepMedium, aetChanged);
					}
					else if(bReturnAllocations) {
						//otherwise just audit that we are deleting
						//no real description to give it, it's defined by its details
						if(nAuditTransactionID == -1) {
							nAuditTransactionID = BeginAuditTransaction();
						}
						AuditEvent(nPatientIDForAudit, strPatientName, nAuditTransactionID, aeiInvAllocationDeleted, nAllocationID, "", "<Deleted>", aepMedium, aetDeleted);
					}
				}

				//no need to loop through details if not deleting the allocation completely
				if(bReturnAllocations) {
					rs = rs->NextRecordset(NULL);
					
					//now loop through any undeleted details
					while(!rs->eof) {
						
						CString strDesc;

						CString strProductName = AdoFldString(rs, "ProductName","");
						_variant_t varSerialNumber = rs->Fields->Item["SerialNum"]->Value;
						_variant_t varExpDate = rs->Fields->Item["ExpDate"]->Value;
						double dblQuantity = AdoFldDouble(rs, "Quantity",1.0);
						
						strDesc = strProductName;
						if(varSerialNumber.vt == VT_BSTR) {
							CString str;
							str.Format(", Serial Num: %s", VarString(varSerialNumber));
							strDesc += str;
						}
						if(varExpDate.vt == VT_DATE) {
							CString str;
							str.Format(", Exp. Date: %s", FormatDateTimeForInterface(VarDateTime(varExpDate), NULL, dtoDate));
							strDesc += str;
						}
						if(dblQuantity != 1.0) {
							//only show the quantity if not 1.0
							CString str;
							str.Format(", Quantity: %g", dblQuantity);
							strDesc += str;
						}

						//and now audit using this description
						if(nAuditTransactionID == -1) {
							nAuditTransactionID = BeginAuditTransaction();
						}
						AuditEvent(nPatientIDForAudit, strPatientName, nAuditTransactionID, aeiInvAllocationDetailDeleted, nAllocationID, strDesc, "<Deleted>", aepMedium, aetDeleted);

						rs->MoveNext();
					}
				}
				rs->Close();
			}

			//DRT 6/30/03 - We need to remove any instances of this appt that is grouped (AptLinkT).
			// (c.haag 2004-03-23 10:39) - We shouldn't do this in a cut-paste operation; the
			// linking will be handled by the scheduler view.
			if (bHandleLinking)
			{
				long nGroupID, nCount;
				// (d.thompson 2010-06-04) - PLID 39020 - Parameterized
				_RecordsetPtr prsGroup = CreateParamRecordset("SELECT GroupID, (SELECT Count(A.AppointmentID) FROM AptLinkT A WHERE A.GroupID = AptLinkT.GroupID) AS ApptCount FROM AptLinkT WHERE AppointmentID = {INT}", nReservationID);
				if(!prsGroup->eof) {
					nGroupID = AdoFldLong(prsGroup, "GroupID");
					nCount = AdoFldLong(prsGroup, "ApptCount");

					//now make sure that group is still valid, not just 1 single item (which would happen
					//if our cancelled appt was the only thing)
					if(nCount <= 2) {
						//there were 1 (bad group to begin with) or 2 (this appt and 1 other) appts in the group.  We can
						//safely delete the whole group.
						strSQLTmp.Format("DELETE FROM AptLinkT WHERE GroupID = %li", nGroupID);
						strSQLFinal += ";" + strSQLTmp;
					}
					else {
						//there is more than 2 appts, so just remove our cancelled one
						strSQLTmp.Format("DELETE FROM AptLinkT WHERE AppointmentID = %li", nReservationID);
						strSQLFinal += ";" + strSQLTmp;
					}
				}
			}

			// (c.haag 2004-03-05 14:38) - If we only want to get the SQL statement, just
			// store it. Otherwise, execute it.
			if (bGenerateSqlOnly)
			{
				if (!pstrSQL)
				{
					ASSERT(FALSE);

					if(nAuditTransactionID != -1) {
						RollbackAuditTransaction(nAuditTransactionID);
					}

					return false;
				}
				*pstrSQL = strSQLFinal;
			}
			else
			{
				//(e.lally 2006-08-10) PLID 21906 - This sql statement is pre-formatted so we want to use ExecuteSqlStd
				ExecuteSqlStd(strSQLFinal);
				// Update Microsoft Outlook
				PPCDeleteAppt(nReservationID); 
				// Update PalmSyncT
				UpdatePalmSyncT((DWORD)nReservationID);
				
				// (j.jones 2007-09-06 15:19) - PLID 27312 - required the EndTime as a parameter
				// (j.jones 2014-08-05 10:35) - PLID 63167 - added PatientID, LocationID, ResourceIDs
				CClient::RefreshAppointmentTable(nReservationID, lPatientID, AdoFldDateTime(prs, "StartTime"), AdoFldDateTime(prs, "EndTime"), 4, AdoFldLong(prs, "ShowState"),
					AdoFldLong(prs, "LocationID"), AdoFldString(prs, "ResourceIDs", ""));

				// (z.manning 2015-11-05 12:13) - PLID 57109 - Update recalls needing attention dialog
				GetMainFrame()->HandleRecallChanged();
			}

			//auditing
			COleDateTime dtDate, dtStart;
			dtDate = COleDateTime(prs->Fields->Item["Date"]->Value.date);
			dtStart = COleDateTime(prs->Fields->Item["StartTime"]->Value.date);
			CString strDate;
			strDate = FormatDateTimeForSql(COleDateTime(dtDate.GetYear(), dtDate.GetMonth(), dtDate.GetDay(), dtStart.GetHour(), dtStart.GetMinute(), dtStart.GetSecond()));
			if(nAuditTransactionID == -1) {
				nAuditTransactionID = BeginAuditTransaction();
			}
			AuditEvent(nPatientIDForAudit, strPatientName, nAuditTransactionID, aeiApptStatus, nReservationID, strDate, "Cancelled", aepHigh);

			// Make sure the time stamp is incremented
			//IncrementTimeStamp(RES_TIME_STAMP_NAME);

			//Track that this appointment has been cancelled.
			if (!bGenerateSqlOnly)
			{
				// (d.thompson 2010-06-04) - PLID 39020 - Removed - we already know the patient ID as lPatientID above!
				//_RecordsetPtr rsPatientID = CreateParamRecordset("SELECT PatientID FROM AppointmentsT WHERE ID = {INT}", nReservationID);
				PhaseTracking::UnapplyEvent(lPatientID, PhaseTracking::ET_AppointmentCreated, nReservationID);
				PhaseTracking::UnapplyEvent(lPatientID, PhaseTracking::ET_ActualAppointment, nReservationID);

				//see if they want to delete the case histories
				// (d.thompson 2010-06-04) - PLID 39020 - Parameterized
				_RecordsetPtr rs = CreateParamRecordset("SELECT Count(ID) AS CountOfID FROM CaseHistoryT WHERE AppointmentID = {INT}",nReservationID);
				if(!rs->eof) {
					long Count = AdoFldLong(rs, "CountOfID",0);
					BOOL bDelete = FALSE;
					if(Count == 1) {
						if(IDYES == MsgBox(MB_ICONQUESTION|MB_YESNO,strPrefix + "This appointment had a case history associated with it. Do you want to delete this case history?")) {
							bDelete = TRUE;
						}
					}
					else if(Count > 1) {
						CString str;
						str.Format("This appointment had %li case histories associated with it. Do you want to delete these case histories?",Count);
						if(IDYES == MsgBox(MB_ICONQUESTION|MB_YESNO,strPrefix + str)) {
							bDelete = TRUE;
						}
					}

					CString strApptDesc;
					// (d.thompson 2010-06-04) - PLID 39020 - Parameterized
					_RecordsetPtr rs2 = CreateParamRecordset("SELECT ID FROM CaseHistoryT WHERE AppointmentID = {INT}", nReservationID);
					while(!rs2->eof) {

						long nCaseHistoryID = AdoFldLong(rs2, "ID",-1);

						bool bDeleted = false;
						if(bDelete) {
							// Make sure it hasn't been billed
								if (CCaseHistoryDlg::IsCaseHistoryBilled(nCaseHistoryID)) {
									AfxMessageBox(strPrefix + "You may not delete this case history because it has already been billed.");
								}
								// (j.jones 2009-08-06 11:10) - PLID 7397 - added check to see if the case is linked to a PIC
								else if(CCaseHistoryDlg::IsCaseHistoryInProcInfo(nCaseHistoryID)) {
									if(IDYES == MessageBox(GetActiveWindow(), strPrefix + "This case history is linked to a Procedure Information Center. Are you sure you wish to delete it?",
										"Practice", MB_ICONQUESTION|MB_YESNO)) {

										CCaseHistoryDlg::DeleteCaseHistory(nCaseHistoryID);
										bDeleted = true;
									}
								}
								else {
									CCaseHistoryDlg::DeleteCaseHistory(nCaseHistoryID);
									bDeleted = true;
								}
						}
						if(!bDeleted) {
							// (d.thompson 2010-06-04) - PLID 39020 - Parameterized
							ExecuteParamSql("UPDATE CaseHistoryT SET AppointmentID = NULL WHERE ID = {INT}", nCaseHistoryID);
							//TES 1/9/2007 - PLID 23575 - Audit that this case history's appointment has changed.
							if(strApptDesc.IsEmpty()) {
								// This is designed to display the appointment information in the same format as it appears
								// on the case history itself, see CCaseHistoryDlg::DisplayAppointmentDesc().
								COleDateTime dtDate = AdoFldDateTime(prs, "Date");
								COleDateTime dtStartTime = AdoFldDateTime(prs, "StartTime");
								CString strType = AdoFldString(prs, "Type", "<No Type>");
								CString strPurpose = AdoFldString(prs, "Purpose", "<No Purpose>");
								if(strPurpose.IsEmpty()) strPurpose = "<No Purpose>";
								CString strResource = AdoFldString(prs, "Resource");
								strApptDesc.Format("%s %s, %s - %s, %s",FormatDateTimeForInterface(dtDate,NULL,dtoDate),
									FormatDateTimeForInterface(dtStartTime, DTF_STRIP_SECONDS, dtoTime),strType,strPurpose,strResource);
							}
							if(nAuditTransactionID == -1) {
								nAuditTransactionID = BeginAuditTransaction();
							}
							// (a.walling 2008-06-04 15:35) - PLID 29900 - Use the correct patient name
							AuditEvent(nPatientIDForAudit, strPatientNameForAudit, nAuditTransactionID, aeiCaseHistoryAppointment, nCaseHistoryID, strApptDesc, "<None> (Appt was cancelled)", aepMedium, aetChanged);
						}

						rs2->MoveNext();
					}
					rs2->Close();
				}
				rs->Close();
			}

			// Allow the user to bring up a list of moveup appointments and
			// fill the now-opened slot
			// (c.haag 2004-03-05 14:42) - We really need to put this in its own function!
			if (!bNoOtherPrompts && UserPermission(SchedulerModuleItem) && GetMainFrame() && !bIsEvent)
			{
				//TES 12/17/2008 - PLID 32478 - If they don't have the Enterprise edition, they can't 
				// use the moveup list, so no need to even create this recordset.
				if(g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrSilent)) {
					// (d.moore 2007-05-22 10:50) - PLID 4013 - Changed the MoveUp bit to query the waiting list table.
					// (d.moore 2007-10-02) - PLID 4013 - I rewrote the whole query to make it more accurate.
					// (d.thompson 2010-06-04) - PLID 39020 - Parameterized
					_RecordsetPtr prs = CreateParamRecordset(
						"SELECT COUNT(WaitingListT.ID) AS MatchCount "
						"FROM WaitingListT "
							"INNER JOIN WaitingListItemT "
							"ON WaitingListT.ID = WaitingListItemT.WaitingListID "
							"LEFT JOIN WaitingListItemResourceT "
							"ON WaitingListItemT.ID = WaitingListItemResourceT.ItemID "
						"WHERE "
						"(WaitingListItemT.StartDate <= {STRING} AND "
							"WaitingListItemT.EndDate >= {STRING} AND "
							"WaitingListItemT.StartTime <= {STRING} AND "
							"WaitingListItemT.EndTime >= {STRING}) "
							"AND "
							"(WaitingListItemT.AllResources = 1 OR "
							"WaitingListItemResourceT.ResourceID = "
								"(SELECT RequestedResourceID "
								"FROM AppointmentsT "
								"WHERE ID = {INT}) OR "
							"WaitingListItemResourceT.ResourceID IN "
								"(SELECT ResourceID "
								"FROM AppointmentResourceT "
								"WHERE AppointmentID = {INT}))", 
						FormatDateTimeForSql(dtDate, dtoDate), 
						FormatDateTimeForSql(dtDate, dtoDate), 
						FormatDateTimeForSql(dtStart, dtoTime), 
						FormatDateTimeForSql(dtStart, dtoTime), 
						nReservationID, nReservationID);
					if (!prs->eof)
					{
						long nMatchCount = AdoFldLong(prs, "MatchCount");
						if (nMatchCount > 0) {
							COleDateTime* pdtStart = new COleDateTime;
							*pdtStart = dtStart;
							GetMainFrame()->PostMessage(NXM_PROMPT_MOVEUP, (WPARAM)nReservationID, (LPARAM)pdtStart);
						}
					}
				}
			}

			if (pAuditID) {
				*pAuditID = nAuditTransactionID;
			}

			//commit our audit
			if(!bGenerateSqlOnly && nAuditTransactionID != -1) {
				CommitAuditTransaction(nAuditTransactionID);
			}

			// (c.haag 2008-02-29 13:02) - PLID 29115 - Update inventory todo alarms
			if (-1 != nInvTodoTransactionID) {
				//TES 11/15/2011 - PLID 44716 - This function needs to know if we're in a transaction
				InvUtils::CommitInventoryTodoAlarmsTransaction(nInvTodoTransactionID, false);
			}

			// (z.manning 2008-07-16 14:34) - PLID 30490 - Handle any HL7 messages relating to this appointment
			// (r.gonet 12/03/2012) - PLID 54108 - Updated to use refactored function.
			SendCancelAppointmentHL7Message(nReservationID);

			// Return success
			return true;
		} NxCatchAllCall("::AppointmentCancel Error", {
			if(nAuditTransactionID != -1) {
				RollbackAuditTransaction(nAuditTransactionID);
			}
			// (c.haag 2008-02-29 13:02) - PLID 29115 - Roll back any inventory todo transactions
			if (-1 != nInvTodoTransactionID) {
				InvUtils::RollbackInventoryTodoAlarmsTransaction(nInvTodoTransactionID);
			}
			return false;
		});
	} else {
		// Return failure
		return false;
	}
}

// Return success boolean
// (j.jones 2014-12-02 11:36) - PLID 64183 - added pParentWnd
bool AppointmentUncancel(CWnd *pParentWnd, long nReservationID)
{
	try {

		_RecordsetPtr prs = AppointmentGrab(nReservationID, TRUE, TRUE);
		// (b.spivey, February 6, 2015) - PLID 64395 - for some reason we check in AppointmentGrab() for eof and return null, but we assume it's valid here. Lets not. 
		if (!prs) {
			AfxMessageBox("Appointment could not be found."); 
			return false; 
		}
		FieldsPtr f = prs->Fields;
		long nPatientID = AdoFldLong(f, "PatientID");
		long nLocationID = AdoFldLong(f, "LocationID");
		long nAptTypeID = AdoFldLong(f, "AptTypeID", -1);
		COleDateTime dtDate = AdoFldDateTime(f, "Date");
		COleDateTime dtStartTime = AdoFldDateTime(f, "StartTime");
		// (j.jones 2014-12-02 11:29) - PLID 64183 - get the primary insured ID
		long nPrimaryInsuredPartyID = AdoFldLong(f, "PrimaryInsuredPartyID", -1);

		CDWordArray adwResourceIDs;
		LoadResourceIDStringIntoArray(AdoFldString(f, "ResourceIDs", ""), adwResourceIDs);

		CDWordArray adwPurposeIDs;
		LoadPurposeIDStringIntoArray(AdoFldString(f, "PurposeIDs", ""), adwPurposeIDs);

		// (c.haag 2003-07-29 12:01) - Check against permissions
		if (!AppointmentValidateByPermissions(prs))
			return false;

		CString strFatalWarnings, strNonFatalWarnings;
		long lResult;
		AppointmentValidateByRules(prs, &strFatalWarnings, &strNonFatalWarnings, &lResult);
		if (lResult & rwIsPrevented)
		{
			if (!strFatalWarnings.IsEmpty()) {
				MsgBox(MB_ICONEXCLAMATION|MB_OK, "Restoring this appointment would violate the following template rules:\n\n%s", strFatalWarnings);
				return false;
			} else {
				MsgBox(MB_ICONEXCLAMATION|MB_OK, "Restoring this appointment would violate at least one template rule.");
				return false;
			}
		}
	  	if (lResult & rwHasWarnings) {
			if (!strNonFatalWarnings.IsEmpty()) {
				TruncateSchedulerWarning(strNonFatalWarnings);
				if (IDNO == MsgBox(MB_ICONEXCLAMATION|MB_YESNO, "This appointment triggers the following warnings:\n\n%s\n\nDo you still wish to restore it?", strNonFatalWarnings))
					return false;
			} else { 
				if (IDNO == MsgBox(MB_ICONEXCLAMATION|MB_YESNO, "This appointment triggers at least one unspecified warning. Do you still wish to restore it?"))
					return false;
			}
		}
		// (c.haag 2003-07-29 12:01) - Check against booking alarms
		//if (!AppointmentValidateByAlarms(prs))
		//	return false;
		strFatalWarnings.Empty();
		strNonFatalWarnings.Empty();
		lResult = 0;
		AppointmentValidateByAlarms(prs, &strFatalWarnings, &strNonFatalWarnings, &lResult);
		if (lResult & rwIsPrevented)
		{
			if (!strFatalWarnings.IsEmpty()) {
				MsgBox(MB_ICONEXCLAMATION|MB_OK, "Restoring this appointment would violate the following booking alarms:\n\n%s", strFatalWarnings);
				return false;
			} else {
				MsgBox(MB_ICONEXCLAMATION|MB_OK, "Restoring this appointment would violate at least one booking alarm.");
				return false;
			}
		}
	  	if (lResult & rwHasWarnings) {
			if (!strNonFatalWarnings.IsEmpty()) {
				TruncateSchedulerWarning(strNonFatalWarnings);
				if (IDNO == MsgBox(MB_ICONEXCLAMATION|MB_YESNO, "This appointment triggers the following booking alarms:\n\n%s\n\nDo you still wish to restore it?", strNonFatalWarnings))
					return false;
			} else { 
				if (IDNO == MsgBox(MB_ICONEXCLAMATION|MB_YESNO, "This appointment triggers at least one unspecified booking alarm. Do you still wish to restore it?"))
					return false;
			}
		}
		
		prs->Close();

		// (j.jones 2014-12-02 11:26) - PLID 64183 - check scheduler mix rules
		std::vector<SchedulerMixRule> overriddenMixRules;
		SelectedFFASlotPtr pSelectedFFASlot;
		pSelectedFFASlot.reset();
		if (!AppointmentValidateByMixRules(pParentWnd, nPatientID, AsDateNoTime(dtDate), nLocationID, nPrimaryInsuredPartyID, nAptTypeID, adwResourceIDs, adwPurposeIDs,
			overriddenMixRules, pSelectedFFASlot)) {
			return false;
		}

		// (j.jones 2014-12-19 10:52) - PLID 64183 - if a new appointment slot was provided, move the appt.
		// to that slot
		if (pSelectedFFASlot != NULL && pSelectedFFASlot->IsValid()) {
			if (!AppointmentModifyFromFFASlot(nReservationID, pSelectedFFASlot)) {
				return false;
			}
		}

		// Mark the appointment as restored
		long nAuditID = BeginNewAuditEvent();
		AuditEvent(nPatientID, GetExistingPatientName(nPatientID), nAuditID, aeiApptStatus, nReservationID, "Cancelled", "Restored", aepHigh);

		// (d.thompson 2010-05-11) - PLID 38581 - Parameterized
		ExecuteParamSql(
			"UPDATE AppointmentsT "
			"SET Status = 1, "
			"ModifiedDate = getdate(), ModifiedLogin = {STRING}, "
			"CancelledDate = NULL, CancelledBy = NULL, CancelledReason = '', CancelReasonID = NULL "
			"WHERE ID = {INT}", GetCurrentUserName(), nReservationID);

		// (j.jones 2014-12-02 14:47) - PLID 64183 - if the user overrode a mix rule,
		// track that they did so
		if (overriddenMixRules.size() > 0) {
			TrackAppointmentMixRuleOverride(nReservationID, overriddenMixRules);
		}
		
		// Make sure the time stamp is incremented
		//IncrementTimeStamp(RES_TIME_STAMP_NAME);

		// Update Microsoft Outlook
		PPCAddAppt(nReservationID);
		// Update PalmSyncT
		UpdatePalmSyncT((DWORD)nReservationID);

		// (z.manning 2008-07-16 14:34) - PLID 30490 - Handle any HL7 messages relating to this appointment
		// (r.gonet 12/03/2012) - PLID 54106 - Updated to use refactored function.
		SendNewAppointmentHL7Message(nReservationID, true);

		// (z.manning 2015-11-05 12:13) - PLID 57109 - Update recalls needing attention dialog
		GetMainFrame()->HandleRecallChanged();

		// (e.frazier 2016-05-27 12:18) - PLID-68670 - Restoring an appointment from the scheduler should remove it from the rescheduling queue
		if (Nx::Scheduler::IsApptInReschedulingQueue(nReservationID))
		{
			Nx::Scheduler::RemoveFromReschedulingQueue(nReservationID);
		}

		//This appointment is back from the dead!  Let's try to track it again.
		//TES 6/13/2008 - PLID 28078 - We need some extra info to pass into CheckCreateAllocation.
		//TES 8/4/2008 - PLID 28078 - Combined and parameterized queries.
		// (j.jones 2014-08-14 09:59) - PLID 63167 - added resources
		_RecordsetPtr rsAppointment = CreateParamRecordset("SELECT PatientID, StartTime, EndTime, CreatedDate, ShowState, AptTypeID, "
			"LocationID FROM AppointmentsT WHERE ID = {INT}; "
			"SELECT PurposeID FROM AppointmentPurposeT WHERE AppointmentID = {INT}; "
			"SELECT ResourceID FROM AppointmentResourceT WHERE AppointmentID = {INT}",
			nReservationID, nReservationID, nReservationID);
		nPatientID = AdoFldLong(rsAppointment, "PatientID");
		COleDateTime dtCreatedDate = AdoFldDateTime(rsAppointment, "CreatedDate");
		nLocationID = AdoFldLong(rsAppointment, "LocationID");
		dtStartTime = AdoFldDateTime(rsAppointment, "StartTime");
		COleDateTime dtEndTime = AdoFldDateTime(rsAppointment, "EndTime");
		long nShowState = AdoFldLong(rsAppointment, "ShowState");
		nAptTypeID = AdoFldLong(rsAppointment, "AptTypeID", -1);

		rsAppointment = rsAppointment->NextRecordset(NULL);
		CDWordArray dwaPurposes;
		while(!rsAppointment->eof) {
			dwaPurposes.Add((DWORD)AdoFldLong(rsAppointment, "PurposeID"));
			rsAppointment->MoveNext();
		}

		// (j.jones 2014-08-14 09:59) - PLID 63167 - added resources
		rsAppointment = rsAppointment->NextRecordset(NULL);
		CString strResourceIDs;
		while (!rsAppointment->eof) {
			if (!strResourceIDs.IsEmpty()) {
				//this is space-delimited, not CSV, to represent dbo.GetResourceIDString
				strResourceIDs += " ";
			}
			strResourceIDs += FormatString("%li", AdoFldLong(rsAppointment, "ResourceID"));
			rsAppointment->MoveNext();
		}
		rsAppointment->Close();

		// (j.jones 2007-09-06 15:19) - PLID 27312 - required the EndTime as a parameter
		// (j.jones 2014-08-05 10:35) - PLID 63167 - added PatientID, LocationID, ResourceIDs
		CClient::RefreshAppointmentTable(nReservationID, nPatientID, dtStartTime, dtEndTime, 1, nShowState, nLocationID, strResourceIDs);
		
		//TES 6/13/2008 - PLID 28078 - Prompt to create an allocation, if appropriate.
		CheckCreateAllocation(nReservationID, nPatientID, nAptTypeID, dwaPurposes, nLocationID);

		PhaseTracking::CreateAndApplyEvent(PhaseTracking::ET_AppointmentCreated, nPatientID, dtCreatedDate, nReservationID);

		// Return success
		return true;
	} NxCatchAllCall("::AppointmentUncancel Error", return false);
}


// Return success boolean (this completely removes records from the data!)
// (j.jones 2007-11-09 16:55) - PLID 27987 - added bReturnAllocations
// (j.jones 2007-11-19 10:11) - PLID 28043 - added bCutPaste so the AppointmentCancel functionality
// knows that the appt. is actually being cut and pasted and not flat out cancelled
// (d.thompson 2010-06-04) - PLID 39020 - Parameterized much of the "lookup" queries.  Did not touch 
//	the output in pstrSQL.
bool AppointmentDeleteNoHistory(long nReservationID, bool bNoPrompt /* = false */,
								bool bGenerateSqlOnly /* = false */, CString* pstrSQL /* = NULL */, OUT long* pAuditID /* = NULL */, 
								bool bVoidSuperbills /*= false*/, bool bReturnAllocations /*= false*/, bool bCutPaste /* = false */)
{
	long nAuditTransactionID = -1;
	int nInvTodoTransactionID = -1; // (c.haag 2008-02-29 13:01) - PLID 29115 - Support for inventory todo alarm transactions

	if (bNoPrompt || (AfxMessageBox("Are you sure you want to completely delete this appointment?  It will not\n"
		 "show up in the patient appointment history and it will not be restorable.",MB_ICONQUESTION|MB_YESNO) == IDYES)) {	

		try {

					
			// (a.walling 2013-01-17 09:26) - PLID 54651 - Check for any appointments linked to EMNs			
			if (!bNoPrompt) {
				// this is dead code, bNoPrompt is always true, but here for posterity

				CString strLinkedEMNs = GetLinkedEMNDescriptionsFromAppointment(nReservationID);

				if (!strLinkedEMNs.IsEmpty()) {
					AfxMessageBox(FormatString("This appointment is linked to the following EMN data and may not be deleted:\r\n\r\n%s", strLinkedEMNs), MB_ICONERROR);
					return false;
				}
			}

			// (j.jones 2006-10-09 15:04) - PLID 22887 - if currently tied to a room, disallow deletion
			if(!bNoPrompt) {
				// (d.thompson 2010-06-04) - PLID 39020 - Parameterized
				_RecordsetPtr prsTestRoom = CreateParamRecordset("SELECT ID FROM RoomAppointmentsT WHERE AppointmentID = {INT} AND StatusID <> -1", nReservationID);
				if(!prsTestRoom->eof) {
					AfxMessageBox("This appointment is currently assigned to a room. You may not delete an appointment while it is assigned to a room.");
					return false;
				}
			}

			// (c.haag 2009-10-12 13:14) - PLID 35722 - If currently tied to a history item, disallow deletion. Currently only applies to photos.
			if(!bNoPrompt) {
				// (d.thompson 2010-06-04) - PLID 39020 - Parameterized
				_RecordsetPtr prsTestHistory = CreateParamRecordset("SELECT ID FROM MailSentProcedureT WHERE AppointmentsID = {INT}", nReservationID);
				if(!prsTestHistory->eof) {
					AfxMessageBox("This appointment is currently assigned to a patient photo.\n\nYou may not delete an appointment while it is assigned to a photo.");
					return false;
				}
			}

			//DRT 7/8/2005 - PLID 16664 - If this appointment is tied to a superbill ID (and not void), give the user another warning.  We
			//	will make them unrelated once the appointment is gone.
			//If we have already gotten bVoidSuperbills set to true, then we don't want to do all this warning
			if(!bVoidSuperbills && !bNoPrompt && (GetRemotePropertyInt("ShowVoidSuperbillPrompt", 1, 0, "<None>", false) == 1)) {
				// (d.thompson 2010-06-04) - PLID 39020 - Parameterized
				_RecordsetPtr prsSuperbill = CreateParamRecordset("SELECT SavedID FROM PrintedSuperbillsT WHERE ReservationID = {INT} AND Void = 0", nReservationID);
				CString strSuperbillIDs = "";
				long nSuperbillIDCnt = 0;
				while(!prsSuperbill->eof) {
					long nID = AdoFldLong(prsSuperbill, "SavedID", -1);
					if(nID > -1) {
						CString str;	str.Format("%li, ", nID);
						strSuperbillIDs += str;
						nSuperbillIDCnt++;
					}

					prsSuperbill->MoveNext();
				}
				strSuperbillIDs.TrimRight(", ");
				prsSuperbill->Close();

				if(nSuperbillIDCnt > 0) {
					//This appointment is tied to superbills, we will warn the user and give them an opportunity to give up
					CString strFmt;
					strFmt.Format("This appointment is tied to %li superbill%s (ID%s:  %s).  "
						"Would you like to mark these superbills VOID?\r\n\r\n"

						" - If you choose YES, the appointment will be deleted and all related superbill IDs will be marked VOID.\r\n"
						" - If you choose NO, the appointment will be deleted, but all related superbill IDs will remain.\r\n"
						" - If you choose CANCEL, the appointment will remain.", nSuperbillIDCnt, nSuperbillIDCnt == 1 ? "" : "s", nSuperbillIDCnt == 1 ? "" : "s", strSuperbillIDs);

					int nRes = AfxMessageBox(strFmt, MB_YESNOCANCEL);
					if(nRes == IDCANCEL)
						return false;
					else if(nRes == IDYES)
						bVoidSuperbills = true;
				}
			}

			// (j.jones 2007-11-07 11:33) - PLID 27987 - handle inventory allocations linked to appts.
			// Check to see if there are any open allocations linked to this appt.
			if(!bReturnAllocations && !bNoPrompt) {
				_RecordsetPtr prsTest = CreateParamRecordset("SELECT TOP 1 ID FROM PatientInvAllocationsT WHERE AppointmentID = {INT} AND Status = {INT}", nReservationID, InvUtils::iasActive);
				if(!prsTest->eof) {
					//there are undeleted allocations, so warn the user
					//(yes, the data structure lets them have more than one, but that's rare - and we may forbid it -
					//so make the message make sense in the 99% of cases where they have only one allocation)
					// (j.jones 2007-11-29 09:57) - PLID 28196 - reworded this to properly explain that only
					// unused products can be returned to stock
					CString strFmt = "This patient has an inventory allocation tied to this appointment.\n"
						"Would you like to return any unused products in this allocation back to purchased stock?\r\n\r\n"

						" - If you choose YES, the appointment will be deleted and any unused allocated products will be returned to stock.\r\n"
						" - If you choose NO, the appointment will be deleted and all allocated products will remain allocated to this patient.\r\n"
						" - If you choose CANCEL, the appointment and allocated products will remain.";

					int nRes = AfxMessageBox(strFmt, MB_YESNOCANCEL);
					if(nRes == IDCANCEL)
						return false;
					else if(nRes == IDYES)
						bReturnAllocations = true;
				}
			}

			// (j.jones 2008-03-18 15:48) - PLID 29309 - warn if the appt. is linked to an order
			// (j.jones 2008-09-24 16:37) - PLID 31493 - changed to check if multiple orders were linked
			if(!bNoPrompt) {
				_RecordsetPtr rsOrders = CreateParamRecordset("SELECT ID FROM OrderAppointmentsT "
					"WHERE AppointmentID = {INT}", nReservationID);
				if(!rsOrders->eof) {
					CString strWarning = "This appointment is linked to an inventory order, are you sure you wish to delete it?";
					if(rsOrders->GetRecordCount() > 1) {
						strWarning = "This appointment is linked to multiple inventory orders, are you sure you wish to delete it?";
					}

					int nRes = MessageBox(GetActiveWindow(), strWarning, "Practice", MB_ICONQUESTION|MB_YESNO);
					if(nRes != IDYES) {
						return false;
					}
				}
				rsOrders->Close();
			}

			// (j.armen 2012-04-10 13:47) - PLID 48299 - Warn that this appointment is linked to a recall
			if(!bNoPrompt) {
				if(ReturnsRecordsParam("SELECT ID FROM RecallT WHERE RecallAppointmentID = {INT}", nReservationID)) {
					if(IDYES == MessageBox(GetActiveWindow(), "This appointment is linked to a recall, are you sure you wish to delete it?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
						return false;
					}
				}
			}

			long lPatientID;
			// (d.thompson 2010-06-04) - PLID 39020 - Parameterized
			_RecordsetPtr prs = CreateParamRecordset("SELECT PatientID, Date, StartTime, EndTime, ShowState, LocationID, dbo.GetResourceIDString(ID) AS ResourceIDs "
				"FROM AppointmentsT WHERE ID = {INT}", nReservationID);
			lPatientID = AdoFldLong(prs, "PatientID");

			CString strPatientName = GetExistingPatientName(lPatientID);
			
			// (d.moore 2007-06-01 12:53) - PLID 4013 - 
			//  Attempt to delete any entries in the waiting list that may be 
			//   associated with this appointment.
			CString strSQLFinal = CWaitingListUtils::BuildWaitingListDeleteQuery(nReservationID);
			
			// (c.haag 2004-03-05 14:28) - Generate our SQL statement
			CString strSQLTmp;
			strSQLTmp.Format("INSERT INTO PalmDeletedT SELECT PalmSettingsTID, RecordID FROM PalmRecordT WHERE AppointmentID = %d", nReservationID);
			strSQLFinal = strSQLFinal + strSQLTmp;
			strSQLTmp.Format("DELETE FROM PalmSyncT WHERE AppointmentID = %d", nReservationID);
			strSQLFinal = strSQLFinal + ";" + strSQLTmp;
			strSQLTmp.Format("DELETE FROM PalmRecordT WHERE AppointmentID = %d", nReservationID);
			strSQLFinal = strSQLFinal + ";" + strSQLTmp;
			strSQLTmp.Format("DELETE FROM AppointmentPurposeT WHERE AppointmentID = %d", nReservationID);
			strSQLFinal = strSQLFinal + ";" + strSQLTmp;
			strSQLTmp.Format("DELETE FROM AppointmentResourceT WHERE AppointmentID = %d", nReservationID);
			strSQLFinal = strSQLFinal + ";" + strSQLTmp;
			strSQLTmp.Format("DELETE FROM AptLinkT WHERE AppointmentID = %d", nReservationID);
			strSQLFinal = strSQLFinal + ";" + strSQLTmp;
			// (j.gruber 2012-08-07 13:28) - PLID 51869 - delete insurance information
			strSQLTmp.Format("DELETE FROM AppointmentInsuredPartyT WHERE AppointmentID = %d", nReservationID);
			strSQLFinal = strSQLFinal + strSQLTmp;
			strSQLTmp.Format("DELETE FROM AptShowStateHistoryT WHERE AppointmentID = %d", nReservationID);
			strSQLFinal = strSQLFinal + ";" + strSQLTmp;
			strSQLTmp.Format("DELETE FROM RoomAppointmentHistoryT WHERE RoomAppointmentID IN (SELECT ID FROM RoomAppointmentsT WHERE AppointmentID = %li)", nReservationID);
			strSQLFinal = strSQLFinal + ";" + strSQLTmp;
			strSQLTmp.Format("DELETE FROM RoomAppointmentsT WHERE AppointmentID = %li", nReservationID);
			strSQLFinal = strSQLFinal + ";" + strSQLTmp;
			// (a.walling 2008-05-23 15:21) - PLID 30158 - Make sure this is handled.
			strSQLTmp.Format("DELETE FROM ProcInfoAppointmentsT WHERE AppointmentID = %li", nReservationID);
			strSQLFinal = strSQLFinal + ";" + strSQLTmp;
			// (j.gruber 2014-12-16 11:03) - PLID 64393 - Rescheduling Queue - In the Appointments tab, if the appointment is deleted, keep the notes on the Patient Note Tab.
			strSQLTmp.Format("UPDATE NoteInfoT SET AppointmentID = NULL WHERE AppointmentID = %li", nReservationID);
			strSQLFinal = strSQLFinal + ";" + strSQLTmp;
			

			// (j.jones 2008-03-18 15:57) - PLID 29309 - detach from orders
			strSQLTmp.Format("DELETE FROM OrderAppointmentsT WHERE AppointmentID = %li", nReservationID);
			strSQLFinal = strSQLFinal + ";" + strSQLTmp;

			// (j.jones 2008-03-19 11:38) - PLID 29316 - audit those detachments
			_RecordsetPtr rsOrderInfo = CreateParamRecordset("SELECT OrderT.ID, OrderT.Description, "
				"Last + ', ' + First + ' ' + Middle AS Name, AppointmentsT.Date, AppointmentsT.StartTime "
				"FROM AppointmentsT "
				"INNER JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID "
				"INNER JOIN OrderAppointmentsT ON AppointmentsT.ID = OrderAppointmentsT.AppointmentID "
				"INNER JOIN OrderT ON OrderAppointmentsT.OrderID = OrderT.ID "
				"WHERE AppointmentsT.ID = {INT}", nReservationID);
			while(!rsOrderInfo->eof) {
				long nOrderID = AdoFldLong(rsOrderInfo, "ID");
				CString strOrderDesc = AdoFldString(rsOrderInfo, "Description","");
				CString strPatientName = AdoFldString(rsOrderInfo, "Name","");
				strPatientName.TrimRight();
				COleDateTime dtApptDate = AdoFldDateTime(rsOrderInfo, "Date");
				COleDateTime dtApptTime = AdoFldDateTime(rsOrderInfo, "StartTime");
				CString strOldApptLinkText;
				strOldApptLinkText.Format("Linked to patient '%s', Appointment Date: %s %s", strPatientName, FormatDateTimeForInterface(dtApptDate, NULL, dtoDate), FormatDateTimeForInterface(dtApptTime, DTF_STRIP_SECONDS, dtoTime));

				CString strOld;
				strOld.Format("Order: %s, %s", strOrderDesc, strOldApptLinkText);

				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}
				AuditEvent(-1, "", nAuditTransactionID, aeiInvOrderApptLinkDeleted, nOrderID, strOld, "<No Linked Appointment>", aepMedium, aetDeleted);

				rsOrderInfo->MoveNext();
			}
			rsOrderInfo->Close();

			// (j.jones 2007-11-07 11:39) - PLID 27987 - if they chose to return allocations, mark only the "active"
			// allocations as deleted (because the details' "returned" status is only used when the allocation is 
			// actually completed, and we shouldn't modify the status of "completed" allocations
			if(bReturnAllocations) {
				// (c.haag 2008-02-29 13:04) - PLID 29115 - Begin an inventory todo transaction
				nInvTodoTransactionID = InvUtils::BeginInventoryTodoAlarmsTransaction();

				// (j.jones 2008-02-27 14:27) - PLID 29102 - remove links from case histories
				strSQLTmp.Format("DELETE FROM CaseHistoryAllocationLinkT WHERE AllocationID IN (SELECT ID FROM PatientInvAllocationsT WHERE AppointmentID = %li)", nReservationID);
				strSQLFinal = strSQLFinal + ";" + strSQLTmp;

				// (j.jones 2008-02-27 14:37) - PLID 29104 - audit those link deletions
				_RecordsetPtr rsCaseHistInfo = CreateParamRecordset("SELECT CaseHistoryT.ID AS CaseHistoryID, CaseHistoryT.Name, "
					"PatientInvAllocationsT.ID AS AllocationID, PatientInvAllocationsT.InputDate "
					"FROM CaseHistoryT "
					"INNER JOIN CaseHistoryAllocationLinkT ON CaseHistoryT.ID = CaseHistoryAllocationLinkT.CaseHistoryID "
					"INNER JOIN PatientInvAllocationsT ON CaseHistoryAllocationLinkT.AllocationID = PatientInvAllocationsT.ID "
					"WHERE PatientInvAllocationsT.AppointmentID = {INT}", nReservationID);
				//with the current design, there should never be more than one linked case history, but the 
				//structure allows it, so let's make it a while loop for accuracy
				while(!rsCaseHistInfo->eof) {
					long nCaseHistoryID = AdoFldLong(rsCaseHistInfo, "CaseHistoryID");
					CString strCaseHistoryName = AdoFldString(rsCaseHistInfo, "Name");
					long nAllocationID = AdoFldLong(rsCaseHistInfo, "AllocationID");
					COleDateTime dtInput = AdoFldDateTime(rsCaseHistInfo, "InputDate");

					CString strAllocationDescription;
					strAllocationDescription.Format("Allocation From %s", FormatDateTimeForInterface(dtInput, NULL, dtoDate));

					//need to audit that we removed the link from both the case history and the allocation
					if(nAuditTransactionID == -1) {
						nAuditTransactionID = BeginAuditTransaction();
					}
					AuditEvent(lPatientID, strPatientName, nAuditTransactionID, aeiCaseHistoryInvAllocationLinkDeleted, nCaseHistoryID, strCaseHistoryName + ", " + strAllocationDescription, "<No Linked Allocation>", aepMedium, aetDeleted);
					AuditEvent(lPatientID, strPatientName, nAuditTransactionID, aeiInvAllocationCaseHistoryLinkDeleted, nAllocationID, strAllocationDescription + ", Case History: " + strCaseHistoryName, "<No Linked Case History>", aepMedium, aetDeleted);
					rsCaseHistInfo->MoveNext();
				}
				rsCaseHistInfo->Close();

				// (c.haag 2008-02-29 13:04) - PLID 29115 - Update the inventory todo transaction
				_RecordsetPtr prsInvTodo = CreateParamRecordset("SELECT ID FROM PatientInvAllocationsT WHERE AppointmentID = {INT}", nReservationID);
				while (!prsInvTodo->eof) {
					InvUtils::AddToInventoryTodoAlarmsTransaction(nInvTodoTransactionID, InvUtils::eInvTrans_AllocationID, AdoFldLong(prsInvTodo, "ID"));
					prsInvTodo->MoveNext();
				} 
				prsInvTodo->Close();

				strSQLTmp.Format("UPDATE PatientInvAllocationDetailsT SET Status = %li WHERE AllocationID IN (SELECT ID FROM PatientInvAllocationsT WHERE Status = %li AND AppointmentID = %li)", InvUtils::iadsDeleted, InvUtils::iasActive, nReservationID);
				strSQLFinal = strSQLFinal + ";" + strSQLTmp;
				strSQLTmp.Format("UPDATE PatientInvAllocationsT SET Status = %li WHERE Status = %li AND AppointmentID = %li", InvUtils::iasDeleted, InvUtils::iasActive, nReservationID);
				strSQLFinal = strSQLFinal + ";" + strSQLTmp;
			}
			// (j.jones 2007-11-07 11:33) - PLID 27987 - always disconnect all allocations from this appt			
			strSQLTmp.Format("UPDATE PatientInvAllocationsT SET AppointmentID = NULL WHERE AppointmentID = %li", nReservationID);
			strSQLFinal = strSQLFinal + ";" + strSQLTmp;

			// (j.jones 2008-06-24 11:52) - PLID 30457 - unlink from charges
			strSQLTmp.Format("UPDATE ChargesT SET AppointmentID = NULL WHERE AppointmentID = %li", nReservationID);
			strSQLFinal = strSQLFinal + ";" + strSQLTmp;

			// (j.jones 2009-12-09 13:22) - PLID 36137 - delete from the TOPS history
			strSQLTmp.Format("DELETE FROM TOPSSubmissionHistoryT WHERE AppointmentID = %li", nReservationID);
			strSQLFinal = strSQLFinal + ";" + strSQLTmp;

			// (z.manning 2010-04-05 09:26) - PLID 24607 - AppointmentRemindersT 
			strSQLFinal += FormatString("DELETE FROM AppointmentRemindersT WHERE AppointmentID = %li", nReservationID);

			// (z.manning 2010-07-14 10:11) - PLID 39422 - Delete any HL7 links to this appointment
			strSQLFinal += FormatString("DELETE FROM HL7CodeLinkT WHERE PracticeID = %li AND Type = %li", nReservationID, hclrtAppointment);

			// (j.jones 2011-10-07 15:52) - PLID 37659 - clear references in EligibilityRequestsT
			strSQLFinal += FormatString("UPDATE EligibilityRequestsT SET AppointmentID = NULL WHERE AppointmentID = %li", nReservationID);

			// (j.armen 2012-04-10 14:09) - PLID 48299 - clear references from the recall table
			strSQLFinal += FormatString("UPDATE RecallT SET AppointmentID = NULL WHERE AppointmentID = %li", nReservationID);
			strSQLFinal += FormatString("UPDATE RecallT SET RecallAppointmentID = NULL WHERE RecallAppointmentID = %li", nReservationID);
			
			// (a.walling 2013-01-17 09:26) - PLID 54651 - Update any EmrMasterT.AppointmentID references to this appt in deleted EMNs
			strSQLFinal += FormatString("UPDATE EmrMasterT SET AppointmentID = NULL WHERE AppointmentID = %li AND Deleted = 1", nReservationID);

			// (j.jones 2014-11-26 16:09) - PLID 64272 - remove entries from AppointmentMixRuleOverridesT
			strSQLFinal += FormatString("DELETE FROM AppointmentMixRuleOverridesT WHERE AppointmentID = %li", nReservationID);

			// (a.walling 2015-02-06 11:38) - PLID 64364 - Rescheduling Queue
			strSQLFinal += FormatString("DELETE FROM ReschedulingQueueT WHERE AppointmentID = %li", nReservationID);
			
			strSQLTmp.Format("DELETE FROM AppointmentsT WHERE ID = %li", nReservationID);
			strSQLFinal = strSQLFinal + ";" + strSQLTmp;
			//DRT 7/8/2005 - PLID 16664
			if(bVoidSuperbills) {
				strSQLTmp.Format("UPDATE PrintedSuperbillsT SET ReservationID = -1, Void = 1, VoidDate = GetDate(), VoidUser = '%s' WHERE ReservationID = %li", 
					_Q(GetCurrentUserName()), nReservationID);
				strSQLFinal = strSQLFinal + ";" + strSQLTmp;
			}

			// (j.jones 2007-11-19 10:15) - PLID 28043 - if we cut and pasted,
			// then we're reassigning the allocation, so we don't need to audit here
			if(!bCutPaste) {
				// (j.jones 2007-11-19 09:39) - PLID 28043 - added allocation auditing
				// we have to audit the appt change for any linked allocations,
				// and then optionally audit the deletion for any linked allocations

				//for auditing, we need to look up the information from the allocation in data
				long nAllocationID = -1;
				_RecordsetPtr rs = CreateParamRecordset("SELECT PatientInvAllocationsT.ID, "
					"CONVERT(datetime, CONVERT(varchar, AppointmentsT.StartTime, 23)) + convert(datetime, RIGHT(CONVERT(varchar, AppointmentsT.StartTime), 7)) AS ApptDateTime "
					"FROM PatientInvAllocationsT "
					"LEFT JOIN AppointmentsT ON PatientInvAllocationsT.AppointmentID = AppointmentsT.ID "
					"WHERE PatientInvAllocationsT.AppointmentID = {INT} AND PatientInvAllocationsT.Status <> {INT}\r\n"
					""
					"SELECT PatientInvAllocationDetailsT.AllocationID, ServiceT.Name AS ProductName, "
					"SerialNum, ExpDate, Quantity "
					"FROM PatientInvAllocationDetailsT "
					"INNER JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID = PatientInvAllocationsT.ID "
					"LEFT JOIN ServiceT ON PatientInvAllocationDetailsT.ProductID = ServiceT.ID "
					"LEFT JOIN ProductItemsT ON PatientInvAllocationDetailsT.ProductItemID = ProductItemsT.ID "
					"WHERE PatientInvAllocationsT.AppointmentID = {INT} "
					"AND PatientInvAllocationsT.Status <> {INT} AND PatientInvAllocationDetailsT.Status <> {INT}", nReservationID, InvUtils::iasDeleted, nReservationID, InvUtils::iasDeleted, InvUtils::iadsDeleted);
				if(!rs->eof) {
					long nAllocationID = AdoFldLong(rs, "ID");
					_variant_t varApptDate = rs->Fields->Item["ApptDateTime"]->Value;					
					if(!bReturnAllocations && varApptDate.vt == VT_DATE) {
						//if not deleting the allocation, audit that the appt. changed
						CString strOldApptDate = FormatDateTimeForInterface(VarDateTime(varApptDate), NULL, dtoDateTime);
						CString strOldValue;
						strOldValue.Format("Appointment: %s", strOldApptDate);
						if(nAuditTransactionID == -1) {
							nAuditTransactionID = BeginAuditTransaction();
						}
						AuditEvent(lPatientID, strPatientName, nAuditTransactionID, aeiInvAllocationAppointment, nAllocationID, strOldValue, "<No Appointment>", aepMedium, aetChanged);
					}
					else if(bReturnAllocations) {
						//otherwise just audit that we are deleting
						//no real description to give it, it's defined by its details
						if(nAuditTransactionID == -1) {
							nAuditTransactionID = BeginAuditTransaction();
						}
						AuditEvent(lPatientID, strPatientName, nAuditTransactionID, aeiInvAllocationDeleted, nAllocationID, "", "<Deleted>", aepMedium, aetDeleted);
					}
				}

				//no need to loop through details if not deleting the allocation completely
				if(bReturnAllocations) {
					rs = rs->NextRecordset(NULL);
					
					//now loop through any undeleted details
					while(!rs->eof) {
						
						CString strDesc;

						CString strProductName = AdoFldString(rs, "ProductName","");
						_variant_t varSerialNumber = rs->Fields->Item["SerialNum"]->Value;
						_variant_t varExpDate = rs->Fields->Item["ExpDate"]->Value;
						double dblQuantity = AdoFldDouble(rs, "Quantity",1.0);
						
						strDesc = strProductName;
						if(varSerialNumber.vt == VT_BSTR) {
							CString str;
							str.Format(", Serial Num: %s", VarString(varSerialNumber));
							strDesc += str;
						}
						if(varExpDate.vt == VT_DATE) {
							CString str;
							str.Format(", Exp. Date: %s", FormatDateTimeForInterface(VarDateTime(varExpDate), NULL, dtoDate));
							strDesc += str;
						}
						if(dblQuantity != 1.0) {
							//only show the quantity if not 1.0
							CString str;
							str.Format(", Quantity: %g", dblQuantity);
							strDesc += str;
						}

						//and now audit using this description
						if(nAuditTransactionID == -1) {
							nAuditTransactionID = BeginAuditTransaction();
						}
						AuditEvent(lPatientID, strPatientName, nAuditTransactionID, aeiInvAllocationDetailDeleted, nAllocationID, strDesc, "<Deleted>", aepMedium, aetDeleted);

						rs->MoveNext();
					}
				}
				rs->Close();
			}

			// Mark the records as deleted
			if (bGenerateSqlOnly)
			{
				if (!pstrSQL)
				{
					ASSERT(FALSE);
					return false;
				}
				*pstrSQL = strSQLFinal;
			}
			else
			{
				// (d.thompson 2010-06-04) - PLID 39020 - Removed. We already looked up the patient ID above as lPatientID!
				//_RecordsetPtr rsPatientID = CreateParamRecordset("SELECT PatientID FROM AppointmentsT WHERE ID = {INT}", nReservationID);
				//if (!rsPatientID->eof)
				{
					//long nPatientID = AdoFldLong(rsPatientID, "PatientID");

					// (a.walling 2015-02-06 11:39) - PLID 64586 - Rescheduling Queue - Send table checker when deleting appt
					bool refreshReschedulingQueue = false;
					if (Nx::Scheduler::IsApptInReschedulingQueue(nReservationID)) {
						refreshReschedulingQueue = true;
					}

					// (z.manning 2008-07-16 14:34) - PLID 30490 - Handle any HL7 messages relating to this appointment
					// (z.manning 2008-08-20 10:05) - Make sure we do this before we delete the appt from data
					// because we need to access data to generate the HL7 message.
					// (r.gonet 12/03/2012) - PLID 54108 - Updated to use refactored function.
					CMainFrame* pMainFrm = GetMainFrame();
					if (nullptr != pMainFrm && pMainFrm->m_arynScheduleHL7GroupIDs.GetCount() > 0)
					{
						// Synchronously send an HL7 message to each group
						boost::scoped_ptr<CHL7Client_Practice> pHL7Client(new CHL7Client_Practice());
						std::vector<HL7ResponsePtr> vecResponses = SendCancelAppointmentHL7Message(pHL7Client.get(), nReservationID, true, "");
						bool bCanDeleteAppointment = true;
						for (auto pResponse : vecResponses)
						{
							// Even though HandleHL7Response is intended for asynchronous calls we should call it to present
							// any necessary messages to the user and perform business logic (that it was never supposed to have
							// but it does anyway)
							pMainFrm->HandleHL7Response(pHL7Client.get(), pResponse);

							// Don't delete the appointment if the result is unexpected
							switch (pResponse->hmssSendStatus)
							{
							case hmssSent:
							case hmssFailure_Batched:
							case hmssBatched:
							case hmssSent_AckReceived:
								// It is safe to delete the appointment if any of these statuses are present
								break;
							default:
								bCanDeleteAppointment = false;
								break;
							}
						}

						// Fail out if we can't delete the appointment
						if (!bCanDeleteAppointment)
						{
							AfxMessageBox("The appointment could not be deleted due to an HL7 export failure. Please try again later.", MB_ICONSTOP);
							return false;
						}
					}

					// TODO: Move this to AFTER it's deleted from Practice
					// Update Microsoft Outlook
					PPCDeleteAppt(nReservationID);
					// (j.jones 2010-04-20 08:50) - PLID 38273 - converted to ExecuteSqlStd
					ExecuteSqlStd(strSQLFinal);

					//Track that this appointment has been deleted.
					if (lPatientID != -25)
					{
						PhaseTracking::UnapplyEvent(lPatientID, PhaseTracking::ET_AppointmentCreated, nReservationID);
						PhaseTracking::UnapplyEvent(lPatientID, PhaseTracking::ET_ActualAppointment, nReservationID);
					}

					// (a.walling 2015-02-06 11:39) - PLID 64586 - Rescheduling Queue - Send table checker when deleting appt
					if (refreshReschedulingQueue) {
						Nx::Scheduler::TableChecker::OnReschedulingQueueRemoved(nReservationID);
					}
				}
				// (d.thompson 2010-06-04) - PLID 39020 - Not necessary with removed lookup
				/*else
				{
					// (c.haag 2004-03-05 15:13) - The appointment was already deleted!!
					if (!bNoPrompt)	MsgBox("The appointment you have tried to delete was already deleted from the system.");
					return true;
				}*/
			}
			
			//auditing
			COleDateTime dtDate, dtStart, dtEnd;
			dtDate = AdoFldDateTime(prs, "Date");
			dtStart = AdoFldDateTime(prs, "StartTime");
			dtEnd = AdoFldDateTime(prs, "EndTime");
			CString strDate;
			strDate = FormatDateTimeForSql(COleDateTime(dtDate.GetYear(), dtDate.GetMonth(), dtDate.GetDay(), dtStart.GetHour(), dtStart.GetMinute(), dtStart.GetSecond()));
			
			if(nAuditTransactionID == -1) {
				nAuditTransactionID = BeginAuditTransaction();
			}			
			AuditEvent(lPatientID, strPatientName, nAuditTransactionID, aeiApptStatus, nReservationID, strDate, "<Deleted>", aepHigh, aetDeleted);

			if (!bGenerateSqlOnly) {
				// (j.jones 2007-09-06 15:19) - PLID 27312 - required the EndTime as a parameter
				// (j.jones 2014-08-05 10:35) - PLID 63167 - added PatientID, LocationID, ResourceIDs
				CClient::RefreshAppointmentTable(nReservationID, lPatientID, dtStart, dtEnd, 4, AdoFldLong(prs, "ShowState"),
					AdoFldLong(prs, "LocationID"), AdoFldString(prs, "ResourceIDs", ""));

				// (z.manning 2015-11-05 12:13) - PLID 57109 - Update recalls needing attention dialog
				GetMainFrame()->HandleRecallChanged();
			}

			if (pAuditID) {
				*pAuditID = nAuditTransactionID;
			}

			//commit our audit
			if(!bGenerateSqlOnly && nAuditTransactionID != -1) {
				CommitAuditTransaction(nAuditTransactionID);
			}

			// (c.haag 2008-02-29 13:02) - PLID 29115 - Update inventory todo alarms
			if (-1 != nInvTodoTransactionID) {
				//TES 11/15/2011 - PLID 44716 - This function needs to know if we're in a transaction
				InvUtils::CommitInventoryTodoAlarmsTransaction(nInvTodoTransactionID, false);
			}

			// Make sure the time stamp is incremented
			//IncrementTimeStamp(RES_TIME_STAMP_NAME);

			// Return success
			return true;
		} NxCatchAllCall("::AppointmentDeleteNoHistory Error 1", {
			if(nAuditTransactionID != -1) {
				RollbackAuditTransaction(nAuditTransactionID);
			}
			// (c.haag 2008-02-29 13:05) - PLID 29115 - Roll back any inventory todo transactions
			if (-1 != nInvTodoTransactionID) {
				InvUtils::RollbackInventoryTodoAlarmsTransaction(nInvTodoTransactionID);
			}
			return false;
		});
	} else {
		// Return failure
		return false;
	}
}

// (j.jones 2010-01-04 11:01) - PLID 32935 - added parameter to disable checking for required allocations
// (j.gruber 2012-07-30 13:23) - PLID 51869 - added insurance information
// (j.gruber 2013-01-08 09:07) - PLID 54483 - added refphysID
long AppointmentCreate(long nPatientID, const CDWordArray& adwResourceID, long nLocationID, const COleDateTime &dtDate, 
							  const COleDateTime &dtStartTime, const COleDateTime &dtEndTime, const COleDateTime &dtArrivalTime, long nConfirmed, 
							  bool bMoveUp, long lNoShow, const CString &strNotes, 
							  long nAptType, const CDWordArray& adwAptPurpose, bool bReady, long nRequestedResourceID /*= -1*/,
							  BOOL bSendTableCheckerMsg /* = TRUE */, bool bCheckForAllocations /*= true*/, const AppointmentInsuranceMap *pmapInsInfo /*=NULL*/, 
							  long nReferringPhysID /*=-1*/)
{
	if (!CheckCurrentUserPermissions(bioAppointment, sptCreate, FALSE, 0, TRUE, TRUE)) {
		// (b.cardillo 2005-04-28 15:06) - PLID 16368 - The current user isn't allowed to create 
		// appointments under any circumstances.  Normally our caller should have checked this 
		// first and caught the problem and notified the user in a friendly way.  But clearly 
		// the caller failed to do this so we have no choice but to throw an exception.  For 
		// debuggers we assert first so the source of the problem can be found more easily.
		ASSERT(FALSE);
		ThrowNxException("AppointmentCreate: The current user does not have the necessary permissions to create appointments!");
	}

	long nNewResID = -1;

	try {		
		// Insert the record into the appointments table

		//Is this appointment actually happening already? (Have they created it as "In" or "Out").
		// (a.walling 2007-02-15 13:12) - PLID 24770 - This is incorrectly checking for "Out" or "No Show". Who knows how long
		// this has been here. For future reference:
		// Pending = 0, In = 1, Out = 2, NoShow = 3
		bool bActualApt = (lNoShow == 1 || lNoShow == 2);
		COleDateTime dtParam = dtDate;	
		
		// (d.moore 2007-05-22 10:53) - PLID 4013 - I removed the MoveUp bit. It is no longer needed. Instead 
		//  queries have been added to copy data into the waiting list tables.
		BEGIN_TRANS("AppointmentCreate") {
			// (d.thompson 2010-05-20) - PLID 38812 - I rewrote all the saving entirely and threw out the old code to be
			//	in a single batch.  Go bhack to sourcesafe version 402 of this file to see the old code if you need it.
			//No major changes, I just made all the saving (a) parameterized and (b) in the same trip to the server.

			//Pre-insertion setup for variable data
			_variant_t varTypeID = g_cvarNull, varPurposeID = g_cvarNull, varReqResourceID = g_cvarNull;
			_variant_t varRefPhysID = g_cvarNull; // (j.gruber 2013-01-08 09:11) - PLID 54483
			if(nAptType != -1) {
				varTypeID = (long)nAptType;
			}
			if(adwAptPurpose.GetSize() > 0) {
				varPurposeID = (long)adwAptPurpose[0];
			}
			if(nRequestedResourceID != -1) {
				varReqResourceID = (long)nRequestedResourceID;
			}
			// (j.gruber 2013-01-08 09:11) - PLID 54483
			if (nReferringPhysID != -1) {
				varRefPhysID = (long)nReferringPhysID;
			}

			CParamSqlBatch sql;
			//(a)  Insert into the AppointmentsT table, the main appt record.
			// (j.armen 2011-07-28 10:42) - PLID 44205 - Added ConfirmedBy field
			// (j.gruber 2013-01-08 09:12) - PLID 54483 - added ReferringPhysID
			sql.Declare("SET NOCOUNT ON; \r\n"
				"DECLARE @Appointment_ID INT; \r\n");
			sql.Add("INSERT INTO AppointmentsT "
				"(PatientID, LocationID, Confirmed, ConfirmedBy, Notes, ShowState, "
				"CreatedDate, CreatedLogin, ModifiedDate, ModifiedLogin, [Date], "
				"StartTime, EndTime, Status, RecordID, AptTypeID, "
				"AptPurposeID, Ready, RequestedResourceID, ArrivalTime, NoShowDate, RefPhysID) "
				"VALUES ({INT}, {INT}, {INT}, {STRING}, {STRING}, {INT}, "	//Through ShowState
				"GetDate(), {STRING}, GetDate(), {STRING}, {STRING}, "		//Through Date
				"{STRING}, {STRING}, {INT}, {INT}, {VT_I4}, "				//Through AptTypeID
				"{VT_I4}, {INT}, {VT_I4}, {STRING}, NULL, "					//Through NoShowDate
				"{VT_I4});  \r\n ",								
				nPatientID, nLocationID, nConfirmed, nConfirmed == acsConfirmed ? GetCurrentUserName() : "", strNotes,  lNoShow, 
				GetCurrentUserName(), GetCurrentUserName(), FormatDateTimeForSql(dtDate, dtoDate), 
				FormatDateTimeForSql(dtDate, dtoDate) + " " + FormatDateTimeForSql(dtStartTime, dtoTime), 
				FormatDateTimeForSql(dtDate, dtoDate) + " " + FormatDateTimeForSql(dtEndTime, dtoTime), 1, -1, 
				varTypeID, varPurposeID, bReady?1:0, varReqResourceID, 
				FormatDateTimeForSql(dtDate, dtoDate) + " " + FormatDateTimeForSql(dtArrivalTime, dtoTime),
				varRefPhysID
				);
			sql.Add("SET @Appointment_ID = @@IDENTITY; \r\n");
			//Minor tweak:  I'm going to always set NoShowDate to NULL, and if it's set, we'll run a second query to update 
			//	the date.  I can't imagine many valid cases where a new appointment is created and already marked as no show.
			if(lNoShow == 3) {
				sql.Add("UPDATE AppointmentsT SET NoShowDate = GetDate() WHERE ID = @Appointment_ID;\r\n");
			}

			//b)  Now insert any appointment purposes
			// (c.haag 2006-05-10 14:14) - PLID 20542 - We now batch the purpose and resource additions. There
			// will probably be minimal benefit most of the time, but it should still be done.
			// (d.thompson 2010-05-20) - PLID 38812 - I spent a good bit of time sampling XML for insertion here, but the
			//	overhead does not seem to be worth it for a small insertion set.  XML seems worthwhile when data gets large.
			for (long i=0; i < adwAptPurpose.GetSize(); i++) {
				sql.Add("INSERT INTO AppointmentPurposeT (AppointmentID, PurposeID) VALUES (@Appointment_ID, {INT});\r\n", adwAptPurpose[i]);
			}

			//c)  Now insert any appointment resources
			// Add to AppointmentResourceT
			for (i=0; i < adwResourceID.GetSize(); i++)	{
				sql.Add("INSERT INTO AppointmentResourceT (AppointmentID, ResourceID) VALUES (@Appointment_ID, {INT});\r\n", adwResourceID[i]);
			}

			// (j.gruber 2012-07-30 13:27) - PLID 51869 - insurance information
			if (pmapInsInfo) {						
				POSITION pos = pmapInsInfo->GetStartPosition();
				InsuranceInfo *pInsInfo;
				long nPlacement;
				while (pos != NULL) {				
					pmapInsInfo->GetNextAssoc( pos, nPlacement, pInsInfo);
					sql.Add("INSERT INTO AppointmentInsuredPartyT(AppointmentID, InsuredPartyID, Placement) VALUES (@Appointment_ID, {INT}, {INT}); \r\n", pInsInfo->nInsuredPartyID, nPlacement);				
				}
			}		

			sql.Declare("SET NOCOUNT OFF; \r\n ");
			sql.Add("SELECT @Appointment_ID AS NewNum; \r\n");
			_RecordsetPtr prsID = sql.CreateRecordsetNoTransaction(GetRemoteData());

			//Pull out the new appointment ID
			nNewResID = AdoFldLong(prsID, "NewNum");

			// (c.haag 2004-04-01 17:39) PLID 11731 - Sends the in patient notification if necessary
			if (lNoShow == 1)
				//TES 6/9/2008 - PLID 23243 - We now pass in the appointment ID, and its resource IDs.
				SendInPatientNotification(nPatientID, nNewResID, adwResourceID);


			// If MoveUp is true, then entries need to be copied into the waiting list tables.
			if (bMoveUp) {
				CString strWaitList = CWaitingListUtils::BuildWaitingListUpdateQuery(
						nNewResID, nPatientID, dtDate, 
						adwAptPurpose, adwResourceID, 
						nAptType, strNotes);
				// (j.jones 2010-04-20 09:55) - PLID 30852 - converted to ExecuteSqlStd
				ExecuteSqlStd(strWaitList);
			}

			// Add to PalmSyncT
			UpdatePalmSyncT(nNewResID);

		} END_TRANS("AppointmentCreate");

		long nAuditID = BeginNewAuditEvent();
		char szDetails[512];
		if(dtStartTime.GetHour() == 0 && dtStartTime.GetMinute() == 0 && dtEndTime.GetHour() == 0 && dtEndTime.GetMinute() == 0)	//event
			sprintf(szDetails, "Event Created for %s", FormatDateTimeForInterface(dtDate, NULL, dtoDate));
		else
			sprintf(szDetails, "Created for %s", FormatDateTimeForInterface(COleDateTime(dtDate.GetYear(), dtDate.GetMonth(), dtDate.GetDay(), dtStartTime.GetHour(), dtStartTime.GetMinute(), dtStartTime.GetSecond()), DTF_STRIP_SECONDS, dtoDateTime));
		AuditEvent(nPatientID, GetExistingPatientName(nPatientID), nAuditID, aeiApptStatus, nNewResID, "", szDetails, aepHigh, aetCreated);
		
		if (bSendTableCheckerMsg) {
			// (j.jones 2007-09-06 15:19) - PLID 27312 - required the EndTime as a parameter
			// (j.jones 2014-08-05 10:35) - PLID 63167 - added PatientID, LocationID, ResourceIDs
			CString strResourceIDs = ArrayAsString(adwResourceID);
			//this is space-delimited
			strResourceIDs.Replace(",", " ");
			CClient::RefreshAppointmentTable(nNewResID, nPatientID, dtStartTime, dtEndTime, 1, lNoShow, nLocationID, strResourceIDs);

			// (z.manning 2008-07-16 14:34) - PLID 30490 - Handle any HL7 messages relating to this appointment
			// Only do this if the caller is sending table checkers.  Otherwise, caller is responsible for
			// sending the appointment to HL7.
			// (r.gonet 12/03/2012) - PLID 54106 - Updated to use refactored function.
			SendNewAppointmentHL7Message(nNewResID, true);
		}

		// Make sure the time stamp is incremented indicating something has changed
		//IncrementTimeStamp(RES_TIME_STAMP_NAME);

		// Update Microsoft Outlook
		PPCAddAppt(nNewResID);

		//Track that this appointment was created.
		if (lNoShow != 3) {
			//TES 6/13/2008 - PLID 28078 - Prompt to create an allocation, if appropriate.
			// (j.jones 2010-01-04 11:03) - PLID 32935 - this can potentially be disabled
			if(bCheckForAllocations) {
				CheckCreateAllocation(nNewResID, nPatientID, nAptType, adwAptPurpose, nLocationID);
			}
			// (a.walling 2007-02-15 13:13) - PLID 24770 - We should not fire an appointment created event if the status is already noshow.
			PhaseTracking::CreateAndApplyEvent(PhaseTracking::ET_AppointmentCreated, nPatientID, COleDateTime::GetCurrentTime(), nNewResID);
		}
		if(bActualApt) {
			COleDateTime dtTmp(dtDate.GetYear(), dtDate.GetMonth(), dtDate.GetDay(), dtStartTime.GetHour(), dtStartTime.GetMinute(), dtStartTime.GetSecond());
			PhaseTracking::CreateAndApplyEvent(PhaseTracking::ET_ActualAppointment, nPatientID, dtTmp, nNewResID);
		}

		// (j.jones 2009-06-26 11:47) - PLID 34734 - if the appt. is marked In, check the room manager preference
		if(lNoShow == 1) {
			long nRoomAssignWhenApptMarkedIn = GetRemotePropertyInt("RoomAssignWhenApptMarkedIn", 0, 0, GetCurrentUserName(), true);

			//do nothing if the appointment is not for today's date
			COleDateTime dtToday = COleDateTime::GetCurrentTime();
			if(dtDate.GetDay() == dtToday.GetDay() && dtDate.GetMonth() == dtToday.GetMonth() && dtDate.GetYear() == dtToday.GetYear()) {

				if(nRoomAssignWhenApptMarkedIn == 1) {

					//prompt to assign to a room

					CRoomSelectorDlg dlg(NULL);
					dlg.m_nAppointmentID = nNewResID;
					dlg.DoModal();

					//the dialog will be responsible for updating data
					//and sending network messages
				}
				else if(nRoomAssignWhenApptMarkedIn == 2) {
					//open room manager
					if(GetMainFrame())
						GetMainFrame()->ShowRoomManager(nNewResID);
				}
				else {
					//do nothing
				}
			}
		}

		// (d.singleton 2014-10-08 16:23) - PLID 62634 - In Preferences > Links Module > NexWeb > General Tab, add a preference to prompt the user with the NexWeb login 
		// information when creating a new appointment, and the patient has not had an appointment within X amount of days. ignore cancelled and no show
		if (GetRemotePropertyInt("NexWebPromptForLoginOnNewAppt", 0, 0, "<None>", true)) {
			long nDays = GetRemotePropertyInt("NexWebPromptForLoginOnNewApptThreshhold", 0, 0, "<None>", true);
			COleDateTimeSpan dtSpan(nDays, 0, 0, 0);
			COleDateTime dtCutOffDate = dtDate - dtSpan;

			// (b.savon 2015-03-12 07:20) - PLID 64668 - Don't prompt for nexweb login info if the user is the built-in account
			if (nPatientID != -25 && !ReturnsRecordsParam("SELECT TOP 1 1 FROM AppointmentsT WHERE PatientID = {INT} AND Date > {STRING} AND Date < {STRING} "
				"AND ShowState <> 3 AND Status <> 4 ", nPatientID, FormatDateTimeForSql(dtCutOffDate), FormatDateTimeForSql(dtDate))) {
				//they dont have any appts within the desired time frame,  so pop up the nexweb login dlg
				CNexWebLoginInfoDlg dlg(nPatientID, NULL);
				dlg.DoModal();
			}
		}

	} NxCatchAllCall("::AppointmentCreate Error 1", nNewResID = -1);
	return nNewResID;
}

//saiAuditItems for auditing only
// (j.jones 2010-01-04 11:01) - PLID 32935 - added parameter to disable checking for required allocations
// (d.thompson 2010-05-21) - PLID 38812 - Made a ton of changes to drastically reduce the number of recordsets involved
//	with this function.
// (j.gruber 2012-07-30 15:24) - PLID 51869 - added insurance information
// (j.gruber 2013-01-08 09:13) - PLID 54483 - added ReferringPhysID
bool AppointmentModify(long nReservationID, long nPatientID, const CDWordArray& adwResourceID, long nLocationID, 
							  const COleDateTime &dtDate, const COleDateTime &dtStartTime, const COleDateTime &dtEndTime, const COleDateTime &dtArrivalTime,
							  long nConfirmed, bool bMoveUp, long lNoShow, const CString &strNotes, 
							  long nAptType, const CDWordArray& adwAptPurpose, SchedAuditItems* saiAuditItems, BOOL bReady,
							  long nRequestedResourceID, bool bCheckForAllocations /*= true*/,
							  const AppointmentInsuranceMap *pmapOrigInsInfo /*=NULL*/,
							  const AppointmentInsuranceMap *pmapInsInfo /*=NULL*/,
							  long nReferringPhysID /*=-1*/)
{
	// (d.thompson 2010-06-04) - PLID 39015 - Move auditing to a transaction model instead of individual trips.  There are 15
	//	audits in this function, much faster to do them in 1 trip.
	try {
			// (c.haag 2010-09-09 08:41) - PLID 40453 - Use a CAuditTransaction object to ensure that if
			// we leave this scope and the transaction was not committed, that it is rolled back.
			CAuditTransaction auditTransaction;
			long nAuditID = auditTransaction.m_nAuditTransactionID;

			bool bCreateNewEvent = false;
			//TES 12/10/2007 - PLID 28313 - For certain modifications, we will want to update Tracking, but not allow
			// it to create new ladders.
			bool bAllowCreateLadder = true;
			CDWordArray dwOld; // Array of old resources for the appointment
			bool bResourcesChanged = false;

			COleDateTime dtNewDate;
			dtNewDate.SetDateTime(dtDate.GetYear(), dtDate.GetMonth(), dtDate.GetDay(), dtStartTime.GetHour(), dtStartTime.GetMinute(), dtStartTime.GetSecond());
			bool bPromptApptLink = false;
			bool bChangeLMTime = false;
			bool bApptChanged = false;

			// (d.moore 2007-07-03 09:14) - PLID 4013 - If the date, time, or resources changed for the
			//  appointment, then check to see if there are matches in the waiting list that may need to be removed.
			bool bPromptAboutWaitingList = false;

			// Update PalmSyncT now because if a resource is removed from this appointment,
			// and a palm user wants to sync the resource, then the palm user will look
			// at this appointment despite the fact it would no longer sync with his
			// resource filters.
			UpdatePalmSyncT((DWORD)nReservationID);

			CString strPatientName = GetExistingPatientName(nPatientID);
			CString strPatientNameForAudit = nPatientID == -25 ? "{No Patient Selected}" : strPatientName;
			long nPatientIDForAudit = nPatientID == -25 ? -1 : nPatientID;

			// (d.moore 2007-05-22 10:57) - PLID 4013 - Changed the MoveUp bit to query the waiting list table.
			// (d.thompson 2010-05-21) - PLID 38812 - Parameterized, appended several other queries into this trip.
			// (j.gruber 2013-01-08 09:14) - PLID 54483 - added refphys
			CParamSqlBatch sql;
			sql.Add("SELECT PatientID, LocationID, Date, StartTime, Endtime, ArrivalTime, "
				"RequestedResourceID, AptShowStateT.Name AS ShowStateName, Confirmed, "
				"CONVERT(bit, CASE WHEN EXISTS (SELECT ID FROM WaitingListT WHERE AppointmentID={INT}) THEN 1 ELSE 0 END) AS MoveUp, "
				"ShowState, AppointmentsT.Notes, AptTypeID, GroupID, Ready, dbo.GetResourceString(AppointmentsT.ID) AS ResourceList, "
				"AptTypeT.Name AS OldTypeName, LocationsT.Name AS OldLocName, AppointmentsT.RefPhysID AS OldRefPhysID, RefPhysPersonT.Last + ', ' + RefPhysPersonT.First + ' ' + RefPhysPersonT.Middle as OldRefPhysName "
				"FROM AppointmentsT "
				"LEFT JOIN AptLinkT ON AptLinkT.AppointmentID = AppointmentsT.ID "
				"LEFT JOIN AptShowStateT ON AppointmentsT.ShowState = AptShowStateT.ID "
				"LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
				"LEFT JOIN LocationsT ON AppointmentsT.LocationID = LocationsT.ID "
				"LEFT JOIN PersonT RefPhysPersonT ON AppointmentsT.RefPhysID = RefPhysPersonT.ID "
				"WHERE AppointmentsT.ID = {INT};\r\n", nReservationID, nReservationID);
			sql.Add("SELECT Name FROM AptTypeT WHERE ID = {INT};\r\n", nAptType);			//For auditing
			sql.Add("SELECT Name FROM LocationsT WHERE ID = {INT};\r\n", nLocationID);		//For auditing
			sql.Add("SELECT Name FROM AptShowStateT WHERE ID = {INT};\r\n", lNoShow);
			sql.Add("SELECT ResourceID FROM AppointmentResourceT WHERE AppointmentID = {INT};\r\n", nReservationID);		//For auditing
			sql.Add("SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle as RefPhysName FROM PersonT WHERE ID = {INT}; \r\n", nReferringPhysID); // (j.gruber 2013-01-08 09:23) - PLID 54483 - for auditing


			_RecordsetPtr rs = sql.CreateRecordset(GetRemoteData());
			FieldsPtr Fields = rs->Fields;
			long nOldPatientID = AdoFldLong(Fields, "PatientID");
			long nOldLocationID = AdoFldLong(Fields, "LocationID");
			CString strOldLocName = AdoFldString(Fields, "OldLocName", "No Location");
			COleDateTime dtOldDate = AdoFldDateTime(Fields, "Date");
			COleDateTime dtOldStartTime = AdoFldDateTime(Fields, "StartTime");
			COleDateTime dtOldEndTime = AdoFldDateTime(Fields, "EndTime");
			COleDateTime dtOldArrivalTime = AdoFldDateTime(Fields, "ArrivalTime");
			long nOldRequestedResourceID = AdoFldLong(Fields, "RequestedResourceID", -1);
			CString strOldShowStateName = AdoFldString(Fields, "ShowStateName");
			long nOldConfirmed = AdoFldLong(Fields, "Confirmed");
			bool bOldMoveUp = AdoFldBool(Fields, "MoveUp")?true:false;
			long nOldShowState = AdoFldLong(Fields, "ShowState");
			CString strOldNotes = AdoFldString(Fields, "Notes");
			long nOldAptTypeID = AdoFldLong(Fields, "AptTypeID", -1);
			CString strOldTypeName = AdoFldString(Fields, "OldTypeName", "No Type");
			long nOldGroupID = AdoFldLong(Fields, "GroupID", -1);
			BOOL bOldReady = AdoFldBool(Fields, "Ready");
			CString strOldResourceList = AdoFldString(Fields, "ResourceList");
			long nOldRefPhysID = AdoFldLong(Fields, "OldRefPhysID", -1);
			CString strOldRefPhysName = AdoFldString(Fields, "OldRefPhysName", "");

			//TODO:  this might need to be moved to the ResEntry::SaveRes function, and send the values of what has changed
			//via parameters to this function
			
			//figure out which items have changed
			CString strOld, strNew;
			bool bothPurposeNull = false, bothTypeNull = false;

			if(nOldPatientID != nPatientID){	//patient
				strOld = nOldPatientID == -25 ? "{No Patient Selected}" : GetExistingPatientName(nOldPatientID);
				strNew = strPatientNameForAudit;

				//This appointment should no longer be applied to the old patient.
				PhaseTracking::UnapplyEvent(nOldPatientID, PhaseTracking::ET_AppointmentCreated, nReservationID);
				PhaseTracking::UnapplyEvent(nOldPatientID, PhaseTracking::ET_ActualAppointment, nReservationID);
				//And it _should_ be applied to the new patient.
				bCreateNewEvent = true;
				bApptChanged = true;
				AuditEvent(nPatientIDForAudit, strPatientName, nAuditID, aeiApptPatient, nReservationID, strOld, strNew, aepMedium);
			}

			// (d.thompson 2010-05-21) - PLID 38812 - Next recordset for type naming
			rs = rs->NextRecordset(NULL);
			if(nOldAptTypeID != nAptType)
			{
				strNew = strOld = "No Type";

				if(nAptType != -1) {
					// (d.thompson 2010-05-21) - PLID 38812 - Embedded this query into our trip above.
					if(!rs->eof) {
						strNew = AdoFldString(rs, "Name", "No Type");
					}
				}

				if(nOldAptTypeID != -1) {
					// (d.thompson 2010-05-21) - PLID 38812 - Pull from query above, don't query again.
					strOld = strOldTypeName;
				}
				if(strNew != strOld)
				{
					bCreateNewEvent = true;
					//TES 12/10/2007 - PLID 28313 - Don't create a new ladder if they just changed the type of an 
					// existing appointment.
					bAllowCreateLadder = false;
				}

				bApptChanged = true;
				AuditEvent(nPatientIDForAudit, strPatientName, nAuditID, aeiApptType, nReservationID, strOld, strNew, aepMedium);
			}
			
			// (d.thompson 2010-05-21) - PLID 38812 - Next recordset for location naming
			rs = rs->NextRecordset(NULL);
			if(nOldLocationID != nLocationID){	//location

				strOld = strNew = "No Location";
				// (d.thompson 2010-05-21) - PLID 38812 - We can just get this out of the query we ran above.
				strOld = strOldLocName;

				if(!rs->eof) {
					// (d.thompson 2010-05-21) - PLID 38812 - Embed into trip we did above
					strNew = AdoFldString(rs, "Name", "No Location");
				}

				bApptChanged = true;
				AuditEvent(nPatientIDForAudit, strPatientName, nAuditID, aeiApptLocation, nReservationID, strOld, strNew, aepMedium);
			}

			CString strOldDate = FormatDateTimeForSql(dtOldDate, dtoDate);
			CString strNewDate = FormatDateTimeForSql(dtDate, dtoDate);
			bool bVoidSuperbills = false;
			CDWordArray arySuperbills;
			if(strOldDate != strNewDate) {	//date
				bApptChanged = true;
				bPromptAboutWaitingList = true;

				// (z.manning, 03/20/2007) - PLID 25238 - We audit the date when we audit start, end, and arrival times,
				// so this is redundant.
				//AuditEvent(strPatientName, nAuditID, aeiApptDate, nReservationID, strOldDate, strNewDate, aepHigh);

				// If this appointment is a member of a group, bring up the
				// link action window
				if (nOldGroupID != -1)
				{
					//DRT 6/27/03 - We can't call this here, b/c it hasn't been saved yet.  If we do, then they make a todo alarm, and it tells them the wrong date/time.
					//Set a boolean for it, and we'll call it once the same is complete.
					bPromptApptLink = true;
				}

				//DRT 7/8/2005 - PLID 16664 - If there are superbills tied to this appointment, give the user the opportunity to mark them VOID.
				//	Only if the date changed, not time.
				if((GetRemotePropertyInt("ShowVoidSuperbillPrompt", 1, 0, "<None>", false) == 1) && GetCurrentUserPermissions(bioVoidSuperbills) & sptWrite) 
				{
					// (d.thompson 2010-05-21) - PLID 38812 - Parameterized this.  I did not batch it because it is only executed if all of 
					//	3 different requirements are met, so it will not be needed during many saves.
					_RecordsetPtr prsSuperbill = CreateParamRecordset("SELECT SavedID FROM PrintedSuperbillsT WHERE ReservationID = {INT} AND Void = 0", nReservationID);
					CString strSuperbillIDs = "";
					long nSuperbillIDCnt = 0;
					while(!prsSuperbill->eof) {
						long nID = AdoFldLong(prsSuperbill, "SavedID", -1);
						if(nID > -1) {
							CString str;	str.Format("%li, ", nID);
							strSuperbillIDs += str;
							arySuperbills.Add(nID);
							nSuperbillIDCnt++;
						}

						prsSuperbill->MoveNext();
					}
					strSuperbillIDs.TrimRight(", ");
					prsSuperbill->Close();

					if(nSuperbillIDCnt > 0) {
						//They are tied to superbills, we will warn the user and give them an opportunity to give up
						CString strFmt;
						strFmt.Format("This appointment is tied to %li superbill%s (ID%s:  %s).  "
							"Do you wish to mark these superbills as VOID?\r\n\r\n"
							" - If you choose YES, all related superbill IDs will be marked VOID.\r\n"
							" - If you choose NO, the superbills will remain tied to this appointment.", 
							nSuperbillIDCnt, nSuperbillIDCnt == 1 ? "" : "s", nSuperbillIDCnt == 1 ? "" : "s", strSuperbillIDs);

						if(AfxMessageBox(strFmt, MB_YESNO) == IDYES) {
							//void these superbills
							bVoidSuperbills = true;
						}
					}
				}
			}

			// (z.manning, 03/20/2007) - PLID 25238 - Include date and time when audting to be consistent with the rest of the program.
			// (a.walling 2008-04-07 09:54) - PLID 29449 - Also include the old date since that is now consolidated into the starttime/endtime. 
			// Otherwise if only the date changed it would not be audited!
			CString strOldStart = FormatDateTimeForInterface(COleDateTime(dtOldDate.GetYear(), dtOldDate.GetMonth(), dtOldDate.GetDay(), dtOldStartTime.GetHour(), dtOldStartTime.GetMinute(), dtOldStartTime.GetSecond()));
			CString strNewStart = FormatDateTimeForInterface(COleDateTime(dtDate.GetYear(), dtDate.GetMonth(), dtDate.GetDay(), dtStartTime.GetHour(), dtStartTime.GetMinute(), dtStartTime.GetSecond()));
			if(strOldStart != strNewStart) {	//start time
				bApptChanged = true;
				bPromptAboutWaitingList = true;
				AuditEvent(nPatientIDForAudit, strPatientName, nAuditID, aeiApptStartTime, nReservationID, strOldStart, strNewStart, aepHigh);
			}

			// (a.walling 2008-04-07 09:54) - PLID 29449 - Also include the old date since that is now consolidated into the starttime/endtime. 
			// Otherwise if only the date changed it would not be audited!
			CString strOldEnd = FormatDateTimeForInterface(COleDateTime(dtOldDate.GetYear(), dtOldDate.GetMonth(), dtOldDate.GetDay(), dtOldEndTime.GetHour(), dtOldEndTime.GetMinute(), dtOldEndTime.GetSecond()));
			CString strNewEnd = FormatDateTimeForInterface(COleDateTime(dtDate.GetYear(), dtDate.GetMonth(), dtDate.GetDay(), dtEndTime.GetHour(), dtEndTime.GetMinute(), dtEndTime.GetSecond()));
			if(strOldEnd != strNewEnd) {	//end time
				bApptChanged = true;
				AuditEvent(nPatientIDForAudit, strPatientName, nAuditID, aeiApptEndTime, nReservationID, strOldEnd, strNewEnd, aepHigh);
			}

			// (a.walling 2008-04-07 09:54) - PLID 29449 - Also include the old date since that is now consolidated into the starttime/endtime. 
			// Otherwise if only the date changed it would not be audited!
			CString strOldArrival = FormatDateTimeForInterface(COleDateTime(dtOldDate.GetYear(), dtOldDate.GetMonth(), dtOldDate.GetDay(), dtOldArrivalTime.GetHour(), dtOldArrivalTime.GetMinute(), dtOldArrivalTime.GetSecond()));
			CString strNewArrival = FormatDateTimeForInterface(COleDateTime(dtDate.GetYear(), dtDate.GetMonth(), dtDate.GetDay(), dtArrivalTime.GetHour(), dtArrivalTime.GetMinute(), dtArrivalTime.GetSecond()));
			if(strOldArrival != strNewArrival) {	// arrival time
				bApptChanged = true;
				AuditEvent(nPatientIDForAudit, strPatientName, nAuditID, aeiApptArrivalTime, nReservationID, strOldArrival, strNewArrival, aepMedium);
			}

			//(e.lally 2005-06-09) PLID 15285 - Order of No, Yes, LM was incorrect.
				//Make value "Unconfirmed", "Confirmed", or "Left Message" for consistency
			if (nOldConfirmed != nConfirmed)
			{
				switch (nOldConfirmed)
				{
				case 0: strOld = "Unconfirmed"; break;
				case 1: strOld = "Confirmed"; break;
				case 2: strOld = "Left Message"; break;
				default: strOld = "Unknown"; break;
				}
				switch (nConfirmed)
				{
				case 0: strNew = "Unconfirmed";	break;
				case 1: strNew = "Confirmed"; break;
				case 2: strNew = "Left Message"; bChangeLMTime = true; break;
				default: strNew = "Unknown"; break;
				}
				bApptChanged = true;
				AuditEvent(nPatientIDForAudit, strPatientName, nAuditID, aeiApptConfirm, nReservationID, strOld, strNew, aepMedium);
			}

			if(bOldMoveUp != bMoveUp){	//moveup
				if(bOldMoveUp){
					strOld.Format("Yes");
					strNew.Format("No");
				} else {
					strOld.Format("No");
					strNew.Format("Yes");
				}
				bApptChanged = true;
				AuditEvent(nPatientIDForAudit, strPatientName, nAuditID, aeiApptMoveUp, nReservationID, strOld, strNew, aepMedium);
			}

			if(bOldReady != bReady ){	//moveup
				if(bOldReady){
					strOld.Format("Yes");
					strNew.Format("No");
				} else {
					strOld.Format("No");
					strNew.Format("Yes");
				}
				bApptChanged = true;
				AuditEvent(nPatientIDForAudit, strPatientName, nAuditID, aeiApptReady, nReservationID, strOld, strNew, aepMedium);
			}

			// (d.thompson 2010-05-21) - PLID 38812 - Move recordset forward for lookup
			rs = rs->NextRecordset(NULL);
			if(nOldShowState != lNoShow){	//noshow

				strOld = strOldShowStateName;

				COleDateTime dtStartTime(dtDate.GetYear(), dtDate.GetMonth(), dtDate.GetDay(), dtStartTime.GetHour(), dtStartTime.GetMinute(), dtStartTime.GetSecond());
				if (!rs->eof) {
					// (d.thompson 2010-05-21) - PLID 38812 - Pull from audit-lookup query at the top
					strNew = AdoFldString(rs, "Name", "");
				}
				else
					strNew = "Unknown";
				switch (lNoShow) {
				case 1: {

					// For Dr. Zeff: If this is marked as in, and we want to notify
					// everyone when this happens, do it.
					// (c.haag 2003-08-08 12:08) - Now we do this all the time; the preference is
					// now a matter of if you want to see them or not.
//					if (GetPropertyInt("NotifyAllWhenPatientMarkedInGbl", 0))
//					{
						//TES 6/9/2008 - PLID 23243 - We now pass in the appointment ID, and its resource IDs.
						SendInPatientNotification(nPatientID, nReservationID, adwResourceID);
//					}

					// (j.jones 2006-10-05 09:42) - PLID 22792 - see if the user wishes to be prompted about a Room
					long nRoomAssignWhenApptMarkedIn = GetRemotePropertyInt("RoomAssignWhenApptMarkedIn", 0, 0, GetCurrentUserName(), true);

					//do nothing if the appointment is currently in a room or has already been checked out,
					//or is not for today's date
					// (d.thompson 2010-05-21) - PLID 38812 - Parameterized the lookup on room, but there's no reason to 
					//	query to see if the appt is for another day... we already know that from the 'dtDate' argument to
					//	this function
					_RecordsetPtr prsTest = CreateParamRecordset("SELECT ID FROM RoomAppointmentsT WHERE AppointmentID = {INT};\r\n", nReservationID);
					if(prsTest->eof) {
						//Our appt is not already in a room
						COleDateTime dtNow = COleDateTime::GetCurrentTime();
						if(dtDate.GetYear() == dtNow.GetYear() && dtDate.GetMonth() == dtNow.GetMonth() && dtDate.GetDay() == dtNow.GetDay()) {
							//The appointment is for today...
							if(nRoomAssignWhenApptMarkedIn == 1) {

								//prompt to assign to a room

								CRoomSelectorDlg dlg(NULL);
								dlg.m_nAppointmentID = nReservationID;
								dlg.DoModal();

								//the dialog will be responsible for updating data
								//and sending network messages
							}
							else if(nRoomAssignWhenApptMarkedIn == 2) {
								//open room manager
								if(GetMainFrame())
									GetMainFrame()->ShowRoomManager(nReservationID);
							}
							else {
								//do nothing
							}
						}
					}

					//Don't create the event now, just set the flag (bCreateNewEvent).
					//PhaseTracking::CreateAndApplyEvent(PhaseTracking::ET_ActualAppointment, nPatientID, dtStartTime, nReservationID);
					bCreateNewEvent = true;
					}
					break;
				case 2: {

					// (j.jones 2006-10-05 09:42) - PLID 22792 - see if the user wishes to be prompted about a Room
					long nRoomClearWhenApptMarkedOut = GetRemotePropertyInt("RoomClearWhenApptMarkedOut", 0, 0, GetCurrentUserName(), true);

					// (j.jones 2008-05-30 10:02) - PLID 27797 - tweaked the wording of these messages to account for the 'Ready To Check Out' status

					//do nothing if the appointment is not in the room manager, or is checked out
					// (j.jones 2008-05-30 10:06) - PLID 27797 - included more information to eliminate
					// a redundant audit recordset
					// (d.thompson 2010-05-21) - PLID 38812 - Run the query here and look for EOF, no sense running it twice
					// (j.jones 2010-12-02 16:04) - PLID 38597 - supported Waiting Rooms
					_RecordsetPtr rs = CreateParamRecordset("SELECT RoomAppointmentsT.ID, RoomsT.Name, RoomsT.WaitingRoom, "
						"RoomAppointmentsT.StatusID, AppointmentsT.PatientID, RoomStatusT.Name AS RoomStatusName "
						"FROM RoomAppointmentsT "
						"INNER JOIN RoomsT ON RoomAppointmentsT.RoomID = RoomsT.ID "
						"INNER JOIN AppointmentsT ON RoomAppointmentsT.AppointmentID = AppointmentsT.ID "
						"INNER JOIN RoomStatusT ON RoomAppointmentsT.StatusID = RoomStatusT.ID "
						"WHERE RoomAppointmentsT.AppointmentID = {INT} "
						"AND RoomAppointmentsT.StatusID <> -1 ORDER BY RoomAppointmentsT.CheckInTime", nReservationID);
					if(!rs->eof) {
						long nPatientID = AdoFldLong(rs, "PatientID",-1);
						long nRoomAppointmentID = AdoFldLong(rs, "ID");
						CString strRoomName = AdoFldString(rs, "Name", "");
						long nStatusID = AdoFldLong(rs, "StatusID", -1);
						CString strOldStatus = AdoFldString(rs, "RoomStatusName","");
						BOOL bWaitingRoom = AdoFldBool(rs, "WaitingRoom", FALSE);

						CString str;
						if(nStatusID == 0) { //Ready To Check Out
							str.Format("This appointment is currently marked 'Ready To Check Out' in the Room Manager.\n"
								"Would you like to check the appointment out?");
						}
						else {
							str.Format("This appointment is currently assigned to a room.\n"
								"Would you like to check the appointment out of '%s'?", strRoomName);
						}

						// (j.jones 2010-12-02 15:46) - PLID 38597 - if a waiting room, auto-check out silently,
						// else check the preference
						if(bWaitingRoom ||
							(nRoomClearWhenApptMarkedOut == 1 && IDYES == MessageBox(GetActiveWindow(), str, "Practice", MB_ICONQUESTION|MB_YESNO))) {

							//audit this
							// (a.walling 2013-02-08 12:59) - PLID 55083 - Parameterize
							CParamSqlBatch batch;

							//update the status		
							batch.Add("UPDATE RoomAppointmentsT SET StatusID = -1, LastUpdateTime = GetDate(), LastUpdateUserID = {INT} WHERE ID = {INT}", GetCurrentUserID(), nRoomAppointmentID);
							
							//and log this in the history
							// (a.walling 2013-06-07 09:46) - PLID 57078 - RoomAppointmentHistoryT no longer has an ID column
							batch.Add("INSERT INTO RoomAppointmentHistoryT (RoomAppointmentID, UpdateUserID, StatusID) "
								"VALUES ({INT}, {INT}, -1)", nRoomAppointmentID, GetCurrentUserID());

							batch.Execute(GetRemoteData());

							//now audit the change
							{
								CString strOldValue, strNewValue;

								strOldValue.Format("Room: '%s', Status: '%s'", strRoomName, strOldStatus);
								strNewValue = "Checked Out";

								long nAuditID = BeginNewAuditEvent();
								AuditEvent(nPatientID == -25 ? -1 : nPatientID, nPatientID == -25 ? "" : GetExistingPatientName(nPatientID), nAuditID, aeiRoomApptCheckout, nReservationID, strOldValue, strNewValue, aepMedium, aetChanged);
							}

							//send a network message
							CClient::RefreshRoomAppointmentTable(nRoomAppointmentID);
						}
						else if(nRoomClearWhenApptMarkedOut == 2) {
							//open room manager
							if(GetMainFrame())
								GetMainFrame()->ShowRoomManager();
						}
						else {
							//do nothing
						}
					}
					rs->Close();

					//Don't create the event now, just set the flag (bCreateNewEvent).
					//PhaseTracking::CreateAndApplyEvent(PhaseTracking::ET_ActualAppointment, nPatientID, dtStartTime, nReservationID);
					bCreateNewEvent = true;
					}
					break;
				case 3:
					{
					// no show!
					// (a.walling 2006-10-24 09:35) - PLID 23195 - Uncomplete tracking steps when marking no show
					PhaseTracking::UnapplyEvent(nPatientID, PhaseTracking::ET_AppointmentCreated, nReservationID);
					PhaseTracking::UnapplyEvent(nPatientID, PhaseTracking::ET_ActualAppointment, nReservationID);
					}
					break;
				default:
					bCreateNewEvent = true;
					break;
				}

				//DRT 1/24/2005 - tom made changes above that removed calls to PhaseTracking::CreateAndApplyEvent... it turns out that
				//	for a long time we've been calling it there, and then if strOld == strNew, calling it again below.
				//However, if you notice, we can only get into this code if nOldShowState != lNoShow... so strOld will always be
				//	different than strNew.  Removing that check, it's slightly confusing given the above changes to the PhaseTracking..
				//	calls.
				//if(strOld != strNew)
				// (a.walling 2006-11-01 08:59) - PLID 23195 - I don't want this set every time I come through here, so I am moving
				//	the statement to the various cases above.
				//	bCreateNewEvent = true;
				bApptChanged = true;
				AuditEvent(nPatientIDForAudit, strPatientName, nAuditID, aeiApptShowState, nReservationID, strOld, strNew, aepMedium);
			}
			
			if(strOldNotes == " ")
				strOldNotes = "";
			if(strOldNotes != strNotes){	//notes
				bApptChanged = true;
				AuditEvent(nPatientIDForAudit, strPatientName, nAuditID, aeiApptNotes, nReservationID, strOldNotes, strNotes, aepMedium);
			}


			// For appointment purpose auditing
			CString strPurposeOld, strPurposeNew;
			//generate a string for strPurposeOld and strPurposeNew
			CStringArray aryTemp;
			aryTemp.Copy(saiAuditItems->aryNewPurpose);
			for(int j = 0; j < aryTemp.GetSize(); j++) {
				strPurposeNew += aryTemp[j] + ", ";
			}
			strPurposeNew.TrimRight(", ");

			aryTemp.Copy(saiAuditItems->aryOldPurpose);
			for(j = 0; j < aryTemp.GetSize(); j++) {
				strPurposeOld += aryTemp[j] + ", ";
			}
			strPurposeOld.TrimRight(", ");
			//TES 6/15/2004: Why would you say that strPurposeNew being empty meant the appointment hadn't changed?!
			if(strPurposeOld != strPurposeNew /*&& strPurposeNew.GetLength() > 0*/)
				bApptChanged = true;
			//

			// For appointment resource auditing

			//DRT 7/7/03 - Added this for new auditing scheme for resources!  We pull this from data and compare against the new ones
			//		(adwResourceID), instead of bothering with that saiAuditItems setup)
			// (d.thompson 2010-05-21) - PLID 38812 - Move to next recordset from trip at beginning of function
			_RecordsetPtr prsResources = rs->NextRecordset(NULL);

			CDWordArray adwOldTmp;
			while(!prsResources->eof) {
				dwOld.Add(AdoFldLong(prsResources, "ResourceID"));
				adwOldTmp.Add(AdoFldLong(prsResources, "ResourceID"));
				prsResources->MoveNext();
			}
			prsResources->Close();
			if(adwResourceID.GetSize() != adwOldTmp.GetSize()) {
				bApptChanged = true;
				bPromptAboutWaitingList = true;
			}
			else
			{
				for (int i=0; i < adwResourceID.GetSize(); i++)
				{
					bool bFound = false;
					for(int j = 0; j < adwOldTmp.GetSize() && !bFound; j++) {
						if(adwResourceID[i] == adwOldTmp.GetAt(j)) {
							adwOldTmp.RemoveAt(j);	//remove from the list so we don't keep looking at it
							bFound = true;	//yes, we do need to audit!
						}
					}
					if(!bFound) {
						bApptChanged = true;
						bPromptAboutWaitingList = true;
					}
				}
			}
			if (nRequestedResourceID != nOldRequestedResourceID)
				bApptChanged = true;

			// (j.gruber 2013-01-08 09:18) - PLID 54483 - refphys
			_RecordsetPtr prsNewRefPhys = rs->NextRecordset(NULL);
			CString strRefPhysName = "";
			if (!prsNewRefPhys->eof) {
				strRefPhysName = AdoFldString(prsNewRefPhys->Fields, "RefPhysName", "");			
			}
			if (nOldRefPhysID != nReferringPhysID) {				
				bApptChanged = true;
				AuditEvent(nPatientIDForAudit, strPatientName, nAuditID, aeiApptRefPhys, nReservationID, strOldRefPhysName, strRefPhysName, aepMedium);
			}


			// (j.gruber 2012-07-31 13:48) - PLID 51869
			BOOL bInsuranceChanged = FALSE;
			if (pmapOrigInsInfo && pmapInsInfo) {				
				POSITION pos = pmapInsInfo->GetStartPosition();
				InsuranceInfo *pInsInfo;
				long nPlacement;				
				if (pmapOrigInsInfo->GetSize() != pmapInsInfo->GetSize()) {
					bInsuranceChanged = TRUE;
				}
				while (pos != NULL) {				
					pmapInsInfo->GetNextAssoc( pos, nPlacement, pInsInfo);					
					if (!bInsuranceChanged) {
						//get the same placement out of the original to compare
						InsuranceInfo *pOrig;
						if (pmapOrigInsInfo->Lookup(nPlacement, pOrig)) {
							if (pOrig->nInsuredPartyID != pInsInfo->nInsuredPartyID) {
								//it changed
								bInsuranceChanged = TRUE;
							}
						}
						else {
							bInsuranceChanged = TRUE;
						}
					}
				}				
			}

			if (bInsuranceChanged) {
				bApptChanged = true;
			}

			if (!bApptChanged)
				return true; // Return because nothing changed, but return true because we did not fail an attempted save

			//for auditing resources
			bool bNeedToAudit = false;	//do we need to audit anything?

			// (c.haag 2013-12-03) - PLID 59886 - True if we need to do this after the batch is done
			bool bNeedToUpdateProcInfoSurgeon = false;

			// (d.moore 2007-07-03 09:14) - PLID 4013 - If the date, time, or resources changed for the
			//  appointment, then check to see if there are matches in the waiting list that may need to be removed.
			// (d.thompson 2010-05-28) - PLID 38812 - Moved outside the transaction -- this function can prompt with
			//	a modal dialog!
			if (bPromptAboutWaitingList && !bMoveUp && !bOldMoveUp) {
				CheckAppointmentAgainstWaitingList(nReservationID, nPatientID, adwResourceID, dtDate, dtStartTime);
			}

			// Now try creating an update query with all the values
			BEGIN_TRANS("AppointmentModify") {
				//You'll remember that dtNewDate is dtDate and dtStartTime, crammed together.
				// (d.thompson 2010-05-28) - PLID 38812 - Moved the whole set of queries into a param batch.
				CParamSqlBatch sql;

				_variant_t varAptTypeID = g_cvarNull, varAptPurposeID = g_cvarNull, varReqResourceID = g_cvarNull;
				_variant_t varRefPhysID = g_cvarNull; // (j.gruber 2013-01-08 09:20) - PLID 54483
				if(nAptType != -1) {
					varAptTypeID = (long)nAptType;
				}
				if(adwAptPurpose.GetSize() > 0) {
					varAptPurposeID = (long)adwAptPurpose[0];
				}
				if(nRequestedResourceID != -1) {
					varReqResourceID = (long)nRequestedResourceID;
				}
				if (nReferringPhysID != -1) {
					varRefPhysID = (long)nReferringPhysID;
				}

				// (d.moore 2007-05-22 17:10) - PLID 4013 - Removed reference to MoveUp. Waiting list behavior
				//   replaces this.
				//(e.lally 2009-09-15) PLID 15841 - Modified date should be server's system time.
				// (j.armen 2011-07-01 15:41) - PLID 44205 - Added field for confirmed by
				// (j.gruber 2013-01-08 09:20) - PLID 54483 - add ref phys
				sql.Add(
					"UPDATE AppointmentsT "
						"SET PatientID = {INT}, LocationID = {INT}, "
						"Date = {STRING}, StartTime = {STRING}, EndTime = {STRING}, Status = 1, "
						"Confirmed = {INT}, ConfirmedBy = {STRING}, ShowState = {INT}, Notes = {STRING}, ModifiedDate = GetDate(), ModifiedLogin = {STRING}, "
						"AptTypeID = {VT_I4}, AptPurposeID = {VT_I4}, Ready = {INT}, RequestedResourceID = {VT_I4}, ArrivalTime = {STRING}, "
						"RefPhysID = {VT_I4} "
						"WHERE ID = {INT}; \r\n", 
					nPatientID, nLocationID, FormatDateTimeForSql(dtDate, dtoDate), FormatDateTimeForSql(dtNewDate, dtoDateTime), FormatDateTimeForSql(COleDateTime(dtDate.GetYear(), dtDate.GetMonth(), dtDate.GetDay(), dtEndTime.GetHour(), dtEndTime.GetMinute(), dtEndTime.GetSecond()), dtoDateTime), 
					nConfirmed, nConfirmed == acsConfirmed ? GetCurrentUserName() : "", lNoShow, strNotes, GetCurrentUserName(), 
					varAptTypeID, varAptPurposeID, bReady?1:0, varReqResourceID,
					FormatDateTimeForSql(COleDateTime(dtDate.GetYear(), dtDate.GetMonth(), dtDate.GetDay(), dtArrivalTime.GetHour(), dtArrivalTime.GetMinute(), dtArrivalTime.GetSecond()), dtoDateTime),
					varRefPhysID,
					nReservationID);

				// (c.haag 2010-04-01 15:10) - PLID 38005 - Delete from the appointment reminder table. We have to
				// cast the dates and times accordingly
				{
					COleDateTime dtOld_DateTime, dtNew_DateTime;
					dtOld_DateTime.SetDateTime(dtOldDate.GetYear(), dtOldDate.GetMonth(), dtOldDate.GetDay(),
						dtOldStartTime.GetHour(), dtOldStartTime.GetMinute(), dtOldStartTime.GetSecond());
					dtNew_DateTime.SetDateTime(dtDate.GetYear(), dtDate.GetMonth(), dtDate.GetDay(),
						dtStartTime.GetHour(), dtStartTime.GetMinute(), dtStartTime.GetSecond());
					if (dtOld_DateTime != dtNew_DateTime) {
						// (d.thompson 2010-05-28) - PLID 38812 - Param batched
						sql.Add("DELETE FROM AppointmentRemindersT WHERE AppointmentID = {INT};\r\n", nReservationID);
					}
				}
				
				// (d.thompson 2010-05-28) - PLID 38812 - Param batched with the main update
				if (lNoShow == 3) {
					sql.Add("UPDATE AppointmentsT SET NoShowDate = GetDate() WHERE ID = {INT};\r\n", nReservationID);
				}
				if (bChangeLMTime) {
					sql.Add("UPDATE AppointmentsT SET LastLM = GetDate() WHERE ID = {INT};\r\n", nReservationID);
				}

				// Add to AppointmentPurposeT
				// (d.thompson 2010-05-28) - PLID 38812 - Param batched, but I left the "delete then re-insert" functionality for now.
				sql.Add("DELETE FROM AppointmentPurposeT WHERE AppointmentID = {INT};\r\n", nReservationID);
				for (long i=0; i < adwAptPurpose.GetSize(); i++)
				{
					sql.Add("INSERT INTO AppointmentPurposeT (AppointmentID, PurposeID) VALUES ({INT}, {INT});\r\n",
						nReservationID, adwAptPurpose[i]);
				}

				//for auditing
				saiAuditItems->aryNewPurpose.RemoveAll();	//clear it out for the next time around
				saiAuditItems->aryOldPurpose.RemoveAll();	//clear it out for the next time around
				//

				//if new and old purposes differ, audit it
				//TES 6/15/2004: Why would you say that strPurposeNew being empty meant the appointment hadn't changed?!
				if(strPurposeOld != strPurposeNew /*&& strPurposeNew.GetLength() > 0*/) {
					bCreateNewEvent = true;
					//TES 12/10/2007 - PLID 28313 - Don't create a new ladder if they've just changed the purpose
					// of an existing appointment.
					bAllowCreateLadder = false;
					// (b.cardillo 2005-11-18 18:00) - PLID 18407 - In accordance with best 
					// practices for auditing, we now send nReservationID in as the RecordID 
					// instead of nPatientID.  The audit trail report has always expected it to 
					// be an appointment id, so now we're coming in line with what it expects.
					AuditEvent(nPatientID, GetExistingPatientName(nPatientID), nAuditID, aeiApptPurpose, nReservationID, strPurposeOld, strPurposeNew, aepMedium, aetChanged);
				}
				//done auditing purposes

				//first, if there are different # of resources, we definitely need to audit
				if(adwResourceID.GetSize() != dwOld.GetSize())
					bNeedToAudit = true;

				// Add to AppointmentResourceT
				// (d.thompson 2010-05-28) - PLID 38812 - Param batched, but left the "delete then re-insert" functionality for now
				sql.Add("DELETE FROM AppointmentResourceT WHERE AppointmentID = {INT};\r\n", nReservationID);
				for (i=0; i < adwResourceID.GetSize(); i++)
				{
					sql.Add("INSERT INTO AppointmentResourceT (AppointmentID, ResourceID) VALUES ({INT}, {INT});\r\n",
						nReservationID, adwResourceID[i]);

					//DRT 7/7/03 - While we're looping through all the new resources, see if it was one of our old ones - 
					//		if not, then mark it for auditing
					bool bFound = false;
					for(int j = 0; j < dwOld.GetSize() && !bNeedToAudit; j++) {
						if(adwResourceID[i] == dwOld.GetAt(j)) {
							dwOld.RemoveAt(j);	//remove from the list so we don't keep looking at it
							bFound = true;	//yes, we do need to audit!
						}
					}
					if(!bFound) {
						bNeedToAudit = true;
						//TES 11/25/2008 - PLID 32066 - Update any ProcInfo records tied to this appointment, they
						// should maybe have a new surgeon.
						// (c.haag 2013-12-03) - PLID 59886 - Don't do this until after the batch
						bNeedToUpdateProcInfoSurgeon = true;						
					}
				}

				//DRT 7/8/2005 - PLID 16664 - Void superbills if the user said they wished to
				// (d.thompson 2010-05-28) - PLID 38812 - param batch
				if(bVoidSuperbills) {
					sql.Add("UPDATE PrintedSuperbillsT SET Void = 1, VoidDate = GetDate(), VoidUser = {STRING} WHERE PrintedSuperbillsT.ReservationID = {INT} AND Void = 0", GetCurrentUserName(), nReservationID);
				}

				// (j.gruber 2012-07-30 15:32) - PLID 51869 - Insurance Information
				if (bInsuranceChanged) {
					sql.Add("DELETE FROM AppointmentInsuredPartyT WHERE AppointmentID = {INT}", nReservationID);
					POSITION pos = pmapInsInfo->GetStartPosition();
					InsuranceInfo *pInsInfo;
					long nPlacement;					
					
					while (pos != NULL) {				
						pmapInsInfo->GetNextAssoc( pos, nPlacement, pInsInfo);
						sql.Add("INSERT INTO AppointmentInsuredPartyT(AppointmentID, InsuredPartyID, Placement) VALUES ({INT}, {INT}, {INT}); \r\n", nReservationID, pInsInfo->nInsuredPartyID, nPlacement);										
					}
					
					//setup the auditing
					CString strOld, strNew;
					strOld = GetApptInsAuditString(pmapOrigInsInfo);
					strNew = GetApptInsAuditString(pmapInsInfo);
					AuditEvent(nPatientID, GetExistingPatientName(nPatientID), nAuditID, aeiAppointmentInsurance, nReservationID, strOld, strNew, aepMedium, aetChanged);					
				}


				sql.CreateRecordsetNoTransaction(GetRemoteData());


				// Waiting list behavior.
				// (d.thompson 2010-05-28) - PLID 38812 - Moved to bottom, this should happen after the commit from above
				CString strQuery;
				if (bMoveUp) {
					strQuery += CWaitingListUtils::BuildWaitingListUpdateQuery(
						nReservationID, nPatientID, dtDate, 
						adwAptPurpose, adwResourceID, 
						nAptType, strNotes);
						
				} else { // MoveUp == FALSE
					// We need to delete the waiting list data for this appointment.
					strQuery += CWaitingListUtils::BuildWaitingListDeleteQuery(nReservationID);
				}
				
				ExecuteSqlStd(strQuery);

			//DRT 6/17/2004 - PLID 13032 - Moved this end_trans because we were really attempting to modify
			//	too much inside the transaction, and eventually (this code has worked for at least a year)
			//	someone modifed one of the functions we call and added another transaction of its own way
			//	down in the call stack.
			} END_TRANS("AppointmentModify");

			// (c.haag 2013-12-03) - PLID 59886 - Update the proc info surgeon if necessary. We have to do it
			// outside the transaction or the API won't be able to do it.
			if (bNeedToUpdateProcInfoSurgeon)
			{
				PhaseTracking::UpdateProcInfoSurgeon(-2, nReservationID);
			}

			//ALSO!  We might have resources in our new list that were not in the old list, so check those too
			//Since we deleted the items from the dwOld list as we encountered them, if anything exists
			//in this list, we need to audit
			if(dwOld.GetSize() > 0)
				bNeedToAudit = true;
			
			//for auditing
			//if anything needs to be written
			if(bNeedToAudit) {
				bResourcesChanged = true;
				CString strResourceOld, strResourceNew;

				strResourceOld = strOldResourceList;	//we pulled this out at the start

				// (d.thompson 2010-06-04) - PLID 39015 - Reworked query to avoid trips in a loop.  We are 100% certain that 
				//	the contents of adwResourceID are written to data... we just did it not too far above here.  So we can
				//	just do 1 query on that table, rather than a lookup.  We'll audit them in alphabetical order.
				_RecordsetPtr prsAudit = CreateParamRecordset("SELECT Item FROM ResourceT WHERE ID IN "
					"(SELECT ResourceID FROM AppointmentResourceT WHERE AppointmentID = {INT}) ORDER BY Item;", nReservationID);
				while(!prsAudit->eof) {
					strResourceNew += AdoFldString(prsAudit, "Item") + ", ";
					prsAudit->MoveNext();
				}

				strResourceNew.TrimRight(", ");

				// (b.cardillo 2005-11-17 15:04) - PLID 18383 - This was passing the patient ID 
				// for the RecordID parameter, when it should have been passing the actual 
				// appointment ID, because it's an appointment audit.  I changed it to pass the 
				// ReservationID, and now the problem is resolved.  I tried to see if there was 
				// some reason this was set to patient, but found no good explanation.  It has 
				// been here since this auditing was added, version 73 of this file, and no 
				// explanation was offered in the code comments or check-in comments.
				AuditEvent(nPatientID, GetExistingPatientName(nPatientID), nAuditID, aeiApptResource, nReservationID, strResourceOld, strResourceNew, aepMedium, aetChanged);
			}
			//done auditing resources

		//Let's create a new event, if we should.
		if(bCreateNewEvent) {
			//TES 6/13/2008 - PLID 28078 - Prompt to create an allocation, if appropriate.
			// (j.jones 2010-01-04 11:03) - PLID 32935 - this can potentially be disabled
			if(bCheckForAllocations) {
				CheckCreateAllocation(nReservationID, nPatientID, nAptType, adwAptPurpose, nLocationID);
			}
			PhaseTracking::CreateAndApplyEvent(PhaseTracking::ET_AppointmentCreated, nPatientID, COleDateTime(dtDate), nReservationID, true, -1, bAllowCreateLadder);
			if(lNoShow == 1 || lNoShow == 2 || lNoShow == 4) {
				PhaseTracking::CreateAndApplyEvent(PhaseTracking::ET_ActualAppointment, nPatientID, COleDateTime(dtDate), nReservationID, true, -1);
			}
		}

		//update case history info, if needed
		if(IsSurgeryCenter(false)) {

			//first compare patient IDs
			// (d.thompson 2010-06-04) - PLID 39015 - Parameterized
			// (d.thompson 2010-06-04) - PLID 39015 - Joined to the apttype name, as it's used in the loop
			_RecordsetPtr rs = CreateParamRecordset("SELECT CaseHistoryT.ID, AptTypeT.Name FROM CaseHistoryT "
				"LEFT JOIN AppointmentsT ON CaseHistoryT.AppointmentID = AppointmentsT.ID "
				"LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
				"WHERE AppointmentID = {INT} AND PersonID <> {INT}", nReservationID, nPatientID);
			CString strApptDesc;
			while(!rs->eof) {
				long nCaseHistoryID = AdoFldLong(rs, "ID");
				// (d.thompson 2010-06-04) - PLID 39015 - Parameterized
				ExecuteParamSql("UPDATE CaseHistoryT SET AppointmentID = NULL WHERE ID = {INT}", nCaseHistoryID);
				//TES 1/9/2007 - PLID 23575 - Audit that this case history's appointment has changed.
				if(strApptDesc.IsEmpty()) {
					// This is designed to display the appointment information in the same format as it appears
					// on the case history itself, see CCaseHistoryDlg::DisplayAppointmentDesc().
					// (d.thompson 2010-06-04) - PLID 39015 - Put query to get type name into the select above
					CString strType = AdoFldString(rs, "Name", "<No Type>");
					CString strPurpose = strPurposeOld;
					if(strPurpose.IsEmpty()) strPurpose = "<No Purpose>";
					CString strResource = strOldResourceList;
					strApptDesc.Format("%s %s, %s - %s, %s",FormatDateTimeForInterface(dtDate,NULL,dtoDate),
						FormatDateTimeForInterface(dtStartTime, DTF_STRIP_SECONDS, dtoTime),strType,strPurpose,strResource);
				}
				// (a.walling 2008-06-04 15:40) - PLID 29900 - Use the correct patient name
				AuditEvent(nPatientIDForAudit, strPatientNameForAudit, BeginNewAuditEvent(), aeiCaseHistoryAppointment, nCaseHistoryID, strApptDesc, "<None> (Appt was re-assigned)", aepMedium, aetChanged);
				rs->MoveNext();
			}

			//now compare dates
			if(strOldDate != strNewDate && IsSurgeryCenter(false)) {
				COleDateTime dtDateOnly;
				dtDateOnly.SetDate(dtDate.GetYear(),dtDate.GetMonth(),dtDate.GetDay());
				long count = 0;
				// (d.thompson 2010-06-04) - PLID 39015 - We don't need to select the count, then turn around and select
				//	the data again if > 0.  Just select the data first time.
				//TES 1/9/2007 - PLID 23575 - Get the old date for auditing.
				_RecordsetPtr rsCaseHistories = CreateParamRecordset("SELECT ID, SurgeryDate FROM CaseHistoryT WHERE AppointmentID = {INT} AND SurgeryDate <> {STRING}", nReservationID, FormatDateTimeForSql(dtDateOnly, dtoDate));
				if(!rsCaseHistories->eof) {
					//If there are any records, we want to prompt about updating them.
					long count = rsCaseHistories->GetRecordCount();
					CString str;
					if(count > 1) {
						str.Format("There are %li case histories attached to this appointment that specify a different surgery date.\n"
							"Would you like to update the surgery date on these case histories to reflect the new appointment date?",count);
					}
					else {
						str = "There is a case history attached to this appointment that specifies a different surgery date.\n"
							"Would you like to update the surgery date on this case history to reflect the new appointment date?";
					}
					if(IDYES == MsgBox(MB_ICONQUESTION|MB_YESNO,str)) {
						//Do all the updates in a batch in case there are multiple
						CParamSqlBatch sql;
						while(!rsCaseHistories->eof) {
							long nCaseHistoryID = AdoFldLong(rsCaseHistories, "ID");
							_variant_t varOldDate = rsCaseHistories->Fields->GetItem("SurgeryDate")->Value;
							CString strOldDate;
							if(varOldDate.vt == VT_DATE) {
								strOldDate = FormatDateTimeForInterface(VarDateTime(varOldDate), NULL, dtoDate);
							}
							else {
								strOldDate = "<None>";
							}
							sql.Add("UPDATE CaseHistoryT SET SurgeryDate = {STRING} WHERE ID = {INT}",FormatDateTimeForSql(dtDateOnly,dtoDate), nCaseHistoryID);
							// (a.walling 2008-06-04 15:40) - PLID 29900 - Use the correct patient name
							AuditEvent(nPatientIDForAudit, strPatientNameForAudit, BeginNewAuditEvent(), aeiCaseHistorySurgeryDate, nCaseHistoryID, strOldDate, FormatDateTimeForInterface(dtDateOnly,NULL,dtoDate), aepMedium, aetChanged);
							rsCaseHistories->MoveNext();
						}
						sql.Execute(GetRemoteData());
					}
				}
			}
		}

		// (d.thompson 2010-06-04) - PLID 39015 - Commit all audits here.  No auditing will be written to data until this line.
		// (c.haag 2010-09-09 08:41) - PLID 40453 - Use the auditTransaction object
		auditTransaction.Commit();

		// (d.thompson 2012-08-01) - PLID 51898 - Changed default to Prompt (1)
		if(GetRemotePropertyInt("ApptCheckOpenSlotsForMoveUps", 1, 0, GetCurrentUserName(), true) == 1)
		{
			COleDateTime dtEnd;
			dtEnd.SetDateTime(dtDate.GetYear(), dtDate.GetMonth(), dtDate.GetDay(),
				dtEndTime.GetHour(), dtEndTime.GetMinute(), dtEndTime.GetSecond());
			AttemptFindAvailableMoveUp(dtOldStartTime, dtOldEndTime, dtDate, dtEnd, nReservationID, bResourcesChanged, dwOld);
		}

		if(bPromptApptLink)
			AttemptAppointmentLinkAction(nReservationID, GetSafeTotalMinuteDiff(dtNewDate, dtOldStartTime));

		// (c.haag 2005-10-19 13:03) - PLID 15431 - Sometimes the date field includes the start time.
		// We need to make 100% sure it does not
		COleDateTime dtTableChange1, dtTableChange2;
		dtTableChange1.SetDateTime(dtDate.GetYear(), dtDate.GetMonth(), dtDate.GetDay(),
			dtStartTime.GetHour(), dtStartTime.GetMinute(), dtStartTime.GetSecond());
		dtTableChange2.SetDateTime(dtDate.GetYear(), dtDate.GetMonth(), dtDate.GetDay(),
			dtEndTime.GetHour(), dtEndTime.GetMinute(), dtEndTime.GetSecond());
		// (j.jones 2007-09-06 15:19) - PLID 27312 - required the EndTime as a parameter
		// (j.jones 2014-08-05 10:35) - PLID 63167 - added PatientID, LocationID, ResourceIDs
		CString strResourceIDs = ArrayAsString(adwResourceID);
		//this is space-delimited
		strResourceIDs.Replace(",", " ");
		CClient::RefreshAppointmentTable(nReservationID, nPatientID, dtTableChange1, dtTableChange2, 1 /* 1 means modified since last palm hotsync -- this is legacy logic */,
			lNoShow, nLocationID, strResourceIDs);


		// Update Microsoft Outlook
		PPCAddAppt(nReservationID);

		// Update PalmSyncT again (look at the comments at the first call
		// to UpdatePalmSyncT)
		UpdatePalmSyncT((DWORD)nReservationID);

		// (z.manning 2008-07-16 14:34) - PLID 30490 - Handle any HL7 messages relating to this appointment
		// (r.gonet 12/03/2012) - PLID 54107 - Updated to use refactored function.
		SendUpdateAppointmentHL7Message(nReservationID);

		// (z.manning 2015-11-05 12:13) - PLID 57109 - Update recalls needing attention dialog
		GetMainFrame()->HandleRecallChanged();

		// Make sure the time stamp is incremented indicating something has changed
		//IncrementTimeStamp(RES_TIME_STAMP_NAME);
		
		return true;
		// (a.walling 2010-09-08 14:30) - PLID 40377 - Yet another rollback with no active transaction!
	} NxCatchAll("::AppointmentModify Error 1");

	return false;
}

// (j.jones 2014-12-19 09:57) - PLID 64480 - given a SelectedFFASlot object and an existing appointment ID,
// change the appointment to use the new slot information, auditing accordingly
bool AppointmentModifyFromFFASlot(long nAppointmentID, SelectedFFASlotPtr newSlot)
{
	try {

		//this requires a real appointment ID
		if (nAppointmentID == -1) {
			ASSERT(FALSE);
			ThrowNxException("AppointmentModifyFromFFASlot called with no appointment ID!");
		}

		//this requires a valid slot
		if (newSlot == NULL || !newSlot->IsValid()) {
			ASSERT(FALSE);
			ThrowNxException("AppointmentModifyFromFFASlot called with no valid slot!");
		}

		_RecordsetPtr rs = CreateParamRecordset("SELECT PatientID, RequestedResourceID, Confirmed, dbo.GetPurposeIDString(ID) AS PurposeIDs, "
			"CONVERT(bit, CASE WHEN EXISTS (SELECT ID FROM WaitingListT WHERE AppointmentID = {INT}) THEN 1 ELSE 0 END) AS MoveUp, "
			"Status, ShowState, AppointmentsT.Notes, AptTypeID, Ready, AppointmentsT.RefPhysID "
			"FROM AppointmentsT "
			"WHERE AppointmentsT.ID = {INT}",
			nAppointmentID, nAppointmentID);

		if (rs->eof) {
			ASSERT(FALSE);
			ThrowNxException("AppointmentModifyFromFFASlot called with invalid appointment ID %li!", nAppointmentID);
		}
		else {

			SchedAuditItems items;
			CDWordArray adwPurposeIDs;
			LoadPurposeIDStringIntoArray(AdoFldString(rs->Fields, "PurposeIDs", ""), adwPurposeIDs);
			long nPatientID = AdoFldLong(rs->Fields, "PatientID");
			long nShowState = AdoFldLong(rs->Fields, "ShowState");

			// (r.farnworth 2016-02-02 12:08) - PLID 68116 - FFA results transmit location to new appointment.
			if (AppointmentModify(nAppointmentID, nPatientID, newSlot->dwaResourceIDs, newSlot->nLocationID,
				AsDateNoTime(newSlot->dtStart), newSlot->dtStart, newSlot->dtEnd, newSlot->dtArrival,
				AdoFldLong(rs->Fields, "Confirmed"), AdoFldBool(rs->Fields, "MoveUp", FALSE) ? true : false, nShowState,
				AdoFldString(rs->Fields, "Notes"), AdoFldLong(rs->Fields, "AptTypeID", -1), adwPurposeIDs, &items, AdoFldBool(rs->Fields, "Ready"),
				AdoFldLong(rs->Fields, "RequestedResourceID", -1), true, NULL, NULL, AdoFldLong(rs->Fields, "RefPhysID", -1))) {

				//send a tablechecker
				CString strResourceIDs = ArrayAsString(newSlot->dwaResourceIDs);
				//this is space-delimited
				strResourceIDs.Replace(",", " ");
				// (r.farnworth 2016-02-02 12:08) - PLID 68116 - FFA results transmit location to new appointment.
				CClient::RefreshAppointmentTable(nAppointmentID, nPatientID, newSlot->dtStart, newSlot->dtEnd,
					AdoFldByte(rs->Fields, "Status"), nShowState, newSlot->nLocationID, strResourceIDs);
				return true;
			}
		}

	}NxCatchAll(__FUNCTION__);

	return false;
}

// Returns whether the scheduler needs to be refreshed
BOOL IsSchedulerDirty()
{
	// TODO: We used to use time-stamps but I'm not sure if we should anymore

	// Always err on the side of returning TRUE
	return TRUE;
}

// Marks the scheduler as not needing to be refreshed
void SchedulerClean()
{
	// TODO: We used to use time-stamps but I'm not sure if we should anymore

}

// For handling single appointments quickly (must call AppointmentLetGo)
_RecordsetPtr AppointmentGrab(long nReservationID, BOOL bIncludeResourceIDs /*= FALSE */, BOOL bIncludePurposeIDs /*= FALSE */)
{
	// (c.haag 2006-05-23 11:57) - We now support getting resource and purpose ID strings.
	// To load the ID's into arrays, use the LoadResourceIDStringIntoArray and 
	// LoadPurposeIDStringIntoArray functions, respectively.
	try { 
		// (d.moore 2007-05-22 11:00) - PLID 4013 - Changed the MoveUp bit to query the waiting list table.
		// (j.armen 2011-11-17 10:34) - PLID 44205 - Added ConfirmedBy
		// (a.walling 2013-01-21 16:48) - PLID 54747 - Populate CancelledDate property for scheduler reservations
		// (a.walling 2013-02-08 12:59) - PLID 55083 - Parameterize		
		// (j.jones 2014-12-02 09:31) - PLID 64182 - get the primary insured ID
		_RecordsetPtr prs = CreateParamRecordset(GetRemoteData(), adUseClient, adOpenDynamic, adLockOptimistic, //adOpenForwardOnly, adLockReadOnly, 
			"SELECT ID, PatientID, LocationID, Date, StartTime, EndTime, ArrivalTime, Status, Confirmed, ConfirmedBy, "
			"CONVERT(bit, CASE WHEN EXISTS (SELECT ID FROM WaitingListT WHERE AppointmentID = {INT}) THEN 1 ELSE 0 END) AS MoveUp, "
			"ShowState, Notes, ModifiedDate, ModifiedLogin, AptTypeID, AptPurposeID, RecordID, "
			"CreatedDate, CreatedLogin, RequestedResourceID, CancelledDate, "
			"PrimaryAppointmentInsuredPartyQ.InsuredPartyID AS PrimaryInsuredPartyID "
			"{CONST_STR} {CONST_STR} "
			"FROM AppointmentsT "
			"LEFT JOIN ("
			"	SELECT AppointmentID, InsuredPartyID FROM AppointmentInsuredPartyT "
			"	WHERE Placement = 1 "
			") AS PrimaryAppointmentInsuredPartyQ ON AppointmentsT.ID = PrimaryAppointmentInsuredPartyQ.AppointmentID "
			"WHERE AppointmentsT.ID = {INT}", 
			nReservationID, 
			bIncludeResourceIDs ? ",dbo.GetResourceIDString(ID) AS ResourceIDs" : "",
			bIncludePurposeIDs ? ",dbo.GetPurposeIDString(ID) AS PurposeIDs" : "",
			nReservationID);
		
		if (prs->eof) {
			// No record so return failure
			prs->Close();
			return NULL;
		} else {
			// Got a record so return the recordset
			return prs;
		}
	} NxCatchAllCall("::AppointmentGrab Error 1", return NULL);
}

OLE_COLOR AppointmentGetBorderColor(long nAptTypeID)
{
	try {
		// (d.thompson 2010-05-11) - PLID 38581 - Parameterized
		_RecordsetPtr prs = CreateParamRecordset(
			"SELECT Color "
			"FROM AptTypeT "
			"WHERE ID = {INT}", nAptTypeID);
		if (prs->eof) {
			prs->Close();
			return DEFAULT_APPT_BORDER_COLOR;
		} else {
			return AdoFldLong(prs, "Color", 0);
		}
	} NxCatchAllCall("::AppointmentGetBorderColor Error 1", return 0);
}

// (j.jones 2014-12-05 09:10) - PLID 64119 - this now optionally returns the status ID
OLE_COLOR AppointmentGetStatusColor(long nResID, OUT long *pnStatusID /*= NULL*/)
{
	try {
		// (d.thompson 2010-05-11) - PLID 38581 - Parameterized
		_RecordsetPtr prs = CreateParamRecordset(
			"SELECT AptShowStateT.ID, AptShowStateT.Color "
			"FROM AptShowStateT "
			"INNER JOIN AppointmentsT ON AppointmentsT.ShowState = AptShowStateT.ID "
			"WHERE AppointmentsT.ID = {INT}", nResID);
		if (prs->eof) {
			prs->Close();
			if (pnStatusID) {
				*pnStatusID = -1;
			}
			return DEFAULT_APPT_BACKGROUND_COLOR;
		} else {
			if (pnStatusID) {
				*pnStatusID = AdoFldLong(prs, "ID", 0);
			}
			return AdoFldLong(prs, "Color", 0);
		}
	} NxCatchAllCall("::AppointmentGetStatusColor Error 1", return 0);
}

CString g_strBoxText;

// (j.jones 2011-02-11 12:01) - PLID 35180 - required dates and a string of resource IDs
const CString &AppointmentGetText(long nReservationID, COleDateTime dtFromDate, COleDateTime dtToDate, CString strResourceIDs, CSchedulerView *pSchedView /* = NULL */, CString strMultiPurpose /* = "" */)
{
	// Clear the return value
	g_strBoxText.Empty();
	
	// Create a quick recordset object
	CReservationReadSet rs;
	rs.m_pSchedView = pSchedView;
	try {		
		// (a.walling 2013-06-18 11:31) - PLID 57204 - Use new ResExtendedQ alternative in scheduler views for significantly better scheduler performance
		// (j.jones 2014-12-03 13:54) - PLID 64275 - if the appointment has overridden a mix rule, show it first
		CSqlFragment query(
			"DECLARE @dateFrom DATETIME; DECLARE @dateTo DATETIME; \r\n"
			"DECLARE @apptID INT; \r\n"
			"SET @dateFrom = {OLEDATETIME}; SET @dateTo = DATEADD(d, 1, {OLEDATETIME}); \r\n"
			"SET @apptID = {INT}; \r\n"
			"{SQL} \r\n" // base query
			"WHERE AppointmentsT.StartTime >= @dateFrom AND AppointmentsT.StartTime < @dateTo \r\n"
			"AND AppointmentResourceT.ResourceID IN ({CONST_STR}) \r\n"
			"AND AppointmentsT.ID = @apptID \r\n"
			"ORDER BY IsMixRuleOverridden DESC, AppointmentsT.StartTime, AppointmentsT.ID, AptPurposeT.Name; \r\n"
			"\r\n"
			"SELECT DISTINCT SecurityGroupDetailsT.PatientID, SecurityGroupDetailsT.SecurityGroupID \r\n"
			"FROM SecurityGroupDetailsT "
			"INNER JOIN AppointmentsT \r\n"
			"ON SecurityGroupDetailsT.PatientID = AppointmentsT.PatientID \r\n"
			"WHERE AppointmentsT.ID = @apptID \r\n"
			"ORDER BY SecurityGroupDetailsT.PatientID, SecurityGroupDetailsT.SecurityGroupID; "
			, AsDateNoTime(dtFromDate), AsDateNoTime(dtToDate)
			, nReservationID
			, Nx::Scheduler::GetResExtendedBase(pSchedView, AsDateNoTime(dtFromDate), AsDateNoTime(dtToDate))
			, SortAndRemoveDuplicates(strResourceIDs) // (a.walling 2013-06-20 13:49) - PLID 57204 - Handles space and comma separated values
		);

		{
			CNxPerform nxp("ReFilter " __FUNCTION__);
			rs.ReFilter(query);
		}

		// Get the box text from the now open recordset
		if (!rs.IsEOF()) {
			// (j.luckoski 2012-06-20 10:55) - PLID 11597 - if cancelled pass in bool to indicate that is cancelled.
			bool bIsCancelled = rs.GetCancelledDate().GetStatus() == COleDateTime::valid;

			// (j.jones 2014-12-03 10:58) - PLID 64274 - added bIsMixRuleOverridden
			bool bIsMixRuleOverridden = rs.GetIsMixRuleOverridden();

			// (a.walling 2013-01-21 16:48) - PLID 54744 - Cancelled date is available to us in the ReadSet thing			
			g_strBoxText = rs.GetBoxText(strMultiPurpose, GetRemotePropertyInt("ColorApptTextWithStatus", GetPropertyInt("ColorApptTextWithStatus", 0, 0, false), 0, GetCurrentUserName(), true), bIsCancelled, bIsMixRuleOverridden);
		}
		
		// Close it
		rs.Close();
	} catch (CException *e) {
		// If there was an error, the return value is already cleared
		e->ReportError();
		e->Delete();
	}
	
	// Make sure the recordset has been closed
	if (rs.IsOpen()) rs.Close();
	
	// Return the answer
	return g_strBoxText;
}

long AppointmentGetPatientID(long nResID)
{
	// (d.thompson 2010-05-11) - PLID 38581 - Parameterized
	_RecordsetPtr pAppt = CreateParamRecordset("SELECT PatientID FROM AppointmentsT WHERE ID = {INT}", nResID);
	if ((pAppt != NULL) && !pAppt->eof) {
		return AdoFldLong(pAppt, "PatientID", -25);
	} else {
		return -25;
	}
}

// (j.jones 2014-12-02 10:00) - PLID 64182 - added pParentWnd
// (j.jones 2014-12-19 12:50) - PLID 64182 - bAuditDates is set to false if the caller does not need to audit
BOOL AppointmentUpdate(CWnd *pParentWnd, ADODB::_RecordsetPtr pAppt, bool &bAuditDates, long nResIDToIgnore /*= -1*/)
{
	if (!AppointmentValidateByPermissions(pAppt))
		return FALSE;

	// If the user will be prompted to decide how to manually deal with
	// linked appointments, we don't have a guarantee the user will
	// move them. Therefore, they should be ignored when we check templates.
	BOOL bIgnoreLinkedAppts;
	if (GetRemotePropertyInt("PromptOnModifyLinkedAppts", 0, 0, "<None>", true))
		bIgnoreLinkedAppts = TRUE;
	else
		bIgnoreLinkedAppts = FALSE;

	if (!AppointmentValidateByRules(pAppt, bIgnoreLinkedAppts))
		return FALSE;

	if (!AppointmentValidateByAlarms(pAppt, nResIDToIgnore))
		return FALSE;

	// Make sure the start time and end time are valid
	COleDateTime dtStart = AdoFldDateTime(pAppt, "StartTime");
	COleDateTime dtEnd = AdoFldDateTime(pAppt, "EndTime");

	// If someone made an appointment in such a way that the end time is
	// 12:00am and the start time is not 12:00am, we assume the appt spilled
	// into the next day, and therefore, we have to change the end time to
	// 11:59pm
	if (dtEnd.GetHour() == 0 && dtEnd.GetMinute() == 0 &&
		!(dtStart.GetHour() == 0 && dtStart.GetMinute() == 0))
	{
		dtEnd.SetDateTime(dtEnd.GetYear(), dtEnd.GetMonth(), dtEnd.GetDay(),
			23,59,0);
		pAppt->Fields->Item["EndTime"]->Value = _variant_t(dtEnd, VT_DATE);
	}

	if (!AppointmentValidateByDuration(pAppt))
		return FALSE;

	// (j.jones 2014-12-02 09:43) - PLID 64182 - validate against Scheduler Mix Rules last
	std::vector<SchedulerMixRule> overriddenMixRules;
	SelectedFFASlotPtr pSelectedFFASlot;
	pSelectedFFASlot.reset();
	if (!AppointmentValidateByMixRules(pParentWnd, pAppt, overriddenMixRules, pSelectedFFASlot, nResIDToIgnore)) {
		return FALSE;
	}

	long nResID = AdoFldLong(pAppt, "ID");
	long nPatientID = AdoFldLong(pAppt, "PatientID");

	// (c.haag 2003-07-31 09:48) - Make sure the location is valid
	long nNewLocationID;
	//TES 8/10/2010 - PLID 39264 - Pass in the Date and StartTime
	if (!GetValidAppointmentLocation(AdoFldLong(pAppt, "LocationID"), nNewLocationID, nResID, AdoFldDateTime(pAppt, "Date"), dtStart))
		return FALSE;
	pAppt->Fields->Item["LocationID"]->Value = nNewLocationID;

	
	//now we can legitimately save	
	bAuditDates = true;
	
	// Write to the table and close the recordset
	pAppt->Update();
	pAppt->Close();

	// (j.jones 2014-12-19 10:52) - PLID 64182 - if a new appointment slot was provided, move the appt.
	// to that slot
	if (pSelectedFFASlot != NULL && pSelectedFFASlot->IsValid()) {
		if (!AppointmentModifyFromFFASlot(nResID, pSelectedFFASlot)) {
			return FALSE;
		}
		//set bAuditDates to false so the caller does not audit twice
		bAuditDates = false;
	}
	
	// Track our modifications
	// (d.thompson 2010-05-11) - PLID 38581 - Parameterized
	ExecuteParamSql(
		"UPDATE AppointmentsT "
		"SET Status = 1, ModifiedDate = getdate(), ModifiedLogin = {STRING} "
		"WHERE ID = {INT}", 
		GetCurrentUserName(), nResID);

	// (j.jones 2014-12-02 15:32) - PLID 64182 - if the user overrode a mix rule,
	// track that they did so
	if (overriddenMixRules.size() > 0) {
		TrackAppointmentMixRuleOverride(nResID, overriddenMixRules);
	}

	// Update Microsoft Outlook
	PPCAddAppt(nResID);
	// Update PalmSyncT
	UpdatePalmSyncT(nResID);

	// (z.manning 2008-07-16 14:34) - PLID 30490 - Handle any HL7 messages relating to this appointment
	// (r.gonet 12/03/2012) - PLID 54107 - Updated to use refactored function.
	SendUpdateAppointmentHL7Message(nResID);

	// (z.manning 2015-11-05 12:13) - PLID 57109 - Update recalls needing attention dialog
	GetMainFrame()->HandleRecallChanged();

	return TRUE;
}

// (a.wilson 2014-08-21 14:33) - PLID 63170 - use the refreshappointmenttable() with all the parameters to prevent extra recordsets.
BOOL AppointmentMarkNoShow(DWORD dwReservationID, BOOL bUpdatingLinkedAppt /* = FALSE */, CReasonDlg *pdlgReason /* = NULL */)
{
	try{

		CString strOldShowState = "", strResourceIDs = "";
		long nPatientID = 0, nLocationID = -1, nStatus = -1;
		COleDateTime dtStart = g_cdtMin, dtEnd = g_cdtMin;
		// (d.thompson 2010-05-11) - PLID 38581 - Parameterized
		_RecordsetPtr rs = CreateParamRecordset("SELECT Name, PatientID, StartTime, EndTime, Status, LocationID, dbo.GetResourceIDString(AppointmentsT.ID) AS ResourceIDs "
			"FROM AppointmentsT INNER JOIN AptShowStateT ON AppointmentsT.ShowState = AptShowStateT.ID WHERE AppointmentsT.ID = {INT}", dwReservationID);
		if(!rs->eof) {
			strResourceIDs = AdoFldString(rs, "ResourceIDs", "");
			strOldShowState = AdoFldString(rs, "Name","");
			nPatientID = AdoFldLong(rs, "PatientID");
			nLocationID = AdoFldLong(rs, "LocationID");
			nStatus = (long)AdoFldByte(rs, "Status");
			dtStart = AdoFldDateTime(rs, "StartTime");
			dtEnd = AdoFldDateTime(rs, "EndTime");
		}
		rs->Close();
		
		CReasonDlg *pdlg = NULL;
		int nResult;
		if(bUpdatingLinkedAppt && pdlgReason != NULL) {
			nResult = IDOK;
			pdlg = pdlgReason;
		}
		else {
			pdlg = new CReasonDlg(NULL);
			pdlg->m_bNoShow = true;
			pdlg->m_strText = "Are you sure you want to mark this appointment as No Show?";
			pdlg->m_nApptID = dwReservationID;
			nResult = pdlg->DoModal();
		}

		if(nResult == IDOK) {

			// (c.haag 2009-08-06 10:28) - PLID 25943 - See if the appointment is linked with superbills. If so,
			// see what the user wants to do with the superbills.
			BOOL bVoidSuperbills = FALSE;
			// (z.manning 2011-04-01 15:29) - PLID 42973 - Converted where clause to a SQL fragment
			switch (NeedsToVoidNoShowSuperbills(CSqlFragment("AppointmentsT.ID = {INT}", dwReservationID)))
			{
				case IDYES: // Yes, there are superbills to void
					bVoidSuperbills = TRUE;
					break;
				case IDNO: // Do not void superbills
					bVoidSuperbills = FALSE;
					break;
				case IDCANCEL: // User changed their mind
					if(pdlg != NULL) {
						if (pdlgReason != pdlg) {
							delete pdlg;
						} else {
							// The caller is responsible for deleting the dialog (see evident
							// usage of UpdateLinkedAppointmentShowState in CResEntryDlg::SaveRes)
						}
					}
					return FALSE;
			}

			//This audit event should be called after we know the user is not going to cancel.
			long nAuditID = BeginNewAuditEvent();
			CString strNewShowState = "No Show";
			AuditEvent(nPatientID, GetExistingPatientName(nPatientID), nAuditID, aeiApptShowState, dwReservationID, strOldShowState, strNewShowState, aepMedium);

			ASSERT(pdlg != NULL);
			// (a.walling 2013-02-08 12:59) - PLID 55083 - Parameterize all these
			if(pdlg->IsCustomReason()) {
				ExecuteParamSql("UPDATE AppointmentsT SET ShowState = 3, NoShowDate = GetDate(), ModifiedDate = GetDate(), ModifiedLogin = {STRING}, NoShowReason = {STRING} WHERE AppointmentsT.ID = {INT}", GetCurrentUserName(), pdlg->m_strReason, dwReservationID);
			}
			else {
				long nReasonID;
				pdlg->GetReason(nReasonID);
				ExecuteParamSql("UPDATE AppointmentsT SET ShowState = 3, NoShowDate = GetDate(), ModifiedDate = GetDate(), ModifiedLogin = {STRING}, NoShowReasonID = {INT} WHERE AppointmentsT.ID = {INT}", GetCurrentUserName(), nReasonID, dwReservationID);
			}
			
			// (a.walling 2014-12-22 10:48) - PLID 64366 - Reschedule when marking as No Show
			if (pdlg->m_bReschedule) {
				Nx::Scheduler::AddToReschedulingQueue(dwReservationID, nPatientID);
			}

			if (bVoidSuperbills) {
				ExecuteParamSql("UPDATE PrintedSuperbillsT SET Void = 1, VoidDate = GetDate(), VoidUser = {STRING} "
					"WHERE PrintedSuperbillsT.ReservationID = {INT} AND Void = 0", GetCurrentUserName(), dwReservationID);
			}

			// (a.walling 2006-10-24 09:35) - PLID 23195 - Uncomplete tracking steps when marking no show
			PhaseTracking::UnapplyEvent(nPatientID, PhaseTracking::ET_AppointmentCreated, (long)dwReservationID);
			PhaseTracking::UnapplyEvent(nPatientID, PhaseTracking::ET_ActualAppointment, (long)dwReservationID);

			// send a network message
			CClient::RefreshAppointmentTable(dwReservationID, nPatientID, dtStart, dtEnd, nStatus, 3, nLocationID, strResourceIDs);
			// Update Microsoft Outlook
			PPCAddAppt((long)dwReservationID);
			// Update PalmSyncT
			UpdatePalmSyncT(dwReservationID);			

			// (z.manning 2008-07-16 14:34) - PLID 30490 - Handle any HL7 messages relating to this appointment
			// (r.gonet 12/03/2012) - PLID 54107 - Updated to use refactored function.
			SendUpdateAppointmentHL7Message(dwReservationID);

			// (z.manning 2015-11-05 12:13) - PLID 57109 - Update recalls needing attention dialog
			GetMainFrame()->HandleRecallChanged();

			if(!bUpdatingLinkedAppt) {
				// (z.manning, 07/26/2007) - PLID 14579 - See if the user wants to update the status of any linked appts.
				UpdateLinkedAppointmentShowState(dwReservationID, assNoShow, strNewShowState, pdlg);
				if(pdlg != NULL) {
					delete pdlg;
				}
			}

			return TRUE;
		}
		else {
			return FALSE;
		}
	}
	NxCatchAll("Error in AppointmentMarkNoShow()");
	return FALSE;
}

//(e.lally 2005-07-11) PLID 16782 - The function should be used if an appointment is marked 'In'.
//It looks for previous 'In' and 'Out' entries that will be overwritten as a result of the save.
//We need to check if the user wants to continue saving and return true or false
BOOL CheckOverwriteMarkIn(DWORD dwReservationID)
{
	if(dwReservationID == -1){
		return FALSE;
	}
	//check for a previous appointment entry for being marked In or Out
	// (d.thompson 2010-05-12) - PLID 38581 - Parameterized
	_RecordsetPtr rsCheck = CreateParamRecordset("SELECT TOP 1 ShowStateID AS ShowStateID "
			"FROM AptShowStateHistoryT "
			"WHERE AppointmentID = {INT} AND ShowStateID In (1,2) "
			"ORDER BY ShowStateID ASC ",
			dwReservationID);
	if(!rsCheck->eof){
		//By making ShowStateID ascending in the recordset, we know that the In entry would be first.

		long nShowState = AdoFldLong(rsCheck, "ShowStateID");
		rsCheck->Close();

		//There is a previous timestamp for marked In, ask if they want to continue saving
		if(nShowState == 1 && IDNO==MsgBox(MB_YESNO, "This appointment has already been marked 'In'. Are you sure you want to continue saving?")){
			//They do not want to overwrite marking the appointment In so return false
			return FALSE;
		}
		//There is no In entry but there is a previous timestamp for marked Out, ask if they want to continue saving
		else if(nShowState == 2 && IDNO==MsgBox(MB_YESNO, "This appointment has already been marked 'Out'. Are you sure you want to continue saving?")){
			//They do not want to overwrite marking the appointment In so return false
			return FALSE;
		}
		
		
	}//end if !eof

	//there was not an In or Out entry to overwrite so just return true
	return TRUE;
}
// (a.wilson 2014-08-21 14:33) - PLID 63170 - use the refreshappointmenttable() with all the parameters to prevent extra recordsets.
BOOL AppointmentMarkIn(DWORD dwReservationID, BOOL bUpdatingLinkedAppt /* = FALSE */)
{
	try{
		//(e.lally 2005-07-11) PLID 16782 - If an appointment is changed to be 'In'
			//we need to see if they want to overwrite the previous timestamp (if any) in the history and
			//continue saving
		// (z.manning, 07/26/2007) - PLID 14579 - Don't do this if we're simply updating a linked appointment.
		if(!bUpdatingLinkedAppt) {
			if(!CheckOverwriteMarkIn(dwReservationID)) {
				return FALSE;
			}
		}

		CString strOldShowState = "", strResourceIDs = "";
		long nPatientID = 0, nOldShowState = -1;
		long nLocationID = -1, nStatus = -1;
		COleDateTime dtStart = g_cdtMin, dtEnd = g_cdtMin;
		// (a.walling 2013-02-08 12:59) - PLID 55083 - Parameterize
		_RecordsetPtr rs = CreateParamRecordset("SELECT Name, PatientID, ShowState, StartTime, EndTime, Status, LocationID, dbo.GetResourceIDString(AppointmentsT.ID) AS ResourceIDs "
			"FROM AppointmentsT INNER JOIN AptShowStateT ON AppointmentsT.ShowState = AptShowStateT.ID WHERE AppointmentsT.ID = {INT}", dwReservationID);
		if(!rs->eof) {
			strOldShowState = AdoFldString(rs, "Name","");
			nOldShowState = AdoFldLong(rs, "ShowState",-1);
			nPatientID = AdoFldLong(rs, "PatientID");
			strResourceIDs = AdoFldString(rs, "ResourceIDs", "");
			nLocationID = AdoFldLong(rs, "LocationID");
			nStatus = (long)AdoFldByte(rs, "Status");
			dtStart = AdoFldDateTime(rs, "StartTime");
			dtEnd = AdoFldDateTime(rs, "EndTime");
		}
		rs->Close();

		long nAuditID = BeginNewAuditEvent();
		CString strNewShowState = "In";
		AuditEvent(nPatientID, GetExistingPatientName(nPatientID), nAuditID, aeiApptShowState, dwReservationID, strOldShowState, strNewShowState, aepMedium);
		
		// (a.walling 2013-02-08 12:59) - PLID 55083 - Parameterize
		ExecuteParamSql("UPDATE AppointmentsT SET ShowState = 1, ModifiedDate = GetDate(), ModifiedLogin = {STRING} WHERE AppointmentsT.ID = {INT}", GetCurrentUserName(), dwReservationID);

		if (nOldShowState == 3) {
			// (a.walling 2006-11-01 09:24) - PLID 23195 - we are removing the former no-show status, so reapply this event
			COleDateTime dtDate = COleDateTime::GetCurrentTime();
			PhaseTracking::CreateAndApplyEvent(PhaseTracking::ET_AppointmentCreated, nPatientID, dtDate, dwReservationID, true, -1);
			PhaseTracking::CreateAndApplyEvent(PhaseTracking::ET_ActualAppointment, nPatientID, dtDate, dwReservationID, true, -1);

			// (z.manning 2015-11-05 12:13) - PLID 57109 - Update recalls needing attention dialog
			GetMainFrame()->HandleRecallChanged();
		}

		// send a network message
		CClient::RefreshAppointmentTable(dwReservationID, nPatientID, dtStart, dtEnd, nStatus, 1, nLocationID, strResourceIDs);

		//Notify Tracking.
		PhaseTracking::CreateAndApplyEvent(PhaseTracking::ET_ActualAppointment, nPatientID, COleDateTime::GetCurrentTime(), dwReservationID);

		// Update Microsoft Outlook
		PPCAddAppt((long)dwReservationID);
		// Update PalmSyncT
		UpdatePalmSyncT(dwReservationID);

		// (z.manning 2008-07-16 14:34) - PLID 30490 - Handle any HL7 messages relating to this appointment
		// (r.gonet 12/03/2012) - PLID 54107 - Updated to use refactored function.
		SendUpdateAppointmentHL7Message(dwReservationID);

		// For Dr. Zeff: If this is marked as in, and we want to notify
		// everyone when this happens, do it.
		// (c.haag 2003-08-08 12:08) - Now we do this all the time; the preference is
		// now a matter of if you want to see them or not.
		// (z.manning, 07/26/2007) - PLID 14579 - We don't need to do this if we're just updating a linked appt
		// because we should have already done it.
		if(!bUpdatingLinkedAppt)
		{
			//TES 6/9/2008 - PLID 23243 - We now pass in the appointment ID, and its resource IDs, so pull them in the
			// same recordset (every appointment must have at least one resource.
			// (a.walling 2013-02-08 12:59) - PLID 55083 - Parameterize
			rs = CreateParamRecordset("SELECT PatientID, ResourceID FROM AppointmentsT INNER JOIN AppointmentResourceT ON "
				"AppointmentsT.ID = AppointmentResourceT.AppointmentID "
				"WHERE AppointmentsT.ID = {INT}", dwReservationID);
			long nPatientID = -1;
			CDWordArray dwaResources;
			while(!rs->eof) {
				if(nPatientID == -1) {
					nPatientID = AdoFldLong(rs, "PatientID");
				}
				dwaResources.Add((DWORD)AdoFldLong(rs, "ResourceID"));
				rs->MoveNext();
			}
			SendInPatientNotification(nPatientID, (long)dwReservationID, dwaResources);
		}

		LogShowStateTime(dwReservationID, 1);

		// (z.manning, 07/26/2007) - PLID 15579 - If we're updating a linked appointment, let's not do this room
		// manager stuff because not only do want want to prompt the user for each linked appointment, but if
		// the room manager needs to handle linked appointments at all it should do it itself.
		if(!bUpdatingLinkedAppt)
		{
			// (j.jones 2006-10-05 09:42) - PLID 22792 - see if the user wishes to be prompted about a Room
			long nRoomAssignWhenApptMarkedIn = GetRemotePropertyInt("RoomAssignWhenApptMarkedIn", 0, 0, GetCurrentUserName(), true);

			//do nothing if the appointment is currently in a room or has already been checked out,
			//or is not for today's date
			// (a.walling 2010-10-19 09:45) - PLID 40965 - Use ReturnsRecordsParam
			if(!ReturnsRecordsParam("SELECT ID FROM RoomAppointmentsT WHERE AppointmentID = {INT}", dwReservationID)
				&& ReturnsRecordsParam("SELECT ID FROM AppointmentsT WHERE Date = dbo.AsDateNoTime({OLEDATETIME}) AND ID = {INT}", COleDateTime::GetCurrentTime(), dwReservationID)) {

				if(nRoomAssignWhenApptMarkedIn == 1) {

					//prompt to assign to a room

					CRoomSelectorDlg dlg(NULL);
					dlg.m_nAppointmentID = dwReservationID;
					dlg.DoModal();

					//the dialog will be responsible for updating data
					//and sending network messages
				}
				else if(nRoomAssignWhenApptMarkedIn == 2) {
					//open room manager
					if(GetMainFrame())
						GetMainFrame()->ShowRoomManager(dwReservationID);
				}
				else {
					//do nothing
				}
			}
		}

		if(!bUpdatingLinkedAppt) {
			// (z.manning, 07/26/2007) - PLID 14579 - See if the user wants to update the status of any linked appts.
			UpdateLinkedAppointmentShowState(dwReservationID, assIn, strNewShowState);
		}

		return TRUE;
	}
	NxCatchAll("Error in OnMarkIn()");
	return FALSE;
}
// (a.wilson 2014-08-21 14:33) - PLID 63170 - use the refreshappointmenttable() with all the parameters to prevent extra recordsets.
BOOL AppointmentMarkOut(DWORD dwReservationID, BOOL bUpdatingLinkedAppt /* = FALSE */)
{
	try{

		CString strOldShowState = "", strResourceIDs = "";
		long nPatientID = 0, nOldShowState = -1;
		long nLocationID = -1, nStatus = -1;
		COleDateTime dtStart = g_cdtMin, dtEnd = g_cdtMin;
		// (d.thompson 2010-05-11) - PLID 38581 - Parameterized
		_RecordsetPtr rs = CreateParamRecordset("SELECT Name, PatientID, StartTime, EndTime, Status, ShowState, LocationID, dbo.GetResourceIDString(AppointmentsT.ID) AS ResourceIDs "
			"FROM AppointmentsT INNER JOIN AptShowStateT ON AppointmentsT.ShowState = AptShowStateT.ID WHERE AppointmentsT.ID = {INT}", dwReservationID);
		if(!rs->eof) {
			strOldShowState = AdoFldString(rs, "Name","");
			nOldShowState = AdoFldLong(rs, "ShowState",-1);
			nPatientID = AdoFldLong(rs, "PatientID");
			strResourceIDs = AdoFldString(rs, "ResourceIDs", "");
			nLocationID = AdoFldLong(rs, "LocationID");
			nStatus = (long)AdoFldByte(rs, "Status");
			dtStart = AdoFldDateTime(rs, "StartTime");
			dtEnd = AdoFldDateTime(rs, "EndTime");
		}
		rs->Close();

		long nAuditID = BeginNewAuditEvent();
		CString strNewShowState = "Out";
		AuditEvent(nPatientID, GetExistingPatientName(nPatientID), nAuditID, aeiApptShowState, dwReservationID, strOldShowState, strNewShowState, aepMedium);
		
		// (a.walling 2013-02-08 12:59) - PLID 55083 - Parameterize
		ExecuteParamSql("UPDATE AppointmentsT SET ShowState = 2, ModifiedDate = GetDate(), ModifiedLogin = {STRING} WHERE AppointmentsT.ID = {INT}", GetCurrentUserName(), dwReservationID);

		if (nOldShowState == 3) {
			// (a.walling 2006-11-01 09:24) - PLID 23195 - we are removing the former no-show status, so reapply this event
			COleDateTime dtDate = COleDateTime::GetCurrentTime();
			PhaseTracking::CreateAndApplyEvent(PhaseTracking::ET_AppointmentCreated, nPatientID, dtDate, dwReservationID, true, -1);
			PhaseTracking::CreateAndApplyEvent(PhaseTracking::ET_ActualAppointment, nPatientID, dtDate, dwReservationID, true, -1);

			// (z.manning 2015-11-05 12:13) - PLID 57109 - Update recalls needing attention dialog
			GetMainFrame()->HandleRecallChanged();
		}

		// send a network message
		CClient::RefreshAppointmentTable(dwReservationID, nPatientID, dtStart, dtEnd, nStatus, 2, nLocationID, strResourceIDs);
		// Update Microsoft Outlook
		PPCAddAppt((long)dwReservationID);
		// Update PalmSyncT
		UpdatePalmSyncT(dwReservationID);

		// (z.manning 2008-07-16 14:34) - PLID 30490 - Handle any HL7 messages relating to this appointment
		// (r.gonet 12/03/2012) - PLID 54107 - Updated to use refactored function.
		SendUpdateAppointmentHL7Message(dwReservationID);

		//Notify Tracking
		PhaseTracking::CreateAndApplyEvent(PhaseTracking::ET_ActualAppointment, nPatientID, COleDateTime::GetCurrentTime(), dwReservationID);

		//log the history
		LogShowStateTime(dwReservationID, 2);

		// (z.manning, 07/26/2007) - PLID 15579 - If we're updating a linked appointment, let's not do this room
		// manager stuff because not only do want want to prompt the user for each linked appointment, but if
		// the room manager needs to handle linked appointments at all it should do it itself.
		if(!bUpdatingLinkedAppt) {

			// (j.jones 2006-10-05 09:42) - PLID 22792 - see if the user wishes to be prompted about a Room
			long nRoomClearWhenApptMarkedOut = GetRemotePropertyInt("RoomClearWhenApptMarkedOut", 0, 0, GetCurrentUserName(), true);

			// (j.jones 2008-05-30 10:02) - PLID 27797 - tweaked the wording of these messages to account for the 'Ready To Check Out' status

			//do nothing if the appointment is not in the room manager, or is checked out					
			// (j.jones 2008-05-30 10:06) - PLID 27797 - included more information to eliminate
			// a redundant audit recordset
			// (j.jones 2010-12-02 16:03) - PLID 38597 - removed a pointless ReturnsRecords recordset,
			// if we are always going to query the data, might as well do it once, not twice
			_RecordsetPtr rs = CreateParamRecordset("SELECT RoomAppointmentsT.ID, RoomsT.Name, RoomsT.WaitingRoom, "
				"RoomAppointmentsT.StatusID, AppointmentsT.PatientID, RoomStatusT.Name AS RoomStatusName "
				"FROM RoomAppointmentsT "
				"INNER JOIN RoomsT ON RoomAppointmentsT.RoomID = RoomsT.ID "
				"INNER JOIN AppointmentsT ON RoomAppointmentsT.AppointmentID = AppointmentsT.ID "
				"INNER JOIN RoomStatusT ON RoomAppointmentsT.StatusID = RoomStatusT.ID "
				"WHERE RoomAppointmentsT.AppointmentID = {INT} "
				"AND RoomAppointmentsT.StatusID <> -1 ORDER BY RoomAppointmentsT.CheckInTime", (long)dwReservationID);
			if(!rs->eof) {
				long nPatientID = AdoFldLong(rs, "PatientID",-1);
				long nRoomAppointmentID = AdoFldLong(rs, "ID");
				CString strRoomName = AdoFldString(rs, "Name", "");
				long nStatusID = AdoFldLong(rs, "StatusID", -1);
				CString strOldStatus = AdoFldString(rs, "RoomStatusName","");
				BOOL bWaitingRoom = AdoFldBool(rs, "WaitingRoom", FALSE);

				CString str;
				if(nStatusID == 0) { //Ready To Check Out
					str.Format("This appointment is currently marked 'Ready To Check Out' in the Room Manager.\n"
						"Would you like to check the appointment out?");
				}
				else {
					str.Format("This appointment is currently assigned to a room.\n"
						"Would you like to check the appointment out of '%s'?", strRoomName);
				}

				// (j.jones 2010-12-02 15:46) - PLID 38597 - if a waiting room, auto-check out silently,
				// else check the preference
				if(bWaitingRoom ||
					(nRoomClearWhenApptMarkedOut == 1 && IDYES == MessageBox(GetActiveWindow(), str, "Practice", MB_ICONQUESTION|MB_YESNO))) {

					//audit this
					// (a.walling 2013-02-08 12:59) - PLID 55083 - Parameterize
					CParamSqlBatch batch;

					//update the status		
					batch.Add("UPDATE RoomAppointmentsT SET StatusID = -1, LastUpdateTime = GetDate(), LastUpdateUserID = {INT} WHERE ID = {INT}", GetCurrentUserID(), nRoomAppointmentID);
					
					//and log this in the history
					// (a.walling 2013-06-07 09:46) - PLID 57078 - RoomAppointmentHistoryT no longer has an ID column
					batch.Add("INSERT INTO RoomAppointmentHistoryT (RoomAppointmentID, UpdateUserID, StatusID) "
						"VALUES ({INT}, {INT}, -1)", nRoomAppointmentID, GetCurrentUserID());

					batch.Execute(GetRemoteData());

					//now audit the change
					{
						CString strOldValue, strNewValue;

						strOldValue.Format("Room: '%s', Status: '%s'", strRoomName, strOldStatus);
						strNewValue = "Checked Out";

						long nAuditID = BeginNewAuditEvent();
						AuditEvent(nPatientID == -25 ? -1 : nPatientID, nPatientID == -25 ? "" : GetExistingPatientName(nPatientID), nAuditID, aeiRoomApptCheckout, dwReservationID, strOldValue, strNewValue, aepMedium, aetChanged);
					}

					//send a network message
					CClient::RefreshRoomAppointmentTable(nRoomAppointmentID);
				}
				else if(nRoomClearWhenApptMarkedOut == 2) {
					//open room manager
					if(GetMainFrame())
						GetMainFrame()->ShowRoomManager();
				}
				else {
					//do nothing
				}
			}
			rs->Close();	
		}

		if(!bUpdatingLinkedAppt) {
			// (z.manning, 07/26/2007) - PLID 14579 - See if the user wants to update the status of any linked appts.
			UpdateLinkedAppointmentShowState(dwReservationID, assOut, strNewShowState);
		}

		return TRUE;
	}
	NxCatchAll("Error in OnMarkOut()");
	return FALSE;
}
// (a.wilson 2014-08-21 14:33) - PLID 63170 - use the refreshappointmenttable() with all the parameters to prevent extra recordsets.
BOOL AppointmentMarkPending(DWORD dwReservationID, BOOL bUpdatingLinkedAppt /* = FALSE */)
{
	try{

		CString strOldShowState = "", strResourceIDs = "";
		long nPatientID = 0, nOldShowState = -1;
		long nLocationID = -1, nStatus = -1;
		COleDateTime dtStart = g_cdtMin, dtEnd = g_cdtMin;
		// (d.thompson 2010-05-11) - PLID 38581 - Parameterized
		_RecordsetPtr rs = CreateParamRecordset("SELECT Name, PatientID, StartTime, EndTime, Status, ShowState, LocationID, dbo.GetResourceIDString(AppointmentsT.ID) AS ResourceIDs "
			"FROM AppointmentsT INNER JOIN AptShowStateT ON AppointmentsT.ShowState = AptShowStateT.ID WHERE AppointmentsT.ID = {INT}", dwReservationID);
		if(!rs->eof) {
			strOldShowState = AdoFldString(rs, "Name","");
			nOldShowState = AdoFldLong(rs, "ShowState",-1);
			nPatientID = AdoFldLong(rs, "PatientID");
			strResourceIDs = AdoFldString(rs, "ResourceIDs", "");
			nLocationID = AdoFldLong(rs, "LocationID");
			nStatus = (long)AdoFldByte(rs, "Status");
			dtStart = AdoFldDateTime(rs, "StartTime");
			dtEnd = AdoFldDateTime(rs, "EndTime");
		}
		rs->Close();

		long nAuditID = BeginNewAuditEvent();
		CString strNewShowState = "Pending";
		AuditEvent(nPatientID, GetExistingPatientName(nPatientID), nAuditID, aeiApptShowState, dwReservationID, strOldShowState, strNewShowState, aepMedium);
		
		// (a.walling 2013-02-08 12:59) - PLID 55083 - Parameterize
		ExecuteParamSql("UPDATE AppointmentsT SET ShowState = 0, ModifiedDate = GetDate(), ModifiedLogin = {STRING}, NoShowReason = '', NoShowReasonID = NULL WHERE AppointmentsT.ID = {INT}", GetCurrentUserName(), dwReservationID);

		if (nOldShowState == 3) {
			// (a.walling 2006-11-01 09:24) - PLID 23195 - we are removing the former no-show status, so reapply this event
			COleDateTime dtDate = COleDateTime::GetCurrentTime();
			PhaseTracking::CreateAndApplyEvent(PhaseTracking::ET_AppointmentCreated, nPatientID, dtDate, dwReservationID, true, -1);
			PhaseTracking::CreateAndApplyEvent(PhaseTracking::ET_ActualAppointment, nPatientID, dtDate, dwReservationID, true, -1);

			// (z.manning 2015-11-05 12:13) - PLID 57109 - Update recalls needing attention dialog
			GetMainFrame()->HandleRecallChanged();
		}

		// send a network message
		CClient::RefreshAppointmentTable(dwReservationID, nPatientID, dtStart, dtEnd, nStatus, 0, nLocationID, strResourceIDs);
		// Update Microsoft Outlook
		PPCAddAppt((long)dwReservationID);
		// Update PalmSyncT
		UpdatePalmSyncT(dwReservationID);

		// (z.manning 2008-07-16 14:34) - PLID 30490 - Handle any HL7 messages relating to this appointment
		// (r.gonet 12/03/2012) - PLID 54107 - Updated to use refactored function.
		SendUpdateAppointmentHL7Message(dwReservationID);

		//log the history
		LogShowStateTime(dwReservationID, 0);

		if(!bUpdatingLinkedAppt) {
			// (z.manning, 07/26/2007) - PLID 14579 - See if the user wants to update the status of any linked appts.
			UpdateLinkedAppointmentShowState(dwReservationID, assPending, strNewShowState);
		}

		return TRUE;
	}
	NxCatchAll("Error in OnMarkPending()");
	return FALSE;
}
// (a.wilson 2014-08-21 14:33) - PLID 63170 - use the refreshappointmenttable() with all the parameters to prevent extra recordsets.
BOOL AppointmentMarkReceived(DWORD dwReservationID, BOOL bUpdatingLinkedAppt /* = FALSE */)
{
	try{

		CString strOldShowState = "", strResourceIDs = "";
		long nPatientID = 0, nOldShowState = -1;
		long nLocationID = -1, nStatus = -1;
		COleDateTime dtStart = g_cdtMin, dtEnd = g_cdtMin;
		// (d.thompson 2010-05-11) - PLID 38581 - Parameterized
		_RecordsetPtr rs = CreateParamRecordset("SELECT Name, PatientID, StartTime, EndTime, Status, ShowState, LocationID, dbo.GetResourceIDString(AppointmentsT.ID) AS ResourceIDs "
			"FROM AppointmentsT INNER JOIN AptShowStateT ON AppointmentsT.ShowState = AptShowStateT.ID WHERE AppointmentsT.ID = {INT}", dwReservationID);
		if(!rs->eof) {
			strOldShowState = AdoFldString(rs, "Name","");
			nOldShowState = AdoFldLong(rs, "ShowState",-1);
			nPatientID = AdoFldLong(rs, "PatientID");
			strResourceIDs = AdoFldString(rs, "ResourceIDs", "");
			nLocationID = AdoFldLong(rs, "LocationID");
			nStatus = (long)AdoFldByte(rs, "Status");
			dtStart = AdoFldDateTime(rs, "StartTime");
			dtEnd = AdoFldDateTime(rs, "EndTime");
		}
		rs->Close();

		long nAuditID = BeginNewAuditEvent();
		CString strNewShowState = "Received";
		AuditEvent(nPatientID, GetExistingPatientName(nPatientID), nAuditID, aeiApptShowState, dwReservationID, strOldShowState, strNewShowState, aepMedium);
		
		// (a.walling 2013-02-08 12:59) - PLID 55083 - Parameterize
		ExecuteParamSql("UPDATE AppointmentsT SET ShowState = 4, ModifiedDate = GetDate(), ModifiedLogin = {STRING} WHERE AppointmentsT.ID = {INT}", GetCurrentUserName(), dwReservationID);

		if (nOldShowState == 3) {
			// (a.walling 2006-11-01 09:24) - PLID 23195 - we are removing the former no-show status, so reapply this event
			COleDateTime dtDate = COleDateTime::GetCurrentTime();
			PhaseTracking::CreateAndApplyEvent(PhaseTracking::ET_AppointmentCreated, nPatientID, dtDate, dwReservationID, true, -1);
			PhaseTracking::CreateAndApplyEvent(PhaseTracking::ET_ActualAppointment, nPatientID, dtDate, dwReservationID, true, -1);

			// (z.manning 2015-11-05 12:13) - PLID 57109 - Update recalls needing attention dialog
			GetMainFrame()->HandleRecallChanged();
		}

		// send a network message
		CClient::RefreshAppointmentTable(dwReservationID, nPatientID, dtStart, dtEnd, nStatus, 4, nLocationID, strResourceIDs);
		// Update Microsoft Outlook
		PPCAddAppt((long)dwReservationID);
		// Update PalmSyncT
		UpdatePalmSyncT(dwReservationID);

		// (z.manning 2008-07-16 14:34) - PLID 30490 - Handle any HL7 messages relating to this appointment
		// (r.gonet 12/03/2012) - PLID 54107 - Updated to use refactored function.
		SendUpdateAppointmentHL7Message(dwReservationID);

		//log the history
		LogShowStateTime(dwReservationID, 4);

		if(!bUpdatingLinkedAppt) {
			// (z.manning, 07/26/2007) - PLID 14579 - See if the user wants to update the status of any linked appts.
			UpdateLinkedAppointmentShowState(dwReservationID, assReceived, strNewShowState);
		}

		return TRUE;
	}
	NxCatchAll("Error in AppointmentMarkReceived()");
	return FALSE;
}
// (a.wilson 2014-08-21 14:33) - PLID 63170 - use the refreshappointmenttable() with all the parameters to prevent extra recordsets.
BOOL AppointmentMarkUserDefined(DWORD dwReservationID, const CString& strShowState, BOOL bUpdatingLinkedAppt /* = FALSE */)
{
	try{

		CString strOldShowState = "", strResourceIDs = "";
		long nPatientID = 0, nOldShowState = -1;
		long nLocationID = -1, nStatus = -1;
		COleDateTime dtStart = g_cdtMin, dtEnd = g_cdtMin;
		// (a.walling 2013-02-08 12:59) - PLID 55083 - Parameterize
		_RecordsetPtr rs = CreateParamRecordset("SELECT Name, PatientID, StartTime, EndTime, Status, ShowState, LocationID, dbo.GetResourceIDString(AppointmentsT.ID) AS ResourceIDs "
			"FROM AppointmentsT INNER JOIN AptShowStateT ON AppointmentsT.ShowState = AptShowStateT.ID WHERE AppointmentsT.ID = {INT}", dwReservationID);
		if(!rs->eof) {
			strOldShowState = AdoFldString(rs, "Name","");
			nOldShowState = AdoFldLong(rs, "ShowState",-1);
			nPatientID = AdoFldLong(rs, "PatientID");
			strResourceIDs = AdoFldString(rs, "ResourceIDs", "");
			nLocationID = AdoFldLong(rs, "LocationID");
			nStatus = (long)AdoFldByte(rs, "Status");
			dtStart = AdoFldDateTime(rs, "StartTime");
			dtEnd = AdoFldDateTime(rs, "EndTime");
		}
		rs->Close();

		long nAuditID = BeginNewAuditEvent();
		AuditEvent(nPatientID, GetExistingPatientName(nPatientID), nAuditID, aeiApptShowState, dwReservationID, strOldShowState, strShowState, aepMedium);

		// (z.manning, 07/26/2007) - PLID 14579 - Optimization not related to this pl item, but we used to just
		// update here, now we also select the ID here instead of doing it a few lines later.
		// (a.walling 2013-02-08 12:59) - PLID 55083 - Parameterize
		_RecordsetPtr rsStatus = CreateParamRecordset(
			"SET NOCOUNT ON "
			"DECLARE @nShowStateID INT "
			"SET @nShowStateID = (SELECT ID FROM AptShowStateT WHERE Name = {STRING}) "
			"UPDATE AppointmentsT SET ShowState = @nShowStateID, ModifiedDate = GetDate(), ModifiedLogin = {STRING} "
			"WHERE AppointmentsT.ID = {INT} "
			"SET NOCOUNT OFF "
			"SELECT @nShowStateID AS ID"
			, strShowState, GetCurrentUserName(), dwReservationID);

		if (nOldShowState == 3) {
			// (a.walling 2006-11-01 09:24) - PLID 23195 - we are removing the former no-show status, so reapply this event
			COleDateTime dtDate = COleDateTime::GetCurrentTime();
			PhaseTracking::CreateAndApplyEvent(PhaseTracking::ET_AppointmentCreated, nPatientID, dtDate, dwReservationID, true, -1);
			PhaseTracking::CreateAndApplyEvent(PhaseTracking::ET_ActualAppointment, nPatientID, dtDate, dwReservationID, true, -1);

			// (z.manning 2015-11-05 12:13) - PLID 57109 - Update recalls needing attention dialog
			GetMainFrame()->HandleRecallChanged();
		}

		// Update Microsoft Outlook
		PPCAddAppt((long)dwReservationID);
		// Update PalmSyncT
		UpdatePalmSyncT(dwReservationID);

		// (z.manning 2008-07-16 14:34) - PLID 30490 - Handle any HL7 messages relating to this appointment
		// (r.gonet 12/03/2012) - PLID 54107 - Updated to use refactored function.
		SendUpdateAppointmentHL7Message(dwReservationID);

		long nShowState = -1;
		if (! rsStatus->eof) {

			nShowState = AdoFldLong(rsStatus, "ID");

			// (d.lange 2015-02-17 15:51) - PLID 64922 - Update the show state with the correct ID
			LogShowStateTime(dwReservationID, nShowState);
		}
		else {
			ASSERT(FALSE);
		}

		// send a network message
		CClient::RefreshAppointmentTable(dwReservationID, nPatientID, dtStart, dtEnd, nStatus, nShowState, nLocationID, strResourceIDs);

		if(!bUpdatingLinkedAppt) {
			// (z.manning, 07/26/2007) - PLID 14579 - See if the user wants to update the status of any linked appts.
			// (r.farnworth 2015-04-28 09:44) - PLID 65752 - We were improperly passing in nStatus as the ID to set the show state to. This was causing all linked appointments to update to In.
			UpdateLinkedAppointmentShowState(dwReservationID, nShowState, strShowState);
		}

		return TRUE;
	}
	NxCatchAll("Error in AppointmentMarkUserDefined()");
	return FALSE;
}

// (a.wilson 2014-08-21 14:33) - PLID 63170 - use the refreshappointmenttable() with all the parameters to prevent extra recordsets.
BOOL AppointmentMarkMoveUp(DWORD dwReservationID, BOOL bSetMoveUp)
{
	try{
		long nPatientID = 0, nShowState = -1;
		long nLocationID = -1, nStatus = -1;
		CString strResourceIDs = "";
		COleDateTime dtStart = g_cdtMin, dtEnd = g_cdtMin;
		// (a.walling 2013-02-08 12:59) - PLID 55083 - Parameterize
		_RecordsetPtr rs = CreateParamRecordset("SELECT PatientID, StartTime, EndTime, Status, ShowState, LocationID, dbo.GetResourceIDString(AppointmentsT.ID) AS ResourceIDs "
			"FROM AppointmentsT WHERE ID = {INT}", dwReservationID);
		if(!rs->eof) {
			nShowState = AdoFldLong(rs, "ShowState");
			nPatientID = AdoFldLong(rs, "PatientID");
			strResourceIDs = AdoFldString(rs, "ResourceIDs", "");
			nLocationID = AdoFldLong(rs, "LocationID");
			nStatus = (long)AdoFldByte(rs, "Status");
			dtStart = AdoFldDateTime(rs, "StartTime");
			dtEnd = AdoFldDateTime(rs, "EndTime");
		}
		rs->Close();

		long nAuditID = BeginNewAuditEvent();
		AuditEvent(nPatientID, GetExistingPatientName(nPatientID), nAuditID, aeiApptMoveUp, dwReservationID, "", bSetMoveUp ? "Move Up" : "Removed Move-Up", aepMedium);
		
		// (d.moore 2007-05-22 09:19) - PLID 4013 - MoveUp functionality has been modified to use a waiting list instead.
		//ExecuteSql("UPDATE AppointmentsT SET MoveUp = %li, ModifiedDate = GetDate(), ModifiedLogin = '%s' WHERE AppointmentsT.ID = %li", bSetMoveUp ? 1 : 0, _Q(GetCurrentUserName()), dwReservationID);
		// (a.walling 2013-02-08 12:59) - PLID 55083 - Parameterize
		ExecuteParamSql("UPDATE AppointmentsT SET ModifiedDate = GetDate(), ModifiedLogin = {STRING} WHERE AppointmentsT.ID = {INT}", GetCurrentUserName(), dwReservationID);
		
		CString strQuery;
		if (bSetMoveUp) {
			// Check to make sure that this appointment isn't already somehow associated
			//  with an entry in the waiting list.
			// (a.walling 2013-02-08 12:59) - PLID 55083 - Parameterize
			_RecordsetPtr prs = CreateParamRecordset(
				"SELECT ID FROM WaitingListT WHERE AppointmentID = {INT}", dwReservationID);
			if (!prs->eof) {
				AfxMessageBox("This appointment is already on the waiting list.");
				return FALSE;
			}
			
			//#pragma TODO("Good candidate for parameterization, but too large a change for this situation")
			// Information about the appointment needs to be copied into the waiting list tables.
			// Store the current date (with out time). This is used several times.
			strQuery.Format(
				"SET NOCOUNT ON; \r\n"
				"DECLARE @TodaysDate nvarchar(10); "
				"SET @TodaysDate = Convert(nvarchar(10), GetDate(), 101); "
				"\r\n"
				"DECLARE @Wait_List_ID INT; \r\n"
				"DECLARE @Wait_List_Item_ID INT; \r\n"

				// Copy the basic appointment data into the WaitingList table.
				"INSERT INTO WaitingListT(AppointmentID, PatientID, TypeID, CreatedDate, Notes) "
				"SELECT ID, PatientID, "
					"(SELECT ID FROM AptTypeT WHERE ID = AptTypeID), "
					"@TodaysDate, Notes "
				"FROM AppointmentsT WHERE ID=%li;"
				"\r\n"

				"SET @Wait_List_ID = @@IDENTITY; \r\n"
				
				// Copy all appointment purpose data
				"INSERT INTO WaitingListPurposeT(WaitingListID, PurposeID) "
				"SELECT @Wait_List_ID, PurposeID "
				"FROM AppointmentPurposeT "
				"WHERE AppointmentID=%li; "
				"\r\n"
				
				//Make an entry for the appointment in the line items table.
				"INSERT INTO WaitingListItemT "
					"(WaitingListID, StartDate, EndDate, "
					"StartTime, EndTime, AllResources) "
				"SELECT @Wait_List_ID, "
				"CASE WHEN AppointmentsT.[Date] < @TodaysDate THEN AppointmentsT.[Date] ELSE @TodaysDate END, "
				"AppointmentsT.[Date], '5:00 AM', '10:00 PM', 0 "
				"FROM AppointmentsT "
				"WHERE ID=%li;"
				"\r\n"

				"SET @Wait_List_Item_ID = @@IDENTITY; \r\n"
				
				// Copy all appointment resource data.
				"INSERT INTO WaitingListItemResourceT "
					"(ItemID, ResourceID) "
				"SELECT @Wait_List_Item_ID, ResourceID  "
				"FROM AppointmentResourceT "
				"WHERE AppointmentID=%li;"
				"\r\n"
				"SET NOCOUNT OFF", 
				dwReservationID, dwReservationID, 
				dwReservationID, dwReservationID);
		} else {
			// Delete the entry from the waiting list.
			//#pragma TODO("Good candidate for parameterization, but too large a change for this situation")
			strQuery = CWaitingListUtils::BuildWaitingListDeleteQuery(dwReservationID);
		}
		ExecuteSqlStd(strQuery);

		// send a network message
		CClient::RefreshAppointmentTable(dwReservationID, nPatientID, dtStart, dtEnd, nStatus, nShowState, nLocationID, strResourceIDs);
		// Update Microsoft Outlook
		PPCAddAppt((long)dwReservationID);
		// Update PalmSyncT
		UpdatePalmSyncT(dwReservationID);

		// (c.haag 2010-09-08 10:25) - PLID 37734 - Preference for opening the waiting list when flagging an appointment as move up
		if (bSetMoveUp) {
			int nWaitingListAction = GetRemotePropertyInt("ApptMoveUpOpenWaitingList", 2, 0, GetCurrentUserName(), true);
			BOOL bOpenWaitingList = FALSE;
			switch (nWaitingListAction)
			{
			case 0: // Do not open the waiting list
				break;
			case 1: // Open the waiting list
				bOpenWaitingList = TRUE;
				break;
			case 2: // Prompt to open the waiting list
				if (IDYES == AfxMessageBox("Would you like to open the waiting list now?", MB_YESNO | MB_ICONQUESTION))
				{
					bOpenWaitingList = TRUE;
				}
				break;
			}
			if (bOpenWaitingList) {
				if(GetMainFrame()->FlipToModule(SCHEDULER_MODULE_NAME)) {
					CNxTabView* pView = (CNxTabView *)GetMainFrame()->GetOpenView(SCHEDULER_MODULE_NAME);
					if (pView) {
						pView->PostMessage(WM_COMMAND, ID_SHOW_MOVEUP_LIST);
					}
				}
			}
		}

		return TRUE;
	}
	NxCatchAll("Error in AppointmentMarkMoveUp()");
	return FALSE;
}

// (a.wilson 2014-08-21 14:33) - PLID 63170 - use the refreshappointmenttable() with all the parameters to prevent extra recordsets.
//(e.lally 2009-09-28) PLID 18809 - Switched the confirm parameter into an enum instead of boolean
BOOL AppointmentMarkConfirmed(DWORD dwReservationID, EAppointmentConfirmState acsNewConfirmState)
{
	try{
		long nPatientID = 0;
		EAppointmentConfirmState acsOldConfirmState = acsUnconfirmed;
		long nLocationID = -1, nStatus = -1, nState = -1;
		CString strOldStateName = "", strResourceIDs = "";
		COleDateTime dtStart = g_cdtMin, dtEnd = g_cdtMin;
		// (d.thompson 2010-05-11) - PLID 38581 - Parameterized
		_RecordsetPtr rs = CreateParamRecordset("SELECT PatientID, Confirmed, StartTime, EndTime, Status, ShowState, LocationID, dbo.GetResourceIDString(AppointmentsT.ID) AS ResourceIDs "
			"FROM AppointmentsT WHERE ID = {INT}", dwReservationID);
		if(!rs->eof) {
			nPatientID = AdoFldLong(rs, "PatientID");
			acsOldConfirmState = (EAppointmentConfirmState)AdoFldLong(rs, "Confirmed");
			strResourceIDs = AdoFldString(rs, "ResourceIDs", "");
			nLocationID = AdoFldLong(rs, "LocationID");
			nStatus = (long)AdoFldByte(rs, "Status");
			nState = AdoFldLong(rs, "ShowState");
			dtStart = AdoFldDateTime(rs, "StartTime");
			dtEnd = AdoFldDateTime(rs, "EndTime");
		}
		rs->Close();

		long nAuditID = BeginNewAuditEvent();
		//(e.lally 2005-06-09) PLID 15285 - Audit value should show old value as "Unconfirmed" or "Confirmed".
			//At this time there is no right-click "Mark as Left Message" function.
		//(e.lally 2009-09-28) PLID 18809 - added the mark as Left Message function, so this function now uses the enumerated old and new confirm states
		CString strOldValue, strNewValue;
		switch(acsOldConfirmState){
			case acsUnconfirmed:
				strOldValue = "Unconfirmed";
				break;
			case acsConfirmed:
				strOldValue = "Confirmed";
				break;
			case acsLeftMessage:
				strOldValue = "Left Message";
				break;
			default:
				ASSERT(FALSE);
				strOldValue = "Unknown";
				break;
		}
		switch(acsNewConfirmState){
			case acsUnconfirmed:
				strNewValue = "Unconfirmed";
				break;
			case acsConfirmed:
				strNewValue = "Confirmed";
				break;
			case acsLeftMessage:
				strNewValue = "Left Message";
				break;
			default:
				ASSERT(FALSE);
				strNewValue = "Unknown";
				break;
		}

		AuditEvent(nPatientID, GetExistingPatientName(nPatientID), nAuditID, aeiApptConfirm, dwReservationID, strOldValue, strNewValue, aepMedium);
		
		//(e.lally 2009-09-28) PLID 18809 - Parameterized
		// (j.armen 2011-07-01 15:42) - PLID 44205 - added field for confirmed by
		// (j.dinatale 2013-05-15 13:48) - PLID 46038 - if the status is marked as left message, we want to grab the current date as the LastLM.
		// (d.lange 2015-02-23 10:29) - PLID 64955 - Update the LastLM field with the current date and time
		CSqlFragment sqlLM;
		if(acsNewConfirmState == acsLeftMessage){
			sqlLM = ", LastLM = GetDate() ";
		}
		ExecuteParamSql("UPDATE AppointmentsT SET Confirmed = {INT}, ConfirmedBy = {STRING}, ModifiedDate = GetDate(), ModifiedLogin = {STRING} {SQL} WHERE AppointmentsT.ID = {INT}", acsNewConfirmState, acsNewConfirmState == acsConfirmed ? GetCurrentUserName() : "", GetCurrentUserName(), sqlLM, dwReservationID);

		// send a network message
		CClient::RefreshAppointmentTable(dwReservationID, nPatientID, dtStart, dtEnd, nStatus, nState, nLocationID, strResourceIDs);
		// Update Microsoft Outlook
		PPCAddAppt((long)dwReservationID);
		// Update PalmSyncT
		UpdatePalmSyncT(dwReservationID);
		// (r.gonet 2015-06-15 12:07) - PLID 66202 - Send an update appointment message.
		SendUpdateAppointmentHL7Message(dwReservationID);

		return TRUE;
	}
	NxCatchAll("Error in AppointmentMarkConfirmed()");
	return FALSE;
}

BOOL AppointmentLink(DWORD nSrcResID, DWORD nDstResID, BOOL bQuietMode)
{
	try {
		// (d.thompson 2010-06-04) - PLID 39020 - Parameterized and grouped the select queries into 1 trip
		CParamSqlBatch sql;
		sql.Add("SELECT GroupID FROM AptLinkT WHERE AppointmentID = {INT}", nSrcResID);
		sql.Add("SELECT GroupID FROM AptLinkT WHERE AppointmentID = {INT}", nDstResID);
		_RecordsetPtr prs = sql.CreateRecordset(GetRemoteData());

		long nGroupID = -1;
		if (!prs->eof)
			nGroupID = AdoFldLong(prs, "GroupID");
		
		// Find out if the selected appointment is a member of another group
		prs = prs->NextRecordset(NULL);
		if (prs->eof)
		{
			if (nGroupID == -1)
			{
				// Neither of our appointments of interest are in a group. Lets link them together.

				// (c.haag 2003-10-29 09:41) - GroupID is not an identity column, but we do
				// need to get a unique number out of it. It looks like we never have problems
				// with key constraints in auditing because it uses a transaction to ensure
				// the resultant ID is unique. So, I'm going to use that code as a role model
				// for getting the GroupID value.
				// (d.thompson 2010-06-04) - PLID 39020 - Parameterized.  Reworked formatting a little, 
				//	no need for the weird recordset / string setups.  Combined the 2 executes into 1.
				{
					CParamSqlBatch sql;
					sql.Declare("DECLARE @nNewID INT \r\n");
					sql.Add("SET @nNewID = (SELECT COALESCE(Max(GroupID), 0) + 1 FROM AptLinkT) \r\n");
					sql.Add("INSERT INTO AptLinkT (GroupID, AppointmentID) VALUES (@nNewID, {INT}) \r\n", nDstResID);
					// (d.thompson 2010-06-04) - PLID 39020 - Just do all the inserts at the same time.
					sql.Add("INSERT INTO AptLinkT (GroupID, AppointmentID) VALUES (@nNewID, {INT})", nSrcResID);

					sql.Execute(GetRemoteData());
				}
			}
			else
			{
				// Add this appointment to our group
				// (d.thompson 2010-06-04) - PLID 39020 - Parameterized
				ExecuteParamSql("INSERT INTO AptLinkT (GroupID, AppointmentID) VALUES ({INT}, {INT})",
					nGroupID, nDstResID);
			}
		}
		else // Yes, it is a member of another group
		{
			if (nGroupID != -1)
			{ 
				// If we are a member of a group too, ask if we want to combine them
				if (bQuietMode || (IDOK == MsgBox(MB_OKCANCEL, "The appointment you have selected is already in another group. This operation would require the two groups to be merged. Are you sure you wish to do this?")))
				{
					// (d.thompson 2010-06-04) - PLID 39020 - Parameterized
					ExecuteParamSql("UPDATE AptLinkT SET GroupID = {INT} WHERE GroupID = {INT}",
						AdoFldLong(prs, "GroupID"), nGroupID);
				}
			}
			else
			{
				// Add this appointment to that group
				// (d.thompson 2010-06-04) - PLID 39020 - Parameterized
				ExecuteParamSql("INSERT INTO AptLinkT (GroupID, AppointmentID) VALUES ({INT}, {INT})",
					AdoFldLong(prs, "GroupID"), nSrcResID);
			}
		}
		return FALSE;
	}
	NxCatchAll("Error editing linked appointment");
	return TRUE;
}

void AttemptAppointmentLinkAction(long nResID, long nDurationChange /*=0*/)
{
	// (d.thompson 2010-05-14) - PLID 38643 - Moved from below so we can do all our auditing in a transaction.
	long nAuditTransactionID = -1;

	//DRT 4/14/03 - Make sure this appt is actually in a group!
	try {
		//DRT 6/30/03 - Added filter to get rid of cancelled appts.  It's possible one, somehow, got snuck in as being linked, and we're not 
		//		moving them or showing them anyways, so don't even popup the dialog if so.
		// (d.thompson 2010-05-14) - PLID 38643 - Parameterized.
		// (d.thompson 2010-05-14) - PLID 38643 - Added Date so we don't need to query again later.
		_RecordsetPtr prs = CreateParamRecordset("SELECT GroupID, AptLinkT.ID, StartTime, Date FROM AptLinkT LEFT JOIN AppointmentsT ON AppointmentsT.ID = AptLinkT.AppointmentID WHERE AppointmentID = {INT} AND AppointmentID IN (SELECT ID FROM AppointmentsT WHERE Status <> 4)", nResID);
		enum { eMoveSameDayOnly, eMoveAll } eMoveType;

		if(!prs->eof) {
			// If an appointment moved by less than a 24 hour span, we only move
			// linked appointments on the same day. Otherwise, we move them all
			// in daily increments.
			COleDateTime dtNew = AdoFldDateTime(prs, "StartTime");
			COleDateTime dtOld = dtNew - COleDateTimeSpan(0,0,nDurationChange,0);
			dtNew.SetDate(dtNew.GetYear(), dtNew.GetMonth(), dtNew.GetDay());
			dtOld.SetDate(dtOld.GetYear(), dtOld.GetMonth(), dtOld.GetDay());

			if (dtNew == dtOld) {
				eMoveType = eMoveSameDayOnly;
			}
			else
			{
				eMoveType = eMoveAll;
				//(z.manning, PLID 16947, 07/12/05)
				//This preference concerns moving linked appts across days, and whether or not to change the times relatively,
				// meaning if the pref is disabled, and someone moves an appt by 2 days, 1 hour, and 15 mins, then linked appts
				// will also move by 2 days, 1 hour, and 15 mins instead of moving by exactly 2 days when this pref is on.
				//(On by default as this is how it behaved before this preference)
				if(0 == GetRemotePropertyInt("LinkedApptsMoveSameDuration", 0, 0, "<None>", true)) {
					if (dtNew > dtOld) {
						nDurationChange = (dtNew - dtOld).GetDays() * 60 * 24;
					}
					else {
						nDurationChange = -(dtOld - dtNew).GetDays() * 60 * 24;
					}
				}
			}

			//it's in a group, we're all set to go

			// See if the user wants us to open the prompt
			if (GetRemotePropertyInt("PromptOnModifyLinkedAppts", 0, 0, "<None>", true))
			{
				CResLinkActionDlg dlg(NULL);
				// (d.thompson 2010-05-14) - PLID 38643 - Moved this query into the first one.  We're already querying the same
				//	appointment information, so just get the data out of there.  Also got rid of the eof/invalid type check, there
				//	is no way the appointment doesn't exist, or we wouldn't be here in the first place.
				//_RecordsetPtr prsDate = CreateRecordset("SELECT [Date], StartTime FROM AppointmentsT WHERE ID = %d", nResID);
				dlg.m_dtPivot = AdoFldDateTime(prs, "Date");
				dlg.m_nDurationChange = nDurationChange;

				// (a.wilson 2014-08-12 14:40) - PLID 63199 - unneccessary.
				dlg.Open(nResID);
				//	GetMainFrame()->PostMessage(WM_TABLE_CHANGED, NetUtils::AppointmentsT, nResID);
			}
			else
			{
				// See if the user wants us to move all linked appointments
				// (j.jones 2010-10-05 09:34) - PLID 40075 - This code branch used to be controlled by
				// a long obsolete preference to auto-move linked appointments, which was formerly mutually
				// exclusive to the preference to prompt. Now, it is just the normal code branch when
				// prompting is disabled.
				if(nDurationChange)
				{
					// (d.thompson 2010-05-14) - PLID 38643 - Parameterized both queries
					_RecordsetPtr prsAppts;
					switch (eMoveType)
					{
					case eMoveSameDayOnly: // Move all the linked appointments in the same day only
						prsAppts = CreateParamRecordset("SELECT AppointmentID, PatientID, Status, Date, StartTime, EndTime, ArrivalTime, "
							"AptTypeT.Name, dbo.GetPurposeString(AppointmentID) AS Purposes "
							"FROM AptLinkT LEFT JOIN AppointmentsT ON AppointmentsT.ID = AptLinkT.AppointmentID "
							"LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
							"WHERE AptLinkT.GroupID = {INT} AND AptLinkT.AppointmentID <> {INT} AND "
							"AppointmentsT.Status <> 4 AND AppointmentsT.[Date] = "
							"(SELECT [Date] FROM AppointmentsT WHERE ID = {INT})",
							AdoFldLong(prs, "GroupID"), nResID, nResID);
						break;
					case eMoveAll: // Move all the linked appointments
						prsAppts = CreateParamRecordset("SELECT AppointmentID, PatientID, Status, Date, StartTime, EndTime, Arrivaltime, "
							"AptTypeT.Name, dbo.GetPurposeString(AppointmentID) AS Purposes FROM AptLinkT "
							"LEFT JOIN AppointmentsT ON AppointmentsT.ID = AptLinkT.AppointmentID "
							"LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
							"WHERE AptLinkT.GroupID = {INT} AND AptLinkT.AppointmentID <> {INT} AND "
							"AppointmentsT.Status <> 4 ",
							AdoFldLong(prs, "GroupID"), nResID);
						break;
					}

					if (!prsAppts->eof)
					{
						CString strPrompt;
						CString strTimeFrame;
						long nAbsDurationChange = nDurationChange >= 0 ? nDurationChange : -nDurationChange;
						long nWeeks = nAbsDurationChange / (60*24*7);
						long nDays = (nAbsDurationChange % (60*24*7)) / (60*24);
						long nHours = (nAbsDurationChange % (60*24)) / 60;
						long nMinutes = nAbsDurationChange % 60;
						if (nWeeks)
						{
							if (nWeeks == 1)
								strTimeFrame = "1 week ";
							else
								strTimeFrame.Format("%d Weeks ", nDays);
						}
						if (nDays)
						{
							CString str;
							if (nDays == 1)
								str = "1 day ";
							else
								str.Format("%d days ", nDays);
							if (strTimeFrame.IsEmpty())
								strTimeFrame = str;
							else
								strTimeFrame += "and " + str;
						}
						if (nHours)
						{
							CString str;
							if (nHours == 1)
								str = "1 hour ";
							else
								str.Format("%d hours ", nHours);
							if (strTimeFrame.IsEmpty())
								strTimeFrame = str;
							else
								strTimeFrame += "and " + str;
						}
						if (nMinutes)
						{
							CString str;
							if (nMinutes == 1)
								str = "1 minute ";
							else
								str.Format("%d minutes ", nMinutes);
							if (strTimeFrame.IsEmpty())
								strTimeFrame = str;
							else
								strTimeFrame += "and " + str;
						}
						
						strPrompt.Format("The following appointments have also been moved by %s(The times below describe when the appointment was beforehand):\r\n\r\n", strTimeFrame);

						// (d.thompson 2010-05-14) - PLID 38643 - Moved from below so we can do all our auditing in a trip
						nAuditTransactionID = BeginAuditTransaction();

						// (d.thompson 2010-05-14) - PLID 38643 - Some queries don't need to be run in the batch, so save them 
						//	for after the while loop to execute.
						CString strBatchedSql = BeginSqlBatch();
						CNxParamSqlArray batchArgs;

						while (!prsAppts->eof)
						{
							CString strAdd;
							COleDateTime dtOldDate = AdoFldDateTime(prsAppts, "Date");
							COleDateTime dtOldStart = AdoFldDateTime(prsAppts, "StartTime");
							COleDateTime dtOldEnd = AdoFldDateTime(prsAppts, "EndTime");
							COleDateTime dtOldArrival = AdoFldDateTime(prsAppts, "ArrivalTime");
							CString strType = AdoFldString(prsAppts, "Name", "");
							CString strPurpose = AdoFldString(prsAppts, "Purposes", "");
							long nCurID = AdoFldLong(prsAppts, "AppointmentID");
							long nPatientID = AdoFldLong(prsAppts, "PatientID", -1);

							// Make sure the arrival time and start time have the same date.
							COleDateTime dtNewStart = dtOldStart + COleDateTimeSpan(0, 0, nDurationChange, 0);
							COleDateTime dtNewArrival = dtOldArrival + COleDateTimeSpan(0, 0, nDurationChange, 0);
							if(dtNewStart.GetDay() != dtNewArrival.GetDay()) {
								dtNewArrival.SetDateTime(dtNewStart.GetYear(), dtNewStart.GetMonth(), dtNewStart.GetDay(), 0, 0, 0);
							}

							// Build our message prompt further
							strAdd.Format("%s-%s %s %s", FormatDateTimeForInterface(dtOldStart,DTF_STRIP_SECONDS), FormatDateTimeForInterface(dtOldEnd, DTF_STRIP_SECONDS, dtoTime), strType, strPurpose);
							strPrompt += strAdd + "\r\n";

							// (c.haag 2010-04-01 15:10) - PLID 38005 - Delete from the appointment reminder table
							// (d.thompson 2010-05-14) - PLID 38643 - Combined and parameterized these updates
							CParamSqlBatch sql;
							sql.Declare("SET NOCOUNT ON");
							sql.Add("UPDATE AppointmentsT SET StartTime = DATEADD(minute, {INT}, StartTime), "
							"EndTime = DATEADD(minute, {INT}, EndTime), "
							"ArrivalTime = {STRING} WHERE ID = {INT}", nDurationChange, nDurationChange, FormatDateTimeForSql(dtNewArrival), nCurID);
							sql.Add("DELETE FROM AppointmentRemindersT WHERE AppointmentID = {INT}", nCurID);
							sql.Add("UPDATE AppointmentsT SET Date = CONVERT(datetime, CONVERT(nvarchar, StartTime, 101)) WHERE ID = {INT}", nCurID);
							sql.Declare("SET NOCOUNT OFF");
							// (d.thompson 2010-05-14) - PLID 38643 - Moved up from below for auditing purposes
							sql.Add("SELECT Date, StartTime, EndTime, Status, ShowState, PatientID, LocationID, dbo.GetResourceIDString(ID) AS ResourceIDs FROM AppointmentsT WHERE ID = {INT}", nCurID);
							// (d.thompson 2010-05-14) - PLID 38643 - Moved up from superbill void prompt below
							sql.Add("SELECT SavedID FROM PrintedSuperbillsT WHERE ReservationID = {INT} AND Void = 0", nCurID);
							// (d.thompson 2010-05-14) - PLID 38643 - Moved up from ASC check below.  Also it was using the result of the new date, 
							//	I replaced that with dtNewStart.
							// (a.walling 2008-06-04 15:41) - PLID 29900 - Grab the personid here
							sql.Add("SELECT ID, SurgeryDate, PersonID FROM CaseHistoryT WHERE AppointmentID = {INT} AND SurgeryDate <> {STRING}", 
								nCurID, FormatDateTimeForSql(COleDateTime(dtNewStart.GetYear(), dtNewStart.GetMonth(), dtNewStart.GetDay(), 0, 0, 0), dtoDate));
							_RecordsetPtr rs = sql.CreateRecordset(GetRemoteData());

							COleDateTime dtNewDate, dtNewTime, dtNewEndTime;
							BYTE nStatus = 1;
							long nShowState = 0;
							long nLocationID = -1;
							CString strResourceIDs;
							if(!rs->eof) {
								dtNewDate = AdoFldDateTime(rs, "Date");
								dtNewTime = AdoFldDateTime(rs, "StartTime");
								dtNewEndTime = AdoFldDateTime(rs, "EndTime");
								nStatus = AdoFldByte(rs, "Status");
								nShowState = AdoFldLong(rs, "ShowState");
								nLocationID = AdoFldLong(rs, "LocationID");
								strResourceIDs = AdoFldString(rs, "ResourceIDs", "");

								// (z.manning, 10/27/05, PLID 17501)
								// Audit when linked appts are moved.
								// (d.thompson 2010-05-14) - PLID 38643 - Use audit transactions to limit database trips
								//long nAuditID = BeginNewAuditEvent();
								CString strOldStart = FormatDateTimeForInterface(dtOldStart);
								CString strOldEnd = FormatDateTimeForInterface(dtOldEnd);
								CString strOldArrival = FormatDateTimeForInterface(dtOldArrival);
								CString strNewStart = FormatDateTimeForInterface(dtNewTime);
								CString strNewEnd = FormatDateTimeForInterface(dtNewEndTime);
								CString strNewArrival = FormatDateTimeForInterface(dtNewArrival);
								CString strPatientName = GetExistingPatientName(AdoFldLong(rs->Fields->Item["PatientID"]));
								if(strNewStart != strOldStart) { //These are formatted in the same way above, so they should match if the same
									// (d.thompson 2010-05-14) - PLID 38643 - Audit in a transaction instead of immediate
									AuditEvent(AdoFldLong(rs->Fields->Item["PatientID"]), strPatientName, nAuditTransactionID, aeiApptStartTime, nCurID, strOldStart, strNewStart, aepHigh);
								}
								if(strNewEnd != strOldEnd) {
									// (d.thompson 2010-05-14) - PLID 38643 - Audit in a transaction instead of immediate
									AuditEvent(AdoFldLong(rs->Fields->Item["PatientID"]), strPatientName, nAuditTransactionID, aeiApptEndTime, nCurID, strOldEnd, strNewEnd, aepHigh);
								}
								if(strNewArrival != strOldArrival) {
									// (d.thompson 2010-05-14) - PLID 38643 - Audit in a transaction instead of immediate
									AuditEvent(AdoFldLong(rs->Fields->Item["PatientID"]), strPatientName, nAuditTransactionID, aeiApptArrivalTime, nCurID, strOldArrival, strNewArrival, aepMedium);
								}

								// Make sure the new appointment location is valid
								long nNewLocationID = nLocationID;
								if (GetValidAppointmentLocation(nLocationID, nNewLocationID, nCurID, dtNewDate, dtNewStart))
								{
									// If the appointment was dropped into a new location template, make sure the LocationID is updated
									if (nLocationID != nNewLocationID)
									{
										// Audit the location change for the appointment
										CString strOldLocation = GetLocationName(nLocationID);
										CString strNewLocation = GetLocationName(nNewLocationID);
										AuditEvent(AdoFldLong(rs->Fields->Item["PatientID"]), strPatientName, nAuditTransactionID, aeiApptLocation, nCurID, strOldLocation, strNewLocation, aepHigh);
										nLocationID = nNewLocationID;
										AddParamStatementToSqlBatch(strBatchedSql, batchArgs, "UPDATE AppointmentsT SET LocationID = {INT} WHERE ID = {INT};", nLocationID, nCurID);
									}
								}
							}

							// (d.thompson 2010-05-14) - PLID 38643 - We now pull the next recordset above to save a trip.  Move to it now.
							rs = rs->NextRecordset(NULL);

							//DRT 10/24/2005 - PLID 16664 - If the preference is enabled for automatically moving
							//	linked appointments, we need to look for superbill IDs to void.
							//compare YMD - if any are different it moved days.
							if( (dtNewDate.GetYear() != dtOldDate.GetYear() || dtNewDate.GetMonth() != dtOldDate.GetMonth() || dtNewDate.GetDay() != dtOldDate.GetDay()) && (GetRemotePropertyInt("ShowVoidSuperbillPrompt", 1, 0, "<None>", false) == 1) && (GetCurrentUserPermissions(bioVoidSuperbills) & sptWrite)) {
								//The date has changed, now look for superbills
								// (d.thompson 2010-05-14) - PLID 38643 - Moved this query into the batch above.
								//_RecordsetPtr prsSuperbill = CreateRecordset("SELECT SavedID FROM PrintedSuperbillsT WHERE ReservationID = %li AND Void = 0", nCurID);
								CString strSuperbillIDs = "";
								long nSuperbillIDCnt = 0;
								while(!rs->eof) {
									long nID = AdoFldLong(rs, "SavedID", -1);
									if(nID > -1) {
										CString str;	str.Format("%li, ", nID);
										strSuperbillIDs += str;
										nSuperbillIDCnt++;
									}

									rs->MoveNext();
								}
								strSuperbillIDs.TrimRight(", ");

								if(nSuperbillIDCnt > 0) {
									//They are tied to superbills, we will warn the user and give them an opportunity to give up
									CString strPurp = strType + " - " + strPurpose;
									CString strDate = FormatDateTimeForInterface(dtOldStart,DTF_STRIP_SECONDS);

									CString strFmt;
									strFmt.Format("A linked appointment (currently %s for %s) is tied to %li superbill%s (ID%s:  %s).  "
										"Do you wish to mark these superbills as VOID?\r\n\r\n"
										" - If you choose YES, all related superbill IDs will be marked VOID.\r\n"
										" - If you choose NO, the superbills will remain tied to this appointment.", 
										strPurp, strDate, nSuperbillIDCnt, nSuperbillIDCnt == 1 ? "" : "s", nSuperbillIDCnt == 1 ? "" : "s", strSuperbillIDs);

									if(AfxMessageBox(strFmt, MB_YESNO) == IDYES) {
										//void these superbills
										// (d.thompson 2010-05-14) - PLID 38643 - Use the batch outside the while loop, we don't need
										//	to make trips while looping.
										AddParamStatementToSqlBatch(strBatchedSql, batchArgs, "UPDATE PrintedSuperbillsT SET Void = 1, VoidDate = GetDate(), VoidUser = {STRING} WHERE PrintedSuperbillsT.ReservationID = {INT} AND Void = 0", GetCurrentUserName(), nCurID);
									}
								}
							}

							// (d.thompson 2010-05-14) - PLID 38643 - We now query the recordset above in batch, so move forward
							rs = rs->NextRecordset(NULL);

							//update case history dates, if needed
							if(IsSurgeryCenter(false)) {
								if(dtOldDate != dtNewDate) {
									// (d.thompson 2010-05-14) - PLID 38643 - Moved this into the batch above to avoid extra trips.  Also quit
									//	using a newly generated dtDateOnly, used the value we already have in this function.
									//COleDateTime dtDateOnly;
									//dtDateOnly.SetDate(dtNewDate.GetYear(),dtNewDate.GetMonth(),dtNewDate.GetDay());
									//_RecordsetPtr rs = CreateRecordset("SELECT Count(ID) AS CountOfID FROM CaseHistoryT WHERE AppointmentID = %li AND SurgeryDate <> '%s'", nCurID, FormatDateTimeForSql(dtDateOnly,dtoDate));
									// (d.thompson 2010-05-14) - PLID 38643 - We no longer run a query to get a count -- just look for EOF
									//	in the "case histories" query above in the batch.
									if(!rs->eof) {
										CString str;
										if(rs->GetRecordCount() > 1) {
											str.Format("There are %li case histories that are attached to one of your linked appointments and specify a different surgery date.\n"
												"Would you like to update the surgery date on these case histories to reflect the new appointment date?", rs->GetRecordCount());
										}
										else {
											str = "There is a case history that is attached to one of your linked appointments and specifies a different surgery date.\n"
												"Would you like to update the surgery date on this case history to reflect the new appointment date?";
										}
										if(IDYES == MsgBox(MB_ICONQUESTION|MB_YESNO,str)) {
											//TES 1/9/2007 - PLID 23575 - Go through each invididually, to audit.
											// (d.thompson 2010-05-14) - PLID 38643 - Rolled this into the above, we don't need to query twice.
											//_RecordsetPtr rsCaseHistories = CreateRecordset("SELECT ID, SurgeryDate, PersonID FROM CaseHistoryT WHERE AppointmentID = %li AND SurgeryDate <> '%s'", nCurID, FormatDateTimeForSql(dtDateOnly,dtoDate));
											while(!rs->eof) {
												long nCaseHistoryID = AdoFldLong(rs, "ID");
												_variant_t varOldDate = rs->Fields->GetItem("SurgeryDate")->Value;
												long nCaseHistoryPatientID = AdoFldLong(rs, "PersonID");
												CString strOldDate;
												if(varOldDate.vt == VT_DATE) {
													strOldDate = FormatDateTimeForInterface(VarDateTime(varOldDate), NULL, dtoDate);
												}
												else {
													strOldDate = "<None>";
												}

												// (d.thompson 2010-05-14) - PLID 38643 - Add to the batch outside the while loop to save trips.
												AddParamStatementToSqlBatch(strBatchedSql, batchArgs, "UPDATE CaseHistoryT SET SurgeryDate = {STRING} WHERE ID = {INT}", FormatDateTimeForSql(dtNewDate, dtoDate), nCaseHistoryID);
												// (d.thompson 2010-05-14) - PLID 38643 - Use the audit transaction outside the loop
												AuditEvent(nCaseHistoryPatientID, GetExistingPatientName(nCaseHistoryPatientID), nAuditTransactionID, aeiCaseHistorySurgeryDate, nCaseHistoryID, strOldDate, FormatDateTimeForInterface(dtNewDate, NULL, dtoDate), aepMedium, aetChanged);
												rs->MoveNext();
											}
										}
									}
								}
							}

							// Update our external links
							UpdatePalmSyncT(nCurID);
							PPCModifyAppt(nCurID);

							// (z.manning 2008-07-16 14:34) - PLID 30490 - Handle any HL7 messages relating to this appointment
							// (r.gonet 12/03/2012) - PLID 54107 - Updated to use refactored function.
							SendUpdateAppointmentHL7Message(nCurID);

							// (c.haag 2003-08-04 15:42) - Why did we used to use PostMessage!??!
							//GetMainFrame()->PostMessage(WM_TABLE_CHANGED, NetUtils::AppointmentsT, nCurID);

							// (j.jones 2007-09-06 15:19) - PLID 27312 - required the EndTime as a parameter
							// (j.jones 2014-08-05 10:35) - PLID 63167 - added PatientID, LocationID, ResourceIDs
							CClient::RefreshAppointmentTable(nCurID, nPatientID, dtNewTime, dtNewEndTime, nStatus, nShowState, nLocationID, strResourceIDs);
							prsAppts->MoveNext();
						}

						// (d.thompson 2010-05-14) - PLID 38643 - Commit anything we batched for later
						ExecuteParamSqlBatch(GetRemoteData(), strBatchedSql, batchArgs);

						// (d.thompson 2010-05-14) - PLID 38643 - Commit all audits
						CommitAuditTransaction(nAuditTransactionID);

						// Show the user what we did
						MsgBox(strPrompt);
					}
				}
			}
		}

		return;
	} NxCatchAll("Error determining appointment link status.");

	try {
		//If the catch was hit, we might need to roll back our audit trans
		if(nAuditTransactionID != -1) {
			RollbackAuditTransaction(nAuditTransactionID);
		}
	} NxCatchAll("AttemptAppointmentLinkAction::AuditingRollback");
}

void UpdateLinkedAppointmentShowState(long nResID, long nShowStateID, CString strShowState, CReasonDlg *pdlgReason /* = NULL */)
{
	// (z.manning, 09/11/2007) - PLID 14579 - Query to load any linked appointments. The PreSelect field in
	// this query is used to determine if the appointment should be checked by default when we later load
	// the multi-select list. Currently, we pre-select appts that occur on the same day as the main appt.
	_RecordsetPtr prsLinkedAppts = CreateParamRecordset(
		"SELECT AppointmentsT.ID, CONVERT(nvarchar, StartTime, 0) + ' - ' + CAST(CASE WHEN DATEPART(hh, EndTime) = 0 THEN 12 WHEN DATEPART(hh, EndTime) <= 12 THEN DATEPART(hh, EndTime) ELSE DATEPART(hh, EndTime) - 12 END AS nvarchar) + ':' + CASE WHEN DATEPART(mi, EndTime) < 10 THEN '0' ELSE '' END + CAST(DATEPART(mi, EndTime) AS nvarchar) + CASE WHEN DATEPART(hh, EndTime) >= 12 THEN 'PM' ELSE 'AM' END + ' (' + AptShowStateT.Symbol + ') ' + COALESCE(AptTypeT.Name, '') + ' ' + COALESCE(dbo.GetPurposeString(AppointmentsT.ID), '') AS DisplayField, \r\n"
		"	CASE WHEN CONVERT(nvarchar, StartTime, 1) = (SELECT CONVERT(nvarchar, StartTime, 1) FROM AppointmentsT WHERE ID = {INT}) THEN CONVERT(bit, 1) ELSE CONVERT(bit, 0) END AS PreSelect \r\n"
		"FROM AptLinkT \r\n"
		"LEFT JOIN AppointmentsT ON AptLinkT.AppointmentID = AppointmentsT.ID \r\n"
		"LEFT JOIN AptShowStateT ON AppointmentsT.ShowState = AptShowStateT.ID \r\n"
		"LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID \r\n"
		"WHERE AptLinkT.GroupID = (SELECT GroupID FROM AptLinkT WHERE AppointmentID = {INT}) \r\n"
		"	AND AppointmentsT.ID <> {INT} AND AppointmentsT.Status <> 4 \r\n"
		, nResID, nResID, nResID);

	if(!prsLinkedAppts->eof)
	{
		// (z.manning, 09/11/2007) - PLID 14579 - We have linked appts. Let's go through them and construct
		// a semicolon delimited list that we can pass to the multi select dialog.
		CString strSource;
		CArray<long,long> arynPreSelect;
		for(; !prsLinkedAppts->eof; prsLinkedAppts->MoveNext())
		{
			long nID = AdoFldLong(prsLinkedAppts, "ID");
			CString strDisplayField = AdoFldString(prsLinkedAppts, "DisplayField", "");
			// (z.manning, 09/11/2007) - Remove any semicolons so we don't have parsing problems.
			strDisplayField.Remove(';');
			strSource += AsString(nID) + ";" + strDisplayField + ";";
			// (z.manning, 09/11/2007) - PLID 14579 - Should we be pre-selecting this appt?
			if(AdoFldBool(prsLinkedAppts, "PreSelect", FALSE)) {
				arynPreSelect.Add(nID);
			}
		}

		// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
		CMultiSelectDlg dlgSelect(NULL, "UpdateLinkedAppointmentShowState");
		dlgSelect.PreSelect(arynPreSelect);
		dlgSelect.m_strNameColTitle = "Appointment";
		CVariantArray aryvar;
		int nResult = dlgSelect.OpenWithDelimitedComboSource(_bstr_t(strSource), aryvar, FormatString("Select linked appointments to mark %s.", strShowState));
		if(nResult == IDOK) 
		{
			// (z.manning, 07/26/2007) - PLID 14579 - Ok, the commited the multi select dialog. Let's find any
			// selected appointments IDs and update their status as well. Note, we pass a boolean parameter
			// that essentially tells the status changing function to not do any sort of prompting (as it 
			// should have been handled by the original appointment).
			CArray<long,long> arynLinkedAppointmentIDs;
			dlgSelect.FillArrayWithIDs(arynLinkedAppointmentIDs);
			for(int i = 0; i < arynLinkedAppointmentIDs.GetSize(); i++) 
			{
				long nID = arynLinkedAppointmentIDs.GetAt(i);
				switch(nShowStateID)
				{
					case assIn:
						AppointmentMarkIn(nID, TRUE);
						break;

					case assPending:
						AppointmentMarkPending(nID, TRUE);
						break;

					case assOut:
						AppointmentMarkOut(nID, TRUE);
						break;

					case assNoShow:
						AppointmentMarkNoShow(nID, TRUE, pdlgReason);
						break;

					case assReceived:
						AppointmentMarkReceived(nID, TRUE);
						break;

					default:
						AppointmentMarkUserDefined(nID, strShowState, TRUE);
						break;
				}
			}
		}
	}
}

void ColorReservationBackground(LPDISPATCH lpDisp, OLE_COLOR clr)
{
	// (j.jones 2015-02-04 13:24) - PLID 64119 - added a safety check,
	// if the reservation doesn't exist, silently fail, as coloring is not critical
	CReservation pRes(__FUNCTION__, lpDisp);
	if (pRes == NULL) {
		ASSERT(FALSE);
		return;
	}

	// (a.wilson 2012-06-14 15:02) - PLID 47966 - get data from old system preferences.
	if (GetRemotePropertyInt("ColorApptBackground", GetPropertyInt("ColorApptBackground", 0, 0, false), 0, GetCurrentUserName(), true))
	{
		// (c.haag 2003-07-30 11:52) - Don't color very dark reservations
		// (c.haag 2006-12-05 10:22) - PLID 23666 - If we are presented with
		// an invalid color, use the default background color for a new appointment
		if (GetRValue(clr) + GetGValue(clr) + GetBValue(clr) < 64) {
			pRes.PutBackColor(GetSysColor(COLOR_INFOBK));
			return;
		}

		long nDeltaPercent = GetRemotePropertyInt("ApptBackgroundColorDelta", GetPropertyInt("ApptBackgroundColorDelta", 200, 0, false), 0, GetCurrentUserName(), true);
		float r = (float)GetRValue(clr) + 64 * (float)nDeltaPercent / 100.0f;
		float g = (float)GetGValue(clr) + 64 * (float)nDeltaPercent / 100.0f;
		float b = (float)GetBValue(clr) + 64 * (float)nDeltaPercent / 100.0f;

		if (r > 255) r = 255;
		if (g > 255) g = 255;
		if (b > 255) b = 255;
		pRes.PutBackColor(RGB((char)r, (char)g, (char)b));
	}
	// (d.thompson 2012-06-27) - PLID 51220 - Changed default to Yes
	else if (GetRemotePropertyInt("ColorApptBgWithStatus", GetPropertyInt("ColorApptBgWithStatus", 1, 0, false), 0, GetCurrentUserName(), true)) {
		pRes.PutBackColor(clr);
	}
	else {
		//TES 4/5/2011 - PLID 41519 - If neither preference is on, then the appointment should have the background color.
		pRes.PutBackColor(DEFAULT_APPT_BACKGROUND_COLOR);
	}
}

void ShowPackageInfo(const IN long nPatientID)
{
	// (j.jones 2007-08-09 11:15) - PLID 27027 - added option to only show info. regarding prepaid packages

	//0 - show all, 1 - show fully paid-off packages, 2 - show packages with one use paid
	long nShowPackageInfo_PrepaidOnly = GetRemotePropertyInt("ShowPackageInfo_PrepaidOnly", 0, 0, "<None>", true);
			
	CWaitCursor pWait;

	// (j.jones 2007-08-09 11:25) - PLID 27027 - converted to a parameter recordset and ordered by date ascending
	// (j.jones 2008-05-30 12:39) - PLID 28898 - ensured we ignore charges that have an outside fee with no practice fee
	_RecordsetPtr rsPackageInfo = CreateParamRecordset("SELECT PackagesQ.QuoteID, PackagesQ.Type, BillsT.Description AS QuoteDescription, "
		"PackagesQ.TotalCount, PackagesQ.CurrentCount, PackagesQ.TotalAmount, PackagesQ.CurrentAmount, "
		"PackagesQ.ChargeDescription, PackagesQ.ChargeTotalCount, PackagesQ.ChargeCurrentCount, PersonT.First, PersonT.Last "
		"FROM ("
			//first select repeatable package information
		"	SELECT PackagesT.QuoteID, TotalAmount, CurrentAmount, PackagesT.Type, "
		"	TotalCount, CurrentCount, 0 AS ChargeTotalCount, 0 AS ChargeCurrentCount, '' AS ChargeDescription "
		"	FROM PackagesT "
		"	WHERE PackagesT.Type = 1 AND CurrentCount > 0"
			//then select all charges for multi-use packages
		"	UNION SELECT PackagesT.QuoteID, TotalAmount, CurrentAmount, PackagesT.Type, "
		"	TotalChargesQ.TotalCount, TotalChargesQ.CurrentCount, "
		"	ChargesT.Quantity AS ChargeTotalCount, ChargesT.PackageQtyRemaining AS ChargeCurrentCount, "
		"	LineItemT.Description AS ChargeDescription "
		"	FROM PackagesT "
		"	INNER JOIN ChargesT ON PackagesT.QuoteID = ChargesT.BillID "
		"	INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
		"	INNER JOIN "
		"		(SELECT ChargesT.BillID, Sum(Quantity) AS TotalCount, Sum(PackageQtyRemaining) AS CurrentCount "
		"		FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID WHERE Deleted = 0 "
		"		AND (LineItemT.Amount > Convert(money,0) OR ChargesT.OthrBillFee = Convert(money,0)) "
		"		GROUP BY BillID "
		"		) AS TotalChargesQ ON PackagesT.QuoteID = TotalChargesQ.BillID "
		"	WHERE PackagesT.Type = 2 AND ChargesT.PackageQtyRemaining > 0 AND LineItemT.Deleted = 0 "
		"	AND (LineItemT.Amount > Convert(money,0) OR ChargesT.OthrBillFee = Convert(money,0)) "
		") AS PackagesQ "
		"INNER JOIN BillsT ON PackagesQ.QuoteID = BillsT.ID "
		"INNER JOIN PersonT ON PersonT.ID = BillsT.PatientID "
		"WHERE BillsT.PatientID = {INT} AND BillsT.Deleted = 0 AND BillsT.Active = 1 "
		"ORDER BY BillsT.Date, PackagesQ.Type, PackagesQ.QuoteID", nPatientID);

	// make sure there is something to show
	if(!rsPackageInfo->eof){
		CString strMessage, strLineToAdd, strFirst, strLast;
		strFirst = AdoFldString(rsPackageInfo->GetFields(), "First");
		strLast = AdoFldString(rsPackageInfo->GetFields(), "Last");

		// (j.jones 2007-08-09 11:28) - PLID 27027 - change the label if only showing prepaid packages
		CString strHeaderMessage;
		strHeaderMessage.Format("%sPackage uses remaining for %s %s", nShowPackageInfo_PrepaidOnly != 0 ? "Prepaid " : "", strFirst, strLast);
		strMessage = strHeaderMessage + ":\r\n";

		BOOL bHasUses = FALSE;

		// (j.jones 2010-01-28 12:04) - PLID 32709 - repeatable packages will return one record per package,
		// but multi-use packages will return one record per remaining charge, so we need to track the last package ID
		long nLastPackageID = -1;
		COleCurrency cyTotalCostNoTax = COleCurrency(0,0);
		COleCurrency cyUnBilledNoTax = COleCurrency(0,0);
		COleCurrency cyPrePays = COleCurrency(0,0);

		// now build the rest of the message
		while(!rsPackageInfo->eof){
			long nPackageID = AdoFldLong(rsPackageInfo->GetFields(), "QuoteID");
			double dblCurrentCount = AdoFldDouble(rsPackageInfo->GetFields(), "CurrentCount");
			double dblTotalCount = AdoFldDouble(rsPackageInfo->GetFields(), "TotalCount");
			CString strQuoteDescription = AdoFldString(rsPackageInfo->GetFields(), "QuoteDescription");
			CString strChargeDescription = AdoFldString(rsPackageInfo->GetFields(), "ChargeDescription");
			double dblChargeCurrentCount = AdoFldDouble(rsPackageInfo->GetFields(), "ChargeCurrentCount");
			double dblChargeTotalCount = AdoFldDouble(rsPackageInfo->GetFields(), "ChargeTotalCount");
			long nPackageType = AdoFldLong(rsPackageInfo->GetFields(), "Type");

			if(nShowPackageInfo_PrepaidOnly != 0) {

				// (j.jones 2007-08-09 11:21) - PLID 27027 - Unfortunately if they want to get prepaid information,
				// the calculations for that are much more complex, and we have to call functions for each package,
				// which themselves call recordsets, so calculating this way will have more of a database hit.
				// It's not huge, just disappointing.

				//logic copied largely from the Show Packages dialog

				// (j.jones 2010-01-28 12:10) - PLID 32709 - only calculate this data if we're on a new package
				if(nPackageID != nLastPackageID) {

					cyTotalCostNoTax = AdoFldCurrency(rsPackageInfo, "TotalAmount",COleCurrency(0,0));			

					cyUnBilledNoTax = AdoFldCurrency(rsPackageInfo, "CurrentAmount",COleCurrency(0,0));

					cyPrePays = CalculatePrePayments(nPatientID, nPackageID, GetRemotePropertyInt("IncludeAllPrePaysInPopUps", 1, 0, "<None>", TRUE) == 1);
				}

				if(cyPrePays == COleCurrency(0,0) && cyUnBilledNoTax > COleCurrency(0,0)) {
					//just to save some work here - if we have no prepayments
					//and there is a positive unbilled amount, just give up
					//as we can't possibly cover one use
					nLastPackageID = nPackageID;
					rsPackageInfo->MoveNext();
					continue;
				}
				
				BOOL bHasTax = FALSE;

				COleCurrency cyTotalCostWithTax = CalculateTotalPackageValueWithTax(nPackageID);
				
				COleCurrency cyUnBilledWithTax;

				//if the balances are different, tax exists, so make sure we also grab the other amounts with tax
				if(cyTotalCostNoTax != cyTotalCostWithTax) {
					bHasTax = TRUE;
					cyUnBilledWithTax = CalculateRemainingPackageValueWithTax(nPackageID);
				}
				else {
					cyUnBilledWithTax = cyUnBilledNoTax;
				}

				//calculate based on the preference requiring a full or one-use pay
				if(nShowPackageInfo_PrepaidOnly == 1) {

					//require a fully paid package
					if(cyPrePays >= cyUnBilledWithTax) {

						// (j.jones 2010-01-28 12:04) - PLID 32709 - output differently per package type
						if(nPackageType == 1) {
							//repeatable package
							strLineToAdd.Format("\r\nRepeatable package '%s' has %g uses remaining out of %g\r\n", strQuoteDescription, dblCurrentCount, dblTotalCount);
							strMessage += strLineToAdd;
						}
						else {
							//multi-use package, output differently if this is a new package or not
							if(nPackageID != nLastPackageID) {
								//add the package header
								strLineToAdd.Format("\r\nMulti-Use package '%s' has the following uses remaining:\r\n", strQuoteDescription, dblChargeCurrentCount, dblChargeTotalCount);
								strMessage += strLineToAdd;
							}

							//now add the charge
							strLineToAdd.Format("   - '%s' has %g uses remaining out of %g\r\n", strChargeDescription, dblChargeCurrentCount, dblChargeTotalCount);
							strMessage += strLineToAdd;
						}

						bHasUses = TRUE;
					}
				}
				else {

					//calculate the value of one instance of the package
					COleCurrency cyOneUse = COleCurrency(0,0);

					if(dblCurrentCount <= 1.0)
						//if there is 1 use left, it is the remaining balance
						//if there is 0 left, then the balance is 0 anyways
						if(bHasTax) {
							cyOneUse = cyUnBilledWithTax;
						}
						else {
							cyOneUse = cyUnBilledNoTax;
						}
					else {
						//one use is the package cost / package count
						if(bHasTax) {
							cyOneUse = (cyTotalCostWithTax/dblTotalCount);
						}
						else {
							cyOneUse = (cyTotalCostNoTax/dblTotalCount);
						}
						RoundCurrency(cyOneUse);
					}

					// (j.jones 2011-07-15 15:21) - PLID 38334 - if there are uses left,
					// but the unbilled amount is zero, then one use is also zero
					if(bHasTax) {
						if(cyUnBilledWithTax <= COleCurrency(0,0)) {
							cyOneUse = COleCurrency(0,0);
						}
					}
					else {
						if(cyUnBilledNoTax <= COleCurrency(0,0)) {
				 			cyOneUse = COleCurrency(0,0);
						}
					}

					//ok, now we have the cyOneUse price, do we have prepayments to cover it?

					if(cyPrePays >= cyOneUse) {

						//we can cover one, but how many can we cover?

						// (j.jones 2010-01-28 12:04) - PLID 32709 - this is not accurate for multi-use packages,
						// so only try this on repeatable packages
						if(nPackageType == 1) {

							//gotta check using SQL 
							double dblUsesCovered = dblCurrentCount;
							if(cyOneUse > COleCurrency(0,0)) {
								_RecordsetPtr rsUsesCovered = CreateParamRecordset("SELECT Convert(float, {STRING}) / Convert(float, {STRING}) AS UsesCovered", FormatCurrencyForSql(cyPrePays), FormatCurrencyForSql(cyOneUse));
								if(!rsUsesCovered->eof) {
									dblUsesCovered = AdoFldDouble(rsUsesCovered, "UsesCovered", dblCurrentCount);
									if(dblUsesCovered > dblCurrentCount)
										dblUsesCovered = dblCurrentCount;
								}
								rsUsesCovered->Close();
							}

							//convert to a whole number if it isn't the current count
							if(dblUsesCovered != dblCurrentCount)
								dblUsesCovered = (long)dblUsesCovered;

							strLineToAdd.Format("\r\nRepeatable package '%s' has %g uses remaining out of %g (prepaid through %g more use%s)\r\n", strQuoteDescription, dblCurrentCount, dblTotalCount, dblUsesCovered, dblUsesCovered > 1.0 ? "s" : "");
							strMessage += strLineToAdd;
						}
						else {
							//multi-use package, output differently if this is a new package or not,
							//don't even bother explaining the pre-payment status
							if(nPackageID != nLastPackageID) {
								//add the package header
								strLineToAdd.Format("\r\nMulti-Use package '%s' has the following uses remaining:\r\n", strQuoteDescription, dblChargeCurrentCount, dblChargeTotalCount);
								strMessage += strLineToAdd;
							}

							//now add the charge
							strLineToAdd.Format("   - '%s' has %g uses remaining out of %g\r\n", strChargeDescription, dblChargeCurrentCount, dblChargeTotalCount);
							strMessage += strLineToAdd;
						}

						bHasUses = TRUE;
					}
				}
			}
			else {
				//just show all remaining uses
				
				// (j.jones 2010-01-28 12:04) - PLID 32709 - output differently per package type
				if(nPackageType == 1) {
					//repeatable package
					strLineToAdd.Format("\r\nRepeatable package '%s' has %g uses remaining out of %g\r\n", strQuoteDescription, dblCurrentCount, dblTotalCount);
					strMessage += strLineToAdd;
				}
				else {
					//multi-use package, output differently if this is a new package or not
					if(nPackageID != nLastPackageID) {
						//add the package header
						strLineToAdd.Format("\r\nMulti-Use package '%s' has the following uses remaining:\r\n", strQuoteDescription, dblChargeCurrentCount, dblChargeTotalCount);
						strMessage += strLineToAdd;
					}

					//now add the charge
					strLineToAdd.Format("   - '%s' has %g uses remaining out of %g\r\n", strChargeDescription, dblChargeCurrentCount, dblChargeTotalCount);
					strMessage += strLineToAdd;
				}

				bHasUses = TRUE;
			}

			//now track the current package ID as the last package ID
			nLastPackageID = nPackageID;

			rsPackageInfo->MoveNext();

		}

		if(bHasUses) {
			// (j.jones 2010-01-28 12:29) - PLID 32709 - converted to a MsgBox
			//which gives us a scrollbar if needed
			CMsgBox dlgMsg(NULL);
			dlgMsg.m_strWindowText = strHeaderMessage;
			dlgMsg.msg = strMessage;
			dlgMsg.DoModal();
		}
	}
}

//TES 8/10/2010 - PLID 39264 - Added dtDate and dtStartTime for use in looking up Location Templates (the function will just look at 
// the date/time portions respectively, so if you have one variable with both you can pass it in as each parameter).
BOOL GetValidAppointmentLocation(long nCurrentLocationID, long& nNewLocationID, long nResID, const COleDateTime &dtDate, const COleDateTime &dtStartTime, const CDWordArray* ResourceIDs /* = NULL */)
{
	nNewLocationID = nCurrentLocationID;

	// (c.haag 2003-07-31 09:29) - If the user is forcing the location of the appt to match the
	// resource location, do that matching.
	// (j.jones 2010-07-20 12:01) - PLID 39629 - ForceApptResourceLocationMatch is now a subpreference of 
	// ApptLocationPref, when it is set to default locations to the resource location (option 3), so to be consistent
	// with the preference interface, do not check ForceApptResourceLocationMatch unless the ApptLocationPref = 3
	if(GetRemotePropertyInt("ApptLocationPref", 1, 0, "<None>", true) == 3
		&& GetRemotePropertyInt("ForceApptResourceLocationMatch", 0, 0, "<None>", true) != 0)
	{
		CDWordArray adwLocations;
		CStringArray astrLocations;
		
		//TES 8/10/2010 - PLID 39264 - First, see if there are any location templates that match.
		//TES 8/10/2010 - PLID 39264 - Gather our resource IDs.
		CString strResourceIDs;
		CDWordArray dwaResourceIDs;
		if(nResID == -1 && ResourceIDs) {
			// (m.cable 2004-05-28 15:52) - This is a new appointment which means we don't have any info in 
			// appointmentsT yet
			for(int x = 0; x < ResourceIDs->GetSize(); x++){
				CString strCurrent;
				strCurrent.Format("%li", ResourceIDs->GetAt(x));
				strResourceIDs +=  strCurrent + ", ";
			}
			// delete the last ", "
			strResourceIDs.Delete(strResourceIDs.GetLength() - 2, 2);

			dwaResourceIDs.Copy(*ResourceIDs);
		}
		else {
			_RecordsetPtr rsResourceIDs = CreateParamRecordset("SELECT ResourceID FROM AppointmentResourceT WHERE AppointmentID = {INT}", nResID);
			while(!rsResourceIDs->eof) {
				long nResourceID = AdoFldLong(rsResourceIDs, "ResourceID");
				strResourceIDs += AsString(nResourceID) + ", ";
				dwaResourceIDs.Add(nResourceID);
				rsResourceIDs->MoveNext();
			}
			// delete the last ", "
			strResourceIDs.Delete(strResourceIDs.GetLength() - 2, 2);
		}
		//TES 8/10/2010 - PLID 39264 - Now, go through the matching location templates, find the highest-priority location for each
		// resource.  If at least one is the location we were given, return TRUE.  Otherwise, track the valid locations for these resources,
		// and prompt them below just like for the resource locations.
		CMap<long,long&,long,long&> mapResourceToLocation, mapResourceToPriority;
		CString str = dtStartTime.Format();
		// (a.walling 2014-04-21 14:47) - PLID 60474 - TemplateHitAllP - open in snapshot isolation
		_CommandPtr pCmd = OpenStoredProc(GetRemoteDataSnapshot(), "TemplateHitAllP");
		//TES 8/16/2011 - PLID 45053 - CheckDate needs to be JUST a date, not a time.
		COleDateTime dtCheckDate = COleDateTime(dtDate.GetYear(), dtDate.GetMonth(), dtDate.GetDay(), 0, 0, 0);
		AddParameterDateTime(pCmd, "CheckDate", dtCheckDate);
		AddParameterString(pCmd, "ResourceIDs", strResourceIDs);
		AddParameterBool(pCmd, "IncludeRuleInfo", false);
		AddParameterBool(pCmd, "IncludeNormalTemplates", false);
		AddParameterBool(pCmd, "IncludeResourceAvailTemplates", true);
		_RecordsetPtr rsLocationTemplates = CreateRecordset(pCmd);
		//TES 8/10/2010 - PLID 39264 - Make sure we're only comparing on times.
		COleDateTime dtStartTimeOnly;
		dtStartTimeOnly.SetTime(dtStartTime.GetHour(), dtStartTime.GetMinute(), dtStartTime.GetSecond());
		while(!rsLocationTemplates->eof) {
			//TES 8/10/2010 - PLID 39264 - See if this item includes our start time.
			COleDateTime dtStart = AdoFldDateTime(rsLocationTemplates, "StartTime");
			COleDateTime dtEnd = AdoFldDateTime(rsLocationTemplates, "EndTime");
			//TES 8/10/2010 - PLID 39264 - Make sure we're only comparing on times.
			dtStart.SetTime(dtStart.GetHour(), dtStart.GetMinute(), dtStart.GetSecond());
			dtEnd.SetTime(dtEnd.GetHour(), dtEnd.GetMinute(), dtEnd.GetSecond());
			if(dtStart <= dtStartTimeOnly && dtEnd > dtStartTimeOnly) {
				//TES 8/10/2010 - PLID 39264 - OK, this matches, see if it's a higher priority than whatever we've got for this resource.
				long nCurrentResourceID = AdoFldLong(rsLocationTemplates, "ResourceID", -1);
				long nPriority = AdoFldLong(rsLocationTemplates, "Priority");
				long nCurrentPriority = -1;
				mapResourceToPriority.Lookup(nCurrentResourceID, nCurrentPriority);
				if(nPriority > nCurrentPriority) {
					//TES 8/10/2010 - PLID 39264 - This one's higher, so use it.
					mapResourceToPriority.SetAt(nCurrentResourceID, nPriority);
					long nLocationID = AdoFldLong(rsLocationTemplates, "LocationID");
					mapResourceToLocation.SetAt(nCurrentResourceID, nLocationID);
				}
			}
			rsLocationTemplates->MoveNext();
		}
		//TES 8/10/2010 - PLID 39264 - OK, we've got a location and priority for each resource (plus potentially one for all resources).  
		// See if any exist, and if so, if any match.
		//TES 8/10/2010 - PLID 39264 - First get our All Resources location and priority.
		long nAllResourcesLocation = -1;
		long nAllResourcesPriority = -1;
		long nAllResourcesID = -1;
		mapResourceToLocation.Lookup(nAllResourcesID, nAllResourcesLocation);
		mapResourceToPriority.Lookup(nAllResourcesID, nAllResourcesPriority);
		for(int i = 0; i < dwaResourceIDs.GetSize(); i++) {
			long nResourceLocation = -1;
			long nResourcePriority = -1;
			long nCurrentResourceID = (long)dwaResourceIDs[i];
			mapResourceToLocation.Lookup(nCurrentResourceID, nResourceLocation);
			mapResourceToPriority.Lookup(nCurrentResourceID, nResourcePriority);
			//TES 8/10/2010 - PLID 39264 - If the All Resources priority is higher, use its location.
			if(nAllResourcesPriority > nResourcePriority) {
				nResourceLocation = nAllResourcesLocation;
			}
			//TES 8/10/2010 - PLID 39264 - Do we have a location?
			if(nResourceLocation != -1) {
				if(nResourceLocation == nCurrentLocationID) {
					//TES 8/10/2010 - PLID 39264 - Got a match!
					return TRUE;
				}
				else {
					//TES 8/10/2010 - PLID 39264 - Nope, so add it to our list of valid, non-matching locations.
					adwLocations.Add(nResourceLocation);
					astrLocations.Add(GetRemoteDataCache().GetLocationName(nResourceLocation));
				}
			}
		}
		if(adwLocations.GetSize() == 0) {
			//TES 8/10/2010 - PLID 39264 - If we get here, there were no location templates, so go on to the old resource location matching.

			_RecordsetPtr prs;
			
			if(nResID == -1 && ResourceIDs){
				// (d.thompson 2010-05-11) - PLID 38581 - Cannot be parameterized.  I did fix a bug that had a second parameter passed in here
				//	that was never used.
				// (a.walling 2013-02-08 12:59) - PLID 55083 - Parameterize
				prs = CreateParamRecordset("SELECT LocationID, Name FROM ResourceLocationConnectT LEFT JOIN LocationsT ON LocationsT.ID = ResourceLocationConnectT.LocationID WHERE ResourceID IN ({INTARRAY}) GROUP BY LocationID, Name", dwaResourceIDs);	
			}
			else{
				// (d.thompson 2010-05-11) - PLID 38581 - Parameterized
				prs = CreateParamRecordset("SELECT LocationID, Name FROM ResourceLocationConnectT LEFT JOIN LocationsT ON LocationsT.ID = ResourceLocationConnectT.LocationID WHERE ResourceID IN (SELECT ResourceID FROM AppointmentResourceT WHERE AppointmentID = {INT}) GROUP BY LocationID, Name", nResID);
			}
			
			FieldsPtr pflds = prs->Fields;
			while (!prs->eof)
			{
				long nID = AdoFldLong(pflds, "LocationID");
				if (nID == nCurrentLocationID)
					break;
				adwLocations.Add(nID);
				astrLocations.Add(AdoFldString(pflds, "Name"));
				prs->MoveNext();
			}
		}

		// (c.haag 2006-03-15 11:11) - PLID 19717 - If there is nothing in adwLocations, we can't
		// assign a new location ID.
		if (adwLocations.GetSize() > 0)
		{
			// (c.haag 2003-07-31 09:34) - If we get here, there are no matches. So, we take
			// a recourse based on the unique number of resource locations.
			if (adwLocations.GetSize() == 1)
			{
				// (c.haag 2003-07-31 09:35) - There is only one location; warn the user we are
				// changing it and then change it.
				MsgBox("This appointment is being saved to a resource whose location is inconsistent with the appointment's location. The appointment location will now be changed to '%s.'", astrLocations[0]);
				nNewLocationID = adwLocations[0];
			}
			else
			{
				// (c.haag 2003-07-31 09:39) - There are several locations to choose from. Give
				// the user the option to choose only one.
				// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
				CMultiSelectDlg dlg(NULL, "LocationsT");
				CString strWhere;
				CString strRes;
				MsgBox("This appointment is being saved to multiple resources whose locations are inconsistent with the appointment's location. Please select which location to assign the appointment to on the following screen.");
				dlg.m_strNameColTitle = "Locations";

				strWhere.Format("ID = %d", adwLocations[0]);
				for (long i=1; i < adwLocations.GetSize(); i++)
				{
					CString str;
					str.Format(" OR ID = %d", adwLocations[i]);
					strWhere += str;
				}
				if (IDCANCEL == dlg.Open("LocationsT", strWhere, "ID", "Name", "Please select one location", 1, 1))
					return FALSE;

				strRes = dlg.GetMultiSelectIDString();
				nNewLocationID = atoi(strRes);
			}
		}
		
		//TES 8/10/2010 - PLID 39264 - There were no location templates, and the resource doesn't have a default, so whatever they 
		// selected is fine (we default to the logged-in location in this case, but don't require it).
	}
	return TRUE;
}

// (j.jones 2014-11-25 11:26) - PLID 64178 - moved AppointmentValidateByDuration to CommonSchedUtils

// (a.wilson 2014-08-21 14:33) - PLID 63170 - use the refreshappointmenttable() with all the parameters to prevent extra recordsets.
void AppointmentCheckPatientProvider(long nPatientID, long nResID)
{
	//TES 2/1/2007 - PLID 24540 - If this is for the sentinel patient, don't bother.
	if(nPatientID == -25 || !GetRemotePropertyInt("ApptTrySaveProviderIfAbsent", 1, 0, "<None>", true))
		return;
	try
	{
		// (d.thompson 2010-05-12) - PLID 38581 - For optimization purposes, parameterized and combined the two 
		//	recordsets into a single trip.  If there is no provider, we'll just ignore the second recordset.
		CParamSqlBatch sql;
		sql.Add("SELECT [Last] + ', ' + [First] + ' ' + [Middle] AS Name, MainPhysician "
			"FROM PatientsT LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID WHERE PersonID = {INT};", nPatientID);
		sql.Add("SELECT [First] + ' ' + [Last] AS Name, ProviderID "
			"FROM ResourceProviderLinkT LEFT JOIN PersonT ON ResourceProviderLinkT.ProviderId = PersonT.ID "
			"WHERE ResourceID IN (SELECT ResourceID FROM AppointmentResourceT WHERE AppointmentID = {INT}) "
			"GROUP BY [First] + ' ' + [Last], ProviderID;", nResID);
		sql.Add("SELECT LocationID, Status, ShowState, StartTime, EndTime, dbo.GetResourceIDString(AppointmentsT.ID) AS ResourceIDs "
			"FROM AppointmentsT WHERE ID = {INT}", nResID);
		_RecordsetPtr prsProvider = sql.CreateRecordset(GetRemoteData());
		
		CDWordArray adwProviders;
		CStringArray astrProviders;
		if (prsProvider->eof)
			return;
		COleVariant vProvider = prsProvider->Fields->Item["MainPhysician"]->Value;
		if (vProvider.vt == VT_I4) // Assume we already have one
			return;

		// (d.thompson 2010-05-12) - PLID 38581 - Now move to the second recordset we already got while doing the first.
		_RecordsetPtr prs = prsProvider->NextRecordset(NULL);

		FieldsPtr pflds = prs->Fields;
		while (!prs->eof)
		{
			adwProviders.Add(AdoFldLong(pflds, "ProviderID"));
			astrProviders.Add(AdoFldString(pflds, "Name"));
			prs->MoveNext();
		}

		long nLocationID = -1, nStatus = -1, nShowState = -1;
		COleDateTime dtStart = g_cdtMin, dtEnd = g_cdtMin;
		CString strResourceIDs = "";
		_RecordsetPtr prsAppt = prs->NextRecordset(NULL);
		if (!prsAppt->eof) {
			nLocationID = AdoFldLong(prsAppt, "LocationID");
			nStatus = (long)AdoFldByte(prsAppt, "Status");
			nShowState = AdoFldLong(prsAppt, "ShowState");
			dtStart = AdoFldDateTime(prsAppt, "StartTime");
			dtEnd = AdoFldDateTime(prsAppt, "EndTime");
			strResourceIDs = AdoFldString(prsAppt, "ResourceIDs", "");
		}

		BOOL bChange = FALSE;
		long nIndex = 0;
		if (adwProviders.GetSize() == 1)
		{
			if (IDYES == MsgBox(MB_YESNO, "This patient currently has no default provider. Would you like to assign %s as the default provider for this patient?", astrProviders[0]))
			{
				// (d.thompson 2010-05-12) - PLID 38581 - Parameterized
				ExecuteParamSql("UPDATE PatientsT SET MainPhysician = {INT} WHERE PersonID = {INT}", adwProviders[0], nPatientID);
				bChange = TRUE;
			}
		}
		else if (adwProviders.GetSize() > 1)
		{
			if (IDYES == MsgBox(MB_YESNO, "This patient currently has no default provider. Would you like to assign one based on the providers linked to the resources for this appointment?"))
			{
				// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
				CMultiSelectDlg dlg(NULL, "ProvidersT");
				CString strWhere;
				CString strRes;

				strWhere.Format("ID = %d", adwProviders[0]);
				for (long i=1; i < adwProviders.GetSize(); i++)
				{
					CString str;
					str.Format(" OR ID = %d", adwProviders[i]);
					strWhere += str;
				}
				if (IDOK == dlg.Open("ProvidersT LEFT JOIN PersonT ON ProvidersT.PersonID = PersonT.ID", strWhere, "ID", "[Last] + ', ' + [First] + ' ' + [Middle]", "Please select one provider", 1, 1))
				{
					strRes = dlg.GetMultiSelectIDString();
					// (d.thompson 2010-05-12) - PLID 38581 - Parameterized.  Changed this from a string insert of strRes to
					//	force a conversion to an integer.  The dialog forces only 1 selection.
					ExecuteParamSql("UPDATE PatientsT SET MainPhysician = {INT} WHERE PersonID = {INT}", atoi(strRes), nPatientID);
					for (nIndex=0; nIndex < (long)adwProviders.GetSize(); nIndex++)
					{
						if ((long)adwProviders[nIndex] == atoi(strRes))
							break;
					}
					bChange = TRUE;
				}
			}
		}
		if (bChange)
		{
			long nAuditID = -1;
			nAuditID = BeginNewAuditEvent();
			if(nAuditID != -1)
			{
				CString strNew = astrProviders[nIndex];
				AuditEvent(nPatientID, AdoFldString(prsProvider, "Name", ""), nAuditID, aeiPatientProvider, nPatientID, "<No Provider>", strNew, aepMedium, aetChanged);
			}
			//TES 2/7/2011 - PLID 40588 - The provider is included in HL7 messages now, so make sure we send an updated message if appropriate.
			// (r.gonet 12/03/2012) - PLID 54105 - Updated to use refactored function.
			UpdateExistingPatientInHL7(nPatientID);
			
		}
		//(ZM, 20-Jun-05, PL15783): Refreshing to fix a bug that would show two appts if the resource was changed with
		// the check for default provider option on.
		CClient::RefreshAppointmentTable(nResID, nPatientID, dtStart, dtEnd, nStatus, nShowState, nLocationID, strResourceIDs);
	}
	NxCatchAll("Error checking patient provider");
}

void OpenAppointment(long nResID)
{
	if (!GetMainFrame())
		return;
	GetMainFrame()->OpenAppointment(nResID);
}

// (c.haag 2003-09-05 11:17) - Start and end parameters require full date and time
void AttemptFindAvailableMoveUp(const COleDateTime& dtOldStart, const COleDateTime& dtOldEnd,
								const COleDateTime& dtNewStart, const COleDateTime& dtNewEnd,
								long nResID, BOOL bResourcesChanged, const CDWordArray& adwChangedResources)
{
	try {
		//TES 12/17/2008 - PLID 32478 - If they don't have the Enterprise edition, they can't use
		// the moveup list, so don't prompt them for something they can't do (just check silently).
		if(!g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrSilent)) {
			return;
		}
		
		long nOldApptDuration = GetSafeTotalMinuteDiff(dtOldEnd, dtOldStart);
		long nNewApptDuration = GetSafeTotalMinuteDiff(dtNewEnd, dtNewStart);
		long nOldApptMovedMinutes = GetSafeTotalMinuteDiff(dtNewStart, dtOldStart);
		if (nOldApptMovedMinutes < 0)
			nOldApptMovedMinutes = -nOldApptMovedMinutes;
		long nDuration = nOldApptMovedMinutes;
		if (nNewApptDuration < nOldApptDuration)
			nDuration = max(nOldApptMovedMinutes, nOldApptDuration - nNewApptDuration);
		
		if (nDuration == 0 && !bResourcesChanged) {
			// If neither the time nor the resources changed, then something is wrong.
			return;
		}

		// (d.thompson 2010-05-12) - PLID 38581 - Reworked the query generation to be (as best we can, given the
		//	multiple resource filters) parameterized.  Mostly just rearranged.  Removed silly GetRecordCount()
		_CommandPtr pCmd = OpenParamQuery("");

		CString strQuery = 
			"SELECT COUNT(WaitingListT.ID) AS RecordCount "
			"FROM WaitingListT "
			"INNER JOIN WaitingListItemT "
			"ON WaitingListT.ID = WaitingListItemT.WaitingListID "
			"INNER JOIN WaitingListItemResourceT "
			"ON WaitingListItemT.ID = WaitingListItemResourceT.ItemID "
			"WHERE  "
			"WaitingListItemT.StartDate <= ? "
			"AND WaitingListItemT.EndDate >= ? ";
		AddParameterString(pCmd, "EndDate", FormatDateTimeForSql(dtOldEnd, dtoDate));
		AddParameterString(pCmd, "StartDate", FormatDateTimeForSql(dtOldStart, dtoDate));

		//Add the resource query section
		strQuery += " AND (WaitingListItemT.AllResources = 1";

		// (d.moore 2007-05-29 14:30) - PLID 4013 - Modified to use waiting list tables.
		if (bResourcesChanged) {
			// Resources changed, so look for items in the waiting list that
			//  match the time slot and the old resources.
			for (long i=0; i < adwChangedResources.GetSize(); i++)
			{
				// (d.thompson 2010-05-12) - PLID 38581 - Added in param fashion
				strQuery += " OR WaitingListItemResourceT.ResourceID = ?";
				AddParameterLong(pCmd, "ResID", adwChangedResources[i]);
			}
		} else {
			// Resources did not change, so look for an item in the waiting list
			//  that matches the time slot and the resources used by the appointment.
			// (d.thompson 2010-05-12) - PLID 38581 - Cleaned up for parameterizing
			strQuery += 
				" OR WaitingListItemResourceT.ResourceID IN "
				"(SELECT ResourceID FROM AppointmentResourceT "
				"WHERE AppointmentID = ?) ";
			AddParameterLong(pCmd, "ResID", nResID);
		}

		//Final part of resource query
		strQuery += ")";

		//Now generate the recordset
		pCmd->PutCommandText(_bstr_t(strQuery));
		_RecordsetPtr prs = CreateRecordset(pCmd);
		
		if (!prs->eof && AdoFldLong(prs, "RecordCount") > 0)
		{
			if (IDNO == MsgBox(MB_YESNO, "The act of modifying this appointment has opened a slot which fits one or more move up appointments. Would you like to check the move up list now?"))
				return;

			COleDateTime* pdtStart = new COleDateTime;
			if (nDuration == nOldApptMovedMinutes)
			{
				if (GetSafeTotalMinuteDiff(dtNewStart, dtOldStart) > 0)
					*pdtStart = dtOldStart;
				else
					*pdtStart = dtOldEnd;
			}
			else
				*pdtStart = dtOldEnd;

			// (c.haag 2003-10-27 11:43) - Post a message to open up the move-up dialog and to
			// have it filter on the available resources.
			CDWordArray* padwRes = new CDWordArray;
			for (long i=0; i < adwChangedResources.GetSize(); i++)
			{
				padwRes->Add(adwChangedResources[i]);
			}
			GetMainFrame()->PostMessage(NXM_OPEN_MOVEUP_DIALOG, (WPARAM)pdtStart, (LPARAM)padwRes);
		}
		
	} NxCatchAll("Error in AttemptFindAvailableMoveUp");
}


//PLID 10707
//Make it so that it can log the time the status is changed
void LogShowStateTime(long nApptID, long nStatus) {

	try {
		// (d.thompson 2010-05-12) - PLID 38581 - Reworked this function.  The old behavior was as follows:
		//	- Query the database to see if our appointment/status combo exists in the ShowStateHistory table.
		//	- If it does not, run another trip with an INSERT query
		//	- If so, run a trip that updates the record from the history
		//	- If the new value is 'In', and an 'Out' exists, delete it (to avoid bad data)
		//The new methodology will perform the same checks, but we can do so in a single trip to the SQL server
		//	with all the logic embedded in the recordset.
		//Known issue:  PLID 38614.  If you mark out as the first action (no 'In' yet), then mark 'In', it won't
		//	clear the out.  Is this intentional?  It's how it's always worked.
		ExecuteParamSql(
			"DECLARE @ApptID int;\r\n"
			"DECLARE @NewStateID int;\r\n"
			"DECLARE @UserID int;\r\n"
			"SET @ApptID = {INT};\r\n"
			"SET @NewStateID = {INT};\r\n"
			"SET @UserID = {INT};\r\n"
			"IF EXISTS( SELECT AppointmentID FROM AptShowStateHistoryT WHERE AppointmentID = @ApptID AND ShowStateID = @NewStateID ) \r\n"
			"BEGIN \r\n"
				//Already exists, so update the history
				// (j.jones 2009-08-10 08:39) - PLID 35145 - we now track the user name
				"UPDATE AptShowStateHistoryT SET TimeStamp = getDate(), UserID = @UserID \r\n"
				"WHERE AppointmentID = @ApptID and ShowStateID = @NewStateID; \r\n"
				//Additionally, if the new status is 'In' (1), we must delete any 'Old' (2) record that might exist, so
				//	that we can properly report on In-Out times without having screwy values.
				"IF @NewStateID = 1 \r\n"
				"BEGIN \r\n"
					"DELETE FROM AptShowStateHistoryT WHERE AppointmentID = @ApptID AND ShowStateID = 2; \r\n"
				"END \r\n"
			"END \r\n"
			"ELSE \r\n"
			"BEGIN \r\n"
				// (j.gruber 2011-07-22 13:19) - PLID 38614 - delete in this situation also
				"IF @NewStateID = 1 \r\n"
				"BEGIN \r\n"
					"DELETE FROM AptShowStateHistoryT WHERE AppointmentID = @ApptID AND ShowStateID = 2; \r\n"
				"END \r\n"
				//Does not exist, just plain insert it newly
				// (j.jones 2009-08-10 08:39) - PLID 35145 - we now track the user ID
				"INSERT INTO AptShowStateHistoryT (AppointmentID, ShowStateID, TimeStamp, UserID) \r\n"
				"VALUES (@ApptID, @NewStateID, getDate(), @UserID); \r\n"
			"END \r\n",
			//DECLARE params
			nApptID, nStatus, GetCurrentUserID());

	}NxCatchAll("Error logging status change");

}

// (c.haag 2010-03-08 16:33) - PLID 37326 - Added bValidateTemplates. When TRUE, the appt is validated against scheduler templates
// (a.walling 2010-06-14 16:36) - PLID 23560 - Resource Sets
// (j.jones 2014-12-02 09:08) - PLID 64178 - added bValidateMixRules
// (j.jones 2014-11-26 10:39) - PLID 64179 - added pParentWnd
// (j.jones 2014-11-26 16:42) - PLID 64178 - Added an optional overriddenMixRules returned,
// this is only filled if the appt. exceeded a scheduler mix rule and a user overrode it. The caller needs to save this info.
BOOL ValidateAppt(CWnd* pParentWnd, APPT_INFO *pApptInfo, CSchedulerView * pView, BOOL bCheckRules, BOOL bSilent, BOOL bIsEvent, BOOL bCheckSurgeryCenter, BOOL bCheckOverWriteIn,
	long nLineItemOverrideID, BOOL bValidateTemplates, BOOL bValidateMixRules, OUT std::vector<SchedulerMixRule> &overriddenMixRules, OUT shared_ptr<SelectedFFASlot> &pSelectedFFASlot) {

	try {

		if(pApptInfo->dtStartTime.GetStatus() != COleDateTime::valid) {
			if (! bSilent) {
				MsgBox("This appointment cannot be saved because you have entered an invalid start time.");
			}
			//GetDlgItem(IDC_START_TIME_BOX)->SetFocus();
			return FALSE;
		}

		if(pApptInfo->dtStartTime.GetStatus() != COleDateTime::valid) {
			if (! bSilent) {
				MsgBox("This appointment cannot be saved because you have entered an invalid end time.");
			}
			//GetDlgItem(IDC_END_TIME_BOX)->SetFocus();
			return FALSE;
		}

		//DRT 7/24/03 - We do this later anyways, but I don't see why we don't do it here...  If the end time is
		//		midnight, move it to 11:59 pm.  That way, all the validation that is going on assumes the right time, 
		//		instead of our current method where it validates all the times of midnight, then when finally saving 
		//		it bumps it down to 11:59.  This also avoids a problem where if you drag from 12:00 am to 12:00 am (the
		//		next day), it was making the whole appointment into an event.
		{
			if(!bIsEvent) {
				if(pApptInfo->dtEndTime.GetHour() == 0 && pApptInfo->dtEndTime.GetMinute() == 0) {
					//it is at midnight!  set it to 11:59 pm
					pApptInfo->dtEndTime.SetDateTime(pApptInfo->dtEndTime.GetYear(), 
						pApptInfo->dtEndTime.GetMonth(), pApptInfo->dtEndTime.GetDay(), 23, 59, 0);
				}
			}
		}

		COleDateTime dtStart = pApptInfo->dtStartTime;
		COleDateTime dtEnd = pApptInfo->dtEndTime;
		COleDateTime dtArrival = pApptInfo->dtArrivalTime;
		COleDateTime dtDate = pApptInfo->dtApptDate;

		if (dtStart > dtEnd)
		{
			if (!bSilent) {
				MsgBox("This appointment cannot be saved because the end time is before the start time.");
			}
			//GetDlgItem(IDC_START_TIME_BOX)->SetFocus();
			return FALSE;
		}

		if (dtArrival > dtStart)
		{
			if (!bSilent) {
				MsgBox("This appointment cannot be saved because the arrival time is after the start time.");
			}
			return FALSE;
		}

		//TES 11/8/2013 - PLID 59396 - Appointments can't have the same start and end time (can cause an infinite loop)
		if(!bIsEvent && dtStart == dtEnd)
		{
			if (!bSilent) {
				MsgBox("This appointment cannot be saved because the end time is the same as the start time.");
			}
			return FALSE;
		}
		
		if (pApptInfo->dwResourceList->GetSize() == 0) {
			if (!bSilent) {
				MsgBox("This appointment cannot be saved because there are no resources selected.  Please select a resource before saving.");
			}
			//GetDlgItem(IDC_APTRESOURCE_COMBO)->SetFocus();
			//m_dlAptResource->PutDropDownState(VARIANT_TRUE);
			return FALSE;
		}

		// (j.gruber 2012-07-31 12:25) - PLID 51869 - insurance validation
		if (pApptInfo->pmapInsInfo) {
			long nHighPlacement = -1;
			POSITION pos = pApptInfo->pmapInsInfo->GetStartPosition();
			InsuranceInfo *pInsInfo;
			long nPlacement = -1;
			CMap<long, long, long, long> mapInsParties;
			long nInsPartyID = -1;

			while (pos != NULL) {
				pApptInfo->pmapInsInfo->GetNextAssoc(pos, nPlacement, pInsInfo);
				//check to make sure they didn't add an insured party more than once
				if (!mapInsParties.Lookup(pInsInfo->nInsuredPartyID, nInsPartyID)) {
					mapInsParties.SetAt(pInsInfo->nInsuredPartyID, pInsInfo->nInsuredPartyID);
				}
				else {
					//oh bother, they did!
					MsgBox("You cannot add the same insured party to different placements on the same appointment.  Please correct this before saving.");
					return FALSE;
				}
				if (nPlacement > nHighPlacement) {
					nHighPlacement = nPlacement;
				}
			}

			for (int i = 1; i <= nHighPlacement; i++){
				if (!pApptInfo->pmapInsInfo->Lookup(i, pInsInfo)){
					MsgBox("You cannot have an insurance placement without all lower placements (ie: a secondary company without a primary company).  Please correct this before saving.");
					return FALSE;
				}
			}
		}

		// (c.haag 2003-07-29 10:35) - Check to see if any resources were deleted
		// (d.thompson 2010-05-13) - PLID 38643 - Parameterized as possible (limited resource set makes this somewhat useful)
		_CommandPtr pCmd = OpenParamQuery("");
		CString strAllCheckSQL = "SELECT ID FROM ResourceT WHERE ID = ? ";
		AddParameterLong(pCmd, "ResID", pApptInfo->dwResourceList->GetAt(0));

		for (long i=1; i < pApptInfo->dwResourceList->GetSize(); i++)
		{
			strAllCheckSQL += "OR ID = ? ";
			AddParameterLong(pCmd, "ResID", pApptInfo->dwResourceList->GetAt(i));
		}

		//end the query
		strAllCheckSQL += ";\r\n";

		// (d.thompson 2010-05-13) - PLID 38643 - Parameterized part (2) -- combine the next check into this check.
		strAllCheckSQL += "SELECT TOP 1 Name FROM RoomsT "
			"WHERE ID IN (SELECT RoomID FROM RoomAppointmentsT "
			"	INNER JOIN AppointmentsT ON RoomAppointmentsT.AppointmentID = AppointmentsT.ID "
			"	WHERE RoomAppointmentsT.StatusID NOT IN (0, -1) "
			"	AND AppointmentsT.ID = ? AND AppointmentsT.LocationID <> ?);\r\n";
		
		AddParameterLong(pCmd, "ApptID", pApptInfo->nApptID);
		AddParameterLong(pCmd, "LocationID", pApptInfo->nLocationID);
		//End query check (2)

		pCmd->PutCommandText(_bstr_t(strAllCheckSQL));
		_RecordsetPtr prsExistingResources = CreateRecordset(pCmd);
		long nRecords = prsExistingResources->RecordCount;
		if (nRecords == 0)
		{
			// (c.haag 2003-07-29 10:41) - If we get here, none of the resources exist.
			if (! bSilent) {
				MsgBox("This appointment cannot be saved because all of the resources you have selected have been deleted by another user. Please select another resource before saving the appointment.");
				//GetDlgItem(IDC_APTRESOURCE_COMBO)->SetFocus();
				//m_dlAptResource->PutDropDownState(VARIANT_TRUE);
			}
			return FALSE;
		}
		else if (nRecords < pApptInfo->dwResourceList->GetSize())
		{
			// (c.haag 2003-07-29 10:41) - If we get here, some of the resources exist.
			if (! bSilent) {
				MsgBox("This appointment cannot be saved because at least one of the resources you have selected have been deleted by another user. Please select another resource before saving the appointment.");
			}
			//GetDlgItem(IDC_APTRESOURCE_COMBO)->SetFocus();
			//m_dlAptResource->PutDropDownState(VARIANT_TRUE);
			return FALSE;
		}

		// (j.jones 2009-08-04 09:58) - PLID 24600 - if the appointment is in a room, and it's currently for another location,
		// stop them from changing the location
		// (d.thompson 2010-05-13) - PLID 38643 - Parameterized and combined into the above trip to the server.  Move forward 1
		//	recordset to pick it up.
		//Query part (2)
		_RecordsetPtr rsCheckRooms = prsExistingResources->NextRecordset(NULL);
		if(!rsCheckRooms->eof) {
			CString strName = AdoFldString(rsCheckRooms, "Name", "");
			CString strWarn;
			strWarn.Format("This appointment is currently marked as being in the room '%s'.\n"
				"This appointment cannot have its location changed while it is in a room in the Room Manager.", strName);
			MessageBox(GetActiveWindow(), strWarn, "Practice", MB_ICONEXCLAMATION|MB_OK);
			return FALSE;
		}
		rsCheckRooms->Close();
		
		/////////////////////////////////////////////////////
		
		long nPatId = pApptInfo->nPatientID;
		long nAptTypeId = pApptInfo->nAptTypeID;
		long nLocationId = pApptInfo->nLocationID;
		CDWordArray * dwResourceList = pApptInfo->dwResourceList;
		CDWordArray * dwPurposeList = pApptInfo->dwPurposeList;

		// Now make sure this appointment follows the rules
		BOOL bRes = TRUE;
		if (bCheckRules)
		{
			// (c.haag 2010-03-08 17:20) - PLID 37326 - Test everything but template rules first
			bRes = (AppointmentValidateByPermissions(nPatId, *dwResourceList, nLocationId, dtDate, dtStart, dtEnd, nAptTypeId, *dwPurposeList, pApptInfo->nApptID)
				&& AppointmentValidateByDuration(dtStart, dtEnd, nAptTypeId, *dwResourceList, *dwPurposeList)				
				&& AppointmentValidateByAlarms(nPatId, dtDate, dtStart, nAptTypeId, *dwPurposeList, *dwResourceList, pApptInfo->nApptID));
			// Now test template rules only if the other tests succeeded and we are allowed to validate them
			if (bRes && bValidateTemplates) {
				bRes = AppointmentValidateByRules(nPatId, *dwResourceList, nLocationId, dtDate, dtStart, dtEnd, nAptTypeId, *dwPurposeList, pApptInfo->nApptID, FALSE, FALSE, nLineItemOverrideID);
			} else {
				// Either a prior test failed, or, we're not allowed to validate template rules (probably because the appointment
				// times, resources, purposes, and type didn't change)
			}

			// (j.jones 2014-11-25 17:22) - PLID 64178 - validate against scheduler mix rules,
			// but only after all other rules have been checked
			if (bRes && bValidateMixRules) {
				long nInsuredPartyID = -1;
				if (pApptInfo->pmapInsInfo) {
					InsuranceInfo *pInsInfo = NULL;
					if (pApptInfo->pmapInsInfo->Lookup(1, pInsInfo) && pInsInfo != NULL) {
						nInsuredPartyID = pInsInfo->nInsuredPartyID;
					}
				}

				bRes = AppointmentValidateByMixRules(pParentWnd, nPatId, dtDate, nLocationId, nInsuredPartyID, nAptTypeId, *dwResourceList, *dwPurposeList, overriddenMixRules, pSelectedFFASlot, pApptInfo->nApptID);
			}
		}

		if (!bRes)
			return FALSE;

		if (bCheckSurgeryCenter) {
			if(IsSurgeryCenter(false)) {

				//see if they changed the patient, and there are case histories
				// (d.thompson 2010-05-13) - PLID 38643 - Parameterized
				_RecordsetPtr rs = CreateParamRecordset("SELECT Count(ID) AS CountOfID FROM CaseHistoryT WHERE AppointmentID = {INT} AND PersonID <> {INT}", pApptInfo->nApptID, nPatId);
				if(!rs->eof) {
					long count = AdoFldLong(rs, "CountOfID",0);
					if(count > 0) {
						CString str;
						if(count > 1) {
							str.Format("There are %li case histories attached to this appointment that belong to a different patient.\n"
								"If you continue, they will be detached from this appointment.\n"
								"Are you sure you wish to continue?",count);
						}
						else {
							str = "There is a case history attached to this appointment that belongs to a different patient.\n"
								"If you continue, it will be detached from this appointment.\n"
								"Are you sure you wish to continue?";
						}
						
						if(IDNO == MsgBox(MB_YESNO, str))
							return FALSE;
					}
				}
				rs->Close();
			}
		}
			

		if (bCheckOverWriteIn) {

			//(e.lally 2005-07-11) PLID 16782 - If an appointment is changed to be 'In'
			//we need to see if they want to overwrite the previous timestamp (if any) in the history and
			//continue saving
			//check if appointment was changed to 'In'
			if(pApptInfo->nApptID != -1 && pApptInfo->nAptInitStatus != 1 && pApptInfo->nShowStateID == 1){
				if(CheckOverwriteMarkIn(pApptInfo->nApptID)==FALSE){
					return FALSE;
				}
			}//end if changed to marked In
		}

		//make sure the view is not null
		if (pView) {
			// (c.haag 2003-08-07 16:48) - Check to see if the appointment was modified
			CString strModifiedLogin;
			if (pView->WasTrackedApptModified(FALSE, strModifiedLogin))
			{
				if (IDNO == MsgBox(MB_YESNO, "This appointment was changed by %s since you started to modify it. Do you wish to overwrite that user's changes?", strModifiedLogin))
				{
					return FALSE;
				}
			}
			pView->ResetTrackedAppt();
		}

		return TRUE;
	} NxCatchAll("CResEntryDlg::ValidateData");

	return FALSE;
}

BOOL IsAptTypeCategorySurgical(short nAptTypeCategory)
{
	switch(nAptTypeCategory)
	{
	case PhaseTracking::AC_MINOR:	// minor procedure
	case PhaseTracking::AC_SURGERY:	// surgery
	case PhaseTracking::AC_OTHER:	// other procedure
		return TRUE;
		break;
	}

	return FALSE;
}

BOOL IsAptTypeCategoryNonProcedural(short nAptTypeCategory)
{
	switch(nAptTypeCategory)
	{
	case PhaseTracking::AC_NON_PROCEDURAL:
	case PhaseTracking::AC_BLOCK_TIME:
		return TRUE;
		break;
	}

	return FALSE;
}

BOOL HasPermissionForResource(long nAppointmentID, ESecurityPermissionType bioAptPerm, ESecurityPermissionType bioAptPermWithPass) {

	//They must have permission to the given ability of bioAppointment
	//and they must also have permission to bioSchedIndivResources - sptWrite ON THIS RESOURCE,
	//if we want to return TRUE

	// (d.thompson 2010-05-11) - PLID 38581 - Parameterized
	_RecordsetPtr rs = CreateParamRecordset("SELECT ResourceID FROM AppointmentResourceT WHERE AppointmentID = {INT}",nAppointmentID);
	while(!rs->eof) {
		
		long nResourceID = AdoFldLong(rs, "ResourceID");

		BOOL bAccessAll = (GetCurrentUserPermissions(bioAppointment) & bioAptPerm);
		BOOL bAccessAllWithPass = (GetCurrentUserPermissions(bioAppointment) & bioAptPermWithPass);
		BOOL bWriteThis = (GetCurrentUserPermissions(bioSchedIndivResources, TRUE, nResourceID) & sptWrite);
		BOOL bWriteThisWithPass = (GetCurrentUserPermissions(bioSchedIndivResources, TRUE, nResourceID) & sptWriteWithPass);

		if((!bAccessAll || !bWriteThis) && !((bWriteThisWithPass || bAccessAllWithPass) && CheckCurrentUserPassword())) {
			// (a.walling 2010-08-02 11:01) - PLID 39182 - Consolidating all these copies of "You do not have permission to access this function"
			// messageboxes with PermissionsFailedMessageBox
			PermissionsFailedMessageBox();
			return FALSE;
		}

		rs->MoveNext();
	}
	rs->Close();

	return TRUE;
}

// Generate a list of the resource IDs this user is not allowed to see
CString CalcExcludedResourceIDs()
{
	CString strAns;

	// Loop through the recordset of all bioSchedIndivResources records in the SecurityObjectT table
	// (d.thompson 2010-05-11) - PLID 38581 - Parameterized
	_RecordsetPtr prs = CreateParamRecordset("SELECT ObjectValue FROM SecurityObjectT WHERE BuiltInID = {INT}", bioSchedIndivResources);
	FieldPtr pfldObjectValue = prs->GetFields()->GetItem("ObjectValue");
	while (!prs->eof) {
		long nObjectValue = VarLong(pfldObjectValue->GetValue());
		// See whether this user is allowed to see the given objectvalue
		if (!(GetCurrentUserPermissions(bioSchedIndivResources, TRUE, nObjectValue) & sptRead)) {
			// The user is NOT allowed to see this ID, so add it to the list
			CString str;
			str.Format("%li, ", nObjectValue);
			strAns += str;
		}
		prs->MoveNext();
	}
	prs->Close();
	
	// Get rid of the last ", " if there is one
	DeleteEndingString(strAns, ", ");

	return strAns;
}

void MakeResLookLikeTemplateItem(LPDISPATCH lpRes, const CString& strTemplateName, COLORREF clrTemplate)
{
	//
	// (c.haag 2006-12-05 09:59) - PLID 23666 - This function changes the appearance of a
	// reservation object to look like a template item. This is for special cases where templates
	// must appear as clickable appointments
	// (c.haag 2007-01-11 14:37) - We now check for nulls
	//
	CReservation pRes(__FUNCTION__, lpRes);
	if (NULL == pRes) return;
	CSingleDay pSingleDay = pRes.GetSingleDayCtrl();	
	if (NULL == pSingleDay) return;
	COLORREF clrFore = pSingleDay.GetBlockTextColor(clrTemplate);
	CString strBoxName = CString("%1b") + strTemplateName; // Show the name in bold
	pRes.PutText((LPCTSTR)strBoxName);
	pRes.PutBackColor(clrTemplate);
	pRes.PutForeColor(clrFore);
	pRes.PutBorderColor(clrFore);
}

// (z.manning, 03/13/2007) - PLID 23635 - Returns the start and end times for the visible range of the schedule based on the preference.
void GetScheduleVisibleTimeRange(IN CLocationOfficeHours lohOfficeHours, IN int nDayOfWeek, OUT COleDateTime &dtStart, OUT COleDateTime &dtEnd)
{
	// (z.manning, 10/12/2006) - PLID 5812 - Based on the global preference, set the range of times
	// that should be visible on the scheduler.

	long nOption = GetRemotePropertyInt("ScheduleTimeRangeOption", 0, 0, "<None>");
	COleDateTime dtMidnight;
	dtMidnight.SetTime(0,0,0);

	switch(nOption)
	{
	case svtroAllTimes:
		dtStart = dtMidnight;
		dtEnd = dtMidnight;
		break;

	case svtroOfficeHours:
		//OK, the A.M. time will either be the opening time, or, if the office is closed, 8:00 AM
		// (z.manning, 03/13/2007) - Note: our office hours class uses 0 - 6 for the day of week,
		// but COleDateTime uses 1 - 7, hence the minus one here.
		if(!lohOfficeHours.GetOfficeHours(nDayOfWeek - 1, dtStart, dtEnd)) {
			// (z.manning, 11/28/2006) - PLID 5812 - Ok, this day doesn't have office hours, meaning the location is
			// probably close on this day. So, let's try to find the day with the minimum amount of hours open
			// and use them.
			if(!lohOfficeHours.GetMinOfficeHours(dtStart, dtEnd)) {
				// Apparently no days have office hours, just use the full range.
				dtStart = dtMidnight;
				dtEnd = dtMidnight;
			}
		}
		break;

	case svtroCustom:
	{
		//(e.lally 2008-01-12) PLID 32682 - changed scheduler time range custom times to be datetimes instead of strings.
		//(e.lally) FYI, I updated the default string in the Preferences to not include the year for these reasons:
			// A. The year 12/31/1899 used with this preference can actually break the time labels on the scheduler in Practice 
				//(because it expects the year to be the true date default of 12/30/1899, don't ask me why it matters).
			// B. The old preferences code saved the string format with the flag to only use the time.
			// C. The Practice code (here) uses the default string value as just the time.

		COleDateTime dtDefault;
		CString strDefault = GetRemotePropertyText("ScheduleTimeRangeStart", "12:00 AM", 0, "<None>", false);
		dtDefault.ParseDateTime(strDefault);
		dtStart = GetRemotePropertyDateTime("ScheduleTimeRangeCustomStart", &dtDefault, 0, "<None>", true);

		strDefault = GetRemotePropertyText("ScheduleTimeRangeEnd", "11:59 PM", 0, "<None>", false);
		dtDefault.ParseDateTime(strDefault);
		dtEnd = GetRemotePropertyDateTime("ScheduleTimeRangeCustomEnd", &dtDefault, 0, "<None>", true);
		break;
	}
	default:
		ASSERT(FALSE);
		dtStart = dtMidnight;
		dtEnd = dtMidnight;
		break;
	}
}


// (z.manning, 05/10/2007) - PLID 11593 - This function will return the multi purpose string for the current
// reservation in the given read set. Note: this code was just copied from CNxSchedulerDlg::UpdateReservations().
CString GetMultiPurposeStringFromCurrentReservationReadSetPosition(CReservationReadSet &rs)
{
	// For this appointment loop through all records until we hit one with a different appointment 
	// id, or a different resource id.  Then we know we're on the next reservation.
	try 
	{
		CString strMultiPurpose;
		long nResID = rs.GetID();
		long nResourceID = rs.GetResourceID();
		strMultiPurpose.Empty();
		for (; !rs.IsEOF() && rs.GetID() == nResID && rs.GetResourceID() == nResourceID; rs.MoveNext())
		{
			if (strMultiPurpose.IsEmpty()) {
				strMultiPurpose = rs.GetAptPurpose();
			}
			else {
				strMultiPurpose += CString(", ") + rs.GetAptPurpose();
			}
		}
		// Now we've moved too far because we're on the NEXT reservation, move back one so we're on the 
		// last record of THIS reservation (because we still need to get info out of this reservation)
		rs.MovePrev();

		return strMultiPurpose;

	}NxCatchAllThrow("CNxSchedulerDlg::GetMultiPurposeStringFromCurrentReservationReadSetPosition");
}


// (d.moore 2007-07-03 09:14) - PLID 4013 - If the date, time, or resources changed for the
//  appointment, then check to see if there is a close match on the move-up/waiting list and 
//  prompt the user to warn them.
void CheckAppointmentAgainstWaitingList (long nAppointmentID, long nPatientID, 
	const CDWordArray& adwResourceID, const COleDateTime &dtDate, 
	const COleDateTime &dtStartTime)
{
	try {
		// (d.moore 2007-10-26) - PLID 26546 - We need to check that there is a valid
		//  appointment ID before proceeding.
		if (nAppointmentID <= 0) {
			return;
		}
		
		// Put all of the resouce IDs into a string for use in queries.
		CString strResourceIdList, strResourceID;
		for (long i = 0; i < adwResourceID.GetSize(); i++) {
			strResourceID.Format("%li, ", adwResourceID[i]);
			strResourceIdList += strResourceID;
		}
		strResourceIdList = strResourceIdList.Left(strResourceIdList.GetLength()-2);
		CString strResourceIdQuery;
		if (strResourceIdList.GetLength() > 0) {
			strResourceIdQuery.Format(" OR WaitingListItemResourceT.ResourceID IN (%s)", strResourceIdList);
		}

		// Format the date and time as strings for the queries.
		// (d.moore 2007-10-31) - PLID 4013 - Changed the date formatting for internationalization.
		CString strDate = FormatDateTimeForSql(dtDate, dtoDate);
		CString strTime = FormatDateTimeForSql(dtStartTime, dtoTime);
		
		// Find any waiting list items that match the patient for the date and time.
		//  These don't need to be exact matches, so we are not checking appointment
		//  type or purpose at this point.
		// (d.thompson 2010-05-13) - PLID 38643 - Parameterized as best we can.  Resource lists are fairly small
		//	and this will help out somewhat with parameterization.
		_RecordsetPtr rs = CreateParamRecordset(
			FormatString("SELECT DISTINCT WaitingListT.ID "
			"FROM WaitingListT "
				"INNER JOIN WaitingListItemT "
				"ON WaitingListT.ID = WaitingListItemT.WaitingListID "
				"LEFT JOIN WaitingListItemResourceT "
				"ON WaitingListItemT.ID = WaitingListItemResourceT.ItemID "
				"LEFT JOIN AppointmentsT "
				"ON WaitingListT.AppointmentID = AppointmentsT.ID "
			"WHERE "
				"WaitingListT.PatientID = {INT} AND "
				"WaitingListItemT.StartDate <= {STRING} AND "
				"WaitingListItemT.EndDate >= {STRING} AND "
				"WaitingListItemT.StartTime <= {STRING} AND "
				"WaitingListItemT.EndTime >= {STRING} AND "
				"(AppointmentsT.Status IS NULL "
					"OR AppointmentsT.Status <> 4) AND " // (d.moore 2007-10-18) - PLID 26546 - Added a filter to prevent canceled appointments from being selected.
				"(WaitingListItemT.AllResources = 1 %s)", strResourceIdQuery),
			nPatientID, strDate, strDate, strTime, strTime);

		CArray<long, long> arIdList;
		long nTempID;
		while (!rs->eof) {
			nTempID = AdoFldLong(rs, "ID", -1);
			if (nTempID > 0) {
				arIdList.Add(nTempID);
			}
			rs->MoveNext();
		}

		if (arIdList.GetSize() == 0) {
			// If there are no items in the list then there is nothing 
			//  further that needs to be done.
			return;
		}

		// Show the user the list of waiting list matches and let
		//  them decide what to do with them.
		CWaitingListMatches dlg(nAppointmentID, arIdList, NULL);
		dlg.DoModal();

	} NxCatchAll("Error In: CheckAppointmentAgainstWaitingList");
}

BOOL CanShowOpenAllocationsInSchedule()
{
	// (c.haag 2008-03-24 09:30) - PLID 29328 - Returns TRUE if we can show open allocation labels
	// on a schedule list.
	//DRT - PLID 30117 - Use BetaHasAdvInventory() for now.  Remove this when beta period is over.
	// (d.thompson 2009-01-27) - PLID 32859 - Replaced beta with C&A module
	if (!g_pLicense->HasCandAModule(CLicense::cflrSilent)) {
		// Fail because we don't have advanced inventory licensing
		return FALSE;
	}
	else if (!GetRemotePropertyInt("SchedShowOpenAllocations", 1, 0, "<None>", true)) {
		// Fail because the preference to display them is off
		return FALSE;
	}
	else {
		// Everything checks out
		return TRUE;
	}
}

BOOL CanShowOpenOrdersInSchedule()
{
	// (c.haag 2008-03-24 09:30) - PLID 29328 - Returns TRUE if we can show open order labels
	// on a schedule list.
	//DRT - PLID 30117 - Use BetaHasAdvInventory() for now.  Remove this when beta period is over.
	// (d.thompson 2009-01-27) - PLID 32859 - Replaced beta with C&A module
	if (!g_pLicense->HasCandAModule(CLicense::cflrSilent)) {
		// Fail because we don't have advanced inventory licensing
		return FALSE;
	}
	else if (!GetRemotePropertyInt("SchedShowOpenOrders", 1, 0, "<None>", true)) {
		// Fail because the preference to display them is off
		return FALSE;
	}
	else {
		// Everything checks out
		return TRUE;
	}
}

// (j.jones 2008-03-24 14:59) - PLID 29388 - TrySendAppointmentTablecheckerForInventory takes in an appt. ID,
// and a boolean for if it is called from an allocation (FALSE means it is an order), checks the
// related scheduler preference, then sends the tablechecker
void TrySendAppointmentTablecheckerForInventory(long nApptID, BOOL bIsAllocation)
{
	try {

		if(nApptID == -1) {
			return;
		}

		//you need the advanced inventory license to use this feature
		//DRT - PLID 30117 - Use BetaHasAdvInventory() for now.  Remove this when beta period is over.
		// (d.thompson 2009-01-27) - PLID 32859 - Replaced beta with C&A module
		if(!g_pLicense->HasCandAModule(CLicense::cflrSilent)) {
			return;
		}

		if(bIsAllocation) {
			//this was called from an allocation
			if(!CanShowOpenAllocationsInSchedule()) {
				//they aren't displaying allocations, so leave now
				return;
			}
		}
		else {
			//this was called from an order
			if(!CanShowOpenOrdersInSchedule()) {
				//they aren't displaying orders, so leave now
				return;
			}
		}

		// (a.wilson 2014-08-12 10:43) - PLID 63199 - changed to use refreshappointmenttable instead of refreshtable.
		//if we get here, we should send an appt tablechecker
		CClient::RefreshAppointmentTable(nApptID);
		
	}NxCatchAll("Error in TrySendInventoryAppointmentTablechecker");
}

//DRT 6/12/2008 - PLID 9679 - This function takes an appointment ID, and will return all
//	the default superbill templates which apply to that ID, using the Configure Superbill Templates
//	dialog setup data.  That data is processed in order -- so if a type matches, it uses the type
//	and will ignore the rest.  Then falls down to resources, pick list, and prompt or global.
void GetDefaultSuperbillTemplates(IN long nApptID, OUT CStringArray *pary, OUT bool &bPromptIfEmpty, IN eDefaultSuperbillPathFormat eOutputFormat)
{
	//Here's the plan:
	//	 - We will build 1 query which will gather al the necessary data, depending which
	//	options are turned on.
	//	 - Execute that query, then step through the results.  We'll quit as soon as we
	//	find valid data, so parts of the query may not be referenced.
	//	 - It is the callers responsibility to form their display of this data however they would like.
	//	 - I attempt to sort by name just so it looks nice, but there is no implied order to the templates assigned.

	//
	//Ensure parameters
	if(!pary) {
		//stupid, do nothing
		return;
	}

	//
	//Determine what options are set
	bool bUseTypes = GetRemotePropertyInt("SuperbillConfig_Types", 0, 0, "<None>", true) == 0 ? false : true;
	bool bUseResources = GetRemotePropertyInt("SuperbillConfig_Resources", 0, 0, "<None>", true) == 0 ? false : true;
	bool bUsePickList = GetRemotePropertyInt("SuperbillConfig_PickList", 0, 0, "<None>", true) == 0 ? false : true;
	bPromptIfEmpty = GetRemotePropertyInt("SuperbillConfig_UseGlobal", 1, 0, "<None>", true) == 1 ? false : true;

	//
	//Build the SQL statement.
	CString strSqlBatch = BeginSqlBatch();
	CNxParamSqlArray aryParams;
	long nQueriesExecuted = 0;

	//We only need the ApptID if they are using types or resources
	if(bUseTypes || bUseResources) {
		AddParamStatementToSqlBatch(strSqlBatch, aryParams, 
			"DECLARE @ApptID int;\r\n "
			"SET @ApptID = {INT};\r\n ", nApptID);
	}

	//Add type query
	if(bUseTypes) {
		AddParamStatementToSqlBatch(strSqlBatch, aryParams, 
			"SELECT MergeTemplatesT.Path "
			"FROM SuperbillTemplateTypeT INNER JOIN SuperbillTemplateTypePathsT ON SuperbillTemplateTypeT.GroupID = SuperbillTemplateTypePathsT.GroupID "
			"INNER JOIN MergeTemplatesT ON SuperbillTemplateTypePathsT.TemplateID = MergeTemplatesT.ID "
			"WHERE SuperbillTemplateTypeT.TypeID IN (SELECT AptTypeID FROM AppointmentsT WHERE ID = @ApptID) "
			"ORDER BY MergeTemplatesT.Path;\r\n ");
		nQueriesExecuted++;
	}

	//Add resource query
	if(bUseResources) {
		AddParamStatementToSqlBatch(strSqlBatch, aryParams, 
			"SELECT MergeTemplatesT.Path "
			"FROM SuperbillTemplateResourceT INNER JOIN SuperbillTemplateResourcePathsT ON SuperbillTemplateResourceT.GroupID = SuperbillTemplateResourcePathsT.GroupID "
			"INNER JOIN MergeTemplatesT ON SuperbillTemplateResourcePathsT.TemplateID = MergeTemplatesT.ID "
			"WHERE SuperbillTemplateResourceT.ResourceID IN (SELECT ResourceID FROM AppointmentResourceT WHERE AppointmentID = @ApptID) "
			"ORDER BY MergeTemplatesT.Path;\r\n ");
		nQueriesExecuted++;
	}

	//Add pick list query
	if(bUsePickList) {
		AddParamStatementToSqlBatch(strSqlBatch, aryParams, 
			"SELECT MergeTemplatesT.Path "
			"FROM SuperbillTemplatePickListT INNER JOIN MergeTemplatesT ON SuperbillTemplatePickListT.TemplateID = MergeTemplatesT.ID "
			"ORDER BY MergeTemplatesT.Path;\r\n ");
		nQueriesExecuted++;
	}

	//If we have any text to query, run it
	if(!strSqlBatch.IsEmpty()) {
		// (e.lally 2009-06-21) PLID 34679 - Fixed to use create recordset function.
		_RecordsetPtr prsTemplates = CreateParamRecordsetBatch(GetRemoteData(), strSqlBatch, aryParams);

		//At this point our query returned some queries to us.  We'll start at the top and progress until we either find some templates, or
		//	run out of queries.
		for(int i = 0; i < nQueriesExecuted; i++) {
			while(!prsTemplates->eof) {
				//The path we get out is always \Templates\Forms\file.  Superbills can only exist in the \templates\forms\ (or subfolders of that), 
				//	never outside that or in another location.
				CString strPath = AdoFldString(prsTemplates, "Path", "");

				//We then want to format according to the users wishes
				switch(eOutputFormat) {
				case eFmtSuperbillPath:
					//Remove the preceeding '\Templates\Forms\' from the path.
					strPath = strPath.Mid(17);
					break;
				case eFmtFullPath:
					//Prepend the shared path so they can just open this whole thing.
					strPath = GetSharedPath() ^ strPath;
					break;
				}

				//Add to the array
				pary->Add(strPath);

				prsTemplates->MoveNext();
			}

			//If we have anything in our array, we quit now, no more processing to do.
			if(pary->GetSize() > 0) {
				return;
			}

			//Otherwise, we continue to iterate through our recordsets
			prsTemplates = prsTemplates->NextRecordset(NULL);
		}
	}

	//If we are still executing at this point, it means that nothing was returned from the type, resource, or pick list options.
	//	We need to fall back to either the prompt preference, or the global default template.

	//bPromptIfEmpty is the status.  If it's set, it means the configuration wants us to prompt.  We don't need anything in the array, 
	//	so we can quit now.
	if(bPromptIfEmpty) {
		return;
	}

	//Finally, if we reach this point, there are no templates setup to merge, and we want to use the global default (per-machine)
	CString strPath = GetPropertyText("DefaultSuperbillFilename", "", 0, false);
	//We then want to format according to the users wishes
	switch(eOutputFormat) {
	case eFmtSuperbillPath:
		//This property is already the path starting after \Templates\Forms\, so do nothing.
		break;
	case eFmtFullPath:
		//Prepend the shared path so they can just open this whole thing.
		strPath = GetSharedPath() ^ "Templates\\Forms" ^ strPath;
		break;
	}

	if(!strPath.IsEmpty()) {
		pary->Add(strPath);
	}
}

//TES 6/12/2008 - PLID 28078 - Check whether this appointment requires creating an allocation, and prompt the user to create
// one if so. 
void CheckCreateAllocation(long nAppointmentID, long nPatientID, long nAptTypeID, const CDWordArray &dwaPurposes, long nLocationID)
{
	//TES 6/13/2008 - PLID 28078 - Do we have the advanced inventory license?
	// (d.thompson 2009-01-27) - PLID 32859 - Replaced beta with C&A module
	if(!g_pLicense->HasCandAModule(CLicense::cflrSilent)) {
		return;
	}

	//TES 6/12/2008 - PLID 28078 - Do we have a valid patient, type, and purpose?
	if(nPatientID == -25) {
		return;
	}
	if(nAptTypeID == -1) {
		return;
	}
	if(!dwaPurposes.GetSize()) {
		return;
	}

	//TES 6/12/2008 - PLID 28078 - Do this type and purpose require an allocation?
	//TES 8/4/2008 - PLID 28078 - Revised this logic to have a single, parameterized query.  If the appointment is tied to 
	// an order or undeleted allocation, then it will return no records, otherwise it will return all the purposes for which
	// there is a requirement with the given type.
	_RecordsetPtr rsPurposes = CreateParamRecordset("SELECT AptPurposeID "
		"FROM ApptsRequiringAllocationsDetailT "
		"WHERE AptTypeID = {INT} "
		"AND {INT} NOT IN "
		"	(SELECT AppointmentID FROM PatientInvAllocationsT "
		"	WHERE AppointmentID Is Not Null "
		"	AND Status <> {INT} "
		"	UNION SELECT AppointmentID FROM OrderAppointmentsT)",
		nAptTypeID, nAppointmentID, (long)InvUtils::iasDeleted);

	//TES 8/4/2008 - PLID 28078 - Now check whether there are any required purposes match a purpose on the appointment.
	bool bMatched = false;
	while(!rsPurposes->eof && !bMatched) {
		long nPurposeID = AdoFldLong(rsPurposes, "AptPurposeID");
		for(int i = 0; i < dwaPurposes.GetSize() && !bMatched; i++) {
			if(dwaPurposes[i] == (DWORD)nPurposeID) {
				bMatched = true;
			}
		}
		rsPurposes->MoveNext();
	}
	if(bMatched) {
		//TES 6/12/2008 - PLID 28078 - We need to prompt them.
		CPromptForAllocationDlg dlg(NULL);
		//TES 6/13/2008 - PLID 28078 - Tell the dialog what they're allowed to do, and make sure there's something they
		// can do before popping up the dialog.
		dlg.m_bCanCreateAllocation = (GetCurrentUserPermissions(bioInventoryAllocation) & SPT____C_______ANDPASS);
		dlg.m_bCanCreateOrder = (GetCurrentUserPermissions(bioInvOrder) & SPT____C_______ANDPASS);
		if(dlg.m_bCanCreateAllocation || dlg.m_bCanCreateOrder) {
			int nReturn = dlg.DoModal();
			if(nReturn == CREATE_ALLOCATION) {
				//TES 6/13/2008 - PLID 28078 - Check their permission, and pop up an allocation entry dialog.
				if(CheckCurrentUserPermissions(bioInventoryAllocation, sptCreate)) {
					CInvPatientAllocationDlg dlg(NULL);
					dlg.m_nID = -1;
					dlg.m_nDefaultPatientID = nPatientID;
					dlg.m_nDefaultAppointmentID = nAppointmentID;
					dlg.m_nDefaultLocationID = nLocationID;
					dlg.DoModal();
				}
			}
			else if(nReturn == CREATE_ORDER) {
				//TES 6/13/2008 - PLID 28078 - Check their permission, and pop up an order entry dialog.
				if(CheckCurrentUserPermissions(bioInvOrder, sptCreate)) {
					CInvEditOrderDlg dlg(NULL);
					dlg.DoModal(-1, FALSE, nAppointmentID, nLocationID);
				}
			}
		}
	}

}

// (z.manning 2008-07-23 15:00) - PLID 30804 - Move here from CNxSchedulerDlg
BOOL IsCurrentBlockBetterFit(BlockReservation& brVictim, BlockReservation& br, VisibleAppointment* pAppt,  const COleDateTime& dtStart, const COleDateTime& dtEnd)
{
	//
	// (c.haag 2007-02-28 14:57) - PLID 24191 - If an appointment has more than one block it
	// can suppress, then we need to have a competition between the blocks to see which one 
	// is a better fit.
	//

	//
	// Criteria 1: Find the block with the best time match. The time match is
	// defined as the sum of the minute differentials between the block start
	// and appointment start times, and the block end and appointment end times
	//
	COleDateTimeSpan dtsVictimStart, dtsVictimEnd, dtsBrStart, dtsBrEnd;

	if (CT_GREATER_THAN & CompareTimes(brVictim.dtStart, dtStart)) {
		dtsVictimStart = brVictim.dtStart - dtStart;
	} else {
		dtsVictimStart = dtStart - brVictim.dtStart;
	}
	if (CT_GREATER_THAN & CompareTimes(brVictim.dtEnd, dtEnd)) {
		dtsVictimEnd = brVictim.dtEnd - dtEnd;
	} else {
		dtsVictimEnd = dtEnd - brVictim.dtEnd;
	}
	if (CT_GREATER_THAN & CompareTimes(br.dtStart, dtStart)) {
		dtsBrStart = br.dtStart - dtStart;
	} else {
		dtsBrStart = dtStart - br.dtStart;
	}
	if (CT_GREATER_THAN & CompareTimes(br.dtEnd, dtEnd)) {
		dtsBrEnd = br.dtEnd - dtEnd;
	} else {
		dtsBrEnd = dtEnd - br.dtEnd;
	}

	int nVictimSpillMinutes = (int)dtsVictimStart.GetTotalMinutes() + (int)dtsVictimEnd.GetTotalMinutes();
	int nBrSpillMinutes = (int)dtsBrStart.GetTotalMinutes() + (int)dtsBrEnd.GetTotalMinutes();

	// If the number of "spillover" minutes of the current block exceeds those of
	// the current victim, then ignore the current block and continue.
	if (nBrSpillMinutes > nVictimSpillMinutes) {
		return FALSE;
	} else if (nBrSpillMinutes < nVictimSpillMinutes) {
		return TRUE;
	}

	//
	// Criteria 2: Narrow it down by appointment type. If the appointment has a type, and
	// block rules include "If the appointment type is not one of A,B,C, then don't book",
	// regardless of the All/Any flag, pick the block which has the rule which has the least
	// amount of appointment types. The most ideal rule is one that states "If the appointment
	// type is not A then don't book" and A equals pAppt->nAptTypeID.
	//
	// To do this, we go through all the rules for both blocks. Whichever has the best rule wins.
	// If both have the best rule, then proceed to criteria 3.
	//
	const VisibleTemplate* pVictimTemplate = brVictim.pTemplate;
	const VisibleTemplate* pCurrentTemplate = br.pTemplate;
	if (pAppt->nAptTypeID != -1) {

		// As mentioned before, the  most ideal rule is one that states "If the appointment
		// type is not X then don't book". In that case, the count would be 1. If there is
		// no such rule that even includes pAppt->nAptTypeID, then the count will be -1.
		int nVictimCount = GetBestFittingAptTypeCountForAnyRule(pVictimTemplate, pAppt->nAptTypeID);
		int nCurrentCount = GetBestFittingAptTypeCountForAnyRule(pCurrentTemplate, pAppt->nAptTypeID);

		if (nCurrentCount == -1) {
			if (nVictimCount != -1) {
				// If we get here, there was at least one good rule for the victim template,
				// but none for the current template. So, the current block is not a better fit.
				return FALSE;
			}
		} else {
			if (nVictimCount == -1) {
				// If we get here, there was at least one good rule for the current template,
				// but none for the victim template. So, the current block is a better fit
				return TRUE;
			} else if (nVictimCount > nCurrentCount) {
				// If we get here, then the current template has a more narrow type filter,
				// so the current block is a better fit.
				return TRUE;
			} else if (nVictimCount < nCurrentCount) {
				// If we get here, then the victim template has a more narrow type filter,
				// so the victim block is a better fit
				return FALSE;
			}
		}

	}

	//
	// Criteria 3: Narrow it down by appointment purposes. If the appointment has purposes, and
	// block rules include "If the appointment purpose is not one of A-B-C, then don't book",
	// regardless of the All/Any flag, pick the block which includes the most purposes of the
	// appointment. If both blocks include the same amount of purposes, then we pick the block
	// which has the rule with the more narrow purpose filter
	//
	if (pAppt->anPurposeIDs.GetSize() > 0) {

		int nVictimMatchingPurposes, nVictimNonMatchingPurposes;
		int nCurrentMatchingPurposes, nCurrentNonMatchingPurposes;

		// Calculate the values for the best rules in the competing templates
		GetBestFittingAptPurposeCountForAnyRule(pVictimTemplate, pAppt->anPurposeIDs,
			nVictimMatchingPurposes, nVictimNonMatchingPurposes);
		GetBestFittingAptPurposeCountForAnyRule(pCurrentTemplate, pAppt->anPurposeIDs,
			nCurrentMatchingPurposes, nCurrentNonMatchingPurposes);

		if (nCurrentMatchingPurposes == -1) {
			if (nVictimMatchingPurposes != -1) {
				// If we get here, there was at least one good rule in the victim template,
				// but none in the current template. The current block is not a better fit.
				return FALSE;
			}
		} else {
			if (nVictimMatchingPurposes == -1) {
				// If we get here, there was at least one good rule in the current template,
				// but none in the victim template. The current block is a better fit.
				return TRUE;
			} else if (nVictimMatchingPurposes > nCurrentMatchingPurposes) {
				// If we get here, the victim template is a better fit
				return FALSE;
			} else if (nVictimMatchingPurposes < nCurrentMatchingPurposes) {
				// If we get here, the current template is a better fit
				return TRUE;
			} else {
				// Ok, both templates have rules that match the same amount of purposes
				// of the appointment. So, pick the one which has less unrelated purposes
				if (nVictimNonMatchingPurposes > nCurrentNonMatchingPurposes) {
					// If we get here, the victim template has more non-matching purposes,
					// so the current template is better
					return TRUE;
				}
				else if (nVictimNonMatchingPurposes < nCurrentNonMatchingPurposes) {
					// If we get here, the current template has more non-matching purposes,
					// so the victim template is better
					return FALSE;
				}
			}
		}
	}

	// Final criteria: Pick the earlier of the two blocks
	if (br.dtStart >= brVictim.dtStart) {
		return FALSE; // The victim block wins because it's earlier!
	} else {	
		return TRUE; // The current block wins because it's earlier!
	}
}

// (z.manning 2008-07-23 15:00) - PLID 30804 - Move here from CNxSchedulerDlg
//TES 10/27/2010 - PLID 40868 - You can now use this function to calculate the TemplateItemID associated with an appointment.  To do
// so, pass in pApptToCheck and pnTemplateItemID. If you do this, the function will fill pnTemplateItemID with the precision template
// blocked by pApptToCheck, or -1 if there is no such precision template, or if pApptToCheck is not in arypAppts.  I admit it's a bit of
// a strange overload, but it ensures that the calculation will always match the display in the scheduler.
void RepopulateBlockResArray_ByAppointments(CArray<BlockReservation, BlockReservation>& aCurBlockRes, CArray<VisibleAppointment*,VisibleAppointment*> &arypAppts, short nOffsetDay, COleDateTime dtWorkingDate, int nInterval, LPDISPATCH lpdispSingleDay, OPTIONAL IN VisibleAppointment* pApptToCheck /*= NULL*/, OPTIONAL OUT long *pnTemplateItemID /*= NULL*/)
{
	CSingleDay pSingleDay(lpdispSingleDay);
	//
	// (c.haag 2006-12-04 11:58) - PLID 23845 - This function takes an array of block reservations, and populates
	// a new array of block reservations such that the new array contains time gaps whenever appointments occur.
	// For example:
	//
	// Before calling this function, aCurBlockRes has two elements:
	//		Block 1: 10:00 - 12:00
	//		Block 2: 12:00 - 3:00
	//
	// After calling this function, aCurBlockRes has three elements because there were some appointments
	// between 10:00 and 3:00:
	//		Block 1: 10:00 - 10:30
	//		Block 1: 11:00 - 11:45
	//		Block 2: 12:30 - 2:00
	//
	//
	// The strategy is:
	//
	//		1. Do for all appointments
	//			2. Determine all the blocks that the appointment can obstruct
	//			3. Build a list of times for each block such that the times obstruct one and only one block
	//		end loop
	//
	//		4. Do for all blocks
	//			5. Append times to the list of times for all blocks that go beyond the visible bounds of the singleday
	//			6. Calculate the new set of times that the block exists on. This is done by adding time ranges to 
	//				aNewBlockRes that occur inside the block, and outside the paStartTimes and paEndTimes arrays
	//		end loop
	//
	//		7. Replace aCurBlockRes with aNewBlockRes
	//
	const long nCurBlocks = aCurBlockRes.GetSize();
	const BOOL bAllowFragmentedBlocks = (GetRemotePropertyInt("ShowTemplateBlockCoveredByAppt", 1, 0, "<None>") == 0) ? FALSE : TRUE;
	const int nAppointments = arypAppts.GetSize();

	// Fail if there are no block templates
	if (0 == nCurBlocks)
		return;

	// These arrays contain lists of all times that at least partially obstruct an existing template block
	CArray<COleDateTime, COleDateTime>* paStartTimes = new CArray<COleDateTime, COleDateTime>[nCurBlocks];
	CArray<COleDateTime, COleDateTime>* paEndTimes = new CArray<COleDateTime, COleDateTime>[nCurBlocks];

	// This array is the output for this function -- this will contain block reservation objects with times that
	// do not conflict with appointments
	CArray<BlockReservation, BlockReservation> aNewBlockRes;

	// This array tells us for any given block whether it should be visible on the screen at all, or not
	BOOL* pabShowBlock = new BOOL[nCurBlocks];

	// An appointment can only obstruct one block. This array links blocks to obstructing appointments.
	long* panBlockAppointmentIDs = new long[nCurBlocks];

	int iCurBlock;
	long i;

	for (i=0; i < nCurBlocks; i++) {
		pabShowBlock[i] = TRUE;
		panBlockAppointmentIDs[i] = -1;
	}

	// 1. Do for all appointments
	for (i=0; i < nAppointments; i++) {
		VisibleAppointment* pAppt = arypAppts[i];
		if (nOffsetDay == pAppt->nOffsetDay) {
			CArray<int,int> anObstructedBlocks;
			COleDateTime dtStart = pAppt->dtStart;
			COleDateTime dtEnd = pAppt->dtEnd;

			// (c.haag 2007-03-01 09:39) - PLID 25013 - Snap the appointment times
			// to the resolution of the current view so that calculated block appearances
			// are consistent.
			//
			// To calculate the start time, all we have to do is round down
			// To calculate the end time, we also round down...but if the result is earlier
			// than the actual end time, then we need to increment it by the value of the interval
			//
			int nNewStartMinutes;
			int nNewEndMinutes;
			int nNewEndHour;
			
			// Only snap if the snapping preference is turned on
			if (nInterval > 0 && GetRemotePropertyInt("SnapResToGrid", 1, 0, "<None>", true) != 0)
			{
				// Calculate the new times
				nNewStartMinutes = (dtStart.GetMinute() / nInterval) * nInterval;
				nNewEndMinutes = (dtEnd.GetMinute() / nInterval) * nInterval;
				nNewEndHour = dtEnd.GetHour();
				if (nNewEndMinutes < dtEnd.GetMinute()) {
					nNewEndMinutes += nInterval;
					// Adjust for spillover
					while (nNewEndMinutes >= 60) {
						nNewEndHour++;
						nNewEndMinutes -= 60;
					}
					if (nNewEndHour >= 24) {
						nNewEndHour = 23;
						nNewEndMinutes = 59;
					}
				}

			} else {
				nNewStartMinutes = dtStart.GetMinute();
				nNewEndHour = dtEnd.GetHour();
				nNewEndMinutes = dtEnd.GetMinute();
			}

			dtStart.SetDateTime( dtWorkingDate.GetYear(), dtWorkingDate.GetMonth(), dtWorkingDate.GetDay(),
				dtStart.GetHour(), nNewStartMinutes, dtStart.GetSecond() );
			dtEnd.SetDateTime( dtWorkingDate.GetYear(), dtWorkingDate.GetMonth(), dtWorkingDate.GetDay(),
				nNewEndHour, nNewEndMinutes, dtEnd.GetSecond() );

			// 2. Determine all the blocks that the appointment can obstruct
			for (iCurBlock=0; iCurBlock < nCurBlocks; iCurBlock++) {
				if (-1 == panBlockAppointmentIDs[iCurBlock]) {
					if (AppointmentCanBeMadeOverBlockTemplate(pAppt, aCurBlockRes[iCurBlock], dtWorkingDate)) {
						anObstructedBlocks.Add(iCurBlock);
					}
				}
			}

			// 3. Build a list of times for each block such that the times obstruct one and only one block. We
			// need to do this in order of ascending block starting times. What happens is we take the earliest
			// block, add dtStart and dtEnd (which initially came from the appointment) to the block's list, then change
			// the start time to that of the block's end time so that we guarantee that only that one is suppressed
			// by the appointment in that block's time range. We repeat the process with the next earliest block,
			// and so on.
			//TES 5/14/2012 - PLID 50368 - We no longer use the time for traversal, because we don't go earliest to latest any more, 
			// we go by "best fit."  We will simply loop until all the blocks have been checked.
			while (anObstructedBlocks.GetSize() > 0 /*&& dtStart < dtEnd*/) {

				// a. Find the earliest block in the list
				const int nObstructedBlocks = anObstructedBlocks.GetSize();
				BlockReservation* pbrVictim = NULL;
				int nVictimBlock = -1;
				int nVictimIndex = -1;
				for (iCurBlock=0; iCurBlock < nObstructedBlocks; iCurBlock++) {
					BlockReservation& br = aCurBlockRes[ anObstructedBlocks[ iCurBlock ] ];
					// Check if the block is still obstructed by dtStart and dtEnd. Otherwise, it cannot be the victim.
					if (!( ((CT_GREATER_THAN | CT_EQUAL) & CompareTimes(br.dtStart, dtEnd)) || ((CT_LESS_THAN | CT_EQUAL) & CompareTimes(br.dtEnd, dtStart))) ) {

						// (c.haag 2007-02-28 15:22) - PLID 24191 - We now use more criteria than just the
						// start time to determine the best fitting block
						//if (NULL == pbrVictim || br.dtStart < pbrVictim->dtStart) {

							// (c.haag 2007-02-28 14:57) - PLID 24191 - If we already a victim block, then
							// we need to have a competition between the two blocks to see which one is a
							// better fit.
							if (NULL != pbrVictim) {
								if (!IsCurrentBlockBetterFit(*pbrVictim, br, pAppt, dtStart, dtEnd)) {
									continue;
								}
							}

							pbrVictim = &br;
							nVictimBlock = anObstructedBlocks[ iCurBlock ];
							nVictimIndex = iCurBlock;
						//}
					}
				}

				// b. Add the current start and end times. It doesn't matter if the times span before or after
				// the block; all that matters is that it at least partially overlaps (which we already established
				// in AppointmentCanBeMadeOverBlockTemplate)
				//
				if (NULL != pbrVictim) {
					//TES 10/27/2010 - PLID 40868 - If we were asked to return a template item ID for this appointment, do so now.
					if(pAppt == pApptToCheck) {
						ASSERT(pnTemplateItemID);
						*pnTemplateItemID = pbrVictim->pTemplate->nTemplateItemID;
						delete[] paStartTimes;
						delete[] paEndTimes;
						delete[] pabShowBlock;
						delete[] panBlockAppointmentIDs;
						return;
					}

					// (c.haag 2006-12-05 16:58) - PLID 23845 - If this conditional is true, then we must
					// ignore the block because a block that is at least partially covered by an appointment
					// must be fully hidden
					if (!bAllowFragmentedBlocks) {
						pabShowBlock[nVictimBlock] = FALSE;
					} else {
						paStartTimes[nVictimBlock].Add(dtStart);
						paEndTimes[nVictimBlock].Add(dtEnd);
					}
					panBlockAppointmentIDs[nVictimBlock] = pAppt->nReservationID;

					// c. Update the start time to be the end time of the current block
					// to guarantee that only one block is covered by the appointment at a given time
					//TES 5/14/2012 - PLID 50368 - This assumes that we are always traversing the blocks chronologically, which is not true
					// (we go by "best fit" now), so there may be blocks before the victim that should still be considered.
					//dtStart = pbrVictim->dtEnd;

					// d. Remove the obstructed block from the list, and continue
					anObstructedBlocks.RemoveAt(nVictimIndex);

					//TES 5/14/2012 - PLID 50368 - Now remove all the blocks which overlap our victim block, we know that we've already
					// checked them and found them to be a worse fit than the victim, so they no longer need analysis.
					for(int iCheckBlock = nObstructedBlocks-2; iCheckBlock >= 0; iCheckBlock--) {
						BlockReservation& br = aCurBlockRes[ anObstructedBlocks[ iCheckBlock ] ];
						if (!( ((CT_GREATER_THAN | CT_EQUAL) & CompareTimes(br.dtStart, pbrVictim->dtEnd)) || ((CT_LESS_THAN | CT_EQUAL) & CompareTimes(br.dtEnd, pbrVictim->dtStart))) ) {
							anObstructedBlocks.RemoveAt(iCheckBlock);
						}
					}


				} else {

					// d. No blocks overlap the resultant time range, so we're done
					anObstructedBlocks.RemoveAll();
				}

			} // while (anObstructedBlocks.GetSize() > 0 && dtStart < dtEnd) {


		} // if (nOffsetDay == pAppt->nOffsetDay) {
		//TES 10/27/2010 - PLID 40868 - If we were asked to return a template item ID for this appointment, then we couldn't find one,
		// so return -1.
		if(pAppt == pApptToCheck) {
			ASSERT(pnTemplateItemID);		
			*pnTemplateItemID = -1;
			delete[] paStartTimes;
			delete[] paEndTimes;
			delete[] pabShowBlock;
			delete[] panBlockAppointmentIDs;
			return;
		}
	} // for (i=0; i < nAppointments; i++) {

	// 4. Do for all blocks
	for (iCurBlock=0; iCurBlock < nCurBlocks; iCurBlock++) {
		const COleDateTime dtTemplateItemStart = aCurBlockRes[iCurBlock].dtStart;
		const COleDateTime dtTemplateItemEnd = aCurBlockRes[iCurBlock].dtEnd;
		COleDateTime dtStart, dtEnd;

		// If we've determined that the block should be completely invisible, just move along to the next one
		if (!pabShowBlock[iCurBlock])
			continue;

		// 5. Update the list of times for blocks that go beyond the visible times of the singleday
		//
		// (c.haag 2006-12-07 12:19) - PLID 23845 - We now consider PLID 5812 in our search; blocks should be
		// hidden if they are outside the user-defined time range of the scheduler. My solution for this is
		// to treat the times that exist outside of the user-defined time range no differently than appointments
		// that supress blocks.
		//
		// We still do the check for the fragmented blocks preference. The point of that preference is that we
		// can ensure that any block, for any reason, is not visible if it is even partially obstructed.
		//
		// (z.manning 2008-07-23 14:40) - PLID 30804 - Only do this if we have a single day control
		if(pSingleDay != NULL)
		{
			COleDateTime dtStartRange;
			COleDateTime dtEndRange;
			dtStartRange.ParseDateTime( (LPCTSTR) pSingleDay.GetBeginTime(), VAR_TIMEVALUEONLY );
			dtEndRange.ParseDateTime( (LPCTSTR) pSingleDay.GetEndTime(), VAR_TIMEVALUEONLY );
			// Only do this processing if the range is valid
			if (dtStartRange.m_dt > 0 && dtEndRange.m_dt > 0 && dtStartRange < dtEndRange) {
				COleDateTime dtMidnight;
				COleDateTime dtEndOfDay;
				dtMidnight.SetDateTime(dtWorkingDate.GetYear(), dtWorkingDate.GetMonth(), dtWorkingDate.GetDay(), 0,0,0);
				dtEndOfDay.SetDateTime(dtWorkingDate.GetYear(), dtWorkingDate.GetMonth(), dtWorkingDate.GetDay(), 23,59,59);
				dtStartRange.SetDateTime( dtWorkingDate.GetYear(), dtWorkingDate.GetMonth(), dtWorkingDate.GetDay(),
					dtStartRange.GetHour(), dtStartRange.GetMinute(), dtStartRange.GetSecond() );
				dtEndRange.SetDateTime( dtWorkingDate.GetYear(), dtWorkingDate.GetMonth(), dtWorkingDate.GetDay(),
					dtEndRange.GetHour(), dtEndRange.GetMinute(), dtEndRange.GetSecond() );

				if (!( ((CT_GREATER_THAN | CT_EQUAL) & CompareTimes(dtMidnight, dtTemplateItemEnd)) || ((CT_LESS_THAN | CT_EQUAL) & CompareTimes(dtStartRange, dtTemplateItemStart))) ) {
					if (!bAllowFragmentedBlocks) {
						pabShowBlock[iCurBlock] = FALSE;
					}
					paStartTimes[iCurBlock].Add(dtMidnight);
					paEndTimes[iCurBlock].Add(dtStartRange);
				}
				if (!( ((CT_GREATER_THAN | CT_EQUAL) & CompareTimes(dtEndRange, dtTemplateItemEnd)) || ((CT_LESS_THAN | CT_EQUAL) & CompareTimes(dtEndOfDay, dtTemplateItemStart))) ) {
					if (!bAllowFragmentedBlocks) {
						pabShowBlock[iCurBlock] = FALSE;
					}
					paStartTimes[iCurBlock].Add(dtEndRange);
					paEndTimes[iCurBlock].Add(dtEndOfDay);
				}
			}
		}

		// If we've determined that the block should be completely invisible, just move along to the next one
		if (!pabShowBlock[iCurBlock])
			continue;

		// 6. Calculate the new set of times that the block exists on
		dtStart = dtTemplateItemStart;
		while (dtStart < dtTemplateItemEnd) {

			// We must find the first time of the current block that does not fall on any existing 
			// appointments. Once we have it, we must find the next open time to end the appointment
			// at
			BOOL bFoundOverlap = FALSE;
			const int nTimes = paStartTimes[iCurBlock].GetSize();
			for (i=0; i < nTimes && !bFoundOverlap; i++) {
				// (c.haag 2006-12-04 16:04) - Zack wrote CompareTimes. I commented the traditional compare
				// so that it's easier to compare and make sure we're doing the right thing
				//if (dtStart >= aStartTimes[i] && dtStart < aEndTimes[i]) {
				if ( ((CT_GREATER_THAN | CT_EQUAL) & CompareTimes(dtStart, paStartTimes[iCurBlock].GetAt(i))) && ((CT_LESS_THAN) == CompareTimes(dtStart, paEndTimes[iCurBlock].GetAt(i))) ) {
					dtStart = paEndTimes[iCurBlock].GetAt(i);
					bFoundOverlap = TRUE;
				}
			}

			// Now we have our start time, we need to find the end time. The end time
			// is the earliest start time of any appointment after dtStart. If there
			// are no appointments, then it is the end time of the template line item
			dtEnd = dtTemplateItemEnd;
			for (i=0; i < nTimes; i++) {
				// (c.haag 2006-12-04 16:04) - Zack wrote CompareTimes. I commented the traditional compare
				// so that it's easier to compare and make sure we're doing the right thing
				//if (aStartTimes[i] >= dtStart && aStartTimes[i] < dtEnd) {
				if ( ((CT_GREATER_THAN | CT_EQUAL) & CompareTimes(paStartTimes[iCurBlock].GetAt(i), dtStart)) && ((CT_LESS_THAN) == CompareTimes(paStartTimes[iCurBlock].GetAt(i), dtEnd)) ) {
					dtEnd = paStartTimes[iCurBlock].GetAt(i);
				}
			}

			// At last, we have our times! Now make the block.
			if (CT_LESS_THAN == CompareTimes(dtStart, dtEnd)) {
				BlockReservation br;
				br.pTemplate = aCurBlockRes[iCurBlock].pTemplate;
				ASSERT(dtStart.GetDay() == dtEnd.GetDay());
				ASSERT(dtStart.GetMonth() == dtEnd.GetMonth());
				ASSERT(dtStart.GetYear() == dtEnd.GetYear());
				br.dtStart = dtStart;
				br.dtEnd = dtEnd;
				aNewBlockRes.Add(br);
			}

			// Now go to the next interval
			dtStart = dtEnd;
		} // while (dtStart < dtTemplateItemEnd) {

	} // for (int iOldBlock=0; iOldBlock < nOldBlocks; iOldBlock++) {

	// 7. Now that aNewBlockRes has our corrected list of block times, replace the contents of
	// aCurBlockRes with it.
	aCurBlockRes.RemoveAll();
	for (i=0; i < aNewBlockRes.GetSize(); i++) {
//#ifdef _DEBUG
//		TRACE(CString("Showing block ") + FormatDateTimeForInterface(aNewBlockRes[i].dtStart,dtoTime)  + "-" + FormatDateTimeForInterface(aNewBlockRes[i].dtEnd,dtoTime) + "\n");
//#endif
		aCurBlockRes.Add( aNewBlockRes[i] );
	}

	delete[] paStartTimes;
	delete[] paEndTimes;
	delete[] pabShowBlock;
	delete[] panBlockAppointmentIDs;
}

// (z.manning 2008-07-23 15:00) - PLID 30804 - Move here from CNxSchedulerDlg
void RepopulateBlockResArray_ByTemplates(CArray<BlockReservation, BlockReservation>& aCurBlockRes, CArray<VisibleTemplate*,VisibleTemplate*> &arypTemplates, short nOffsetDay, COleDateTime dtWorking, BOOL bEnsureMatchingResource)
{
	//
	// (c.haag 2006-12-04 11:58) - PLID 23666 - This function takes an array of block reservations, and populates
	// a new array of block reservations such that the new array contains time gaps whenever higher priority
	// non-block templates occur.
	// 
	//
	// Before calling this function, aCurBlockRes has two elements:
	//		Block 1: 10:00 - 12:00
	//		Block 2: 12:00 - 3:00
	//
	// After calling this function, aCurBlockRes has three elements because there were some higher priority
	// non-block templates between 10:00 and 3:00:
	//		Block 1: 10:00 - 10:30
	//		Block 1: 11:00 - 11:45
	//		Block 2: 12:30 - 2:00
	//
	// The strategy is:
	//		1. Do for all blocks
	//			2. Do for all templates
	//				3. If the template is not a block and the template is a higher priority
	//					Add the template times to the aStartTimes and aEndTimes arrays
	//				end if
	//			end loop
	//			
	//			4. Add time ranges to aNewBlockRes that occur inside the block, and outside
	//			the aStartTimes and aEndTimes arrays
	//		end loop
	//
	//		5. Replace aCurBlockRes with aNewBlockRes
	//
	const long nCurBlocks = aCurBlockRes.GetSize();
	const int nVisibleTemplates = arypTemplates.GetSize();
	CArray<BlockReservation, BlockReservation> aNewBlockRes;
	int i;

	// 1. Do for all blocks
	for (int iCurBlock=0; iCurBlock < nCurBlocks; iCurBlock++) {
		const COleDateTime dtTemplateItemStart = aCurBlockRes[iCurBlock].dtStart;
		const COleDateTime dtTemplateItemEnd = aCurBlockRes[iCurBlock].dtEnd;
		CArray<COleDateTime, COleDateTime> aStartTimes;
		CArray<COleDateTime, COleDateTime> aEndTimes;
		COleDateTime dtStart, dtEnd;
		int nTimes = 0;

		// 2. Do for all templates
		for (i=0; i < nVisibleTemplates; i++) {
			VisibleTemplate* pvisTemplate = arypTemplates[i];

			// 3. Skip this template if it's a block, if it's a lower or equal priority, or if it's not even on the working column
			if (pvisTemplate->nSingleDayColumn != nOffsetDay || pvisTemplate->bIsBlock || pvisTemplate->nPriority <= aCurBlockRes[iCurBlock].pTemplate->nPriority) {
				continue;
			}

			// (z.manning 2008-07-25 17:39) - PLID 30804 - If we are supposed to check for the same
			// resource ID, do that now.
			if(bEnsureMatchingResource && pvisTemplate->nResourceID != aCurBlockRes.GetAt(iCurBlock).pTemplate->nResourceID) {
				continue;
			}

			//TES 9/3/2010 - PLID 39630 - This single item may now have multiple ranges, so go through each of them.
			CArray<TimeRange,TimeRange&> arSegments;
			pvisTemplate->ctrRange.GetRanges(arSegments);
			for(int nSegment = 0; nSegment < arSegments.GetSize(); nSegment++) {
				TimeRange tr = arSegments[nSegment];
				dtStart.SetDateTime(
					dtWorking.GetYear(), dtWorking.GetMonth(), dtWorking.GetDay(),
					tr.dtStart.GetHour(), tr.dtStart.GetMinute(), tr.dtStart.GetSecond()
					);
				dtEnd.SetDateTime(
					dtWorking.GetYear(), dtWorking.GetMonth(), dtWorking.GetDay(),
					tr.dtEnd.GetHour(), tr.dtEnd.GetMinute(), tr.dtEnd.GetSecond()
					);

				if (// (c.haag 2006-12-04 16:04) - Zack wrote CompareTimes. I commented the traditional compare
					// so that it's easier to compare and make sure we're doing the right thing
					//!(dtStart >= dtTemplateItemEnd || dtEnd <= dtTemplateItemStart))
					!( ((CT_GREATER_THAN | CT_EQUAL) & CompareTimes(dtStart, dtTemplateItemEnd)) || ((CT_LESS_THAN | CT_EQUAL) & CompareTimes(dtEnd, dtTemplateItemStart))) )
				{
					aStartTimes.Add(dtStart);
					aEndTimes.Add(dtEnd);
					nTimes++;
				}
			}
		}


		// 4. Do for all times that span the template item
		dtStart = dtTemplateItemStart;
		while (dtStart < dtTemplateItemEnd) {

			// We must find the first time of the current block that does not fall on any existing 
			// appointments. Once we have it, we must find the next open time to end the appointment
			// at
			BOOL bFoundOverlap = FALSE;
			for (i=0; i < nTimes && !bFoundOverlap; i++) {
				// (c.haag 2006-12-04 16:04) - Zack wrote CompareTimes. I commented the traditional compare
				// so that it's easier to compare and make sure we're doing the right thing
				//if (dtStart >= aStartTimes[i] && dtStart < aEndTimes[i]) {
				if ( ((CT_GREATER_THAN | CT_EQUAL) & CompareTimes(dtStart, aStartTimes[i])) && ((CT_LESS_THAN) == CompareTimes(dtStart, aEndTimes[i])) ) {
					dtStart = aEndTimes[i];
					bFoundOverlap = TRUE;
				}
			}

			// Now we have our start time, we need to find the end time. The end time
			// is the earliest start time of any appointment after dtStart. If there
			// are no appointments, then it is the end time of the template line item
			dtEnd = dtTemplateItemEnd;
			for (i=0; i < nTimes; i++) {
				// (c.haag 2006-12-04 16:04) - Zack wrote CompareTimes. I commented the traditional compare
				// so that it's easier to compare and make sure we're doing the right thing
				//if (aStartTimes[i] >= dtStart && aStartTimes[i] < dtEnd) {
				if ( ((CT_GREATER_THAN | CT_EQUAL) & CompareTimes(aStartTimes[i], dtStart)) && ((CT_LESS_THAN) == CompareTimes(aStartTimes[i], dtEnd)) ) {
					dtEnd = aStartTimes[i];
				}
			}

			// At last, we have our times! Now make the block.
			if (dtStart < dtEnd) {
				BlockReservation br;
				br.pTemplate = aCurBlockRes[iCurBlock].pTemplate;
				ASSERT(dtStart.GetDay() == dtEnd.GetDay());
				ASSERT(dtStart.GetMonth() == dtEnd.GetMonth());
				ASSERT(dtStart.GetYear() == dtEnd.GetYear());
				br.dtStart = dtStart;
				br.dtEnd = dtEnd;
				aNewBlockRes.Add(br);
			}

			// Now go to the next interval
			dtStart = dtEnd;
		}
	}

	// 5. Now that aNewBlockRes has our corrected list of block times, replace the contents of
	// aCurBlockRes with it.
	aCurBlockRes.RemoveAll();
	for (i=0; i < aNewBlockRes.GetSize(); i++) {
		aCurBlockRes.Add( aNewBlockRes[i] );
	}
}

// (z.manning 2008-07-23 15:00) - PLID 30804 - Move here from CNxSchedulerDlg
int GetBestFittingAptTypeCountForAnyRule(const VisibleTemplate* pTemplate,
														  long nTargetAptTypeID)
{
	//
	// (c.haag 2007-02-28 15:57) - PLID 24191 - This function will return the smallest
	// number of appointment types contained in a rule which states "If the appointment
	// is not one of the following types..." which also includes nTargetAptTypeID. The
	// very best fitting scenario possible is where we return 1, meaning that the rule
	// only looks for whatever nTargetAptTypeID is
	//
	int nBestItemCount = -1;
	int i,j;

	// Go through all the rules on the template and find the one that includes
	// the appointment type and has the least number of other types
	for (i=0; i < pTemplate->aRules.GetSize(); i++) {
		VisibleTemplateRule* pRule = pTemplate->aRules[i];
		CDWordArray anAptTypes;
		BOOL bRuleContainsTargetType = FALSE;

		for (j=0; j < pRule->anObjectID.GetSize(); j++) {
			switch (pRule->anObjectType[j]) {
			case 101: // If the appointment type is not one of...
				anAptTypes.Add(pRule->anObjectID[j]);
				if ((long)pRule->anObjectID[j] == nTargetAptTypeID) {
					bRuleContainsTargetType = TRUE;
				}
				break;
			}
		}
		// If this rule has nothing to do with this appointment type, then ignore it
		if (anAptTypes.GetSize() == 0 || !bRuleContainsTargetType) {
			continue;
		} else if (-1 == nBestItemCount || anAptTypes.GetSize() < nBestItemCount) {
			// If we get here, then we've found a superior rule
			nBestItemCount = anAptTypes.GetSize();
		}
	} // for (i=0; i < pTemplate->aRules.GetSize(); i++) {

	return nBestItemCount;
}

// (z.manning 2008-07-23 15:00) - PLID 30804 - Move here from CNxSchedulerDlg
void GetBestFittingAptPurposeCountForAnyRule(const VisibleTemplate* pTemplate,
														  const CDWordArray& anApptPurposes,
														  OUT int& nMatchingPurposes,
														  OUT int& nNonMatchingPurposes)
{
	//
	// (c.haag 2007-02-28 16:15) - PLID 24191 - This function will find which 
	// "If the appointment is not one of the following purposes..." rule contains the
	// most number of purposes included in anApptPurposes; and then report the number
	// of matching and non-matching purposes for that rule to the caller
	//
	int i,j,k;	

	// Reset the counters
	nMatchingPurposes = -1;
	nNonMatchingPurposes = -1;

	// Go through all the rules on the template and find the one that includes
	// the most purposes in anApptPurposes
	for (i=0; i < pTemplate->aRules.GetSize(); i++) {
		VisibleTemplateRule* pRule = pTemplate->aRules[i];
		CDWordArray anTotalAptPurposes, anCommonAptPurposes;

		for (j=0; j < pRule->anObjectID.GetSize(); j++) {
			switch (pRule->anObjectType[j]) {
			case 102: // If the appointment purpose is not one of...
				anTotalAptPurposes.Add(pRule->anObjectID[j]);
				// If we get here, we have found an object type of interest. Lets
				// see if the corresponding purpose is in the appointment purpose
				// list, and if it is, add it to anCommonAptPurposes
				for (k=0; k < anApptPurposes.GetSize(); k++) {
					if (anApptPurposes[k] == pRule->anObjectID[j]) {
						anCommonAptPurposes.Add(pRule->anObjectID[j]);
						break;
					}
				}
				break;
			}
		}
		// If this rule has nothing to do with this appointment type, then ignore it
		if (anTotalAptPurposes.GetSize() == 0 || anCommonAptPurposes.GetSize() == 0) {
			continue;
		} else if (-1 == nMatchingPurposes || nMatchingPurposes < anCommonAptPurposes.GetSize()) {
			// If we get here, then we've found a superior rule
			nMatchingPurposes = anCommonAptPurposes.GetSize();
			nNonMatchingPurposes = anTotalAptPurposes.GetSize() - nMatchingPurposes;
		}
	} // for (i=0; i < pTemplate->aRules.GetSize(); i++) {
}

/*  TemplateRuleDetailsT.ObjectType:
-- 	0 == ignore type list
--	1 == an appointment type IS in the list
--	2 == an appointment purpose IS in the list
--	3 == may be booked against n or more appointments
--	101 == an appointment type IS NOT in the list
--	102 == an appointment purpose IS NOT in the list
*/

// (z.manning 2008-07-23 15:00) - PLID 30804 - Move here from CNxSchedulerDlg
void CheckRule_OfAnyKind(const VisibleTemplateRule* pRule, BOOL& bSelected)
{
	// (c.haag 2006-12-14 09:42) - PLID 23845 - Set bSelected equal to whether a rule applies to all appts
	bSelected = pRule->bAllAppts;
}

// (z.manning 2008-07-23 15:00) - PLID 30804 - Move here from CNxSchedulerDlg
void CheckRule_Type(const VisibleAppointment* pAppt, const VisibleTemplateRule* pRule, BOOL& bSelected, BOOL& bSatisfied)
{
	// (c.haag 2006-12-14 09:42) - PLID 23845 - Builds a list of appointment types to include and exclude, and compares the
	// list with the appointment type. If a list exists, then bSelected is true. If the type satisfies the rule, then bSatisfied is true.
	CDWordArray anIsOneOfTheFollowing;
	CDWordArray anIsNotOneOfTheFollowing;
	int i;

	for (i=0; i < pRule->anObjectID.GetSize(); i++) {
		switch (pRule->anObjectType[i]) {
		case 1:
			anIsOneOfTheFollowing.Add(pRule->anObjectID[i]);
			break;
		case 101:
			anIsNotOneOfTheFollowing.Add(pRule->anObjectID[i]);
			break;
		}
	}
	if (anIsOneOfTheFollowing.GetSize() > 0) {
		// If we get here, determine if the appointment type satisfies the "is" rules
		ASSERT(0 == anIsNotOneOfTheFollowing.GetSize());
		bSelected = TRUE;
		bSatisfied = FALSE;
		for (i=0; i < anIsOneOfTheFollowing.GetSize() && !bSatisfied; i++) {
			if (pAppt->nAptTypeID == (long)anIsOneOfTheFollowing[i]) {
				bSatisfied = TRUE;
			}
		}
	}
	else if (anIsNotOneOfTheFollowing.GetSize() > 0) {
		// If we get here, determine if the appointment type satisfies the "is not" rules
		ASSERT(0 == anIsOneOfTheFollowing.GetSize());
		bSelected = TRUE;
		bSatisfied = TRUE;
		for (i=0; i < anIsNotOneOfTheFollowing.GetSize() && bSatisfied; i++) {
			if (pAppt->nAptTypeID == (long)anIsNotOneOfTheFollowing[i]) {
				bSatisfied = FALSE;
			}
		}
	}
	else {
		// If we get here, there are no type rules
		bSelected = FALSE;
	}
}

// (z.manning 2008-07-23 15:00) - PLID 30804 - Move here from CNxSchedulerDlg
void CheckRule_Purpose(const VisibleAppointment* pAppt, const VisibleTemplateRule* pRule, BOOL& bSelected, BOOL& bSatisfied)
{
	// (c.haag 2006-12-14 09:42) - PLID 23845 - Builds a list of appointment purposes to include and exclude, and compares the
	// list with the appointment purposes. If a list exists, then bSelected is true. If the purposes satisfy the rule, then bSatisfied is true.
	CDWordArray anIsOneOfTheFollowing;
	CDWordArray anIsNotOneOfTheFollowing;
	int i,j;

	for (i=0; i < pRule->anObjectID.GetSize(); i++) {
		switch (pRule->anObjectType[i]) {
		case 2:
			anIsOneOfTheFollowing.Add(pRule->anObjectID[i]);
			break;
		case 102:
			anIsNotOneOfTheFollowing.Add(pRule->anObjectID[i]);
			break;
		}
	}
	if (anIsOneOfTheFollowing.GetSize() > 0) {
		// If we get here, determine if the appointment type satisfies the "is" rules
		ASSERT(0 == anIsNotOneOfTheFollowing.GetSize());
		bSelected = TRUE;
		bSatisfied = FALSE;
		for (i=0; i < anIsOneOfTheFollowing.GetSize() && !bSatisfied; i++) {
			for (j=0; j < pAppt->anPurposeIDs.GetSize() && !bSatisfied; j++) {
				if (pAppt->anPurposeIDs[j] == anIsOneOfTheFollowing[i]) {
					bSatisfied = TRUE;
				}
			}
		}
	}
	else if (anIsNotOneOfTheFollowing.GetSize() > 0) {
		// If we get here, determine if the appointment type satisfies the "is not" rules
		ASSERT(0 == anIsOneOfTheFollowing.GetSize());
		bSelected = TRUE;
		bSatisfied = TRUE;
		for (i=0; i < anIsNotOneOfTheFollowing.GetSize() && bSatisfied; i++) {
			for (j=0; j < pAppt->anPurposeIDs.GetSize() && bSatisfied; j++) {
				if (pAppt->anPurposeIDs[j] == anIsNotOneOfTheFollowing[i]) {
					bSatisfied = FALSE;
				}
			}
		}
	}
	else {
		// If we get here, there are no purpose rules
		bSelected = FALSE;
	}
}

// (z.manning 2008-07-23 15:00) - PLID 30804 - Move here from CNxSchedulerDlg
BOOL AppointmentSatisfiesRule(const VisibleAppointment* pAppt, const VisibleTemplateRule* pRule)
{
	//
	// (c.haag 2006-12-13 17:51) - PLID 23845 - Given an appointment and a template, this returns TRUE if the
	// appointment satisfies the rule. If nnn_Selected is FALSE, then nnn_Satisifed is undefined.
	//
	// The three booleans correspond to the first three checkboxes in the Edit Rule Details window. 
	//
	BOOL bOfAnyKind_Selected,
		bType_Selected, bType_Satisfied,
		bPurpose_Selected, bPurpose_Satisfied;

	// Check the rules out
	CheckRule_OfAnyKind(pRule, bOfAnyKind_Selected);
	CheckRule_Type(pAppt, pRule, bType_Selected, bType_Satisfied);
	CheckRule_Purpose(pAppt, pRule, bPurpose_Selected, bPurpose_Satisfied);

	//TES 7/23/2012 - PLID 26700 - If this rule doesn't have either of the first three checkboxes checked, it must be just a rule to prevent
	// double-booking.  This function is only used to calculate when appointments should cover precision templates, and that decision should
	// not be affected by any double-booking considerations.  So, in that case, we'll return FALSE, which will then tell our caller that
	// the appointment can cover this precision template (at least, as far as this rule is concerned). 
	if(!bOfAnyKind_Selected && !bType_Selected && !bPurpose_Selected) {
		return FALSE;
	}

	//
	// Given all of our options, make the final decision.
	//
	if (bOfAnyKind_Selected) {
		return TRUE; // Any appointment of any kind cannot fail
	}
	else if (pRule->bAndDetails) {
		// If we get here, the appt must meet ALL requirements
		if (bType_Selected && !bType_Satisfied) return FALSE;
		if (bPurpose_Selected && !bPurpose_Satisfied) return FALSE;
		return TRUE;
	}
	else {
		// If we get here, the appt must meet ANY requirements
		if (bType_Selected && bType_Satisfied) return TRUE;
		if (bPurpose_Selected && bPurpose_Satisfied) return TRUE;
		return FALSE;
	}
}

// (z.manning 2008-07-23 15:00) - PLID 30804 - Move here from CNxSchedulerDlg
// (z.manning 2011-06-14 16:44) - PLID 41131 - Added bIgnoreWarnings
BOOL AppointmentCanBeMadeOverBlockTemplate(const VisibleAppointment* pAppt, const BlockReservation& br, const COleDateTime& dtWorkingDate, BOOL bIgnoreWarnings /* = FALSE */)
{
	//
	// (c.haag 2006-12-13 17:51) - PLID 23845 - Given an appointment and a template, this returns TRUE if the
	// appointment can be created over the template.
	//
	const VisibleTemplate* pTemplate = br.pTemplate;

	// Ensure that the appointment at least partially overlaps the block
	const COleDateTime dtTemplateItemStart = br.dtStart;
	const COleDateTime dtTemplateItemEnd = br.dtEnd;
	COleDateTime dtStart = pAppt->dtStart;
	COleDateTime dtEnd = pAppt->dtEnd;
	dtStart.SetDateTime( dtWorkingDate.GetYear(), dtWorkingDate.GetMonth(), dtWorkingDate.GetDay(),
		dtStart.GetHour(), dtStart.GetMinute(), dtStart.GetSecond() );
	dtEnd.SetDateTime( dtWorkingDate.GetYear(), dtWorkingDate.GetMonth(), dtWorkingDate.GetDay(),
		dtEnd.GetHour(), dtEnd.GetMinute(), dtEnd.GetSecond() );

	if (// (c.haag 2006-12-04 16:04) - Zack wrote CompareTimes. I commented the traditional compare
		// so that it's easier to compare and make sure we're doing the right thing
		//(dtStart >= dtTemplateItemEnd || dtEnd <= dtTemplateItemStart))
		( ((CT_GREATER_THAN | CT_EQUAL) & CompareTimes(dtStart, dtTemplateItemEnd)) || ((CT_LESS_THAN | CT_EQUAL) & CompareTimes(dtEnd, dtTemplateItemStart))) )
	{
		// If we get here, there is no overlap. It is therefore impossible to make the appointment
		// over the block
		return FALSE;
	}

	//TES 6/15/2011 - PLID 43973 - Make sure the resources match
	bool bMatched = false;
	for(int i = 0; i < pAppt->anResourceIDs.GetSize(); i++) {
		// (d.singleton 2012-01-10 16:19) - PLID 46522 - needed to also check to see if the template is set to AllResources
		if(pAppt->anResourceIDs[i] == pTemplate->nResourceID || pTemplate->bAllResources) bMatched = true;
	}
	if(!bMatched) {
		return FALSE;
	}

	// If we get here, there is at least a partial overlap. Now check the template rules
	for (int i=0; i < pTemplate->aRules.GetSize(); i++)
	{
		VisibleTemplateRule *pRule = pTemplate->aRules[i];
		BOOL bRuleSatisfied = FALSE;
		// (z.manning 2011-06-14 16:47) - PLID 41131 - We may want to ignore a rule if it's only a warning.
		if(!bIgnoreWarnings || pRule->bPreventOnFail) {
			bRuleSatisfied = AppointmentSatisfiesRule(pAppt, pRule);
		}

		// If the appointment meets all the rule criteria, that means it is susceptible to warnings and
		// prevention of being saved. Therefore, we return false.
		if (bRuleSatisfied) {
			return FALSE;
		}
	}
	return TRUE;
}

// (c.haag 2009-01-19 10:27) - PLID 25578 - Given an array of procedures, this function will
// look for all the detail procedures, and ensure their masters don't exist in the array.
//
// Here are some example use cases:
//
//	========================================================================================================
//	Input									Output
//	========================================================================================================
//	Abdominoplasty							Abdominoplasty
//	Abdominoplasty, Abd. Mini				Abd. Mini
//	Abdominoplasty, Abd. Mini, Abd. Ext.	Abd. Mini, Abd. Ext
//	Abd., Abd. Mini, Blepharoplasty			Abd. Mini, Blepharoplasty
//	Abd., Abd. Mini, Bleph, Bleph. Upper	Abd. Mini, Bleph. Upper
//
void PruneMasterIDsOfDetailProcedures(CArray<long,long>& anProcedures,
									  const CMap<long,long,long,long>& mapProcedureMasterIDs)
{
	CArray<long,long> anTmp;
	anTmp.Copy(anProcedures);
	anProcedures.RemoveAll();
	int i,j;

	// Do for all procedures
	for (i=0; i < anTmp.GetSize(); i++) {
		const long nProcedureID = anTmp[i];
		long nMasterID = -1;

		// Get the procedure's master ID, if any
		mapProcedureMasterIDs.Lookup(nProcedureID, nMasterID);

		if (nMasterID > -1) {
			// This is a detail procedure. Always bring it over.
			anProcedures.Add(nProcedureID);
		} else {
			// This is a master procedure. Only add it to anprocedures if it's 
			// not the master of any details in anTmp.
			BOOL bFoundDetail = FALSE;
			for (j=0; j < anTmp.GetSize() && !bFoundDetail; j++) {
				long nProcID = anTmp[j];
				long nProcMasterID = -1;
				mapProcedureMasterIDs.Lookup(nProcID, nProcMasterID);
				if (nProcMasterID != -1 && nProcedureID == nProcMasterID) {
					bFoundDetail = TRUE;
				}
			}
			if (!bFoundDetail) {
				// If we get here, no detail is selected for this master procedure.
				// So, add the master procedure to anProcedures.
				anProcedures.Add(nProcedureID);
			} else {
				// If we get here, nProcedureID is the master of at least one detail procedure
				// in anTmp. So, don't add it.
			}
		}
	} // for (i=0; i < anTmp.GetSize(); i++) {
}

// (c.haag 2009-01-19 09:54) - PLID 25578 - Given an array of procedures from a source, and array of procedures
// for an existing consult, this function calculate what procedures should be assigned to that existing consult.
//
// In summary: We want the consult to match the source as closely as possible; but if detail procedure content is
// inconsistent for the same master procedure, then assign the master procedure to the output instead of any details.
//
// The calculation is a multi-phase process:
//
//	1. Copy all master procedures from anSourceProcedures to anResults. (The contents of anConsultProcedures do not matter).
//	2. For all groups of detail procedures in anSourceProcedures with the same master:
//		A. If anConsultProcedures contains those and ONLY those details with the same master, add the detail procedures to anResults
//		B. Otherwise, add the master of those detail procedures to anResults instead
//
// Some examples below:
//
//	========================================================================================================
//	Source									Consult								Result
//	========================================================================================================
//	Abdominoplasty							Breast Aug.							Abdominoplasty	*(replace)
//	Abdominoplasty, Abd. Mini				Breast Aug.							Abdominoplasty	*(replace w/ master only; appt didn't have Mini)
//	Abdominoplasty, Abd. Mini				Abd. Mini							Abd. Mini		*(do nothing; appt already had mini)
//	Abdominoplasty, Abd. Mini				Abd. Ext							Abdominoplasty	*(replace w/ master only; appt didn't have Mini)
//	Abdominoplasty, Abd. Mini, Abd. Ext.	Breast Aug.							Abdominoplasty	*(replace w/ master only; appt didn't have Mini or Ext)
//	Abd., Abd. Mini, Blepharoplasty			Abd. Mini, Blepharoplasty			Abd. Mini, Blepharoplasty	*(do nothing; appt already had mini and bleph)
//	Abd., Abd. Ext., Bleph. Bleph. Upper	Abd. Ext., Bleph. Upper				Abd. Ext., Bleph. Upper *(do nothing; appt already had Abd. Ext. and Bleph. Upper)
//	Abd., Abd. Ext., Bleph. Bleph. Upper	Abd. Mini, Bleph. Upper				Abdominoplasty, Bleph. Upper *(replace w/ master only; appt did not have Abd. Ext)
//	Abd., Abd. Ext., Bleph. Bleph. Upper	Abd. Mini, Abd. Ext. Bleph. Upper	Abdominoplasty, Bleph. Upper *(replace w/ master only; PIC doesn't have Abd. Mini)
//
//
// This function assumes that anSourceProcedures does not contain any ID's that correspond to
// the master procedure of any detail procedures contained therein.
//
//
void CalculateIdealConsultProcedures(const CArray<long,long>& anSourceProcedures,
									 const CArray<long,long>& anConsultProcedures,
									 const CMap<long,long,long,long>& mapProcedureMasterIDs,
									 CArray<long,long>& anResults)
{
	CMap<long,long,long,long> mapRequiredMasterIDs;
	ASSERT(0 == anResults.GetSize()); // Should always be empty here
	int i,j;

	// First, add the source's master procedures to mapRequiredMasterIDs. This is the easy part.
	for (i=0; i < anSourceProcedures.GetSize(); i++) {
		const long nProcedureID = anSourceProcedures[i];
		long nMasterID = -1;
		mapProcedureMasterIDs.Lookup(nProcedureID, nMasterID);
		if (-1 == nMasterID) {
			// This is a master procedure. Put it in our mapRequiredMasterIDs map.
			mapRequiredMasterIDs.SetAt(nProcedureID, nProcedureID);
		} else {
			// This is a detail procedure. Ignore it.
		}
	}
	// Now find all the source detail procedures that don't exist in the consult, and
	// remember their master procedure ID's
	for (i=0; i < anSourceProcedures.GetSize(); i++) {
		const long nProcedureID = anSourceProcedures[i];
		long nMasterID = -1;
		mapProcedureMasterIDs.Lookup(nProcedureID, nMasterID);
		if (nMasterID > -1) {
			// This is a source detail procedure. Look for it in the consult purpose list.
			BOOL bFound = FALSE;
			for (j=0; j < anConsultProcedures.GetSize() && !bFound; j++) {
				if (anConsultProcedures[j] == nProcedureID) {									
					bFound = TRUE; // We found it
				} else {
					// Keep looking
				}
			}
			if (!bFound) {
				// If we get here, the detail procedure exists in the source but not the consult.
				// So, we need to flag the master procedure for being written to the output. 
				mapRequiredMasterIDs.SetAt(nMasterID,nMasterID);
			}
		} else {
			// This is a source master procedure; just ignore it.
		}
	}
	// Now find all the detail procedures in the consult that don't exist in the source, and
	// remember their master procedure ID's
	for (i=0; i < anConsultProcedures.GetSize(); i++) {
		const long nProcedureID = anConsultProcedures[i];
		long nMasterID = -1;
		mapProcedureMasterIDs.Lookup(nProcedureID, nMasterID);
		if (nMasterID > -1) {
			// This is a consult detail procedure. Find it in the source procedure list.
			BOOL bFound = FALSE;
			for (j=0; j < anSourceProcedures.GetSize() && !bFound; j++) {
				if (anSourceProcedures[j] == nProcedureID) {
					bFound = TRUE; // We found it
				} else {
					// Keep looking
				}
			}
			if (!bFound) {
				// If we get here, the detail procedure exists in the consult, but not the source.
				// So, we need to flag the master procedure for being written to the output. 
				mapRequiredMasterIDs.SetAt(nMasterID,nMasterID);
			}
		} else {
			// This is a consult master procedure; just ignore it.
		}
	}

	//
	// Now it's time to generate the output
	//

	// 1. Add the source master procedures, and any master procedures that have details which
	// don't match up between the source and consult, to anResults
	POSITION pos = mapRequiredMasterIDs.GetStartPosition();
	while (pos) {
		long nDummy, nMasterID;
		mapRequiredMasterIDs.GetNextAssoc(pos, nDummy, nMasterID);
		anResults.Add(nMasterID);
	}
	// 2. Add the source detail procedures whose masters dont exist in mapRequiredMasterIDs,
	// but perfectly match the detail procedures with the same master in the consult, to anResults.
	for (i=0; i < anSourceProcedures.GetSize(); i++) {
		const long nProcedureID = anSourceProcedures[i];
		long nMasterID = -1;
		mapProcedureMasterIDs.Lookup(nProcedureID, nMasterID);
		if (nMasterID > -1) {
			// nProcedureID is a detail procedure. If it's not in mapRequiredMasterIDs, add it.
			long nDummy;
			if (!mapRequiredMasterIDs.Lookup(nMasterID, nDummy)) {
				anResults.Add(nProcedureID);
			}
		} else {
			// nProcedureID is a master procedure. Ignore it.
		}
	}
}

// (c.haag 2009-01-19 10:41) - PLID 25578 - This function is a utility function for SyncSurgeryPurposesWithConsult
// and UpdateLinkedPICPurposes. Procedure-related data is read in from a recordset and stored in an array and various maps.
void FillProcedureData(_RecordsetPtr& prs,
						CArray<long,long>& anProcedureIDs,
						CMap<long,long,CString,LPCTSTR>& mapProcedureIDToName,
						CMap<long,long,long,long>& mapProcedureIDToMasterID
						)
{
	FieldsPtr f = prs->Fields;
	while (!prs->eof) {
		long nID = AdoFldLong(f, "ID");
		long nMasterID = AdoFldLong(f, "MasterProcedureID", -1);

		// Write to the name and master ID maps
		mapProcedureIDToName.SetAt(nID, AdoFldString(f, "Name"));
		if (nMasterID > -1) {
			mapProcedureIDToName.SetAt(nMasterID, AdoFldString(f, "MasterName"));
			// (c.haag 2009-01-28 11:52) - PLID 25578 - Only update mapProcedureIDToMasterID
			// if a master ID is present
			mapProcedureIDToMasterID.SetAt(nID, nMasterID);
		}

		// Ensure the ID is in anProcedureIDs
		if (!ExistsInArray(nID, anProcedureIDs)) {
			anProcedureIDs.Add(nID);
		}
		prs->MoveNext();
	} // while (!prs->eof) {
}

// (c.haag 2009-01-14 15:31) - PLID 25578 - Given a ProcInfoID, this function will invoke a dialog to update all
// the purposes of appointments linked with the record to match the purposes of those on the procedure info record
//
// In this implementation, we'll just follow a preference. In the case of a prompt, just use a simple Yes / No message box.
// We won't tweak it further unless it's necessary.
//
// This is a utility function; no exception handling will be implemented.
//
// (a.wilson 2014-08-12 14:31) - PLID 63170 - replace with table checker ex.
void UpdateLinkedPICPurposes(long nProcInfoID)
{
	if (nProcInfoID <= 0) {
		// Do nothing; maybe it's a new procedure info record and this was inadvertenly called
		ASSERT(FALSE);
		return;
	}
	int nAutoUpdatePICApptsWhenPICChanges = GetRemotePropertyInt("AutoUpdatePICApptsWhenPICChanges", 1, 0, "<None>", false);
	if (0 == nAutoUpdatePICApptsWhenPICChanges) {
		// Do nothing
		return;
	}


	// Also get a list of all non-cancelled, non-no-show procedural appointments linked with the PIC. These
	// will be changed.
	//
	int i;
	_RecordsetPtr prs = CreateParamRecordset(
		// (d.thompson 2009-08-12) - PLID 35187 - Fixed a bad join from AppointmentPurposeT.ID = ProcedureT.ID in the second query
		FormatString(
			"DECLARE @ProcInfoID INT "
			"SET @ProcInfoID = {INT} "
			"/* Get a list of all the master and selected child procedures for the PIC */\r\n"
			"SELECT ProcedureT.ID, ProcedureT.MasterProcedureID, ProcedureT.Name, Master.Name AS MasterName "
			"FROM ProcedureT "
			"INNER JOIN ProcInfoDetailsT ON ProcInfoDetailsT.ProcedureID = ProcedureT.ID "
			"LEFT JOIN ProcedureT Master ON ProcedureT.MasterProcedureID = Master.ID "
			"WHERE (ProcedureT.MasterProcedureID IS NULL OR (ProcedureT.MasterProcedureID IS NOT NULL AND ProcInfoDetailsT.Chosen = 1)) "
			"AND ProcInfoDetailsT.ProcInfoID = @ProcInfoID "
			"ORDER BY ProcedureT.Name;\r\n "
			"\r\n"

			"/* Get a list of all procedures and procedure master ID's and names for all the non-no-show procedural appointments linked with the PIC. */\r\n"
			"SELECT ProcedureT.ID, ProcedureT.MasterProcedureID, ProcedureT.Name, Master.Name AS MasterName "
			"FROM StepsT "
			"LEFT JOIN (EventAppliesT INNER JOIN EventsT ON EventAppliesT.EventID = EventsT.ID) ON StepsT.ID = EventAppliesT.StepID "
			"INNER JOIN AppointmentsT ON AppointmentsT.ID = EventsT.ItemID "
			"INNER JOIN AppointmentPurposeT ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID "
			"INNER JOIN ProcedureT ON AppointmentPurposeT.PurposeID = ProcedureT.ID "
			"LEFT JOIN AptTypeT ON AptTypeT.ID = AptTypeID "
			"LEFT JOIN ProcedureT Master ON ProcedureT.MasterProcedureID = Master.ID "
			"WHERE EventsT.Type IN (%d,%d) AND StepsT.LadderID IN (SELECT ID FROM LaddersT WHERE ProcInfoID = @ProcInfoID) "
			"AND AppointmentsT.Status <> 4 AND AppointmentsT.ShowState <> %d AND Category IN (%d,%d,%d,%d,%d) "
			"GROUP BY ProcedureT.ID, ProcedureT.MasterProcedureID, ProcedureT.Name, Master.Name;\r\n "
			"\r\n"

			"/* Get the patient ID */\r\n"
			"SELECT PatientID FROM ProcInfoT WHERE ID = @ProcInfoID;\r\n"
			"\r\n"

			"/* Get a detailed list of all non-cancelled, non-no-show procedural appointments linked with the PIC. */\r\n"
			"SELECT * FROM ("
			"SELECT AppointmentsT.ID AS AppointmentID, AppointmentsT.PatientID, AppointmentsT.StartTime, AppointmentsT.EndTime, AppointmentsT.Status, "
			"AppointmentsT.ShowState, AppointmentsT.LocationID, AptTypeT.Name, dbo.GetPurposeString(AppointmentsT.ID) AS Purposes, "
			"dbo.GetPurposeIDString(AppointmentsT.ID) AS PurposeIDs, AppointmentsT.Notes, AptTypeT.Category, dbo.GetResourceIDString(AppointmentsT.ID) AS ResourceIDs "
			"FROM StepsT "
			"LEFT JOIN (EventAppliesT INNER JOIN EventsT ON EventAppliesT.EventID = EventsT.ID) ON StepsT.ID = EventAppliesT.StepID "
			"INNER JOIN AppointmentsT ON AppointmentsT.ID = EventsT.ItemID "
			"LEFT JOIN AptTypeT ON AptTypeT.ID = AptTypeID "
			"WHERE EventsT.Type IN (%d,%d) AND StepsT.LadderID IN (SELECT ID FROM LaddersT WHERE ProcInfoID = @ProcInfoID) "
			"AND AppointmentsT.Status <> 4 AND AppointmentsT.ShowState <> %d AND Category IN (%d,%d,%d,%d,%d) "
			"UNION "
			"SELECT AppointmentsT.ID AS AppointmentID, AppointmentsT.PatientID, AppointmentsT.StartTime, AppointmentsT.EndTime, AppointmentsT.Status, "
			"AppointmentsT.ShowState, AppointmentsT.LocationID, AptTypeT.Name, dbo.GetPurposeString(AppointmentsT.ID) AS Purposes, "
			"dbo.GetPurposeIDString(AppointmentsT.ID) AS PurposeIDs, AppointmentsT.Notes, AptTypeT.Category, dbo.GetResourceIDString(AppointmentsT.ID) AS ResourceIDs "
			"FROM AppointmentsT "
			"LEFT JOIN AptTypeT ON AptTypeT.ID = AptTypeID "
			"WHERE AppointmentsT.ID IN (SELECT SurgeryApptID FROM ProcInfoT WHERE SurgeryApptID IS NOT NULL AND ID = @ProcInfoID) "
			") SubQ GROUP BY AppointmentID, PatientID, LocationID, StartTime, EndTime, Status, ShowState, Name, Notes, Category, Purposes, PurposeIDs, ResourceIDs;\r\n "
			"\r\n"
			,PhaseTracking::ET_AppointmentCreated, PhaseTracking::ET_ActualAppointment, CAppointmentsDlg::astNoShow
			,PhaseTracking::AC_CONSULT, PhaseTracking::AC_PREOP, PhaseTracking::AC_MINOR, PhaseTracking::AC_SURGERY, PhaseTracking::AC_FOLLOW_UP
			,PhaseTracking::ET_AppointmentCreated, PhaseTracking::ET_ActualAppointment, CAppointmentsDlg::astNoShow
			,PhaseTracking::AC_CONSULT, PhaseTracking::AC_PREOP, PhaseTracking::AC_MINOR, PhaseTracking::AC_SURGERY, PhaseTracking::AC_FOLLOW_UP			
			)
		,nProcInfoID);

	if (prs->eof) {
		// Either there are no procedures, or, the ProcInfo record was deleted. Don't do anything; lest we lose
		// historic appointment data. Nobody would know why the patient ever came in the first place.
		return;
	}

	// Now populate the array of PIC procedure ID's, and map each procedure ID to a master ID and name.
	CArray<long,long> anPICProcedureIDs;
	CMap<long,long,CString,LPCTSTR> mapProcedureNames;
	CMap<long,long,long,long> mapProcedureMasterIDs;
	FillProcedureData(prs, anPICProcedureIDs, mapProcedureNames, mapProcedureMasterIDs);

	if (0 == anPICProcedureIDs.GetSize()) {
		return; // No PIC procedures? Nothing to do.
	}

	// Now fill mapProcedureMasterIDs with all relevant combinations of detail and master procedures
	// for all the appointments linked to this PIC.
	{
		CArray<long,long> anDummy;
		prs = prs->NextRecordset(NULL);
		FillProcedureData(prs, anDummy, mapProcedureNames, mapProcedureMasterIDs);
	}

	// Now get the patient ID
	long nPatientID;
	prs = prs->NextRecordset(NULL);
	if (prs->eof) {
		// This should never happen
		return;
	} else {
		nPatientID = AdoFldLong(prs, "PatientID");
	}

	// Now go through each appointment we mean to change, determine whether we need to change it, and
	// put the change in a SQL batch if necessary.
	prs = prs->NextRecordset(NULL);
	if (prs->eof) {
		// No appointments to update
		return;
	}
	long nAuditTransactionID = -1; // Auditing
	try {
		struct ChangedAppointment {
			long nAppointmentID;
			long nPatientID;
			long nLocationID;
			long nStatus;
			long nShowState;
			COleDateTime dtStart, dtEnd;
			CString strResourceIDs;
		};

		nAuditTransactionID = BeginAuditTransaction();
		// (d.thompson 2010-05-13) - PLID 38643 - Parameterized the batch
		CString strSqlBatch = BeginSqlBatch();
		CNxParamSqlArray args;
		CString strMsg = "Would you like to update the procedures of the following appointments:\r\n\r\n";
		CArray<ChangedAppointment> anChangedAppts;

		// Before we do anything, we need to build a list of PIC procedures that meet ONE OF the following criteria:
		//
		//	1. The procedure is a detail procedure that is marked "In Use" (Chosen)
		//	2. The procedure is a master procedure with no details that are marked as "In Use"
		//
		// The reason for this will be made clear in code beneath this block. anPICProcedureIDs presently
		// contains all the procedures (both master and detail) for the PIC. This code will copy that content
		// into anQualifyingProcedures, and then remove all master procedures from anQualifyingProcedures which
		// are masters of detail procedures in anQualifyingProcedures.
		//
		CArray<long,long> anQualifyingProcedures;
		anQualifyingProcedures.Copy(anPICProcedureIDs);
		PruneMasterIDsOfDetailProcedures(anQualifyingProcedures, mapProcedureMasterIDs);

		// Now we do the fun, complicated part. We update the appt. procedure data with the PIC content. Now, there's
		// two branches in this logic: One for consult appts, and one for non-consult appts. Non-consults are easy;
		// we simply copy the contents of anQualifyingProcedures to the appointment.
		//
		// Consults are more complicated. For every group of detail procedures with a common master, we have to see that 
		// the appointment has those and only those for the same master procedure. For example, say you have a PIC with
		// Abdominoplasty-Mini and Abdominoplasty-Extended. That's two detail procedures with the same master: Abdominoplasty.
		// If the appointment purposes are "Abd. Mini and Abd. Extended", then they match. If the appointment purposes are 
		// "Bleph, Abd. Mini and Abd. Extended", it's still a match because Bleph is not a detail of Abdominoplasty. If the
		// appt has "Abd. Mini", then it's not a match because it's missing Abd. Extended. If the appt has "Abd. Mini,
		// Abd. Ext. and Abd. Upper"...then it's not a match because Abd. Upper isn't part of the PIC.
		//
		FieldsPtr f = prs->Fields;
		int nApptsChanging = 0;
		// Do for all appointments
		while (!prs->eof) {
			// First, gather basic information
			const long nAppointmentID = AdoFldLong(f, "AppointmentID");
			const long nCategoryID = AdoFldByte(f, "Category");
			//(e.lally 2009-04-27) PLID 34084 - It is possible this appointment has no purpose assigned, so these fields would
			//	be null. Default to an empty string.
			CString strApptPurposes = AdoFldString(f, "Purposes","");
			CString strApptPurposeIDs = AdoFldString(f,"PurposeIDs","");
			CDWordArray adwApptPurposeIDs;
			CArray<long, long> anApptPurposeIDs;
			ParseDelimitedStringToDWordArray(strApptPurposeIDs, " ", adwApptPurposeIDs);
			for (i=0; i < adwApptPurposeIDs.GetSize(); i++) {
				anApptPurposeIDs.Add(adwApptPurposeIDs[i]);
			}

			// See if the appt. procedures match exactly what we plan to assign to this appointment already;
			// because if they do, then we don't need to do anything.
			if (!AreArrayContentsMatched(anApptPurposeIDs,anPICProcedureIDs)) {

				// This array contains a list of all procedures we will be assigning to this appointment
				CArray<long, long> anNewProcedureIDs;

				// The appointment purposes doesn't match the PIC procedures. Handle this case based on whether
				// the appointment is a consult or not.
				if (PhaseTracking::AC_CONSULT != nCategoryID) {

					// The appointment is not a consult. We need to assign the following procedures to the appointment:
					//	1. PIC detail procedures that are marked "In Use" (Chosen)
					//	2. PIC Master procedures with no details that are marked as "In Use"
					// Look familiar? They should be; it's what we built anQualifyingProcedures with.
					//
					// Here are some example use cases:
					//
					//	Non-Consults
					//	========================================================================================================
					//	PIC Procedures							Appointment output
					//	========================================================================================================
					//	Abdominoplasty							Abdominoplasty
					//	Abdominoplasty, Abd. Mini				Abd. Mini
					//	Abdominoplasty, Abd. Mini, Abd. Ext.	Abd. Mini, Abd. Ext
					//	Abd., Abd. Mini, Blepharoplasty			Abd. Mini, Blepharoplasty
					//	Abd., Abd. Mini, Bleph, Bleph. Upper	Abd. Mini, Bleph. Upper
					//
					anNewProcedureIDs.Copy(anQualifyingProcedures);
				}
				else {
					// The appointment is a consult. This is where it gets tricky, because now we actually do care what the existing procedures 
					// are already assigned to. All the work is done in CalculateIdealConsultProcedures, which has more detailed comments.
					CalculateIdealConsultProcedures(anQualifyingProcedures, anApptPurposeIDs, mapProcedureMasterIDs, anNewProcedureIDs);
				}

				// Now that anNewProcedureIDs is filled in, do one final test to see whether we should write to data
				if (!AreArrayContentsMatched(anNewProcedureIDs, anApptPurposeIDs)) {

					// Add to the batch
					// (d.thompson 2010-05-13) - PLID 38643 - Parameterized as possible.  In many offices the set of procedures is
					//	fairly small, so this will help at least somewhat.
					AddParamStatementToSqlBatch(strSqlBatch, args, FormatString("DELETE FROM AppointmentPurposeT WHERE AppointmentID = {INT};\r\n"
						"INSERT INTO AppointmentPurposeT (AppointmentID, PurposeID) SELECT AppointmentsT.ID, ProcedureT.ID "
						"FROM AppointmentsT, ProcedureT WHERE AppointmentsT.ID = {INT} AND ProcedureT.ID IN (%s) ",ArrayAsString(anNewProcedureIDs)),
						nAppointmentID, nAppointmentID);

					// Add to the audit transaction
					CString strOld = strApptPurposes;
					CString strNew;
					for (i=0; i < anNewProcedureIDs.GetSize(); i++) {
						CString strProcName;
						mapProcedureNames.Lookup(anNewProcedureIDs[i], strProcName);
						if (strNew.IsEmpty()) {
							strNew = strProcName;
						} else {
							strNew += ", " + strProcName;
						}
					}
					AuditEvent(nPatientID, GetExistingPatientName(nPatientID), nAuditTransactionID, aeiApptPurpose, nAppointmentID,
						strOld, strNew, aepMedium, aetChanged);

					// Now add to the user message box, if applicable
					if (1 == nAutoUpdatePICApptsWhenPICChanges) {
						nApptsChanging++;
						if (nApptsChanging < 10) {
							CString strNotes = AdoFldString(prs, "Notes", "");
							if (!strNotes.IsEmpty()) { strNotes = " - " + strNotes; }
							if (strApptPurposes.IsEmpty()) { strApptPurposes = "<No Purposes>"; }
							CString strDesc = FormatString("%s - %s - %s%s", FormatDateTimeForInterface(AdoFldDateTime(prs, "StartTime")),
								AdoFldString(prs, "Name", "<No Type>"), strApptPurposes, strNotes);
							strMsg += strDesc + "\r\n";
						} else if (nApptsChanging == 10) {
							strMsg += "<more omitted>\r\n";
						} else {
							// Message text has been truncated; too many appointments
						}
					}

					// Add to the changed appt. list
					ChangedAppointment ca;
					ca.nAppointmentID = nAppointmentID;
					ca.nPatientID = AdoFldLong(prs, "PatientID");
					ca.nLocationID = AdoFldLong(prs, "LocationID");
					ca.nStatus = (long)AdoFldByte(prs, "Status");
					ca.nShowState = AdoFldLong(prs, "ShowState");
					ca.dtStart = AdoFldDateTime(prs, "StartTime");
					ca.dtEnd = AdoFldDateTime(prs, "EndTime");
					ca.strResourceIDs = AdoFldString(prs, "ResourceIDs", "");
					anChangedAppts.Add(ca);

				} // if (!AreArrayContentsMatched(anNewProcedureIDs, anApptPurposeIDs)) {
				else 
				{
					// Nothing to do; the appointment already has these procedures
				}
			} // if (!AreArrayContentsMatched(anApptPurposeIDs,anPICProcedureIDs)) {
			else
			{
				// This appointment would be unchanged. Ignore it.
			}
			prs->MoveNext(); // Go to the next appointment
		} // while (!prs->eof) {

		// Quit if nothing to do
		if (0 == anChangedAppts.GetSize()) {
			if (-1 != nAuditTransactionID) {
				RollbackAuditTransaction(nAuditTransactionID);
			}
			return; // User declined
		}

		// Now prompt the user if necessary
		if (1 == nAutoUpdatePICApptsWhenPICChanges) {
			strMsg += "\r\nTo the procedures assigned to this PIC? (Note: Consult appointments will only inherit master procedures if their detail procedures do not match with the PIC)";
			if (IDNO == AfxMessageBox(strMsg, MB_YESNO | MB_ICONQUESTION)) {
				if (-1 != nAuditTransactionID) {
					RollbackAuditTransaction(nAuditTransactionID);
				}
				return; // User declined
			} else {
				// User accepted the changes
			}
		} else {
			// Do without prompting
		}

		// Run the statement and perform auditing
		// (d.thompson 2010-05-13) - PLID 38643 - Parameterized
		ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, args);
		CommitAuditTransaction(nAuditTransactionID);

		// Send table checkers. Sending a -1 doesn't do anything!
		// (a.wilson 2014-08-12 14:31) - PLID 63199 - replace with table checker ex.
		foreach(ChangedAppointment ap, anChangedAppts) {
			CClient::RefreshAppointmentTable(ap.nAppointmentID, ap.nPatientID, ap.dtStart, ap.dtEnd, ap.nStatus, ap.nShowState, ap.nLocationID, ap.strResourceIDs);
		}
	}
	catch (...) {
		if (-1 != nAuditTransactionID) {
			RollbackAuditTransaction(nAuditTransactionID);
		}
		throw;
	}
}

// (c.haag 2009-08-06 10:20) - PLID 25943 - This function detects whether appointments
// that are about to be marked as No Show are tied to superbills, and asks the user whether
// they want to void them or not. Returns a message box value: IDYES to void the superbills,
// IDNO to not void them, and IDCANCEL to abort the operation entirely.
// (z.manning 2011-04-01 15:12) - PLID 42973 - Added optional param for whether or not to have a cancel option.
UINT NeedsToVoidNoShowSuperbills(CSqlFragment sqlfragmentApptWhere, bool bCancelOption /* = true */)
{
	int nPropValue = GetRemotePropertyInt("DisplayNoShowVoidSuperbillPrompt", 1, 0, "<None>", false);
	if((nPropValue > 0) && (GetCurrentUserPermissions(bioVoidSuperbills) & sptWrite))
	{
		CString strSuperbillIDs = "";
		long nSuperbillIDCnt = 0;
		// (d.thompson 2010-05-11) - PLID 38581 - Cannot be simply parameterized
		// (z.manning 2011-04-01 15:20) - PLID 42973 - Now it can.
		_RecordsetPtr prsSuperbill = CreateParamRecordset(
			"SELECT SavedID \r\n"
			"FROM PrintedSuperbillsT \r\n"
			"WHERE ReservationID IN (SELECT AppointmentsT.ID FROM AppointmentsT WHERE {SQL}) AND Void = 0 \r\n"
			, sqlfragmentApptWhere);
		while(!prsSuperbill->eof) {
			long nID = AdoFldLong(prsSuperbill, "SavedID", -1);
			if(nID > -1) {
				CString str;	str.Format("%li, ", nID);
				strSuperbillIDs += str;
				nSuperbillIDCnt++;
			}

			prsSuperbill->MoveNext();
		}
		strSuperbillIDs.TrimRight(", ");
		prsSuperbill->Close();

		if(nSuperbillIDCnt > 0) {
			//They are tied to superbills, we will warn the user and give them an opportunity to give up
			if (2 == nPropValue) {
				// 2 means the user always wants to void them
				return IDYES;
			}
			if (1 == nPropValue) {
				CString strFmt;
				strFmt.Format("The appointment(s) being marked No Show are tied to %li superbill%s (ID%s:  %s).  "
					"Would you like to mark these superbills VOID?\r\n\r\n"
					" - If you choose YES, the appointments will be marked No Show and all related superbill IDs will be marked VOID.\r\n"
					" - If you choose NO, the appointments will be marked No Show, but all related superbill IDs remain."
					, nSuperbillIDCnt, nSuperbillIDCnt == 1 ? "" : "s", nSuperbillIDCnt == 1 ? "" : "s", strSuperbillIDs);

				// (z.manning 2011-04-01 15:17) - PLID 42973 - We may not want to include cancel as an option in the message box.
				UINT nMessageBoxType;
				if(bCancelOption) {
					strFmt += "\r\n - If you choose CANCEL, the appointments will remain unchanged.";
					nMessageBoxType = MB_YESNOCANCEL;
				}
				else {
					nMessageBoxType = MB_YESNO;
				}

				return AfxMessageBox(strFmt, nMessageBoxType);
			}
			else {
				// This should never happen
				return IDNO; 
			}
		}
		else {
			// No superbills. Nothing to do
			return IDNO;
		}
	}
	else {
		// Preference is off, or user doesn't have permission
		return IDNO;
	}	
}

// (c.haag 2010-11-16 13:49) - PLID 39444 - Given a location name, this will return
// the location in abbreviated form.
CString GetLocationAbbreviation(CString strLocation)
{
	// - Every abbreviation will be limited to 3 letters
	// - Multi-word locations like "Central Outpatient Surgery" will have their abbreviations formed by the first letter of each name (e.g. "COS")
	// - Single-word locations will have the first three letters of their name be the abbreviation (e.g. "Office" will be "Off")
	CString strResult;

	// Trim right whitespaces
	strLocation.TrimRight();

	// Quit if the location name is empty
	if (strLocation.IsEmpty()) 
	{
		return "";
	}
	// Handle multi-word names
	else if (-1 != strLocation.Find(' ')) 
	{
		strResult.AppendChar(strLocation[0]);
		int n = strLocation.Find(' ');
		while (n > -1 && strResult.GetLength() < 3) {
			strResult.AppendChar(strLocation[n+1]);
			// Skip over all adjacent whitespaces. No need to test for end of string
			// because we already trimmed all the trailing whitespaces off.
			while (strLocation[n] == ' ') {
				n++;
			}
			n = strLocation.Find(' ', n);
		}
		// Always in all caps
		strResult.MakeUpper();
	}
	else // Handle single-word names
	{
		strResult = strLocation.Left( min(strLocation.GetLength(), 3) );
	}

	// All done
	return strResult;
}

// (z.manning 2011-05-06 15:33) - PLID 43601
void FillDefaultDurationGroups(CArray<CDurationGroup*,CDurationGroup*> *paryDurationGroups, const CArray<long,long> *paryResourceIDs)
{
	if(paryResourceIDs->GetSize() == 0) {
		return;
	}

	//(b.spivey - June 24th, 2014) - PLID 60788 - Added a distinct clause. Sometimes what would happen is we would get repeated records and end up with a purpose string of "213, 213" or something like that. 
	//	 This will ensure that every combination is unique, and should eliminate that situation. 
	_RecordsetPtr prs = CreateParamRecordset(
		"SELECT DISTINCT ProviderSchedDefDurationT.ID, AptTypeID, DurationMinutes, AptPurposeID \r\n"
		"FROM ProviderSchedDefDurationT \r\n"
		"INNER JOIN ProviderSchedDefDurationDetailT ON ProviderSchedDefDurationT.ID = ProviderSchedDefDurationDetailT.ProviderSchedDefDurationID \r\n"
		"INNER JOIN ResourceProviderLinkT ON ResourceProviderLinkT.ProviderID = ProviderSchedDefDurationT.ProviderID \r\n"
		"WHERE ResourceID IN ({INTARRAY}) \r\n"
		"ORDER BY ProviderSchedDefDurationT.ID \r\n"
		, *paryResourceIDs);
	if(prs->eof) {
		return;
	}

	CDurationGroup* pCurGroup = new CDurationGroup(AdoFldLong(prs, "AptTypeID"), AdoFldLong(prs, "DurationMinutes"));
	long nPreviousDurationID = AdoFldLong(prs, "ID");
	CMap<CString,LPCTSTR,CDurationGroup*,CDurationGroup*> mapTypePurposesToDuration;
	while(!prs->eof)
	{
		pCurGroup->AddPurpose(AdoFldLong(prs, "AptPurposeID"));

		// (z.manning 2011-05-10 08:51) - PLID 43601 - Now move to the next record and see if that begins a new
		// default duration group.
		prs->MoveNext();
		if(prs->eof || AdoFldLong(prs, "ID") != nPreviousDurationID)
		{
			if(pCurGroup != NULL && pCurGroup->GetPurposeCount() > 0)
			{
				// (z.manning 2011-05-10 08:52) - PLID 43601 - Since we now support multiple resources for default durations,
				// it's possible that we'll get the exact same type/purposes combination more than once. To be consistent with
				// j.jones implemention from PLID 36473, we use whatever one had the longer duration.
				CString strTypePurpose = FormatString("%li|%s", pCurGroup->GetAptTypeID(), pCurGroup->GetPurposeString());
				CDurationGroup* pExistingGroup = NULL;
				if(mapTypePurposesToDuration.Lookup(strTypePurpose, pExistingGroup))
				{
					// (z.manning 2011-05-09 11:28) - PLID 43601 - We have an existing group for the same set of purposes so we
					// want to keep whichever one has the higher duration.
					if(pExistingGroup->GetDuration() > pCurGroup->GetDuration()) {
						// (z.manning 2011-05-09 11:26) - PLID 43601 - The existing duration group has a higher duration so keep using it.
						delete pCurGroup;
						pCurGroup = NULL;
					}
					else {
						// (z.manning 2011-05-09 11:28) - PLID 43601 - The current duration group has a higher duration so let's use
						// it and clean up memory of the existing one.
						mapTypePurposesToDuration.SetAt(strTypePurpose, pCurGroup);
						delete pExistingGroup;
						pExistingGroup = NULL;
					}
				}
				else {
					// (z.manning 2011-05-09 11:29) - PLID 43601 - This is the first duration group for this set of purposes.
					mapTypePurposesToDuration.SetAt(strTypePurpose, pCurGroup);
				}
			}

			// (z.manning 2011-05-10 09:11) - PLID 43601 - Assuming we're not at the end of the recordset then we need to 
			// start the next duration group.
			if(!prs->eof) {
				pCurGroup = new CDurationGroup(AdoFldLong(prs, "AptTypeID"), AdoFldLong(prs, "DurationMinutes"));
			}
		}

		if(!prs->eof) {
			nPreviousDurationID = AdoFldLong(prs, "ID");
		}
	}

	POSITION pos = mapTypePurposesToDuration.GetStartPosition();
	while(pos != NULL)
	{
		CString strPurposeIDs;
		CDurationGroup *pDurationGroup = NULL;
		mapTypePurposesToDuration.GetNextAssoc(pos, strPurposeIDs, pDurationGroup);
		paryDurationGroups->Add(pDurationGroup);
	}
}

// (c.haag 2011-06-23) - PLID 44287 - Shows a patient warning in a scheduler-favorable layout.
// All the code in this function was moved from CSchedulerView::ShowPatientWarning to here.
void ShowSchedulerPatientWarning(CWnd* pParentWnd,
		long nPatientID,
		BOOL bSuppressPtWarning /*= FALSE */,
		BOOL bSuppressInsReferralWarning /*= FALSE */,
		BOOL bSuppressCoPayWarning /*= FALSE */,
		BOOL bSuppressAllergyWarning /*= FALSE */,
		BOOL bSuppressEMRProblemWarning /*= FALSE */,
		BOOL bSuppressRewardsWarning /*= FALSE */) // (j.luckoski 2013-03-04 12:59) - PLID 33548 
{
	// (a.walling 2009-06-05 13:01) - PLID 34496 - Use a proper parent for all these dialogs
	if (!bSuppressPtWarning && GetRemotePropertyInt("ResShowPatientWarning", 1, 0, GetCurrentUserName(), false)) {
		// (d.thompson 2010-05-06) - PLID 38525 - Parameterized.  Note that this query was previously on 'adOpenDynamic' and 'adLockOptimistic', but
		//	I see no valid reason to maintain that.  We're just iterating through a 1-time query like almost all our other ones.
		_RecordsetPtr prs = CreateParamRecordset(
			"SELECT DisplayWarning, WarningMessage, WarningDate, WarningExpireDate, WarningUseExpireDate, UserName, "
			"Last + ', ' + First + ' ' + Middle AS FullName "
			"FROM PersonT  "
			"LEFT JOIN UsersT ON UsersT.PersonID = PersonT.WarningUserID "
			"WHERE PersonT.ID = {INT}", nPatientID);
		// Proceed only if we are on a good record
		if (!prs->eof) {
			FieldsPtr flds = prs->Fields;

			// Handle warnings
			if (AdoFldBool(flds, "DisplayWarning")) {

				BOOL bWarn = TRUE;

				//check to see if they are using the expiration date
				if (AdoFldBool(flds, "WarningUseExpireDate")) {

					//check to see if the warning has expired
					COleDateTime dtExpire = VarDateTime(flds->Item["WarningExpireDate"]->Value);
					COleDateTime dtToday;
					dtToday.SetDate(COleDateTime::GetCurrentTime().GetYear(), COleDateTime::GetCurrentTime().GetMonth(), COleDateTime::GetCurrentTime().GetDay());
					if (dtExpire < dtToday) {

						//don't show it
						bWarn = FALSE;
					}
				}

				if(bWarn) {

					CUserWarningDlg dlg(pParentWnd);
					BOOL bKeepWarning = dlg.DoModalWarning(nPatientID, TRUE);
					// If the user set it to not display take it out of the data
					if (!bKeepWarning) {
						// (d.thompson 2010-05-06) - PLID 38525 - Parameterized
						ExecuteParamSql("UPDATE PersonT SET DisplayWarning = {INT} WHERE ID = {INT}",bKeepWarning ? 1 : 0, nPatientID);
						//auditing
						long nAuditID = BeginNewAuditEvent();
						AuditEvent(nPatientID, GetExistingPatientName(nPatientID), nAuditID, aeiPatientWarning, nPatientID, (bKeepWarning == 0 ? "Warn" : "Don't Warn"), (bKeepWarning == 0 ? "Don't Warn" : "Warn"), aepMedium, aetChanged);

					}
				}
			}
		}
	}

	if(!bSuppressInsReferralWarning && GetRemotePropertyInt("ShowInsuranceReferralWarning",0,0,"<None>",TRUE) == 1) {
		//show active insurance referrals
		// (c.haag 2008-11-21 11:04) - PLID 32136 - Filter out inactive insured parties
		// (d.thompson 2010-05-06) - PLID 38525 - Parameterized
		// (j.jones 2011-09-29 15:53) - PLID 44980 - ignore referrals if in use on a voided bill
		// (j.gruber 2011-10-05 13:24) - PLID 45837 - add provider, location, and comments
		// (j.gruber 2011-10-05 15:11) - PLID 45838 - changed to be any referral that isn't expired currently
		_RecordsetPtr rsInsuranceReferrals = CreateParamRecordset("SELECT InsuranceCoT.Name AS InsCoName, "
			"AuthNum, StartDate, EndDate, NumVisits, (CASE WHEN NumUsed Is NULL THEN 0 ELSE NumUsed END) AS NumUsed, "
			" PersonProvT.ID as ProviderID, PersonProvT.Last + ', ' + PersonProvT.First + ' ' + PersonProvT.Middle AS ProviderName, "
			" LocationsT.ID as LocationID, LocationsT.Name as LocationName, InsuranceReferralsT.Comments "
			"FROM InsuranceReferralsT "
			"INNER JOIN InsuredPartyT ON InsuranceReferralsT.InsuredPartyID = InsuredPartyT.PersonID "
			"INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
			"LEFT JOIN ProvidersT ON InsuranceReferralsT.ProviderID = ProvidersT.PersonID "
			"LEFT JOIN PersonT PersonProvT ON ProvidersT.PersonID = PersonProvT.ID "
			"LEFT JOIN LocationsT ON InsuranceReferralsT.LocationId = LocationsT.ID "					
			"LEFT JOIN (SELECT Count(InsuranceReferralID) AS NumUsed, InsuranceReferralID FROM BillsT WHERE Deleted = 0 AND InsuranceReferralID IS NOT NULL AND BillsT.ID NOT IN (SELECT OriginalBillID FROM BillCorrectionsT) "
			"GROUP BY InsuranceReferralID) AS NumUsedQ ON InsuranceReferralsT.ID = NumUsedQ.InsuranceReferralID "
			"WHERE NumVisits > (CASE WHEN NumUsed Is NULL THEN 0 ELSE NumUsed END) AND EndDate >= {STRING} AND InsuredPartyT.PatientID = {INT} "
			"AND InsuredPartyT.RespTypeID <> -1 "
			"ORDER BY StartDate", FormatDateTimeForSql(COleDateTime::GetCurrentTime(),dtoDate), nPatientID);

		if(!rsInsuranceReferrals->eof) {

			CString strPrompt;
			strPrompt = "This patient has the following active insurance referrals:";

			while(!rsInsuranceReferrals->eof) {

				CString strInsCoName = AdoFldString(rsInsuranceReferrals, "InsCoName","");
				CString strAuthNum = AdoFldString(rsInsuranceReferrals, "AuthNum","");
				COleDateTime dtStartDate = AdoFldDateTime(rsInsuranceReferrals, "StartDate");
				COleDateTime dtEndDate = AdoFldDateTime(rsInsuranceReferrals, "EndDate");
				long NumVisits = AdoFldLong(rsInsuranceReferrals, "NumVisits",0);
				long NumUsed = AdoFldLong(rsInsuranceReferrals, "NumUsed",0);

				CString strDates;
				if(dtStartDate == dtEndDate) {
					strDates.Format("Allowed Date: %s;",FormatDateTimeForInterface(dtStartDate,NULL,dtoDate));
				}
				else {
					strDates.Format("Allowed Dates: %s - %s;",FormatDateTimeForInterface(dtStartDate,NULL,dtoDate),FormatDateTimeForInterface(dtEndDate,NULL,dtoDate));
				}

				// (j.gruber 2011-10-05 12:22) - PLID 45837 - added providers, locations, and comments
				long nProviderID = AdoFldLong(rsInsuranceReferrals, "ProviderID", -1);
				CString strProvName;
				if (nProviderID != -1) {
					strProvName = "Provider: " + AdoFldString(rsInsuranceReferrals, "ProviderName", "") + ";";
				}
				CString strLocName;
				long nLocationID = AdoFldLong(rsInsuranceReferrals, "LocationID", -1);
				if (nLocationID != -1) {
					strLocName = "Location: " + AdoFldString(rsInsuranceReferrals, "LocationName", "") + ";";
				}

				CString strTemp;
				strTemp = AdoFldString(rsInsuranceReferrals, "Comments", "");
				CString strComments;
				if (!strTemp.IsEmpty()) {
					strComments = "Comments: " + strTemp + ";";
				}
				

				CString strWarning;
				strWarning.Format("\n\nInsurance: %s; Allowed Visits: %li, Remaining: %li; %s %s %s %s",strInsCoName,NumVisits,NumVisits - NumUsed,strDates, strProvName, strLocName, strComments);
				strPrompt += strWarning;

				rsInsuranceReferrals->MoveNext();
			}

			pParentWnd->MessageBox(strPrompt, NULL, MB_ICONINFORMATION);
		}

		rsInsuranceReferrals->Close();
	}

	// (j.jones 2010-08-02 11:23) - PLID 39937 - WarnCoPay is now a preference, and no longer a setting per insured party,
	// when loading an appt. we would have been passed in false if this patient had no copays
	// (j.jones 2010-09-01 15:58) - PLID 40356 - changed so we never warn about $0.00 / 0% copays
	if (!bSuppressCoPayWarning && GetRemotePropertyInt("WarnCopays", 0, 0, GetCurrentUserName(), true) == 1) {
		//now check for the CoPay Warning
		// (j.jones 2009-09-17 10:33) - PLID 35572 - ensure we do not warn about inactive insurances
		_RecordsetPtr rsCoPay = CreateParamRecordset(
			" SELECT [Last] + ', ' + [First] AS FullName, InsuranceCoT.Name AS InsCoName, "
			" CoPayMoney, CopayPercentage, ServicePayGroupsT.Name AS PayGroupName "
			" FROM InsuredPartyT "
			" INNER JOIN PatientsT ON InsuredPartyT.PatientID = PatientsT.PersonID "
			" INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			" INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
			" INNER JOIN InsuredPartyPayGroupsT ON InsuredPartyT.PersonID = InsuredPartyPayGroupsT.InsuredPartyID "
			" INNER JOIN ServicePayGroupsT ON InsuredPartyPayGroupsT.PayGroupID = ServicePayGroupsT.ID "
			" WHERE PatientsT.PersonID = {INT} AND InsuredPartyT.RespTypeID <> -1 "
			" AND ((CoPayMoney Is Not Null AND CopayMoney <> Convert(money,0)) OR (CopayPercentage Is Not Null AND CopayPercentage <> 0)) "
			" ORDER BY RespTypeID, ServicePayGroupsT.Name", nPatientID);

		if(!rsCoPay->eof) {

			CString strName = AdoFldString(rsCoPay, "FullName","") + ":";
			CString strCoPayWarning = "This patient has the following copays:\r\n";

			while(!rsCoPay->eof) {

				CString str, strCopay;

				// (j.jones 2010-08-03 13:17) - PLID 39937 - the copay is now in two fields,
				// mutually exclusive, they cannot both be filled
				_variant_t varCoPayMoney = rsCoPay->Fields->Item["CoPayMoney"]->Value;
				_variant_t varCopayPercentage = rsCoPay->Fields->Item["CopayPercentage"]->Value;

				// (j.jones 2010-09-01 15:58) - PLID 40356 - changed so we never warn about $0.00 / 0% copays
				if(varCoPayMoney.vt == VT_CY && VarCurrency(varCoPayMoney) != COleCurrency(0,0)) {
					//flat amount
					strCopay.Format("%s",FormatCurrencyForInterface(VarCurrency(varCoPayMoney,COleCurrency(0,0))));
				}
				else if(varCopayPercentage.vt == VT_I4 && VarLong(varCopayPercentage) != 0) {
					//percentage
					strCopay.Format("%li%%",VarLong(varCopayPercentage,0));
				}
				else {
					//shady, why do we even have results?
					rsCoPay->MoveNext();
					continue;
				}

				str.Format("\n%s - %s: %s",
					AdoFldString(rsCoPay, "InsCoName",""),
					AdoFldString(rsCoPay, "PayGroupName",""),
					strCopay);
				strCoPayWarning += str;

				rsCoPay->MoveNext();
			}


			// Display it
			pParentWnd->MessageBox(strCoPayWarning, "CoPay Warning", MB_ICONINFORMATION|MB_OK);
		}
		HR(rsCoPay->Close());
	}

	if (!bSuppressAllergyWarning) {
		//ok, now check for the Allergy Warning, this code is basically copied from above with changes for the recordset, etc
		// Open the recordset
		// (j.gruber 2009-06-02 15:14) - PLID 34450 - take out inactive medications
		// (d.thompson 2010-05-06) - PLID 38525 - Parameterized.  Note that this query was previously on 'adOpenDynamic' and 'adLockOptimistic', but
		//	I see no valid reason to maintain that.  We're just iterating through a 1-time query like almost all our other ones.
		// (j.jones 2013-08-12 15:52) - PLID 57977 - Used snapshot isolation and changed this query to get the allergies itself and not call the UDF function.
		// This is a left join because if warning is enabled, but they have no allergies, we say "<None>".
		_RecordsetPtr rsAllergy = CreateParamRecordset(GetRemoteDataSnapshot(), 
			"SELECT PersonT.FullName, PatientAllergyQ.AllergyName "
			"FROM PatientsT "
			"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			"LEFT JOIN (SELECT EmrDataT.Data AS AllergyName, PatientAllergyT.PersonID "
			"	FROM PatientAllergyT "
			"	LEFT JOIN AllergyT ON PatientAllergyT.AllergyID = AllergyT.ID "
			"	LEFT JOIN EmrDataT ON EmrDataT.ID = AllergyT.EmrDataID "
			"	WHERE PatientAllergyT.Discontinued = 0) AS PatientAllergyQ ON PatientsT.PersonID = PatientAllergyQ.PersonID "
			"WHERE PatientsT.PersonID = {INT} "
			"AND PersonT.DisplayAllergyWarning = 1 "
			"ORDER BY PatientAllergyQ.AllergyName", nPatientID);
		if(!rsAllergy->eof) {
			FieldsPtr flds = rsAllergy->Fields;
			CString strAllergyPatientName = AdoFldString(flds, "FullName");
			CString strAllergyList = "";

			while(!rsAllergy->eof) {
				//if we have only one record, and there are no allergies, we show "<None>"
				CString strAllergyName = AdoFldString(flds, "AllergyName", "<None>");
				if(!strAllergyList.IsEmpty()) {
					strAllergyList += ", ";
				}
				strAllergyList += strAllergyName;
				rsAllergy->MoveNext();
			}
				
			CString strAllergyWarning = "This patient has the following allergies:  " + strAllergyList;

			// Display it
			CUserWarningDlg dlgWarning(pParentWnd);
			// (a.walling 2010-07-01 16:34) - PLID 18081 - Warning categories
			BOOL bKeepWarning = dlgWarning.DoModalWarning(strAllergyWarning, 
				TRUE, strAllergyPatientName + ":", "Allergy Warning", NULL, -1, "");

			//if they turned off the warning, save the change
			if(bKeepWarning == 0) {
				ExecuteParamSql("UPDATE PersonT SET DisplayAllergyWarning = 0 WHERE ID = {INT}", nPatientID);
			}
		}
		rsAllergy->Close();
	}
	
	// (j.luckoski 2013-03-04 13:00) - PLID 33548 - Added rewards warning.
	if (!bSuppressRewardsWarning) {
		
		COleCurrency cyRewards;
		cyRewards = Rewards::GetTotalPoints(nPatientID);

		_RecordsetPtr rsRewards = CreateParamRecordset("Select PersonT.DisplayRewardsWarning, PersonT.FullName from PersonT where ID = {INT}", nPatientID);

		if(!rsRewards->eof && cyRewards > g_ccyZero && GetRemotePropertyInt("DisplayRewardsWarning",0,0, GetCurrentUserName(), true) == 1 && g_pLicense->CheckForLicense(CLicense::lcNexSpa, CLicense::cflrSilent)) {
				
				FieldsPtr flds = rsRewards->Fields;
				CString strRewards = FormatCurrencyForInterface(cyRewards, FALSE, TRUE);

				FieldPtr fldDispWarning = flds->Item["DisplayRewardsWarning"];

				// If the patient is displaying a warning
				BOOL bDispWarning = AdoFldBool(fldDispWarning);
				if (bDispWarning) {


				CString strRewardsWarning = "This patient has available reward points:  " +  strRewards;

				// Display it
				CUserWarningDlg dlgWarning(pParentWnd);
				// (a.walling 2010-07-01 16:34) - PLID 18081 - Warning categories
				BOOL bKeepWarning = dlgWarning.DoModalWarning(strRewardsWarning, 
					TRUE, AdoFldString(flds, "FullName") + ":", "Reward Points Warning", NULL, -1, "");

				// If the user asked to change the displaywarning status of the patient
				if((bKeepWarning == 0 && bDispWarning != 0) || (bKeepWarning != 0 && bDispWarning == 0)) {
					// Change it
					fldDispWarning->Value = bKeepWarning?TRUE:FALSE;
					rsRewards->Update();
				}
			}
		}
		HR(rsRewards->Close());
	}


	if (!bSuppressEMRProblemWarning) {
		// (c.haag 2006-07-03 16:54) - PLID 19977 - Show patient EMR problem warnings
		// (a.walling 2009-06-05 12:59) - PLID 34496 - Include parent window
		PromptPatientEMRProblemWarning(nPatientID, pParentWnd);
	}
}

// (c.haag 2011-06-23) - PLID 44287 - This function is used to edit an inventory allocation for an appointment
void AppointmentEditInvAllocation(CWnd* pParentWnd, long nApptID, BOOL bInSchedulerView,
								  long nPatientID /* = -1 */, long nLocationID /* = -1 */)
{
	// (a.walling 2008-03-21 12:24) - PLID 28946 - Check for adv inventory licensing
	//DRT - PLID 30117 - Use BetaHasAdvInventory() for now.  Remove this when beta period is over.
	// (d.thompson 2009-01-27) - PLID 32859 - Replaced beta with C&A module
	if(!g_pLicense->HasCandAModule(CLicense::cflrUse) || !g_pLicense->CheckForLicense(CLicense::lcInv, CLicense::cflrUse)) {	
		MsgBox("An Inventory Allocation can only be edited if you have purchased the Inventory license.\n"
			"Please contact NexTech Sales if you wish to add this module.",MB_ICONEXCLAMATION|MB_OK);
		return;
	}

	// Get the patient and location
	if (-1 == nPatientID && -1 == nLocationID) 
	{
		_RecordsetPtr rs = CreateParamRecordset("SELECT PatientID, LocationID FROM AppointmentsT WHERE ID = {INT}", nApptID);
		if(!rs->eof) {
			nPatientID = AdoFldLong(rs, "PatientID",-1);
			nLocationID = AdoFldLong(rs, "LocationID",-1);
		}
		else {
			AfxMessageBox("No patient could be found for this appointment. It is possible the patient or appointment has been deleted.");
			return;
		}
		rs->Close();
	}

	if(nPatientID == -25) {
		//do not give a warning
		return;
	}

	if (bInSchedulerView) {
		ShowSchedulerPatientWarning(pParentWnd, nPatientID);
	}

	//and now create the allocation
	CWaitCursor pWait;
	_RecordsetPtr rsAllocation = CreateParamRecordset("SELECT ID FROM PatientInvAllocationsT WHERE AppointmentID = {INT} AND Status != {INT}", nApptID, InvUtils::iasDeleted);
	if (! rsAllocation->eof) 
	{
		long nAllocationID = AdoFldLong(rsAllocation, "ID", -1);
		CInvPatientAllocationDlg dlg(pParentWnd);
		dlg.m_nID = nAllocationID;
		dlg.m_nDefaultPatientID = nPatientID;
		dlg.m_nDefaultAppointmentID = nApptID;
		dlg.m_nDefaultLocationID = nLocationID;
		
		// (c.haag 2008-03-24 09:23) - PLID 29328 - Now that we can display whether a patient has an allocation
		// in the scheduler, we may need to refresh the view
		if (IDOK == dlg.DoModal()) {
			if (bInSchedulerView && CanShowOpenAllocationsInSchedule()) {
				// (z.manning 2010-10-14 10:10) - PLID 40929 - Post a message instead of calling UpdateView directly
				pParentWnd->PostMessage(NXM_UPDATEVIEW);
			}
		}	
	}
	else {
		AfxMessageBox("The allocation could not be found for this appointment. It is possible that it has been deleted.");
	}
}

// (c.haag 2011-06-23) - PLID 44287 - This function is used to create an inventory allocation for an appointment
void AppointmentCreateInvAllocation(CWnd* pParentWnd, long nApptID, BOOL bInSchedulerView,
								  long nPatientID /* = -1 */, long nLocationID /* = -1 */)
{
	// (a.walling 2008-03-21 12:24) - PLID 28946 - Check for adv inventory licensing
	//DRT - PLID 30117 - Use BetaHasAdvInventory() for now.  Remove this when beta period is over.
	// (d.thompson 2009-01-27) - PLID 32859 - Replaced beta with C&A module
	if(!g_pLicense->HasCandAModule(CLicense::cflrUse) || !g_pLicense->CheckForLicense(CLicense::lcInv, CLicense::cflrUse)) {	
		MsgBox("An Inventory Allocation can only be created if you have purchased the Inventory license.\n"
			"Please contact NexTech Sales if you wish to add this module.",MB_ICONEXCLAMATION|MB_OK);
		return;
	}

	if(!CheckCurrentUserPermissions(bioInventoryAllocation, sptCreate)) {
		return;
	}

	// Get the patient and location
	if (-1 == nPatientID && -1 == nLocationID) 
	{
		_RecordsetPtr rs = CreateParamRecordset("SELECT PatientID, LocationID FROM AppointmentsT WHERE ID = {INT}", nApptID);
		if(!rs->eof) {
			nPatientID = AdoFldLong(rs, "PatientID",-1);
			nLocationID = AdoFldLong(rs, "LocationID",-1);
		}
		else {
			AfxMessageBox("No patient could be found for this appointment. It is possible the patient or appointment has been deleted.");
			return;
		}
		rs->Close();
	}

	if(nPatientID == -25) {
		//do not give a warning
		return;
	}

	if (bInSchedulerView) {
		ShowSchedulerPatientWarning(pParentWnd, nPatientID);
	}

	//and now create the allocation

	CWaitCursor pWait;
	CInvPatientAllocationDlg dlg(pParentWnd);
	dlg.m_nDefaultPatientID = nPatientID;
	dlg.m_nDefaultAppointmentID = nApptID;
	dlg.m_nDefaultLocationID = nLocationID;
	// (c.haag 2008-03-24 09:23) - PLID 29328 - Now that we can display whether a patient has an allocation
	// in the scheduler, we may need to refresh the view
	if (IDOK == dlg.DoModal()) {
		if (bInSchedulerView && CanShowOpenAllocationsInSchedule()) {
			// (z.manning 2010-10-14 10:10) - PLID 40929 - Post a message instead of calling UpdateView directly
			pParentWnd->PostMessage(NXM_UPDATEVIEW);
		}
	}	
}

// (c.haag 2011-06-23) - PLID 44287 - This function is used to create an inventory order for an appointment
void AppointmentCreateInvOrder(CWnd* pParentWnd, long nApptID, BOOL bInSchedulerView,
								  long nPatientID /* = -1 */, long nLocationID /* = -1 */)
{
	//DRT - PLID 30117 - Use BetaHasAdvInventory() for now.  Remove this when beta period is over.
	// (d.thompson 2009-01-26) - PLID 32851 - Renamed to 'Consignment and Allocation' from 'Advanced Inventory'
	// (d.thompson 2009-01-27) - PLID 32859 - Replaced beta with C&A module
	if(!g_pLicense->HasCandAModule(CLicense::cflrUse) || !g_pLicense->CheckForLicense(CLicense::lcInv, CLicense::cflrUse)) {
		MsgBox("An Inventory Order can only be created if you have purchased the Inventory license.\n"
			"Please contact NexTech Sales if you wish to add this module.",MB_ICONEXCLAMATION|MB_OK);
		return;
	}

	if(!CheckCurrentUserPermissions(bioInvOrder, sptCreate)) {
		return;
	}

	// Get the patient and location
	if (-1 == nPatientID && -1 == nLocationID) 
	{
		_RecordsetPtr rs = CreateParamRecordset("SELECT PatientID, LocationID FROM AppointmentsT WHERE ID = {INT}", nApptID);
		if(!rs->eof) {
			nPatientID = AdoFldLong(rs, "PatientID",-1);
			nLocationID = AdoFldLong(rs, "LocationID",-1);
		}
		else {
			AfxMessageBox("No patient could be found for this appointment. It is possible the patient or appointment has been deleted.");
			return;
		}
		rs->Close();
	}

	if(nPatientID == -25) {
		//do not give a warning
		return;
	}

	if (bInSchedulerView) {
		ShowSchedulerPatientWarning(pParentWnd, nPatientID);
	}
	//and now create the order

	CWaitCursor pWait;
	// (j.jones 2008-03-18 13:57) - PLID 29309 - cancel if there is already an order linked to this appt.
	// (j.jones 2008-09-24 16:37) - PLID 31493 - changed to allow multiple orders, but warn first
	_RecordsetPtr rsOrders = CreateParamRecordset("SELECT ID FROM OrderAppointmentsT "
		"WHERE AppointmentID = {INT}", nApptID);
	if(!rsOrders->eof) {
		CString strWarning = "There is already an order linked to this appointment. "
			"Are you sure you wish to create an additional order linked to this appointment?";
		if(rsOrders->GetRecordCount() > 1) {
			strWarning = "There are already multiple orders linked to this appointment. "
				"Are you sure you wish to create an additional order linked to this appointment?";
		}

		int nRes = pParentWnd->MessageBox(strWarning, "Practice", MB_ICONQUESTION|MB_YESNO);
		if(nRes != IDYES) {
			return;
		}
	}
	rsOrders->Close();

	CMainFrame *p = GetMainFrame();
	CNxTabView *pView;

	if(p) {
		if(p->FlipToModule(INVENTORY_MODULE_NAME)) {

			pView = (CNxTabView *)p->GetOpenView(INVENTORY_MODULE_NAME);
			if (pView)  {
				if(pView->GetActiveTab() == InventoryModule::OrderTab) {
					pView->UpdateView();
				}
				else {
					pView->SetActiveTab(InventoryModule::OrderTab);
				}

				((CInvOrderDlg *)pView->GetActiveSheet())->CreateOrder(FALSE, nApptID, nLocationID);
			}
		}
	}

}

// (c.haag 2011-06-23) - PLID 44287 - This function is used to create a new bill for a patient given an appointment ID
void AppointmentCreateNewBill(CWnd* pParentWnd, long nApptID, BOOL bInSchedulerView,
								  long nPatientID /* = -1 */)
{
	if (-1 == nPatientID) { nPatientID = AppointmentGetPatientID(nApptID); }

	if(nPatientID == -25) {
		//do not give a warning
		return;
	}

	if(!CheckCurrentUserPermissions(bioBill, sptCreate)) {
		return;
	}

	//will this appointment exist in the bill list?
	CString str;
	// (a.walling 2010-10-19 09:45) - PLID 40965 - Use ReturnsRecordsParam
	str.Format("%s AND Q.ID = {INT}", GetBillAppointmentQuery());
	// (z.manning 2008-12-11 09:05) - PLID 32397 - Fixed IsRecordsetEmpty call to prevent text formatting errors
	if(!ReturnsRecordsParam(str, nApptID)) {
		//explain why the appt. may not be billable
		// (j.gruber 2010-07-20 15:57) - PLID 39739 - added types as billable
		AfxMessageBox("This appointment is not able to be billed. Billable appointments are those that are for a procedural type "
			"(Office Procedure, Surgery, etc.) and have a purpose that has service codes or inventory items linked to it "
			"or are those that have a non-procedural appointment type and have service codes or inventory items linked to their appointment type. "
			"The appointment also must not be marked as a no-show.");
		return;
	}

	if (bInSchedulerView) {
		// (s.dhole 2012-02-29 17:09) - PLID 48521 we have to pass patient ID
		ShowSchedulerPatientWarning(pParentWnd, nPatientID);
	}

	//and now create the bill
	
	// (a.walling 2009-12-22 17:27) - PLID 7002 - Maintain only one instance of a bill
	if (GetMainFrame()->IsBillingModuleOpen(true)) {
		return;
	}

	CWaitCursor pWait;

	CMainFrame *pMainFrame = GetMainFrame();

	if(pMainFrame) {
		//switch to the patients module, switch to that patient, and open a bill

		//Set the active patient
		if(!pMainFrame->m_patToolBar.DoesPatientExistInList(nPatientID)) {
			if(IDNO == pParentWnd->MessageBox("This patient is not in the current lookup. \n"
				"Do you wish to reset the lookup to include all patients?","Practice",MB_ICONQUESTION|MB_YESNO)) {
				return;
			}
		}
		
		//TES 1/7/2010 - PLID 36761 - This function may fail now
		if(!pMainFrame->m_patToolBar.TrySetActivePatientID(nPatientID)) {
			return;
		}
		
		//Now just flip to the patient's module and set the active Patient
		if(pMainFrame->FlipToModule(PATIENT_MODULE_NAME)) {
			CNxTabView *pView = pMainFrame->GetActiveView();
			if(pView) {
				pView->UpdateView();
			}

			//now open a bill
			CPatientView* pPatView = (CPatientView*)pMainFrame->GetOpenView(PATIENT_MODULE_NAME);
			// (z.manning 2008-09-04 11:08) - PLID 31252 - Switch to the billing tab.
			pView->SetActiveTab(PatientsModule::BillingTab);
			CBillingModuleDlg *pBillingDlg = pPatView->GetBillingDlg();

			pBillingDlg->m_pFinancialDlg = pPatView->GetFinancialDlg();
			pBillingDlg->m_nPatientID = GetActivePatientID();
			pBillingDlg->OpenWithBillID(-1, BillEntryType::Bill, 1);

			pBillingDlg->PostMessage(NXM_BILL_APPOINTMENT, nApptID);
		}
	}
	else {
		AfxThrowNxException("Cannot Open MainFrame");
	}
}

// (c.haag 2011-06-23) - PLID 44319 - This function is used to create a new case history for a patient given an appointment ID
void AppointmentCreateCaseHistory(long nApptID, long nPatientID)
{
	if(nPatientID == -25) {
		return;
	}

	//incase one exists, open it
	_RecordsetPtr rsCase = CreateParamRecordset("SELECT TOP 1 ID FROM CaseHistoryT WHERE AppointmentID = {INT}\r\nSELECT StartTime FROM AppointmentsT WHERE ID = {INT}", nApptID, nApptID);
	if(!rsCase->eof) {
		AppointmentEditCaseHistory(nApptID, nPatientID);
		return;
	}

	rsCase = rsCase->NextRecordset(NULL);	
	if(rsCase->eof) {
		AfxMessageBox("A case history could not be created. The appointment may have been deleted.");
		return;
	}
	COleDateTime dtSurgeryDate = AdoFldDateTime(rsCase->Fields, "StartTime");

	//make a case history
	CString strCaseHistoryName;
	long nProviderID;
	// (j.jones 2009-08-31 15:07) - PLID 35378 - we now allow multiple preference cards to be chosen
	CArray<long, long> arynPreferenceCardIDs;
	// (j.jones 2009-08-31 17:54) - PLID 17734 - this now takes in the appointment ID
	if(CCaseHistoryDlg::ChoosePreferenceCards(arynPreferenceCardIDs, strCaseHistoryName, nProviderID, nPatientID, nApptID)) {
		CCaseHistoryDlg dlg(NULL);
		dlg.OpenNewCase(nPatientID, arynPreferenceCardIDs, strCaseHistoryName, nProviderID, dtSurgeryDate, nApptID);
	}
}

// (c.haag 2011-06-23) - PLID 44319 - This function is used to edit an existing case history for a patient given an appointment ID
void AppointmentEditCaseHistory(long nApptID, long nPatientID)
{
	if(nPatientID == -25) {
		return;
	}

	_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 ID FROM CaseHistoryT WHERE AppointmentID = {INT}", nApptID);
	if(!rs->eof) {
		CCaseHistoryDlg dlg(NULL);
		dlg.OpenExistingCase(AdoFldLong(rs, "ID"));
	}
	else {
		AfxMessageBox("No case histories are associated with this appointment.");
	}
	rs->Close();
}

// (z.manning 2011-12-07 12:13) - PLID 46910 - Function to load resources from data into a map
void FillResourceMap(CMap<long,long,CString,LPCTSTR> *pmapResources)
{
	FillResourceMap(pmapResources, NULL);
}

// (z.manning 2011-12-07 12:13) - PLID 46910 - Function to load resources from data into a map
void FillResourceMap(CMap<long,long,CString,LPCTSTR> *pmapResources, CArray<long> *parynResourceIDs)
{
	CSqlFragment sqlWhere;
	if(parynResourceIDs != NULL && parynResourceIDs->GetCount() > 0) {
		sqlWhere = CSqlFragment("WHERE ResourceT.ID IN ({INTARRAY}) ", *parynResourceIDs);
	}
	_RecordsetPtr prs = CreateParamRecordset("SELECT ID, Item FROM ResourceT {SQL}", sqlWhere);
	for(; !prs->eof; prs->MoveNext())
	{
		const long nResourceID = AdoFldLong(prs, "ID");
		const CString strResourceName = AdoFldString(prs, "Item", "");
		pmapResources->SetAt(nResourceID, strResourceName);
	}
}

// (j.jones 2012-04-11 09:48) - PLID 44174 - added global function for deleting resources
void DeleteSchedulerResource(CString &strSqlBatch, long nResourceIDToDelete)
{
	if(IsNexTechInternal()) {
		// (z.manning, 02/15/2008) - PLID 28909 - Delete from DepartmentResourceLinkT
		AddStatementToSqlBatch(strSqlBatch, "DELETE FROM DepartmentResourceLinkT WHERE ResourceID = %li", nResourceIDToDelete);
	}

	
	// (z.manning, 07/20/2007) - PLID 26469 - Delete from TemplateItemResourceT
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM TemplateItemResourceT WHERE ResourceID = %li", nResourceIDToDelete);
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM ResourcePurposeTypeT WHERE ResourceID = %li", nResourceIDToDelete);
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM ResourceViewDetailsT WHERE ResourceID = %li", nResourceIDToDelete);
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM UserResourcesT WHERE ResourceID = %li", nResourceIDToDelete);
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM ResourceLocationConnectT WHERE ResourceId = %li", nResourceIDToDelete);
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM ResourceProviderLinkT WHERE ResourceID = %li", nResourceIDToDelete);
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM ResourceUserLinkT WHERE ResourceID = %li", nResourceIDToDelete);
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM ResourceAvailTemplateItemResourceT WHERE ResourceID = %li", nResourceIDToDelete);

	// (a.walling 2010-06-15 15:49) - PLID 39184 - Clear our any resource set details using this resource
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM ResourceSetDetailsT WHERE ResourceID = %d", nResourceIDToDelete);

	// (a.walling 2010-06-16 17:47) - PLID 39184 - Need to ensure that links are cleared out before clearing out the empty resource sets
	AddStatementToSqlBatch(strSqlBatch, 
		"DELETE EmptyAptResourceSetLinksT "
		"FROM AptResourceSetLinksT "
			"EmptyAptResourceSetLinksT "
		"INNER JOIN ResourceSetT "
			"ON EmptyAptResourceSetLinksT.ResourceSetID = ResourceSetT.ID "
		"LEFT JOIN ResourceSetDetailsT "
			"ON ResourceSetT.ID = ResourceSetDetailsT.ResourceSetID "
		"WHERE ResourceSetDetailsT.ResourceSetID IS NULL ");
	AddStatementToSqlBatch(strSqlBatch, 
		"DELETE EmptyResourceSetsT "
		"FROM ResourceSetT "
			"EmptyResourceSetsT "
		"LEFT JOIN ResourceSetDetailsT "
			"ON EmptyResourceSetsT.ID = ResourceSetDetailsT.ResourceSetID "
		"WHERE ResourceSetDetailsT.ResourceSetID IS NULL ");

	// (z.manning 2010-07-01 11:27) - PLID 39422
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM HL7CodeLinkT WHERE PracticeID = %li AND Type = %li", nResourceIDToDelete, hclrtResource);
	
	// (r.gonet 09-19-2010 3:54) - PLID 39282 - Delete any Appointment Reminder Scheduler Filters that have this resource
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM ApptReminderFiltersT WHERE AppointmentResource = %li", nResourceIDToDelete);

	// (z.manning 2014-08-22 09:12) - PLID 63251 - Handle UserLocationResourceExclusionT
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM UserLocationResourceExclusionT WHERE ResourceID = %li", nResourceIDToDelete);

	// (r.goldschmidt 2014-11-07 14:07) - PLID 63835 - Need to clear out in case subsequent creation of new resources reuses the id (edge case, but still)
	AddStatementToSqlBatch(strSqlBatch, "UPDATE AppointmentsT SET RequestedResourceID = NULL WHERE RequestedResourceID = %li", nResourceIDToDelete);

	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM ResourceT WHERE ID = %li", nResourceIDToDelete);
}

// (a.walling 2013-01-21 16:48) - PLID 54745 - Appointment count calculation in scheduler reverted to stop using recordsets and instead use Reservation objects' Data[] property map
//long GetCountOpen(CString strIDs);

// (j.gruber 2012-07-31 12:14) - PLID 51869
CString GetApptInsAuditString(const AppointmentInsuranceMap *pmapInsInfo)
{
	//first we need to loop through the map and get the highest placement
	long nHighPlacement = -1;
	POSITION pos = pmapInsInfo->GetStartPosition();
	InsuranceInfo *pInsInfo;
	long nPlacement = SchedulerInsurancePlacements::priIns;

	while (pos != NULL) {				
		pmapInsInfo->GetNextAssoc(pos, nPlacement, pInsInfo);
		if (nPlacement > nHighPlacement) {
			nHighPlacement = nPlacement;
		}
	}

	CString strAudit;
	//now construct our string in placement order
	for (int i=1; i <=nHighPlacement; i++){
		if (pmapInsInfo->Lookup(i, pInsInfo)){
			strAudit += AsString(i) + ": " + pInsInfo->strInsCoName + ";";
		}
		else {
			//this shouldn't happen since we already checked that there were no prior placements before we let them save.
			//still its not the end of the world, just need to check how we got here.
			ASSERT(FALSE);
		}
	}

	return strAudit;

}

// (j.gruber 2012-08-01 09:20) - PLID 51885
BOOL AddNewInsuredParty(IN CWnd *pParent, IN CNewPatientInsuredParty patientInsInfo, IN long nPatientID, IN CString strPatientName, OUT long &nInsPartyID, OUT CString &strInsName, OUT CString &strInsCategory)
{	
	CNewPatientInsuredParty insParty;
	//copy the information from the patient to the party
	insParty.m_strInsFirst  = patientInsInfo.m_strInsFirst;
	insParty.m_strInsMiddle  = patientInsInfo.m_strInsMiddle;
	insParty.m_strInsLast  = patientInsInfo.m_strInsLast;
	// (j.jones 2012-10-25 09:39) - PLID 36305 - added Title
	insParty.m_strInsTitle  = patientInsInfo.m_strInsTitle;
	insParty.m_strInsAddress1  = patientInsInfo.m_strInsAddress1;
	insParty.m_strInsAddress2  = patientInsInfo.m_strInsAddress2;
	insParty.m_strInsCity  = patientInsInfo.m_strInsCity;
	insParty.m_strInsState  = patientInsInfo.m_strInsState;
	insParty.m_strInsZip  = patientInsInfo.m_strInsZip;
	// (j.jones 2012-11-12 13:32) - PLID 53622 - added m_strInsCountry
	insParty.m_strInsCountry  = patientInsInfo.m_strInsCountry;
	insParty.m_strInsPhone  = patientInsInfo.m_strInsPhone;
	insParty.m_strInsSSN  = patientInsInfo.m_strInsSSN;
	insParty.m_nInsGender  = patientInsInfo.m_nInsGender;
	insParty.m_strInsEmployer  = patientInsInfo.m_strInsEmployer;
	insParty.m_dtInsBirthDate  = patientInsInfo.m_dtInsBirthDate;	

	CNewPatientAddInsuredDlg dlg(insParty, patientInsInfo, pParent, FALSE, nPatientID);
	if (IDOK == dlg.DoModal())
	{		
		//save the insured party		
		if (SaveInsuredParty(nInsPartyID, insParty, nPatientID, strPatientName)) {

			//set our other variables we need
			strInsName = insParty.m_strInsCompanyName;
			strInsCategory = insParty.m_strRespType;

			
			if (insParty.GetRespTypeID() == -1) {
				MsgBox("You have created an Inactive insured party.  It is saved to the patient's account, but you will not be able to select it on this appointment.");
				return FALSE;
			}
			
			return TRUE;
		}
		else {
			return FALSE;
		}
	}

	return FALSE;

}

// (j.gruber 2012-08-01 09:20) - PLID 51885
BOOL SaveInsuredParty(OUT long &nInsPartyID, CNewPatientInsuredParty party, long nPatientID, CString strPatientName)
{
	long nInsRespTypeID, nInsCoPersonID;
	_variant_t varInsPlanID;	

	if (party.m_strRelationToPt != "Self")
	{
		// (j.jones 2012-10-25 09:39) - PLID 36305 - added Title
		// (j.jones 2012-11-12 13:32) - PLID 53622 - added Country
		// (r.goldschmidt 2014-07-24 16:33) - PLID 63111 - added insurance pay group information
		if (!CreateNewInsuredPartyRecord(nPatientID, strPatientName, nInsPartyID, varInsPlanID, nInsRespTypeID,
			nInsCoPersonID, party.GetInsuranceCompanyID(), -1, party.GetRespTypeID(), party.m_strPatientIDNumber, party.m_strGroupNumber,
			party.m_bPerPayGroup, party.m_cyTotalDeductible, party.m_cyTotalOOP, party.m_mapPayGroupVals, party.m_strRelationToPt,
			party.m_strInsFirst, party.m_strInsMiddle, party.m_strInsLast, party.m_strInsTitle,
			party.m_strInsAddress1, party.m_strInsAddress2, party.m_strInsCity, party.m_strInsState, party.m_strInsZip, party.m_strInsCountry, party.m_strInsPhone,
			party.m_strInsEmployer, party.m_nInsGender, party.m_dtInsBirthDate, party.m_strInsSSN)) 
		{
			return FALSE;					
		}
	}
	else {

		//first create the record
		// (r.goldschmidt 2014-07-24 16:33) - PLID 63111 - added insurance pay group information
		if (!CreateNewInsuredPartyRecord(nPatientID, strPatientName, nInsPartyID, varInsPlanID, nInsRespTypeID,
			nInsCoPersonID, party.GetInsuranceCompanyID(), -1, party.GetRespTypeID(), party.m_strPatientIDNumber, party.m_strGroupNumber,
			party.m_bPerPayGroup, party.m_cyTotalDeductible, party.m_cyTotalOOP, party.m_mapPayGroupVals, "Self"))
		{
			return FALSE;
		}
		else {
			//now copy the patient information
			if (!CopyPatientInfoToInsuredParty(nPatientID, nInsPartyID, strPatientName ) )
			{
				return FALSE;
			}
		}
	}
	return TRUE;
}

// (j.jones 2014-11-14 10:58) - PLID 64169 - added a global function to fill an InsuranceInfo map
void SetAppointmentInsuranceMap(AppointmentInsuranceMap &mapInsPlacements, long nPlacement,
	long nInsuredPartyID, CString strInsCoName, CString strRespType, bool bErrorOnExistence /*= false*/)
{
	// (j.jones 2014-11-14 11:07) - PLID 64169 - moved this logic from CResEntryDlg::SetInsurance
	InsuranceInfo *pInsInfo;

	if (mapInsPlacements.Lookup(nPlacement, pInsInfo)) {

		if (bErrorOnExistence) {
			ThrowNxException("Error in SetAppointmentInsuranceMap - Unexpected Insurance Placement Exists");
		}

		//remove it
		mapInsPlacements.RemoveKey(nPlacement);
	}
	else {
		//make a new one
		pInsInfo = new InsuranceInfo();
	}

	ASSERT(pInsInfo);

	//now replace the information
	pInsInfo->nInsuredPartyID = nInsuredPartyID;
	pInsInfo->strInsCoName = strInsCoName;
	pInsInfo->strRespType = strRespType;

	mapInsPlacements.SetAt(nPlacement, pInsInfo);
}

// (j.jones 2014-11-14 13:41) - PLID 64169 - added a global function to clear an InsuranceInfo map
void ClearAppointmentInsuranceMap(AppointmentInsuranceMap &mapInsPlacements)
{
	POSITION pos = mapInsPlacements.GetStartPosition();
	InsuranceInfo *pInsInfo;
	long nPlacement;
	while (pos != NULL) {
		mapInsPlacements.GetNextAssoc(pos, nPlacement, pInsInfo);
		if (pInsInfo) {
			delete pInsInfo;
		}
	}
	mapInsPlacements.RemoveAll();
}

// (j.jones 2014-11-14 13:30) - PLID 64169 - The ability to load the appointment insurance map
// from a patient's insurance is now a global function.
// Returns true if we actually made changes to the insurance map, false if we did not.
bool TryAutoFillAppointmentInsurance(long nPatientID, IN OUT AppointmentInsuranceMap &mapInsPlacements)
{
	//ignore the -25 patient
	if (nPatientID == -25) {
		return false;
	}
	
	//do nothing if the user does not want appt. insurance to auto-fill
	if(GetRemotePropertyInt("AutoFillApptInsurance", 0, 0, GetCurrentUserName(), true) == 0) {
		return false;
	}

	// (j.jones 2014-11-14 13:35) - PLID 64116 - Calculate which primary insured party should be used.
	// If there is also a secondary insurance in the same category, use it as well.

	//find the default category (Medical, Vision, etc.) they want to load from
	RespCategoryType eCategory = (RespCategoryType)GetRemotePropertyInt("AutoFillApptInsurance_DefaultCategory", RespCategoryType::rctMedical, 0, GetCurrentUserName(), true);

	//this query finds all primary and secondary insured parties for the patient,
	//returning the desired category first, other categories afterwards in order of priority
	_RecordsetPtr rsInsurance = CreateParamRecordset("SELECT InsuredPartyT.PersonID AS InsuredPartyID, "
		"RespTypeT.CategoryPlacement, RespTypeT.CategoryType, RespTypeT.TypeName, "
		"InsuranceCoT.Name AS InsCoName "
		"FROM InsuredPartyT "
		"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
		"INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
		"WHERE InsuredPartyT.PatientID = {INT} AND RespTypeT.CategoryPlacement IN (1,2) "
		"ORDER BY (CASE WHEN RespTypeT.CategoryType = {INT} THEN 0 ELSE 1 END), "
		"RespTypeT.Priority, RespTypeT.CategoryPlacement",
		nPatientID, (long)eCategory);

	long nPrimaryInsuredPartyID = -1, nSecondaryInsuredPartyID = -1;
	CString strPrimaryInsCoName, strSecondaryInsCoName;
	CString strPrimaryRespType, strSecondaryRespType;
	RespCategoryType ePrimaryInsuredPartyCategory = RespCategoryType::rctInvalidRespCategory;

	while (!rsInsurance->eof && (nPrimaryInsuredPartyID == -1 || nSecondaryInsuredPartyID == -1)) {

		long nCurInsuredPartyID = VarLong(rsInsurance->Fields->Item["InsuredPartyID"]->Value);
		long nCurPlacement = VarLong(rsInsurance->Fields->Item["CategoryPlacement"]->Value, -1);
		RespCategoryType eCurCategory = (RespCategoryType)VarLong(rsInsurance->Fields->Item["CategoryType"]->Value);
		CString strCurInsCoName = VarString(rsInsurance->Fields->Item["InsCoName"]->Value, "");
		CString strCurRespType = VarString(rsInsurance->Fields->Item["TypeName"]->Value, "");

		//is this a primary placement, and do we have a primary insured party?
		if (nPrimaryInsuredPartyID == -1 && nCurPlacement == 1) {
			//We found a primary insurance. If it's not the category from the preference
			//it would be the first primary from any other category, sorted by priority.
			nPrimaryInsuredPartyID = nCurInsuredPartyID;
			strPrimaryInsCoName = strCurInsCoName;
			strPrimaryRespType = strCurRespType;
			ePrimaryInsuredPartyCategory = eCurCategory;
		}

		//is this a secondary placement for the same category as our primary?
		if (nPrimaryInsuredPartyID != -1 && nCurPlacement == 2
			&& nSecondaryInsuredPartyID == -1
			&& eCurCategory == ePrimaryInsuredPartyCategory) {
			//We found a secondary insurance for the same category as our primary.
			nSecondaryInsuredPartyID = nCurInsuredPartyID;
			strSecondaryInsCoName = strCurInsCoName;
			strSecondaryRespType = strCurRespType;
		}

		rsInsurance->MoveNext();
	}
	rsInsurance->Close();

	//now assign the primary and, if available, secondary insurance
	if (nPrimaryInsuredPartyID != -1) {

		//clear the current map, though it should be empty
		if (mapInsPlacements.GetSize() > 0) {
			//all callers should be clearing this map prior to calling this function
			ASSERT(FALSE);
		}

		ClearAppointmentInsuranceMap(mapInsPlacements);

		SetAppointmentInsuranceMap(mapInsPlacements, priIns, nPrimaryInsuredPartyID, strPrimaryInsCoName, strPrimaryRespType);

		if (nSecondaryInsuredPartyID != -1) {
			SetAppointmentInsuranceMap(mapInsPlacements, secIns, nSecondaryInsuredPartyID, strSecondaryInsCoName, strSecondaryRespType);
		}

		return true;
	}
	else {
		return false;
	}
}


// (s.tullis 2015-07-08 11:17) - PLID 63851 
// Need to check and warn Template Rules when deleting Appointment types and purposes
BOOL CheckWarnTemplateRuleDetails(BOOL bIsApptTypeOrPurpose, long nObjectID)
{
	// these object typeIDs are used when the type/purpose is to be included in the template rule
	long nObjectTypeIN = bIsApptTypeOrPurpose ? 1 : 2;
	// these object typeIDs are used when the type/purpose is to be not included in the template rule
	long nObjectTypeNotIN = bIsApptTypeOrPurpose ? 101 : 102;

	CString strTypeOrPurpose = bIsApptTypeOrPurpose ? "type" : "purpose";

	CString strTemplateRules = GenerateDelimitedListFromRecordsetColumn(CreateParamRecordset(
		R"(Select 'Template Name : ' + TemplateT.Name + '  ' + 'Rule Name : ' + TemplateRuleT.Description AS Name
				FROM TemplateT
				Inner Join TemplateRuleT
				ON TemplateT.ID = TemplateRuleT.TemplateID
				Inner Join TemplateRuleDetailsT
				ON TemplateRuleDetailsT.TemplateRuleID = TemplateRuleT.ID
				WHERE ( TemplateRuleDetailsT.ObjectType = {INT} OR TemplateRuleDetailsT.ObjectType = {INT} )
                       AND TemplateRuleDetailsT.ObjectID = {INT} )"
					   , nObjectTypeIN, nObjectTypeNotIN, nObjectID)
					   , AsVariant("Name"), "", "\r\n");

	if (!strTemplateRules.IsEmpty())
	{
		CString strMessage = FormatString(
			"This %s is referenced by the following template rule(s): \r\n\r\n "
			"%s  \r\n\r\n"
			"This %s will also be removed from the referenced template rule(s). \r\n"
			"Are you sure you want to continue?"
			, strTypeOrPurpose, strTemplateRules, strTypeOrPurpose);
		int nResult = AfxMessageBox(strMessage, MB_YESNO | MB_ICONQUESTION);
		if (nResult == IDNO)
		{
			return FALSE;
		}
	}
	return TRUE;
}
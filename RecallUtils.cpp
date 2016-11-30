// RecallUtils.cpp : implementation file
//

// (j.armen 2012-03-05 09:23) - PLID 48303

#include "stdafx.h"
#include "Practice.h"
#include "RecallUtils.h"
#include "AuditTrail.h"
#include <map>
#include <set>
#include <vector>
#include "NxAPI.h"

using namespace ADODB;

namespace RecallUtils
{
	// (a.walling 2013-11-25 11:44) - PLID 60007 - We don't need to update recalls every time we call SelectRecalls -- split into UpdateRecalls
	void UpdateRecalls(long nUpdatePatID /*=-1*/)
	{
		try
		{
			// (j.armen 2012-03-28 11:21) - PLID 48480 - If we don't have the license, then don't run the update sql statement.
			if(g_pLicense->CheckForLicense(CLicense::lcRecall, CLicense::cflrSilent))
			{
				// (j.armen 2012-03-05 17:48) - PLID 48634 - Create the next recall in a series
				CParamSqlBatch sqlBatch;
				sqlBatch.Declare("SET NOCOUNT ON");
				sqlBatch.Declare("DECLARE @PatID INT");
				sqlBatch.Declare("DECLARE @TempT TABLE (CurrentRecallID INT, PreviousRecallID INT)");
				sqlBatch.Declare("DECLARE @RecallT TABLE (ID INT, PatientID INT, RecallTemplateID INT, RecallStepID INT, SetID INT, SetOrder INT, IsAdvancing BIT)");

				sqlBatch.Add("SET @PatID = {INT}\r\n", nUpdatePatID);

				// (b.cardillo 2013-01-09 16:11) - PLID 54271 - For significant performance improvement don't use dbo.AsDateNoTime()
				// (j.jones 2016-02-29 10:02) - PLID 68348 - use the same locationID, providerID as the recall we're copying from
				sqlBatch.Declare(
					"INSERT INTO RecallT (PatientID, RecallTemplateID, ProviderID, LocationID, RecallStepID, DiagCodeID, EMRID, LabID, AppointmentID, CreatedUserID, RecallDate, SetID, SetOrder)\r\n"
					"OUTPUT inserted.ID, inserted.PatientID, inserted.RecallTemplateID, inserted.RecallStepID, inserted.SetID, inserted.SetOrder, 1 INTO @RecallT\r\n"
					"SELECT SubQ.PatientID, SubQ.RecallTemplateID, SubQ.ProviderID, SubQ.LocationID, NextRecallStep.ID AS NextRecallStepID, DiagCodeID, EMRID, LabID, AppointmentID, CreatedUserID,\r\n"
						"DATEADD(DAY, NextRecallStep.Days, DATEADD(WEEK, NextRecallStep.Weeks, DATEADD(MONTH, NextRecallStep.Months, DATEADD(YEAR, NextRecallStep.Years, CONVERT(DATETIME, FLOOR(CONVERT(FLOAT, AppointmentsT.StartTime))))))),\r\n"
						"SubQ.SetID, CurrentRecall.SetOrder + 1 AS SetOrder\r\n"
					"FROM (\r\n"
						"SELECT RecallT.PatientID, RecallT.RecallTemplateID, ProviderID, LocationID, SetID, MAX(RecallStepT.StepOrder) AS StepOrder, MAX(RecallT.SetOrder) AS SetOrder\r\n"
						"FROM RecallT\r\n"
						"INNER JOIN RecallStepT ON RecallT.RecallStepID = RecallStepT.ID\r\n"
						"AND PatientID = CASE WHEN @PatID = -1 THEN PatientID ELSE @PatID END\r\n"
						"GROUP BY RecallT.PatientID, RecallT.RecallTemplateID, RecallT.ProviderID, RecallT.LocationID, SetID\r\n"
					") SubQ\r\n"
					"INNER JOIN RecallTemplateT ON SubQ.RecallTemplateID = RecallTemplateT.ID\r\n"
					"INNER JOIN RecallStepT CurrentRecallStep ON SubQ.RecallTemplateID = CurrentRecallStep.RecallTemplateID\r\n"
						"AND SubQ.StepOrder = CurrentRecallStep.StepOrder\r\n"
					"INNER JOIN RecallT CurrentRecall ON CurrentRecallStep.ID = CurrentRecall.RecallStepID\r\n"
						"AND SubQ.SetID = CurrentRecall.SetID\r\n"
					"INNER JOIN RecallStepT NextRecallStep ON SubQ.RecallTemplateID = NextRecallStep.RecallTemplateID\r\n"
						"AND (\r\n"
							"SubQ.StepOrder + 1 = NextRecallStep.StepOrder\r\n"
							"OR (\r\n"
								"RecallTemplateT.RepeatLastStep = 1\r\n"
								"AND SubQ.StepOrder = NextRecallStep.StepOrder\r\n"
								"AND SubQ.StepOrder = (SELECT MAX(StepOrder) FROM RecallStepT WHERE RecallTemplateID = SubQ.RecallTemplateID)\r\n"
								"AND CurrentRecall.SetOrder =\r\n"
								"(\r\n"
									"SELECT MAX(SetOrder)\r\n"
									"FROM RecallT\r\n"
									"LEFT JOIN AppointmentsT ON RecallT.RecallAppointmentID = AppointmentsT.ID\r\n"
									"WHERE RecallT.SetID = SubQ.SetID\r\n"
								")\r\n"
							")\r\n"
						")\r\n"
					"INNER JOIN AppointmentsT ON CurrentRecall.RecallAppointmentID = AppointmentsT.ID\r\n"
					"WHERE Discontinued = 0\r\n"
						"AND AppointmentsT.StartTime < GETDATE()\r\n"
						"AND AppointmentsT.Status <> 4\r\n"
						"AND AppointmentsT.ShowState <> 3\r\n");

				// (j.armen 2012-03-19 13:22) - PLID 48972 - Detect if the second to last performed recall is no longer complete.  
				// If so, then delete the most current step in favor of the user completing the last recall.  This handles the case when
				// a user no show's an appointment after the start date.
				sqlBatch.Declare(
					"INSERT INTO @TempT\r\n"
					"	SELECT CurrentRecallT.ID, PreviousRecallT.ID FROM RecallT CurrentRecallT\r\n"
					"	LEFT JOIN (SELECT * FROM RecallT) PreviousRecallT\r\n"
					"		ON CurrentRecallT.SetID = PreviousRecallT.SetID AND CurrentRecallT.SetOrder = (PreviousRecallT.SetOrder + 1)\r\n"
					"	LEFT JOIN AppointmentsT ON PreviousRecallT.RecallAppointmentID = AppointmentsT.ID\r\n"
					"	WHERE CurrentRecallT.SetOrder = (SELECT MAX(SetOrder) FROM RecallT WHERE RecallT.SetID = CurrentRecallT.SetID)\r\n"
					"		AND PreviousRecallT.Discontinued = 0\r\n"
					"		AND (AppointmentsT.ID IS NULL OR AppointmentsT.StartTime > GETDATE() OR AppointmentsT.Status = 4 OR AppointmentsT.ShowState = 3)\r\n"
					"		AND CurrentRecallT.PatientID = CASE WHEN @PatID = -1 THEN CurrentRecallT.PatientID ELSE @PatID END\r\n");

				sqlBatch.Declare(
					"UPDATE Notes\r\n"
					"SET RecallID = PreviousRecallID\r\n"
					"FROM Notes\r\n"
					"INNER JOIN @TempT TempT ON Notes.RecallID = CurrentRecallID\r\n"
					"WHERE RecallID = CurrentRecallID\r\n");

				sqlBatch.Declare(
					"DELETE FROM RecallT\r\n"
					"OUTPUT deleted.ID, deleted.PatientID, deleted.RecallTemplateID, deleted.RecallStepID, deleted.SetID, deleted.SetOrder, 0 INTO @RecallT\r\n"
					"WHERE ID IN (SELECT CurrentRecallID FROM @TempT)");

				sqlBatch.Declare(
					"SET NOCOUNT OFF\r\n"
					"SELECT\r\n"
					"	CASE\r\n"
					"		WHEN IsAdvancing = 1 THEN NewRecallT.ID\r\n"
					"		ELSE					  OldRecallT.ID\r\n"
					"	END AS ID,\r\n"
					"	IsAdvancing, "
					"	NewRecallT.PatientID,\r\n"
					"	CASE\r\n"
					"		WHEN IsAdvancing = 1 THEN NewRecallTemplateT.Name + ': (' + CAST(NewRecallT.SetOrder AS NVARCHAR) + ') ' + NewRecallStepT.Name\r\n"
					"		ELSE					  OldRecallTemplateT.Name + ': (' + CAST(OldRecallT.SetOrder AS NVARCHAR) + ') ' + OldRecallStepT.Name\r\n"
					"	END AS NewValue,\r\n"
					"	CASE\r\n"
					"		WHEN IsAdvancing = 1 THEN OldRecallTemplateT.Name + ': (' + CAST(OldRecallT.SetOrder AS NVARCHAR) + ') ' + OldRecallStepT.Name\r\n"
					"		ELSE					  NewRecallTemplateT.Name + ': (' + CAST(NewRecallT.SetOrder AS NVARCHAR) + ') ' + NewRecallStepT.Name\r\n"
					"	END AS OldValue\r\n"
					"FROM @RecallT NewRecallT\r\n"
					"	INNER JOIN RecallT OldRecallT ON NewRecallT.SetID = OldRecallT.SetID AND (NewRecallT.SetOrder - 1) = OldRecallT.SetOrder\r\n"
					"	INNER JOIN RecallTemplateT NewRecallTemplateT ON NewRecallT.RecallTemplateID = NewRecallTemplateT.ID\r\n"
					"	INNER JOIN RecallTemplateT OldRecallTemplateT ON OldRecallT.RecallTemplateID = OldRecallTemplateT.ID\r\n"
					"	INNER JOIN RecallStepT NewRecallStepT ON NewRecallT.RecallStepID = NewRecallStepT.ID\r\n"
					"	INNER JOIN RecallStepT OldRecallStepT ON OldRecallT.RecallStepID = OldRecallStepT.ID");

				_RecordsetPtr prs = sqlBatch.CreateRecordset(GetRemoteData());

				CAuditTransaction auditTran;
				for(; !prs->eof; prs->MoveNext())
				{
					AuditEvent(
						AdoFldLong(prs, "PatientID"), 
						GetExistingPatientName(AdoFldLong(prs, "PatientID")), 
						auditTran, 
						AdoFldBool(prs, "IsAdvancing") ? aeiPatientRecallAdvanceToNextStep : aeiPatientRecallRevertToPreviousStep, 
						AdoFldLong(prs, "ID"), 
						AdoFldString(prs, "OldValue"), 
						AdoFldString(prs, "NewValue"), 
						aepMedium, 
						aetChanged);
				}
				auditTran.Commit();

				// (z.manning 2015-11-05 12:13) - PLID 57109 - Update recalls needing attention dialog
				GetMainFrame()->HandleRecallChanged();
			}
		}NxCatchAll(__FUNCTION__);
	}

	// (a.walling 2013-12-12 16:51) - PLID 60007 - We don't need to update recalls every time we call SelectRecalls -- split into UpdateRecalls
	CSqlFragment SelectRecalls()
	{
		// (j.armen 2012-03-19 09:10) - PLID 48780 - Added HasNote flag
		// (j.armen 2012-03-20 13:44) - PLID 48913 - Added Group Information that contains the most recent recall's info
		// (j.armen 2012-04-10 17:29) - PLID 49057 - Show cancelled appointments since they are still linked
		// (b.cardillo 2013-01-09 16:11) - PLID 54271 - For significant performance improvement don't use dbo.AsDateNoTime()
		// (j.jones 2016-02-18 11:42) - PLID 68349 - now recalls have their own provider/location, if either is null use the
		// G1 provider, G2 location, do not pay attention to the properties of the linked appt.
		return CSqlFragment(
			"SELECT\r\n"
			"	CASE WHEN RecallT.ID = MasterRecallQ.MasterRecallID THEN RecallT.ID END AS GroupID,\r\n"
			"	CASE WHEN RecallT.ID <> MasterRecallQ.MasterRecallID THEN MasterRecallQ.MasterRecallID END AS ParentGroupID,\r\n"
			"	MasterRecallQ.ParentProviderID,\r\n"
			"	MasterRecallQ.ParentRecallTemplateID,\r\n"
			"	MasterRecallQ.ParentLocationID,\r\n"
			"	MasterRecallQ.ParentRecallStatusID,\r\n"
			"	MasterRecallQ.ParentRecallDate,\r\n"
			"	RecallT.SetID,\r\n"
			"	RecallT.SetOrder,\r\n"
			"	RecallT.ID AS RecallID,\r\n"
			"	RecallT.RecallDate,\r\n"
			"	CASE WHEN NotesQ.RecallID IS NOT NULL THEN 1 ELSE 0 END AS HasNote,\r\n"
			"	RecallT.Discontinued,\r\n"
			"	LocationsT.ID AS LocationID,\r\n"
			"	LocationsT.Name AS LocationName,\r\n"
			"	RecallTemplateT.ID AS RecallTemplateID,\r\n"
			"	RecallTemplateT.Name AS RecallTemplate,\r\n"
			"	RecallStepT.Name AS RecallStep,\r\n"
			"	ProvPersonT.Last + ', ' + ProvPersonT.First + ' ' + ProvPersonT.Middle AS ProviderName,\r\n"
			"	ProvPersonT.ID AS ProviderID,\r\n"
			"	PatPersonT.Last + ', ' + PatPersonT.First + ' ' + PatPersonT.Middle AS PatientName,\r\n"
			"	PatPersonT.ID AS PatientID,\r\n"
			"	CASE\r\n"
			"		WHEN PatientsT.PreferredContact = 1 THEN PatPersonT.HomePhone + ' (Home)'\r\n"
			"		WHEN PatientsT.PreferredContact = 2 THEN PatPersonT.WorkPhone + ' (Work)'\r\n"
			"		WHEN PatientsT.PreferredContact = 3 THEN PatPersonT.CellPhone + ' (Cell)'\r\n"
			"		WHEN PatientsT.PreferredContact = 4 THEN PatPersonT.Pager + ' (Pager)'\r\n"
			"		WHEN PatientsT.PreferredContact = 5 THEN PatPersonT.OtherPhone + ' (Other)'\r\n"
			"		WHEN PatientsT.PreferredContact = 6 THEN PatPersonT.Email\r\n"
			"		WHEN PatientsT.PreferredContact = 7 THEN PatPersonT.CellPhone + ' (Text Message)'\r\n"
			"		ELSE\r\n"
			"			CASE\r\n"
			"				WHEN PatPersonT.HomePhone NOT LIKE '' THEN PatPersonT.HomePhone + ' (Home)'\r\n"
			"				WHEN PatPersonT.WorkPhone NOT LIKE '' THEN PatPersonT.WorkPhone + ' (Work)'\r\n"
			"				WHEN PatPersonT.CellPhone NOT LIKE '' THEN PatPersonT.CellPhone + ' (Cell)'\r\n"
			"				WHEN PatPersonT.Pager NOT LIKE '' THEN PatPersonT.Pager + ' (Pager)'\r\n"
			"				WHEN PatPersonT.OtherPhone NOT LIKE '' THEN PatPersonT.OtherPhone + ' (Other)'\r\n"
			"				WHEN PatPersonT.Email NOT LIKE '' THEN PatPersonT.Email\r\n"
			"			END\r\n"
			"	END AS PreferredContact,\r\n"
			"	CASE\r\n"
			"		WHEN RecallT.Discontinued = 1 THEN {CONST_INT}\r\n"
			"		WHEN RecallT.RecallAppointmentID IS NOT NULL AND AppointmentsT.Status <> 4 AND AppointmentsT.ShowState <> 3 AND GETDATE() >= AppointmentsT.StartTime THEN {CONST_INT}\r\n"
			"		WHEN RecallT.RecallAppointmentID IS NOT NULL AND AppointmentsT.Status <> 4 AND AppointmentsT.ShowState <> 3 AND GETDATE() <  AppointmentsT.StartTime THEN {CONST_INT}\r\n"
			"		WHEN (RecallT.RecallAppointmentID IS NULL OR AppointmentsT.Status = 4 OR AppointmentsT.ShowState = 3) AND CONVERT(DATETIME, FLOOR(CONVERT(FLOAT, GETDATE()))) < CONVERT(DATETIME, FLOOR(CONVERT(FLOAT, RecallT.RecallDate))) THEN {CONST_INT}\r\n"
			"		ELSE {CONST_INT}\r\n"
			"	END AS RecallStatusColor,\r\n"
			"	CASE\r\n"
			"		WHEN RecallT.Discontinued = 1 THEN {CONST_INT}\r\n"
			"		WHEN RecallT.RecallAppointmentID IS NOT NULL AND AppointmentsT.Status <> 4 AND AppointmentsT.ShowState <> 3 AND GETDATE() >= AppointmentsT.StartTime THEN {CONST_INT}\r\n"
			"		WHEN RecallT.RecallAppointmentID IS NOT NULL AND AppointmentsT.Status <> 4 AND AppointmentsT.ShowState <> 3 AND GETDATE() <  AppointmentsT.StartTime THEN {CONST_INT}\r\n"
			"		WHEN (RecallT.RecallAppointmentID IS NULL OR AppointmentsT.Status = 4 OR AppointmentsT.ShowState = 3) AND CONVERT(DATETIME, FLOOR(CONVERT(FLOAT, GETDATE()))) < CONVERT(DATETIME, FLOOR(CONVERT(FLOAT, RecallT.RecallDate))) THEN {CONST_INT}\r\n"
			"		ELSE {CONST_INT}\r\n"
			"	END AS RecallStatusID,\r\n"
			"	CASE\r\n"
			"		WHEN RecallT.Discontinued = 1 THEN {CONST_STRING}\r\n"
			"		WHEN RecallT.RecallAppointmentID IS NOT NULL AND AppointmentsT.Status <> 4 AND AppointmentsT.ShowState <> 3 AND GETDATE() >= AppointmentsT.StartTime THEN {CONST_STRING}\r\n"
			"		WHEN RecallT.RecallAppointmentID IS NOT NULL AND AppointmentsT.Status <> 4 AND AppointmentsT.ShowState <> 3 AND GETDATE() <  AppointmentsT.StartTime THEN {CONST_STRING}\r\n"
			"		WHEN (RecallT.RecallAppointmentID IS NULL OR AppointmentsT.Status = 4 OR AppointmentsT.ShowState = 3) AND CONVERT(DATETIME, FLOOR(CONVERT(FLOAT, GETDATE()))) < CONVERT(DATETIME, FLOOR(CONVERT(FLOAT, RecallT.RecallDate))) THEN {CONST_STRING}\r\n"
			"		ELSE {CONST_STRING}\r\n"
			"	END AS RecallStatus,\r\n"
			"	AppointmentsT.ID AS AppointmentID,\r\n"
			"	AppointmentsT.StartTime AS AppointmentDate,\r\n"
			"	AptTypeT.Name AS AppointmentType\r\n"
			"FROM RecallT\r\n"
			"INNER JOIN RecallTemplateT ON RecallT.RecallTemplateID = RecallTemplateT.ID\r\n"
			"INNER JOIN RecallStepT ON RecallT.RecallStepID = RecallStepT.ID\r\n"
			"INNER JOIN PatientsT ON RecallT.PatientID = PatientsT.PersonID\r\n"
			"INNER JOIN PersonT PatPersonT ON PatientsT.PersonID = PatPersonT.ID\r\n"
			"INNER JOIN (\r\n"
			"	SELECT\r\n"
			"		ChildRecallT.ID AS RecallID,\r\n"
			"		MasterRecallT.ID AS MasterRecallID,\r\n"
			"		ProvPersonT.ID AS ParentProviderID,\r\n"
			"		MasterRecallT.RecallTemplateID AS ParentRecallTemplateID,\r\n"
			"		LocationsT.ID AS ParentLocationID,\r\n"
			"		MasterRecallT.RecallDate AS ParentRecallDate,\r\n"
			"		CASE\r\n"
			"			WHEN MasterRecallT.Discontinued = 1 THEN {CONST_INT}\r\n"
			"			WHEN MasterRecallT.RecallAppointmentID IS NOT NULL AND AppointmentsT.Status <> 4 AND AppointmentsT.ShowState <> 3 AND GETDATE() >= AppointmentsT.StartTime THEN {CONST_INT}\r\n"
			"			WHEN MasterRecallT.RecallAppointmentID IS NOT NULL AND AppointmentsT.Status <> 4 AND AppointmentsT.ShowState <> 3 AND GETDATE() <  AppointmentsT.StartTime THEN {CONST_INT}\r\n"
			"			WHEN (MasterRecallT.RecallAppointmentID IS NULL OR AppointmentsT.Status = 4 OR AppointmentsT.ShowState = 3) AND CONVERT(DATETIME, FLOOR(CONVERT(FLOAT, GETDATE()))) < CONVERT(DATETIME, FLOOR(CONVERT(FLOAT, MasterRecallT.RecallDate))) THEN {CONST_INT}\r\n"
			"			ELSE {CONST_INT}\r\n"
			"		END AS ParentRecallStatusID\r\n"
			"	FROM RecallT MasterRecallT\r\n"
			"	INNER JOIN (SELECT ID, SetID FROM RecallT) ChildRecallT ON MasterRecallT.SetID = ChildRecallT.SetID\r\n"
			"		AND MasterRecallT.SetOrder = ((SELECT MAX(SetOrder) FROM RecallT MaxSetOrderT WHERE MaxSetOrderT.SetID = MasterRecallT.SetID))\r\n"
			"	INNER JOIN PatientsT ON MasterRecallT.PatientID = PatientsT.PersonID\r\n"
			"	INNER JOIN PersonT PatPersonT ON PatientsT.PersonID = PatPersonT.ID\r\n"
			"	LEFT JOIN AppointmentsT ON MasterRecallT.RecallAppointmentID = AppointmentsT.ID\r\n"
			"	LEFT JOIN LocationsT ON IsNull(MasterRecallT.LocationID, PatPersonT.Location) = LocationsT.ID\r\n"
			"	LEFT JOIN PersonT ProvPersonT ON IsNull(MasterRecallT.ProviderID, PatientsT.MainPhysician) = ProvPersonT.ID\r\n"
			"	) MasterRecallQ ON RecallT.ID = MasterRecallQ.RecallID\r\n"
			"LEFT JOIN AppointmentsT ON RecallT.RecallAppointmentID = AppointmentsT.ID\r\n"
			"LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID\r\n"
			"LEFT JOIN PersonT ProvPersonT ON IsNull(RecallT.ProviderID, PatientsT.MainPhysician) = ProvPersonT.ID\r\n"
			"LEFT JOIN LocationsT ON IsNull(RecallT.LocationID, PatPersonT.Location) = LocationsT.ID\r\n"
			"LEFT JOIN (SELECT DISTINCT RecallID FROM Notes) NotesQ ON RecallT.ID = NotesQ.RecallID\r\n",
			eDiscontinuedColor, eCompleteColor, eScheduledColor, eNeedToScheduleColor, ePastDueColor,
			eDiscontinued, eComplete, eScheduled, eNeedToSchedule, ePastDue,
			"'Discontinued'", "'Complete'", "'Scheduled'", "'Need to Schedule'", "'Past Due'",
			eDiscontinued, eComplete, eScheduled, eNeedToSchedule, ePastDue);
	}

	// (j.jones 2016-02-22 09:03) - PLID 68376 - removed obsolete functions that are now in the API

	// (a.walling 2013-12-13 10:20) - PLID 60010 - Creating recalls now in the shared RecallUtils namespace
	// (j.jones 2016-02-17 16:59) - PLID 68348 - recalls now have an optional provider ID and location ID, either can be -1
	void CreateRecalls(long nPatientID, long nEMRID, long nApptID, long nLabID, long nProviderID, long nLocationID,
		CArray<RecallListMap, RecallListMap>& aryRecallListMap)
	{
		//throw exceptions to the caller

		// (j.jones 2016-02-18 16:46) - PLID 68376 - this is now in an API function
		Nx::SafeArray<IUnknown *> saRecalls;

		for (int iRecall = 0; iRecall < aryRecallListMap.GetSize(); iRecall++) {

			long nDiagCodeID = aryRecallListMap.GetAt(iRecall).nDiagCodeID;
			long nRecallTemplateID = aryRecallListMap.GetAt(iRecall).nRecallTemplateID;

			NexTech_Accessor::_CreateRecallInputPtr pRecall(__uuidof(NexTech_Accessor::CreateRecallInput));
			pRecall->patientPersonID = _bstr_t(AsString(nPatientID));
			if (nEMRID != -1) {
				pRecall->emrGroupID = _bstr_t(AsString(nEMRID));
			}
			if (nApptID != -1) {
				pRecall->appointmentID = _bstr_t(AsString(nApptID));
			}
			if (nLabID != -1) {
				pRecall->labID = _bstr_t(AsString(nLabID));
			}

			//if provider or location ID is -1, send it as -1, it means
			//we really want no provider or no location
			pRecall->providerID = _bstr_t(AsString(nProviderID));
			pRecall->locationID = _bstr_t(AsString(nLocationID));

			if (nDiagCodeID != -1) {
				pRecall->diagCodeID = _bstr_t(AsString(nDiagCodeID));
			}
			pRecall->recallTemplateID = _bstr_t(AsString(nRecallTemplateID));

			saRecalls.Add(pRecall);
		}

		if (saRecalls.GetSize() > 0) {
			NexTech_Accessor::_CreateRecallsInputPtr pRecalls(__uuidof(NexTech_Accessor::CreateRecallsInput));
			pRecalls->newRecalls = saRecalls;
			GetAPI()->CreatePatientRecalls(GetAPISubkey(), GetAPILoginToken(), pRecalls);

			// (z.manning 2015-11-05 12:13) - PLID 57109 - Update recalls needing attention dialog
			GetMainFrame()->HandleRecallChanged();
		}
	}

	// (a.walling 2013-12-13 10:20) - PLID 60010 - Gets any templates/diagcode pairs that can be automatically created for an EMR
	// (r.gonet 04/13/2014) - PLID 60870 - Ripped out the entire guts of this function and replaced logic being done in SQL with logic done primarily in C++
	// Effectively does the same thing except now accounts for ICD-10 codes.
	// (j.jones 2016-02-18 08:41) - PLID 68390 - this now also loads the providerID and locationID
	// (j.jones 2016-02-19 11:03) - PLID 68378 - renamed and added a bool to control auto-creating recalls
	void CalculateRecallsForEMR(bool bAutoCreate, long nEMRGroupID, OUT long &nProviderID, OUT long &nLocationID, CArray<RecallListMap, RecallListMap>& aryRecallListMap)
	{
		nProviderID = -1;
		nLocationID = -1;
		aryRecallListMap.RemoveAll();

		//if no EMRGroupID is given, return now
		if (nEMRGroupID == -1) {
			return;
		}

		// (j.jones 2016-02-19 11:00) - PLID 68378 - moved all of this logic to the API		
		NexTech_Accessor::_CalculateEMRRecallsInputPtr pInput(__uuidof(NexTech_Accessor::CalculateEMRRecallsInput));
		pInput->emrGroupID = _bstr_t(AsString(nEMRGroupID));
		pInput->autoCreateRecalls = bAutoCreate;
		NexTech_Accessor::_EMRRecallInfoPtr pResult = GetAPI()->CalculateRecallsForEMR(GetAPISubkey(), GetAPILoginToken(), pInput);

		if (pResult != NULL) {
						
			// (j.jones 2016-02-19 11:48) - PLID 68390 - get the provider and location,
			// they may have been calculated even if we found no recall templates
			CString strProviderID = (LPCTSTR)pResult->providerID;
			if (strProviderID != "") {
				nProviderID = atoi(strProviderID);
			}

			CString strLocationID = (LPCTSTR)pResult->locationID;
			if (strLocationID != "") {
				nLocationID = atoi(strLocationID);
			}

			//see if we have any recall templates
			if (pResult->recallDetails != NULL) {

				Nx::SafeArray<IUnknown *> saRecalls = pResult->recallDetails;

				if (saRecalls.GetCount() > 0) {
					
					//fill the map
					for each(NexTech_Accessor::_EMRRecallTemplateDetailPtr pRecall in saRecalls) {
						long nDiagCodeID = AsLong((LPCTSTR)pRecall->diagCodeID);
						long nRecallTemplateID = AsLong((LPCTSTR)pRecall->recallTemplateID);
						aryRecallListMap.Add(RecallListMap(nDiagCodeID, nRecallTemplateID));
					}

					// (j.jones 2016-02-19 11:06) - PLID 68378 - we may have already created the recalls,
					// which we know happened if we have results, and bAutoCreate was true
					if (bAutoCreate) {
						// (z.manning 2015-11-05 12:13) - PLID 57109 - Update recalls needing attention dialog
						GetMainFrame()->HandleRecallChanged();
					}
				}
			}
		}
	}

	//(a.wilson 2012-3-5) PLID 48485 - returns the max status id for showing the worst value in the patients recalls.
	eRecallStatusColors GeneratePatientRecallStatusTextColor(const long& nPatientID)
	{
		// (j.armen 2012-03-28 10:54) - PLID 48480 - If we don't have the recall license, just return None, which is black
		if(!g_pLicense->CheckForLicense(CLicense::lcRecall, CLicense::cflrSilent))
			return eNoneColor;

		// (a.walling 2013-12-12 16:51) - PLID 60009 - (see 59999) RecallStatusTextColor only needs a subset of the full recalls query
		ADODB::_RecordsetPtr rsRecallStatus = CreateParamRecordset(GetRemoteDataSnapshot(), 
			"SELECT "
				"MAX( "
					"CASE "
						"WHEN RecallT.Discontinued = 1 THEN 1 "
						"WHEN RecallT.RecallAppointmentID IS NOT NULL AND AppointmentsT.Status <> 4 AND AppointmentsT.ShowState <> 3 AND GETDATE() >= AppointmentsT.StartTime THEN 0 "
						"WHEN RecallT.RecallAppointmentID IS NOT NULL AND AppointmentsT.Status <> 4 AND AppointmentsT.ShowState <> 3 AND GETDATE() <  AppointmentsT.StartTime THEN 3 "
						"WHEN (RecallT.RecallAppointmentID IS NULL OR AppointmentsT.Status = 4 OR AppointmentsT.ShowState = 3) AND dbo.AsDateNoTime(GETDATE()) < dbo.AsDateNoTime(RecallT.RecallDate) THEN 4 "
						"ELSE 5 "
					"END "
				") AS RecallStatusID "
			"FROM RecallT "
			"LEFT JOIN AppointmentsT "
				"ON RecallT.RecallAppointmentID = AppointmentsT.ID "
			"WHERE RecallT.PatientID = {INT} "
			, nPatientID
		);

		if (!rsRecallStatus->eof) {
			//only need specific colors for best quality text buttons.
			switch(eRecallStatusID(AdoFldLong(rsRecallStatus, "RecallStatusID", eNone))) {
				case eNeedToSchedule:
				case ePastDue:
					return eNeedToScheduleTextColor;
					break;
				case eScheduled:
				case eComplete:
					return eScheduledTextColor;
					break;
				default:
					return eNoneColor;
					break;
			}
		}
		return eNoneColor;
	}

	// (j.jones 2016-02-18 10:53) - PLID 68350 - checks to see if a recall and appt. share the same provider/location,
	// warns if they do not, asks the user to continue.
	// Return value is true if the link is ok to create.
	bool CanLinkRecallAndApptByProviderLocation(CWnd *pParent, long nRecallID, long nAppointmentID, OUT bool &bWasWarned)
	{
		bWasWarned = false;

		// warn if the recall already has a provider or location that is different from the
		// appointment's provider/location, using G1/G1 prov/loc of the recall doesn't have one

		// this won't complain about provider mismatches if the appt. does not have any providers
		// tied to its resources
		ADODB::_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 "
			"RecProvPersonT.FullName AS RecallProvider, "
			"RecLocationT.Name AS RecallLocation, "
			"dbo.GetResourceString(AppointmentsT.ID) AS ApptResources, "
			"ApptLocationT.Name AS ApptLocation "
			"FROM RecallT "
			"INNER JOIN PatientsT ON RecallT.PatientID = PatientsT.PersonID "
			"INNER JOIN PersonT PatPersonT ON PatientsT.PersonID = PatPersonT.ID "
			"LEFT JOIN PersonT RecProvPersonT ON IsNull(RecallT.ProviderID, PatientsT.MainPhysician) = RecProvPersonT.ID "
			"LEFT JOIN LocationsT RecLocationT ON IsNull(RecallT.LocationID, PatPersonT.Location) = RecLocationT.ID "
			"CROSS JOIN AppointmentsT "
			"LEFT JOIN LocationsT ApptLocationT ON AppointmentsT.LocationID = ApptLocationT.ID "
			"WHERE RecallT.ID = {INT} AND AppointmentsT.ID = {INT} "
			"AND ("
			"	(RecLocationT.ID Is Not Null AND ApptLocationT.ID Is Not Null AND RecLocationT.ID <> ApptLocationT.ID) "
			"	OR (RecProvPersonT.ID Is Not Null AND NOT EXISTS "
			//don't warn if one of the appt. resources is our recall provider
			"		(SELECT ResourceProviderLinkT.ProviderID "
			"		FROM AppointmentsT "
			"		INNER JOIN AppointmentResourceT ON AppointmentsT.ID = AppointmentResourceT.AppointmentID "
			"		INNER JOIN ResourceProviderLinkT ON AppointmentResourceT.ResourceID = ResourceProviderLinkT.ResourceID "
			"		WHERE ResourceProviderLinkT.ProviderID = RecProvPersonT.ID "
			"		AND AppointmentsT.ID = {INT} "
			"		) "
			//don't warn if none of the appt. resources are linked to providers at all
			"		AND EXISTS (SELECT ResourceProviderLinkT.ProviderID "
			"		FROM AppointmentsT "
			"		INNER JOIN AppointmentResourceT ON AppointmentsT.ID = AppointmentResourceT.AppointmentID "
			"		INNER JOIN ResourceProviderLinkT ON AppointmentResourceT.ResourceID = ResourceProviderLinkT.ResourceID "
			"		AND AppointmentsT.ID = {INT} "
			"		) "
			"	) "
			") ",
			nRecallID, nAppointmentID, nAppointmentID, nAppointmentID);
		if (!rs->eof) {
			CString strRecallProvider = AdoFldString(rs, "RecallProvider", "<No Provider>");
			CString strRecallLocation = AdoFldString(rs, "RecallLocation", "<No Location>");
			CString strApptResources = AdoFldString(rs, "ApptResources", "<No Resources>");
			CString strApptLocation = AdoFldString(rs, "ApptLocation", "<No Location>");

			CString strWarning;
			strWarning.Format("This recall is for provider %s and location %s, "
				"but the appointment you are linking to is for the resource %s and location %s.\n\n"
				"Are you sure you wish to link this recall to this appointment?",
				strRecallProvider, strRecallLocation,
				strApptResources, strApptLocation);
			if (IDNO == MessageBox(pParent == NULL ? GetActiveWindow() : pParent->GetSafeHwnd(), strWarning, "Practice", MB_ICONWARNING | MB_YESNO)) {
				return false;
			}

			//tell the caller that the user was prompted, so we can skip successive prompts
			bWasWarned = true;
		}
		rs->Close();

		//if we get here, either the provider/locations match, or they approved the link anyways
		return true;
	}
};
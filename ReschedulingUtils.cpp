#include "StdAfx.h"

#include "ReschedulingUtils.h"

#include "TemplateLineItemInfo.h"

#include "GlobalSchedUtils.h"

#include "ReschedulingCancelAppt.h"

#include "NxTaskDialog.h"

#include "AuditTrail.h"


#include "SchedulerView.h"
#include "NxSchedulerDlg.h"

namespace Nx
{
	namespace Scheduler
	{

		CSqlFragment MakeFromClause(const RescheduleAppointmentsInfo& info)
		{
			CSqlFragment sqlResource;
			if (!info.resources.empty()) {
				sqlResource.Create("AND AppointmentResourceT.ResourceID IN ({INTVECTOR})", info.resources);
			}

			// past dates are now allowed
			return CSqlFragment(
R"(AppointmentsT
INNER JOIN AppointmentResourceT
	ON AppointmentsT.ID = AppointmentResourceT.AppointmentID
INNER JOIN ResourceT
	ON AppointmentResourceT.ResourceID = ResourceT.ID
WHERE Status <> 4
AND StartTime >= {OLEDATETIME}
AND StartTime < {OLEDATETIME}
AND 
(
	dbo.AsTimeNoDate(EndTime) > {OLEDATETIME}
	AND
	dbo.AsTimeNoDate(StartTime) < {OLEDATETIME}
)
{SQL})"
				, info.from
				, info.to + COleDateTimeSpan(1, 0, 0, 0) // since the range is exclusive at to, [from, to), add a day
				, g_cdtSqlZero + info.offsetFrom
				, g_cdtSqlZero + info.offsetTo
				, sqlResource
			);
		}

		CSqlFragment MakeFromClauseWithExclusions(const RescheduleAppointmentsInfo& info)
		{
			auto sql = MakeFromClause(info);
			if (info.excludedAppts.empty()) {
				return sql;
			}
			else {
				return CSqlFragment("{SQL} AND AppointmentsT.ID NOT IN ({INTVECTOR})", sql, info.excludedAppts);
			}
		}

		// (a.walling 2015-01-07 09:43) - PLID 64380 - Rescheduling Queue - warning when rescheduling appointments
		/*
			You have selected to reschedule the appointments for <<Resource, Resource>> on <<start date – end date >> <<if there is a time range selected display ‘from start time – end time on each day’>>.
			Resource 1 - # of appointments
			Resource 2 - # of appointments, etc.
			Are you sure you wish to do this?
			Yes / No
		*/
		RescheduleAppointmentsInfo ConfirmRescheduleAppointments(CWnd* pWnd, RescheduleAppointmentsInfo info)
		{
			auto pCon = GetRemoteDataSnapshot();
			
			// There was some confusion about the counts
			// but note this is count per resource, which may be > total # of appts if multiple resources for one appt

			ADODB::_RecordsetPtr prs = CreateParamRecordset(pCon,
				R"(
SELECT COUNT(DISTINCT AppointmentsT.ID) AS TotalAppts FROM {SQL};
SELECT MAX(ResourceT.Item) AS Resource, COUNT(*) AS NumAppts
FROM {SQL}
GROUP BY ResourceT.ID
ORDER BY MAX(ResourceT.Item)
				)"
				, MakeFromClause(info)
				, MakeFromClause(info)
			);

			CString range;
			if (info.from == info.to) {
				range.Format("on %s", FormatDateTimeForInterface(info.from, 0, dtoDate));
			}
			else {
				range.Format("from %s to %s", FormatDateTimeForInterface(info.from, 0, dtoDate), FormatDateTimeForInterface(info.to, 0, dtoDate));
			}

			if (!info.IsWholeDay()) {
				range.AppendFormat(" from %s to %s on each day", FormatDateTimeForInterface(COleDateTime() + info.offsetFrom, DTF_STRIP_SECONDS, dtoTime), FormatDateTimeForInterface(COleDateTime() + info.offsetTo, DTF_STRIP_SECONDS, dtoTime));
			}

			CString resList;
			CString apptList;
			long apptCount = AdoFldLong(prs, "TotalAppts");
			prs = prs->NextRecordset(NULL);

			for (; !prs->eof; prs->MoveNext()) {
				auto res = AdoFldString(prs, "Resource");

				resList.AppendFormat("%s, ", res);
				apptList.AppendFormat("%s - %li appointments.\r\n", res, AdoFldLong(prs, "NumAppts"));
			}
			resList.TrimRight(", ");
			apptList.TrimRight("\r\n");
			
			// Should we allow them to continue even if no appointments? it could add the template block, at least
			// For now, no.
			if (apptList.IsEmpty()) {
				pWnd->MessageBox(FormatString("There are no appointments to reschedule for these resources %s!", range), nullptr, MB_ICONSTOP);
				return{};
			}

			if (info.resources.empty()) {
				resList = "all resources";
			}

			auto externalResourceAppts = GetAppointmentsWithUnselectedResources(info);

			CString unselectedInfo = "";
			if (!externalResourceAppts.empty()) {
				unselectedInfo = FormatString(" Of these, %li appointments have other resources that are not selected.", externalResourceAppts.size());
			}
			
			auto prompt = FormatString(
				"You have selected to reschedule the appointments for %s %s.%s\r\n\r\n%s"
				, resList
				, range
				, unselectedInfo
				, apptList
			);

			enum {
				eContinue = 100
				, eReview
			};
			
			NxTaskDialog dlg;
			dlg.Config()
				.WarningIcon()
				.CancelOnly()
				.MainInstructionText(FormatString("Rescheduling %li appointments.", apptCount))
				.ContentText(prompt)				
				.DefaultButton(IDCANCEL);

			if (externalResourceAppts.empty()) {
				dlg.Config()
					.AddCommand(eContinue, "Continue\nCancel appointments and reschedule.")
					.AddCommand(eReview, "Review\nList all appointments before rescheduling.");
			}
			else {
				dlg.Config()
					.AddCommand(eReview, "Review\nList all appointments with unselected resources before rescheduling.");
			}
			int ret = dlg.DoModal();

			if (ret == IDCANCEL) {
				return{};
			}
			
			if (!externalResourceAppts.empty() || (ret == eReview)) {
				CReschedulingCancelAppt dlg(pWnd);
				if (externalResourceAppts.empty() && (ret == eReview)) {
					dlg.ShowAll();
				}
				dlg.SetAppointmentListFromClause(info);
				if (IDOK != dlg.DoModal()) {
					return{};
				}

				info.excludedAppts = dlg.GetAppointmentIDsNotCancelled();
			}

			return info;
		}

		boost::container::flat_set<long> GetAppointmentsWithUnselectedResources(const RescheduleAppointmentsInfo& info)
		{			
			boost::container::flat_set<long> appts;

			if (info.resources.empty()) {
				return appts;
			}

			auto pCon = GetRemoteDataSnapshot();
						
			ADODB::_RecordsetPtr prs = CreateParamRecordset(pCon, 
				R"(
;WITH TargetAppts AS
(
	SELECT AppointmentResourceT.AppointmentID
	FROM {SQL}
)
SELECT TargetAppts.AppointmentID FROM TargetAppts
INNER JOIN AppointmentResourceT
	ON TargetAppts.AppointmentID = AppointmentResourceT.AppointmentID
WHERE AppointmentResourceT.ResourceID NOT IN ({INTVECTOR});
				)"
				, MakeFromClause(info)
				, info.resources
			);

			for (; !prs->eof; prs->MoveNext()) {
				appts.insert(AdoFldLong(prs, "AppointmentID"));
			}

			return appts;
		}

		// (a.walling 2015-01-07 16:00) - PLID 64383 - Rescheduling Queue - If an appointment being rescheduled has multiple resources listed on the appointment, and those resources aren’t selected when rescheduling, prompt the user
		bool HasUnselectedResources(const RescheduleAppointmentsInfo& info)
		{
			return !GetAppointmentsWithUnselectedResources(info).empty();
		}

		// (a.walling 2015-01-08 11:14) - PLID 64381 - Creates a template line item for the rescheduling range
		static void CreateTemplateLineItem(const RescheduleAppointmentsInfo& info)
		{
			if (-1 == info.templateID) {
				return;
			}

			CTemplateLineItemInfo lineItem;

			// if we want to match current behavior, this is 127, but really i think it should be 0.
			// (a.walling 2015-02-19 10:54) - PLID 64540 - Scheduler template line items does not always need to create DayOfWeek entries in TemplateDetailsT
			lineItem.m_nInclude = 0;

			lineItem.m_embBy = mbNone;

			lineItem.m_nTemplateID = info.templateID;

			lineItem.m_dtStartDate = info.from;
			lineItem.m_dtEndDate = info.to;

			if (info.IsWholeDay()) {
				lineItem.m_dtStartTime = COleDateTime();
				lineItem.m_dtEndTime = COleDateTime() + COleDateTimeSpan(0, 23, 59, 00);
			}
			else {
				lineItem.m_dtStartTime = COleDateTime() + info.offsetFrom;
				lineItem.m_dtEndTime = COleDateTime() + info.offsetTo;
			}

			if (!info.resources.empty()) {
				lineItem.m_bAllResources = false;
				for (auto id : info.resources) {
					// name is unnecessary for our purposes now
					lineItem.AddResource(id, "");
				}
			}

			ExecuteSqlStd(lineItem.GenerateSaveString());
		}

		static std::vector<long> GatherAppointmentIDsOrderByStartTime(const RescheduleAppointmentsInfo& info)
		{
			std::vector<long> appts;
			
			ADODB::_RecordsetPtr prs = CreateParamRecordset(GetRemoteDataSnapshot(), 
				R"(
SELECT AppointmentsT.ID
FROM {SQL}
GROUP BY AppointmentsT.StartTime, AppointmentsT.ID
ORDER BY AppointmentsT.StartTime
				)"
				, MakeFromClauseWithExclusions(info)
			);

			for (; !prs->eof; prs->MoveNext()) {
				appts.push_back(AdoFldLong(prs, "ID"));
			}

			return appts;
		}

		// (a.walling 2015-01-08 09:24) - PLID 64381 - Cancel multiple appointments for rescheduling
		bool RescheduleAppointments(CWnd* pWnd, const RescheduleAppointmentsInfo& info)
		{
			try {
				CreateTemplateLineItem(info);

				auto appts = GatherAppointmentIDsOrderByStartTime(info);


				long cancelCount = 0;
				for (auto id : appts) {
					// (a.walling 2015-01-30 08:23) - PLID 64542 - bUsePromptPrefix will prefix any prompts with the appointment and patient info
					if (!AppointmentCancel(id, true, false, false, Reason(info.cancelReasonID), false, nullptr, nullptr, false, false, true, true)) {
						if (IDOK != pWnd->MessageBox("An appointment was not cancelled for rescheduling. Do you want to continue?", nullptr, MB_OKCANCEL)) {
							return false;
						}
					}
					else {
						++cancelCount;
					}
				}

				if (auto* pView = GetMainFrame()->GetOpenView(SCHEDULER_MODULE_NAME)) {
					//TES 4/13/2015 - PLID 65184 - Just calling UpdateView() won't display any changed templates. Use the behavior from CMainFrame::OpenTemplateEditor().
					CNxSchedulerDlg *dlgScheduler = (CNxSchedulerDlg*)pView->GetActiveSheet();
					dlgScheduler->UpdateBlocks(true, true);
					CWnd::FromHandle((HWND)(dlgScheduler->m_pSingleDayCtrl.GethWnd()))->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
					dlgScheduler->m_pSingleDayCtrl.Refresh();
					if (dlgScheduler->m_pEventCtrl) {
						CWnd::FromHandle((HWND)(dlgScheduler->m_pEventCtrl.GethWnd()))->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
						dlgScheduler->m_pEventCtrl.Refresh();
					}
				}

				pWnd->MessageBox(FormatString("Successfully cancelled %li appointments.", cancelCount), nullptr, MB_OK);

			} NxCatchAll(__FUNCTION__);

			return false;
		}

		// (a.walling 2015-01-29 13:39) - PLID 64586 - Rescheduling Queue - Refresh, table checkers
		namespace TableChecker {
			void OnReschedulingQueueAdded(long apptID)
			{
				CTableCheckerDetails details;
				details.AddXml(FormatString("<tc><add id=\"%li\"/></tc>", apptID));
				CClient::RefreshTable(NetUtils::ReschedulingQueueT, apptID, &details);
			}

			void OnReschedulingQueueRemoved(long apptID)
			{
				CTableCheckerDetails details;
				details.AddXml(FormatString("<tc><del id=\"%li\"/></tc>", apptID));
				CClient::RefreshTable(NetUtils::ReschedulingQueueT, apptID, &details);
			}
		}
		
		// (a.walling 2014-12-22 10:24) - PLID 64364 - Rescheduling Queue
		// (r.goldschmidt 2015-01-15 16:07) - PLID 64386 - added auditing
		void AddToReschedulingQueue(long apptID, long nPatientID)
		{
			long nAffected = 0;
			ExecuteParamSql(GetRemoteData(), &nAffected,
				R"(
SET NOCOUNT OFF;
IF NOT EXISTS (SELECT NULL FROM ReschedulingQueueT WHERE AppointmentID = {INT})
	INSERT INTO ReschedulingQueueT(AppointmentID) VALUES({INT});)"
				, apptID
				, apptID
				);

			// audit if it was not already in the rescheduling queue, and therefore just got added to the queue
			if (nAffected){
				long nAuditID = BeginNewAuditEvent();
				AuditEvent(nPatientID == -25 ? -1 : nPatientID, GetExistingPatientName(nPatientID), nAuditID, aeiApptReschedulingQueueAdd, apptID, "", "", aetChanged);

				// (a.walling 2015-01-29 13:39) - PLID 64586 - Rescheduling Queue - Refresh, table checkers
				TableChecker::OnReschedulingQueueAdded(apptID);
			}
		}

		void RemoveFromReschedulingQueue(long apptID)
		{
			long nAffected = 0;
			ExecuteParamSql(GetRemoteData(), &nAffected, "SET NOCOUNT OFF; DELETE FROM ReschedulingQueueT WHERE AppointmentID = {INT}"
				, apptID
			);

			if (nAffected) {
				// (a.walling 2015-01-29 13:39) - PLID 64586 - Rescheduling Queue - Refresh, table checkers
				TableChecker::OnReschedulingQueueRemoved(apptID);
			}
		}

		// (b.spivey, February 6, 2015) - PLID 64395 - This has the potential to fail, lets handle that and tell the calling code that it failed.
		bool RemoveFromReschedulingQueueWithNote(long nApptID, long nPersonID, long nUserID, CString strPurpose, CString strResource, CString strUserName, COleDateTime dtApptDate)
		{
			if (!ReturnsRecordsParam("SELECT TOP 1 ID FROM AppointmentsT WHERE ID = {INT}", nApptID)) {
				return false; 
			}
			// (b.spivey, February 2, 2015) - PLID 64395 - Fixed note creation to account for empty strings. 
			CString strNote;
			strNote.Format("%s - RE: Appointment on %s-%s%s for %s\r\n\r\nRemoved from rescheduling queue.",
				strUserName,
				FormatDateTimeForInterface(dtApptDate, NULL, dtoDate),
				FormatDateTimeForInterface(dtApptDate, DTF_STRIP_SECONDS, dtoTime),
				strPurpose.IsEmpty() ? "" : " for " + strPurpose,
				strResource
				);

			// (b.spivey, February 4 2015) - PLID 64395 - Use the notes default category preference, which is already cached in mainframe. 
			long nCatID = GetRemotePropertyInt("ApptNotesDefaultCategory", NULL, 0, "<None>", TRUE);
			_variant_t vtCatID = g_cvarNull;

			if (nCatID > 0) {
				vtCatID = nCatID; 
			} 

			// (b.spivey, February 9th, 2015) PLID 64395 - insert null into the DB. 
			ExecuteParamSql(
				R"(
					DECLARE @newNoteID INT;
					DECLARE @apptID INT;

					SET @apptID = {INT}

					DELETE FROM ReschedulingQueueT WHERE AppointmentID = @apptID

					INSERT INTO Notes (PersonID, Date, UserID, Note, Category) VALUES 
					({INT}, GetDate(), {INT}, {STRING}, {VT_I4})
			
					SET @newNoteID = SCOPE_IDENTITY()
					INSERT INTO NoteInfoT (NoteID, AppointmentID) VALUES (@newNoteID, @apptID)
				)", nApptID, nPersonID, nUserID, strNote, vtCatID);
			
			
			// (a.walling 2015-01-29 13:39) - PLID 64586 - Rescheduling Queue - Refresh, table checkers
			TableChecker::OnReschedulingQueueRemoved(nApptID);

			return true; 
		}

		
		// (a.walling 2015-01-26 14:46) - PLID 64687
		long GetFirstAlphabeticalResource(long nApptID)
		{			
			auto prs = CreateParamRecordset(GetRemoteDataSnapshot(),
	R"(SELECT TOP 1
		AppointmentResourceT.ResourceID
	FROM AppointmentsT
	INNER JOIN AppointmentResourceT
		ON AppointmentsT.ID = AppointmentResourceT.AppointmentID
	INNER JOIN ResourceT
		ON AppointmentResourceT.ResourceID = ResourceT.ID
	WHERE AppointmentsT.ID = {INT}
	ORDER BY ResourceT.Item
	)", nApptID);

			if (prs->eof) {
				return -1;
			}

			long nResourceID = AdoFldLong(prs, "ResourceID");

			return nResourceID;
		}

		// (a.walling 2015-01-26 14:46) - PLID 64687
		void OpenResourceInDayViewForToday(long nResourceID)
		{
			try {
				PracticeModulePtr module(g_Modules[Modules::Scheduler]);
				if (module) {
					module->ActivateTab(SchedulerModule::DayTab);
				}
				if (GetMainFrame()->FlipToModule(SCHEDULER_MODULE_NAME)) {
					if (auto* pSchedulerView = dynamic_cast<CSchedulerView*>(GetMainFrame()->GetActiveView())) {						
						bool refresh = false;

						// (a.walling 2015-01-26 14:56) - PLID 64688 - Ensure this is setting the dates and resources appropriately
						COleDateTime dtNow = AsDateNoTime(COleDateTime::GetCurrentTime());
						if (!pSchedulerView->IsDateInCurView(dtNow)) {
							if (auto pSheet = (CNxSchedulerDlg*)pSchedulerView->GetActiveSheet()) {								
								pSheet->SetActiveDate(dtNow);
								refresh = true;
							}
						}
						if (pSchedulerView->GetActiveResourceID() != nResourceID) {
							if (pSchedulerView->IsResourceInCurView(nResourceID)) {								
								pSchedulerView->SetActiveResourceID(nResourceID, FALSE);
								refresh = true;
							}
							else {
								AfxMessageBox("The current view does not contain this appointment's resource!");
							}
						}

						if (refresh) {
							pSchedulerView->UpdateView(true);
						}
					}
				}
			} NxCatchAllThrow(__FUNCTION__);
		}

		// (a.walling 2015-01-27 12:14) - PLID 64416
		std::vector<long> GetAppointmentsInReschedulingQueueForPatient(long nPersonID)
		{
			try {
				std::vector<long> appts;

				auto prs = CreateParamRecordset(
R"(SELECT
	ReschedulingQueueT.AppointmentID
FROM ReschedulingQueueT
INNER JOIN AppointmentsT
	ON ReschedulingQueueT.AppointmentID = AppointmentsT.ID
WHERE AppointmentsT.PatientID = {INT}
ORDER BY AppointmentsT.StartTime)"
					, nPersonID
				);

				for (; !prs->eof; prs->MoveNext()) {
					appts.push_back(AdoFldLong(prs, "AppointmentID"));
				}

				return appts;

			} NxCatchAllThrow(__FUNCTION__);
		}
		
		// (a.walling 2015-01-27 12:14) - PLID 64416
		bool IsPatientInReschedulingQueue(long nPersonID)
		{
			try {
				return !!ReturnsRecordsParam(GetRemoteDataSnapshot(), 
R"(SELECT
	NULL
FROM ReschedulingQueueT
INNER JOIN AppointmentsT
	ON ReschedulingQueueT.AppointmentID = AppointmentsT.ID
WHERE AppointmentsT.PatientID = {INT})"
					, nPersonID
				);
			} NxCatchAllThrow(__FUNCTION__);
		}

		// (a.walling 2015-02-04 09:12) - PLID 64412
		bool IsApptInReschedulingQueue(long apptID)
		{
			try {
				return !!ReturnsRecordsParam(GetRemoteDataSnapshot(), 
R"(SELECT
	NULL
FROM ReschedulingQueueT
WHERE ReschedulingQueueT.AppointmentID = {INT})"
					, apptID
				);
			} NxCatchAllThrow(__FUNCTION__);
		}
	}
}


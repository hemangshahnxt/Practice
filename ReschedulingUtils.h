#pragma once

namespace Nx
{
	namespace Scheduler
	{
		struct RescheduleAppointmentsInfo
		{
			// [from, to)
			COleDateTime from = g_cdtNull;
			COleDateTime to = g_cdtNull;

			// [offsetFrom, offsetTo)
			// StartTime < offsetTo && EndTime > offsetFrom
			COleDateTimeSpan offsetFrom;
			COleDateTimeSpan offsetTo = 1.0; // one day or 24hrs

			std::vector<long> resources;

			std::vector<long> excludedAppts;

			long cancelReasonID = -1;
			long templateID = -1;

			explicit operator bool() const
			{
				return from != g_cdtNull && to != g_cdtNull;
			}

			bool IsWholeDay() const
			{
				return offsetFrom == COleDateTimeSpan() && offsetTo == COleDateTimeSpan(1.0);
			}
		};

		CSqlFragment MakeFromClause(const RescheduleAppointmentsInfo& info);
		CSqlFragment MakeFromClauseWithExclusions(const RescheduleAppointmentsInfo& info);

		///

		bool HasUnselectedResources(const RescheduleAppointmentsInfo& info);
		boost::container::flat_set<long> GetAppointmentsWithUnselectedResources(const RescheduleAppointmentsInfo& info);

		///

		// may add exclusions to returned info
		RescheduleAppointmentsInfo ConfirmRescheduleAppointments(CWnd* pWnd, RescheduleAppointmentsInfo info);

		// (a.walling 2015-01-08 09:24) - PLID 64381 - Cancel multiple appointments for rescheduling
		bool RescheduleAppointments(CWnd* pWnd, const RescheduleAppointmentsInfo& info);
		
		// (a.walling 2014-12-22 10:24) - PLID 64364 - Rescheduling Queue
		void AddToReschedulingQueue(long apptID, long nPatientID);

		void RemoveFromReschedulingQueue(long apptID);
		// (b.spivey, February 6, 2015) - PLID 64395 - This has the potential to fail, lets handle that and tell the calling code that it failed.
		bool RemoveFromReschedulingQueueWithNote(long nApptID, long nPersonID, long nUserID, CString strPurpose, CString strResource, CString strUserName, COleDateTime dtApptDate);

		// (a.walling 2015-01-26 14:46) - PLID 64687
		long GetFirstAlphabeticalResource(long nApptID);

		// (a.walling 2015-01-26 14:46) - PLID 64687
		void OpenResourceInDayViewForToday(long nResourceID);

		// (a.walling 2015-01-27 12:14) - PLID 64416
		bool IsPatientInReschedulingQueue(long nPersonID); // faster than GetAppointmentsInReschedulingQueueForPatient
		std::vector<long> GetAppointmentsInReschedulingQueueForPatient(long nPersonID);

		// (a.walling 2015-02-04 09:12) - PLID 64412
		bool IsApptInReschedulingQueue(long apptID); 

		namespace TableChecker {
			void OnReschedulingQueueAdded(long apptID);			
			void OnReschedulingQueueRemoved(long apptID); // does not check for existence of appt
		}
	}
}


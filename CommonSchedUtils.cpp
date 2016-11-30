#include "stdafx.h"
#include "CommonSchedUtils.h"
#include "GlobalSchedUtils.h"
#include "GlobalDataUtils.h"
#ifdef _USRDLL
#include "NxAdo.h"
#endif
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "AuditTrail.h"
using namespace ADODB;
// (c.haag 2010-02-04 12:03) - PLID 37221 - Accesses to reservation objects should be done through the
// CReservation class
//#import "SingleDay.tlb" rename("Delete", "DeleteRes")
#include "Reservation.h"
#include "InsuranceReferralsSelectDlg.h"
#include "SchedulerMixWarningDlg.h"
#include "MonthDlg.h"
#include "NxAPI.h"

using namespace SINGLEDAYLib;

// (a.walling 2010-01-21 16:43) - PLID 37021 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.


// (b.eyers 2015-03-24) - PLID 65420 - moved this to api
long AppointmentGetBookedCount(long nResourceID, const COleDateTime &dtDate, const COleDateTime &dtStartTime, const COleDateTime &dtEndTime, 
							   const COleDateTime& dtMin, const COleDateTime& dtMax, long nIgnoreApptID)
{

	COleDateTime dtStart, dtEnd;
	dtStart.SetDateTime(dtDate.GetYear(),dtDate.GetMonth(),dtDate.GetDay(),dtStartTime.GetHour(), dtStartTime.GetMinute(), dtStartTime.GetSecond());
	dtEnd.SetDateTime(dtDate.GetYear(),dtDate.GetMonth(),dtDate.GetDay(),dtEndTime.GetHour(), dtEndTime.GetMinute(), dtEndTime.GetSecond());


	NexTech_Accessor::_PracticeMethodsPtr pApi = GetAPI();
	if (pApi == NULL) {
		ThrowNxException("Could not call GetAppointmentBookedCount due to an invalid API.");
	}

	NexTech_Accessor::_GetAppointmentBookedSearchInfoPtr searchInfo(__uuidof(NexTech_Accessor::GetAppointmentBookedSearchInfo));
	searchInfo->StartDateTime = dtStart;
	searchInfo->EndDateTime = dtEnd;
	searchInfo->ResourceID = nResourceID == -1 ? "" : _bstr_t(AsString(nResourceID));
	searchInfo->IgnoreApptID = nIgnoreApptID == -1 ? "" : _bstr_t(AsString(nIgnoreApptID));

	NexTech_Accessor::_AppointmentBookedCountInfoPtr pResult = pApi->GetAppointmentBookedCount(GetAPISubkey(), GetAPILoginToken(), searchInfo);
	
	long nBookedCount = pResult->BookedCount;

	return nBookedCount;

}

BOOL TimesOverlapOrContiguous(const COleDateTime& s1, const COleDateTime& e1, const COleDateTime& s2, const COleDateTime& e2)
{
	// (c.haag 2006-12-08 09:24) - PLID 23666 - Two time blocks overlap if the following is false: The start time of
	// one is on or after the end time of another, or the end time of one is on or before the start time of another
	// To suppliment that logic, we remove the equal operator to include times that do not overlap but are simply
	// contiguous
	if (((CT_GREATER_THAN) & CompareTimes(s1, e2)) || ((CT_LESS_THAN) & CompareTimes(e1, s2))) {
		return FALSE;
	} else {
		return TRUE;
	}
}

void DefragmentTimes(CArray<COleDateTime, COleDateTime>& aStartTimes, CArray<COleDateTime, COleDateTime>& aEndTimes)
{
	// (c.haag 2006-12-08 09:24) - PLID 23666 - This function searches for any overlapping or adjacent time
	// ranges, and combines them. Then the search is repeated recursively.
	ASSERT(aStartTimes.GetSize() == aEndTimes.GetSize());
	BOOL bRecurse = FALSE;
	for (int i=0; i < aStartTimes.GetSize(); i++) {
		const COleDateTime& s1 = aStartTimes[i];
		const COleDateTime& e1 = aEndTimes[i];
		for (int j = i+1; j < aStartTimes.GetSize(); j++) {
			const COleDateTime& s2 = aStartTimes[j];
			const COleDateTime& e2 = aEndTimes[j];
			// If the following is true, then we have an overlap, so we need to merge the two
			// elements together into the i'th element
			if (TimesOverlapOrContiguous(s1,e1, s2,e2)) {
// Uncomment this handy code when testing thoroughly
//#ifdef _DEBUG
//				TRACE(CString("Merging ") + FormatDateTimeForInterface(s1,dtoTime)  + "-" + FormatDateTimeForInterface(e1,dtoTime) + " with " + FormatDateTimeForInterface(s2,dtoTime)  + "-" + FormatDateTimeForInterface(e2,dtoTime) + "..." );
//#endif
				if (CT_GREATER_THAN == CompareTimes(s1, s2)) {
					aStartTimes[i] = s2;					
				}
				if (CT_LESS_THAN == CompareTimes(e1, e2)) {
					aEndTimes[i] = e2;
				}

// Uncomment this handy code when testing thoroughly
//#ifdef _DEBUG
//				TRACE(CString("Result: ") + FormatDateTimeForInterface(aStartTimes[i],dtoTime)  + "-" + FormatDateTimeForInterface(aEndTimes[i],dtoTime) + "\n" );
//#endif
				aStartTimes.RemoveAt(j);
				aEndTimes.RemoveAt(j);
				bRecurse = TRUE;
				j--;
			}
		}
	}

	// Now repeat if necessary
	if (bRecurse) {
		DefragmentTimes(aStartTimes, aEndTimes);
	}
}

// (j.dinatale 2012-11-21 10:20) - PLID 54162 - need to determine if info about our appt makes it an event
bool IsAppointmentEvent(const COleDateTime& dtDate, const COleDateTime& dtStartTime, const COleDateTime& dtEndTime)
{
	if(!dtDate.GetStatus()){
		if(dtStartTime.GetHour() == 0 && dtStartTime.GetMinute() == 0){
			if(dtEndTime.GetHour() == 0 && dtEndTime.GetMinute() == 0){
				return true;
			}
		}
	}

	return false;
}

// (a.walling 2010-06-14 16:56) - PLID 23560 - Resource Sets
//TES 2/25/2011 - PLID 42523 - Added bCheckPrecisionTemplates.  If FALSE, then this function will assume that the appointment is not
// on a precision template, and so will only check non-precision templates.
BOOL AppointmentValidateByTemplateOnlyScheduling(const COleDateTime& dtDate, const COleDateTime& dtStartTime, const COleDateTime& dtEndTime,
												 const CDWordArray& anResources, BOOL bCheckPrecisionTemplates, BOOL bSilent = FALSE)
{
	// (j.dinatale 2012-11-21 14:19) - PLID 54162 - if they are scheduling an event, we dont need to validate against templating
	if(IsAppointmentEvent(dtDate, dtStartTime, dtEndTime)){
		return TRUE;
	}

	// (c.haag 2006-12-08 09:12) - PLID 23666 - This function returns FALSE if we cannot schedule an appointment
	// with the given date and times because there is at least one time slot where no template exists. THE
	// OFFICE HOURS TEMPLATE DOES NOT COUNT.
	BOOL bFullyCoveredByTemplate = FALSE;
	const int nResources = anResources.GetSize();
	int i, iResource;

	// (c.haag 2006-12-07 17:18) - PLID 23666 - We now have a preference to force users to only be
	// able to schedule in template date ranges unless they have permission to do so otherwise.
	// (c.haag 2007-02-19 16:52) - PLID 23666 - This preference contradicts with the user-specific
	// precision templating permissions. Prior to this note, the preference would have to be turned
	// on for the user-specific permissions to have any meaning. It's a case of a preference controlling
	// permissions, not vice versa. After talking with Zack and Jennie, we've decided to ditch the 
	// preference completely and now we only check user-specific permissions, which are defaulted to on
	// for anyone who has create permissions for appointments (which are most users). The preference never
	// should have existed in the first place; we should have always been permission based.
	/*if (!GetRemotePropertyInt("UseTemplateOnlyScheduling", 0, 0, "<None>", true)) {
		return TRUE;
	}*/

	BOOL bAccess = GetCurrentUserPermissions(bioBlockScheduling) & sptDynamic0;
	BOOL bAccessWithPass = GetCurrentUserPermissions(bioBlockScheduling) & sptDynamic0WithPass;
	if (!bAccess && bAccessWithPass) {
		// Prompt for password
		if (CheckCurrentUserPassword())	{
			bAccess = TRUE;
		}
	}
	if (bAccess) {
		return TRUE;
	}

	// These arrays contain the time ranges of all templates that apply here
	CArray<COleDateTime, COleDateTime> aTemplateStartTimes;
	CArray<COleDateTime, COleDateTime> aTemplateEndTimes;
	for (iResource=0; iResource < nResources; iResource++) {
		//
		// (c.haag 2006-12-07 17:51) - Pull up all templates that occur on the same day as the appointment
		//
		// (a.walling 2014-04-21 14:47) - PLID 60474 - TemplateHitAllP - open in snapshot isolation
		_CommandPtr pcmd = OpenStoredProc(GetRemoteDataSnapshot(), "TemplateHitAllP");
		// (b.spivey, December 08, 2011) - PLID 46947 - We need to only send dates here. 
		//		If we compare a datetime to a date with a default time for a template line item that runs to/from the same day, 
		//		then we'll never get results because we'll always be after the end date
		//		(the end date is stored as "the date" at midnight, and compared against the time that you input, 
		//			i.e. WHERE '12-08-2011 00:00:00' > '12-08-2011 13:25:00')
		COleDateTime dtSqlDate;
		dtSqlDate.SetDate(dtDate.GetYear(), dtDate.GetMonth(), dtDate.GetDay());
		AddParameterDateTime(pcmd, "CheckDate", dtSqlDate); 
		
		// (z.manning, 05/25/2007) - PLID 26062 - Resource ID paramater is now a string to support multiple IDs.
		// Also added IncludeRuleInfo parameter.
		AddParameterString(pcmd, "ResourceIDs", FormatString("%li",anResources[iResource]));
		AddParameterBool(pcmd, "IncludeRuleInfo", false);
		_RecordsetPtr prs = CreateRecordset(pcmd);
		FieldsPtr f = prs->Fields;

		//
		// (c.haag 2006-12-08 09:15) - If we get records, there is at least one template that partially spans
		// the appointment times. We need to put all of the template times into an array, and then merge adjacent
		// and overlapping times. After that is done, we have to check if any results completely overlap the
		// appointment times. If they do not, the test fails.
		//
		while (!prs->eof) {
			// (c.haag 2007-01-09 09:21) - PLID 23666 - Only include regular templates, not "template blocks"
			// (c.haag 2007-02-19 17:24) - PLID 23666 - Include both block and non-block templates
			//TES 2/25/2011 - PLID 42523 - Added an option to exclude precision templates
			if (bCheckPrecisionTemplates || FALSE == AdoFldBool(f, "IsBlock")) {
				COleDateTime dts = AdoFldDateTime(f, "StartTime");
				COleDateTime dte = AdoFldDateTime(f, "EndTime");
				dts.SetTime(dts.GetHour(), dts.GetMinute(), 0);
				dte.SetTime(dte.GetHour(), dte.GetMinute(), 0);
				aTemplateStartTimes.Add(dts);
				aTemplateEndTimes.Add(dte);
			}
			prs->MoveNext();
		}
	}

	// Merge adjacent and overlapping times (recursively)
	DefragmentTimes(aTemplateStartTimes, aTemplateEndTimes);

// Uncomment this handy code when testing thoroughly
//#ifdef _DEBUG
//	TRACE(CString("Final results for comparing ") + FormatDateTimeForInterface(dtStartTime,dtoTime) + " and " + FormatDateTimeForInterface(dtEndTime,dtoTime) + (":\n"));
//	for (i=0; i < aTemplateStartTimes.GetSize(); i++) {
//		TRACE(FormatDateTimeForInterface(aTemplateStartTimes[i],dtoTime)  + "-" + FormatDateTimeForInterface(aTemplateEndTimes[i],dtoTime) + "\n");
//	}
//	TRACE("\n");
//#endif

	// Now look for any time span where dtStartTime and dtEndTime are fully contained. If we find one,
	// it must mean that the appointment is completely covered by a template. We use CompareTimes because
	// the COleDateTime operators fail at the precision we need. (If there is an issue with CompareTimes,
	// please refer to the PL item for which the function was written)
	const COleDateTime& s1 = dtStartTime;
	const COleDateTime& e1 = dtEndTime;
	for (i=0; i < aTemplateStartTimes.GetSize() && !bFullyCoveredByTemplate; i++) {
		const COleDateTime& s2 = aTemplateStartTimes[i];
		const COleDateTime& e2 = aTemplateEndTimes[i];
		if (((CT_GREATER_THAN_OR_EQUAL) & CompareTimes(s1, s2)) &&
			((CT_LESS_THAN_OR_EQUAL) & CompareTimes(s1, e2)) &&
			((CT_GREATER_THAN_OR_EQUAL) & CompareTimes(e1, s2)) &&
			((CT_LESS_THAN_OR_EQUAL) & CompareTimes(e1, e2))) {
			bFullyCoveredByTemplate = TRUE;
		}
	}
	
	// (a.walling 2010-06-14 16:55) - PLID 23560 - Resource Sets
	if (!bSilent && !bFullyCoveredByTemplate)
	{
		MsgBox("This appointment cannot be saved because it does not completely fall within the date range of any regular scheduler template.");
	}
	return bFullyCoveredByTemplate;
}

///////////////////////////////////
///////// Big speed hit ///////////
BOOL AppointmentValidateByRules(ADODB::_RecordsetPtr pAppt, long bIgnoreLinkedAppts)
{
	FieldsPtr pflds = pAppt->Fields;
	CDWordArray adwResourceID;
	CDWordArray adwPurposeID;
	long nApptID = AdoFldLong(pflds, "ID", -1);

	// (a.walling 2013-02-08 12:59) - PLID 55083 - Parameterize
	_RecordsetPtr rs = CreateParamRecordset("SELECT ResourceID FROM AppointmentResourceT WHERE AppointmentID = {INT}", nApptID);
	while (!rs->eof)
	{
		adwResourceID.Add(AdoFldLong(rs, "ResourceID"));
		rs->MoveNext();
	}
	rs->Close();
	rs.Detach();

	// (a.walling 2013-02-08 12:59) - PLID 55083 - Parameterize
	rs = CreateParamRecordset("SELECT PurposeID FROM AppointmentPurposeT WHERE AppointmentID = {INT}", nApptID);
	while (!rs->eof)
	{
		adwPurposeID.Add(AdoFldLong(rs, "PurposeID"));
		rs->MoveNext();
	}
	rs->Close();
	rs.Detach();

	return AppointmentValidateByRules( 
		AdoFldLong(pflds, "PatientID"), 
		adwResourceID, 
		AdoFldLong(pflds, "LocationID"),
		AdoFldDateTime(pflds, "Date"),
		AdoFldDateTime(pflds, "StartTime"),
		AdoFldDateTime(pflds, "EndTime"),
		AdoFldLong(pflds, "AptTypeID", -1),
		adwPurposeID,
		nApptID,
		bIgnoreLinkedAppts,
		FALSE);
}

BOOL AppointmentValidateByRules(ADODB::_RecordsetPtr pAppt, OUT CString *pstrFatalWarnings, OUT CString *pstrNonfatalWarnings, OUT long *pnRuleWarningResult)
{
	FieldsPtr pflds = pAppt->Fields;
	CDWordArray adwResourceID;
	CDWordArray adwPurposeID;
	long nApptID = AdoFldLong(pflds, "ID", -1);
	long nPatientID = AdoFldLong(pflds, "PatientID");
	long nLocationID = AdoFldLong(pflds, "LocationID");
	COleDateTime dtDate = AdoFldDateTime(pflds, "Date");
	COleDateTime dtStartTime = AdoFldDateTime(pflds, "StartTime");
	COleDateTime dtEndTime = AdoFldDateTime(pflds, "EndTime");
	long nAptType = AdoFldLong(pflds, "AptTypeID", -1);
	CString strFatalWarnings;
	CString strNonfatalWarnings;
	long nRuleWarningResult = 0;
	BOOL bRes = TRUE;

	// (a.walling 2013-02-08 12:59) - PLID 55083 - Parameterize
	_RecordsetPtr rs = CreateParamRecordset("SELECT ResourceID FROM AppointmentResourceT WHERE AppointmentID = {INT}", nApptID);
	while (!rs->eof)
	{
		adwResourceID.Add(AdoFldLong(rs, "ResourceID"));
		rs->MoveNext();
	}
	rs->Close();
	rs.Detach();

	// (c.haag 2006-12-07 17:13) - PLID 23666 - If we require users to schedule within templates,
	// we should make sure the times are ok
	//TES 2/25/2011 - PLID 42523 - Tell the function to check both precision and non-precision templates.
	if (!AppointmentValidateByTemplateOnlyScheduling(dtDate, dtStartTime, dtEndTime, adwResourceID, TRUE)) {
		return FALSE;
	}

	// (a.walling 2013-02-08 12:59) - PLID 55083 - Parameterize
	rs = CreateParamRecordset("SELECT PurposeID FROM AppointmentPurposeT WHERE AppointmentID = {INT}", nApptID);
	while (!rs->eof)
	{
		adwPurposeID.Add(AdoFldLong(rs, "PurposeID"));
		rs->MoveNext();
	}
	rs->Close();
	rs.Detach();

	for (int i=0; i < adwResourceID.GetSize(); i++)
	{
		if (!adwPurposeID.GetSize())
		{
			if (!AppointmentValidateByRules(nPatientID, adwResourceID[i], nLocationID, dtDate, dtStartTime, dtEndTime, 
									  nAptType, -1, nApptID, TRUE, pstrFatalWarnings, pstrNonfatalWarnings, pnRuleWarningResult))
			{
				bRes = FALSE;
			}
		}
		else
		{
			for (int j=0; j < adwPurposeID.GetSize(); j++)
			{
				if (!AppointmentValidateByRules(nPatientID, adwResourceID[i], nLocationID, dtDate, dtStartTime, dtEndTime, 
										  nAptType, adwPurposeID[j], nApptID, TRUE, pstrFatalWarnings, pstrNonfatalWarnings, pnRuleWarningResult))
				{
					bRes = FALSE;
				}
			}
		}
	}
	return bRes;
}

////////////////////////////////////
/*
	CREATE TABLE #CalcTimesQ (
		PriorityDashTemplateID nvarchar(21),
		ItemStartTime nvarchar(12),
		ItemEndTime nvarchar(12),
		ApptStartTime nvarchar(12),
		ApptEndTime nvarchar(12),
		OverStartTime nvarchar(12),
		OverEndTime nvarchar(12));

	INSERT INTO #CalcTimesQ (PriorityDashTemplateID, ItemStartTime, ItemEndTime, ApptStartTime, ApptEndTime)
	SELECT DISTINCT 
		CONVERT(char(1), TemplateT.Priority/1000) + CONVERT(char(1), (TemplateT.Priority%1000)/100) + CONVERT(char(1), ((TemplateT.Priority%1000)%100)/10) + CONVERT(char(1), ((TemplateT.Priority%1000)%100)%10) + '-' + CONVERT(nvarchar, TemplateT.ID),
		CONVERT(nvarchar, TemplateItemT.StartTime, 114) AS ItemStartTime,
		CONVERT(nvarchar, TemplateItemT.EndTime, 114) AS ItemEndTime,
		CONVERT(nvarchar, @CheckStartTime, 114) AS ApptStartTime,
		CONVERT(nvarchar, @CheckEndTime, 114) AS ApptEndTime
	FROM  
		TemplateT INNER JOIN  
		TemplateConnectT ON TemplateT.ID = TemplateConnectT.TemplateID INNER JOIN  
		TemplateItemT ON TemplateT.ID = TemplateItemT.TemplateID LEFT JOIN 
		TemplateDetailsT ON TemplateItemT.ID = TemplateDetailsT.TemplateItemID 
	WHERE 
		--  
		--	The templateitem DOES NOT overlap the given appointment time range when (T=Template, A=Appointment) 
		--		(T.end <= A.start) OR (T.start >= A.end) 
		--	This assumes of course that all 'start's are before their corresponding 'end's. 
 		--
		--	To put this formula into the context of this query: 
		--		(TemplateItemT.EndTime <= @CheckStartTime) OR (TemplateItemT.StartTime >= @CheckEndTime) 
 		--  
		--	BUT, we want to filter OUT any templates that don't overlap with the given appointment time range so just put a NOT in front 
		--		NOT ((TemplateItemT.EndTime <= @CheckStartTime) OR (TemplateItemT.StartTime >= @CheckEndTime)) 
		-- 
		(NOT ((CONVERT(nvarchar, TemplateItemT.EndTime, 114) <= CONVERT(nvarchar, @CheckStartTime, 114)) OR (CONVERT(nvarchar, TemplateItemT.StartTime, 114) >= CONVERT(nvarchar, @CheckEndTime, 114)))) AND 

		(TemplateT.StartDate IS NULL OR TemplateT.StartDate <= @CheckDate) AND  
		(TemplateT.EndDate IS NULL OR TemplateT.EndDate >= @CheckDate) AND  
		((TemplateConnectT.ResourceID = -1) OR (TemplateConnectT.ResourceID = @CheckItemID)) AND (( 
			--TemplateHitOnceQ 
			(TemplateItemT.Scale = 1) AND 
			(TemplateItemT.PivotDate = @CheckDate)  
		) OR ( 
			--TemplateHitDailyQ 
			(TemplateItemT.Scale = 2) AND 
			((DateDiff(dd, PivotDate, @CheckDate) % Period) = 0) AND 
			((DatePart(dw, @CheckDate) - 1) = TemplateDetailsT.DayOfWeek) 
		) OR ( 
			--TemplateHitWeeklyQ 
			(TemplateItemT.Scale = 3) AND 
			((DateDiff(wk, PivotDate, @CheckDate) % Period) = 0) AND 
			((DatePart(dw, @CheckDate) - 1) = TemplateDetailsT.DayOfWeek) 
		) OR ( 
			--TemplateHitMonthlyByDateQ 
			(TemplateItemT.Scale = 4) AND 
			(TemplateItemT.MonthBy = 2) AND 
			((DateDiff(mm, PivotDate, @CheckDate) % Period) = 0) AND 
			(DatePart(dd, @CheckDate) = TemplateItemT.DayNumber) 
		) OR ( 
			--TemplateHitMonthlyByPatternQ 
			(TemplateItemT.Scale = 4) AND  
			(TemplateItemT.MonthBy = 1) AND 
			(((DatePart(dd, @CheckDate) - 1) / 7 + 1) = PatternOrdinal) AND 
			((DateDiff(mm, PivotDate, @CheckDate) % Period) = 0) AND 
			((DatePart(dw, @CheckDate) - 1) = TemplateDetailsT.DayOfWeek) 
		) OR ( 
			--TemplateHitYearlyQ 
			(TemplateItemT.Scale = 5) AND 
			(DatePart(mm, PivotDate) = DatePart(mm, @CheckDate)) AND 
			(DatePart(dd, PivotDate) = DatePart(dd, @CheckDate)) 
		)) 

	UPDATE #CalcTimesQ SET
		OverStartTime = (case when (ItemStartTime > ApptStartTime) then (ItemStartTime) else (ApptStartTime) end),
		OverEndTime = (case when (ItemEndTime < ApptEndTime) then (ItemEndTime) else (ApptEndTime) end)


	CREATE TABLE #OutputQ (TemplateID int);

	INSERT INTO #OutputQ (TemplateID)
		SELECT SUBSTRING(MAX(#CalcTimesQ.PriorityDashTemplateID), 6, 11) AS TemplateID
		FROM #CalcTimesQ
		GROUP BY #CalcTimesQ.OverStartTime, #CalcTimesQ.OverEndTime
	
	SELECT DISTINCT 
		TemplateRuleT.ID, 
		TemplateRuleT.Type, 
		TemplateRuleDetailsT.ID AS TemplateRuleDetailID, 
		TemplateRuleDetailsT.Quantity, 
		TemplateRuleDetailsT.ObjectID, 
		#OutputQ.TemplateID, 
		TemplateRuleT.Description
	FROM 
		#OutputQ LEFT JOIN
		TemplateRuleT ON #OutputQ.TemplateID = TemplateRuleT.TemplateID LEFT JOIN 
		TemplateRuleDetailsT ON TemplateRuleT.ID = TemplateRuleDetailsT.TemplateRuleID 
	WHERE 
		TemplateRuleT.ID IS NOT NULL;
*/

/*
BOOL AppointmentValidateByRules(long nPatientID, long nResourceID, long nLocationID, const COleDateTime &dtDate, 
							  const COleDateTime &dtStartTime, const COleDateTime &dtEndTime, 
							  long nAptType, long nAptPurpose)
{
	// Open the stored proc and give it the necessary parameters
	_CommandPtr pcmd = OpenStoredProc("TemplateRulesByTimeP");
	AddParameterDateTime(pcmd, "CheckDate", dtDate);
	AddParameterDateTime(pcmd, "CheckStartTime", dtStartTime);
	AddParameterDateTime(pcmd, "CheckEndTime", dtEndTime);
	AddParameterLong(pcmd, "CheckItemID", nResourceID);
	
	// Open the recordset based on the command proc
	_RecordsetPtr prs = CreateRecordset(pcmd);

	FieldsPtr flds = prs->Fields;
	while (!prs->eof) {
		if (AfxMessageBox(AdoFldString(flds, "Description"), MB_YESNO) == IDNO) {
			return FALSE;
		}
		prs->MoveNext();
	}

	return TRUE;
}
*/

_RecordsetPtr Rule_ReadRuleSet(const COleDateTime &dtDate,		// Date of the appointment being tested
							 const COleDateTime &dtStartTime,	// Start time of the appointment being tested
							 const COleDateTime &dtEndTime,		// End time of the appointment being tested
							 long nResourceID,					// Resource ID of the appointment (must call Rule_CreateCalcTimes once for each appointment resource)
							 long nLineItemOverrideID			// -1 to check all templates, or the ID of a specific template line item to test
																//TES 2/25/2011 - PLID 42523 - Make that < 0 to check all templates (could be -2)
							 )
{
	// (c.haag 2007-01-02 13:07) - PLID 18602 - This function is a consolidation of Rule_CreateCalcTimes, Rule_CreateRuleSet and Rule_GetRuleSet.
	// It also uses table variables instead of temp tables. This should eliminate the Invalid Object Name #CalcTimes... errors.
	CString strSqlBatch;

	// Rule_CreateCalcTimes BEGIN ////////////////////////////////////////////////////////////////
	// (z.manning, 10/26/2006) - PLID 23236 - Updated query to now use StartDate, EndDate, and resource as a property on line items.
	// (z.manning, 11/09/2006) - PLID 23276 - Now that date range is a property of template line items, the PivotDate column is not needed.
	// (c.haag 2006-12-04 09:01) - PLID 23629 - Adding support for template exceptions and line item exceptions
	// (c.haag 2006-12-12 09:03) - PLID 23666 - We now have the ability to force this query to recognize rules for a specific line item
	// (c.haag 2007-01-02 13:23) - PLID 18602 - Added SET NOCOUNT ON so that we don't get the "execute parts" of the query returned in our result.
	strSqlBatch.Format(
		"SET NOCOUNT ON \r\n"
		"DECLARE @CheckDate datetime; \r\n"
		"DECLARE @CheckStartTime datetime; \r\n"
		"DECLARE @CheckEndTime datetime; \r\n"
		"DECLARE @CheckItemID int; \r\n"
		" \r\n"
		"SELECT @CheckDate = '%s'; \r\n"
		"SELECT @CheckStartTime = '%s'; \r\n"
		"SELECT @CheckEndTime = '%s'; \r\n"
		"SELECT @CheckItemID = %li; \r\n"
		" \r\n"
		"DECLARE @CalcTimesT TABLE ( \r\n"
		"	PriorityDashTemplateID nvarchar(21), \r\n"
		"	ItemStartTime nvarchar(12), \r\n"
		"	ItemEndTime nvarchar(12), \r\n"
		"	ApptStartTime nvarchar(12), \r\n"
		"	ApptEndTime nvarchar(12), \r\n"
		"	OverStartTime nvarchar(12), \r\n"
		"	OverEndTime nvarchar(12)); \r\n"
		" \r\n"
		"INSERT INTO @CalcTimesT (PriorityDashTemplateID, ItemStartTime, ItemEndTime, ApptStartTime, ApptEndTime) \r\n"
		"SELECT DISTINCT  \r\n"

		// (c.haag 2006-12-04 09:19) - PLID 23629 - We now have to calculate the priority, making ugly code even more ugly
		//"	CONVERT(char(1), TemplateT.Priority/1000) + CONVERT(char(1), (TemplateT.Priority%%1000)/100) + CONVERT(char(1), ((TemplateT.Priority%%1000)%%100)/10) + CONVERT(char(1), ((TemplateT.Priority%%1000)%%100)%%10) "
		"	CONVERT(CHAR(1), (CASE WHEN (CASE WHEN TemplateExceptionT.Flags IS NULL THEN 0 ELSE TemplateExceptionT.Flags END) & 1 <> 0 THEN (SELECT Max(Priority)+1 FROM TemplateT) ELSE TemplateT.Priority END)/1000) + CONVERT(char(1), ((CASE WHEN (CASE WHEN TemplateExceptionT.Flags IS NULL THEN 0 ELSE TemplateExceptionT.Flags END) & 1 <> 0 THEN (SELECT Max(Priority)+1 FROM TemplateT) ELSE TemplateT.Priority END) %% 1000)/100) +  \r\n"
		"		CONVERT(char(1), (((CASE WHEN (CASE WHEN TemplateExceptionT.Flags IS NULL THEN 0 ELSE TemplateExceptionT.Flags END) & 1 <> 0 THEN (SELECT Max(Priority)+1 FROM TemplateT) ELSE TemplateT.Priority END) %% 1000) %% 100)/10) + CONVERT(char(1), (((CASE WHEN (CASE WHEN TemplateExceptionT.Flags IS NULL THEN 0 ELSE TemplateExceptionT.Flags END) & 1 <> 0 THEN (SELECT Max(Priority)+1 FROM TemplateT) ELSE TemplateT.Priority END) %% 1000) %% 100) %% 10) +  \r\n"
		// (c.haag 2006-12-04 09:19) - PLID 23629 - END CHANGE

		"   '-' + CONVERT(nvarchar, TemplateT.ID), \r\n"
		"	CONVERT(nvarchar, TemplateItemT.StartTime, 114) AS ItemStartTime, \r\n"
		"	CONVERT(nvarchar, TemplateItemT.EndTime, 114) AS ItemEndTime, \r\n"
		"	CONVERT(nvarchar, @CheckStartTime, 114) AS ApptStartTime, \r\n"
		"	CONVERT(nvarchar, @CheckEndTime, 114) AS ApptEndTime \r\n"
		"FROM   \r\n"
		"	TemplateT   \r\n"
		"	INNER JOIN TemplateItemT ON TemplateT.ID = TemplateItemT.TemplateID  \r\n"
		"	LEFT JOIN TemplateItemResourceT ON TemplateItemT.ID = TemplateItemResourceT.TemplateItemID   \r\n"
		"	LEFT JOIN TemplateDetailsT ON TemplateItemT.ID = TemplateDetailsT.TemplateItemID  \r\n"

		/* (c.haag 2006-11-21 12:02) - PLID 23629 - The left join on TemplateExceptionT will not duplicate line item records because of the governing rules for TemplateExceptionT.Flags.  
		If TemplateExceptionT.Flags is 1 for the given @dtCheckDate, then it's a "Top priority" template, and no other record in the entire TemplateExceptionT  
		table should meet this criteria. If TemplateExceptionT.Flags is 2, then the records must not be visible (refer to the conditional below in the WHERE  
		clause). Therefore, they cannot be duplicated. */  
		"		LEFT JOIN TemplateExceptionT ON (TemplateT.ID = TemplateExceptionT.TemplateID AND @CheckDate >= TemplateExceptionT.StartDate AND @CheckDate <= TemplateExceptionT.EndDate)     \r\n"
		"		LEFT JOIN TemplateItemExceptionT ON (TemplateItemT.ID = TemplateItemExceptionT.TemplateItemID AND @CheckDate = TemplateItemExceptionT.Date)     \r\n"
		// (c.haag 2006-11-21 12:06) - PLID 23629 - END CHANGE

		"WHERE  \r\n"
		"	--   \r\n"
		"	--	The templateitem DOES NOT overlap the given appointment time range when (T=Template, A=Appointment)  \r\n"
		"	--		(T.end <= A.start) OR (T.start >= A.end)  \r\n"
		"	--	This assumes of course that all 'start's are before their corresponding 'end's.  \r\n"
 		"	-- \r\n"
		"	--	To put this formula into the context of this query:  \r\n"
		"	--		(TemplateItemT.EndTime <= @CheckStartTime) OR (TemplateItemT.StartTime >= @CheckEndTime)  \r\n"
 		"	--   \r\n"
		"	--	BUT, we want to filter OUT any templates that don't overlap with the given appointment time range so just put a NOT in front  \r\n"
		"	--		NOT ((TemplateItemT.EndTime <= @CheckStartTime) OR (TemplateItemT.StartTime >= @CheckEndTime))  \r\n"
		"	--  \r\n"
		"	(NOT ((CONVERT(nvarchar, TemplateItemT.EndTime, 114) <= CONVERT(nvarchar, @CheckStartTime, 114)) OR (CONVERT(nvarchar, TemplateItemT.StartTime, 114) >= CONVERT(nvarchar, @CheckEndTime, 114)))) AND  \r\n"

		// (c.haag 2006-12-04 09:20) - PLID 23629 - Exclude templates and template line items that are not visible today
		"		(TemplateExceptionT.Flags IS NULL OR (TemplateExceptionT.Flags & 2) = 0)  AND \r\n"
		"		(TemplateItemExceptionT.ID IS NULL) AND \r\n"
		// (c.haag 2006-12-04 09:20) - PLID 23629 - END CHANGE

		"  \r\n"
		"	(TemplateItemT.StartDate IS NULL OR TemplateItemT.StartDate <= @CheckDate) AND   \r\n"
		"	(TemplateItemT.EndDate IS NULL OR TemplateItemT.EndDate >= @CheckDate) AND   \r\n"
		"	(TemplateItemT.AllResources = 1 OR (TemplateItemResourceT.ResourceID = @CheckItemID)) AND \r\n"
		// (c.haag 2007-01-09 09:18) - PLID 23666 - We now only check regular templates (as opposed to "template blocks")
		// unless the user clicked on a specific template block to schedule over it
		// (z.manning 2008-07-08 09:35) - PLID 29743 - Chris' comment is now only true if the user clicked on a precision
		// template. Otherwise, we check all templates.
		"%s "

		// (c.haag 2006-12-12 09:07) - PLID 23666 - We can now pull rules based on only one template line item
		"   (%d IN (-1, TemplateItemT.ID)) AND \r\n"
		// (c.haag 2006-12-12 09:07) - PLID 23666 - END CHANGE

		"((  \r\n"
		"		--TemplateHitOnceQ  \r\n"
		"		(TemplateItemT.Scale = 1)  \r\n"
		"	) OR (  \r\n"
		"		--TemplateHitDailyQ  \r\n"
		"		(TemplateItemT.Scale = 2) AND  \r\n"
		"		((DateDiff(dd, TemplateItemT.StartDate, @CheckDate) %% Period) = 0) AND  \r\n"
		"		((DatePart(dw, @CheckDate) - 1) = TemplateDetailsT.DayOfWeek)  \r\n"
		"	) OR (  \r\n"
		"		--TemplateHitWeeklyQ  \r\n"
		"		(TemplateItemT.Scale = 3) AND  \r\n"
		"		((DateDiff(wk, TemplateItemT.StartDate, @CheckDate) %% Period) = 0) AND  \r\n"
		"		((DatePart(dw, @CheckDate) - 1) = TemplateDetailsT.DayOfWeek)  \r\n"
		"	) OR (  \r\n"
		"		--TemplateHitMonthlyByDateQ  \r\n"
		"		(TemplateItemT.Scale = 4) AND  \r\n"
		"		(TemplateItemT.MonthBy = 2) AND  \r\n"
		"		((DateDiff(mm, TemplateItemT.StartDate, @CheckDate) %% Period) = 0) AND  \r\n"
		"		(DatePart(dd, @CheckDate) = TemplateItemT.DayNumber)  \r\n"
		"	) OR (  \r\n"
		"		--TemplateHitMonthlyByPatternQ  \r\n"
		"		(TemplateItemT.Scale = 4) AND   \r\n"
		"		(TemplateItemT.MonthBy = 1) AND  \r\n"
		"		( ( ((DatePart(dd, @CheckDate) - 1) / 7 + 1) = PatternOrdinal) OR (PatternOrdinal = -1 AND DATEPART(mm, DATEADD(ww, 1, @CheckDate)) != DATEPART(mm, @CheckDate)) ) AND  \r\n"
		"		((DateDiff(mm, TemplateItemT.StartDate, @CheckDate) %% Period) = 0) AND  \r\n"
		"		((DatePart(dw, @CheckDate) - 1) = TemplateDetailsT.DayOfWeek)  \r\n"
		"	) OR (  \r\n"
		"		--TemplateHitYearlyQ  \r\n"
		"		(TemplateItemT.Scale = 5) AND  \r\n"
		"		(DatePart(mm, TemplateItemT.StartDate) = DatePart(mm, @CheckDate)) AND  \r\n"
		"		(DatePart(dd, TemplateItemT.StartDate) = DatePart(dd, @CheckDate))  \r\n"
		"	)); \r\n"
		" \r\n"
		"/* Check for overlaps with the appointment against each template */ \r\n"
		"UPDATE @CalcTimesT SET \r\n"
		"	OverStartTime = (case when (ItemStartTime > ApptStartTime) then (ItemStartTime) else (ApptStartTime) end), \r\n"
		"	OverEndTime = (case when (ItemEndTime < ApptEndTime) then (ItemEndTime) else (ApptEndTime) end);"
		" \r\n"
		,
		FormatDateTimeForSql(dtDate, dtoDate), FormatDateTimeForSql(dtStartTime, dtoDateTime), FormatDateTimeForSql(dtEndTime, dtoDateTime), nResourceID,
		// (c.haag 2006-12-12 09:49) - PLID 23666 - Added nLineItemOverrideID and removed a bunch of extra calc times parameters
		// (shame on whoever changed the query without cleaning them up!)
		// (z.manning 2008-07-08 09:37) - PLID 29743 - We now only filter on non-precision templates if the user clicked
		// on a precision template.
		//TES 2/25/2011 - PLID 42523 - Compare on > 0 rather than != -1
		nLineItemOverrideID > 0 ? FormatString("	(TemplateItemT.IsBlock = 0 OR TemplateItemT.ID = %d) AND \r\n", nLineItemOverrideID) : "",
		nLineItemOverrideID);


	//JMJ 5/14/2004 - why would we want to do this??? It needs to look at all available templates.
	//TES 10/8/2004 - PLID 14345 - There are sometimes valid reasons for this.  We'll use a preference.
	if(!GetRemotePropertyInt("SchedulerUseOverlappedTemplateRules", 0, 0, "<None>", true)) {
		strSqlBatch += "-- Delete the templates that aren't on top \r\n"			
			// (b.cardillo 2005-07-28 13:24) - PLID 16992 - This used to be too simplistic.  It just 
			// grouped by OverStartTime and OverEndTime, and eliminated any duplicates based on that 
			// grouping.  But that just didn't cut it because, while it did eliminate obscured 
			// templates that the appointment cut through in the same way as the top level template 
			// (the one that would ultimately be considered the "hit" template), it FAILED to 
			// eliminate those obscured templates whose silouette in the light of the appointment was 
			// DIFFERENT from the top level template's silouette.  So the new sql I'm putting here 
			// instead is much more advanced.  You'll notice, in the most baseic of situations, this 
			// sql still simplifies down to exactly what we used to do: group by OverStartTime and 
			// OverEndTime.  But now, it does so by matching each template item against all other 
			// template items, and first eliminating any that are completely obscured by higher-
			// priority template items, then leaving the remaining ones with new OverStartTime and 
			// OverEndTime values, values that reflect only the VISIBLE (i.e. top-level) PORTION of 
			// that template item.  Thus we're left with a list of template items that are known to 
			// be top level, and are also known to be overlapping with the appointment.  Thus those 
			// are the ones that officially "hit" the appointment.  In order to pull of such a magic 
			// trick, it has to keep looping, repeatedly adjusting the values of any remainig template 
			// items comparing against all remaining higher-priority template items.  In most cases 
			// it will only iterate twice through that loop: once to do the work, and the second time 
			// to make sure all the work is done.  But in some cases, that first iteration didn't 
			// finish all the work.  Maybe it adjusted one template's time range, but the now reduced 
			// time range still could be obscured by other template items and therefore would need to 
			// be reduced further.  So it keeps looping until no more reductions can be made, thus 
			// leaving only the few survivors of the time-reduction massacre.  Those survivors are 
			// precisely the template items that belong on top.
			// Also, Rule_CreateRuleSet() used to try to further eliminate overlapped template items, 
			// but again it was too simplistic; it only successfully eliminated a given template if 
			// the template that overlapped it was ONLY overlapping IT (and not any other templates).
			// This new logic here takes care of both steps at once.
			"DECLARE @bContinue BIT \r\n"
			"SET @bContinue = 1 \r\n"
			"WHILE @bContinue = 1 BEGIN \r\n"
			" UPDATE A SET \r\n"
			"  A.OverStartTime = CASE WHEN A.OverStartTime >= B.OverStartTime THEN B.OverEndTime ELSE A.OverStartTime END, \r\n"
			"  A.OverEndTime = CASE WHEN A.OverEndTime <= B.OverEndTime THEN B.OverStartTime ELSE A.OverEndTime END \r\n"
			" FROM @CalcTimesT A INNER JOIN @CalcTimesT B ON A.PriorityDashTemplateID < B.PriorityDashTemplateID AND A.OverStartTime < B.OverEndTime AND A.OverEndTime > B.OverStartTime \r\n"
			" WHERE \r\n"
			"  A.OverStartTime <> CASE WHEN A.OverStartTime >= B.OverStartTime THEN B.OverEndTime ELSE A.OverStartTime END OR \r\n"
			"  A.OverEndTime <> CASE WHEN A.OverEndTime <= B.OverEndTime THEN B.OverStartTime ELSE A.OverEndTime END \r\n"
			"\r\n"
			" IF @@ROWCOUNT = 0 SET @bContinue = 0 \r\n"
			"\r\n"
			" DELETE FROM @CalcTimesT WHERE OverStartTime >= OverEndTime \r\n"
			"END \r\n";
	}
	// Rule_CreateCalcTimes END ////////////////////////////////////////////////////////////////


	// Rule_CreateRuleSet BEGIN ////////////////////////////////////////////////////////////////
	//DRT 1/21/2005 - PLID 15375 - This function was never copied over from the patch branch of source code, making
	//	the feature not work in this branch.
	//TES 10/8/2004 - PLID 14345 - Use a preference to decide whether to check for overlaps.
	// (b.cardillo 2005-07-28 16:21) - PLID 16992 - We no longer have to do anything special here 
	// when the SchedulerUseOverlappedTemplateRules preference is turned off.  Now, we simply 
	// extract the template id from all the remaining records in the temp table, and that's our 
	// official list of templates.  We don't have to do anything because the sql in the 
	// Rule_CreateCalcTimes() function above now takes care of eliminating the overlapped template
	// items.  See the comment there under this plid for details on how it does this.
	strSqlBatch +=
		"DECLARE @RuleSet TABLE (TemplateID int);\r\n"
		"INSERT INTO @RuleSet (TemplateID) "
		"	SELECT DISTINCT SUBSTRING(PriorityDashTemplateID, 6, 11) AS TemplateID "
		"	FROM @CalcTimesT A \r\n";
	// Rule_CreateRuleSet END ////////////////////////////////////////////////////////////////


	// Rule_GetRuleSet BEGIN ////////////////////////////////////////////////////////////////
	//TES 8/31/2010 - PLID 39630 - Added OverrideLocationTemplating
	strSqlBatch +=
		"SELECT DISTINCT "
		"	TemplateRuleT.ID, "
		"	TemplateRuleT.Description, "
		"	TemplateRuleT.WarningOnFail, "
		"	TemplateRuleT.PreventOnFail, "
		"	TemplateRuleT.AndDetails, "
		"	TemplateRuleT.OverrideLocationTemplating "
		"FROM "
		"	@RuleSet RSET LEFT JOIN "
		"	TemplateRuleT ON RSET.TemplateID = TemplateRuleT.TemplateID "
		"WHERE "
		"	TemplateRuleT.ID IS NOT NULL";
	// Rule_GetRuleSet END ////////////////////////////////////////////////////////////////


	// (c.haag 2006-12-04 09:21) - PLID 23629 - This will help us debug the query
/*#ifdef _DEBUG
			OutputDebugString("\r\--- Begin Rule_ReadRuleSet Query ---\r\n\r\n");
			const int nInterval = 256;
			const int nLen = strSqlBatch.GetLength();
			int x = 0;
			while (x < nLen - nInterval) {
				OutputDebugString(strSqlBatch.Mid(x, nInterval));
				Sleep(10);	// It's possible for too much text to be sent to the debugger window in
							// a short period of time
				x += nInterval;
			}
			OutputDebugString(strSqlBatch.Mid(x, nLen - x));
#endif*/

	return CreateRecordsetStd(strSqlBatch);
}

BOOL FindInArray(DWORD nValue, const CDWordArray &aryList);

// Takes a rule ID and processes all it's details against the 
// given appointment properties.  Returns whether the 
// appointment is valid or not based on the rule details
EProcessRuleResult ProcessRule(long nRuleID, BOOL bAndDetails, long nPatientID, long nResourceID, long nLocationID, 
							   const COleDateTime &dtDate, const COleDateTime &dtStartTime, const COleDateTime &dtEndTime, 
							   long nAptType, long nAptPurpose, long nIgnoreApptID)
{
	BOOL bOneOfIsType = FALSE, bOneOfNotType = FALSE, bOneOfIsPurpose = FALSE, bOneOfNotPurpose = FALSE;
	BOOL bCheckIsType = FALSE, bCheckNotType = FALSE, bCheckIsPurpose = FALSE, bCheckNotPurpose = FALSE;
	long nBookingMax = -1;

	BOOL bAllAppts = FALSE;

	// Fill the above variables based on all the details of this rule
	try
	{
		_RecordsetPtr prs = CreateRecordset(
			"SELECT TemplateRuleT.AllAppts, "
			"	TemplateRuleDetailsT.ID, "
			"	TemplateRuleDetailsT.ObjectType, "
			"	TemplateRuleDetailsT.ObjectID "
			"FROM "
			"	TemplateRuleT LEFT JOIN TemplateRuleDetailsT "
			"	ON TemplateRuleT.ID = TemplateRuleDetailsT.TemplateRuleID "
			"WHERE "
			"	TemplateRuleT.ID = %li "
			"   AND (AllAppts = 1 OR (AllAppts = 0 AND TemplateRuleDetailsT.ID Is Not Null))",
			nRuleID);
		
		FieldsPtr flds = prs->Fields;
		while (!prs->eof) {

			bAllAppts = AdoFldBool(flds, "AllAppts");

			//if all appointments, just leave now, as it is a catch-all
			if(bAllAppts)
				return prAppointmentInvalid;

			switch (AdoFldLong(flds, "ObjectType")) {
			case 1: // Type IS IN ObjectID
				bCheckIsType = TRUE;
				if (!bOneOfIsType) {
					if (nAptType == AdoFldLong(flds, "ObjectID")) {
						bOneOfIsType = TRUE;
					}
				}
				break;
			case 101: // Type IS NOT in ObjectID
				bCheckNotType = TRUE;
				if (!bOneOfNotType) {
					if (nAptType == AdoFldLong(flds, "ObjectID")) {
						bOneOfNotType = TRUE;
					}
				}
				break;
			case 2: // Purpose IS in ObjectID
				bCheckIsPurpose = TRUE;
				if (!bOneOfIsPurpose) {
					if (nAptPurpose == AdoFldLong(flds, "ObjectID")) {
						bOneOfIsPurpose = TRUE;
					}
				}
				break;
			case 102:  // Purpose IS NOT in ObjectID
				bCheckNotPurpose = TRUE;
				if (!bOneOfNotPurpose) {
					if (nAptPurpose == AdoFldLong(flds, "ObjectID")) {
						bOneOfNotPurpose = TRUE;
					}
				}
				break;
			case 3: // Booked against ObjectID or more other appointments
				ASSERT(nBookingMax == -1); // if nBookingMax has already been set then this rule has two booking max details, which makes it an invalid rule
				if (nBookingMax == -1) {
					nBookingMax = AdoFldLong(flds, "ObjectID");
				}
				break;
			default:
				ASSERT(FALSE);
				AfxThrowNxException("Unhandled rule detail object type");
				break;
			}

			prs->MoveNext();
		}
	}
#ifdef _USRDLL
	NxCatchAllCallThrow_WithParent(NULL, "Error in ProcessRule (Step 1)", {});
#else
	NxCatchAllCallThrow("Error in ProcessRule (Step 1)", {});
#endif

	try {
		// Now we know all the details of this rule so check the appointment against the details
		if (bCheckIsType || bCheckNotType || bCheckIsPurpose || bCheckNotPurpose || nBookingMax != -1) {
			
			BOOL bMatch;
			COleDateTime dtMin, dtMax; // Min and max date range for booking test
			if (bAndDetails) {
				// Start out assuming our appointment matches all the details
				bMatch = TRUE;
				
				// If we care about IsType and we're not in the IsType list, then we don't match anymore
				if (bCheckIsType && !bOneOfIsType) bMatch = FALSE;
				// If we care about NotType and we are in the NotType list, then we don't match anymore
				if (bCheckNotType && bOneOfNotType) bMatch = FALSE;
				// If we care about IsPurpose and we're not in the IsPurpose list, then we don't match anymore
				if (bCheckIsPurpose && !bOneOfIsPurpose) bMatch = FALSE;
				// If we care about NotPurpose and we are in the NotPurpose list, then we don't match anymore
				if (bCheckNotPurpose && bOneOfNotPurpose) bMatch = FALSE;

				// If we still think we match then we have to try to make a last-ditch effort to break the 
				// match with the booking rule (I say last-ditch because this is a relatively slow test)
				if (bMatch && nBookingMax != -1) {
					/*JMJ 11/20/2003 - this doesn't work right at all if you have multiple Template Line Items,
					and moreover I don't see the need for this part at all

					_RecordsetPtr prsTimes = CreateRecordset("SELECT TemplateItemT.StartTime, "
						"TemplateItemT.EndTime FROM TemplateItemT "
						"INNER JOIN TemplateRuleT ON TemplateItemT.TemplateID = TemplateRuleT.TemplateID "
						"WHERE (TemplateRuleT.ID = %d) ", nRuleID);

					COleDateTime dtStart = AdoFldDateTime(prsTimes, "StartTime");
					COleDateTime dtEnd = AdoFldDateTime(prsTimes, "EndTime");
					dtMin.SetDateTime(dtDate.GetYear(), dtDate.GetMonth(), dtDate.GetDay(), dtStart.GetHour(), dtStart.GetMinute(), dtStart.GetSecond());
					dtMax.SetDateTime(dtDate.GetYear(), dtDate.GetMonth(), dtDate.GetDay(), dtEnd.GetHour(), dtEnd.GetMinute(), dtEnd.GetSecond());
					*/

					if (AppointmentGetBookedCount(nResourceID, dtDate, dtStartTime, dtEndTime, dtMin, dtMax, nIgnoreApptID) < nBookingMax) {
						bMatch = FALSE;
					}
				}
			} else {
				// Start out assuming our appointment doesn't match any of the details
				bMatch = FALSE;

				/*JMJ 11/20/2003 - this doesn't work right at all if you have multiple Template Line Items,
					and moreover I don't see the need for this part at all
				_RecordsetPtr prsTimes = CreateRecordset("SELECT TemplateItemT.StartTime, "
					"TemplateItemT.EndTime FROM TemplateItemT "
					"INNER JOIN TemplateRuleT ON TemplateItemT.TemplateID = TemplateRuleT.TemplateID "
					"WHERE (TemplateRuleT.ID = %d) ", nRuleID);

				COleDateTime dtStart = AdoFldDateTime(prsTimes, "StartTime");
					COleDateTime dtEnd = AdoFldDateTime(prsTimes, "EndTime");
					dtMin.SetDateTime(dtDate.GetYear(), dtDate.GetMonth(), dtDate.GetDay(), dtStart.GetHour(), dtStart.GetMinute(), dtStart.GetSecond());
					dtMax.SetDateTime(dtDate.GetYear(), dtDate.GetMonth(), dtDate.GetDay(), dtEnd.GetHour(), dtEnd.GetMinute(), dtEnd.GetSecond());
				*/
				
				// If we care about IsType and we are in the IsType list, then we match now
				if (bCheckIsType && bOneOfIsType) bMatch = TRUE;
				// If we care about NotType and we're not in the NotType list, then we match now
				if (bCheckNotType && !bOneOfNotType) bMatch = TRUE;
				// If we care about IsPurpose and we are in the IsPurpose list, then we match now
				if (bCheckIsPurpose && bOneOfIsPurpose) bMatch = TRUE;
				// If we care about NotPurpose and we're not in the NotPurpose list, then match now
				if (bCheckNotPurpose && !bOneOfNotPurpose) bMatch = TRUE;

				// If we still don't have a match then we have to make a last-ditch effort to match 
				// against over-booking (I say last-ditch because this is a relatively slow test)
				if (!bMatch && nBookingMax != -1) {
					if (AppointmentGetBookedCount(nResourceID, dtDate, dtStartTime, dtEndTime, dtMin, dtMax, nIgnoreApptID) >= nBookingMax) {
						bMatch = TRUE;
					}
				}
			}

			if (bMatch) {
				return prAppointmentInvalid;
			} else {
				return prAppointmentValid;
			}
		} else {
			// This rule has no details so ignore it (i.e. you can't fail a rule that has no details)
			return prAppointmentValid;
		}
	}
#ifdef _USRDLL
	NxCatchAllCallThrow_WithParent(NULL, "Error in ProcessRule (Step 2)", {});
#else
	NxCatchAllCallThrow("Error in ProcessRule (Step 2)", {});
#endif
	return prAppointmentInvalid; // Should never get here
}

// Returns a bit combination of ERuleWarningsResult values
long AppointmentReadRules(long nPatientID, long nResourceID, long nLocationID, const COleDateTime &dtDate, 
							  const COleDateTime &dtStartTime, const COleDateTime &dtEndTime, 
							  long nAptType, long nAptPurpose, long nIgnoreApptID, BOOL bIsFFA /*= FALSE*/, OUT CString *pstrFatalWarnings /*= NULL*/, OUT CString *pstrNonfatalWarnings /*= NULL*/,
							  _RecordsetPtr prsRuleSet /*= NULL*/)
{
	try {
		// (c.haag 2006-05-23 15:19) - PLID 20665 - We can now pass in the rule set. This is to avoid
		// calculateing the same rules over and over again when calling this function for differing
		// appointment purposes.
		_RecordsetPtr prs;
		if (NULL != prsRuleSet) {
			prs = prsRuleSet;
			if (prs->eof && !prs->bof) {
				prs->MoveFirst();
			}
		} else {
			// (c.haag 2007-01-02 12:46) - PLID 18602 - We now use a single function to get the template rules
			prs = Rule_ReadRuleSet(dtDate, dtStartTime, dtEndTime, nResourceID, -1);
		}
		BOOL bValidatedWithPermission = FALSE;

		BOOL bValidateSuccess = TRUE;
		BOOL bWarnings = FALSE;
		BOOL bAccess = FALSE;
		if (pstrFatalWarnings) (*pstrFatalWarnings) = "";
		if (pstrNonfatalWarnings) (*pstrNonfatalWarnings) = "";

		FieldsPtr flds = prs->Fields;
		while (!prs->eof) {
			// Find out whether the appointment is valid or not based on the rule
			EProcessRuleResult nResult = ProcessRule(
				AdoFldLong(flds, "ID"), AdoFldBool(flds, "AndDetails"), nPatientID, nResourceID, nLocationID, 
				dtDate, dtStartTime, dtEndTime, nAptType, nAptPurpose, nIgnoreApptID);
			// If the appointment is not valid because of the rule
			if (nResult == prAppointmentInvalid) {
				BOOL bPrevent = AdoFldBool(flds, "PreventOnFail");

				// (c.haag 2003-08-01 10:31) - Check to see if the logged in user
				// has permission to override the template restriction.
				// (c.haag 2003-09-29 11:24) - Someone made a change to the permissions
				// in the not-too-distant future that allows administrators to have all
				// permissions, including complete template override.
				bAccess = (GetCurrentUserPermissions(bioSchedTemplateRules, TRUE, AdoFldLong(flds, "ID"))) & sptDynamic0;
				BOOL bAccessWithPass = (GetCurrentUserPermissions(bioSchedTemplateRules, TRUE, AdoFldLong(flds, "ID"))) & sptDynamic0WithPass;
				if (bAccess) {
					bPrevent = FALSE;
				}

				else if (bAccessWithPass)
				{
					if (CheckCurrentUserPassword())
					{
						bAccess = TRUE;
						bValidatedWithPermission = TRUE;
						bPrevent = FALSE;
					}
				}

				// Then handle the rule warning properties
				if (bPrevent) {
					bValidateSuccess = FALSE;
					if (pstrFatalWarnings) {
						_variant_t varWarning = flds->Item["WarningOnFail"]->Value;
						if (varWarning.vt != VT_NULL) {
							(*pstrFatalWarnings) += VarString(varWarning) + "\n";
						}
					}
				} else {
					_variant_t varWarning = flds->Item["WarningOnFail"]->Value;
					if (varWarning.vt != VT_NULL) {
						bWarnings = TRUE;
						if (pstrNonfatalWarnings) {
							(*pstrNonfatalWarnings) += VarString(varWarning) + "\n";
						}
					}
				}
			}
			// Move to the next rule
			prs->MoveNext();
		}

		//If they are an administrator warn them that they usually can't save an appointment at this time
		//DRT 5/19/2006 - PLID 20724 - We now cache the status of the user being an administrator.
		//_RecordsetPtr prsUserType = CreateRecordset("SELECT Administrator FROM UsersT WHERE PersonID = %li", GetCurrentUserID());
		BOOL bIsAdmin = IsCurrentUserAdministrator();
		/*if (!prsUserType->eof) {
			bIsAdmin = AdoFldBool(prsUserType, "Administrator");
		}*/
		UINT nResult = IDNO;
		if (bIsAdmin && bAccess && !bIsFFA) {
			bWarnings = TRUE;
			if (pstrNonfatalWarnings) {
				CString strWarning = "Normally, due to a template rule, an appointment cannot be saved for this time and date,\nbut because you are an administrator you can still save the appointment.";
				(*pstrNonfatalWarnings) += "\n" + strWarning + "\n";
			}
		}

		// Calculate the return value
		long nAns = rwNoRulesApplied;
		if (bWarnings) {
			nAns |= rwHasWarnings;
		}
		if (!bValidateSuccess) {
			nAns |= rwIsPrevented;
		}
		if (bValidatedWithPermission) {
			nAns |= rwNeededPermission;
		}
		return nAns;
	}
#ifdef _USRDLL
	NxCatchAllCallThrow_WithParent(NULL, "Error in AppointmentReadRules", {});
#else
	NxCatchAllCallThrow("Error in AppointmentReadRules", {});
#endif
	return 0; // Should never get here
}

///////////////////////////////////
///////// Big speed hit ///////////

// (a.walling 2010-06-14 17:04) - PLID 23560 - Resource Sets
// (z.manning 2016-04-19 12:29) - NX-100244 - Check admin password function no longer has any parameters
#define VALIDATE_BY_RULES(patid, resourceid, location, date, start, end, type, purpose, ignoreapptid, fatal, nonfatal, ruleset) { \
	if (!AppointmentValidateByRules(patid, resourceid, location, date, start, end, type, purpose, ignoreapptid, TRUE, fatal, nonfatal, &nRuleWarningResult, ruleset)) { \
		if (bSilent) return FALSE; \
		if (!strFatalWarnings.IsEmpty()) { \
			if (IDYES == MsgBox(MB_ICONEXCLAMATION|MB_YESNO, "This appointment could not be saved for the following reasons:\n\n%s\n\nWould you like to save the appointment anyway? This operation requires an administrative password.", strFatalWarnings)) { \
				if (CheckAdministratorPassword()) { \
					nRuleWarningResult = (nRuleWarningResult | rwNeededPermission) & ~rwIsPrevented; \
					return TRUE; \
				} \
			} \
		} else { \
			if (IDYES == MsgBox(MB_ICONEXCLAMATION|MB_YESNO, "This appointment could not be saved because it breaks at least one template rule.\n\nWould you like to save the appointment anyway? This operation requires an administrative password.")) { \
				if (CheckAdministratorPassword()) { \
					nRuleWarningResult = (nRuleWarningResult | rwNeededPermission) & ~rwIsPrevented; \
					return TRUE; \
				} \
			} \
		} \
	  	if (nRuleWarningResult & rwHasWarnings) { \
			if (!strNonfatalWarnings.IsEmpty()) { \
				TruncateSchedulerWarning(strNonfatalWarnings); \
				MsgBox(MB_ICONEXCLAMATION|MB_OK, "This appointment also triggers the following warnings:\n\n%s", strNonfatalWarnings); \
			} else { \
				MsgBox(MB_ICONEXCLAMATION|MB_OK, "This appointment also triggers at least one unspecified warning."); \
			}  \
		} \
		return FALSE; \
	} \
	if (strNonfatalWarnings.GetLength() && -1 == strCompleteNonFatalWarnings.Find(strNonfatalWarnings)) \
		strCompleteNonFatalWarnings += strNonfatalWarnings + "\n"; \
	nTotalRuleWarningResult |= nRuleWarningResult; \
}

// (a.walling 2010-06-14 17:04) - PLID 23560 - Resource Sets
#define LINKED_VALIDATE_BY_RULES(patid, resourceid, location, date, start, end, type, purpose, ignoreapptid, fatal, nonfatal, ruleset) { \
	if (!AppointmentValidateByRules(patid, resourceid, location, date, start, end, type, purpose, ignoreapptid, TRUE, fatal, nonfatal, &nRuleWarningResult, ruleset)) { \
		if (bSilent) return FALSE; \
		if (!strFatalWarnings.IsEmpty()) { \
			if (IDYES == MsgBox(MB_ICONEXCLAMATION|MB_YESNO, "This appointment could not be saved because a linked appointment could not be moved to %s %s for the following reasons:\n\n%s\n\nWould you like to save the appointment anyway? This operation requires an administrative password.", FormatDateTimeForInterface(date, NULL, dtoDate), FormatDateTimeForInterface(start, DTF_STRIP_SECONDS, dtoTime), strFatalWarnings)) { \
				if (CheckAdministratorPassword()) { \
					nRuleWarningResult = (nRuleWarningResult | rwNeededPermission) & ~rwIsPrevented; \
					return TRUE; \
				} \
			} \
		} else { \
			if (IDYES == MsgBox(MB_ICONEXCLAMATION|MB_YESNO, "This appointment could not be saved because a linked appointment could not be moved to %s %s since it would break at least one template rule.\n\nWould you like to save the appointment anyway? This operation requires an administrative password.", FormatDateTimeForInterface(date, NULL, dtoDate), FormatDateTimeForInterface(start, DTF_STRIP_SECONDS, dtoTime))) { \
				if (CheckAdministratorPassword()) { \
					nRuleWarningResult = (nRuleWarningResult | rwNeededPermission) & ~rwIsPrevented; \
					return TRUE; \
				} \
			} \
		} \
	  	if (nRuleWarningResult & rwHasWarnings) { \
			if (!strNonfatalWarnings.IsEmpty()) { \
				TruncateSchedulerWarning(strNonfatalWarnings); \
				MsgBox(MB_ICONEXCLAMATION|MB_OK, "This appointment also triggers the following warnings:\n\n%s", strNonfatalWarnings); \
			} else { \
				MsgBox(MB_ICONEXCLAMATION|MB_OK, "This appointment also triggers at least one unspecified warning."); \
			}  \
		} \
		return FALSE; \
	} \
	if (strNonfatalWarnings.GetLength() && -1 == strCompleteNonFatalWarnings.Find(strNonfatalWarnings)) \
		strCompleteNonFatalWarnings += strNonfatalWarnings + "\n"; \
	nTotalRuleWarningResult |= nRuleWarningResult; \
}

// (a.walling 2010-06-14 16:51) - PLID 23560 - Resource Sets
//TES 2/25/2011 - PLID 42523 - Changed nLineItemOverrideID to default to -2.  -2 means that no precision template was calculated, whereas -1
// means that precision templates were checked, and the appointment isn't on one.  This is used when checking the "Schedule Outside Of Templates"
// permission; -2 means that the appointment might be on a precision template, -1 means it definitely isn't, and of course a positive
// value means that it definitely is.
BOOL AppointmentValidateByRules(long nPatientID, const CDWordArray& adwResourceID, long nLocationID, const COleDateTime &dtDate, 
							  const COleDateTime &dtStartTime, const COleDateTime &dtEndTime, 
							  long nAptType, const CDWordArray& adwAptPurpose, long nIgnoreApptID, BOOL bIgnoreLinkedAppts, BOOL bIgnoreNonLinkedAppts,
							  long nLineItemOverrideID /*= -2*/, BOOL bSilent /*= FALSE*/, int nRuleTolerance /*= -1*/)
{
	CString strCompleteNonFatalWarnings;
	long nTotalRuleWarningResult = 0;

	// (c.haag 2006-12-07 17:13) - PLID 23666 - If we require users to schedule within templates,
	// we should make sure the times are ok
	//TES 2/25/2011 - PLID 42523 - Compare on < 0 rather than -1
	if (nLineItemOverrideID < 0) {
		// (a.walling 2010-06-14 16:56) - PLID 23560 - Resource Sets
		//TES 2/25/2011 - PLID 42523 - If we have been told that this appointment is definitely not on a precision template, then 
		// tell this function not to check precision templates.
		if (!AppointmentValidateByTemplateOnlyScheduling(dtDate, dtStartTime, dtEndTime, adwResourceID, nLineItemOverrideID == -1?FALSE:TRUE, bSilent)) {
			return FALSE;
		}
	}

	if (!bIgnoreNonLinkedAppts)
	{
		if (nAptType != -1)
		{
			CString strResources, strPurposes;
			CString str;

			// (a.walling 2007-11-05 13:07) - PLID 27977 - VS2008 - for() loops
			long i = 0;

			for (i=0; i < adwResourceID.GetSize(); i++)
			{
				str.Format("%d,", adwResourceID[i]);
				strResources += str;
			} strResources.TrimRight(",");

			// (j.jones 2008-02-21 10:35) - PLID 29048 - For accuracy, make sure we are comparing a list
			// of unique purpose IDs, since our result will only include unique purpose IDs.
			// Give a warning if duplicates exist, since if we bypassed this resource/purpose/type
			// combination warning, we would just get a key violation when saving.
			CDWordArray dwLocalAptPurposes;

			for (i=0; i < adwAptPurpose.GetSize(); i++)
			{
				//quickly search and see if it is in our local array
				BOOL bFound = FALSE;
				for (int j=0; j<dwLocalAptPurposes.GetSize() && !bFound;j++) {
					if(dwLocalAptPurposes[j] == adwAptPurpose[i]) {
						bFound = TRUE;

						//If we hit this point, it means our adwAptPurpose
						//array managed to have the same purpose in it twice.
						//Find out how this happened, and fix it.
						ASSERT(FALSE);
						// (a.walling 2010-06-14 16:56) - PLID 23560 - Resource Sets
						if (!bSilent) {
							MsgBox("This appointment could not be saved because it has the same purpose selected twice. Please review your appointment purposes.");
						}
						return FALSE;
					}
				}

				//if not found, it is a unique purpose ID, so add it to our filter,
				//and our local list
				if(!bFound) {
					dwLocalAptPurposes.Add(adwAptPurpose[i]);
					str.Format("%d,", adwAptPurpose[i]);
					strPurposes += str;
				}

			}
			strPurposes.TrimRight(",");

			if (dwLocalAptPurposes.GetSize() > 0) {
				/*
				str.Format("SELECT ResourceID FROM ResourcePurposeTypeT WHERE ResourceID IN (%s) AND AptTypeID = %d AND AptPurposeID IN (%s) GROUP BY ResourceID, AptTypeID, AptPurposeID",
					strResources, nAptType, strPurposes);
				if (GetRecordCount(str) != dwLocalAptPurposes.GetSize() * adwResourceID.GetSize())
				*/
				// (a.walling 2013-02-08 12:59) - PLID 55083 - Parameterize
				// (a.wilson 2013-02-21 14:00) - PLID 55286 - Logic change caused this query not to work, change now pulls the correct count
				ADODB::_RecordsetPtr prs = CreateParamRecordset(
					"SELECT	Count(*) AS Count FROM "
					"( "
					"SELECT ResourceID	"
					"FROM ResourcePurposeTypeT "
					"WHERE ResourceID IN ({INTARRAY}) "
					"AND AptTypeID = {INT} "
					"AND AptPurposeID IN ({INTARRAY}) "
					"GROUP BY ResourceID, AptTypeID, AptPurposeID "
					") SubQ ", adwResourceID, nAptType, dwLocalAptPurposes);

				if (AdoFldLong(prs, "Count", 0) != dwLocalAptPurposes.GetSize() * adwResourceID.GetSize())
				{
					// (a.walling 2010-06-14 16:56) - PLID 23560 - Resource Sets
					if (!bSilent) {
						MsgBox("This appointment could not be saved because it violates the allowed list of appointment resource, purpose and type combinations.");
					}
					return FALSE;
				}
			}
			// (c.haag 2004-06-03 11:34) - PLID 12190 - If the appointment has no purpose, we
			// forego the resource-type-purpose allowance enforcement. Don can back me up on
			// this.
			/*else
			{
				str.Format("SELECT ResourceID FROM ResourcePurposeTypeT WHERE ResourceID IN (%s) AND AptTypeID = %d GROUP BY ResourceID, AptTypeID",
					strResources, nAptType);
				if (GetRecordCount(str) != adwResourceID.GetSize())
				{
					MsgBox("This appointment could not be saved because it violates the allowed list of appointment resource, purpose and type combinations.");
					return FALSE;
				}
			}*/
		}

		for (int i=0; i < adwResourceID.GetSize(); i++)
		{
			CString strFatalWarnings;
			CString strNonfatalWarnings;
			long nRuleWarningResult = 0;

			// (c.haag 2006-05-23 15:12) - PLID 20665 - We read in the rule set now because all it
			// needs is a date, time, duration and resource id
			// (c.haag 2007-01-02 12:47) - PLID 18602 - We now do this in a single function
			_RecordsetPtr prsRuleSet = Rule_ReadRuleSet(dtDate, dtStartTime, dtEndTime, adwResourceID[i], nLineItemOverrideID);

			if (!adwAptPurpose.GetSize())
			{
				VALIDATE_BY_RULES(nPatientID, adwResourceID[i], nLocationID, dtDate, dtStartTime, dtEndTime, nAptType, -1,
					nIgnoreApptID, &strFatalWarnings, &strNonfatalWarnings, prsRuleSet);
			}
			else
			{
				for (int j=0; j < adwAptPurpose.GetSize(); j++)
				{
					VALIDATE_BY_RULES(nPatientID, adwResourceID[i], nLocationID, dtDate, dtStartTime, dtEndTime, nAptType, adwAptPurpose[j],
						nIgnoreApptID, &strFatalWarnings, &strNonfatalWarnings, prsRuleSet);
				}
			}
		}
	}

	// Now repeat the template validation for linked appointments. This
	// code is ridiculously slow.
	if (!bIgnoreLinkedAppts)
	{
		// (a.walling 2013-02-08 12:59) - PLID 55083 - Parameterize
		_RecordsetPtr prsOldTimes = CreateParamRecordset("SELECT StartTime FROM AppointmentsT WHERE ID = {INT}", nIgnoreApptID);
		_RecordsetPtr prsLinked = CreateParamRecordset("SELECT ID, StartTime, EndTime, LocationID, [Date], AptTypeID, dbo.GetResourceIDString(ID) AS ResourceIDs, dbo.GetPurposeIDString(ID) AS PurposeIDs FROM AppointmentsT WHERE ID IN (SELECT AppointmentID FROM AptLinkT WHERE GroupID IN (SELECT GroupID FROM AptLinkT WHERE AppointmentID = {INT})) AND ID <> {INT} AND Status <> 4", nIgnoreApptID, nIgnoreApptID);

		if (!prsOldTimes->eof)
		{
			// Get the old and new start time and dates
			COleDateTime dtOldStartTime = AdoFldDateTime(prsOldTimes, "StartTime");
			COleDateTime dtNewStartTime = dtStartTime;
			dtNewStartTime.SetDateTime(dtDate.GetYear(), dtDate.GetMonth(), dtDate.GetDay(),
				dtStartTime.GetHour(), dtStartTime.GetMinute(), dtStartTime.GetSecond());

			// Get the old and new dates only so we know how many minutes the template is
			// moving against
			COleDateTime dtOldDate = dtOldStartTime;
			COleDateTime dtNewDate = dtNewStartTime;
			dtOldDate.SetDate(dtOldDate.GetYear(), dtOldDate.GetMonth(), dtOldDate.GetDay());
			dtNewDate.SetDate(dtNewDate.GetYear(), dtNewDate.GetMonth(), dtNewDate.GetDay());

			// Calculate the number of minutes we're actually moving the linked appointments by
			long nLinkedMinutesDiff;
			if (dtNewDate == dtOldDate)
				nLinkedMinutesDiff = GetSafeTotalMinuteDiff(dtNewStartTime, dtOldStartTime);
			else
			{
				if (dtNewDate > dtOldDate)
					nLinkedMinutesDiff = (dtNewDate - dtOldDate).GetDays() * 60 * 24;
				else
					nLinkedMinutesDiff = -(dtOldDate - dtNewDate).GetDays() * 60 * 24;
			}

			// Don't do all this if the appointment didn't change chronologically
			if (nLinkedMinutesDiff != 0)
			{
				FieldsPtr f = prsLinked->Fields;
				while (!prsLinked->eof)
				{
					CDWordArray adwLinkedResources, adwLinkedPurposes;
					CString strResourceIDs = AdoFldString(f, "ResourceIDs", "");
					CString strPurposeIDs = AdoFldString(f, "PurposeIDs", "");
					strResourceIDs.TrimRight();
					strPurposeIDs.TrimRight();

					// (c.haag 2006-05-23 11:55) - PLID 20665 - We now get the lists from the initial recordset
					// Get the list of purposes for this linked appointment
					//_RecordsetPtr prsPurposes = CreateRecordset("SELECT PurposeID FROM AppointmentPurposeT WHERE AppointmentID = %d",
					//	AdoFldLong(prsLinked, "ID"));
					//while (!prsPurposes->eof)
					//{
					//	adwLinkedPurposes.Add(AdoFldLong(prsPurposes, "PurposeID"));
					//	prsPurposes->MoveNext();
					//}
					//prsPurposes->Close();

					// Get the list of resources for this linked appointment
					//_RecordsetPtr prsResources = CreateRecordset("SELECT ResourceID FROM AppointmentResourceT WHERE AppointmentID = %d",
					//	AdoFldLong(prsLinked, "ID"));
					//while (!prsResources->eof)
					//{
					//	adwLinkedResources.Add(AdoFldLong(prsResources, "ResourceID"));
					//	prsResources->MoveNext();
					//}
					//prsResources->Close();
					LoadResourceIDStringIntoArray(strResourceIDs, adwLinkedResources);
					LoadPurposeIDStringIntoArray(strPurposeIDs, adwLinkedPurposes);

					// Validate this linked appointment
					COleDateTime dtDate, dtStart, dtEnd;
					dtStart = AdoFldDateTime(f, "StartTime") + COleDateTimeSpan(0,0,nLinkedMinutesDiff,0);
					dtEnd = AdoFldDateTime(f, "EndTime") + COleDateTimeSpan(0,0,nLinkedMinutesDiff,0);
					dtDate.SetDateTime(dtStart.GetYear(), dtStart.GetMonth(), dtStart.GetDay(), 0,0,0);

					//DRT 6/27/03 - If we move an appointment by x minutes, it *may* push a linked appointment into an invalid state, such
					//		as a start time at 11 pm and an end time of 1 am.  This is not allowed.  It may, however, push an appt to another
					//		day.  Because this is something that may be wanted (dragging in the week view, they move appt 3 days ahead), we need
					//		to allow them to do this.
					{
						COleDateTime dtCurStart, dtCurEnd;

						dtCurStart.SetDateTime(dtStart.GetYear(), dtStart.GetMonth(), dtStart.GetDay(), 0, 0, 0);	//don't want times
						dtCurEnd.SetDateTime(dtEnd.GetYear(), dtEnd.GetMonth(), dtEnd.GetDay(), 0, 0, 0);	//don't want times

						//now do comparisons.  We need to make sure that the current start time and the current end time are on the same day
						if(dtCurStart != dtCurEnd) {
							// (a.walling 2010-06-14 16:56) - PLID 23560 - Resource Sets
							if (!bSilent) {
								MsgBox("By moving this appointment, at least one linked appointment would be moved into an invalid state.  The start date "
									"and end date of the appointment cannot be on different days.");
							}
							return FALSE;
						}
					}

					//If we're moving in hourly increments, make sure the day doesn't change
					if ((nLinkedMinutesDiff < 0 ? -nLinkedMinutesDiff : nLinkedMinutesDiff) < 60*24)
					{
						COleDateTime dtOldStart = AdoFldDateTime(f, "StartTime");
						dtOldStart.SetDate(dtOldStart.GetYear(), dtOldStart.GetMonth(), dtOldStart.GetDay());
						if (dtOldStart != dtDate)
						{
							// (a.walling 2010-06-14 16:56) - PLID 23560 - Resource Sets
							if (!bSilent) {
								MsgBox("By moving this appointment, at least one linked appointment would be moved to a different day.  If you move an appointment "
									"by less than one day, then all linked appointments on the same day must remain on the same day.");
							}
							return FALSE;
						}
					}

					//and then trim the date part off these, we only want to look at times
					dtStart.SetTime(dtStart.GetHour(), dtStart.GetMinute(), dtStart.GetSecond());
					dtEnd.SetTime(dtEnd.GetHour(), dtEnd.GetMinute(), dtEnd.GetSecond());


					//AppointmentValidateByRules(nPatientID, adwLinkedResources, AdoFldLong(f, "LocationID"),
					//	dtDate, dtStart, dtEnd, AdoFldLong(f, "AptTypeID"), adwLinkedPurposes, AdoFldLong(f, "ID"), TRUE);

					for (int i=0; i < adwLinkedResources.GetSize(); i++)
					{
						CString strFatalWarnings;
						CString strNonfatalWarnings;
						long nRuleWarningResult = 0;


						// (c.haag 2006-05-23 15:12) - PLID 20665 - We read in the rule set now because all it
						// needs is a date, time, duration and resource id
						// (c.haag 2007-01-02 12:47) - PLID 18602 - We now do this from a single function
						_RecordsetPtr prsRuleSet = Rule_ReadRuleSet(dtDate, dtStart, dtEnd, adwLinkedResources[i], -1);

						if (!adwLinkedPurposes.GetSize())
						{
							LINKED_VALIDATE_BY_RULES(nPatientID, adwLinkedResources[i], AdoFldLong(f, "LocationID"), dtDate, dtStart, dtEnd, AdoFldLong(f, "AptTypeID", -1), -1,
								AdoFldLong(f, "ID"), &strFatalWarnings, &strNonfatalWarnings, prsRuleSet);
						}
						else
						{
							for (int j=0; j < adwLinkedPurposes.GetSize(); j++)
							{
								// (z.manning 2008-08-07 11:53) - PLID 30981 - adwLinkedPurposes needs to use j as an index, not i!
								LINKED_VALIDATE_BY_RULES(nPatientID, adwLinkedResources[i], AdoFldLong(f, "LocationID"), dtDate, dtStart, dtEnd, AdoFldLong(f, "AptTypeID", -1), adwLinkedPurposes[j],
									AdoFldLong(f, "ID"), &strFatalWarnings, &strNonfatalWarnings, prsRuleSet);
							}
						}
					}
					prsLinked->MoveNext();
				}
			}
		}
	}

	// (c.haag 2004-03-17 12:12) PL 11453 - Optional warning for two appointments
	// for the same resource and a different location
	long nDoubleBookDiffLocations = GetRemotePropertyInt("DoubleBookDiffLocations", 0, 0, "<None>", true);

	// (c.haag 2006-05-11 10:01) - PLID 20580 - The recordset within this conditional will always be empty if
	// nIgnoreApptID is -1
	if (1 /* Always book*/ != nDoubleBookDiffLocations && -1 != nIgnoreApptID)
	{
		COleDateTime dtS, dtE;
		dtS.SetDateTime(dtDate.GetYear(), dtDate.GetMonth(), dtDate.GetDay(),
				dtStartTime.GetHour(), dtStartTime.GetMinute(), dtStartTime.GetSecond());
		dtE.SetDateTime(dtDate.GetYear(), dtDate.GetMonth(), dtDate.GetDay(),
				dtEndTime.GetHour(), dtEndTime.GetMinute(), dtEndTime.GetSecond());

		//_RecordsetPtr prs = CreateRecordset("SELECT TOP 1 ID FROM AppointmentsT WHERE ID <> %d AND Status <> 4 AND LocationID <> %d AND "
		//	"NOT ((StartTime < '%s' AND EndTime <= '%s') OR (StartTime >= '%s' AND EndTime > '%s')) AND ID IN " // It must overlap the times of the would-be appointment
		//	"(SELECT AppointmentID FROM AppointmentResourceT WHERE ResourceID IN (SELECT ResourceID FROM AppointmentResourceT WHERE AppointmentID = %d)) ", // Same resource
		//	nIgnoreApptID, nLocationID,
		//	FormatDateTimeForSql(dtS, dtoDateTime), FormatDateTimeForSql(dtS, dtoDateTime),
		//	FormatDateTimeForSql(dtE, dtoDateTime), FormatDateTimeForSql(dtE, dtoDateTime),
		//	nIgnoreApptID);

		// (a.walling 2013-02-08 13:04) - PLID 55084 - Avoid large appointment index scans in the scheduler
		// By filtering the StartTime to the day in question, we avoid the scan of the entire AppointmentsT table
		BOOL bOverlapsAppt = ReturnsRecordsParam(
			"SELECT TOP 1 ID "
			"FROM AppointmentsT "
			"WHERE ID <> {INT} "
			"AND Status <> 4 "
			"AND LocationID <> {INT} "
			"AND StartTime >= {OLEDATETIME} "
			"AND StartTime < {OLEDATETIME} "
			"AND NOT ( "
				"(StartTime < {OLEDATETIME} AND EndTime <= {OLEDATETIME}) "
				"OR "
				"(StartTime >= {OLEDATETIME} AND EndTime > {OLEDATETIME}) "
			") "
			"AND ID IN ( "
				"SELECT AppointmentID "
				"FROM AppointmentResourceT "
				"WHERE ResourceID IN ( "
					"SELECT ResourceID "
					"FROM AppointmentResourceT "
					"WHERE AppointmentID = {INT} "
				") "
			")"
			, nIgnoreApptID
			, nLocationID
			, AsDateNoTime(dtS)
			, AsDateNoTime(dtE + COleDateTimeSpan(1, 0, 0, 0))
			, dtS, dtS
			, dtE, dtE
			, nIgnoreApptID
		);

		if (bOverlapsAppt)
		{
			switch (nDoubleBookDiffLocations)
			{
			case 0: // Prompt
				strCompleteNonFatalWarnings += "This appointment is being booked for a resource that has another appointment scheduled at the same time but at a different location.\n";
				nTotalRuleWarningResult |= rwHasWarnings;
				break;
			case 2: // Prevent
				// (a.walling 2010-06-14 16:56) - PLID 23560 - Resource Sets
				if (!bSilent) {
					MsgBox(MB_ICONEXCLAMATION, "This appointment could not be saved because it is being booked for a resource that has another appointment scheduled at the same time but at a different location.");
				}
				nTotalRuleWarningResult |= rwIsPrevented;
				return FALSE;
			}
		}
	}

	// (a.walling 2010-06-14 16:56) - PLID 23560 - Resource Sets
	// Now display the warnings
	if (!bSilent && nTotalRuleWarningResult & rwHasWarnings) {
		UINT nResult = IDNO;
		if (!strCompleteNonFatalWarnings.IsEmpty()) {
			TruncateSchedulerWarning(strCompleteNonFatalWarnings);
			nResult = MsgBox(MB_ICONEXCLAMATION|MB_YESNO, "This action triggers the following warnings:\n\n%s\nWould you like to save it anyway?", strCompleteNonFatalWarnings);
		} else {
			nResult = MsgBox(MB_ICONEXCLAMATION|MB_YESNO, "This action triggers at least one unspecified warning.\n\nWould you like to save it anyway?");
		}
		if (nResult == IDYES) {
			// The user wants to save in spite of the warnings
			return TRUE;
		} else {
			// The user did not want to save because of the warnings
			return FALSE;
		}
	}

	if (bSilent && nRuleTolerance != -1) {
		if (nRuleTolerance == 0) {
			if (nTotalRuleWarningResult & (rwHasWarnings | rwIsPrevented)) {
				return FALSE;
			}
		} else if (nRuleTolerance == rwHasWarnings) {
			if (nTotalRuleWarningResult & rwIsPrevented) {
				return FALSE;
			}
		} else if (nRuleTolerance == rwIsPrevented) {
			ASSERT(FALSE); // Asserting here in case someone does this accidentally while developing.
			return TRUE;
		}
	}

	return TRUE;
}
///////////////////////////////////

BOOL AppointmentValidateByRules(long nPatientID, long nResourceID, long nLocationID, const COleDateTime &dtDate, 
							  const COleDateTime &dtStartTime, const COleDateTime &dtEndTime, 
							  long nAptType, long nAptPurpose, long nIgnoreApptID, BOOL bSilent /*= FALSE*/, OUT CString *pstrFatalWarnings /*= NULL*/, OUT CString *pstrNonfatalWarnings /*= NULL*/, OUT long *pnRuleWarningResult /*= NULL*/,
							  ADODB::_RecordsetPtr prsRuleSet /*= NULL */)
{
	// Get the rule warning information
	CString strFatalWarnings, strNonfatalWarnings;

	// Make sure that, one way or the other, we have a place to dump the warnings.
	if (!pstrFatalWarnings) pstrFatalWarnings = &strFatalWarnings;
	if (!pstrNonfatalWarnings) pstrNonfatalWarnings = &strNonfatalWarnings;

	long nRuleWarningResult = AppointmentReadRules(
		nPatientID, nResourceID, nLocationID, dtDate, dtStartTime, dtEndTime, 
		nAptType, nAptPurpose, nIgnoreApptID, FALSE, pstrFatalWarnings, pstrNonfatalWarnings,
		prsRuleSet);

	if (pnRuleWarningResult)
		*pnRuleWarningResult = nRuleWarningResult;

	// Tell the user why they can't or shouldn't make an appointment
	if (nRuleWarningResult & rwIsPrevented) {
		if (!pstrFatalWarnings->IsEmpty()) {
			TruncateSchedulerWarning(*pstrFatalWarnings);
			if (!bSilent) MsgBox(MB_ICONEXCLAMATION|MB_OK, "This appointment could not be saved for the following reasons:\n\n%s", *pstrFatalWarnings);
		} else {
			if (!bSilent) MsgBox(MB_ICONEXCLAMATION|MB_OK, "This appointment could not be saved because it breaks at least one template rule.");
		}
		if (nRuleWarningResult & rwHasWarnings) {
			if (!pstrNonfatalWarnings->IsEmpty()) {
				TruncateSchedulerWarning(*pstrNonfatalWarnings);
				if (!bSilent) MsgBox(MB_ICONEXCLAMATION|MB_OK, "This appointment also triggers the following warnings:\n\n%s", *pstrNonfatalWarnings);
			} else {
				if (!bSilent) MsgBox(MB_ICONEXCLAMATION|MB_OK, "This appointment also triggers at least one unspecified warning.");
			}
		}
		return FALSE;
	} else if (nRuleWarningResult & rwHasWarnings) {
		UINT nResult = IDNO;
		if (!bSilent)
		{
			if (!pstrNonfatalWarnings->IsEmpty()) {
				TruncateSchedulerWarning(*pstrNonfatalWarnings);
				nResult = MsgBox(MB_ICONEXCLAMATION|MB_YESNO, "This appointment triggers the following warnings:\n\n%s\n\nWould you like to save it anyway?", *pstrNonfatalWarnings);
			} else {
				nResult = MsgBox(MB_ICONEXCLAMATION|MB_YESNO, "This appointment triggers at least one unspecified warning.\n\nWould you like to save it anyway?");
			}
		}
		else
		{
			// If we are running silent, we assume the user wants to save in spite of
			// the warnings, or that the caller will handle the act of asking the user.
			nResult = IDYES;
		}
		if (nResult == IDYES) {
			// The user wants to save in spite of the warnings
			return TRUE;
		} else {
			// The user did not want to save because of the warnings
			return FALSE;
		}
	} else {
		return TRUE;
	}
}

BOOL AppointmentValidateDoubleBooking(COleDateTime dtStart, COleDateTime dtEnd, CDWordArray& adwResources, long nIgnoreApptID /*= -1*/)
{
	//(e.lally 2006-12-18) PLID 21543 - This code was needed in the FFA too so I pulled it into its own function.


	// CAH 5/5/2003: Check double booking permissions to decide if we
	// want to commit the appointment
	if (!(GetCurrentUserPermissions(bioAppointment) & sptDynamic3))
	{
		CString strSQL;

		// Get a list of all the appointments that would double book with this one
		//DRT 12/2/2003 - PLID 10276 - Fixed the query to no longer select items that were cancelled.
		/*strSQL.Format(
			"SELECT ID "
			"FROM AppointmentsT "
			"WHERE Status <> 4 "
			"AND ("
				"(StartTime <= '%s' AND EndTime > '%s') "
				"OR "
				"(StartTime < '%s' AND EndTime >= '%s')"
			") "
			"AND ID <> %d "
			"AND ID IN ("
				"SELECT AppointmentID "
				"FROM AppointmentResourceT "
				"WHERE ResourceID = %d",
			FormatDateTimeForSql(dtStart, dtoDateTime), FormatDateTimeForSql(dtStart, dtoDateTime), 
			FormatDateTimeForSql(dtEnd, dtoDateTime), FormatDateTimeForSql(dtEnd, dtoDateTime), nIgnoreApptID, adwResources[0]);

		for (long i=1; i < adwResources.GetSize(); i++)
		{
			CString str;
			str.Format(" OR ResourceID = %d", adwResources[i]);
			strSQL += str;
		}
		strSQL += ")";

		_RecordsetPtr prsAppts = CreateRecordset(strSQL);	*/		

		// If at least one appointment exists that can be double-booked against, we need to
		// formally check the permissions so that if it's flagged to require a password, the
		// user will be prompted for it.
		// (a.walling 2013-02-08 13:04) - PLID 55084 - Avoid large appointment index scans in the scheduler
		// By filtering the StartTime to the day in question, we avoid the scan of the entire AppointmentsT table
		BOOL bDoubleBooked = ReturnsRecordsParam(
			"SELECT ID "
			"FROM AppointmentsT "
			"WHERE Status <> 4 "
			// Same day
			"AND StartTime >= {OLEDATETIME} "
			"AND StartTime < {OLEDATETIME} "
			"AND ("
				"(StartTime <= {OLEDATETIME} AND EndTime > {OLEDATETIME}) "
				"OR "
				"(StartTime < {OLEDATETIME} AND EndTime >= {OLEDATETIME})"
			") "
			"AND ID <> {INT} "
			"AND ID IN ("
				"SELECT AppointmentID "
				"FROM AppointmentResourceT "
				"WHERE ResourceID IN ({INTARRAY}) "
			")"
			, AsDateNoTime(dtStart)
			, AsDateNoTime(dtStart + COleDateTimeSpan(1, 0, 0, 0))
			, dtStart, dtStart
			, dtEnd, dtEnd
			, nIgnoreApptID
			, adwResources
		);

		if (bDoubleBooked)
		{
			//DRT 12/2/2003 - PLID 10276 - Fixed this to compare the permissions ourself - this way we only give
			//	1 prevent message instead of 2.
			//If we reached this point, we know we do not have permission (see the if above the try).  Therefore, we
			//need to test permissions.  If they have password permission set, we need to do the password prompt box.
			//Otherwise, just give them a good error message so they know why they can't make the appointment.
			bool bPassed = false;
			if(GetCurrentUserPermissions(bioAppointment) & sptDynamic3WithPass)
			{
				//they have passworded permission, we need to prompt them
				if(CheckCurrentUserPassword("Please Enter Password"))
					bPassed = true;
			}
			if(!bPassed) {
				MsgBox("You do not have permission to double-book appointments. Please see your office manager for assistance.");
				return FALSE;
			}
		}
	}
	return TRUE;
}

BOOL AppointmentValidateByPermissions(long nPatientID, const CDWordArray& adwResourceID, long nLocationID, const COleDateTime &dtDate, 
							  const COleDateTime &dtStartTime, const COleDateTime &dtEndTime, 
							  long nAptType, const CDWordArray& adwAptPurpose, long nIgnoreApptID)
{
	BOOL bSuccess = TRUE;

	// CAH 5/5/2003: This should never happen, but we need safeguards.
	if (!adwResourceID.GetSize())
		return TRUE;

	//TES 1/14/2010 - PLID 36762 - If this patient is blocked, we can't modify it.
	if(!GetMainFrame()->CanAccessPatient(nPatientID, false)) {
		return FALSE;
	}

	// CAH 5/7/2003: Blow up if we require an appointment type, but there is none
	// MSC 7/17/2003:We should also make sure that this is a new appt since that is what the permission says
	//DRT 8/22/2005 - PLID 15299 - This particular thing has come up 3 times now and we keep messing with the implementation.  PLID 8811 was a request to 
	//	clean this up for all appts, but not to hassle people who just open & close an old appointment which might have no type.  We failed in doing that, 
	//	and instead it was changed to only affect new appts.  What should happen:
	// - If appt is new, require a type or purpose as directed by the preference.
	// - If appt is existing, and type or purpose is "no type" or "no purpose", do not require them to have a type.  This is old legacy data.  Once they set a type
	//		for the first time, it then becomes the next case.
	// - If appt is existing, and type or purpose is set, then we do require them to keep a purpose or type.  This is a modification of existing data.

	// (d.thompson 2012-06-27) - PLID 51220 - Changed default to Yes
	if(GetRemotePropertyInt("ResEntryRequireAptType", 1, 0, "<None>", true))
	{
		if (nAptType < 1)
		{
			//DRT 8/22/2005 - PLID 15299 - If this appointment previously had no type (legacy data), then we allow this to happen
			// (a.walling 2010-10-19 09:45) - PLID 40965 - Use ReturnsRecordsParam
			if(nIgnoreApptID == -1 || ReturnsRecordsParam("SELECT TOP 1 ID FROM AppointmentsT WHERE ID = {INT} AND AptTypeID IS NOT NULL", nIgnoreApptID)) {
 				AfxMessageBox("You must select an appointment type before saving this appointment");
				return FALSE;
			}
		}
	}
	// CAH 5/7/2003: Utterly fail if we require an appointment purpose, but there is none
	// MSC 7/17/2003:We should also make sure that this is a new appt since that is what the permission says
	//DRT 8/22/2005 - PLID 15299 - See notes above.
	// (d.thompson 2012-06-27) - PLID 51220 - Changed default to Yes
	if(GetRemotePropertyInt("ResEntryRequireAptPurpose", 1, 0, "<None>", true))
	{
		if (!adwAptPurpose.GetSize())
		{
			//DRT 8/22/2005 - PLID 15299 - If this appointment previously had no purposes (legacy data), then we will allow
			//	despite the preference.
			// (a.walling 2010-10-19 09:45) - PLID 40965 - Use ReturnsRecordsParam
			if(nIgnoreApptID == -1 || ReturnsRecordsParam("SELECT TOP 1 ID FROM AppointmentPurposeT WHERE AppointmentID = {INT}", nIgnoreApptID)) {
				AfxMessageBox("You must select at least one appointment purpose before saving this appointment");
				return FALSE;
			}
		}
	}

	// (j.jones 2005-04-18 14:34) - PLID 15812 - Check for the ability to save the current procedure for the resource
	if(!CheckWarnProcedureCredentials(adwResourceID, adwAptPurpose))
		return FALSE;

	//DRT 5/7/03 - Warn them for insurance authorizations.  If they said "no" to the warning, we don't want to save the appointment
	//		This really has nothing to do with permissions, but this warning should be popping up anywhere you'd be checking for
	//		double booking, so it's a logical place to put it
	//DRT 6/10/03 - Someone removed the check on bNeedToCheckAppt in the other overloaded ValidateByPermissions function, which is the 
	//		only reason this code was here.  Now it's showing the warning 2x, because this function calls another function that warns 
	//		them.  So no more calling the function here.
	//	if(!AttemptWarnForInsAuth(nPatientID, dtDate))
	//		return FALSE;

	// CAH 5/5/2003: Check double booking permissions to decide if we
	// want to commit the appointment

	//(e.lally 2006-12-19) PLID 21543 - Pulled the logic for validating the appointment against double booking
	//permissions into its own function.
	COleDateTime dtStart, dtEnd;
	
	CDWordArray adwResources;
	adwResources.Copy(adwResourceID);
	try{
		dtStart.SetDateTime(dtDate.GetYear(), dtDate.GetMonth(), dtDate.GetDay(),
			dtStartTime.GetHour(), dtStartTime.GetMinute(), dtStartTime.GetSecond());
		dtEnd.SetDateTime(dtDate.GetYear(), dtDate.GetMonth(), dtDate.GetDay(),
		dtEndTime.GetHour(), dtEndTime.GetMinute(), dtEndTime.GetSecond());
		if(!AppointmentValidateDoubleBooking(dtStart, dtEnd, adwResources, nIgnoreApptID))
			return FALSE;
	}
#ifdef _USRDLL
	NxCatchAll_WithParent(NULL, "Error determining if an appointment is double-booked");
#else
	NxCatchAll("Error determining if an appointment is double-booked");
#endif

	// JMJ 6/5/2003: Check resource permissions to decide if we can save based on this resource
	try {
		
		CDWordArray adwResourceNames;
		BOOL bPasswordPassed = TRUE;
		BOOL bContinue = TRUE;
		// (a.walling 2007-11-05 13:07) - PLID 27977 - VS2008 - for() loops
		long i = 0;

		for (i=0; i < adwResourceID.GetSize(); i++) {
			CPermissions perm = GetCurrentUserPermissions(bioSchedIndivResources,TRUE,(long)adwResourceID[i]);
			if(!(perm & (sptWrite|sptWriteWithPass))) {
				adwResourceNames.Add((DWORD)adwResourceID[i]);
				bContinue = FALSE;
			}
			if(!bPasswordPassed && (perm & sptWriteWithPass)) {
				//if any resource needs a password, only prompt once
				bPasswordPassed = CheckCurrentUserPermissions(bioSchedIndivResources,sptWrite,TRUE,(long)adwResourceID[i]);
			}
		}
		if(!bContinue || !bPasswordPassed) {
			CString str, strResourceNames;
			for(i=0; i < adwResourceNames.GetSize(); i++) {
				// (a.walling 2013-02-08 12:59) - PLID 55083 - Parameterize
				_RecordsetPtr rs = CreateParamRecordset("SELECT Item FROM ResourceT WHERE ID = {INT}",adwResourceNames[i]);
				if(!rs->eof) {
					strResourceNames += AdoFldString(rs, "Item","");
					strResourceNames += ", ";
				}
				rs->Close();
			}
			strResourceNames.TrimRight(", ");

			str.Format("You do not have permission to schedule appointments for %s. Please see your office manager for assistance.",strResourceNames);
			MsgBox(str);
			bSuccess = FALSE;
		}
	}
#ifdef _USRDLL
	NxCatchAll_WithParent(NULL, "Error determining if an appointment is double-booked");
#else
	NxCatchAll("Error determining if an appointment is double-booked");
#endif
	return bSuccess;
}

BOOL AppointmentValidateByPermissions(_RecordsetPtr pAppt)
{
	//BOOL bNeedToCheckAppt = FALSE;

	FieldsPtr pflds = pAppt->Fields;

	// Whenever you add logic for appointment validation relating to permissions, you need
	// to filter it below like I did with double booking. This is a speed improvement.

	/* JMJ 6/5/2003 - Unfortunately, this if statement is out of date because we have to check
	//the resources for the resource permissions.

	// CAH 5/5/2003: Check double booking permission before we even use precious SQL resources.
	if (!(GetCurrentUserPermissions(bioAppointment) & (SPT_________3_)))
		bNeedToCheckAppt = TRUE;
	*/

	//DRT 5/7/03 - Warn them for insurance authorizations.  If they said "no" to the warning, we don't want to save the appointment
	//		This really has nothing to do with permissions, but this warning should be popping up anywhere you'd be checking for
	//		double booking, so it's a logical place to put it
	//		The recordset (pAppt) is already open, so we're not using much resources to read the fields out of it
	if(!AttemptWarnForInsAuth(AdoFldLong(pflds, "PatientID"), AdoFldDateTime(pflds, "Date")))
		return FALSE;

	// If this user has permissions such that they could potentially prevent the appointment
	// from being saved, we need to check them one by one
	//if (bNeedToCheckAppt)	{
		try {
			CDWordArray adwResourceID;
			CDWordArray adwPurposeID;
			long nApptID = AdoFldLong(pflds, "ID", -1);

			// (a.walling 2013-02-08 12:59) - PLID 55083 - Parameterize
			_RecordsetPtr rs = CreateParamRecordset("SELECT ResourceID FROM AppointmentResourceT WHERE AppointmentID = {INT}", nApptID);
			while (!rs->eof)
			{
				adwResourceID.Add(AdoFldLong(rs, "ResourceID"));
				rs->MoveNext();
			}
			rs->Close();
			rs.Detach();

			// (a.walling 2013-02-08 12:59) - PLID 55083 - Parameterize
			rs = CreateParamRecordset("SELECT PurposeID FROM AppointmentPurposeT WHERE AppointmentID = {INT}", nApptID);
			while (!rs->eof)
			{
				adwPurposeID.Add(AdoFldLong(rs, "PurposeID"));
				rs->MoveNext();
			}
			rs->Close();
			rs.Detach();

			return AppointmentValidateByPermissions(
				AdoFldLong(pflds, "PatientID"), 
				adwResourceID, 
				AdoFldLong(pflds, "LocationID"),
				AdoFldDateTime(pflds, "Date"),
				AdoFldDateTime(pflds, "StartTime"),
				AdoFldDateTime(pflds, "EndTime"),
				AdoFldLong(pflds, "AptTypeID", -1),
				adwPurposeID,
				nApptID);
		}
#ifdef _USRDLL
		NxCatchAll_WithParent(NULL, "Error in AppointmentValidateByPermissions");
#else
		NxCatchAll("Error in AppointmentValidateByPermissions");
#endif
	return TRUE;
}

BOOL AppointmentValidateByAlarms(ADODB::_RecordsetPtr pAppt, long nResIDToIgnore /*= -1*/)
{
	FieldsPtr pflds = pAppt->Fields;
	CDWordArray adwPurposeID, adwResourceID;
	long nApptID = AdoFldLong(pflds, "ID", -1);

	try {
		_RecordsetPtr rs = CreateRecordset("SELECT PurposeID FROM AppointmentPurposeT WHERE AppointmentID = %d", nApptID);
		while (!rs->eof)
		{
			adwPurposeID.Add(AdoFldLong(rs, "PurposeID"));
			rs->MoveNext();
		}
		rs->Close();
		rs.Detach();

		rs = CreateRecordset("SELECT ResourceID FROM AppointmentResourceT WHERE AppointmentID = %d", nApptID);
		while (!rs->eof)
		{
			adwResourceID.Add(AdoFldLong(rs, "ResourceID"));
			rs->MoveNext();
		}
		rs->Close();
		rs.Detach();

		return AppointmentValidateByAlarms(
			AdoFldLong(pflds, "PatientID"), 
			AdoFldDateTime(pflds, "Date"),
			AdoFldDateTime(pflds, "StartTime"),
			AdoFldLong(pflds, "AptTypeID", -1),
			adwPurposeID, adwResourceID,
			nApptID,
			nResIDToIgnore);
	}
#ifdef _USRDLL
	NxCatchAllCallThrow_WithParent(NULL, "Error in AppointmentValidateByAlarms", {});
#else
	NxCatchAllCallThrow("Error in AppointmentValidateByAlarms", {});
#endif
	return FALSE; // Should never get here
}

// (j.jones 2009-08-12 12:48) - PLID 25230 - I cleaned up the wording of these messages,
// and removed bSilent, because it was always true
#define VALIDATE_BY_ALARMS(patid, date, type, purpose, resource, ignoreapptid1, ignoreapptid2, fatal, nonfatal) { \
	if (!AppointmentValidateByAlarms(patid, date, type, purpose, resource, ignoreapptid1, fatal, nonfatal, &nAlarmWarningResult, ignoreapptid2)) { \
		if (!strFatalWarnings.IsEmpty()) { \
			TruncateSchedulerWarning(strFatalWarnings); \
			MsgBox(MB_ICONEXCLAMATION|MB_OK, "This appointment could not be saved because it violates the following booking alarms:\n\n%s", strFatalWarnings); \
		} else { \
			MsgBox(MB_ICONEXCLAMATION|MB_OK, "This appointment could not be saved because it violates at least one booking alarm."); \
		} \
	  	if (nAlarmWarningResult & rwHasWarnings) { \
			if (!strNonfatalWarnings.IsEmpty()) { \
				TruncateSchedulerWarning(strNonfatalWarnings); \
				MsgBox(MB_ICONEXCLAMATION|MB_OK, "This appointment also triggers the following booking alarm warnings:\n\n%s", strNonfatalWarnings); \
			} else { \
				MsgBox(MB_ICONEXCLAMATION|MB_OK, "This appointment also triggers at least one unspecified booking alarm warning."); \
			}  \
		} \
		return FALSE; \
	} \
	if (strNonfatalWarnings.GetLength() && -1 == strCompleteNonFatalWarnings.Find(strNonfatalWarnings)) \
		strCompleteNonFatalWarnings += strNonfatalWarnings + "\n"; \
	nTotalAlarmWarningResult |= nAlarmWarningResult; \
}

BOOL AppointmentValidateByAlarms(ADODB::_RecordsetPtr pAppt, OUT CString *pstrFatalWarnings, OUT CString *pstrNonfatalWarnings, OUT long *pnRuleWarningResult, long nResIDToIgnore /*= -1*/)
{
	FieldsPtr pflds = pAppt->Fields;
	CDWordArray adwPurposeID, adwResourceID;
	long nApptID = AdoFldLong(pflds, "ID", -1);
	long nPatientID = AdoFldLong(pflds, "PatientID");
	long nLocationID = AdoFldLong(pflds, "LocationID");
	COleDateTime dtDate = AdoFldDateTime(pflds, "Date");
	COleDateTime dtStartTime = AdoFldDateTime(pflds, "StartTime");
	COleDateTime dtEndTime = AdoFldDateTime(pflds, "EndTime");
	CString strFatalWarnings;
	CString strNonfatalWarnings;
	long nRuleWarningResult = 0;
	long nAptType = AdoFldLong(pflds, "AptTypeID", -1);
	BOOL bRes = TRUE;

	_RecordsetPtr rs = CreateParamRecordset("SELECT PurposeID FROM AppointmentPurposeT WHERE AppointmentID = {INT}", nApptID);
	while (!rs->eof)
	{
		adwPurposeID.Add(AdoFldLong(rs, "PurposeID"));
		rs->MoveNext();
	}
	rs->Close();
	rs.Detach();

	rs = CreateParamRecordset("SELECT ResourceID FROM AppointmentResourceT WHERE AppointmentID = {INT}", nApptID);
	while (!rs->eof)
	{
		adwResourceID.Add(AdoFldLong(rs, "ResourceID"));
		rs->MoveNext();
	}
	rs->Close();
	rs.Detach();

	if (!adwPurposeID.GetSize())
	{
		// (j.jones 2009-08-12 12:52) - PLID 25230 - removed bSilent, because it was always true
		if (!AppointmentValidateByAlarms(nPatientID, dtDate, nAptType, -1, adwResourceID, nApptID, pstrFatalWarnings, pstrNonfatalWarnings, pnRuleWarningResult, nResIDToIgnore))
		{
			bRes = FALSE;
		}
	}
	else
	{
		for (int j=0; j < adwPurposeID.GetSize(); j++)
		{
			// (j.jones 2009-08-12 12:52) - PLID 25230 - removed bSilent, because it was always true
			if (!AppointmentValidateByAlarms(nPatientID, dtDate, nAptType, adwPurposeID[j], adwResourceID, nApptID, pstrFatalWarnings, pstrNonfatalWarnings, pnRuleWarningResult, nResIDToIgnore))
			{
				bRes = FALSE;
			}
		}
	}
	return bRes;
}

BOOL AppointmentValidateByAlarms(long nPatientID, const COleDateTime &dtDate, const COleDateTime& dtTime, long nAptType, const CDWordArray& adwAptPurpose, const CDWordArray& adwAptResource, long nIgnoreApptID1, long nIgnoreApptID2 /*= -1*/)
{
	//TES 4/20/2004 - PLID 15782 - We do actually want to check if it's in the past.
	//don't bother checking if it is the -25 patient
	/*if(nPatientID == -25)
		return TRUE;*/

	CString strCompleteNonFatalWarnings;
	long nTotalAlarmWarningResult = 0;

	CString strFatalWarnings;
	CString strNonfatalWarnings;
	long nAlarmWarningResult = 0;

	if(nPatientID != -25) {
		if (!adwAptPurpose.GetSize())
		{
			VALIDATE_BY_ALARMS(nPatientID, dtDate, nAptType, -1, adwAptResource, 
				nIgnoreApptID1, nIgnoreApptID2, &strFatalWarnings, &strNonfatalWarnings);
		}
		else
		{
			for (int j=0; j < adwAptPurpose.GetSize(); j++)
			{
				VALIDATE_BY_ALARMS(nPatientID, dtDate, nAptType, adwAptPurpose[j], adwAptResource, 
					nIgnoreApptID1, nIgnoreApptID2, &strFatalWarnings, &strNonfatalWarnings);
			}
		}
	}

	// (c.haag 2003-07-17 13:45) - Warn if the appointment is made in the past
	if(GetRemotePropertyInt("WarnOnMakingPastAppt", 0, 0, "<None>", true))
	{
		COleDateTime dt(dtDate.GetYear(), dtDate.GetMonth(), dtDate.GetDay(),
			dtTime.GetHour(), dtTime.GetMinute(), dtTime.GetSecond());
		if (dt < COleDateTime::GetCurrentTime())
		{
			nTotalAlarmWarningResult |= rwHasWarnings;
			if (strCompleteNonFatalWarnings.GetLength() > 2)
			{
				// (j.jones 2003-09-15 16:02) - Strip one of the two \n\n's
				if (strCompleteNonFatalWarnings.Right(2) == "\n\n")
					strCompleteNonFatalWarnings = strCompleteNonFatalWarnings.Left( strCompleteNonFatalWarnings.GetLength() - 1 );
			}
			strCompleteNonFatalWarnings += "You are attempting to save an appointment in the past.\n\n\n";
		}
	}

	// Now display the warnings
	if (nTotalAlarmWarningResult & rwHasWarnings) {
		UINT nResult = IDNO;
		if (!strCompleteNonFatalWarnings.IsEmpty()) {
			TruncateSchedulerWarning(strCompleteNonFatalWarnings);
			nResult = MsgBox(MB_ICONEXCLAMATION|MB_YESNO, "This action triggers the following booking alarm warnings:\n\n%sWould you like to save it anyway?", strCompleteNonFatalWarnings);
		} else {
			nResult = MsgBox(MB_ICONEXCLAMATION|MB_YESNO, "This action triggers at least one unspecified booking alarm warning.\n\nWould you like to save it anyway?");
		}
		if (nResult == IDYES) {
			// The user wants to save in spite of the warnings
			return TRUE;
		} else {
			// The user did not want to save because of the warnings
			return FALSE;
		}
	}

	return TRUE;
}

// (j.jones 2009-08-12 12:52) - PLID 25230 - removed bSilent, because it was always true
BOOL AppointmentValidateByAlarms(long nPatientID, const COleDateTime &dtDate, long nAptType, long nAptPurpose, const CDWordArray& adwAptResource, 
								 long nIgnoreApptID, OUT CString *pstrFatalWarnings /*= NULL*/, OUT CString *pstrNonfatalWarnings /*= NULL*/, OUT long *pnAlarmWarningResult /*= NULL*/, long nResIDToIgnore /*= -1*/)
{
	//don't bother checking if it is the -25 patient
	if(nPatientID == -25)
		return TRUE;

	// Get the alarm warning information
	CString strFatalWarnings, strNonfatalWarnings;

	// Make sure that, one way or the other, we have a place to dump the warnings.
	if (!pstrFatalWarnings) pstrFatalWarnings = &strFatalWarnings;
	if (!pstrNonfatalWarnings) pstrNonfatalWarnings = &strNonfatalWarnings;

	long nAlarmWarningResult = AppointmentReadAlarms(
		nPatientID, dtDate, nAptType, nAptPurpose, adwAptResource, nIgnoreApptID, pstrFatalWarnings, pstrNonfatalWarnings, nResIDToIgnore);

	if (pnAlarmWarningResult)
		*pnAlarmWarningResult = nAlarmWarningResult;

	// Tell the user why they can't or shouldn't make an appointment
	// (j.jones 2009-08-12 12:48) - PLID 25230 - I removed the warnings here, because bSilent was always true
	if (nAlarmWarningResult & rwIsPrevented) {
		if (!pstrFatalWarnings->IsEmpty()) {
			TruncateSchedulerWarning(*pstrFatalWarnings);
		}
		if (nAlarmWarningResult & rwHasWarnings) {
			if (!pstrNonfatalWarnings->IsEmpty()) {
				TruncateSchedulerWarning(*pstrNonfatalWarnings);
			}
		}
		return FALSE;
	} else if (nAlarmWarningResult & rwHasWarnings) {
		UINT nResult = IDNO;

		//the caller will ask the user if they want to save
		return TRUE;

	} else {
		return TRUE;
	}
}

long AppointmentReadAlarms(long nPatientID, const COleDateTime &dtDate, long nAptType, long nAptPurpose, const CDWordArray& adwAptResource, long nIgnoreApptID1, OUT CString *pstrFatalWarnings /*= NULL*/, OUT CString *pstrNonfatalWarnings /*= NULL*/, long nIgnoreApptID2 /*= -1*/)
{
	BOOL bValidateSuccess = TRUE;
	BOOL bWarnings = FALSE;
	if (pstrFatalWarnings) (*pstrFatalWarnings) = "";
	if (pstrNonfatalWarnings) (*pstrNonfatalWarnings) = "";

	try {
		// (j.jones 2009-08-12 12:19) - PLID 25230 - pull the alarm description
		_RecordsetPtr prs = CreateRecordset("SELECT ID, AllowSave, Description FROM AptBookAlarmT");
		FieldsPtr flds = prs->Fields;
		while (!prs->eof) {
			// Find out whether the appointment is valid or not based on the rule
			CString strConflictingAppts;
			EProcessAlarmResult nResult = ProcessAlarm(AdoFldLong(flds, "ID"), nPatientID, dtDate, nAptType, nAptPurpose, adwAptResource, nIgnoreApptID1, strConflictingAppts, nIgnoreApptID2);
			// If the appointment is not valid because of the rule
			if (nResult == paAppointmentInvalid) {
				// Then handle the rule warning properties
				if (!AdoFldBool(flds, "AllowSave")) {
					bValidateSuccess = FALSE;
					if (pstrFatalWarnings) {
						// (j.jones 2009-08-12 12:19) - PLID 25230 - include the alarm description						
						CString strAlarmDesc = AdoFldString(prs, "Description", "");
						CString strWarn;
						strWarn.Format("The alarm '%s' is triggered by the existing appointment(s):\n", strAlarmDesc);
						(*pstrFatalWarnings) += strWarn + strConflictingAppts;
					}
				} else {
					bWarnings = TRUE;
					if (pstrNonfatalWarnings) {
						// (j.jones 2009-08-12 12:19) - PLID 25230 - include the alarm description						
						CString strAlarmDesc = AdoFldString(prs, "Description", "");
						CString strWarn;
						strWarn.Format("The alarm '%s' is triggered by the existing appointment(s):\n", strAlarmDesc);
						(*pstrNonfatalWarnings) += strWarn + strConflictingAppts;
					}
				}
			}
			// Move to the next rule
			prs->MoveNext();
		}
		prs->Close();

		// Calculate the return value
		long nAns = rwNoRulesApplied;
		if (bWarnings) {
			nAns |= rwHasWarnings;
		}
		if (!bValidateSuccess) {
			nAns |= rwIsPrevented;
		}
		
		return nAns;
	}
#ifdef _USRDLL
	NxCatchAllCallThrow_WithParent(NULL, "Error in AppointmentReadAlarms", {});
#else
	NxCatchAllCallThrow("Error in AppointmentReadAlarms", {});
#endif
	return 0; // Should never get here
}

BOOL ArraysMatch(const CDWordArray& a1, const CDWordArray& a2)
{
	if (a1.GetSize() != a2.GetSize())
		return FALSE;

	for (long j = 0; j < a1.GetSize(); j++)
	{
		for (long k = 0; k < a2.GetSize(); k++)
		{
			if (a2[k] == a1[j])
				break;
		}
		if (k == a2.GetSize())
			return FALSE;
	}
	return TRUE;
}

// (j.jones 2014-11-25 11:26) - PLID 64178 - moved from GlobalSchedUtils
BOOL AppointmentValidateByDuration(const COleDateTime& dtStart, const COleDateTime& dtEnd, long nAptTypeID,
	const CDWordArray& adwResource, const CDWordArray& adwPurpose)
{
	//TES 2/1/2007 - PLID 24543 - If this is an event, then the duration can't be changed, so it must be all right.
	if (dtStart == dtEnd && dtStart.GetHour() == 0 && dtStart.GetMinute() == 0) {
		return TRUE;
	}

	if (!GetRemotePropertyInt("ApptValidateByDuration", 0, 0, "<None>", true))
		return TRUE;
	if (!adwResource.GetSize())
		return TRUE;
	COleDateTimeSpan dt = dtEnd - dtStart;
	long nApptDuration = (long)dt.GetTotalMinutes();

	try
	{
		// (c.haag 2003-08-04 17:52) - Calculate the minimum duration as the minimum of
		// all durations for the given resources
		long nFinalDuration;
		BOOL bFinalDurationSet = 0;

		// (d.thompson 2010-05-14) - PLID 38643 - Optimized.  The simplest and cleanest "fix" seemed to be to loop twice.
		//	The first iteration will just build the parameterized query to be executed.  We'll then make a single trip to
		//	the SQL server, getting all results.
		//Since most appointments have only a single resource attached to them, the loop comes into play fairly infrequently.
		CString strSqlBatch = BeginSqlBatch();
		CNxParamSqlArray args;
		for (long i = 0; i < adwResource.GetSize(); i++) {
			// (d.thompson 2010-05-14) - PLID 38643 - Parameterized to the batch
			AddParamStatementToSqlBatch(strSqlBatch, args, "SELECT ProviderSchedDefDurationT.ID, "
				"	 DurationMinimum, "
				"    ProviderSchedDefDurationDetailT.AptPurposeID "
				"FROM ResourceProviderLinkT LEFT OUTER JOIN "
				"    ProviderSchedDefDurationT ON "
				"    ResourceProviderLinkT.ProviderID = ProviderSchedDefDurationT.ProviderID "
				"    LEFT OUTER JOIN "
				"    ProviderSchedDefDurationDetailT ON "
				"    ProviderSchedDefDurationT.ID = ProviderSchedDefDurationDetailT.ProviderSchedDefDurationID "
				"WHERE ResourceID = {INT} AND AptTypeID = {INT} AND ProviderSchedDefDurationT.ID IS NOT NULL",
				adwResource[i], nAptTypeID);
		}

		//We now have a single string batch to execute, resulting in 1 trip, but yielding (possibly) multiple recordsets
		_RecordsetPtr prs = CreateParamRecordsetBatch(GetRemoteData(), strSqlBatch, args);

		//We now have a set of recordsets, and can move forward after we've finished with each.
		for (long i = 0; i < adwResource.GetSize(); i++)
		{
			CDWordArray adwDefPurposes;
			long lGroupID = -1;
			long nGroupDuration;
			adwDefPurposes.RemoveAll();

			if (prs->eof)
				continue;
			while (!prs->eof)
			{
				// If we are at a new group, check our current group for a perfect
				// match
				if (lGroupID != AdoFldLong(prs, "ID"))
				{
					if (lGroupID != -1)
					{
						if (ArraysMatch(adwDefPurposes, adwPurpose))
						{
							if (!bFinalDurationSet)
								nFinalDuration = nGroupDuration;
							else
								nFinalDuration = min(nGroupDuration, nFinalDuration);
							bFinalDurationSet = TRUE;
						}
					}
					lGroupID = AdoFldLong(prs, "ID");
					adwDefPurposes.RemoveAll();
					nGroupDuration = AdoFldLong(prs, "DurationMinimum");
				}
				if (prs->Fields->Item["AptPurposeID"]->Value.vt != VT_NULL &&
					prs->Fields->Item["AptPurposeID"]->Value.vt != VT_EMPTY)
				{
					adwDefPurposes.Add(AdoFldLong(prs, "AptPurposeID"));
				}
				prs->MoveNext();
			}
			if (ArraysMatch(adwDefPurposes, adwPurpose))
			{
				if (!bFinalDurationSet)
					nFinalDuration = nGroupDuration;
				else
					nFinalDuration = min(nGroupDuration, nFinalDuration);
				bFinalDurationSet = TRUE;
			}

			// (d.thompson 2010-05-14) - PLID 38643 - Now move along to the next recordset in the batch
			prs = prs->NextRecordset(NULL);
		}

		if (bFinalDurationSet && nApptDuration < nFinalDuration)
		{
			CString str;
			str.Format("The appointment duration is shorter than %d minutes. Are you sure you wish to save this appointment?",
				nFinalDuration);
			if (IDNO == MsgBox(MB_YESNO, str))
				return FALSE;
		}
	}
	NxCatchAll("Error validating appointment by duration");
	return TRUE;
}

// (j.jones 2014-11-25 11:26) - PLID 64178 - moved from GlobalSchedUtils
BOOL AppointmentValidateByDuration(ADODB::_RecordsetPtr pAppt)
{
	FieldsPtr pflds = pAppt->Fields;
	CDWordArray adwResourceID;
	CDWordArray adwPurposeID;

	//TES 2/1/2007 - PLID 24543 - If this is an event, then the duration can't be changed, so it must be all right.
	COleDateTime dtStart = AdoFldDateTime(pflds, "StartTime");
	COleDateTime dtEnd = AdoFldDateTime(pflds, "EndTime");
	if (dtStart == dtEnd && dtStart.GetHour() == 0 && dtStart.GetMinute() == 0) {
		return TRUE;
	}

	if (!GetRemotePropertyInt("ApptValidateByDuration", 0, 0, "<None>", true))
		return TRUE;

	long nApptID = AdoFldLong(pflds, "ID", -1);

	// (d.thompson 2010-05-14) - PLID 38643 - Parameterized and combined both queries into a single trip
	_RecordsetPtr rs = CreateParamRecordset("SELECT ResourceID FROM AppointmentResourceT WHERE AppointmentID = {INT};\r\n"
		"SELECT PurposeID FROM AppointmentPurposeT WHERE AppointmentID = {INT};\r\n", nApptID, nApptID);

	while (!rs->eof)
	{
		adwResourceID.Add(AdoFldLong(rs, "ResourceID"));
		rs->MoveNext();
	}

	// (d.thompson 2010-05-14) - PLID 38643 - Parameterized and combined both queries into a single trip.  Move ahead now.
	rs = rs->NextRecordset(NULL);
	while (!rs->eof)
	{
		adwPurposeID.Add(AdoFldLong(rs, "PurposeID"));
		rs->MoveNext();
	}
	rs->Close();
	rs.Detach();

	return AppointmentValidateByDuration(
		dtStart,
		dtEnd,
		AdoFldLong(pflds, "AptTypeID", -1),
		adwResourceID,
		adwPurposeID);
}

// (j.jones 2014-11-25 08:55) - PLID 64178 - Added validation for scheduler mix rules.
// The fields are the slot the user is trying to schedule into, not the slot where the
// appointment currently exists.
// (j.jones 2014-11-26 16:42) - PLID 64272 - the callers have an optional overriddenMixRules returned,
// this is only filled if the appt. exceeded a rule and a user overrode it, the caller needs to save this info
bool AppointmentValidateByMixRules(CWnd *pParentWnd, ADODB::_RecordsetPtr pAppt, OUT std::vector<SchedulerMixRule> &overriddenMixRules, OUT SelectedFFASlotPtr &pSelectedFFASlot,
	OPTIONAL long nAppointmentIDToIgnore /*= -1*/, OPTIONAL long nAppointmentID2ToIgnore /*= -1*/)
{
	//clear the override rule
	overriddenMixRules.clear();

	FieldsPtr pflds = pAppt->Fields;
	CDWordArray adwResourceID;
	CDWordArray adwPurposeID;

	COleDateTime dtApptDate = AdoFldDateTime(pflds, "Date");

	long nApptID = AdoFldLong(pflds, "ID");
	long nPatientID = AdoFldLong(pflds, "PatientID");
	long nAptTypeID = AdoFldLong(pflds, "AptTypeID", -1);
	long nLocationID = AdoFldLong(pflds, "LocationID");
	long nPrimaryInsuredPartyID = AdoFldLong(pflds, "PrimaryInsuredPartyID", -1);

	_RecordsetPtr rs = CreateParamRecordset("SELECT ResourceID FROM AppointmentResourceT WHERE AppointmentID = {INT};\r\n"
		"SELECT PurposeID FROM AppointmentPurposeT WHERE AppointmentID = {INT};",
		nApptID, nApptID);

	while (!rs->eof)
	{
		adwResourceID.Add(AdoFldLong(rs, "ResourceID"));
		rs->MoveNext();
	}

	rs = rs->NextRecordset(NULL);
	while (!rs->eof)
	{
		adwPurposeID.Add(AdoFldLong(rs, "PurposeID"));
		rs->MoveNext();
	}

	rs->Close();
	rs.Detach();

	return AppointmentValidateByMixRules(pParentWnd,
		nPatientID,
		dtApptDate,
		nLocationID,
		nPrimaryInsuredPartyID,
		nAptTypeID,
		adwResourceID,
		adwPurposeID,
		overriddenMixRules,
		pSelectedFFASlot,
		nAppointmentIDToIgnore,
		nAppointmentID2ToIgnore);
}

// (j.jones 2014-11-26 16:42) - PLID 64272 - the callers have an optional overriddenMixRules returned,
// this is only filled if the appt. exceeded a rule and a user overrode it, the caller needs to save this info
bool AppointmentValidateByMixRules(CWnd *pParentWnd, long nPatientID, const COleDateTime& dtApptDate, long nLocationID, long nInsuredPartyID, long nAptTypeID,
	const CDWordArray& adwResource, const CDWordArray& adwPurpose, OUT std::vector<SchedulerMixRule> &overriddenMixRules, OUT SelectedFFASlotPtr &pSelectedFFASlot,
	OPTIONAL long nAppointmentIDToIgnore /*= -1*/, OPTIONAL long nAppointmentID2ToIgnore /*= -1*/)
{
	//clear the override rule
	overriddenMixRules.clear();

	//this still processes if the patient ID or insured party ID is -1

	std::vector<SchedulerMixRule> exceededMixRules;

	if (!AppointmentValidateByMixRules(dtApptDate, nLocationID, nInsuredPartyID, nAptTypeID, adwResource, adwPurpose, exceededMixRules, nAppointmentIDToIgnore, nAppointmentID2ToIgnore)) {
		
		//show the warning
		CSchedulerMixWarningDlg dlg(pParentWnd);
		int nResult = dlg.DoModal(exceededMixRules);
		if (nResult == (int)SchedulerMixWarningReturnValue::smwrvOverride) {
			//return the overridden rules
			overriddenMixRules.clear();
			overriddenMixRules.insert(overriddenMixRules.end(), exceededMixRules.begin(), exceededMixRules.end());
			return true;
		}
		else if (nResult == (int)SchedulerMixWarningReturnValue::smwrvFFA) {

			//TES 12/3/2014 - PLID 64180 - FFA only supports one purpose, so if they've selected multiple they'll need to pick one.
			if (adwPurpose.GetSize() > 1) {
				MsgBox("This appointment has multiple purposes. Please select the default purpose from the Find First Available screen and add multiple purposes upon creating your appointment.");
			}
			CNxTabView* pView = (CNxTabView *)GetMainFrame()->GetOpenView(SCHEDULER_MODULE_NAME);
			if (pView) {
				//TES 12/3/2014 - PLID 64180 - Now call the FFA, pass in the settings to use
				// Not good coding practice right here....
				CNxSchedulerDlg* pDlg = (CNxSchedulerDlg*)pView->GetActiveSheet();
				//TES 12/18/2014 - PLID 64466 - Pass in a pointer where it can return the slot they selected
				pSelectedFFASlot.reset();
				// (r.farnworth 2015-06-08 12:42) - PLID 65639 - We need to override Location
				pDlg->FindFirstAvailableApptWithPresets(nPatientID, nAptTypeID, adwPurpose, adwResource, nInsuredPartyID, pSelectedFFASlot, nLocationID);
				if (pSelectedFFASlot != NULL && pSelectedFFASlot->IsValid()) {
					return true;
				}
			}
			return false;
		}
		else {
			return false;
		}
	}

	return true;
}

// (j.jones 2014-12-12 10:17) - PLID 64178 - added local function for converting appointment properties
// into a GetAppointmentPropertiesInput object, this should no go in a .h file if it can be avoided
NexTech_Accessor::_GetAppointmentPropertiesInputPtr ConvertAppointmentInfoToAPIInput(const COleDateTime& dtApptDate, long nLocationID, long nInsuredPartyID, long nAptTypeID, const CDWordArray& adwResource, const CDWordArray& adwPurpose)
{
	Nx::SafeArray<BSTR> saryResourceIDs, saryPurposeIDs;

	for (int i = 0; i < adwResource.GetSize(); i++) {
		saryResourceIDs.Add(_bstr_t(AsString(adwResource[i])));
	}

	for (int i = 0; i < adwPurpose.GetSize(); i++) {
		saryPurposeIDs.Add(_bstr_t(AsString(adwPurpose[i])));
	}

	NexTech_Accessor::_GetAppointmentPropertiesInputPtr apptInfo(__uuidof(NexTech_Accessor::GetAppointmentPropertiesInput));
	apptInfo->date = AsDateNoTime(dtApptDate);
	apptInfo->ResourceIDs = saryResourceIDs;
	apptInfo->locationID = nLocationID == -1 ? "" : _bstr_t(AsString(nLocationID));
	apptInfo->TypeID = nAptTypeID == -1 ? "" : _bstr_t(AsString(nAptTypeID));
	apptInfo->PurposeIDs = saryPurposeIDs;
	apptInfo->InsuredPartyID = nInsuredPartyID == -1 ? "" : _bstr_t(AsString(nInsuredPartyID));

	return apptInfo;
}

bool AppointmentValidateByMixRules(const COleDateTime& dtApptDate, long nLocationID, long nInsuredPartyID, long nAptTypeID,
	const CDWordArray& adwResource, const CDWordArray& adwPurpose, OUT std::vector<SchedulerMixRule> &exceededMixRules, OPTIONAL long nAppointmentIDToIgnore /*= -1*/, OPTIONAL long nAppointmentID2ToIgnore /*= -1*/)
{
	//this still processes if the insured party ID is -1

	//clear the rule info
	exceededMixRules.clear();

	//convert our parameters for the API

	NexTech_Accessor::_PracticeMethodsPtr pApi = GetAPI();
	if (pApi == NULL) {
		ThrowNxException("Could not call AppointmentValidateByMixRules due to an invalid API.");
	}
	
	NexTech_Accessor::_GetAppointmentPropertiesInputPtr apptInfo = ConvertAppointmentInfoToAPIInput(dtApptDate, nLocationID, nInsuredPartyID, nAptTypeID, adwResource, adwPurpose);

	Nx::SafeArray<BSTR> saryApptIDsToIgnore;
	if (nAppointmentIDToIgnore != -1) {
		saryApptIDsToIgnore.Add(_bstr_t(AsString(nAppointmentIDToIgnore)));
	}
	if (nAppointmentID2ToIgnore != -1) {
		saryApptIDsToIgnore.Add(_bstr_t(AsString(nAppointmentID2ToIgnore)));
	}

	//the returned exceeded rule list includes all rule details that were applicable,
	//but that rule detail list does not filter down to only rule details that were exceeded
	NexTech_Accessor::_SchedulingMixRulesPtr pRules = pApi->AppointmentValidateByMixRules(GetAPISubkey(), GetAPILoginToken(), apptInfo, saryApptIDsToIgnore);
	Nx::SafeArray<IUnknown *> saResults = pRules->Results;

	//convert our results back to our local objects
	for each(NexTech_Accessor::_SchedulingMixRulePtr rulePtr in saResults) {
		SchedulerMixRule ruleInfo;
		ruleInfo.nID = atoi((LPCTSTR)rulePtr->GetruleID());
		ruleInfo.strName = (LPCTSTR)rulePtr->GetRuleName();
		ruleInfo.nColor = rulePtr->GetColor();

		//get the details, remember these are all the applicable details,
		//but not necessarily details that were exceeded
		Nx::SafeArray<IUnknown *> saDetails = rulePtr->details;
		for each(NexTech_Accessor::_SchedulingMixRuleDetailPtr detailPtr in saDetails) {
			
			SchedulerMixRuleDetail detailInfo;
			detailInfo.nID = atoi((LPCTSTR)detailPtr->GetdetailID());
			detailInfo.nMaxAppts = detailPtr->GetMaxAppts();
			ruleInfo.aryRuleDetails.push_back(detailInfo);
		}

		exceededMixRules.push_back(ruleInfo);
	}

	//now return true or false based on whether this appt. can be saved
	if (exceededMixRules.size() > 0) {
		return false;
	}
	else {
		return true;
	}
}

// (j.jones 2014-12-04 15:14) - PLID 64119 - Returns -1 if no mix rule color applies.
// Otherwise returns the color of the applicable mix rule. If multiple rules apply,
// this returns the highest color value, for consistency. This decision is arbitrary.
long GetAppointmentMixRuleColor(long nAppointmentID)
{
	_RecordsetPtr rs = AppointmentGrab(nAppointmentID, TRUE, TRUE);
	if (rs == NULL || rs->eof) {
		//how did this happen?
		ASSERT(FALSE);
		return -1;
	}
	else {
		COleDateTime dtApptDate = VarDateTime(rs->Fields->Item["Date"]->Value);
		long nLocationID = VarLong(rs->Fields->Item["LocationID"]->Value);
		long nPrimaryInsuredPartyID = VarLong(rs->Fields->Item["PrimaryInsuredPartyID"]->Value, -1);
		long nAptTypeID = VarLong(rs->Fields->Item["AptTypeID"]->Value, -1);
		CDWordArray adwResourceIDs;
		LoadResourceIDStringIntoArray(VarString(rs->Fields->Item["ResourceIDs"]->Value, ""), adwResourceIDs);
		CDWordArray adwPurposeIDs;
		LoadPurposeIDStringIntoArray(VarString(rs->Fields->Item["PurposeIDs"]->Value, ""), adwPurposeIDs);

		return GetAppointmentMixRuleColor(dtApptDate, nLocationID, nPrimaryInsuredPartyID, nAptTypeID, adwResourceIDs, adwPurposeIDs);
	}
}

// (j.jones 2014-12-04 15:14) - PLID 64119 - Returns -1 if no mix rule color applies.
// Otherwise returns the color of the applicable mix rule. If multiple rules apply,
// this returns the highest color value, for consistency. This decision is arbitrary.
long GetAppointmentMixRuleColor(const COleDateTime& dtApptDate, long nLocationID, long nInsuredPartyID, long nAptTypeID,
	const CDWordArray& adwResource, const CDWordArray& adwPurpose)
{

	NexTech_Accessor::_PracticeMethodsPtr pApi = GetAPI();
	if (pApi == NULL) {
		ThrowNxException("Could not call GetAppointmentMixRuleColor due to an invalid API.");
	}

	NexTech_Accessor::_GetAppointmentPropertiesInputPtr apptInfo = ConvertAppointmentInfoToAPIInput(dtApptDate, nLocationID, nInsuredPartyID, nAptTypeID, adwResource, adwPurpose);
	NexTech_Accessor::_AppointmentMixRuleColorInfoPtr pResult = pApi->GetAppointmentMixRuleColor(GetAPISubkey(), GetAPILoginToken(), apptInfo);
	//convert from hex to int
	CString strHexColor = (LPSTR)pResult->Color;
	if (strHexColor.GetLength() == 0) {
		//empty string means no color, which we track as -1
		return -1;
	}
	else if (strHexColor.GetLength() == 7) {
		long nColor = ConvertHexStringToCOLORREF(strHexColor);
		return nColor;
	}
	else {
		//this is not a valid hex string! don't throw an exception, just ignore the color
		//devs. should track down how this happened
		ASSERT(FALSE);
		return -1;
	}
}

// (j.jones 2014-12-05 10:45) - PLID 64274 - Given an ID of an appt. known to have an entry in
// AppointmentMixRuleOverridesT, this function will re-check every overridden rule on the appt.
// If that rule is still exceeded on the appt. date, this returns true. If the rule is no longer
// exceeded, or if the appointment is cancelled, this returns false.
bool IsAppointmentMixRuleOverrideCurrent(long nAppointmentID)
{
	NexTech_Accessor::_PracticeMethodsPtr pApi = GetAPI();
	if (pApi == NULL) {
		ThrowNxException("Could not call IsAppointmentMixRuleOverrideCurrent due to an invalid API.");
	}

	NexTech_Accessor::_AppointmentMixRuleOverrideCurrentInfoPtr pResult = pApi->IsAppointmentMixRuleOverrideCurrent(GetAPISubkey(), GetAPILoginToken(), _bstr_t(AsString((nAppointmentID))));
	if (pResult->OverrideIsCurrent == VARIANT_TRUE) {
		return true;
	}
	else {
		return false;
	}
}

// (j.jones 2014-12-05 13:58) - PLID 64274 - Finds all appointments in the provided date range
// that have entries in AppointmentMixRuleOverridesT, and revalidates the mix rules for that
// appointment date to confirm whether the rule is still exceeded. If it isn't, then the
// entry in AppointmentMixRuleOverridesT is meaningless. Cancelled appointments are ignored.
// Returns a list of Appointment IDs with overrides that still apply.
std::vector<long> GetOverrideAppointmentsInDateRange(COleDateTime dtStartDate, COleDateTime dtEndDate)
{
	std::vector<long> aryOverrideAppointmentIDs;

	NexTech_Accessor::_PracticeMethodsPtr pApi = GetAPI();
	if (pApi == NULL) {
		ThrowNxException("Could not call GetOverrideAppointmentsInDateRange due to an invalid API.");
	}

	NexTech_Accessor::_GetDateRangeInputPtr dateRange(__uuidof(NexTech_Accessor::GetDateRangeInput));
	dateRange->RangeBeginDate = AsDateNoTime(dtStartDate);
	dateRange->RangeEndDate = AsDateNoTime(dtEndDate);
	NexTech_Accessor::_MixRuleOverrideAppointmentsPtr pResult = pApi->GetMixRuleOverrideAppointmentsInDateRange(GetAPISubkey(), GetAPILoginToken(), dateRange);
	Nx::SafeArray<BSTR> saryOverrideAppointmentIDs = pResult->OverrideAppointmentIDs;

	for each (_bstr_t apptID in saryOverrideAppointmentIDs)
	{
		CString strApptID = (LPCSTR)apptID;
		long nAppointmentID = atoi(strApptID);
		aryOverrideAppointmentIDs.push_back(nAppointmentID);
	}

	return aryOverrideAppointmentIDs;
}

// Takes an alarm ID and processes all it's details against the 
// given appointment properties.  Returns whether the 
// appointment is valid or not based on the alarm details
EProcessAlarmResult ProcessAlarm(long nAlarmID, long nPatientID, const COleDateTime &dtDate, long nAptType, long nAptPurpose, const CDWordArray& adwAptResource, long nIgnoreApptID1, CString &strConflictingAppts, long nIgnoreApptID2 /*= -1*/)
{
	_RecordsetPtr rsEvent = CreateParamRecordset("SELECT StartTime, EndTime FROM AppointmentsT WHERE ID = {INT}", nIgnoreApptID1);
	if (!rsEvent->eof) {
		COleDateTime dtNewStart = AdoFldDateTime(rsEvent, "StartTime", COleDateTime(0, 0, 0, 8, 0, 0));
		if (dtNewStart.GetHour() == 0 && dtNewStart.GetMinute() == 0) {
			//this might be an event.
			COleDateTime dtToEnd = AdoFldDateTime(rsEvent, "EndTime", COleDateTime(0, 0, 0, 9, 0, 0));
			if (dtToEnd.GetHour() == 0 && dtToEnd.GetMinute() == 0) {
				//This is an event, we don't check on events
				return paAppointmentValid;
			}
		}
	}
	rsEvent->Close();


	BOOL bCheckBefore = FALSE;
	long nDaysBefore = -1;
	BOOL bCheckAfter = FALSE;
	long nDaysAfter = -1;
	BOOL bAllowSave = TRUE;
	BOOL bSameTypeOnly = FALSE;
	BOOL bSamePurposeOnly = FALSE;
	BOOL bSameResourceOnly = FALSE;
	BOOL bIncludeNoShows = FALSE;

	//first we must see if the apt. types and purposes of this alarm match this appointment
	if (!MatchAppointmentToAlarmDetails(nAptType, nAptPurpose, nAlarmID))
		return paAppointmentValid;


	// Fill the above variables based on all the details of this rule
	{
		_RecordsetPtr prs = CreateParamRecordset(
			"SELECT "
			"	AptBookAlarmT.ID, "
			"	AptBookAlarmT.CheckBefore, "
			"	AptBookAlarmT.DaysBefore, "
			"	AptBookAlarmT.CheckAfter, "
			"	AptBookAlarmT.DaysAfter, "
			"	AptBookAlarmT.AllowSave, "
			"	AptBookAlarmT.SameType, "
			"	AptBookAlarmT.SamePurpose, "
			"	AptBookAlarmT.SameResource, "
			"	AptBookAlarmT.IncludeNoShows "
			"FROM "
			"	AptBookAlarmT "
			"WHERE "
			"	AptBookAlarmT.ID = {INT}", nAlarmID);

		FieldsPtr flds = prs->Fields;

		if (!prs->eof) {

			bCheckBefore = AdoFldBool(flds, "CheckBefore", FALSE);
			nDaysBefore = AdoFldLong(flds, "DaysBefore", -1);
			bCheckAfter = AdoFldBool(flds, "CheckAfter", FALSE);
			nDaysAfter = AdoFldLong(flds, "DaysAfter", -1);
			bAllowSave = AdoFldBool(flds, "AllowSave", TRUE);
			bSameTypeOnly = AdoFldBool(flds, "SameType", FALSE);
			bSamePurposeOnly = AdoFldBool(flds, "SamePurpose", FALSE);
			bSameResourceOnly = AdoFldBool(flds, "SameResource", FALSE);
			bIncludeNoShows = AdoFldBool(flds, "IncludeNoShows", FALSE);
		}

		prs->Close();
	}

	// Now we know the basic details of this alarm so check the appointment against the details
	if (bCheckBefore || bCheckAfter) {

		COleDateTime dtMin, dtAppt, dtMax; // dates for use in our query

		//set the days we won't check on to be zero
		if (!bCheckBefore)
			nDaysBefore = 0;

		if (!bCheckAfter)
			nDaysAfter = 0;

		//here we go, here's what drives the whole thing!

		CString strAndType = "", strAndPurpose = "", strAndResource = "";
		//if we are only comparing the same type, same purpose, and/or same resource, append and statements for them
		if (bSameTypeOnly) {
			if (nAptType != -1)
				strAndType.Format(" AND AptTypeID = %li", nAptType);
			else
				// (j.jones 2005-06-27 16:28) - PLID 16735 - don't compare if there is no type,
				// treat as NULL, where NULL <> NULL, so in turn ignore this alarm if it requires same type only
				//strAndType.Format(" AND AptTypeID Is Null");
				return paAppointmentValid;
		}
		if (bSamePurposeOnly) {
			if (nAptPurpose != -1)
				strAndPurpose.Format(" AND ID IN (SELECT AppointmentID FROM AppointmentPurposeT WHERE PurposeID = %li)", nAptPurpose);
			else
				// (j.jones 2005-06-27 16:28) - PLID 16735 - don't compare if there is no purpose,
				// treat as NULL, where NULL <> NULL, so in turn ignore this alarm if it requires same purpose only
				//strAndPurpose.Format(" AND ID NOT IN (SELECT AppointmentID FROM AppointmentPurposeT)");
				return paAppointmentValid;
		}

		if (bSameResourceOnly) {
			if (adwAptResource.GetSize() > 0) {
				CString strResourceIDs = "";
				for (int i = 0; i<adwAptResource.GetSize(); i++) {
					if (!strResourceIDs.IsEmpty())
						strResourceIDs += ",";
					strResourceIDs += AsString((long)adwAptResource.GetAt(i));
				}
				strAndResource.Format(" AND ID IN (SELECT AppointmentID FROM AppointmentResourceT WHERE ResourceID IN (%s))", strResourceIDs);
			}
		}

		//#pragma TODO("Good candidate for parameterization, but too large a change for this situation")
		_RecordsetPtr rs = CreateRecordset("SELECT ID, StartTime, EndTime FROM AppointmentsT WHERE "
			"(ShowState <> 3 OR 1 = %li) AND Status <> 4 AND Date >= DATEADD(day,%li,Convert(datetime,'%s')) AND Date <= DATEADD(day,%li,Convert(datetime,'%s')) AND PatientID = %li AND ID != %li AND ID != %li %s %s %s",
			(bIncludeNoShows ? 1 : 0), -nDaysBefore, FormatDateTimeForSql(dtDate, dtoDate), nDaysAfter, FormatDateTimeForSql(dtDate, dtoDate), nPatientID, nIgnoreApptID1, nIgnoreApptID2, strAndType, strAndPurpose, strAndResource);

		BOOL bMatch = FALSE;

		while (!rs->eof) {

			BOOL bIsEvent = FALSE;
			COleDateTime dtNewStart = AdoFldDateTime(rs, "StartTime", COleDateTime(0, 0, 0, 8, 0, 0));
			if (dtNewStart.GetHour() == 0 && dtNewStart.GetMinute() == 0) {
				//this might be an event.
				COleDateTime dtToEnd = AdoFldDateTime(rs, "EndTime", COleDateTime(0, 0, 0, 9, 0, 0));
				if (dtToEnd.GetHour() == 0 && dtToEnd.GetMinute() == 0) {
					//This is an event.
					bIsEvent = TRUE;
				}
			}

			if (!bIsEvent) {
				//don't bother if what we found was an event
				//JMJ - we may want an option to check events later, but we see no need for it now

				bMatch = TRUE;

				CString str = GetAppointmentAlarmConflictInfo(AdoFldLong(rs, "ID"));
				str += "\n\n";
				strConflictingAppts += str;
			}

			rs->MoveNext();
		}
		rs->Close();

		if (bMatch) {
			return paAppointmentInvalid;
		}
		else {
			return paAppointmentValid;
		}

	}
	else {
		// if the alarm does not check for appts. before or after, then the alarm is pointless
		return paAppointmentValid;
	}
}

BOOL MatchAppointmentToAlarmDetails(long nAptType, long nAptPurpose, long nAlarmID) {

	//we must check to see if:
	//  --there are no apt. details for this alarm, meaning all types and all purposes
	//	--both the apt. type and purpose of the alarm is NULL, meaning all types and all purposes
	//  --the alarm's apt. type matches and the purpose is null
	//  --the alarm's apt. purpose matches and the type is null
	//  --both the the alarm't apt. type and purpose match this appointment

	//return TRUE if the Alarm Details matches the Appointment, FALSE if it does not

	//check that there are no apt. details for this alarm, meaning all types and all purposes
	// (a.walling 2010-10-19 09:45) - PLID 40965 - Use ReturnsRecordsParam
	if(!ReturnsRecordsParam("SELECT ID FROM AptBookAlarmDetailsT WHERE AptBookAlarmID = {INT}",nAlarmID))
		return TRUE;

	//check that both the apt. type and purpose of the alarm is NULL, meaning all types and all purposes
	// (a.walling 2010-10-19 09:45) - PLID 40965 - Use ReturnsRecordsParam
	if(ReturnsRecordsParam("SELECT ID FROM AptBookAlarmDetailsT WHERE AptTypeID Is NULL AND AptPurposeID Is NULL "
		"AND AptBookAlarmID = {INT}",nAlarmID))
		return TRUE;

	//check that the alarm's apt. type matches and the purpose is null
	// (a.walling 2010-10-19 09:45) - PLID 40965 - Use ReturnsRecordsParam
	if(ReturnsRecordsParam("SELECT ID FROM AptBookAlarmDetailsT WHERE AptBookAlarmID = {INT} AND "
		"AptTypeID = {INT} AND AptPurposeID Is Null",nAlarmID,nAptType))
		return TRUE;

	//check that the alarm's apt. purpose matches and the type is null
	// (a.walling 2010-10-19 09:45) - PLID 40965 - Use ReturnsRecordsParam
	if(ReturnsRecordsParam("SELECT ID FROM AptBookAlarmDetailsT WHERE AptBookAlarmID = {INT} AND "
		"AptPurposeID = {INT} AND AptTypeID Is Null",nAlarmID,nAptPurpose))
		return TRUE;

	//check that both the the alarm't apt. type and purpose match this appointment
	// (a.walling 2010-10-19 09:45) - PLID 40965 - Use ReturnsRecordsParam
	if(ReturnsRecordsParam("SELECT ID FROM AptBookAlarmDetailsT WHERE AptBookAlarmID = {INT} AND "
		"AptPurposeID = {INT} AND AptTypeID = {INT}",nAlarmID,nAptPurpose,nAptType))
		return TRUE;

	//if we got this far, it does not match
	return FALSE;
}

CString GetAppointmentAlarmConflictInfo(long nConflictingApptID) {

	CString strInfo;

	// (a.walling 2013-02-08 12:59) - PLID 55083 - Parameterize
	_RecordsetPtr rs = CreateParamRecordset("SELECT Date, StartTime, EndTime, AptTypeT.Name, "
		"dbo.GetPurposeString(AppointmentsT.ID) AS Purpose, dbo.GetResourceString(AppointmentsT.ID) AS Resource FROM AppointmentsT "
		"LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID WHERE AppointmentsT.ID = {INT}",nConflictingApptID);

	if(!rs->eof) {
		COleDateTime dtDate = AdoFldDateTime(rs, "Date");
		COleDateTime dtStartTime = AdoFldDateTime(rs, "StartTime");
		COleDateTime dtEndTime = AdoFldDateTime(rs, "EndTime");
		CString strAptTypeName = AdoFldString(rs, "Name","<No Type>");
		CString strAptPurposeName = AdoFldString(rs, "Purpose","<No Purpose>");
		CString strAptResourceName = AdoFldString(rs, "Resource","<No Resource>");

		strInfo.Format("Appointment Date: %s, %s - %s, Type: %s, Purpose: %s, Resource: %s",
			FormatDateTimeForInterface(dtDate), FormatDateTimeForInterface(dtStartTime, DTF_STRIP_SECONDS, dtoTime), FormatDateTimeForInterface(dtEndTime, DTF_STRIP_SECONDS, dtoTime), strAptTypeName, strAptPurposeName, strAptResourceName);
	}
	rs->Close();

	return strInfo;
}

// CAH 5/16/2003 - COleDateTimeSpan doesn't work right if you subtract a time in the future
// from a time in the past...at least right by my idea of how it should work, anyway. This
// function works around that defect to give us the total number of minutes that dt1 and dt2
// are different.
long GetSafeTotalMinuteDiff(const COleDateTime& dt1, const COleDateTime& dt2)
{
	COleDateTimeSpan dts;
	if (dt1 > dt2)
	{
		dts = dt1 - dt2;
		return (long)dts.GetTotalMinutes();
	}
	else
	{
		dts = dt2 - dt1;
		return -(long)dts.GetTotalMinutes();
	}
	return 0;	
}

bool AttemptWarnForInsAuth(long nPatientID, COleDateTime dtApptDate) {
	//Return values:  Returns true if they either said 'Yes' to the warning, or they were never
	//				prompted.
	//				  Returns false if they said 'No' to the warning, or some error condition
	//				occurred.

	
	
	// (b.cardillo 2003-06-05 5:55p) - Just talked with Don, this should just do nothing if the -25 
	// patient id is passed in.  We can almost prove that it would return true (without prompting) 
	// anyway, but this way we guarantee it and we don't incur the slight performace hit.
	if (nPatientID == -25) {
		return true;
	}


	try {

		//if they don't want to be warned, don't warn them
		// (b.spivey, March 26, 2012) - PLID 47435 - "2" is 'Don't Warn.'
		if(GetRemotePropertyInt("WarnForInsAuth", 0, 0, "<None>", true) == 2)
			return true;

		_RecordsetPtr rs, rs2;
		/////////////////////////////////////
		//1)  See if this patient has an active insurance company that requires authorizations.
		// (j.jones 2008-09-02 15:32) - PLID 30258 - ensure we only check for active insured parties
		rs = CreateParamRecordset("SELECT InsuredPartyT.PersonID AS InsPartyID, InsuranceCoT.PersonID AS InsCoID "
			"FROM PersonT "
			"LEFT JOIN InsuredPartyT ON PersonT.ID = InsuredPartyT.PatientID "
			"LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
			"WHERE InsuredPartyT.RespTypeID <> -1"
			"AND UseReferrals = 1 "
			"AND PersonT.ID = {INT} ", nPatientID);

		if(rs->eof)
			//they have no insurance which requires warning
			return true;

		bool bAvail = false;
		while(!bAvail && !rs->eof) {
			//they could have multiples, if any have valid, don't warn them

			/////////////////////////////////////
			//2)  If so, see if they have no active auths.
			long nInsPartyID = AdoFldLong(rs, "InsPartyID", -1);
			COleDateTime dtNow = COleDateTime::GetCurrentTime();

			// (j.jones 2011-09-29 15:53) - PLID 44980 - ignore referrals if in use on a voided bill
			// (a.walling 2013-02-08 12:59) - PLID 55083 - Parameterize
			rs2 = CreateParamRecordset("SELECT InsuranceReferralsT.ID, AuthNum, StartDate, EndDate, NumVisits, "
				"CASE WHEN NumUsed IS NULL THEN 0 ELSE NumUsed END AS NumUsed, InsuredPartyID "
				"FROM InsuranceReferralsT LEFT JOIN (SELECT Count(InsuranceReferralID) AS NumUsed, InsuranceReferralID FROM BillsT WHERE Deleted = 0 AND InsuranceReferralID IS NOT NULL AND BillsT.ID NOT IN (SELECT OriginalBillID FROM BillCorrectionsT) "
				"GROUP BY InsuranceReferralID) AS NumUsedQ ON InsuranceReferralsT.ID = NumUsedQ.InsuranceReferralID "
				"WHERE InsuredPartyID = {INT} AND NumVisits > (CASE WHEN NumUsed Is NULL THEN 0 ELSE NumUsed END) AND StartDate <= {OLEDATETIME} AND EndDate >= {OLEDATETIME}", 
				nInsPartyID, AsDateNoTime(dtApptDate), AsDateNoTime(dtApptDate));

			if(!rs2->eof) {
				//there is an available referral for this insured party
				bAvail = true;
			}

			rs->MoveNext();
		}

		/////////////////////////////////////
		//3)  If they have active ones, do nothing
		if(bAvail)
			return true;
		
		// (b.spivey, March 23, 2012) - PLID 47435 - this branches off in two different directions now. 
		/////////////////////////////////////
		//4)	If they wanted a prompt, we give them the choice to enter a referral if there is no active one. 
		if(GetRemotePropertyInt("WarnForInsAuth", 0, 0, "<None>", true) == 1) {
			if(CheckCurrentUserPermissions(bioPatientInsurance, sptWrite, 0, 0, TRUE)) {
				rs->MoveFirst(); 
				int nGaur = AdoFldLong(rs, "InsPartyID", -1); 
				CInsuranceReferralsSelectDlg dlg(NULL, true);
				dlg.m_nPatientID = nPatientID; 
				dlg.m_InsuredPartyID = nGaur; 
				// (b.spivey, March 27, 2012) - PLID 47435 - Get the date for the appointment to show the most appropriate referrals. 
				dlg.m_dtFilterDate.SetDate(dtApptDate.GetYear(), dtApptDate.GetMonth(), dtApptDate.GetDay()); 
				//If we cancelled, we just jump out. 
				if(dlg.DoModal() == IDCANCEL){
					return false;
				}
				else {
					//Anything else and return true
					return true;
				}
			}
			else {
				//fall through to 5
			}
		}

		/////////////////////////////////////
		//5)  If they do not have active ones, then we must warn them
		if(MsgBox(MB_YESNO, "This patient does not have any active insurance referrals during this time period.\n"
			"Do you still wish to save changes to this appointment?") == IDNO)
			return false;
		else
			return true;
	}
#ifdef _USRDLL
	NxCatchAll_WithParent(NULL, "Error attempting to warn for insurance authorization.");
#else
	NxCatchAll("Error attempting to warn for insurance authorization.");
#endif

	return false;
}

void TruncateSchedulerWarning(CString& strText)
{
	long i, nFirst;
	for (i = 0, nFirst = 0; i < 14 && nFirst > -1; i++)
	{
		nFirst = strText.Find('\n', nFirst);
		if (nFirst != -1) nFirst++;
	}	
	if (-1 == nFirst)
		return; // The string isn't big enough to be truncated

	strText = strText.Left(nFirst) + "<More items omitted>\n\n";
}

BOOL CheckWarnProcedureCredentials(const CDWordArray& adwResourceID, const CDWordArray& adwAptPurpose)
{
	BOOL bWarnCredentials = FALSE;
	BOOL bWarnLicensing = FALSE;

	CString strWarnCredentials;
	CString strWarnLicensing;

	BOOL bCheckAndWarnCredentials = GetRemotePropertyInt("CredentialWarnAppointments",0,0,"<None>",true) == 1;
	BOOL bCheckAndWarnLicensing = GetRemotePropertyInt("ExpLicenseWarnAppointments",0,0,"<None>",true) == 1;

	//if they don't want to check credentials nor licensing in the scheduler, then leave immediately
	if(!bCheckAndWarnCredentials && !bCheckAndWarnLicensing)
		return TRUE;

	//compare each resource to each purpose
	for(int i=0;i<adwResourceID.GetSize();i++) {

		//get the provider ID linked to the resource
		long nProviderID = -1;

		// (a.walling 2013-02-08 12:59) - PLID 55083 - Parameterize
		_RecordsetPtr rs = CreateParamRecordset("SELECT ProviderID FROM ResourceProviderLinkT WHERE ResourceID = {INT}",(long)adwResourceID.GetAt(i));
		if(!rs->eof) {
			nProviderID = AdoFldLong(rs, "ProviderID",-1);
		}
		rs->Close();

		//only compare if there is a provider linked to the resource
		if(nProviderID != -1) {

			//check credentials
			if(bCheckAndWarnCredentials) {

				for(int j=0;j<adwAptPurpose.GetSize();j++) {

					long nProcedureID = (long)adwAptPurpose.GetAt(j);

					ECredentialWarning eCredWarning = CheckProcedureCredential(nProviderID, nProcedureID);

					if(eCredWarning == eFailedCredential) {

						bWarnCredentials = TRUE;

						CString strProviderName, strProcedureName;

						//get the provider name
						// (a.walling 2013-02-08 12:59) - PLID 55083 - Parameterize
						rs = CreateParamRecordset("SELECT Last + ', ' + First + ' '  + Middle AS Name FROM PersonT WHERE ID = {INT}",nProviderID);
						if(!rs->eof) {
							strProviderName = AdoFldString(rs, "Name","");
						}
						rs->Close();

						//get the procedure name of the failed credential
						// (a.walling 2013-02-08 12:59) - PLID 55083 - Parameterize
						rs = CreateParamRecordset("SELECT Name FROM ProcedureT WHERE ID = {INT}",nProcedureID);
						if(!rs->eof) {
							strProcedureName = AdoFldString(rs, "Name","");
						}
						rs->Close();

						//now add the warning
						CString str;
						str.Format("- Provider '%s' is not credentialed for procedure '%s'.\n",strProviderName,strProcedureName);
						strWarnCredentials += str;
					}
				}
			}

			//check licensing
			if(bCheckAndWarnLicensing) {

				ECredentialWarning eCredWarning = CheckPersonCertifications(nProviderID);

				if(eCredWarning != ePassedAll) {

					bWarnLicensing = TRUE;

					//get the provider name
					CString strProviderName;
					// (a.walling 2013-02-08 12:59) - PLID 55083 - Parameterize
					rs = CreateParamRecordset("SELECT Last + ', ' + First + ' '  + Middle AS Name FROM PersonT WHERE ID = {INT}",nProviderID);
					if(!rs->eof) {
						strProviderName = AdoFldString(rs, "Name","");
					}
					rs->Close();

					if(eCredWarning == eFailedLicenseExpired) {

						CString strLicenses;
						// (a.walling 2013-02-08 12:59) - PLID 55083 - Parameterize
						_RecordsetPtr rs2 = CreateParamRecordset("SELECT '''' + Name + ''' - Expired: ' + Convert(nvarchar,ExpDate,1) AS ExpiredLicense FROM PersonCertificationsT "
							"WHERE PersonID = {INT} AND ExpDate < Convert(datetime,(Convert(nvarchar,GetDate(),1)))",nProviderID);
						while(!rs2->eof) {
							CString str;
							str.Format("    %s\n",AdoFldString(rs2, "ExpiredLicense",""));
							strLicenses += str;
							rs2->MoveNext();
						}
						rs2->Close();

						CString str;
						str.Format("- Provider '%s' has the following expired licenses:\n%s",strProviderName,strLicenses);
						strWarnLicensing += str;
					}
					else if(eCredWarning == eFailedLicenseExpiringSoon) {

						//check if a license will expire within the given day range
						long nLicenseWarnDayRange = GetRemotePropertyInt("DefaultASCLicenseWarnDayRange",30,0,"<None>",true);

						CString strLicenses;
						// (a.walling 2013-02-08 12:59) - PLID 55083 - Parameterize
						_RecordsetPtr rs2 = CreateParamRecordset("SELECT '''' + Name + ''' - Expires on: ' + Convert(nvarchar,ExpDate,1) AS ExpiredLicense FROM PersonCertificationsT "
							"WHERE PersonID = {INT} AND ExpDate < DateAdd(day,{INT},Convert(datetime,(Convert(nvarchar,GetDate(),1))))",nProviderID,nLicenseWarnDayRange);
						while(!rs2->eof) {
							CString str;
							str.Format("    %s\n",AdoFldString(rs2, "ExpiredLicense",""));
							strLicenses += str;
							rs2->MoveNext();
						}
						rs2->Close();

						CString str;
						str.Format("- Provider '%s' has the following licenses about to expire:\n%s",strProviderName,strLicenses);
						strWarnLicensing += str;
					}
				}
			}
		}
	}

	if(bWarnCredentials || bWarnLicensing) {

		CString strFinalWarn = "The Provider(s) linked to the resource(s) on this appointment have the following warnings:\n\n";
		if(adwResourceID.GetSize() <= 1)
			strFinalWarn.Replace("(s)","");

		if(bWarnCredentials) {
			strFinalWarn += strWarnCredentials;
		}

		if(bWarnLicensing) {
			strFinalWarn += strWarnLicensing;
		}

		strFinalWarn += "\nAre you sure you wish to save this appointment?";

		if(IDNO == MessageBox(GetActiveWindow(),strFinalWarn,"Practice",MB_ICONEXCLAMATION|MB_YESNO)) {
			return FALSE;
		}
	}


	return TRUE;
}

void LoadResourceIDStringIntoArray(CString strResourceIDs, CDWordArray& adwIDs)
{
	while (strResourceIDs.GetLength()) {
		long nID;
		if (-1 != strResourceIDs.Find(" ")) {
			nID = atoi(strResourceIDs.Left( strResourceIDs.Find(" ") ));
			strResourceIDs = strResourceIDs.Right(strResourceIDs.GetLength() - strResourceIDs.Find(" ") - 1);
		} else  {
			nID = atoi(strResourceIDs);
			strResourceIDs.Empty();
		}
		adwIDs.Add(nID);
	}
}

// (r.farnworth 2015-05-28 08:33)
void LoadResourceIDStringIntoMap(CString strResourceIDs, CArray<LPARAM, LPARAM>& naryResourceIDs, CMap<long, long, long, long>& mapIDs)
{
	int i = 0;
	while (strResourceIDs.GetLength()) {
		long nID, pID;
		if (-1 != strResourceIDs.Find(" ")) {
			nID = atoi(strResourceIDs.Left(strResourceIDs.Find(" ")));
			strResourceIDs = strResourceIDs.Right(strResourceIDs.GetLength() - strResourceIDs.Find(" ") - 1);
		}
		else  {
			nID = atoi(strResourceIDs);
			strResourceIDs.Empty();
		}

		for (long i = 0; i < naryResourceIDs.GetCount(); i++)
		{
			pID = naryResourceIDs.GetAt(i);
			if (pID == nID) {
				mapIDs[pID] = nID;
			}
		}
	}
}

void LoadPurposeIDStringIntoArray(CString strPurposeIDs, CDWordArray& adwIDs)
{
	while (strPurposeIDs.GetLength()) {
		long nID;
		if (-1 != strPurposeIDs.Find(" ")) {
			nID = atoi(strPurposeIDs.Left( strPurposeIDs.Find(" ") ));
			strPurposeIDs = strPurposeIDs.Right(strPurposeIDs.GetLength() - strPurposeIDs.Find(" ") - 1);
		} else  {
			nID = atoi(strPurposeIDs);
			strPurposeIDs.Empty();
		}
		adwIDs.Add(nID);
	}
}

// (z.manning, 01/23/2007) - PLID 24392 - Moved this here instead of the TemplateEntryDlg class since it's in 
// no way a member function.
void RemoveTemplate(long nTemplateID)
{
	//for auditing - get the name
	CString strTemplateName;
	try {
		_RecordsetPtr rs = CreateRecordset("SELECT Name FROM TemplateT WHERE ID = %li", nTemplateID);
		if(!rs->eof)
			strTemplateName = AdoFldString(rs, "Name", "");
	} NxCatchAll("Error getting template name.");

	// (c.haag 2015-01-09 - PLID 64444 - When deleting from TemplateItemT, handle collection records.
	// (c.haag 2015-01-15) - PLID 64595 - We no longer detect or delete orphaned TemplateCollectionApplyT records
	// because Trigger_DisallowOrphanedTemplateCollectionApplies does that for us when we delete from TemplateItemT.
	ExecuteParamSql(R"(
SET XACT_ABORT ON
BEGIN TRAN
DECLARE @templateID INT
SET @templateID = {INT}

/* Figure out which TemplateItemT records will be affected. This includes not only ones assigned to the template,
but ones with TemplateCollectionTemplateID's tied to TemplateCollectionTemplateT records that are assigned to
the template. */

DECLARE @affectedTemplateItems TABLE (ID INT NOT NULL)
DELETE FROM @affectedTemplateItems
INSERT INTO @affectedTemplateItems
    SELECT DISTINCT TemplateItemT.ID
    FROM TemplateItemT
    LEFT JOIN TemplateCollectionTemplateT ON TemplateCollectionTemplateT.ID = TemplateItemT.TemplateCollectionTemplateID
    WHERE TemplateItemT.TemplateID = @templateID
        OR TemplateCollectionTemplateT.TemplateID = @templateID

/* Delete from all the template item tables */

DELETE TemplateItemExceptionT
    FROM TemplateItemExceptionT
    INNER JOIN @affectedTemplateItems A ON A.ID = TemplateItemExceptionT.TemplateItemID
DELETE TemplateDetailsT
    FROM TemplateDetailsT
    INNER JOIN @affectedTemplateItems A ON A.ID = TemplateDetailsT.TemplateItemID
DELETE TemplateItemResourceT
    FROM TemplateItemResourceT
    INNER JOIN @affectedTemplateItems A ON A.ID = TemplateItemResourceT.TemplateItemID
DELETE TemplateItemT
    FROM TemplateItemT
    INNER JOIN @affectedTemplateItems A ON A.ID = TemplateItemT.ID

/* Delete from all the collection tables */

DELETE FROM TemplateCollectionTemplateT WHERE TemplateID = @templateID

/* Delete from all the template tables */

DELETE FROM TemplateExceptionT WHERE TemplateID = @templateID
DELETE FROM TemplateRuleDetailsT WHERE TemplateRuleID IN
    (SELECT ID FROM TemplateRuleT WHERE TemplateID = @templateID)
DELETE FROM TemplateRuleT WHERE TemplateID = @templateID
DELETE FROM TemplateT WHERE ID = @templateID

COMMIT TRAN
)", nTemplateID);

	DeleteTemplatePermissions(nTemplateID);

	//audit the deletion
	long nAuditID = BeginNewAuditEvent();
	AuditEvent(-1, strTemplateName, nAuditID, aeiTemplateDelete, nTemplateID, "", "<Deleted>", aepHigh, aetDeleted);

	// (c.haag 2006-12-12 10:58) - PLID 23808 - Notify other computers about the change.
	CClient::RefreshTable(NetUtils::TemplateT, -1);
}

//TES 6/19/2010 - PLID 5888 - Deletes a Resource Availability template
void RemoveResourceAvailTemplate(long nTemplateID)
{
	//for auditing - get the name
	CString strTemplateName;
	try {
		_RecordsetPtr rs = CreateRecordset("SELECT Name FROM ResourceAvailTemplateT WHERE ID = %li", nTemplateID);
		if(!rs->eof)
			strTemplateName = AdoFldString(rs, "Name", "");
	} NxCatchAll("Error getting resource availability template name.");

	// Actually delete
	CString strSql = BeginSqlBatch();
	AddStatementToSqlBatch(strSql, "DELETE FROM ResourceAvailTemplateItemExceptionT WHERE TemplateItemID IN "
		"(SELECT ID FROM ResourceAvailTemplateItemT WHERE TemplateID = %li)", nTemplateID);
	AddStatementToSqlBatch(strSql, "DELETE FROM ResourceAvailTemplateDetailsT WHERE TemplateItemID IN "
		"(SELECT ID FROM ResourceAvailTemplateItemT WHERE TemplateID = %li)", nTemplateID);
	// (c.haag 2006-11-07 17:37) - PLID 23336 - Delete template item resources
	AddStatementToSqlBatch(strSql, "DELETE FROM ResourceAvailTemplateItemResourceT WHERE TemplateItemID IN "
		"(SELECT ID FROM ResourceAvailTemplateItemT WHERE TemplateID = %li)", nTemplateID);
	// (c.haag 2006-11-13 10:33) - PLID 5993 - Delete template exceptions
	AddStatementToSqlBatch(strSql, "DELETE FROM ResourceAvailTemplateExceptionT WHERE TemplateID = %li", nTemplateID);
	AddStatementToSqlBatch(strSql, "DELETE FROM ResourceAvailTemplateItemT WHERE TemplateID = %li", nTemplateID);
	AddStatementToSqlBatch(strSql, "DELETE FROM ResourceAvailTemplateT WHERE ID = %li", nTemplateID);
	ExecuteSqlBatch(strSql);

	//audit the deletion
	long nAuditID = BeginNewAuditEvent();
	AuditEvent(-1, strTemplateName, nAuditID, aeiResourceAvailTemplateDelete, nTemplateID, "", "<Deleted>", aepHigh, aetDeleted);

	// (c.haag 2006-12-12 10:58) - PLID 23808 - Notify other computers about the change.
	CClient::RefreshTable(NetUtils::ResourceAvailTemplateT, -1);
}

// (z.manning, 01/23/2007) - PLID 24392 - Moved this here instead of the TemplateEntryDlg class since it's in 
// no way a member function.
void DeleteTemplatePermissions(long nTemplateID)
{
	_RecordsetPtr prsPerm = CreateRecordset("SELECT ObjectValue FROM SecurityObjectT WHERE BuiltInID = %d AND ObjectValue IN (SELECT ID FROM TemplateRuleT Where TemplateID = %d)",
		bioSchedTemplateRules, nTemplateID);
	while (!prsPerm->eof)
	{
		DeleteUserDefinedPermission(bioSchedTemplateRules, AdoFldLong(prsPerm, "ObjectValue"));
		prsPerm->MoveNext();
	}
}

// (z.manning 2011-12-09 09:49) - PLID 46906 - Added a function to delete permissions for given rule IDs
void DeleteRulePermissions(CArray<long,long> *parynRuleIDs)
{
	if(parynRuleIDs->IsEmpty()) {
		return;
	}

	_RecordsetPtr prsPerm = CreateParamRecordset(
		"SELECT ObjectValue FROM SecurityObjectT \r\n"
		"WHERE BuiltInID = {INT} AND ObjectValue IN ({INTARRAY}) \r\n"
		, bioSchedTemplateRules, *parynRuleIDs);
	for(; !prsPerm->eof; prsPerm->MoveNext())
	{
		DeleteUserDefinedPermission(bioSchedTemplateRules, AdoFldLong(prsPerm, "ObjectValue"));
	}
}

// (a.walling 2010-06-22 09:23) - PLID 23560 - Check resource sets
bool CheckExistingApptResourceSetConflicts(int nAptTypeID, int nFromResID, const CDWordArray& dwaResourceIDs, OUT CString& strMessage)
{
	strMessage.Empty();

	_RecordsetPtr prs = CreateParamRecordset(
		"IF EXISTS ( "
			"SELECT * FROM AptResourceSetLinksT "
			"INNER JOIN ResourceSetT "
				"ON AptResourceSetLinksT.ResourceSetID = ResourceSetT.ID "
			"INNER JOIN ResourceSetDetailsT "
				"ON ResourceSetT.ID = ResourceSetDetailsT.ResourceSetID "
			"INNER JOIN ResourceT "
				"ON ResourceSetDetailsT.ResourceID = ResourceT.ID "
				"WHERE AptTypeID = {INT} "
			"AND AptPurposeID IS NOT NULL "
			"AND AptPurposeID IN ( "
				"SELECT PurposeID "
				"FROM AppointmentPurposeT "
				"WHERE AppointmentID = {INT} "
			") "
			"AND ResourceT.Inactive = 0 "
		") "
			"SELECT ResourceSetDetailsT.ResourceSetID, ResourceSetT.Name AS ResourceSetName, ResourceT.ID AS ResourceID, ResourceT.Item AS ResourceName "
			"FROM AptResourceSetLinksT "
			"INNER JOIN ResourceSetT "
				"ON AptResourceSetLinksT.ResourceSetID = ResourceSetT.ID "
			"INNER JOIN ResourceSetDetailsT "
				"ON ResourceSetT.ID = ResourceSetDetailsT.ResourceSetID "
			"INNER JOIN ResourceT "
				"ON ResourceSetDetailsT.ResourceID = ResourceT.ID "
			"WHERE AptResourceSetLinksT.AptTypeID = {INT} "
			"AND AptResourceSetLinksT.AptPurposeID IN ( "
				"SELECT PurposeID "
				"FROM AppointmentPurposeT "
				"WHERE AppointmentID = {INT} "
			") "
			"AND ResourceT.Inactive = 0 "
			"ORDER BY ResourceSetT.Name, ResourceSetDetailsT.ResourceSetID, ResourceSetDetailsT.OrderIndex "
		"ELSE "
			"SELECT ResourceSetDetailsT.ResourceSetID, ResourceSetT.Name AS ResourceSetName, ResourceT.ID AS ResourceID, ResourceT.Item AS ResourceName "
			"FROM AptResourceSetLinksT "
			"INNER JOIN ResourceSetT "
				"ON AptResourceSetLinksT.ResourceSetID = ResourceSetT.ID "
			"INNER JOIN ResourceSetDetailsT "
				"ON ResourceSetT.ID = ResourceSetDetailsT.ResourceSetID "
			"INNER JOIN ResourceT "
				"ON ResourceSetDetailsT.ResourceID = ResourceT.ID "
			"WHERE AptResourceSetLinksT.AptTypeID = {INT} "
			"AND AptResourceSetLinksT.AptPurposeID IS NULL "
			"AND ResourceT.Inactive = 0 "
			"ORDER BY ResourceSetT.Name, ResourceSetDetailsT.ResourceSetID, ResourceSetDetailsT.OrderIndex",
			nAptTypeID, nFromResID, nAptTypeID, nFromResID, nAptTypeID
	);

	return CheckResourceSetConflicts(prs, dwaResourceIDs, strMessage, true);
}

bool CheckNewApptResourceSetConflicts(int nAptTypeID, const CDWordArray& dwaPurposeIDs, const CDWordArray& dwaResourceIDs, OUT CString& strMessage)
{
	// (z.manning 2010-12-09 09:56) - PLID 41767 - I reworked this function a bit. The main reason was that it
	// had previously been throwing an exception if there were not any appt purposes. In addition to fixing that
	// I also fully parameterized it and formatted it better.

	CSqlFragment sqlFragment(
		"DECLARE @nAptTypeID int \r\n"
		"SET @nAptTypeID = {INT} \r\n"
		, nAptTypeID);

	if(dwaPurposeIDs.GetSize() > 0)
	{
		sqlFragment += CSqlFragment(
			"IF EXISTS ( \r\n"
			"	SELECT * FROM AptResourceSetLinksT \r\n"
			"	INNER JOIN ResourceSetT ON AptResourceSetLinksT.ResourceSetID = ResourceSetT.ID \r\n"
			"	INNER JOIN ResourceSetDetailsT ON ResourceSetT.ID = ResourceSetDetailsT.ResourceSetID \r\n"
			"	INNER JOIN ResourceT ON ResourceSetDetailsT.ResourceID = ResourceT.ID \r\n"
			"	WHERE AptTypeID = @nAptTypeID AND AptPurposeID IS NOT NULL \r\n"
			"		AND AptPurposeID IN ({INTSTRING}) AND ResourceT.Inactive = 0 \r\n"
			") \r\n"
			"	SELECT ResourceSetDetailsT.ResourceSetID, ResourceSetT.Name AS ResourceSetName, ResourceT.ID AS ResourceID, ResourceT.Item AS ResourceName \r\n"
			"	FROM AptResourceSetLinksT \r\n"
			"	INNER JOIN ResourceSetT ON AptResourceSetLinksT.ResourceSetID = ResourceSetT.ID \r\n"
			"	INNER JOIN ResourceSetDetailsT ON ResourceSetT.ID = ResourceSetDetailsT.ResourceSetID \r\n"
			"	INNER JOIN ResourceT ON ResourceSetDetailsT.ResourceID = ResourceT.ID \r\n"
			"	WHERE AptResourceSetLinksT.AptTypeID = @nAptTypeID AND AptResourceSetLinksT.AptPurposeID IN ({INTSTRING}) \r\n"
			"		AND ResourceT.Inactive = 0 \r\n"
			"	ORDER BY ResourceSetT.Name, ResourceSetDetailsT.ResourceSetID, ResourceSetDetailsT.OrderIndex \r\n"
			"ELSE \r\n"
			, ArrayAsString(dwaPurposeIDs, true), ArrayAsString(dwaPurposeIDs, true));
	}

	sqlFragment += CSqlFragment(
		"	SELECT ResourceSetDetailsT.ResourceSetID, ResourceSetT.Name AS ResourceSetName, ResourceT.ID AS ResourceID, ResourceT.Item AS ResourceName \r\n"
		"	FROM AptResourceSetLinksT \r\n"
		"	INNER JOIN ResourceSetT ON AptResourceSetLinksT.ResourceSetID = ResourceSetT.ID \r\n"
		"	INNER JOIN ResourceSetDetailsT ON ResourceSetT.ID = ResourceSetDetailsT.ResourceSetID \r\n"
		"	INNER JOIN ResourceT ON ResourceSetDetailsT.ResourceID = ResourceT.ID \r\n"
		"	WHERE AptResourceSetLinksT.AptTypeID = @nAptTypeID AND AptResourceSetLinksT.AptPurposeID IS NULL \r\n"
		"		AND ResourceT.Inactive = 0 \r\n"
		"	ORDER BY ResourceSetT.Name, ResourceSetDetailsT.ResourceSetID, ResourceSetDetailsT.OrderIndex \r\n"
		);

	_RecordsetPtr prs = CreateParamRecordset("{SQL}", sqlFragment);
	return CheckResourceSetConflicts(prs, dwaResourceIDs, strMessage, false);
}

bool CheckResourceSetConflicts(ADODB::_RecordsetPtr prs, const CDWordArray& dwaResourceIDs, OUT CString& strMessage, bool bIncludeSaveText)
{	
	if (prs->eof) {
		// no resource sets to fulfill
		return true;
	} else {
		long nCurrentResourceSetID = -1;
		bool bCurrentResourceSetOK = false;
		CString strCurrentMessage;
		while (!prs->eof) {
			long nResourceSetID = AdoFldLong(prs, "ResourceSetID");
			CString strResourceSetName = AdoFldString(prs, "ResourceSetName");
			long nReqResourceID = AdoFldLong(prs, "ResourceID");
			CString strReqResourceName = AdoFldString(prs, "ResourceName");

			if (nCurrentResourceSetID != nResourceSetID) {
				if (!bCurrentResourceSetOK && nCurrentResourceSetID != -1 && !strCurrentMessage.IsEmpty()) {
					strCurrentMessage.TrimRight(", ");
					strMessage += strCurrentMessage;
					strMessage += "\r\n\r\n";
				}

				strCurrentMessage.Format("The '%s' resource set requires at least one of these resources:\r\n\t", strResourceSetName);
				nCurrentResourceSetID = nResourceSetID;
				bCurrentResourceSetOK = false;
			}

			strCurrentMessage += strReqResourceName + ", ";
			if (IsIDInArray((DWORD)nReqResourceID, &dwaResourceIDs)) {
				bCurrentResourceSetOK = true;
			}

			prs->MoveNext();
		}
		if (!bCurrentResourceSetOK && nCurrentResourceSetID != -1 && !strCurrentMessage.IsEmpty()) {
			strCurrentMessage.TrimRight(", ");
			strMessage += strCurrentMessage;
			strMessage += "\r\n\r\n";
		}

		if (strMessage.IsEmpty()) {
			// all resource sets are fulfilled
			return true;
		} else {
			if (bIncludeSaveText) {
				// (a.walling 2010-08-11 15:36) - PLID 23560 - Was trying to send ourselves as a parameter to Format!
				strMessage.Insert(0, "This appointment cannot be saved until the following issues are resolved:\r\n\r\n");
				strMessage += "You may continue saving, but this operation requires an administrative password.";
			}
			
			return false;
		}
	}
}

//TES 12/12/2014 - PLID 64120 - Cache whether this database has any ScheduleMixRulesT records in it
enum DatabaseHasMixRules {
	dhmrCheckData = 0,
	dhmrYes = 1,
	dhmrNo = 2,
};
DatabaseHasMixRules g_dhmr = dhmrCheckData;

//TES 12/12/2014 - PLID 64120 - Determines whether this database has any ScheduleMixRulesT records in it
bool DoesDatabaseHaveMixRules()
{
	switch (g_dhmr) {
	case dhmrCheckData:
	{
		//TES 12/12/2014 - PLID 64120 - Check the data, remember what we find
		_RecordsetPtr rsRule = CreateRecordset(GetRemoteData(), "SELECT TOP 1 * FROM ScheduleMixRulesT");
		if (rsRule->eof) {
			g_dhmr = dhmrNo;
			return false;
		}
		else {
			g_dhmr = dhmrYes;
			return true;
		}
	}
		break;
	case dhmrYes:
		return true;
		break;
	case dhmrNo:
		return false;
		break;
	default:
		ASSERT(FALSE);
		return false;
		break;
	}
}

//TES 2/11/2015 - PLID 64120 - Call this when first creating a mix rule
void SetDatabaseHasMixRules()
{
	g_dhmr = dhmrYes;
}

// (j.jones 2014-12-11 15:07) - PLID 64178 - moved CComplexTimeRange to NxSchedulerDlg

// (j.jones 2014-12-02 12:00) - PLID 64272 - tracks if a scheduler mix rule has been overridden for an appointment
void TrackAppointmentMixRuleOverride(long nAppointmentID, const IN std::vector<SchedulerMixRule> &overriddenMixRules)
{
	//throw exceptions to the caller

	if (overriddenMixRules.size() == 0) {
		//nobody should call this code if there are no override rules
		//go fix your calling code
		ASSERT(FALSE);
		return;
	}

	if (nAppointmentID == -1) {
		//this is a total failure
		ASSERT(FALSE);
		ThrowNxException("TrackAppointmentMixRuleOverride called with no appointment ID!");
	}

	Nx::SafeArray<BSTR> saryOverrideMixRuleIDs;

	for each(SchedulerMixRule rule in overriddenMixRules)
	{
		saryOverrideMixRuleIDs.Add(_bstr_t(AsString(rule.nID)));
	}

	NexTech_Accessor::_PracticeMethodsPtr pApi = GetAPI();
	if (pApi == NULL) {
		ThrowNxException("Could not call TrackAppointmentMixRuleOverride due to an invalid API.");
	}

	pApi->TrackAppointmentMixRuleOverride(GetAPISubkey(), GetAPILoginToken(), _bstr_t(AsString(nAppointmentID)), saryOverrideMixRuleIDs);
}
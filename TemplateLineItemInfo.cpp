#include "stdafx.h"
#include "Practice.h"
#include "TemplateLineItemInfo.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "CommonSchedUtils.h"
#include <foreach.h>
#include "GlobalSchedUtils.h"
#include "TemplateItemEntryGraphicalDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NexTech_Accessor;


CString GetDayText(int nDayNumber, BOOL bForceEnglish /* = FALSE*/)
{
	//JMJ - 7/28/2003 - We need to get the names of certain days internationally.
	//The "cheating" way to do this is to get the names out of a date we know corresponds to a
	//Monday, Tuesday, etc.

	COleDateTime dt;

	switch (nDayNumber) {
	case 0:
		//Sunday
		if(bForceEnglish)
			return "Sunday";
		dt.SetDate(2003,7,13);		
		break;
	case 1:
		//Monday
		if(bForceEnglish)
			return "Monday";
		dt.SetDate(2003,7,14);
		break;
	case 2:
		//Tuesday
		if(bForceEnglish)
			return "Tuesday";
		dt.SetDate(2003,7,15);
		break;
	case 3:
		//Wednesday
		if(bForceEnglish)
			return "Wednesday";
		dt.SetDate(2003,7,16);
		break;
	case 4:
		//Thursday
		if(bForceEnglish)
			return "Thursday";
		dt.SetDate(2003,7,17);
		break;
	case 5:
		//Friday
		if(bForceEnglish)
			return "Friday";
		dt.SetDate(2003,7,18);
		break;
	case 6:
		//Saturday
		if(bForceEnglish)
			return "Saturday";
		dt.SetDate(2003,7,19);
		break;
	default:
		//Sunday
		if(bForceEnglish)
			return "Sunday";
		dt.SetDate(2003,7,13);
		break;
	}

	return FormatDateTimeForInterface(dt,"%A");
}

CTemplateLineItemInfo::CTemplateLineItemInfo()
{
	m_nID = -1;
	m_nTemplateID = -1;
	m_nInclude = 0;
	// (c.haag 2006-11-03 10:44) - PLID 23336 - This field is now superceded by
	// TemplateItemT.StartDate; or in this class, m_dtStartDate
	//m_dtPivotDate = COleDateTime::GetCurrentTime();
	m_embBy = mbPattern;
	m_esScale = sOnce;


	// Standard default values
	m_nPeriod = 1;
	m_nPatternOrdinal = 1;
	m_nDayNumber = 1;

	// (b.cardillo 2005-10-14 16:50) - PLID 17954 - Use the COleDateTime directly rather than 
	// converting back and forth to CString, so as to avoid any problems from regional settings.
	m_dtStartTime.SetTime(9,0,0);
	m_dtEndTime.SetTime(17,0,0);

	// (c.haag 2006-11-28 09:57) - PLID 23336 - The default start and end dates should not have
	// a time value
	m_dtStartDate = COleDateTime::GetCurrentTime();
	m_dtStartDate.SetDate(m_dtStartDate.GetYear(), m_dtStartDate.GetMonth(), m_dtStartDate.GetDay());
	m_dtEndDate = m_dtStartDate;

	m_bAllResources = TRUE;
	m_bIsBlock = FALSE;

	m_bUseResourceAvailTemplates = false;

	m_nCollectionID = -1;
}

CTemplateLineItemInfo::CTemplateLineItemInfo(CTemplateLineItemInfo* pSourceLineItem)
{
	CopyFrom(pSourceLineItem);
}

CTemplateLineItemInfo::~CTemplateLineItemInfo()
{
}

void CTemplateLineItemInfo::CopyFrom(CTemplateLineItemInfo* pSourceLineItem)
{
	m_nID = pSourceLineItem->m_nID;
	m_nTemplateID = pSourceLineItem->m_nTemplateID;
	m_nInclude = pSourceLineItem->m_nInclude;
	m_embBy = pSourceLineItem->m_embBy;
	m_esScale = pSourceLineItem->m_esScale;
	m_nPeriod = pSourceLineItem->m_nPeriod;
	m_nPatternOrdinal = pSourceLineItem->m_nPatternOrdinal;
	m_nDayNumber = pSourceLineItem->m_nDayNumber;
	m_dtStartTime = pSourceLineItem->m_dtStartTime;
	m_dtEndTime = pSourceLineItem->m_dtEndTime;
	m_dtStartDate = pSourceLineItem->m_dtStartDate;
	m_dtEndDate = pSourceLineItem->m_dtEndDate;
	m_bAllResources = pSourceLineItem->m_bAllResources;
	m_bIsBlock = pSourceLineItem->m_bIsBlock;

	m_aryResources.RemoveAll();
	m_aryResources.Append(pSourceLineItem->m_aryResources);

	m_arydtExceptionDates.RemoveAll();
	m_arydtExceptionDates.Copy(pSourceLineItem->m_arydtExceptionDates);

	m_bUseResourceAvailTemplates = pSourceLineItem->m_bUseResourceAvailTemplates;

	// (z.manning 2014-12-18 15:23) - PLID 64295
	m_nCollectionID = pSourceLineItem->m_nCollectionID;
}

void CTemplateLineItemInfo::operator =(CTemplateLineItemInfo &lineitemSource)
{
	CopyFrom(&lineitemSource);
}

// (z.manning 2011-12-07 16:13) - PLID 46906
bool CTemplateLineItemInfo::operator ==(CTemplateLineItemInfo &lineitemCompare)
{
	if(m_nID != lineitemCompare.m_nID ||
		m_nTemplateID != lineitemCompare.m_nTemplateID ||
		m_esScale != lineitemCompare.m_esScale ||
		m_nPeriod != lineitemCompare.m_nPeriod ||
		m_nInclude != lineitemCompare.m_nInclude ||
		m_embBy != lineitemCompare.m_embBy ||
		m_nPatternOrdinal != lineitemCompare.m_nPatternOrdinal ||
		m_nDayNumber != lineitemCompare.m_nDayNumber ||
		m_dtStartTime != lineitemCompare.m_dtStartTime ||
		m_dtEndTime != lineitemCompare.m_dtEndTime ||
		m_dtStartDate != lineitemCompare.m_dtStartDate ||
		m_dtEndDate != lineitemCompare.m_dtEndDate ||
		m_bAllResources != lineitemCompare.m_bAllResources ||
		m_bIsBlock != lineitemCompare.m_bIsBlock ||
		m_bUseResourceAvailTemplates != lineitemCompare.m_bUseResourceAvailTemplates ||
		m_nCollectionID != lineitemCompare.m_nCollectionID // (z.manning 2014-12-18 15:31) - PLID 64295
		)
	{
		return FALSE;
	}

	if(m_arydtExceptionDates.GetCount() != lineitemCompare.m_arydtExceptionDates.GetCount()) {
		return FALSE;
	}
	for(int nExceptionIndex = 0; nExceptionIndex < m_arydtExceptionDates.GetCount(); nExceptionIndex++) {
		COleDateTime dtException = m_arydtExceptionDates.GetAt(nExceptionIndex);
		if(!IsDateTimeInDateTimeArray(dtException, &lineitemCompare.m_arydtExceptionDates)) {
			return FALSE;
		}
	}

	if(m_aryResources.GetCount() != lineitemCompare.m_aryResources.GetCount()) {
		return FALSE;
	}
	for(int nResourceIndex = 0; nResourceIndex < m_aryResources.GetCount(); nResourceIndex++)
	{
		Resource resource = m_aryResources.GetAt(nResourceIndex);
		BOOL bFound = FALSE;
		for(int nCompareResource = 0; nCompareResource < lineitemCompare.m_aryResources.GetCount() && !bFound; nCompareResource++) {
			Resource resourceCompare = lineitemCompare.m_aryResources.GetAt(nCompareResource);
			if(resource == resourceCompare) {
				bFound = TRUE;
			}
		}

		if(!bFound) {
			return FALSE;
		}
	}

	return TRUE;
}

// (z.manning 2011-12-08 09:50) - PLID 46906
bool CTemplateLineItemInfo::operator !=(CTemplateLineItemInfo &lineitemCompare)
{
	return !(*this == lineitemCompare);
}

CString GetScaleWord(EScale esScale, bool bNormalCase /* = true */, bool bSingular /* = true */)
{
	//
	// (c.haag 2006-11-08 15:51) - PLID 23336 - Given a scale value, this function returns the
	// string equivalent.
	//
	CString strAns;

	switch (esScale) {
	case sDaily:
		strAns = "Day";
		break;
	case sWeekly:
		strAns = "Week";
		break;
	case sMonthly:
		strAns = "Month";
		break;
	case sYearly:
		strAns = "Year";
		break;
	default:
		strAns = "";
		break;
	}

	if (!strAns.IsEmpty()) {
		if (!bNormalCase) {
			strAns.MakeLower();
		}
		
		if (!bSingular) {
			strAns += 's';
		}
	}

	return strAns;
}

CString GetDayNumberText(int nDayNumber, bool bNormalCase)
{
	//
	// (c.haag 2006-11-08 15:22) - PLID 23336 - This function takes a day number
	// and returns 'Day x' where x is the day.
	//
	// If the day number is -1, it means the day is invalid, and we should just
	// return an empty string.
	//
	ASSERT(nDayNumber >= -1);
	if (nDayNumber >= 0) {
		CString strAns;
		if (nDayNumber > 0 && nDayNumber < 32) {
			strAns.Format("Day %i ", nDayNumber);
		} else {
			strAns.Format("Day x ");
		}
		if (!bNormalCase) strAns.MakeLower();
		return strAns;
	} else {
		return "";
	}
}

void GetDays(EScale esScale, EMonthBy embBy, int nData,
								   int &nDays, int &nDayCnt, int nCheckVal /* = 1 */)
{
	if ((esScale == sOnce) || (esScale == sYearly) || ((esScale == sMonthly) && (embBy == mbDate))) {
		nDays = dowEveryday;
		nDayCnt = 7;
		return;
	}
	nDays = 0;
	nDayCnt = 0;
	if (((nData & dowMonday) ? 1 : 0) == nCheckVal) {
		nDays = nDays | dowMonday;
		nDayCnt++;
	}
	if (((nData & dowTuesday) ? 1 : 0) == nCheckVal) {
		nDays = nDays | dowTuesday;
		nDayCnt++;
	}
	if (((nData & dowWednesday) ? 1 : 0) == nCheckVal) {
		nDays = nDays | dowWednesday;
		nDayCnt++;
	}
	if (((nData & dowThursday) ? 1 : 0) == nCheckVal) {
		nDays = nDays | dowThursday;
		nDayCnt++;
	}
	if (((nData & dowFriday) ? 1 : 0) == nCheckVal) {
		nDays = nDays | dowFriday;
		nDayCnt++;
	}
	if (((nData & dowSaturday) ? 1 : 0) == nCheckVal) {
		nDays = nDays | dowSaturday;
		nDayCnt++;
	}
	if (((nData & dowSunday) ? 1 : 0) == nCheckVal) {
		nDays = nDays | dowSunday;
		nDayCnt++;
	}
}

void GetDaysEx(EScale esScale, EMonthBy embBy, int nData,
									 int &nDays, int &nDayCnt, UINT& nIncluded)
{
	if ((esScale == sOnce) || (esScale == sYearly) || ((esScale == sMonthly) && (embBy == mbDate))) {
		nDays = dowEveryday;
		nDayCnt = 7;
		nIncluded = 1;
		return;
	}
	nDays = 0;
	nDayCnt = 0;
	if (nData & dowMonday) {
		nDays = nDays | dowMonday;
		nDayCnt++;
	}
	if (nData & dowTuesday) {
		nDays = nDays | dowTuesday;
		nDayCnt++;
	}
	if (nData & dowWednesday) {
		nDays = nDays | dowWednesday;
		nDayCnt++;
	}
	if (nData & dowThursday) {
		nDays = nDays | dowThursday;
		nDayCnt++;
	}
	if (nData & dowFriday) {
		nDays = nDays | dowFriday;
		nDayCnt++;
	}
	if (nData & dowSaturday) {
		nDays = nDays | dowSaturday;
		nDayCnt++;
	}
	if (nData & dowSunday) {
		nDays = nDays | dowSunday;
		nDayCnt++;
	}

	if (nDayCnt > 3)
	{
		nDays ^= (UINT)-1;
		nDayCnt = 7 - nDayCnt;
		nIncluded = 0;
	}
	else
		nIncluded = 1;
}

CString GetDaysText(EScale esScale, EMonthBy embBy, int nData, UINT nCheckVal)
{
	//
	// (c.haag 2006-11-08 15:35) - PLID 23336 - This function takes in an integer
	// consisting of weekday flags (monday, tuesday, wednesday...) and converts
	// it into a string describing what days are flagged, such as "every day"
	// or "weekends" or "monday, tuesday and thursday".
	//
	CString strComma = "";
	CString strAns = "";
	int nDays;
	int nDayCnt = 0;

	GetDays(esScale, embBy, nData, nDays, nDayCnt, nCheckVal);

	if (nDayCnt == 0) {
		strAns = "none";
	} else {
		if (nDays == dowEveryday) {
			strAns = "every day";
		} else if (nDays == dowWeekends) { // Weekends
			strAns = "weekend days";
		} else if (nDays == dowWeekdays) { // Weekdays
			strAns = "week days";
		} else { // Enumerate days
			strComma = (nDayCnt-- > 1) ? " and ": "";
			if (nDays & dowSunday) {
				strAns = strComma + GetDayText(0) + strAns;
				strComma = (--nDayCnt > 0)? ", ": "";
			}
			if (nDays & dowSaturday) {
				strAns = strComma + GetDayText(6) + strAns;
				strComma = (--nDayCnt > 0)? ", ": "";
			}
			if (nDays & dowFriday) {
				strAns = strComma + GetDayText(5) + strAns;
				strComma = (--nDayCnt > 0)? ", ": "";
			}
			if (nDays & dowThursday) { 
				strAns = strComma + GetDayText(4) + strAns;
				strComma = (--nDayCnt > 0)? ", ": "";
			}
			if (nDays & dowWednesday) { 
				strAns = strComma + GetDayText(3) + strAns;
				strComma = (--nDayCnt > 0)? ", ": "";
			}
			if (nDays & dowTuesday) { 
				strAns = strComma + GetDayText(2) + strAns;
				strComma = (--nDayCnt > 0)? ", ": "";
			}
			if (nDays & dowMonday) { 
				strAns = strComma + GetDayText(1) + strAns;
				strComma = (--nDayCnt > 0)? ", ": "";
			}
		}
	}

	return strAns;
}

CString GetDaysTextEx(EScale esScale, EMonthBy embBy, int nData, UINT& nIncluded)
{
	CString strComma = "";
	CString strAns = "";
	int nDays;
	int nDayCnt;

	GetDaysEx(esScale, embBy, nData, nDays, nDayCnt, nIncluded);

	if (nDayCnt == 0) {
		strAns = "none";
	} else {
		if (nDays == dowEveryday) {
			strAns = "every day";
		} else if (nDays == dowWeekends) { // Weekends
			strAns = "weekend days";
		} else if (nDays == dowWeekdays) { // Weekdays
			strAns = "week days";
		} else { // Enumerate days
			strComma = (nDayCnt-- > 1) ? " and ": "";
			if (nDays & dowSunday) {
				strAns = strComma + GetDayText(0) + strAns;
				strComma = (--nDayCnt > 0)? ", ": "";
			}
			if (nDays & dowSaturday) {
				strAns = strComma + GetDayText(6) + strAns;
				strComma = (--nDayCnt > 0)? ", ": "";
			}
			if (nDays & dowFriday) {
				strAns = strComma + GetDayText(5) + strAns;
				strComma = (--nDayCnt > 0)? ", ": "";
			}
			if (nDays & dowThursday) { 
				strAns = strComma + GetDayText(4) + strAns;
				strComma = (--nDayCnt > 0)? ", ": "";
			}
			if (nDays & dowWednesday) { 
				strAns = strComma + GetDayText(3) + strAns;
				strComma = (--nDayCnt > 0)? ", ": "";
			}
			if (nDays & dowTuesday) { 
				strAns = strComma + GetDayText(2) + strAns;
				strComma = (--nDayCnt > 0)? ", ": "";
			}
			if (nDays & dowMonday) { 
				strAns = strComma + GetDayText(1) + strAns;
				strComma = (--nDayCnt > 0)? ", ": "";
			}
		}
	}

	if (!strAns.IsEmpty())
		strAns += " ";
	return strAns;
}

// (c.haag 2006-11-08 15:43) - PLID 23336 - This is a new function I wrote.
// Given a pattern ordinal value, it returns text
CString GetPatternText(int nPatternOrdinal)
{
	switch (nPatternOrdinal) {
	case -1: return "Last";
	case 1: return "First";
	case 2: return "Second";
	case 3: return "Third";
	case 4: return "Fourth";
	case 5: return "Fifth";
	default: 
		ASSERT(FALSE);
		return "";
	}
}

CString GetPreTimeRangeText(EScale esScale,
	const COleDateTime& dtStartTime, const COleDateTime& dtEndTime, bool &bNormalCase)
{
	// (z.manning 2014-12-12 15:19) - PLID 64228 - Skip if times aren't valid
	if (dtStartTime.GetStatus() != COleDateTime::valid && dtEndTime.GetStatus() != COleDateTime::valid) {
		return "";
	}

	CString strAns;
	switch (esScale) {
	case sOnce:
	case sDaily:
	case sMonthly:
	case sYearly: {
		// From start time to end time
		CString strStart, strEnd;
		strStart = FormatDateTimeForInterface(dtStartTime, DTF_STRIP_SECONDS, dtoTime);
		strEnd = FormatDateTimeForInterface(dtEndTime, DTF_STRIP_SECONDS, dtoTime);
		strAns.Format("From %s to %s ", strStart, strEnd); }
		if (bNormalCase) {
			bNormalCase = false;
		} else {
			strAns.MakeLower();
		}
		break;
	default:
		strAns.Empty();
		break;
	}
	return strAns;
}

CString GetPreIncludeText(EScale esScale, EMonthBy embBy, int nInclude, const COleDateTime& dtStartDate, 
												 int nDayNumber, int nPatternOrdinal, bool &bNormalCase)
{
	CString strAns;
	CString strOn;
	CString strMonth;
	if (bNormalCase) {
		strOn = "On";
	} else {
		strOn = "on";
	}
	switch (esScale) {
	case sDaily:
		// On 
		strAns = strOn + " ";
		bNormalCase = false;
		break;
	case sMonthly:
		if (embBy == mbDate) {
			// On day 2 of
			strAns.Format("%s %sof ", strOn, GetDayNumberText(nDayNumber, false));
		} else {
			// On the first Monday of
			CString strTemp = GetPatternText(nPatternOrdinal);
			strTemp.MakeLower();
			strAns.Format("%s the %s %s of ", strOn, strTemp, GetDaysText(esScale, embBy, nInclude, 1));
		}
		bNormalCase = false;
		break;
	case sYearly:
		// On September 27 of
		// (c.haag 2006-11-03 11:02) - PLID 23336 - The pivot date
		// field has been replaced with the template item start date
		strMonth = FormatDateTimeForInterface(dtStartDate,"%B");
		//strAns.Format("%s %s %i of ", strOn, m_dtPivotDate.Format("%B"), m_dtPivotDate.GetDay());
		strAns.Format("%s %s %i of ", strOn, strMonth, dtStartDate.GetDay());
		/*
		strMonth = FormatDateTimeForInterface(m_dtPivotDate,"%B");
		//strAns.Format("%s %s %i of ", strOn, m_dtPivotDate.Format("%B"), m_dtPivotDate.GetDay());
		strAns.Format("%s %s %i of ", strOn, strMonth, m_dtPivotDate.GetDay());
		*/
		bNormalCase = false;
		break;
	default:
		strAns.Empty();
		break;
	}

	return strAns;
}

CString GetPeriodText(EScale esScale, int nPeriod, bool &bNormalCase)
{
	CString strAns;
	switch (esScale) {
	case sDaily:
	case sWeekly:
	case sMonthly:
	case sYearly:
		if (1 == nPeriod) {
			strAns.Format("Every %s ", GetScaleWord(esScale, false));
		} else if (2 == nPeriod) {
			strAns.Format("Every other %s ", GetScaleWord(esScale, false));
		} else {
			CString strPeriod;
			strAns.Format("Every %d %s ", nPeriod, GetScaleWord(esScale, false, false));
		}
		if (bNormalCase) {
			bNormalCase = false;
		} else {
			strAns.MakeLower();
		}
		break;
	default:
		strAns.Empty();
		break;
	}
	return strAns;
}

CString GetPostIncludeText(EScale esScale, EMonthBy embBy, int nData, bool &bNormalCase)
{
	CString strAns;
	UINT uiChecked;

	switch (esScale) {
	/*case sDaily:
		// Excluding Saturdays and Sundays, 
		if (bNormalCase) {
			strAns.Format("Excluding %s ", GetDaysText(0));
			bNormalCase = false;
		} else {
			strAns.Format("excluding %s ", GetDaysText(0));
		}
		break;
	case sWeekly:
		// On Mondays, Tuesdays, and Wednesdays
		if (bNormalCase) {
			strAns.Format("On %s ", GetDaysText(1));
			bNormalCase = false;
		} else {
			strAns.Format("on %s ", GetDaysText(1));
		}
		break;*/

	case sDaily:
	case sWeekly:
		strAns = GetDaysTextEx(esScale, embBy, nData, uiChecked);
		if (!uiChecked)
		{
			if (bNormalCase) {
				strAns = CString("Excluding ") + strAns;
				bNormalCase = false;
			} else {
				strAns = CString("excluding ") + strAns;
			}
		}
		else {
			if (bNormalCase) {
				strAns = CString("On ") + strAns;
				bNormalCase = false;
			} else {
				strAns = CString("on ") + strAns;
			}
		}
		break;

	default:
		strAns.Empty();
		break;
	}

	return strAns;
}

CString GetPostTimeRangeText(EScale esScale, const COleDateTime& dtStartTime, 
													const COleDateTime& dtEndTime,	bool &bNormalCase)
{
	// (z.manning 2014-12-12 15:19) - PLID 64228 - Skip if times aren't valid
	if (dtStartTime.GetStatus() != COleDateTime::valid && dtEndTime.GetStatus() != COleDateTime::valid) {
		return "";
	}

	CString strAns;

	switch (esScale) {
	case sWeekly: {
		// From start time to end time
		CString strStart, strEnd;
		strStart = FormatDateTimeForInterface(dtStartTime, DTF_STRIP_SECONDS, dtoTime);
		strEnd = FormatDateTimeForInterface(dtEndTime, DTF_STRIP_SECONDS, dtoTime);
		strAns.Format("From %s to %s ", strStart, strEnd); }
		if (bNormalCase) {
			bNormalCase = false;
		} else {
			strAns.MakeLower();
		}
		break;
	default:
		strAns.Empty();
		break;
	}
	return strAns;
}

CString GetDateRangeText(EScale esScale, const COleDateTime& dtStartDate, 
												const COleDateTime& dtEndDate, bool &bNormalCase)
{
	//
	// (c.haag 2006-11-08 17:32) - PLID 23336 - This returns formatted text given a date range.
	// It applies to all scales now.
	//
	CString strAns;
	BOOL bUseTimeRange = (COleDateTime::valid == dtEndDate.GetStatus() && dtEndDate != dtStartDate) ? TRUE : FALSE;
	CString strStartDate;
	CString strEndDate;

	switch (esScale) {
	case sMonthly:
		strStartDate = dtStartDate.Format("%B %Y");
		strEndDate = dtEndDate.Format("%B %Y");
		break;

	case sYearly:
		strStartDate.Format("%d", dtStartDate.GetYear());
		strEndDate.Format("%d", dtEndDate.GetYear());
		break;

	default:
		strStartDate = FormatDateTimeForInterface(dtStartDate,DTF_STRIP_SECONDS, dtoDate);
		strEndDate = FormatDateTimeForInterface(dtEndDate,DTF_STRIP_SECONDS, dtoDate);
		break;
	}

	// On September 27, 1998
	if (bNormalCase) {
		//strAns.Format("On %s ", m_dtPivotDate.Format("%#x"));

		// (c.haag 2006-11-03 11:02) - PLID 23336 - The pivot date
		// field has been replaced with the template item start date
		//strAns.Format("On %s ", FormatDateTimeForInterface(m_dtPivotDate,DTF_STRIP_SECONDS, dtoDate));

		if (COleDateTime::invalid == dtEndDate.GetStatus()) {
			strAns.Format("Starting on %s ", strStartDate);
		} else {
			if (bUseTimeRange) {
				strAns.Format("From %s ", strStartDate);
			} else {
				strAns.Format("On %s ", strStartDate);
			}
		}
		bNormalCase = false;
	} else {
		//strAns.Format("on %s ", m_dtPivotDate.Format("%#x"));

		// (c.haag 2006-11-03 11:02) - PLID 23336 - The pivot date
		// field has been replaced with the template item start date
		//strAns.Format("on %s ", FormatDateTimeForInterface(m_dtPivotDate,DTF_STRIP_SECONDS, dtoDate));
		if (COleDateTime::invalid == dtEndDate.GetStatus()) {
			strAns.Format("starting on %s ", strStartDate);
		} else {
			if (bUseTimeRange) {
				strAns.Format("from %s ", strStartDate);
			} else {
				strAns.Format("on %s ", strStartDate);
			}
		}
	}
	// Change the wording if it's a range or dates.
	// (c.haag 2006-11-09 09:41) - PLID 23336 - Ignore this if there is no end time
	//if(m_dtEndDate > m_dtPivotDate) {
	if (bUseTimeRange) {
		strAns = strAns + "through " + strEndDate + " ";
	}
	return strAns;
}

CString GetDateExceptionText(const CArray<COleDateTime>& arydtExceptionDates, bool &bNormalCase)
{
	//
	// (c.haag 2006-11-20 10:35) - PLID 23589 - Add text that describes the state of line item exception dates
	//
	CString strAns;
	int nExceptions = arydtExceptionDates.GetSize();
	if (0 == nExceptions) {
		return "";
	} else if (1 == nExceptions) {
		if (bNormalCase) {
			strAns.Format("Excluding %s ", FormatDateTimeForInterface(arydtExceptionDates[0], 0, dtoDate));
		} else {
			strAns.Format("and excluding %s ", FormatDateTimeForInterface(arydtExceptionDates[0], 0, dtoDate));
		}
	} else {
		if (bNormalCase) {
			strAns.Format("Excluding ");
		} else {
			strAns.Format("and excluding ");
		}
		for (int i=0; i < nExceptions; i++) {
			strAns += FormatDateTimeForInterface(arydtExceptionDates[i], 0, dtoDate);
			if (i < nExceptions - 2) {
				strAns += ", ";
			} else if (i < nExceptions - 1) {
				strAns += " and ";
			}
		}
		strAns += " ";
	}

	bNormalCase = false;
	return strAns;
}

CString GetResourceText(BOOL bAllResources, const CArray<Resource>& aryResources, bool &bNormalCase)
{
	//
	// (c.haag 2006-11-09 10:23) - PLID 23336 - This new function pulls a text description of resource
	// of resource names. We use GetTableField to get the resource name because, while not optimal, it's
	// cleaner than changing all other areas of Practice to pull a recordset of resource names. If it's
	// a major issue after all, we should revisit it later.
	//
	CString strAns;
	if (bAllResources) {
		if (bNormalCase) {
			strAns = "For all resources ";
		} else {
			strAns = "for all resources ";
		}
	} else {
		if (aryResources.GetSize() > 0) {
			if (bNormalCase) {
				strAns = "For the ";
			} else {
				strAns = "for the ";
			}
			for (int i=0; i < aryResources.GetSize(); i++) {
				strAns += aryResources.GetAt(i).m_strName;
				if (i < aryResources.GetSize() - 2) {
					strAns += ", ";
				} else if (i < aryResources.GetSize() - 1) {
					strAns += " and ";
				}
			}

			if (i == 1) {
				strAns += " resource ";
			} else {
				strAns += " resources ";
			}
		}
	}
	bNormalCase = false;
	return strAns;
}

// (z.manning 2015-01-30 15:43) - PLID 64230 - Added type param
CString CTemplateLineItemInfo::GetApparentDescription(ESchedulerTemplateEditorType eTemplateType)
{
	//
	// (c.haag 2006-11-08 12:25) - PLID 9036 - This function returns the apparent description of this
	// template line item based on the contents of this specific object
	//
	return ::GetApparentDescription(eTemplateType, m_esScale, m_nPeriod, m_nInclude, m_embBy, m_nPatternOrdinal,
		m_nDayNumber, m_dtStartTime, m_dtEndTime, m_dtStartDate, m_dtEndDate, m_bAllResources,
		m_aryResources, m_arydtExceptionDates);
}

// (z.manning 2014-12-12 11:50) - PLID 64228 - This is no longer a class member
// (z.manning 2015-01-30 15:43) - PLID 64230 - Added type param
CString GetApparentDescription(ESchedulerTemplateEditorType eTemplateType, EScale esScale, int nPeriod, int nInclude, EMonthBy embBy, int nPatternOrdinal,
	int nDayNumber, const COleDateTime& dtStartTime, const COleDateTime& dtEndTime,	const COleDateTime& dtStartDate,
	const COleDateTime& dtEndDate, BOOL bAllResources, const CArray<Resource>& aryResources, const CArray<COleDateTime>& arydtExceptionDates)
{
	//
	// (c.haag 2006-11-08 12:26) - PLID 9036 - This function calculates and returns the apparent
	// description of an arbitrary line item given its parameters.
	//
	// Unlike in the past, we deal with some sets of values as invalid
	//
	int i;
	CString strItemDesc;
	if (eTemplateType == stetCollection) {
		strItemDesc = "collection apply";
	}
	else {
		strItemDesc = "template line item";
	}

	if (COleDateTime::valid == dtEndDate.GetStatus() && dtStartDate > dtEndDate) {
		return FormatString("*** This %s is invalid because the start date is after the end date ***", strItemDesc);
	}
	else if (dtStartTime.GetStatus() == COleDateTime::valid && dtEndTime.GetStatus() == COleDateTime::valid && dtStartTime >= dtEndTime) {
		return FormatString("*** This %s is invalid because the start time is on or after the end time ***", strItemDesc);
	}
	else if (nPeriod <= 0) {
		return FormatString("*** This %s is invalid because an invalid number of days was specified for the period ***", strItemDesc);
	}
	else if (sMonthly == esScale && mbNone == embBy) {
		return FormatString("*** This %s is invalid because a valid monthly scale was not selected ***", strItemDesc);
	}
	else if (sOnce != esScale && sYearly != esScale && !nInclude) {
		return FormatString("*** This %s is invalid because no weekdays were selected ***", strItemDesc);
	}
	else if (sMonthly == esScale && mbDate == embBy && !(nDayNumber > 0 && nDayNumber < 32)) {
		return FormatString("*** This %s is invalid because the day of month field is not properly set ***", strItemDesc);
	}
	// (c.haag 2006-11-20 10:29) - PLID 23589 - Warn if the template line item exception dates are out of bounds
	for (i=0; i < arydtExceptionDates.GetSize(); i++) {
		COleDateTime dtException = arydtExceptionDates[i];
		if (dtException < dtStartDate) {
			return FormatString("*** This %s is invalid because there is a date in the exclude list which occurs before the start date ***", strItemDesc);
		} else if (COleDateTime::valid == dtEndDate.GetStatus() && dtException > dtEndDate) {
			return FormatString("*** This %s is invalid because there is a date in the exclude list which occurs after the end date ***", strItemDesc);
		}
	}

	CString strResults;
	strResults.Empty();
	bool bNormalCase = true;
	strResults += GetPreTimeRangeText(esScale, dtStartTime, dtEndTime, bNormalCase);
	strResults += GetPreIncludeText(esScale, embBy, nInclude, dtStartDate, nDayNumber, nPatternOrdinal, bNormalCase);
	strResults += GetPeriodText(esScale, nPeriod, bNormalCase);
	strResults += GetPostIncludeText(esScale, embBy, nInclude, bNormalCase);
	strResults += GetPostTimeRangeText(esScale, dtStartTime, dtEndTime, bNormalCase);
	strResults += GetDateRangeText(esScale, dtStartDate, dtEndDate, bNormalCase);
	strResults += GetDateExceptionText(arydtExceptionDates, bNormalCase);
	strResults += GetResourceText(bAllResources, aryResources, bNormalCase);

	//we're going to re-word things now based on various cases
	//remove the "on every day" if we are specifying non-excluded days
	if((strResults.Find("on every day") != -1 || strResults.Find("on Every day") != -1)
		&& strResults.Find("excluding") == -1) {
		strResults.Replace(" on every day","");
		strResults.Replace(" on Every day","");
	}
	//remove "excluding none" - it's not needed
	if(strResults.Find("excluding none") != -1) {
		strResults.Replace(" excluding none","");
	}
	return strResults;
}

// (z.manning 2010-08-31 13:29) - PLID 39596 - Changed return type to bool and added silent parameter.
BOOL CTemplateLineItemInfo::LoadFromID(long nLineItemID, BOOL bSilent /* = FALSE */)
{
	//TES 6/19/2010 - PLID 39262 - Pull from the appropriate tables.
	CString strTablePrefix = m_bUseResourceAvailTemplates?"ResourceAvail":"";
	// (z.manning 2011-12-07 10:47) - PLID 46910 - Changed this query to also load template item exceptions
	// (c.haag 2014-12-16) - PLID 64256 - Added CollectionID
	ADODB::_RecordsetPtr prs = CreateParamRecordset(FormatString(
		"SELECT %sTemplateItemT.*, dbo.Get%sTemplateItemResourceIDString(%sTemplateItemT.ID) AS ResourceIDString \r\n"
		"	, Date AS ExceptionDate \r\n"
		"%s"
		"FROM %sTemplateItemT \r\n"
		"%s"
		"LEFT JOIN %sTemplateItemExceptionT ON %sTemplateItemT.ID = %sTemplateItemExceptionT.TemplateItemID \r\n"
		"WHERE %sTemplateItemT.ID = {INT} \r\n"
		"ORDER BY %sTemplateItemT.ID \r\n"
		, strTablePrefix, strTablePrefix, strTablePrefix
		, m_bUseResourceAvailTemplates ? ", NULL AS CollectionID \r\n" : ", CollectionID \r\n"
		, strTablePrefix
		, m_bUseResourceAvailTemplates ? "" : "LEFT JOIN TemplateCollectionApplyT ON TemplateCollectionApplyT.ID = TemplateItemT.TemplateCollectionApplyID \r\n"
		, strTablePrefix, strTablePrefix, strTablePrefix
		, strTablePrefix
		, strTablePrefix)
		, nLineItemID);
	if(prs->eof) {
		// (z.manning 2010-08-31 13:32) - PLID 39596 - We may want to ignore this exception
		if(!bSilent) {
			AfxThrowNxException(FormatString("Unable to find data for line item with ID = %li",nLineItemID));
		}
		return FALSE;
	}
	LoadFromRecordset(prs, TRUE, NULL);

	// (z.manning 2011-12-07 14:29) - PLID 46910 - Since we did not pass in a resource map to LoadFromRecordset we need
	// to handle resource names here.
	if(m_aryResources.GetCount() > 0)
	{
		CArray<long> arynResourceIDs;
		for(int nResourceIndex = 0; nResourceIndex < m_aryResources.GetCount(); nResourceIndex++) {
			arynResourceIDs.Add(m_aryResources.GetAt(nResourceIndex).m_nID);
		}

		CMap<long,long,CString,LPCTSTR> mapResources;
		FillResourceMap(&mapResources, &arynResourceIDs);

		for(nResourceIndex = 0; nResourceIndex < m_aryResources.GetCount(); nResourceIndex++)
		{
			Resource resource = m_aryResources.GetAt(nResourceIndex);
			mapResources.Lookup(resource.m_nID, resource.m_strName);
			m_aryResources.SetAt(nResourceIndex, resource);
		}
	}

	return TRUE;
}

void CTemplateLineItemInfo::LoadFromRecordset(ADODB::_RecordsetPtr prsTemplateItemT, BOOL bLoadExceptions, CMap<long,long,CString,LPCTSTR> *pmapResources)
{
	m_nID = AdoFldLong(prsTemplateItemT, "ID");
	m_nTemplateID = AdoFldLong(prsTemplateItemT, "TemplateID");
	m_embBy = (EMonthBy)AdoFldLong(prsTemplateItemT, "MonthBy");
	m_esScale = (EScale)AdoFldLong(prsTemplateItemT, "Scale");
	m_nDayNumber = AdoFldLong(prsTemplateItemT, "DayNumber");
	m_nInclude = AdoFldLong(prsTemplateItemT, "ApparentInclude");
	m_nPatternOrdinal = AdoFldLong(prsTemplateItemT, "PatternOrdinal");
	m_nPeriod = AdoFldLong(prsTemplateItemT, "Period");
	// (c.haag 2006-11-20 11:23) - PLID 23605 - Load block status
	//TES 6/19/2010 - PLID 39262 - Resource Availability templates don't have this field
	m_bIsBlock = m_bUseResourceAvailTemplates ? FALSE : AdoFldBool(prsTemplateItemT, "IsBlock");
	// (c.haag 2006-11-02 16:26) - PLID 23336 - StartDate and EndDate fields now belong
	// in template line items
	COleDateTime dtInvalid;
	dtInvalid.m_status = COleDateTime::invalid;
	m_dtStartDate = AdoFldDateTime(prsTemplateItemT, "StartDate", dtInvalid);
	m_dtEndDate = AdoFldDateTime(prsTemplateItemT, "EndDate", dtInvalid);
	// (z.manning 2014-12-18 16:02) - PLID 64295
	m_nCollectionID = AdoFldLong(prsTemplateItemT, "CollectionID", -1);

	// (b.cardillo 2005-10-14 16:50) - PLID 17954 - Use the COleDateTime directly rather than 
	// converting back and forth to CString, so as to avoid any problems from regional settings.
	// Also make sure the datetime contains ONLY time.
	COleDateTime dt;
	dt = AdoFldDateTime(prsTemplateItemT, "StartTime");
	m_dtStartTime.SetTime(dt.GetHour(), dt.GetMinute(), 0);
	dt = AdoFldDateTime(prsTemplateItemT, "EndTime");
	m_dtEndTime.SetTime(dt.GetHour(), dt.GetMinute(), 0);

	m_bAllResources = AdoFldBool(prsTemplateItemT, "AllResources");
	//
	// (c.haag 2006-11-06 11:31) - PLID 23336 - Load resource ID's. They are read in as a
	// space-delimited array of numbers within a string
	//
	CDWordArray arydwResourceIDs;
	LoadResourceIDStringIntoArray(AdoFldString(prsTemplateItemT, "ResourceIDString", ""), arydwResourceIDs);
	// (z.manning 2011-12-07 12:32) - PLID 46910 - We must also load resource names here too which we do
	// using a map of resource IDs to resource names passed into this function.
	foreach(DWORD dwResourceID, arydwResourceIDs)
	{
		CString strResourceName;
		if(pmapResources != NULL) {
			pmapResources->Lookup(dwResourceID, strResourceName);
		}
		else {
			// (z.manning 2011-12-07 12:31) - PLID 46910 - The caller did not pass in a resource map so either
			// it will fill the resource names or it does not need them.
		}

		AddResource(dwResourceID, strResourceName);
	}

	// (z.manning, 11/17/2006) - Load all the exceptions for this template item.
	// (z.manning 2011-12-07 10:53) - PLID 46910 - Exceptions now must be part of the passed in recordset as opposed
	// to running a separate query here as it used to be. There is also a flag to skip them.
	if(bLoadExceptions)
	{
		m_arydtExceptionDates.RemoveAll();
		do
		{
			_variant_t varExceptionDate = prsTemplateItemT->GetFields()->GetItem("ExceptionDate")->GetValue();
			if(varExceptionDate.vt != VT_NULL) {
				m_arydtExceptionDates.Add(VarDateTime(varExceptionDate));
			}

			prsTemplateItemT->MoveNext();
		}
		while(!prsTemplateItemT->eof && AdoFldLong(prsTemplateItemT, "ID") == m_nID);
		prsTemplateItemT->MovePrevious();
	}
}

CString CTemplateLineItemInfo::GenerateSaveString()
{
	CString strSql = BeginSqlBatch();
	AddSaveStringToBatch(strSql);
	return strSql;
}

void CTemplateLineItemInfo::AddSaveStringToBatch(CString &strSqlBatch)
{
	if(m_nTemplateID == -1) {
		AfxThrowNxException("Attempted to save a template line item without a template.");
	}
	
	// Decide what the actual scale will be
	// (z.manning 2010-04-27 09:30) - PLID 23726 - This does not provide any benefit and can just confuse users.
	/*if ((m_esScale == sDaily) && (m_nPeriod == 1)) {
		m_esScale = sWeekly;
	}*/

	//TES 6/19/2010 - PLID 39262 - Use the appropriate tables throughout
	CString strTablePrefix = m_bUseResourceAvailTemplates?"ResourceAvail":"";
		
	CString strLineItemID;
	if(IsNew()) {
		// (z.manning, 11/16/2006) - Only declare the line item variable if it hasn't been declared already.
		CString strDeclare = "DECLARE @nLineItemID INT";
		if(strSqlBatch.Find(strDeclare) == -1) {
			AddDeclarationToSqlBatch(strSqlBatch, strDeclare);
		}
		AddStatementToSqlBatch(strSqlBatch, "SET @nLineItemID = (SELECT COALESCE(MAX(ID),0) + 1 FROM %sTemplateItemT WITH(UPDLOCK, HOLDLOCK))", strTablePrefix);
		strLineItemID = "@nLineItemID";

		// (z.manning, 11/13/2006) - We need to create a new line item.
		//TES 6/19/2010 - PLID 39262 - Resource Availability templates don't have the IsBlock field
		AddStatementToSqlBatch(strSqlBatch,
			"INSERT INTO %sTemplateItemT (ID, TemplateID, Scale, Period, MonthBy, PatternOrdinal, DayNumber, "
			"	StartTime, EndTime, StartDate, EndDate, ApparentInclude, AllResources%s) "
			"SELECT %s, %li, %li, %li, %li, %li, %li, '%s', '%s', '%s', %s, %li, %d%s ",
			strTablePrefix,
			m_bUseResourceAvailTemplates?"":", IsBlock",
			strLineItemID,
			m_nTemplateID, 
			m_esScale, 
			m_nPeriod, 
			m_embBy, 
			m_nPatternOrdinal, 
			m_nDayNumber, 
			FormatDateTimeForSql(m_dtStartTime, dtoTime),
			FormatDateTimeForSql(m_dtEndTime, dtoTime),
			FormatDateTimeForSql(m_dtStartDate, dtoDate),
			(COleDateTime::valid != m_dtEndDate.GetStatus()) ? "NULL" : CString("'") + FormatDateTimeForSql(m_dtEndDate, dtoDate) + "'",
			m_nInclude,
			m_bAllResources ? 1 : 0,
			m_bUseResourceAvailTemplates?"":(m_bIsBlock ? ", 1" : ", 0"));
	}
	else {
		// (z.manning, 11/13/2006) - Update the existing line item.
		//TES 6/19/2010 - PLID 39262 - Resource Availability templates don't have the IsBlock field
		strLineItemID = AsString(m_nID);
		AddStatementToSqlBatch(strSqlBatch,
			"UPDATE %sTemplateItemT "
			"SET TemplateID = %li, "
			"	 Scale = %li, "
			"	 Period = %li, "
			"	 MonthBy = %li, "
			"	 PatternOrdinal = %li, "
			"	 DayNumber = %li, "
			"	 StartTime = '%s', "
			"	 EndTime = '%s', "
			"	 StartDate = '%s', "
			"	 EndDate = %s, "
			"	 ApparentInclude = %li, "
			"	 AllResources = %d%s "
			"WHERE ID = %s ",
			strTablePrefix,
			m_nTemplateID, 
			m_esScale, 
			m_nPeriod, 
			m_embBy, 
			m_nPatternOrdinal, 
			m_nDayNumber, 
			FormatDateTimeForSql(m_dtStartTime, dtoTime),
			FormatDateTimeForSql(m_dtEndTime, dtoTime),
			FormatDateTimeForSql(m_dtStartDate, dtoDate),
			(COleDateTime::valid != m_dtEndDate.GetStatus()) ? "NULL" : CString("'") + FormatDateTimeForSql(m_dtEndDate, dtoDate) + "'",
			m_nInclude, 
			m_bAllResources ? 1 : 0,
			m_bUseResourceAvailTemplates?"":(m_bIsBlock ? ", IsBlock = 1" : ", IsBlock = 0"),
			strLineItemID);

		// (z.manning 2015-01-06 10:34) - PLID 64521 - If there's no collection ID then ensure the collection fields are null
		if (!m_bUseResourceAvailTemplates && m_nCollectionID == -1) {
			AddStatementToSqlBatch(strSqlBatch,
				"UPDATE TemplateItemT SET TemplateCollectionApplyID = NULL, TemplateCollectionTemplateID = NULL WHERE ID = %s \r\n"
				, strLineItemID);
		}

		AddStatementToSqlBatch(strSqlBatch, "DELETE FROM %sTemplateItemExceptionT WHERE TemplateItemID = %s", strTablePrefix, strLineItemID);

		AddStatementToSqlBatch(strSqlBatch, "DELETE FROM %sTemplateItemResourceT WHERE TemplateItemID = %s ", strTablePrefix, strLineItemID);

		AddStatementToSqlBatch(strSqlBatch, "DELETE FROM %sTemplateDetailsT WHERE TemplateItemID = %s ", strTablePrefix, strLineItemID);
	}

	for (int i=0; i < m_aryResources.GetSize(); i++) {
		AddStatementToSqlBatch(strSqlBatch, 
			"INSERT INTO %sTemplateItemResourceT (TemplateItemID, ResourceID) "
			"VALUES (%s, %li) ", 
			strTablePrefix, strLineItemID, m_aryResources.GetAt(i).m_nID);
	}

	// (a.walling 2015-02-19 10:54) - PLID 64540 - Scheduler template line items does not always need to create DayOfWeek entries in TemplateDetailsT
	// for line items with a scale of once (scale=1), monthly by date (scale=4, monthby=2), or yearly (scale=5).
	if (m_esScale == sDaily || m_esScale == sWeekly || (m_esScale == sMonthly && m_embBy != mbDate) ) {
		for (int nDayIndex = 0; nDayIndex < 7; nDayIndex++) {
			if (m_nInclude & DAYVAL(nDayIndex)) {
				AddStatementToSqlBatch(strSqlBatch,
					"INSERT INTO %sTemplateDetailsT (TemplateItemID, DayOfWeek) "
					"VALUES (%s, %i) ",
					strTablePrefix, strLineItemID, nDayIndex);
			}
		}
	}

	for(i = 0; i < m_arydtExceptionDates.GetSize(); i++) {
		AddStatementToSqlBatch(strSqlBatch, GenerateExceptionString(m_arydtExceptionDates.GetAt(i),strLineItemID));
	}
}

BOOL CTemplateLineItemInfo::NeverEnds()
{
	if(m_dtEndDate.GetStatus() == COleDateTime::invalid) {
		return TRUE;
	}
	
	return FALSE;
}

void CTemplateLineItemInfo::SetNew()
{
	m_nID = -1;
	
	// (z.manning 2015-01-05 17:18) - PLID 64521 - Clear the collection ID too
	m_nCollectionID = -1;
}

BOOL CTemplateLineItemInfo::IsNew()
{
	return (m_nID == -1);
}

CString CTemplateLineItemInfo::GenerateExceptionString(COleDateTime dtExceptionDate, CString strLineItemIDOverride /* = "" */)
{
	if(m_nID == -1 && strLineItemIDOverride.IsEmpty()) {
		AfxThrowNxException("Tried to add an exception to a line item with an ID of -1.");
	}

	CString strLineItemID;
	if(strLineItemIDOverride.IsEmpty()) {
		strLineItemID = FormatString("%li", m_nID);
	}
	else {
		strLineItemID = strLineItemIDOverride;
	}

	//TES 6/19/2010 - PLID 39262 - Use the correct table.
	return FormatString(
		"INSERT INTO %sTemplateItemExceptionT (ID, TemplateItemID, Date)  \r\n"
		"SELECT (SELECT COALESCE(MAX(ID),0) + 1 FROM %sTemplateItemExceptionT WITH(UPDLOCK, HOLDLOCK)), %s, '%s'  \r\n",
		m_bUseResourceAvailTemplates?"ResourceAvail":"", m_bUseResourceAvailTemplates?"ResourceAvail":"", strLineItemID, FormatDateTimeForSql(dtExceptionDate, dtoDate));
}

CString CTemplateLineItemInfo::GenerateDeleteString()
{
	if(IsNew()) {
		AfxThrowNxException("Tried to delete an unsaved template line item.");
	}

	//TES 6/19/2010 - PLID 39262 - Use the correct tables.
	CString strTablePrefix = m_bUseResourceAvailTemplates?"ResourceAvail":"";
	CString strSql;
	strSql += FormatString("DELETE FROM %sTemplateItemExceptionT WHERE TemplateItemID = %li;", strTablePrefix,m_nID);
	strSql += FormatString("DELETE FROM %sTemplateDetailsT WHERE TemplateItemID = %li;", strTablePrefix,m_nID);
	strSql += FormatString("DELETE FROM %sTemplateItemResourceT WHERE TemplateItemID = %li;", strTablePrefix,m_nID);
	strSql += FormatString("DELETE FROM %sTemplateItemT WHERE ID = %li;", strTablePrefix,m_nID);

	return strSql;
}

// (z.manning 2011-12-07 11:29) - PLID 46910
int CTemplateLineItemInfo::GetResourceCount()
{
	return m_aryResources.GetCount();
}

// (z.manning 2011-12-07 11:29) - PLID 46910
Resource CTemplateLineItemInfo::GetResourceByIndex(const int nIndex)
{
	return m_aryResources.GetAt(nIndex);
}

// (z.manning 2011-12-07 11:29) - PLID 46910
void CTemplateLineItemInfo::AddResource(const long nResourceID, LPCTSTR strResourceName)
{
	Resource resource;
	resource.m_nID = nResourceID;
	resource.m_strName = strResourceName;
	m_aryResources.Add(resource);
}

// (z.manning 2011-12-07 11:29) - PLID 46910
void CTemplateLineItemInfo::RemoveAllResources()
{
	m_aryResources.RemoveAll();
}
// (s.tullis 2015-08-24 14:37) - PLID 66411 
long CTemplateLineItemInfo::GetResourceIndexByID(const int nResourceID)
{
	try{
		const int nNumResources = GetResourceCount();
		for (int nResourceIndex = 0; nResourceIndex < nNumResources; nResourceIndex++)
		{
			if (GetResourceByIndex(nResourceIndex).m_nID == nResourceID)
			{
				return nResourceIndex;
			}
		}
	}NxCatchAll(__FUNCTION__)
	return -1;
}

// (z.manning 2014-12-12 12:00) - PLID 64228
CString GetApparentDescriptionForCollectionApply(NexTech_Accessor::_SchedulerTemplateCollectionApplyPtr pApply, const bool bIgnoreResources)
{
	CArray<Resource, const Resource&> aryResources;
	if (!bIgnoreResources && pApply->Resources != NULL)
	{
		Nx::SafeArray<IUnknown*> saResources(pApply->Resources);
		for each(_SchedulerResourcePtr pResource in saResources) {
			Resource res;
			res.m_nID = AsLong(pResource->ID);
			res.m_strName = (LPCTSTR)pResource->Name;
			aryResources.Add(res);
		}
	}

	CArray<COleDateTime, const COleDateTime&> arydtExceptions;
	if (pApply->Exceptions != NULL)
	{
		Nx::SafeArray<IUnknown*> saExceptions(pApply->Exceptions);
		for each (_SchedulerTemplateItemExceptionPtr pException in saExceptions) {
			arydtExceptions.Add(pException->ExceptionDate);
		}
	}

	COleDateTime dtEndDate = g_cdtInvalid;
	if (!pApply->EndDate->IsNull()) {
		dtEndDate = pApply->EndDate->GetValue();
	}
	return GetApparentDescription(stetCollection, ConvertAPISchedulerTemplateItemScale(pApply->Scale), pApply->Period
		, pApply->ApparentInclude, ConvertAPISchedulerTemplateItemMonthBy(pApply->MonthBy)
		, pApply->PatternOrdinal->GetValueOrDefault(1), pApply->DayNumber, g_cdtInvalid, g_cdtInvalid
		, pApply->StartDate, dtEndDate, pApply->AllResources
		, aryResources, arydtExceptions);
}

// (z.manning 2014-12-16 11:05) - PLID 64232
CTemplateLineItemInfo* GetLineItemFromCollectionApply(NexTech_Accessor::_SchedulerTemplateCollectionApplyPtr pApply)
{
	CTemplateLineItemInfo *pLineItem = new CTemplateLineItemInfo;
	pLineItem->m_nID = AsLong(pApply->ID);
	pLineItem->m_bUseResourceAvailTemplates = false;
	pLineItem->m_dtStartDate = pApply->StartDate;
	if (pApply->EndDate->IsNull()) {
		pLineItem->m_dtEndDate = g_cdtInvalid;
	}
	else {
		pLineItem->m_dtEndDate = pApply->EndDate->GetValue();
	}
	pLineItem->m_esScale = ConvertAPISchedulerTemplateItemScale(pApply->Scale);
	pLineItem->m_nInclude = pApply->ApparentInclude;
	pLineItem->m_nDayNumber = pApply->DayNumber;
	pLineItem->m_nPeriod = pApply->Period;
	pLineItem->m_nPatternOrdinal = pApply->PatternOrdinal->GetValueOrDefault(1);
	pLineItem->m_embBy = ConvertAPISchedulerTemplateItemMonthBy(pApply->MonthBy);
	pLineItem->m_bAllResources = pApply->AllResources ? TRUE : FALSE;
	
	pLineItem->m_dtStartTime = g_cdtInvalid;
	pLineItem->m_dtEndTime = g_cdtInvalid;

	if (pApply->Resources != NULL)
	{
		Nx::SafeArray<IUnknown*> saResources(pApply->Resources);
		for each (_SchedulerResourcePtr pResource in saResources)
		{
			pLineItem->AddResource(AsLong(pResource->ID), pResource->Name);
		}
	}

	if (pApply->Exceptions != NULL)
	{
		Nx::SafeArray<IUnknown*> saExceptions(pApply->Exceptions);
		for each (_SchedulerTemplateItemExceptionPtr pException in saExceptions)
		{
			pLineItem->m_arydtExceptionDates.Add(pException->ExceptionDate);
		}
	}

	return pLineItem;
}

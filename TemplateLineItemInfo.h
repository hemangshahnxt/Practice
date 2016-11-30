#ifndef TEMPLATE_LINE_ITEM_SET_H
#define TEMPLATE_LINE_ITEM_SET_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000 

// (z.manning 2014-12-11 16:26) - PLID 64230 - Moved scale and month by enums to library
#include "SharedScheduleUtils.h"
#include "NxAPI.h"

#define DAYVAL(nDayIndex) (int)((int)1 << (nDayIndex))
#define DAYTEXT(a,bForceEnglish) GetDayText(a,bForceEnglish);

const int dowSunday = DAYVAL(0);
const int dowMonday = DAYVAL(1);
const int dowTuesday = DAYVAL(2);
const int dowWednesday = DAYVAL(3);
const int dowThursday = DAYVAL(4);
const int dowFriday = DAYVAL(5);
const int dowSaturday = DAYVAL(6);
const int dowEveryday = dowSunday | dowMonday | dowTuesday | dowWednesday | dowThursday | dowFriday | dowSaturday;
const int dowWeekdays = dowMonday | dowTuesday | dowWednesday | dowThursday | dowFriday;
const int dowWeekends = dowSaturday | dowSunday;

CString GetDayText(int nDayNum, BOOL bForceEnglish = FALSE);

struct Resource
{
	long m_nID;
	CString m_strName;

	BOOL operator ==(Resource &resourceCompare)
	{
		if(m_nID != resourceCompare.m_nID ||
			m_strName != resourceCompare.m_strName)
		{
			return FALSE;
		}
		return TRUE;
	}
};

class CTemplateLineItemInfo
{
public:

	long m_nID;        // The template item's ID
	long m_nTemplateID;// The template item that the item belongs to.
	EScale m_esScale;  // Once, daily, weekly, etc.
	int m_nPeriod;		// Every other blah, Every 3 blahs, etc
	int m_nInclude;		// Monday, Tuesday, and Wednesday
	EMonthBy m_embBy; // month by Pattern or by Date
	int m_nPatternOrdinal; // the Xth Monday of the month
	int m_nDayNumber;	// Day 5 of the month

	// (c.haag 2006-11-03 10:44) - PLID 23336 - This field is now superceded by
	// TemplateItemT.StartDate; or in this class, m_dtStartDate
	//COleDateTime m_dtPivotDate; // Any apply date for this line item

	// (b.cardillo 2005-10-14 16:50) - PLID 17954 - Use the COleDateTime directly rather than 
	// converting back and forth to CString, so as to avoid any problems from regional settings.
	COleDateTime m_dtStartTime;	// The start time of the template for each apply date
	COleDateTime m_dtEndTime;	// The end time of the template for any given apply date

	// (c.haag 2006-11-02 16:24) - PLID 23336 - Start dates now apply to template line items
	COleDateTime m_dtStartDate;

	// (c.haag 2006-11-02 16:23) - PLID 23336 - The end date used to only be used for the
	// 'once' scale, but now it has actual value as the ending apply date for a template line item
	COleDateTime m_dtEndDate;

	// (c.haag 2006-11-03 12:27) - PLID 23336 - Scheduler resources are now assigned per template item
	BOOL m_bAllResources;

	// (z.manning 2014-12-18 15:22) - PLID 64295 - Added collection ID
	long m_nCollectionID;

protected:
	// (z.manning 2011-12-07 11:26) - PLID 46910 - This variable is no longer public and is now an array
	// of resource objects rather than an array resource IDs
	CArray<Resource> m_aryResources;

public:
	int GetResourceCount();
	Resource GetResourceByIndex(const int nIndex);
	void AddResource(const long nResourceID, LPCTSTR strResourceName);
	void RemoveAllResources();
	// (s.tullis 2015-08-24 14:37) - PLID 66411 
	long GetResourceIndexByID(const int nResourceID);

public:
	// (z.manning, 11/17/2006) - PLID 23574 - Stores all the exception dates for the line item.
	CArray<COleDateTime> m_arydtExceptionDates;

	// (c.haag 2006-11-20 11:19) - PLID 23605 - This is non-zero if the line item is to be shown
	// on the scheduler as an array of block times that the user can easily schedule in with one click
	BOOL m_bIsBlock;

	//TES 6/19/2010 - PLID 39262
	bool m_bUseResourceAvailTemplates;

public:
	CTemplateLineItemInfo();
	CTemplateLineItemInfo(CTemplateLineItemInfo* pSourceLineItem);
	~CTemplateLineItemInfo();
	void operator =(CTemplateLineItemInfo &lineitemSource);
	bool operator ==(CTemplateLineItemInfo &lineitemCompare); // (z.manning 2011-12-07 16:09) - PLID 46906
	bool operator !=(CTemplateLineItemInfo &lineitemCompare); // (z.manning 2011-12-07 16:09) - PLID 46906

	// (c.haag 2006-11-08 12:29) - PLID 9036 - All prototypes below are used for calculating the
	// apparent description of a line item
	//
protected:
	void CopyFrom(CTemplateLineItemInfo* pSourceLineItem);

public:
	// (z.manning 2015-01-30 15:51) - PLID 64230 - Added type param
	CString GetApparentDescription(ESchedulerTemplateEditorType eTemplateType);

	BOOL NeverEnds();

	BOOL IsNew();
	void SetNew();

	// (z.manning 2010-08-31 13:29) - PLID 39596 - Changed return type to bool and added silent parameter.
	BOOL LoadFromID(long nLineItemID, BOOL bSilent = FALSE);
	void LoadFromRecordset(ADODB::_RecordsetPtr prsTemplateItemT, BOOL bLoadExceptions, CMap<long,long,CString,LPCTSTR> *pmapResources);

	CString GenerateSaveString();
	CString GenerateDeleteString();
	void AddSaveStringToBatch(CString &strSqlBatch);
	CString GenerateExceptionString(COleDateTime dtExceptionDate, CString strLineItemIDOverride = "");

	//
	// (c.haag 2006-11-08 12:29) - PLID 9036 - End prototype list
};

// (z.manning 2014-12-12 11:50) - PLID 64228 - These are no longer class members
// (z.manning 2015-01-30 15:43) - PLID 64230 - Added type param
CString GetApparentDescription(ESchedulerTemplateEditorType eTemplateType, EScale esScale, int nPeriod, int nInclude, EMonthBy embBy, int nPatternOrdinal,
	int nDayNumber, const COleDateTime& dtStartTime, const COleDateTime& dtEndTime, const COleDateTime& dtStartDate,
	const COleDateTime& dtEndDate, BOOL bAllResources, const CArray<Resource>& aryResources,
	const CArray<COleDateTime>& arydtExceptionDates);

// (z.manning 2014-12-12 12:00) - PLID 64228
CString GetApparentDescriptionForCollectionApply(NexTech_Accessor::_SchedulerTemplateCollectionApplyPtr pApply, const bool bIgnoreResources);

CString GetScaleWord(EScale esScale, bool bNormalCase = true, bool bSingular = true);
CString GetDayNumberText(int nDayNumber, bool bNormalCase);
void GetDays(EScale eScale, EMonthBy embBy, int nData,
	int &nDays, int &nDayCnt, int nCheckVal = 1);
void GetDaysEx(EScale eScale, EMonthBy embBy, int nData,
	int &nDays, int &nDayCnt, UINT& nIncluded);
CString GetDaysText(EScale esScale, EMonthBy embBy, int nData, UINT nCheckVal);
CString GetDaysTextEx(EScale esScale, EMonthBy embBy, int nData, UINT& nIncluded);
CString GetPatternText(int nPatternOrdinal);
CString GetPreTimeRangeText(EScale esScale, const COleDateTime& dtStartTime, const COleDateTime& dtEndTime,
	bool &bNormalCase);
CString GetPreIncludeText(EScale esScale, EMonthBy embBy, int nInclude, const COleDateTime& dtStartDate,
	int nDayNumber, int nPatternOrdinal, bool &bNormalCase);
CString GetPeriodText(EScale esScale, int nPeriod, bool &bNormalCase);
CString GetPostIncludeText(EScale esScale, EMonthBy embBy, int nData, bool &bNormalCase);
CString GetPostTimeRangeText(EScale esScale, const COleDateTime& dtStartTime, const COleDateTime& dtEndTime,
	bool &bNormalCase);
CString GetDateRangeText(EScale esScale, const COleDateTime& dtStartDate,
	const COleDateTime& dtEndDate, bool &bNormalCase);
CString GetDateExceptionText(const CArray<COleDateTime>& arydtExceptionDates, bool &bNormalCase);
CString GetResourceText(BOOL bAllResources, const CArray<Resource>& aryResources, bool &bNormalCase);


// (z.manning 2014-12-16 11:05) - PLID 64232
CTemplateLineItemInfo* GetLineItemFromCollectionApply(NexTech_Accessor::_SchedulerTemplateCollectionApplyPtr pApply);


#endif
// TemplateHitSet.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "GlobalUtils.h"
#include "TemplateHitSet.h"
#include "TemplateEntryDlg.h"
#include "NxStandard.h"

#include "DateTimeUtils.h"

#include "GlobalDataUtils.h"
using namespace ADODB;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTemplateHitSet

CTemplateHitSet::CTemplateHitSet()
{
	m_TemplateLineItemID = 0;
	m_StartTime = (DATE)0;
	m_EndTime = (DATE)0;
	m_Color = 0;
	m_Priority = 0;
	m_bIsBlock = FALSE;
	m_nRuleID = -1;
	m_bRuleAndDetails = FALSE;
	m_bRuleAllAppts = FALSE;
	m_bRuleOverrideLocationTemplating = FALSE;
	m_nRuleObjectType = -1;
	m_nRuleObjectID = -1;
	m_nResourceID = -1;
	m_bAllResources = FALSE;
	m_nTemplateID = -1;
	m_nCollectionID = -1;
}

BOOL CTemplateHitSet::IsEOF()
{
	if (m_prs->eof) {
		return TRUE;
	} else {
		return FALSE;
	}
}

BOOL CTemplateHitSet::IsBOF()
{
	if (m_prs->bof) {
		return TRUE;
	} else {
		return FALSE;
	}
}

// (z.manning, 05/24/2007) - PLID 26062 - Move back to the first record.
void CTemplateHitSet::MoveFirst()
{
	HR(m_prs->MoveFirst());
	LoadRecord();
}

void CTemplateHitSet::MoveNext()
{
	// Move to the next record
	HR(m_prs->MoveNext());

	// Fill in our local variables with the data for the current record
	LoadRecord();
}

//TES 6/22/2010 - PLID 39278 - Added a LocationID parameter, determines whether to include Resource Availability templates
// (a.walling 2010-06-25 12:36) - PLID 39278 - separate param for including Resource Availability template
void CTemplateHitSet::Requery(COleDateTime dtApplyDate, long nItemID, BOOL bIncludeTemplateRules, long nLocationID, bool bIncludeTemplates, bool bIncludeResourceAvailabilityTemplates)
{
	CArray<long,long> arynTemplateID;
	arynTemplateID.Add(nItemID);
	Requery(dtApplyDate, arynTemplateID, bIncludeTemplateRules, nLocationID, bIncludeTemplates, bIncludeResourceAvailabilityTemplates);
}

//TES 6/22/2010 - PLID 39278 - Added a LocationID parameter, determines whether to include Resource Availability templates
// (a.walling 2010-06-25 12:36) - PLID 39278 - separate param for including Resource Availability template
void CTemplateHitSet::Requery(COleDateTime dtApplyDate, CArray<long,long> &arynResourceIDs, BOOL bIncludeTemplateRules, long nLocationID, bool bIncludeTemplates, bool bIncludeResourceAvailabilityTemplates)
{
	// (z.manning, 05/24/2007) - PLID 26062 - We better have at least once resource ID.
	if(arynResourceIDs.GetSize() <= 0) {
		ASSERT(FALSE);
		return;
	}

	ASSERT(bIncludeTemplates || bIncludeResourceAvailabilityTemplates);

	// Use only the date portion of the dtApplyDate (I don't know if this matters at all anymore)
	COleDateTime dt;
	dt.SetDate(dtApplyDate.GetYear(), dtApplyDate.GetMonth(), dtApplyDate.GetDay());

	// (a.walling 2010-06-24 10:11) - PLID 39263 - Since we need to know the availability template info regardless, then we need to
	// always query it, unless we specifically say that we only want the ResourceAvailTemplateHitAllP results.
	
	// (a.walling 2010-06-25 09:08) - PLID 39278 - Same code path now that the SP is unified
	//if(m_bUseResourceAvailTemplates /* || nLocationID == -1*/) {
		// (c.haag 2006-12-13 13:35) - PLID 23845 - If we are using block/template-only scheduling,
		// then we have to run our own version of TemplateHitAllP that pulls in rules. I would like
		// to explore a better way to do this in future releases. The reason we do not change 
		// TemplateHitAllP to support this is so that everyone not using block scheduling will not
		// suffer any speed hit.
		// (z.manning, 05/24/2007) - PLID 26062 - The parameter for whether or not we include rules
		// is now passed on to TemplateHitAllP so that we don't have redundant SQL code here.
		// Open the stored proc and give it the necessary parameters
		//TES 6/19/2010 - PLID 5888 - Use the correct procedure, depending on whether we're pulling Resource Availability templates
		// (a.walling 2010-06-25 09:08) - PLID 39278 - Unified ResourceAvailTemplateHitAllP into TemplateHitAllP
		//_CommandPtr pcmd = OpenStoredProc(m_bUseResourceAvailTemplates?"ResourceAvailTemplateHitAllP":"TemplateHitAllP");
		// (a.walling 2014-04-21 14:47) - PLID 60474 - TemplateHitAllP - open in snapshot isolation
		_CommandPtr pcmd = OpenStoredProc(GetRemoteDataSnapshot(), "TemplateHitAllP");
		AddParameterDateTime(pcmd, "CheckDate", dt);
		// (z.manning, 05/25/2007) - PLID 26062 - Resource ID paramater is now a string to support multiple IDs.
		// Also added IncludeRuleInfo parameter.
		AddParameterString(pcmd, "ResourceIDs", ArrayAsString(arynResourceIDs,false));
		AddParameterBool(pcmd, "IncludeRuleInfo", bIncludeTemplateRules ? true : false);

		// (a.walling 2010-06-25 09:08) - PLID 39278 - Explicitly setting optional params
		AddParameterBool(pcmd, "IncludeNormalTemplates", bIncludeTemplates);
		// (a.walling 2010-06-25 12:36) - PLID 39278 - including Resource Availability templates if requested
		AddParameterBool(pcmd, "IncludeResourceAvailTemplates", bIncludeResourceAvailabilityTemplates);

		// Open the recordset based on the command proc
		m_prs = CreateRecordset(pcmd);
	//}	
		
	// (a.walling 2010-06-25 09:08) - PLID 39278 - Same code path now that the SP is unified
	//else {
	//	//TES 6/22/2010 - PLID 39278 - We're getting scheduler templates, and filtering on a location.  Therefore, we need to return a recordset
	//	// that contains both resource availability and regular templates.
	//	//NOTE: This isn't very parameterized, when I tried to make it so it kept failing and I don't really have time to figure out why.
	//	//  After copying this code to patch I'll revisit.
	//	CString strResources = ArrayAsString(arynResourceIDs,false);
	//	m_prs = CreateParamRecordset(FormatString("/*Declare the table that will hold our results*/\r\n"
	//		"DECLARE @TemplateHits TABLE (TemplateID INT NOT NULL, Name nvarchar(50) NOT NULL, \r\n"
	//		"TemplateLineItemID INT NOT NULL, StartTime DATETIME NOT NULL, EndTime DATETIME NOT NULL, Color INT NOT NULL, \r\n"
	//		"IsBlock BIT NOT NULL, Priority INT, RuleID INT, AndDetails BIT, AllAppts BIT, ObjectType INT, ObjectID INT, ResourceID INT, \r\n"
	//		"AllResources BIT NOT NULL, LocationID INT)\r\n"
	//		"\r\n"
	//		"/*Fill it with the scheduler templates*/\r\n"
	//		"INSERT @TemplateHits (TemplateID, Name, TemplateLineItemID, StartTime, EndTime, Color, IsBlock, Priority, RuleID, AndDetails, \r\n"
	//		"AllAppts, ObjectType, ObjectID, ResourceID, AllResources) \r\n"
	//		"EXEC TemplateHitAllP '%s', '%s', %li \r\n"
	//		"\r\n"
	//		"/*Now add the resource availability templates*/\r\n"
	//		"INSERT @TemplateHits (TemplateID, Name, TemplateLineItemID, StartTime, EndTime, Color, Priority, ResourceID, AllResources, \r\n"
	//		"IsBlock, LocationID) \r\n"
	//		"EXEC ResourceAvailTemplateHitAllP '%s', '%s', %li \r\n"
	//		"/*Now update the resource availability templates (they're the ones with LocationIDs, and change their priorities and colors, \r\n"
	//		"so that they'll always be on top and dark gray*/\r\n"
	//		"UPDATE @TemplateHits SET Priority = 10000+Priority, Color = 986895 WHERE LocationID Is Not Null\r\n"
	//		"/*Now return the results*/\r\n"
	//		"SELECT * FROM @TemplateHits", FormatDateTimeForSql(dt), strResources, bIncludeTemplateRules?1:0,FormatDateTimeForSql(dt), strResources, bIncludeTemplateRules?1:0));
	//	//TES 6/22/2010 - PLID 39278 - Skip past the intermediate recordsets (this, too, shouldn't be necessary and will be revisited
	//	// after copying to patch).
	//	m_prs = m_prs->NextRecordset(NULL);
	//	m_prs = m_prs->NextRecordset(NULL);
	//	m_prs = m_prs->NextRecordset(NULL);
	//}
	
	

	// Attach our field objects to the open recordset
	FieldsPtr flds = m_prs->Fields;
	m_fldLineItemID = flds->Item["TemplateLineItemID"];
	m_fldStartTime = flds->Item["StartTime"];
	m_fldEndTime = flds->Item["EndTime"];
	m_fldColor = flds->Item["Color"];
	m_fldName = flds->Item["Name"];
	m_fldPriority = flds->Item["Priority"];
	m_fldIsBlock = flds->Item["IsBlock"];
	// (z.manning, 05/24/2007) - PLID 26062 - ResourceID and AllResources.
	m_fldResourceID = flds->Item["ResourceID"];
	m_fldAllResources = flds->Item["AllResources"];
	// (z.manning 2009-07-18 12:27) - PLID 34939 - Added TemplateID
	m_fldTemplateID = flds->GetItem("TemplateID");

	// (c.haag 2006-12-13 16:55) - PLID 23845 - Include template rule items if we are
	// planning on reading in block template items
	if (bIncludeTemplateRules) {
		//TES 6/19/2010 - PLID 5888 - Resource Availability templates don't have rules
		//ASSERT(!m_bUseResourceAvailTemplates);
		m_fldRuleID = flds->Item["RuleID"];
		m_fldRuleAndDetails = flds->Item["AndDetails"];
		m_fldRuleAllAppts = flds->Item["AllAppts"];
		m_fldRuleObjectType = flds->Item["ObjectType"];
		m_fldRuleObjectID = flds->Item["ObjectID"];
		m_fldRuleOverrideLocationTemplating = flds->Item["OverrideLocationTemplating"]; //TES 9/3/2010 - PLID 39630
	} else {
		m_fldRuleID = NULL;
		m_fldRuleAndDetails = NULL;
		m_fldRuleAllAppts = NULL;
		m_fldRuleObjectType = NULL;
		m_fldRuleObjectID = NULL;
	}

	//TES 6/22/2010 - PLID 39278 - If we have resource availability templates, then pull the location ID, otherwise we don't have one.
	/*
	if(!m_bUseResourceAvailTemplates && nLocationID == -1) {
		m_fldLocationID = NULL;
	}
	else {
		m_fldLocationID = flds->Item["LocationID"];
	}
	*/
	// (a.walling 2010-06-24 10:11) - PLID 39263 - We need to know the availability template info regardless
	m_fldLocationID = flds->Item["LocationID"];

	// (c.haag 2014-12-16) - PLID 64256
	m_fldCollectionID = flds->Item["CollectionID"];
	m_fldCollectionName = flds->Item["CollectionName"];

	// Fill in our local variables with the data for the current record
	LoadRecord();
}

// Throws exceptions
// Allocates and returns a CSchedTemplateInfo object if a template matches the given criteria
// If no templates match, returns NULL
// NOTE: If this function returns non-NULL, it is the caller's responsibility for deallocating the CSchedTemplateInfo object
// (z.manning, 11/20/2006) - This function is not called from anywhere, so we should stop maintaining it.
//  (It's basically a copy of the stored procedure, TemplateHitAllP.)
CSchedTemplateInfo *CalcSchedTemplateInfo(IN const COleDateTime &dtApplyDate, IN const COleDateTime &dtApplyTime, IN const long nResourceID)
{
	ASSERT(FALSE);
	return NULL;
/*
	// Use only the date portion of the dtApplyDate (I don't know if this matters at all anymore)
	CString strDate, strTime;
	strDate = FormatDateTimeForSql(dtApplyDate, dtoDate);
	strTime = FormatDateTimeForSql(dtApplyTime, dtoTime);

	// Open the recordset based on the parameters
	// (z.manning, 10/26/2006) - PLID 23236 - Updated query to now use StartDate, EndDate, and resource as a property on line items.
	// (z.manning, 11/09/2006) - PLID 23276 - Now that date range is a property of template line items, the PivotDate column is not needed.
	_RecordsetPtr prs = CreateRecordset(
		"SELECT DISTINCT TemplateT.Name, TemplateItemT.Color, TemplateT.Priority FROM "
		"	TemplateT "
		"	INNER JOIN TemplateItemT ON TemplateT.ID = TemplateItemT.TemplateID "
		"	LEFT JOIN TemplateItemResourceT ON TemplateItemT.ID = TemplateItemResourceT.TemplateItemID "
		"	LEFT JOIN TemplateDetailsT ON TemplateItemT.ID = TemplateDetailsT.TemplateItemID "
		"WHERE "
		"	(TemplateItemT.StartDate IS NULL OR TemplateItemT.StartDate <= '%s') AND "
		"	(TemplateItemT.EndDate IS NULL OR TemplateItemT.EndDate >= '%s') AND "
		"	(CONVERT(datetime,CONVERT(nvarchar, TemplateItemT.StartTime, 108)) <= '%s') AND "
		"	(CONVERT(datetime,CONVERT(nvarchar, TemplateItemT.EndTime, 108)) > '%s') AND "
		"	((TemplateItemT.AllResources = 1) OR (TemplateItemResourceT.ResourceID = %li)) AND (( "
		"		(TemplateItemT.Scale = 1) "
		"	) OR ( "
		"		(TemplateItemT.Scale = 2) AND "
		"		((DateDiff(dd, StartDate, '%s') %% [Period]) = 0) AND "
		"		((DatePart(dw, '%s') - 1) = TemplateDetailsT.DayOfWeek) "
		"	) OR ( "
		"		(TemplateItemT.Scale = 3) AND "
		"		((DateDiff(wk, StartDate, '%s') %% Period) = 0) AND "
		"		((DatePart(dw, '%s') - 1) = TemplateDetailsT.DayOfWeek) "
		"	) OR ( "
		"		(TemplateItemT.Scale = 4) AND "
		"		(TemplateItemT.MonthBy = 2) AND "
		"		((DateDiff(mm, StartDate, '%s') %% Period) = 0) AND "
		"		(DatePart(dd, '%s') = TemplateItemT.DayNumber) "
		"	) OR ( "
		"		(TemplateItemT.Scale = 4) AND  "
		"		(TemplateItemT.MonthBy = 1) AND "
		"		( ( ((DatePart(dd, '%s') - 1) / 7 + 1) = PatternOrdinal) OR (PatternOrdinal = -1 AND DATEPART(mm, DATEADD(ww, 1, '%s')) != DATEPART(mm, '%s')) ) AND  \r\n"
		"		((DateDiff(mm, StartDate, '%s') %% Period) = 0) AND "
		"		((DatePart(dw, '%s') - 1) = TemplateDetailsT.DayOfWeek) "
		"	) OR ( "
		"		(TemplateItemT.Scale = 5) AND "
		"		(DatePart(mm, StartDate) = DatePart(mm, '%s')) AND "
		"		(DatePart(dd, StartDate) = DatePart(dd, '%s')) "
		"	)) "
		"ORDER BY Priority ASC", strDate, strDate, strTime, strTime, 
		nResourceID, strDate, strDate, strDate, strDate, strDate, strDate, strDate, strDate,
		strDate, strDate, strDate, strDate, strDate, strDate, strDate, strDate,
		strDate, strDate, strDate, strDate);
	
	if (!prs->eof) {
		// If there are recordset move to the last one because due to the sorting, that's the one that will be on top of the given view
		prs->MoveLast();
		// Get the info off this last record and return it through the OUT parameters
		FieldsPtr pflds = prs->GetFields();
		CSchedTemplateInfo *pAns = new CSchedTemplateInfo;
		pAns->m_clrColor = AdoFldLong(pflds, "Color");
		pAns->m_strName = AdoFldString(pflds, "Name");
		// Return the info for the template that was found
		return pAns;
	} else {
		// There were no templates matching the given properties so return NULL
		return NULL;
	}
*/
}	

void CTemplateHitSet::LoadRecord()
{
	if ((m_prs != NULL) && (!m_prs->eof)) {
		m_TemplateLineItemID = AdoFldLong(m_fldLineItemID);
		m_StartTime = AdoFldDateTime(m_fldStartTime);
		m_StartTime.SetTime(m_StartTime.GetHour(), m_StartTime.GetMinute(), m_StartTime.GetSecond());
		m_EndTime = AdoFldDateTime(m_fldEndTime);
		m_EndTime.SetTime(m_EndTime.GetHour(), m_EndTime.GetMinute(), m_EndTime.GetSecond());
		m_Color = AdoFldLong(m_fldColor);
		m_Priority = AdoFldLong(m_fldPriority);
		m_strText = AdoFldString(m_fldName);
		m_bIsBlock = AdoFldBool(m_fldIsBlock, FALSE);
		// (z.manning, 05/24/2007) - PLID 26062 - Added ResourceID and AllResources fields.
		m_nResourceID = AdoFldLong(m_fldResourceID, -1);
		m_bAllResources = AdoFldBool(m_fldAllResources, FALSE);
		// (z.manning 2009-07-18 12:27) - PLID 34939 - Added TemplateID
		m_nTemplateID = AdoFldLong(m_fldTemplateID);

		// (c.haag 2006-12-13 16:55) - PLID 23845 - Read in template rules if the fields are available
		if (NULL != m_fldRuleID) { m_nRuleID = AdoFldLong(m_fldRuleID, -1); } else {
			m_nRuleID = -1;
		}
		if (NULL != m_fldRuleAndDetails) { m_bRuleAndDetails = AdoFldBool(m_fldRuleAndDetails, FALSE); } else {
			m_bRuleAndDetails = FALSE;
		}
		if (NULL != m_fldRuleAllAppts) { m_bRuleAllAppts = AdoFldBool(m_fldRuleAllAppts, FALSE); } else {
			m_bRuleAllAppts = FALSE;
		}
		//TES 8/31/2010 - PLID 39630
		if (NULL != m_fldRuleOverrideLocationTemplating) { m_bRuleOverrideLocationTemplating = AdoFldBool(m_fldRuleOverrideLocationTemplating, FALSE); } else {
			m_bRuleOverrideLocationTemplating = FALSE;
		}
		if (NULL != m_fldRuleObjectType) { m_nRuleObjectType = AdoFldLong(m_fldRuleObjectType, -1); } else {
			m_nRuleObjectType = -1;
		}
		if (NULL != m_fldRuleObjectID) { m_nRuleObjectID = AdoFldLong(m_fldRuleObjectID, -1); } else {
			m_nRuleObjectID = -1;
		}
		//TES 6/22/2010 - PLID 39278 - Added LocationID in certain situations
		if (NULL != m_fldLocationID) { m_nLocationID = AdoFldLong(m_fldLocationID, -1); } else {
			m_nLocationID = -1;
		}
		// (c.haag 2014-12-16) - PLID 64256
		m_nCollectionID = AdoFldLong(m_fldCollectionID, -1);
		m_strCollectionName = AdoFldString(m_fldCollectionName, "");
	}
}

long CTemplateHitSet::GetLineItemID()
{
	ASSERT(m_prs != NULL);
	return m_TemplateLineItemID;
}

const COleDateTime &CTemplateHitSet::GetStartTime()
{
	ASSERT(m_prs != NULL);
	return m_StartTime;
}

const COleDateTime &CTemplateHitSet::GetEndTime()
{
	ASSERT(m_prs != NULL);
	return m_EndTime;
}

long CTemplateHitSet::GetColor()
{
	ASSERT(m_prs != NULL);
	return m_Color;
}

const CString &CTemplateHitSet::GetText()
{
	ASSERT(m_prs != NULL);
	return m_strText;
}

long CTemplateHitSet::GetPriority()
{
	ASSERT(m_prs != NULL);
	return m_Priority;
}

BOOL CTemplateHitSet::GetIsBlock()
{
	ASSERT(m_prs != NULL);
	return m_bIsBlock;
}

long CTemplateHitSet::GetRuleID()
{
	ASSERT(m_prs != NULL);
	return m_nRuleID;
}

BOOL CTemplateHitSet::GetRuleAndDetails()
{
	ASSERT(m_prs != NULL);
	return m_bRuleAndDetails;
}

BOOL CTemplateHitSet::GetRuleAllAppts()
{
	ASSERT(m_prs != NULL);
	return m_bRuleAllAppts;
}

BOOL CTemplateHitSet::GetRuleOverrideLocationTemplating()
{
	//TES 8/31/2010 - PLID 39630
	ASSERT(m_prs != NULL);
	return m_bRuleOverrideLocationTemplating;
}

long CTemplateHitSet::GetRuleObjectType()
{
	ASSERT(m_prs != NULL);
	return m_nRuleObjectType;
}

long CTemplateHitSet::GetRuleObjectID()
{
	ASSERT(m_prs != NULL);
	return m_nRuleObjectID;
}

// (z.manning, 05/24/2007) - PLID 26062
long CTemplateHitSet::GetResourceID()
{
	ASSERT(m_prs != NULL);
	return m_nResourceID;
}

// (z.manning, 05/24/2007) - PLID 26062
BOOL CTemplateHitSet::GetIsAllResources()
{
	ASSERT(m_prs != NULL);
	return m_bAllResources;
}

// (z.manning 2009-07-18 12:29) - PLID 34939
long CTemplateHitSet::GetTemplateID()
{
	ASSERT(m_prs != NULL);
	return m_nTemplateID;
}

//TES 6/22/2010 - PLID 39278
long CTemplateHitSet::GetLocationID()
{
	ASSERT(m_prs != NULL);
	return m_nLocationID;
}

// (c.haag 2014-12-16) - PLID 64256
long CTemplateHitSet::GetCollectionID()
{
	ASSERT(m_prs != NULL);
	return m_nCollectionID;
}

// (c.haag 2014-12-16) - PLID 64256
const CString &CTemplateHitSet::GetCollectionName()
{
	ASSERT(m_prs != NULL);
	return m_strCollectionName;
}
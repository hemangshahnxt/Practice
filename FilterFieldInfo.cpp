// FilterFieldInfo.cpp: implementation of the CFilterFieldInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Practice.h"
#include "FilterFieldInfo.h"
#include "Filter.h"
#include "FilterDetail.h"

#include "WhereClause.h"

#include "GlobalDataUtils.h"
#include "DateTimeUtils.h"
#include "InternationalUtils.h"

using namespace ADODB;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

extern unsigned long FILTER_FIELD_NEXT_INFO_ID;

CMapStringToString CFilterFieldInfo::m_mapReplaceFieldNames;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// This overload is used for typical fields (they will be visible)
// (j.jones 2010-08-27 15:54) - PLID 39855 - added bCheckExists, defaults to true, such that if we set it to false it means we
// are explicitly filtering on records that do NOT match the successive filter
CFilterFieldInfo::CFilterFieldInfo( unsigned	long nInfoId, const CString &strFieldApparent, const CString &strFieldInternal, long nFilterType, 
												long nAvailableOperators /*= foAll*/, FieldOperatorEnum foDefault /*= foEqual*/, 
												FieldTypeEnum ftFieldType /*= ftText*/, LPCTSTR strParams /*= NULL*/, 
												LPCTSTR strJoinClause /*= NULL*/, unsigned long nJoinDependsOn /*= FILTER_DEPENDS_ON_BASE*/, 
												LPCTSTR strExistsSource /*= NULL*/, LPCTSTR strExistsBaseWhere /*= NULL*/,
												LPCTSTR strValueFormat /*= NULL*/, LPCTSTR strSubFilterDisplayName /*= NULL*/, long nSubfilterType /*= 0*/,
												LPCTSTR strMultiSelectParams /*= NULL*/, bool bCheckExists /*= true*/)
:	m_strFieldNameApparent(strFieldApparent), 
	m_strFieldNameInternal(strFieldInternal),
	m_strDefaultReplacement("")
{

	m_nInfoId = nInfoId;
	m_nAvailableOperators = nAvailableOperators;
	m_ftFieldType = ftFieldType;
	m_foDefault = foDefault;
	
	m_strMarker = GetMarker(m_ftFieldType);

	// Find the length by going until we find two consecutive null characters
	if (strParams) {
		long nLen = 0;
		while (*(strParams+nLen) || *(strParams+nLen+1)) nLen++;
		m_pstrParameters = (TCHAR *)new BYTE[(nLen+2)*sizeof(TCHAR)];
		memcpy((void *)m_pstrParameters, (const void *)strParams, (nLen+2)*sizeof(TCHAR));
	} else {
		m_pstrParameters = NULL;
	}

	// Find the length by going until we find two consecutive null characters
	if (strMultiSelectParams) {
		long nLen = 0;
		while (*(strMultiSelectParams+nLen) || *(strMultiSelectParams+nLen+1)) nLen++;
		m_pstrMultiSelectParams = (TCHAR *)new BYTE[(nLen+2)*sizeof(TCHAR)];
		memcpy((void *)m_pstrMultiSelectParams, (const void *)strMultiSelectParams, (nLen+2)*sizeof(TCHAR));
	} else {
		m_pstrMultiSelectParams = NULL;
	}

	// Fill FROM clause helper parameters
	m_strJoinClause = strJoinClause ? strJoinClause : "";
	m_nJoinDependsOn = nJoinDependsOn;
	
	// Fill the exists strings
	m_strExistsSource = strExistsSource ? strExistsSource : "";
	m_strExistsBaseWhere = strExistsBaseWhere ? strExistsBaseWhere : "";
	m_bCheckExists = bCheckExists;

	// Default to showing the field
	m_bVisible = true;
	m_nFilterType = nFilterType;
	m_bObsolete = false;
	m_bRemoved = false;
	m_strRemovalExplanation = "";
	m_strRemovalHelpLocation = "";
	m_strRemovalHelpBookmark = "";	

	m_bUseValueFormat = (strValueFormat != NULL) ? TRUE : FALSE;
	m_strValueFormat = strValueFormat;
	
	m_strSubfilterDisplayName = strSubFilterDisplayName;
	m_nSubfilterType = nSubfilterType;
}

// This override is used for join dependencies only (the field will be flagged as non-visible)
CFilterFieldInfo::CFilterFieldInfo(	unsigned long nInfoId, long nFilterID, LPCTSTR strFieldNameInternal, LPCTSTR strJoinClause, unsigned long nJoinDependsOn /*= FILTER_DEPENDS_ON_NONE*/)
:	m_strFieldNameApparent(""), 
	m_strFieldNameInternal(strFieldNameInternal),
	m_strDefaultReplacement("")
{

	m_nInfoId = nInfoId;

	// Fill all default (these don't matter at all)
	m_nAvailableOperators = foInvalidOperator;
	m_ftFieldType = ftNumber;
	m_foDefault = foInvalidOperator;
	m_strMarker = "";
	m_pstrParameters = NULL;
	m_pstrMultiSelectParams = NULL;

	// Fill FROM clause helper parameters
	m_strJoinClause = strJoinClause ? strJoinClause : "";
	m_nJoinDependsOn = nJoinDependsOn;
	
	// Default to NOT showing the field
	m_bVisible = false;
	//However, the field is still relevant.
	m_bObsolete = false;
	m_bRemoved = false;

	m_strRemovalExplanation = "";
	m_strRemovalHelpLocation = "";
	m_strRemovalHelpBookmark = "";

	m_nFilterType = -1;

	m_bUseValueFormat = false;
	m_strValueFormat = "";

	m_bCheckExists = true;
}

//This override is used only for obsolete filter fields.
CFilterFieldInfo::CFilterFieldInfo(unsigned long nInfoId, CString strDefaultReplacement)
:	m_strFieldNameApparent(""),
	m_strFieldNameInternal(""),
	m_strDefaultReplacement(strDefaultReplacement)
{
	m_nInfoId = nInfoId;

	// Fill all default (these don't matter at all)
	m_nAvailableOperators = foInvalidOperator;
	m_ftFieldType = ftNumber;
	m_foDefault = foInvalidOperator;
	m_strMarker = "";
	m_pstrParameters = NULL;
	m_pstrMultiSelectParams = NULL;

	// Fill FROM clause helper parameters
	m_strJoinClause = "";
	m_nJoinDependsOn = FILTER_DEPENDS_ON_NONE;
	
	// Default to NOT showing the field (since it's obsolete.
	m_bVisible = false;
	
	m_bObsolete = true;
	m_bRemoved = false;
	m_strRemovalExplanation = "";
	m_strRemovalHelpLocation = "";
	m_strRemovalHelpBookmark = "";

	m_nFilterType = -1;

	m_bUseValueFormat = false;
	m_strValueFormat = "";

	m_bCheckExists = true;
}

//This override is used only for filter fields that have been removed and can't be replaced.
CFilterFieldInfo::CFilterFieldInfo(CString strOldFieldName, unsigned long nInfoId, CString strRemovalExplanation, CString strRemovalHelpLocation, CString strRemovalHelpBookmark)
:	m_strFieldNameInternal(""),
	m_strDefaultReplacement("")
{
	m_nInfoId = nInfoId;
	m_strFieldNameApparent = strOldFieldName;

	// Fill all default (these don't matter at all)
	//TES 6/20/03: Make all operators available; sometimes it's going to load one just so they can see
	//what the field used to look like.
	m_nAvailableOperators = foAll;
	m_ftFieldType = ftText;
	m_foDefault = foInvalidOperator;
	m_strMarker = "";
	m_pstrParameters = NULL;
	m_pstrMultiSelectParams = NULL;

	// Fill FROM clause helper parameters
	m_strJoinClause = "";
	m_nJoinDependsOn = FILTER_DEPENDS_ON_NONE;
	
	// Default to NOT showing the field (since it's obsolete.
	m_bVisible = false;
	
	m_bObsolete = false;
	m_bRemoved = true;
	m_strRemovalExplanation = strRemovalExplanation;
	m_strRemovalHelpLocation = strRemovalHelpLocation;
	m_strRemovalHelpBookmark = strRemovalHelpBookmark;

	m_nFilterType = -1;

	m_bUseValueFormat = false;
	m_strValueFormat = "";

	m_bCheckExists = true;
}

CFilterFieldInfo::~CFilterFieldInfo()
{
	if (m_pstrParameters) {
		delete[] (BYTE*)m_pstrParameters;
		m_pstrParameters = NULL;
	}
	if (m_pstrMultiSelectParams) {
		delete[] (BYTE*)m_pstrMultiSelectParams;
		m_pstrMultiSelectParams = NULL;
	}
}

CString CFilterFieldInfo::GetFieldNameApparent() const
{
	return m_strFieldNameApparent;
}

LPCTSTR CFilterFieldInfo::GetNextParam(LPCTSTR strCurParam, BOOL bMultiSelect /* = FALSE*/) const
{
	TCHAR *m_pstrParam;
	if (bMultiSelect)
		m_pstrParam = m_pstrMultiSelectParams;
	else
		m_pstrParam = m_pstrParameters;

	if (strCurParam == NULL) {
		// The caller wants the pointer to the first parameter
		return m_pstrParam;
	} else if (m_pstrParam) {
		// Find the beginning of the next parameter
		return strCurParam + strlen(strCurParam) + 1;
	} else {
		// If we made it here there are no parameters
		return NULL;
	}
}

bool CFilterFieldInfo::IsVisible() const
{
	return m_bVisible;
}

bool CFilterFieldInfo::IsObsolete() const
{
	return m_bObsolete;
}

bool CFilterFieldInfo::IsRemoved() const
{
	return m_bRemoved;
}

BOOL CFilterFieldInfo::HasAdvancedFilter() const {

	if (m_ftFieldType == ftAdvanced) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}

// (c.haag 2010-12-07 17:20) - PLID 40640 - We now take in an optional connection pointer
// (r.gonet 10/23/2013) - PLID 56236 - Extracted the new method FormatWhereCondition
bool CFilterFieldInfo::GetWhereClause(FieldOperatorEnum foOperator, const CString &strValue, bool bUseOr, CMapStringToString &mapUsedFilters, CString &strOutClause, bool bIncludePrefix, BOOL bSilent /*=FALSE*/,
									  _ConnectionPtr pCon /* = NULL */) const
{
	//TES 6/20/03: Before we do anything else...
	if(m_bRemoved) {
		return false;
	}
	
	CString strPrefix;

	// Decide on the appropriate prefix
	if (bIncludePrefix) {
		if (bUseOr) {
			strPrefix = " OR ";
		} else {
			strPrefix = " AND ";
		}
	} else {
		strPrefix.Empty();
	}

	// Calculate the exists before and after strings
	CString strExistsBefore, strExistsAfter;
	if (!m_strExistsSource.IsEmpty()) {
		CString strBase;
		if (!m_strExistsBaseWhere.IsEmpty()) {
			strBase.Format("%s AND ", m_strExistsBaseWhere);
		}
		// (j.jones 2010-08-27 16:02) - PLID 38955 - if m_bCheckExists is false (rare), we want to insert
		// a NOT statement at the beginning
		strExistsBefore.Format("%s EXISTS (SELECT * FROM %s WHERE %s ", m_bCheckExists ? "" : "NOT", m_strExistsSource, strBase);
		strExistsAfter.Format(")");
	}

	// (r.gonet 10/23/2013) - PLID 56236 - Extracted the new method FormatWhereCondition
	return FormatWhereCondition(foOperator, strValue, strOutClause, mapUsedFilters, strPrefix, strExistsBefore, strExistsAfter, bSilent, pCon);
}

// (r.gonet 10/23/2013) - PLID 56236 - Extracted the new method FormatWhereCondition
// (r.gonet 10/23/2013) - PLID 56236 - Added strInternalFieldNameOverride. If non-NULL, then this value will 
//  be used as the internal field name. If this is NULL, then the CFilterFieldInfo's internal field name will be used.
bool CFilterFieldInfo::FormatWhereCondition(FieldOperatorEnum foOperator, const CString &strValue, OUT CString &strWhereCondition, CMapStringToString &mapUsedFilters, CString &strPrefix, CString &strExistsBefore, CString &strExistsAfter, BOOL bSilent/* = FALSE*/, ADODB::_ConnectionPtr pCon/* = NULL*/, LPCTSTR strInternalFieldNameOverride/* = NULL*/) const
{
	CString strInternalFieldName = (strInternalFieldNameOverride == NULL ? m_strFieldNameInternal : strInternalFieldNameOverride);
	// If this detail has multiple items selected change the operator accordingly
	if (strValue.Find(",") != -1 && m_pstrMultiSelectParams) {
		if (foOperator == foEqual)
			foOperator = foIn;
		else if (foOperator == foNotEqual)
			foOperator = foNotIn;
		else
			// This isn't a supported operator for multiple selected items
			ASSERT(FALSE);
	}

	CString strFormattedValue;
	if (m_ftFieldType == ftAdvanced) {
		// Advanced filters are simply WHERE clauses, so just return it with the AND/OR in front
		strWhereCondition.Format("%s(%s)", strPrefix, (LPCTSTR)strValue);
		return true;
	
	} else {
		//JJ - 11/21/2001 - If the value is blank, and the marker is blank, then we're going
		//to get an error. If either of these contain something, however, we're okay
		if(strValue.GetLength() == 0 && m_strMarker.GetLength() == 0 &&
			foOperator != foBlank && foOperator != foNotBlank) {
			return false;
		}
		// (c.haag 2010-12-07 17:20) - PLID 40640 - Pass the connection pointer to each call to FormatValue. I don't know
		// why the silent flag is ignored; but I'm doing it this way for consistency. We can fix the silent flag reference in another PL item.
		switch (foOperator) {
		case foEqual:
			if (!FormatValue(strValue, strFormattedValue, mapUsedFilters, NULL, NULL, FALSE, pCon)) return false;
			strWhereCondition.Format("%s%s(%s = %s)%s", strPrefix, strExistsBefore, strInternalFieldName, strFormattedValue, strExistsAfter);
			return true;
		case foGreater:
			if (!FormatValue(strValue, strFormattedValue, mapUsedFilters, NULL, NULL, FALSE, pCon)) return false;
			strWhereCondition.Format("%s%s(%s > %s)%s", strPrefix, strExistsBefore, strInternalFieldName, strFormattedValue, strExistsAfter);
			return true;
		case foGreaterEqual:
			if (!FormatValue(strValue, strFormattedValue, mapUsedFilters, NULL, NULL, FALSE, pCon)) return false;
			strWhereCondition.Format("%s%s(%s >= %s)%s", strPrefix, strExistsBefore, strInternalFieldName, strFormattedValue, strExistsAfter);
			return true;
		case foLess:
			if (!FormatValue(strValue, strFormattedValue, mapUsedFilters, NULL, NULL, FALSE, pCon)) return false;
			strWhereCondition.Format("%s%s(%s < %s)%s", strPrefix, strExistsBefore, strInternalFieldName, strFormattedValue, strExistsAfter);
			return true;
		case foLessEqual:
			if (!FormatValue(strValue, strFormattedValue, mapUsedFilters, NULL, NULL, FALSE, pCon)) return false;
			strWhereCondition.Format("%s%s(%s <= %s)%s", strPrefix, strExistsBefore, strInternalFieldName, strFormattedValue, strExistsAfter);
			return true;
		case foNotEqual:
			if (!FormatValue(strValue, strFormattedValue, mapUsedFilters, NULL, NULL, FALSE, pCon)) return false;
			strWhereCondition.Format("%s%s((%s Is Null) OR (%s <> %s))%s", strPrefix, strExistsBefore, strInternalFieldName, strInternalFieldName, strFormattedValue, strExistsAfter);
			return true;
		case foLike:
			// (a.walling 2011-06-17 15:01) - PLID 37518 - We actually want LIKE patterns characters here, so don't pass true for bIsLikePattern
			if (!FormatValue(strValue, strFormattedValue, mapUsedFilters, "'", "'", FALSE, pCon)) return false;
			strWhereCondition.Format("%s%s(%s Like %s)%s", strPrefix, strExistsBefore, strInternalFieldName, strFormattedValue, strExistsAfter);
			return true;
		case foNotLike:
			// (a.walling 2011-06-17 15:01) - PLID 37518 - We actually want LIKE patterns characters here, so don't pass true for bIsLikePattern
			if (!FormatValue(strValue, strFormattedValue, mapUsedFilters, "'", "'", FALSE, pCon)) return false;
			strWhereCondition.Format("%s%s(%s Not Like %s)%s", strPrefix, strExistsBefore, strInternalFieldName, strFormattedValue, strExistsAfter);
			return true;
		case foBeginsWith:
			if (!FormatValue(strValue, strFormattedValue, mapUsedFilters, "'", "%%'", FALSE, pCon, true)) return false;
			strWhereCondition.Format("%s%s(%s Like %s)%s", strPrefix, strExistsBefore, strInternalFieldName, strFormattedValue, strExistsAfter);
			return true;
		case foEndsWith:
			if (!FormatValue(strValue, strFormattedValue, mapUsedFilters, "'%%", "'", FALSE, pCon, true)) return false;
			strWhereCondition.Format("%s%s(%s Like %s)%s", strPrefix, strExistsBefore, strInternalFieldName, strFormattedValue, strExistsAfter);
			return true;
		case foContains:
			if (!FormatValue(strValue, strFormattedValue, mapUsedFilters, "'%%", "%%'", FALSE, pCon, true)) return false;
			strWhereCondition.Format("%s%s(%s Like %s)%s", strPrefix, strExistsBefore, strInternalFieldName, strFormattedValue, strExistsAfter);
			return true;
		case foBlank:
			strWhereCondition.Format("%s%s(%s Is Null OR RTRIM(LTRIM(CONVERT(nvarchar(50), %s))) = '')%s", strPrefix, strExistsBefore, strInternalFieldName, strInternalFieldName, strExistsAfter);
			return true;
		case foNotBlank:
			strWhereCondition.Format("%s%s(%s Is Not Null AND RTRIM(LTRIM(CONVERT(nvarchar(50), %s))) != '')%s", strPrefix, strExistsBefore, strInternalFieldName, strInternalFieldName, strExistsAfter);
			return true;
		case foIn:
			if(!FormatValue(strValue, strFormattedValue, mapUsedFilters, NULL, NULL, bSilent, pCon)) return false;
			strWhereCondition.Format("%s%s(%s In (%s))%s", strPrefix, strExistsBefore, strInternalFieldName, strFormattedValue, strExistsAfter);
			return true;
		case foNotIn:
			if(!FormatValue(strValue, strFormattedValue, mapUsedFilters, NULL, NULL, FALSE, pCon)) return false;
			strWhereCondition.Format("%s%s(%s NOT In (%s))%s", strPrefix, strExistsBefore, strInternalFieldName, strFormattedValue, strExistsAfter);
			return true;

		case foInvalidOperator:
		default:
			// Shouldn't get here because either an unknown operator was passed or the known invalid 
			// operator (foInvalidOperator); these cases are undefined
			ASSERT(FALSE);
			return false;
		}
	}

	// If we made it to here we failed
	return false;
}

CString CFilterFieldInfo::GetMarker(FieldTypeEnum ftType)
{
	switch (ftType) {
	case ftText:
	case ftPhoneNumber:
		// Text uses " as marker
		return "'";
	case ftDate:
	case ftTime:
		// Date uses # as marker
		return "'";
	default:
		// Default is NO marker
		return "";
	}
}
bool ParseRelativeDate(const CString &strPlusMinusText, COleDateTimeSpan &dtsOut)
{
	CString strInput = strPlusMinusText;
	strInput.TrimLeft();
	strInput.TrimRight();
	if(strInput.IsEmpty()) {
		//It was just whitespace, so just return 0.
		dtsOut.SetDateTimeSpan(0,0,0,0);
		return true;
	}
	else {
		if(strInput.Left(1) == "+") {
			//OK, they're adding some days.
			strInput = strInput.Mid(1);
			strInput.TrimLeft();
			if(strInput.SpanIncluding("1234567890").GetLength() == strInput.GetLength()) {
				//Great, it's just numbers.
				dtsOut.SetDateTimeSpan(atol(strInput),0,0,0);
				return true;
			}
			else {
				//Non-numeric text!
				return false;
			}
		}
		else if(strInput.Left(1) == "-") {
			//OK, they're subtracting some days.
			strInput = strInput.Mid(1);
			strInput.TrimLeft();
			if(strInput.SpanIncluding("1234567890").GetLength() == strInput.GetLength()) {
				//Great, it's just numbers.
				dtsOut.SetDateTimeSpan(-1 * atol(strInput),0,0,0);
				return true;
			}
			else {
				//Non-numeric text!
				return false;
			}
		}
		else {
			//Invalid char!
			return false;
		}
	}
}

bool FormatValueDate(const CString &strIn, CString &strOut)
{
	// Convert it to a date
	//First, is it one of our "special" values?
	COleDateTimeSpan dtsOneDay(1,0,0,0);
	if(strIn.Left(5).CompareNoCase("Today") == 0) {
		//Is there more?
		if(strIn.GetLength() > 5) {
			//Yes. Let's parse it.
			COleDateTimeSpan dtsDiff;
			if(ParseRelativeDate(strIn.Mid(5), dtsDiff)) {
				strOut = FormatDateTimeForSql(COleDateTime::GetCurrentTime() + dtsDiff, dtoDate);
				return true;
			}
			else {
				//Invalid text!
				return false;
			}
		}
		else {
			//Nope, so just return "today"
			strOut = FormatDateTimeForSql(COleDateTime::GetCurrentTime(), dtoDate);
			return true;
		}
	}
	else if(strIn.Left(9).CompareNoCase("Yesterday") == 0) {
		//Is there more?
		if(strIn.GetLength() > 9) {
			//Yes. Let's parse it.
			COleDateTimeSpan dtsDiff;
			if(ParseRelativeDate(strIn.Mid(9), dtsDiff)) {
				strOut = FormatDateTimeForSql(COleDateTime::GetCurrentTime() - dtsOneDay + dtsDiff, dtoDate);
				return true;
			}
			else {
				//Invalid text!
				return false;
			}
		}
		else {
			//Nope, so just return "yesterday"
			strOut = FormatDateTimeForSql(COleDateTime::GetCurrentTime() - dtsOneDay, dtoDate);
			return true;
		}
	}
	else if(strIn.Left(8).CompareNoCase("Tomorrow") == 0) {
		//Is there more?
		if(strIn.GetLength() > 8) {
			//Yes. Let's parse it.
			COleDateTimeSpan dtsDiff;
			if(ParseRelativeDate(strIn.Mid(8), dtsDiff)) {
				strOut = FormatDateTimeForSql(COleDateTime::GetCurrentTime() + dtsOneDay + dtsDiff, dtoDate);
				return true;
			}
			else {
				//Invalid text!
				return false;
			}
		}
		else {
			strOut = FormatDateTimeForSql((COleDateTime::GetCurrentTime() + dtsOneDay), dtoDate);
			return true;
		}
	}
	else {
		COleDateTime dt;
		if (dt.ParseDateTime(strIn)) {
			strOut = FormatDateTimeForSql(dt, dtoDate);
			//strOut = dt.Format();
			return true;
		} else {
			strOut = strIn;
			return false;
		}
	}
}

bool FormatValueTime(const CString &strIn, CString &strOut)
{
	// Convert it to a time
	COleDateTime dt;
	if (dt.ParseDateTime(strIn)) {
		strOut = FormatDateTimeForSql(dt, dtoTime);
		//strOut = dt.Format();
		return true;
	} else {
		strOut = strIn;
		return false;
	}
}

bool FormatValueNumber(const CString &strIn, CString &strOut)
{
	// Trim the incoming string
	long nOut = atol(strIn);
	if (nOut != 0) {
		// If it returns a number that's not zero, it must be a valid number so format it as such
		strOut.Format("%li", nOut);
		return true;
	} else {
		CString strTrimIn(strIn);
		strTrimIn.TrimLeft(); strTrimIn.TrimRight();
		if (strIn.GetLength() > 0 && strIn.GetAt(0) == '0') {
			// If the string starts with a 0 and the above atol returned 0 then this must a true zero 
			strOut.Format("%li", nOut);
			return true;
		} else {
			// It's a non-numeric string so fail
			return false;
		}
	}
}

bool FormatValueCurrency(const CString &strIn, CString &strOut)
{
	if(!IsValidCurrencyText(strIn)) return false;
	strOut = FormatCurrencyForSql(ParseCurrencyFromInterface(strIn));
	return true;
}
// (a.walling 2011-06-17 15:01) - PLID 37518 - Like patterns need different escaping compared to other operators
bool FormatValueText(const CString &strIn, CString &strOut, bool bIsLikePattern = false)
{
	strOut = _Q(strIn);

	if (bIsLikePattern) {
		// escape out special chars in LIKE clauses
		strOut.Replace("[", "[[]");		// Same for T-SQL
		strOut.Replace("_", "[_]");		// For T-SQL use underscore (_)
		strOut.Replace("%", "[%]");		// For T-SQL use percent (%)
	}
	return true;
}

bool FormatValuePhoneNumber(const CString &strIn, CString &strOut)
{
	for (int i = 0; i < strIn.GetLength(); i++) {
		if (strIn.GetAt(i) > 47 && strIn.GetAt(i) < 58) 
			strOut+= (CString)strIn.GetAt(i);
	}
	return true;
}

// (c.haag 2010-12-07 17:20) - PLID 40640 - We now take in an optional connection pointer
// (r.gonet 10/23/2013) - PLID 56236 - Extracted new method ParseSubFilterValue
bool CFilterFieldInfo::FormatSubfilter(const CString &strIn, CString &strOut, CMapStringToString &mapUsedFilters, long nFilterType, BOOL bSilent /*= FALSE*/,
									   _ConnectionPtr pCon /* = NULL */) const		
{
	try {
		//Let's look up the main field for this subfilter type.
		// (c.haag 2010-12-07 17:20) - PLID 40640 - Ensure the connection object is valid
		CString strFilterText;
		long nID;
		
		// (r.gonet 10/23/2013) - PLID 56236 - Extracted a method which parses the subfilter value into the ID of the subfilter and the filter.
		if(!ParseSubFilterValue(strIn, nID, strFilterText, bSilent, pCon)) {
			return false;
		}
		long nBaseIndex = GetInfoIndex(nFilterType, (CFilterFieldInfo*)CFilterDetail::g_BaseDependencies, CFilterDetail::g_nBaseDependencyCount);
		CString strSubFilterField = CFilterDetail::g_BaseDependencies[nBaseIndex].m_strFieldNameInternal;
		// Get the clauses from the filter so we can generate a "SELECT ID FROM <subfilter>"
		CString strFrom, strWhere;
		
		if (CFilter::ConvertFilterStringToClause(nID, strFilterText, m_nFilterType, &strWhere, &strFrom, &mapUsedFilters)) {
			// Build the subfilter
			strOut = "SELECT DISTINCT " + strSubFilterField + " FROM " + strFrom + " WHERE " + strWhere;
			return true;
		}
		else {
			return false;
		}
	} NxCatchAllCall("CFilterFieldInfo::FormatSubFilter", return false);
}

// (r.gonet 10/23/2013) - PLID 56236 - Parses the subfilter value into the ID of the subfilter and the filter. Returns true if the parse was successful and false if the parse failed.
bool CFilterFieldInfo::ParseSubFilterValue(const CString &strValue, OUT long &nSubFilterID, OUT CString &strSubFilterString, BOOL bSilent /*= FALSE*/,
											 _ConnectionPtr pCon /* = NULL */) const
{
	if (NULL == pCon) {
		pCon = GetRemoteData();
	}
	if(strValue.Left(1) == "0") {
		//OK, this is embedded.
		strSubFilterString = strValue.Mid(strValue.Find("-")+1);
		if(strSubFilterString.IsEmpty()) {
			if (!bSilent)
				MsgBox("You must add at least one field to your one-time subfilter");
			return false;
		}
		nSubFilterID = 0;
	}
	else {
		//We need to look this up, it will be in our select clause.
		CString strQueryNoOrderBy = m_pstrParameters;
		long nOrderByStart, nOrderByEnd;
		FindClause(strQueryNoOrderBy, fcOrderBy, nOrderByStart, nOrderByEnd);
		if(nOrderByEnd >= strQueryNoOrderBy.GetLength()-1) {
			strQueryNoOrderBy = strQueryNoOrderBy.Left(nOrderByStart);
		}
		else {
			strQueryNoOrderBy = strQueryNoOrderBy.Left(nOrderByStart) + strQueryNoOrderBy.Mid(nOrderByEnd);
		}
		_RecordsetPtr rsFilter = CreateRecordset(pCon, "SELECT Filter FROM (%s) SubQ WHERE SubQ.ID = %s", strQueryNoOrderBy, strValue);
		if(rsFilter->eof) {
			if (!bSilent)
				MsgBox("Filter string references invalid subfilter");
			return false;
		}
		strSubFilterString = AdoFldString(rsFilter, "Filter");
		nSubFilterID = atol(strValue);
	}
	return true;
}

// (c.haag 2010-12-07 17:20) - PLID 40640 - We now take in an optional connection pointer
// (a.walling 2011-06-17 15:01) - PLID 37518 - Like patterns need different escaping compared to other operators
bool CFilterFieldInfo::FormatValue(const CString &strIn, CString &strOut, CMapStringToString &mapExcludeParentSubfilters, LPCTSTR strPreMarker /*= NULL*/, LPCTSTR strPostMarker /*= NULL*/, BOOL bSilent /*= FALSE*/,
								   _ConnectionPtr pCon /* = NULL */, bool bIsLikePattern /* = false*/) const 
{
	//OK, the plan is: first fix up our value.  This means to get it ready for putting inside markers, then, if
	//we were passed in markers, to put them on, otherwise using markers based on the field type.  Finally,
	//if appropriate, we'll put our value into the ValueFormat string and return that.
	
	// Decide how to handle the value
	CString strValue;
	switch (m_ftFieldType) {
	case ftComboValues:
		// Combo values are just the exact text
		strValue = strIn;
		break;
	case ftComboSelect:
		// Combo values are just the exact text (because it's the ID anyway)
		strValue = strIn;
		break;
	case ftText:
		// Quotation marks need to be replaced with two quotation marks
		// (a.walling 2011-06-17 15:01) - PLID 37518 - Like patterns need different escaping compared to other operators
		if(!FormatValueText(strIn, strValue, bIsLikePattern)) return false;
		break;
	case ftPhoneNumber:
		// Quotation marks need to be replaced with two quotation marks
		if(!FormatValuePhoneNumber(strIn, strValue)) return false;
		break;
	case ftDate:
		// Format the incoming text as a date
		if(!FormatValueDate(strIn, strValue)) return false;
		break;
	case ftTime:
		// Format the incoming text as a time
		if(!FormatValueTime(strIn, strValue)) return false;
		break;
	case ftNumber:
		// Format the incoming text as a long integer
		if(!FormatValueNumber(strIn, strValue)) return false;
		break;
	case ftCurrency:
		// Format the incoming text as a currency
		if(!FormatValueCurrency(strIn, strValue)) return false;
		break;
	case ftSubFilter:
	case ftSubFilterEditable:
		{
			//We're not adding any markers or crap, so just get strOut and return.
			return FormatSubfilter(strIn, strOut, mapExcludeParentSubfilters, m_nSubfilterType, bSilent, pCon);
		}
	default:
		// Anything else is not handled
		return false;
	}	

	//At this point, strValue is ready to get put inside markers.  Let's decide what markers to put.
	if(strPreMarker) {
		strValue = strPreMarker + strValue;
	}
	else {
		strValue = m_strMarker + strValue;
	}
	if(strPostMarker) {
		strValue += strPostMarker;
	}
	else {
		strValue += m_strMarker;
	}
	
	//OK, now let's put it inside our more complex format string (if appropriate).
	if(m_bUseValueFormat) {
		strOut = m_strValueFormat;
		strOut.Replace("{UserEnteredValue}", strValue);
	}
	else {
		strOut = strValue;
	}
	return true;


}

long CFilterFieldInfo::GetInfoIndex(unsigned long nInfoId, CFilterFieldInfo *arFields, long nFieldCount)
{
	// Search through all the fields to find
	for (long i=0; i<nFieldCount; i++) {
		if (arFields[i].m_nInfoId == nInfoId) {
			return i;
		}
	}
	// Failed
	return -1;
}

long CFilterFieldInfo::GetInfoIndex(const CString &strApparentField, CFilterFieldInfo *arFields, long nFieldCount)
{
	// Search for the index of the item that has this apparent field name
	for (long i=0; i<nFieldCount; i++) {
		if (arFields[i].GetFieldNameApparent().CompareNoCase(strApparentField) == 0) {
			// We found which field type this is, so save it and stop searching
			return i;
		}
	}

	// Failed
	return -1;
}

bool CFilterFieldInfo::AddJoinToMap(CFilterDetail *pDetail, CMapStringToPtr &mapJoins, CMapPtrToWord &mapExcludeParents) const
{
	// We only add this one if it hasn't been added already
	void *pbuf;
	if (!mapJoins.Lookup(m_strJoinClause, pbuf)) {
		// Make sure all the info field's dependency joins are in there
		mapJoins.SetAt(m_strJoinClause, (void *)this);
	}

	// Now check this filterfield's direct join dependencies
	if(m_nJoinDependsOn == FILTER_DEPENDS_ON_BASE) {
		//We need to get this out of the BaseDependencies array.
		long nIndex = GetInfoIndex(m_nFilterType, (CFilterFieldInfo*)CFilterDetail::g_BaseDependencies, CFilterDetail::g_nBaseDependencyCount);
		//Make sure the index is valid
		if((nIndex >= 0) && (nIndex < CFilterDetail::g_nBaseDependencyCount)) {
			//Now add it
			if (!CFilterDetail::g_BaseDependencies[nIndex].AddJoinToMap(NULL, mapJoins, mapExcludeParents)) {
				//If this add failed, we failed
				return false;
			}
			else {
				return true;
			}
		}
		else {
			// If it's not a valid id, we failed!
			return false;
		}
	}
	else if(m_nJoinDependsOn == FILTER_DEPENDS_ON_NONE) {
		//That's fine, they don't want any dependencies, we won't give them any.
		return true;
	}
	else {
		//OK, we're looking for an actual filter.
		// Find the index that this InfoId is under
		long nIndex = GetInfoIndex(m_nJoinDependsOn, (CFilterFieldInfo*)CFilterDetail::g_FilterFields, CFilterDetail::g_nFilterFieldCount);
		// Make sure the index is valid
		if ((nIndex >= 0) && (nIndex < CFilterDetail::g_nFilterFieldCount)) {
			// Now add it
			if (!CFilterDetail::g_FilterFields[nIndex].AddJoinToMap(NULL, mapJoins, mapExcludeParents)) {
				// If this add failed, we failed
				return false;
			}
			else {
				return true;
			}
		} else {
			// If it's not a valid id, we failed!
			ASSERT(FALSE);
			return false;
		}
	}

}

void CFilterFieldInfo::RefreshFieldNames()
{
	m_mapReplaceFieldNames.RemoveAll();
}
// FilterFieldInfo.h: interface for the CFilterFieldInfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FILTERFIELDINFO_H__3F331A22_46B0_11D4_957A_00C04F4C8415__INCLUDED_)
#define AFX_FILTERFIELDINFO_H__3F331A22_46B0_11D4_957A_00C04F4C8415__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define FILTER_DEPENDS_ON_BASE					(unsigned long)0
#define FILTER_DEPENDS_ON_NONE					(unsigned long)-1
#define FILTER_BUILT_IN(num)					(0xffff - (num))

//This field is now added and maintained by the framework.
//#define FILTER_FIELD_SPECIAL_NONE			FILTER_BUILT_IN(1)
#define FILTER_FIELD_SPECIAL_SUBFILTER		FILTER_BUILT_IN(2)
#define FILTER_FIELD_SPECIAL_ADVANCED		FILTER_BUILT_IN(3)
#define FILTER_FIELD_SPECIAL_GROUP			FILTER_BUILT_IN(4)

enum FieldOperatorEnum {
	// Invalid operator
	foInvalidOperator = 0x0000,

	// Basic operators
	foEqual				= 0x0001,
	foGreater			= 0x0002,
	foGreaterEqual		= 0x0004,
	foLess				= 0x0008,
	foLessEqual			= 0x0010,
	foNotEqual			= 0x0020,
	foLike				= 0x0040,
	foNotLike			= 0x0080,
	foBeginsWith		= 0x0100,
	foEndsWith			= 0x0200,
	foContains			= 0x0400,
	foIn				= 0x0800,
	foNotIn				= 0x1000,
	foBlank				= 0x2000,
	foNotBlank			= 0x4000,

	// Common Operator option groups
	foAllGreater		= foGreater		| foGreaterEqual,
	foAllLess			= foLess		| foLessEqual,
	foEquality			= foEqual		| foNotEqual	| foBlank		| foNotBlank | foAllGreater | foAllLess,
	foSimilarity		= foLike		| foNotLike		| foBeginsWith	| foEndsWith | foContains,
	foAll				= foEquality	| foSimilarity,
	// (z.manning 2009-06-03 09:18) - PLID 34430 - Added a group for the options that are considered searching.
	foSearching			= foEqual | foLike | foBeginsWith | foEndsWith | foContains,
};

enum FieldTypeEnum {
	ftText				= 0x0001,
	ftDate				= 0x0002,
	ftNumber			= 0x0008,
	ftComboValues		= 0x0010,
	ftComboSelect		= 0x0020,
	ftTime				= 0x0040,
	ftPhoneNumber		= 0x0080,
	ftSubFilter			= 0x0100,
	ftSubFilterEditable = 0x0200,
	ftAdvanced			= 0x0400,
	ftCurrency			= 0x0800,
};

enum SupportedActionsEnum {
	saAdd				= 0x0001,
	saEdit				= 0x0002,
	saDelete			= 0x0003,
};


// It is assumed that this is greater or equal to 1
#define PARAM_COUNT_MAX		512

class CFilterDetail;

// The actual class
class CFilterFieldInfo
{
public:
	// (j.jones 2010-08-27 15:54) - PLID 39855 - added bCheckExists, defaults to true, such that if we set it to false it means we
	// are explicitly filtering on records that do NOT match the successive filter
	CFilterFieldInfo(		unsigned long nInfoId, const CString &strFieldApparent, const CString &strFieldInternal, long nFilterType, long nAvailableOperators = foAll, 
							FieldOperatorEnum foDefault = foBeginsWith, FieldTypeEnum ftFieldType = ftText, LPCTSTR strParams = NULL, 
							LPCTSTR strJoinClause = NULL, unsigned long nJoinDependsOn = FILTER_DEPENDS_ON_BASE, LPCTSTR strExistsSource = NULL, LPCTSTR strExistsBaseWhere = NULL, 
							LPCTSTR strValueFormat = NULL, LPCTSTR strSubFilterDisplayName = NULL, long nSubfilterType = 0, LPCTSTR strMultiSelectParams = NULL,
							bool bCheckExists = true);
	//TS: This constructor, after the modifications I made to include nFilterType, was more trouble than it was worth.
	//CFilterFieldInfo(	long nInfoId, long nFilterType, const CString &strFieldApparent, const CString &strFieldInternal, LPCTSTR strJoinClause = NULL, long nJoinDependsOn = 0);
	CFilterFieldInfo(	unsigned long nInfoId, long nFilterType, LPCTSTR strFieldInternal, LPCTSTR strJoinClause, unsigned long nJoinDependsOn = FILTER_DEPENDS_ON_NONE);
	//TS: This constructor is for obsolete filter fields.
	CFilterFieldInfo(	unsigned long nInfoId, CString strDefaultReplacement);
	//TES 6/20/03: This constructor is used for removed filter fields.
	CFilterFieldInfo(	CString strOldFilterName, unsigned long nInfoID, CString strRemovalExplanation, CString strRemovalHelpLocation, CString strRemovalHelpBookmark);
	virtual ~CFilterFieldInfo();

public:
	// Standard fields
	unsigned long m_nInfoId;
	CString GetFieldNameApparent() const;
	CString m_strFieldNameInternal;
	long m_nAvailableOperators;
	FieldOperatorEnum m_foDefault;
	FieldTypeEnum m_ftFieldType;
	CString m_strMarker;
	long m_nFilterType;

	// Dynamic parameter field (null-delimited text parameter list, double-null terminated)
	LPCTSTR GetNextParam(LPCTSTR strCurParam, BOOL bMultiSelect = FALSE) const;
	bool IsVisible() const;
	TCHAR *m_pstrParameters;
	TCHAR *m_pstrMultiSelectParams;

	bool IsObsolete() const;
	CString m_strDefaultReplacement;//Used to update filters that use obsolete fields.

	bool IsRemoved() const;
	CString m_strRemovalExplanation;//Used to explain to the user that their filter field is no more.
	CString m_strRemovalHelpLocation;
	CString m_strRemovalHelpBookmark;	

	// Strings used for creating joins in a dynamic FROM clause
	CString m_strJoinClause;
	unsigned long m_nJoinDependsOn;

	// String used for optionally using an EXISTS operator in the WHERE clause
	// ex. SELECT * FROM {Generated FROM clause} WHERE field1 > value1 AND EXISTS(SELECT * FROM m_strExistsSource WHERE m_strExistsBaseWhere AND field2 > value2)
	CString m_strExistsSource;
	CString m_strExistsBaseWhere;
	// (j.jones 2010-08-27 16:02) - PLID 39855 - added m_bCheckExists, almost always true,
	// but if false will insert a NOT before the EXISTS call
	bool m_bCheckExists;

	CString m_strSubfilterDisplayName;//Used ONLY by ftSubfilterEditable types.
	long m_nSubfilterType;//Used ONLY by ftSubfilterEditable types.

	BOOL m_bUseValueFormat;
	CString m_strValueFormat;

protected:
	CString m_strFieldNameApparent;
	bool m_bVisible;
	bool m_bObsolete;
	bool m_bRemoved;
	
	//Used to cache all the custom names so we don't have to keep requerying the data.
	static CMapStringToString m_mapReplaceFieldNames;

public:
	static CString GetMarker(FieldTypeEnum ftType);
	static long CFilterFieldInfo::GetInfoIndex(unsigned long nInfoId, CFilterFieldInfo *arFields, long nFieldCount);
	static long CFilterFieldInfo::GetInfoIndex(const CString &strApparentField, CFilterFieldInfo *arFields, long nFieldCount);

	//Clears out the cache of custom names.
	static void RefreshFieldNames();

public:
	// (c.haag 2010-12-07 17:20) - PLID 40640 - We now take in an optional connection pointer
	bool GetWhereClause(FieldOperatorEnum foOperator, const CString &strValue, bool bUseOr, CMapStringToString &mapUsedFilters, CString &strOutClause, bool bIncludePrefix, BOOL bSilent = FALSE, ADODB::_ConnectionPtr pCon = NULL) const;
	// (r.gonet 10/23/2013) - PLID 56236 - Extracted from GetWhereClause.
	bool FormatWhereCondition(FieldOperatorEnum foOperator, const CString &strValue, OUT CString &strWhereCondition, CMapStringToString &mapUsedFilters, CString &strPrefix, CString &strExistsBefore, CString &strExistsAfter, BOOL bSilent = FALSE, ADODB::_ConnectionPtr pCon = NULL, LPCTSTR strInternalFieldNameOverride = NULL) const;
	bool AddJoinToMap(CFilterDetail *pDetail, CMapStringToPtr &mapJoins, CMapPtrToWord &mapExcludeParents) const;

	// (c.haag 2010-12-07 17:20) - PLID 40640 - We now take in an optional connection pointer
	// (a.walling 2011-06-17 15:01) - PLID 37518 - Like patterns need different escaping compared to other operators
	bool FormatValue(const CString &strIn, CString &strOut, CMapStringToString &mapExcludeParentSubfilters, LPCTSTR strPreMarker = NULL, LPCTSTR strPostMarker = NULL, BOOL bSilent = FALSE, ADODB::_ConnectionPtr pCon = NULL, bool bIsLikePattern = false) const;
	//Note the exclusion map below is a list of subfilters, NOT of joins.  Each subfilter maintains its own join list.
	// (c.haag 2010-12-07 17:20) - PLID 40640 - We now take in an optional connection pointer
	bool FormatSubfilter(const CString &strIn, CString &strOut, CMapStringToString &mapExcludeParentSubfilters, long nFilterType, BOOL bSilent = FALSE, ADODB::_ConnectionPtr pCon = NULL) const;
	// (r.gonet 10/09/2013) - PLID 56236 - Extracted from FormatSubFilter. Parses the subfilter value into the ID of the subfilter and the filter. Returns true if the parse was successful and false if the parse failed.
	bool ParseSubFilterValue(const CString &strValue, OUT long &nSubFilterID, OUT CString &strSubFilterString, BOOL bSilent = FALSE, ADODB::_ConnectionPtr pCon = NULL) const;

	//find out if its an advanced filter
	BOOL HasAdvancedFilter() const;
};

#endif // !defined(AFX_FILTERFIELDINFO_H__3F331A22_46B0_11D4_957A_00C04F4C8415__INCLUDED_)

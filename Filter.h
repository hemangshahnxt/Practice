#if !defined(AFX_FILTER_H__23BC0ED3_4DE4_11D4_957C_00C04F4C8415__INCLUDED_)
#define AFX_FILTER_H__23BC0ED3_4DE4_11D4_957C_00C04F4C8415__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FilterDlg.h : header file
//

#include "AuditTrail.h"

/////////////////////////////////////////////////////////////////////////////
// CFilter dialog

// (r.gonet 10/09/2013) - PLID 56236 - Forward declaration
enum FilterBasedOnEnum;

static const long ITEM_ARRAY_SIZE = 255;
static const long ITEM_ARRAY_MAX = (ITEM_ARRAY_SIZE-1);
#include "FilterFieldInfo.h"
class CFilter
{
	friend class CFilterDetail;

private:
	// (c.haag 2010-12-07 17:20) - PLID 40640 - We now take in an optional connection pointer
	ADODB::_ConnectionPtr m_pCon;

// Construction
public:
	// (c.haag 2010-12-07 17:20) - PLID 40640 - We now take in an optional connection pointer
	CFilter(long nFilterId, long nFilterBase, BOOL (WINAPI *pfnIsActionSupported)(SupportedActionsEnum, long), BOOL (WINAPI* pfnCommitSubfilterAction)(SupportedActionsEnum, long, long&, CString&, CString&, CWnd*), BOOL (WINAPI* pfnGetNewFilterString)(long, long, CString&, LPCTSTR, CString&), ADODB::_ConnectionPtr pCon = NULL);
	virtual ~CFilter();   // standard destructor

public:
	// (c.haag 2010-12-07 17:20) - PLID 40640 - We now take in an optional connection pointer
	static bool ConvertFilterStringToClause(long nFilterId, const CString &strFilterString, long nFilterType, OUT CString *pstrWhereClause = NULL, OUT CString *pstrFromClause = NULL, CMapStringToString *pmapUsedFilters = NULL, BOOL (WINAPI* pfnGetNewFilterString)(long, long, CString&, LPCTSTR, CString&) = NULL, BOOL bSilent = FALSE, ADODB::_ConnectionPtr pCon = NULL);

	// For manipulating details (adding/removing)
public:
	bool GetFilterString(CString &strFilter);
	long SetFilterString(LPCTSTR strFilter);
	void RemoveAll();
	long AddDetail(const CString &strApparentFieldName, long nOperator, const CString &strValue, bool bUseOr, long nDynamicID /*= -1*/);
	long AddDetail(long nFieldIndex, long nOperator, const CString &strValue, bool bUseOr, long nDynamicID /*= -1*/);
	long AddDetail(const CString &strDetailString);
	long AddDetail();
	void RemoveDetail(long nIndex);

	// For generating clauses
public:
	bool GetJoinDependencies(CMapStringToPtr &mapJoins, CMapPtrToWord &mapExcludeParentJoins);
	// (z.manning 2009-06-02 16:26) - PLID 34430 - Added an optional parameter for the audit transaction
	bool GetWhereClause(CString &strOutClause, CMapStringToString &mapUsedFilters, BOOL bSilent = FALSE, CAuditTransaction *pAuditTransaction = NULL);
	bool GetFromClause(CString &strOutClause);

	//PLID find out if there is an advanced filter
public:
	BOOL HasAdvancedFilter();

public:
	virtual CFilterDetail *CreateNewDetail();
	virtual void Refresh();

public:
	//Clears out the cache of custom names.
	void RefreshReplaceNames();

	CFilterDetail *GetFilterDetail(long nItem);
	long GetDetailAryItemCount();
	// (r.gonet 10/09/2013) - PLID 56236 - Gets the base of the filter
	FilterBasedOnEnum GetFilterBase();
	bool ValidateFilter();

// Implementation
protected:
	void MoveDetail(long nFromIndex, long nToIndex);
	LPCTSTR SetNextDetail(LPCTSTR strStart);
	CFilterDetail * m_arypFilterDetails[ITEM_ARRAY_SIZE];
	long m_nItemCount;
	long m_nFilterId;
	long m_nFilterBase;

	//Used to cache custom names so we don't need to keep requerying them.
	CMapStringToString m_mapReplaceFieldNames;

	BOOL (WINAPI* m_pfnIsActionSupported)(SupportedActionsEnum, long);
	BOOL (WINAPI* m_pfnCommitSubfilterAction)(SupportedActionsEnum, long, long&, CString&, CString&, CWnd*);

	BOOL (WINAPI* m_pfnGetNewFilterString)(long, long, CString&, LPCTSTR, CString&);
};

// (r.gonet 10/09/2013) - PLID 56236 - This was a global method defined in Filter.cpp but not declared in the header file.
bool MoveJoinFromMapToClause(const CFilterFieldInfo *pffi, CMapStringToPtr &mapJoins, CString &strOutClause);

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FILTERDLG_H__23BC0ED3_4DE4_11D4_957C_00C04F4C8415__INCLUDED_)

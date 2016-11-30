// FilterDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Filter.h"
#include "FilterDetail.h"
#include "FilterFieldInfo.h"
#include "FilterEditDlg.h"

//This include should be "filterdetailcallback.h"
#include "Groups.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37023 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



/////////////////////////////////////////////////////////////////////////////
// CFilter dialog
// (c.haag 2010-12-07 17:20) - PLID 40640 - We now take in an optional connection pointer
CFilter::CFilter(long nFilterId, long nFilterBase, BOOL (WINAPI *pfnIsActionSupported)(SupportedActionsEnum, long), BOOL (WINAPI* pfnCommitSubfilterAction)(SupportedActionsEnum, long, long&, CString&, CString&, CWnd*), BOOL (WINAPI* pfnGetNewFilterString)(long, long, CString&, LPCTSTR, CString&) /*= NULL*/, ADODB::_ConnectionPtr pCon /*= NULL*/)
{
	m_pCon = pCon;
	m_nItemCount = 0;
	m_nFilterId = nFilterId;

	memset(m_arypFilterDetails, 0, sizeof(m_arypFilterDetails));
	
	m_nFilterBase = nFilterBase;

	m_pfnIsActionSupported = pfnIsActionSupported;
	m_pfnCommitSubfilterAction = pfnCommitSubfilterAction;
	m_pfnGetNewFilterString = pfnGetNewFilterString;
}

CFilter::~CFilter()
{
	RemoveAll();
}

BOOL CFilter::HasAdvancedFilter() {

	BOOL bHasAdvancedFilter = FALSE;
	for (int i = 0; i < m_nItemCount; i++) {

		if (m_arypFilterDetails[i]) {
			if (m_arypFilterDetails[i]->HasAdvancedFilter()) {
				bHasAdvancedFilter = TRUE;
			}
		}
	}
	return bHasAdvancedFilter;
}

// (z.manning 2009-06-02 16:26) - PLID 34430 - Added an optional parameter for the audit transaction
bool CFilter::GetWhereClause(CString &strOutClause, CMapStringToString &mapUsedFilters, BOOL bSilent /*= FALSE*/, CAuditTransaction *pAuditTransaction /* = NULL */)
{
	//Very first thing, check whether this subfilter's already been used:
	bool bFoundSelf = false;
	if(m_nFilterId != FILTER_ID_TEMPORARY && m_nFilterId != 0) {
		//This is a "One Time" subfilter, and we can use as many of those as we want.
		CString strFilterID;
		strFilterID.Format("%li|%li", m_nFilterId, m_nFilterBase);
		CString strTmp;
		if(!mapUsedFilters.Lookup(strFilterID, strTmp)) {
			//Let's add ourselves.
			mapUsedFilters.SetAt(strFilterID, strTmp);
		}
		else {
			//Ruh-roh, this subfilter's already in use.
			bFoundSelf = true;
		}
	}
	if(!bFoundSelf) {
		
		// Loop through all details building a where clause
		CString strAns, strItemClause;
		long nCount = 0;
		bool bSuccess = false;
		for (long i=0; i<m_nItemCount; i++) {
			if (m_arypFilterDetails[i]) {
				//OK, we ONLY want to exclude anything that was in mapUsedFilters on the FIRST iteration of the loop, 
				//not anything added in previous iterations.  So, we reset each time.
				CMapStringToString mapTempUsedFilters;
				CString strKey, strValue;
				POSITION pos;
				for( pos = mapUsedFilters.GetStartPosition(); pos != NULL; ) {
					mapUsedFilters.GetNextAssoc(pos, strKey, strValue);
					mapTempUsedFilters.SetAt(strKey, strValue);
				}


				// Get the WHERE clause of the detail; don't ask for the prefix if this is the first iteration
				bSuccess = m_arypFilterDetails[i]->GetWhereClause(strItemClause, mapTempUsedFilters, (i==0) ? false : true, bSilent);
				if (bSuccess)
				{
					if (!strItemClause.IsEmpty()) {
						// Got the clause so append it
						strAns += strItemClause;
						nCount++;
					}

					FieldOperatorEnum eOperator = m_arypFilterDetails[i]->GetDetailOperator();
					if(pAuditTransaction != NULL && (eOperator & foSearching) != 0)
					{
						// (z.manning 2009-06-02 17:45) - PLID 34430 - We need to audit when looking up
						// by last name or by birth date for CCHIT.
						const unsigned long nFilterInfoID = m_arypFilterDetails[i]->g_FilterFields[m_arypFilterDetails[i]->GetDetailInfoIndex()].m_nInfoId;
						const CString strFilterValue = m_arypFilterDetails[i]->GetDetailValue();
						switch(nFilterInfoID)
						{
							case 79: // Patient last name
								AuditEvent(-1, strFilterValue, *pAuditTransaction, aeiLookupPatientLastName, nFilterInfoID, "", "", aepLow, aetOpened);
								break;

							case 93: // Patient brith date
								AuditEvent(-1, strFilterValue, *pAuditTransaction, aeiLookupPatientBirthDate, nFilterInfoID, "", "", aepLow, aetOpened);
								break;

							default:
								break;
						}
					}

				} 
				else {
					// An attempt to get the clause of one of the items failed
					return false;
				}
			} else {
				// One of the items was null, so we wouldn't be able to get its clause
				return false;
			}
		}

		// We successfully got all the clauses so put them in a big parentheses.
		if(nCount > 0) {
			strOutClause = "(" + strAns + ")";
		}
		else {
			strOutClause = "";
		}

		return true;
	} else {
		// The only way this can happen is if this filter was 
		// already added to the exclusion list once; if this 
		// happens, we don't want to continue because that 
		// would imply a state of infinite recursion
		return false;
	}
}

long CFilter::AddDetail(const CString &strDetailString)
{
	long nItem = AddDetail();
	m_arypFilterDetails[nItem]->SetDetailString(strDetailString);
	return nItem;
}

long CFilter::AddDetail(const CString &strApparentFieldName, long nOperator, const CString &strValue, bool bUseOr, long nDynamicID /*= -1*/)
{
	long nItem = AddDetail();
	m_arypFilterDetails[nItem]->SetDetail(strApparentFieldName, nOperator, strValue, bUseOr, nDynamicID);
	return nItem;
}

long CFilter::AddDetail(long nFieldIndex, long nOperator, const CString &strValue, bool bUseOr, long nDynamicID /*= -1*/)
{
	long nItem = AddDetail();
	m_arypFilterDetails[nItem]->SetDetail(nFieldIndex, nOperator, strValue, bUseOr, nDynamicID);
	return nItem;
}

long CFilter::AddDetail()
{
	// Make sure we have room to create more items
	if (m_nItemCount <= ITEM_ARRAY_MAX) {
		// If the last item is unused use it; otherwise proceed with making a new one
		if ((m_nItemCount > 0) && m_arypFilterDetails[m_nItemCount-1] && m_arypFilterDetails[m_nItemCount-1]->IsEmpty()) {
			return m_nItemCount-1;
		}

		// Try to allocate the window object
		CFilterDetail *pDlg = CreateNewDetail();
		if (pDlg) {
			// Everything succeeded so store it and return its index in the array
			m_arypFilterDetails[m_nItemCount] = pDlg;
			m_nItemCount++;
			Refresh();			
			return m_nItemCount-1;
		} else {
			// We couldn't allocate the window
			AfxThrowNxException("Could not allocate filter detail dialog");
			return -1; 
		}
	} else {
		// We ran out of room in the array
		AfxThrowNxException("Too many filter details");
		return -1; 
	}
}

LPCTSTR CFilter::SetNextDetail(LPCTSTR strStart)
{
	if (!strStart) {
		AfxThrowNxException("Invalid filter string");
		return NULL;
	}

	LPCTSTR pPos, pstrStart = strStart;
	TCHAR ch;
	bool bInQuotes = false;
	
	// Find the end
	for (pPos=pstrStart; ; pPos++) {
		ch = *pPos;
		if (ch == '\"') {
			bInQuotes = !bInQuotes;
		} else if (ch == '\0') {
			// We hit the end of the string before hitting the end of the detail
			AfxThrowNxException("Unexpected end of string when parsing filter detail");
			return NULL;
		} else if (!bInQuotes) {
			if (ch == '}') {
				// Found the end! Yay!
				AddDetail(CString(pstrStart, pPos - pstrStart + 1));
				return pPos + 1;
			}
		}
	}

	// It should be able to get here
	AfxThrowNxException("Invalid filter detail syntax");
	return NULL;
}

// A saved filter to parse looks like this:
//		DET1 + DET2 + DET3 * DET4 * DET5 + DET6 + DET7
//  which indicates "FilterDetail1 or FilterDetail2 or (FilterDetail3 and FilterDetail4 and FilterDetail5) or FilterDetail6 or FilterDetail7
// A filter detail (such as DET2) in the above example looks like this:
//    {"ApparentFieldName", opEnum, "literal value"}
//  where 
//		-"ApparentFieldName" corresponds to one of the CFilterDetail::g_FilterFields[].m_strFieldNameApparent
//		-opEnum corresponds to the actual value of one of the FieldOperatorEnums
//		-"literal value" is the quoted text used in the final WHERE clause (do not include markers (such as pound for date, or quote for string))
//
// Throws CException
long CFilter::SetFilterString(LPCTSTR strFilter)
{
	// Clear our current filter
	RemoveAll();

	// Get the pointer to the beginning of the first detail item
	LPCTSTR pstrCurDetail = strFilter;
	long nCount = 0;
	while (*pstrCurDetail) {
 		pstrCurDetail = SetNextDetail(pstrCurDetail);
		nCount++;
	}

	// Return the count of added details
	return nCount;
}

void CFilter::RemoveAll()
{
	while (m_nItemCount > 0) {
		m_nItemCount--;
		delete m_arypFilterDetails[m_nItemCount];
		m_arypFilterDetails[m_nItemCount] = NULL;
	}
}

bool CFilter::GetFilterString(CString &strFilter)
{
	CString strAns, strDetailString;
	long nCount = 0;
	bool bSuccess = false;
	for (long i=0; i<m_nItemCount; i++) {
		if (m_arypFilterDetails[i]) {
			if (i == 0) {
				// For the first iteration, don't include the operator prefix
				bSuccess = m_arypFilterDetails[i]->GetDetailString(strDetailString, false);
			} else {
				// For the remaining iterations, include the operator prefix
				bSuccess = m_arypFilterDetails[i]->GetDetailString(strDetailString, true);
			}
			if (bSuccess) {
				if (!strDetailString.IsEmpty()) {
					// Got the clause so append it
					strAns += strDetailString;
					nCount++;
				}
			} else {
				// An attempt to get the clause of one of the items failed
				return false;
			}
		} else {
			// One of the items was null, so we wouldn't be able to get its clause
			return false;
		}
	}

	strFilter = strAns;

	return true;
}

// (c.haag 2010-12-07 17:20) - PLID 40640 - We now take in an optional connection pointer
bool CFilter::ConvertFilterStringToClause(long nFilterId, const CString &strFilterString, long nFilterType, OUT CString *pstrWhereClause /*= NULL*/, OUT CString *pstrFromClause /*= NULL*/, CMapStringToString *pmapUsedFilters /*= NULL*/, BOOL (WINAPI* pfnGetNewFilterString)(long, long, CString&, LPCTSTR, CString&)/*= NULL*/, BOOL bSilent/*= FALSE*/,
										  ADODB::_ConnectionPtr pCon)
{
	CFilter filter(nFilterId, nFilterType, NULL, NULL, pfnGetNewFilterString, pCon);
	if (filter.SetFilterString(strFilterString) >= 0) {
		// Some variables used throughout
		CString strWhere, strFrom;

		// Get the WHERE clause if requested
		if (pstrWhereClause) {
			
			//If we weren't passed in a exclusion set of subfilters, create one.
			CMapStringToString *pmapLocUsedFilters = NULL;
			if(pmapUsedFilters == NULL) {
				pmapLocUsedFilters = new CMapStringToString;
			}

			// Get the WHERE clause
			bool bGotWhereClause = filter.GetWhereClause(strWhere, pmapLocUsedFilters ? *pmapLocUsedFilters : *pmapUsedFilters, bSilent);
			// Destroy the exclusion list, but only if we created it
			if (pmapLocUsedFilters) {
				delete pmapLocUsedFilters;
				pmapLocUsedFilters = NULL;
			}
			// Failure here means the whole function failed
			if (!bGotWhereClause) {
				return false;
			}
		} 

		// Get the FROM clause if requested
		if (pstrFromClause) {
			if (!filter.GetFromClause(strFrom)) {
				// Failure here means the whole function failed
				return false;
			}
		}						 
		
		// If nothing failed, then return the values
		if (pstrWhereClause) *pstrWhereClause = strWhere;
		if (pstrFromClause) *pstrFromClause = strFrom;
		return true;
	}
	
	return false;
}

CFilterDetail *CFilter::CreateNewDetail()
{
	// (c.haag 2010-12-07 17:37) - PLID 40640 - We now take in a connection object
	return new CFilterDetail(m_nFilterBase, m_pfnGetNewFilterString, m_pCon);
}

void CFilter::Refresh()
{
	// Handled by derived class if desired
}

void CFilter::RemoveDetail(long nIndex)
{
	// Loop through all remaining items moving the next one to this one
	long nNewCount = m_nItemCount-1;

	if (nIndex != nNewCount) // Means we are not deleting the last item
	{
		for (long i=nIndex; i<nNewCount; i++) {
			MoveDetail(i+1, i);
		}
	}
	else { // Means we are deleting the last item
		delete m_arypFilterDetails[nIndex];
		m_arypFilterDetails[nIndex] = NULL;
	}
	m_nItemCount = nNewCount;
}

void CFilter::MoveDetail(long nFromIndex, long nToIndex)
{
	if (nFromIndex >= 0 && nFromIndex < m_nItemCount &&
		 nToIndex >= 0 && nToIndex < m_nItemCount) {
		// We are within the index range so first delete the destination if it exists
		if (m_arypFilterDetails[nToIndex]) {
			delete m_arypFilterDetails[nToIndex];
			m_arypFilterDetails[nToIndex] = NULL;
		}
		
		// Now do the move
		m_arypFilterDetails[nToIndex] = m_arypFilterDetails[nFromIndex];
		m_arypFilterDetails[nFromIndex] = NULL;
		m_arypFilterDetails[nToIndex]->SetPosition(nToIndex);
	} else {
		AfxThrowNxException("CFilter::MoveDetail Error 500: Could not move detail from %li to %li", nFromIndex, nToIndex);
	}
}

bool MoveJoinFromMapToClause(const CFilterFieldInfo *pffi, CMapStringToPtr &mapJoins, CString &strOutClause)
{
	if (pffi) {
		// Make sure it hasn't already been added
		void *pbuf; // Unused variable for temp storage in lookup
		if (mapJoins.Lookup(pffi->m_strJoinClause, pbuf)) {
			// First add any dependencies
			if (pffi->m_nJoinDependsOn == FILTER_DEPENDS_ON_NONE) {
				//They don't want to add any joins, that's fine.
			}
			else if (pffi->m_nJoinDependsOn == FILTER_DEPENDS_ON_BASE) {
				long nIndex = CFilterFieldInfo::GetInfoIndex(pffi->m_nFilterType, (CFilterFieldInfo*)CFilterDetail::g_BaseDependencies, CFilterDetail::g_nBaseDependencyCount);
				if((nIndex >= 0) && (nIndex <= CFilterDetail::g_nBaseDependencyCount)) {
					//Got the valid base dependency, so add it
					if (!MoveJoinFromMapToClause(&(CFilterDetail::g_BaseDependencies[nIndex]), mapJoins, strOutClause)) {
						// If this add failed, we failed
						return false;
					}
				} else {
					// There was a base dependency but it wasn't a valid filter field
					return false;
				}
			}
			else {
				long nIndex = CFilterFieldInfo::GetInfoIndex(pffi->m_nJoinDependsOn, (CFilterFieldInfo*)CFilterDetail::g_FilterFields, CFilterDetail::g_nFilterFieldCount);
				//We need to make sure that it's valid AND of the correct type.
				if ((nIndex >= 0) && (nIndex <= CFilterDetail::g_nFilterFieldCount)
					&& (pffi->m_nFilterType == CFilterDetail::g_FilterFields[nIndex].m_nFilterType)) {
					// Got the valid filter field dependency, so add it
					if (!MoveJoinFromMapToClause(&(CFilterDetail::g_FilterFields[nIndex]), mapJoins, strOutClause)) {
						// If this add failed, we failed
						return false;
					}
				} else {
					// There was a join dependency but it wasn't a valid filter field, or was the wrong type.
					ASSERT(FALSE);
					return false;
				}
			}
			// Use the filter-field's table/alias name and join-on string to add a join
			if (!pffi->m_strJoinClause.IsEmpty()) {
				if(strOutClause.IsEmpty()) {
					strOutClause = pffi->m_strJoinClause;
				}
				else {
					// Add the join
					strOutClause = "(" + strOutClause + " LEFT JOIN " + pffi->m_strJoinClause + ")";
				}
			}

			// We've added the string, so now remove it from the map
			mapJoins.RemoveKey(pffi->m_strJoinClause);
		}

		// Done
		return true;
	}

	// If we made it to here we failed
	return false;
}

bool CFilter::GetJoinDependencies(CMapStringToPtr &mapJoins, CMapPtrToWord &mapExcludeParents)
{
	// Only get dependencies if this filter hasn't already been included
	WORD buf = NULL;
	if (!mapExcludeParents.Lookup((void *)m_nFilterId, buf)) {
		// We are including our joins, so add us to the exclusion list
		mapExcludeParents.SetAt((void *)m_nFilterId, 0);
		// Now proceed with adding all our join dependencies
		long nCount = 0;
		long nIndex = -1;
		bool bSuccess = false;
		for (long i=0; i<m_nItemCount; i++) {
			// Make sure the filterdeail exists
			if (m_arypFilterDetails[i]) {
				if (m_arypFilterDetails[i]->Store()) {
					// Make sure the filter field that the detail points to is valid
					nIndex = m_arypFilterDetails[i]->m_nFieldIndex;
					// Check to see if the index is valid
					if ((nIndex >= 0) && (nIndex < CFilterDetail::g_nFilterFieldCount)) {
						// The index is valid so add the field's join clauses to the map
						if (!CFilterDetail::g_FilterFields[nIndex].AddJoinToMap(m_arypFilterDetails[i], mapJoins, mapExcludeParents)) {
							// If this one can't be retrieved the whole thing must fail
							return false;
						}
					}
				} else {
					return false;
				}
			} else {
				// One of the items was null, so we wouldn't be able to get its clause
				return false;
			}
		}
		// If we made it to here we have success
		return true;
	} else {
		// This filter is already included, so this condition
		// means it was a subfilter somewhere down the line;
		// We don't want infinite recursion so fail
		return false;
	}
}

// Creates a full from clause based on details in the filter
bool CFilter::GetFromClause(CString &strOutClause)
{
	CMapStringToPtr mapJoins;
	CMapPtrToWord mapExcludeParents;
	const CFilterFieldInfo *pffi;
	CString strAns;

	// Get a list of all the join clauses; this automatically and efficiently eliminate duplicates
	mapJoins.RemoveAll();
	if (GetJoinDependencies(mapJoins, mapExcludeParents)) {
		// Build the full FROM clause based on the list of joins
		POSITION pos;
		CString strJoinClause;
		//ALL base tables should be included in the filterfieldinfo, we have no "base join" anymore.
		strAns = "";
		for (pos=mapJoins.GetStartPosition(); pos; ) {
			// Get the filter field info
			mapJoins.GetNextAssoc(pos, strJoinClause, (void *&)pffi);
			if (pffi) {
				// Try to add the clause to the string, and if it fails, we fail
				if (!MoveJoinFromMapToClause(pffi, mapJoins, strAns)) {
					// If we couldn't handle this one, the whole thing fails
					return false;
				}
			}
			// Now begin loop again because items have been removed
			pos=mapJoins.GetStartPosition();
		}
		
		// Success
		strOutClause = strAns;
		return true;
	} else {
		// Failure
		return false;
	}
	
}

void CFilter::RefreshReplaceNames()
{
	m_mapReplaceFieldNames.RemoveAll();
}

CFilterDetail * CFilter::GetFilterDetail(long nItem)
{
	return m_arypFilterDetails[nItem];
}

long CFilter::GetDetailAryItemCount()
{
	return m_nItemCount;
}

// (r.gonet 10/09/2013) - PLID 56236 - Gets the base of the filter.
FilterBasedOnEnum CFilter::GetFilterBase()
{
	return (FilterBasedOnEnum)m_nFilterBase;
}

bool CFilter::ValidateFilter()
{
	CString strFilterString;
	if (GetFilterString(strFilterString)) {
		if (!strFilterString.IsEmpty()) {
			// Got the filter string successfully
			// Now try to get the WHERE and FROM clauses
			CString strLocalWhere;
			CMapStringToString mapUsedFilters;
			if (GetWhereClause(strLocalWhere, mapUsedFilters)) {
				CString strLocalFrom;
				if (GetFromClause(strLocalFrom)) {
					// We successfully got the from and where clauses, so see if they can work to query the data
					try {
						// Validate the query by using an Execute statenent that returns no recordset
						CString strSql;
						strSql.Format("SELECT * FROM "
						"%s WHERE (%s)", strLocalFrom, strLocalWhere);
						ExecuteSqlStd(strSql, NULL, ADODB::adCmdText|ADODB::adExecuteNoRecords);
						
						// We were able to create it successfully, so we have a valid filter
						return true;
					} catch (_com_error e) {
						// Put it into a statement to present to the user
						Log("The filter you entered is invalid (%s %s): SELECT * FROM "
							"%s WHERE (%s)", (LPCTSTR)e.Description(), e.ErrorMessage(), strLocalFrom, strLocalWhere);
						MsgBox(MB_ICONINFORMATION|MB_OK, "The filter you entered is invalid:\n\n%s (%s)", (LPCTSTR)e.Description(), e.ErrorMessage());
						return false;
					} catch (CException *e) {
						// Unexpected error
						HandleException(e, "CFilter::ValidateFilter Error 150:", __LINE__, __FILE__);
						return false;
					}
				} else {
					// Could not obtain FROM clause
					MsgBox(MB_ICONINFORMATION|MB_OK, "CFilter::ValidateFilter Error 250: Could not obtain FROM clause.");
					return false;
				}
			} else {
				// Could not obtain WHERE clause
				MsgBox(MB_ICONINFORMATION|MB_OK, "This filter cannot be saved.  Please confirm that each line has been entered correctly.");
				return false;
			}
		} else {
			// The filter string was empty
			MsgBox(MB_ICONINFORMATION|MB_OK, "You may not save a blank filter.");
			return false;
		}
	} else {
		// GetFilterString failed
		MsgBox(MB_ICONEXCLAMATION|MB_OK, "CFilter::ValidateFilter Error 450: Could not obtain Filter String");
		return false;
	}
}
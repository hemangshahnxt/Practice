//
// (c.haag 2008-10-15 12:52) - PLID 31700 - This class exists to hold functionality
// mutual to the EmrItemAdvTableDlg and EMRItemAdvPopupWnd classes.  All functions are
// merged from the two classes, with minimal alterations where necessary for them to
// work properly.
//
#include "stdafx.h"
#include "EmrItemAdvTableBase.h"
#include "NxExpression.h"
#include "EmrTableEditCalculatedFieldDlg.h"
#include "EmrItemAdvTableDlg.h"
#include "MultiSelectDlg.h"
#include "DontShowDlg.h"
#include <boost/range/mfc.hpp>
#include "EMRTopic.h"
#include "EmrCodingGroupManager.h"
#include "EMN.h"
#include "EMNDetail.h"

#include "NxCache.h"

using namespace NXDATALIST2Lib;
using namespace ADODB;

// (a.walling 2011-08-11 16:43) - PLID 45021 - TableRow.m_pID is now TableRow.m_ID, which is not allocated on the heap.

// (c.haag 2008-10-17 08:19) - PLID 31709 - The first column (regardless of table flippage) that
// is used for actual table data in the datalist
#define FIRST_DL_DATA_ROW			0
#define FIRST_DL_DATA_COLUMN		1

// (z.manning 2010-04-13 16:55) - PLID 38175 - Added a define for this as we now use it in multiple places
#define EMR_TABLE_CELL_GRAY RGB(240,240,240)
//TES 10/12/2012 - PLID 39000 - This red color is used when formulas use fields that have numbers that get truncated before calculation.
#define EMR_TABLE_CELL_RED	RGB(240,125,125)
//TES 10/12/2012 - PLID 39000
#define EMR_UNUSED_NUMBERS_TOOLTIP	"This formula includes values which have numbers that have been ignored.  All characters after the first "\
	"non-numeric digit in any cell will be truncated, so that, for example, \"5 cm - 8 cm\" will be interpreted as \"5.\""

CEmrItemAdvTableBase::CEmrItemAdvTableBase(void) :
	m_nMultipleSelectDropdownSentinelValue(-2),
	m_nAddNewDropdownSentinelValue(-3),
	m_nFirstAvailableDropdownSentinelValue(-4),
	m_rcTableAndCommonButtonsArea(0,0,0,0)
	, m_bTabPressed(false)
{
}

CEmrItemAdvTableBase::~CEmrItemAdvTableBase(void)
{
}

//DRT 7/24/2008 - PLID 30824 - Provide a helpful function to perform the lookup in the map.  Note
//	that due to requirements to maintain strict compatibility with previous implementation, and
//	the fact that the datalist and detail row table array are not really connected, anything that
//	does not match will throw an exception.  You must ensure you are referencing valid indices.
NXDATALIST2Lib::IRowSettingsPtr CEmrItemAdvTableBase::LookupDatalistRowFromTableRowIndex(long nIndex)
{
	LPDISPATCH lpRow = NULL;

	if(!m_mapIndexToDatalistRow.Lookup(nIndex, lpRow)) {
		//failure to find this index!  Uh-oh.
		ASSERT(FALSE);
		AfxThrowNxException("LookupDatalistRowFromTableRowIndex could not find a row object at index %li.", nIndex);
		throw;
	}

	return lpRow;
}

//DRT 7/24/2008 - PLID 30824 - And also a way to go backwards.  Follow the same comments from 
//	LookupDatalistRowFromTableRowIndex.
// (z.manning 2011-02-23 16:24) - PLID 42574 - Added a param for whether or not to throw an exception
long CEmrItemAdvTableBase::LookupTableRowIndexFromDatalistRow(NXDATALIST2Lib::IRowSettingsPtr pRow, BOOL bExceptionIfNotFound /* = TRUE */)
{
	//DRT 7/25/2008 - Speed optimized by adding a reverse map.
	long nIndex = -1;

	if(!m_mapDatalistRowToIndex.Lookup(pRow, nIndex)) {
		// (z.manning 2011-02-23 16:24) - PLID 42574 - Throwing an exception is now optional
		if(bExceptionIfNotFound) {
			//failure to find this index!
			AfxThrowNxException("LookupTableRowIndexFromDatalistRow could not find an index for row object.");
			throw;
		}
		else {
			return -1;
		}
	}

	return nIndex;
}

//DRT 7/24/2008 - PLID 30824 - Clear our all the map data
void CEmrItemAdvTableBase::ClearIndexDatalistMaps()
{
	m_mapIndexToDatalistRow.RemoveAll();

	m_mapDatalistRowToIndex.RemoveAll();
}

void CEmrItemAdvTableBase::CalculateTableSize(CEMNDetail* pDetail, CSize &sz)
{
	if(m_Table) {

		long nWidth = 0, nHeight = 0;

		//get the height of the system font
		long nFontHeight = CalcSysIconTitleFontHeight();
		
		//now pad to account for the size of a datalist row
		// (a.walling 2011-03-30 15:27) - PLID 42692 - This always ends up being slightly off
		long nRowHeight = nFontHeight + (nFontHeight / 2) + 1;

		//sum up the width of each column, plus 1 to account for the line between each column
		for(int i=0;i<m_Table->GetColumnCount();i++) {			
			if(!pDetail->GetSaveTableColumnWidths()) {
				// (c.haag 2006-03-31 17:55) - PLID 19934 - The second parameter needs to be false because
				// we do not want to auto-size the table
				if(i==0)
					nWidth += m_Table->GetColumn(i)->CalcWidthFromData(TRUE, TRUE) + 1;
				else
					// (c.haag 2006-03-31 17:55) - PLID 19934 - The second parameter needs to be false because
					// we do not want to auto-size the table
					// (j.jones 2006-08-01 14:51) - PLID 21729 - Only when not the first column
					nWidth += m_Table->GetColumn(i)->CalcWidthFromData(TRUE, FALSE) + 1;
			}
			else {
				nWidth += m_Table->GetColumn(i)->GetStoredWidth() + 1;
			}
		}
		
		//calculate the border
		long nBorderWidth;
		{
		 CRect rcBorder(0, 0, 100, 100);
		 m_wndTable.CalcWindowRect(&rcBorder, CWnd::adjustOutside);
		 nBorderWidth = rcBorder.Width() - 100;
		}
		nWidth += nBorderWidth;

		nWidth += GetSystemMetrics(SM_CXVSCROLL);
		
		/* JMJ - this function is to calculate the ideal size...
		so we don't need to account for the scrollbar!
		//add extra space for the vertical scrollbar
		nWidth += nRowHeight;
		*/

		//sum up the height of the datalist, which would be the font size times row height,
		//plus an additional row for the column headers

		// (a.walling 2011-03-30 15:27) - PLID 42692 - Take hidden rows into account
		const bool bFilteredView = pDetail->IsCurrentMedicationsTable() || pDetail->IsAllergiesTable();

		long nRowCount = 0;

		if (!bFilteredView) {
			nRowCount = m_Table->GetRowCount();
		} else {
			for (IRowSettingsPtr pVisibleRow = m_Table->GetFirstRow(); pVisibleRow; pVisibleRow = pVisibleRow->GetNextRow()) {
				if (pVisibleRow->GetVisible()) {
					nRowCount++;
				}
			}
			
			// (a.walling 2011-03-30 15:27) - PLID 42692 - Add some space for at least 2 new ones
			nHeight += (2 * nRowHeight);
		}

		if (nRowCount > 0) {
			nHeight += nRowHeight * nRowCount;
		}

		//add space to account for column names
		nHeight += nRowHeight;

		/* JMJ - this function is to calculate the ideal size...
		so we don't need to account for the scrollbar!
		//add extra space for the horizontal scrollbar
		nHeight += nRowHeight;
		*/

		// CAH - I disagree for this release. We need to factor in the horizontal scrollbar height all the
		// time. Although we could have the caller determine the need for a scrollbar based on the
		// horizontal size we return here, that would only result in every caller having to micromanage
		// all datalist and datalist column resizing events (which I don't think is supported right now).
		// At least here we have a behavior that guarantees the scrollbar will not obstruct line
		// items unless the caller does something evil. It may not be aesthetically pleasing at best,
		// but neither is obstructing data.
		nHeight += GetSystemMetrics(SM_CYHSCROLL);

		sz.cx = nWidth;
		sz.cy = nHeight;
	}
	else {
		sz.cx = 0;
		sz.cy = 0;
		ASSERT(FALSE);
	}
}

// (c.haag 2011-03-18) - PLID 42906 - Returns the size of a common list button
CSize CEmrItemAdvTableBase::GetCommonListButtonSize() const
{
	return CSize(60, 24);
}

// (c.haag 2011-03-18) - PLID 42906 - Returns the buffer area around a common list button
CSize CEmrItemAdvTableBase::GetCommonListButtonBufferSize() const
{
	return CSize(6, 6);
}

// (c.haag 2011-03-18) - PLID 42906 - Calculates the space needed for the common buttons region.
// This takes in the size of the table itself as it were before any button region calculations.
CSize CEmrItemAdvTableBase::CalculateCommonButtonRegionSize() const
{
	// For now we'll start with the simplest possible implementation: fixed button size, stack buttons
	// if we run out of horizontal room.
	const CSize szButton = GetCommonListButtonSize(); // Button size
	const CSize szBuffer = GetCommonListButtonBufferSize(); // Number of pixels between each button

	// Figure out how many buttons we need
	const int nButtons = m_apCommonListButtons.GetSize();
	if (0 == nButtons) {
		return CSize(0,0); // No buttons
	}
	// Figure out how many buttons there are per row
	const int nButtonsPerRow = (int)( (float)m_rcTableAndCommonButtonsArea.Width() / (float)(szButton.cx + szBuffer.cx) );
	if (0 == nButtonsPerRow) {
		return CSize(0,0); // Control is too narrow to fit any buttons
	}
	// Figure out how many rows we need
	const int nRows = (int)ceilf( (float)nButtons / (float)nButtonsPerRow );
	// Now we can calculate the region size
	const CSize szOut(m_rcTableAndCommonButtonsArea.Width(), nRows * (szButton.cy + szBuffer.cy));
	// Make sure the table is at least some pixels high
	const int nMinTableHeight = 20;
	if (m_rcTableAndCommonButtonsArea.Height() - szOut.cy < nMinTableHeight) {		
		return CSize(0,0); // Table is too small. Don't show any buttons.
	}
	else {
		return szOut; // Success
	}
}

void CEmrItemAdvTableBase::UpdateLinkedItemList(CEMNDetail* pDetail, BOOL bIsPopupWnd)
{
	if (!bIsPopupWnd) {
		m_strLinkedItemList = pDetail->m_pParentTopic->GetParentEMN()->GetTableItemList();
	}

	//update the linked sql for all linked columns
	// (j.jones 2007-08-10 14:18) - PLID 27046 - removed limitation for 1000 table items
	for (int i=0; i<pDetail->GetColumnCount(); i++) {
		TableColumn tc = pDetail->GetColumn(i);
		long nListType = tc.nType;
		
		if(nListType == LIST_TYPE_LINKED) {
			if(m_Table) {
				//update using the passed-in combo sql
				// (c.haag 2008-10-17 09:58) - PLID 31709 - These functions respect table flipping
				SetComboSource(pDetail, i, m_strLinkedItemList);
				SetEmbeddedComboDropDownMaxHeight(pDetail, i, 400);
			}
		}
	}
}

BOOL CEmrItemAdvTableBase::StateDropdownValueHasMultipleSelections(const _variant_t& v) const
{
	// (c.haag 2008-01-15 17:38) - PLID 17936 - This function return TRUE if the given
	// variant, which is the DATA field of a (X;Y;DATA) state value, represents a
	// dropdown value with multiple selections.

	if (VT_BSTR != v.vt) {
		return FALSE; // Not a string; it cannot be multiple selection data
	} else {
		CString str(VarString(v));
		if (-1 != str.Find(",")) {
			return TRUE; // We found a comma delimiter; since the data can only have numbers,
						// it must be multiple selection data
		} else {
			return FALSE; // No comma delimeter; must be a single number
		}
	}
}

BOOL CEmrItemAdvTableBase::DataListDropdownValueHasMultipleSelections(const _variant_t& v) const
{
	// (c.haag 2008-01-15 17:52) - PLID 17936 - This function return TRUE if the given
	// variant is a value from the dropdown column of a datalist where the value represents 
	// multiple dropdown selections.

	if (VT_I4 != v.vt) {
		return FALSE; // Not a number; not sure how this could ever happen, but it doesn't qualify
	} else {
		if (VarLong(v) <= m_nFirstAvailableDropdownSentinelValue) {
			return TRUE; // The number represents multiple selections
		} else {
			return FALSE; // The number does not represent multiple selections
		}
	}
}

// (c.haag 2008-10-17 09:21) - PLID 31709 - Made it so it only takes in a detail column index (as opposed to a datalist column index)
// (z.manning 2012-04-03 16:20) - PLID 33710 - Added detail row parameter
_variant_t CEmrItemAdvTableBase::FormatValueForDataList(CEMNDetail* pDetail, long nDetailRow, short nDetailCol, const _variant_t& v)
{
	// (c.haag 2008-01-15 17:36) - PLID 17936 - A table state consists of a string made up of an
	// array of semi-colon delimited values in the form (X;Y;DATA). Normally, the data segment
	// plugs right into a datalist cell. However, in some cases it does not; and it must be 
	// formatted to appear correctly in the list. That is the task of this function.
	//
	// For dropcolumn columns, this function will always return a long integer variant.
	//

	// Validation
	if (nDetailCol < 0 || nDetailCol >= pDetail->GetColumnCount()) {
		ASSERT(FALSE);
		ThrowNxException(FormatString("Called FormatValueForDataList with column index %d for a list with %d columns!",
			nDetailCol, pDetail->GetColumnCount()));
	}
	if (nDetailRow < 0 || nDetailRow >= pDetail->GetRowCount()) {
		ASSERT(FALSE);
		ThrowNxException(FormatString("Called FormatDataListValue with row index %d for a list with %d rows!",
			nDetailRow, pDetail->GetRowCount()));
	}

	// v can be an empty or null variant if we're previewing a table from the EMR item edit dialog.
	// In that case, there's nothing to format; just return it as is.
	if (v.vt <= VT_NULL) {
		return v;
	}

	// Get the table column
	TableColumn *ptc = pDetail->GetColumnPtr(nDetailCol);
	TableRow *ptr = pDetail->GetRowPtr(nDetailRow);

	// (z.manning 2012-04-03 16:15) - PLID 33710 - Don't need to do anything special for calculated values
	if(ptr->IsCalculated() || ptr->IsCalculated()) {
		return v;
	}

	// Determine the type of table column. If it's a dropdown column, we need to do
	// special handling. Otherwise, we don't; and we just return the value as is
	switch (ptc->nType)
	{
	case LIST_TYPE_DROPDOWN:

		ASSERT(VT_BSTR == v.vt);

		// If we get here, we're formatting a simple dropdown value. Check whether
		// the data actually represents multiple dropdown selections.
		if (StateDropdownValueHasMultipleSelections(v)) {
			
			// If we get here, the data consists of multiple selections. We need to figure out
			// whether this exact combination of selections is represented by a sentinel value.
			// If it is, we will return that value. If not, we will create a new one and return
			// that.

			// First we need to get the table dropdown source information.
			TableDropdownSourceInfo* pInfo;
			if (!m_mapDropdownSourceInfo.Lookup(ptc->nID, pInfo)) {
				// This should never happen; the map should have been created when the dropdown
				// source was assigned for this column!
				ASSERT(FALSE);
				ThrowNxException("Attempted to reference an invalid column in FormatValueForDataList!");
			}

			// Now get the sentinel ID for this string
			const CString strMultiSelect = VarString(v);
			long nSentinelID;
			void* pval;
			if (!pInfo->mapMultiSelectStringToSentinelID.Lookup(strMultiSelect, pval)) {

				// If we get here, there is no existing sentinel ID for this particular matchup.
				// So, create one now. First, we get a new unique ID.
				POSITION pos = pInfo->mapMultiSelectStringToSentinelID.GetStartPosition();
				nSentinelID = m_nFirstAvailableDropdownSentinelValue;
				while (pos != NULL) {
					CString str;
					pInfo->mapMultiSelectStringToSentinelID.GetNextAssoc(pos, str, pval);
					if ((long)pval <= nSentinelID) {
						nSentinelID = (long)pval - 1;
					}
				}

				// Now assign the multi-select ID string and new sentinel ID to the map.
				pInfo->mapMultiSelectStringToSentinelID.SetAt(strMultiSelect, (void*)nSentinelID);

				// Now we need to add the multi-selection to the embedded dropdown combo. We have to
				// actually get the data for each ID, so this will require some data accesses which we
				// need to optimize out of existence in a future check-in.				
				TableElement teTmp;
				CStringArray astrTmp;
				teTmp.m_pColumn = ptc;

				CString strParse = strMultiSelect;
				while (!strParse.IsEmpty())	{
					long comma = strParse.Find(',');
					
					// If we have no comma, we must have 1 more value in the string
					if(comma != -1) {
						teTmp.m_anDropdownIDs.Add(atol(strParse.Left(comma)));
						strParse = strParse.Right(strParse.GetLength() - strParse.Left(comma+1).GetLength());
					} else {
						teTmp.m_anDropdownIDs.Add(atol(strParse));
						strParse = "";
					}
				}

				// (a.walling 2011-05-31 11:56) - PLID 42448
				CString strValue = teTmp.GetValueAsOutput(pDetail, false, astrTmp);
				// (z.manning 2011-10-11 10:09) - PLID 42061 - This is now an array
				pInfo->aryDelimitedSentinelSqlFragment.Add(TableDropdownItem(nSentinelID, strValue, FALSE));

				// Now we have rebuild the embedded combo source for this column
				CString strComboSql = GetDropdownSource(ptc->nID, -1);
				SetComboSource(pDetail, nDetailCol, strComboSql);
			}
			else {

				// If we get here, we found a sentinel ID that corresponds to the comma-delimited
				// selection list. This is the one we want to return.
				nSentinelID = (long)pval;
			}

			return nSentinelID;

		} else {

			// If we get here, there is just one selection. This means that v is equal to a
			// value in EMRTableDropdownInfoT.ID in string form (i.e. "12345"). We will store
			// this value in long integer form in the datalist cell.
			//
			// On a side note, 0 and -1 are acceptable values which mean 'no selection'
			//
			return atol(VarString(v));
		}
		break;
	default:
		break;
	}

	// No special handling is required for this value, just return it as is
	return v;
}

// (c.haag 2008-10-17 09:21) - PLID 31709 - Made it so it only takes in a detail column index (as opposed to a datalist column index)
// (z.manning 2012-04-03 16:20) - PLID 33710 - Added detail row parameter
_variant_t CEmrItemAdvTableBase::UnformatDataListValue(CEMNDetail* pDetail, long nDetailRow, short nDetailCol, const _variant_t& v) const
{
	// (c.haag 2008-01-15 17:51) - PLID 17936 - A table state consists of string made up of an
	// array of semi-colon delimited values in the form (X;Y;DATA). Normally, the data segment
	// plugs right into a datalist cell. However, in some cases it does not; and when you pull
	// such a value back out of a datalist cell, it needs to be converted into its original
	// form in the table state. That is the task of this function.
	//
	// For dropcolumn columns, this function will always return a string variant.
	//

	// Validation
	if (nDetailCol < 0 || nDetailCol >= pDetail->GetColumnCount()) {
		ASSERT(FALSE);
		ThrowNxException(FormatString("Called UnformatDataListValue with column index %d for a list with %d columns!",
			nDetailCol, pDetail->GetColumnCount()));
	}
	if (nDetailRow < 0 || nDetailRow >= pDetail->GetRowCount()) {
		ASSERT(FALSE);
		ThrowNxException(FormatString("Called UnformatDataListValue with row index %d for a list with %d rows!",
			nDetailRow, pDetail->GetRowCount()));
	}

	// v can be an empty variant if we're previewing a table from the EMR item edit dialog.
	// In that case, there's nothing to format; just return it as is.
	if (VT_EMPTY == v.vt) {
		return v;
	}

	// Get the table column
	TableColumn *ptc = pDetail->GetColumnPtr(nDetailCol);
	TableRow *ptr = pDetail->GetRowPtr(nDetailRow);

	// (z.manning 2012-04-03 16:15) - PLID 33710 - Don't need to do anything special for calculated values
	if(ptr->IsCalculated() || ptr->IsCalculated()) {
		return v;
	}

	// Determine the type of table column. If it's a dropdown column, we need to do
	// special handling. Otherwise, we don't; and we just return the value as is
	CString strTemp;
	switch (ptc->nType)
	{
	case LIST_TYPE_DROPDOWN:

		ASSERT(VT_I4 == v.vt);
		if(ptc->IsAdditionalEmbeddedComboID(VarLong(v),strTemp)) {
			return _variant_t(AsString(v));
		}
		// If we get here, we're unformatting a dropdown value. Check whether
		// the cell data actually represents multiple dropdown selections.
		else if (DataListDropdownValueHasMultipleSelections(v)) {

			// If we get here, the cell represents multiple selections, and v is a long integer sentinel ID which
			// we must map to a comma-delimited string of those selections.

			// First we need to get the table dropdown source information.
			TableDropdownSourceInfo* pInfo;
			if (!m_mapDropdownSourceInfo.Lookup(ptc->nID, pInfo)) {
				// This should never happen; the map should have been created when the dropdown
				// source was assigned for this column!
				ASSERT(FALSE);
				ThrowNxException("Attempted to reference an invalid column in UnformatDataListValue!");
			}

			// Now do a reverse lookup for the sentinel ID
			POSITION pos = pInfo->mapMultiSelectStringToSentinelID.GetStartPosition();
			const long nSentinelID = VarLong(v);
			while (pos != NULL) {
				CString strAns;
				void* pval;
				pInfo->mapMultiSelectStringToSentinelID.GetNextAssoc(pos, strAns, pval);
				if ((const long)pval == nSentinelID) {
					// We found the sentinel ID! Return the result.
					//TES 3/27/2008 - PLID 29432 - VS2008 - Need to explicitly return a _variant_t
					return _variant_t(strAns);
				}
			}

			// This should never happen; it means the sentinel value doesn't exist as far as the column
			// is concerned
			ASSERT(FALSE);
			ThrowNxException(FormatString("Attempted to return a non-existent multi-select string for the sentinel value %d!", VarLong(v)));

		} else {

			// The cell does not represent multiple selections. This means v must be a value
			// in EMRTableDropdownInfoT.ID in long integer form. Return it as a string because
			// thats how we store the value in EMRDetailTableDataT.Data
			//TES 3/27/2008 - PLID 29432 - VS2008 - Need to explicitly return a _variant_t
			return _variant_t(AsString(v));
		}
		break;
	default:
		break;
	}

	// No special handling is required for this value, just return it as is
	return v;
}

void CEmrItemAdvTableBase::ClearDropdownSourceInfoMap()
{
	// (c.haag 2008-01-15 17:41) - PLID 17936 - This function cleans and clears
	// m_mapDropdownSourceInfo
	TableDropdownSourceInfo* pInfo;
	long nColumnID = -1;
	POSITION pos = m_mapDropdownSourceInfo.GetStartPosition();
	while (NULL != pos) {
		m_mapDropdownSourceInfo.GetNextAssoc(pos, nColumnID, pInfo);
		if (NULL != pInfo) {
			 delete pInfo;
		}
	}
	m_mapDropdownSourceInfo.RemoveAll();
}

// (a.walling 2010-04-14 11:17) - PLID 34406 - These need to be accessed outside of the actual table window for windowless updates of calculated values
// so, they are no longer virtual, and they are static, with a nullable EmrItemAdvTableBase* for datalist stuff.
void CEmrItemAdvTableBase::CalculateAndUpdateCellValue(CEMNDetail* pDetail, long nDetailRow, short nDetailCol, CEmrItemAdvTableBase* pAdvTable)
{
	// (z.manning 2011-05-27 11:16) - PLID 42131 - Split this logic into 2 functions
	CString strFormulaResult;
	//TES 10/12/2012 - PLID 39000 - Added bHasUnusedNumbers
	BOOL bHasUnusedNumbers = FALSE;
	if(CalculateCellValue(pDetail, nDetailRow, nDetailCol, pAdvTable, strFormulaResult, bHasUnusedNumbers)) {
		UpdateCalculatedCellValue(strFormulaResult, pDetail, nDetailRow, nDetailCol, pAdvTable, &bHasUnusedNumbers);
	}
}

//TES 10/12/2012 - PLID 39000 - Added an output parameter, bHasUnusedNumbers, which will be set to true if the formula includes a cell in
// its calculations which has any numeric digits after the first non-numeric digits (i.e., "5 cm. - 8 cm.", which would resolve to "5")
BOOL CEmrItemAdvTableBase::CalculateCellValue(CEMNDetail* pDetail, long nDetailRow, short nDetailCol, CEmrItemAdvTableBase* pAdvTable, OUT CString &strResult, OUT BOOL &bHasUnusedNumbers)
{
	// (c.haag 2008-10-16 16:10) - PLID 31735 - Added support for operating on flipped tables
	TableRow* pRow = pDetail->GetRowPtr(nDetailRow);
	TableColumn* pCol = pDetail->GetColumnPtr(nDetailCol);
	long nTableRow, nTableCol;
	DetailPositionToTablePosition(pDetail, nDetailRow, nDetailCol, nTableRow, nTableCol);
	IRowSettingsPtr pDatalistRow;
	// (a.walling 2010-04-14 11:20) - PLID 34406
	if (pAdvTable) {
		pDatalistRow = pAdvTable->LookupDatalistRowFromTableRowIndex(nTableRow);
	}

	// (z.manning, 05/29/2008) - PLID 30155 - We need to update our detail's table element array so
	// that other things that display the detail (e.g. preview pane and merging) know about the value
	// in this cell even though this value isn't actually a part of the detail's state.
	TableElement te;
	te.m_pRow = pRow;
	te.m_pColumn = pCol;

	// (z.manning 2008-06-05 14:56) - PLID 30155 - If both row and column are calculated then rather than choose
	// one just gray out this cell
	// (z.manning 2010-05-03 11:03) - PLID 38175 - Also do nullify the cell if its a label
	if((pRow->IsCalculated() && pCol->IsCalculated()) || te.IsLabel()) {
		// (a.walling 2010-04-14 11:21) - PLID 34406
		if (pDatalistRow) {
			pDatalistRow->PutValue((short)nTableCol, g_cvarNull);
		}
		return FALSE;
	}

	// (j.jones 2013-01-10 17:08) - PLID 54559 - don't calculate if it is a hidden med. or allergy row
	if(pRow->m_bIsCurrentMedOrAllergy && !pRow->m_bIsCurrentMedOrAllergyChecked) {
		return FALSE;
	}

	// (r.gonet 08/03/2012) - PLID 51952 - Get the name of the field
	CString strField;
	if(nTableCol != -1) {
		strField = GetColumnOperand(nTableCol);
	} else if(nTableRow != -1) {
		strField = GetRowOperand(nTableRow);
	}

	CString strFormula;
	BYTE nDecimalPlaces;
	BOOL bShowPlusSign;
	// (z.manning 2008-06-06 09:09) - PLID 30155 - Determine if we should be using a row or column formula
	// (z.manning 2011-06-01 16:32) - PLID 42131 - Also check for a transform formula
	if(pCol->IsCalculated() || (pCol->HasTransformFormula() && !pRow->IsCalculated())) {
		strFormula = pCol->m_strFormula;
		nDecimalPlaces = pCol->m_nDecimalPlaces;
		bShowPlusSign = pCol->ShouldShowPlusSign();
	}
	else if(pRow->IsCalculated()) {
		strFormula = pRow->m_strFormula;
		nDecimalPlaces = pRow->m_nDecimalPlaces;
		bShowPlusSign = pRow->ShouldShowPlusSign();
	}
	else {
		// (z.manning 2008-06-05 15:01) - PLID 30155 - We shouldn't have called this function
		ASSERT(FALSE);
		return FALSE;
	}

	CNxExpression expression(strFormula);
	// (r.gonet 08/03/2012) - PLID 51952 - If this formula references another formula, then it will have to be flattened
	//  so the referenced formula filled fields are eliminated. We do this by substitution,
	//  which is performed by the CNxExpression
	if(pCol->IsCalculated() || pRow->IsCalculated()) {
		// (r.gonet 08/03/2012) - PLID 51952 - Iterate through all fields
		//  and for the ones that have formulas, add the field operand name and
		//  its formula to the expression flattener.
		long nFieldCount = nDetailCol != -1 ? pDetail->GetColumnCount() : pDetail->GetRowCount();
		for(int i = 0; i < nFieldCount; i++) {
			long nRefTableRow = -1, nRefTableCol = -1;
			CString strRefField;
			CString strRefExpression;
			if(nTableCol != -1) {
				// (r.gonet 08/03/2012) - PLID 51952 - This is a column field
				DetailPositionToTablePosition(pDetail, -1, i, nRefTableRow, nRefTableCol);
				strRefField = GetColumnOperand(nRefTableCol);
				TableColumn *pRefCol = pDetail->GetColumnPtr(i);
				if(pRefCol->IsCalculated()) {
					strRefExpression = pRefCol->m_strFormula;
				} else {
					// (r.gonet 08/03/2012) - PLID 51952 - This field isn't formula filled,
					//  so we don't need to add it because CNxExpression allows simple substitutions
					//  of fields for values.
				}
			} else if(nDetailRow != -1) {
				// (r.gonet 08/03/2012) - PLID 51952 - This is a row field
				DetailPositionToTablePosition(pDetail, i, -1, nRefTableRow, nRefTableCol);
				strRefField = GetRowOperand(nRefTableRow);
				TableRow *pRefRow = pDetail->GetRowPtr(i);
				if(pRefRow->IsCalculated()) {
					strRefExpression = pRefRow->m_strFormula;
				}
			}
			if(!strRefExpression.IsEmpty()) {
				expression.AddFormula(strRefField, strRefExpression);
			}
		}
	} else {
		// This is either a raw value field or a transform field, neither of which will need to recurse referenced formulas.
	}

	CStringArray arystrTokens;
	expression.GetTokens(arystrTokens);
	// (z.manning, 05/29/2008) - PLID 30155 - We need to replace any column or row identifiers (which have the
	// format C1, where 1 is this 1-based index of the column) with the actual value that's there now.
	BOOL bAtLeastOneNonBlankOperand = FALSE;
	BOOL bAtLeastOneOperand = FALSE;
	for(int nTokenIndex = 0; nTokenIndex < arystrTokens.GetSize(); nTokenIndex++)
	{
		const CString strToken = arystrTokens.GetAt(nTokenIndex);
		short nColIndex = GetColumnIndexFromOperand(strToken);
		long nRowIndex = GetRowIndexFromOperand(strToken);
		BOOL bBlank = TRUE;
		if(nColIndex != -1) {
			// (z.manning 2008-06-06 09:18) - nColIndex refers to a 1 based index, so subtract one since we
			// want a 0-based index.
			//DRT 7/28/2008 - PLID 30824 - Converted to datalist2
			//TES 10/12/2012 - PLID 39000 - Check whether the calculation includes any cells with unused numbers
			BOOL bThisHasUnusedNumbers = FALSE;
			double dblValue = GetValueForCalculation(pDetail, nDetailRow, nColIndex - 1, bBlank, bThisHasUnusedNumbers);
			if(bThisHasUnusedNumbers) {
				bHasUnusedNumbers = TRUE;
			}
			expression.AddOperandReplacement(strToken, AsString(dblValue));
			bAtLeastOneOperand = TRUE;
		}
		else if(nRowIndex != -1) {
			// (z.manning, 05/29/2008) - There is no column pointer for the row title column, hence 
			// our subtraction of 1 here.
			// (c.haag 2008-10-20 12:54) - PLID 31735 - We now use detail indices instead of table indices;
			// so the previous comment is deprecated
			// (z.manning 2008-06-06 09:18) - nRowIndex refers to a 1 based index, so subtract one since we
			// want a 0-based index.
			//TES 10/12/2012 - PLID 39000 - Check whether the calculation includes any cells with unused numbers
			BOOL bThisHasUnusedNumbers = FALSE;
			double dblValue = GetValueForCalculation(pDetail, nRowIndex - 1, nDetailCol, bBlank, bThisHasUnusedNumbers);
			if(bThisHasUnusedNumbers) {
				bHasUnusedNumbers = TRUE;
			}
			expression.AddOperandReplacement(strToken, AsString(dblValue));
			bAtLeastOneOperand = TRUE;
		}

		if(!bBlank) {
			bAtLeastOneNonBlankOperand = TRUE;
		}
	}
	//TES 10/12/2012 - PLID 39000 - If any cells had unused numbers, remember that.
	if(bHasUnusedNumbers) {
		te.m_bFormulaHasUnusedNumbers = TRUE;
		pDetail->SetTableElement(te, TRUE, FALSE);
	}
	
	// (a.walling 2010-04-14 11:21) - PLID 34406
	if (pDatalistRow && (pRow->IsCalculated() || pCol->IsCalculated())) {
		//TES 10/12/2012 - PLID 39000 - Set the color and tooltip, based on whether the formula included unused numbers
		pDatalistRow->PutCellBackColor((short)nTableCol, te.m_bFormulaHasUnusedNumbers?EMR_TABLE_CELL_RED:EMR_TABLE_CELL_GRAY);
		pDatalistRow->PutCellToolTipOverride((short)nTableCol, te.m_bFormulaHasUnusedNumbers?EMR_UNUSED_NUMBERS_TOOLTIP:"");
	}

	if(bAtLeastOneOperand && !bAtLeastOneNonBlankOperand) {
		// (z.manning 2008-06-30 09:22) - PLID 30155 - We have at least one row or column operand
		// but none that are filled out, so we do not want to evaluate this formula, so just set
		// this cell to a blank value.
		strResult = "";
		return TRUE;
	}

	double dblResult = expression.Evaluate();
	// (z.manning, 06/02/2008) - "Round" our result.
	CString strFormat = FormatString(".%lif", nDecimalPlaces);
	strResult = FormatString("%" + strFormat, dblResult);
	// (z.manning 2008-06-11 12:47) - Originally I was truncating trailing zeros, but EMR team says
	// to not do that.
	//if(strResult.Find('.') != -1) {
	//	strResult.TrimRight('0');
	//	strResult.TrimRight('.');
	//}

	// (z.manning 2011-05-26 17:46) - PLID 42131 - Do we want to show a plus sign?
	if(bShowPlusSign && LooseCompareDouble(dblResult, 0, 0.000000001) == 1) {
		strResult = '+' + strResult;
	}

	return TRUE;
}

// (z.manning 2011-05-31 14:50) - PLID 42131 - This returns true if something actually changed.
//TES 10/12/2012 - PLID 39000 - Added an optional parameter indicating whether the formula included cells with unused numbers
BOOL CEmrItemAdvTableBase::UpdateCalculatedCellValue(LPCTSTR strValue, CEMNDetail* pDetail, long nDetailRow, short nDetailCol, CEmrItemAdvTableBase* pAdvTable, OPTIONAL IN BOOL *pbHasUnusedNumbers /*= NULL*/)
{
	// (c.haag 2008-10-16 16:10) - PLID 31735 - Added support for operating on flipped tables
	TableRow* pRow = pDetail->GetRowPtr(nDetailRow);
	TableColumn* pCol = pDetail->GetColumnPtr(nDetailCol);
	long nTableRow, nTableCol;
	DetailPositionToTablePosition(pDetail, nDetailRow, nDetailCol, nTableRow, nTableCol);

	// (j.jones 2013-01-10 14:19) - PLID 54559 - if the cell is empty, and the new value is also empty, change nothing
	{
		TableElement te = GetTableElementByTablePosition(pDetail, nTableRow, nTableCol);
		CString strCurValue = te.GetValueAsString();
		CString strNewValue = strValue;
		if(strCurValue.IsEmpty() && strNewValue.IsEmpty()) {
			//no point in changing an empty value to remain an empty value
			return FALSE;
		}
	}

	IRowSettingsPtr pDatalistRow = NULL;
	// (a.walling 2010-04-14 11:20) - PLID 34406
	if (pAdvTable) {
		pDatalistRow = pAdvTable->LookupDatalistRowFromTableRowIndex(nTableRow);
	}

	// (z.manning, 05/29/2008) - PLID 30155 - We need to update our detail's table element array so
	// that other things that display the detail (e.g. preview pane and merging) know about the value
	// in this cell even though this value isn't actually a part of the detail's state.
	TableElement te;
	te.m_pRow = pRow;
	te.m_pColumn = pCol;

	te.LoadValueFromString(strValue, NULL);
	//TES 10/12/2012 - PLID 39000 - If we were given a value for bHasUnusedNumbers, remember it
	if(pbHasUnusedNumbers) {
		te.m_bFormulaHasUnusedNumbers = *pbHasUnusedNumbers;
	}
	
	// (a.walling 2010-04-14 11:21) - PLID 34406
	if (pDatalistRow && (pRow->IsCalculated() || pCol->IsCalculated())) {
		if(pbHasUnusedNumbers) {
			pDatalistRow->PutCellBackColor((short)nTableCol, te.m_bFormulaHasUnusedNumbers?EMR_TABLE_CELL_RED:EMR_TABLE_CELL_GRAY);
			pDatalistRow->PutCellToolTipOverride((short)nTableCol, te.m_bFormulaHasUnusedNumbers?EMR_UNUSED_NUMBERS_TOOLTIP:"");
		}
	}

	// (a.walling 2010-04-14 13:53) - PLID 34406 - We don't need to recreate the state every time we update, just at the end.
	// This also means that whomever calls CalculateAndUpdateCellValue is responsible for calling RecreateStateFromContent.
	BOOL bReturn = pDetail->SetTableElement(te, TRUE, FALSE);

	switch(pCol->nType)
	{
		case LIST_TYPE_LINKED:
		case LIST_TYPE_CHECKBOX:
			// (z.manning 2008-06-11 12:56) - Not supported
			// (a.walling 2010-04-14 11:23) - PLID 34406
			if (pDatalistRow) {
				pDatalistRow->PutValue((short)nTableCol, g_cvarNull);
			}
			break;

		case LIST_TYPE_TEXT:
			// (a.walling 2010-04-14 11:23) - PLID 34406
			if (pDatalistRow) {
				pDatalistRow->PutValue((short)nTableCol, _bstr_t(strValue));
			}
			break;

		case LIST_TYPE_DROPDOWN:
			
			// (a.walling 2010-06-03 11:51) - PLID 34406 - Do not attempt any of this unless we have valid table and row pointers.
			// Thanks to Zack for finding and fixing this in the patch; I am basically just copying the same thing over here.
			if(pAdvTable != NULL && pDatalistRow != NULL)
			{
				long nComboID = pCol->GetEmbeddedComboIDFor(strValue);
				if(nComboID == 0) {
					// (z.manning 2008-06-12 08:28) - PLID 30155 - If the result of our calcuation is not an
					// avialable field in our embedded combo then we need to add it.
					nComboID = pCol->AddEmbeddedComboValue(strValue, FALSE);
					pAdvTable->SetComboSource(pDetail, nDetailCol, pAdvTable->GetDropdownSource(pCol->nID, -1));
				}
				pDatalistRow->PutValue((short)nTableCol, nComboID);
			}
			break;
	}

	return bReturn;
}

// (a.walling 2010-04-14 11:17) - PLID 34406 - These need to be accessed outside of the actual table window for windowless updates of calculated values
// so, they are no longer virtual, and they are static, with a nullable EmrItemAdvTableBase* for datalist stuff.
void CEmrItemAdvTableBase::UpdateCalculatedFields(CEMNDetail* pDetail, CEmrItemAdvTableBase* pAdvTable)
{
	// (a.walling 2010-04-14 13:53) - PLID 34406 - We don't need to recreate the state every time we update, just at the end.
	// (a.walling 2011-03-30 14:23) - PLID 42962 - This flag simply tells us if we have calculated data or not.
	bool bHasCalculatedData = false;

	// (c.haag 2008-10-16 16:10) - PLID 31735 - Rewritten to support table flipping
	const int nDetailRows = pDetail->GetRowCount();
	const int nDetailColumns = pDetail->GetColumnCount();

	DWORD dwStart = GetTickCount();

	for (int iRow=0; iRow < nDetailRows; iRow++) {
		TableRow *pTableRow = pDetail->GetRowPtr(iRow);
		for (short iCol=0; iCol < nDetailColumns; iCol++) {
			TableColumn *pTableColumn = pDetail->GetColumnPtr(iCol);
			// (j.jones 2013-01-10 17:08) - PLID 54559 - don't calculate if it is a hidden med. or allergy row
			if((pTableColumn->IsCalculated() || pTableRow->IsCalculated()) && (!pTableRow->m_bIsCurrentMedOrAllergy || pTableRow->m_bIsCurrentMedOrAllergyChecked)) {
				// (a.walling 2010-04-14 13:53) - PLID 34406 - We don't need to recreate the state every time we update, just at the end.
				bHasCalculatedData = true;
				CalculateAndUpdateCellValue(pDetail, iRow, iCol, pAdvTable);
			}
		}
	}

	// (j.jones 2013-01-10 15:49) - PLID 54559 - If updating formulas in this table took more than 250ms,
	// trace it. If it took more than 1 second, log it.
	DWORD dwFinish = GetTickCount();
	DWORD dwTotal = dwFinish - dwStart;
	if(dwTotal > 250) {
		CString strLogTime;
		strLogTime.Format("Formula calculations on table %s took %li ms. (EMRInfoID %li)", pDetail->GetMergeFieldName(FALSE), dwTotal, pDetail->m_nEMRInfoID);
		TRACE(strLogTime);
		if(dwTotal > 1000) {
			LogDetail(strLogTime);
		}
	}

	if (bHasCalculatedData) {
		
		// (a.walling 2011-03-30 14:23) - PLID 42962 - Are we filtering the view? Only if we have a pAdvTable pointer.
		const bool bFilteredView = pAdvTable && (pDetail->IsCurrentMedicationsTable() || pDetail->IsAllergiesTable());

		if (bFilteredView) {
			const int nDetailRows = pDetail->GetRowCount();
			const int nDetailColumns = pDetail->GetColumnCount();
			// If we get here, we have to refill either every cell in the datalist 
			// with values, or only the ones in grouped columns.
			CMap<__int64, __int64, int, int> mapTable;
			pDetail->PopulateTableElementMap(mapTable);

			for (int iRow=0; iRow < nDetailRows; iRow++) {
				TableRow* pRow = pDetail->GetRowPtr(iRow);
				// (z.manning 2010-02-18 11:09) - PLID 37427 - Row ID is now a struct
				TableRowID *pRowID = &pRow->m_ID;

				// (a.walling 2011-03-21 12:34) - PLID 42962 - If the row has any non-empty data, we will show it.
				bool bRowHasData = false;

				for (int iCol=0; iCol < nDetailColumns && !bRowHasData; iCol++) {
					TableColumn* pCol = pDetail->GetColumnPtr(iCol);

					// (a.walling 2011-03-21 12:34) - PLID 42962
					const long nColumnID = pCol->nID;
					__int64 nKey = pDetail->GetTableElementKey(pRow, nColumnID);
					TableElement te;
					int nTableIndex;

					if (mapTable.Lookup(nKey, nTableIndex)) {
						pDetail->GetTableElementByIndex(nTableIndex, te);
						//(e.lally 2011-12-08) PLID 46471 - We're no longer looking for any non-empty value
						//	Since this is a current med or allergy table, we're only concerned with the official usage col checkbox and if it is marked
						if (te.m_bChecked && te.m_pColumn->m_bIsCurrentMedOrAllergyUsageCol) {
							bRowHasData = true;
						}
					}
				} // for (int iCol=0; iCol < nDetailColumns; iCol++) {

				// (a.walling 2011-03-21 12:34) - PLID 42962 - Remember whether to show or hide this row
				IRowSettingsPtr pThisRow = pAdvTable->LookupDatalistRowFromTableRowIndex(FIRST_DL_DATA_ROW + iRow);

				if (bRowHasData) {
					pThisRow->PutVisible(VARIANT_TRUE);
				} else {
					pThisRow->PutVisible(VARIANT_FALSE);
				}
			}
		}

		pDetail->RecreateStateFromContent();
	}
}

// (j.jones 2008-06-05 09:14) - PLID 18529 - TryAdvanceNextDropdownList is called when
// any non-text cell is edited, and tries to auto-dropdown the next dropdown or linked
// list, only if the next column is that type, and the preference is on
//DRT 7/24/2008 - PLID 30824 - Converted to use NxDataList2
void CEmrItemAdvTableBase::TryAdvanceNextDropdownList(CWnd* pWnd, CEMNDetail* pDetail, LPDISPATCH lpRow, short nTableCol)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			//This is new with PLID 30824, previously it never checked for bad rows
			return;
		}

		//if any logic in this function changes, you must also change
		//the same function in CEMRItemAdvPopupWnd

		//check the preference
		if(GetRemotePropertyInt("EmnTableDropdownAutoAdvance", 1, 0, GetCurrentUserName(), true) == 1) {

			IRowSettingsPtr pRowToCheck = NULL;
			short nTableColToCheck = -1;

			// (z.manning 2010-04-12 09:36) - PLID 38129 - We now go top to bottom then left to right on 
			// flipped tables.
			if(pDetail != NULL && pDetail->m_bTableRowsAsFields)
			{
				if(m_Table->GetLastRow() == pRow) {
					//we're at the end of the column, see if there is a next column
					nTableColToCheck = nTableCol + 1;
					if(nTableColToCheck >= m_Table->GetColumnCount()) {
						//there is no next colum, so leave
						return;
					}
					else {
						pRowToCheck = m_Table->GetFirstRow();
					}
				}
				else {
					//we're not at the end of the row, so try the next row
					pRowToCheck = pRow->GetNextRow();
					nTableColToCheck = nTableCol;
				}
			}
			else
			{
				// (a.walling 2011-03-23 18:08) - PLID 42962 - Skip hidden rows (including this row)
				if (!pRow->GetVisible() || (m_Table->GetColumnCount() - 1 <= nTableCol)) {
					//we're at the end of the row, see if there is a next row
					pRowToCheck = pRow->GetNextRow();
					// (a.walling 2011-03-23 18:08) - PLID 42962 - Skip hidden rows
					while (pRowToCheck && !pRowToCheck->GetVisible()) {
						pRowToCheck = pRowToCheck->GetNextRow();
					}
					if(pRowToCheck == NULL) {
						//there is no next row, so leave
						return;
					}
					else {
						//pRowToCheck is now the row we want to move to
						nTableColToCheck = 1;  //1, not 0, don't forget the first column is always a label
					}
				}
				else {
					//we're not at the end of the row, so try the next column
					pRowToCheck = pRow;
					nTableColToCheck = nTableCol + 1;
				}
			}

			//now see if this cell is a dropdown list

			// (c.haag 2008-10-20 12:19) - PLID 31709 - Changed to support table flipping
			long nTableRowIndex = LookupTableRowIndexFromDatalistRow(pRowToCheck);
			TableElement te = GetTableElementByTablePosition(pDetail, nTableRowIndex, nTableColToCheck);

			//is it a dropdown / linked list, and if so is it filled in?				
			// (z.manning 2011-04-14 11:34) - PLID 42618 - Also support advancing to text type columns that have dropdown elements.
			if((te.m_pColumn->nType == LIST_TYPE_DROPDOWN && !te.IsDropdownValueSelected())
				|| (te.m_pColumn->nType == LIST_TYPE_LINKED && !te.m_pLinkedDetail)
				|| (te.m_pColumn->nType == LIST_TYPE_TEXT && te.m_pColumn->m_bHasActiveDropdownElements)
				)
			{
				ASSERT(pRowToCheck->GetVisible());

				//it's empty, let's edit it
				pWnd->PostMessage(NXM_START_EDITING_EMR_TABLE, (WPARAM)pRowToCheck.Detach(), nTableColToCheck);
			}
			else {
				//not a dropdown list, so leave
				return;
			}
		} // if(GetRemotePropertyInt("EmnTableDropdownAutoAdvance", 1, 0, GetCurrentUserName(), true) == 1) {
	} 
	NxCatchAll("Error in CEmrItemAdvTableBase::TryAdvanceNextDropdownList");
}

// (j.jones 2008-06-05 10:07) - PLID 18529 - TryAdvanceNextDropdownList will potentially send
// NXM_START_EDITING_EMR_TABLE which will fire this function, which will start editing the
// row and column in question
//DRT 7/24/2008 - PLID 30824 - Converted to use NxDataList2
//	Also updating comments.  This exists as a PostMessage handler because you cannot call StartEditing on 1 cell
//	from within the OnEditingFinished of another cell, or the datalist doesn't allow you to ever really commit
//	the 2nd edit.
// (a.walling 2008-10-02 09:16) - PLID 31564 - VS2008 - Message handlers must fit the LRESULT fn(WPARAM, LPARAM) format
void CEmrItemAdvTableBase::StartEditingEMRTable(WPARAM wParam, LPARAM lParam)
{
	LPDISPATCH lpRow = (LPDISPATCH)wParam;
	short nCol = (short)lParam;
	IRowSettingsPtr pRow(lpRow);

	//the caller should have checked this already, but confirm these
	//coordinates reference a valid cell
	if(m_Table == NULL
		|| nCol < 0
		|| m_Table->GetColumnCount() - 1 < nCol
		|| lpRow == NULL
		|| !pRow->GetVisible()) {

		ASSERT(FALSE);
		return;
	}

	m_Table->CurSel = pRow;
	m_Table->StartEditing(pRow, nCol);
	return;
}

EColumnFieldType CEmrItemAdvTableBase::GetColumnFieldType(long nListType) const
{
	switch(nListType) {
		case LIST_TYPE_CHECKBOX:	//checkbox
			return cftBoolCheckbox;
			break;

		case LIST_TYPE_DROPDOWN:	//dropdown
		case LIST_TYPE_LINKED:		//linked
			return cftComboSimple;
			break;

		// (j.gruber 2007-02-07 10:59) - PLID 24635 - Make text fields word wrap
		case LIST_TYPE_TEXT:	//text
		default:
			return cftTextWordWrap;
			break;

	}
}

VARENUM CEmrItemAdvTableBase::GetColumnDataType(long nListType) const
{
	switch(nListType) {
		case LIST_TYPE_CHECKBOX:	//checkbox
			return VT_BOOL;
			break;

		case LIST_TYPE_DROPDOWN:	//dropdown
			return VT_I4;
			break;

		case LIST_TYPE_TEXT:	//text
		case LIST_TYPE_LINKED:		//linked
		default:
			return VT_BSTR;
			break;

	}
}

// (c.haag 2008-10-17 11:46) - PLID 31700 - Reflects the table's state
void CEmrItemAdvTableBase::ReflectTableState(CWnd* pWnd, CEMNDetail* pDetail, BOOL bIsPopupWnd)
{
	try {
		if(NULL != m_Table) {
			SetTableRedraw(FALSE);

			//now reflect the column lists
			UpdateLinkedItemList(pDetail, bIsPopupWnd);
			
			CString strState = AsString(pDetail->GetState());
			
			// (a.walling 2011-03-21 12:34) - PLID 42962 - Filter if we are a current medication or allergies table
			bool bFilteredView = pDetail->IsCurrentMedicationsTable() || pDetail->IsAllergiesTable();

			CArray<IRowSettingsPtr> rowsToShow;
			CArray<IRowSettingsPtr> rowsToHide;

			BOOL bGenerateState = TRUE;
			BOOL bStateIsNew = FALSE;

			if(strState.IsEmpty()) {
				pDetail->RecreateStateFromContent();
				strState = AsString(pDetail->GetState());
				bStateIsNew = TRUE;
				bGenerateState = FALSE;
			}

			long nEditingFinishedRowIndex = LookupTableRowIndexFromDatalistRow(m_pEditingFinishedRow, FALSE);

			// (c.haag 2007-08-20 15:12) - PLID 27127 - By the time we are done with
			// the following branch of code, the datalist must be fully up to date with
			// the data as defined by the table element array. There are three possible
			// values for m_ReflectCurrentStateDLHint here:
			// 
			// eRCS_FullDatalistUpdate - Repopulate every cell in the whole datalist
			// eRCS_GroupedColumnUpdate - Repopulate every grouped column in the datalist
			// eRCS_SingleCellUpdate - Repopulate one cell in the datalist
			//
			// (c.haag 2008-10-17 11:23) - PLID 31709 - Updated for table flipping
			const int nDetailRows = pDetail->GetRowCount();
			const int nDetailColumns = pDetail->GetColumnCount();
			// (z.manning 2011-02-23 16:28) - PLID 42574 - If the hint is eRCS_SingleCellUpdate it's possible that
			// the row we want to update no longer exists because content was reloaded somewhere in between the editing
			// finished handler and here. So if we could not find the row index of the editing finished row, then do a
			// full update to be safe.
			if (eRCS_FullDatalistUpdate == m_ReflectCurrentStateDLHint ||
				eRCS_GroupedColumnUpdate == m_ReflectCurrentStateDLHint ||
				(eRCS_SingleCellUpdate == m_ReflectCurrentStateDLHint && nEditingFinishedRowIndex == -1))
			{
				// If we get here, we have to refill either every cell in the datalist 
				// with values, or only the ones in grouped columns.
				CMap<__int64, __int64, int, int> mapTable;
				pDetail->PopulateTableElementMap(mapTable);
				BOOL bExcludeUngroupedColumns = (eRCS_GroupedColumnUpdate == m_ReflectCurrentStateDLHint) ? TRUE : FALSE;

				for (int iRow=0; iRow < nDetailRows; iRow++) {
					TableRow* pRow = pDetail->GetRowPtr(iRow);
					// (z.manning 2010-02-18 11:09) - PLID 37427 - Row ID is now a struct
					TableRowID *pRowID = &pRow->m_ID;

					// (a.walling 2011-03-21 12:34) - PLID 42962 - If the row has any non-empty data, we will show it.
					bool bRowHasData = false;

					for (int iCol=0; iCol < nDetailColumns; iCol++) {
						TableColumn* pCol = pDetail->GetColumnPtr(iCol);

						// (a.walling 2011-03-21 12:34) - PLID 42962
						if (bFilteredView || !bExcludeUngroupedColumns || (bExcludeUngroupedColumns && pCol->bIsGrouped))
						{							
							const long nColumnID = pCol->nID;
							__int64 nKey = pDetail->GetTableElementKey(pRow, nColumnID);
							TableElement te;
							int nTableIndex;

							if (mapTable.Lookup(nKey, nTableIndex)) {
								pDetail->GetTableElementByIndex(nTableIndex, te);
							} else {
								te.m_pColumn = pCol;
								te.m_pRow = pRow;
							}
							if(te.IsLabel()) {
								// (z.manning 2010-04-14 11:40) - PLID 38175 - Nullify all label cells
								SetTableValue(pDetail, iRow, iCol, g_cvarNull, EMR_TABLE_CELL_GRAY);
								bRowHasData = true;
							}
							else {
								//TES 10/12/2012 - PLID 39000 - If this is a calculated formula whose calculation included unused numbers,
								// set the background to red
								OLE_COLOR nCellColor = te.IsCalculated() ? (te.m_bFormulaHasUnusedNumbers?EMR_TABLE_CELL_RED:EMR_TABLE_CELL_GRAY) : RGB(255,255,255);
								SetTableValue(pDetail, iRow, iCol, FormatValueForDataList(pDetail,iRow,iCol,te.GetValueAsVariant()), nCellColor, te.m_bFormulaHasUnusedNumbers);
								
								// (a.walling 2011-03-21 12:34) - PLID 42962 - If the row has any non-empty data, we will show it.
								//(e.lally 2011-12-08) PLID 46471 - We're no longer looking for any non-empty value
								//	Since this is a current med or allergy table, we're only concerned with the official usage col checkbox and if it is marked
								if (bFilteredView) {
									if (!bRowHasData && te.m_bChecked && te.m_pColumn->m_bIsCurrentMedOrAllergyUsageCol) {
										bRowHasData = true;
									}
								}
							}
						} // if (!bExcludeUngroupedColumns || (bExcludeUngroupedColumns && pCol->bIsGrouped)) {	
					} // for (int iCol=0; iCol < nDetailColumns; iCol++) {

					
					// (a.walling 2011-03-21 12:34) - PLID 42962 - Remember whether to show or hide this row
					if (bFilteredView) {
						IRowSettingsPtr pThisRow = LookupDatalistRowFromTableRowIndex(FIRST_DL_DATA_ROW + iRow);

						if (bRowHasData) {
							rowsToShow.Add(pThisRow);
						} else {
							rowsToHide.Add(pThisRow);
						}
					}
				} // for (int iRow=0; iRow < nDetailRows; iRow++) {
			}
			else if (eRCS_SingleCellUpdate == m_ReflectCurrentStateDLHint) {
				// If we get here, we came from OnEditingFinishedTable, and we only
				// need to update the cell the user edited. I don't think this code
				// actually has any net effect other than to ensure the value of the
				// changing cell is precisely consisent with that of the table element.

				// (c.haag 2008-10-20 10:42) - PLID 31709 - Support table flipping
				long iDetailRow, iDetailCol;
				TablePositionToDetailPosition(pDetail, nEditingFinishedRowIndex, m_nEditingFinishedCol, iDetailRow, iDetailCol);
				TableRow* pRow = pDetail->GetRowPtr(iDetailRow);

				{
					TableColumn* pCol = pDetail->GetColumnPtr(iDetailCol);

					// (a.walling 2011-08-24 12:35) - PLID 45171 - We only need to look for existing table elements
					TableElement te;
					pDetail->GetExistingTableElement(&pRow->m_ID, pCol->nID, te);
					m_pEditingFinishedRow->PutValue(m_nEditingFinishedCol,FormatValueForDataList(pDetail,iDetailRow,(short)iDetailCol,te.GetValueAsVariant()));
				}

				
				// (a.walling 2011-03-21 12:34) - PLID 42962 - If the row has any non-empty data, we will show it.
				if (bFilteredView) {
					bool bRowHasData = false;

					for (int iCheckCol = 0; !bRowHasData && (iCheckCol < nDetailColumns); iCheckCol++) {
						TableColumn* pCheckCol = pDetail->GetColumnPtr(iCheckCol);

						if (pCheckCol) {
							// (a.walling 2011-08-24 12:35) - PLID 45171 - We only need to look for existing table elements
							TableElement te;
							if (pDetail->GetExistingTableElement(&pRow->m_ID, pCheckCol->nID, te)) {
								//(e.lally 2011-12-08) PLID 46471 - We're no longer looking for any non-empty value
								//	Since this is a current med or allergy table, we're only concerned with the official usage col checkbox and if it is marked
								if (te.m_bChecked && te.m_pColumn->m_bIsCurrentMedOrAllergyUsageCol) {
									bRowHasData = true;
								}
							}
						}
					}
					
					IRowSettingsPtr pThisRow = LookupDatalistRowFromTableRowIndex(FIRST_DL_DATA_ROW + iDetailRow);

					if (bRowHasData) {
						rowsToShow.Add(pThisRow);
					} else {
						rowsToHide.Add(pThisRow);
					}
				}
			}

			// (a.walling 2011-03-21 12:34) - PLID 42962 - Show or hide all appropriate rows
			int r;
			
			for (r = 0; r < rowsToHide.GetSize(); r++) {
				IRowSettingsPtr pThisRow = rowsToHide[r];

				if (pThisRow) {
					pThisRow->PutVisible(VARIANT_FALSE);
				}
			}

			for (r = 0; r < rowsToShow.GetSize(); r++) {
				IRowSettingsPtr pThisRow = rowsToShow[r];

				if (pThisRow) {
					pThisRow->PutVisible(VARIANT_TRUE);
				}
			}
			
			// (a.walling 2010-04-14 11:19) - PLID 34406
			// (z.manning 2012-03-28 11:02) - PLID 33710 - Calculated cells are now saved to data so we now only update them
			// when the table is changed.
			//UpdateCalculatedFields(pDetail, this);

			if (!pDetail->GetSaveTableColumnWidths()) {
				// (c.haag 2008-10-20 12:57) - PLID 31709 - If the table is flipped, rows could potentially have
				// word wrap styles. If any does, we need to do CalcWidthFromData for all columns
				int i;
				if (pDetail->m_bTableRowsAsFields) {
					IRowSettingsPtr pRow = m_Table->GetFirstRow();
					const long nTableColumns = GetTableColumnCount();
					while (NULL != pRow) {
						IFormatSettingsPtr pFormatRow = pRow->FormatOverride;
						if (NULL != pFormatRow && pFormatRow->GetFieldType() == cftTextWordWrap) {
							for (i=0; i < nTableColumns; i++) {
								m_Table->GetColumn(i)->CalcWidthFromData(TRUE, TRUE);
							}
							break;
						}
						pRow = pRow->GetNextRow();
					} // while (NULL != pRow) {
				}
				else {
					const long nTableColumns = GetTableColumnCount();
					for (i=0; i < nTableColumns; i++) {
						// (j.gruber 2007-02-07 16:34) - PLID 24635 - make text multiline
						if(m_Table->GetColumn(i)->GetFieldType() == cftTextWordWrap) {
							m_Table->GetColumn(i)->CalcWidthFromData(TRUE, TRUE);
						}
					}
				}
			}

			//re-generate the varstate from this table because
			//- it ay be blank, and needs to be created
			//- we may have reloaded the table with less or more columns, and the varstate needs to reflect this
			if(bGenerateState) {
				CString strNewState = GenerateNewVarState();
				bStateIsNew = AsString(pDetail->GetState()) != strNewState;
				pDetail->SetState(_bstr_t(strNewState));
			}

			//load the sentence types so we can save time for the narratives and tooltips
			if(bStateIsNew) {
				// (z.manning 2010-02-26 08:34) - PLID 37412 - Moved this code into its own function
				pDetail->UpdateTableDataOutput();
			}

			//SetTableRedraw(TRUE);

			CRect rcWindow;
			pWnd->GetWindowRect(&rcWindow);
			// (a.walling 2010-06-21 15:07) - PLID 38779 - This is never called with not bCalcOnly
			RepositionTableControls(CSize(rcWindow.Width(), rcWindow.Height()));

			/*m.hancock - 4/4/2006 - PLID 19991 - Flickering on a table when changing values in the cells
			SetRedraw() is disabled and enabled again by RepositionControls() and this SetRedraw(TRUE) may not 
			be necessary, but to be complete, it's being moved here.  If in future, if we do any drawing between 
			ReflectCurrentState and RepositionControls, we should use another approach such as reference counting.*/
			SetTableRedraw(TRUE);
		}
	} NxCatchAll("Error displaying data.");
}

// (c.haag 2006-04-03 09:20) - PLID 19934 - This function was rewritten; the
// behavior of the datalist is actually considered here.
// (r.gonet 02/14/2013) - PLID 40017 - Added a flag to tell the function that the table being updated is in a popup.
void CEmrItemAdvTableBase::UpdateColumnStyle(CEMNDetail* pDetail, BOOL bIsPoppedUp)
{
	int nNumColumns = m_Table->GetColumnCount();
	for (int i = 0; i < nNumColumns; i++) {
		long nStyle = m_Table->GetColumn(i)->ColumnStyle;
		if (!pDetail->GetSaveTableColumnWidths()) {
			if (!(nStyle & csWidthAuto)) {
				m_Table->GetColumn(i)->StoredWidth = -1;
				if(i == 0)
					//the first column should size to the width of the data
					m_Table->GetColumn(i)->ColumnStyle = nStyle | csWidthData;
				else
					m_Table->GetColumn(i)->ColumnStyle = nStyle | csWidthAuto;
			}
		}
		else {
			//if the style is Auto or Data, remove those properties
			if (nStyle & csWidthAuto) {
				long nWidth = m_Table->GetColumn(i)->StoredWidth;
				m_Table->GetColumn(i)->ColumnStyle = nStyle & (~csWidthAuto);
				m_Table->GetColumn(i)->StoredWidth = nWidth;
			}
			else if (nStyle & csWidthData) {
				long nWidth = m_Table->GetColumn(i)->StoredWidth;
				m_Table->GetColumn(i)->ColumnStyle = nStyle & (~csWidthData);
				m_Table->GetColumn(i)->StoredWidth = nWidth;
			}

			// (j.jones 2006-08-01 10:58) - PLID 21704 - also store this width in member variables
			CString strColumnTitle;
			strColumnTitle.Format("%s", AsString(m_Table->GetColumn(i)->GetColumnTitle()));
			strColumnTitle.TrimLeft();
			//DRT 7/10/2007 - PLID 24105 - Since we have changed the stored width of the column, we need to 
			//	save that new width in our tracking member.
			// (c.haag 2008-06-26 11:15) - PLID 27968 - Special case for the 0th column
			if (0 == i) {
				// (r.gonet 02/14/2013) - PLID 40017 - If we are popped up, save to the popped up column width instead of the normal one.
				if(!bIsPoppedUp) {
					pDetail->GetColumnWidths()->SetFirstColumnWidth(m_Table->GetColumn(0)->StoredWidth);
				} else {
					pDetail->GetColumnWidths()->SetFirstPopupColumnWidth(m_Table->GetColumn(0)->StoredWidth);
				}
			} else {
				// (r.gonet 02/14/2013) - PLID 40017 - If we are popped up, save to the popped up column width instead of the normal one.
				if(!bIsPoppedUp) {
					pDetail->GetColumnWidths()->SetWidthByName(strColumnTitle, m_Table->GetColumn(i)->StoredWidth);
				} else {
					pDetail->GetColumnWidths()->SetPopupWidthByName(strColumnTitle, m_Table->GetColumn(i)->StoredWidth);
				}
			}
		}
	}
}

// (c.haag 2008-10-20 11:32) - PLID 31709 - Converts a detail position to a position in the table
void CEmrItemAdvTableBase::DetailPositionToTablePosition(CEMNDetail* pDetail, long nDetailRow, long nDetailCol, long& nTableRow, long& nTableCol)
{
	if (pDetail->m_bTableRowsAsFields) {
		nTableRow = nDetailCol + FIRST_DL_DATA_ROW;
		nTableCol = (short)(nDetailRow + FIRST_DL_DATA_COLUMN);
	} else {
		nTableRow = (short)(nDetailRow + FIRST_DL_DATA_ROW);
		nTableCol = nDetailCol + FIRST_DL_DATA_COLUMN;
	}
}

// (c.haag 2008-10-20 11:32) - PLID 31709 - Converts a table position to a position in the detail
void CEmrItemAdvTableBase::TablePositionToDetailPosition(CEMNDetail* pDetail, long nTableRow, long nTableCol, long& nDetailRow, long& nDetailCol)
{
	if (pDetail->m_bTableRowsAsFields) {
		nDetailRow = nTableCol - FIRST_DL_DATA_COLUMN;
		nDetailCol = (short)(nTableRow - FIRST_DL_DATA_ROW);
	} else {
		nDetailRow = (short)(nTableRow - FIRST_DL_DATA_ROW);
		nDetailCol = nTableCol - FIRST_DL_DATA_COLUMN;
	}
}

// (c.haag 2008-10-20 11:32) - PLID 31709 - Given a position in table coordinates, return the corresponding detail element
TableElement CEmrItemAdvTableBase::GetTableElementByTablePosition(CEMNDetail* pDetail, long nTableRow, long nTableCol)
{
	TableElement te;
	long nDetailRow, nDetailCol;
	TablePositionToDetailPosition(pDetail, nTableRow, nTableCol, nDetailRow, nDetailCol);
	pDetail->GetTableElement(&pDetail->GetRowPtr(nDetailRow)->m_ID, pDetail->GetColumn(nDetailCol).nID, te);
	return te;
}

// (c.haag 2008-10-20 09:47) - PLID 31700 - Merged EditingStarting handler
void CEmrItemAdvTableBase::HandleEditingStartingTable(LPDISPATCH lpRow, short nTableCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue, CEMNDetail* pDetail) 
{
	try
	{
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		int nTableRow = LookupTableRowIndexFromDatalistRow(pRow);
		long nDetailRow, nDetailCol;
		TablePositionToDetailPosition(pDetail, nTableRow, nTableCol, nDetailRow, nDetailCol);
		TableRow *ptr = pDetail->GetRowPtr(nDetailRow);
		TableColumn *ptc = pDetail->GetColumnPtr(nDetailCol);

		// (z.manning 2010-12-20 08:59) - PLID 41886 - Moved this to its own function
		*pbContinue = CanEditCell(lpRow, nTableCol, pDetail);

		if(*pbContinue)
		{
			// (z.manning 2011-10-25 11:06) - PLID 42061 - We only need to apply this to smart stamp tables since
			// those are the only tables that can have a stamp based row. Also, for not at least, only regular
			// dropdown columns support this (not text-dropdowns).
			if(ptc->nType == LIST_TYPE_DROPDOWN && pDetail->IsSmartStampTable())
			{
				// (z.manning 2011-10-11 13:14) - PLID 42061 - Set the dropdown source based on
				// the stamp ID of the current row.
				// (c.haag 2012-10-26) - PLID 53440 - Use the new getter functions
				CString strComboSource = GetDropdownSource(ptc->nID, ptr->m_ID.GetImageStampID());
				SetComboSource(pDetail, nDetailCol, strComboSource);
			}
		}

	}NxCatchAll("Error in CEmrItemAdvTableBase::HandleEditingStartingTable");
}

// (c.haag 2008-10-20 12:30) - PLID 31700 - Merged EditingFinishing handler from EmrItemAdvPopupWnd/TableDlg
void CEmrItemAdvTableBase::HandleEditingFinishingTable(LPDISPATCH lpRow, short nTableCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue,
	CWnd* pWnd, CEMNDetail* pDetail)
{
	// (c.haag 2008-01-14 12:30) - PLID 17936 - Intercept this event for handling when the user chooses
	// "<multiple>" on a dropdown selection
	try {
		// If pvarNewValue is in an unexpected state, reset the commit flag
		if (NULL == pvarNewValue || VT_NULL == pvarNewValue->vt || VT_EMPTY == pvarNewValue->vt) {
			*pbCommit = FALSE;
		}

		// Don't continue if we're not committing in the first place (maybe the user clicked off the dropdown)
		if (!*pbCommit) {
			return;
		}

		// (c.haag 2008-10-20 12:35) - PLID 31709 - Added support for table flipping
		long nDetailRow, nDetailCol;
		int nTableRow = LookupTableRowIndexFromDatalistRow(lpRow);
		TablePositionToDetailPosition(pDetail, nTableRow, nTableCol, nDetailRow, nDetailCol);
		TableRow *ptr = pDetail->GetRowPtr(nDetailRow);

		// Determine the type of table column. If it's a dropdown column, we need to do special handling.
		const TableColumn& tc = pDetail->GetColumn(nDetailCol);
		if (LIST_TYPE_DROPDOWN == tc.nType) {
			
			// Check whether the selected value is "<multiple>"
			if (m_nMultipleSelectDropdownSentinelValue == VarLong(*pvarNewValue)) {

				// Yes, the user elected multiple selections. We need to open the multi-select
				// dialog with values already selected in it.
				// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
				CMultiSelectDlg dlg(m_wndTable.GetParent(), "CEmrItemAdvTableBase::HandleEditingFinishingTable");
				CString str = VarString(UnformatDataListValue(pDetail,nDetailRow,(short)nDetailCol,varOldValue));
				TableDropdownSourceInfo* pInfo = NULL;
				if (!m_mapDropdownSourceInfo.Lookup(tc.nID, pInfo)) {
					// This should never happen; the map should have been created when the dropdown
					// source was assigned for this column!
					*pbCommit = FALSE;
					ASSERT(FALSE);
					ThrowNxException("Attempted to reference an invalid column in CEmrItemAdvTableBase::HandleEditingFinishingTable!");
				}

				// Ensure that it's even possible to make multiple selections.
				if (pInfo->nComboDataItems == 0) {
					*pbCommit = FALSE;
					ASSERT(FALSE);
					ThrowNxException("Attempted to perform a multiple selection on a dropdown column with no selections!");
				}

				// Pre-select the values
				while (!str.IsEmpty())	{
					long comma = str.Find(',');
					if(comma != -1) {
						dlg.PreSelect(atol(str));
						str = str.Right(str.GetLength() - str.Left(comma+1).GetLength());
					} else {
						dlg.PreSelect(atol(str));
						str = "";
					}
				}

				// (z.manning 2009-03-19 12:03) - PLID 33576 - Don't sort this list (the items are
				// already in sort order).
				dlg.m_bNoSort = TRUE;

				// Invoke the multi-select dialog
				// (z.manning, 03/28/2008) - PLID 29450 - Now filter on only visible dropdown elements so
				// we don't show inactive ones.
				// (z.manning 2009-03-19 12:04) - PLID 33576 - The combo source is no longer a sql query
				// so I fixed this to use the semicolon delimited 
				CVariantArray vaIDsToSkip;
				vaIDsToSkip.Add(_variant_t((long)0, VT_I4));
				vaIDsToSkip.Add(_variant_t((long)-1, VT_I4));
				vaIDsToSkip.Add(_variant_t((long)-2, VT_I4));
				vaIDsToSkip.Add(_variant_t((long)-3, VT_I4));
				// (z.manning 2011-10-11 15:24) - PLID 42061 - We may need to filter the combo text based on stamp ID.
				// (c.haag 2012-10-26) - PLID 53440 - Use the new getter functions
				if (IDCANCEL == dlg.OpenWithDelimitedComboSource(_bstr_t(pInfo->aryComboItems.GetComboText(ptr->m_ID.GetImageStampID())), vaIDsToSkip,
					"Please select at least one item"))
				{
					// If we get here, the user cancelled out
					*pbCommit = FALSE;
					return;
				}

				// If we get here, the user made one or more selections. Generate a new value in place
				// of the "<multiple>" value
				CString strNewDataString = dlg.GetMultiSelectIDString(",");
				// (a.walling 2010-08-16 17:08) - PLID 40131 - Fix leak and crash
				VariantClear(pvarNewValue);
				*pvarNewValue = FormatValueForDataList(pDetail, nDetailRow, (short)nDetailCol, _bstr_t(strNewDataString)).Detach();
			} // if (m_nMultipleSelectDropdownSentinelValue == VarLong(*pvarNewValue)) {
			else if (m_nAddNewDropdownSentinelValue == VarLong(*pvarNewValue)) {
				// (c.haag 2008-01-22 09:15) - PLID 28686 - Handle cases where we allow the user
				// to add new dropdown items on the fly. All we do is open the EMR item edit dialog
				// in such a way that it takes the user directly to the "edit dropdown contents"
				// dialog.

				// Undo the selection (or else "<add new>" will actually be selected in the cell!)
				*pbCommit = FALSE;

				// We'll actually perform the actions outside of OnEditingFinishingTable. We don't
				// want to call UpdateItem from here; the datalist might act up if things change
				// around
				pWnd->PostMessage(NXM_EMR_ADD_NEW_DROPDOWN_COLUMN_SELECTION, nDetailCol);
			}
		}
	}
	NxCatchAll("Error in CEmrItemAdvTableBase::HandleEditingFinishingTable");
}

// (c.haag 2008-10-20 09:41) - PLID 31700 - Merged EditingFinished handler
//DRT 7/28/2008 - PLID 30824 - Converted to datalist2
void CEmrItemAdvTableBase::HandleEditingFinishedTable(LPDISPATCH lpRow, short nTableCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit,
												 CWnd* pWnd, CWnd* pParentDlg, CEMNDetail* pDetail, BOOL bIsPopupWnd) 
{
	// (c.haag 2008-01-18 17:07) - PLID 17936 - If we're not committing the change, don't do anything.
	// Otherwise, we will wipe out the current selection.
	//TES 5/8/2008 - PLID 29975 - We want to do our advanced Tab handling even if they're not committing.
	//if (!bCommit)
	//	return;

	// (c.haag 2007-08-20 15:44) - PLID 27127 - Later on when we call ReflectTableState, the datalist
	// is updated with the variant representation of the table element that corresponds to this cell.
	// Rather than having ReflectTableState update -every- cell, we only want it to update one cell;
	// the one we're editing; unless the column is grouped. To do this, we change m_ReflectCurrentStateDLHint
	// once we confirm that the column has changed and is grouped. We also assign values to m_nEditingFinishedRow
	// and m_nEditingFinishedCol so that ReflectTableState knows exactly what single cell to update.
	EReflectCurrentStateDLHint rtshintOld = m_ReflectCurrentStateDLHint;

	try {
		// (c.haag 2008-10-20 09:58) - PLID 31709
		int nTableRow = LookupTableRowIndexFromDatalistRow(lpRow);

		// Preserve m_nEditingFinished... variables regardless in case we want to use them in the future
		m_pEditingFinishedRow = lpRow;
		m_nEditingFinishedCol = nTableCol;

		// (c.haag 2008-10-20 09:55) - PLID 31709 - Calculate the detail row and column indices given the
		// datalist row and column indices
		long nDetailRow, nDetailCol;
		TablePositionToDetailPosition(pDetail, nTableRow, nTableCol, nDetailRow, nDetailCol);

		//TES 5/8/2008 - PLID 29975 - Copied from CEMRItemAdvTableDlg
		//TES 8/8/2006 - Check whether they're tabbing.
		BOOL bIsShiftDown = (GetAsyncKeyState(VK_SHIFT) & 0x80000000);
		BOOL bIsTabDown = (GetAsyncKeyState(VK_TAB) & 0x80000000) || IsMessageInQueue(NULL, WM_KEYUP, VK_TAB, 0, IMIQ_MATCH_WPARAM);
		// (a.walling 2012-08-30 07:05) - PLID 51953 - Ignore if TAB was previously held down
		if (!IsTabPressed()) {
			bIsTabDown = FALSE;
		}

		// (j.jones 2008-06-05 09:09) - PLID 18529 - track whether they edited a non-text cell
		BOOL bEditedNonTextValue = FALSE;
		BOOL bStartedEditingOtherCell = FALSE;

		//DRT 7/24/2007 - PLID 26798 - If we haven't chosen to commit... then don't do any of this stuff.  Also, if nothing changed, (clicked in and out
		//	of a text cell), then there's no point going through all the updating.
		if(bCommit) {
			BOOL bChanged = TRUE;
			//Add comparisons here as we add types available for tables
			if(varOldValue.vt == VT_I4 || varNewValue.vt == VT_I4) {
				if(VarLong(varOldValue, -1) == VarLong(varNewValue, -1))
					bChanged = FALSE;
			}
			else if(varOldValue.vt == VT_BSTR || varNewValue.vt == VT_BSTR) {
				// (a.walling 2010-03-25 08:32) - PLID 37823 - This should be a case-sensitive comparison!
				if(VarString(varOldValue, "").Compare(VarString(varNewValue, "")) == 0)
					bChanged = FALSE;
			}
			//Anything else we don't know, and thus will be left to update the state just in case

			TableElement te;
			pDetail->GetTableElement(&pDetail->GetRowPtr(nDetailRow)->m_ID, pDetail->GetColumn(nDetailCol).nID, te);
			const TableElement teOld = te;

			// (j.jones 2008-06-05 09:11) - PLID 18529 - if a dropdown or linked type,
			// track that we edited a dropdown value, even if we didn't actually change the value
			if(te.m_pColumn->nType != LIST_TYPE_TEXT) {
				bEditedNonTextValue = TRUE;
			}

			if(bChanged)
			{
				_variant_t varOldState = pDetail->GetState();
				//first see if this value is blank
				BOOL bIsNotEmpty = FALSE;
				if(te.m_pColumn->nType == LIST_TYPE_CHECKBOX) {
					if(varNewValue.vt == VT_BOOL && VarBool(varNewValue)) {
						bIsNotEmpty = TRUE;
					}				
				}
				else if(te.m_pColumn->nType == LIST_TYPE_DROPDOWN) {
					// (c.haag 2008-01-15 17:49) - PLID 17936 - With dropdowns, we now use
					// negative values as sentinel placeholders for multi-select values
					if(varNewValue.vt == VT_I4 && (VarLong(varNewValue) > 0 || VarLong(varNewValue) <= m_nFirstAvailableDropdownSentinelValue)) {
						bIsNotEmpty = TRUE;
					}				
				}
				else {
					if(varNewValue.vt == VT_BSTR && VarString(varNewValue) != "") {
						bIsNotEmpty = TRUE;
					}
				}

				BOOL bChangedOtherColumns = FALSE;
				if(te.m_pColumn->bIsGrouped)
				{
					if(bIsNotEmpty)
					{
						//clear out all other grouped columns
						for(int i=0;i<pDetail->GetColumnCount();i++) {
							if(i != nDetailCol) {
								// (a.walling 2011-08-24 12:35) - PLID 45171 - We only need to look for existing table elements
								TableElement teOther;
								pDetail->GetExistingTableElement(&pDetail->GetRowPtr(nDetailRow)->m_ID, pDetail->GetColumn(i).nID, teOther);

								if(teOther.m_pColumn && teOther.m_pColumn->bIsGrouped) {
									if(teOther.m_pColumn->nType == LIST_TYPE_CHECKBOX) {
										teOther.m_bChecked = FALSE;
									}
									else if(teOther.m_pColumn->nType == LIST_TYPE_DROPDOWN) {
										// (c.haag 2008-01-11 17:41) - PLID 17936 - We now store dropdown ID's in
										// an array. For consistency, we will store the -1 there
										//teOther.m_nDropdownID = -1;
										teOther.m_anDropdownIDs.RemoveAll();
										teOther.m_anDropdownIDs.Add(-1);
									}
									else if(teOther.m_pColumn->nType == LIST_TYPE_LINKED) {
										teOther.m_pLinkedDetail = NULL;
										teOther.m_strLinkedSentenceHtml = teOther.m_strLinkedSentenceNonHtml = "";
										teOther.m_strValue = "";
									}
									else {
										teOther.m_strValue = "";
									}
									pDetail->SetTableElement(teOther);
									bChangedOtherColumns = TRUE;
								}
							}
						}
					}
					// (c.haag 2007-08-20 15:44) - PLID 27127 - We confirmed that this is a grouped
					// column, so all grouped columns should update
					m_ReflectCurrentStateDLHint = eRCS_GroupedColumnUpdate;
				} // if(te.m_pColumn->bIsGrouped) {
				else {
					// (c.haag 2007-08-20 15:44) - PLID 27127 - We confirmed that this is not a grouped
					// column, so only one cell should update.
					m_ReflectCurrentStateDLHint = eRCS_SingleCellUpdate;
				}

				if(te.m_pColumn->nType == LIST_TYPE_LINKED && bIsPopupWnd) {
					//Since we don't have an EMN to look up the linked detail in, override the default behavior.
					if(varNewValue.vt == VT_BSTR) {
						te.m_strValue = VarString(varNewValue);
					}
					else {
						te.m_strValue = "";
					}
				}
				else {
					// (c.haag 2008-01-15 17:50) - PLID 17936 - Before we pass in varNewValue into LoadValueFromVariant,
					// we must convert it from a datalist value into a table state data value.
					te.LoadValueFromVariant(UnformatDataListValue(pDetail,nDetailRow,(short)nDetailCol,varNewValue), pDetail->m_pParentTopic->GetParentEMN());
				}
				pDetail->SetTableElement(te);

				// (z.manning 2011-03-21 15:44) - PLID 30608 - Check to see if we need to autofill any columns in the
				// just edited row. We do not want to do this if they cleared out a cell, however.
				// Also, make sure we do this before generating the new state below.
				if(bIsNotEmpty) {
					if(pDetail->UpdateAutofillColumns(&te)) {
						bChangedOtherColumns = TRUE;
						m_ReflectCurrentStateDLHint = eRCS_FullDatalistUpdate;
					}
				}

				CString strNewVarState = GenerateNewVarState();

				// Don't request the state change if nothing's actually changing
				if (bChangedOtherColumns || varOldState.vt == VT_EMPTY || strNewVarState.Compare(VarString(varOldState, "")) != 0) {

					// (z.manning 2009-03-04 11:10) - PLID 33072 - Table states get recreated before
					// this is called, so we need to pass in the old state for spawning purposes.
					if (pDetail->RequestStateChange((LPCTSTR)strNewVarState, varOldState)) {
						if (bIsPopupWnd) {
							//TES 1/15/2008 - PLID 24157 - Notify our parent.
							pParentDlg->SendMessage(NXM_EMR_POPUP_POST_STATE_CHANGED, (WPARAM)this, (LPARAM)pDetail);
						}

						// (z.manning 2009-04-22 17:20) - PLID 33072 - This table may not even exist anymore
						// if, for example, it just unspawned the topic on which it was located.
						if(IsWindow(m_wndTable.GetSafeHwnd())) {
							CRect rc;
							m_wndTable.GetWindowRect(&rc);
							pWnd->ScreenToClient(&rc);
							// (b.cardillo 2006-02-22 10:57) - PLID 19376 - Since we now clip 
							// children, invalidating the child area is not good enough, so we 
							// have to actually tell each child to invalidate itself.  Also, we 
							// used to ask it to erase too, because according to an old comment 
							// related to plid 13344, the "SetRedraw" state might be false at 
							// the moment.  (I'm deleting that comment on this check-in, so to 
							// see it you'll have to look in source history.)  But I think the 
							// "SetRedraw" state no longer can be false, and even if it were I 
							// think now that we're invalidating the control itself, we wouldn't 
							// need to erase here anyway.
							m_wndTable.RedrawWindow(NULL, NULL, RDW_INVALIDATE);
						}
					}

					// (z.manning, 06/02/2008) - PLID 30155 - Move this call AFTER pDetail->RequestStateChange
					// because calculated fields do not get updated until it is called.
					if (bIsPopupWnd) { ReflectTableState(pWnd, pDetail, bIsPopupWnd); }

					// (z.manning 2010-02-26 08:34) - PLID 37412 - Moved this code into its own function
					//TES 3/11/2010 - PLID 37535 - RequestStateChange() now handles calling UpdateTableDataOutput()
					//pDetail->UpdateTableDataOutput();
				}
			} // if(bChanged) {
		} // if(bCommit) {

		//TES 5/8/2008 - PLID 29975 - Copied from CEMRItemAdvTableDlg
		//TES 8/8/2006 - PLID 21849 - If there's just one row, use the standard datalist behavior, otherwise, try to wrap.
		//DRT 7/24/2007 - PLID 26798 - Even if we chose not to commit above, we still want this tabbing behavior to be followed.
		//DRT 7/29/2008 - PLID 30824 - Converted to datalist2
		// (c.haag 2008-10-20 11:02) - PLID 31709 - Make it so the traversal is at the table level rather than the detail level
		//TES 5/9/2011 - PLID 43622 - We can't count on the standard datalist behavior any more, so we need to go through all this
		// even if there's only one row.
		if(bIsTabDown)
		{
			if(pDetail->m_bTableRowsAsFields)
			{
				// (z.manning 2010-04-12 15:22) - PLID 38129 - We now tab top to bottom on flipped tables so we
				// need to handle that case separately.
				const long nTableRowCount = GetTableRowCount();
				short nFirstTableNonCheckbox = 0;
				TableElement te = GetTableElementByTablePosition(pDetail, nFirstTableNonCheckbox, nTableCol);
				//TES 5/9/2011 - PLID 43622 - Check IsReadOnly() rather than IsCalculated(),
				// IsReadOnly() also takes into account smart stamp columns, for example
				while (nFirstTableNonCheckbox < nTableRowCount && (te.IsReadOnly() || te.m_pColumn->nType == LIST_TYPE_CHECKBOX)) {
					nFirstTableNonCheckbox++;
					if (nFirstTableNonCheckbox < nTableRowCount) {
						te = GetTableElementByTablePosition(pDetail, nFirstTableNonCheckbox, nTableCol);
					}
				};

				if(nFirstTableNonCheckbox != nTableRowCount) {
					int nLastTableNonCheckbox = nTableRowCount - 1;
					te = GetTableElementByTablePosition(pDetail, nLastTableNonCheckbox, nTableCol);
					//TES 5/9/2011 - PLID 43622 - Check IsReadOnly() rather than IsCalculated(),
					// IsReadOnly() also takes into account smart stamp columns, for example
					while (nLastTableNonCheckbox >= 0 && (te.IsReadOnly() || te.m_pColumn->nType == LIST_TYPE_CHECKBOX)) {
						nLastTableNonCheckbox--;
						if (nLastTableNonCheckbox >= 0) {
							te = GetTableElementByTablePosition(pDetail, nLastTableNonCheckbox, nTableCol);
						}
					} 

					ASSERT(nLastTableNonCheckbox >= 0); //If all rows where checkboxes, we wouldn't have gotten here.
					
					if(nTableRow == nLastTableNonCheckbox && bIsTabDown && !bIsShiftDown) {
						int nTabCol = nTableCol + 1;
						while(nTabCol < GetTableColumnCount() && pDetail->GetRowPtr(nTabCol - FIRST_DL_DATA_COLUMN)->IsCalculated()) {
							nTabCol++;
						}
						//At this point nTabCol is either off the end of the table, or the next non-calculated column.
						short nNextCol = -1;
						if(nTabCol < GetTableColumnCount()) {
							nNextCol = nTabCol;
						}

						if(nNextCol == -1) {
							//we're on the last column, so tab out
							StopTableEditing(TRUE);
						}
						else {
							long nNewRow = nFirstTableNonCheckbox;
							IRowSettingsPtr pNewRow = this->LookupDatalistRowFromTableRowIndex(nNewRow);
							//start editing the next row
							StartTableEditing(pNewRow, nNextCol);
							bStartedEditingOtherCell = TRUE;
						}
					}
					else if(nTableRow == nFirstTableNonCheckbox && bIsTabDown && bIsShiftDown) {
						int nTabCol = nTableCol - 1;
						while(nTabCol >= FIRST_DL_DATA_COLUMN && pDetail->GetRowPtr(nTabCol - FIRST_DL_DATA_COLUMN)->IsCalculated()) {
							nTabCol--;
						}
						short nPrevCol = -1;
						if(nTabCol >= FIRST_DL_DATA_COLUMN) {
							nPrevCol = nTabCol;
						}

						if(nPrevCol == -1) {
							StopTableEditing(TRUE);
						}
						else {
							long nNewRow = nLastTableNonCheckbox;
							IRowSettingsPtr pNewRow = this->LookupDatalistRowFromTableRowIndex(nNewRow);
							//start editing the next column
							StartTableEditing(pNewRow, nPrevCol);
							bStartedEditingOtherCell = TRUE;
						}
					}
					else {
						//TES 5/9/2011 - PLID 43622 - It's possible that editing a table will cause it (the datalist, that is) to get
						// re-created.  Thus, we can't ever just let the datalist handle the tabbing, but must calculate the next position
						// ourselves.
						int nIncrement = 1;
						if(bIsShiftDown) {
							ASSERT(nTableRow != nFirstTableNonCheckbox);
							nIncrement = -1;
						}
						else {
							ASSERT(nTableRow != nLastTableNonCheckbox);
							nIncrement = 1;
						}

						// (a.walling 2011-09-13 14:17) - PLID 45283 - Ensure the index never exceeds the count
						int nNextRow;
						for (nNextRow = nTableRow + nIncrement; nNextRow < nTableRowCount; nNextRow += nIncrement) {
							TableElement te = GetTableElementByTablePosition(pDetail, nNextRow, nTableCol);

							if (te.m_pColumn->nType != LIST_TYPE_CHECKBOX && !te.IsReadOnly()) {
								break;
							}
						}
						if (nNextRow < nTableRowCount) {
							IRowSettingsPtr pRow = LookupDatalistRowFromTableRowIndex(nNextRow);
							StartTableEditing(pRow, nTableCol);
							bStartedEditingOtherCell = TRUE;
						}
					}
				} // if(nFirstTableNonCheckbox != nTableColumns) {
			}
			else
			{
				const short nTableColumns = GetTableColumnCount();
				short nFirstTableNonCheckbox = FIRST_DL_DATA_COLUMN;
				// (z.manning 2008-06-12 08:32) - PLID 30155 - We also want to skip over calculated fields
				// since they aren't editable.
				// (c.haag 2008-10-20 11:21) - PLID 31735 - Now calculation traversal is table level, not detail level
				TableElement te = GetTableElementByTablePosition(pDetail, nTableRow, nFirstTableNonCheckbox);
				//TES 5/9/2011 - PLID 43622 - Check IsReadOnly() rather than IsCalculated(),
				// IsReadOnly() also takes into account smart stamp columns, for example
				while (nFirstTableNonCheckbox < nTableColumns && (te.IsReadOnly() || te.m_pColumn->nType == LIST_TYPE_CHECKBOX)) {
					nFirstTableNonCheckbox++;
					if (nFirstTableNonCheckbox < nTableColumns) {
						te = GetTableElementByTablePosition(pDetail, nTableRow, nFirstTableNonCheckbox);
					}
				};

				if(nFirstTableNonCheckbox != nTableColumns) {
					int nLastTableNonCheckbox = nTableColumns-1;
					te = GetTableElementByTablePosition(pDetail, nTableRow, nLastTableNonCheckbox);
					//TES 5/9/2011 - PLID 43622 - Check IsReadOnly() rather than IsCalculated(),
					// IsReadOnly() also takes into account smart stamp columns, for example
					while (nLastTableNonCheckbox >= 0 && (te.IsReadOnly() || te.m_pColumn->nType == LIST_TYPE_CHECKBOX)) {
						nLastTableNonCheckbox--;
						if (nLastTableNonCheckbox >= 0) {
							te = GetTableElementByTablePosition(pDetail, nTableRow, nLastTableNonCheckbox);
						}
					} 

					ASSERT(nLastTableNonCheckbox >= 0); //If all columns where checkboxes, we wouldn't have gotten here.
					
					if(nTableCol == nLastTableNonCheckbox && bIsTabDown && !bIsShiftDown) {
						// (z.manning 2008-06-12 10:23) - PLID 30155 - Skip over calculated rows.
						// (c.haag 2008-10-20 11:58) - PLID 31735 - Support table flipping

						// (a.walling 2011-03-23 18:08) - PLID 42962 - Skip hidden rows
						int nTabRow = nTableRow;
						IRowSettingsPtr pNextRow;
						do {
							nTabRow++;

							while(nTabRow < GetTableRowCount() && pDetail->GetRowPtr(nTabRow - FIRST_DL_DATA_ROW)->IsCalculated()) {
								nTabRow++;
							}
							//At this point nTabRow is either off the end of the table, or the next non-calculated row.
							if(nTabRow < GetTableRowCount()) {
								pNextRow = LookupDatalistRowFromTableRowIndex(nTabRow);
							} else {
								pNextRow = NULL;
							}
						} while (pNextRow && !pNextRow->GetVisible());

						//we're on the rightmost editable column, and are tabbing
						if(pNextRow == NULL) {
							//we're on the last row, so tab out
							StopTableEditing(TRUE);
						}
						else {
							short nNewCol = nFirstTableNonCheckbox;
							//start editing the next row
							// (a.walling 2011-05-11 14:20) - PLID 42962 - Ignore a hidden row
							bStartedEditingOtherCell = StartTableEditing(pNextRow, nNewCol);
						}
					}
					else if(nTableCol == nFirstTableNonCheckbox && bIsTabDown && bIsShiftDown) {
						// (z.manning 2008-06-12 10:23) - PLID 30155 - Skip over calculated rows.
						// (c.haag 2008-10-20 11:58) - PLID 31735 - Support table flipping

						// (a.walling 2011-03-23 18:08) - PLID 42962 - Skip hidden rows
						int nTabRow = nTableRow;
						IRowSettingsPtr pPrevRow;						
						do {
							nTabRow--;

							while(nTabRow >= 0 && pDetail->GetRowPtr(nTabRow - FIRST_DL_DATA_ROW)->IsCalculated()) {
								nTabRow--;
							}
							//At this point nTabRow is either off the top of the table, or the previous non-calculated row
							if(nTabRow >= 0) {
								pPrevRow = LookupDatalistRowFromTableRowIndex(nTabRow);
							} else {
								pPrevRow = NULL;
							}
						} while (pPrevRow && !pPrevRow->GetVisible());

						//we're on the leftmost editable column, and are tabbing backwards
						if(pPrevRow == NULL) {
							//we're on the first row, so tab out
							StopTableEditing(TRUE);
						}
						else {
							short nNewCol = nLastTableNonCheckbox;
							//start editing the next row
							// (a.walling 2011-05-11 14:20) - PLID 42962 - Ignore a hidden row
							bStartedEditingOtherCell = StartTableEditing(pPrevRow, nNewCol);
						}
					}
					else 
					{
						//TES 5/9/2011 - PLID 43622 - It's possible that editing a table will cause it (the datalist, that is) to get
						// re-created.  Thus, we can't ever just let the datalist handle the tabbing, but must calculate the next position
						// ourselves.
						int nNextRowCol = -1;
						int nIncrement = 1;
						if(bIsShiftDown) {
							ASSERT(nTableCol != nFirstTableNonCheckbox);
							nIncrement = -1;
							nNextRowCol = nLastTableNonCheckbox;
						}
						else {
							ASSERT(nTableCol != nLastTableNonCheckbox);
							nIncrement = 1;
							nNextRowCol = nLastTableNonCheckbox;
						}

						IRowSettingsPtr pNextRow = LookupDatalistRowFromTableRowIndex(nTableRow);

						if (!pNextRow->GetVisible()) {
							long nTabRow = nTableRow;						
							do {
								nTableRow = nTabRow;

								while(nTabRow < GetTableRowCount() && nTabRow >= 0 && pDetail->GetRowPtr(nTabRow - FIRST_DL_DATA_ROW)->IsCalculated()) {
									nTabRow += nIncrement;
								}
								//At this point nTabRow is either off the end of the table, or the next non-calculated row.
								if(nTabRow < GetTableRowCount() && nTabRow >= 0) {
									pNextRow = LookupDatalistRowFromTableRowIndex(nTabRow);
								} else {
									pNextRow = NULL;
								}

								nTabRow += nIncrement;
							} while (pNextRow && !pNextRow->GetVisible());

							if (pNextRow) {
								// (a.walling 2011-05-11 14:20) - PLID 42962 - Ignore a hidden row
								bStartedEditingOtherCell = StartTableEditing(pNextRow, nNextRowCol);
							}
						} else {

							// (a.walling 2011-09-13 14:17) - PLID 45283 - Ensure the index never exceeds the count
							int nNextCol;
							for (nNextCol = nTableCol + nIncrement; nNextCol < nTableColumns; nNextCol += nIncrement) {
								TableElement te = GetTableElementByTablePosition(pDetail, nTableRow, nNextCol);

								if (te.m_pColumn->nType != LIST_TYPE_CHECKBOX && !te.IsReadOnly()) {
									break;
								}
							}

							if (nNextCol < nTableColumns) {
								// (a.walling 2011-05-11 14:20) - PLID 42962 - Ignore a hidden row
								bStartedEditingOtherCell = StartTableEditing(pNextRow, nNextCol);
							}
						} 
					}
				} // if(nFirstTableNonCheckbox != nTableColumns) {
			} // if(pDetail->m_bTableRowsAsFields)
		} // if(GetTableRowCount() > 1 && bIsTabDown) {

		// (j.jones 2008-06-05 10:09) - PLID 18529 - try to advance to the next cell if it is a dropdown, but only
		// if they committed a non-text cell, and tab key is not down or we didn't start editing another cell
		// (TryAdvanceNextDropdownList will post a message, it won't happen immediately)
		if(bCommit && bEditedNonTextValue && !bIsTabDown && !bStartedEditingOtherCell) {
			// (z.manning 2011-03-03 12:57) - PLID 42574 - It's possible the row was destroyed and recreated 
			// before we got here so if that's the case let's attempt to find the new equivalent row.
			long nCurrentRowIndex = LookupTableRowIndexFromDatalistRow(lpRow, FALSE);
			if(nCurrentRowIndex == -1) {
				lpRow = LookupDatalistRowFromTableRowIndex(nTableRow);
			}
			TryAdvanceNextDropdownList(pWnd, pDetail, lpRow, nTableCol);
		}

	}NxCatchAll("Error editing the table.");	

	// (c.haag 2007-08-20 15:45) - PLID 27127 - Restore our previous hint
	m_ReflectCurrentStateDLHint = rtshintOld;
	m_pEditingFinishedRow = NULL;
	m_nEditingFinishedCol = -1;
}

// (c.haag 2008-10-16 10:12) - PLID 31700 - Ensures m_Table is a valid object, and has rows and columns properly
// assembled in it
void CEmrItemAdvTableBase::EnsureTableObject(CEMNDetail* pDetail, CWnd* pParent, const CString& strLabel, BOOL bReadOnly)
{
	if (m_Table == NULL) {
		//TES 1/29/2008 - PLID 28673 - Use a #defined IDC, not a numeric literal.
		//DRT 7/24/2008 - PLID 30824 - Converted to use NxDataList2
		// (a.walling 2011-11-11 11:11) - PLID 46627 - Just create windows with an empty rect initially rather than a 1x1 rect
		m_wndTable.CreateControl(_T("NXDATALIST2.NxDataListCtrl.1"), strLabel, WS_CHILD, CRect(0,0,0,0), pParent, TABLE_IDC);

		if (m_wndTable.GetSafeHwnd()) {
			m_wndTable.ModifyStyleEx(WS_EX_NOPARENTNOTIFY, 0);
		}
		
		m_Table = m_wndTable.GetControlUnknown();

		ASSERT(m_Table != NULL); // We need to be able to get the control unknown and set an NxDataListPtr to point to it

		m_Table->IsComboBox = FALSE;
		// (a.walling 2010-07-28 14:10) - PLID 39871 - Explicitly NOT choosing snapshot isolation connection here, although it seems moot anyway
		m_Table->AdoConnection = GetRemoteData();
		m_Table->AllowSort = FALSE;
		m_Table->GridVisible = TRUE;

		// (a.walling 2011-11-11 11:11) - PLID 46621 - Unify font handing for emr items
		// Datalist font was never set previously; I imagine modifying it could cause some havoc with the saved column widths and etc.

		m_wndTable.ShowWindow(SW_SHOW);

		//
		// (c.haag 2007-02-12 11:26) - PLID 24376 - If this is a Current Medications detail, old
		// or new, then the datalist itself must be read-only. We do not allow users to change
		// the state of Current Medications details on templates
		//
		if (pDetail->m_bIsTemplateDetail && pDetail->IsCurrentMedicationsTable()) {
			m_Table->PutReadOnly(VARIANT_TRUE);
			SetCommonListButtonsReadOnly(TRUE);
		} else {
			// (c.haag 2007-04-05 13:40) - PLID 25516 - If this is an Allergies detail, old or new,
			// then the datalist itself must be read-only. We do not allow users to change the state
			// of Allergies details on templates
			if (pDetail->m_bIsTemplateDetail && pDetail->IsAllergiesTable()) {
				m_Table->PutReadOnly(VARIANT_TRUE);
				SetCommonListButtonsReadOnly(TRUE);
			} else {
				m_Table->PutReadOnly(bReadOnly);
				SetCommonListButtonsReadOnly(bReadOnly);
			}
		}
	}
}

// (c.haag 2008-10-16 17:29) - PLID 31709 - Adds a field to a standard unflipped table (legacy code)
// (r.gonet 02/14/2013) - PLID 40017 - Added a flag to tell us that the table is in a popup.
void CEmrItemAdvTableBase::AddTableField(CEMNDetail* pDetail, int nDetailColumn, BOOL bIsPoppedUp)
{
	TableColumn tc = pDetail->GetColumn(nDetailColumn);
	long lDataColumnStyle = 0;
	long nListType = tc.nType;
	long nDataID = tc.nID;

	if (!pDetail->GetSaveTableColumnWidths()) {
		// Fit column widths to the table area
		lDataColumnStyle = csWidthAuto;
	}

	//DRT 7/10/2007 - PLID 24105 - We want to size by the saved width of each column.  This will force the detail box to grow
	//	if the user has just added a column, rather than resizing columns the user has already explicitly sized.
	long nWidth; 
	// (r.gonet 02/14/2013) - PLID 40017 - If the table is not popped up or we don't want to remember the column sizes in the popup, then use the regular saved width.
	if(!bIsPoppedUp || GetRemotePropertyInt(GetRemoteData(), "EMRRememberPoppedUpTableColumnWidths", 0, 0, "<None>", true) == 0) {
		nWidth = pDetail->GetColumnWidths()->GetWidthByName(tc.strName);
	} else {
		// (r.gonet 02/14/2013) - PLID 40017 - If it is in a popup and we want to remember the column sizes in the popup, then recall the popped up column size.
		nWidth = pDetail->GetColumnWidths()->GetPopupWidthByName(tc.strName, true);
	}
	if(nWidth == -1)
		//This must be a new column, so we'll calculated the needed size
		nWidth = CalcSysIconTitleFontWidth(tc.strName + "     "); //throw in an extra five spaces to account for padding

	// (z.manning, 05/28/2008) - PLID 30155 - Calculated fields aren't editable
	// (z.manning 2010-03-02 10:38) - PLID 37230 - Neither are smart stamp quantity fields
	// (z.manning 2010-04-13 16:40) - PLID 38175 - No editing label columns
	DWORD dwEditable = 0;
	// (z.manning 2010-04-19 16:22) - PLID 38228 - Put this logic in its own function
	if(!tc.IsReadOnly()) {
		dwEditable = csEditable;
	}

	IColumnSettingsPtr pCol = m_Table->GetColumn(m_Table->InsertColumn(nDetailColumn+FIRST_DL_DATA_COLUMN, _T(_bstr_t(tc.strName)), _T(_bstr_t(tc.strName)), nWidth, csVisible|lDataColumnStyle|dwEditable));
	pCol->FieldType = GetColumnFieldType(nListType);
	// (z.manning 2009-01-16 08:59) - PLID 32724 - Set the InputMask for this column if we have one
	if(!tc.m_strInputMask.IsEmpty()) {
		pCol->PutInputMask(_bstr_t(tc.m_strInputMask));
	}
	m_Table->GetColumn(nDetailColumn+FIRST_DL_DATA_COLUMN)->DataType = GetColumnDataType(nListType);
	// (z.manning 2011-09-22 10:00) - PLID 41954 - Set the dropdown text separator
	pCol->PutDropdownTextSeparator(_bstr_t(tc.m_strDropdownSeparator));

	// (z.manning 2011-03-11) - PLID 42618 - Text columns now have a combo source
	if(tc.CanHaveDropdownElements()) {
		SetEmbeddedComboDropDownMaxHeight(pDetail, nDetailColumn, 400);

		//lookup the passed-in combo sql
		// (c.haag 2008-01-15 17:55) - PLID 17936 - We now get the combo SQL from a member utility function
		CString strComboSql = GetDropdownSource(tc.nID, -1);
		SetComboSource(pDetail, nDetailColumn, strComboSql);
	}

	if(nListType == LIST_TYPE_LINKED) {
		//lookup the passed-in combo sql
		CSqlFragment sqlCombo = pDetail->GetColumnSql(tc.nID);

		SetComboSource(pDetail, nDetailColumn, sqlCombo.Flatten());
		SetEmbeddedComboDropDownMaxHeight(pDetail, nDetailColumn, 400);
	}
}

// (c.haag 2008-10-16 17:29) - PLID 31709 - Adds a field (which is a row in a flipped table) to a flipped table
void CEmrItemAdvTableBase::AddTableField_Flipped(CEMNDetail* pDetail, int nDetailColumn)
{
	TableColumn tc = pDetail->GetColumn(nDetailColumn);
	long nListType = tc.nType;
	long nDataID = tc.nID;

	// (z.manning, 05/28/2008) - PLID 30155 - Calculated fields aren't editable
	// (z.manning 2010-03-02 10:38) - PLID 37230 - Neither are smart stamp quantity fields
	// (z.manning 2010-04-13 16:40) - PLID 38175 - No editing label columns
	DWORD dwEditable = 0;
	// (z.manning 2010-04-19 16:22) - PLID 38228 - Put this logic in its own function
	if(!tc.IsReadOnly()) {
		dwEditable = csEditable;
	}

	long nNewRowIndex = m_Table->GetRowCount();
	IRowSettingsPtr pRow = m_Table->GetNewRow();
	m_Table->AddRowAtEnd(pRow, NULL);
	m_mapIndexToDatalistRow.SetAt(nNewRowIndex, pRow);
	m_mapDatalistRowToIndex.SetAt(pRow, nNewRowIndex);

	pRow->Value[0L] = _bstr_t(tc.strName);
	if(tc.m_bIsLabel) {
		// (z.manning 2010-04-13 17:16) - PLID 38175 - Grey out label rows
		pRow->PutCellBackColor(0, EMR_TABLE_CELL_GRAY);
	}
	// (z.manning 2012-03-28 10:09) - PLID 33710 - Need to color formula cells
	for(int nDetailRow = 0; nDetailRow < pDetail->GetRowCount(); nDetailRow++) {
		TableRow *ptr = pDetail->GetRowPtr(nDetailRow);
		if(ptr->IsCalculated()) {
			long nTableRow, nTableColumn;
			DetailPositionToTablePosition(pDetail, nDetailRow, nDetailColumn, nTableRow, nTableColumn);
			//TES 10/12/2012 - PLID 39000 - Set the color and tooltip based on whether this cell is a formula that includes values
			// with unused numbers
			TableElement te;
			pDetail->GetTableElement(&(ptr->m_ID), nDetailColumn, te);
			pRow->PutCellBackColor((short)nTableColumn, te.m_bFormulaHasUnusedNumbers?EMR_TABLE_CELL_RED:EMR_TABLE_CELL_GRAY);
			pRow->PutCellToolTipOverride((short)nTableColumn, te.m_bFormulaHasUnusedNumbers?EMR_UNUSED_NUMBERS_TOOLTIP:"");
		}
	}

	// Set up physical row formatting
	IFormatSettingsPtr pFormatRow(__uuidof(FormatSettings));			
	pFormatRow->Connection = _variant_t((LPDISPATCH)GetRemoteData());
	pFormatRow->DataType = GetColumnDataType(nListType);
	pFormatRow->FieldType = GetColumnFieldType(nListType);
	pFormatRow->Editable = (dwEditable) ? _variant_t(VARIANT_TRUE, VT_BOOL) : _variant_t(VARIANT_FALSE, VT_BOOL);
	// (z.manning 2009-01-16 08:59) - PLID 32724 - Set the input mask for this row if we have one.
	if(!tc.m_strInputMask.IsEmpty()) {
		pFormatRow->PutInputMask(_bstr_t(tc.m_strInputMask));
	}
	// (z.manning 2011-03-11) - PLID 42618 - Text columns now have a combo source
	if(tc.CanHaveDropdownElements()) {
		pFormatRow->EmbeddedComboDropDownMaxHeight = 400;

		//lookup the passed-in combo sql
		// (c.haag 2008-01-15 17:55) - PLID 17936 - We now get the combo SQL from a member utility function
		CString strComboSql = GetDropdownSource(tc.nID, -1);
		pFormatRow->PutComboSource(_bstr_t(strComboSql));
	}
	// (z.manning 2011-09-22 10:00) - PLID 41954 - Set the dropdown text separator
	pFormatRow->PutDropdownTextSeparator(_bstr_t(tc.m_strDropdownSeparator));

	if(nListType == LIST_TYPE_LINKED) {
		//lookup the passed-in combo sql
		CSqlFragment sqlCombo = pDetail->GetColumnSql(tc.nID);

		pFormatRow->PutComboSource(_bstr_t(sqlCombo.Flatten()));
		pFormatRow->EmbeddedComboDropDownMaxHeight = 400;
	}
	pRow->FormatOverride = pFormatRow;

	// Set up 0th physical column formatting
	IFormatSettingsPtr pFormat0thColumn(__uuidof(FormatSettings));
	pFormat0thColumn->DataType = VT_BSTR;
	pFormat0thColumn->FieldType = cftTextWordWrap;
	pFormat0thColumn->Editable = _variant_t(VARIANT_FALSE, VT_BOOL);
	pRow->CellFormatOverride[0L] = pFormat0thColumn;
}

// (c.haag 2008-10-16 14:33) - PLID 31700 - Adds fields to the table. Normally, these
// are table columns. In the case of a "flipped" table; that is, a table where the
// rows are the fields, then this will utilize the FormatOverride object in the datalist.
void CEmrItemAdvTableBase::AddTableFields(CEMNDetail* pDetail, BOOL bIsPopupWnd)
{
	// (r.gonet 02/14/2013) - PLID 40017 - Get the flag telling us we need to remember the column sizes in the popup.
	BOOL bRememberPoppedUpTableColumnWidths = GetRemotePropertyInt("EMRRememberPoppedUpTableColumnWidths", 0, 0, "<None>", true) != 0 ? TRUE : FALSE;
	// (c.haag 2008-10-16 15:07) - PLID 31709 - True if we're flipping the table "90 degrees"
	// so that rows effectively act as columns as we know them.
	const BOOL bTableRowsAsFields = pDetail->m_bTableRowsAsFields;
	// (a.walling 2007-11-05 16:08) - PLID 27980 - VS2008 - for() loops
	int i = 0;

	// Set up the data
	//first add the columns
	long lFirstColumnStyle = 0;
	if (!pDetail->GetSaveTableColumnWidths()) {
		// Size first column by data
		lFirstColumnStyle = csWidthData;
	}

	//DRT 7/10/2007 - PLID 24105 - We want to size by the saved width of each column.  This will force the detail box to grow
	//	if the user has just added a column, rather than resizing columns the user has already explicitly sized.
	// (c.haag 2008-06-26 11:25) - PLID 27968 - Replaced GetWidthByName with GetFirstColumnWidth
	long n0thWidth;
	// (r.gonet 02/14/2013) - PLID 40017 - If the table is not popped up or we don't want to remember the column sizes in the popup, then use the regular saved width.
	if(!bIsPopupWnd || !bRememberPoppedUpTableColumnWidths) {
		n0thWidth = pDetail->GetColumnWidths()->GetFirstColumnWidth();
	} else {
		n0thWidth = pDetail->GetColumnWidths()->GetFirstPopupColumnWidth(true);
	}
	if(n0thWidth == -1)
		//This must be a new column, so we'll calculated the needed size
		n0thWidth = 10;		//This was the previous default size

	//column 0 is blank
	// (j.gruber 2007-02-07 15:28) - PLID 24635 - make the column wordwrap
	IColumnSettingsPtr(m_Table->GetColumn(m_Table->InsertColumn(0, _T("Row Name"), _T("  "), n0thWidth, csVisible|lFirstColumnStyle)))->FieldType = cftTextWordWrap;
	m_Table->GetColumn(0)->DataType = VT_BSTR;

	// (c.haag 2008-10-16 15:51) - PLID 31709 - Because the logic below will resize physical columns, and physical columns
	// can be resized by users, we need to create those columns here and now.
	if (bTableRowsAsFields) {
		for (i=0; i<pDetail->GetRowCount(); i++) {
			TableRow *ptr = pDetail->GetRowPtr(i);
			long lDataColumnStyle = 0;

			if (!pDetail->GetSaveTableColumnWidths()) {
				// Fit column widths to the table area
				lDataColumnStyle = csWidthAuto;
			}

			//DRT 7/10/2007 - PLID 24105 - We want to size by the saved width of each column.  This will force the detail box to grow
			//	if the user has just added a column, rather than resizing columns the user has already explicitly sized.
			long nWidth;
			if(!bIsPopupWnd || !bRememberPoppedUpTableColumnWidths) {
				nWidth = pDetail->GetColumnWidths()->GetWidthByName(ptr->strName);
			} else {
				// (r.gonet 02/14/2013) - PLID 40017 - If it is in a popup and we want to remember the column sizes in the popup, then recall the popped up column size.
				nWidth = pDetail->GetColumnWidths()->GetPopupWidthByName(ptr->strName, true);
			}
			if(nWidth == -1) {
				//This must be a new column, so we'll calculated the needed size
				nWidth = CalcSysIconTitleFontWidth(ptr->strName + "     "); //throw in an extra five spaces to account for padding
			}

			m_Table->InsertColumn(i+1, _T(_bstr_t(ptr->strName)), _T(_bstr_t(ptr->strName)), nWidth, csVisible|lDataColumnStyle);
		}
	}

	// (a.wetta 2007-02-07 17:32) - PLID 24564 - If this is an active current medications list, then color the medication's
	// column's text blue.
	// (c.haag 2007-04-02 15:46) - PLID 25465 - If this is an active allergy table, then color the column text blue
	// (c.haag 2008-10-16 15:15) - PLID 31709 - We don't permit flipping system tables
	if (!bTableRowsAsFields && (m_bIsActiveCurrentMedicationsTable || m_bIsActiveAllergiesTable)) {
		m_Table->GetColumn(0)->ForeColor = RGB(0,0,128);
	}

	//(e.lally 2007-01-29) PLID 24444 - Set the first column (row names) to be the text search column.
	//We will not allow sorting though since that is a property of the EMN item and can be a user
	//defined ordering.
	//TES 4/30/2008 - PLID 28958 - Need to set the text search column, just like CEMRItemAdvTableDlg does.
	m_Table->TextSearchCol = 0;

	// (j.jones 2007-08-10 14:18) - PLID 27046 - removed limitation for 1000 table items
	for (i=0; i<pDetail->GetColumnCount(); i++) {
		// (c.haag 2008-10-16 15:17) - PLID 31709 - If this is a flipped table, we are actually adding rows here.
		if (bTableRowsAsFields) {
			AddTableField_Flipped(pDetail, i);
		} else {
			// (r.gonet 02/14/2013) - PLID 40017 - Need to know if we are in a popup
			AddTableField(pDetail, i, bIsPopupWnd);
		}
	} // for (i=0; i<pDetail->GetColumnCount(); i++) {

	// (a.wetta 2005-11-04 13:12) - PLID 18168 - Get column width information	
	long lDetailID = pDetail->m_nEMRDetailID;
	// (m.hancock 2006-06-02 09:47) - PLID 20519 - Copying EMR tables is not applying proper column widths.
	// We keep track of the source table's EMRDetailID, so retrieve that in the event we are copying tables.
	// (j.jones 2009-04-10 09:23) - PLID 33956 - renamed from m_nSourceEMRDetailID to m_nCopiedFromEMRDetailID
	// (c.haag 2010-07-15 15:47) - PLID 39664 - We need bReadFromTemplateDetail to track whether lDetailID
	// originated from EMRDetailsT or EMRTemplateDetailsT. m_nCopiedFromEMRDetailID is set from m_nEMNDetailID,
	// so when it's stored in lDetailID we consider it a detail ID. When lDetailID takes a m_nEMRTemplateDetailID value,
	// we can be sure it corresponds to a template detail ID.
	BOOL bReadFromTemplateDetail = FALSE;
	if (lDetailID == -1) {
		lDetailID = pDetail->m_nCopiedFromEMRDetailID;
	}
	//If we're on a template, or we're not copying a table, use the template's column widths.
	if (pDetail->m_bIsTemplateDetail || lDetailID == -1) {		
		lDetailID = pDetail->m_nEMRTemplateDetailID;
		bReadFromTemplateDetail = TRUE; // (c.haag 2010-07-15 15:47) - PLID 39664
	}

	// (m.hancock 2006-06-06 14:20) - PLID 20519 - If lDetailID is -1, we can't apply the column widths.  We
	// should have already applied the default widths above.
	// (c.haag 2008-06-26 11:25) - PLID 27968 - Replaced GetWidthByName with GetFirstColumnWidth
	// (c.haag 2008-10-16 17:36) - PLID 31709 - This code deals with physical column widths. No changes for
	// table flipping are necessary here.
	if ((lDetailID != -1) && (pDetail->GetSaveTableColumnWidths())) {
		//DRT 7/10/2007 - PLID 24105 - Due to our changes in sizing new stuff above, we've already put the saved values into
		//	action when we created the columns.  Only if we haven't cached anything yet (the nameless column is -1 width)
		//	would we need to do anything.
		// (c.haag 2008-06-26 11:25) - PLID 27968 - Replaced GetWidthByName with GetFirstColumnWidth
		// (r.gonet 02/14/2013) - PLID 40017 - If we are not popped up and there is a saved width,
		//  or if we are popped up, we don't want to remember the column widths of the popped up table, and we have a width for the first column in the regular table
		//  or if we are popped up, we want to remember the column widths of the popped up table, and we have a width for the first column in the popped up table
		//  then we must go set the stored widths to their appropriate values.
		if((!bIsPopupWnd && pDetail->GetColumnWidths()->GetFirstColumnWidth() != -1)
			|| (bIsPopupWnd && !bRememberPoppedUpTableColumnWidths && pDetail->GetColumnWidths()->GetFirstColumnWidth() != -1)
			|| (bIsPopupWnd && bRememberPoppedUpTableColumnWidths && pDetail->GetColumnWidths()->GetFirstPopupColumnWidth(true) != -1)) {
			if (bIsPopupWnd) {
				// (c.haag 2008-06-26 11:27) - PLID 27968 - The 0th column requires special handling
				long nSavedWidth;
				// (r.gonet 02/14/2013) - PLID 40017 - If aren't remembering pop up table's column widths, then use the regular column width, otherwise try to use the popped up version.
				if(!bRememberPoppedUpTableColumnWidths) {
					nSavedWidth = pDetail->GetColumnWidths()->GetFirstColumnWidth();  
				} else {
					nSavedWidth = pDetail->GetColumnWidths()->GetFirstPopupColumnWidth(true);
				}
				m_Table->GetColumn(0)->StoredWidth = nSavedWidth;

				for (int k = 1; k < m_Table->GetColumnCount(); k++) {
					CString strColumnTitle;
					strColumnTitle.Format("%s", AsString(m_Table->GetColumn(k)->GetColumnTitle()));
					strColumnTitle.TrimLeft();
					//DRT 7/10/2007 - PLID 24105 - Get the width by the column name, and if it's known, set
					//	the stored width of the datalist.
					// (r.gonet 02/14/2013) - PLID 40017 - If aren't remembering pop up table's column widths, then use the regular column width, otherwise try to use the popped up version.
					if(!bRememberPoppedUpTableColumnWidths) {
						nSavedWidth = pDetail->GetColumnWidths()->GetWidthByName(strColumnTitle);
					} else {
						nSavedWidth = pDetail->GetColumnWidths()->GetPopupWidthByName(strColumnTitle, true);
					}
					if(nSavedWidth != -1)
						m_Table->GetColumn(k)->StoredWidth = nSavedWidth;
				}
			} else {
				//In this case, we already read the cached values above when creating the columns
			}
		}
		else {
			//load from data
			// (j.jones 2007-02-13 10:05) - PLID 21704 - only if the ID is not -1
			if(lDetailID != -1) {
				// (c.haag 2008-10-21 11:23) - PLID 31709 - If the table is flipped, then EMRDataID_X actually takes the role of a column
				// in the context of table column widths. So, we need to use that field instead of EMRDataID_Y. EMRDataID_X did not exist
				// in the column width SQL table prior to 9000 scope.
				CString strEMRDataField = (pDetail->m_bTableRowsAsFields) ? "EMRDataID_X" : "EMRDataID_Y";

				// (j.jones 2010-06-09 15:18) - PLID 37981 - made separate parameterized permutations of this recordset in order
				// to easily support generic tables, which use EMRDetailTableOverrideDataT only on EMN details
				_RecordsetPtr rsColumnWidths;
				// (c.haag 2010-07-15 15:49) - PLID 39664 - Check bReadFromTemplateDetail, not pDetail->m_bIsTemplateDetail,
				// because this detail could be on a patient EMN that was created from a template.
				if(bReadFromTemplateDetail) {
					//load from the template
					// (r.gonet 02/14/2013) - PLID 40017 - Get the column widths of the popped up table.
					rsColumnWidths = CreateParamRecordset(FormatString("SELECT EMRTemplateTableColumnWidthsT.ColumnWidth, EMRTemplateTableColumnWidthsT.PopupColumnWidth, "
						"EMRDataT.Data, EMRTemplateTableColumnWidthsT.%s "
						"FROM EMRTemplateTableColumnWidthsT "
						"LEFT JOIN EMRDataT ON EMRTemplateTableColumnWidthsT.%s = EMRDataT.ID "
						"WHERE EMRTemplateTableColumnWidthsT.EMRDetailID = {INT}", strEMRDataField, strEMRDataField),
						lDetailID);
				}
				else {
					//load from the EMN detail
					// (r.gonet 02/14/2013) - PLID 40017 - Get the column widths of the popped up table.
					rsColumnWidths = CreateParamRecordset(FormatString("SELECT EMRTableColumnWidthsT.ColumnWidth, EMRTableColumnWidthsT.PopupColumnWidth, "
						"CASE WHEN EMRInfoT.DataSubType = {INT} AND EMRDetailTableOverrideDataT.Name Is Not Null THEN EMRDetailTableOverrideDataT.Name ELSE EmrDataT.Data END AS Data, "
						"EMRTableColumnWidthsT.%s "
						"FROM EMRTableColumnWidthsT "
						"LEFT JOIN EMRDataT ON EMRTableColumnWidthsT.%s = EMRDataT.ID "
						"LEFT JOIN EMRInfoT ON EMRDataT.EMRInfoID = EMRInfoT.ID "
						"LEFT JOIN EMRDetailTableOverrideDataT ON EMRDataT.ID = EMRDetailTableOverrideDataT.EMRDataID AND EMRTableColumnWidthsT.EMRDetailID = EMRDetailTableOverrideDataT.EMRDetailID "
						"WHERE EMRTableColumnWidthsT.EMRDetailID = {INT}", strEMRDataField, strEMRDataField),
						eistGenericTable, lDetailID);
				}
				
				if (!rsColumnWidths->eof) {
					CString strColumnTitle;
					for (int j = 0; j < rsColumnWidths->GetRecordCount(); j++) {
						if (-1 == AdoFldLong(rsColumnWidths, strEMRDataField, -1)) {
							// (c.haag 2008-06-26 11:13) - PLID 27968 - If we get here, it means this is the
							// non-data-entry column. Don't do a name search; or else we may have have conflicts
							// with other title-less columns.
							long nSavedWidth = AdoFldLong(rsColumnWidths->GetFields()->GetItem("ColumnWidth"));
							// (r.gonet 02/14/2013) - PLID 40017 - Get the column's width in the popped up table.
							long nSavedPopupWidth = AdoFldLong(rsColumnWidths->GetFields()->GetItem("PopupColumnWidth"), -1);
							if(!bRememberPoppedUpTableColumnWidths) {
								// (r.gonet 02/14/2013) - PLID 40017 - We don't want to remember the popped up width, so act like its nothing.
								nSavedPopupWidth = -1;
							}
							// (r.gonet 02/14/2013) - PLID 40017 - Use the regular column width when we aren't popped up or we don't have anything saved for the popped up width.
							if(!bIsPopupWnd || nSavedPopupWidth == -1) {
								m_Table->GetColumn(0)->StoredWidth = nSavedWidth;
							} else {
								m_Table->GetColumn(0)->StoredWidth = nSavedPopupWidth;
							}
							pDetail->GetColumnWidths()->SetFirstColumnWidth(nSavedWidth);
							// (r.gonet 02/14/2013) - PLID 40017 - Assign the popped up table column's width
							pDetail->GetColumnWidths()->SetFirstPopupColumnWidth(nSavedPopupWidth);
						}
						else {
							// (c.haag 2008-06-26 11:36) - PLID 27968 - Start at iteration 1; 0 is handled above
							CString strData = AdoFldString(rsColumnWidths->GetFields()->GetItem("Data"), "");
							for (int k = 1; k < m_Table->GetColumnCount(); k++) {
								strColumnTitle.Format("%s", AsString(m_Table->GetColumn(k)->GetColumnTitle()));
								strColumnTitle.TrimLeft();
								if (strColumnTitle == strData) {
									long nSavedWidth = AdoFldLong(rsColumnWidths->GetFields()->GetItem("ColumnWidth"));
									// (r.gonet 02/14/2013) - PLID 40017 - Get the column's width in the popped up table.
									long nSavedPopupWidth = AdoFldLong(rsColumnWidths->GetFields()->GetItem("PopupColumnWidth"), -1);
									if(!bRememberPoppedUpTableColumnWidths) {
										// (r.gonet 02/14/2013) - PLID 40017 - We don't want to remember the popped up width, so act like its nothing.
										nSavedPopupWidth = -1;
									}
									// (r.gonet 02/14/2013) - PLID 40017 - Use the regular column width when we aren't popped up or we don't have anything saved for the popped up width.
									if(!bIsPopupWnd || nSavedPopupWidth == -1) {
										m_Table->GetColumn(k)->StoredWidth = nSavedWidth;
									} else {
										m_Table->GetColumn(k)->StoredWidth = nSavedPopupWidth;
									}
									//also keep the width stored in memory
									//DRT 7/10/2007 - PLID 24105 - We are saving this in a new structure that is based off the 
									//	column name.
									pDetail->GetColumnWidths()->SetWidthByName(strColumnTitle, nSavedWidth);
									// (r.gonet 02/14/2013) - PLID 40017 - Assign the popped up table column's width
									pDetail->GetColumnWidths()->SetPopupWidthByName(strColumnTitle, nSavedPopupWidth);
								}
							}
						}
						rsColumnWidths->MoveNext();
					}
				} // if (!rsColumnWidths->eof) {
				// (r.gonet 02/14/2013) - PLID 40017 - We now always want to update the column style because we can have separate widths for the popup.
				else {// if (!bIsPopupWnd) {
					//if we don't have a stored value and there is nothing in data, then force
					//the detail to use the automatic widths that are used when then column widths are reset
					pDetail->SetSaveTableColumnWidths(FALSE);
					UpdateColumnStyle(pDetail, bIsPopupWnd);
				}
				rsColumnWidths->Close();
			} // if(lDetailID != -1) {
			// (r.gonet 02/14/2013) - PLID 40017 - We now always want to update the column style because we can have separate widths for the popup.
			else {// if (!bIsPopupWnd) {
				//we're reloading an unsaved new table, we have no stored value
				pDetail->SetSaveTableColumnWidths(FALSE);
				UpdateColumnStyle(pDetail, bIsPopupWnd);
			}
		}

	} // if ((lDetailID != -1) && (pDetail->GetSaveTableColumnWidths())) {

}

// (c.haag 2008-10-16 11:00) - PLID 31700 - Adds content to m_Table
void CEmrItemAdvTableBase::AddTableContent(CEMNDetail* pDetail)
{
	// (c.haag 2008-10-16 15:07) - PLID 31709 - True if we're flipping the table
	const BOOL bTableRowsAsFields = pDetail->m_bTableRowsAsFields;

	// (z.manning 2010-04-12 15:21) - PLID 38129 - If this is a flipped table then set the tabbing to be
	// top to bottom instead of left to right.
	m_Table->PutTabTopToBottom(bTableRowsAsFields ? VARIANT_TRUE : VARIANT_FALSE);

	//
	// (c.haag 2007-02-20 09:15) - PLID 24679 - CEMNDetail::GetTableElement is slow.
	// By building a CMap object with all of the table elements at the beginning, and
	// then using that map within the innermost loop, we get a performance gain.
	//
	const int nDetailRows = pDetail->GetRowCount();
	const int nDetailColumns = pDetail->GetColumnCount();
	CMap<__int64, __int64, int, int> mapTable; // This maps row/column combinations to elements
	const int nDetailElements = pDetail->GetTableElementCount();
	int i = 0;

	for(i = 0; i < nDetailElements; i++) {
		TableElement te;
		pDetail->GetTableElementByIndex(i, te);
		__int64 nKey = (((__int64)te.m_pRow) << 32) + (__int64)te.m_pColumn->nID;
		mapTable[nKey] = i;
	}

	// (a.walling 2011-03-21 12:34) - PLID 42962 - Filter if we are a current medication or allergies table
	bool bFilteredView = pDetail->IsCurrentMedicationsTable() || pDetail->IsAllergiesTable();

	// (j.jones 2007-08-10 14:18) - PLID 27046 - removed limitation for 1000 table items
	// (c.haag 2008-10-17 09:36) - PLID 31709 - If this is not a flipped table, proceed to
	// fill the table with empty rows. A flipped table already has all its rows by this point.
	if (!bTableRowsAsFields) {
		for (i=0; i<nDetailRows; i++) {
			TableRow *ptr = pDetail->GetRowPtr(i);

			IRowSettingsPtr pRow = m_Table->GetNewRow();
			pRow->PutValue(0, _bstr_t(ptr->strName));
			if(ptr->m_bIsLabel || ptr->IsCalculated()) {
				// (z.manning 2010-04-13 17:16) - PLID 38175 - Grey out label rows
				TableElement te;
				pRow->PutCellBackColor(0, EMR_TABLE_CELL_GRAY);
			}
			// (z.manning 2012-03-28 10:09) - PLID 33710 - Need to color formula cells
			for(int nDetailCol = 0; nDetailCol < pDetail->GetColumnCount(); nDetailCol++) {
				TableColumn *ptc = pDetail->GetColumnPtr(nDetailCol);
				if(ptc->IsCalculated()) {
					long nTableRow, nTableColumn;
					DetailPositionToTablePosition(pDetail, i, nDetailCol, nTableRow, nTableColumn);
					//TES 10/12/2012 - PLID 39000 - Set the color and tooltip based on whether this cell is a formula that includes values
					// with unused numbers
					TableElement te;
					pDetail->GetTableElement(&(ptr->m_ID), nDetailCol, te);
					pRow->PutCellBackColor((short)nTableColumn, te.m_bFormulaHasUnusedNumbers?EMR_TABLE_CELL_RED:EMR_TABLE_CELL_GRAY);
					pRow->PutCellToolTipOverride((short)nTableColumn, te.m_bFormulaHasUnusedNumbers?EMR_UNUSED_NUMBERS_TOOLTIP:"");
				}
			}

			// (a.walling 2011-03-21 12:34) - PLID 42962 - If we are filtering, all rows will be invisible by default
			if (bFilteredView) {
				pRow->PutVisible(VARIANT_FALSE);
			}

			//if we ever support defaults, this would be the place to load them
			//DRT 7/24/2008 - PLID 30824 - Converted to use NxDataList2
			m_Table->AddRowAtEnd(pRow, NULL);

			//DRT 7/24/2008 - PLID 30824 - Converted to use NxDataList2, so now we need to save
			//	this information in our map.
			m_mapIndexToDatalistRow.SetAt(i, pRow);
			//DRT 7/25/2008 - PLID 30824 - Additionally set the same in reverse.  This is needed for
			//	speed optimization, it can be too slow to iterate through that map if you need to
			//	go backwards.
			m_mapDatalistRowToIndex.SetAt(pRow, i);
		}
	}

	// (a.walling 2011-03-21 12:34) - PLID 42962 - Rows to show
	CArray<IRowSettingsPtr> rowsToShow;

	// (c.haag 2008-10-17 09:40) - PLID 31709 - Now fill the datalist table with values
	for (i=0; i<nDetailRows; i++) {
		TableRow *ptr = pDetail->GetRowPtr(i);

		bool bRowHasData = false;

		for(int j=0; j<nDetailColumns; j++) {
			TableElement te;

			// (c.haag 2007-02-20 09:21) - PLID 24679 - Use the map instead of
			// GetTableElement for efficiency
			//pDetail->GetTableElement(tr.nID, pDetail->GetColumn(j).nID, te);
			TableColumn tc = pDetail->GetColumn(j);
			// (z.manning 2010-02-18 11:12) - PLID 37427 - Row ID is now a struct
			TableRowID *pRowID = &ptr->m_ID;
			const int nColumnID = tc.nID;
			// (z.manning, 05/28/2008) - PLID 30155 - We will handle calculated fields later.
			// (z.manning 2012-03-28 10:19) - PLID 33710 - Calculated cell values are now saved to and loaded from data
			//if(!ptr->IsCalculated() && !tc.IsCalculated())
			{
				__int64 nKey = (((__int64)ptr) << 32) + (__int64)nColumnID;
				int nTableIndex;
				if (mapTable.Lookup(nKey, nTableIndex)) {
					// If this cell has an element, assign it to the datalist
					pDetail->GetTableElementByIndex(nTableIndex, te);
				}
				else {
					// If this cell has no element, assign it a default. This should
					// be the responsibility of TableElement::GetValueAsVariant. You
					// might think "But te was never explicitly assigned a value from
					// this function!". ::GetTableElement never assigned one, so neither
					// should we. With regards to generating data output, we must strive
					// to be as consistent as possible with the old behavior.
					te.m_pRow = ptr;
					te.m_pColumn = &tc;
					
				}

				if(te.IsLabel()) {
					// (z.manning 2010-04-13 17:12) - PLID 38175 - Handle labels separately
					SetTableValue(pDetail, i, j, g_cvarNull, EMR_TABLE_CELL_GRAY);
					bRowHasData = true;
				}
				else {
					// (c.haag 2008-10-17 09:42) - PLID 31709 - Assign the value to the datalist
					//TES 10/12/2012 - PLID 39000 - If this is a calculated formula whose calculation included unused numbers,
					// set the background to red
					OLE_COLOR nCellColor = te.IsCalculated() ? (te.m_bFormulaHasUnusedNumbers?EMR_TABLE_CELL_RED:EMR_TABLE_CELL_GRAY) : RGB(255,255,255);
					SetTableValue(pDetail, i, j, FormatValueForDataList(pDetail, i, j, te.GetValueAsVariant()), nCellColor, te.m_bFormulaHasUnusedNumbers);

					// (a.walling 2011-03-21 12:34) - PLID 42962  - If the row has any non-empty data, we will show it.
					if (bFilteredView) {
						//(e.lally 2011-12-08) PLID 46471 - We're no longer looking for any non-empty value
						//	Since this is a current med or allergy table, we're only concerned with the official usage col checkbox and if it is marked
						if (!bRowHasData && te.m_bChecked && te.m_pColumn->m_bIsCurrentMedOrAllergyUsageCol) {
							bRowHasData = true;
						}
					}
				}
			}
		} // for(int j=0; j<nDetailColumns; j++) {

		// (a.walling 2011-03-21 12:34) - PLID 42962
		if (bFilteredView && bRowHasData) {
			rowsToShow.Add(LookupDatalistRowFromTableRowIndex(FIRST_DL_DATA_ROW + i));
		}
	} // for (i=0; i<nDetailRows; i++) {
	
	// (a.walling 2011-03-21 12:34) - PLID 42962 - Show all rows with data
	for (int r = 0; r < rowsToShow.GetSize(); r++) {
		IRowSettingsPtr pThisRow = rowsToShow[r];

		if (pThisRow) {
			pThisRow->PutVisible(VARIANT_TRUE);
		}
	}

	// (c.haag 2011-03-18) - PLID 42891 - Now add the common list buttons
	AddCommonListButtons(pDetail);
}

// (c.haag 2011-03-18) - PLID 42891 - Ensures all the common list buttons are present. This is called from within EnsureTableObject.
void CEmrItemAdvTableBase::AddCommonListButtons(CEMNDetail* pDetail)
{
	// Don't add buttons if it's not a system table
	if (!pDetail->IsCurrentMedicationsTable() && !pDetail->IsAllergiesTable()) {
		return;
	}

	const CEmrInfoCommonListCollection& commonLists = pDetail->GetCommonLists();
	int nLists = commonLists.GetListCount();
	int i;

	// We're not sizing the buttons here; we're simply creating them.
	for (i=0; i < nLists; i++) 
	{
		CEmrInfoCommonList list = commonLists.GetListByIndex(i);
		if (!list.IsInactive() && list.GetItemCount() > 0) 
		{
			CNxIconButton* pBtn = new CNxIconButton();
			// (a.walling 2011-11-11 11:11) - PLID 46627 - Just create windows with an empty rect initially rather than a 1x1 rect
			pBtn->Create(list.GetName(), WS_CHILD|WS_VISIBLE|BS_CENTER|BS_PUSHBUTTON|BS_OWNERDRAW, CRect(0,0,0,0), m_wndTable.GetParent(), IDC_FIRST_COMMON_LIST_BUTTON + i + 1);
			// (a.walling 2011-11-11 11:11) - PLID 46621 - Unify font handing for emr items - This never even worked beforehand (was using GetFont, which was returning the icky system font)
			pBtn->SetFont(CFont::FromHandle(EmrFonts::GetRegularFont()));
			pBtn->SetTextColor(list.GetColor());
			if (pDetail->GetReadOnly() || (NULL != m_Table && VARIANT_FALSE != m_Table->GetReadOnly())) {
				pBtn->EnableWindow(FALSE);
			}
			m_apCommonListButtons.Add(pBtn);
		}
	}
	// (c.haag 2011-03-31) - PLID 43062 - Add the "All" button
	{
		CNxIconButton* pBtn = new CNxIconButton();
		// (a.walling 2011-11-11 11:11) - PLID 46627 - Just create windows with an empty rect initially rather than a 1x1 rect
		pBtn->Create("All", WS_CHILD|WS_VISIBLE|BS_CENTER|BS_PUSHBUTTON|BS_OWNERDRAW, CRect(0,0,0,0), m_wndTable.GetParent(), IDC_FIRST_COMMON_LIST_BUTTON);
		// (a.walling 2011-11-11 11:11) - PLID 46621 - Unify font handing for emr items - This never even worked beforehand (was using GetFont, which was returning the icky system font)
		pBtn->SetFont(CFont::FromHandle(EmrFonts::GetRegularFont()));
		pBtn->SetTextColor(RGB(0,0,0));
		if (pDetail->GetReadOnly() || (NULL != m_Table && VARIANT_FALSE != m_Table->GetReadOnly())) {
			pBtn->EnableWindow(FALSE);
		}
		m_apCommonListButtons.Add(pBtn);
	}

	// If m_rcTableAndCommonButtonsArea is valid, then resize the buttons here
	if (m_rcTableAndCommonButtonsArea.Width() > 0 &&
		m_rcTableAndCommonButtonsArea.Height() > 0)
	{
		ResizeCommonListButtons();
		// (c.haag 2011-03-18) - PLID 42906 - Redraw the buttons
		for (i=0; i < m_apCommonListButtons.GetSize(); i++) 
		{
			CNxIconButton* pBtn = m_apCommonListButtons[i];
			pBtn->RedrawWindow(NULL, NULL, RDW_INVALIDATE);
		}
	}
	else {
		// If we get here, we're probably opening for the first time. We're relying on
		// the buttons getting resized in RepositionControls.
	}
}

// (c.haag 2011-03-18) - PLID 42906 - Called to resize the common list buttons
void CEmrItemAdvTableBase::ResizeCommonListButtons()
{
	const CSize szButton = GetCommonListButtonSize();
	const CSize szBuffer = GetCommonListButtonBufferSize();
	const CSize szCommonButtons = CalculateCommonButtonRegionSize();
	int i;
	if (CSize(0,0) == szCommonButtons) {		
		// Either there are no buttons, or the EMR item area is too small to fit the buttons. Make sure they're hidden
		for (i=0; i < m_apCommonListButtons.GetSize(); i++) 
		{
			CNxIconButton* pBtn = m_apCommonListButtons[i];
			if (pBtn->IsWindowVisible()) {
				pBtn->ShowWindow(SW_HIDE);
			}
		}
		return;
	}
	const int nButtonsPerRow = (int)( (float)szCommonButtons.cx / (float)(szButton.cx + szBuffer.cx) );

	for (i=0; i < m_apCommonListButtons.GetSize(); i++) 
	{
		CNxIconButton* pBtn = m_apCommonListButtons[i];
		int x = m_rcTableAndCommonButtonsArea.left + (szButton.cx + szBuffer.cx) * (i % nButtonsPerRow);
		int y = m_rcTableAndCommonButtonsArea.top + (szButton.cy + szBuffer.cy) * (i / nButtonsPerRow);
		if (!pBtn->IsWindowVisible()) {
			pBtn->ShowWindow(SW_SHOWNA); // Button may have been hidden earlier. Show it now.
		}
		pBtn->MoveWindow(x,y,szButton.cx,szButton.cy,FALSE);		
	}
}

// (c.haag 2008-10-16 17:40) - PLID 31709 - Sets the combo source of a detail column. This will behave
// differently depending on whether this is a flipped table or not.
void CEmrItemAdvTableBase::SetComboSource(CEMNDetail* pDetail, long nDetailColumn, const CString& strSourceText)
{
	if (NULL != m_Table) {
		if (pDetail->m_bTableRowsAsFields) {
			// Flipped table
			IRowSettingsPtr pRow = LookupDatalistRowFromTableRowIndex(FIRST_DL_DATA_ROW + nDetailColumn);
			IFormatSettingsPtr pFormat = pRow->FormatOverride;
			pFormat->PutComboSource(_bstr_t(strSourceText));
		} else {
			// Normal handling
			//update using the passed-in combo sql
			m_Table->GetColumn(FIRST_DL_DATA_COLUMN + (short)nDetailColumn)->PutComboSource(_bstr_t(strSourceText));
		}
	}
}

// (c.haag 2008-10-16 17:45) - PLID 31709 - Sets the max embedded combo height of a detail column. This
// will behave differently depending on whether this is a flipped table or not.
void CEmrItemAdvTableBase::SetEmbeddedComboDropDownMaxHeight(CEMNDetail* pDetail, long nDetailColumn, long nHeight)
{
	if (NULL == m_Table) {
		return;
	}
	if (pDetail->m_bTableRowsAsFields) {
		// Flipped table
		IRowSettingsPtr pRow = LookupDatalistRowFromTableRowIndex(FIRST_DL_DATA_ROW + nDetailColumn);
		IFormatSettingsPtr pFormat = pRow->FormatOverride;
		pFormat->EmbeddedComboDropDownMaxHeight = nHeight;
	} else {
		// Normal handling
		//update using the passed-in combo sql
		m_Table->GetColumn(FIRST_DL_DATA_COLUMN + (short)nDetailColumn)->EmbeddedComboDropDownMaxHeight = nHeight;
	}
}

// (c.haag 2008-10-17 09:18) - PLID 31709 - Assigns a value to the table datalist
void CEmrItemAdvTableBase::SetTableValue(CEMNDetail* pDetail, long nDetailRow, long nDetailColumn, const _variant_t& v)
{
	SetTableValue(pDetail, nDetailRow, nDetailColumn, v, RGB(255,255,255));
}

// (z.manning 2010-04-13 16:56) - PLID 38175 - Added an overload that take a cell background color
//TES 10/12/2012 - PLID 39000 - Added an optional parameter for whether this is a calculated cell that includes unused numbers in its calculation.
void CEmrItemAdvTableBase::SetTableValue(CEMNDetail* pDetail, long nDetailRow, long nDetailColumn, const _variant_t& v, OLE_COLOR nCellBackColor, BOOL bFormulaHasUnusedNumbers /*= FALSE*/)
{
	if (NULL == m_Table) {
		ASSERT(FALSE);
		return;
	}

	TableRow *pTableRow = pDetail->GetRowPtr(nDetailRow);
	TableColumn *pTableColumn = pDetail->GetColumnPtr(nDetailColumn);
	BOOL bIsCalculated = (pTableRow->IsCalculated() || pTableColumn->IsCalculated());

	IRowSettingsPtr pRow;
	short nCol;
	if (pDetail->m_bTableRowsAsFields) {
		// Flipped table
		pRow = LookupDatalistRowFromTableRowIndex(FIRST_DL_DATA_ROW + nDetailColumn);
		nCol = (short)(FIRST_DL_DATA_COLUMN + nDetailRow);
	}
	else {
		// Normal handling
		pRow = LookupDatalistRowFromTableRowIndex(FIRST_DL_DATA_ROW + nDetailRow);
		nCol = (short)(FIRST_DL_DATA_COLUMN + nDetailColumn);
	}

	if(bIsCalculated && v.vt != VT_NULL && v.vt != VT_EMPTY) {
		// (z.manning 2012-04-03 16:23) - PLID 33710 - Special handling for calculated cells (mostly to handle displaying
		// them in dropdown columns).
		CEmrItemAdvTableBase::UpdateCalculatedCellValue(AsString(v), pDetail, nDetailRow, (short)nDetailColumn, this);
	}
	else {
		pRow->PutValue(nCol, v);
	}

	pRow->PutCellBackColor(nCol, nCellBackColor);
	//TES 10/12/2012 - PLID 39000 - Set the tooltip based on bFormulaHasUnusedNumbers
	pRow->PutCellToolTipOverride(nCol, bFormulaHasUnusedNumbers?EMR_UNUSED_NUMBERS_TOOLTIP:"");
}

// (c.haag 2008-10-17 10:13) - PLID 31700 - Returns TRUE if the table object is not null
BOOL CEmrItemAdvTableBase::IsTableControlValid() const
{
	return (NULL == m_Table) ? FALSE : TRUE;
}

// (c.haag 2008-10-17 10:08) - PLID 31700 - Called when this object is being destroyed
void CEmrItemAdvTableBase::ClearTableControl()
{
	if (NULL != m_Table) {
		// remove all rows
		m_Table->Clear();

		//DRT 7/29/2008 - PLID 30824 - Converted to datalist2, empty the maps
		ClearIndexDatalistMaps();

		// remove all columns
		for (int n = m_Table->GetColumnCount() - 1; n >= 0; n--) {
			m_Table->RemoveColumn(n);
		}
	}
	// (c.haag 2011-03-18) - PLID 42891 - Clear common list buttons
	ClearCommonListButtons();
}

// (c.haag 2011-03-18) - PLID 42891 - Clear common list buttons
void CEmrItemAdvTableBase::ClearCommonListButtons()
{
	for (int i=0; i < m_apCommonListButtons.GetSize(); i++)
	{
		if (NULL != m_apCommonListButtons[i]) {
			if (IsWindow(m_apCommonListButtons[i]->GetSafeHwnd()))
			{
				m_apCommonListButtons[i]->DestroyWindow();
			}
			delete m_apCommonListButtons[i];
		}
	}
	m_apCommonListButtons.RemoveAll();
}

// (c.haag 2008-10-17 10:08) - PLID 31700 - Set the read-only property of a table
void CEmrItemAdvTableBase::SetTableReadOnly(BOOL bReadOnly)
{
	if (NULL != m_Table) {
		m_Table->ReadOnly = bReadOnly ? _variant_t(VARIANT_TRUE, VT_BOOL) : _variant_t(VARIANT_FALSE, VT_BOOL);
	}
	SetCommonListButtonsReadOnly(bReadOnly); // (c.haag 2011-03-18) - PLID 42891
}

// (c.haag 2011-03-18) - PLID 42891 - Set common list buttons read only (called from SetTableReadOnly)
void CEmrItemAdvTableBase::SetCommonListButtonsReadOnly(BOOL bReadOnly)
{
	for (int i=0; i < m_apCommonListButtons.GetSize(); i++)
	{
		if (NULL != m_apCommonListButtons[i]) {
			if (IsWindow(m_apCommonListButtons[i]->GetSafeHwnd()))
			{
				m_apCommonListButtons[i]->EnableWindow( (bReadOnly) ? FALSE : TRUE );
			}
		}
	}
}

// (c.haag 2008-10-17 10:14) - PLID 31700 - Set the redraw property of a table
void CEmrItemAdvTableBase::SetTableRedraw(BOOL bRedraw)
{
	if (NULL != m_Table) {
		m_Table->SetRedraw(bRedraw ? _variant_t(VARIANT_TRUE, VT_BOOL) : _variant_t(VARIANT_FALSE, VT_BOOL));
	}
}

// (c.haag 2008-10-17 10:37) - PLID 31700 - Resets the stored column widths of a table. This is used
// as a workaround to hide the table's horizontal scrollbar if it doesn't need to be visible.
void CEmrItemAdvTableBase::ResetTableColumnWidths()
{
	// (j.jones 2006-08-09 09:28) - PLID 21859 - re-set the column widths to
	// force the datalist to realize it doesn't need a scrollbar
	if (NULL != m_Table) {
		for(int i=0; i<m_Table->GetColumnCount(); i++) {
			long nWidth = m_Table->GetColumn(i)->GetStoredWidth();
			m_Table->GetColumn(i)->PutStoredWidth(nWidth);
		}
	}
}

// (c.haag 2008-10-17 12:02) - PLID 31700 - Returns the table (datalist)'s row count
long CEmrItemAdvTableBase::GetTableRowCount()
{
	if (NULL != m_Table) {
		return m_Table->GetRowCount();
	} else {
		ASSERT(FALSE);
		return -1;
	}
}

// (c.haag 2008-10-17 12:02) - PLID 31700 - Returns the table (datalist)'s column count
short CEmrItemAdvTableBase::GetTableColumnCount()
{
	if (NULL != m_Table) {
		return m_Table->GetColumnCount();
	} else {
		ASSERT(FALSE);
		return -1;
	}
}

// (c.haag 2008-10-17 12:05) - PLID 31700 - Begins a table edit
// (a.walling 2011-05-11 14:21) - PLID 42962 - return a BOOL instead, ignores non-visible rows
BOOL CEmrItemAdvTableBase::StartTableEditing(NXDATALIST2Lib::IRowSettingsPtr pRow, short nCol)
{
	if (pRow && m_Table) {
		if (pRow->GetVisible()) {
			return m_Table->StartEditing(pRow, nCol) ? TRUE : FALSE;
		} else {
			return FALSE;
		}
	} else {
		ASSERT(FALSE);
		return FALSE;
	}
}

// (c.haag 2008-10-17 12:03) - PLID 31700 - Stops a table edit in progress
HRESULT CEmrItemAdvTableBase::StopTableEditing(VARIANT_BOOL vbCommit)
{
	if (NULL != m_Table) {
		return m_Table->StopEditing(vbCommit);		
	} else {
		ASSERT(FALSE);
		return E_NOINTERFACE;
	}
}

// (c.haag 2008-10-17 12:11) - PLID 31700 - Sets the current row
void CEmrItemAdvTableBase::SetTableCurSel(struct NXDATALIST2Lib::IRowSettings * lpRow)
{
	if (NULL != m_Table) {
		m_Table->PutCurSel(lpRow);
	} else {
		ASSERT(FALSE);
	}
}

// (c.haag 2008-10-17 12:23) - PLID 31700 - Gets the stored width
long CEmrItemAdvTableBase::GetTableColumnStoredWidth(short nCol)
{
	if (NULL != m_Table) {
		return m_Table->GetColumn(nCol)->StoredWidth;
	} else {
		ASSERT(FALSE);
		return -1;
	}
}

// (c.haag 2008-10-17 12:23) - PLID 31700 - Gets the column name
CString CEmrItemAdvTableBase::GetTableColumnTitle(short nCol)
{
	if (NULL != m_Table) {
		return AsString(m_Table->GetColumn(nCol)->GetColumnTitle());
	} else {
		ASSERT(FALSE);
		return "";
	}
}

// (j.jones 2009-10-02 11:59) - PLID 35161 - added ability to append data to a text field
void CEmrItemAdvTableBase::AppendTextToTableCell(LPDISPATCH lpRow, short nTableCol,
												 CEMNDetail* pDetail, CString strTextToAppend,
												 CWnd* pWnd, CWnd* pParentDlg, BOOL bIsPopupWnd)
{
	IRowSettingsPtr pRow(lpRow);

	if(pDetail == NULL || pRow == NULL || strTextToAppend.IsEmpty() || nTableCol == -1) {
		return;
	}

	int nTableRow = LookupTableRowIndexFromDatalistRow(pRow);
					
	long nDetailRow, nDetailCol;
	TablePositionToDetailPosition(pDetail, nTableRow, nTableCol, nDetailRow, nDetailCol);

	// (j.jones 2009-10-02 12:10) - PLID 35161 - this code is largely taken from HandleEditingFinishedTable,
	// but trimmed down to only handle appending valid text to an existing text cell

	//while we're not really in the EditingFinished handler, ReflectTableState is dependent
	//on these variables being set
	m_pEditingFinishedRow = lpRow;
	m_nEditingFinishedCol = nTableCol;
	EReflectCurrentStateDLHint rtshintOld = m_ReflectCurrentStateDLHint;

	TableElement te;
	pDetail->GetTableElement(&pDetail->GetRowPtr(nDetailRow)->m_ID, pDetail->GetColumn(nDetailCol).nID, te);

	_variant_t varOldState = pDetail->GetState();
	BOOL bChangedOtherColumns = FALSE;
	if(te.m_pColumn->bIsGrouped) {

		//clear out all other grouped columns
		for(int i=0;i<pDetail->GetColumnCount();i++) {
			if(i != nDetailCol) {
				// (a.walling 2011-08-24 12:35) - PLID 45171 - We only need to look for existing table elements
				TableElement teOther;
				pDetail->GetExistingTableElement(&pDetail->GetRowPtr(nDetailRow)->m_ID, pDetail->GetColumn(i).nID, teOther);

				if(teOther.m_pColumn && teOther.m_pColumn->bIsGrouped) {
					// (z.manning 2010-12-20 10:21) - PLID 41886 - Moved the clearing logic to its own function
					teOther.Clear();
					pDetail->SetTableElement(teOther);
					bChangedOtherColumns = TRUE;
				}
			}
		}
		m_ReflectCurrentStateDLHint = eRCS_GroupedColumnUpdate;
	}
	else {
		m_ReflectCurrentStateDLHint = eRCS_SingleCellUpdate;
	}

	//append our string
	CString strValue = te.GetValueAsString();
	if(!strValue.IsEmpty()) {
		strValue += " ";
	}
	strValue += strTextToAppend;

	// (c.haag 2008-01-15 17:50) - PLID 17936 - Before we pass in varNewValue into LoadValueFromVariant,
	// we must convert it from a datalist value into a table state data value.
	te.LoadValueFromVariant(UnformatDataListValue(pDetail, nDetailRow, (short)nDetailCol, (LPCTSTR)strValue), pDetail->m_pParentTopic->GetParentEMN());

	pDetail->SetTableElement(te);

	// (z.manning 2011-03-21 15:44) - PLID 30608 - Check to see if we need to autofill any columns in the
	// just edited row. We do not want to do this if they cleared out a cell, however.
	// Also, make sure we do this before generating the new state below.
	if(!strTextToAppend.IsEmpty()) {
		if(pDetail->UpdateAutofillColumns(&te)) {
			m_ReflectCurrentStateDLHint = eRCS_FullDatalistUpdate;
		}
	}
	
	// (z.manning 2010-12-20 10:45) - PLID 41866 - Moved the state change logic to its own function
	RequestTableStateChange(pDetail, varOldState, pWnd, pParentDlg, bIsPopupWnd);

	// (z.manning 2010-02-26 08:34) - PLID 37412 - Moved this code into its own function
	//TES 3/11/2010 - PLID 37535 - RequestStateChange() now handles calling UpdateTableDataOutput()
	//pDetail->UpdateTableDataOutput();

	m_ReflectCurrentStateDLHint = rtshintOld;
	m_pEditingFinishedRow = NULL;
	m_nEditingFinishedCol = -1;
}

void CEmrItemAdvTableBase::RequestTableStateChange(CEMNDetail *pDetail, _variant_t varOldState, CWnd *pWnd, CWnd *pParentDlg, BOOL bIsPopupWnd)
{
	CString strNewVarState = GenerateNewVarState();

	// (z.manning 2009-03-04 11:10) - PLID 33072 - Table states get recreated before
	// this is called, so we need to pass in the old state for spawning purposes.
	if (pDetail->RequestStateChange((LPCTSTR)strNewVarState, varOldState)) {
		if(bIsPopupWnd) {
			//TES 1/15/2008 - PLID 24157 - Notify our parent.
			pParentDlg->SendMessage(NXM_EMR_POPUP_POST_STATE_CHANGED, (WPARAM)this, (LPARAM)pDetail);
		}

		// (z.manning 2009-04-22 17:20) - PLID 33072 - This table may not even exist anymore
		// if, for example, it just unspawned the topic on which it was located.
		if(IsWindow(m_wndTable.GetSafeHwnd())) {
			CRect rc;
			m_wndTable.GetWindowRect(&rc);
			pWnd->ScreenToClient(&rc);
			// (b.cardillo 2006-02-22 10:57) - PLID 19376 - Since we now clip 
			// children, invalidating the child area is not good enough, so we 
			// have to actually tell each child to invalidate itself.  Also, we 
			// used to ask it to erase too, because according to an old comment 
			// related to plid 13344, the "SetRedraw" state might be false at 
			// the moment.  (I'm deleting that comment on this check-in, so to 
			// see it you'll have to look in source history.)  But I think the 
			// "SetRedraw" state no longer can be false, and even if it were I 
			// think now that we're invalidating the control itself, we wouldn't 
			// need to erase here anyway.
			m_wndTable.RedrawWindow(NULL, NULL, RDW_INVALIDATE);
		}
	}

	// (z.manning, 06/02/2008) - PLID 30155 - Move this call AFTER pDetail->RequestStateChange
	// because calculated fields do not get updated until it is called.
	if (bIsPopupWnd) {
		ReflectTableState(pWnd, pDetail, bIsPopupWnd);
	}
}

// (z.manning 2010-12-17 16:48) - PLID 41886
// (z.manning 2010-12-22 09:34) - PLID 41887 - Renamed this function as it now all appends copy options
// (z.manning 2011-05-27 09:02) - PLID 42131 - Renamed this function to AppendSharedMenuOptions
void CEmrItemAdvTableBase::AppendSharedMenuOptions(CMenu *pMenu, LPDISPATCH lpRow, const short nCol, CEMNDetail *pDetail)
{
	IRowSettingsPtr pRow(lpRow);
	if(pRow == NULL || nCol < 0) {
		return;
	}

	long nTableRowIndex = LookupTableRowIndexFromDatalistRow(lpRow);
	TableElement te = GetTableElementByTablePosition(pDetail, nTableRowIndex, nCol);

	UINT nCellOption, nRowOption, nColOption, nTableOption;
	nCellOption = nRowOption = nColOption = nTableOption = MF_GRAYED|MF_DISABLED;
	// (z.manning 2010-12-22 10:27) - PLID 41887 - Copy cell and row options
	UINT nCopyCellUpOption, nCopyCellDownOption, nCopyRowUpOption, nCopyRowDownOption;
	nCopyCellUpOption = nCopyCellDownOption = nCopyRowUpOption = nCopyRowDownOption = MF_GRAYED|MF_DISABLED;
	
	// (z.manning 2010-12-20 13:11) - PLID 41886 - Is the current cell editable?
	if(CanEditCell(pRow, nCol, pDetail)) {
		nCellOption = MF_ENABLED;
		nTableOption = MF_ENABLED;
	}

	// (z.manning 2010-12-20 13:11) - PLID 41886 - Is anything in the current row editable?
	// (z.manning 2010-12-20 11:39) - PLID 41886 - Skip column 0 (row name)
	const short nColCount = m_Table->GetColumnCount();
	for(short nColIndex = 1; nColIndex < nColCount; nColIndex++)
	{
		if(CanEditCell(pRow, nColIndex, pDetail)) {
			nRowOption = MF_ENABLED;
			nTableOption = MF_ENABLED;
			// (z.manning 2010-12-22 10:44) - PLID 41887 - We have at least one non-readonly column so allow
			// copying up and down on non-flipped tables.
			if(!pDetail->m_bTableRowsAsFields) {
				nCopyRowUpOption = nCopyRowDownOption = MF_ENABLED;
				if(!te.m_pColumn->IsReadOnly()) {
					nCopyCellUpOption = nCopyCellDownOption = MF_ENABLED;
				}
			}
			break;
		}
	}

	// (z.manning 2010-12-20 13:11) - PLID 41886 - Is anything in the current column editable?
	for(IRowSettingsPtr prowTemp = m_Table->GetFirstRow(); prowTemp != NULL; prowTemp = prowTemp->GetNextRow())
	{
		if(CanEditCell(prowTemp, nCol, pDetail)) {
			nColOption = MF_ENABLED;
			nTableOption = MF_ENABLED;
			// (z.manning 2010-12-22 10:44) - PLID 41887 - We have at least one non-readonly row so allow
			// copying left and right on flipped tables.
			if(pDetail->m_bTableRowsAsFields) {
				nCopyRowUpOption = nCopyRowDownOption = MF_ENABLED;
				if(!te.m_pColumn->IsReadOnly()) {
					nCopyCellUpOption = nCopyCellDownOption = MF_ENABLED;
				}
			}
			break;
		}
	}

	// (z.manning 2010-12-22 11:01) - PLID 41887 - Don't allow copying to non-existant rows (i.e. can't copy up from the top row).
	long nPosition, nMaxPos;
	if(!pDetail->m_bTableRowsAsFields) {
		nPosition = pRow->CalcRowNumber();
		nMaxPos = m_Table->GetRowCount() - 1;
	}
	else {
		// (z.manning 2010-12-22 11:59) - PLID 41887 - Subtract 1 more from each of these to account for row titles
		nPosition = nCol - 1;
		nMaxPos = m_Table->GetColumnCount() - 2;
	}
	if(nPosition == 0) {
		nCopyCellUpOption = nCopyRowUpOption = MF_GRAYED|MF_DISABLED;
	}
	if(nPosition >= nMaxPos) {
		nCopyCellDownOption = nCopyRowDownOption = MF_GRAYED|MF_DISABLED;
	}

	// (z.manning 2010-12-20 13:23) - PLID 41886 - Don't allow them to ever mass-update allergies and current meds tables.
	BOOL bAllowTable = TRUE, bAllowRow = TRUE, bAllowColumn = TRUE;
	if(pDetail->IsAllergiesTable() || pDetail->IsCurrentMedicationsTable()) {
		bAllowTable = FALSE;
		if(pDetail->m_bTableRowsAsFields) {
			bAllowRow = FALSE;
		}
		else {
			bAllowColumn = FALSE;
		}
	}

	// (z.manning 2010-12-22 11:05) - PLID 41887 - Copy options
	CString strUpWord = "Up";
	CString strDownWord = "Down";
	CString strRowWord = "Row";
	if(pDetail->m_bTableRowsAsFields) {
		strUpWord = "Left";
		strDownWord = "Right";
		strRowWord = "Column";
	}
	pMenu->AppendMenu(nCopyRowDownOption, IDM_COPY_TABLE_ROW_DOWN, "Cop&y " + strRowWord + " " + strDownWord);
	pMenu->AppendMenu(nCopyRowUpOption, IDM_COPY_TABLE_ROW_UP, "C&opy " + strRowWord + " " + strUpWord);
	pMenu->AppendMenu(nCopyCellDownOption, IDM_COPY_TABLE_CELL_DOWN, "Copy Ce&ll " + strDownWord);
	pMenu->AppendMenu(nCopyCellUpOption, IDM_COPY_TABLE_CELL_UP, "Copy C&ell " + strUpWord);

	pMenu->AppendMenu(MF_SEPARATOR);

	pMenu->AppendMenu(nCellOption, IDM_CLEAR_TABLE_CELL, "Clea&r Cell");
	if(bAllowRow) {
		pMenu->AppendMenu(nRowOption, IDM_CLEAR_TABLE_ROW, "Clear Ro&w");
	}
	if(bAllowColumn) {
		pMenu->AppendMenu(nColOption, IDM_CLEAR_TABLE_COLUMN, "Clear Colu&mn");
	}
	if(bAllowTable) {
		pMenu->AppendMenu(nTableOption, IDM_CLEAR_TABLE, "Clear Ta&ble");
	}

	// (z.manning 2011-05-27 09:10) - PLID 42131 - Menu option to apply transformation to a row
	if(pDetail->HasTransformFormulas())
	{
		pMenu->AppendMenu(MF_SEPARATOR);
		pMenu->AppendMenu(MF_ENABLED, IDM_TRANSFORM_ROW, "Apply &Transformation to " + strRowWord);
	}

	// (z.manning 2011-07-14 12:00) - PLID 44469 - See if the current row is tied to a coding group.
	CEmrCodingGroup *pCodingGroup = GetCodingGroupFromRow(te.m_pRow);
	if(pCodingGroup != NULL && pDetail->GetParentEMN() != NULL)
	{
		// (z.manning 2011-07-14 12:01) - PLID 44469 - Now see if the coding group we found is in use on this EMN
		// and if so give an option to pop up a dialog to modify the coding group.
		CEmnCodingGroupInfo *pEmnCodingInfo = pDetail->GetParentEMN()->GetCodingGroupInfoArray()->FindByCodingGroupID(pCodingGroup->GetID());
		if(pEmnCodingInfo->m_nGroupQuantity > 0) {
			pMenu->AppendMenu(MF_SEPARATOR);
			CString strGroupName = pCodingGroup->GetName();
			strGroupName.Replace("&", "&&");
			pMenu->AppendMenu(MF_ENABLED, IDM_EDIT_CODING_GROUP, FormatString("Modify %s Coding &Group", strGroupName));
		}
	}
}

// (z.manning 2011-07-14 10:38) - PLID 44469
CEmrCodingGroup* CEmrItemAdvTableBase::GetCodingGroupFromRow(TableRow *ptr)
{
	// (c.haag 2012-10-26) - PLID 53440 - Use the new getter functions
	if(ptr->m_ID.GetImageStampID() != -1)
	{
		// (z.manning 2011-07-14 11:58) - PLID 44469 - This row is associated with a stamp. Let's go through all the actions
		// for this stamp and see if there are any charge actions for a charge that's also part of a coding group and if so
		// return that coding group object.
		EMRImageStamp *pStamp = GetMainFrame()->GetEMRImageStampByID(ptr->m_ID.GetImageStampID());
		for(int nActionIndex = 0; nActionIndex < pStamp->aryActions.GetCount(); nActionIndex++)
		{
			EmrAction ea = pStamp->aryActions.GetAt(nActionIndex);
			if(ea.eaoDestType == eaoCpt)
			{
				CEmrCodingGroup *pCodingGroup = GetMainFrame()->GetEmrCodingGroupManager()->GetCodingGroupByCptID(ea.nDestID);
				if(pCodingGroup != NULL) {
					// (z.manning 2011-07-14 10:42) - PLID 44469 - It's possible to have a stamp with actions for more than one
					// coding group, but for now we'll just return the first one and be done with it.
					return pCodingGroup;
				}
			}
		}
	}

	return NULL;
}

// (z.manning 2011-07-14 10:45) - PLID 44469
void CEmrItemAdvTableBase::OpenChargePromptDialogForRow(CEMNDetail *pDetail, LPDISPATCH lpRow, const short nCol)
{
	IRowSettingsPtr pRow(lpRow);
	if(pRow == NULL) {
		return;
	}

	long nTableRowIndex = LookupTableRowIndexFromDatalistRow(lpRow);
	TableElement te = GetTableElementByTablePosition(pDetail, nTableRowIndex, nCol);

	// (z.manning 2011-07-14 11:55) - PLID 44469 - Find the coding group correspdonding to this row
	// and then open the charge prompt dialog for any codes in that group.
	CEmrCodingGroup *pCodingGroup = GetCodingGroupFromRow(te.m_pRow);
	if(pCodingGroup != NULL && pDetail->GetParentEMN() != NULL)
	{
		pDetail->GetParentEMN()->UpdateCodingGroup(pCodingGroup, TRUE, 0);
	}
}

// (z.manning 2010-12-17 17:15) - PLID 41886 - Move the logic from HandleEditingStarting to here
BOOL CEmrItemAdvTableBase::CanEditCell(LPDISPATCH lpRow, const short nCol, CEMNDetail *pDetail)
{
	IRowSettingsPtr pRow(lpRow);
	if(pRow == NULL || nCol < 0) {
		return FALSE;
	}

	long nTableRowIndex = LookupTableRowIndexFromDatalistRow(lpRow);
	TableElement te = GetTableElementByTablePosition(pDetail, nTableRowIndex, nCol);
	
	if(te.IsReadOnly()) {
		return FALSE;
	}

	// (j.jones 2011-05-04 15:52) - PLID 43527 - Special handling for the current medications table,
	// if this column is the Sig column and the medication in question is from NewCrop, the Sig
	// is read-only. Since it is not visually tagged as such, we have to explain why they can't edit.
	if(pDetail->IsCurrentMedicationsTable()) {

		//this is a current medications table, find out if this is the Sig column
	
		long nDetailRow, nDetailCol;
		TablePositionToDetailPosition(pDetail, LookupTableRowIndexFromDatalistRow(pRow), nCol, nDetailRow, nDetailCol);
		TableColumn tc = pDetail->GetColumn(nDetailCol);

		if(pDetail->IsCurrentMedicationsTable() && tc.nSubType == lstCurrentMedicationSig) {
			//this is the Sig column, now we need to see if this medication is from NewCrop
			long nDataID = te.m_pRow->m_ID.nDataID;
			
			//unfortunately, we have to run a recordset to confirm whether this is a NewCrop medication
			if(ReturnsRecordsParam("SELECT CurrentPatientMedsT.ID "
				"FROM CurrentPatientMedsT "
				"LEFT JOIN DrugList ON DrugList.ID = CurrentPatientMedsT.MedicationID "
				"LEFT JOIN EMRDataT ON DrugList.EMRDataID = EMRDataT.ID "
				"WHERE NewCropGUID Is Not Null AND EMRDataT.ID = {INT} AND CurrentPatientMedsT.PatientID = {INT}", nDataID, pDetail->GetPatientID())) {

				//this is a NewCrop medication, they cannot edit the Sig
				MessageBox(GetActiveWindow(), "This medication was added through E-Prescribing and cannot have its Sig modified in Practice.", "Practice", MB_ICONEXCLAMATION|MB_OK);
				return FALSE;
			}
		}
	}

	return TRUE;
}

// (z.manning 2010-12-20 13:12) - PLID 41886
void CEmrItemAdvTableBase::ClearTable(EClearTableType eClearType, LPDISPATCH lpRow, const short nCol, CEMNDetail *pDetail, CWnd *pwnd, CWnd *pParentDlg, BOOL bIsPopupWnd)
{
	// (z.manning 2010-12-20 13:12) - PLID 41886 - Make sure the detail is not read-only.
	if(pDetail->GetReadOnly()) {
		return;
	}

	// (z.manning 2010-12-20 13:12) - PLID 41886 - Confirm we have a valid cell
	IRowSettingsPtr pRow(lpRow);
	if(pRow == NULL || nCol < 0) {
		return;
	}

	const short nColCount = m_Table->GetColumnCount();
	_variant_t varOldState = pDetail->GetState();

	long nTableRowIndex = LookupTableRowIndexFromDatalistRow(lpRow);

	// (z.manning 2010-12-20 13:13) - PLID 41886 - Depending on what we're clearing, go through and add
	// the relevant table elements to an array.
	CArray<TableElement,TableElement&> aryElementsToClear;
	switch(eClearType)
	{
		case cttCell:
			{
				TableElement teTemp = GetTableElementByTablePosition(pDetail, nTableRowIndex, nCol);
				if(!teTemp.IsEmpty() && !teTemp.IsReadOnly()) {
					aryElementsToClear.Add(teTemp);
				}
			}
			break;

		case cttRow:
			// (z.manning 2010-12-20 11:39) - PLID 41886 - Skip column 0 (row name)
			for(short nTempCol = 1; nTempCol < nColCount; nTempCol++) {
				TableElement teTemp = GetTableElementByTablePosition(pDetail, nTableRowIndex, nTempCol);
				if(!teTemp.IsEmpty() && !teTemp.IsReadOnly()) {
					aryElementsToClear.Add(teTemp);
				}
			}
			break;

		case cttColumn:
			for(IRowSettingsPtr prowTemp = m_Table->GetFirstRow(); prowTemp != NULL; prowTemp = prowTemp->GetNextRow()) {
				long nTempIndex = LookupTableRowIndexFromDatalistRow(prowTemp);
				TableElement teTemp = GetTableElementByTablePosition(pDetail, nTempIndex, nCol);
				if(!teTemp.IsEmpty() && !teTemp.IsReadOnly()) {
					aryElementsToClear.Add(teTemp);
				}
			}
			break;

		case cttTable:
			for(int nElementIndex = 0; nElementIndex < pDetail->GetTableElementCount(); nElementIndex++) {
				TableElement teTemp;
				if(pDetail->GetTableElementByIndex(nElementIndex, teTemp)) {
					if(!teTemp.IsEmpty() && !teTemp.IsReadOnly()) {
						aryElementsToClear.Add(teTemp);
					}
				}
			}
			break;
	}

	if(aryElementsToClear.GetSize() == 0) {
		// (z.manning 2010-12-20 11:33) - PLID 41886 - We didn't actually change anything
		return;
	}

	if(eClearType == cttTable) {
		// (z.manning 2010-12-20 13:13) - PLID 41886 - Mandatory warning when clearing an entire table
		int nResult = pwnd->MessageBox("Are you sure you want to clear all data from this table?\r\n\r\nThis cannot be undone.", NULL, MB_YESNO|MB_ICONQUESTION);
		if(nResult != IDYES) {
			return;
		}
	}
	else if(eClearType == cttRow || eClearType == cttColumn) {
		// (z.manning 2010-12-20 13:16) - PLID 41886 - Don't show me warning when clearing a row or column
		CString strType = (eClearType == cttRow ? "row" : "column");
		CString strWarning = FormatString("Are you sure you want to clear all data from this %s?\r\n\r\nThis cannot be undone.", strType);
		if(DontShowMeAgain(pwnd, strWarning, "EmrClearTableRowOrColumn", "", FALSE, TRUE) == IDNO) {
			return;
		}
	}

	// (z.manning 2010-12-20 13:16) - PLID 41886 - Now let's actually go through and clear out these table elements.
	for(int nElementIndex = 0; nElementIndex < aryElementsToClear.GetSize(); nElementIndex++)
	{
		TableElement teTemp = aryElementsToClear.GetAt(nElementIndex);
		teTemp.Clear();
		// (z.manning 2010-12-20 11:34) - PLID 41886 - Pass in false for recreate state from content and we'll
		// just do it once later.
		pDetail->SetTableElement(teTemp, TRUE, FALSE);
	}
	pDetail->RecreateStateFromContent();

	EReflectCurrentStateDLHint rtshintOld = m_ReflectCurrentStateDLHint;

	m_ReflectCurrentStateDLHint = eRCS_FullDatalistUpdate;
	RequestTableStateChange(pDetail, varOldState, pwnd, pParentDlg, bIsPopupWnd);

	m_ReflectCurrentStateDLHint = rtshintOld;
}

// (z.manning 2010-12-22 13:46) - PLID 41887
void CEmrItemAdvTableBase::CopyAndPasteTableRowToAdjacentRow(LPDISPATCH lpRowSource, const short nCol, CEMNDetail *pDetail, BOOL bCellOnly, BOOL bCopyForward, CWnd *pwnd, CWnd *pParentDlg, BOOL bIsPopupWnd)
{
	// (z.manning 2010-12-20 13:12) - PLID 41887 - Confirm we have a valid cell
	IRowSettingsPtr pSourceRow(lpRowSource);
	if(pSourceRow == NULL || nCol < 0) {
		return;
	}

	// (z.manning 2010-12-22 17:58) - PLID 41887 - First copy the elements in the source row
	CArray<TableElement,TableElement&> aryElementsToCopy;
	CopyTableRow(lpRowSource, nCol, pDetail, bCellOnly, aryElementsToCopy);

	// (z.manning 2010-12-22 17:59) - PLID 41887 - Now find the destination TableRow pointer (which is actually
	// a column in flipped tables, which is what we want).
	TableRow *pDestTableRow = NULL;
	if(!pDetail->m_bTableRowsAsFields)
	{
		// (z.manning 2010-12-22 14:57) - PLID 41887 - Non-flipped tables: get the next or previous row
		IRowSettingsPtr pDestRow = bCopyForward ? pSourceRow->GetNextRow() : pSourceRow->GetPreviousRow();
		if(pDestRow != NULL) {
			long nDestRowIndex = LookupTableRowIndexFromDatalistRow(pDestRow);
			TableElement te = GetTableElementByTablePosition(pDetail, nDestRowIndex, nCol);
			pDestTableRow = te.m_pRow;
		}
	}
	else
	{
		// (z.manning 2010-12-22 14:57) - PLID 41887 - Flipped tables: get the next or previous column
		short nDestCol = bCopyForward ? nCol + 1 : nCol - 1;
		// (z.manning 2010-12-22 15:07) - PLID 41887 - Column 0 is not valid to paste to because that contains
		// the row names.
		if(nDestCol > 0 && nDestCol < m_Table->GetColumnCount()) {
			long nSourceRowIndex = LookupTableRowIndexFromDatalistRow(pSourceRow);
			TableElement te = GetTableElementByTablePosition(pDetail, nSourceRowIndex, nDestCol);
			pDestTableRow = te.m_pRow;
		}
	}
	
	if(pDestTableRow != NULL) {
		// (z.manning 2010-12-22 18:00) - PLID 41887 - Now let's actually paste the contents in the specified row
		// (z.manning 2011-04-04 17:24) - PLID 30608 - If we are only updating a single cell then let's update
		// any auto-fill columns. If we're pasting an entire row then we'll want to paste it all as is.
		PasteTableRow(aryElementsToCopy, pDestTableRow, pDetail, pwnd, pParentDlg, bIsPopupWnd, bCellOnly);
	}
}

// (z.manning 2010-12-22 13:46) - PLID 41887
void CEmrItemAdvTableBase::PasteTableRow(CArray<TableElement,TableElement&> &aryElementsToPaste, TableRow *pDestRow, CEMNDetail *pDetail, CWnd *pwnd, CWnd *pParentDlg, BOOL bIsPopupWnd, BOOL bUpdateAutofillColumns)
{
	// (z.manning 2010-12-20 13:12) - PLID 41887 - Make sure the detail is not read-only.
	if(pDetail->GetReadOnly()) {
		return;
	}

	_variant_t varOldState = pDetail->GetState();

	// (z.manning 2010-12-20 13:16) - PLID 41886 - Now let's actually go through and clear out these table elements.
	for(int nElementIndex = 0; nElementIndex < aryElementsToPaste.GetSize(); nElementIndex++)
	{
		TableElement teSource = aryElementsToPaste.GetAt(nElementIndex);
		// (z.manning 2010-12-22 15:15) - PLID 41887 - We do not permit pasting into read-only cells.
		if(!pDestRow->IsReadOnly() && !teSource.m_pColumn->IsReadOnly())
		{
			// (z.manning 2010-12-22 15:18) - PLID 41887 - Copy the contents of the source element to the 
			// destination element
			TableElement teDest = teSource;
			teDest.m_pRow = pDestRow;
			// (z.manning 2010-12-20 11:34) - PLID 41887 - Pass in false for recreate state from content and we'll
			// just do it once later.
			pDetail->SetTableElement(teDest, TRUE, FALSE);

			// (z.manning 2011-04-04 17:25) - PLID 30608 - Handle any auto-filling we may need to do.
			if(bUpdateAutofillColumns && !teDest.IsEmpty()) {
				pDetail->UpdateAutofillColumns(&teDest);
			}
		}
	}
	pDetail->RecreateStateFromContent();

	EReflectCurrentStateDLHint rtshintOld = m_ReflectCurrentStateDLHint;

	m_ReflectCurrentStateDLHint = eRCS_FullDatalistUpdate;
	RequestTableStateChange(pDetail, varOldState, pwnd, pParentDlg, bIsPopupWnd);

	m_ReflectCurrentStateDLHint = rtshintOld;
}

// (z.manning 2010-12-22 12:46) - PLID 41887
void CEmrItemAdvTableBase::CopyTableRow(LPDISPATCH lpRow, const short nCol, CEMNDetail *pDetail, BOOL bCellOnly, OUT CArray<TableElement,TableElement&> &aryCopiedElements)
{
	aryCopiedElements.RemoveAll();

	// (z.manning 2010-12-20 13:12) - PLID 41887 - Confirm we have a valid cell
	IRowSettingsPtr pRow(lpRow);
	if(pRow == NULL || nCol < 0) {
		return;
	}

	long nTableRowIndex = LookupTableRowIndexFromDatalistRow(lpRow);
	TableElement teSource = GetTableElementByTablePosition(pDetail, nTableRowIndex, nCol);

	if(bCellOnly) {
		// (z.manning 2010-12-22 18:01) - PLID 41887 - We only want to copy the current cell
		aryCopiedElements.Add(teSource);
	}
	else {
		// (z.manning 2010-12-22 18:01) - PLID 41887 - Copy the entire row
		for(int nColIndex = 0; nColIndex < pDetail->GetColumnCount(); nColIndex++) {
			TableColumn *ptc = pDetail->GetColumnPtr(nColIndex);
			TableElement teCurrent;
			TableElement *pteTemp = pDetail->GetTableElementByRowColPtr(teSource.m_pRow, ptc);
			if(pteTemp != NULL) {
				// (z.manning 2010-12-22 18:01) - PLID 41887 - We found an existing element for this row and column
				// so copy its contents exactly.
				teCurrent = *pteTemp;
			}
			else {
				// (z.manning 2010-12-22 18:02) - PLID 41887 - There is not an element for this row/column. However,
				// we must still copy this element so that we know that it's blank when pasting it, so just use the
				// blank TableElement object with the current row and column.
				teCurrent.m_pRow = teSource.m_pRow;
				teCurrent.m_pColumn = ptc;
			}
			aryCopiedElements.Add(teCurrent);
		}
	}
}

// (z.manning 2011-03-11) - PLID 42778 - There is now only 1 copy of this function
// (z.manning 2011-10-11 11:01) - PLID 42061 - Added stamp ID parameter
CString CEmrItemAdvTableBase::GetDropdownSource(long nColumnID, CEMNDetail *pDetail, BOOL bOnEmn, long nStampID)
{
	// (c.haag 2008-01-15 17:41) - PLID 17936 - This function returns the source
	// string for a dropdown column given a column ID. The string is generated
	// from information stored inside a member map where the key is the column ID.
	// (c.haag 2008-03-20 17:21) - PLID 17936 - We cannot use a straight combo source
	// string because users may put semi-colons in data. The code now returns a SQL
	// statement to pass to the embedded combo.
	//
	// (z.manning 2009-03-18 15:47) - PLID 33576 - The solution to the semicolon problem
	// was very inefficient (we now query column data twice). I am changing this back to 
	// a straight combo source and will fix that datalist to handle semicolons somehow.
	//
	if (NULL == pDetail) {
		ASSERT(FALSE);
		ThrowNxException("Called CEmrItemAdvTableBase::GetDropdownSource with a NULL value for pDetail!");
	}

	TableColumn *ptc = pDetail->GetColumnByID(nColumnID);

	// First, look up the dropdown source info object from our source map for the given column.
	// The object will tell us what the combo source would look like if we disregard sentinel
	// values.
	TableDropdownSourceInfo* pInfo = NULL;
	if (!m_mapDropdownSourceInfo.Lookup(nColumnID, pInfo))
	{
		// If we get here, it doesn't exist. Create it now.
		pInfo = new TableDropdownSourceInfo;
		pInfo->nComboDataItems = 0;
		m_mapDropdownSourceInfo.SetAt(nColumnID, pInfo);

		// (c.haag 2008-01-29 12:07) - PLID 28686 - Option for adding additional dropdown items.
		// (c.haag 2008-02-18 14:56) - If m_pRealDetail is NULL, that means we are not being popped up from an EMN. There is no real
		// detail; therefore, we cannot invoke the EMR item entry dialog. Therefore, we can't implement <Add New>.
		// (z.manning 2011-03-16 09:46) - PLID 42618 - Only show this on dropdown type columns.
		if (bOnEmn && ptc->nType == LIST_TYPE_DROPDOWN) {
			pInfo->aryComboItems.Add(TableDropdownItem(m_nAddNewDropdownSentinelValue, "<Add New>", TRUE));
		}

		long nActiveCount = 0;
		CTableDropdownItemArray aryDataItems;
		
		// (z.manning 2011-03-11) - PLID 42778 - Only query data if we know this column has dropdown elements.
		if(ptc->ShouldLoadDropdownElementsFromData())
		{
			// (z.manning 2011-10-12 14:59) - PLID 45728 - Clear stamp dropdown default info as we are about to reload it.
			ptc->ClearStampDefaultDropdownInfo();

			// (a.walling 2013-07-23 21:13) - PLID 57685 - CEmrItemAdvTableBase::GetDropdownSource can be refactored to use Nx::Cache for significant performance improvements

			CNxPerform nxp(__FUNCTION__" - Nx::Cache");
			using namespace Nx;

			sqlite::Connection& sql = Nx::Cache::GetConnection();

			sqlite::Statement dropdownInfo;

			
			long nProviderIDForFloatingData = pDetail->GetProviderIDForFloatingData();

			if(nProviderIDForFloatingData != -1) {
				if(ptc->m_bAutoAlphabetizeDropdown) {
					dropdownInfo.Prepare(sql, 
						"select EmrTableDropdownInfoT.ID, EmrTableDropdownInfoT.DropdownGroupID, EmrTableDropdownInfoT.Inactive, EmrTableDropdownInfoTextT.Data, case when EmrProviderFloatTableDropdownT.Count is null then 0 else 1 end as IsFloated, EmrTableDropdownStampFilterT.Stamps, EmrTableDropdownStampDefaultsT.Stamps "
						"from EmrTableDropdownInfoT "
						"inner join EmrTableDropdownInfoTextT on EmrTableDropdownInfoT.TextID = EmrTableDropdownInfoTextT.rowid "
						"left join EmrTableDropdownStampDefaultsT on EmrTableDropdownInfoT.ID = EmrTableDropdownStampDefaultsT.EmrTableDropdownInfoID "
						"left join EmrTableDropdownStampFilterT on EmrTableDropdownInfoT.ID = EmrTableDropdownStampFilterT.EmrTableDropdownInfoID "
						"left join EmrProviderFloatTableDropdownT on EmrTableDropdownInfoT.DropdownGroupID = EmrProviderFloatTableDropdownT.EmrTableDropdownGroupID "
						"and EmrProviderFloatTableDropdownT.ProviderID = ? "
						"where EmrTableDropdownInfoT.EmrDataID = ? "
						"order by case when EmrProviderFloatTableDropdownT.Count is null then 0 else 1 end desc, EmrTableDropdownInfoTextT.Data collate nocase "
					);
				} else {
					dropdownInfo.Prepare(sql, 
						"select EmrTableDropdownInfoT.ID, EmrTableDropdownInfoT.DropdownGroupID, EmrTableDropdownInfoT.Inactive, EmrTableDropdownInfoTextT.Data, case when EmrProviderFloatTableDropdownT.Count is null then 0 else 1 end as IsFloated, EmrTableDropdownStampFilterT.Stamps, EmrTableDropdownStampDefaultsT.Stamps "
						"from EmrTableDropdownInfoT "
						"inner join EmrTableDropdownInfoTextT on EmrTableDropdownInfoT.TextID = EmrTableDropdownInfoTextT.rowid "
						"left join EmrTableDropdownStampDefaultsT on EmrTableDropdownInfoT.ID = EmrTableDropdownStampDefaultsT.EmrTableDropdownInfoID "
						"left join EmrTableDropdownStampFilterT on EmrTableDropdownInfoT.ID = EmrTableDropdownStampFilterT.EmrTableDropdownInfoID "
						"left join EmrProviderFloatTableDropdownT on EmrTableDropdownInfoT.DropdownGroupID = EmrProviderFloatTableDropdownT.EmrTableDropdownGroupID "
						"and EmrProviderFloatTableDropdownT.ProviderID = ? "
						"where EmrTableDropdownInfoT.EmrDataID = ? "
						"order by coalesce(EmrProviderFloatTableDropdownT.Count, -1) desc, EmrTableDropdownInfoT.SortOrder "
					);
				}

				dropdownInfo.Reset().Bind() << nProviderIDForFloatingData << ptc->nID;
			} else {			
				if(ptc->m_bAutoAlphabetizeDropdown) {
					dropdownInfo.Prepare(sql, 
						"select EmrTableDropdownInfoT.ID, EmrTableDropdownInfoT.DropdownGroupID, EmrTableDropdownInfoT.Inactive, EmrTableDropdownInfoTextT.Data, 0 as IsFloated, EmrTableDropdownStampFilterT.Stamps, EmrTableDropdownStampDefaultsT.Stamps "
						"from EmrTableDropdownInfoT "
						"inner join EmrTableDropdownInfoTextT on EmrTableDropdownInfoT.TextID = EmrTableDropdownInfoTextT.rowid "
						"left join EmrTableDropdownStampDefaultsT on EmrTableDropdownInfoT.ID = EmrTableDropdownStampDefaultsT.EmrTableDropdownInfoID "
						"left join EmrTableDropdownStampFilterT on EmrTableDropdownInfoT.ID = EmrTableDropdownStampFilterT.EmrTableDropdownInfoID "
						"where EmrTableDropdownInfoT.EmrDataID = ? "
						"order by EmrTableDropdownInfoTextT.Data collate nocase "
					);
				} else {
					dropdownInfo.Prepare(sql, 
						"select EmrTableDropdownInfoT.ID, EmrTableDropdownInfoT.DropdownGroupID, EmrTableDropdownInfoT.Inactive, EmrTableDropdownInfoTextT.Data, 0 as IsFloated, EmrTableDropdownStampFilterT.Stamps, EmrTableDropdownStampDefaultsT.Stamps "
						"from EmrTableDropdownInfoT "
						"inner join EmrTableDropdownInfoTextT on EmrTableDropdownInfoT.TextID = EmrTableDropdownInfoTextT.rowid "
						"left join EmrTableDropdownStampDefaultsT on EmrTableDropdownInfoT.ID = EmrTableDropdownStampDefaultsT.EmrTableDropdownInfoID "
						"left join EmrTableDropdownStampFilterT on EmrTableDropdownInfoT.ID = EmrTableDropdownStampFilterT.EmrTableDropdownInfoID "
						"where EmrTableDropdownInfoT.EmrDataID = ? "
						"order by EmrTableDropdownInfoT.SortOrder "
					);
				}

				dropdownInfo.Reset().Bind() << ptc->nID;
			}


			enum DropdownInfoColumns {
				  ID
				, DropdownGroupID
				, Inactive
				, Data
				, IsFloated
				, StampFilter
				, StampDefaults
			};

			bool useDropdownStampFilters = !GetRemotePropertyInt("DisableSmartStampTableDropdownStampFilters", 0, 0, "<None>", true);

			CEMR *pEMR = pDetail->GetParentEMR();
			bool wasFloating = false;
			while (sqlite::Row row = dropdownInfo.Next()) {
				long id = row[ID];
				long groupID = row[DropdownGroupID];
				bool inactive = !!row[Inactive].Int();
				bool isFloated = !!row[IsFloated].Int();

				//if this item is not floated, but the last item was, add a separator
				if(!isFloated && wasFloating) {
					aryDataItems.Add(TableDropdownItem(0, "----------", TRUE));
				}
				//now track this item's status
				wasFloating = isFloated;
			
				pInfo->nComboDataItems++;
				if(!inactive) {
					nActiveCount++;
				}

				aryDataItems.Add(TableDropdownItem(id, row[Data], !inactive));

				// (a.walling 2013-07-23 21:36) - PLID 57686 - CEMR::m_mapDropdownIDToDropdownGropupID (sic) no longer necessary; we already have this in Nx::Cache. 
				
				{
					sqlite::Blob<long> defaults = row[StampDefaults];
					for each(long nStampID in defaults) {
						ptc->AddStampDefaultDropdownID(nStampID, id);
					}
				}

				if (useDropdownStampFilters) {
					TableDropdownItem& item = aryDataItems[aryDataItems.GetSize() - 1];
					sqlite::Blob<long> filter = row[StampFilter];
					for each(long nStampID in filter) {
						item.AddStamp(nStampID);
					}
				}
			}
			nxp.Tick("#=%li", aryDataItems.GetSize());
		}

		// Only add "<multiple>" if there is more than one possible selection
		// (z.manning, 03/28/2008) - PLID 29450 - Make sure we are only counting active selections.
		// (j.jones 2008-06-03 17:07) - PLID 30261 - don't add the multiple option if this is in the EMR item entry preview'
		// (z.manning 2011-03-11) - PLID 42618 - Only show multiple on dropdown type columns
		if (nActiveCount > 1 && bOnEmn && ptc->nType == LIST_TYPE_DROPDOWN) {
			pInfo->aryComboItems.Add(TableDropdownItem(m_nMultipleSelectDropdownSentinelValue, "<Multiple>", TRUE));
		}

		// (z.manning 2011-03-16 09:47) - PLID 42618 - No need for a blank/sentinel selection on text columns
		if(ptc->nType == LIST_TYPE_DROPDOWN) {
			pInfo->aryComboItems.Add(TableDropdownItem(0, "", TRUE));
		}
		pInfo->aryComboItems.Append(aryDataItems);
	}

	CTableDropdownItemArray aryAllItems;
	aryAllItems.Append(pInfo->aryComboItems);
	// (z.manning 2008-06-11 15:24) - PLID 30155 - Do we have any addition SQL for this combo
	aryAllItems.Append(ptc->aryAdditionalEmbeddedComboValues);
	// (c.haag 2008-03-20 17:24) - Return the completed combo SQL
	aryAllItems.Append(pInfo->aryDelimitedSentinelSqlFragment);

	CString strDropdownSource = aryAllItems.GetComboText(nStampID);
	return strDropdownSource;
}

// (c.haag 2011-03-18) - PLID 42891 - Handle common list button presses
BOOL CEmrItemAdvTableBase::HandleCommonListButtonPress(CEMNDetail* pDetail, WPARAM wParam)
{
	int nID = LOWORD(wParam);
	const CEmrInfoCommonListCollection& commonLists = pDetail->GetCommonLists();
	// (c.haag 2011-03-31) - PLID 43062 - Add 1 for the sentinel "All" item
	const int nSentinelButtons = 1; // The number of special non-common-list buttons that can produce selection lists
	if (nID >= IDC_FIRST_COMMON_LIST_BUTTON && nID < IDC_FIRST_COMMON_LIST_BUTTON + commonLists.GetListCount() + nSentinelButtons)
	{
		// Throw an exception if this is anything but a system Current Medications / Allergy list (outdated lists are acceptable)")
		if (!pDetail->IsCurrentMedicationsTable() && !pDetail->IsAllergiesTable())
		{
			ThrowNxException("HandleCommonListButtonPress called for a non-system table!"); 
		}

		// Populate arrays of Emr Data ID's and names
		CArray<long,long> anIDs;
		CStringArray astrNames;
		int i;

		// (c.haag 2011-03-31) - PLID 43062 - Special handling for the "All" button
		if (IDC_FIRST_COMMON_LIST_BUTTON == nID)
		{
			for (IRowSettingsPtr pRow = m_Table->GetFirstRow(); NULL != pRow; pRow = pRow->GetNextRow()) 
			{
				const short nNameCol = 0;
				const short nRxCol = 1;
				_variant_t vSelected = pRow->GetValue(nRxCol);

				// vSelected is VT_I2 by default, but bad experiences with VT_I2's becoming VT_BOOL's later on leads
				// me to be cautious.
				BOOL bSelected = FALSE;
				if (VT_I2 != vSelected.vt && VT_BOOL != vSelected.vt && VT_NULL != vSelected.vt) {
					ThrowNxException("Unexpected variable type for vSelected in HandleCommonListButtonPress: %d", vSelected.vt);
				}

				if (	(VT_I2 == vSelected.vt && 0 != VarShort(vSelected)) ||
					(VT_BOOL == vSelected.vt && FALSE != VarBool(vSelected)))
				{
					bSelected = TRUE;
				}

				if (!bSelected)
				{
					const CString strName = VarString(pRow->GetValue(nNameCol));
					long nRowIndex = -1;
					if (!m_mapDatalistRowToIndex.Lookup((LPDISPATCH)pRow, nRowIndex) || -1 == nRowIndex) {
						ThrowNxException("Failed to acquire row index from pRow in HandleCommonListButtonPress!");
					}
					TableRow* tr = pDetail->GetRowPtr(nRowIndex);
					if (NULL == tr) {
						ThrowNxException("Failed to acquire TableRow object from pRow in HandleCommonListButtonPress!");
					}
					const long nEmrDataID = tr->m_ID.nDataID;
					anIDs.Add(nEmrDataID);
					astrNames.Add(strName);
				}
			}			
		}
		else
		{
			// Fetch the common list corresponding to this ID
			const CEmrInfoCommonListCollection& commonLists = pDetail->GetCommonLists();
			// (c.haag 2011-03-31) - PLID 43062 - Subtract nSentinelButtons because the ID of the first non-sentinel list
			// is actually IDC_FIRST_COMMON_LIST_BUTTON + nSentinelButtons
			int nListIndex = nID - IDC_FIRST_COMMON_LIST_BUTTON - nSentinelButtons;
			CEmrInfoCommonList list = commonLists.GetListByIndex(nListIndex);

			// Now get a list of Emr Data ID's already selected in the table
			CArray<long,long> anIDsToIgnore;
			if (VT_BSTR == pDetail->GetStateVarType()) 
			{
				TableElement te;
				int nElements = pDetail->GetTableElementCount();
				for (i=0; i < nElements; i++) 
				{
					pDetail->GetTableElementByIndex(i, te);
					if (te.m_bChecked)
					{
						anIDsToIgnore.Add(te.m_pRow->m_ID.nDataID);
					}
				}
			}

			int nItems = list.GetItemCount();
			for (i=0; i < nItems; i++)
			{
				CEmrInfoCommonListItem item = list.GetItemByIndex(i);
				if (!IsIDInArray(item.GetEmrDataID(), anIDsToIgnore))
				{
					anIDs.Add(item.GetEmrDataID());
					astrNames.Add(item.GetData());
				}
			}
		}

		if (0 == anIDs.GetSize())
		{
			AfxMessageBox("All of the items in this common list have already been assigned to this detail.");
			return FALSE;
		}

		// Now create a multi-select window
		// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
		CMultiSelectDlg dlg(m_wndTable.GetParent(), "CEmrItemAdvTableBase::HandleCommonListButtonPress");
		if (IDOK == dlg.Open(anIDs, astrNames, "Please select one or more items from the list", 1))
		{
			// If the user clicked OK, we need to populate the EMN detail with the new values
			// and update its state
			CArray<long,long> anSelections;
			dlg.FillArrayWithIDs(anSelections);
			// Do for all selected values
			for (i=0; i < anSelections.GetSize(); i++)
			{
				TableRowID tr(anSelections[i],-1,NULL,-1,-1);
				TableColumn tc = pDetail->GetColumn(0); // This is always the Rx column
				TableElement te;
				// (a.walling 2011-03-21 12:34) - PLID 42962 - Set the checkbox for the table row
				if (pDetail->GetTableElement(&tr, tc.nID, te)) {
					//te.m_strValue = "-1"; // Selected
					te.m_bChecked = TRUE;
					pDetail->SetTableElement(te, TRUE, FALSE);
				}
			}
			
			return TRUE;
		}
		else 
		{
			return FALSE;
		}
	}
	else {
		// Can be another legitimate command. Just return false because
		// the state is unchanged.
		return FALSE;
	}
}

// (z.manning 2011-05-05 15:35) - PLID 43568
void CEmrItemAdvTableBase::SetDatalistRedraw(BOOL bRedraw)
{
	m_Table->SetRedraw(bRedraw ? VARIANT_TRUE : VARIANT_FALSE);
}

// (z.manning 2011-05-27 10:42) - PLID 42131
void CEmrItemAdvTableBase::ApplyTransformation(CEMNDetail *pDetail, LPDISPATCH lpRow, const short nCol, CWnd *pwnd, CWnd *pParentDlg, BOOL bIsPopupWnd)
{
	IRowSettingsPtr pRow(lpRow);
	if(pRow == NULL) {
		return;
	}

	_variant_t varOldTableState = pDetail->GetState();

	// (z.manning 2011-05-31 13:15) - PLID 42131 - First let's find the tab detail row and col indices from the given
	// datalist row and datalist column index.
	long nTableRowIndex = LookupTableRowIndexFromDatalistRow(pRow);
	long nDetailRow, nDetailColDummy;
	TablePositionToDetailPosition(pDetail, nTableRowIndex, nCol, nDetailRow, nDetailColDummy);
	TableRow *ptr = pDetail->GetRowPtr(nDetailRow);

	// (z.manning 2011-05-31 13:15) - PLID 42131 - Now let's go through every column in the detail and check for
	// transformation formulas.
	CArray<CalcCellInfo,CalcCellInfo&> aryCalcInfo;
	for(int nColIndex = 0; nColIndex < pDetail->GetColumnCount(); nColIndex++)
	{
		TableColumn *ptc = pDetail->GetColumnPtr(nColIndex);
		if(ptc->HasTransformFormula())
		{
			// (z.manning 2011-05-31 13:18) - PLID 42131 - This column has a transformation formula so let's calculate
			// the result for this cell. We keep track of all results in an array and update the table all at once later
			// because we want all transformation calculations to be based on the original values in the table.
			//TES 10/12/2012 - PLID 39000 - Remember whether or not the calculation included unused numbers
			CString strResult;
			BOOL bHasUnusedNumbers = FALSE;
			if(CalculateCellValue(pDetail, nDetailRow, nColIndex, this, strResult, bHasUnusedNumbers)) {
				CalcCellInfo calcinfo;
				calcinfo.nDetailRow = nDetailRow;
				calcinfo.nDetailCol = nColIndex;
				calcinfo.strResult = strResult;
				calcinfo.bHasUnusedNumbers = bHasUnusedNumbers;
				aryCalcInfo.Add(calcinfo);
			}
		}
	}

	if(aryCalcInfo.GetSize() > 0)
	{
		BOOL bSomethingChanged = FALSE;
		for(int nCalcInfoIndex = 0; nCalcInfoIndex < aryCalcInfo.GetSize(); nCalcInfoIndex++)
		{
			CalcCellInfo calcinfo = aryCalcInfo.GetAt(nCalcInfoIndex);
			//TES 10/12/2012 - PLID 39000 - Pass in whether the calculation included unused numbers
			if(UpdateCalculatedCellValue(calcinfo.strResult, pDetail, calcinfo.nDetailRow, calcinfo.nDetailCol, this, &(calcinfo.bHasUnusedNumbers))) {
				bSomethingChanged = TRUE;
			}
		}

		if(bSomethingChanged) {
			pDetail->UpdateAutofillColumnsByRow(ptr, NULL);
			RequestTableStateChange(pDetail, varOldTableState, pwnd, pParentDlg, bIsPopupWnd);
		}
	}
}

// (j.jones 2011-08-15 14:10) - PLID 45033 - exposed m_bIsActiveCurrentMedicationsTable, for read-only purposes
BOOL CEmrItemAdvTableBase::GetIsActiveCurrentMedicationsTable()
{
	return m_bIsActiveCurrentMedicationsTable;
}

// (j.jones 2011-08-15 14:10) - PLID 45033 - exposed m_bIsActiveAllergiesTable, for read-only purposes
BOOL CEmrItemAdvTableBase::GetIsActiveAllergiesTable()
{
	return m_bIsActiveAllergiesTable;
}

// (a.walling 2012-08-30 07:05) - PLID 51953 - Detect actual VK_TAB keypresses to ignore stuck keys
BOOL CEmrItemAdvTableBase::HandlePreTranslateMessage(MSG* pMsg)
{
	if (VK_TAB == pMsg->wParam) {
		switch (pMsg->message) {
			case WM_KEYDOWN:
				if (!(HIWORD(pMsg->lParam) & KF_REPEAT)) {
					m_bTabPressed = true;
				} else {
					TRACE("VK_TAB repeating %lu...\n", LOWORD(pMsg->lParam));
				}
				break;
			case WM_KEYUP:
				m_bTabPressed = false;
				break;
		}
	}

	return FALSE;
}


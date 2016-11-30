#pragma once

#include "EMNDetailStructures.h"

// (j.jones 2013-05-16 15:41) - PLID 56596 - changed the EMNDetail.h include into a forward declare
class CEMNDetail;

//TES 1/29/2008 - PLID 28673 - Define IDCs for any controls we create.
#define TABLE_IDC	1000

// (c.haag 2011-03-18) - PLID 42891 - ID of the first common list button
#define IDC_FIRST_COMMON_LIST_BUTTON		2000

#define ID_SHOW_LINKED_DETAIL		44641
// (z.manning 2010-12-20 09:28) - PLID 41886 - Clear options
#define IDM_CLEAR_TABLE_CELL		44642
#define IDM_CLEAR_TABLE_ROW			44643
#define IDM_CLEAR_TABLE_COLUMN		44644
#define IDM_CLEAR_TABLE				44645
// (z.manning 2010-12-22 11:08) - PLID 41887 - Copy options
#define IDM_COPY_TABLE_CELL_UP		44646
#define IDM_COPY_TABLE_CELL_DOWN	44647
#define IDM_COPY_TABLE_ROW_UP		44648
#define IDM_COPY_TABLE_ROW_DOWN		44649
// (z.manning 2011-05-27 10:03) - PLID 42313 - Transform option
#define IDM_TRANSFORM_ROW			44650
#define IDM_EDIT_CODING_GROUP		44651 // (z.manning 2011-07-14 10:21) - PLID 44469


class CEmrCodingGroup;

//
// (c.haag 2008-10-15 12:52) - PLID 31700 - This class exists to hold functionality
// mutual to the EmrItemAdvTableDlg and EMRItemAdvPopupWnd classes.  All functions are
// merged from the two classes, with minimal alterations where necessary for them to
// work properly.
//
class CEmrItemAdvTableBase
{
protected:
	//For table
	CWnd m_wndTable;
private:
	// (c.haag 2008-10-17 09:57) - PLID 31709 - The table may now only be accessed by the base
	// class so that no inherited class can disrespect table flipping
	//DRT 7/28/2008 - PLID 30824 - Converted to datalist2
	NXDATALIST2Lib::_DNxDataListPtr m_Table;

protected:
	// (c.haag 2011-03-18) - PLID 42891 - Common list buttons
	CArray<CNxIconButton*, CNxIconButton*> m_apCommonListButtons;

protected:
	//DRT 7/24/2008 - PLID 30824 - Conversion to NxDatalist2 requires that we maintain our
	//	own map of how things are related.  Previously all code operated on the assumption
	//	that datalist row index == EMNDetail TableRow array index.  That assumption still 
	//	stands, but we are always working with datalist pointers, not row indices any 
	//	longer.
	//
	//Reasoning and thought process on this.  c.haag and I discussed for a while to come up
	//	with a decent solution here with the datalist2 change.  The fundamental problem is
	//	that the CEMNDetail::m_arTableRows array and the datalist we use here have nothing
	//	linking each other together, there is just an (unstated) assumption that the
	//	datalist row index will always match up with the EMNDetail array index.  So I 
	//	attempted to maintain that existing assumption as much as possible for consistency.
	//	I avoided any places where I could have linked the datalist row to the TableRow
	//	object directly, instead favoring to get the mapped row index and use that to 
	//	lookup the TableRow.  This is consistent with past behavior, and seemed safest
	//	within the context of this dialog.  Also, I maintain the pre-existing behavior that
	//	all iterations are done over the EMNDetail array of TableRow objects, and never
	//	iterate over the datalist interface.  Some code in this dialog could be simplified
	//	if we did that.
	//	I did take advantage of any opportunities to clean up existing code that could be 
	//	simplified and still maintain this scheme.

	//The map links the index with the datalist row.  These indexes should NEVER be accessed directly, 
	//	use the Lookup functions.
	CMap<long, long, LPDISPATCH, LPDISPATCH> m_mapIndexToDatalistRow;
	//Also add a map to go in reverse.  This is filled at the same time.
	CMap<LPDISPATCH, LPDISPATCH, long, long> m_mapDatalistRowToIndex;

	//DRT 7/24/2008 - PLID 30824 - And then provide a helpful function to perform the lookup in the above
	//	map.
	NXDATALIST2Lib::IRowSettingsPtr LookupDatalistRowFromTableRowIndex(long nIndex);

	//DRT 7/24/2008 - PLID 30824 - And also a way to go backwards.
	// (z.manning 2011-02-23 16:24) - PLID 42574 - Added a param for whether or not to throw an exception
	long LookupTableRowIndexFromDatalistRow(NXDATALIST2Lib::IRowSettingsPtr pRow, BOOL bExceptionIfNotFound = TRUE);

	//DRT 7/24/2008 - PLID 30824 - Clear our all the map data
	void ClearIndexDatalistMaps();

protected:
	// (c.haag 2008-01-14 11:39) - PLID 17936 - This structure is used for
	// calculating the source string of a dropdown column.
	struct TableDropdownSourceInfo {

		// (c.haag 2008-03-20 17:20) - This is a SQL statement fragment that must
		// be appended to the embedded table column combo SQL to include the sentinel
		// items which represent multiple selections
		// (z.manning 2011-10-11 10:00) - PLID 42061 - This is now an array instead of a strying
		CTableDropdownItemArray aryDelimitedSentinelSqlFragment;

		// (c.haag 2008-03-20 17:23) - This is the embedded combo string minus the
		// strDelimitedSentinelSqlFragment at the end.
		// (z.manning 2009-03-19 12:57) - PLID 33576 - This is no longer SQL code so
		// I renamed this variable.
		// (z.manning 2011-10-11 09:20) - PLID 42061 - This is now an array instead of a string
		CTableDropdownItemArray aryComboItems;

		// The number of items in the embedded combo that represent single data selections
		long nComboDataItems;

		// This object maps a comma-delimited string of selection ID's to a
		// sentinel ID
		CMapStringToPtr mapMultiSelectStringToSentinelID;
	};

	// (c.haag 2008-01-14 11:45) - PLID 17936 - The map of dropdown source info objects
	CMap<long,long,TableDropdownSourceInfo*,TableDropdownSourceInfo*> m_mapDropdownSourceInfo;

public:
	//Because this is a copy of a detail, we can't calculate the linked items, so we need this.
	CString m_strLinkedItemList;

protected:
	BOOL m_bIsActiveCurrentMedicationsTable;
	BOOL m_bIsActiveAllergiesTable; // (c.haag 2007-04-02 15:44) - PLID 25465 - TRUE if this belongs to a
									// detail which uses the active allergies table info item

public:

	// (j.jones 2011-08-15 14:10) - PLID 45033 - exposed m_bIsActiveCurrentMedicationsTable, for read-only purposes
	BOOL GetIsActiveCurrentMedicationsTable();
	// (j.jones 2011-08-15 14:10) - PLID 45033 - exposed m_bIsActiveAllergiesTable, for read-only purposes
	BOOL GetIsActiveAllergiesTable();

	// (c.haag 2007-08-20 15:15) - PLID 27127 - This defines how to treat the datalist in ReflectTableState:
	//
	// eRTS_FullDatalistUpdate (default value) - When ReflectTableState is called, every cell in the entire datalist
	// is updated. This is the legacy behavior for every call to ReflectTableState
	//
	// eRTS_GroupedColumnUpdate - When ReflectTableState is called, every cell in the entire datalist
	// that is part of a grouped column is updated.
	//
	// eRTS_SingleCellUpdate - When OnEditingFinished is called, we should set m_ReflectTableStateDLHint to
	// this value so that ReflectTableState knows to only update the cell being edited, as opposed to every cell
	// in the whole list
	//
	typedef enum { eRCS_FullDatalistUpdate, eRCS_GroupedColumnUpdate, eRCS_SingleCellUpdate, eRCS_NoDatalistUpdate } EReflectCurrentStateDLHint;
	
protected:
	// (c.haag 2007-08-20 14:47) - PLID 27126 - This defines how to treat the datalist in ReflectCurrentState
	EReflectCurrentStateDLHint m_ReflectCurrentStateDLHint;

public:
	// (c.haag 2007-08-20 14:51) - PLID 27126 - Utility functions for getting and setting the ReflectCurrentState hint
	inline EReflectCurrentStateDLHint GetReflectCurrentStateDLHint() const { return m_ReflectCurrentStateDLHint; }
	inline void SetReflectCurrentStateDLHint(EReflectCurrentStateDLHint rcshint) { m_ReflectCurrentStateDLHint = rcshint; }

protected:
	// (c.haag 2007-08-20 14:47) - PLID 27126 - These store the location of the cell whose editing is being finished.
	// These are reset to -1 when the editing is done.
	//DRT 7/24/2008 - PLID 30824 - Converted to use NxDataList2
	NXDATALIST2Lib::IRowSettingsPtr m_pEditingFinishedRow;
	short m_nEditingFinishedCol;

protected:
	// (c.haag 2008-01-15 17:44) - PLID 17936 - We now use special sentinel ID's for
	// datalist dropdown column values that pertain to multiple selections.
	const long m_nMultipleSelectDropdownSentinelValue;
	// (c.haag 2008-01-29 12:07) - PLID 28686 - Special value for adding a new item on the fly
	const long m_nAddNewDropdownSentinelValue;
	const long m_nFirstAvailableDropdownSentinelValue;

protected:
	// (c.haag 2011-03-18) - PLID 42906 - This rectangle contains the combined region of
	// common buttons and table control
	CRect m_rcTableAndCommonButtonsArea;

public:
	CEmrItemAdvTableBase(void);
	virtual ~CEmrItemAdvTableBase(void);

public:
	virtual void CalculateTableSize(CEMNDetail* pDetail, CSize &sz);
	// (c.haag 2011-03-18) - PLID 42906 - Returns the size of a common list button
	CSize GetCommonListButtonSize() const;
	// (c.haag 2011-03-18) - PLID 42906 - Returns the buffer area around a common list button
	CSize GetCommonListButtonBufferSize() const;
	// (c.haag 2011-03-18) - PLID 42906 - Calculates the space needed for the common buttons region.
	// This takes in the size of the table itself as it were before any button region calculations.
	CSize CalculateCommonButtonRegionSize() const;
	virtual void UpdateLinkedItemList(CEMNDetail* pDetail, BOOL bIsPopupWnd);

	// (c.haag 2008-01-15 17:42) - PLID 17936 - This function cleans and clears
	// m_mapDropdownSourceInfo
	// (j.jones 2012-10-31 13:49) - PLID 53450 - made this a public function
	virtual void ClearDropdownSourceInfoMap();

protected:
	// (c.haag 2008-01-11 16:04) - PLID 17936 - This function return TRUE if the given
	// variant, which is the DATA field of a (X;Y;DATA) state value, represents a
	// dropdown value with multiple selections.
	virtual BOOL StateDropdownValueHasMultipleSelections(const _variant_t& v) const;

	// (c.haag 2008-01-15 17:53) - PLID 17936 - This function return TRUE if the given
	// variant is a value from the dropdown column of a datalist where the value represents 
	// multiple dropdown selections.
	virtual BOOL DataListDropdownValueHasMultipleSelections(const _variant_t& v) const;

protected:
	// (c.haag 2008-01-15 17:37) - PLID 17936 - A table state consists of a string made up of an
	// array of semi-colon delimited values in the form (X;Y;DATA). Normally, the data segment
	// plugs right into a datalist cell. However, in some cases it does not; and it must be 
	// formatted to appear correctly in the list. That is the task of this function.
	//
	// For dropcolumn columns, this function will always return a long integer variant.
	//
	// (c.haag 2008-10-17 09:21) - PLID 31709 - Made it so it only takes in a detail column index (as opposed to a datalist column index)
	// (z.manning 2012-04-03 16:20) - PLID 33710 - Added detail row parameter
	virtual _variant_t FormatValueForDataList(CEMNDetail* pDetail, long nDetailRow, short nDetailCol, const _variant_t& v);

	// (c.haag 2008-01-15 17:52) - PLID 17936 - A table state consists of string made up of an
	// array of semi-colon delimited values in the form (X;Y;DATA). Normally, the data segment
	// plugs right into a datalist cell. However, in some cases it does not; and when you pull
	// such a value back out of a datalist cell, it needs to be converted into its original
	// form in the table state. That is the task of this function.
	//
	// For dropcolumn columns, this function will always return a string variant.
	//
	// (c.haag 2008-10-17 09:21) - PLID 31709 - Made it so it only takes in a detail column index (as opposed to a datalist column index)
	// (z.manning 2012-04-03 16:20) - PLID 33710 - Added detail row parameter
	virtual _variant_t UnformatDataListValue(CEMNDetail* pDetail, long nDetailRow, short nDetailCol, const _variant_t& v) const;

protected:
	// (z.manning 2011-06-02 09:47) - PLID 42131 - Created a simple struct to store data used when calculating table formulas.
	struct CalcCellInfo
	{
		int nDetailRow;
		int nDetailCol;
		CString strResult;
		BOOL bHasUnusedNumbers; //TES 10/12/2012 - PLID 39000
	};
	// (z.manning, 05/28/2008) - PLID 30155 - Added functions to calculate the values for calculated fields
	//DRT 7/28/2008 - PLID 30824 - Converted to datalist2
	// (a.walling 2010-04-14 11:17) - PLID 34406 - These need to be accessed outside of the actual table window for windowless updates of calculated values
	// so, they are no longer virtual, and they are static, with a nullable EmrItemAdvTableBase* for datalist stuff.
	static void CalculateAndUpdateCellValue(CEMNDetail* pDetail, long nDetailRow, short nDetailCol, CEmrItemAdvTableBase* pAdvTable);
	// (z.manning 2011-05-27 11:01) - PLID 42131 - Split the logic of CalculateAndUpdateCellValue into two functions
	//TES 10/12/2012 - PLID 39000 - Added an output parameter, bHasUnusedNumbers, which will be set to true if the formula includes a cell in
	// its calculations which has any numeric digits after the first non-numeric digits (i.e., "5 cm. - 8 cm.", which would resolve to "5")
	static BOOL CalculateCellValue(CEMNDetail* pDetail, long nDetailRow, short nDetailCol, CEmrItemAdvTableBase* pAdvTable, OUT CString &strResult, OUT BOOL &bHasUnusedNumbers);
	// (z.manning 2011-05-31 14:50) - PLID 42131 - This returns true if something actually changed.
	//TES 10/12/2012 - PLID 39000 - Added an optional parameter indicating whether the formula included cells with unused numbers
	static BOOL UpdateCalculatedCellValue(LPCTSTR strValue, CEMNDetail* pDetail, long nDetailRow, short nDetailCol, CEmrItemAdvTableBase* pAdvTable, OPTIONAL IN BOOL *pbHasUnusedNumbers = NULL);
public:
	static void UpdateCalculatedFields(CEMNDetail* pDetail, CEmrItemAdvTableBase* pAdvTable);

protected:
	// (j.jones 2008-06-05 09:14) - PLID 18529 - TryAdvanceNextDropdownList is called when
	// any non-text cell is edited, and tries to auto-dropdown the next dropdown or linked
	// list, only if the next column is that type, and the preference is on
	//DRT 7/24/2008 - PLID 30824 - Converted to use NxDataList2
	virtual void TryAdvanceNextDropdownList(CWnd* pWnd, CEMNDetail* pDetail, LPDISPATCH lpRow, short nCol);

protected:
	// (j.jones 2008-06-05 10:08) - PLID 18529 - TryAdvanceNextDropdownList will potentially send
	// NXM_START_EDITING_EMR_TABLE which will fire this function, which will start editing the
	// row and column in question
	//DRT 7/24/2008 - PLID 30824 - Converted to use NxDataList2
	// (a.walling 2008-10-02 09:16) - PLID 31564 - VS2008 - Message handlers must fit the LRESULT fn(WPARAM, LPARAM) format
	virtual void StartEditingEMRTable(WPARAM wParam, LPARAM lParam);

protected:
	//DRT 7/24/2008 - PLID 30824 - Converted to NxDataList2
	NXDATALIST2Lib::EColumnFieldType GetColumnFieldType(long nListType) const;
	VARENUM GetColumnDataType(long nListType) const;

protected:
	// (c.haag 2008-10-15 16:12) - PLID 31700 - This function has too many differences to be merged safely; but it's required
	// by existing functions. Leave this unmerged.
	// (z.manning 2011-03-11) - PLID 42778 - The 2 copies of this function were nearly identical, so I combined them.
	// (z.manning 2011-10-11 11:14) - PLID 42061 - Added stamp ID
	virtual CString GetDropdownSource(long nColumnID, const long nStampID) = 0;
	// (z.manning 2011-10-11 11:01) - PLID 42061 - Added stamp ID parameter
	virtual CString GetDropdownSource(long nColumnID, CEMNDetail *pDetail, BOOL bOnEmn, const long nStampID);

protected:
	// (c.haag 2008-10-17 11:46) - PLID 31700 - Reflects the table's state
	void ReflectTableState(CWnd* pWnd, CEMNDetail* pDetail, BOOL bIsPopupWnd);

	// (c.haag 2008-10-17 11:44) - PLID 31709 - Necessary for ReflectTableState
	// (a.walling 2010-06-21 15:07) - PLID 38779 - This is never called with not bCalcOnly
	virtual BOOL RepositionTableControls(IN OUT CSize &szArea) = 0;

	// (c.haag 2008-10-17 11:53) - PLID 31709 - Necessary for ReflectTableState
	virtual CString GenerateNewVarState() = 0;

protected:
	// (r.gonet 02/14/2013) - PLID 40017 - Now we need to know if the detail is popped up.
	virtual void UpdateColumnStyle(CEMNDetail* pDetail, BOOL bIsPoppedUp);

// (z.manning 2009-08-07 17:06) - PLID 35144 - Changed from private to protected
protected:
	// (a.walling 2010-04-14 11:26) - PLID 34406 - These are all static now
	// (c.haag 2008-10-20 11:32) - PLID 31709 - Converts a detail position to a position in the table
	static void DetailPositionToTablePosition(CEMNDetail* pDetail, long nDetailRow, long nDetailCol, long& nTableRow, long& nTableCol);
	// (c.haag 2008-10-20 11:32) - PLID 31709 - Converts a table position to a position in the detail
	static void TablePositionToDetailPosition(CEMNDetail* pDetail, long nTableRow, long nTableCol, long& nDetailRow, long& nDetailCol);
	// (c.haag 2008-10-20 11:32) - PLID 31709 - Given a position in table coordinates, return the corresponding detail element
	static TableElement GetTableElementByTablePosition(CEMNDetail* pDetail, long nTableRow, long nTableCol);

protected:
	// (c.haag 2008-10-20 09:47) - PLID 31700 - Merged EditingStarting handler from EmrItemAdvPopupWnd/TableDlg
	void HandleEditingStartingTable(LPDISPATCH lpRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue, CEMNDetail* pDetail);

	// (c.haag 2008-10-20 12:30) - PLID 31700 - Merged EditingFinishing handler from EmrItemAdvPopupWnd/TableDlg
	void HandleEditingFinishingTable(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue,
		CWnd* pWnd, CEMNDetail* pDetail);

	// (c.haag 2008-10-20 09:41) - PLID 31700 - Merged EditingFinished handler from EmrItemAdvPopupWnd/TableDlg
	void HandleEditingFinishedTable(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit,
												 CWnd* pWnd, CWnd* pParentDlg, CEMNDetail* pDetail, BOOL bIsPopupWnd);

protected:
	// (c.haag 2008-10-16 10:12) - PLID 31700 - Ensures m_Table is a valid object, and has rows and columns properly
	// assembled in it
	virtual void EnsureTableObject(CEMNDetail* pDetail, CWnd* pParent, const CString& strLabel, BOOL bReadOnly);

	// (c.haag 2008-10-16 14:33) - PLID 31700 - Adds fields to the table. Normally, these
	// are table columns. In the case of a "flipped" table; that is, a table where the
	// rows are the fields, then this will utilize the FormatOverride object in the datalist
	virtual void AddTableFields(CEMNDetail* pDetail, BOOL bIsPopupWnd);

	// (c.haag 2008-10-16 11:00) - PLID 31700 - Adds rows to m_Table
	virtual void AddTableContent(CEMNDetail* pDetail);
	// (c.haag 2011-03-18) - PLID 42891 - Adds all the common list buttons
	virtual void AddCommonListButtons(CEMNDetail* pDetail);
	// (c.haag 2011-03-18) - PLID 42906 - Called to resize the common list buttons
	virtual void ResizeCommonListButtons();

protected:
	// All functions below are utility functions for table flipping

	// (c.haag 2008-10-16 17:29) - PLID 31709 - Adds a field to a standard unflipped table (legacy code)
	// (r.gonet 02/14/2013) - PLID 40017 - we need to know if the detail is popped up. Note that we don't need to do this in the flipped version
	//  since we currently do not support saving column widths in flipped tables.
	void AddTableField(CEMNDetail* pDetail, int nDetailColumn, BOOL bIsPoppedUp);
	// (c.haag 2008-10-16 17:29) - PLID 31709 - Adds a field (which is a row in a flipped table) to a flipped table
	void AddTableField_Flipped(CEMNDetail* pDetail, int nDetailColumn);

	// (c.haag 2008-10-16 17:40) - PLID 31709 - Sets the combo source of a detail column. This will behave
	// differently depending on whether this is a flipped table or not.
	void SetComboSource(CEMNDetail* pDetail, long nDetailColumn, const CString& strSourceText);
	// (c.haag 2008-10-16 17:45) - PLID 31709 - Sets the max embedded combo height of a detail column. This
	// will behave differently depending on whether this is a flipped table or not.
	void SetEmbeddedComboDropDownMaxHeight(CEMNDetail* pDetail, long nDetailColumn, long nHeight);

	// (c.haag 2008-10-17 09:18) - PLID 31709 - Assigns a value to the table datalist
	void SetTableValue(CEMNDetail* pDetail, long nDetailRow, long nDetailColumn, const _variant_t& v);
	// (z.manning 2010-04-13 16:56) - PLID 38175 - Added an overload that take a cell background color
	//TES 10/12/2012 - PLID 39000 - Added an optional parameter for whether this is a calculated cell that includes unused numbers in its calculation.
	void SetTableValue(CEMNDetail* pDetail, long nDetailRow, long nDetailColumn, const _variant_t& v, OLE_COLOR nCellBackColor, BOOL bFormulaHasUnusedNumbers = FALSE);

protected:
	// (c.haag 2011-03-18) - PLID 42891 - Handle common list button presses
	BOOL HandleCommonListButtonPress(CEMNDetail* pDetail, WPARAM wParam);

protected:
	// All functions below make it possible for m_Table to be a private, as opposed to a protected, member

	// (c.haag 2008-10-17 10:13) - PLID 31700 - Returns TRUE if the table object is not null
	BOOL IsTableControlValid() const;

	// (c.haag 2008-10-17 10:08) - PLID 31700 - Called when this object is being destroyed
	void ClearTableControl();

	// (c.haag 2011-03-18) - PLID 42891 - Clear common list buttons
	void ClearCommonListButtons();

	// (c.haag 2008-10-17 10:08) - PLID 31700 - Set the read-only property of a table
	void SetTableReadOnly(BOOL bReadOnly);

	// (c.haag 2011-03-18) - PLID 42891 - Set common list buttons read only (called from SetTableReadOnly)
	void SetCommonListButtonsReadOnly(BOOL bReadOnly);

	// (c.haag 2008-10-17 10:14) - PLID 31700 - Set the redraw property of a table
	void SetTableRedraw(BOOL bRedraw);

	// (c.haag 2008-10-17 10:37) - PLID 31700 - Resets the stored column widths of a table. This is used
	// as a workaround to hide the table's horizontal scrollbar if it doesn't need to be visible.
	void ResetTableColumnWidths();

	// (c.haag 2008-10-17 12:02) - PLID 31700 - Returns the table (datalist)'s row count
	long GetTableRowCount();

	// (c.haag 2008-10-17 12:02) - PLID 31700 - Returns the table (datalist)'s column count
	short GetTableColumnCount();

	// (c.haag 2008-10-17 12:05) - PLID 31700 - Begins a table edit
	// (a.walling 2011-05-11 14:21) - PLID 42962 - return a BOOL instead, ignores non-visible rows
	BOOL StartTableEditing(NXDATALIST2Lib::IRowSettingsPtr pRow, short nCol);

	// (c.haag 2008-10-17 12:03) - PLID 31700 - Stops a table edit in progress
	HRESULT StopTableEditing(VARIANT_BOOL vbCommit);

	// (c.haag 2008-10-17 12:11) - PLID 31700 - Sets the current row
	void SetTableCurSel(struct NXDATALIST2Lib::IRowSettings * lpRow);

	// (c.haag 2008-10-17 12:23) - PLID 31700 - Gets the stored width
	long GetTableColumnStoredWidth(short nCol);

	// (c.haag 2008-10-17 12:23) - PLID 31700 - Gets the column name
	CString GetTableColumnTitle(short nCol);

	// (j.jones 2009-10-02 11:59) - PLID 35161 - added ability to append data to a text field
	void AppendTextToTableCell(LPDISPATCH lpRow, short nTableCol,
								CEMNDetail* pDetail, CString strTextToAppend,
								CWnd* pWnd, CWnd* pParentDlg, BOOL bIsPopupWnd);

	// (z.manning 2010-12-20 10:45) - PLID 41886
	void RequestTableStateChange(CEMNDetail *pDetail, _variant_t varOldState, CWnd *pWnd, CWnd *pParentDlg, BOOL bIsPopupWnd);

	// (z.manning 2010-12-17 16:48) - PLID 41886
	// (z.manning 2010-12-22 09:34) - PLID 41887 - Renamed this function as it now all appends copy options
	// (z.manning 2011-05-27 09:02) - PLID 42131 - Renamed this function to AppendSharedMenuOptions
	void AppendSharedMenuOptions(CMenu *pMenu, LPDISPATCH lpRow, const short nCol, CEMNDetail *pDetail);
	BOOL CanEditCell(LPDISPATCH lpRow, const short nCol, CEMNDetail *pDetail);

	// (z.manning 2010-12-20 10:58) - PLID 41886
	enum EClearTableType {
		cttInvalid = -1,
		cttCell = 1,
		cttRow,
		cttColumn,
		cttTable,
	};
	void ClearTable(EClearTableType eClearType, LPDISPATCH lpRow, const short nCol, CEMNDetail *pDetail, CWnd *pwnd, CWnd *pParentDlg, BOOL bIsPopupWnd);

	// (z.manning 2010-12-22 12:26) - PLID 41887 - Copy related functions
	void CopyAndPasteTableRowToAdjacentRow(LPDISPATCH lpRowSource, const short nCol, CEMNDetail *pDetail, BOOL bCellOnly, BOOL bCopyForward, CWnd *pwnd, CWnd *pParentDlg, BOOL bIsPopupWnd);
	void CopyTableRow(LPDISPATCH lpRow, const short nCol, CEMNDetail *pDetail, BOOL bCellOnly, OUT CArray<TableElement,TableElement&> &aryCopiedElements);
	// (z.manning 2011-04-04 17:27) - PLID 30608 - Added a param for whether or not to auto-fill
	void PasteTableRow(CArray<TableElement,TableElement&> &aryElementsToPaste, TableRow *pDestRow, CEMNDetail *pDetail, CWnd *pwnd, CWnd *pParentDlg, BOOL bIsPopupWnd, BOOL bUpdateAutofillColumns);

	// (z.manning 2011-05-05 15:35) - PLID 43568
	void SetDatalistRedraw(BOOL bRedraw);

	// (z.manning 2011-05-27 10:42) - PLID 42131
	void ApplyTransformation(CEMNDetail *pDetail, LPDISPATCH lpRow, const short nCol, CWnd *pwnd, CWnd *pParentDlg, BOOL bIsPopupWnd);

	// (z.manning 2011-07-14 10:38) - PLID 44469
	CEmrCodingGroup* GetCodingGroupFromRow(TableRow *ptr);
	void OpenChargePromptDialogForRow(CEMNDetail *pDetail, LPDISPATCH lpRow, const short nCol);

	// (a.walling 2012-08-30 07:05) - PLID 51953 - Detect actual VK_TAB keypresses to ignore stuck keys
	bool m_bTabPressed;
	bool IsTabPressed() const { return m_bTabPressed; }

	BOOL HandlePreTranslateMessage(MSG* pMsg);
};

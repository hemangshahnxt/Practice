// MultiSelectDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MultiSelectDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define AUTO_SELECT_MULTI_HIGHLIGHT

using namespace NXDATALIST2Lib;
/////////////////////////////////////////////////////////////////////////////
// CMultiSelectDlg dialog


// (j.armen 2012-06-08 09:32) - PLID 49607 - Sizing ConfigRT entry
CMultiSelectDlg::CMultiSelectDlg(CWnd* pParent, const CString& strSizingConfigRT)
	: CNxDialog(CMultiSelectDlg::IDD, pParent), m_strSizingConfigRT(strSizingConfigRT)
{
	m_strNameColTitle = "";
	m_nIDToSelect = -1;
	m_strOtherButtonText = "";
	m_pfnContextMenuProc = NULL;
	m_nContextMenuProcParam = 0;
	m_bWrapText = false;
	// (a.walling 2007-06-13 17:05) - PLID 26211
	m_bAutoCheckMultipleSelections = FALSE;
	m_bPreSelectAll = FALSE;
	m_eOtherButtonStyle = NXB_NOTUSED;
	m_bNoSort = FALSE; // (z.manning 2009-04-03 14:57) - PLID 33576
	m_bShowFilterButton = FALSE;
	m_bFiltered = FALSE;
	m_bPutSelectionsAtTop = FALSE;
	// (s.dhole 2014-03-11 10:51) - PLID 61318
	m_bSetEqualColumnWidth = FALSE;
}

CMultiSelectDlg::~CMultiSelectDlg()
{
	if (m_pSourceList) {
		m_pSourceList.Detach();
	}
	// (r.gonet 05/22/2014) - PLID 62250 - Detach the datalist2
	if (m_pSourceList2) {
		m_pSourceList2.Detach();
	}
}

void CMultiSelectDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMultiSelectDlg)
	DDX_Control(pDX, IDC_STATIC_DESCRIPTION, m_nxstaticDescription);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_OTHER_BTN, m_btnOther);
	DDX_Control(pDX, IDC_MULTI_SELECT_FILTER, m_btnFilter);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMultiSelectDlg, CNxDialog)
	//{{AFX_MSG_MAP(CMultiSelectDlg)
	ON_WM_CLOSE()
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_OTHER_BTN, OnOtherBtn)
	ON_WM_CONTEXTMENU()
	ON_BN_CLICKED(IDC_MULTI_SELECT_FILTER, OnFilterBtn)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BEGIN_EVENTSINK_MAP(CMultiSelectDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CMultiSelectDlg)
	ON_EVENT(CMultiSelectDlg, IDC_MULTI_SELECT_LIST, 6, CMultiSelectDlg::OnRButtonDownMultiSelectList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)	
	ON_EVENT(CMultiSelectDlg, IDC_MULTI_SELECT_LIST, 3, CMultiSelectDlg::OnDblClickCellMultiSelectList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CMultiSelectDlg, IDC_MULTI_SELECT_LIST, 18, CMultiSelectDlg::OnRequeryFinishedMultiSelectList, VTS_I2)
	//}}AFX_EVENTSINK_MAP	
	ON_EVENT(CMultiSelectDlg, IDC_MULTI_SELECT_LIST, 10, CMultiSelectDlg::OnEditingFinishedMultiSelectList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
END_EVENTSINK_MAP()


/////////////////////////////////////////////////////////////////////////////
// CMultiSelectDlg message handlers

void CMultiSelectDlg::PreSelect(long ID)
{
	m_arPreSelect.Add(_variant_t(ID));
}

void CMultiSelectDlg::PreSelect(CArray<long,long> &arIDs)
{
	for (int i=0; i < arIDs.GetSize(); i++)
	{
		m_arPreSelect.Add(_variant_t(arIDs[i]));
	}
}

//TES 4/10/2009 - PLID 33889 - Added (useful because this is what GetRemotePropertyArray() uses)
void CMultiSelectDlg::PreSelect(CArray<int,int> &arIDs)
{
	for (int i=0; i < arIDs.GetSize(); i++)
	{
		m_arPreSelect.Add(_variant_t((long)arIDs[i]));
	}
}

void CMultiSelectDlg::PreSelect(CDWordArray& adwIDs)
{
	for (int i=0; i < adwIDs.GetSize(); i++)
	{
		m_arPreSelect.Add(_variant_t((long)adwIDs[i]));
	}
}

void CMultiSelectDlg::PreSelect(CVariantArray &arIDs)
{
	for (int i=0; i < arIDs.GetSize(); i++)
	{
		m_arPreSelect.Add(arIDs[i]);
	}
}

// (j.gruber 2011-11-07 14:23) - PLID 46316 - added bool to say to use the string as IDs or not
void CMultiSelectDlg::PreSelect(const CString& strIDs, BOOL bUseAsIDs /*=FALSE*/) // Whitespace-separated
{
	for (long i=0; i < strIDs.GetLength();)
	{
		long nEnd = strIDs.Find(' ', i);
		if (nEnd == -1) nEnd = strIDs.GetLength();
		if (!bUseAsIDs) {
			PreSelect(atoi(strIDs.Mid(i, nEnd - i)));
		}
		else {
			m_arPreSelect.Add(_variant_t(_bstr_t(strIDs.Mid(i, nEnd - i))));
		}

		i = nEnd + 1;
	}
}

//DRT 1/12/2007 - PLID 24223 - Allow users to preselect by name
void CMultiSelectDlg::PreSelectByName(CString strName)
{
	m_arPreSelectByName.Add(strName);
}

static BOOL VariantInArray(_variant_t &var, CVariantArray &va)
{
	for(int i = 0; i < va.GetSize(); i++) {
		if(va[i] == var) return TRUE;
	}
	return FALSE;
}

// (a.walling 2013-07-25 15:52) - PLID 57698 - This is for supporting the extended format of embedded combo sources; adds a new row
static void AddRowFromEmbeddedComboSource(const CString& strID, const CString& strName, long nVisible, CVariantArray &vaIDsToSkip, NXDATALIST2Lib::_DNxDataList* dlList, bool bPreSelectAll)
{
	_variant_t varID(strID);

	// (z.manning, 07/18/2007) - PLID 26727 - Make sure this value is not supposed to be skipped.
	// Also, make sure it's visible.
	if(nVisible != 0 && !VariantInArray(varID, vaIDsToSkip))
	{
		// (j.jones 2009-08-11 18:02) - PLID 35189 - converted m_dlList into a datalist2
		IRowSettingsPtr pRow = dlList->GetNewRow();
		pRow->PutValue(CMultiSelectDlg::mslcID, varID);
		pRow->PutValue(CMultiSelectDlg::mslcName, (LPCTSTR)strName);
		// (z.manning, 07/26/2007) - PLID 14579 - Select all by default if specified.
		pRow->PutValue(CMultiSelectDlg::mslcSelected, _variant_t(bPreSelectAll ? VARIANT_TRUE : VARIANT_FALSE, VT_BOOL));
		dlList->AddRowSorted(pRow, NULL);
	}
}

// (a.walling 2013-07-25 15:52) - PLID 57698 - Extended format shamelessly ripped from NxDatalist code
static CString GetNextExtendedField(const CString& strRow, int& iPos)
{
	CString str;
	if (iPos >= strRow.GetLength()) {
		return str;
	}

	int delim = strRow.Find('\x02', iPos);
	if (delim == -1) {
		delim = strRow.GetLength();
	}

	int len = delim - iPos;
	if (len) {
		str = strRow.Mid(iPos, len);
	}
	iPos = delim + 1;

	return str;
}

// (a.walling 2012-07-05 16:02) - PLID 51389 - New format, implicit visibility, using STX (0x02) as field delimiter, ETX (0x03) as row delimiter
static void LoadComboItems_ExtendedFormat(const CString& strSource, int iBegin, CVariantArray &vaIDsToSkip, NXDATALIST2Lib::_DNxDataList* dlList, bool bPreSelectAll)
{
	// popuplates these members:

	//m_arypvarEditComboItemId
	//m_arystrEditComboItemText
	//m_arybEditComboShowItem
	//m_nEditComboItemCount

	CString strRow;
	int iRowPos = iBegin;
		
	// process rows
	strRow = strSource.Tokenize("\x03", iRowPos);
	while (!strRow.IsEmpty()) {
		// process fields

		int iPos = 0;
		CString strID = GetNextExtendedField(strRow, iPos);
		CString strText = GetNextExtendedField(strRow, iPos);
		CString strVisible = GetNextExtendedField(strRow, iPos);
		
		long nVisible = atoi(strVisible);

		AddRowFromEmbeddedComboSource(strID, strText, nVisible, vaIDsToSkip, dlList, bPreSelectAll);

		strRow = strSource.Tokenize("\x03", iRowPos);
	}
}

BOOL CMultiSelectDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try
	{
		// (z.manning, 04/30/2008) - PLID 29845 - Set button styles
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		if (m_strOtherButtonText.IsEmpty()) {
			GetDlgItem(IDC_OTHER_BTN)->EnableWindow(FALSE);
			GetDlgItem(IDC_OTHER_BTN)->ShowWindow(SW_HIDE);
		}
		else {
			SetDlgItemText(IDC_OTHER_BTN, m_strOtherButtonText);
			GetDlgItem(IDC_OTHER_BTN)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_OTHER_BTN)->EnableWindow(TRUE);
			m_btnOther.AutoSet(m_eOtherButtonStyle);
		}

		// (z.manning 2009-08-19 17:25) - PLID 17932 - Should we show the filter button?
		if(m_bShowFilterButton) {
			m_btnFilter.ShowWindow(SW_SHOW);
			m_btnFilter.EnableWindow(TRUE);
		}
		else {
			m_btnFilter.ShowWindow(SW_HIDE);
			m_btnFilter.EnableWindow(FALSE);
		}

		if (m_dlList == NULL) {
			// (j.jones 2009-08-12 09:06) - PLID 35189 - converted to a dl2
			m_dlList = BindNxDataList2Ctrl(this, IDC_MULTI_SELECT_LIST, GetRemoteData(), false);
		}

		// (z.manning 2009-03-19 12:00) - PLID 33576 - Added option to not sort.
		if(m_bNoSort) {
			m_dlList->GetColumn(mslcName)->SortPriority = (short)-1;
		}

		// (z.manning 2011-08-04 10:46) - PLID 44347 - Option to sort by the checked column descending
		IColumnSettingsPtr pCheckCol = m_dlList->GetColumn(mslcSelected);
		if(m_bPutSelectionsAtTop) {
			pCheckCol->PutSortPriority((short)0);
			// (r.gonet 05/24/2014) - PLID 62250 - Fixed this from sorting asc to sort desc. It was doing the opposite of its intention.
			pCheckCol->PutSortAscending(VARIANT_FALSE);
		}

		if(m_pSourceList) {
			for(int i = 0; i < m_pSourceList->GetRowCount(); i++) {
				// (j.jones 2009-08-11 18:02) - PLID 35189 - converted m_dlList into a datalist2
				IRowSettingsPtr pRow = m_dlList->GetNewRow();
				if(!VariantInArray(m_pSourceList->GetValue(i,0), m_vaIDsToSkip)) {			
					pRow->PutValue(mslcID, m_pSourceList->GetValue(i,0));
					pRow->PutValue(mslcName, m_pSourceList->GetValue(i,1));
					// (z.manning, 07/26/2007) - PLID 14579 - Select all by default if specified.
					pRow->PutValue(mslcSelected, _variant_t(m_bPreSelectAll ? VARIANT_TRUE : VARIANT_FALSE, VT_BOOL));
					m_dlList->AddRowSorted(pRow, NULL);
				}
			}
		}
		else if (m_pSourceList2) {
			// (r.gonet 05/22/2014) - PLID 62250 - We have a datalist2 that would like to populate our multi-select list.
			// First, adjust the column style of the name column to match what is in the source datalist, unless we have been told to do otherwise
			NXDATALIST2Lib::IColumnSettingsPtr pNameColumn = m_dlList->GetColumn(mslcName);
			NXDATALIST2Lib::IColumnSettingsPtr pSourceNameColumn = m_pSourceList2->GetColumn(m_nSourceNameColumnIndex);
			if (pNameColumn == NULL) {
				ThrowNxException("%s : Could not get the Name column from the multi-select dialog datalist.", __FUNCTION__);
			}
			if (pSourceNameColumn == NULL) {
				ThrowNxException("%s : Could not get the Name column from the source datalist.", __FUNCTION__);
			}
			if (!m_bSetEqualColumnWidth) {
				// Copy the whole style except for the editable bits.
				pNameColumn->ColumnStyle = pSourceNameColumn->ColumnStyle & ~csEditable;
				pNameColumn->StoredWidth = pSourceNameColumn->StoredWidth;
			}
			if (m_strNameColTitle == "") {
				// Use our own name
				pNameColumn->ColumnTitle = pSourceNameColumn->ColumnTitle;
			}

			// (r.gonet 05/22/2014) - PLID 62250 - Get the number of columns in the list
			short nBaseNumberColumns = m_dlList->GetColumnCount();
			// (r.gonet 05/22/2014) - PLID 62250 - Add any extra columns
			for (short i = 0; i < (short)m_arySourceExtraColumnIndices.GetSize(); i++) {
				short nExtraColumnIndex = m_arySourceExtraColumnIndices.GetAt(i);
				NXDATALIST2Lib::IColumnSettingsPtr pExtraColumn = m_pSourceList2->GetColumn(nExtraColumnIndex);
				if (!pExtraColumn) {
					ThrowNxException("%s : Could not find a specified extra column in the source datalist2.", __FUNCTION__);
				}
				long nColumnStyle = pExtraColumn->ColumnStyle;
				if (m_bSetEqualColumnWidth) {
					// (r.gonet 05/22/2014) - PLID 62250 - Replace it all with auto width.
					nColumnStyle = csWidthAuto;
				}
				
				IColumnSettingsPtr pNewColumn = m_dlList->GetColumn(m_dlList->InsertColumn(-1, _T(pExtraColumn->FieldName), _T(pExtraColumn->ColumnTitle), pExtraColumn->GetStoredWidth(), nColumnStyle));
				pNewColumn->FieldType = pExtraColumn->FieldType;
				pNewColumn->DataType = pExtraColumn->DataType;
			}

			// (r.gonet 05/22/2014) - PLID 62250 - Now popluate our list with the source list's rows
			NXDATALIST2Lib::IRowSettingsPtr pSourceRow = m_pSourceList2->GetFirstRow();
			while (pSourceRow) {
				IRowSettingsPtr pRow = m_dlList->GetNewRow();
				if (!VariantInArray(pSourceRow->GetValue(m_nSourceIDColumnIndex), m_vaIDsToSkip)) {
					pRow->PutValue(mslcID, pSourceRow->GetValue(m_nSourceIDColumnIndex));
					pRow->PutValue(mslcName, pSourceRow->GetValue(m_nSourceNameColumnIndex));
					for (short i = 0; i < (short)m_arySourceExtraColumnIndices.GetSize(); i++) {
						// (r.gonet 05/22/2014) - PLID 62250 - Add the values to the extra columns
						short nExtraColumnIndex = m_arySourceExtraColumnIndices.GetAt(i);
						pRow->PutValue(i + nBaseNumberColumns, pSourceRow->GetValue(nExtraColumnIndex));
					}
					// Select all by default if specified.
					pRow->PutValue(mslcSelected, _variant_t(m_bPreSelectAll ? VARIANT_TRUE : VARIANT_FALSE, VT_BOOL));
					m_dlList->AddRowSorted(pRow, NULL);
				}
				pSourceRow = pSourceRow->GetNextRow();
			}
		}
		// (z.manning, 07/18/2007) - PLID 26727 - If we have an datalist column embedded combo source, let's use that.
		else if(!m_strEmbeddedComboSource.IsEmpty()) {
			CString strSource = m_strEmbeddedComboSource;
			// (z.manning, 07/18/2007) - PLID 26727 - Check for the visibility flag. It's a tag at the beginning
			// of the combo source in the format: ;;n;; where n is the visiblity flag.
			long nInvisibilityFlag = 0;
			if(strSource.Left(2) == ";;") {
				nInvisibilityFlag = atoi(((LPCTSTR)strSource) + 2);
				if (nInvisibilityFlag == -1) {
					strSource.Delete(0, 6);
				} else {
					strSource.Delete(0, 5);
				}
			}
			if (nInvisibilityFlag == -1) {
				// (a.walling 2013-07-25 15:52) - PLID 57698 - Extended format shamelessly ripped from NxDatalist code
				LoadComboItems_ExtendedFormat(strSource, 0, m_vaIDsToSkip, m_dlList, !!m_bPreSelectAll);
			} else {
				while(!strSource.IsEmpty())
				{
					// (z.manning, 07/18/2007) - PLID 26727 - Go through the combo source and parse out the ID
					// and name values and use them to populate the list.
					CString strID = strSource.Left(strSource.Find(';'));
					strSource.Delete(0, strSource.Find(';') + 1);
					int nNameEnd = strSource.Find(';');
					// (z.manning 2009-03-19 12:53) - PLID 33576 - I added support for the new embedded combo
					// type that has quotes around the data text.
					if(nInvisibilityFlag == 2) {
						bool bEndOfData = false;
						while(!bEndOfData) {
							if(nNameEnd == -1) {
								break;
							}
							int nQuoteCount = 0;
							char ch = strSource.GetAt(nNameEnd - 1);
							for(int nTempPos = nNameEnd - 1; nTempPos > 0 && strSource.GetAt(nTempPos) == '\"'; nTempPos--) {
								nQuoteCount++;
							}

							if(nQuoteCount % 2 == 1) {
								bEndOfData = true;
							}
							else {
								nNameEnd = strSource.Find(';', nNameEnd + 1);
							}
						}
					}
					ASSERT(nNameEnd != -1);
					CString strName = strSource.Left(nNameEnd);
					strSource.Delete(0, nNameEnd + 1);
					long nVisible = 1;
					if(nInvisibilityFlag == 1 || nInvisibilityFlag == 2) {
						// (z.manning, 07/18/2007) - PLID 26727 - We have a visibility flag, so check it.
						nVisible = AsLong(_bstr_t(strSource.Left(strSource.Find(';'))));
						strSource.Delete(0, strSource.Find(';') + 1);

						// (z.manning 2009-03-19 12:54) - PLID 33576 - If the data has quotes around it
						// get rid of them before we display it.
						if(nInvisibilityFlag == 2 && strName.GetLength() >= 2) {
							if(strName.GetAt(0) == '\"') {
								strName.Delete(0, 1);
							}
							else {
								ASSERT(FALSE);
							}
							if(strName.GetAt(strName.GetLength() - 1) == '\"') {
								strName.Delete(strName.GetLength() - 1, 1);
							}
							else {
								ASSERT(FALSE);
							}
							strName.Replace("\"\"", "\"");
						}
					}
					AddRowFromEmbeddedComboSource(strID, strName, nVisible, m_vaIDsToSkip, m_dlList, !!m_bPreSelectAll);
				}
			}
		}
		// (c.haag 2010-11-22 16:50) - PLID 41588 - Support simple string arrays. This does not support anything beyond
		// a basic sorted list population.
		else if (!m_astrStringArraySource.IsEmpty())
		{
			for (int i=0; i < m_astrStringArraySource.GetSize(); i++) 
			{
				IRowSettingsPtr pRow = m_dlList->GetNewRow();
				pRow->PutValue(mslcID, _bstr_t(AsString((long)i)));
				pRow->PutValue(mslcName, _bstr_t(m_astrStringArraySource[i]));
				pRow->PutValue(mslcSelected, _variant_t(m_bPreSelectAll ? VARIANT_TRUE : VARIANT_FALSE, VT_BOOL));
				m_dlList->AddRowSorted(pRow, NULL);
			}
		}
		// (c.haag 2011-03-18 16:58) - PLID 42908 - Added the ability to open based on an array of ID's and names
		else if (!m_astrNameIDSource_IDs.IsEmpty())
		{
			for (int i=0; i < m_astrNameIDSource_IDs.GetSize(); i++) 
			{
				IRowSettingsPtr pRow = m_dlList->GetNewRow();
				pRow->PutValue(mslcID, _bstr_t(AsString(m_astrNameIDSource_IDs[i])));
				pRow->PutValue(mslcName, _bstr_t(m_astrNameIDSource_Names[i]));
				pRow->PutValue(mslcSelected, _variant_t(m_bPreSelectAll ? VARIANT_TRUE : VARIANT_FALSE, VT_BOOL));
				m_dlList->AddRowSorted(pRow, NULL);
			}
		}
		else {
			// (a.walling 2007-03-27 12:39) - PLID 25367 - Allow skipping IDs even if we don't use a source DL.
			if (m_vaIDsToSkip.GetSize() > 0) {
				// We have IDs to skip. Luckily we can just put it into the where clause!
				if (m_strWhere.GetLength() > 0) {
					m_strWhere += " AND ";
				}

				CString strExcludedIDs;
				strExcludedIDs.Format(" %s NOT IN (%s)", m_strIDField, ArrayAsString(m_vaIDsToSkip, true));

				m_strWhere += strExcludedIDs;
			}

			m_dlList->FromClause = (LPCTSTR)m_strFrom;
			m_dlList->WhereClause = (LPCTSTR)m_strWhere;

			IColumnSettingsPtr(m_dlList->GetColumn(mslcID))->FieldName = (LPCTSTR)m_strIDField;
			IColumnSettingsPtr(m_dlList->GetColumn(mslcName))->FieldName = (LPCTSTR)m_strValueField;
			if(m_bPreSelectAll) {
				// (z.manning, 07/26/2007) - PLID 14579 - Select all by default if specified.
				m_dlList->GetColumn(mslcSelected)->FieldName = "CONVERT(bit, 1)";
			}

			//m.hancock - 2/24/2006 - PLID 19428 - Enable text wrapping
			if(m_bWrapText == true)
				IColumnSettingsPtr(m_dlList->GetColumn(mslcName))->FieldType = cftTextWordWrap;

			int index = 2;
			long nExtraColumnFields = 0; // (c.haag 2008-02-13 16:28) - PLID 28855 - This is the count of extra column fields
																				// defined by that which invoked this dialog

			if(m_straryExtraColumnFields.GetSize() > 0 &&
				m_straryExtraColumnFields.GetSize() == m_straryExtraColumnNames.GetSize()) {
				// (s.dhole 2014-03-11 10:50) - PLID 61318 set auto width
				short nWidthStyle = csWidthData;
				if (m_bSetEqualColumnWidth){
					nWidthStyle	= csWidthAuto;
				}
				for(int i=0;i<m_straryExtraColumnFields.GetSize();i++) {
					//add extra columns
					index++;
					IColumnSettingsPtr(m_dlList->GetColumn(m_dlList->InsertColumn(index, _T(_bstr_t(m_straryExtraColumnFields.GetAt(i))), _T(_bstr_t(m_straryExtraColumnNames.GetAt(i))), 50, csVisible|nWidthStyle)))->FieldType = cftTextSingleLine;
				}
				nExtraColumnFields = m_straryExtraColumnFields.GetSize();
			}

			// (c.haag 2008-02-13 16:23) - PLID 28855 - Include user-defined sort columns. If there are none, then we
			// always sort on the name/data column. If there are, we sort ONLY on these user-defined columns.
			if (m_astrOrderColumns.GetSize() > 0 || m_abSortAscending.GetSize() > 0) {
				if (m_astrOrderColumns.GetSize() == m_abSortAscending.GetSize()) {

					// Remove the default sort
					m_dlList->GetColumn(mslcName)->SortPriority = (short)-1;

					// Add the sort columns and sort by them
					const int nColumns = m_astrOrderColumns.GetSize();
					for (int i=0; i < nColumns; i++) {
						index++; // (c.haag 2008-08-29 09:52) - PLID 27337 - Increment the index before, not after, inserting the column
						IColumnSettingsPtr pCol = m_dlList->GetColumn(m_dlList->InsertColumn(index, _T(_bstr_t(m_astrOrderColumns[i])), _bstr_t(m_astrOrderColumns[i]), 0, csVisible|csFixedWidth));
						if (NULL != pCol) {
							pCol->SortPriority = (short)i;
							pCol->SortAscending = (m_abSortAscending[i]) ? VARIANT_TRUE : VARIANT_FALSE;
						}
					}

				} else {
					ASSERT(FALSE); // Bad! Make sure there are as many order columns as there are sorting booleans!
				}
			} else {
				// No user-defined sorting. Just sort by the name/data column, which is the default behavior
			}

			// (c.haag 2007-01-31 11:07) - PLID 24428 - If necessary, add an extra column for
			// color definitions. This must ALWAYS be the last column.
			if (!m_strColorField.IsEmpty()) {
				index++; // (c.haag 2008-08-29 09:52) - PLID 27337 - Increment the index before, not after, inserting the column
				IColumnSettingsPtr pCol = m_dlList->GetColumn(m_dlList->InsertColumn(index, _T(_bstr_t(m_strColorField)), "Color", 0, csVisible|csFixedWidth));
				if (NULL != pCol) {
					pCol->FieldType = cftSetRowForeColor;
				}
			}

			m_dlList->Requery();

			// (c.haag 2008-02-13 16:22) - PLID 28855 - 26012 never considered the case where m_strColorField
			// was not empty, and would perform a width increase on that hidden column. To make it work with
			// my code, I'm going to ensure it only counts towards outside-added columns. All other additional
			// columns should be hidden.
			if(nExtraColumnFields > 0) {

				//we added columns successfully, so now calculate their size
				//and increase the dialog width accordingly			

				// (j.jones 2007-05-15 16:14) - PLID 26012 - add 100 pixels per column added
				// (c.haag 2008-02-13 16:27) - PLID 28855 - Get ready to feel the power of multiplication!
				//for(int i=0;i<nExtraColumnFields;i++) {
				//	nTotalWidthIncrease += 100;
				//}
				long nTotalWidthIncrease = nExtraColumnFields * 100;

				if(nTotalWidthIncrease > 0) {

					//get the dialog size
					CRect rcWindow;
					GetWindowRect(&rcWindow);
					ScreenToClient(&rcWindow);
					int iMaxWidth = GetSystemMetrics(SM_CXSCREEN);
					if(rcWindow.Width() + nTotalWidthIncrease > iMaxWidth) {
						//don't make the dialog wider than the screen
						nTotalWidthIncrease = iMaxWidth - rcWindow.Width();
					}

					//increase the width of the dialog by nTotalWidthIncrease, SWP_NOMOVE will keep it centered
					SetWindowPos(NULL, rcWindow.left, rcWindow.top, rcWindow.Width() + nTotalWidthIncrease, rcWindow.Height(), SWP_NOMOVE|SWP_NOZORDER);

					// (j.armen 2012-06-08 09:35) - PLID 49607 - The window will take care of repositioning the rest of the items OnSize
				}
			}
		}

		if(m_strNameColTitle != "") {
			IColumnSettingsPtr(m_dlList->GetColumn(mslcName))->PutColumnTitle(_bstr_t(m_strNameColTitle));
		}

		BOOL bSelChanged = FALSE;

		for (int i=0; i < m_arPreSelect.GetSize(); i++)
		{
			COleVariant var = m_arPreSelect[i];

			// (j.jones 2009-08-11 18:02) - PLID 35189 - converted m_dlList into a datalist2
			IRowSettingsPtr pRow = m_dlList->FindByColumn(mslcID, var, 0, FALSE);

			if(pRow) {
				pRow->PutValue(mslcSelected, g_cvarTrue); 
				bSelChanged = TRUE;
				//for auditing
				m_aryOldName.Add(pRow->GetValue(mslcName));
			}
		}

		//DRT 1/12/2007 - PLID 24223 - Allow preselect by the name column as well as ID.  Pretty much copied
		//	from the above array with the different field
		for(int j = 0; j < m_arPreSelectByName.GetSize(); j++) {
			CString str = m_arPreSelectByName.GetAt(j);

			// (j.jones 2009-08-11 18:02) - PLID 35189 - converted m_dlList into a datalist2
			IRowSettingsPtr pRow = m_dlList->FindByColumn(mslcName, _bstr_t(str), 0, FALSE);
			if(pRow) {
				pRow->PutValue(mslcSelected, g_cvarTrue);
				bSelChanged = TRUE;
				//for auditing
				m_aryOldName.Add(_variant_t(str));
			}
		}

		if(bSelChanged && pCheckCol->GetSortPriority() != -1) {
			m_dlList->Sort();
		}

		GetDlgItem(IDC_STATIC_DESCRIPTION)->SetWindowText(m_strDescription);

		// (j.armen 2012-06-06 12:50) - PLID 49607 - Set the dlg to remember the dlg's size and position.
		//	Make sure that this is called after any sizing so that the default size of the dlg is actually set correctly
		//	We also are setting a minimum size
		SetMinSize(420,375);
		SetMaxSize(1000,700);

		// The ConfigRT entry should not be left empty.  In some cases, the constructor may have just set it using "" and then later
		// set it via the setter function.  If this assert is hit, the dlg size will still be remembered, just using 'CMultiSelectDlg-'.
		// this will not cause failure of the dlg worthy of exception, but should be corrected by setting the sizing configrt
		ASSERT(!m_strSizingConfigRT.IsEmpty());
		SetRecallSizeAndPosition("CMultiSelectDlg-" + m_strSizingConfigRT, true);
		//Let's keep the dlg centered
		CenterWindow();

	}NxCatchAll(__FUNCTION__);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

HRESULT CMultiSelectDlg::Open(CString strFrom, CString strWhere,
						   CString strIDField, CString strValueField, CString strDescription,						   
						   DWORD nMinSelections /* = 0 */, DWORD nMaxSelections /* = 0xFFFFFFFF */,
						   CStringArray *straryExtraColumnFields /*= NULL*/, CStringArray *straryExtraColumnNames /*= NULL*/,
						   BOOL bAutoCheckMultipleSelections /*= FALSE*/)
{
	m_strFrom = strFrom;
	m_strWhere = strWhere;
	m_strIDField = strIDField;
	m_strValueField = strValueField;
	m_strDescription = strDescription;
	m_nMinSelections = nMinSelections;
	m_nMaxSelections = nMaxSelections;
	if(straryExtraColumnFields)
		m_straryExtraColumnFields.Copy(*straryExtraColumnFields);
	if(straryExtraColumnNames)
		m_straryExtraColumnNames.Copy(*straryExtraColumnNames);
	// (a.walling 2007-06-13 17:07) - PLID 26211 - default to false
	m_bAutoCheckMultipleSelections = bAutoCheckMultipleSelections;
	return DoModal();
}

// (j.jones 2009-08-11 17:59) - PLID 35189 - renamed the datalist parameter to make it more obvious that it is a datalist1
HRESULT CMultiSelectDlg::Open(NXDATALISTLib::_DNxDataList *pDataList1, CVariantArray &vaIDsToSkip, CString strDescription, unsigned long nMinSelections /*= 0*/,
							  unsigned long nMaxSelections /*=0xFFFFFFFF*/)
{
	m_pSourceList.Attach(pDataList1);
	m_strDescription = strDescription;
	m_nMinSelections = nMinSelections;
	m_nMaxSelections = nMaxSelections;
	m_vaIDsToSkip.Copy(vaIDsToSkip);
	return DoModal();
}

// (r.gonet 05/22/2014) - PLID 62250 - Added the ability to open based on a datalist2 with extra column support.
HRESULT CMultiSelectDlg::OpenWithDataList2(NXDATALIST2Lib::_DNxDataListPtr pDataList2, CVariantArray &vaIDsToSkip, CString strDescription, unsigned long nMinSelections/* = 0*/, unsigned long nMaxSelections/* = 0xFFFFFFFF*/,
	short nSourceIDColumnIndex/* = 0*/, short nSourceNameColumnIndex/* = 1*/, CArray<short, short> *parySourceExtraColumnIndices/* = NULL*/,
	BOOL bAutoCheckMultipleSelections/* = FALSE*/)
{
	m_pSourceList2.Attach(pDataList2);
	m_strDescription = strDescription;
	m_nMinSelections = nMinSelections;
	m_nMaxSelections = nMaxSelections;
	m_vaIDsToSkip.Copy(vaIDsToSkip);
	m_nSourceIDColumnIndex = nSourceIDColumnIndex;
	m_nSourceNameColumnIndex = nSourceNameColumnIndex;
	if (parySourceExtraColumnIndices) {
		m_arySourceExtraColumnIndices.Copy(*parySourceExtraColumnIndices);
	}
	// default to false
	m_bAutoCheckMultipleSelections = bAutoCheckMultipleSelections;
	return DoModal();
}

// (c.haag 2011-03-18 16:58) - PLID 42908 - Added the ability to open based on an array of ID's and names
HRESULT CMultiSelectDlg::Open(CArray<long,long>& anIDs, CStringArray& astrNames, CString strDescription, unsigned long nMinSelections /*= 0*/, unsigned long nMaxSelections /*= 0xFFFFFFFF*/)
{
	m_astrNameIDSource_IDs.Copy(anIDs);
	m_astrNameIDSource_Names.Copy(astrNames);
	m_strDescription = strDescription;
	m_nMinSelections = nMinSelections;
	m_nMaxSelections = nMaxSelections;
	return DoModal();
}

// (z.manning, 07/18/2007) - PLID 26727 - Added the ability to populate the list based on a semicolon
// delimited list from a datalist's embedded combo source.
HRESULT CMultiSelectDlg::OpenWithDelimitedComboSource(_bstr_t bstrEmbeddedComboSource, CVariantArray &vaIDsToSkip, CString strDescription, unsigned long nMinSelections /* = 0 */, unsigned long nMaxSelections /* = 0xFFFFFFFF */)
{
	m_strEmbeddedComboSource = (LPCTSTR)bstrEmbeddedComboSource;
	m_strDescription = strDescription;
	m_nMinSelections = nMinSelections;
	m_nMaxSelections = nMaxSelections;
	m_vaIDsToSkip.RemoveAll();
	// (z.manning, 07/18/2007) - PLID 26727 - Go through the IDs to skip and IDs to pre-select arrays
	// and change them to strings so that we know what we'll be dealing with.
	for(int i = 0; i < vaIDsToSkip.GetSize(); i++) {
		_variant_t var;
		var.ChangeType(VT_BSTR, &vaIDsToSkip.GetAt(i));
		m_vaIDsToSkip.Add(var);
	}
	for(i = 0; i < m_arPreSelect.GetSize(); i++) {
		_variant_t var;
		var.ChangeType(VT_BSTR, &m_arPreSelect.GetAt(i));
		m_arPreSelect.SetAt(i, var);
	}
	return DoModal();
}

// (c.haag 2010-11-22 16:50) - PLID 41588 - Added the ability to open the list pulling from a simple string array.
HRESULT CMultiSelectDlg::OpenWithStringArray(CStringArray& astrSource, CString strDescription, unsigned long nMinSelections /* = 0 */, unsigned long nMaxSelections /* = 0xFFFFFFFF */)
{
	m_astrStringArraySource.Copy(astrSource);
	m_strDescription = strDescription;
	m_nMinSelections = nMinSelections;
	m_nMaxSelections = nMaxSelections;
	return DoModal();
}

BOOL CMultiSelectDlg::ValidateAndStore()
{
	DWORD nSelections = 0;

	// Fill the selection map with all of the selected rows
	m_arySelectedRows.RemoveAll();
	m_aryUnselectedRows.RemoveAll();
	m_aryNewName.RemoveAll();

	/*
	for (long i=0; i < m_dlList->GetRowCount(); i++)
	{
		if (m_dlList->GetValue(i, mslcSelected).boolVal != FALSE)
		{
			MultiSelectRow msr;
			msr.varID = m_dlList->GetValue(i,mslcID);
			_variant_t varName = m_dlList->GetValue(i,mslcName);
			msr.varName = varName;
			m_arySelectedRows.Add(msr);
			m_aryNewName.Add(varName);
			nSelections++;
		}
	}
	*/

	// (a.walling 2008-05-06 13:23) - PLID 28805 - Best list iteration method; much faster.
	// (j.jones 2009-08-11 18:22) - PLID 35189 - converted m_dlList into a datalist2
	IRowSettingsPtr pRow = m_dlList->GetFirstRow();
	while(pRow)
	{
		MultiSelectRow msr;
		msr.varID = pRow->GetValue(mslcID);
		_variant_t varName = pRow->GetValue(mslcName);
		msr.varName = varName;
		if(pRow->GetValue(mslcSelected).boolVal != VARIANT_FALSE) {
			m_arySelectedRows.Add(msr);
			m_aryNewName.Add(varName);
			nSelections++;
		}
		else {
			// (z.manning 2011-10-25 14:12) - PLID 39401 - Also keep track of unselected rows
			m_aryUnselectedRows.Add(msr);
		}

		pRow = pRow->GetNextRow();
	}

	if (nSelections < m_nMinSelections)
	{
		MsgBox("You must select at least %d item(s) from the list", m_nMinSelections);
		return FALSE;
	}
	if (nSelections > m_nMaxSelections)
	{
		MsgBox("You cannot select more than %d item(s) from the list", m_nMaxSelections);
		return FALSE;
	}
	
	return TRUE;
}

void CMultiSelectDlg::OnOK() 
{
	// (j.jones 2005-03-25 10:22) - PLID 16102 - If this window was opened with any items preselected,
	// then we will not auto-select the highlighted item OnOK.
	//DRT 1/12/2007 - PLID 24223 - Follows above rule for name preselects too
	// (z.manning 2009-05-22 14:34) - PLID 28554 - Need to also factor in m_bPreSelectAll here
	BOOL bUseHighlighted = ((m_arPreSelect.GetSize() == 0) && (m_arPreSelectByName.GetSize() == 0) && !m_bPreSelectAll);

	// (j.jones 2004-11-24 14:41) - we should be able to highlight one and click ok without having to 
	//check it, if we wanted to (now only if nothing was preselected)
	// (j.jones 2009-08-12 09:13) - PLID 35189 - converted to datalist2
	IRowSettingsPtr pRow = m_dlList->GetCurSel();
	if(bUseHighlighted && pRow != NULL) {
		SetIfOnlyItem(pRow);
	}

	if (ValidateAndStore()) {
		CNxDialog::OnOK();
	}
}

//DRT 1/11/2007 - PLID 24217 - Modified to use the delimiter as a parameter, rather than
//	a hardcoded ", "
CString CMultiSelectDlg::GetMultiSelectString(CString strDelimiter /*= ", "*/)
{
	CString strMultiName;
	for (int i=0; i < m_aryNewName.GetSize(); i++)
	{
		strMultiName += AsString(m_aryNewName[i]) + strDelimiter;
	}

	if (strMultiName.GetLength() >= strDelimiter.GetLength())
		strMultiName = strMultiName.Left( strMultiName.GetLength() - strDelimiter.GetLength() );
	return strMultiName;
}

CString CMultiSelectDlg::GetMultiSelectIDString(const CString &strSeparator /*= " "*/)
{
	CString strMultiID;
	for(int i = 0; i < m_arySelectedRows.GetSize(); i++)
	{
		strMultiID += AsString(m_arySelectedRows[i].varID) + strSeparator;
	}

	if (strMultiID.GetLength() >= strSeparator.GetLength())
		strMultiID = strMultiID.Left( strMultiID.GetLength() - strSeparator.GetLength() );
	return strMultiID;
}

void CMultiSelectDlg::FillArrayWithIDs(CArray<long,long> &arIDs)
{
	arIDs.RemoveAll();
	for(int i = 0; i < m_arySelectedRows.GetSize(); i++) {
		arIDs.Add(AsLong(m_arySelectedRows[i].varID));
	}
}

void CMultiSelectDlg::FillArrayWithIDs(CDWordArray& adw)
{
	adw.RemoveAll();
	for(int i = 0; i < m_arySelectedRows.GetSize(); i++)
	{
		adw.Add((DWORD)AsLong(m_arySelectedRows[i].varID));
	}
}

void CMultiSelectDlg::FillArrayWithIDs(CVariantArray& ar)
{
	ar.RemoveAll();
	for(int i = 0; i < m_arySelectedRows.GetSize(); i++)
	{
		ar.Add(m_arySelectedRows[i].varID);
	}
}

// (c.haag 2015-01-05) - PLID 64257
void CMultiSelectDlg::FillArrayWithIDs(std::set<long>& arIDs)
{
	arIDs.clear();
	for (int i = 0; i < m_arySelectedRows.GetSize(); i++)
	{
		arIDs.insert(m_arySelectedRows[i].varID);
	}
}

// (z.manning 2011-10-25 14:09) - PLID 39401 - If you want the unselected IDs
void CMultiSelectDlg::FillArrayWithUnselectedIDs(CArray<long,long> *parynIDs)
{
	parynIDs->RemoveAll();
	for(int nUnselectedIndex = 0; nUnselectedIndex < m_aryUnselectedRows.GetCount(); nUnselectedIndex++)
	{
		MultiSelectRow msr = m_aryUnselectedRows.GetAt(nUnselectedIndex);
		parynIDs->Add(msr.varID);
	}
}

// (r.gonet 2016-01-22 15:05) - PLID 68041 - Gets the selected IDs as a variant array.
void CMultiSelectDlg::FillArrayWithUnselectedIDs(CVariantArray *paryIDs)
{
	paryIDs->RemoveAll();
	for (int nUnselectedIndex = 0; nUnselectedIndex < m_aryUnselectedRows.GetCount(); nUnselectedIndex++) {
		MultiSelectRow msr = m_aryUnselectedRows.GetAt(nUnselectedIndex);
		paryIDs->Add(msr.varID);
	}
}

//TES 5/22/2009 - PLID 34302 - Did you know that the declaration for this function has been in the .h file for 
// over 3 years, but it was never implemented until now?  True story!
void CMultiSelectDlg::FillArrayWithNames(CVariantArray& ar)
{
	ar.RemoveAll();
	for(int i = 0; i < m_arySelectedRows.GetSize(); i++)
	{
		ar.Add(m_arySelectedRows[i].varName);
	}
}


// (a.walling 2007-03-27 11:58) - PLID 25367 - Need a simple way to set IDs to skip
// Do not add any of the IDs in this array to the list.
void CMultiSelectDlg::SkipIDsInArray(CDWordArray &adwIDs)
{
	for (int i = 0; i < adwIDs.GetSize(); i++) {
		m_vaIDsToSkip.Add(_variant_t(static_cast<long>(adwIDs.GetAt(i))));
	}
}

// (a.walling 2007-03-27 11:58) - PLID 25367 - Need a simple way to set IDs to skip
// Do not add any of the IDs in this array to the list.
void CMultiSelectDlg::SkipIDsInArray(CArray<long,long> &arIDs)
{
	for (int i = 0; i < arIDs.GetSize(); i++) {
		m_vaIDsToSkip.Add(_variant_t(arIDs.GetAt(i), VT_I4));
	}
}

// (a.walling 2007-03-27 11:58) - PLID 25367 - Need a simple way to set IDs to skip
// Do not add any of the IDs in this array to the list.
void CMultiSelectDlg::SkipIDsInArray(CVariantArray &arIDs)
{
	for (int i = 0; i < arIDs.GetSize(); i++) {
		m_vaIDsToSkip.Add(arIDs.GetAt(i));
	}
}

void CMultiSelectDlg::OnOtherBtn() 
{
	if (ValidateAndStore()) {
		EndDialog(rvOtherBtn);
	}
}

void CMultiSelectDlg::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	try {
		// Handle the context menu
		if (m_pfnContextMenuProc && pWnd->GetSafeHwnd() && pWnd->GetSafeHwnd() == GetDlgItem(IDC_MULTI_SELECT_LIST)->GetSafeHwnd()) {
			IRowSettingsPtr pRow = m_dlList->GetCurSel();
			if(pRow) {
				m_pfnContextMenuProc(this, m_nContextMenuProcParam, pRow, pWnd, point, m_aryOtherChangedMasterIDs);
			} else {
				m_pfnContextMenuProc(this, m_nContextMenuProcParam, NULL, pWnd, point, m_aryOtherChangedMasterIDs);
			}
		}
	} NxCatchAll("CMultiSelectDlg::OnContextMenu");
}

// (j.jones 2009-08-12 08:45) - PLID 35189 - changed the parameter to be a row
BOOL CMultiSelectDlg::SetIfOnlyItem(NXDATALIST2Lib::IRowSettingsPtr pSelRow)
{
	if(pSelRow == NULL) {
		return FALSE;
	}

	// (j.jones 2005-03-25 10:21) - PLID 16102 - will only be called if from
	// double-clicking or if nothing was originally preselected

	//When given a row index, check and see if any other rows are selected.
	//If so, return FALSE, as it is not the only item.
	//If not, verify the row is checked, and return TRUE.

	//if we were passed a count of mininum selections, then ignore this function,
	//as we can never use this function if you require more than one selection
	if(m_nMinSelections > 1)
		return FALSE;

	/*
	for(int i=0;i<m_dlList->GetRowCount();i++) {
		if(nRowIndex != i && VarBool(m_dlList->GetValue(i,mslcSelected))) {
			//a row aside from this one is checked, so don't allow this functionality
			return FALSE;
		}
	}*/

	// (a.walling 2007-06-13 17:09) - PLID 26211 - For some reason this is significantly faster on Vista debug mode
	IRowSettingsPtr pFoundRow = m_dlList->FindByColumn(mslcSelected, _variant_t(VARIANT_TRUE, VT_BOOL), 0, FALSE);

	if(pFoundRow == NULL) {
		if(m_bAutoCheckMultipleSelections) {
			// (a.walling 2007-06-13 17:00) - PLID 26211 - If using this option, automatically check
			// (j.jones 2009-08-11 18:22) - PLID 35189 - converted m_dlList into a datalist2
			IRowSettingsPtr pRow = m_dlList->GetFirstSelRow();
			long nCount = 0;
			while(pRow) {
				nCount++;
				pRow->PutValue(mslcSelected, _variant_t(VARIANT_TRUE, VT_BOOL));

				pRow = pRow->GetNextSelRow();
			}

			return nCount > 0 ? TRUE : FALSE;
		} else { 
			// (a.walling 2007-06-13 17:00) - PLID 26211 - If there are multiple items selected,
			// don't try to choose a particular one, just ignore
			// (j.jones 2009-08-11 18:22) - PLID 35189 - converted m_dlList into a datalist2
			IRowSettingsPtr pRow = m_dlList->GetFirstSelRow();
			long nCount = 0;
			while(pRow && nCount < 2) {
				nCount++;

				pRow = pRow->GetNextSelRow();
			}

			if (nCount >= 2) {
				return FALSE;
			}

			//we are indeed allowed to proceed, so check the box if it is unchecked, and commit the change
			IRowSettingsPtr pFirstRow = m_dlList->GetFirstSelRow();
			if(pFirstRow) {
				pFirstRow->PutValue(mslcSelected, _variant_t(VARIANT_TRUE, VT_BOOL));
			}

			return TRUE;
		}
	} else {
		// Another row is checked
		return FALSE;
	}
}

BOOL CMultiSelectDlg::PreTranslateMessage(MSG* pMsg) 
{
	switch(pMsg->message) {
	case WM_KEYDOWN:
		if(pMsg->wParam == VK_RETURN) {
			// (j.jones 2005-03-25 10:22) - PLID 16102 - If this window was opened with any items preselected,
			// then we will not auto-select the highlighted item OnOK.
			//DRT 1/12/2007 - PLID 24223 - Follow the same rule for name preselects too

			BOOL bUseHighlighted = ((m_arPreSelect.GetSize() == 0) && (m_arPreSelectByName.GetSize() == 0));

			// (j.jones 2004-11-24 14:41) - we should be able to highlight one and click ok without having to 
			//check it, if we wanted to (now only if nothing was preselected)
			// (j.jones 2009-08-12 08:46) - PLID 35189 - converted to a datalist2
			IRowSettingsPtr pRow = m_dlList->GetCurSel();
			if(bUseHighlighted && pRow != NULL) {
				if(SetIfOnlyItem(pRow)) {
					OnOK();
				}
			}
		}
		break;
	}
	
	return CNxDialog::PreTranslateMessage(pMsg);
}

// (j.jones 2009-08-12 08:29) - PLID 35189 - converted to a datalist2
void CMultiSelectDlg::OnRButtonDownMultiSelectList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		if (m_pfnContextMenuProc) {
			// Get the pointer to the window, because we'll be using it more than once
			CWnd *pWnd = GetDlgItem(IDC_MULTI_SELECT_LIST);
			// Give the datalist focus
			pWnd->SetFocus();
			// Set the selection to the row that was clicked on
			// (j.jones 2009-08-11 18:22) - PLID 35189 - converted m_dlList into a datalist2
			IRowSettingsPtr pRow(lpRow);
			m_dlList->PutCurSel(pRow);
		}
	} NxCatchAll("CMultiSelectDlg::OnRButtonDownMultiSelectList");
}

// (j.jones 2009-08-12 08:29) - PLID 35189 - converted to a datalist2
void CMultiSelectDlg::OnDblClickCellMultiSelectList(LPDISPATCH lpRow, short nColIndex)
{
	try {

		IRowSettingsPtr pRow(lpRow);

		if(pRow == NULL) {
			return;
		}

		// (j.jones 2005-03-25 10:21) - PLID 16102 - always allow the double-click to work
		// regardless of whether or not anything was originally preselected

		//if they double-click a row, we should select it and return IF no other rows are checked
		//it doesn't matter if this row is checked
		if(SetIfOnlyItem(pRow)) {
			OnOK();
		}

	}NxCatchAll("CMultiSelectDlg::OnDblClickCellMultiSelectList");
}

// (j.jones 2009-08-12 08:29) - PLID 35189 - converted to a datalist2
void CMultiSelectDlg::OnRequeryFinishedMultiSelectList(short nFlags)
{
	try {
	
		// (j.jones 2007-03-14 15:21) - PLID 25195 - if m_nIDToSelect is set, 
		// set the corresponding row as the current selection
		if(m_nIDToSelect != -1) {
			COleVariant var = m_nIDToSelect;
			IRowSettingsPtr pRow = m_dlList->SetSelByColumn(mslcID, var);

			if(pRow) {
				m_dlList->EnsureRowInView(pRow);
			}
		}
		// (j.jones 2007-10-05 14:56) - PLID 25195 - removed this functionality,
		// because otherwise the user could hit enter and auto-select this row,
		// when they never chose it to begin with
		/*
		else if (m_dlList->GetRowCount() > 0)
			m_dlList->PutCurSel(0);
		*/

	}NxCatchAll("Error in CMultiSelectDlg::OnRequeryFinishedMultiSelectList");
}

// (z.manning 2009-08-19 17:35) - PLID 19932 - Added an optional filter button
void CMultiSelectDlg::OnFilterBtn()
{
	try
	{
		if(m_bFiltered) {
			// (z.manning 2009-08-19 17:52) - PLID 17932 - List is filtered so unfilter
			for (short nCol = 0; nCol < m_dlList->GetColumnCount(); nCol++) {
				m_dlList->GetColumn(nCol)->PutBackColor(RGB(255,255,255));
			}
			m_dlList->PutWhereClause(_bstr_t(m_strWhere));
			m_dlList->Requery();
			m_bFiltered = FALSE;
			m_btnFilter.SetWindowText("Filter");
		}
		else {
			// (z.manning 2009-08-19 17:52) - PLID 17932 - List is unfiltered so prompt to filter
			if(FilterDatalist2(m_dlList, mslcName, mslcID)) {
				m_bFiltered = TRUE;
				m_btnFilter.SetWindowText("Unfilter");
			}
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2015-03-02 15:48) - PLID 65093 - detect when they checked a box
void CMultiSelectDlg::OnEditingFinishedMultiSelectList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if (pRow == NULL) {
			return;
		}

		if (nCol == CMultiSelectDlg::mslcSelected) {

			// (j.jones 2015-03-02 15:51) - PLID 65093 - if the maximum selection is 1, and another box is
			// already checked, uncheck the existing box when a new one is checked
			if (m_nMaxSelections == 1 && varNewValue.vt == VT_BOOL && VarBool(varNewValue, false)) {
				//uncheck all existing boxes, except for this row
				m_dlList->WaitForRequery(dlPatienceLevelWaitIndefinitely);
				IRowSettingsPtr pRowToUncheck = m_dlList->GetFirstRow();
				while (pRowToUncheck) {

					if (pRowToUncheck != pRow) {
						pRowToUncheck->PutValue(CMultiSelectDlg::mslcSelected, _variant_t(VARIANT_FALSE, VT_BOOL));
					}

					pRowToUncheck = pRowToUncheck->GetNextRow();
				}
			}
		}

	}NxCatchAll(__FUNCTION__);
}

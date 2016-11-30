// (r.gonet 12/02/2013) - PLID 59830 - Added.

// PatientListsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "Groups.h"
#include "FilterEditDlg.h"
#include "Filter.h"
#include "FilterDetail.h"
#include "FilterFieldInfo.h"
#include "PatientListsDlg.h"
#include "FilterDlg.h"
#include "NxUILib\DatalistUtils.h"
using namespace ADODB;
using namespace NXDATALIST2Lib;

// CPatientListsDlg dialog

IMPLEMENT_DYNAMIC(CPatientListsDlg, CNxDialog)

// (r.gonet 12/02/2013) - PLID 59830 - Constructs a new CPatientListsDlg
CPatientListsDlg::CPatientListsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CPatientListsDlg::IDD, pParent)
{
}

// (r.gonet 12/02/2013) - PLID 59830 - Destructor
CPatientListsDlg::~CPatientListsDlg()
{
}

// (r.gonet 12/02/2013) - PLID 59830
void CPatientListsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PATIENT_LISTS_COLOR, m_nxcolorBackground);
	DDX_Control(pDX, IDC_PATIENT_LISTS_FILTER_HEADER_STATIC, m_nxstaticFilterHeader);
	DDX_Control(pDX, IDC_PATIENT_LISTS_NEW_FILTER_BTN, m_btnNew);
	DDX_Control(pDX, IDC_PATIENT_LISTS_EDIT_FILTER_BTN, m_btnEdit);
	DDX_Control(pDX, IDC_PATIENT_LISTS_DELETE_FILTER_BTN, m_btnDelete);
	DDX_Control(pDX, IDC_PATIENT_LISTS_MATCHES_STATIC, m_nxstaticMatchesHeader);
	DDX_Control(pDX, IDC_PATIENT_LISTS_COUNT_STATIC, m_nxstaticCountLabel);
	DDX_Control(pDX, IDC_PATIENT_LISTS_COPY_TO_CLIP_BTN, m_btnCopyToClipboard);
	DDX_Control(pDX, IDCANCEL, m_btnClose);
}


BEGIN_MESSAGE_MAP(CPatientListsDlg, CNxDialog)
	ON_WM_CREATE()
	ON_BN_CLICKED(IDC_PATIENT_LISTS_NEW_FILTER_BTN, &CPatientListsDlg::OnBnClickedPatientListsNewFilterBtn)
	ON_BN_CLICKED(IDC_PATIENT_LISTS_EDIT_FILTER_BTN, &CPatientListsDlg::OnBnClickedPatientListsEditFilterBtn)
	ON_BN_CLICKED(IDC_PATIENT_LISTS_DELETE_FILTER_BTN, &CPatientListsDlg::OnBnClickedPatientListsDeleteFilterBtn)
	ON_BN_CLICKED(IDC_PATIENT_LISTS_COPY_TO_CLIP_BTN, &CPatientListsDlg::OnBtnCopyToClipboard)
END_MESSAGE_MAP()


// CPatientListsDlg message handlers

// (r.gonet 12/02/2013) - PLID 59830 - Initializes the dialog
BOOL CPatientListsDlg::OnInitDialog()
{
	try {
		CNxDialog::OnInitDialog();

		m_nxcolorBackground.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));

		m_btnNew.AutoSet(NXB_NEW);
		m_btnEdit.AutoSet(NXB_MODIFY);
		m_btnDelete.AutoSet(NXB_DELETE);
		m_btnClose.AutoSet(NXB_CLOSE);
		// (j.jones 2014-05-20 16:40) - PLID 61814 - added clipboard functionality
		m_btnCopyToClipboard.AutoSet(NXB_EXPORT);
		
		m_pFilterCombo = BindNxDataList2Ctrl(IDC_PATIENT_LISTS_FILTER_COMBO, false);
		m_pPatientList = BindNxDataList2Ctrl(IDC_PATIENT_LISTS_PATIENTS_DL, false);

		// (r.gonet 12/02/2013) - PLID 59830 - Fill the filter combo and select the one time filter.
		FillFilterCombo(FILTER_ID_TEMPORARY);
	} NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CPatientListsDlg, CNxDialog)
	ON_EVENT(CPatientListsDlg, IDC_PATIENT_LISTS_FILTER_COMBO, 16, CPatientListsDlg::SelChosenPatientListsFilterCombo, VTS_DISPATCH)
	ON_EVENT(CPatientListsDlg, IDC_PATIENT_LISTS_FILTER_COMBO, 1, CPatientListsDlg::SelChangingPatientListsFilterCombo, VTS_DISPATCH VTS_PDISPATCH)
END_EVENTSINK_MAP()

// (r.gonet 12/02/2013) - PLID 58821 - Add the dynamic columns of the datalist given the recordset of the select statement and a map of the fields in the select statement.
// Returns a map of the recordset column indexes to datalist column indexes.
void CPatientListsDlg::LoadPatientListColumns(_RecordsetPtr prs, CMapStringToField &mapSelectedFields, CMap<long, long, short, short> &mapRsFieldIndexToDatalistColIndex)
{
	if(prs->GetState() != adStateClosed) {
		long nFieldCount = prs->Fields->Count;
		short nDatalistColumnIndex = 0;
		// Go through all the columns in the recordset and add each, up to 256 columns, to the datalist.
		for (long nRecordsetFieldIndex = 0; nRecordsetFieldIndex < nFieldCount && nDatalistColumnIndex < 256; nRecordsetFieldIndex++) {
			ADODB::FieldPtr pDatabaseField = prs->Fields->Item[nRecordsetFieldIndex];
			
			CString strFieldName = (LPCSTR)pDatabaseField->Name;
			// We should have a corresponding field in the select map
			CSelectedFieldPtr pSelectedField;
			if(!mapSelectedFields.Lookup(strFieldName, pSelectedField)) {
				ThrowNxException("%s : The column '%s' in the recordset is not present in the selected fields map!", __FUNCTION__, strFieldName);
			}

			// Determine the properties of the column we need to add to the datalist
			ADODB::DataTypeEnum dteFieldType = pDatabaseField->Type;
			NXDATALIST2Lib::EColumnFieldType cftColumnFieldType;
			VARTYPE vtColumnDataType;
			// Get an appropriate datalist field type
			AdoType2DatalistFieldType(pSelectedField, dteFieldType, cftColumnFieldType);
			// Get the variant type for this column
			if(AdoType2DatalistDataType(dteFieldType, vtColumnDataType)) {
				CString strColumnName = strFieldName;
				if(pSelectedField->GetBaseType() == fboPerson) {
					// When adding the person related columns, don't bother prefixing the fields.
					strColumnName = pSelectedField->GetUnaffixedFieldName();
				}

				// Calculate the width of the column based on the size of the field name string
				short nInsertedDatalistColumnIndex;
				long nColumnWidth = 0;
				DWORD dwColumnFlags = 0;
				if(!pSelectedField->GetIsKey()) {
					// Non-key columns always get displayed.
					CDC *pdc = GetDC();
					CSize szColumnNameSize = ::GetTabbedTextExtent(*pdc, (LPCSTR)strColumnName, strColumnName.GetLength(), 0, NULL);
					szColumnNameSize.cx += 20; // Need some space for the sort icon
					long nMinColumnWidth = 60;
					long nMaxColumnWidth = 255;
					nColumnWidth = szColumnNameSize.cx;
					// Don't go past our limts for max and min column size.
					if(nColumnWidth < nMinColumnWidth) {
						nColumnWidth = nMinColumnWidth;
					}
					if(nColumnWidth > nMaxColumnWidth) {
						nColumnWidth = nMaxColumnWidth;
					}

					// Try to determine if we should use a fixed size or a size to data.
					// For NVARCHAR(MAX) fields we obviously want to use a fixed size.
					long nDefinedSize = pDatabaseField->DefinedSize;
					CSize szCharacterSize = ::GetTabbedTextExtent(*pdc, "W", 1, 0, NULL);
					long long nDefinedWidth = szCharacterSize.cx * nDefinedSize;

					dwColumnFlags |= csVisible;
					if(nDefinedWidth <= nMaxColumnWidth) {
						dwColumnFlags |= csWidthData;
					}
				} else {
					// Key columns are hidden since they are just ids
					nColumnWidth = 0;
					dwColumnFlags |= csFixedWidth;
				}
				// Add the column to the datalist
				nInsertedDatalistColumnIndex = m_pPatientList->InsertColumn(nDatalistColumnIndex, _bstr_t(strColumnName), _bstr_t(strColumnName), nColumnWidth, dwColumnFlags);
				IColumnSettingsPtr pCol = m_pPatientList->GetColumn(nInsertedDatalistColumnIndex);
				pCol->FieldType = cftColumnFieldType;
				pCol->DataType = vtColumnDataType;
				// Map the datalist column index back to the recordset field index
				mapRsFieldIndexToDatalistColIndex.SetAt(nRecordsetFieldIndex, nDatalistColumnIndex);
				nDatalistColumnIndex++;
			} else {
				// Don't display this value in the datalist
				mapRsFieldIndexToDatalistColIndex.SetAt(nRecordsetFieldIndex, -1);
			}						
		}
	}
}

// (r.gonet 12/02/2013) - PLID 58821 - Gets the corresponding datalist field type given a selected field and a database type.
void CPatientListsDlg::AdoType2DatalistFieldType(CSelectedFieldPtr pField, ADODB::DataTypeEnum dteFieldType, OUT NXDATALIST2Lib::EColumnFieldType &cftColumnFieldType)
{
	const CFilterFieldInfo *pFilterFieldInfo = NULL;
	if(pField->GetFilterFieldInfoID() >= 0) {
		long nFilterFieldInfoIndex = CFilterFieldInfo::GetInfoIndex(pField->GetFilterFieldInfoID(), (CFilterFieldInfo*)CFilterDetail::g_FilterFields, CFilterDetail::g_nFilterFieldCount);
		if(nFilterFieldInfoIndex >= 0 && nFilterFieldInfoIndex < CFilterDetail::g_nFilterFieldCount) {
			pFilterFieldInfo = &(CFilterDetail::g_FilterFields[nFilterFieldInfoIndex]);
		}
	}

	switch(dteFieldType)
	{
	case adFileTime:
		cftColumnFieldType = NXDATALIST2Lib::cftTextSingleLine;
		break;
	case adBoolean:
		cftColumnFieldType = NXDATALIST2Lib::cftBoolTrueFalse;
		break;
	case adDate:
		if(pFilterFieldInfo != NULL) {
			// We have some additional information about how the column should be displayed
			// given to us by the filter field
			if(pFilterFieldInfo->m_ftFieldType == ftDate) {
				cftColumnFieldType = cftDateShort;
			} else if(pFilterFieldInfo->m_ftFieldType == ftTime) {
				cftColumnFieldType = cftTime;
			} else {
				cftColumnFieldType = cftDateAndTime;
			}
		} else {
			cftColumnFieldType = NXDATALIST2Lib::cftDateAndTime;
		}
		break;
	case adDBDate:
		cftColumnFieldType = NXDATALIST2Lib::cftDateShort;
		break;
	case adDBTime:
		cftColumnFieldType = NXDATALIST2Lib::cftTime;
		break;
	case adDBTimeStamp:
		if(pFilterFieldInfo != NULL) {
			// We have some additional information about how the column should be displayed
			// given to us by the filter field
			if(pFilterFieldInfo->m_ftFieldType == ftDate) {
				cftColumnFieldType = cftDateShort;
			} else if(pFilterFieldInfo->m_ftFieldType == ftTime) {
				cftColumnFieldType = cftTime;
			} else {
				cftColumnFieldType = cftDateAndTime;
			}
		} else {
			cftColumnFieldType = NXDATALIST2Lib::cftDateAndTime;
		}
		break;
	default:
		{
			cftColumnFieldType = NXDATALIST2Lib::cftTextSingleLine;
		}
		break;
	}
}

// (r.gonet 12/02/2013) - PLID 58821 - Gets the corresponding variant type given a database type. Returns true if a variant type could be
// determined and false if one could not.
bool CPatientListsDlg::AdoType2DatalistDataType(ADODB::DataTypeEnum dteFieldType, OUT VARTYPE &vt)
{
	vt = VT_EMPTY;
	switch(dteFieldType)
	{
		case adBigInt:
			vt = VT_I8;
			return true;
		case adBinary:
			return false;
		case adBoolean:
			vt = VT_BOOL;
			return true;
		case adBSTR:
			vt = VT_BSTR;
			return true;
		case adChapter:
			return false;
		case adChar:
			vt = VT_BSTR;
			return true;
		case adCurrency:
			vt = VT_CY;
			return true;
		case adDate:
			vt = VT_DATE;
			return true;
		case adDBDate:
			vt = VT_DATE;
			return true;
		case adDBTime:
			vt = VT_DATE;
			return true;
		case adDBTimeStamp:
			vt = VT_DATE;
			return true;
		case adDecimal:
			vt = VT_R8;
			return true;
		case adDouble:
			vt = VT_R4;
			return true;
		case adEmpty:
			return false;
		case adError:
			return false;
		case adFileTime:
			return false;
		case adGUID:
			vt = VT_BSTR;
			return true;
		case adIDispatch:
			return false;
		case adInteger:
			vt = VT_I4;
			return true;
		case adIUnknown:
			return false;
		case adLongVarBinary:
			return false;
		case adLongVarChar:
			vt = VT_BSTR;
			return true;
		case adLongVarWChar:
			vt = VT_BSTR;
			return true;
		case adNumeric:
			vt = VT_R8;
			return true;
		case adPropVariant:
			return false;
		case adSingle:
			vt = VT_R4;
			return true;
		case adSmallInt:
			vt = VT_R8;
			return true;
		case adTinyInt:
			vt = VT_I1;
			return true;
		case adUnsignedBigInt:
			vt = VT_UI8;
			return true;
		case adUnsignedInt:
			vt = VT_UI4;
			return true;
		case adUnsignedSmallInt:
			vt = VT_UI2;
			return true;
		case adUnsignedTinyInt:
			vt = VT_UI1;
			return true;
		case adUserDefined:
			return false;
		case adVarBinary:
			return false;
		case adVarChar:
			vt = VT_BSTR;
			return true;
		case adVariant:
			return false;
		case adVarNumeric:
			vt = VT_R8;
			return true;
		case adVarWChar:
			vt = VT_BSTR;
			return true;
		case adWChar:
			vt = VT_BSTR;
			return true;
		default:
			return false;
	}
}

// (r.gonet 12/02/2013) - PLID 59830 - Opens the filter editor dialog. If bNewFilter is true, then the filter will be a new filter. If bNewFilter is false, then we are editing an existing filter.
void CPatientListsDlg::OpenFilterEditor(bool bNewFilter)
{
	long nFilterID;
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pFilterCombo->CurSel;
	if(!pRow) {
		return;
	} else {
		nFilterID = VarLong(pRow->GetValue(eplfcID));
	}

	CFilterEditDlg dlg(this, fboPerson, CGroups::IsActionSupported, CGroups::CommitSubfilterAction, NULL, "Patient Filter");
	int nResult = 0;
	if (bNewFilter) {
		nResult = dlg.NewFilter();
	} else {	
		nResult = dlg.EditFilter(nFilterID, (nFilterID == FILTER_ID_TEMPORARY ? (LPCTSTR)m_strOneTimeFilter : NULL));
	}

	if (nResult == IDOK) {
		FillFilterCombo(dlg.GetFilterId(), dlg.m_strFilterString);
		ChangeFilterSelection(dlg.GetFilterId());
	} else {
		// Stay with the current filter
	}
}

// (r.gonet 12/02/2013) - PLID 59830 - Populates the filter combo with the databases' letter writing filters.
void CPatientListsDlg::FillFilterCombo(long nSelectId, LPCTSTR strFilterString/*= NULL*/)
{
	CWaitCursor wc;	

	// Prepare variables
	int nIndex = 0;
	try {
		m_pFilterCombo->Requery();
		m_pFilterCombo->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);;
		
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pFilterCombo->GetNewRow();
		pRow->PutValue(eplfcID, (long)FILTER_ID_TEMPORARY);
		pRow->PutValue(eplfcName, _bstr_t("{One-Time Filter}"));
		m_pFilterCombo->AddRowBefore(pRow, m_pFilterCombo->GetFirstRow());
		
		if(nSelectId == FILTER_ID_TEMPORARY) {
			if(strFilterString) {
				m_strOneTimeFilter = strFilterString;
			} else {
				m_strOneTimeFilter = "";
			}
		} else {
			// No need to store any values since the other filters are stored in the database.
		}

		// Set the current filter to be what was specified above or default
		pRow = m_pFilterCombo->FindByColumn(eplfcID, nSelectId, 0, VARIANT_FALSE);
		if(pRow == NULL) {
			//Our selected ID wasn't valid (possibly it just got deleted).  Use the default.
			nSelectId = FILTER_ID_TEMPORARY;
		}
		m_pFilterCombo->SetSelByColumn(eplfcID, nSelectId);
		EnsureControls();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 12/02/2013) - PLID 59830 - Refreshes the data so as to ensure that it is consistent with the specified filter ID (-1 indicates NEW GROUP)
void CPatientListsDlg::ChangeFilterSelection(long nFilterId)
{
	CWaitCursor wc;

	// (r.gonet 12/02/2013) - PLID 59830 - Get the filter string for the chosen filter
	NXDATALIST2Lib::IRowSettingsPtr pFilterRow = m_pFilterCombo->CurSel;
	if(!pFilterRow) {
		ThrowNxException("%s : pFilterRow is NULL!", __FUNCTION__);
	}
	long nFilterID = VarLong(pFilterRow->GetValue(eplfcID));
	CString strFilter;
	if(nFilterID == FILTER_ID_TEMPORARY) {
		strFilter = m_strOneTimeFilter;
	} else {
		_RecordsetPtr prs = CreateRecordset("SELECT Name, Filter FROM FiltersT WHERE ID = %li", nFilterId);
		if (!prs->eof) {
			// (r.gonet 12/02/2013) - PLID 59830 - Get the filter SQL string to filter on
			strFilter = AdoFldString(prs, "Filter");
		} else {
			MessageBox("The current filter could not be found. It may have been deleted by another user.", "Error", MB_ICONERROR|MB_OK);
			return;
		}
	}

	if(strFilter.IsEmpty()) {
		// (r.gonet 12/02/2013) - PLID 59830 - Nothing to filter on. Load nothing by default.
		ClearResultsList();
		return;
	}

	// (r.gonet 12/02/2013) - PLID 59830 - Create a filter object from the filter string
	CFilterPtr pFilter(new CFilter(nFilterId, fboPerson, CGroups::IsActionSupported, CGroups::CommitSubfilterAction, NULL));
	pFilter->SetFilterString(strFilter);

	boost::scoped_ptr<CFilteredSelect> pFilteredSelect(new CFilteredSelect(pFilter));
	if(!pFilteredSelect->Create()) {
		MessageBox("The current filter cannot be used for selection.", "Error", MB_ICONERROR|MB_OK);
		return;				
	}

	// (r.gonet 12/02/2013) - PLID 59830 - Clean up any previous filter's results
	ClearResultsList();
	CString strCount = FormatString("Selecting patients...");
	m_nxstaticCountLabel.SetWindowText(strCount);
	// (r.gonet 12/02/2013) - PLID 59830 - Pump the message queue so the label redraws
	PeekAndPump();

	// (r.gonet 12/02/2013) - PLID 59830 - Get the SQL string from the query
	CString strSqlSelectStatement = pFilteredSelect->ToString();
	_RecordsetPtr prs = CreateParamRecordset(strSqlSelectStatement);

	// (r.gonet 12/02/2013) - PLID 58821 - Load the datalist columns
	CMap<long, long, short, short> mapRsFieldIndexToDatalistColIndex;
	LoadPatientListColumns(prs, pFilteredSelect->GetSelectedFieldsMap(), mapRsFieldIndexToDatalistColIndex);
	
	// (r.gonet 12/02/2013) - PLID 59830 - The number of records may be greater than the number of patients. For instance if
	// the select returns 1 patient with 2 appointments. The top level count here will be the number of patients.
	long nRecordCount = 0, nTopLevelCount = -1;
	if(!prs->eof) {
		// (r.gonet 12/02/2013) - PLID 59830 - Get the number fo raw records
		nRecordCount = prs->RecordCount;
	}
	while(!prs->eof) {
		// (r.gonet 12/02/2013) - PLID 59830 - Go through all the records and add them to the datalist
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pPatientList->GetNewRow();
		long nFieldCount = prs->Fields->Count;
		short nDisplayedCol = 0;
		for (long nRecordsetFieldIndex = 0; nRecordsetFieldIndex < nFieldCount && nDisplayedCol < 256; nRecordsetFieldIndex++) {
			short nDatalistColumnIndex;
			// Get the datalist column that is mapped to this recordset column
			if(mapRsFieldIndexToDatalistColIndex.Lookup(nRecordsetFieldIndex, nDatalistColumnIndex) && nDatalistColumnIndex != -1) {
				pRow->PutValue(nDatalistColumnIndex, prs->Fields->Item[nRecordsetFieldIndex]->Value);
				nDisplayedCol++;
			} else {
				// (r.gonet 12/02/2013) - PLID 59830 - The column is not displayable
			}
		}
		m_pPatientList->AddRowAtEnd(pRow, NULL);
		prs->MoveNext();
	}
	prs = prs->NextRecordset(NULL);
	if(!prs->eof) {
		// (r.gonet 12/02/2013) - PLID 59830 - The filtered select helpfully selects the number of top level records (patients in this case)
		nTopLevelCount = AdoFldLong(prs->Fields, "TopLevelCount");
	}
	prs->Close();

	// (r.gonet 12/02/2013) - PLID 59830 - Set the count of records label
	if(nTopLevelCount == -1) {
		strCount = FormatString("Count: %li record(s)", nRecordCount);
	} else {
		if(nTopLevelCount == nRecordCount) {
			strCount = FormatString("Count: %li patient(s)", nTopLevelCount);
		} else {
			strCount = FormatString("Count: %li patient(s), %li record(s)", nTopLevelCount, nRecordCount);
		}
	}
	m_nxstaticCountLabel.SetWindowText(strCount);
}

// (r.gonet 12/02/2013) - PLID 59830 - Clears the results list, removes all the dynamic columns, and resets the count label.
void CPatientListsDlg::ClearResultsList()
{
	m_pPatientList->Clear();
	while(m_pPatientList->ColumnCount > 0) {
		m_pPatientList->RemoveColumn(0);
	}
	CString strCount = "";
	m_nxstaticCountLabel.SetWindowText(strCount);
}

// (r.gonet 12/02/2013) - PLID 59830 - Ensures the buttons are enabled or disabled appropriately.
void CPatientListsDlg::EnsureControls()
{
	NXDATALIST2Lib::IRowSettingsPtr pFilterRow = m_pFilterCombo->CurSel;
	if(!pFilterRow) {
		m_btnNew.EnableWindow(TRUE);
		m_btnEdit.EnableWindow(FALSE);
		m_btnDelete.EnableWindow(FALSE);		
	} else {
		long nFilterID = VarLong(pFilterRow->GetValue(eplfcID));
		if(nFilterID == FILTER_ID_TEMPORARY) {
			m_btnNew.EnableWindow(TRUE);
			m_btnEdit.EnableWindow(TRUE);
			m_btnDelete.EnableWindow(FALSE);
		} else {
			m_btnNew.EnableWindow(TRUE);
			m_btnEdit.EnableWindow(TRUE);
			m_btnDelete.EnableWindow(TRUE);
		}
	}
	
}

// (r.gonet 12/02/2013) - PLID 59830 - Handler for when the user clicks the new filter button
void CPatientListsDlg::OnBnClickedPatientListsNewFilterBtn()
{
	try {
		if (!UserPermission(EditFilter)) {
			return;
		}

		OpenFilterEditor(true);
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 12/02/2013) - PLID 59830 - Handler for when the user clicks the edit filter button
void CPatientListsDlg::OnBnClickedPatientListsEditFilterBtn()
{
	try {
		if (!UserPermission(EditFilter)) {
			return;
		}

		OpenFilterEditor(false);
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 12/02/2013) - PLID 59830 - Handler for when the user clicks the delete filter button
void CPatientListsDlg::OnBnClickedPatientListsDeleteFilterBtn()
{
	try {
		if (!UserPermission(EditFilter)) {
			return;
		}

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pFilterCombo->CurSel;
		if(!pRow) {
			return;
		}
		long nFilterID = VarLong(pRow->GetValue(eplfcID));
		CString strName = VarString(pRow->GetValue(eplfcName));
		if(nFilterID <= 0) {
			// (r.gonet 12/02/2013) - PLID 59830 - This is an error condition because the save button should be disabled if the filter is not modified or if it's read-only
			ASSERT(FALSE);
			MessageBox("The currently selected filter is read-only and cannot be deleted.", "Error", MB_ICONERROR|MB_OK);
			return;
		}
		
		CString strFilter;
		if(CGroups::CommitSubfilterAction(saDelete, fboPerson, nFilterID, strName, strFilter, this)) {
			CString strEmptyFilter;
			FillFilterCombo(FILTER_ID_TEMPORARY, strEmptyFilter);
			ChangeFilterSelection(FILTER_ID_TEMPORARY);
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 12/02/2013) - PLID 59830 - Handler for when the user has chosen a filter row from the filter combo
void CPatientListsDlg::SelChosenPatientListsFilterCombo(LPDISPATCH lpRow)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow) {
			long nFilterID = VarLong(pRow->GetValue(eplfcID));
			if(nFilterID != FILTER_ID_TEMPORARY) {
				m_strOneTimeFilter = "";
			}

			ChangeFilterSelection(nFilterID);
		} else {
			m_strOneTimeFilter = "";
		}
		EnsureControls();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 12/02/2013) - PLID 59830 - Handler for when the filter combo is changing. Don't let the user select nothing.
void CPatientListsDlg::SelChangingPatientListsFilterCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		//Don't let them select nothing
		if(*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}
	} NxCatchAll(__FUNCTION__);
}

// (j.jones 2014-05-20 16:40) - PLID 61814 - added clipboard functionality
void CPatientListsDlg::OnBtnCopyToClipboard()
{
	try {

		//if we have no columns, cancel this process
		//if we have columns but no rows, continue anyways
		if (m_pPatientList->GetColumnCount() == 0) {
			AfxMessageBox("You must first select a filter prior to copying results to the clipboard.");
			return;
		}

		// (j.jones 2014-05-21 08:38) - PLID 61814 - this is a global function in NxUILib\DatalistUtils
		CopyDatalistToClipboard(this, m_pPatientList);

	}
	NxCatchAll(__FUNCTION__);
}
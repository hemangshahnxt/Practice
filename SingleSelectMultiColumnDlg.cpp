// SingleSelectMultiColumnDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "SingleSelectMultiColumnDlg.h"


// CSingleSelectMultiColumnDlg dialog

// (b.savon 2013-02-27 13:31) - PLID 54713 - Created

IMPLEMENT_DYNAMIC(CSingleSelectMultiColumnDlg, CNxDialog)

#define ID_COLUMN 0

CSingleSelectMultiColumnDlg::CSingleSelectMultiColumnDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CSingleSelectMultiColumnDlg::IDD, pParent)
{
	m_strFrom = "";
	m_strWhere = "";
	m_strDescription = "Please select an item.";
	m_strTitle = "Select an Item";
	m_strDisplayColumn = "[1]";
}

CSingleSelectMultiColumnDlg::~CSingleSelectMultiColumnDlg()
{
}

void CSingleSelectMultiColumnDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
}

BOOL CSingleSelectMultiColumnDlg::OnInitDialog()
{
	try{
		CNxDialog::OnInitDialog();

		// Do the buttons and the dialog title/description
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		SetWindowText(m_strTitle);
		GetDlgItem(IDC_SINGLE_MULTI_DISPLAY_STATIC)->SetWindowText(m_strDescription);

		// Bind the datalist
		if( m_nxdlSingleMultiColumn == NULL ){
			m_nxdlSingleMultiColumn = BindNxDataList2Ctrl(IDC_NXDL_SINGLE_MULTI, false);
		}

		LoadDatalist();

	}NxCatchAll(__FUNCTION__);

	return TRUE;
}

// (j.jones 2013-08-01 13:45) - PLID 53317 - added constructor for an optional sort ascending list,
// where a value of true means ascending, false means descending
HRESULT CSingleSelectMultiColumnDlg::Open(const CString &strFrom, const CString &strWhere, const CStringArray &aryColumns, const CStringArray &aryColumnHeaders,
										  const CSimpleArray<short> &arySortOrder, OPTIONAL CSimpleArray<bool> &arySortAscending, const CString &strDisplayColumn, 
										  const CString &strDescription, const CString &strTitle)
{
	m_strFrom = strFrom;
	m_strWhere = strWhere;
	// You must pass in the same number of items in the array
	if( aryColumnHeaders.GetSize() != aryColumns.GetSize() || aryColumnHeaders.GetSize() != arySortOrder.GetSize() ){
		ThrowNxException("Call to SingleSelectMultiColumnDlg: Invalid count in arrays!  The size must be the same for each array parameter.");
	}
	
	// (j.jones 2013-08-01 13:45) - PLID 53317 - permit an empty sort ascending list,
	// assume it is all ascending if empty, but if it is non-empty and not the same size
	// as the sort order list, throw an exception
	if(arySortAscending.GetSize() == 0) {
		//fill as ascending
		for( short idx = ID_COLUMN; idx < arySortOrder.GetSize(); idx++ ){
			arySortAscending.Add(true);
		}
	}
	if( arySortAscending.GetSize() > 0 && arySortAscending.GetSize() != arySortOrder.GetSize() ){
		ThrowNxException("Call to SingleSelectMultiColumnDlg: Invalid count in sort arrays!  The size must be the same for each sort array parameter.");
	}

	for( short idx = ID_COLUMN; idx < aryColumns.GetSize(); idx++ ){
		m_aryColumns.Add(aryColumns.GetAt(idx));
		m_aryColumnHeaders.Add(aryColumnHeaders.GetAt(idx));
		m_larySortOrder.Add(arySortOrder[idx]);
		m_barySortAscending.Add(arySortAscending[idx]);
	}
	m_strDisplayColumn = strDisplayColumn;
	m_strDescription = strDescription;
	m_strTitle = strTitle;
	return DoModal();
}

// (j.jones 2013-08-01 13:45) - PLID 53317 - use this constructor if no columns sort or if all columns
// sort ascending, and none need to sort descending
HRESULT CSingleSelectMultiColumnDlg::Open(const CString &strFrom, const CString &strWhere, const CStringArray &aryColumns, const CStringArray &aryColumnHeaders, 
										  const CSimpleArray<short> &arySortOrder, const CString &strDisplayColumn, 
										  const CString &strDescription, const CString &strTitle)
{
	// (j.jones 2013-08-01 13:45) - PLID 53317 - this constructor is for the more common calls that do not need
	// a sort descending value, so call our main constructor and assume all our sorts (if any)
	// sort ascending
	CSimpleArray<bool> arySortAscending;
	for( short idx = ID_COLUMN; idx < arySortOrder.GetSize(); idx++ ){
		arySortAscending.Add(true);
	}
	return Open(strFrom, strWhere, aryColumns, aryColumnHeaders, arySortOrder, arySortAscending, strDisplayColumn, strDescription, strTitle);
}

void CSingleSelectMultiColumnDlg::LoadDatalist()
{
	// Load if valid.
	if( m_nxdlSingleMultiColumn && m_strFrom != "" ){
		// Set the style for the ID column and the Values
		long nStyleID = NXDATALIST2Lib::csVisible|NXDATALIST2Lib::csFixedWidth;
		long nStyleValues = NXDATALIST2Lib::csVisible|NXDATALIST2Lib::csWidthAuto;

		for( short idx = ID_COLUMN; idx < m_aryColumns.GetSize(); idx++ ){
			//Insert the columns into the datalist
			short nColumnIdx = m_nxdlSingleMultiColumn->InsertColumn(
														idx, 
														_bstr_t(m_aryColumns.GetAt(idx)), 
														_bstr_t(m_aryColumnHeaders.GetAt(idx)), 
														-1, 
														idx == 0 ? nStyleID : nStyleValues);
			//Set the sort order
			NXDATALIST2Lib::IColumnSettingsPtr pCol = m_nxdlSingleMultiColumn->GetColumn(nColumnIdx);
			pCol->PutSortPriority(m_larySortOrder[idx]);
			// (j.jones 2013-08-01 14:04) - PLID 53317 - supported sort ascending, if false
			// then set this to VARIANT_FALSE, which means descending
			if(!m_barySortAscending[idx]){ 
				pCol->PutSortAscending(VARIANT_FALSE);
			}
			//Put the ID column style
			if( idx == ID_COLUMN ){	
				pCol->PutStoredWidth(0);
			}
		}

		m_nxdlSingleMultiColumn->PutDisplayColumn(_bstr_t(m_strDisplayColumn));
		m_nxdlSingleMultiColumn->PutAllowSort(VARIANT_TRUE);

		// Requery
		m_nxdlSingleMultiColumn->PutFromClause(_bstr_t(m_strFrom));
		m_nxdlSingleMultiColumn->PutWhereClause(_bstr_t(m_strWhere));
		m_nxdlSingleMultiColumn->Requery();
	}
}


BEGIN_MESSAGE_MAP(CSingleSelectMultiColumnDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, &CSingleSelectMultiColumnDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CSingleSelectMultiColumnDlg message handlers

void CSingleSelectMultiColumnDlg::OnBnClickedOk()
{
	try{
		CNxDialog::OnOK();
	}NxCatchAll(__FUNCTION__);
}

void CSingleSelectMultiColumnDlg::GetSelectedValues(CVariantArray &varySelectedValues)
{
	for( short idx = ID_COLUMN; idx < m_varySelectedValues.GetSize(); idx++ ){
		varySelectedValues.Add(m_varySelectedValues.GetAt(idx));
	}
}

BEGIN_EVENTSINK_MAP(CSingleSelectMultiColumnDlg, CNxDialog)
ON_EVENT(CSingleSelectMultiColumnDlg, IDC_NXDL_SINGLE_MULTI, 16, CSingleSelectMultiColumnDlg::SelChosenNxdlSingleMulti, VTS_DISPATCH)
END_EVENTSINK_MAP()

void CSingleSelectMultiColumnDlg::SelChosenNxdlSingleMulti(LPDISPATCH lpRow)
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if( pRow ){
			m_varySelectedValues.RemoveAll();
			//Save the values
			for( short idx = ID_COLUMN; idx < m_nxdlSingleMultiColumn->GetColumnCount(); idx++ ){
				m_varySelectedValues.Add(pRow->GetValue(idx));
			}
		}else if( m_varySelectedValues.GetCount() > 0 ){
			m_nxdlSingleMultiColumn->SetSelByColumn(ID_COLUMN, m_varySelectedValues.GetAt(ID_COLUMN));
		}
	}NxCatchAll(__FUNCTION__);
}

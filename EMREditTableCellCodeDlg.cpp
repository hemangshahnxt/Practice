// EditTableCellCodeDlg.cpp : implementation file
//

// (j.gruber 2013-09-30 10:58) - PLID 58816 - created for

#include "stdafx.h"
#include "Practice.h"
#include "EMREditTableCellCodeDlg.h"
#include "EMRUtils.h"
#include "AdministratorRC.h"
#include "UTSSearchDlg.h"

#define COLOR_GRAYED_OUT RGB(240,240,240)
#define COLOR_HAS_CODES RGB(186,252,206)
#define COLOR_NO_CODES RGB(255,255,255)
#define COLOR_SELECTED RGB(255,0,0)


#define ID_REMOVE_CODE        52345

enum CodeListColumn
{
	clcID = 0,
	clcVocab,
	clcCode,
	clcName,
};

using namespace NXDATALIST2Lib;

// CEMREditTableCellCodeDlg dialog

IMPLEMENT_DYNAMIC(CEMREditTableCellCodeDlg, CNxDialog)

CEMREditTableCellCodeDlg::CEMREditTableCellCodeDlg(CEmrInfoDataElementArray &aryRows, CEmrInfoDataElementArray &aryColumns,
											CEmrItemEntryDlg* pParent, EmrInfoSubType eDataSubType,
											CEMRTableCellCodes *pTableCodes)	
											: CNxDialog(CEMREditTableCellCodeDlg::IDD, pParent)
{
	m_pmapCodes = pTableCodes;
	m_aryRows.AppendCopy(aryRows);
	m_aryColumns.AppendCopy(aryColumns);
	m_nEditingRowSortOrder = -1;
	m_nEditingColumnSortOrder = -1;
//m_nCaretIndex = 0;
	m_pEmrItemEntryDlg = pParent;
//	m_pButtonFont = NULL;
	m_bTableRowsAsFields = FALSE;
	// (j.jones 2011-07-08 13:18) - PLID 43032 - added m_DataSubType
	m_DataSubType = eDataSubType;

	m_strCurrentRowID = "";
	m_strCurrentColID = "";
	m_pCurrentRow = NULL;
	m_nCurrentColListIndex = -1;
	m_nCurrentRowDataIndex = -1;
	m_nCurrentColDataIndex = -1;

}

CEMREditTableCellCodeDlg::~CEMREditTableCellCodeDlg()
{
}

void CEMREditTableCellCodeDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);	
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
}


BEGIN_MESSAGE_MAP(CEMREditTableCellCodeDlg, CNxDialog)
/*ON_BN_CLICKED(IDC_ADD_CODE, &CEMREditTableCellCodeDlg::OnBnClickedAddCode)
	ON_BN_CLICKED(IDC_EDIT_CODE, &CEMREditTableCellCodeDlg::OnBnClickedEditCode)
	ON_BN_CLICKED(IDC_REMOVE_CODE, &CEMREditTableCellCodeDlg::OnBnClickedRemoveCode)*/
	ON_BN_CLICKED(IDC_OPEN_UMLS, &CEMREditTableCellCodeDlg::OnBnClickedOpenUmls)
	ON_BN_CLICKED(IDOK, &CEMREditTableCellCodeDlg::OnBnClickedOk)
END_MESSAGE_MAP()



/*CArray<CodeStruct, CodeStruct>* CEMREditTableCellCodeDlg::GetCellCodes(long nRowID, long nColID, BOOL bRemove =FALSE)
{	
	TableCellCodes::iterator it = m_pmapCodes->find(pair<long, long>(nRowID, nColID));
	CArray<CodeStruct, CodeStruct>* pReturn = NULL;

	if (it != m_pmapCodes->end()) 
	{
		pReturn = it->second;

		if (bRemove) {
			m_pmapCodes->erase(it);
		}	
	}

	return pReturn;

}*/

int CEMREditTableCellCodeDlg::GetCellColor(CEmrInfoDataElement *pColumnInfo, CEmrInfoDataElement *pRowInfo) 
{
	if (m_bTableRowsAsFields) {
		if (pRowInfo->m_nListType != LIST_TYPE_TEXT && pRowInfo->m_nListType != LIST_TYPE_CHECKBOX)
		{
			return COLOR_GRAYED_OUT;
		}

		CEMRTableCell cell(pColumnInfo->m_strData, pRowInfo->m_strData);
		//well, we have either a text box or a checkbox, so now we need to know if there is a code already here
		if (m_pmapCodes->GetCodeCount(cell) > 0) 
		{
			return COLOR_HAS_CODES;
		}
		else 
		{
			return COLOR_NO_CODES;
		}	
	}
	else {
		if (pColumnInfo->m_nListType != LIST_TYPE_TEXT && pColumnInfo->m_nListType != LIST_TYPE_CHECKBOX)
		{
			return COLOR_GRAYED_OUT;
		}

		//well, we have either a text box or a checkbox, so now we need to know if there is a code already here
		CEMRTableCell cell(pRowInfo->m_strData, pColumnInfo->m_strData);
		if (m_pmapCodes->GetCodeCount(cell) > 0) 
		{
			return COLOR_HAS_CODES;
		}
		else 
		{
			return COLOR_NO_CODES;
		}	
	}

}



// CEMREditTableCellCodeDlg message handlers
BOOL CEMREditTableCellCodeDlg::OnInitDialog() 
{
	try
	{
		CNxDialog::OnInitDialog();
		
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		// (z.manning 2010-11-29 14:38) - PLID 39025 - We need to make sure these arrays are in the same order
		// in this array. We already have them in order of their sort orders, but sort order doesn't matter if
		// the auto-alphabatize option is enabled.
		qsort(m_aryRows.GetData(), m_aryRows.GetSize(), sizeof(CEmrInfoDataElement*), CompareInfoDataElementsByVisibleIndex);
		qsort(m_aryColumns.GetData(), m_aryColumns.GetSize(), sizeof(CEmrInfoDataElement*), CompareInfoDataElementsByVisibleIndex);

		//initialize our code table first
		m_pCodeList = BindNxDataList2Ctrl(IDC_CODE_LIST, GetRemoteData(), false);

		m_pSelectedList = BindNxDataList2Ctrl(IDC_CODE_SELECTED_LIST, GetRemoteData(), true);

		m_pdlTable = BindNxDataList2Ctrl(IDC_TABLE, GetRemoteData(), false);
		m_pdlTable->PutGridVisible(VARIANT_TRUE);
		m_pdlTable->PutHighlightVisible(VARIANT_FALSE);

		//Insert first column for row titles
		m_pdlTable->InsertColumn(0, _T(""), _T(""), 0, csVisible);
		
		// (c.haag 2008-10-23 12:42) - PLID 31834 - Table flipping support
		int nTableRows = (m_bTableRowsAsFields) ? m_aryColumns.GetSize() : m_aryRows.GetSize();
		int nTableColumns = (m_bTableRowsAsFields) ? m_aryRows.GetSize() : m_aryColumns.GetSize();

		short nColIndex;
		for(nColIndex = 0; nColIndex < nTableColumns; nColIndex++)
		{
			// Support for table flipping
			CEmrInfoDataElement *pColumnInfo = (m_bTableRowsAsFields) ? m_aryRows.GetAt(nColIndex) : m_aryColumns.GetAt(nColIndex);
			if(!pColumnInfo->m_bInactive) {				
				IColumnSettingsPtr pCol = m_pdlTable->GetColumn(m_pdlTable->InsertColumn(nColIndex + 1, _T(""), _bstr_t(pColumnInfo->m_strData), 0, csVisible));
				long nIdealColumnWidth = pCol->CalcWidthFromData(VARIANT_TRUE, VARIANT_FALSE);				
				//prevent really small columns
				if (nIdealColumnWidth < 70)
				{
					nIdealColumnWidth = 70;
				}
				pCol->PutStoredWidth(nIdealColumnWidth);			
				pCol->FieldType = cftTextSingleLine;
				pCol->DataType = VT_BSTR;
			}
			else {
				// Get rid of any inactive columns for this dialog
				// Support for table flipping
				if (m_bTableRowsAsFields) {
					delete m_aryRows.GetAt(nColIndex);
					m_aryRows.RemoveAt(nColIndex);
				} else {
					delete m_aryColumns.GetAt(nColIndex);
					m_aryColumns.RemoveAt(nColIndex);
				}
				nColIndex--;
				nTableColumns--;
			}
		}

		// (z.manning, 05/22/2008) - Add the row AFTER adding the columns because the datalist RowSettingsPtr
		// doesn't seem to handle dynamic column adding/removing well.
		/*IRowSettingsPtr pTitleRow = m_pdlTable->GetNewRow();
		m_pdlTable->AddRowAtEnd(pTitleRow, NULL);
		_variant_t varNull;
		varNull.vt = VT_NULL;
		pTitleRow->PutValue(0, varNull);

		// (c.haag 2008-10-23 12:42) - PLID 31834 - Table flipping support
		for(nColIndex = 0; nColIndex < nTableColumns; nColIndex++)
		{
			// (c.haag 2008-10-23 12:39) - PLID 31834 - Consider table flipping
			CEmrInfoDataElement *pColumnData = (m_bTableRowsAsFields) ? m_aryRows.GetAt(nColIndex) : m_aryColumns.GetAt(nColIndex);
			pTitleRow->PutValue(nColIndex + 1, _bstr_t(pColumnData->m_strData));
			pTitleRow->PutCellLinkStyle(nColIndex + 1, dlLinkStyleTrue);
			pTitleRow->PutCellForeColor(nColIndex + 1, RGB(0,0,200));
			// (z.manning, 05/22/2008) - Auto-resize each active column to the size of the column title.
			IColumnSettingsPtr pCol = m_pdlTable->GetColumn(nColIndex + 1);
			long nIdealColumnWidth = pCol->CalcWidthFromData(VARIANT_TRUE, VARIANT_FALSE);
			pCol->PutStoredWidth(nIdealColumnWidth);			
		}*/

		long nIdealTableHeight = 0;//pTitleRow->GetHeight();

		for(int nRowIndex = 0; nRowIndex < nTableRows; nRowIndex++)
		{
			// Consider table flipping
			CEmrInfoDataElement *pRowData = (m_bTableRowsAsFields) ? m_aryColumns.GetAt(nRowIndex) : m_aryRows.GetAt(nRowIndex);
			if(pRowData->m_bInactive) {
				if (m_bTableRowsAsFields) {
					delete m_aryColumns.GetAt(nRowIndex);
					m_aryColumns.RemoveAt(nRowIndex);
				} else {
					delete m_aryRows.GetAt(nRowIndex);
					m_aryRows.RemoveAt(nRowIndex);
				}
				nRowIndex--;
				nTableRows--;
				continue;
			}
			IRowSettingsPtr pRow = m_pdlTable->GetNewRow();
			pRow->PutValue(0, _bstr_t(pRowData->m_strData));
			//pRow->PutCellLinkStyle(0, dlLinkStyleTrue);
			//pRow->PutCellForeColor(0, RGB(0,0,200));
			pRow->PutCellBackColor(0, COLOR_GRAYED_OUT);

			m_pdlTable->AddRowAtEnd(pRow, NULL);
			nIdealTableHeight += pRow->GetHeight();
		}
		
		// (z.manning 2008-06-06 15:50) - Handle the row title column
		IColumnSettingsPtr pFirstColumn = m_pdlTable->GetColumn(0);
		long nColWidth = pFirstColumn->CalcWidthFromData(VARIANT_TRUE, VARIANT_FALSE);
		pFirstColumn->PutStoredWidth(nColWidth);


		//color the cells accordingly
		for(nColIndex = 0; nColIndex < nTableColumns; nColIndex++)
		{
			// Support for table flipping
			CEmrInfoDataElement *pColumnInfo = (m_bTableRowsAsFields) ? m_aryRows.GetAt(nColIndex) : m_aryColumns.GetAt(nColIndex);

			//get the first row in the list
			IRowSettingsPtr pColorRow = m_pdlTable->GetFirstRow();
			for(int nRowIndex = 0; nRowIndex < nTableRows; nRowIndex++)
			{
				ASSERT(pColorRow);
				// Consider table flipping
				CEmrInfoDataElement *pRowData = (m_bTableRowsAsFields) ? m_aryColumns.GetAt(nRowIndex) : m_aryRows.GetAt(nRowIndex);				

				pColorRow->PutCellBackColor(nColIndex+1, GetCellColor(pColumnInfo, pRowData));
				pColorRow = pColorRow->GetNextRow();
			}
		}

		// (z.manning, 05/22/2008) - Auto-resize the dialog's height based on the ideal table height
		/*{
			CRect rcDialog, rcDatalist;
			GetWindowRect(rcDialog);
			ScreenToClient(rcDialog);
			CWnd *pwndDatalist = GetDlgItem(IDC_TABLE);
			pwndDatalist->GetWindowRect(rcDatalist);
			ScreenToClient(rcDatalist);
			if(nIdealTableHeight < rcDatalist.Height())
			{
				int nVerticalShrink = rcDatalist.Height() - nIdealTableHeight;
				pwndDatalist->MoveWindow(rcDatalist.left, rcDatalist.top, rcDatalist.Width(), rcDatalist.Height() - nVerticalShrink);
				MoveWindow(rcDialog.left, rcDialog.top, rcDialog.Width(), rcDialog.Height() - nVerticalShrink);
				CenterWindow();
			}
		}*/

		CEmrInfoDataElement *peideCurrent = GetCurrentData();
		if(peideCurrent != NULL /*&& !CanHaveFormula(peideCurrent, TRUE)*/) {
			m_nEditingRowSortOrder = m_nEditingColumnSortOrder = -1;
		}		

		//disable our controsl
		EnableControls(FALSE);
	
	}NxCatchAll(__FUNCTION__);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

CEmrInfoDataElement* CEMREditTableCellCodeDlg::GetCurrentData()
{
	if(m_nEditingColumnSortOrder >= 0) {
		for(int nCol = 0; nCol < m_aryColumns.GetSize(); nCol++) {
			CEmrInfoDataElement *pColData = m_aryColumns.GetAt(nCol);
			if(pColData->m_nSortOrder == m_nEditingColumnSortOrder) {
				return pColData;
			}
		}
	}
	else if(m_nEditingRowSortOrder >= 0) {
		for(int nRow = 0; nRow < m_aryRows.GetSize(); nRow++) {
			CEmrInfoDataElement *pRowData = m_aryRows.GetAt(nRow);
			if(pRowData->m_nSortOrder == m_nEditingRowSortOrder) {
				return pRowData;
			}
		}
	}

	return NULL;
}
/*
void CEMREditTableCellCodeDlg::OnBnClickedAddCode()
{
	try
	{
		IRowSettingsPtr pRow = m_pCodeList->GetNewRow();		
		if (pRow)
		{
			pRow->PutValue(clcID, -1);
			pRow->PutValue(clcInactive, g_cvarFalse);

			m_pCodeList->AddRowAtEnd(pRow, NULL);
			m_pCodeList->StartEditing(pRow, clcCode);
		}
		
	}NxCatchAll(__FUNCTION__);
}

void CEMREditTableCellCodeDlg::OnBnClickedEditCode()
{
	try {

		//get the currently selected row
		IRowSettingsPtr pRow = m_pCodeList->CurSel;
		if (pRow) {
			m_pCodeList->StartEditing(pRow, clcCode);
		}
	}NxCatchAll(__FUNCTION__);
}

void CEMREditTableCellCodeDlg::OnBnClickedRemoveCode()
{
	try {

		IRowSettingsPtr pRow = m_pCodeList->CurSel;
		if (pRow) {
			if (IDYES == MsgBox(MB_YESNO, "Deleting this code will make it no longer exportable in the CCDA and possibly other places.  This may cause these exports not to function correctly.  Are you sure you wish to continue?"))
			{
				//remove it
				m_pdlTable->RemoveRow(pRow);
			}
		}

	}NxCatchAll(__FUNCTION__);
}*/

void CEMREditTableCellCodeDlg::OnBnClickedOpenUmls()
{
	try{
		CUTSSearchDlg dlg;
		int nResult = dlg.DoModal();

		if (nResult == IDOK)
		{
			//requery our code list
			m_pSelectedList->Requery();
		}
	}NxCatchAll(__FUNCTION__);
}
BEGIN_EVENTSINK_MAP(CEMREditTableCellCodeDlg, CNxDialog)
	ON_EVENT(CEMREditTableCellCodeDlg, IDC_TABLE, 19, CEMREditTableCellCodeDlg::LeftClickTable, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEMREditTableCellCodeDlg, IDC_CODE_LIST, 2, CEMREditTableCellCodeDlg::SelChangedCodeList, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CEMREditTableCellCodeDlg, IDC_TABLE, 4, CEMREditTableCellCodeDlg::LButtonDownTable, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEMREditTableCellCodeDlg, IDC_CODE_SELECTED_LIST, 16, CEMREditTableCellCodeDlg::SelChosenCodeSelectList, VTS_DISPATCH)
	ON_EVENT(CEMREditTableCellCodeDlg, IDC_CODE_LIST, 7, CEMREditTableCellCodeDlg::RButtonUpCodeList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
END_EVENTSINK_MAP()

void CEMREditTableCellCodeDlg::LoadCodeList(long nDataRowIndex, long nDataColIndex)
{
	//first clear the list
	m_pCodeList->Clear();

	//load our rows and columns	
	CEmrInfoDataElement *pColumnInfo = (m_bTableRowsAsFields) ? m_aryRows.GetAt(nDataColIndex) : m_aryColumns.GetAt(nDataColIndex);
	CEmrInfoDataElement *pRowData = (m_bTableRowsAsFields) ? m_aryColumns.GetAt(nDataRowIndex) : m_aryRows.GetAt(nDataRowIndex);

	CString strRowID, strColID;	
	if (m_bTableRowsAsFields) 
	{
		strColID = pRowData->m_strData;
		strRowID = pColumnInfo->m_strData;
	}
	else {
		strRowID = pRowData->m_strData;
		strColID = pColumnInfo->m_strData;
	}
	
	CEMRTableCell cell(strRowID, strColID);
	CEMRCodeArray *pAry = m_pmapCodes->GetCodes(cell, FALSE);

	if (pAry) {

		for(int i=0; i < pAry->GetCount(); i++) 
		{
			CEMRCode code = pAry->GetAt(i);

			IRowSettingsPtr pRow = m_pCodeList->GetNewRow();
			if (pRow) {
				pRow->PutValue(clcID, code.GetID());
				pRow->PutValue(clcVocab, _variant_t(code.GetVocab()));
				pRow->PutValue(clcCode, _variant_t(code.GetCode()));
				pRow->PutValue(clcName, _variant_t(code.GetName()));
				m_pCodeList->AddRowAtEnd(pRow, NULL);
			}
		}
	}

	//set our current ID variables
	m_strCurrentRowID = pRowData->m_strData;
	m_strCurrentColID = pColumnInfo->m_strData;

	//open up our buttons
	/*GetDlgItem(IDC_ADD_CODE)->EnableWindow(TRUE);*/
	EnableControls(TRUE);

	//edit and delete shouldn't be enabled
	/*GetDlgItem(IDC_EDIT_CODE)->EnableWindow(FALSE);
	GetDlgItem(IDC_REMOVE_CODE)->EnableWindow(FALSE);	*/

}


void CEMREditTableCellCodeDlg::SaveCodeList(CEMRTableCell cell)
{
	//do we already have some codes saved for this pair?
	CEMRCodeArray *pAry = m_pmapCodes->GetCodes(cell, TRUE);

	if (pAry) {

		//clear the list
		pAry->RemoveAll();
	}
	else {
		pAry = new CEMRCodeArray();
	}


	//now we'll add to it
	IRowSettingsPtr pRow = m_pCodeList->GetFirstRow();
	while (pRow)
	{
		long nID = VarLong(pRow->GetValue(clcID));
		CString strVocab = VarString(pRow->GetValue(clcVocab));
		CString strCode = VarString(pRow->GetValue(clcCode));
		CString strName = VarString(pRow->GetValue(clcName));

		CEMRCode code(nID, strVocab, strCode, strName);		

		pAry->Add(code);

		pRow = pRow->GetNextRow();
	}
	
	//add the array back to the map
	//only if there is something in it
	if (pAry->GetCount() > 0) {
		m_pmapCodes->insert(cell, pAry);
	}
}

void CEMREditTableCellCodeDlg::ColorCell(IRowSettingsPtr pRow, long nListColIndex, int color)
{
	
	if (pRow) {		
		//get the column
		pRow->PutCellBackColor((short)nListColIndex, color);
	}

}

BOOL CEMREditTableCellCodeDlg::CanClick(long nDataRowIndex, long nDataColIndex)
{
	if (nDataRowIndex < 0 || nDataColIndex < 0) {
		return FALSE;
	}

	//do these need to be switched?
	CEmrInfoDataElement *pColumnInfo = (m_bTableRowsAsFields) ? m_aryRows.GetAt(nDataColIndex) : m_aryColumns.GetAt(nDataColIndex);
	CEmrInfoDataElement *pRowData = (m_bTableRowsAsFields) ? m_aryColumns.GetAt(nDataRowIndex) : m_aryRows.GetAt(nDataRowIndex);

	int Color = GetCellColor(pColumnInfo, pRowData);

	if (Color == COLOR_GRAYED_OUT) {
		return FALSE;
	}
	else {
		return TRUE;
	}
}


//responsible for saving the last set of data and changing the colors
void CEMREditTableCellCodeDlg::NavigateCells()
{

	if (!m_strCurrentRowID.IsEmpty()) {
		
		//do these need to be switched?
		CEmrInfoDataElement *pColumnInfo = (m_bTableRowsAsFields) ? m_aryRows.GetAt(m_nCurrentColDataIndex) : m_aryColumns.GetAt(m_nCurrentColDataIndex);
		CEmrInfoDataElement *pRowData = (m_bTableRowsAsFields) ? m_aryColumns.GetAt(m_nCurrentRowDataIndex) : m_aryRows.GetAt(m_nCurrentRowDataIndex);

		//first, we need to save our previous values
		CString strRowID; 
		CString strColID;
		if (m_bTableRowsAsFields) 
		{
			strColID = pRowData->m_strData;
			strRowID = pColumnInfo->m_strData;
		}
		else {
			strRowID = pRowData->m_strData;
			strColID = pColumnInfo->m_strData;
		}

		CEMRTableCell cell(strRowID, strColID);
		SaveCodeList(cell);

		if (m_pCurrentRow) {
			m_pCurrentRow->PutCellBackColor((short)m_nCurrentColListIndex, GetCellColor(pColumnInfo, pRowData));
		}	
		
	}
		
}

void CEMREditTableCellCodeDlg::LeftClickTable(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	
}

void CEMREditTableCellCodeDlg::SelChangedCodeList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try
	{
		IRowSettingsPtr pRow(lpNewSel);
		if (pRow) 
		{
			//enable the edit and the delete
			/*GetDlgItem(IDC_EDIT_CODE)->EnableWindow(TRUE);
			GetDlgItem(IDC_REMOVE_CODE)->EnableWindow(TRUE);*/
		}
		else {
			/*GetDlgItem(IDC_EDIT_CODE)->EnableWindow(FALSE);
			GetDlgItem(IDC_REMOVE_CODE)->EnableWindow(FALSE);*/
		}

	}NxCatchAll(__FUNCTION__);
}

void CEMREditTableCellCodeDlg::LButtonDownTable(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try
	{

		//this takes care of saving our previous selection and coloring		
		NavigateCells();

		IRowSettingsPtr pRow(lpRow);
		long nRowIndex = -1;
		long nColDataIndex = -1;
		if (pRow) {

			/*if (m_bTableRowsAsFields) {				
				nColDataIndex= pRow->CalcRowNumber();	
				
				nRowIndex = nCol - 1;	
			}
			else 
			{*/
				//now we have to get our row/column IDs
				nRowIndex = pRow->CalcRowNumber();	
				//we need the column idex, with is out cell column minus the title row column
				nColDataIndex = nCol - 1;	
			/*}*/

			
				
			//did they click on a code cell?
			if (CanClick(nRowIndex, nColDataIndex)) {
	
				//enable our buttons
/*				GetDlgItem(IDC_ADD_CODE)->EnableWindow(FALSE);*/
				EnableControls(TRUE);

				//color our cell that we are editing				
				pRow->PutCellBackColor(nCol, COLOR_SELECTED);				

				//now load our list with our data indexes
				LoadCodeList(nRowIndex, nColDataIndex);		

				//set our current variables
				m_nCurrentRowDataIndex = nRowIndex;
				m_nCurrentColDataIndex = nColDataIndex;

				m_pCurrentRow = pRow;
				m_nCurrentColListIndex = nCol;
			}
			else {
/*				GetDlgItem(IDC_ADD_CODE)->EnableWindow(FALSE);*/
				EnableControls(FALSE);

				//set our current variables
				m_nCurrentRowDataIndex = -1;
				m_nCurrentColDataIndex = -1;

				m_pCurrentRow = NULL;
				m_nCurrentColListIndex = -1;

				m_strCurrentColID = "";
				m_strCurrentRowID = "";
			}			
			
		}
		else {

/*			GetDlgItem(IDC_ADD_CODE)->EnableWindow(FALSE);*/
			EnableControls(FALSE);

			//set our current variables
			m_nCurrentRowDataIndex = -1;
			m_nCurrentColDataIndex = -1;

			m_pCurrentRow = NULL;
			m_nCurrentColListIndex = -1;

			m_strCurrentColID = "";
			m_strCurrentRowID = "";
		}
		
		
		
	}NxCatchAll(__FUNCTION__);
}

void CEMREditTableCellCodeDlg::SelChosenCodeSelectList(LPDISPATCH lpRow)
{
	try {
		IRowSettingsPtr pRow(lpRow);

		if (pRow)
		{
			//make sure the code isn't already in our list
			IRowSettingsPtr pRowFound = m_pCodeList->FindByColumn(clcID, pRow->GetValue(clcID), NULL, g_cvarFalse);
			if (pRowFound)
			{
				MsgBox("This code is already selected");
				return;
			}

			IRowSettingsPtr pNewCodeRow = m_pCodeList->GetNewRow();
			if (pNewCodeRow) {
				pNewCodeRow->PutValue(clcID, pRow->GetValue(clcID));
				pNewCodeRow->PutValue(clcCode, pRow->GetValue(clcCode));
				pNewCodeRow->PutValue(clcVocab, pRow->GetValue(clcVocab));
				pNewCodeRow->PutValue(clcName, pRow->GetValue(clcName));
			
				m_pCodeList->AddRowSorted(pNewCodeRow, NULL);
			}

		}

		//now clear the selection
		m_pSelectedList->CurSel = NULL;
	}NxCatchAll(__FUNCTION__);
}

void CEMREditTableCellCodeDlg::RButtonUpCodeList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		CMenu Popup;
		Popup.m_hMenu = CreatePopupMenu();		
		IRowSettingsPtr pRow(lpRow);
		if(pRow) {		
			m_pCodeList->CurSel = pRow;
			Popup.InsertMenu(3, MF_BYPOSITION, ID_REMOVE_CODE, "Delete Code");
		}		

		CPoint pt;
		pt.x = x;
		pt.y = y;
		CWnd* pwnd = GetDlgItem(IDC_CODE_LIST);
		if (pwnd != NULL) {
			pwnd->ClientToScreen(&pt);
			Popup.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this, NULL);
		}
	}NxCatchAll(__FUNCTION__);
}

void CEMREditTableCellCodeDlg::EnableControls(BOOL bEnable) 
{
	GetDlgItem(IDC_OPEN_UMLS)->EnableWindow(bEnable);
	m_pCodeList->PutReadOnly(!bEnable);
	m_pCodeList->PutEnabled(bEnable);
	m_pSelectedList->PutReadOnly(!bEnable);
	m_pSelectedList->PutEnabled(bEnable);
}

BOOL CEMREditTableCellCodeDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	switch (wParam) {
		case ID_REMOVE_CODE:			
			try{
				IRowSettingsPtr pRow = m_pCodeList->CurSel;
				if (pRow) {
					if (IDYES == MsgBox(MB_YESNO, "Deleting this code will make it no longer exportable in the CCDA and possibly other places.  This may cause these exports not to function correctly.  Are you sure you wish to continue?"))
					{
						//remove it
						m_pCodeList->RemoveRow(pRow);
					}
				}
			}NxCatchAll(" Error in CEMREditTableCellCodeDlg::OnCommand - Cannot Remove Code");
		break;

		return TRUE;
	}

	return CNxDialog::OnCommand(wParam, lParam);
}


void CEMREditTableCellCodeDlg::OnBnClickedOk()
{
	try {
		
		//save our last change
		NavigateCells();

		CNxDialog::OnOK();
	}NxCatchAll(__FUNCTION__);
}

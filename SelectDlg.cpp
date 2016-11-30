// SelectDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "practicerc.h"
#include "SelectDlg.h"
#include "EditComboBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
using namespace ADODB;

/////////////////////////////////////////////////////////////////////////////
// CSelectDlg dialog

// (a.walling 2013-02-13 11:41) - PLID 55148 - Allow no selection
CSelectDlg::CSelectDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CSelectDlg::IDD, pParent)
	, m_bAllowNoSelection(false)
{
	//{{AFX_DATA_INIT(CSelectDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_varPreSelectedID = _variant_t();
	m_nPreSelectColumn = -1;
	// (c.haag 2009-08-24 16:10) - PLID 29310
	m_bAllowAddRecord = FALSE;
	m_nRecordIDColumn = -1;
	m_nRecordNameColumn = -1;
	m_bWereRecordsAdded = FALSE;
}


void CSelectDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSelectDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_SELECT_CAPTION, m_nxstaticSelectCaption);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_BTN_SELECTDLGCFG, m_btnAdd);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSelectDlg, CNxDialog)
	//{{AFX_MSG_MAP(CSelectDlg)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BTN_SELECTDLGCFG, &CSelectDlg::OnBnClickedBtnAdd)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSelectDlg message handlers

BOOL CSelectDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-04-25 14:53) - PLID 29793 - NxIconify buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnAdd.AutoSet(NXB_NEW);
		
		SetWindowText(m_strTitle);

		//TES 12/3/2008 - PLID 32309 - Calculate the size of the caption.
		CRect rCaption;
		GetDlgItem(IDC_SELECT_CAPTION)->GetWindowRect(&rCaption);
		ScreenToClient(&rCaption);
		//TES 12/3/2008 - PLID 32309 - Track the original rectangle DrawTextOnDialog will modify the rect a bit
		// more than we want it to.
		CRect rOrigCaption = rCaption;
		
		//TES 12/3/2008 - PLID 32309 - Now have DrawTextOnDialog calculate how much space this text needs (the text
		// is centered, just as it always has been for this dialog).
		CDC *pDC = GetDC();
		DrawTextOnDialog(this, pDC, rCaption, m_strCaption, dtsText, true, DT_CENTER, false, true);
		ReleaseDC(pDC);

		//TES 12/3/2008 - PLID 32309 - Put in a 20-pixel buffer between the text and the list.
		int nCaptionBottom = rCaption.bottom + 20;
		
		//TES 12/3/2008 - PLID 32309 - Now, make sure we still have at least 50 pixels for the list.
		CRect rList;
		GetDlgItem(IDC_LIST)->GetWindowRect(&rList);
		ScreenToClient(&rList);
		if(nCaptionBottom > rList.bottom - 50) nCaptionBottom = rList.bottom - 50;

		//TES 12/3/2008 - PLID 32309 - Now we've got the height, so move our caption, keeping the original width.
		rCaption.SetRect(rOrigCaption.left, rCaption.top, rOrigCaption.right, nCaptionBottom);
		GetDlgItem(IDC_SELECT_CAPTION)->MoveWindow(&rCaption);
		SetDlgItemText(IDC_SELECT_CAPTION, m_strCaption);

		//TES 12/3/2008 - PLID 32309 - Finally, shrink the list to just below the caption (we've already factored the
		// 20-pixel buffer in).
		rList.SetRect(rList.left, nCaptionBottom, rList.right, rList.bottom);
		GetDlgItem(IDC_LIST)->MoveWindow(&rList);

		m_pList = BindNxDataListCtrl(this, IDC_LIST, GetRemoteData(), false);

		m_pList->FromClause = _bstr_t(m_strFromClause);
		m_pList->WhereClause = _bstr_t(m_strWhereClause);
		m_pList->GroupByClause = _bstr_t(m_strGroupByClause);
		
		for(int i = 0; i < m_arColumns.GetSize(); i++) {
			int nActualIndex = m_pList->InsertColumn(i, m_arColumns.GetAt(i).strField, m_arColumns.GetAt(i).strTitle, m_arColumns.GetAt(i).nWidth, m_arColumns.GetAt(i).nStyle);
			m_pList->GetColumn(nActualIndex)->PutSortPriority(m_arColumns.GetAt(i).nSortPriority);
			m_pList->GetColumn(nActualIndex)->PutSortAscending(m_arColumns.GetAt(i).bSortAsc);

			//m.hancock - 2/23/2006 - PLID 19428 - Set the FieldType for text wrapping
			if(m_arColumns.GetAt(i).bWordWrap) {
				m_pList->GetColumn(nActualIndex)->FieldType = NXDATALISTLib::cftTextWordWrap;
			// (r.gonet 03/05/2014) - PLID 61187 - Support columns which dictate the color of the row background.
			} else if(m_arColumns.GetAt(i).bRowBackColor) {
				m_pList->GetColumn(nActualIndex)->FieldType = NXDATALISTLib::cftSetRowBackColor;
			}
		}

		// (c.haag 2009-08-24 16:10) - PLID 29310 - Support for adding records on the fly
		if (m_bAllowAddRecord) {
			GetDlgItem(IDC_BTN_SELECTDLGCFG)->ShowWindow(SW_SHOW);
		}

		m_pList->Requery();

		if (m_bAllowNoSelection && (m_nPreSelectColumn == -1 || m_varPreSelectedID.vt <= VT_NULL) ) {
			GetDlgItem(IDOK)->EnableWindow(TRUE);
		}

		// (r.galicki 2008-09-10 16:52) - PLID 31280 - select item if preselect has been defined
		// (b.eyers 2015-04-20) - PLID 65653 - Changed the TrySetSel to SetSelByColumn and Ok button needs to enable if something was preselected
		if(m_nPreSelectColumn!=-1) {
			int nSelect = m_pList->SetSelByColumn(m_nPreSelectColumn, m_varPreSelectedID);
			OnSelChangedList(nSelect);
		}
	}
	NxCatchAll("Error in CSelectDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSelectDlg::OnOK() 
{
	// (a.walling 2013-02-13 11:41) - PLID 55148 - Allow no selection
	if (m_pList->CurSel != -1) {
		for(int i = 0; i < m_arColumns.GetSize(); i++) {
			m_arSelectedValues.Add(m_pList->GetValue(m_pList->CurSel, i));
		}
	} else if (m_bAllowNoSelection) {
		for(int i = 0; i < m_arColumns.GetSize(); i++) {
			m_arSelectedValues.Add(g_cvarNull);
		}
	} else {
		AfxMessageBox("Please select an item");
		return;
	}

	CNxDialog::OnOK();
}

BEGIN_EVENTSINK_MAP(CSelectDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CSelectDlg)
	ON_EVENT(CSelectDlg, IDC_LIST, 2 /* SelChanged */, OnSelChangedList, VTS_I4)
	ON_EVENT(CSelectDlg, IDC_LIST, 3 /* DblClickCell */, OnDblClickCellList, VTS_I4 VTS_I2)
	ON_EVENT(CSelectDlg, IDC_LIST, 20 /* TrySetSelFinished */, OnTrySetSelFinishedList, VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CSelectDlg::OnSelChangedList(long nNewSel) 
{
	// (a.walling 2013-02-13 11:41) - PLID 55148 - Allow no selection
	if(!m_bAllowNoSelection && nNewSel == -1) {
		GetDlgItem(IDOK)->EnableWindow(FALSE);
	}
	else {
		GetDlgItem(IDOK)->EnableWindow(TRUE);
	}
	
}

void CSelectDlg::OnDblClickCellList(long nRowIndex, short nColIndex) 
{
	OnOK();
}

// (a.walling 2009-04-14 17:58) - PLID 33951 - Data width style
// (j.jones 2009-08-28 11:54) - PLID 29185 - added data width size and optional sort priorities
void CSelectDlg::AddColumn(CString strField, CString strTitle, BOOL bVisible, BOOL bWordWrap, BOOL bDataWidth /*= FALSE*/,
		long nDataWidthSize /*= -1*/, long nSortPriority /*= -1*/, BOOL bSortAscending /*= TRUE*/)
{
	DatalistColumn dc;
	dc.strField = _bstr_t(strField);
	dc.strTitle = _bstr_t(strTitle);
	if(bVisible) {
		// (a.walling 2009-04-14 17:58) - PLID 33951 - Data width style
		dc.nStyle = NXDATALISTLib::csVisible|(bDataWidth ? NXDATALISTLib::csWidthData : NXDATALISTLib::csWidthAuto);
		dc.bSortAsc = bSortAscending;
		
		// (j.jones 2009-08-28 11:56) - PLID 29185 - if we pass in nDataWidthSize, use that size as the min width
		if(bDataWidth && nDataWidthSize > 0) {
			dc.nWidth = nDataWidthSize;
		}

		// (j.jones 2009-08-28 11:56) - PLID 29185 - if we pass in sort priority,
		// increment any sort priorities we already have, as appropriate
		if(nSortPriority != -1) {
			for(int i = 0; i < m_arColumns.GetSize(); i++) {
				if(m_arColumns[i].nSortPriority >= nSortPriority) {
					//increment the sort priority
					m_arColumns[i].nSortPriority++;
				}
			}

			dc.nSortPriority = nSortPriority;
		}
		
		if(nSortPriority == -1) {
			int nHighestPriority = -1;
			for(int i = 0; i < m_arColumns.GetSize(); i++) {
				if(m_arColumns[i].nSortPriority > nHighestPriority) {
					nHighestPriority = m_arColumns[i].nSortPriority;
				}
			}
			dc.nSortPriority = nHighestPriority+1;			
		}
	}
	else {
		dc.nStyle = NXDATALISTLib::csVisible|NXDATALISTLib::csFixedWidth;
	}
	//m.hancock - 2/23/2006 - PLID 19428 - Set the flag for text wrapping
	if(bWordWrap) {
		dc.bWordWrap = TRUE;
	}
	m_arColumns.Add(dc);
}

// (r.gonet 03/05/2014) - PLID 61187 - Add a column which dictates the color of the row background.
void CSelectDlg::AddRowBackColorColumn(CString strField)
{
	DatalistColumn dc;
	dc.strField = _bstr_t(strField);
	dc.strTitle = _bstr_t(strField);
	dc.nStyle = NXDATALISTLib::csVisible|NXDATALISTLib::csFixedWidth;
	dc.bRowBackColor = TRUE;
	m_arColumns.Add(dc);
}

// (r.galicki 2008-09-10 16:50) - PLID 31280 - Allows for preselecting a list item
void CSelectDlg::SetPreSelectedID(short nColumnID, _variant_t varID) {
	m_varPreSelectedID = varID;
	m_nPreSelectColumn = nColumnID;
}


// (r.galicki 2008-09-11 10:18) - PLID 31280 - Handle TrySetSelFinished, enable OK button if patient was found/selection made
void CSelectDlg::OnTrySetSelFinishedList(long nRowEnum, long nFlags) 
{
	//Selection made, enable OK button

	if(nFlags == NXDATALISTLib::dlTrySetSelFinishedSuccess ) {
		NXDATALISTLib::IRowSettingsPtr pRow = m_pList->GetRowByEnum(nRowEnum);

		if (pRow) {
			m_pList->WaitForRequery(NXDATALISTLib::dlPatienceLevelWaitIndefinitely);
			m_pList->EnsureRowVisible(pRow->GetIndex());
		}

		OnSelChangedList(m_pList->CurSel);
	} else {
		OnSelChangedList(m_pList->CurSel);
	}
}

void CSelectDlg::OnBnClickedBtnAdd()
{
	try {
		// (c.haag 2009-08-24 15:50) - PLID 29310 - Allows a user to add a new record to the list
		if (m_bAllowAddRecord) {
			BOOL bRetry = TRUE;
			while (bRetry) {
				CString strResult;
				if (IDCANCEL == InputBox(this, FormatString("Please enter a name for the new %s", m_strRecordType), strResult, "")) {
					bRetry = FALSE;
				}
				else if (strResult.IsEmpty()) {
					MessageBox("Please enter a non-empty name.", NULL, MB_ICONSTOP | MB_OK);
				}
				else {
					// The user entered a name. See if it already exists in the table
					_RecordsetPtr prs = CreateParamRecordset(FormatString("SELECT {STRING} FROM %s WHERE %s = {STRING}", m_strRecordTable, m_strRecordField), strResult, strResult);
					if (!prs->eof) {
						// Already exists. Fail the addition.
						MessageBox("A record with this name already exists (it may be inactive).\n\nPlease try another name.", NULL, MB_ICONSTOP | MB_OK);
					} else {
						// Does not exist. Add the record
						prs->Close();
						_RecordsetPtr prs = CreateParamRecordset(m_strParamSqlRecordAdd, strResult);
						long nID = AdoFldLong(prs, "NewID");
						// Now add the row to the list
						IRowSettingsPtr pRow = m_pList->GetRow(-1);
						pRow->Value[m_nRecordIDColumn] = nID;
						pRow->Value[m_nRecordNameColumn] = _bstr_t(strResult);
						m_pList->AddRow(pRow);
						m_pList->Sort();
						// Auto-select the row
						OnSelChangedList(m_pList->FindByColumn(m_nRecordIDColumn, nID, 0, VARIANT_TRUE));
						bRetry = FALSE;
						m_bWereRecordsAdded = TRUE;
					}
				}
			} // while (bRetry) {
		} // if (m_bAllowAddRecord) {
	}
	NxCatchAll("Error in CSelectDlg::OnBnClickedBtnAdd");
}

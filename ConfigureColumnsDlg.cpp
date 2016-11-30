// ConfigureColumnsDlg.cpp : implementation file
//
// (a.wilson 2014-07-10 09:58) - PLID 62526 - created to handle rearranging columns in a datalist.

#include "stdafx.h"
#include "Practice.h"
#include "ConfigureColumnsDlg.h"
#include "afxdialogex.h"

// CConfigureColumnsDlg dialog

IMPLEMENT_DYNAMIC(CConfigureColumnsDlg, CNxDialog)

enum class ConfigureListColumns {
	ColumnID = 0,
	Name = 1,
	OrderIndex = 2,
};

CConfigureColumnsDlg::CConfigureColumnsDlg(CWnd* pParent /*=NULL*/) : CNxDialog(CConfigureColumnsDlg::IDD, pParent)
{}

CConfigureColumnsDlg::CConfigureColumnsDlg(const CArray<ConfigureColumn>& aryColumns, CWnd* pParent /*=NULL*/) : CNxDialog(CConfigureColumnsDlg::IDD, pParent)
{
	m_bOrderChanged = false;
	m_aryConfigureColumns.Copy(aryColumns);
}

CConfigureColumnsDlg::~CConfigureColumnsDlg()
{}

void CConfigureColumnsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CONFIGURE_COLUMN_UP, m_btnUp);
	DDX_Control(pDX, IDC_CONFIGURE_COLUMN_DOWN, m_btnDown);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
}

BEGIN_MESSAGE_MAP(CConfigureColumnsDlg, CNxDialog)
	ON_BN_CLICKED(IDC_CONFIGURE_COLUMN_UP, &CConfigureColumnsDlg::OnBnClickedConfigureColumnUp)
	ON_BN_CLICKED(IDC_CONFIGURE_COLUMN_DOWN, &CConfigureColumnsDlg::OnBnClickedConfigureColumnDown)
END_MESSAGE_MAP()

BOOL CConfigureColumnsDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try {
		m_btnUp.AutoSet(NXB_UP);
		m_btnUp.EnableWindow(FALSE);
		m_btnDown.AutoSet(NXB_DOWN);
		m_btnDown.EnableWindow(FALSE);
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_pColumnList = BindNxDataList2Ctrl(IDC_CONFIGURE_COLUMN_LIST, false);

		//input the array into the datalist.
		int nCount = m_aryConfigureColumns.GetCount();
		if (nCount > 0) {
			for (int i = 0; i < nCount; i++) {
				ConfigureColumn cc = m_aryConfigureColumns.GetAt(i);

				NXDATALIST2Lib::IRowSettingsPtr pRow = m_pColumnList->GetNewRow();
				if (pRow) {
					pRow->PutValue((short)ConfigureListColumns::ColumnID, cc.nColumnID);
					pRow->PutValue((short)ConfigureListColumns::Name, _bstr_t(cc.strName));
					pRow->PutValue((short)ConfigureListColumns::OrderIndex, cc.nOrderIndex);
					m_pColumnList->AddRowAtEnd(pRow, NULL);
				}
			}
		}

	} NxCatchAll(__FUNCTION__);

	return TRUE;
}

// (a.wilson 2014-07-10 10:00) - PLID 62526 - move the current row up 1 spot.
void CConfigureColumnsDlg::OnBnClickedConfigureColumnUp()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pCurRow = m_pColumnList->GetCurSel();
		NXDATALIST2Lib::IRowSettingsPtr pPrevRow = pCurRow->GetPreviousRow();

		SwitchRowOrder(pCurRow, pPrevRow);
		SelSetConfigureColumnList(pCurRow);
	} NxCatchAll(__FUNCTION__);
}

// (a.wilson 2014-07-10 10:00) - PLID 62526 - move the current row down 1 spot.
void CConfigureColumnsDlg::OnBnClickedConfigureColumnDown()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pCurRow = m_pColumnList->GetCurSel();
		NXDATALIST2Lib::IRowSettingsPtr pNextRow = pCurRow->GetNextRow();

		SwitchRowOrder(pCurRow, pNextRow);
		SelSetConfigureColumnList(pCurRow);
	} NxCatchAll(__FUNCTION__);
}

// (a.wilson 2014-07-10 09:59) - PLID 62526 - this will move the current row to the other rows position and vice versa. also updates the array.
void CConfigureColumnsDlg::SwitchRowOrder(NXDATALIST2Lib::IRowSettingsPtr pCurrentRow, NXDATALIST2Lib::IRowSettingsPtr pOtherRow)
{
	if (pCurrentRow && pOtherRow) {
		long nCurOrder = pCurrentRow->GetValue((short)ConfigureListColumns::OrderIndex), nCurID = pCurrentRow->GetValue((short)ConfigureListColumns::ColumnID);
		long nOtherOrder = pOtherRow->GetValue((short)ConfigureListColumns::OrderIndex), nOtherID = pOtherRow->GetValue((short)ConfigureListColumns::ColumnID);

		//update the array of columns to match.
		bool bCurChanged = false, bOtherChanged = false;
		for (int i = 0; i < m_aryConfigureColumns.GetCount(); i++) {
			ConfigureColumn cc = m_aryConfigureColumns.GetAt(i);
			if (cc.nColumnID == nCurID) {
				cc.nOrderIndex = nOtherOrder;
				m_aryConfigureColumns.SetAt(i, cc);
				bCurChanged = true;
			}
			else if (cc.nColumnID == nOtherID) {
				cc.nOrderIndex = nCurOrder;
				m_aryConfigureColumns.SetAt(i, cc);
				bOtherChanged = true;
			}
			if (bCurChanged  && bOtherChanged) {
				break;
			}
		}

		//update the datalist to match the change.
		pCurrentRow->PutValue((short)ConfigureListColumns::OrderIndex, nOtherOrder);
		pOtherRow->PutValue((short)ConfigureListColumns::OrderIndex, nCurOrder);
		//resort to resemble the change.
		m_pColumnList->Sort();
		m_bOrderChanged = true;
	}
}

BEGIN_EVENTSINK_MAP(CConfigureColumnsDlg, CNxDialog)
	ON_EVENT(CConfigureColumnsDlg, IDC_CONFIGURE_COLUMN_LIST, 29, CConfigureColumnsDlg::SelSetConfigureColumnList, VTS_DISPATCH)
END_EVENTSINK_MAP()

// (a.wilson 2014-07-10 09:59) - PLID 62526 - if a selection was made alter the state of the up and down buttons to resemble the options.
void CConfigureColumnsDlg::SelSetConfigureColumnList(LPDISPATCH lpSel)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpSel);
		//if null row disable up and down.
		if (!pRow) {
			m_btnUp.EnableWindow(FALSE);
			m_btnDown.EnableWindow(FALSE);
		}
		else {
			//if next row is not null enable it, otherwise disable.
			if (pRow->GetNextRow()) {
				m_btnDown.EnableWindow(TRUE);
			}
			else {
				m_btnDown.EnableWindow(FALSE);
			}
			//if previous row is not null enable it, otherwise disable.
			if (pRow->GetPreviousRow()) {
				m_btnUp.EnableWindow(TRUE);
			}
			else {
				m_btnUp.EnableWindow(FALSE);
			}
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.wilson 2014-07-10 09:58) - PLID 62526 - allow whoever is using this dialog to get the changes.
CArray<ConfigureColumn>& CConfigureColumnsDlg::GetOrderedColumns()
{
	return m_aryConfigureColumns;
}
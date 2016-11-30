// EditWarningCategoriesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EditWarningCategoriesDlg.h"


// CEditWarningCategoriesDlg dialog

// (a.walling 2010-07-02 15:54) - PLID 39514 - Dialog to add and modify warning categories.
// It is inevitable that this will eventually be expanded when we start implementing multiple
// warnings for a patient.

enum EWarningCategoriesColumns {
	lcID = 0,
	lcName,
	lcColor,
	lcSelColor, // same as lcColor
	lcOrderIndex,
};


IMPLEMENT_DYNAMIC(CEditWarningCategoriesDlg, CNxDialog)

CEditWarningCategoriesDlg::CEditWarningCategoriesDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEditWarningCategoriesDlg::IDD, pParent)
{

}

CEditWarningCategoriesDlg::~CEditWarningCategoriesDlg()
{
}

void CEditWarningCategoriesDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_WARNING_CATEGORIES_NXCOLOR, m_bkgColor);
}

BOOL CEditWarningCategoriesDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try {
		m_bkgColor.SetColor(GetNxColor(GNC_ADMIN, 0));
		
		((CNxIconButton*)SafeGetDlgItem<CNexTechIconButton>(IDC_BUTTON_MOVECATUP))->AutoSet(NXB_UP);
		((CNxIconButton*)SafeGetDlgItem<CNexTechIconButton>(IDC_BUTTON_MOVECATDOWN))->AutoSet(NXB_DOWN);
		((CNxIconButton*)SafeGetDlgItem<CNexTechIconButton>(IDC_BUTTON_CHOOSE_COLOR))->AutoSet(NXB_MODIFY);
		((CNxIconButton*)SafeGetDlgItem<CNexTechIconButton>(IDC_BUTTON_ADD_WARNING_CATEGORY))->AutoSet(NXB_NEW);
		((CNxIconButton*)SafeGetDlgItem<CNexTechIconButton>(IDC_BUTTON_REMOVE_WARNING_CATEGORY2))->AutoSet(NXB_DELETE);
		((CNxIconButton*)SafeGetDlgItem<CNexTechIconButton>(IDOK))->AutoSet(NXB_OK);
		((CNxIconButton*)SafeGetDlgItem<CNexTechIconButton>(IDCANCEL))->AutoSet(NXB_CANCEL);

		SafeGetDlgItem<CWnd>(IDC_EDIT_PREVIEW_WARNING)->SetWindowText("This is an example warning to preview the way colored backgrounds will look for different warning categories. You can reset to default coloring by choosing solid black or white.");

		m_pList = BindNxDataList2Ctrl(this, IDC_LIST_EDIT_WARNING_CATEGORIES, GetRemoteData(), true);
		
		{
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pList->GetNewRow();
			pRow->PutValue(lcID, g_cvarNull);
			pRow->PutValue(lcName, (LPCTSTR)GetRemotePropertyText("DefaultWarningCategoryName", "General", 0, "<None>", true));
			// (a.walling 2010-07-30 17:30) - PLID 18081 - Default warning category color
			COLORREF defaultColor = GetRemotePropertyInt("DefaultWarningCategoryColor", 0, 0, "<None>", true);
			if (defaultColor == 0 || defaultColor == RGB(0xFF, 0xFF, 0xFF)) {
				pRow->PutValue(lcColor, g_cvarNull);
				pRow->PutValue(lcSelColor, g_cvarNull);
			} else {
				pRow->PutValue(lcColor, _variant_t((long)defaultColor));
				pRow->PutValue(lcSelColor, _variant_t((long)defaultColor));
				pRow->PutBackColor(defaultColor);
				pRow->PutBackColorSel(defaultColor);
			}
			pRow->PutValue(lcOrderIndex, (long)LONG_MIN);

			m_pList->CurSel = m_pList->AddRowSorted(pRow, NULL);
		}

		UpdateButtons();
	} NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_MESSAGE_MAP(CEditWarningCategoriesDlg, CNxDialog)
	ON_BN_CLICKED(IDC_BUTTON_MOVECATUP, &CEditWarningCategoriesDlg::OnBnClickedButtonMovecatup)
	ON_BN_CLICKED(IDC_BUTTON_MOVECATDOWN, &CEditWarningCategoriesDlg::OnBnClickedButtonMovecatdown)
	ON_BN_CLICKED(IDC_BUTTON_CHOOSE_COLOR, &CEditWarningCategoriesDlg::OnBnClickedButtonChooseColor)
	ON_BN_CLICKED(IDOK, &CEditWarningCategoriesDlg::OnBnClickedOk)
	ON_BN_DOUBLECLICKED(IDC_BUTTON_MOVECATUP, &CEditWarningCategoriesDlg::OnBnDoubleclickedButtonMovecatup)
	ON_BN_DOUBLECLICKED(IDC_BUTTON_MOVECATDOWN, &CEditWarningCategoriesDlg::OnBnDoubleclickedButtonMovecatdown)
	ON_BN_CLICKED(IDC_BUTTON_ADD_WARNING_CATEGORY, &CEditWarningCategoriesDlg::OnBnClickedButtonAddWarningCategory)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE_WARNING_CATEGORY2, &CEditWarningCategoriesDlg::OnBnClickedButtonRemoveWarningCategory)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CEditWarningCategoriesDlg, CNxDialog)
	ON_EVENT(CEditWarningCategoriesDlg, IDC_LIST_EDIT_WARNING_CATEGORIES, 28, CEditWarningCategoriesDlg::CurSelWasSetListWarningCategories, VTS_NONE)
	ON_EVENT(CEditWarningCategoriesDlg, IDC_LIST_EDIT_WARNING_CATEGORIES, 3, CEditWarningCategoriesDlg::DblClickCellListWarningCategories, VTS_DISPATCH VTS_I2)
	ON_EVENT(CEditWarningCategoriesDlg, IDC_LIST_EDIT_WARNING_CATEGORIES, 9, CEditWarningCategoriesDlg::EditingFinishingListWarningCategories, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CEditWarningCategoriesDlg, IDC_LIST_EDIT_WARNING_CATEGORIES, 6, CEditWarningCategoriesDlg::RButtonDownListWarningCategories, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEditWarningCategoriesDlg, IDC_LIST_EDIT_WARNING_CATEGORIES, 7, CEditWarningCategoriesDlg::RButtonUpListWarningCategories, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
END_EVENTSINK_MAP()

// CEditWarningCategoriesDlg message handlers

void CEditWarningCategoriesDlg::OnBnClickedButtonMovecatup()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pList->CurSel;

		if (pRow) {
			NXDATALIST2Lib::IRowSettingsPtr pPreviousRow = pRow->GetPreviousRow();

			if (pPreviousRow) {
				_variant_t varOrderIndex = pPreviousRow->GetValue(lcOrderIndex);
				pPreviousRow->PutValue(lcOrderIndex, pRow->GetValue(lcOrderIndex));
				pRow->PutValue(lcOrderIndex, varOrderIndex);

				m_pList->Sort();
			}
		}
		
		UpdateButtons();
	} NxCatchAll(__FUNCTION__);
}

void CEditWarningCategoriesDlg::OnBnDoubleclickedButtonMovecatup()
{
	try {
		OnBnClickedButtonMovecatup();
	} NxCatchAll(__FUNCTION__);
}

void CEditWarningCategoriesDlg::OnBnClickedButtonMovecatdown()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pList->CurSel;

		if (pRow) {
			NXDATALIST2Lib::IRowSettingsPtr pNextRow = pRow->GetNextRow();

			if (pNextRow) {
				_variant_t varOrderIndex = pNextRow->GetValue(lcOrderIndex);
				pNextRow->PutValue(lcOrderIndex, pRow->GetValue(lcOrderIndex));
				pRow->PutValue(lcOrderIndex, varOrderIndex);

				m_pList->Sort();
			}
		}

		UpdateButtons();
	} NxCatchAll(__FUNCTION__);
}

void CEditWarningCategoriesDlg::OnBnDoubleclickedButtonMovecatdown()
{
	try {
		OnBnClickedButtonMovecatdown();
	} NxCatchAll(__FUNCTION__);
}


void CEditWarningCategoriesDlg::OnBnClickedButtonChooseColor()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pList->CurSel;

		if (pRow == NULL) {
			return;
		}

		static const COLORREF colorWhite = RGB(0xFF, 0xFF, 0xFF);

		COLORREF color = (COLORREF)VarLong(pRow->GetValue(lcColor), colorWhite);

		CColorDialog dlg(color, 0, this);
		if (IDOK == dlg.DoModal()) {
			color = dlg.GetColor() & colorWhite; // get rid of anything in the unused bytes so we can safely compare

			if (color == 0) {
				color = colorWhite;
			}
			_variant_t varColor;
			if (color == colorWhite) {
				varColor = g_cvarNull;
			} else {
				varColor = (long)color;
			}

			// datalist bug -- does not respect colors that are set dynamically.
			pRow->PutValue(lcColor, varColor);
			pRow->PutValue(lcSelColor, varColor);

			NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_pList->GetNewRow();
			pNewRow->PutValue(lcID, pRow->GetValue(lcID));
			pNewRow->PutValue(lcName, pRow->GetValue(lcName));
			pNewRow->PutValue(lcColor, pRow->GetValue(lcColor));
			pNewRow->PutValue(lcSelColor, pRow->GetValue(lcSelColor));
			pNewRow->PutValue(lcOrderIndex, pRow->GetValue(lcOrderIndex));

			pNewRow->PutBackColor(color);
			pNewRow->PutBackColorSel(color);

			m_pList->RemoveRow(pRow);
			m_pList->AddRowSorted(pNewRow, NULL);
			m_pList->CurSel = pNewRow;

			UpdateButtons();
		}

	} NxCatchAll(__FUNCTION__);
}

void CEditWarningCategoriesDlg::OnBnClickedOk()
{
	try {
		m_pList->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);

		if (m_listRemovedCategories.IsEmpty() && m_pList->GetRowCount() == 1) {
			CNxDialog::OnOK();
			return;
		}

		CParamSqlBatch batch;

		{
			POSITION pos = m_listRemovedCategories.GetHeadPosition();
			while (pos) {
				int nDoomedID = m_listRemovedCategories.GetNext(pos);
				batch.Add("DELETE FROM WarningCategoriesT WHERE ID = {INT}", nDoomedID);
			}
		}

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pList->GetFirstRow();
		long nOrderIndex = 0;
		while (pRow) {
			long nOrderIndex = VarLong(pRow->GetValue(lcOrderIndex), -1);
			if (nOrderIndex == LONG_MIN) {
				COLORREF defaultColor = GetRemotePropertyInt("DefaultWarningCategoryColor", 0, 0, "<None>", true);
				if (defaultColor == RGB(0xFF, 0xFF, 0xFF)) {
					defaultColor = 0;
				}
				long newDefaultColor = VarLong(pRow->GetValue(lcColor), 0);
				if (newDefaultColor == RGB(0xFF, 0xFF, 0xFF)) {
					newDefaultColor = 0;
				}

				if (defaultColor != newDefaultColor) {
					SetRemotePropertyInt("DefaultWarningCategoryColor", newDefaultColor, 0, "<None>");
				}

				CString defaultName = GetRemotePropertyText("DefaultWarningCategoryName", "General", 0, "<None>", true);
				CString newDefaultName = VarString(pRow->GetValue(lcName));

				if (defaultName != newDefaultName) {
					SetRemotePropertyText("DefaultWarningCategoryName", newDefaultName, 0, "<None>");
				}

			} else {
			long nID = VarLong(pRow->GetValue(lcID), -1);
				if (nID != -1) {
					batch.Add("UPDATE WarningCategoriesT SET Name = {STRING}, Color = {VT_I4}, OrderIndex = {INT} WHERE ID = {INT}",
						VarString(pRow->GetValue(lcName)), pRow->GetValue(lcColor), nOrderIndex, nID);
				} else {
					batch.Add("INSERT INTO WarningCategoriesT(Name, Color, OrderIndex) VALUES({STRING}, {VT_I4}, {INT})",
						VarString(pRow->GetValue(lcName)), pRow->GetValue(lcColor), nOrderIndex);
				}
				
				nOrderIndex++;
			}

			pRow = pRow->GetNextRow();
		}

		batch.Execute(GetRemoteData());

		m_listRemovedCategories.RemoveAll();

		CNxDialog::OnOK();
	} NxCatchAll(__FUNCTION__);
}

void CEditWarningCategoriesDlg::OnBnClickedButtonAddWarningCategory()
{
	try {
		long nMaxOrderIndex = -1;
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pList->GetFirstRow();
		while (pRow) {
			nMaxOrderIndex = max(nMaxOrderIndex, VarLong(pRow->GetValue(lcOrderIndex)));
			pRow = pRow->GetNextRow();
		}
		nMaxOrderIndex++;
		if (nMaxOrderIndex < 0) { 
			nMaxOrderIndex = 0;
		}

		CString strNewCategoryName;
		long nNewCategoryOrdinal = 0;
		bool bFound = false;
		do {
			bFound = false;

			nNewCategoryOrdinal++;
			strNewCategoryName.Format("New Category %li", nNewCategoryOrdinal);
			
			NXDATALIST2Lib::IRowSettingsPtr pCheckRow = m_pList->GetFirstRow();
			while (pCheckRow) {
				CString strName = VarString(pCheckRow->GetValue(lcName));
				if (strName.CompareNoCase(strNewCategoryName) == 0) {
					bFound = true;
					break;
				}

				pCheckRow = pCheckRow->GetNextRow();
			}
		} while (bFound);

		pRow = m_pList->GetNewRow();
		pRow->PutValue(lcID, g_cvarNull);
		pRow->PutValue(lcName, (LPCTSTR)strNewCategoryName);
		pRow->PutValue(lcColor, g_cvarNull);
		pRow->PutValue(lcSelColor, g_cvarNull);
		pRow->PutValue(lcOrderIndex, (long)nMaxOrderIndex);

		m_pList->AddRowSorted(pRow, NULL);

		m_pList->CurSel = pRow;
		UpdateButtons();

		m_pList->StartEditing(pRow, lcName);
	} NxCatchAll(__FUNCTION__);
}


void CEditWarningCategoriesDlg::OnBnClickedButtonRemoveWarningCategory()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(m_pList->CurSel);

		if (pRow && VarLong(pRow->GetValue(lcOrderIndex), -1) != LONG_MIN) {
			RemoveWarningCategoryByRow(pRow);
		}
	} NxCatchAll(__FUNCTION__);
}

void CEditWarningCategoriesDlg::CurSelWasSetListWarningCategories()
{
	try {
		UpdateButtons();
	} NxCatchAll(__FUNCTION__);
}

void CEditWarningCategoriesDlg::DblClickCellListWarningCategories(LPDISPATCH lpRow, short nColIndex)
{
	try {
		OnBnClickedButtonChooseColor();
	} NxCatchAll(__FUNCTION__);
}

void CEditWarningCategoriesDlg::EditingFinishingListWarningCategories(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try {
		if (*pbCommit) {
			NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

			if (pRow == NULL) {
				*pbCommit = FALSE;
				*pbContinue = TRUE;
				return;
			}

			CString strUserEnteredName = strUserEntered;
			strUserEnteredName.TrimRight();
			strUserEnteredName.TrimLeft();

			if (strUserEnteredName.GetLength() > 50) {
				*pbCommit = FALSE;
				//*pbContinue = MessageBox("This category name is too long.", NULL, MB_ICONSTOP | MB_OKCANCEL) == IDOK ? FALSE : TRUE;
				MessageBox("This category name is too long.", NULL, MB_ICONEXCLAMATION);
				return;
			}

			NXDATALIST2Lib::IRowSettingsPtr pCheckRow = m_pList->GetFirstRow();
			while (pCheckRow) {
				if (!pCheckRow->IsSameRow(pRow)) {
					CString strName = VarString(pCheckRow->GetValue(lcName));
					if (strName.CompareNoCase(strUserEnteredName) == 0) {					
						*pbCommit = FALSE;
						//*pbContinue = MessageBox("This category name is already in use.", NULL, MB_ICONSTOP | MB_OKCANCEL) == IDOK ? FALSE : TRUE;
						MessageBox("This category name is already in use.", NULL, MB_ICONEXCLAMATION);
						return;
					}
				}

				pCheckRow = pCheckRow->GetNextRow();
			}
		}

	} NxCatchAll(__FUNCTION__);
}

void CEditWarningCategoriesDlg::UpdateButtons()
{
	try {
		if (!m_pList) {
			return;
		}

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pList->CurSel;

		BOOL bCanMoveUp = FALSE;
		BOOL bCanMoveDown = FALSE;
		_variant_t varColor = g_cvarNull;

		BOOL bCanRemove = TRUE;
		if (pRow) {
			varColor = pRow->GetValue(lcColor);

			if (VarLong(pRow->GetValue(lcOrderIndex), -1) == LONG_MIN) {
				bCanMoveDown = FALSE;
				bCanMoveUp = FALSE;
				bCanRemove = FALSE;
			} else {
				if (pRow->GetNextRow()) {
					bCanMoveDown = TRUE;
				}

				if (pRow->GetPreviousRow()) {
					if (VarLong(pRow->GetPreviousRow()->GetValue(lcOrderIndex), -1) != LONG_MIN) {
						bCanMoveUp = TRUE;
					}
				}
			}
		
			SafeGetDlgItem<CWnd>(IDC_BUTTON_CHOOSE_COLOR)->EnableWindow(TRUE);
		} else {
			SafeGetDlgItem<CWnd>(IDC_BUTTON_CHOOSE_COLOR)->EnableWindow(FALSE);
		}

		SafeGetDlgItem<CWnd>(IDC_BUTTON_MOVECATUP)->EnableWindow(bCanMoveUp);
		SafeGetDlgItem<CWnd>(IDC_BUTTON_MOVECATDOWN)->EnableWindow(bCanMoveDown);
		
		SafeGetDlgItem<CWnd>(IDC_BUTTON_REMOVE_WARNING_CATEGORY2)->EnableWindow(bCanRemove);

		CNxEdit* pNxEdit = SafeGetDlgItem<CNxEdit>(IDC_EDIT_PREVIEW_WARNING);
		if (varColor.vt == VT_I4 && VarLong(varColor) != RGB(0xFF, 0xFF, 0xFF)) {
			COLORREF color = (COLORREF)VarLong(varColor);
			pNxEdit->SetBackgroundColorStandard(color, true, true);
			pNxEdit->SetBackgroundColorHovered(color, true, true);
			pNxEdit->SetBackgroundColorFocus(color, true, true);
			pNxEdit->SetBackgroundColorHoveredFocus(color, true, true);
		} else {
			pNxEdit->ResetBackgroundColorStandard();
			pNxEdit->ResetBackgroundColorHovered();			
			pNxEdit->ResetBackgroundColorFocus();
			pNxEdit->ResetBackgroundColorHoveredFocus();
		}

		pNxEdit->Invalidate();

	} NxCatchAll(__FUNCTION__);
}

void CEditWarningCategoriesDlg::RButtonDownListWarningCategories(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		m_pList->CurSel = pRow;
	} NxCatchAll(__FUNCTION__);
}

void CEditWarningCategoriesDlg::RButtonUpListWarningCategories(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if (pRow && VarLong(pRow->GetValue(lcOrderIndex), -1) != LONG_MIN) {

			CMenu menu;
			menu.CreatePopupMenu();

			menu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, -1, "Remove");

			CPoint pt;
			GetCursorPos(&pt);

			int nCmdId = menu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this, NULL);
			if (nCmdId == -1) {
				RemoveWarningCategoryByRow(pRow);
			}
		}
	} NxCatchAll(__FUNCTION__);
}

void CEditWarningCategoriesDlg::RemoveWarningCategoryByRow(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	if (pRow && VarLong(pRow->GetValue(lcOrderIndex), -1) != LONG_MIN) {

		int nID = VarLong(pRow->GetValue(lcID), -1);

		if (nID != -1) {
			ADODB::_RecordsetPtr prs = CreateParamRecordset("SELECT COUNT(*) AS NumRecords FROM PersonT WHERE WarningCategoryID = {INT}", nID);
			if (!prs->eof) {
				int nCount = AdoFldLong(prs, "NumRecords", 0);
				if (nCount > 0) {
					MessageBox(FormatString("This category may not be removed; it is used on %li patient warnings.", nCount), NULL, MB_ICONSTOP);
					return;
				}
			}
			m_listRemovedCategories.AddTail(nID);
		}

		m_pList->RemoveRow(pRow);

		UpdateButtons();
	}
}
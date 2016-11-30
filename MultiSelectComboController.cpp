#include "StdAfx.h"
#include "MultiSelectComboController.h"

// (r.gonet 2016-01-22) - PLID 68041 - Added.
/// <summary>Callback for the multi-select dialog enabling the user to select all and unselect all.</summary>
/// <param name="pwndMultSelDlg">Pointer to the CMultiSelectDlg that called this callback.</param>
/// <param name="lpRow">Pointer to the datalist2 row that the user right-clicked on.</param>
/// <returns>TRUE if the context menu was shown. FALSE if not.</returns>
BOOL CALLBACK MultiSelectDlgDefaultContextMenuProc(IN CMultiSelectDlg *pwndMultSelDlg, IN LPARAM pParam, IN NXDATALIST2Lib::IRowSettings *lpRow, IN CWnd* pContextWnd, IN const CPoint &point, IN OUT CArray<long, long> &m_aryOtherChangedMasterIDs)
{
	// The context menu for the data element list is based on the current selection
	if (lpRow) {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		// Build the menu for the current row
		CMenu mnu;
		mnu.CreatePopupMenu();
		mnu.AppendMenu(MF_ENABLED | MF_STRING | MF_BYPOSITION, 1, "&Select All");
		mnu.AppendMenu(MF_ENABLED | MF_STRING | MF_BYPOSITION, 2, "&Unselect All");

		// Pop up the menu and gather the immediate response
		CPoint pt = CalcContextMenuPos(pContextWnd, point);
		long nMenuResult = mnu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, pt.x, pt.y, pwndMultSelDlg);
		switch (nMenuResult) {
		case 1: // select all
		{
			// Check all the rows in the multi-select list that are unchecked
			NXDATALIST2Lib::IRowSettingsPtr pRowIter = pwndMultSelDlg->GetDataList2()->GetFirstRow();
			while (pRowIter) {
				if (!VarBool(pRowIter->GetValue(CMultiSelectDlg::mslcSelected), FALSE)) {
					pRowIter->PutValue(CMultiSelectDlg::mslcSelected, g_cvarTrue);
				}
				pRowIter = pRowIter->GetNextRow();
			}
			break;
		}
		case 2: // unselect all
		{
			// Uncheck all the rows in the multi-select list that are checked
			NXDATALIST2Lib::IRowSettingsPtr pRowIter = pwndMultSelDlg->GetDataList2()->GetFirstRow();
			while (pRowIter) {
				if (VarBool(pRowIter->GetValue(CMultiSelectDlg::mslcSelected), TRUE)) {
					pRowIter->PutValue(CMultiSelectDlg::mslcSelected, g_cvarFalse);
				}
				pRowIter = pRowIter->GetNextRow();
			}
			break;
		}
		case 0:
			// The user canceled, do nothing
			break;
		default:
			// Unexpected response!
			ASSERT(FALSE);
			ThrowNxException("%s : Unexpected return value %li from context menu!", __FUNCTION__, nMenuResult);
		}
		return TRUE;
	} else {
		return FALSE;
	}
}

// (r.gonet 2016-01-22) - PLID 68041 - Creates a new CMultiSelectComboController.
CMultiSelectComboController::CMultiSelectComboController(CWnd *pParent, CString strElementName,
	CString strMultiSelectPromptDescription)
{
	m_pParent = pParent;
	m_strElementName = strElementName;
	m_strMultiSelectPromptDescription = strMultiSelectPromptDescription;
}

// (r.gonet 2016-01-22) - PLID 68041 - Creates a new CMultiSelectComboController.
CMultiSelectComboController::CMultiSelectComboController(CWnd *pParent, CString strElementName,
	CString strMultiSelectPromptDescription, CString strRememberedIDsPropertyName)
	: CMultiSelectComboController(pParent, strElementName, strMultiSelectPromptDescription)
{
	m_strRememberedIDsPropertyName = strRememberedIDsPropertyName;
}

// (r.gonet 2016-01-22) - PLID 68041 - Binds controls with the controller. Needs to be called first.
void CMultiSelectComboController::BindController(CNxLabel &nxlMultiLabel, NXDATALIST2Lib::_DNxDataListPtr pCombo, UINT nComboControlID, bool bAutoInitialize,
	short nIDColumnIndex/* = 0*/, short nDescriptionColumnIndex/* = 1*/)
{
	m_pMultiLabel = &nxlMultiLabel;
	m_pCombo = pCombo;
	m_nComboControlID = nComboControlID;
	m_nIDColumnIndex = nIDColumnIndex;
	m_nDescriptionColumnIndex = nDescriptionColumnIndex;
	m_vtIDType = pCombo->GetColumn(m_nIDColumnIndex)->DataType;
	m_bIsBound = true;

	if (bAutoInitialize) {
		InitializeCombo(true);
	}
}

// (r.gonet 2016-01-22) - PLID 68041 - Initializes the combo box, loads its rows, and remembers previous selections
// if set to remember. Needs to be called after BindController.
void CMultiSelectComboController::InitializeCombo(bool bRemoveExistingRows)
{
	AssertIsBound();
	if (bRemoveExistingRows) {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pCombo->GetFirstRow();
		while ((pRow = m_pCombo->GetFirstRow()) != nullptr) {
			m_pCombo->RemoveRow(pRow);
		}
	}
	LoadSpecialRows();
	LoadComboRows();
	m_bIsInitialized = true;

	if (m_bRememberSelections) {
		// We're remembering on this combo, so load the previous selections.
		LoadRememberedSelections();
	} else {
		// Load the default selection.
		SelectDefault_Internal();
	}
	SyncControls();
	if (m_bRememberSelections) {
		SaveSelections();
	}
}

// (r.gonet 2016-01-22) - PLID 68041 - Checks and throws an exception if the controller is not bound to controls.
void CMultiSelectComboController::AssertIsBound()
{
	if (!m_bIsBound) {
		ThrowNxException("%s : Controller is not bound to dialog controls.", __FUNCTION__);
	}
}

// (r.gonet 2016-01-22) - PLID 68041 - Checks and throws an exception if the controller is not initialized
void CMultiSelectComboController::AssertIsInitialized()
{
	AssertIsBound();
	if (!m_bIsInitialized) {
		ThrowNxException("%s : Controller is not initialized.", __FUNCTION__);
	}
}

// (r.gonet 2016-01-22) - PLID 68041 - Validates the selections and synchronizes the controls with the selections.
void CMultiSelectComboController::SyncControls()
{
	// Bring the controller's set of selected IDs up to date with what actually exists in the datalist.
	ValidateSelections();

	// (r.gonet 2016-01-22) - PLID 68041 - Updates the on-screen controls to reflect the selections
	// stored in the controller. Shows and hides the combo box or hyperlink label as appropriate.
	UpdateControls();
}

// (r.gonet 2016-01-22) - PLID 68041 - Updates the on-screen controls to reflect the selections
// stored in the controller. Shows and hides the combo box or hyperlink label as appropriate.
void CMultiSelectComboController::UpdateControls()
{
	AssertIsInitialized();

	if (m_bVisible) {
		// If the all row is available in the combo box and the user selected all the rows, then select the All row in the combo box.
		if (m_bAllSelected == true || m_arySelectedIDs.GetSize() == 0) {
			// Select the all row.
			if (!m_pCombo->FindByColumn(m_nIDColumnIndex, m_varAllRowID, m_pCombo->GetFirstRow(), VARIANT_TRUE)) {
				ThrowNxException("%s : Could not select the All row.", __FUNCTION__);
			}

			// Hide any hyperlink we might have been showing and show the combo box instead.
			m_pMultiLabel->SetText("");
			m_pMultiLabel->SetType(dtsDisabledHyperlink);
			m_pMultiLabel->ShowWindow(SW_HIDE);
			m_pParent->GetDlgItem(m_nComboControlID)->ShowWindow(SW_SHOW);
		} else if (m_arySelectedIDs.GetSize() == 1) {
			// Show single selections in the combo box.
			
			// Select the row with the selected ID.
			_variant_t varSelectedID = m_arySelectedIDs.GetAt(0);
			NXDATALIST2Lib::IRowSettingsPtr pSelectedRow = m_pCombo->FindByColumn(m_nIDColumnIndex, varSelectedID, m_pCombo->GetFirstRow(), VARIANT_TRUE);
			if (pSelectedRow == nullptr) {
				// We expect the IDs in the selected ID array to correspond with a row in the combo box, especially since we just
				// validated them in UpdateController.
				ThrowNxException("%s : Could not find selected row with ID = %s.", __FUNCTION__, AsString(varSelectedID));
			} else {
				// We found the row corresponding to our selected ID.
			}

			// Hide the multiple selection label.
			m_pMultiLabel->SetText("");
			m_pMultiLabel->SetType(dtsDisabledHyperlink);
			m_pMultiLabel->ShowWindow(SW_HIDE);
			// Show the combo.
			m_pParent->GetDlgItem(m_nComboControlID)->ShowWindow(SW_SHOW);
		} else if (m_arySelectedIDs.GetSize() > 1) {
			// Show multiple selections (other than All selections) in the multi select label
			CString strMultiSelectString;

			// Build the multiple selection string to show in the label.
			for (int i = 0; i < m_arySelectedIDs.GetSize(); i++) {
				_variant_t varSelectedID = m_arySelectedIDs[i];
				// Lookup the name in the combo.
				NXDATALIST2Lib::IRowSettingsPtr pSelectedRow = m_pCombo->FindByColumn(m_nIDColumnIndex, varSelectedID, m_pCombo->GetFirstRow(), VARIANT_FALSE);
				if (pSelectedRow) {
					// Found the row in the datalist. Add its name to the multi select label text.
					CString strSelectedDescription = pSelectedRow->GetValue(m_nDescriptionColumnIndex);
					strMultiSelectString += strSelectedDescription + ", ";
				} else {
					// We expect the IDs in the selected ID array to correspond with a row in the combo box, especially since we just
					// validated them in UpdateController.
					ThrowNxException("%s : Could not find selected row with ID = %s", __FUNCTION__, AsString(varSelectedID));
				}
			}

			// Trim the trailing comma.
			if (strMultiSelectString.GetLength() >= 2) {
				strMultiSelectString = strMultiSelectString.Left(strMultiSelectString.GetLength() - 2);
			} else {
				// Its empty. This shouldn't be possible due to the above conditions.
			}

			// Limit the length of the resultant string.
			if (strMultiSelectString.GetLength() > m_nMaxLabelCharLength) {
				strMultiSelectString = strMultiSelectString.Left(m_nMaxLabelCharLength);
			} else {
				// Under the limit, we are good.
			}

			// Hide the combo box.
			m_pParent->GetDlgItem(m_nComboControlID)->ShowWindow(SW_HIDE);
			// Show the multiple selection label.
			m_pMultiLabel->ShowWindow(SW_SHOW);
			m_pMultiLabel->SetText(strMultiSelectString);
			m_pMultiLabel->SetType(dtsHyperlink);
		} else {
			// OK was pressed but no row was selected. This should be impossible.
			ThrowNxException("%s : No row was selected in the multi-select dialog but OK was pressed.", __FUNCTION__);
		}

		// The text updated maybe. Force the parent to redraw it.
		m_pMultiLabel->AskParentToRedrawWindow();
	} else {
		// The controller's controls are supposed to be hidden.
		m_pParent->GetDlgItem(m_nComboControlID)->ShowWindow(SW_HIDE);
		m_pMultiLabel->ShowWindow(SW_HIDE);
	}
}

// (r.gonet 2016-01-23) - PLID 68041 - Compares the controller's set of selected IDs with the bound datalist's rows and removes
// from the controller any selected IDs that don't have corresponding rows in the datalist.
void CMultiSelectComboController::ValidateSelections()
{
	AssertIsInitialized();

	// Record the old selection count and All state in order to see if something changed at the end of this function, and save the selections
	// if necessary.
	long nOldSelectionCount = m_arySelectedIDs.GetSize();
	bool bOldAllSelected = m_bAllSelected;

	// Gather the selected IDs that exist in the bound datalist.
	CVariantArray aryIDsToKeep;
	for (int i = 0; i < m_arySelectedIDs.GetSize(); i++) {
		_variant_t varSelectedID = m_arySelectedIDs[i];
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pCombo->FindByColumn(m_nIDColumnIndex, varSelectedID, m_pCombo->GetFirstRow(), VARIANT_FALSE);
		if (pRow == nullptr) {
			// A row with this ID doesn't exist.
		} else {
			// It exists. Keep it selected.
			aryIDsToKeep.Add(varSelectedID);
		}
	}

	if (m_arySelectedIDs.GetSize() != aryIDsToKeep.GetSize()) {
		// Some selected IDs are invalid. Use the subset that are still valid. Select that subset.
		Select_Internal(aryIDsToKeep);
	} else {
		// All selected IDs are valid.
	}

	// Check to see if the selected IDs are such that we should select a special row and then 
	// select that special row if necessary.
	if (ShouldDefaultRowBeSelected()) {
		OnSelectDefault();
	} else if (ShouldAllRowBeSelected()) {
		OnSelectAll();
	} else {
		// Whatever is selected now is fine.
	}

	// Since ValidateSelections may change the selected IDs, save the potentially new selections.
	if ((m_arySelectedIDs.GetSize() != nOldSelectionCount || m_bAllSelected != bOldAllSelected) && m_bRememberSelections) {
		SaveSelections();
	}
}

// (r.gonet 2016-01-23) - PLID 68041 - Returns true if the controller state is such that the default row should be selected, even
// if it isn't currently.
bool CMultiSelectComboController::ShouldDefaultRowBeSelected()
{
	// If there is nothing selected and the all row isn't selected, the default row should be selected.
	return !m_bAllSelected && m_arySelectedIDs.GetSize() == 0;
}

// (r.gonet 2016-01-23) - PLID 68041 - Returns true if the controller state is such that the All row should be selected, even
// if it isn't currently.
bool CMultiSelectComboController::ShouldAllRowBeSelected()
{
	if (!m_bAllSelected) {
		if (m_arySelectedIDs.GetSize() == 0) {
			// The default row should be selected (which actually may be the All row).
			return false;
		}
		// Count how many actual non-special rows the combo has.
		long nNumRealComboRows = 0;
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pCombo->GetFirstRow();
		while (pRow != nullptr) {
			_variant_t varRowID = pRow->GetValue(m_nIDColumnIndex);
			if (varRowID != m_varAllRowID && varRowID != m_varMultipleRowID) {
				nNumRealComboRows++;
			} else {
				// Skip the < All > and < Multiple > rows, since they are the special rows.
			}

			pRow = pRow->GetNextRow();
		}

		if (m_arySelectedIDs.GetSize() == nNumRealComboRows) {
			if (m_arySelectedIDs.GetSize() != 1) {
				// All IDs are selected, so yes the All row should be selected instead. Note that we don't want to
				// use the All row when we just have a single row in the combo. Although either would be valid,
				// selecting the actual row looks nicer and is less confusing to the usrer, IMHO.
				return true;
			} else {
				// We don't count a selection of a single row as All. It must be a selection of more than one row.
				return false;
			}
		} else {
			// There are at least some rows not selected.
			return false;
		}
	} else {
		// Naturally.
		return true;
	}
}

// (r.gonet 2016-01-22) - PLID 68041 - Sets whether all bound controls are visible.
void CMultiSelectComboController::SetVisible(bool bVisible)
{
	m_bVisible = bVisible;
	// Redraw the controls.
	SyncControls();
}

// (r.gonet 2016-01-22) - PLID 68041 - Programmatically selects a row by ID.
bool CMultiSelectComboController::Select(_variant_t varID)
{
	bool bResult = Select_Internal(varID);

	SyncControls();
	if (m_bRememberSelections) {
		SaveSelections();
	}

	return bResult;
}

// (r.gonet 2016-01-22) - PLID 68041 - Programmatically selects multiple rows by ID.
void CMultiSelectComboController::Select(CVariantArray &aryIDs)
{
	Select_Internal(aryIDs);

	SyncControls();
	if (m_bRememberSelections) {
		SaveSelections();
	}
}

// (r.gonet 2016-01-22) - PLID 68041 - Programmatically selects All rows and sets the selection to < All >
void CMultiSelectComboController::SelectAll()
{
	SelectAll_Internal();

	SyncControls();
	if (m_bRememberSelections) {
		SaveSelections();
	}
}

// (r.gonet 2016-01-22) - PLID 68041 - Programmatically selects a row by ID.
bool CMultiSelectComboController::Select_Internal(_variant_t varID)
{
	CVariantArray aryIDs;
	aryIDs.Add(varID);
	Select_Internal(aryIDs);
	// Return whether selection was successful.
	return m_arySelectedIDs.GetCount() > 0 && m_arySelectedIDs[0] == varID;
}

// (r.gonet 2016-01-22) - PLID 68041 - Programmatically selects multiple rows by ID.
void CMultiSelectComboController::Select_Internal(CVariantArray &aryIDs)
{
	AssertIsInitialized();

	// Remove all previous selections.
	m_arySelectedIDs.RemoveAll();
	m_bAllSelected = false;

	if (aryIDs.GetSize() == 0) {
		// If there are no IDs to be selected, select the default row.
		OnSelectDefault();
	} else {
		// Otherwise ensure the rows the caller wants to select actually exist, then select them.
		for (int i = 0; i < aryIDs.GetSize(); i++) {
			_variant_t varSelectedID = aryIDs[i];
			
			// (r.gonet 2016-01-23) - PLID 68041 - Note that there is no need to check if a row with this ID actually exists
			// here. When we update the controls, we'll weed out any selected IDs that don't exist in the datalist.
			
			if (varSelectedID == m_varAllRowID) {
				// This is the correct way to select All. If they pass its ID to Select.
				OnSelectAll();
				break;
			} else if (varSelectedID == m_varMultipleRowID) {
				// How did this get selected? Ignore it. If the caller wants to do multiple selection, use either DoMultiSelect
				// or programmatically pass each ID to be selected to this function.
				ASSERT(FALSE);
			} else {
				// Add this ID to our array of selected IDs.
				m_arySelectedIDs.Add(varSelectedID);
			}
		}
	}
}

// (r.gonet 2016-01-22) - PLID 68041 - Programmatically selects All rows and sets the selection to < All >
void CMultiSelectComboController::SelectAll_Internal()
{
	AssertIsInitialized();

	CVariantArray aryIDs;
	aryIDs.Add(m_varAllRowID);
	Select_Internal(aryIDs);
}

// (r.gonet 2016-01-22) - PLID 68041 - Selects the default ID. Currently a simple wrapper around SelectAll_Internal(), but allows the
// caller to demonstrate intent.
void CMultiSelectComboController::SelectDefault_Internal()
{
	SelectAll_Internal();
}

// (r.gonet 2016-01-22) - PLID 68041 - Loads the remembered selections from ConfigRT. If those selections
// are no longer available in the datalist, then those selections are not re-selected. If loading fails
// to find any rows in the datalist that are remembered, then < All > is selected. If loading finds all
// remembered selections constitute all available selections, then < All > is selected.
void CMultiSelectComboController::LoadRememberedSelections()
{
	if (!m_bRememberSelections) {
		ThrowNxException("%s : Attempted to load selections while selection remembering is disabled for this multi-select combo.", __FUNCTION__);
	}

	// Load the remembered IDs from a per-user property. If this ever needs to be global, add a flag parameter for that in EnableRememberSelections.
	CString strSelectedIDs = GetRemotePropertyText(m_strRememberedIDsPropertyName, "", 0, GetCurrentUserName(), true);
	CStringArray sarySelectedIDs;
	// Selected IDs are stored in a comma separated value string.
	SplitString(strSelectedIDs, ",", FALSE, sarySelectedIDs);

	// Note that this remembered list of IDs may include the All ID. The Select_Internal function will handle that.

	// Convert the remembered selected IDs to variants and store them in an array.
	CVariantArray arySavedSelectedIDs;
	for (int i = 0; i < sarySelectedIDs.GetSize(); i++) {
		CString strSelectedID = sarySelectedIDs[i];
		_variant_t varSelectedID;
		// The remembered IDs must be the type of the ID column.
		switch (m_vtIDType) {
		case VT_I4:
			varSelectedID = _variant_t(atol(strSelectedID), VT_I4);
			break;
		case VT_BSTR:
			varSelectedID = _variant_t(_bstr_t(strSelectedID));
			break;
		default:
			ThrowNxException("%s : Unhandled ID variant type.", __FUNCTION__);
		}

		arySavedSelectedIDs.Add(varSelectedID);
	}

	// Now select the remembered IDs.
	Select_Internal(arySavedSelectedIDs);
}

// (r.gonet 2016-01-22) - PLID 68041 - Saves the current selections to ConfigRT.
void CMultiSelectComboController::SaveSelections()
{
	if (!m_bRememberSelections) {
		ThrowNxException("%s : Attempted to save selections while selection remembering is disabled for this multi-select combo.", __FUNCTION__);
	}

	CString strSelectedIDs;
	if (!m_bAllSelected) {
		// Build a comma separated value list of the selected IDs.
		for (int i = 0; i < m_arySelectedIDs.GetSize(); i++) {
			_variant_t varID = m_arySelectedIDs[i];
			strSelectedIDs += FormatString("%s,", AsString(varID));
		}
		strSelectedIDs = strSelectedIDs.TrimRight(',');
	} else {
		// Store the All ID in the string. I'd rather do this now than have to try to differentiate between All and None in the future if we add an option for None.
		strSelectedIDs = AsString(m_varAllRowID);
	}

	// Save them in a per-user text property.
	SetRemotePropertyText(m_strRememberedIDsPropertyName, strSelectedIDs, 0, GetCurrentUserName());
}

// (r.gonet 2016-01-22) - PLID 68041 - Handles when the user manually selects a row. Should be called in the datalist
// OnSelChosen or OnSelChanged handler.
// Returns true if a selection was made and false if it was cancelled.
bool CMultiSelectComboController::HandleSelection()
{
	AssertIsInitialized();

	// Declare the variable storing the return value. Assume selection will be done.
	bool bSelectionMade = true;
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pCombo->CurSel;
	if (pRow == nullptr) {
		// Somehow the user managed to select a null row. Select the default row instead.
		SelectDefault_Internal();
		SyncControls();
	} else {
		// See what the user selected and handle it so our internal list of selected IDs is kept up to date.
		_variant_t varID = pRow->GetValue(m_nIDColumnIndex);
		if (varID == m_varAllRowID) {
			// they selected the All row. Good selection.
			OnSelectAll();
		} else if (varID == m_varMultipleRowID) {
			// Ew, they selected < Multiple >. This requires action on our part. Launch the multi-select dialog.
			bSelectionMade = OnSelectMultiple();
			// Reflect the multiple selections in the controls. Multi select is the only legit thing they could select that requires
			// the controller to change the controls.
			SyncControls();
		} else {
			// They selected a single normal row. An easy selection.
			OnSelectSingle();
		}
	}

	// Save the selections if remembering is enabled.
	if (m_bRememberSelections) {
		SaveSelections();
	}

	return bSelectionMade;
}

// (r.gonet 2016-01-22) - PLID 68041 - Handles when the user clicks a single row. Protected because there is no reason an external class
// needs to call this. OnSelect() should be called instead by external classes.
void CMultiSelectComboController::OnSelectSingle()
{
	AssertIsInitialized();

	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pCombo->CurSel;
	if (pRow == nullptr) {
		ThrowNxException("%s : pRow is NULL", __FUNCTION__);
	}
	_variant_t varID = pRow->GetValue(m_nIDColumnIndex);
	if (varID == m_varAllRowID || varID == m_varMultipleRowID) {
		ThrowNxException("%s : Special IDs are unsupported by this function.", __FUNCTION__);
	}
	m_arySelectedIDs.RemoveAll();
	m_arySelectedIDs.Add(varID);
	m_bAllSelected = false;
}

// (r.gonet 2016-01-22) - PLID 68041 - Handles when the user manually selects the < Multiple > row. Launches the CMultiSelectDlg popup.
// Public because external classes need to call this when handling OnLabelClick.
// Returns true if a selection was made and false if it was cancelled.
bool CMultiSelectComboController::DoMultiSelect()
{
	// Launch the Multi Select dialog and get the user's selections.
	bool bSelectionMade = OnSelectMultiple();
	SyncControls();
	// Remember the selections if we are remembering.
	if (m_bRememberSelections) {
		SaveSelections();
	}
	
	return bSelectionMade;
}

// (r.gonet 2016-01-22) - PLID 68041 - Handles when the user manually selects the < Multiple > row. Launches the CMultiSelectDlg popup.
// Public because external classes need to call this when handling OnLabelClick.
// Returns true if a selection was made and false if it was cancelled.
bool CMultiSelectComboController::OnSelectMultiple()
{
	AssertIsInitialized();

	CMultiSelectDlg dlg(m_pParent, _Q(m_strElementName));
	// Setup the CMultiSelectDlg. The following function is virtual so child classes can change the dialog's setup.
	SetMultiSelectDialogOptions(dlg);

	if (m_arySelectedIDs.GetSize() > 1) {
		// Preselect whatever we had selected before.
		dlg.PreSelect(m_arySelectedIDs);
	}

	// Ensure that we don't end up showing the special rows.
	CVariantArray vaIDsToSkip;
	vaIDsToSkip.Add(m_varAllRowID);
	vaIDsToSkip.Add(m_varMultipleRowID);

	// Now show the user the multi-select dialog.
	if (IDOK == dlg.OpenWithDataList2(m_pCombo, vaIDsToSkip, m_strMultiSelectPromptDescription, 1, 0xFFFFFFFF, m_nIDColumnIndex, m_nDescriptionColumnIndex, nullptr)) {
		// OK, they selected some records. Reflect their selections in the combo box and multi-label.

		// We have some new selections.
		CVariantArray aryNewSelectedIDs;
		dlg.FillArrayWithIDs(aryNewSelectedIDs);
		// Update the controller's stored IDs.
		Select_Internal(aryNewSelectedIDs);
		
		return true;
	} else {
		// They cancelled. Great. We don't need to revert anything internally. The datalist may need reverted to the old selections
		// but SyncControls will get that.

		return false;
	}
}

// (r.gonet 2016-01-22) - PLID 68041 - Programmatically selects All rows and sets the selection to < All >
void CMultiSelectComboController::OnSelectAll()
{
	// A selection of All means the selected IDs array is empty and we set the all flag. If we ever added a None option, 
	// I guess it would mean the array being empty and the All flag being false. In fact, I think this is what I did over
	// in the Charge Level Providers dialog, which I took a lot of this code from.
	m_arySelectedIDs.RemoveAll();
	m_bAllSelected = true;
}

// (r.gonet 2016-01-22) - PLID 68041 - Programmatically selects the default row.
void CMultiSelectComboController::OnSelectDefault()
{
	// Currently a thin wrapper on OnSelectAll()
	OnSelectAll();
}

// (r.gonet 2016-01-22) - PLID 68041 - Initializes the ID and text of a special row in the combo.
void CMultiSelectComboController::SetSpecialRowID(ESpecialRow eSpecialRow, _variant_t varID, CString strDisplayText)
{
	// You may think that we would care to check the variant type at this point and compare it with the datalist ID's type. 
	// However, we don't because this function may be called before the controller is bound to controls. We put the burden
	// on the caller to know the correct ID types to pass, which they should know anyway.
	switch (eSpecialRow) {
	case ESpecialRow::All:
		m_varAllRowID = varID;
		m_strAllRowDisplayText = strDisplayText;
		break;
	case ESpecialRow::Multiple:
		m_varMultipleRowID = varID;
		m_strMultipleRowDisplayText = strDisplayText;
		break;
	default:
		ThrowNxException("%s : Unhandled special row enum value.", __FUNCTION__);
	}
}

// (r.gonet 2016-01-22) - PLID 68041 - Gets the ID of a special row in the combo.
_variant_t CMultiSelectComboController::GetSpecialRowID(ESpecialRow eSpecialRow) const
{
	switch (eSpecialRow) {
	case ESpecialRow::All:
		return m_varAllRowID;
	case ESpecialRow::Multiple:
		return m_varMultipleRowID;
	default:
		ThrowNxException("%s : Unhandled special row enum value.", __FUNCTION__);
	}
}

// (r.gonet 2016-01-22) - PLID 68041 - Sets up the extra options for the CMultiSelectDlg. I figure child classes may want this different, 
// so its virtual.
void CMultiSelectComboController::SetMultiSelectDialogOptions(CMultiSelectDlg &dlg)
{
	dlg.m_pfnContextMenuProc = MultiSelectDlgDefaultContextMenuProc;
	dlg.m_nContextMenuProcParam = (LPARAM)nullptr;
	dlg.m_bPutSelectionsAtTop = TRUE;
}

// (r.gonet 2016-01-22) - PLID 68041 - Loads the combo's rows. The base class doesn't actually load any rows besides the special rows.
void CMultiSelectComboController::LoadComboRows()
{
	// Has no rows by default.
}

// (r.gonet 2016-01-22) - PLID 68041 - Loads the special rows into the datalist. Private because the base class should be the only
// class needs to know which rows are special and are not real rows.
void CMultiSelectComboController::LoadSpecialRows()
{
	AssertIsBound();

	// Add the < Multiple > row
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pCombo->GetNewRow();
	pRow->PutValue(m_nIDColumnIndex, m_varMultipleRowID);
	pRow->PutValue(m_nDescriptionColumnIndex, _variant_t(_bstr_t(m_strMultipleRowDisplayText)));
	m_pCombo->AddRowBefore(pRow, m_pCombo->GetFirstRow());

	// Add the < All > row
	pRow = m_pCombo->GetNewRow();
	pRow->PutValue(m_nIDColumnIndex, m_varAllRowID);
	pRow->PutValue(m_nDescriptionColumnIndex, _variant_t(_bstr_t(m_strAllRowDisplayText)));
	m_pCombo->AddRowBefore(pRow, m_pCombo->GetFirstRow());
}
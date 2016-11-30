#pragma once

// (r.gonet 2016-01-22) - PLID 68041 - Encapsulates the logic controlling the common pattern
// of using a datalist dropdown with a < Multiple > row that displays a popup and then replaces
// itself with a hyperlink with a comma separated list of the multiple selections.
class CMultiSelectComboController {
public:
	// (r.gonet 2016-01-22) - PLID 68041 - The special rows in the dropdown.
	enum class ESpecialRow {
		// < All >
		All,
		// < Multiple >
		Multiple,
	};
protected:
	// (r.gonet 2016-01-22) - PLID 68041 - Parent window of the bound controls.
	CWnd *m_pParent;
	// (r.gonet 2016-01-22) - PLID 68041 - The datalist combo box the controller is bound to.
	NXDATALIST2Lib::_DNxDataListPtr m_pCombo = nullptr;
	// (r.gonet 2016-01-22) - PLID 68041 - The resource ID of the datalist combo box the controller is bound to.
	UINT m_nComboControlID = -1;
	// (r.gonet 2016-01-22) - PLID 68041 - The hyperlink label the controller is bound to.
	CNxLabel *m_pMultiLabel = nullptr;
	// (r.gonet 2016-01-22) - PLID 68041 - Whether or not the controller has been bound to controls yet.
	bool m_bIsBound = false;
	bool m_bIsInitialized = false;

	// (r.gonet 2016-01-22) - PLID 68041 - The array of IDs that are currently selected. Special row IDs
	// will not be included in this array. If < Multiple > is selected, then this will be filled with
	// the ID of each row selected. If < All > is selected, then this will be empty.
	CVariantArray m_arySelectedIDs;
	// (r.gonet 2016-01-22) - PLID 68041 - Whether or not < All > is currently selected.
	bool m_bAllSelected = true;

	// (r.gonet 2016-01-22) - PLID 68041 - The datalist column index for the ID column.
	short m_nIDColumnIndex = 0;
	// (r.gonet 2016-01-22) - PLID 68041 - the datalist column index for the Description column.
	short m_nDescriptionColumnIndex = 1;

	// (r.gonet 2016-01-22) - PLID 68041 - The type of the IDs being stored in the datalist ID column.
	VARTYPE m_vtIDType = VT_I4;
	// (r.gonet 2016-01-22) - PLID 68041 - The ID of the < All > special row.
	_variant_t m_varAllRowID = _variant_t(-1L, VT_I4);
	// (r.gonet 2016-01-22) - PLID 68041 - The text of the < All > special row.
	CString m_strAllRowDisplayText = " < All >";
	// (r.gonet 2016-01-22) - PLID 68041 - The ID of the < Multiple > special row.
	_variant_t m_varMultipleRowID = _variant_t(-2L, VT_I4);
	// (r.gonet 2016-01-22) - PLID 68041 - The text of the < Multiple > special row.
	CString m_strMultipleRowDisplayText = " < Multiple >";

	// (r.gonet 2016-01-22) - PLID 68041 - The name of the things being stored in this combo.
	// This is really dumb and I'm going to hide it from the caller. The element name is actually
	// to be used with the CMultiSelectDlg as a suffix on a ConfigRT property name.
	CString m_strElementName;
	// (r.gonet 2016-01-22) - PLID 68041 - The text displayed in the header of the CMultiSelectDlg.
	CString m_strMultiSelectPromptDescription = "Please make one or more selections:";

	// (r.gonet 2016-01-22) - PLID 68041 - If true, then selections are auto-remembered and also loaded
	// upon Initialize(). If false, then selections are not remembered. 
	bool m_bRememberSelections = false;
	// (r.gonet 2016-01-22) - PLID 68041 - The name of the ConfigRT text property storing the remembered selection IDs.
	CString m_strRememberedIDsPropertyName;
	// (r.gonet 2016-01-22) - PLID 68041 - Whether or not the combo and hyperlink should be visible at all.
	// This is necessary so that calls to SyncControls() do not violate control hiding.
	bool m_bVisible = true;
	long m_nMaxLabelCharLength = 255;
public:
	// (r.gonet 2016-01-22) - PLID 68041 - Creates a new CMultiSelectComboController with remembering disabled.
	// - pParent: The parent window.
	// - strElementName: What the elements being stored in the combo are called. i.e. Providers.
	// - strMultiSelectPromptDescription: The text to put at the top of the multi-select dialog.
	CMultiSelectComboController(CWnd *pParent, CString strElementName,
		CString strMultiSelectPromptDescription);
	// (r.gonet 2016-01-22) - PLID 68041 - Creates a new CMultiSelectComboController with remembering enabled.
	// - pParent: The parent window.
	// - strElementName: What the elements being stored in the combo are called. i.e. Providers.
	// - strMultiSelectPromptDescription: The text to put at the top of the multi-select dialog.
	// - strRememberedIDsPropertyName: The name of the text property to store the selected IDs.
	CMultiSelectComboController(CWnd *pParent, CString strElementName,
		CString strMultiSelectPromptDescription, CString strRememberedIDsPropertyName);

	// (r.gonet 2016-01-22) - PLID 68041 - Binds controls with the controller. Needs to be called first.
	void BindController(CNxLabel &nxlMultiLabel, NXDATALIST2Lib::_DNxDataListPtr pCombo, UINT nComboControlID, bool bAutoInitialize,
		short nIDColumnIndex = 0, short nDescriptionColumnIndex = 1);
	// (r.gonet 2016-01-22) - PLID 68041 - Initializes the combo box, loads its rows, and remembers previous selections
	// if set to remember. Needs to be called after BindController.
	void InitializeCombo(bool bRemoveExistingRows);

	// (r.gonet 2016-01-22) - PLID 68041 - Turns on auto-remembering of selections. Default is Off.
	// - strSavedSelectedIDsPropertyName: The name of the ConfigRT text property to be used to store
	//   the selection IDs.
	inline void EnableRemembering(CString strRememberedIDsPropertyName)
	{
		m_bRememberSelections = true;
		m_strRememberedIDsPropertyName = strRememberedIDsPropertyName;
	}

	// (r.gonet 2016-01-22) - PLID 68041 - Turns off auto-remembering of selections. Default is Off.
	inline void DisableRemembering()
	{
		m_bRememberSelections = false;
		m_strRememberedIDsPropertyName = "";
	}

	// (r.gonet 2016-01-22) - PLID 68041 - Gets whether selections are remembered.
	inline bool IsRememberingEnabled() const
	{
		return m_bRememberSelections;
	}

	// (r.gonet 2016-01-22) - PLID 68041 - Validates the selections and synchronizes the controls with the selections.
	void SyncControls();

	// (r.gonet 2016-01-22) - PLID 68041 - Sets whether all bound controls are visible.
	void SetVisible(bool bVisible);
	// (r.gonet 2016-01-22) - PLID 68041 - Gets whether all bound controls are visible.
	inline bool IsVisible() const
	{
		return m_bVisible;
	}

	// (r.gonet 2016-01-22) - PLID 68041 - Programmatically selects a row by ID.
	bool Select(_variant_t varID);
	// (r.gonet 2016-01-22) - PLID 68041 - Programmatically selects multiple rows by ID.
	void Select(CVariantArray &aryIDs);
	// (r.gonet 2016-01-22) - PLID 68041 - Programmatically selects All rows and sets the selection to < All >
	void SelectAll();

	// (r.gonet 2016-01-22) - PLID 68041 - Handles when the user manually selects a row. Should be called in the datalist
	// OnSelChosen or OnSelChanged handler.
	// Returns true if a selection was made and false if it was cancelled.
	bool HandleSelection();
	// (r.gonet 2016-01-22) - PLID 68041 - Handles when the user manually selects the < Multiple > row. Launches the CMultiSelectDlg popup.
	// Public because external classes need to call this when handling OnLabelClick.
	// Returns true if a selection was made and false if it was cancelled.
	bool DoMultiSelect();

	// (r.gonet 2016-01-22) - PLID 68041 - Initializes the ID and text of a special row in the combo.
	void SetSpecialRowID(ESpecialRow eSpecialRow, _variant_t varID, CString strDisplayText);
	// (r.gonet 2016-01-22) - PLID 68041 - Gets the ID of a special row in the combo.
	_variant_t GetSpecialRowID(ESpecialRow eSpecialRow) const;

	// (r.gonet 2016-01-22) - PLID 68041 - Gets a reference to the array containing the selected IDs.
	inline const CVariantArray& GetSelectedIDs() const
	{
		return m_arySelectedIDs;
	}

	// (r.gonet 2016-01-22) - PLID 68041 - Gets whether or not all rows are selected, i.e. < All > is selected.
	inline bool IsAllSelected() const
	{
		return m_bAllSelected;
	}
protected:
	// (r.gonet 2016-01-22) - PLID 68041 - Checks and throws an exception if the controller is not bound to controls.
	void AssertIsBound();
	// (r.gonet 2016-01-22) - PLID 68041 - Checks and throws an exception if the controller is not initialized
	void AssertIsInitialized();
	// (r.gonet 2016-01-22) - PLID 68041 - Sets up the extra options for the CMultiSelectDlg. I figure callers may want this different, so its virtual.
	virtual void SetMultiSelectDialogOptions(CMultiSelectDlg &dlg);
	// (r.gonet 2016-01-22) - PLID 68041 - Loads the combo's rows. The base class doesn't actually load any rows besides the special rows.
	virtual void LoadComboRows();
private:
	// (r.gonet 2016-01-22) - PLID 68041 - Loads the special rows into the datalist. Private because the base class should be the only
	// class needs to know which rows are special and are not real rows.
	void LoadSpecialRows();
	// (r.gonet 2016-01-22) - PLID 68041 - Loads the remembered selections from ConfigRT. If those selections
	// are no longer available in the datalist, then those selections are not re-selected. If loading fails
	// to find any rows in the datalist that are remembered, then < All > is selected. If loading finds all
	// remembered selections constitute all available selections, then < All > is selected.
	void LoadRememberedSelections();
	// (r.gonet 2016-01-22) - PLID 68041 - Saves the current selections to ConfigRT.
	void SaveSelections();
	// (r.gonet 2016-01-23) - PLID 68041 - Returns true if the controller state is such that the default row should be selected, even
	// if it isn't currently.
	bool ShouldDefaultRowBeSelected();
	// (r.gonet 2016-01-23) - PLID 68041 - Returns true if the controller state is such that the All row should be selected, even
	// if it isn't currently.
	bool ShouldAllRowBeSelected();
	// (r.gonet 2016-01-22) - PLID 68041 - Updates the controller's stored selections after the a single, non-special row is selected from the combo.
	void OnSelectSingle();
	// (r.gonet 2016-01-22) - PLID 68041 - Launches a multi select dialog and then updates the controller's stored selections after the <Multiple> row is selected from the combo.
	bool OnSelectMultiple();
	// (r.gonet 2016-01-22) - PLID 68041 - Updates the controller's stored selections after the <All> row is selected from the combo.
	void OnSelectAll();
	// (r.gonet 2016-01-22) - PLID 68041 - Updates the controller's stored selections after the default row is selected from the combo.
	void OnSelectDefault();

	// (r.gonet 2016-01-22) - PLID 68041 - Programmatically selects a row by ID.
	bool Select_Internal(_variant_t varID);
	// (r.gonet 2016-01-22) - PLID 68041 - Programmatically selects multiple rows by ID.
	void Select_Internal(CVariantArray &aryIDs);
	// (r.gonet 2016-01-22) - PLID 68041 - Programmatically selects All rows and sets the selection to < All >
	void SelectAll_Internal();
	void SelectDefault_Internal();

	// (r.gonet 2016-01-22) - PLID 68041 - Updates the on-screen controls to reflect the selections
	// stored in the controller. Shows and hides the combo box or hyperlink label as appropriate.
	void UpdateControls();
	// (r.gonet 2016-01-22) - PLID 68041 - Updates the selections the controller knows of from the combo box since the parent 
	// may change the number of rows in the combo box.
	void ValidateSelections();
};
// (r.gonet 07/03/2014) - PLID 62533 - Dialog that lets the user add, edit, and delete custom bill statuses.
// BillStatusSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "BillStatusSetupDlg.h"
#include "afxdialogex.h"
#include "GlobalFinancialUtils.h"

using namespace ADODB;

// CBillStatusSetupDlg dialog

IMPLEMENT_DYNAMIC(CBillStatusSetupDlg, CNxDialog)

// (r.gonet 07/03/2014) - PLID 62533 - Added.
/// <summary>
/// Initializes a new instance of the CBillStatusSetupDlg class.
/// </summary>
/// <param name="pParent">The parent window.</param>
/// <param name="dwBackgroundColor">Color of the background nxcolor.</param>
CBillStatusSetupDlg::CBillStatusSetupDlg(CWnd* pParent, DWORD dwBackgroundColor)
	: CNxDialog(CBillStatusSetupDlg::IDD, pParent)
{
	m_dwBackgroundColor = dwBackgroundColor;
}

// (r.gonet 07/03/2014) - PLID 62533 - Added.
/// <summary>
/// Finalizes an instance of the CBillStatusSetupDlg class.
/// </summary>
CBillStatusSetupDlg::~CBillStatusSetupDlg()
{
}

void CBillStatusSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BSS_NXCOLOR, m_nxcolorBackground);
	DDX_Control(pDX, IDC_BSS_HEADER_STATIC, m_nxstaticHeader);
	DDX_Control(pDX, IDC_BSS_ADD_BTN, m_btnAdd);
	DDX_Control(pDX, IDC_BSS_DELETE_BTN, m_btnDelete);
	DDX_Control(pDX, IDOK, m_btnClose);
}


BEGIN_MESSAGE_MAP(CBillStatusSetupDlg, CNxDialog)
	ON_BN_CLICKED(IDC_BSS_ADD_BTN, &CBillStatusSetupDlg::OnBnClickedAddBtn)
	ON_BN_CLICKED(IDC_BSS_DELETE_BTN, &CBillStatusSetupDlg::OnBnClickedDeleteBtn)
END_MESSAGE_MAP()


// CBillStatusSetupDlg message handlers


// (r.gonet 07/03/2014) - PLID 62533 - Added.
/// <summary>
/// Dialog initialization.
/// </summary>
BOOL CBillStatusSetupDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try {
		m_nxcolorBackground.SetColor(m_dwBackgroundColor);
		m_btnAdd.AutoSet(NXB_NEW);
		m_btnDelete.AutoSet(NXB_DELETE);
		m_btnClose.AutoSet(NXB_CLOSE);

		m_pBillStatusList = BindNxDataList2Ctrl(IDC_BSS_STATUS_LIST);
		// Disable the delete button because it is row contextual.
		EnsureControls();
	} NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

// (r.gonet 07/03/2014) - PLID 62533 - Added.
/// <summary>
/// Called when the Add button is clicked. Adds a new bill status row and starts the user typing the name of the status out.
/// </summary>
void CBillStatusSetupDlg::OnBnClickedAddBtn()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pBillStatusList->GetNewRow();
		// -1 initially. Once the user finishes editing the status name, then we'll save it to the database.
		pRow->PutValue((short)EBillStatusListColumns::ID, (long)-1);
		pRow->PutValue((short)EBillStatusListColumns::Name, _bstr_t(""));
		pRow->PutValue((short)EBillStatusListColumns::OnHold, g_cvarFalse);
		pRow->PutValue((short)EBillStatusListColumns::Inactive, g_cvarFalse);
		pRow->PutValue((short)EBillStatusListColumns::Custom, g_cvarTrue);
		m_pBillStatusList->AddRowAtEnd(pRow, NULL);
		m_pBillStatusList->CurSel = pRow;
		EnsureControls();
		m_pBillStatusList->EnsureRowInView(pRow);
		m_pBillStatusList->StartEditing(pRow, (short)EBillStatusListColumns::Name);
	} NxCatchAll(__FUNCTION__);
}


// (r.gonet 07/03/2014) - PLID 62533 - Added.
/// <summary>
/// Called when the Delete button is clicked. Tries to remove the current row and delete it from the database, if it is not in use.
/// </summary>
void CBillStatusSetupDlg::OnBnClickedDeleteBtn()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pBillStatusList->CurSel;
		if (!pRow) {
			// Somehow the delete button is enabled when there is no row selected. This should be impossible if EnsureControls() is being called.
			ASSERT(FALSE);
			MessageBox("Please select a row before clicking Delete.", "Error", MB_ICONERROR | MB_OK);
			return;
		}
		if (!IsCustomStatus(pRow)) {
			// Somehow the delete button is enabled when the current row is a non-custom row. This should be impossible if EnsureControls() is being called.
			ASSERT(FALSE);
			MessageBox("You cannot delete non-custom statuses.", "Error", MB_ICONERROR | MB_OK);
			return;
		}

		long nID = VarLong(pRow->GetValue((short)EBillStatusListColumns::ID));
		CString strName = VarString(pRow->GetValue((short)EBillStatusListColumns::Name), "");
		
		// If this status is in use, we cannot delete it.
		if (IsInUse(nID)) {
			MessageBox(FormatString("The \"%s\" status is in use by at least one existing bill and cannot be deleted. If you wish, you may inactivate the status instead.", strName), "Status in Use", MB_ICONERROR | MB_OK);
			return;
		} else {
			// Confirm they really want to delete the status.
			int nDialogResult = MessageBox(FormatString("You have selected to delete the \"%s\" status. You will not be able to use this status on any bill and it will be removed from your list of statuses unless it is manually re-added. Are you sure you wish to do this?", strName), NULL, MB_ICONWARNING | MB_YESNO);
			if (nDialogResult != IDYES) {
				// They didn't really want to do this action afterall.
				return;
			} else {
				// Let's continue with deletion.
			}
		}

		// Do the deletion.
		// Deleted bills with this status do not factor into the Bill Status being In Use, so null delete bill statuses out so we don't get a FK violation.
		ExecuteParamSql(
			"DECLARE @BillStatusID INT SET @BillStatusID = {INT}; "
			"UPDATE BillsT SET BillsT.StatusID = NULL WHERE BillsT.Deleted = 1 AND StatusID = @BillStatusID; "
			"DELETE FROM BillStatusT WHERE ID = @BillStatusID; "
			, nID);
		// Reflect the deletion in the datalist.
		m_pBillStatusList->RemoveRow(pRow);
		EnsureControls();
		// Send a table checker so the bill dialogs can refresh their status combos.
		CClient::RefreshTable(NetUtils::BillStatusT, nID);
	} NxCatchAll(__FUNCTION__);
}

BEGIN_EVENTSINK_MAP(CBillStatusSetupDlg, CNxDialog)
ON_EVENT(CBillStatusSetupDlg, IDC_BSS_STATUS_LIST, 18, CBillStatusSetupDlg::RequeryFinishedBscStatusList, VTS_I2)
ON_EVENT(CBillStatusSetupDlg, IDC_BSS_STATUS_LIST, 9, CBillStatusSetupDlg::EditingFinishingBssStatusList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
ON_EVENT(CBillStatusSetupDlg, IDC_BSS_STATUS_LIST, 10, CBillStatusSetupDlg::EditingFinishedBssStatusList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
ON_EVENT(CBillStatusSetupDlg, IDC_BSS_STATUS_LIST, 2, CBillStatusSetupDlg::SelChangedBssStatusList, VTS_DISPATCH VTS_DISPATCH)
END_EVENTSINK_MAP()

// (r.gonet 07/03/2014) - PLID 62533 - Added.
/// <summary>
/// Called when the status list has finished requerying. Makes sure that the non-custom statuses are not editable.
/// </summary>
/// <param name="nFlags">The reason why the requerying finished.</param>
void CBillStatusSetupDlg::RequeryFinishedBscStatusList(short nFlags)
{
	try {
		// Create some cell format overrides so we can make the Name, On Hold, and Inactive columns non-editable.
		NXDATALIST2Lib::IFormatSettingsPtr pNameReadOnlyFormat(__uuidof(NXDATALIST2Lib::FormatSettings));
		pNameReadOnlyFormat->Editable = VARIANT_FALSE;
		pNameReadOnlyFormat->DataType = VT_BSTR;
		pNameReadOnlyFormat->FieldType = NXDATALIST2Lib::cftTextSingleLine;
		NXDATALIST2Lib::IFormatSettingsPtr pCheckboxReadOnlyFormat(__uuidof(NXDATALIST2Lib::FormatSettings));
		pCheckboxReadOnlyFormat->Editable = VARIANT_FALSE;
		pCheckboxReadOnlyFormat->DataType = VT_BOOL;
		pCheckboxReadOnlyFormat->FieldType = NXDATALIST2Lib::cftBoolCheckbox;

		// Find the non-custom fields and apply the cell format overrides.
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pBillStatusList->GetFirstRow();
		while (pRow) {
			if (!IsCustomStatus(pRow)) {
				// Non-custom fields should look like the background so as to look non-mutable.
				pRow->BackColor = m_dwBackgroundColor;

				pRow->CellFormatOverride[(short)EBillStatusListColumns::Name] = pNameReadOnlyFormat;
				pRow->CellFormatOverride[(short)EBillStatusListColumns::OnHold] = pCheckboxReadOnlyFormat;
				pRow->CellFormatOverride[(short)EBillStatusListColumns::Inactive] = pCheckboxReadOnlyFormat;
				// Don't taunt the user with the ability to inactivate when they really can't.
				pRow->PutValue((short)EBillStatusListColumns::Inactive, g_cvarNull);
			}
			pRow = pRow->GetNextRow();
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 07/03/2014) - PLID 62533 - Added.
/// <summary>
/// Called when the user is just finishing editing a cell in the Status List but before their edits are committed. Ensures they are properly warned.
/// </summary>
/// <param name="lpRow">The row of the cell being edited.</param>
/// <param name="nCol">The column index of the cell being edited.</param>
/// <param name="varOldValue">The old value of the cell before editing.</param>
/// <param name="strUserEntered">The string the user typed.</param>
/// <param name="pvarNewValue">A pointer to the new value of the cell.</param>
/// <param name="pbCommit">A pointer to a boolean flag to commit this new value.</param>
/// <param name="pbContinue">A pointer to a boolean flag to continue committing or force the user to keep editing.</param>
void CBillStatusSetupDlg::EditingFinishingBssStatusList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (!IsCustomStatus(pRow)) {
			// How did they even get here? There are multiple checks in place to prevent editing a non-custom status.
			::VariantClear(pvarNewValue);
			::VariantCopy(pvarNewValue, &varOldValue);
			*pbCommit = FALSE;
			return;
		}

		long nID = VarLong(pRow->GetValue((short)EBillStatusListColumns::ID), -1);
		CString strName = VarString(pRow->GetValue((short)EBillStatusListColumns::Name));
		// We don't allow certain edits if the status is in use.
		bool bInUse = IsInUse(nID);

		switch (nCol) {
		case (short)EBillStatusListColumns::Name:
		{
			// Validate the name is non-empty.
			CString str = strUserEntered;

			str.TrimLeft();
			str.TrimRight();
			
			if (str.GetLength() == 0) {
				// Don't let them have a blank status name.
				if (nID != -1) {
					// Saved. So tell them they cannot do this.
					MessageBox("Bill status names cannot not be blank.", "Blank Bill Status Name", MB_ICONERROR | MB_OK);
					::VariantClear(pvarNewValue);
					::VariantCopy(pvarNewValue, &varOldValue);
				}
				*pbCommit = FALSE;
				return;
			}

			// Don't allow duplicate names
			// (j.jones 2014-07-30 08:41) - PLID 62947 - ensured this is a case-insensitive check
			if (ReturnsRecordsParam("SELECT NULL FROM BillStatusT WHERE Name LIKE {STRING} AND ID <> {INT}"
				, str, nID)) {
				MessageBox("There is already a bill status with this name.", "Duplicate Bill Status Name", MB_ICONERROR | MB_OK);
				*pbContinue = FALSE;
				return;
			}
			// Intentionally falls through to next case.
		}
		case (short)EBillStatusListColumns::OnHold:
		{
			if (bInUse) {
				// Don't let them change the name or on hold status for statuses belonging to existing bills, since that could change the meaning of the status on legacy data.
				MessageBox(FormatString("The \"%s\" status is in use by at least one existing bill and cannot be changed. If you wish, you may inactivate the status instead and add a new one.", strName), "Status in Use", MB_ICONWARNING | MB_OK);
				::VariantClear(pvarNewValue);
				::VariantCopy(pvarNewValue, &varOldValue);
				*pbCommit = FALSE;
			}
			break;
		}
		case (short)EBillStatusListColumns::Inactive:
			// If they are inactivating an active status, then warn them first.
			if (!VarBool(varOldValue, FALSE) && pvarNewValue != NULL && VarBool(*pvarNewValue, FALSE)) {
				int nDialogResult = MessageBox(FormatString("You selected to inactivate the \"%s\" status. You will not be able to use this as a status on any bill unless the status is reactivated. Are you sure you wish to do this?",
					strName), NULL, MB_ICONWARNING | MB_YESNO);
				if (nDialogResult != IDYES) {
					// They backed out.
					::VariantClear(pvarNewValue);
					::VariantCopy(pvarNewValue, &varOldValue);
					*pbCommit = FALSE;
				}
			}
			break;
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 07/03/2014) - PLID 62533 - Added.
/// <summary>
/// Called after the client has finished editing a datalist cell. Ensures the edit is committed to the database.
/// </summary>
/// <param name="lpRow">The row of the cell being edited.</param>
/// <param name="nCol">The column index of the cell being edited.</param>
/// <param name="varOldValue">The old value before editing.</param>
/// <param name="varNewValue">The new value after editing.</param>
/// <param name="bCommit">Boolean flag telling if the new value has been committed to the datalist or not.</param>
void CBillStatusSetupDlg::EditingFinishedBssStatusList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (!pRow) {
			// How did they even get here?
			return;
		}

		// We're just going to save everything at once. No need to split cases.
		long nID = VarLong(pRow->GetValue((short)EBillStatusListColumns::ID), -1);
		CString strName = VarString(pRow->GetValue((short)EBillStatusListColumns::Name));
		BOOL bOnHold = VarBool(pRow->GetValue((short)EBillStatusListColumns::OnHold));
		BOOL bInactive = VarBool(pRow->GetValue((short)EBillStatusListColumns::Inactive), FALSE);
		BOOL bCustom = VarBool(pRow->GetValue((short)EBillStatusListColumns::Custom));
		if (nID == -1) {
			// An unsaved status.
			if (!bCommit) {
				// The edit was aborted. We don't want unsaved rows in our list. Remove the row.
				m_pBillStatusList->RemoveRow(pRow);
				EnsureControls();
				return;
			}

			// Unsaved, so save it.
			_RecordsetPtr prs = CreateParamRecordset(
				"SET NOCOUNT ON; "
				"INSERT INTO BillStatusT (Name, Type, Inactive, Custom) VALUES ({STRING}, {VT_I4}, {BIT}, {BIT}); "
				"SET NOCOUNT OFF; "
				"SELECT CONVERT(INT, SCOPE_IDENTITY()) AS NewID; "
				, strName, (bOnHold ? _variant_t((long)EBillStatusType::OnHold, VT_I4) : g_cvarNull), bInactive, bCustom);
			if (!prs->eof) {
				nID = AdoFldLong(prs->Fields, "NewID");
				pRow->PutValue((short)EBillStatusListColumns::ID, nID);
			} else {
				// Something bad happened in SQL. Don't leave an unsaved row.
				m_pBillStatusList->RemoveRow(pRow);
				EnsureControls();
				ThrowNxException("%s : Could not save new bill status.", __FUNCTION__);
			}
			// The new status is at the end of the list right now, so make it go into its proper place by sorting the list.
			m_pBillStatusList->Sort();
			m_pBillStatusList->EnsureRowInView(pRow);
		} else {
			if (!bCommit) {
				// The edit was aborted. Don't run any query.
				return;
			}
			// Update the Bill Status record.
			ExecuteParamSql(
				"UPDATE BillStatusT SET "
				"	Name = {STRING}, "
				"	Type = {VT_I4}, "
				"	Inactive = {BIT} "
				"WHERE ID = {INT}; "
				, strName, (bOnHold ? _variant_t((long)EBillStatusType::OnHold, VT_I4) : g_cvarNull), bInactive, nID);
		}

		EnsureControls();
		// Send a table checker to notify the bill dialogs to refresh their status combo.
		CClient::RefreshTable(NetUtils::BillStatusT, nID);
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 07/03/2014) - PLID 62533 - Added.
/// <summary>
/// Called when the user has selected a row from the datalist.
/// </summary>
/// <param name="lpOldSel">The old selected row.</param>
/// <param name="lpNewSel">The new selected row.</param>
void CBillStatusSetupDlg::SelChangedBssStatusList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try {
		// Make sure the delete button is in a valid state.
		EnsureControls();
	} NxCatchAll(__FUNCTION__);
}


// (r.gonet 07/03/2014) - PLID 62533 - Added.
/// <summary>
/// Determines whether the status datalist row is a custom status.
/// </summary>
/// <param name="pRow">The row to check.</param>
/// <returns>true if the row is a custom status. false if it is a non-custom status.</returns>
bool CBillStatusSetupDlg::IsCustomStatus(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	if (!pRow) {
		// Umm..
		return false;
	}

	if (VarBool(pRow->GetValue((short)EBillStatusListColumns::Custom))) {
		return true;
	} else {
		return false;
	}
}

// (r.gonet 07/03/2014) - PLID 62533 - Added.
/// <summary>
/// Determines whether a status is in use by some non-deleted bill. Deleted bills with the status don't count.
/// </summary>
/// <param name="nID">The ID of the status to check.</param>
/// <returns>true if the status is in use by a non-deleted bill. false if it isn't.</returns>
bool CBillStatusSetupDlg::IsInUse(long nID)
{
	if (nID == -1) {
		// This is an unsaved row, so no, it is impossible for it to be in use already.
		return false;
	}
	// Check for any non-deleted bills.
	_RecordsetPtr prs = CreateParamRecordset(
		"SELECT TOP 1 NULL "
		"FROM BillsT "
		"WHERE BillsT.StatusID = {INT} "
		"AND BillsT.Deleted = 0; "
		, nID);
	if (!prs->eof) {
		return true;
	} else {
		return false;
	}
}

// (r.gonet 07/03/2014) - PLID 62533 - Added.
/// <summary>
/// Ensures the controls are in valid states given the current context.
/// </summary>
void CBillStatusSetupDlg::EnsureControls()
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pBillStatusList->CurSel;
	if (!pRow || !IsCustomStatus(pRow)) {
		// If no status row is selected or the status is not a custom status, then we can't delete.
		m_btnDelete.EnableWindow(FALSE);
	} else {
		m_btnDelete.EnableWindow(TRUE);
	}
}
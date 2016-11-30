// (r.gonet 08/05/2014) - PLID 63099 - Initial implementation
// EditBillTestCodesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "EditBillTestCodesDlg.h"
#include "afxdialogex.h"
#include "BillingRc.h"
#include "AuditTrail.h"

// (r.gonet 08/05/2014) - PLID 63099 - Gray color for the background of inactive test codes.
#define COLOR_GRAYED_OUT RGB(192, 192, 192)

// CEditBillTestCodesDlg dialog

IMPLEMENT_DYNAMIC(CEditBillTestCodesDlg, CNxDialog)

// (r.gonet 08/05/2014) - PLID 63099 - Constructs a new CEditBillTestCodesDlg.
// (r.gonet 08/06/2014) - PLID 63102 - Added vecPreSelectedTestCodeIDs
// - pParent: Parent window.
// - vecPreSelectedTestCodeIDs: IDs of Test codes to select automatically when the window is shown.
CEditBillTestCodesDlg::CEditBillTestCodesDlg(CWnd* pParent, std::vector<long> &vecPreSelectedTestCodeIDs)
: CNxDialog(CEditBillTestCodesDlg::IDD, pParent)
{
	// (r.gonet 08/05/2014) - PLID 63102 - Store them so we can select them in OnInitDialog.
	m_vecPreSelectedTestCodeIDs.assign(vecPreSelectedTestCodeIDs.begin(), vecPreSelectedTestCodeIDs.end());
}

CEditBillTestCodesDlg::~CEditBillTestCodesDlg()
{
}

void CEditBillTestCodesDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_TEST_CODES_BACKGROUND_NXCOLOR, m_nxcolorBackground);
	DDX_Control(pDX, IDC_EDIT_TEST_CODES_ADD_BTN, m_btnAdd);
	DDX_Control(pDX, IDC_EDIT_TEST_CODES_DELETE_BTN, m_btnDelete);
	DDX_Control(pDX, IDC_EDIT_TEST_CODES_HIDE_INACTIVE_CHECK, m_checkHideInactive);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
}


BEGIN_MESSAGE_MAP(CEditBillTestCodesDlg, CNxDialog)
	ON_BN_CLICKED(IDC_EDIT_TEST_CODES_HIDE_INACTIVE_CHECK, &CEditBillTestCodesDlg::OnBnClickedEditTestCodesHideInactiveCheck)
	ON_BN_CLICKED(IDOK, &CEditBillTestCodesDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_EDIT_TEST_CODES_DELETE_BTN, &CEditBillTestCodesDlg::OnBnClickedEditTestCodesDeleteBtn)
	ON_BN_CLICKED(IDC_EDIT_TEST_CODES_ADD_BTN, &CEditBillTestCodesDlg::OnBnClickedEditTestCodesAddBtn)
END_MESSAGE_MAP()


// CEditBillTestCodesDlg message handlers

BOOL CEditBillTestCodesDlg::OnInitDialog()
{
	try {
		CNxDialog::OnInitDialog();

		// (r.gonet 08/05/2014) - PLID 63100 - Cache the properties used.
		g_propManager.CachePropertiesInBulk("CEditBillTestCodesDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'EditBillTestCodesDlg_HideInactiveTestCodes'"
			")",
			_Q(GetCurrentUserName()));

		m_btnAdd.AutoSet(NXB_NEW);
		m_btnDelete.AutoSet(NXB_DELETE);
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_pTestCodeList = BindNxDataList2Ctrl(IDC_EDIT_TEST_CODES_LIST, true);
		
		// (r.gonet 08/05/2014) - PLID 63102 - Pre-select the test codes that the caller has told us to pre-select.
		for each(long nID in m_vecPreSelectedTestCodeIDs)
		{
			// (r.gonet 08/05/2014) - PLID 63102 - FindByColumn ensures the rows are loaded. No need to WaitForRequery yet.
			NXDATALIST2Lib::IRowSettingsPtr pPreSelectedRow = m_pTestCodeList->FindByColumn((short)ETestCodeListColumns::ID, nID, m_pTestCodeList->GetFirstRow(), VARIANT_FALSE);
			pPreSelectedRow->PutValue((short)ETestCodeListColumns::Selected, g_cvarTrue);
		}
		// (r.gonet 08/05/2014) - PLID 63102 - Ensure the selected test codes appear on top.
		m_pTestCodeList->Sort();

		// (r.gonet 08/05/2014) - PLID 63100 - See if the user wants us to hide the inactive test codes. We do by default.
		bool bHideInactiveTestCodes = GetRemotePropertyInt("EditBillTestCodesDlg_HideInactiveTestCodes", TRUE, 0, GetCurrentUserName(), true) ? true : false;
		m_checkHideInactive.SetCheck(bHideInactiveTestCodes ? BST_CHECKED : BST_UNCHECKED);
		// (r.gonet 08/05/2014) - PLID 63100 - Need to ensure that all the rows are loaded before we iterate them.
		m_pTestCodeList->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);

		// (r.gonet 08/05/2014) - PLID 63100 - Color the inactive codes gray.
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pTestCodeList->FindAbsoluteFirstRow(VARIANT_FALSE);
		while (pRow) {
			bool bInactive = VarBool(pRow->GetValue((short)ETestCodeListColumns::Inactive), FALSE) ? true : false;
			if (bInactive) {
				pRow->BackColor = COLOR_GRAYED_OUT;
			}
			pRow = m_pTestCodeList->FindAbsoluteNextRow(pRow, VARIANT_FALSE);
		}
		// (r.gonet 08/05/2014) - PLID 63100 - Show or hide the inactive test codes depending on the user's preference.
		ShowInactiveTestCodes(!bHideInactiveTestCodes);
		// (r.gonet 08/05/2014) - PLID 63099 - Ensure that the delete button is in a valid enabled state.
		EnsureControls();
		
	} NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

// (r.gonet 08/05/2014) - PLID 63099 - Ensures the controls are in valid states.
void CEditBillTestCodesDlg::EnsureControls()
{
	// (r.gonet 08/05/2014) - PLID 63099 - If we have a row, then the delete button should be enabled.
	if (m_pTestCodeList->CurSel) {
		m_btnDelete.EnableWindow(TRUE);
	} else {
		m_btnDelete.EnableWindow(FALSE);
	}
}

// (r.gonet 08/05/2014) - PLID 63100 - Shows or hides the inactive test code rows. 
// (r.gonet 08/06/2014) - PLID 63102 - Does not hide selected inactive rows.
// - bShow: true to show the inactive test codes. false to hide the inactive test codes.
void CEditBillTestCodesDlg::ShowInactiveTestCodes(bool bShow)
{
	// (r.gonet 08/05/2014) - PLID 63100 - Loop and show/hide inactive test codes. 
	// (r.gonet 08/06/2014) - PLID 63102 - But only if they are not selected.
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pTestCodeList->FindAbsoluteFirstRow(VARIANT_FALSE);
	while (pRow) {
		// (r.gonet 08/06/2014) - PLID 63102 - Is this row selected?
		bool bSelected = VarBool(pRow->GetValue((short)ETestCodeListColumns::Selected), FALSE) ? true : false;
		bool bInactive = VarBool(pRow->GetValue((short)ETestCodeListColumns::Inactive), FALSE) ? true : false;
		// (r.gonet 08/06/2014) - PLID 63102 - Don't hide selected rows.
		if (!bShow && bInactive && !bSelected) {
			pRow->Visible = VARIANT_FALSE;
		} else {
			pRow->Visible = VARIANT_TRUE;
		}
		pRow = m_pTestCodeList->FindAbsoluteNextRow(pRow, VARIANT_FALSE);
	}
}

// (r.gonet 08/05/2014) - PLID 63100 - Handles the event when the user checks the hide inactive checkbox.
void CEditBillTestCodesDlg::OnBnClickedEditTestCodesHideInactiveCheck()
{
	try {
		// (r.gonet 08/05/2014) - PLID 63100 - Remember the user's preference
		bool bHideInactive = m_checkHideInactive.GetCheck() == BST_CHECKED;
		SetRemotePropertyInt("EditBillTestCodesDlg_HideInactiveTestCodes", bHideInactive ? TRUE : FALSE, 0, GetCurrentUserName());
		// (r.gonet 08/05/2014) - PLID 63100 - Show or hide the inactive test codes.
		ShowInactiveTestCodes(!bHideInactive);
		// (r.gonet 08/05/2014) - PLID 63100 - The selected row may have disappeared, so reflect that with the delete button.
		EnsureControls();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 08/05/2014) - PLID 63099 - Handles the event when the user clicks the OK button. Saves the test code
// changes to the database.
void CEditBillTestCodesDlg::OnBnClickedOk()
{
	long nAuditTransactionID = -1;
	try {
		CParamSqlBatch sqlSaveBatch;

		// (r.gonet 08/05/2014) - PLID 63099 - Create a temp table that will record all the changes so we can audit them in batch later.
		// (r.gonet 08/05/2014) - PLID 63100 - Added OldInactive NewInactive
		sqlSaveBatch.Add(R"(
SET NOCOUNT ON;
DECLARE @ChangedBillTestCodesT TABLE
(
	TempID INT NULL,
	ID INT NOT NULL,
	OldCode NVARCHAR(20) NULL,
	NewCode NVARCHAR(20) NULL,
	OldDescription NVARCHAR(255) NULL,
	NewDescription NVARCHAR(255) NULL,
	OldInactive BIT NULL,
	NewInactive BIT NULL,
	Inserted BIT NOT NULL DEFAULT(0), Updated BIT NOT NULL DEFAULT(0), Deleted BIT NOT NULl DEFAULT(0)
);
)");

		// (r.gonet 08/05/2014) - PLID 63099 - If anything was modified, we should execute the save query and audit.
		bool bIsAnythingModified = false;

		if (m_setDeletedTestCodeIDs.size() > 0) {
			// (r.gonet 08/05/2014) - PLID 63099 - Test codes were deleted. Clear deleted charges of test codes that reference these.
			std::vector<long> vecDeletedTestCodeIDs(m_setDeletedTestCodeIDs.begin(), m_setDeletedTestCodeIDs.end());
			sqlSaveBatch.Add(R"(
DELETE FROM ChargeLabTestCodesT  
WHERE BillLabTestCodeID IN ({INTVECTOR})
AND ChargeID IN
(
	SELECT ChargesT.ID
	FROM ChargesT
	INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID
	INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID
	WHERE BillsT.Deleted = 1 OR LineItemT.Deleted = 1
);
)"
, vecDeletedTestCodeIDs);
			// (r.gonet 08/05/2014) - PLID 63099 - Test codes were deleted. Add delete SQL to the save batch.
			// (r.gonet 08/05/2014) - PLID 63100 - Added Inactive.
			sqlSaveBatch.Add(R"(
DELETE FROM BillLabTestCodesT
OUTPUT DELETED.ID, DELETED.Code, DELETED.Description, DELETED.Inactive, 1 
INTO @ChangedBillTestCodesT (ID, OldCode, OldDescription, OldInactive, Deleted)
WHERE ID IN ({INTVECTOR})
)"
, vecDeletedTestCodeIDs);
			// (r.gonet 08/05/2014) - PLID 63099 - Stuff was modified.
			bIsAnythingModified = true;
		}

		// (r.gonet 08/05/2014) - PLID 63099 - Loop over the datalist rows and see if any test codes changed or were added.
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pTestCodeList->FindAbsoluteFirstRow(VARIANT_FALSE);
		while (pRow) {
			long nID = VarLong(pRow->GetValue((short)ETestCodeListColumns::ID), -1);
			ETestCodeListRowState eState = (ETestCodeListRowState)VarLong(pRow->GetValue((short)ETestCodeListColumns::State), (long)ETestCodeListRowState::Unchanged);
			CString strCode = VarString(pRow->GetValue((short)ETestCodeListColumns::Code), "");
			CString strDescription = VarString(pRow->GetValue((short)ETestCodeListColumns::Description), "");
			// (r.gonet 08/05/2014) - PLID 63100 - Save the inactive flag.
			BOOL bInactive = VarBool(pRow->GetValue((short)ETestCodeListColumns::Inactive), FALSE);
			
			if (eState == ETestCodeListRowState::Modified) {
				// (r.gonet 08/05/2014) - PLID 63099 - The test code changed. So add update SQL.
				// (r.gonet 08/05/2014) - PLID 63100 - Added Inactive.
				sqlSaveBatch.Add(R"(
UPDATE BillLabTestCodesT 
SET Code = {STRING}, Description = {STRING}, Inactive = {BIT}
OUTPUT INSERTED.ID, DELETED.Code, INSERTED.Code, DELETED.Description, INSERTED.Description, DELETED.Inactive, INSERTED.Inactive, 1 
INTO @ChangedBillTestCodesT (ID, OldCode, NewCode, OldDescription, NewDescription, OldInactive, NewInactive, Updated)
WHERE ID = {INT}; 
)"
, strCode, strDescription, bInactive, nID);
				// (r.gonet 08/05/2014) - PLID 63099 - Stuff was modified.
				bIsAnythingModified = true;
			} else if (eState == ETestCodeListRowState::New) {
				// (r.gonet 08/05/2014) - PLID 63099 - The test code was added. So add insert SQL.
				// (r.gonet 08/05/2014) - PLID 63100 - Added Inactive.
				sqlSaveBatch.Add(R"(
INSERT BillLabTestCodesT 
(Code, Description, Inactive) 
OUTPUT {INT} AS TempID, INSERTED.ID, INSERTED.Code, INSERTED.Description, INSERTED.Inactive, 1 
INTO @ChangedBillTestCodesT (TempID, ID, NewCode, NewDescription, NewInactive, Inserted)
VALUES ({STRING}, {STRING}, {BIT}); 
)"
, nID, strCode, strDescription, bInactive);
				// (r.gonet 08/05/2014) - PLID 63099 - Stuff was modified.
				bIsAnythingModified = true;
			}
			pRow = m_pTestCodeList->FindAbsoluteNextRow(pRow, VARIANT_FALSE);
		}

		// (r.gonet 08/05/2014) - PLID 63099 - Select all the changes so we can audit them.
		// (r.gonet 08/05/2014) - PLID 63100 - Added Inactive.
		sqlSaveBatch.Add(R"(
SET NOCOUNT OFF;
SELECT TempID, ID, OldCode, NewCode, OldDescription, NewDescription, OldInactive, NewInactive, Inserted, Updated, Deleted
FROM @ChangedBillTestCodesT
)");
		
		// (r.gonet 08/05/2014) - PLID 63099 - If anything was modified, execute the save batch and audit.
		if (bIsAnythingModified) {
			nAuditTransactionID = BeginAuditTransaction();
			// (r.gonet 08/05/2014) - PLID 63099 - Save the changes.
			ADODB::_RecordsetPtr prs = sqlSaveBatch.CreateRecordset(GetRemoteData());
			while (!prs->eof) {
				// (r.gonet 08/05/2014) - PLID 63099 - Get the old and new values.

				// (r.gonet 08/05/2014) - PLID 63099 - TempID is the negative ID we assigned to the new test code rows.
				//  We can use this to find those rows in the datalist and update them with the actual IDs.
				long nTempID = AdoFldLong(prs->Fields, "TempID", -1);
				// (r.gonet 08/05/2014) - PLID 63099 - ID of the test code in the database.
				long nID = AdoFldLong(prs->Fields, "ID");
				CString strOldCode = AdoFldString(prs->Fields, "OldCode", "");
				CString strNewCode = AdoFldString(prs->Fields, "NewCode", "");
				CString strOldDescription = AdoFldString(prs->Fields, "OldDescription", "");
				CString strNewDescription = AdoFldString(prs->Fields, "NewDescription", "");
				// (r.gonet 08/05/2014) - PLID 63100 - Get the old and new Inactive flags so we can audit them.
				BOOL bOldInactive = AdoFldBool(prs->Fields, "OldInactive", FALSE);
				BOOL bNewInactive = AdoFldBool(prs->Fields, "NewInactive", FALSE);

				if (AdoFldBool(prs->Fields, "Deleted", FALSE)) {
					// (r.gonet 08/05/2014) - PLID 63099 - Do auditing for deleted
					AuditEvent(-1, "", nAuditTransactionID, aeiBillLabTestCodeDeleted, nID, strOldCode, "<Deleted>", aepMedium, aetDeleted);
				} else if (AdoFldBool(prs->Fields, "Updated", FALSE)) {
					// (r.gonet 08/05/2014) - PLID 63099 - Do auditing for updated
					if (strOldCode != strNewCode) {
						AuditEvent(-1, "", nAuditTransactionID, aeiBillLabTestCodeCode, nID, strOldCode, strNewCode, aepMedium, aetChanged);
					}
					if (strOldDescription != strNewDescription) {
						AuditEvent(-1, "", nAuditTransactionID, aeiBillLabTestCodeDescription, nID, FormatString("%s: %s", strOldCode, strOldDescription), strNewDescription, aepMedium, aetChanged);
					}
					// (r.gonet 08/05/2014) - PLID 63100 - Audit if the inactive flag changed.
					if (bOldInactive != bNewInactive) {
						AuditEvent(-1, "", nAuditTransactionID, aeiBillLabTestCodeInactive, nID, FormatString("%s: %s", strOldCode, bOldInactive ? "Inactive" : "Active"), (bNewInactive ? "Inactive" : "Active"), aepMedium, aetChanged);
					}
				} else if (AdoFldBool(prs->Fields, "Inserted", FALSE)) {
					// (r.gonet 08/05/2014) - PLID 63099 - Put the ID back in the list
					NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_pTestCodeList->FindByColumn((short)ETestCodeListColumns::ID, nTempID, m_pTestCodeList->GetFirstRow(), VARIANT_FALSE);
					if (pNewRow) {
						pNewRow->PutValue((short)ETestCodeListColumns::ID, nID);
					}

					// (r.gonet 08/05/2014) - PLID 63099 - Do auditing for inserted
					AuditEvent(-1, "", nAuditTransactionID, aeiBillLabTestCodeAdded, nID, "", strNewCode, aepMedium, aetCreated);
				}
				prs->MoveNext();
			}
			prs->Close();

			CommitAuditTransaction(nAuditTransactionID);
		}
		

		// (r.gonet 08/05/2014) - PLID 63102 - Now gather a vector of all the selected test code IDs so
		// the caller can obtain them after the dialog closes. 
		// (r.gonet 08/05/2014) - PLID 63100 - Note that all selected rows are visible, even if they 
		// are inactive.
		m_vecSelectedTestCodeIDs.clear();
		pRow = m_pTestCodeList->GetFirstRow();
		while (pRow) {
			long nID = VarLong(pRow->GetValue((short)ETestCodeListColumns::ID));
			BOOL bSelected = VarBool(pRow->GetValue((short)ETestCodeListColumns::Selected));
			if (bSelected) {
				m_vecSelectedTestCodeIDs.push_back(nID);
			}
			pRow = pRow->GetNextRow();
		}

		CNxDialog::OnOK();

	} NxCatchAllCall(__FUNCTION__,
	{
		// (r.gonet 08/05/2014) - PLID 63099 - Something bad happened. Rollback the audit.
		if (nAuditTransactionID != -1) {
			RollbackAuditTransaction(nAuditTransactionID);
		}
	});
}

// (r.gonet 08/05/2014) - PLID 63099 - Handle the event when the user clicks the delete button. Delete a test code.
void CEditBillTestCodesDlg::OnBnClickedEditTestCodesDeleteBtn()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pTestCodeList->CurSel;
		if (!pRow) {
			// (r.gonet 08/05/2014) - PLID 63099 - No test code selected. This should not have been callable.
			EnsureControls();
			return;
		}

		// (r.gonet 08/05/2014) - PLID 63099 - If this an unsaved test code, then just remove it from the list. It was never saved
		// and so there is no reason to run a DELETE command for it.
		ETestCodeListRowState eCurrentState = (ETestCodeListRowState)VarLong(pRow->GetValue((short)ETestCodeListColumns::State));
		if (eCurrentState != ETestCodeListRowState::New) {
			long nID = VarLong(pRow->GetValue((short)ETestCodeListColumns::ID), -1);
			if (nID != -1) {
				// (r.gonet 08/05/2014) - PLID 63099 - If the test code is in use by some charge, then it cannot be deleted.
				if (IsInUse(nID)) {
					CString strCode = VarString(pRow->GetValue((short)ETestCodeListColumns::Code), "");
					// (r.gonet 08/05/2014) - PLID 63100 - Since there is the ability to inactivate test codes, give that as an option.
					MessageBox(FormatString("The \"%s\" test code is in use by at least one existing charge and cannot be deleted. If you wish, you may inactivate the test code instead.", strCode), "Test Code in Use", MB_ICONERROR | MB_OK);
					return;
				}
				// (r.gonet 08/05/2014) - PLID 63099 - Since it is out of the datalist, remember it was deleted.
				m_setDeletedTestCodeIDs.insert(nID);
			} else {
				ASSERT(FALSE);
			}
		} else {
			// (r.gonet 08/05/2014) - PLID 63099 - Just remove it right here since it was never saved to begin with.
		}

		// (r.gonet 08/05/2014) - PLID 63099 - We're about to remove the datalist row. Ensure some other row is selected.
		if (pRow->GetNextRow()) {
			m_pTestCodeList->CurSel = pRow->GetNextRow();
		} else if(pRow->GetPreviousRow()) {
			m_pTestCodeList->CurSel = pRow->GetPreviousRow();
			
		}
		m_pTestCodeList->RemoveRow(pRow);
		if (m_pTestCodeList->CurSel) {
			m_pTestCodeList->EnsureRowInView(m_pTestCodeList->CurSel);
		}
		// (r.gonet 08/05/2014) - PLID 63099 - We may have no row, so update the delete button's enable state.
		EnsureControls();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 08/05/2014) - PLID 63099 - Returns true if the test code is in use by some non-deleted charge. Returns false if it is not.
bool CEditBillTestCodesDlg::IsInUse(long nTestCodeID)
{
	return ReturnsRecordsParam(R"(
SELECT NULL
FROM ChargeLabTestCodesT
INNER JOIN ChargesT ON ChargeLabTestCodesT.ChargeID = ChargesT.ID
INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID
INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID
WHERE ChargeLabTestCodesT.BillLabTestCodeID = {INT} 
AND LineItemT.Deleted = 0 
AND BillsT.Deleted = 0
)"
		, nTestCodeID) ? true : false;
}

// (r.gonet 08/05/2014) - PLID 63099 - Handles the event when the user clicks the Add button. Adds a new test code row.
void CEditBillTestCodesDlg::OnBnClickedEditTestCodesAddBtn()
{
	try {
		CString strCode;
		// (r.gonet 08/05/2014) - PLID 63099 - Get the new test code's codenumber.
		if (IDOK != InputBoxLimited(this, "Please type in the new test code.", strCode, "", 20, false, false, "Cancel")) {
			return;
		}
		// (r.gonet 08/05/2014) - PLID 63099 - Can't be empty.
		if (strCode.GetLength() == 0) {
			MessageBox("The code may not be blank.", "Error", MB_ICONERROR | MB_OK);
			return;
		}

		// (r.gonet 08/05/2014) - PLID 63099 - We can't just say the new code's ID is -1. We have to
		// ensure that we can find it later when we save and have to update the datalist row's ID to the actual saved ID.
		// So find the minimum negative ID and use the ID after that. -1 will never be used since it is sentinel.
		long nMinID = -1;
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pTestCodeList->FindAbsoluteFirstRow(VARIANT_FALSE);
		while (pRow) {
			long nID = VarLong(pRow->GetValue((short)ETestCodeListColumns::ID));
			if (nID < nMinID) {
				nMinID = nID;
			}
			pRow = m_pTestCodeList->FindAbsoluteNextRow(pRow, VARIANT_FALSE);
		}

		// (r.gonet 08/05/2014) - PLID 63099 - Add a new row to the test code list.
		pRow = m_pTestCodeList->GetNewRow();
		pRow->PutValue((short)ETestCodeListColumns::Selected, g_cvarFalse);
		pRow->PutValue((short)ETestCodeListColumns::ID, _variant_t(nMinID - 1, VT_I4));
		pRow->PutValue((short)ETestCodeListColumns::State, (long)ETestCodeListRowState::New);
		pRow->PutValue((short)ETestCodeListColumns::Code, _bstr_t(strCode));
		pRow->PutValue((short)ETestCodeListColumns::Description, "");
		// (r.gonet 08/05/2014) - PLID 63100 - Initialize the inactive cell.
		pRow->PutValue((short)ETestCodeListColumns::Inactive, g_cvarFalse);
		m_pTestCodeList->AddRowAtEnd(pRow, NULL);
		m_pTestCodeList->CurSel = pRow;
		m_pTestCodeList->EnsureRowInView(pRow);
		// (r.gonet 08/05/2014) - PLID 63099 - Not required to have a description but prompt the user to enter it anyway
		// since they probably will want to have one.
		m_pTestCodeList->StartEditing(pRow, (short)ETestCodeListColumns::Description);
		// (r.gonet 08/05/2014) - PLID 63099 - We have something selected now. The delete button can be enabled if it was disabled.
		EnsureControls();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 08/05/2014) - PLID 63102 - Fills a vector with the selected test code IDs
// - vecSelectedTestCodeIDs: vector to hold the selected test code IDs.
void CEditBillTestCodesDlg::GetSelectedTestCodeIDs(OUT std::vector<long> &vecSelectedTestCodeIDs)
{
	vecSelectedTestCodeIDs.clear();
	vecSelectedTestCodeIDs.assign(m_vecSelectedTestCodeIDs.begin(), m_vecSelectedTestCodeIDs.end());
}

BEGIN_EVENTSINK_MAP(CEditBillTestCodesDlg, CNxDialog)
ON_EVENT(CEditBillTestCodesDlg, IDC_EDIT_TEST_CODES_LIST, 2, CEditBillTestCodesDlg::SelChangedEditTestCodesList, VTS_DISPATCH VTS_DISPATCH)
ON_EVENT(CEditBillTestCodesDlg, IDC_EDIT_TEST_CODES_LIST, 10, CEditBillTestCodesDlg::EditingFinishedEditTestCodesList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
ON_EVENT(CEditBillTestCodesDlg, IDC_EDIT_TEST_CODES_LIST, 9, CEditBillTestCodesDlg::EditingFinishingEditTestCodesList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
END_EVENTSINK_MAP()

// (r.gonet 08/05/2014) - PLID 63099 - Handles the event when the user selects a different row from the test code list.
void CEditBillTestCodesDlg::SelChangedEditTestCodesList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try {
		// (r.gonet 08/05/2014) - PLID 63099 - A selected row means the delete button can be enabled.
		EnsureControls();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 08/05/2014) - PLID 63099 - Handles the event when the user is finishing editing a cell in the test code list.
void CEditBillTestCodesDlg::EditingFinishingEditTestCodesList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (!pRow) {
			// (r.gonet 08/05/2014) - PLID 63099 - Then why was this called?
			return;
		}

		long nID = VarLong(pRow->GetValue((short)ETestCodeListColumns::ID), -1);
		ETestCodeListRowState eCurrentState = (ETestCodeListRowState)VarLong(pRow->GetValue((short)ETestCodeListColumns::State));
		switch (nCol) {
		case (long)ETestCodeListColumns::Selected:
			// (r.gonet 08/05/2014) - PLID 63102 - They can always select a test code row.
			break;
		case (long)ETestCodeListColumns::Code:
			// (r.gonet 08/05/2014) - PLID 63099 - Can't leave a code number as empty.
			if (pvarNewValue && VarString(*pvarNewValue, "") == "") {
				MessageBox("The code may not be blank.", "Error", MB_ICONERROR | MB_OK);
				*pvarNewValue = varOldValue;
				*pbCommit = FALSE;
				return;
			}

			if (pvarNewValue && VarString(varOldValue, "") == VarString(*pvarNewValue, "")) {
				// (r.gonet 08/05/2014) - PLID 63099 - Nothing changed.
				*pvarNewValue = varOldValue;
				*pbCommit = FALSE;
				return;
			}

			// (r.gonet 08/05/2014) - PLID 63099 - Ensure the test code is not used by some existing charge.
			if (eCurrentState != ETestCodeListRowState::New && IsInUse(nID)) {
				MessageBox("This code is used on one or more bills and cannot be modified. You must create a new Test Code.", "Test Code in Use", MB_ICONERROR | MB_OK);
				*pvarNewValue = varOldValue;
				*pbCommit = FALSE;
				return;
			}
			break;
		case (long)ETestCodeListColumns::Description:
			if (pvarNewValue && VarString(varOldValue, "") == VarString(*pvarNewValue, "")) {
				// (r.gonet 08/05/2014) - PLID 63099 - Nothing changed.
				*pvarNewValue = varOldValue;
				*pbCommit = FALSE;
				return;
			}

			break;
		case (long)ETestCodeListColumns::Inactive:
			// (r.gonet 08/05/2014) - PLID 63100 - Handle changes to the inactive cell.
			if (pvarNewValue && VarBool(varOldValue, FALSE) == VarBool(*pvarNewValue, FALSE)) {
				// (r.gonet 08/05/2014) - PLID 63100 - Nothing changed.
				*pvarNewValue = varOldValue;
				*pbCommit = FALSE;
				return;
			}
			break;
		default:
			// (r.gonet 08/05/2014) - PLID 63099 - Some other unhandled column.
			ASSERT(FALSE);
			break;
		}

	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 08/05/2014) - PLID 63099 - Handles the event where the user has just finished editing a test code list cell.
void CEditBillTestCodesDlg::EditingFinishedEditTestCodesList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (!pRow) {
			// (r.gonet 08/05/2014) - PLID 63099 - Then why was this called?
			return;
		}
		if (!bCommit) {
			// (r.gonet 08/05/2014) - PLID 63099 - No changes should be saved.
			return;
		}

		long nID = VarLong(pRow->GetValue((short)ETestCodeListColumns::ID), -1);
		ETestCodeListRowState eCurrentState = (ETestCodeListRowState)VarLong(pRow->GetValue((short)ETestCodeListColumns::State));
		switch (nCol) {
		case (long)ETestCodeListColumns::Selected:
			// (r.gonet 08/05/2014) - PLID 63102 - They can change this all they like.
			break;
		case (long)ETestCodeListColumns::Code:
		case (long)ETestCodeListColumns::Description:
			if (VarString(varOldValue, "") == VarString(varNewValue, "")) {
				// (r.gonet 08/05/2014) - PLID 63099 - No changes.
				return;
			}

			if (eCurrentState != ETestCodeListRowState::New) {
				// (r.gonet 08/05/2014) - PLID 63099 - Unless it is a new row (which needs an insert statement later), set
				// it as modified.
				pRow->PutValue((short)ETestCodeListColumns::State, (long)ETestCodeListRowState::Modified);
			}
			break;
		case (long)ETestCodeListColumns::Inactive:
			if (VarBool(varOldValue, FALSE) == VarBool(varNewValue, FALSE)) {
				// (r.gonet 08/05/2014) - PLID 63100 - No changes.
				return;
			}
			// (r.gonet 08/05/2014) - PLID 63100 - The row has become inactive. May need hidden.
			SetInactive(pRow, VarBool(varNewValue, FALSE) ? true : false);
			break;
		default:
			// (r.gonet 08/05/2014) - PLID 63099 - Some unknown column.
			ASSERT(FALSE);
			break;
		}
		
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 08/05/2014) - PLID 63100 - Sets a database row to inactive, grays it, and hides it if necessary.
// Undoes this if setting to active.
// - pRow: Test code list row to inactivate/activate.
// - bInactive: true to inactivate the row. false to activate it.
void CEditBillTestCodesDlg::SetInactive(NXDATALIST2Lib::IRowSettingsPtr pRow, bool bInactive)
{
	if (!pRow) {
		return;
	}
	// (r.gonet 08/05/2014) - PLID 63100 - Ensure the row's inactive value is correct.
	pRow->PutValue((short)ETestCodeListColumns::Inactive, bInactive ? g_cvarTrue : g_cvarFalse);
	
	ETestCodeListRowState eCurrentState = (ETestCodeListRowState)VarLong(pRow->GetValue((short)ETestCodeListColumns::State));
	if (eCurrentState != ETestCodeListRowState::New) {
		// (r.gonet 08/05/2014) - PLID 63100 - Ensure the row is now modified.
		pRow->PutValue((short)ETestCodeListColumns::State, (long)ETestCodeListRowState::Modified);
	}

	// (r.gonet 08/05/2014) - PLID 63100 - Show or hide the row depending on whether we are hiding inactive
	// and gray it if it is inactive.
	// (r.gonet 08/06/2014) - PLID 63102 - Don't hide it if the row is selected.
	BOOL bSelected = VarBool(pRow->GetValue((short)ETestCodeListColumns::Selected), FALSE);
	if (bInactive) {
		if (m_checkHideInactive.GetCheck() == BST_CHECKED && !bSelected) {
			pRow->Visible = g_cvarFalse;
		}
		pRow->BackColor = COLOR_GRAYED_OUT;
	} else {
		pRow->Visible = g_cvarTrue;
		pRow->BackColor = RGB(255, 255, 255);
	}
	// (r.gonet 08/05/2014) - PLID 63100 - We may have hidden or shown a row, so enable/disable the delete button.
	EnsureControls();
}
// (r.gonet 03/29/2012) - PLID 49299 - PLID 49299 - Created.

#include "stdafx.h"
#include "Practice.h"
#include "LabProcedureGroupsSetupDlg.h"
#include "LabFormNumberEditorDlg.h"

// LabProcedureGroupsSetupDlg.cpp : implementation file
//

using namespace ADODB;
using namespace NXDATALIST2Lib;

// CLabProcedureGroupsSetupDlg dialog

IMPLEMENT_DYNAMIC(CLabProcedureGroupsSetupDlg, CNxDialog)

// (r.gonet 03/29/2012) - PLID 49299 - Constructs a new dialog to edit lab procedure groups.
CLabProcedureGroupsSetupDlg::CLabProcedureGroupsSetupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CLabProcedureGroupsSetupDlg::IDD, pParent)
{
	// (r.gonet 03/29/2012) - PLID 49299 - Initialize the current group being setup to the default group's ID.
	m_nCurrentGroupID = -1;
}

CLabProcedureGroupsSetupDlg::~CLabProcedureGroupsSetupDlg()
{
}

// (r.gonet 03/29/2012) - PLID 49299 - Requery the groups combo box.
void CLabProcedureGroupsSetupDlg::ReloadGroupsComboBox()
{
	m_pGroupsComboBox->Requery();
	m_pGroupsComboBox->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);

	// Ensure a row is selected
	if(m_pGroupsComboBox->CurSel == NULL) {
		// Select the default group.
		m_pGroupsComboBox->SetSelByColumn(lpgcID, -1);
	}
}

// (r.gonet 03/29/2012) - PLID 49299 - Requery the left hand side unselected lab procedures list.
//  These are lab procedures that are members of not members of any non-default group and not members of the current group.
void CLabProcedureGroupsSetupDlg::ReloadUnselectedList()
{
	if(m_nCurrentGroupID == -1) {
		// (r.gonet 03/29/2012) - PLID 49299 - There are no unselected lab procedures for the default group,
		//  either Lab Procedures belong to a different group or they are Selected as members in the default group.
		m_pUnselectedLabProceduresList->Clear();
	} else {
		// (r.gonet 03/29/2012) - PLID 49299 - Requery non-default groups from the data. Just get all Lab Procedures 
		//  that do not belong to any non-default groups.
		m_pUnselectedLabProceduresList->Requery();
		m_pUnselectedLabProceduresList->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
	}
}

// (r.gonet 03/29/2012) - PLID 49299 - Requery the right hand side selected lab procedures list.
// These are lab procedures that are members of the current group.
void CLabProcedureGroupsSetupDlg::ReloadSelectedList()
{
	if(m_nCurrentGroupID == -1) {
		// (r.gonet 03/29/2012) - PLID 49299 - Default group gets all lab procedures that are not members of any group.
		m_pSelectedLabProceduresList->WhereClause = _bstr_t(
			FormatString("LabProceduresT.LabProcedureGroupID IS NULL AND Inactive = 0"));
	} else {
		// (r.gonet 03/29/2012) - PLID 49299 - Non-Default groups get all lab procedures that are members of this group.
		m_pSelectedLabProceduresList->WhereClause = _bstr_t(
			FormatString("LabProceduresT.LabProcedureGroupID = %li AND Inactive = 0", m_nCurrentGroupID));
	}
	m_pSelectedLabProceduresList->Requery();
	m_pSelectedLabProceduresList->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
}

// (r.gonet 03/29/2012) - PLID 49299 - Enables and disables controls depending on the current state of the dialog data.
void CLabProcedureGroupsSetupDlg::EnsureControls()
{
	bool bGroupSelected = m_pGroupsComboBox->CurSel != NULL ? true : false;
	bool bDefaultGroupSelected = m_nCurrentGroupID == -1;
	bool bUnselectedListRowSelected = m_pUnselectedLabProceduresList->CurSel != NULL ? true : false;
	bool bUnselectedListHasRows = m_pUnselectedLabProceduresList->GetRowCount() > 0;
	bool bSelectedListRowSelected = m_pSelectedLabProceduresList->CurSel != NULL ? true : false;
	bool bSelectedListHasRows = m_pSelectedLabProceduresList->GetRowCount() > 0;

	// (r.gonet 03/29/2012) - PLID 49299 - Don't let the user delete or rename the default group.
	m_nxbRemoveGroup.EnableWindow(bGroupSelected && !bDefaultGroupSelected ? TRUE : FALSE);
	m_nxbRenameGroup.EnableWindow(bGroupSelected && !bDefaultGroupSelected ? TRUE : FALSE);

	// (r.gonet 03/29/2012) - PLID 49299 - Disable the unselected list if we have the default group selected. It doesn't make sense here.
	m_pUnselectedLabProceduresList->Enabled = (bGroupSelected && !bDefaultGroupSelected ? TRUE : FALSE);
	m_nxbSelectOne.EnableWindow(bGroupSelected && !bDefaultGroupSelected && bUnselectedListRowSelected ? TRUE : FALSE);
	m_nxbSelectAll.EnableWindow(bGroupSelected && !bDefaultGroupSelected && bUnselectedListHasRows ? TRUE : FALSE);
	m_nxbUnselectAll.EnableWindow(bGroupSelected && !bDefaultGroupSelected && bSelectedListHasRows ? TRUE : FALSE);
	m_nxbUnselectOne.EnableWindow(bGroupSelected && !bDefaultGroupSelected && bSelectedListRowSelected ? TRUE : FALSE);
	m_pSelectedLabProceduresList->Enabled = (bGroupSelected ? TRUE : FALSE);

	m_nxbFormNumberSetup.EnableWindow(bGroupSelected ? TRUE : FALSE);
}

void CLabProcedureGroupsSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLabProcedureGroupsSetupDlg)
	DDX_Control(pDX, IDC_LPG_ADD_GROUP_BTN, m_nxbAddGroup);
	DDX_Control(pDX, IDC_LPG_REMOVE_GROUP_BTN, m_nxbRemoveGroup);
	DDX_Control(pDX, IDC_LPG_RENAME_GROUP_BTN, m_nxbRenameGroup);
	DDX_Control(pDX, IDC_LPG_SELECT_ONE_BTN, m_nxbSelectOne);
	DDX_Control(pDX, IDC_LPG_SELECT_ALL_BTN, m_nxbSelectAll);
	DDX_Control(pDX, IDC_LPG_UNSELECT_ALL_BTN, m_nxbUnselectAll);
	DDX_Control(pDX, IDC_LPG_UNSELECT_ONE_BTN, m_nxbUnselectOne);
	DDX_Control(pDX, IDC_LPG_FORM_NUMBER_SETTINGS_BTN, m_nxbFormNumberSetup);
	DDX_Control(pDX, IDOK, m_nxbOK);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CLabProcedureGroupsSetupDlg, CNxDialog)
	ON_BN_CLICKED(IDC_LPG_ADD_GROUP_BTN, &CLabProcedureGroupsSetupDlg::OnBnClickedLpgAddGroupBtn)
	ON_BN_CLICKED(IDC_LPG_REMOVE_GROUP_BTN, &CLabProcedureGroupsSetupDlg::OnBnClickedLpgRemoveGroupBtn)
	ON_BN_CLICKED(IDC_LPG_RENAME_GROUP_BTN, &CLabProcedureGroupsSetupDlg::OnBnClickedLpgRenameGroupBtn)
	ON_BN_CLICKED(IDC_LPG_SELECT_ONE_BTN, &CLabProcedureGroupsSetupDlg::OnBnClickedLpgSelectOneBtn)
	ON_BN_CLICKED(IDC_LPG_SELECT_ALL_BTN, &CLabProcedureGroupsSetupDlg::OnBnClickedLpgSelectAllBtn)
	ON_BN_CLICKED(IDC_LPG_UNSELECT_ALL_BTN, &CLabProcedureGroupsSetupDlg::OnBnClickedLpgUnselectAllBtn)
	ON_BN_CLICKED(IDC_LPG_UNSELECT_ONE_BTN, &CLabProcedureGroupsSetupDlg::OnBnClickedLpgUnselectOneBtn)
	ON_BN_CLICKED(IDC_LPG_FORM_NUMBER_SETTINGS_BTN, &CLabProcedureGroupsSetupDlg::OnBnClickedLpgFormNumberSettingsBtn)
END_MESSAGE_MAP()


// CLabReqCustomFieldsDlg message handlers

BOOL CLabProcedureGroupsSetupDlg::OnInitDialog()
{
	try {
		CNxDialog::OnInitDialog();

		// (r.gonet 03/29/2012) - PLID 49299 - Autoset CNxIconButtons
		m_nxbAddGroup.AutoSet(NXB_NEW);
		m_nxbRemoveGroup.AutoSet(NXB_DELETE);
		m_nxbRenameGroup.AutoSet(NXB_MODIFY);
		m_nxbSelectOne.AutoSet(NXB_RIGHT);
		m_nxbSelectAll.AutoSet(NXB_RRIGHT);
		m_nxbUnselectAll.AutoSet(NXB_LLEFT);
		m_nxbUnselectOne.AutoSet(NXB_LEFT);
		m_nxbFormNumberSetup.AutoSet(NXB_MODIFY);
		m_nxbOK.AutoSet(NXB_CLOSE);

		// (r.gonet 03/29/2012) - PLID 49299 - Bind datalists to controls
		m_pGroupsComboBox = BindNxDataList2Ctrl(this, IDC_LPG_GROUPS_LIST, GetRemoteData(), false);
		m_pUnselectedLabProceduresList = BindNxDataList2Ctrl(this, IDC_LPG_UNSELECTED_PROCEDURES_LIST, GetRemoteData(), false);
		m_pSelectedLabProceduresList = BindNxDataList2Ctrl(this, IDC_LPG_SELECTED_PROCEDURES_LIST, GetRemoteData(), false);

		// (r.gonet 03/29/2012) - PLID 49299 - Requery datalists
		ReloadGroupsComboBox();
		ReloadUnselectedList();
		ReloadSelectedList();

		// (r.gonet 03/29/2012) - PLID 49299 - Make sure the controls are shown properly.
		EnsureControls();
	} NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

// (r.gonet 03/29/2012) - PLID 49299 - Creates a new lab procedure group, saves it, and adds it to the groups combo box
void CLabProcedureGroupsSetupDlg::OnBnClickedLpgAddGroupBtn()
{
	try {
		// (r.gonet 03/29/2012) - PLID 49299 - Get the name from the user.
		CString strNewName;
		if (IDOK == (InputBoxLimited(this, "Please enter a name for the new group.", strNewName, "", 255, false, false, NULL))) {		
			// Check for blanks and duplicates
			if(strNewName.IsEmpty()) {
				MessageBox("All groups must have names.", "Error", MB_ICONERROR);
				return;
			}

			// (r.gonet 03/29/2012) - PLID 49299 - Check to make sure the name is unique.
			if(ReturnsRecordsParam("SELECT Name FROM LabProcedureGroupsT WHERE Name LIKE {STRING}", strNewName)) {
				MessageBox("A group already exists with that name. All groups must have unique names.", "Error", MB_ICONERROR);
				return;
			}

			// (r.gonet 03/29/2012) - PLID 49299 - It was, so save it.
			_RecordsetPtr prs = CreateParamRecordset(GetRemoteData(), 
				"SET NOCOUNT ON; "
				"INSERT INTO LabProcedureGroupsT (Name) "
				"VALUES" 
				"({STRING}); "
				"SET NOCOUNT OFF; "
				"SELECT CONVERT(INT, SCOPE_IDENTITY()) AS NewID; ",
				strNewName);

			// (r.gonet 03/29/2012) - PLID 49299 - We're going to be switching to the new group now.
			m_nCurrentGroupID = VarLong(prs->Fields->Item["NewID"]->Value);

			// Add it to the groups combo and select it.
			NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_pGroupsComboBox->GetNewRow();
			pNewRow->PutValue(lpgcID, _variant_t(m_nCurrentGroupID));
			pNewRow->PutValue(lpgcName, _variant_t(strNewName));
			m_pGroupsComboBox->AddRowSorted(pNewRow, NULL);
			m_pGroupsComboBox->SetSelByColumn(lpgcID, m_nCurrentGroupID);

			EnsureControls();
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 03/29/2012) - PLID 49299 - Let the user remove the currently selected group from the database.
//  Don't let the user remove the default group.
void CLabProcedureGroupsSetupDlg::OnBnClickedLpgRemoveGroupBtn()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pGroupsComboBox->CurSel;
		if(pRow) {
			if(m_nCurrentGroupID == -1){
				// (r.gonet 03/29/2012) - PLID 49299 - Ensure controls should protect us from this and this branch will never be called.
				MsgBox("The default group cannot be removed.");
				return;
			}

			// Make sure the user knows what this is going to do.
			if(IDNO == DontShowMeAgain(this, 
				"This will delete the current Lab Procedure Group. This will not delete the actual lab procedures nor will it "
				"affect any existing patient lab data.\r\n"
				"\r\n"
				"Do you want to proceed?", "LabProcedureGroupDelete", "", FALSE, TRUE, FALSE))
			{
				return;
			}

			_RecordsetPtr prs = CreateParamRecordset(GetRemoteData(), 
				"UPDATE LabProceduresT SET LabProcedureGroupID = NULL WHERE LabProcedureGroupID = {INT}; "
				"DELETE FROM LabProcedureGroupsT "
				"WHERE ID = {INT}; ",
				m_nCurrentGroupID,
				m_nCurrentGroupID);

			// Remove it from the datalist
			m_pGroupsComboBox->RemoveRow(pRow);
			m_nCurrentGroupID = -1;
			m_pGroupsComboBox->SetSelByColumn(lpgcID, m_nCurrentGroupID);

			EnsureControls();
		} else {
			// (r.gonet 03/29/2012) - PLID 49299 - Ensure controls should make sure this branch never gets called but just in case.
			MsgBox("Please select a group to delete.");
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 03/29/2012) - PLID 49299
void CLabProcedureGroupsSetupDlg::OnBnClickedLpgRenameGroupBtn()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pGroupsComboBox->CurSel;
		if(pRow)
		{
			if(m_nCurrentGroupID == -1){
				// (r.gonet 03/29/2012) - PLID 49299 - We should never get here. Otherwise EnsureControls was missed.
				MsgBox("The default group cannot be renamed.");
				return;
			}

			CString strNewName = VarString(pRow->GetValue(lpgcName));
			if (IDOK == (InputBoxLimited(this, "Please enter a new name for the new template.", strNewName, "", 255, false, false, NULL))) {
				// Check for blanks and duplicates
				if(strNewName.IsEmpty()) {
					MessageBox("All groups must have names.", "Error", MB_ICONERROR);
					return;
				}

				// (r.gonet 03/29/2012) - PLID 49299 - Don't check against self.
				if(ReturnsRecordsParam("SELECT Name FROM LabProcedureGroupsT WHERE Name LIKE {STRING} AND ID <> {INT}", strNewName, m_nCurrentGroupID)) {
					MessageBox("A group already exists with that name. All groups must have unique names.", "Error", MB_ICONERROR);
					return;
				}

				ExecuteParamSql(GetRemoteData(), 
					"UPDATE LabProcedureGroupsT SET Name = {STRING} "
					"WHERE ID = {INT}; ",
					strNewName,
					m_nCurrentGroupID);

				// Rename the current group in the data list
				pRow->PutValue(lpgcName, _variant_t(strNewName));

				EnsureControls();
			}
		} else {
			MsgBox("Please select a group to rename.");
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 03/29/2012) - PLID 49299 - Moves one unselected lab procedure from left to right. Saves to current group.
void CLabProcedureGroupsSetupDlg::OnBnClickedLpgSelectOneBtn()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pUnselectedLabProceduresList->CurSel;
		if(pRow) {
			// (r.gonet 03/29/2012) - PLID 49299 - Get the ID of the lab procedure
			long nLabProcedureID = VarLong(pRow->GetValue(lpgucID));
			m_pSelectedLabProceduresList->TakeCurrentRowAddSorted(m_pUnselectedLabProceduresList, NULL);

			// (r.gonet 03/29/2012) - PLID 49299 - Update that lab procedure in data to belong to the current group.
			ExecuteParamSql(GetRemoteData(), 
				"UPDATE LabProceduresT SET LabProcedureGroupID = {INT} WHERE LabProceduresT.ID = {INT}; ",
				m_nCurrentGroupID, nLabProcedureID);

			EnsureControls();
		} else {
			// (r.gonet 03/29/2012) - PLID 49299 - Should not be possible.
			MsgBox("Please select a lab procedure from the Unselected list.");
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 03/29/2012) - PLID 49299 - Moves all unselected lab procedures to the current group.
void CLabProcedureGroupsSetupDlg::OnBnClickedLpgSelectAllBtn()
{
	try {
		// (r.gonet 03/29/2012) - PLID 49299 - Move the datalist rows from left to right
		m_pSelectedLabProceduresList->TakeAllRows(m_pUnselectedLabProceduresList);

		// (r.gonet 03/29/2012) - PLID 49299 - Save to data.
		ExecuteParamSql(GetRemoteData(), 
			"UPDATE LabProceduresT SET LabProcedureGroupID = {INT} WHERE LabProceduresT.LabProcedureGroupID IS NULL; ",
			m_nCurrentGroupID);

		EnsureControls();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 03/29/2012) - PLID 49299 - Removes currently grouped lab procedures from their groups and makes them available again.
void CLabProcedureGroupsSetupDlg::OnBnClickedLpgUnselectAllBtn()
{
	try {
		// (r.gonet 03/29/2012) - PLID 49299 - Move all rows from right to left
		m_pUnselectedLabProceduresList->TakeAllRows(m_pSelectedLabProceduresList);

		// (r.gonet 03/29/2012) - PLID 49299 - Save to data
		ExecuteParamSql(GetRemoteData(), 
			"UPDATE LabProceduresT SET LabProcedureGroupID = NULL WHERE LabProceduresT.LabProcedureGroupID = {INT}; ",
			m_nCurrentGroupID);

		EnsureControls();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 03/29/2012) - PLID 49299 - Removes the currently selected grouped lab procedure from its group and makes it available again.
void CLabProcedureGroupsSetupDlg::OnBnClickedLpgUnselectOneBtn()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pSelectedLabProceduresList->CurSel;
		if(pRow) {
			// (r.gonet 03/29/2012) - PLID 49299 - Get the lab procedure's ID
			long nLabProcedureID = VarLong(pRow->GetValue(lpgscID));
			// (r.gonet 03/29/2012) - PLID 49299 - Move the row from right to left
			m_pUnselectedLabProceduresList->TakeCurrentRowAddSorted(m_pSelectedLabProceduresList, NULL);

			// (r.gonet 03/29/2012) - PLID 49299 - Make it available in data
			ExecuteParamSql(GetRemoteData(), 
				"UPDATE LabProceduresT SET LabProcedureGroupID = NULL WHERE LabProceduresT.ID = {INT}; ",
				nLabProcedureID);

			EnsureControls();
		} else {
			// (r.gonet 03/29/2012) - PLID 49299 - Should not be possible.
			MsgBox("Please select a lab procedure from the Selected list.");
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 03/29/2012) - PLID 49299
void CLabProcedureGroupsSetupDlg::OnBnClickedLpgFormNumberSettingsBtn()
{
	try {
		// (r.gonet 03/29/2012) - PLID 49299 - Lab form number formats can now be per lab procedure group.
		CLabFormNumberEditorDlg dlg(m_nCurrentGroupID, this);
		dlg.DoModal();
	} NxCatchAll(__FUNCTION__);
}

BEGIN_EVENTSINK_MAP(CLabProcedureGroupsSetupDlg, CNxDialog)
	ON_EVENT(CLabProcedureGroupsSetupDlg, IDC_LPG_GROUPS_LIST, 1, CLabProcedureGroupsSetupDlg::SelChangingLpgGroupsList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CLabProcedureGroupsSetupDlg, IDC_LPG_UNSELECTED_PROCEDURES_LIST, 2, CLabProcedureGroupsSetupDlg::SelChangedLpgUnselectedProceduresList, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CLabProcedureGroupsSetupDlg, IDC_LPG_SELECTED_PROCEDURES_LIST, 2, CLabProcedureGroupsSetupDlg::SelChangedLpgSelectedProceduresList, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CLabProcedureGroupsSetupDlg, IDC_LPG_GROUPS_LIST, 28, CLabProcedureGroupsSetupDlg::CurSelWasSetLpgGroupsList, VTS_NONE)
END_EVENTSINK_MAP()

// (r.gonet 03/29/2012) - PLID 49299 - Current selection of groups list should ALWAYS be reflected on the interface.
void CLabProcedureGroupsSetupDlg::CurSelWasSetLpgGroupsList()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pGroupsComboBox->CurSel;
		if(pRow) {
			// (r.gonet 03/29/2012) - PLID 49299 - Set our selection to the group the user selected
			m_nCurrentGroupID = VarLong(pRow->GetValue(lpgcID));
		} else {
			// (r.gonet 03/29/2012) - PLID 49299 - Null was selected? This shouldn't happen. Revert to default.
			m_nCurrentGroupID = -1;
		}

		// (r.gonet 03/29/2012) - PLID 49299 - Requery the member lists.
		ReloadUnselectedList();
		ReloadSelectedList();
		EnsureControls();
	} NxCatchAll(__FUNCTION__);
}

void CLabProcedureGroupsSetupDlg::SelChangingLpgGroupsList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if (*lppNewSel == NULL) {
			// (r.gonet 03/29/2012) - PLID 49299 - Don't let the user select NULL.
			SafeSetCOMPointer(lppNewSel, lpOldSel);

			EnsureControls();
		}
	}NxCatchAll(__FUNCTION__);
}

// (r.gonet 03/29/2012) - PLID 49299
void CLabProcedureGroupsSetupDlg::SelChangedLpgUnselectedProceduresList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try {
		EnsureControls();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 03/29/2012) - PLID 49299
void CLabProcedureGroupsSetupDlg::SelChangedLpgSelectedProceduresList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try {
		EnsureControls();
	} NxCatchAll(__FUNCTION__);
}

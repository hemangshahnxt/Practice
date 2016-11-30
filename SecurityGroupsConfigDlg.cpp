// SecurityGroupsConfigDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PatientsRc.h"
#include "SecurityGroupsConfigDlg.h"

//TES 1/5/2010 - PLID 35774 - Created, inspired by CPatientGroupsConfigDlg.
// CSecurityGroupsConfigDlg dialog

IMPLEMENT_DYNAMIC(CSecurityGroupsConfigDlg, CNxDialog)

CSecurityGroupsConfigDlg::CSecurityGroupsConfigDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CSecurityGroupsConfigDlg::IDD, pParent)
{

}

CSecurityGroupsConfigDlg::~CSecurityGroupsConfigDlg()
{
}

void CSecurityGroupsConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_nxbOK);
	DDX_Control(pDX, IDCANCEL, m_nxbCancel);
	DDX_Control(pDX, IDC_ADD_GROUP, m_nxbAdd);
	DDX_Control(pDX, IDC_RENAME_GROUP, m_nxbRename);
	DDX_Control(pDX, IDC_DELETE_GROUP, m_nxbDelete);
}


BEGIN_MESSAGE_MAP(CSecurityGroupsConfigDlg, CNxDialog)
	ON_BN_CLICKED(IDC_ADD_GROUP, &CSecurityGroupsConfigDlg::OnAddGroup)
	ON_BN_CLICKED(IDC_DELETE_GROUP, &CSecurityGroupsConfigDlg::OnDeleteGroup)
	ON_BN_CLICKED(IDC_RENAME_GROUP, &CSecurityGroupsConfigDlg::OnBnClickedRenameGroup)
	ON_BN_CLICKED(IDOK, &CSecurityGroupsConfigDlg::OnOK)
END_MESSAGE_MAP()

using namespace NXDATALIST2Lib;
using namespace ADODB;

enum SecurityGroupListColumns
{
	sglcID = 0,
	sglcName = 1,
};

// CSecurityGroupsConfigDlg message handlers
BOOL CSecurityGroupsConfigDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try {
		//TES 1/5/2010 - PLID 35774 - Bind our list
		m_pGroups = BindNxDataList2Ctrl(IDC_SECURITY_GROUPS_LIST);

		//TES 1/5/2010 - PLID 35774 - Set all our NxIconButtons
		m_nxbOK.AutoSet(NXB_OK);
		m_nxbCancel.AutoSet(NXB_CANCEL);
		m_nxbAdd.AutoSet(NXB_NEW);
		m_nxbRename.AutoSet(NXB_MODIFY);
		m_nxbDelete.AutoSet(NXB_DELETE);

		//TES 1/5/2010 - PLID 35774 - Reflect whether a group is selected.
		UpdateButtons();

	}NxCatchAll(__FUNCTION__);

	return TRUE;
}
void CSecurityGroupsConfigDlg::OnAddGroup()
{
	try {
		//TES 1/5/2010 - PLID 35774 - Prompt for a name
		CString groupName;

		if (Prompt("Enter a name for the new Security Group:", groupName, 100) != IDOK)
			return;

		groupName.TrimRight();
		groupName.TrimLeft();

		//TES 1/5/2010 - PLID 35774 - Check that it doesn't already exist.
		if(!IsGroupNameValid(groupName))	{
			return;
		}

		//TES 1/5/2010 - PLID 35774 - Just add to the array, we'll save at the edn.
		long nNewID;
		if(m_arAddedGroups.GetSize() == 0) {
			nNewID = -1;
		}
		else {
			nNewID = m_arAddedGroups.GetAt(m_arAddedGroups.GetUpperBound()).nID-1;
		}
		SecurityGroupInfo sgi;
		sgi.nID = nNewID;
		sgi.strName = groupName;

		m_arAddedGroups.Add(sgi);

		//TES 1/5/2010 - PLID 35774 - Add to the datalist.
		IRowSettingsPtr pRow = m_pGroups->GetNewRow();
		pRow->PutValue(sglcID, nNewID);
		pRow->PutValue(sglcName, _bstr_t(groupName));
		m_pGroups->AddRowSorted(pRow, NULL);

		//TES 1/5/2010 - PLID 35774 - Reflect whether a group is selected.
		UpdateButtons();
	}NxCatchAll(__FUNCTION__);
}

BOOL CSecurityGroupsConfigDlg::IsGroupNameValid(const CString &strNewName, OPTIONAL const CString &strOldName /*= ""*/)
{
	//TES 1/5/2010 - PLID 35774 - Check to make sure the name isn't blank
	if(strNewName == "") {
		MessageBox("You cannot create a Security Group with a blank name.");
		return FALSE;
	}

	//TES 1/5/2010 - PLID 35774 - If the new name and the old name are the same, then the name is valid
	if(strNewName.CompareNoCase(strOldName) == 0){
		return TRUE;
	}
	
	//TES 1/5/2010 - PLID 35774 - Check to make sure the group name isn't duplicated
	IRowSettingsPtr pRow = m_pGroups->GetFirstRow();
	while(pRow){
		_variant_t var = pRow->GetValue(sglcName);
		
		CString strGroupName;
		strGroupName = VarString(var);
		
		if(strNewName.CompareNoCase(VarString(pRow->GetValue(sglcName))) == 0)
		{
			AfxMessageBox("Security Group names must be unique");
			return FALSE;
		}

		pRow = pRow->GetNextRow();
	}
	return TRUE;
}
void CSecurityGroupsConfigDlg::OnDeleteGroup()
{
	try {
		IRowSettingsPtr pRow = m_pGroups->CurSel;
		if(pRow == NULL) {
			return;
		}
		long nGroupID = VarLong(pRow->GetValue(sglcID));
		if(nGroupID < 0) {
			//TES 1/5/2010 - PLID 35774 - This has never been saved, just go ahead and remove it.
			m_arDeletedGroupIDs.Add(nGroupID);
			m_pGroups->RemoveRow(pRow);
			UpdateButtons();
			return;
		}
		else {
			//TES 1/5/2010 - PLID 35774 - Make sure they understand.
			if(IDYES != MsgBox(MB_YESNO|MB_ICONEXCLAMATION, "WARNING! Deleting this Security Group will make all patients in this group accessible to EVERY NexTech user "
				"(unless the patient is also in another Security Group).  Are you SURE you wish to do this?")) {
					return;
			}

			//TES 1/5/2010 - PLID 35774 - Remove it.
			m_arDeletedGroupIDs.Add(nGroupID);
			m_pGroups->RemoveRow(pRow);
			//TES 1/5/2010 - PLID 35774 - Reflect whether a group is selected.
			UpdateButtons();
		}
	}NxCatchAll(__FUNCTION__);
}

void CSecurityGroupsConfigDlg::OnBnClickedRenameGroup()
{
	try {
		IRowSettingsPtr pRow = m_pGroups->CurSel;
		if(pRow == NULL) {
			return;
		}

		CString strNewName;

		if (Prompt("Enter a new name for this Security Group:", strNewName, 100) != IDOK)
			return;

		strNewName.TrimRight();
		strNewName.TrimLeft();
		
		//TES 1/5/2010 - PLID 35774 - Get the group name we are trying to rename
		CString strOldName = VarString(pRow->GetValue(sglcName));
		
		//TES 1/5/2010 - PLID 35774 - Validate it
		if(!IsGroupNameValid(strNewName, strOldName)){
			return;
		}

		long nID = VarLong(pRow->GetValue(sglcID));
		//TES 1/5/2010 - PLID 35774 - Just save to array for now.
		SecurityGroupInfo sgi;
		sgi.nID = nID;
		sgi.strName = strNewName;
		
		//TES 1/5/2010 - PLID 35774 - If the group has been newly added, then we just want to rename the group in the AddedGroups array
		if (!RenameElements(m_arAddedGroups, sgi)) {
			//TES 1/5/2010 - PLID 35774 - The group wasn't found in the added array so check and see if it's been renamed
			if (!RenameElements(m_arRenamedGroups, sgi)) {
				//TES 1/5/2010 - PLID 35774 - The group hasn't been newly renamed so add it to the renamed array
				m_arRenamedGroups.Add(sgi);
			}
		}
		
		//TES 1/5/2010 - PLID 35774 - Update the datalist.
		pRow->PutValue(sglcName, _bstr_t(strNewName));
	
	}NxCatchAll(__FUNCTION__);
}

BOOL CSecurityGroupsConfigDlg::RenameElements(CArray<SecurityGroupInfo, SecurityGroupInfo&> &ary, IN SecurityGroupInfo &sgiNewName)
{
	for(int i = 0; i < ary.GetSize(); i++)
	{
		if(ary[i].nID == sgiNewName.nID)
		{
			//TES 1/5/2010 - PLID 35774 - Found it, rename and exit.
			ary.SetAt(i, sgiNewName);
			return true;
		}
	}
	//TES 1/5/2010 - PLID 35774 - We didn't find the element in the array so return false
	return false;
}

void CSecurityGroupsConfigDlg::OnOK()
{
	try {
		//TES 1/5/2010 - PLID 35774 - Set up a SQL batch.
		CString strSql;
		AddDeclarationToSqlBatch(strSql, "SET NOCOUNT ON");
		//TES 1/11/2010 - PLID 36761 - Track whether we actually change anything.
		bool bSomethingChanged = false;

		//TES 1/5/2010 - PLID 35774 - First, delete all deleted groups.
		for(int i = 0; i <= m_arDeletedGroupIDs.GetUpperBound(); i++) {
			long nDoomedID = m_arDeletedGroupIDs.GetAt(i);
			//TES 1/5/2010 - PLID 35774 - OK, first, take it out of the renamed and checked arrays.
			bool bRemoved = false;
			for(int j = 0; j <= m_arRenamedGroups.GetUpperBound() && !bRemoved; j++) {
				if(m_arRenamedGroups.GetAt(j).nID == nDoomedID) {
					bRemoved = true;
					m_arRenamedGroups.RemoveAt(j, 1);
				}
			}
			bRemoved = false;
			
			//TES 1/5/2010 - PLID 35774 - OK, now, if it's positive, it needs to be deleted.
			if(nDoomedID > 0) {
				AddStatementToSqlBatch(strSql, "DELETE FROM SecurityGroupDetailsT WHERE SecurityGroupID = %li", nDoomedID);
				AddStatementToSqlBatch(strSql, "DELETE FROM SecurityGroupsT WHERE ID = %li", nDoomedID);
				// (j.armen 2014-01-28 16:47) - PLID 60146 - Make sure we reset the preference for default security group when removing a group
				AddStatementToSqlBatch(strSql, "UPDATE ConfigRT SET IntParam = -1 WHERE Username = '<None>' AND Name = 'DefaultSecurityGroup' AND IntParam = %li", nDoomedID);
				//TES 1/5/2010 - PLID 35774 - Update the permission object.
				DeleteUserDefinedPermission(bioIndivSecurityGroups, nDoomedID, TRUE);
				bSomethingChanged = true;
			}
			else {
				//TES 1/5/2010 - PLID 35774 - If it's negative, it's not in the data yet, so take it out of the array to make sure it doesn't get added later.
				bool bRemoved = false;
				for(int j = 0; j <= m_arAddedGroups.GetUpperBound() && !bRemoved; j++) {
					if(m_arAddedGroups.GetAt(j).nID == nDoomedID) {
						bRemoved = true;
						m_arAddedGroups.RemoveAt(j, 1);
					}
				}
			}
		}
		
		// MSC - 5/30/03 - The Groups that were renamed need to be saved before the new groups are added, otherwise
		//if a group that is added has the same name as a renamed group that had the name before, then it would give a 
		//primary key violation
		if(m_arRenamedGroups.GetSize() > 0) {
			CString strCaseStatements, strIDs;
			for(i = 0; i <= m_arRenamedGroups.GetUpperBound(); i++) {
				long nID = m_arRenamedGroups.GetAt(i).nID;
				CString strCaseStatementToAdd, strIDToAdd;
				CString strName = m_arRenamedGroups.GetAt(i).strName;
				strCaseStatementToAdd.Format(" WHEN ID = %li THEN '%s' ", nID, _Q(strName));
				strCaseStatements += strCaseStatementToAdd;
				
				strIDToAdd.Format("%li,", nID);
				strIDs += strIDToAdd;

				//TES 1/5/2010 - PLID 35774 - Update the permission object.
				UpdateUserDefinedPermissionName(bioIndivSecurityGroups, nID, strName);
			}
				
			DeleteEndingString(strIDs, ",");
			AddStatementToSqlBatch(strSql, "UPDATE SecurityGroupsT SET Name = CASE %s END WHERE ID IN (%s)", strCaseStatements, strIDs);
			bSomethingChanged = true;
		}

		//TES 1/5/2010 - PLID 35774 - Now, store anything that was added (and survived the deleting process.
		AddDeclarationToSqlBatch(strSql, "DECLARE @NewGroupsT TABLE (ID INT, Name nvarchar(100))");
		for(i = 0; i <= m_arAddedGroups.GetUpperBound(); i++) {
			//TES 1/5/2010 - PLID 35774 - Add it, and remember that we did.
			AddStatementToSqlBatch(strSql, "INSERT INTO SecurityGroupsT (Name) VALUES ('%s')", _Q(m_arAddedGroups.GetAt(i).strName));
			AddStatementToSqlBatch(strSql, "INSERT INTO @NewGroupsT (ID, Name) SELECT Convert(int, SCOPE_IDENTITY()), '%s'", _Q(m_arAddedGroups.GetAt(i).strName));
			bSomethingChanged = true;
		}

		AddDeclarationToSqlBatch(strSql, "SET NOCOUNT OFF");
		//TES 1/5/2010 - PLID 35774 - Now, we need to pull all the groups we added so that we can update their permission objects.
		AddStatementToSqlBatch(strSql, "SELECT ID, Name FROM @NewGroupsT");
		_RecordsetPtr rsNewGroups = CreateRecordset("%s", strSql);
		while(!rsNewGroups->eof) {
			//TES 1/5/2010 - PLID 35774 - Add it, with no permission as the default.
			// (j.jones 2010-01-14 13:48) - PLID 35775 - pass in a the dynamic perm name
			AddUserDefinedPermission(AdoFldLong(rsNewGroups, "ID"), sptRead|sptReadWithPass|sptDynamic0|sptDynamic0WithPass, AdoFldString(rsNewGroups, "Name"), 
				"Controls ability to access information about patients who are members of this security group, in the Patients Module and Scheduler Module only.", 
				bioIndivSecurityGroups, 0, "Emergency Access");
			rsNewGroups->MoveNext();
		}

		//TES 1/11/2010 - PLID 36761 - If we changed something, send a tablechecker.
		if(bSomethingChanged) {
			CClient::RefreshTable(NetUtils::SecurityGroupsT, -1);
		}

		CNxDialog::OnOK();

	}NxCatchAll(__FUNCTION__);
}


void CSecurityGroupsConfigDlg::UpdateButtons()
{
	//TES 1/5/2010 - PLID 35774 - If something's selected, we can rename and delete it, otherwise we can't.
	if(m_pGroups->CurSel == NULL) {
		GetDlgItem(IDC_RENAME_GROUP)->EnableWindow(FALSE);
		GetDlgItem(IDC_DELETE_GROUP)->EnableWindow(FALSE);
	}
	else {
		GetDlgItem(IDC_RENAME_GROUP)->EnableWindow(TRUE);
		GetDlgItem(IDC_DELETE_GROUP)->EnableWindow(TRUE);
	}
}

BEGIN_EVENTSINK_MAP(CSecurityGroupsConfigDlg, CNxDialog)
ON_EVENT(CSecurityGroupsConfigDlg, IDC_SECURITY_GROUPS_LIST, 2, CSecurityGroupsConfigDlg::OnSelChangedSecurityGroupsList, VTS_DISPATCH VTS_DISPATCH)
END_EVENTSINK_MAP()

void CSecurityGroupsConfigDlg::OnSelChangedSecurityGroupsList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try {
		//TES 1/5/2010 - PLID 35774 - Reflect whether a group is selected.
		UpdateButtons();
	}NxCatchAll(__FUNCTION__);
}

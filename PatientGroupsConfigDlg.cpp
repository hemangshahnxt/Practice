// PatientGroupsConfigDlg.cpp : implementation file
//

#include "stdafx.h"
#include "patientsrc.h"
#include "PatientGroupsConfigDlg.h"
#include "Filter.h"
#include "Groups.h"
using namespace ADODB;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define COLUMN_ID		0
#define COLUMN_NAME		1

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CPatientGroupsConfigDlg dialog


CPatientGroupsConfigDlg::CPatientGroupsConfigDlg(CWnd* pParent)
	: CNxDialog(CPatientGroupsConfigDlg::IDD, pParent), m_groupChecker(NetUtils::Groups)
{
	//{{AFX_DATA_INIT(CPatientGroupsConfigDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CPatientGroupsConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPatientGroupsConfigDlg)
	DDX_Control(pDX, IDC_NEW, m_btnNew);
	DDX_Control(pDX, IDC_RENAME_GROUP, m_btnRenameGroup);
	DDX_Control(pDX, IDC_DELETE_GROUP, m_btnDeleteGroup);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPatientGroupsConfigDlg, CNxDialog)
	//{{AFX_MSG_MAP(CPatientGroupsConfigDlg)
	ON_BN_CLICKED(IDC_NEW, OnNew)
	ON_BN_CLICKED(IDC_DELETE_GROUP, OnDeleteGroup)
	ON_BN_CLICKED(IDC_RENAME_GROUP, OnRenameGroup)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPatientGroupsConfigDlg message handlers

BEGIN_EVENTSINK_MAP(CPatientGroupsConfigDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CPatientGroupsDlg)
	ON_EVENT(CPatientGroupsConfigDlg, IDC_GROUP_LIST, 10 /* EditingFinished */, OnEditingFinishedGroupList, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CPatientGroupsConfigDlg, IDC_GROUP_LIST, 9 /* EditingFinishing */, OnEditingFinishingGroupList, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CPatientGroupsConfigDlg, IDC_GROUP_LIST, 19 /* LeftClick */, OnLeftClickGroupList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CPatientGroupsConfigDlg, IDC_GROUP_LIST, 2 /* SelChanged */, OnSelChangedGroupList, VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

BOOL CPatientGroupsConfigDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-05-01 12:36) - PLID 29866 - NxIconify buttons
		m_btnNew.AutoSet(NXB_NEW);
		m_btnRenameGroup.AutoSet(NXB_MODIFY);
		m_btnDeleteGroup.AutoSet(NXB_DELETE);
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		CString sql;
		IColumnSettingsPtr pCol;
		
		m_groupList = BindNxDataListCtrl(IDC_GROUP_LIST, true);
		RefreshButtons();
	}
	NxCatchAll("Error in CPatientGroupsConfigDlg::OnInitDialog");

	return TRUE;
}

void CPatientGroupsConfigDlg::OnEditingFinishedGroupList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	//We don't save anything until the end anymore.
}

void CPatientGroupsConfigDlg::OnNew() 
{
	if (!UserPermission(EditGroup))
		return;

	CString groupName;

	if (Prompt("Enter the name for this group:", groupName, 50) != IDOK)
		return;

	long groupID = NewNumber("GroupsT", "ID");

	try
	{
		groupName.TrimRight();
		groupName.TrimLeft();

		if(!IsGroupNameValid(groupName))	{
			return;
		}

		//Just add to the array, for the moment.
		long nNewID;
		if(m_arAddedGroups.GetSize() == 0) {
			nNewID = -1;
		}
		else {
			nNewID = m_arAddedGroups.GetAt(m_arAddedGroups.GetUpperBound()).nID-1;
		}
		IdName NewName;
		NewName.nID = nNewID;
		NewName.strName = groupName;

		//m_arAddedGroups must always have unique nIDs
		m_arAddedGroups.Add(NewName);

		//Add to the list.
		IRowSettingsPtr pRow = m_groupList->GetRow(-1);
		pRow->PutValue(0, nNewID);
		pRow->PutValue(1, _bstr_t(groupName));
		//DRT 3/3/2004 - PLID 11250 - There are only 2 columns in this datalist, I don't know why this line is here.
		//pRow->PutValue(2, false);
		m_groupList->AddRow(pRow);
		
		m_groupChecker.Refresh();

		RefreshButtons();
	}
	NxCatchAll("Could not create new group");
}

void CPatientGroupsConfigDlg::OnCancel() 
{
	CNxDialog::OnCancel();
}

void CPatientGroupsConfigDlg::OnDeleteGroup() 
{
	if (m_groupList->CurSel == -1)
		return;

	if (!UserPermission(EditGroup))
		return;

	long groupID = VarLong(m_groupList->Value[m_groupList->CurSel][COLUMN_ID]);
	//(a.wilson 2011-9-22) PLID 28266 - store the groups name for auditing
	CString strGroupName = VarString(m_groupList->Value[m_groupList->CurSel][COLUMN_NAME]);
	
	// (c.haag 2003-10-01 15:06) - Check to see which filters use this group.
	CString strFilters = "This group is used in the following filters:\r\n\r\n";
	_RecordsetPtr prsFilters = CreateRecordset("SELECT ID, Name, Filter FROM FiltersT WHERE Filter LIKE '%%\"%d\"%%'", groupID);
	BOOL bInFilters = FALSE;
	while (!prsFilters->eof)
	{
		// Get the filter SQL string to filter on
		CString strFilter = AdoFldString(prsFilters, "Filter");
		CString strWhere, strFrom;
		
		// Grab only the WHERE clause from the filter string
		if (CFilter::ConvertFilterStringToClause(AdoFldLong(prsFilters, "ID"), strFilter, fboPerson, &strWhere, &strFrom))
		{
			CString strSearch;
			strSearch.Format("GroupID = %d", groupID);
			if (-1 != strWhere.Find(strSearch, 0))
			{
				bInFilters = TRUE;
				strFilters += AdoFldString(prsFilters, "Name", "") + "\r\n";
				break;
			}
		}
		prsFilters->MoveNext();
	}
	prsFilters->Close();
	if (bInFilters)
	{
		strFilters += "\r\nThese filters must be changed before you can delete this group.";
		if (AfxMessageBox(strFilters))
			return;
	}

	if (IDYES != AfxMessageBox("Are you sure you want to delete this group and remove all patients from it?", MB_YESNO))
		return;

	try
	{	
		//(a.wilson 2011-9-22) PLID 28266
		//Just add it to the array for the moment.
		IdName delGroup;
		delGroup.nID = groupID;
		delGroup.strName = strGroupName;
		m_arDeletedGroups.Add(delGroup);
		m_groupList->RemoveRow(m_groupList->CurSel);

		RefreshButtons();
	}
	NxCatchAll("Couldn't delete group");
}

void CPatientGroupsConfigDlg::OnRenameGroup() 
{
	if (m_groupList->CurSel == -1)
		return;

	if (!UserPermission(EditGroup))
		return;

	CString groupName;

	if (Prompt("Enter a new name for this group:", groupName, 50) != IDOK)
		return;

	try{
		groupName.TrimRight();
		groupName.TrimLeft();
		
		// get the group name we are trying to rename
		CString strOldGroupName = VarString(m_groupList->GetValue(m_groupList->GetCurSel(), 1));
		
		if(!IsGroupNameValid(groupName, strOldGroupName)){
			return;
		}

		long id = VarLong(m_groupList->Value[m_groupList->CurSel][COLUMN_ID]);		
		//Just save to array for now.
		IdName NewName;
		NewName.nID = id;
		NewName.strName = groupName;
		
		//if the group has been newly added, then we just want to rename the group in the AddedGroups array
		if (!RenameElements(m_arAddedGroups, NewName)) {
			//the group wasn't found in the added array so check and see if it's been renamed
			if (!RenameElements(m_arRenamedGroups, NewName)) {
				//the group hasn't been newly renamed so add it to the renamed array
				//m_arRenamedGroups must always have unique nIDs
				m_arRenamedGroups.Add(NewName);
			}
		}
		
		//Update list.
		m_groupList->PutValue(m_groupList->CurSel, 1, _bstr_t(groupName));

		RefreshButtons();
	}
	NxCatchAll("Could not rename group");
}


void CPatientGroupsConfigDlg::OnOK() 
{
	//Store the checkboxes, while we still have a datalist.
	if(!StoreData_Flat())
		return;

	CNxDialog::OnOK();
}

BOOL CPatientGroupsConfigDlg::StoreData_Flat()
{
	//(a.wilson 2011-9-22) PLID 28266 - auditing transaction for group changes
	long nAuditTransactionID = -1;
	try {
		//First, delete all added groups.
		for(int i = 0; i <= m_arDeletedGroups.GetUpperBound(); i++) {
			long nDoomedID = m_arDeletedGroups.GetAt(i).nID;
			//(a.wilson 2011-9-22) PLID 28266
			CString strDoomedName = m_arDeletedGroups.GetAt(i).strName;
			//OK, first, take it out of the renamed and checked arrays.
			bool bRemoved = false;
			for(int j = 0; j <= m_arRenamedGroups.GetUpperBound() && !bRemoved; j++) {
				if(m_arRenamedGroups.GetAt(j).nID == nDoomedID) {
					bRemoved = true;
					m_arRenamedGroups.RemoveAt(j, 1);
				}
			}
			bRemoved = false;
			for(j = 0; j <= m_arCheckedGroupIDs.GetUpperBound() && !bRemoved; j++) {
				if(m_arCheckedGroupIDs.GetAt(j) == nDoomedID) {
					bRemoved = true;
					m_arCheckedGroupIDs.RemoveAt(j, 1);
				}
			}

			//OK, now, if it's positive, it needs to be deleted.
			if(nDoomedID > 0) {
				ExecuteSql("DELETE FROM GroupDetailsT WHERE GroupID = %li", nDoomedID);
				ExecuteSql("DELETE FROM GroupsT WHERE ID = %li", nDoomedID);
				//(a.wilson 2011-9-22) PLID 28266 - auditing the deletion of the group
				if (nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}
				AuditEvent(-1, "", nAuditTransactionID, aeiGroupDeleted, -1, strDoomedName, "<Deleted>", aepMedium, aetDeleted);
			}
			else {
				//If it's negative, it's not in the data yet, so take it out of the array to make sure it doesn't get added later.
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
			}
				
			DeleteEndingString(strIDs, ",");
			//(a.wilson 2011-9-22) PLID 28266 - getting the old names for auditing
			_RecordsetPtr prs = CreateRecordset("SELECT ID, Name FROM GroupsT WHERE ID IN (%s)", strIDs);
			ExecuteSql("UPDATE GroupsT SET Name = CASE %s END WHERE ID IN (%s)", strCaseStatements, strIDs);
			//(a.wilson 2011-9-22) PLID 28266 - auditing the renaming of the groups
			for(int nameLoc = 0; !prs->eof; prs->MoveNext(), nameLoc++) {
				if (nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}
				CString strOldGroupName = AdoFldString(prs->Fields, "Name");
				long nID = AdoFldLong(prs->Fields, "ID");
				AuditEvent(-1, "", nAuditTransactionID, aeiGroup, -1, strOldGroupName, m_arRenamedGroups.GetAt(nameLoc).strName, aepMedium, aetChanged);
			}
			
		}

		//Now, store anything that was added (and survived the deleting process.
		for(i = 0; i <= m_arAddedGroups.GetUpperBound(); i++) {
			//This is the negative ID.
			long nTempID = m_arAddedGroups.GetAt(i).nID;
			//Now, get a real id, and update everything accordingly.
			long nNewID = NewNumber("GroupsT", "ID");
			ASSERT(nNewID >= 0);
			ExecuteSql("INSERT INTO GroupsT (ID, Name) VALUES (%li, '%s')", nNewID, _Q(m_arAddedGroups.GetAt(i).strName));
			//(a.wilson 2011-9-22) PLID 28266 - auditing the new group creation.
			if (nAuditTransactionID == -1) {
				nAuditTransactionID = BeginAuditTransaction();
			}
			AuditEvent(nNewID, "", nAuditTransactionID, aeiGroupCreated, -1, "", _Q(m_arAddedGroups.GetAt(i).strName), aepMedium, aetCreated);
			//Now, go through the checked and renamed arrays, and update their ids.
			for(int j = 0; j <= m_arCheckedGroupIDs.GetUpperBound(); j++) {
				if(m_arCheckedGroupIDs.GetAt(j) == nTempID) {
					m_arCheckedGroupIDs.SetAt(j, nNewID);
				}
			}
			//Same for the renamed ones.
			for(j = 0; j <= m_arRenamedGroups.GetUpperBound(); j++) {
				if(m_arRenamedGroups.GetAt(j).nID == nTempID) {
					IdName NewID = m_arRenamedGroups.GetAt(j);
					NewID.nID = nNewID;
					m_arRenamedGroups.SetAt(j, NewID);
				}
			}

			//OK, done.
		}
		m_groupChecker.Refresh();
		//(a.wilson 2011-9-22) PLID 28266 - commit audits if everything went well.
		if (nAuditTransactionID != -1) {
			CommitAuditTransaction(nAuditTransactionID);
		}

		return TRUE;

	//(a.wilson 2011-9-22) PLID 28266 - updated to rollback in case of error.
	}NxCatchAllCall("Error saving groups.", if (nAuditTransactionID != -1) { RollbackAuditTransaction(nAuditTransactionID); } );

	return FALSE;
}

bool CPatientGroupsConfigDlg::IsGroupNameValid(CString strGroupNameToValidate, CString strOldGroupName /* ="" */)
{
	//check to make sure the name isn't blank
	if(strGroupNameToValidate == "") {
			MessageBox("You cannot enter a group with a blank name.");
			return false;
	}

	// if the new name and the old name are the same, then the name is valid
	if(strGroupNameToValidate.CompareNoCase(strOldGroupName) == 0){
		return true;
	}
	
	//check to make sure the group name isn't duplicated
	long pCurRowEnum = m_groupList->GetFirstRowEnum();
	while(pCurRowEnum != 0){
		IRowSettingsPtr pRow;
		{
			IDispatch *lpDisp;
			m_groupList->GetNextRowEnum(&pCurRowEnum, &lpDisp);
			pRow = lpDisp;
			lpDisp->Release();
			lpDisp = NULL;
		}

		ASSERT(pRow != NULL);
		_variant_t var = pRow->GetValue(1);
		
		CString strGroupName;
		strGroupName = VarString(var);
		
		if(strGroupNameToValidate.CompareNoCase(strGroupName) == 0)
		{
			AfxMessageBox("Group names must be unique");
			return false;
		}
	}
	return true;
}

bool CPatientGroupsConfigDlg::RenameElements(CArray<IdName,IdName> &ary, IN const IdName &NewName)
{
	for(int x = 0; x <= ary.GetUpperBound(); x++)
	{
		if(ary.GetAt(x).nID == NewName.nID)
		{
			ary.SetAt(x, NewName);
			//we can break out of the for loop here because we know that the arrays must have unique IDs
			return true;
		}
	}
	//we didn't find the element in the array so return false
	return false;
}

void CPatientGroupsConfigDlg::OnEditingFinishingGroupList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	switch(nCol) {
	case COLUMN_NAME:
		if(*pbCommit) {
			CString strNewGroupName;
			strNewGroupName = VarString(pvarNewValue);
			CString strOldGroupName;
			strOldGroupName = VarString(varOldValue);
			try{
				strNewGroupName.TrimRight();
				strNewGroupName.TrimLeft();
				
				if(!IsGroupNameValid(strNewGroupName, strOldGroupName)){
					*pbCommit = FALSE;
					*pbContinue = FALSE;
					return;
				}
				else{
					// set pvarNewVale to the value we validated - strNewValue, the only difference between them 
					// is if pvarNewValue was trimmed
					_variant_t varNewValue;
					varNewValue = _bstr_t((LPCTSTR)strNewGroupName);
					(*pvarNewValue) = varNewValue.Detach();

					long id = VarLong(m_groupList->Value[m_groupList->CurSel][COLUMN_ID]);		
					//Just save to array for now.
					IdName NewName;
					NewName.nID = id;
					NewName.strName = strNewGroupName;
					
					//if the group has been newly added, then we just want to rename the group in the AddedGroups array
					if (!RenameElements(m_arAddedGroups, NewName)) {
						//the group wasn't found in the added array so check and see if it's been renamed
						if (!RenameElements(m_arRenamedGroups, NewName)) {
							//the group hasn't been newly renamed so add it to the renamed array
							//m_arRenamedGroups must always have unique nIDs
							m_arRenamedGroups.Add(NewName);
						}
					}
				}
			}NxCatchAll("Error renaming group.");
		}
		break;
	}	
}

void CPatientGroupsConfigDlg::OnLeftClickGroupList(long nRow, short nCol, long x, long y, long nFlags)
{
	// (m.cable 06/14/2004 10:28) - this is now handled in OnSelChangedGroupList
	//RefreshButtons();
}

void CPatientGroupsConfigDlg::RefreshButtons()
{
	if(m_groupList->GetCurSel() == sriNoRow){
		GetDlgItem(IDC_DELETE_GROUP)->EnableWindow(FALSE);
		GetDlgItem(IDC_RENAME_GROUP)->EnableWindow(FALSE);
	}
	else{
		GetDlgItem(IDC_DELETE_GROUP)->EnableWindow(TRUE);
		GetDlgItem(IDC_RENAME_GROUP)->EnableWindow(TRUE);
	}
}

void CPatientGroupsConfigDlg::OnSelChangedGroupList(long nNewSel)
{
	RefreshButtons();
}
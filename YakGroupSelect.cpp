// YakGroupSelect.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "YakGroupSelect.h"
#include "GlobalDataUtils.h"
#include "GlobalUtils.h"
#include "YakGroupEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
using namespace ADODB;
/////////////////////////////////////////////////////////////////////////////
// CYakGroupSelect dialog


CYakGroupSelect::CYakGroupSelect(CWnd* pParent)
	: CNxDialog(CYakGroupSelect::IDD, pParent)
{
	m_nMode = 0;
	//{{AFX_DATA_INIT(CYakGroupSelect)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CYakGroupSelect::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CYakGroupSelect)
	DDX_Control(pDX, IDC_ADD_YAK_GROUP, m_btnAddYakGroup);
	DDX_Control(pDX, IDC_EDIT_YAK_GROUP, m_btnEditYakGroup);
	DDX_Control(pDX, IDC_DELETE_YAK_GROUP, m_btnDeleteYakGroup);
	DDX_Control(pDX, IDOK, m_btnOk);
	//}}AFX_DATA_MAP
}

// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
BEGIN_MESSAGE_MAP(CYakGroupSelect, CNxDialog)
	//{{AFX_MSG_MAP(CYakGroupSelect)
	ON_BN_CLICKED(IDC_ADD_YAK_GROUP, OnAddGroup)
	ON_BN_CLICKED(IDC_EDIT_YAK_GROUP, OnEditGroup)
	ON_BN_CLICKED(IDC_DELETE_YAK_GROUP, OnDeleteYakGroup)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CYakGroupSelect message handlers

void CYakGroupSelect::OnAddGroup() 
{
	CString strGroupName;

	if (Prompt("Enter a name for this group:", strGroupName, 50) != IDOK)
		return;

	long groupID = NewNumber("YakGroupsT", "ID");

	try
	{
		strGroupName.TrimRight();
		strGroupName.TrimLeft();

		// make sure the name is ok
		while(!IsGroupNameValid(strGroupName))	{
			if (Prompt("Enter a name for this group:", strGroupName, 50) != IDOK)
				return;
		}

		
		// ok open the window that allows them to configure the group
		CYakGroupEdit dlgYakGroupEdit(this);
		dlgYakGroupEdit.DoModal(groupID, strGroupName);

		m_groupList->Requery();

	}
	NxCatchAll("Could not create new group");
	
}

void CYakGroupSelect::OnEditGroup() 
{
	try{
		if(m_groupList->GetCurSel() == -1) {
			AfxMessageBox("Please select a group to edit.");
			return;
		}
		
		long nSelectedID = m_groupList->GetValue(m_groupList->GetCurSel(), 0);
		CYakGroupEdit dlgYakGroupEdit(this);
		dlgYakGroupEdit.DoModal(nSelectedID, VarString(m_groupList->GetValue(m_groupList->GetCurSel(), 2)));
	}NxCatchAll("Error Editing Yak Group");
}

void CYakGroupSelect::OnOK() 
{
	// find out which groups are selected and add them to the array of selected groups

	try{
		m_dwGroupsAry.RemoveAll();
		m_strGroupsAry.RemoveAll();

		if(m_nMode == mSelect) {
			for(int i = 0; i < m_groupList->GetRowCount(); i++) {
				// if the group is checked, add it to the array of groups
				_variant_t var = m_groupList->GetValue(i, 1);
				BOOL bIsChecked = VarBool(m_groupList->GetValue(i, 1));
				if(bIsChecked) {
					m_dwGroupsAry.Add((DWORD)VarLong(m_groupList->GetValue(i, 0)));
					m_strGroupsAry.Add(VarString(m_groupList->GetValue(i, 2)));
				}
			}
		}
	}NxCatchAll("Error Adding Groups To Array");
	
	CDialog::OnOK();
}

BOOL CYakGroupSelect::OnInitDialog() 
{
	CNxDialog::OnInitDialog(); // (a.walling 2011-01-14 16:44) - no PLID - Fix bad base class OnInitDialog call
	m_groupList = BindNxDataListCtrl(IDC_YAK_GROUP_LIST);
	long nRow;
	
	m_btnAddYakGroup.AutoSet(NXB_NEW);
	m_btnEditYakGroup.AutoSet(NXB_MODIFY);
	m_btnDeleteYakGroup.AutoSet(NXB_DELETE);
	m_btnOk.AutoSet(NXB_CLOSE);
	
	// if there was something selected before, reselect it
	for(int i = 0; i < m_dwGroupsAry.GetSize(); i++) {
		nRow = m_groupList->FindByColumn(0, (long)m_dwGroupsAry.GetAt(i), 0, FALSE);
		if(nRow >= 0) {
			m_groupList->PutValue(nRow, 1, true);
		}
	}

	// if they are in select mode,  hide the buttons that are for editing groups
	if(m_nMode == mSelect) {
		GetDlgItem(IDC_ADD_YAK_GROUP)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_YAK_GROUP)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_DELETE_YAK_GROUP)->ShowWindow(SW_HIDE);
	}
	else{
		// they are in edit mode, so hide the checkbox column
		IColumnSettingsPtr pCol = m_groupList->GetColumn(1);
		if(pCol) {
			pCol->PutStoredWidth(0);
		}
	}

	if(m_nMode == mSelect){
		this->SetWindowText("Select Yak Groups");
	}
	else if (m_nMode == mEdit) {
		this->SetWindowText("Edit Yak Groups");
	}
	else{
		// MSC - 2-25-04 - This should never happen since these are the only two modes right now
		ASSERT(FALSE);
	}

	// (c.haag 2006-05-16 12:07) - PLID 20621 - Set the user-defined color of the window
	((CNxColor*)GetDlgItem(IDC_NXCOLORCTRL1))->SetColor(GetRemotePropertyInt("YakWindowColor", 0x00FF8080, 0, GetCurrentUserName(), true));

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CYakGroupSelect::OnDeleteYakGroup() 
{
	if(m_groupList->GetCurSel() == -1) {
		AfxMessageBox("Please select a group to delete.");
		return;
	}
	
	if(IDNO == MessageBox("Are you sure you wish to delete this group?", "Practice", MB_YESNO)){
		return;
	}

	try{
		long nCurrentSelected = m_groupList->GetCurSel();
		long nGroupID = m_groupList->GetValue(nCurrentSelected, 0);

		// (j.jones 2008-06-02 11:48) - PLID 30219 - clear the default room manager yak group preference
		_RecordsetPtr rs = CreateParamRecordset("SELECT Username FROM ConfigRT WHERE Name = 'RoomMgrReadyCheckout_PracYakGroupID' AND IntParam = {INT}", nGroupID);
		while(!rs->eof) {
			//have to call SetRemotePropertyInt to update the cache
			SetRemotePropertyInt("RoomMgrReadyCheckout_PracYakGroupID", -1, 0, AdoFldString(rs, "Username",""));
			rs->MoveNext();
		}
		rs->Close();

		CString strSql;
		strSql.Format("DELETE FROM YakGroupDetailsT WHERE GroupID = %li", nGroupID);
		ExecuteSql("%s", strSql);
		strSql.Format("DELETE FROM YakGroupsT WHERE ID = %li", nGroupID);
		ExecuteSql("%s", strSql);
		m_groupList->Requery();
	}NxCatchAll("Error Deleting Yak Group");
}

bool CYakGroupSelect::IsGroupNameValid(CString strGroupNameToValidate, CString strOldGroupName /* ="" */)
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
	
	try{
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
			_variant_t var = pRow->GetValue(2);
			
			CString strGroupName;
			strGroupName = VarString(var);
			
			if(strGroupNameToValidate.CompareNoCase(strGroupName) == 0)
			{
				AfxMessageBox("Group names must be unique");
				return false;
			}
		}
	}NxCatchAll("Error Validating Group Name");
	return true;
}

// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
BEGIN_EVENTSINK_MAP(CYakGroupSelect, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CYakGroupSelect)
	ON_EVENT(CYakGroupSelect, IDC_YAK_GROUP_LIST, 3 /* DblClickCell */, OnDblClickCellYakGroupList, VTS_I4 VTS_I2)
	ON_EVENT(CYakGroupSelect, IDC_YAK_GROUP_LIST, 9 /* EditingFinishing */, OnEditingFinishingYakGroupList, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CYakGroupSelect::OnDblClickCellYakGroupList(long nRowIndex, short nColIndex) 
{
	if(m_nMode == mSelect){
		// if they are in select mode
		// then send to the group they double clicked on 
		m_dwGroupsAry.Add((DWORD)VarLong(m_groupList->GetValue(nRowIndex, 0)));
		m_strGroupsAry.Add(VarString(m_groupList->GetValue(nRowIndex, 2)));
		CDialog::OnOK();
	}	
	else if(m_nMode == mEdit){
		// they are in edit mode
		// so let them edit the group they double clicked on
		OnEditGroup();
	}
	else{
		// invalid mode
		// nothing horrendous, but we shouldn't be in this situation

		ASSERT(FALSE);
	}
}

int CYakGroupSelect::DoModal(long nMode /* = 0 */) 
{
	// set the mode
	m_nMode = nMode;
			
	return CDialog::DoModal();
}



void CYakGroupSelect::OnEditingFinishingYakGroupList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	if(nCol != 2){
		return;
	}
	
	if(!IsGroupNameValid(VarString(pvarNewValue), VarString(varOldValue))) {
		*pbCommit = FALSE;
		*pbContinue = FALSE;
		return;
	}
	else{
		CString strSql;
		strSql.Format("UPDATE YakGroupsT SET GroupName = '%s' WHERE GroupName = '%s'", _Q(VarString(pvarNewValue)), _Q(VarString(varOldValue)));
		ExecuteSql("%s", strSql);
	}
}

//DRT 6/2/2008 - PLID 30230 - Added OnCancel handler to keep behavior the same as pre-NxDialog changes
void CYakGroupSelect::OnCancel()
{
	//Eat the message
}

// PatientGroupsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PatientGroupsDlg.h"
#include "GlobalDataUtils.h"
#include "client.h"
#include "GlobalUtils.h"
#include "PatientGroupsConfigDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (j.armen 2012-04-25 13:04) - PLID 49600 - Since I changed the column order, 
//	I created an enum to use here and made sure that everything in this dlg uses it
namespace GroupList {
	enum {
		ID = 0,
		Check = 1,
		Name = 2,
	};
};

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CPatientGroupsDlg dialog

using namespace ADODB;

CPatientGroupsDlg::CPatientGroupsDlg(CWnd* pParent)
: CNxDialog(CPatientGroupsDlg::IDD, pParent), m_groupChecker(NetUtils::Groups)
{
	//{{AFX_DATA_INIT(CPatientGroupsDlg)
	m_bAutoWriteToData = true;
	//}}AFX_DATA_INIT
}


void CPatientGroupsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPatientGroupsDlg)
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_CONFIG_GROUPS, m_btnConfigGroups);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPatientGroupsDlg, CNxDialog)
	//{{AFX_MSG_MAP(CPatientGroupsDlg)
	ON_BN_CLICKED(IDC_CONFIG_GROUPS, OnConfigGroups)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPatientGroupsDlg message handlers

BEGIN_EVENTSINK_MAP(CPatientGroupsDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CPatientGroupsDlg)
	ON_EVENT(CPatientGroupsDlg, IDC_GROUP_LIST, 10 /* EditingFinished */, OnEditingFinishedGroupList, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

BOOL CPatientGroupsDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-05-01 12:39) - PLID 29866 - NxIconify buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnConfigGroups.AutoSet(NXB_MODIFY);

		CString sql;
		IColumnSettingsPtr pCol;
		
		m_groupList = BindNxDataListCtrl(IDC_GROUP_LIST, false);

		pCol = m_groupList->GetColumn(GroupList::Check);
		if(m_bAutoWriteToData) {
			sql.Format (
				"CASE WHEN ("
					"SELECT TOP 1 ID FROM GroupDetailsT "
					"WHERE GroupID = GroupsT.ID "
					"AND PersonID = %i"
					") IS NULL THEN 0 ELSE 1 END", m_nPatID);
			
			pCol->FieldName = _bstr_t(sql);
		}
		else {
			pCol->FieldName = _bstr_t("0");
		}

		//	m_groupList->something = sql
		m_groupList->Requery();

		//DRT 1/30/03 - If they open the Groups dialog a second time, we need to check all the boxes that are selected
		try {
			for(int i = 0; i < m_arCheckedGroupIDs.GetSize(); i++) {
				int nData = m_arCheckedGroupIDs.GetAt(i);
				
				//find the row it's in
				long nRow = m_groupList->FindByColumn(GroupList::ID, long(nData), 0, false);

				//check the box in that row
				m_groupList->PutValue(nRow, GroupList::Check, true);
			}
		}
		NxCatchAll("Could not update procedure list.");


		GetMainFrame()->DisableHotKeys();
	}
	NxCatchAll("Error in CPatientGroupsDlg::OnInitDialog");

	return TRUE;
}

void CPatientGroupsDlg::OnEditingFinishedGroupList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	//We don't save anything until the end anymore.
}


void CPatientGroupsDlg::OnCancel() 
{
	GetMainFrame()->EnableHotKeys();

	CDialog::OnCancel();
}

void CPatientGroupsDlg::UpdateCheckedArray()
{
	try
	{
		if(m_groupList == NULL)
			return;

		//Update the procedures list.
		long	i = 0,
				p = m_groupList->GetFirstRowEnum();

		LPDISPATCH pDisp = NULL;

		m_arCheckedGroupIDs.RemoveAll();

		while (p)
		{
			m_groupList->GetNextRowEnum(&p, &pDisp);

			IRowSettingsPtr pRow(pDisp);

			BOOL bChecked = FALSE;

			_variant_t var = pRow->GetValue(GroupList::Check);
			if(var.vt == VT_I4 && var.lVal != 0)
				bChecked = TRUE;
			else if(var.vt == VT_BOOL && var.boolVal)
				bChecked = TRUE;

			if (bChecked)
				m_arCheckedGroupIDs.Add(VarLong(pRow->GetValue(GroupList::ID)));
			pDisp->Release();
		}
	}
	NxCatchAll("Could not update procedure list.");
}

void CPatientGroupsDlg::OnOK() 
{
	//Store the checkboxes, while we still have a datalist.
	UpdateCheckedArray();
	if(m_bAutoWriteToData) {
		if(!StoreData_Flat())
			return;
	}

	GetMainFrame()->EnableHotKeys();
	CDialog::OnOK();
}

BOOL CPatientGroupsDlg::StoreData_Flat()
{ 
	try {
		//Now, store all the checkboxes, since the checked array should contain only valid IDs.
		//First, "uncheck" everything.
		ExecuteSql("DELETE FROM GroupDetailsT WHERE PersonID = %li", m_nPatID);
		//Now, add in all the checked ones.
		for(int i = 0; i <= m_arCheckedGroupIDs.GetUpperBound(); i++) {
			//Only if the row still exists (it could have been deleted).
			if(m_groupList->FindByColumn(GroupList::ID, (long)m_arCheckedGroupIDs.GetAt(i), 0, FALSE) != -1)
				ExecuteSql("INSERT INTO GroupDetailsT (GroupID, PersonID) VALUES (%li, %li)", m_arCheckedGroupIDs.GetAt(i), m_nPatID);
		}
		//That ought to do it.

		return TRUE;

	}NxCatchAll("Error saving groups.");

	return FALSE;
}
bool IdInArray(int nID, const CArray<int,int> &ar) {
	for(int i = 0; i < ar.GetSize(); i++) {
		if(ar.GetAt(i) == nID) return true;
	}
	return false;
}
void CPatientGroupsDlg::Refresh()
{
	try {
		if(m_groupList == NULL)
			return;

		//First, write any checkboxes on screen to our array.
		UpdateCheckedArray();
		//Now, requery the list of groups.
		//Don't show them the outdated checkboxes which are about to appear.
		m_groupList->SetRedraw(FALSE);
		m_groupList->Requery();
		m_groupList->WaitForRequery(dlPatienceLevelWaitIndefinitely);
		//Now, go through and check each box according to our array.
		long p = m_groupList->GetFirstRowEnum();
		LPDISPATCH pDisp = NULL;
		while (p)
		{
			m_groupList->GetNextRowEnum(&p, &pDisp);

			IRowSettingsPtr pRow(pDisp);

			if(IdInArray(VarLong(pRow->GetValue(GroupList::ID)), m_arCheckedGroupIDs)) {
				pRow->PutValue(GroupList::Check, (long)1);
			}
			else {
				pRow->PutValue(GroupList::Check, (long)0);
			}
			pDisp->Release();
		}
		//Now, show the list.
		m_groupList->SetRedraw(TRUE);

	}NxCatchAll("Error in CPatientGroupsDlg::Refresh()");
}

void CPatientGroupsDlg::OnConfigGroups() 
{
	CPatientGroupsConfigDlg dlg(this);
	if(IDOK == dlg.DoModal()) {
		Refresh();
	}
}

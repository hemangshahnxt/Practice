// GroupSecurityDlg.cpp : implementation file
//

#include "stdafx.h"
#include "contacts.h"
#include "GroupSecurityDlg.h"
#include "UserGroupSecurityDlg.h"
#include "NewPermissionTemplateDlg.h"

#define COL_GROUPID		0
#define COL_NAME		1
#define COL_DESCRIPTION	2

using namespace ADODB;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CGroupSecurityDlg dialog


CGroupSecurityDlg::CGroupSecurityDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CGroupSecurityDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGroupSecurityDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CGroupSecurityDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGroupSecurityDlg)
	DDX_Control(pDX, IDC_BTN_NEW_TEMPLATE, m_btnNewTemplate);
	DDX_Control(pDX, IDC_BTN_DELETE_TEMPLATE, m_btnDeleteTemplate);
	DDX_Control(pDX, IDOK, m_btnClose);
	DDX_Control(pDX, IDC_BTN_EDITGROUPPERMISSIONS, m_btnEditTemplatePermissions);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGroupSecurityDlg, CNxDialog)
	//{{AFX_MSG_MAP(CGroupSecurityDlg)
	ON_BN_CLICKED(IDC_BTN_EDITGROUPPERMISSIONS, OnBtnEditGroupPermissions)
	ON_BN_CLICKED(IDC_BTN_GROUPSECURITY_HELP, OnBtnHelp)
	ON_BN_CLICKED(IDC_BTN_NEW_TEMPLATE, OnBtnNewTemplate)
	ON_BN_CLICKED(IDC_BTN_DELETE_TEMPLATE, OnBtnDeleteTemplate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGroupSecurityDlg message handlers

BOOL CGroupSecurityDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();
		
		m_btnNewTemplate.AutoSet(NXB_NEW);
		m_btnDeleteTemplate.AutoSet(NXB_DELETE);
		m_btnClose.AutoSet(NXB_CLOSE);
		m_btnEditTemplatePermissions.AutoSet(NXB_MODIFY);
		
		// Create the datalist and fill it with groups
		m_dlGroups = BindNxDataListCtrl(this, IDC_MEMBEROF_LIST, GetRemoteData(), true);

		GetDlgItem(IDC_BTN_EDITGROUPPERMISSIONS)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_DELETE_TEMPLATE)->EnableWindow(FALSE);

	} NxCatchAll(__FUNCTION__);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

int CGroupSecurityDlg::Open(long nPersonID)
{
	m_lPersonID = nPersonID;
	return DoModal();
}

void CGroupSecurityDlg::OnBtnEditGroupPermissions() 
{
	try {
		CUserGroupSecurityDlg dlg(this);
		long lRow = m_dlGroups->GetCurSel();
		if (lRow != -1)
		{
			dlg.Open(VarLong(m_dlGroups->GetValue(lRow, COL_GROUPID)), TRUE);
		}
	} NxCatchAll(__FUNCTION__);
}

BEGIN_EVENTSINK_MAP(CGroupSecurityDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CGroupSecurityDlg)
	ON_EVENT(CGroupSecurityDlg, IDC_MEMBEROF_LIST, 2 /* SelChanged */, OnSelChangedMemberofList, VTS_I4)
	ON_EVENT(CGroupSecurityDlg, IDC_MEMBEROF_LIST, 10 /* EditingFinished */, OnEditingFinishedMemberofList, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CGroupSecurityDlg, IDC_MEMBEROF_LIST, 3 /* DblClickCell */, OnDblClickCellMemberofList, VTS_I4 VTS_I2)
	ON_EVENT(CGroupSecurityDlg, IDC_MEMBEROF_LIST, 9 /* EditingFinishing */, OnEditingFinishingMemberofList, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CGroupSecurityDlg::OnSelChangedMemberofList(long nNewSel) 
{
	try {
		if (m_dlGroups->GetCurSel() != -1)
		{
			GetDlgItem(IDC_BTN_EDITGROUPPERMISSIONS)->EnableWindow(TRUE);
			GetDlgItem(IDC_BTN_DELETE_TEMPLATE)->EnableWindow(TRUE);
		}
		else
		{
			GetDlgItem(IDC_BTN_EDITGROUPPERMISSIONS)->EnableWindow(FALSE);
			GetDlgItem(IDC_BTN_DELETE_TEMPLATE)->EnableWindow(FALSE);
		}
	} NxCatchAll(__FUNCTION__);
}

void CGroupSecurityDlg::OnEditingFinishedMemberofList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try {
		switch (nCol)
		{
		case COL_NAME:
			ExecuteSql("UPDATE UserGroupsT SET Name = '%s' WHERE PersonID = %d",
				_Q(VarString(varNewValue)), VarLong(m_dlGroups->GetValue(nRow, COL_GROUPID)));
			break;

		case COL_DESCRIPTION:
			ExecuteSql("UPDATE UserGroupsT SET Description = '%s' WHERE PersonID = %d",
				_Q(VarString(varNewValue)), VarLong(m_dlGroups->GetValue(nRow, COL_GROUPID)));
			break;
		}
		// (j.gruber 2010-08-17 10:06) - PLID 40138 - change template to groups
	} NxCatchAll("Error saving group name or description");
}

void CGroupSecurityDlg::OnBtnHelp() 
{
	try {
		//(j.anspach 06-09-2005 10:53 PLID 16662) - Updating the OpenManual call to work with the new help system.
		OpenManual("NexTech_Practice_Manual.chm", "System_Setup/User_Setup/configure_permission_templates.htm");
	} NxCatchAll(__FUNCTION__);
}

void CGroupSecurityDlg::OnBtnNewTemplate() 
{
	try {
		CNewPermissionTemplateDlg dlg(this);
		if (IDOK == dlg.DoModal())
		{
			m_dlGroups->Requery();
			m_dlGroups->CurSel = 0;
		}
	} NxCatchAll(__FUNCTION__);
}

void CGroupSecurityDlg::OnBtnDeleteTemplate() 
{
	try {
		CString str;
		// (j.gruber 2010-08-17 10:06) - PLID 40138 - change template to groups
		str.Format("Are you sure you wish to delete the %s group?",
			VarString(m_dlGroups->GetValue( m_dlGroups->CurSel, COL_NAME )));
		if (IDNO == MsgBox(MB_YESNO, str))
			return;

		long lTemplateID = VarLong(m_dlGroups->GetValue( m_dlGroups->CurSel, COL_GROUPID ));

		// (j.gruber 2010-04-14 12:45) - PLID 37948 - make sure they don't have any users associated
		_RecordsetPtr rsUsers = CreateParamRecordset("SELECT UserID FROM UserGroupDetailsT WHERE GroupID = {INT}", lTemplateID);
		if (!rsUsers->eof) {
			MsgBox("There are users associated with this group.  Please remove all users from this group before deleting.");
			return;
		}

		ExecuteSql("DELETE FROM PermissionT WHERE UserGroupID = %d", lTemplateID);
		ExecuteSql("DELETE FROM UserGroupDetailsT WHERE GroupID = %d", lTemplateID);
		ExecuteSql("DELETE FROM UserGroupsT WHERE PersonID = %d", lTemplateID);
		ExecuteSql("DELETE FROM PersonT WHERE ID = %d", lTemplateID);
		m_dlGroups->Requery();
		m_dlGroups->CurSel = 0;
		// (j.gruber 2010-08-17 10:06) - PLID 40138 - change template to groups
	} NxCatchAll("Error deleting group");
}

void CGroupSecurityDlg::OnDblClickCellMemberofList(long nRowIndex, short nColIndex) 
{
	try {
		CUserGroupSecurityDlg dlg(this);
		dlg.Open(VarLong(m_dlGroups->GetValue(nRowIndex, COL_GROUPID)), TRUE);
	} NxCatchAll(__FUNCTION__);
}

void CGroupSecurityDlg::OnEditingFinishingMemberofList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	try {
		switch (nCol)
		{
		case COL_NAME:
			if(!IsRecordsetEmpty("SELECT PersonID FROM UserGroupsT WHERE Name = '%s' AND PersonID <> %li",_Q(strUserEntered),m_dlGroups->GetValue(nRow,0).lVal)) {
				*pbCommit = FALSE;
				*pbContinue = FALSE;
				// (j.gruber 2010-08-17 10:06) - PLID 40138 - change template to groups
				MsgBox("There is already a group with this name. Please enter a different name.");
			}
			break;
		}
		// (j.gruber 2010-08-17 10:06) - PLID 40138 - change template to groups
	} NxCatchAll("Error saving group name or description");
}

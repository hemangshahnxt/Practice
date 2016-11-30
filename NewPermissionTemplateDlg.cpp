// NewPermissionTemplateDlg.cpp : implementation file
//

#include "stdafx.h"
#include "contacts.h"
#include "NewPermissionTemplateDlg.h"
#include "copypermissionsdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;

/////////////////////////////////////////////////////////////////////////////
// CNewPermissionTemplateDlg dialog


CNewPermissionTemplateDlg::CNewPermissionTemplateDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CNewPermissionTemplateDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNewPermissionTemplateDlg)
	m_strName = _T("");
	m_strDescription = _T("");
	//}}AFX_DATA_INIT
}


void CNewPermissionTemplateDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNewPermissionTemplateDlg)
	DDX_Text(pDX, IDC_EDIT_NEWPERMTEMP_NAME, m_strName);
	DDV_MaxChars(pDX, m_strName, 255);
	DDX_Text(pDX, IDC_EDIT_NEWPERMTEMP_DESCRIPTION, m_strDescription);
	DDV_MaxChars(pDX, m_strDescription, 1024);
	DDX_Control(pDX, IDC_EDIT_NEWPERMTEMP_NAME, m_nxeditEditNewpermtempName);
	DDX_Control(pDX, IDC_EDIT_NEWPERMTEMP_DESCRIPTION, m_nxeditEditNewpermtempDescription);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNewPermissionTemplateDlg, CNxDialog)
	//{{AFX_MSG_MAP(CNewPermissionTemplateDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNewPermissionTemplateDlg message handlers

BOOL CNewPermissionTemplateDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);
	
	m_dlItems = BindNxDataListCtrl(this, IDC_LIST_NPT_COPYFROM, GetRemoteData(), true);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CNewPermissionTemplateDlg::OnOK() 
{
	UpdateData(TRUE);
	m_strName.TrimLeft();
	m_strName.TrimRight();
	if (m_strName.IsEmpty())
	{
		// (j.gruber 2010-08-17 10:06) - PLID 40138 - change template to groups
		MsgBox("You must enter a name for this group");
		GetDlgItem(IDC_EDIT_NEWPERMTEMP_NAME)->SetFocus();
		return;
	}

	if(!IsRecordsetEmpty("SELECT PersonID FROM UserGroupsT WHERE Name = '%s'",_Q(m_strName))) {
		// (j.gruber 2010-08-17 10:06) - PLID 40138 - change template to groups
		MsgBox("There is already a group with this name. Please enter a different name.");
		GetDlgItem(IDC_EDIT_NEWPERMTEMP_NAME)->SetFocus();
		return;
	}

	BeginWaitCursor();
	try {
		long nTemplateID = NewNumber("PersonT", "ID");

		CString strSql = BeginSqlBatch();

		// Create the template
		AddStatementToSqlBatch(strSql, "INSERT INTO PersonT (ID) VALUES (%d)", nTemplateID);
		AddStatementToSqlBatch(strSql, "INSERT INTO UserGroupsT (PersonID, Name, Description) VALUES (%d, '%s', '%s')",
			nTemplateID, _Q(m_strName), _Q(m_strDescription));

		// Add the permissions
		for (long i=0; i < m_dlItems->GetRowCount(); i++)
		{
			if (m_dlItems->GetValue(i, 1).boolVal)
			{
				CCopyPermissionsDlg::CopyPermissions(VarLong(m_dlItems->GetValue(i, 0)), nTemplateID, strSql);
			}
		}

		if(!strSql.IsEmpty()) {
			ExecuteSqlBatch(strSql);
		}

	// (j.gruber 2010-08-17 10:06) - PLID 40138 - change template to groups
	} NxCatchAll("Failed to create the permission group");
	EndWaitCursor();
	
	CNxDialog::OnOK();
}

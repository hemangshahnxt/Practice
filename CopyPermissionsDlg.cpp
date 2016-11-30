// CopyPermissionsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "contacts.h"
#include "CopyPermissionsDlg.h"
#include "AuditTrail.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
using namespace NXDATALISTLib;

// (a.walling 2010-01-21 16:43) - PLID 37021 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.


/////////////////////////////////////////////////////////////////////////////
// CCopyPermissionsDlg dialog


CCopyPermissionsDlg::CCopyPermissionsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CCopyPermissionsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCopyPermissionsDlg)
	m_nUserGroup = 0;
	m_pmapPermChanges = NULL;
	m_padwUserGroups = NULL;
	m_bAdministratorSet = FALSE;
	//}}AFX_DATA_INIT
}


void CCopyPermissionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCopyPermissionsDlg)
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCopyPermissionsDlg, CNxDialog)
	//{{AFX_MSG_MAP(CCopyPermissionsDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCopyPermissionsDlg message handlers

/* Import permissions from one user/template to another user/template through
data writes. */
HRESULT CCopyPermissionsDlg::CopyPermissions(long lSrcUserGroup, long lDstUserGroup, CString &strSql)
{
	try 
	{
		_RecordsetPtr rs = CreateRecordset("SELECT * FROM (SELECT PermissionT.Permissions, "
			"PermissionT.ObjectID FROM UserGroupsT LEFT OUTER JOIN PermissionT ON "
			"UserGroupsT.PersonID = PermissionT.UserGroupID WHERE PersonID = %d) SubQ1 "
			"UNION "
			"SELECT * FROM (SELECT PermissionT.Permissions, "
			"PermissionT.ObjectID FROM PermissionT LEFT OUTER JOIN UsersT ON "
			"UsersT.PersonID = PermissionT.UserGroupID WHERE PersonID = %d) SubQ2",
			lSrcUserGroup, lSrcUserGroup);

		while (!rs->eof)
		{
			if (rs->Fields->Item["Permissions"]->Value.vt != VT_NULL)
			{
				long nPermissions = AdoFldLong(rs, "Permissions");
				long nObjectID = AdoFldLong(rs, "ObjectID");

				// (j.jones 2006-05-18 10:15) - you can copy multiple groups at once so the check
				// for existing permissions must be within the batched sql statement (faster anyways)

				CString strElse = "";
				if(nPermissions != 0) {
					strElse.Format("ELSE "
						"BEGIN "
						"INSERT INTO PermissionT (UserGroupID, ObjectID, Permissions) VALUES (%d, %d, %d) "
						"END ",
						lDstUserGroup, nObjectID, nPermissions);
				}
				//(e.lally 2006-08-15) PLID 21977 - We need to convert the bit wise OR operation to BIGINTs then back to INT to avoid an error
				//when a carry occurrs that does not have an open space for it.
				AddStatementToSqlBatch(strSql, "IF EXISTS (SELECT Permissions FROM PermissionT WHERE UserGroupID = %d AND ObjectID = %d) "
					"BEGIN "
					"UPDATE PermissionT "
					"SET Permissions = CONVERT(INT, CONVERT(BIGINT, Permissions) | CONVERT(BIGINT, %d)) "
					"WHERE UserGroupID = %d AND ObjectID = %d "
					"END "
					"%s ",
					lDstUserGroup, nObjectID, nPermissions, lDstUserGroup, nObjectID, strElse);				
			}
			rs->MoveNext();
		}

		return 0;
	}
	NxCatchAll_NoParent("Error copying permissions"); // (a.walling 2014-05-05 13:32) - PLID 61945

	return -1;
}

/* Import permissions from one user/template to another user/template WITHOUT DATA
WRITES; but rather, putting the results in a map */
HRESULT CCopyPermissionsDlg::CopyPermissions(long lSrcUserGroup, long lDstUserGroup, CMap<EBuiltInObjectIDs,EBuiltInObjectIDs,DWORD,DWORD>* pmapPermChanges)
{
	try 
	{
		_RecordsetPtr rs = CreateRecordset("SELECT * FROM (SELECT PermissionT.Permissions, "
			"PermissionT.ObjectID FROM UserGroupsT LEFT OUTER JOIN PermissionT ON "
			"UserGroupsT.PersonID = PermissionT.UserGroupID WHERE PersonID = %d) SubQ1 "
			"UNION "
			"SELECT * FROM (SELECT PermissionT.Permissions, "
			"PermissionT.ObjectID FROM PermissionT LEFT OUTER JOIN UsersT ON "
			"UsersT.PersonID = PermissionT.UserGroupID WHERE PersonID = %d) SubQ2",
			lSrcUserGroup, lSrcUserGroup);
		while (!rs->eof)
		{
			if (rs->Fields->Item["Permissions"]->Value.vt != VT_NULL)
			{
				unsigned long dwPerms;
				if (!pmapPermChanges->Lookup((EBuiltInObjectIDs)AdoFldLong(rs, "ObjectID"), dwPerms))
				{
					// This security object does not exist in the map; lets add it
					pmapPermChanges->SetAt((EBuiltInObjectIDs)AdoFldLong(rs, "ObjectID"), AdoFldLong(rs, "Permissions"));
				}
				else
				{
					// Logically OR the permissions of this security object with the existing permissions
					pmapPermChanges->SetAt((EBuiltInObjectIDs)AdoFldLong(rs, "ObjectID"), AdoFldLong(rs, "Permissions") | dwPerms);
				}
			}
			rs->MoveNext();
		}
		return 0;
	}
	NxCatchAll_NoParent("Error copying permissions");
	return -1;
}

/* Import permissions from several user/teampltes to another user/template */
HRESULT CCopyPermissionsDlg::CopyPermissions(CDWordArray* padwSrcUserGroup, long lDstUserGroup)
{
	CString strSql = BeginSqlBatch();

	for (int i=0; i < padwSrcUserGroup->GetSize(); i++)
	{
		CopyPermissions(padwSrcUserGroup->GetAt(i), lDstUserGroup, strSql);
	}

	// (j.jones 2006-05-18 10:01) - only this state requires writing to data in this function
	if(!strSql.IsEmpty()) {

		ExecuteSqlBatch(strSql);

		_RecordsetPtr rsUsers = CreateRecordset("SELECT UserName FROM UsersT WHERE PersonID = %li",lDstUserGroup);
		//if it is a group rather than a user, we don't need to audit
		if(!rsUsers->eof) {
			long AuditID = BeginNewAuditEvent();
			AuditEvent(-1, AdoFldString(rsUsers, "UserName",""),AuditID,aeiUserPermissionChanged,lDstUserGroup,"","Permissions Copied",aepMedium,aetChanged);
		}
		rsUsers->Close();
	}

	return 0;
}

BOOL CCopyPermissionsDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	m_dlItems = BindNxDataListCtrl(this, IDC_LIST_CP_COPYFROM, GetRemoteData(), true);
	
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);

	/* If we have a pointer to an array users/templates, fill our list
	with checkboxes	to correspond with that array */
	if (m_padwUserGroups)
	{
		m_dlItems->WaitForRequery(dlPatienceLevelWaitIndefinitely);
		for (int i=0; i < m_padwUserGroups->GetSize(); i++)
		{
			for (int j=0; j < m_dlItems->GetRowCount(); j++)
			{
				if (VarLong(m_dlItems->GetValue(j,0)) == (long)m_padwUserGroups->GetAt(i))
				{
					COleVariant v;
					v.vt = VT_BOOL;
					v.boolVal = TRUE;
					m_dlItems->PutValue(j,1,v);
				}
			}
		}
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCopyPermissionsDlg::OnOK() 
{
	CWaitCursor pWait;

	try {

		//JMJ - 6/20/2003 - we originally skipped this block of code if they were already an administrator BUT
		//they can't get into this dialog if they are an administrator so it was unnecessary. I am putting this comment
		//here because having this unnecessary check caused problems in one circumstance:
		//If you edited an admin user, removed admin status, which gave you the ability to copy permissions,
		//you could copy the perms of an admin user, but not get prompted, because the fact that you removed their admin
		//status didn't get saved yet. Yes, it's retarded, but it proves that we don't need this check.

		//first check for administrator users, but only if the permissions we are editing are for a non-administrative user
		//if(!IsRecordsetEmpty("SELECT PersonID FROM UsersT WHERE Administrator = 0 AND PersonID = %li",m_nUserGroup)) {

			//if we find an administrator, we want to prompt the user about it, but only once
			BOOL bAdministratorFound = FALSE;

			// (a.walling 2007-11-05 15:18) - PLID 27977 - VS2008 - for() loops
			long i = 0;

			for (i=0; i < m_dlItems->GetRowCount(); i++) {

				if (m_dlItems->GetValue(i, 1).boolVal) {
					long nUserGroupID = VarLong(m_dlItems->GetValue(i, 0));

					if(!bAdministratorFound && !IsRecordsetEmpty("SELECT PersonID FROM UsersT WHERE Administrator = 1 AND PersonID = %li",nUserGroupID)) {
						//we are copying from an administrator user, prompt to see if they want to copy this status
						bAdministratorFound = TRUE;
						CString str;
						str.Format("At least one user you are copying from is an Administrator user. Do you want to copy this status as well?\n"
							"(If you click 'No', you will still copy the user's regular permissions, and not their administrative status.");
						int nResult = MessageBox(str,"Practice",MB_ICONQUESTION|MB_YESNOCANCEL);
						if(nResult == IDYES) {
							//tell the parent dialog that we will be setting the administrator status
							m_bAdministratorSet = TRUE;
						}
						else if(nResult == IDNO){
							//do nothing now, continue normally
						}
						else if(nResult == IDCANCEL) {
							//return before we do any permission copying
							return;
						}
					}
				}
			}
		//}

		CString strSql = BeginSqlBatch();

		// Add the permissions
		for (i=0; i < m_dlItems->GetRowCount(); i++)
		{
			if (m_dlItems->GetValue(i, 1).boolVal)
			{
				long nUserGroupID = VarLong(m_dlItems->GetValue(i, 0));			

				if (m_pmapPermChanges)
					CopyPermissions(nUserGroupID, m_nUserGroup, m_pmapPermChanges);
				else if (m_padwUserGroups)
					m_padwUserGroups->Add(nUserGroupID);
				else
					CopyPermissions(nUserGroupID, m_nUserGroup, strSql);
			}
		}

		// (j.jones 2006-05-18 10:01) - only this state requires writing to data in this function
		if(!m_pmapPermChanges && !m_padwUserGroups && !strSql.IsEmpty()) {

			ExecuteSqlBatch(strSql);

			_RecordsetPtr rsUsers = CreateRecordset("SELECT UserName FROM UsersT WHERE PersonID = %li",m_nUserGroup);
			//if it is a group rather than a user, we don't need to audit
			if(!rsUsers->eof) {
				long AuditID = BeginNewAuditEvent();
				AuditEvent(-1, AdoFldString(rsUsers, "UserName",""),AuditID,aeiUserPermissionChanged,m_nUserGroup,"","Permissions Copied",aepMedium,aetChanged);
			}
			rsUsers->Close();
		}

	// (j.gruber 2010-08-17 10:48) - PLID 40138 -change all references from permission templates to groups
	} NxCatchAll("Failed to create the permissions group");
	
	CNxDialog::OnOK();
}

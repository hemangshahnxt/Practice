// UserGroupSecurityDlg.cpp : implementation file
//

#include "stdafx.h"
#include "contacts.h"
#include "UserGroupSecurityDlg.h"
#include "copypermissionsdlg.h"
#include "SchedulerView.h"
#include "AuditTrail.h"
#include "Dontshowdlg.h"
#include "ConfigurePermissionGroupsDlg.h"
#include "PermissionUtils.h"
#include "msgbox.h"
#include "ReportInfo.h"
#include "Reports.h"
#include "GlobalReportUtils.h"
#include "GeneratePermissionUpdateBatch.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
using namespace NXDATALISTLib;

// (a.walling 2010-01-21 16:43) - PLID 37026 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.

// (j.gruber 2010-03-29 16:05) - PLIDb 37946
enum SummaryListColumns {
	slcID = 0,
	slcName,
};


/////////////////////////////////////////////////////////////////////////////
// CUserGroupSecurityDlg dialog


CUserGroupSecurityDlg::CUserGroupSecurityDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CUserGroupSecurityDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CUserGroupSecurityDlg)
	m_lPersonID = -1;
	m_dwCurPerms = 0;
	m_eCurSecurityObject = bioInvalidID;
	m_bIsGroup = FALSE;
	m_bSetAdministrator = FALSE;	
	//}}AFX_DATA_INIT
}


void CUserGroupSecurityDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUserGroupSecurityDlg)
	DDX_Control(pDX, IDC_PERM_COUPLING_CHECK, m_btnPermCoupling);
	DDX_Control(pDX, IDC_TREE_SECURITY, m_treeSecurity);
	DDX_Control(pDX, IDC_EDIT_SECURITYOBJECT_DESCRIPTION, m_nxeditEditSecurityobjectDescription);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_BTN_COPY_FROM, m_btnImportFrom);
	DDX_Control(pDX, IDC_USER_SEC_CONFIGURE_GROUPS, m_btnConfigureGroups);
	DDX_Control(pDX, IDC_PERMS_CLEAR, m_btnClearPerms);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CUserGroupSecurityDlg, CNxDialog)
	//{{AFX_MSG_MAP(CUserGroupSecurityDlg)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_SECURITY, OnSelchangedTreeSecurity)
	ON_BN_CLICKED(IDC_BTN_USERGROUP_HELP, OnBtnHelp)
	ON_BN_CLICKED(IDC_BTN_COPY_FROM, OnBtnCopyFrom)
	ON_BN_CLICKED(IDC_PERM_COUPLING_CHECK, OnPermCouplingCheck)
	ON_WM_CONTEXTMENU()
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_USER_SEC_CONFIGURE_GROUPS, &CUserGroupSecurityDlg::OnBnClickedUserSecConfigureGroups)	
	ON_BN_CLICKED(IDC_PERMS_CLEAR, &CUserGroupSecurityDlg::OnBnClickedPermsClear)
END_MESSAGE_MAP()

HTREEITEM CUserGroupSecurityDlg::FindSecurityObject(HTREEITEM hParent, EBuiltInObjectIDs obj)
{
	HTREEITEM hItem = hParent;
	if (!hItem)
		return NULL;

	// Look through this level
	while (hItem)
	{
		HTREEITEM hRes;
		if ((EBuiltInObjectIDs)m_treeSecurity.GetItemData(hItem) == obj)
			return hItem;

		// Look through any children
		hRes = FindSecurityObject(m_treeSecurity.GetNextItem(hItem, TVGN_CHILD), obj);
		if (hRes)
			return hRes;
		hItem = m_treeSecurity.GetNextItem(hItem, TVGN_NEXT);
	}
	return NULL;
}

void CUserGroupSecurityDlg::FillSecurityTree()
{
	POSITION pos = GetFirstBuiltInObjectPosition();

	// Add all parents first
	while (pos)
	{
		const CBuiltInObject* pObj = GetNextBuiltInObject(pos);
		HTREEITEM hItem;
		if (pObj->m_eParentID == bioInvalidID)
		{
			//DRT 5/11/2004 - We must be able to hide this id for only internal use
			bool bAdd = false;
			if(pObj->m_eBuiltInID == bioInternalOnly) {
				if(IsNexTechInternal())
					bAdd = true;
			}
			else 
				bAdd = true;

			if(bAdd) {
				hItem = m_treeSecurity.InsertItem(pObj->m_strDisplayName, TVI_ROOT);
				m_treeSecurity.SetItemData(hItem, pObj->m_eBuiltInID);
			}
		}
	}

	// Now add children
	pos = GetFirstBuiltInObjectPosition();
	while (pos)
	{
		const CBuiltInObject* pObj = GetNextBuiltInObject(pos);
		HTREEITEM hItem = TVI_ROOT, hParent = TVI_ROOT;
		if (pObj->m_eParentID != bioInvalidID)
		{
			hParent = FindSecurityObject(m_treeSecurity.GetNextItem(NULL, TVGN_CHILD), pObj->m_eParentID);
			//TES 6/15/2004: Don't add the item if its parent wasn't added! (For example, if its parent is bioInternalOnly).
			if(hParent) {
				hItem = m_treeSecurity.InsertItem(pObj->m_strDisplayName, hParent);
				m_treeSecurity.SetItemData(hItem, pObj->m_eBuiltInID);
			}
		}
	}

	//Now add user-defined items
	// (z.manning, 02/21/2007) - PLID 23764 - Dont' show inactive resources.
	_RecordsetPtr rs = CreateRecordset(
		"SELECT SecurityObjectT.ID, BuiltInID, ObjectValue, DisplayName, Description, AvailablePermissions  \r\n"
		"FROM SecurityObjectT  \r\n"
		"LEFT JOIN ResourceT ON SecurityObjectT.ObjectValue = ResourceT.ID AND BuiltInID = %li  \r\n"
		"WHERE COALESCE(ResourceT.Inactive, 0) = 0  \r\n"
		"ORDER BY DisplayName "
		, bioSchedIndivResources);
	while(!rs->eof) {

		long ID = AdoFldLong(rs, "ID");
		long BuiltInID = AdoFldLong(rs, "BuiltInID");
		long ObjectValue = AdoFldLong(rs, "ObjectValue");
		CString DisplayName = AdoFldString(rs, "DisplayName","");
		CString Description = AdoFldString(rs, "Description","");
		long AvailablePermissions = AdoFldLong(rs, "AvailablePermissions");

		HTREEITEM hItem = TVI_ROOT, hParent = TVI_ROOT;
		if (BuiltInID != bioInvalidID)
		{
			hParent = FindSecurityObject(m_treeSecurity.GetNextItem(NULL, TVGN_CHILD), (EBuiltInObjectIDs)BuiltInID);
			hItem = m_treeSecurity.InsertItem(DisplayName, hParent);
			m_treeSecurity.SetItemData(hItem, ID);
		}

		rs->MoveNext();
	}
	rs->Close();


	// Now sort the whole tree
	HTREEITEM hItem = m_treeSecurity.GetNextItem(NULL, TVGN_CHILD);
	while (hItem)
	{
		m_treeSecurity.SortChildren(hItem);
		hItem = m_treeSecurity.GetNextItem(hItem, TVGN_NEXT);
	}
	m_treeSecurity.SortChildren(TVI_ROOT);
}

// Fill the permission list (static and dynamic permissions) based on the currently selected object in the tree.
void CUserGroupSecurityDlg::FillPermissionList()
{
	// Get the currently selected tree item
	HTREEITEM hItem = m_treeSecurity.GetSelectedItem();

	// This should never happen anyway
	if (hItem == NULL)
		return;

	// (b.cardillo 2005-07-08 12:42) - This way of getting the CBuiltInObject seems wacky, 
	// but apparently someone took the time to make sure it worked so I'm not going to 
	// change it right now.  See comments on the definition of GetUserDefinedObject().
	EBuiltInObjectIDs selID = (EBuiltInObjectIDs)m_treeSecurity.GetItemData(hItem);

	const CBuiltInObject* pObj = NULL;
	if(selID < 0)
		pObj = GetBuiltInObject(selID);
	else {
		pObj = GetUserDefinedObject(selID);
	}

	if(!pObj)
		return;

	m_dlPerms->Clear();

	// Add the static permission rows
	// (b.cardillo 2005-07-08 12:23) - We do this in order roughly from most to least 
	// "powerful" permission.  That seems counterintuitive to me but we've always done 
	// it in that order so I don't want to reverse things and confuse the user.
	{
		// Most of the permissions are simple
		AddPermissionRowToList("Delete", sptDelete, sptDeleteWithPass, pObj);
		AddPermissionRowToList("Create", sptCreate, sptCreateWithPass, pObj);
		AddPermissionRowToList("Write", sptWrite, sptWriteWithPass, pObj);
		AddPermissionRowToList("Read", sptRead, sptReadWithPass, pObj);
		// "Access" is special, in that we don't let the user see it unless it's the 
		// only available permission for a given object.
		if ((pObj->m_AvailPermissions.nPermissions & (~(sptView|sptViewWithPass))) == 0) {
			// The ONLY available perms for this object are sptViewWithPass and/or 
			// sptView, so make them available (lest the user see no perms at all).
			AddPermissionRowToList("Access", sptView, sptViewWithPass, pObj);
		}
	}

	#if NX_SECURITY_DYNAMIC_PERMISSION_COUNT != 5
	#error This code assumes there are 5 dynamic permission types.  If that number is changed, this code must be changed too.
	#endif
	// Add the dynamic permission rows
	AddPermissionRowToList(pObj->m_strDynamicPermissionNames[0], sptDynamic0, sptDynamic0WithPass, pObj);
	AddPermissionRowToList(pObj->m_strDynamicPermissionNames[1], sptDynamic1, sptDynamic1WithPass, pObj);
	AddPermissionRowToList(pObj->m_strDynamicPermissionNames[2], sptDynamic2, sptDynamic2WithPass, pObj);
	AddPermissionRowToList(pObj->m_strDynamicPermissionNames[3], sptDynamic3, sptDynamic3WithPass, pObj);
	AddPermissionRowToList(pObj->m_strDynamicPermissionNames[4], sptDynamic4, sptDynamic4WithPass, pObj);
}

// Adds a row to the list of permissions.  
// If the permission type is available on the given object, the row is added with the permission type checkbox.
// If the permission type with password is available on the given object, the row is added with the permission type with password checkbox.
// If both are available on the given object, the row is added with both checkboxes.
// If neither is available, the row is not added.
// Returns TRUE if the row is added, FALSE if it is not.
BOOL CUserGroupSecurityDlg::AddPermissionRowToList(IN LPCTSTR strPermissionName, IN const ESecurityPermissionType esptPermType, IN const ESecurityPermissionType esptPermTypeWithPass, IN const CBuiltInObject *pbioSecurityObject)
{
	// Determine which, if either, of the "perm type" or the "perm type with pass" permissions are available
	BOOL bPermExists, bPermWithPassExists;
	{
		if (pbioSecurityObject->m_AvailPermissions.nPermissions & esptPermType) {
			bPermExists = TRUE;
		}
		else {
			bPermExists = FALSE;
		}

		// (z.manning 2010-05-12 09:59) - PLID 37400 - We now can exclude certain permissions within the
		// same object to not have the with password permission.
		if ((pbioSecurityObject->m_AvailPermissions.nPermissions & esptPermTypeWithPass) &&
			!ExcludeWithPassPermission(pbioSecurityObject->m_eBuiltInID, esptPermTypeWithPass))
		{
			bPermWithPassExists = TRUE;
		} 
		else {
			bPermWithPassExists = FALSE;
		}
	}

	// Now if either permission is available, we're going to be adding the row
	if (bPermExists || bPermWithPassExists) {
		// Create the row
		IRowSettingsPtr pRow = m_dlPerms->GetRow(-1);
		
		// Put the permission name (which we are given) into the row
		pRow->PutValue(COL_PERMNAME, strPermissionName);		
		
		// Now if the permission exists, initialize the checkbox and set the 
		// permission cell so we can get our ESecurityPermissionType back out 
		// later.  If the permission isn't available, set both the checkbox and 
		// the permission cell to VT_NULL.  Do this for both the "perm type" 
		// and the "perm type with pass" sets of cells.
		{
			// Do it for the "perm type" permission
			if (bPermExists) {
				pRow->PutValue(COL_USERSPECIFIC, g_cvarFalse);
				pRow->PutValue(COL_PERM, (long)esptPermType);				
			} else {
				pRow->PutValue(COL_USERSPECIFIC, g_cvarNull);
				pRow->PutValue(COL_PERM, g_cvarNull);
			}
			// Do it for the "perm type with pass" permission
			if (bPermWithPassExists) {
				pRow->PutValue(COL_USERSPECIFIC_WITHPASS, g_cvarFalse);
				pRow->PutValue(COL_PERM_WITHPASS, (long)esptPermTypeWithPass);				
			} else {
				pRow->PutValue(COL_USERSPECIFIC_WITHPASS, g_cvarNull);
				pRow->PutValue(COL_PERM_WITHPASS, g_cvarNull);
			}
		}
		
		// Finally add the row
		m_dlPerms->AddRow(pRow);	
		
		// And return TRUE because we added a row
		return TRUE;
	} else {
		// We didn't do anything so return FALSE
		return FALSE;
	}
}

// (z.manning 2010-05-12 09:45) - PLID 37400 - Function to determine if certain permissions should be
// excluded from having the w/ pass option
BOOL CUserGroupSecurityDlg::ExcludeWithPassPermission(IN const EBuiltInObjectIDs eObjectID, IN const ESecurityPermissionType ePermType)
{
	switch(eObjectID)
	{
		case bioPatientLabs:
			// (z.manning 2010-05-12 10:00) - PLID 37400 - We do not support with pass option for
			// the labs editing HL7 results permission.
			if(ePermType == sptDynamic1WithPass) {
				return TRUE;
			}
			// (j.jones 2010-05-27 15:13) - PLID 38863 - Not supported on the Edit Comments permission either,
			// though it is not defined in NxSecurityBuiltInObjectInfo so in theory we should never get here
			else if(ePermType == sptDynamic2WithPass) {
				return TRUE;
			}
			break;
	}

	return FALSE;
}

// (j.gruber 2010-04-13 16:32) - PLID 38015 - grey out the name if the permission comes from the template
void CUserGroupSecurityDlg::SetRowEnabledStatus(NXDATALISTLib::IRowSettingsPtr pRow, IN const ESecurityPermissionType espt, unsigned long dwPerms)
{
	if (pRow) {

		//first check to see if there are any templates for this user
		if (m_mapTemplatePermissions.GetSize() == 0) {
			//this user has no templates, so we don't need to do anything
			return;
		}

		//get the templates ORed permission out of our map
		unsigned long nTemplatePerms;
		if (!m_mapTemplatePermissions.Lookup(m_eCurSecurityObject, nTemplatePerms)) {
			//this should also never happen
			ASSERT(FALSE);
		}

		//check to see if this permission is in our template and matches our user's permissions
		if (((nTemplatePerms & espt) == (dwPerms & espt)) && ((nTemplatePerms & espt) != 0)) {
			//its in the template, so grey out the row
			pRow->PutForeColor(RGB(125,125,125));		
		}
	}
}

void CUserGroupSecurityDlg::ParsePermissionsToList(unsigned long dwPerms)
{
	// Go through the list, checking or unchecking the "perm" and "perm with pass" cells 
	// appropriately for each row based on its presence or absence in the dwPerms variable.
	long p = m_dlPerms->GetFirstRowEnum();
	while (p) {
		// Get the row
		LPDISPATCH lpDisp;
		m_dlPerms->GetNextRowEnum(&p, &lpDisp);
		ASSERT(lpDisp);
		if (lpDisp) {
			// We've got the row
			IRowSettingsPtr pRow(lpDisp);
			lpDisp->Release();
			
			// Handle "perm"
			{
				// Get the "perm" cell value
				_variant_t varspt = pRow->GetValue(COL_PERM);
				if (varspt.vt != VT_NULL) {
					// It has a permission type, get it
					ESecurityPermissionType espt = (ESecurityPermissionType)VarLong(varspt);
					// See if that type is in the dwPerms variable, and set the checkbox appropriately
					pRow->PutValue(COL_USERSPECIFIC, (dwPerms & espt) ? g_cvarTrue : g_cvarFalse);
					// (j.gruber 2010-04-13 16:32) - PLID 38015 - set the status of this row
					if (!m_bIsGroup) {
						SetRowEnabledStatus(pRow, espt, dwPerms);
					}
				} else {
					// It doesn't have a type, which means this permission isn't supported for the 
					// currently selected security object (in the tree).
					ASSERT(pRow->GetValue(COL_USERSPECIFIC).vt == VT_NULL);
				}
			}

			// Handle "perm with pass"
			{
				// Get the "perm with pass" cell value
				_variant_t varspt = pRow->GetValue(COL_PERM_WITHPASS);
				if (varspt.vt != VT_NULL) {
					// It has a permission type, get it
					ESecurityPermissionType espt = (ESecurityPermissionType)VarLong(varspt);
					// See if that type is in the dwPerms variable, and set the checkbox appropriately
					pRow->PutValue(COL_USERSPECIFIC_WITHPASS, (dwPerms & espt) ? g_cvarTrue : g_cvarFalse);
					if (!m_bIsGroup) {
						SetRowEnabledStatus(pRow, espt, dwPerms);
					}
				} else {
					// It doesn't have a type, which means this permission isn't supported for the 
					// currently selected security object (in the tree).
					ASSERT(pRow->GetValue(COL_USERSPECIFIC_WITHPASS).vt == VT_NULL);
				}
			}
		}
	}
}

BOOL CUserGroupSecurityDlg::LoadPermissions()
{
	try {
		HTREEITEM hItem = m_treeSecurity.GetSelectedItem();

		// This should never happen anyway
		if (hItem == NULL)
			return TRUE;

		m_eCurSecurityObject = (EBuiltInObjectIDs)m_treeSecurity.GetItemData(hItem);


		////////////////////////////////////////////
		// See if it's changed first
		POSITION pos = m_mapPermChanges.GetStartPosition();
		while (pos)
		{
			EBuiltInObjectIDs obj;
			DWORD dwPerms;
			m_mapPermChanges.GetNextAssoc(pos, obj, dwPerms);

			if (obj == m_eCurSecurityObject)
			{
				m_dwCurPerms = dwPerms;
				ParsePermissionsToList(dwPerms);
				return FALSE;
			}
		}

		////////////////////////////////////////////
		// Load current individual permissions
		_RecordsetPtr prs = CreateRecordset("SELECT Permissions FROM PermissionT WHERE UserGroupID = %d AND ObjectID = %d",
			m_lPersonID, m_treeSecurity.GetItemData(hItem));

		if (!prs->eof)
		{
			unsigned long dwPerms = AdoFldLong(prs->Fields->Item["Permissions"], 0);
			ParsePermissionsToList(dwPerms);

			// m_lCurPerms is used to compare with when the user changed which security
			// object to see permissions for.
			m_dwCurPerms = dwPerms;
		}
		else
		{
			// No current individual permissions for this security object
			ParsePermissionsToList(0);

			// m_lCurPerms is used to compare with when the user changed which security
			// object to see permissions for.
			m_dwCurPerms = 0;
		}

		prs->Close();

		return FALSE;
	} NxCatchAll("CUserGroupSecurityDlg::LoadPermissions");
	
	return TRUE;
}


// ::SaveChanges
BOOL CUserGroupSecurityDlg::SavePermissions(BOOL bWriteToData, BOOL bPrompt /* = TRUE*/)
{
	// If there is no current security object, ignore this function
	if (m_eCurSecurityObject == bioInvalidID)
		return FALSE;

	DWORD dwNewPerms = GetPermissionsFromList();

	// Save changes in a map of changed permissions we keep
	if (dwNewPerms != m_dwCurPerms)
	{
		m_mapPermChanges[m_eCurSecurityObject] = dwNewPerms;
	}

	// Traverse the whole map and write all the changes to the data
	// (j.gruber 2010-04-13 16:33) - PLID 23982 - changed the way saving works completely
	if (bWriteToData && m_mapPermChanges.GetCount() > 0)
	{
		/*if(bPrompt) {
			CString strMsg;
			// Make sure the user wants to save the changes
			if (m_bIsGroup)
				strMsg = "You have made changes to the permissions of this template. These changes will only affect future imports from this group. Are you sure you wish to save these changes?";
			else
				strMsg = "You have made changes to the permissions of this user. Are you sure you wish to save these changes?";
			if (IDNO == MsgBox(MB_YESNO, strMsg))
				return TRUE;
		}

		if(m_bSetAdministrator && !m_bIsGroup) {
			ExecuteSql("UPDATE UsersT SET Administrator = 1 WHERE PersonID = %li",m_lPersonID);
		}*/

		long nAuditTransactionID = -1;
		CString strSql = BeginSqlBatch();

		
		//TES 5/1/2008 - PLID 27580 - Need to have our own exception handling here, that will rollback the audit 
		// transaction if necessary.
		try {

			/*CString strUserName = "";
			if(!m_bIsGroup) {
				_RecordsetPtr rsUsers = CreateRecordset("SELECT UserName FROM UsersT WHERE PersonID = %li",m_lPersonID);
				if(!rsUsers->eof) {
					strUserName = AdoFldString(rsUsers, "UserName","");
				}
				rsUsers->Close();
			}

			// Traverse the map making the appropriate adjustments to the Permissions table
			POSITION pos = m_mapPermChanges.GetStartPosition();
			/*while (pos)
			{
				_RecordsetPtr rs;
				EBuiltInObjectIDs obj;
				m_mapPermChanges.GetNextAssoc(pos, obj, dwNewPerms);

				// If the built-in object has sptView AND any other permissions available, then set 
				// dwNewPerms to 0 iff view is the only one set, and set dwNewPerms |= sptView if 
				// dwNewPerms has any other perms set.
				AdjustViewPerms(obj, dwNewPerms);

				if (dwNewPerms)
				{
					rs = CreateRecordset("SELECT Permissions FROM PermissionT WHERE UserGroupID = %d AND ObjectID = %d",
						m_lPersonID, obj);
					if (!rs->eof)	// If the permission already exists, change it
					{
						rs->Close();
						AddStatementToSqlBatch(strSql, "UPDATE PermissionT SET Permissions = %d WHERE UserGroupID = %d AND ObjectID = %d",
							dwNewPerms, m_lPersonID, obj);
					}
					else	// Otherwise, add it as new
					{
						rs->Close();
						AddStatementToSqlBatch(strSql, "INSERT INTO PermissionT (UserGroupID, ObjectID, Permissions) VALUES (%d, %d, %d)",
							m_lPersonID, obj, dwNewPerms);
					}
					rs.Detach();
				}
				else // The new permissions are empty. Just delete the permissiont record.
				{
					AddStatementToSqlBatch(strSql, "DELETE FROM PermissionT WHERE UserGroupID = %d AND ObjectID = %d",
						m_lPersonID, obj);
				}

				const CBuiltInObject* pObj = NULL;
				pObj = GetBuiltInObject(obj);
				//if it is a group rather than a user, we don't need to audit
				if(!m_bIsGroup && pObj) {

					if(nAuditTransactionID == -1)
						nAuditTransactionID = BeginAuditTransaction();

					AuditEvent(-1, strUserName,nAuditTransactionID,aeiUserPermissionChanged,m_lPersonID,"",pObj->m_strDisplayName,aepMedium,aetChanged);
				}
			}*/
			//first we need to generate our template map			
			if (!m_bIsGroup) {			

				if(bPrompt) {
					CString strMsg;
					// Make sure the user wants to save the changes
					strMsg = "You have made changes to the permissions of this user. Are you sure you wish to save these changes?";
					if (IDNO == MsgBox(MB_YESNO, strMsg))
						return TRUE;
				}

				//load their existing permissions from data
				CString strTableName, strBlank;
				CMap<EBuiltInObjectIDs,EBuiltInObjectIDs,DWORD,DWORD> pMapUserPerms;

				LoadPermissionsIntoMap(strTableName, m_lPersonID, &pMapUserPerms, -1, TRUE);
				CString strUserName = GetExistingUserName(m_lPersonID);

				//(e.lally 2010-10-14) PLID 40912 - We only want to save the changes values and template values (not the previously saved values), so the first parameter is false.
				// (d.thompson 2013-03-25) - PLID 55847 - Converted to CGeneratePermissionUpdateBatch
				CGeneratePermissionUpdateBatch batch;
				batch.AddPermissionUpdate(false, m_lPersonID, &m_mapPermChanges, &pMapUserPerms, &m_mapTemplatePermissions, strUserName, nAuditTransactionID, strBlank);
				CSqlFragment sql = batch.ToSqlFragment();
				if(!sql.IsEmpty())
					ExecuteParamSql(sql);

				if(nAuditTransactionID != -1) {					
					CommitAuditTransaction(nAuditTransactionID);
				}
			}
			else {

				//first we need to save the template
				// (d.thompson 2013-03-25) - PLID 55847 - Converted to CGeneratePermissionUpdateBatch
				CGeneratePermissionUpdateBatch batch;
				batch.AddPermissionUpdate(m_lPersonID, &m_mapPermChanges);

				CString strUserGroupsChanged;
				long nCountChanged = 0;

				//we have to loop through each user that is on this template, applying the changes
				_RecordsetPtr rsUserGroups = CreateParamRecordset("SELECT UserID, (SELECT UserName FROM UsersT WHERE UsersT.PersonID = UserGroupDetailsT.UserID) as UserName FROM UserGroupDetailsT WHERE GroupID = {INT}", m_lPersonID);				
				while (!rsUserGroups->eof) {

					CMap<EBuiltInObjectIDs,EBuiltInObjectIDs,DWORD,DWORD> mapTemplates;
					CMap<EBuiltInObjectIDs,EBuiltInObjectIDs,DWORD,DWORD> mapUser;
					//first load the permissions for this user
					long nUserID = AdoFldLong(rsUserGroups, "UserID");
					CString strUserName = AdoFldString(rsUserGroups, "UserName", "");
					CString strTableName;
					LoadPermissionsIntoMap(strTableName, nUserID, &mapUser, -1, TRUE);					
					LoadPermissionsIntoMap(strTableName, nUserID, &mapTemplates, m_lPersonID, FALSE);
					CString strTemp;
					
					//(e.lally 2010-10-14) PLID 40912 - We only want to save the changes values and template values (not the previously saved values), so the first parameter is false.
					// (d.thompson 2013-03-25) - PLID 55847 - Converted to CGeneratePermissionUpdateBatch
					batch.AddPermissionUpdate(false, nUserID, &m_mapPermChanges, &mapUser, &mapTemplates, strUserName, nAuditTransactionID, strTemp);

					if (!strTemp.IsEmpty()) {
						strUserGroupsChanged += strUserName + " \r\n";
						// (j.jones 2012-05-23 17:52) - PLID 50593 - track the count of changed users
						nCountChanged++;
					}

					rsUserGroups->MoveNext();
				}

				// (j.gruber 2010-04-13 16:50) - PLID 38085 - output message box of users that are changing
				if (!strUserGroupsChanged.IsEmpty() && bPrompt) {
					// (j.gruber 2010-08-17 09:44) - PLID 40138 - change template to group
					// (j.jones 2012-05-23 16:42) - PLID 50593 - If you have too many users, this list will be too long. 
					// So if there are more than 15 affected users, I changed changed this to a scrollable messagebox,
					// because aborting the list and saying <more...> once the list is too long is not acceptable here.
					// If 15 users or less, we still use a message box, only because the message box is more attractive
					// than the scrollable list.
					if(nCountChanged <= 15) {
						CString strPrompt;
						strPrompt.Format("This will change permissions for the following users:\n"
							"%s\n"
							"Note: Some users on this group may not have a change in permission because the user exists in other groups that have this permission.\n\n"
							"Are you sure you wish to continue?", strUserGroupsChanged);
						if (IDNO == MessageBox(strPrompt, "Practice", MB_YESNO)) {
							return TRUE;
						}
					}
					else {
						CString strPrompt;
						strPrompt.Format("This will change permissions for the following users:\r\n"
							"%s\r\n"
							"Note: Some users on this group may not have a change in permission because the user exists in other groups that have this permission.\r\n\r\n"
							"Click OK if you are sure you want to update these permissions.", strUserGroupsChanged);
						CMsgBox dlgMsg(this);
						dlgMsg.m_strWindowText = "Confirm User Permission Changes";
						dlgMsg.m_bAllowCancel = TRUE;
						dlgMsg.msg = strPrompt;
						if(IDCANCEL == dlgMsg.DoModal()) {
							return TRUE;
						}
					}
				}

				CSqlFragment sql = batch.ToSqlFragment();
				if (!sql.IsEmpty()) {
					// (a.walling 2012-02-09 17:13) - PLID 48115 - Use NxAdo::PushMaxRecordsWarningLimit and NxAdo::PushPerformanceWarningLimit
					NxAdo::PushMaxRecordsWarningLimit pmr(100000);
					ExecuteParamSql(sql);	
				}

				if(nAuditTransactionID != -1) {					
					CommitAuditTransaction(nAuditTransactionID);
				}
			}			
			
			
		}NxCatchAllSilentCallThrow(
			if(nAuditTransactionID != -1) {
				RollbackAuditTransaction(nAuditTransactionID);
			}
		)

		if (m_lPersonID == GetCurrentUserID())
		{
			LogInUser(GetCurrentUserName(), GetCurrentUserPassword(), GetCurrentLocationID());
			GetMainFrame()->UpdateToolBarButtons(TRUE);

			//we could have changed resource permissions, so update your views
			//TODO: see what it will take to only do this if we changed resource permissions - see if it is worth the overhead
			
			// Now if the scheduler is open, since the user just clicked OK on a dialog, we feel safe to call the UI 
			// handler to reload the current list of resources, just in case the permissions for any of them have changed.
			CSchedulerView *pExistingSchedView = (CSchedulerView *)GetMainFrame()->GetOpenView(SCHEDULER_MODULE_NAME);
			if (pExistingSchedView) {
				pExistingSchedView->OnReloadCurrentResourceList();
			}
		}

		//we could have changed resource permissions, so have everyone update their views
		//TODO: see what it will take to only do this if we changed resource permissions - see if it is worth the overhead
		CClient::RefreshTable(NetUtils::Resources);
				
	}
	return FALSE;
}

unsigned long CUserGroupSecurityDlg::GetPermissionsFromList()
{
	unsigned long dwPerms = 0;

	// Go through the list, bitwise-or'ing the "perm" and "perm with pass" into dwPerms 
	// whenever the corresponding checkbox is checked.
	long p = m_dlPerms->GetFirstRowEnum();
	while (p) {
		// Get the row
		LPDISPATCH lpDisp;
		m_dlPerms->GetNextRowEnum(&p, &lpDisp);
		ASSERT(lpDisp);
		if (lpDisp) {
			// We've got the row
			IRowSettingsPtr pRow(lpDisp);
			lpDisp->Release();
			
			// Handle "perm"
			{
				// Get the "perm" cell value
				_variant_t varspt = pRow->GetValue(COL_PERM);
				if (varspt.vt != VT_NULL) {
					// It has a permission type, get it
					ESecurityPermissionType espt = (ESecurityPermissionType)VarLong(varspt);
					// Put that type into the dwPerms variable if the checkbox is set
					if (VarBool(pRow->GetValue(COL_USERSPECIFIC))) {
						dwPerms |= espt;
					}
				} else {
					// It doesn't have a type, which means this permission isn't supported for the 
					// currently selected security object (in the tree).
					ASSERT(pRow->GetValue(COL_USERSPECIFIC).vt == VT_NULL);
				}
			}

			// Handle "perm with pass"
			{
				// Get the "perm with pass" cell value
				_variant_t varspt = pRow->GetValue(COL_PERM_WITHPASS);
				if (varspt.vt != VT_NULL) {
					// It has a permission type, get it
					ESecurityPermissionType espt = (ESecurityPermissionType)VarLong(varspt);
					// Put that type into the dwPerms variable if the checkbox is set
					if (VarBool(pRow->GetValue(COL_USERSPECIFIC_WITHPASS))) {
						dwPerms |= espt;
					}
				} else {
					// It doesn't have a type, which means this permission isn't supported for the 
					// currently selected security object (in the tree).
					ASSERT(pRow->GetValue(COL_USERSPECIFIC_WITHPASS).vt == VT_NULL);
				}
			}
		}
	}

	// Return our answer
	return dwPerms;
}

/////////////////////////////////////////////////////////////////////////////
// CUserGroupSecurityDlg message handlers

// (j.gruber 2010-04-13 16:36) - PLID 37947 - load users 
BOOL CUserGroupSecurityDlg::LoadSummaryUsers() 
{

	//include inactives 
	
	_RecordsetPtr rs = CreateParamRecordset("SELECT UsersT.PersonID, UserName FROM UsersT INNER JOIN UserGroupDetailsT ON UsersT.PersonID = UserGroupDetailsT.UserID "
		" WHERE UserGroupDetailsT.GroupID = {INT} AND UsersT.PersonID > 0 ORDER BY UserName ", m_lPersonID);
	
	BOOL bReturn = FALSE;

	while (!rs->eof) {

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pSummaryList->GetNewRow();
		if (pRow) {
			bReturn = TRUE;
			pRow->PutValue(slcID, AdoFldLong(rs, "PersonID"));
			pRow->PutValue(slcName, _variant_t(AdoFldString(rs, "UserName", "")));
			m_pSummaryList->AddRowAtEnd(pRow, NULL);
		}

		rs->MoveNext();
	}
	return bReturn;
}

// (j.gruber 2010-04-13 16:37) - PLID 37947 - load tempaltes
BOOL CUserGroupSecurityDlg::LoadSummaryTemplates() 
{

		
	_RecordsetPtr rs = CreateParamRecordset("SELECT GroupID, Name FROM UserGroupsT INNER JOIN UserGroupDetailsT ON UserGroupsT.PersonID = UserGroupDetailsT.GroupID "
		" WHERE UserGroupDetailsT.UserID = {INT} AND UserID > 0 ORDER BY Name", m_lPersonID);

	BOOL bReturn = FALSE;

	while (!rs->eof) {

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pSummaryList->GetNewRow();
		if (pRow) {
			bReturn = TRUE;
			pRow->PutValue(slcID, AdoFldLong(rs, "GroupID"));
			pRow->PutValue(slcName, _variant_t(AdoFldString(rs, "Name", "")));
			m_pSummaryList->AddRowAtEnd(pRow, NULL);
		}

		rs->MoveNext();
	}

	return bReturn;
}

// (j.gruber 2010-04-13 16:37) - PLID 37947 - check if they have additional permissions
BOOL CUserGroupSecurityDlg::CheckPermissionSummary() 
{


	//first check if this user is an administrator
	if (m_bSetAdministrator) {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pSummaryList->GetNewRow();
		if (pRow) {
			pRow->PutValue(slcID, (long)-2);
			pRow->PutValue(slcName, _variant_t("Administrator: Has All Permissions"));
			m_pSummaryList->AddRowAtEnd(pRow, NULL);

			return TRUE;
		}
	}

	//we need to load our user template
	CString strTableName;
	CMap<EBuiltInObjectIDs,EBuiltInObjectIDs,DWORD,DWORD> mapUserPerms;
	LoadPermissionsIntoMap(strTableName, m_lPersonID, &mapUserPerms, -1, TRUE);

	POSITION pos = mapUserPerms.GetStartPosition();

	while (pos) {
		DWORD dwUserPerms;
		EBuiltInObjectIDs ebio;

		mapUserPerms.GetNextAssoc(pos, ebio, dwUserPerms);

		//now look it up in our tempalte map
		DWORD dwTempPerms;
		if (m_mapTemplatePermissions.Lookup(ebio, dwTempPerms)) {

			AdjustViewPerms(ebio, dwUserPerms);
			AdjustViewPerms(ebio, dwTempPerms);

			//check to see if they match
			if (dwUserPerms != dwTempPerms) {

				const CBuiltInObject *pBio;
				if (ebio < 0) {
					pBio = GetBuiltInObject(ebio);
				}
				else {
					pBio = GetUserDefinedObject(ebio);
				}
				//need to go through each type of permission and check in case the permission changed
				if (CheckPermissionChanged(pBio, sptDelete, sptDeleteWithPass, dwUserPerms, dwTempPerms)) {
					return TRUE;
				}
				
				if (CheckPermissionChanged(pBio, sptCreate, sptCreateWithPass, dwUserPerms, dwTempPerms)) {
					return TRUE;
				}

				if (CheckPermissionChanged(pBio, sptRead, sptReadWithPass, dwUserPerms, dwTempPerms)) {
					return TRUE;
				}

				if (CheckPermissionChanged(pBio, sptWrite, sptWriteWithPass, dwUserPerms, dwTempPerms)) {
					return TRUE;
				}

				if (CheckPermissionChanged(pBio, sptView, sptViewWithPass, dwUserPerms, dwTempPerms)) {
					return TRUE;
				}

				if (CheckPermissionChanged(pBio, sptDynamic0, sptDynamic0WithPass, dwUserPerms, dwTempPerms)) {
					return TRUE;
				}

				if (CheckPermissionChanged(pBio, sptDynamic1, sptDynamic1WithPass, dwUserPerms, dwTempPerms)) {
					return TRUE;
				}

				if (CheckPermissionChanged(pBio, sptDynamic2, sptDynamic2WithPass, dwUserPerms, dwTempPerms)) {
					return TRUE;
				}

				if (CheckPermissionChanged(pBio, sptDynamic3, sptDynamic3WithPass, dwUserPerms, dwTempPerms)) {
					return TRUE;
				}

				if (CheckPermissionChanged(pBio, sptDynamic4, sptDynamic4WithPass, dwUserPerms, dwTempPerms)) {
					return TRUE;
				}
				
			}
		}
		else {
			ASSERT(FALSE);
		}
	}

	//if we got here, they match!
	return FALSE;	
}

// (j.gruber 2010-04-13 16:38) - PLID 37946
void CUserGroupSecurityDlg::LoadSummaryList() 
{
	try {

		m_pSummaryList->Clear();
		m_pSummaryList->AllowSort = false;

		if (!m_bIsGroup) {

			//its a user
			//add a title row
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pSummaryList->GetNewRow();
			LoadSummaryTemplates();			

			//now the permissions
			//this is taken care of with 37947
			pRow = m_pSummaryList->GetNewRow();
			pRow->PutValue(slcID, (long)-1);						
			BOOL bReturn;
			bReturn = CheckPermissionSummary();					

			if (!bReturn) {
				//none were added
				//only show this if they have templates
				if (m_pSummaryList->GetRowCount() != 0) {
					pRow->PutValue(slcName, "<No Permissions outside of Groups>");
					m_pSummaryList->AddRowAtEnd(pRow, NULL);
				}
			}
			else {
				pRow->PutValue(slcName, "<Permissions exist outside of Groups - Click Here to See>");
				pRow->PutCellLinkStyle(slcName, NXDATALIST2Lib::dlLinkStyleTrue);
				m_pSummaryList->AddRowAtEnd(pRow, NULL);
			}

			
			
		}
		else {			

			//template	
			LoadSummaryUsers();			

		}

	}NxCatchAll(__FUNCTION__);

}

BOOL CUserGroupSecurityDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {

		m_dlPerms = BindNxDataListCtrl(this, IDC_LIST_PERMISSIONS, NULL, false);
		
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnImportFrom.AutoSet(NXB_MODIFY);
		m_btnConfigureGroups.AutoSet(NXB_MODIFY);
		// (j.gruber 2010-04-13 16:39) - PLID 37946
		m_btnClearPerms.AutoSet(NXB_MODIFY);

		// (j.gruber 2010-03-29 16:23) - PLID 37946 - hide the import button
		m_btnImportFrom.ShowWindow(SW_HIDE);

		// (j.gruber 2010-03-29 16:24) - PLID 37946 - don't show info
		// (j.gruber 2010-08-17 09:44) - PLID 40138 - change template to group
		DontShowMeAgain(this, "You can now assign Users to Permission Groups.  This allows you to associate users with what were previously called Permission Templates in order to change many users' permissions at once.  To access the group configuration, click the Configure Users or Configure Groups button on the bottom left of this screen or in Activities->Configure Permissions Groups in the Contacts Module.", "ShowNewPermissionHelp", "Permission Templating Changes", FALSE, FALSE, FALSE);  
		
		// (b.cardillo 2005-07-11 13:34) - PLID 16513 - Recall the preference for permission-coupling
		CheckDlgButton(IDC_PERM_COUPLING_CHECK, GetRemotePropertyInt("ContactsPermissionCoupling", 0, 0, GetCurrentUserName(), false) ? 1 : 0);

		// (j.gruber 2010-04-13 10:46) - PLID 37946
		m_pSummaryList = BindNxDataList2Ctrl(IDC_SUMMARY_LIST, false);

		if (!m_bIsGroup)
		{
			_RecordsetPtr prs = CreateRecordset("SELECT UserName FROM UsersT WHERE PersonID = %d", m_lPersonID);
			CString strName = AdoFldString(prs, "UserName");
			CString strWindowText;
			strWindowText.Format("Permissions for user %s", strName);
			SetWindowText(strWindowText);

			// (j.gruber 2010-04-13 16:40) - PLID 37946
			m_btnClearPerms.SetWindowText("Reset and Reapply");
			m_btnConfigureGroups.SetWindowText("Configure Groups");
			
			NXDATALIST2Lib::IColumnSettingsPtr pCol = m_pSummaryList->GetColumn(slcName);
			if (pCol) {
				pCol->ColumnTitle = "Permission Groups/Additional Permissions";
			}
			

		}
		else
		{
			_RecordsetPtr prs = CreateRecordset("SELECT Name FROM UserGroupsT WHERE PersonID = %d", m_lPersonID);
			CString strGroup = AdoFldString(prs, "Name");
			CString strWindowText;
			strWindowText.Format("Permissions for Group %s", strGroup);
			SetWindowText(strWindowText);

			// (j.gruber 2010-04-13 16:40) - PLID 37946
			m_btnClearPerms.SetWindowText("Clear Permissions");
			m_btnConfigureGroups.SetWindowText("Configure Users");
			NXDATALIST2Lib::IColumnSettingsPtr pCol = m_pSummaryList->GetColumn(slcName);
			if (pCol) {
				pCol->ColumnTitle = "Users Assigned to this Group";
			}
		}	

		// (j.gruber 2010-04-13 16:40) - PLID 23982 - load our templates
		if (!m_bIsGroup) {
			CString strTableName;
			LoadPermissionsIntoMap(strTableName, m_lPersonID, &m_mapTemplatePermissions, -1, FALSE);
		}

		// Fill the security tree with all security objects
		FillSecurityTree();

		// Fill the permission list
		FillPermissionList();

		// Now we load the permissions for the user
		LoadPermissions();

		// Now select the first visible item
		m_treeSecurity.SelectFirstVisible();

		// (j.gruber 2010-03-29 15:17) - PLID 37946 - summary datalist	
		LoadSummaryList();	
	}NxCatchAll(__FUNCTION__);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CUserGroupSecurityDlg::SetSecurityObjectDescription()
{
	HTREEITEM hItem = m_treeSecurity.GetSelectedItem();

	// Set the description text at the top
	if (!hItem)
	{
		SetDlgItemText(IDC_EDIT_SECURITYOBJECT_DESCRIPTION, "");
		return;
	}

	EBuiltInObjectIDs ebio = (EBuiltInObjectIDs)m_treeSecurity.GetItemData(hItem);
	const CBuiltInObject* pObj = NULL;
	if(ebio < 0)
		pObj = GetBuiltInObject(ebio);
	else
		pObj = GetUserDefinedObject(ebio);

	if (!pObj)
		return;

	SetDlgItemText(IDC_EDIT_SECURITYOBJECT_DESCRIPTION, CString("Description: ") + pObj->m_strDescription);
}

void CUserGroupSecurityDlg::OnSelchangedTreeSecurity(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	// Save any changes we made
	SavePermissions(FALSE);

	// Set the description of the newly selected security object at the top of the window
	SetSecurityObjectDescription();

	// Fill the permission list
	FillPermissionList();
	
	// Assign the visible permissions based on the user
	LoadPermissions();

	if (pResult)
		*pResult = 0;
}

int CUserGroupSecurityDlg::Open(long nPersonID, BOOL bIsGroup)
{
	m_lPersonID = nPersonID;
	m_bIsGroup = bIsGroup;
	return DoModal();
}

BEGIN_EVENTSINK_MAP(CUserGroupSecurityDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CUserGroupSecurityDlg)
	ON_EVENT(CUserGroupSecurityDlg, IDC_LIST_PERMISSIONS, 10 /* EditingFinished */, OnEditingFinishedListPermissions, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CUserGroupSecurityDlg, IDC_LIST_PERMISSIONS, 9 /* EditingFinishing */, OnEditingFinishingListPermissions, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CUserGroupSecurityDlg, IDC_LIST_PERMISSIONS, 6 /* RButtonDown */, OnRButtonDownListPermissions, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
	ON_EVENT(CUserGroupSecurityDlg, IDC_SUMMARY_LIST, 19, CUserGroupSecurityDlg::LeftClickSummaryList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CUserGroupSecurityDlg, IDC_LIST_PERMISSIONS, 8, CUserGroupSecurityDlg::EditingStartingListPermissions, VTS_I4 VTS_I2 VTS_PVARIANT VTS_PBOOL)
END_EVENTSINK_MAP()

void CUserGroupSecurityDlg::OnEditingFinishingListPermissions(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue)
{
	// We assume we're on a valid row
	ASSERT(nRow != sriNoRow);
	if (nRow == sriNoRow) {
		return;
	}

	// Right now we only care about the checkbox cells
	if (nCol == COL_USERSPECIFIC || nCol == COL_USERSPECIFIC_WITHPASS) {
		// It's one of the checkboxes, just make sure the user isn't trying to 
		// check one that doesn't exist.
		if (varOldValue.vt == VT_NULL) {
			// Yep, it doesn't exist.  Don't allow the change.
			*pbCommit = FALSE;
		}
	}
}

void CUserGroupSecurityDlg::OnEditingFinishedListPermissions(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	// We assume we're on a valid row
	ASSERT(nRow != sriNoRow);
	if (nRow == sriNoRow) {
		return;
	}


	// Right now we only care about the checkbox columns, and only when they have a checkbox in them
	if ((nCol == COL_USERSPECIFIC || nCol == COL_USERSPECIFIC_WITHPASS) && varOldValue.vt != VT_NULL) {
		// (b.cardillo 2005-07-08 15:33) - PLID 16513 - The only restriction we enforce 
		// now is the physical requirement that only one of the two checkboxes be checked 
		// at any given time.
		BOOL bIsSelecting = AsBool(varNewValue);
		if (bIsSelecting) {
			// Something's being checked, make sure its counterpart becomes unchecked

			// First make sure the counterpart exists
			short nColCounterpartPerm = (nCol == COL_USERSPECIFIC) ? COL_PERM_WITHPASS : COL_PERM;
			if (m_dlPerms->GetValue(nRow, nColCounterpartPerm).vt != VT_NULL) {
				// It's available, so determing the checkbox counterpart
				short nColCounterpartCheckbox = (nCol == COL_USERSPECIFIC) ? COL_USERSPECIFIC_WITHPASS : COL_USERSPECIFIC;
				if (AsBool(m_dlPerms->GetValue(nRow, nColCounterpartCheckbox))) {
					// The counterpart WAS checked, we have to uncheck it
					m_dlPerms->PutValue(nRow, nColCounterpartCheckbox, g_cvarFalse);
				}
			}
		}

	
	
		// If the feature is turned on, then propogate the change to all appropriate rows
		if (IsDlgButtonChecked(IDC_PERM_COUPLING_CHECK) && !IsKeyDown(VK_SHIFT)) {
			// Get the permission type that was clicked
			ESecurityPermissionType esptChangedPermType = (ESecurityPermissionType)VarLong(m_dlPerms->GetValue(nRow, (nCol == COL_USERSPECIFIC_WITHPASS) ? COL_PERM_WITHPASS : COL_PERM));

			// Decide which types the selecting or unselecting of esptChangedPermType should cause
			long nAutoSetPermTypes = 0;
			long nAutoClearPermTypes = 0;
			if (bIsSelecting) {
				// Delete -> Create -> Write -> Read
				// If the user selects any in the chain, we should auto-select each of the 
				// ones below it in the chain.  Whether the one the user selected was a 
				// "w/pass" or not, we still auto-select without pass.
				switch (esptChangedPermType) {
				case sptDelete: case sptDeleteWithPass:
					nAutoSetPermTypes |= sptCreate|sptCreateWithPass;
					//fallthrough
				case sptCreate: case sptCreateWithPass:
					nAutoSetPermTypes |= sptWrite|sptWriteWithPass;
					//fallthrough
				case sptWrite: case sptWriteWithPass:
					nAutoSetPermTypes |= sptRead|sptReadWithPass;
					break;
				}
			} else {
				// Delete -> Create -> Write -> Read
				// If the user unselects any in the chain, we should auto-unselect each of 
				// the ones above it in the chain.  Whether the one the user unselected was 
				// a "w/pass" or not, we unselect all higher in the chain.
				switch (esptChangedPermType) {
				case sptRead: case sptReadWithPass:
					nAutoClearPermTypes |= sptWrite|sptWriteWithPass;
					//fallthrough
				case sptWrite: case sptWriteWithPass:
					nAutoClearPermTypes |= sptCreate|sptCreateWithPass;
					//fallthrough
				case sptCreate: case sptCreateWithPass:
					nAutoClearPermTypes |= sptDelete|sptDeleteWithPass;
					break;
				}
			}

			// Now we know what to add and what to delete, loop through each row, and 
			// check/uncheck correspondingly.
			long p = m_dlPerms->GetFirstRowEnum();
			while (p) {
				// Get the row
				LPDISPATCH lpDisp;
				m_dlPerms->GetNextRowEnum(&p, &lpDisp);
				ASSERT(lpDisp);
				if (lpDisp) {
					// We've got the row
					IRowSettingsPtr pRow(lpDisp);
					lpDisp->Release();

					// Look at the state of both checkboxes
					BOOL bIsChecked = VarBool(pRow->GetValue(COL_USERSPECIFIC), FALSE);
					BOOL bIsWithPassChecked = VarBool(pRow->GetValue(COL_USERSPECIFIC_WITHPASS), FALSE);

					// Decide if we have to uncheck both, or check one
					BOOL bNeedCheck, bNeedUncheck;
					{
						// See if either checkbox is checked
						if (bIsChecked || bIsWithPassChecked) {
							// One is checked, so we know we don't have to check
							bNeedCheck = FALSE;
							// But see if we need to uncheck by seeing if either the perm or perm 
							// with pass overlaps anything in the nAutoClearPermTypes variable
							bNeedUncheck = ((VarLong(pRow->GetValue(COL_PERM), 0) & nAutoClearPermTypes) || (VarLong(pRow->GetValue(COL_PERM_WITHPASS), 0) & nAutoClearPermTypes));
						} else {
							// Neither is checked, so we know we don't have to uncheck
							bNeedUncheck = FALSE;
							// But see if we need to check by seeing if either the perm or perm with 
							// pass overlaps anything in the nAutoSetPermTypes variable
							bNeedCheck = ((VarLong(pRow->GetValue(COL_PERM), 0) & nAutoSetPermTypes) || (VarLong(pRow->GetValue(COL_PERM_WITHPASS), 0) & nAutoSetPermTypes));
						}
					}

					// Perform the action if we decided an action is necessary
					if (bNeedUncheck) {
						// Need to uncheck both
						if (bIsChecked) {
							pRow->PutValue(COL_USERSPECIFIC, g_cvarFalse);
						}
						if (bIsWithPassChecked) {
							pRow->PutValue(COL_USERSPECIFIC_WITHPASS, g_cvarFalse);
						}
					} else if (bNeedCheck) {
						// Need to check one
						// Try the non-with-pass one first
						if (pRow->GetValue(COL_PERM).vt != VT_NULL) {
							pRow->PutValue(COL_USERSPECIFIC, g_cvarTrue);
						} else if (pRow->GetValue(COL_PERM_WITHPASS).vt != VT_NULL) {
							pRow->PutValue(COL_USERSPECIFIC_WITHPASS, g_cvarTrue);
						} else {
							ASSERT(FALSE);
						}
					} else {
						// Do nothing
					}
				}
			}
		}
	}
}

void CUserGroupSecurityDlg::OnOK() 
{
	if (SavePermissions(TRUE))
		return;
	CDialog::OnOK();
}

void CUserGroupSecurityDlg::OnBtnHelp() 
{
	//(j.anspach 06-09-2005 10:53 PLID 16662) - Updating the OpenManual call to work with the new help system.
	OpenManual("NexTech_Practice_Manual.chm", "System_Setup/User_Setup/configure_permission_templates.htm");	
}

void CUserGroupSecurityDlg::OnBtnCopyFrom() 
{
	CCopyPermissionsDlg dlg(this);
	dlg.m_nUserGroup = m_lPersonID;
	dlg.m_pmapPermChanges = &m_mapPermChanges;
	if (IDOK == dlg.DoModal())
	{
		// Fill the permission list
		FillPermissionList();
	
		// Assign the visible permissions based on the user
		LoadPermissions();

		if(dlg.m_bAdministratorSet) {
			m_bSetAdministrator = TRUE;
			if(IDYES == MessageBox("By giving Administrator status to this user, the permissions editor is no longer needed and will close.\n\n"
				"Are you sure you wish to save these changes?","Practice",MB_ICONQUESTION|MB_YESNO)) {
				if(SavePermissions(TRUE,FALSE))
					return;
				CDialog::OnOK();
			}
			else {
				CDialog::OnCancel();
			}
		}
	}
}

void CUserGroupSecurityDlg::OnPermCouplingCheck()
{
	try {
		// Remember the setting
		SetRemotePropertyInt("ContactsPermissionCoupling", IsDlgButtonChecked(IDC_PERM_COUPLING_CHECK) ? 1 : 0, 0, GetCurrentUserName());
	} NxCatchAll("CUserGroupSecurityDlg::OnPermCouplingCheck");
}

void CUserGroupSecurityDlg::OnRButtonDownListPermissions(long nRow, short nCol, long x, long y, long nFlags) 
{
	try {
		// (j.gruber 2010-04-13 11:21) - PLID 38015 - disable for users
		if (m_bIsGroup) {
			// As usual, make sure the window is valid
			CWnd *pWnd = GetDlgItem(IDC_LIST_PERMISSIONS);
			if (pWnd->GetSafeHwnd()) {
				// Give the datalist focus
				pWnd->SetFocus();
				// Set the selection to the row that was clicked on
				m_dlPerms->PutCurSel(nRow);

				// Now we let the context menu handler take care of the rest
			}
		}
	} NxCatchAll("CUserGroupSecurityDlg::OnRButtonDownListPermissions");
}

void CUserGroupSecurityDlg::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	try {
		// (j.gruber 2010-04-13 11:21) - PLID 38015 - disable for users
		if (m_bIsGroup) {
			// Handle the context menu
			if (pWnd->GetSafeHwnd() && pWnd->GetSafeHwnd() == GetDlgItem(IDC_LIST_PERMISSIONS)->GetSafeHwnd()) {
				CMenu mnu;
				if (mnu.CreatePopupMenu()) {
					enum {
						miSelectAll = 1,
						miSelectAllWithPass = 2,
						miUnselectAll = 3,
					};

					mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, miSelectAll, "Allow &All");
					mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, miSelectAllWithPass, "Allow All w/ &Pass");
					mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, miUnselectAll, "&Clear All");

					CPoint pt = CalcContextMenuPos(pWnd, point);
					int nResult = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this);
					if (nResult != 0) {
						// The user chose a menu item, so we know we're going to loop through the whole list
						
						long p = m_dlPerms->GetFirstRowEnum();
						while (p) {
							// Get the row
							LPDISPATCH lpDisp;
							m_dlPerms->GetNextRowEnum(&p, &lpDisp);
							ASSERT(lpDisp);
							if (lpDisp) {
								// We've got the row
								IRowSettingsPtr pRow(lpDisp);
								lpDisp->Release();

								// Decide if we have to uncheck both, or check one
								{
									BOOL bPermAvail = (pRow->GetValue(COL_PERM).vt != VT_NULL);
									BOOL bPermWithPassAvail = (pRow->GetValue(COL_PERM_WITHPASS).vt != VT_NULL);
									switch (nResult) {
									case miSelectAll:
										// Try to select the perm and unselect the perm w/ pass
										if (bPermAvail) {
											// We can set the perm, so set it and try to unset the perm w/ pass
											pRow->PutValue(COL_USERSPECIFIC, g_cvarTrue);
											if (bPermWithPassAvail) {
												pRow->PutValue(COL_USERSPECIFIC_WITHPASS, g_cvarFalse);
											}
										} else if (bPermWithPassAvail) {
											// Couldn't set or unset the perm, so just make sure the perm w/ pass is set
											pRow->PutValue(COL_USERSPECIFIC_WITHPASS, g_cvarTrue);
										} else {
											ASSERT(FALSE);
										}
										break;
									case miSelectAllWithPass:
										// Try to select the perm w/ pass and unselect the perm
										if (bPermWithPassAvail) {
											// We can set the perm, so set it and try to unset the perm w/ pass
											pRow->PutValue(COL_USERSPECIFIC_WITHPASS, g_cvarTrue);
											if (bPermAvail) {
												pRow->PutValue(COL_USERSPECIFIC, g_cvarFalse);
											}
										} else if (bPermAvail) {
											// Couldn't set or unset the perm w/ pass, so just make sure the perm is set
											pRow->PutValue(COL_USERSPECIFIC, g_cvarTrue);
										} else {
											ASSERT(FALSE);
										}
										break;
									case miUnselectAll:
										if (bPermWithPassAvail) {
											// We can unset the perm, so unset it 
											pRow->PutValue(COL_USERSPECIFIC_WITHPASS, g_cvarFalse);
										}
										if (bPermAvail) {
											// We can unset the perm w/ pass, so unset it 
											pRow->PutValue(COL_USERSPECIFIC, g_cvarFalse);
										}
										break;	
									}
								}
							}
						}
					}
				} else {
					ASSERT(FALSE);
				}
			}
		}
	} NxCatchAll("CUserGroupSecurityDlg::OnContextMenu");
}

HBRUSH CUserGroupSecurityDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	switch(pWnd->GetDlgCtrlID())
	{
		case IDC_EDIT_SECURITYOBJECT_DESCRIPTION:
		{
			// (z.manning, 05/16/2008) - PLID 30050 - make borderless edit controls transparent
			pDC->SetBkColor(GetSolidBackgroundColor());
			return m_brBackground;
		}
		break;
	}

	return CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}


// (j.gruber 2010-04-13 16:42) - PLID 37948 - open the configuration dialog
void CUserGroupSecurityDlg::OnBnClickedUserSecConfigureGroups()
{
	try {

		//make sure permissions are saved
		SavePermissions(FALSE);

		//check that they have permission to open other users first
		if (CheckCurrentUserPermissions(bioEditOtherUserPermissions, sptWrite)) {

			//close this screen and open the next
			if (m_mapPermChanges.GetSize() != 0) {
				if (!m_bIsGroup) {
					if (IDNO == MsgBox(MB_YESNO, "The permissions of this user must be saved before configuring Permission Groups. \nWould you like to save these permissions now?")) {
						return;
					}
				}
				else {
					// (j.gruber 2010-08-17 09:43) - PLID 40138 - change template to groups
					if (IDNO == MsgBox(MB_YESNO, "The permissions of this group must be saved before configuring Permission Groups. \nWould you like to save these permissions now?")) {
						return;
					}
				}

			}

			if (SavePermissions(TRUE, TRUE)) {
				return;
			}

			CNxDialog::OnOK();
			
			CConfigurePermissionGroupsDlg dlg(!m_bIsGroup, m_lPersonID, this);
			dlg.DoModal();
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2010-04-13 16:44) - PLID 37947
void CUserGroupSecurityDlg::RunSummaryReport() 
{

	//first see if they want to save
	if (m_mapPermChanges.GetSize() > 0) {
		if (!m_bIsGroup) {
			if (IDNO == MsgBox(MB_YESNO, "The permissions for this user need to be saved before running this report.  Would you like to continue?")) {
				//reenabled the list				
				m_pSummaryList->Enabled = true;
				return;
			}
		}
		else {
			// (j.gruber 2010-08-17 09:44) - PLID 40138 - change template to group
			if (IDNO == MsgBox(MB_YESNO, "The permissions for this group need to be saved before running this report.  Would you like to continue?")) {
				//reenabled the list				
				m_pSummaryList->Enabled = true;
				return;
			}
		}
	}

	if (SavePermissions(TRUE, FALSE)) {
		return;
	}

	//setup the report
	CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(555)]);
	infReport.nProvider = 1;
	infReport.strExternalFilter = "PermsByUserName.PersonID = " + AsString(m_lPersonID);
	RunReport(&infReport, TRUE, this, "Additional Permissions");

	//close the dialog
	CNxDialog::OnOK();


}

// (j.gruber 2010-04-13 16:44) - PLID 37947
void CUserGroupSecurityDlg::LeftClickSummaryList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow) {
			if (nCol == slcName) {

				long nID = VarLong(pRow->GetValue(slcID));
				if (pRow->GetCellLinkStyle(slcName) == NXDATALIST2Lib::dlLinkStyleTrue){

					if (nID == -1) {
						//they clicked the link
						//make sure they can't click on it again
						m_pSummaryList->Enabled = false;
						// (j.gruber 2010-08-05 09:52) - PLID 37947 - Save any changes we may have made to memory
						SavePermissions(FALSE, FALSE);
						RunSummaryReport();
					}
				}
			}
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2010-04-13 16:45) - PLID 38015
void CUserGroupSecurityDlg::EditingStartingListPermissions(long nRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue)
{
	try {

		NXDATALISTLib::IRowSettingsPtr pRow = m_dlPerms->GetRow(nRow);

		if (pRow) {
			//get the forecolor
			OLE_COLOR color = pRow->GetForeColor();
			if (color == RGB(125,125,125)) {
				//its disabled
				*pbContinue = FALSE;
				//give a messagebox
				// (j.gruber 2010-08-17 09:44) - PLID 40138 - change template to group
				MsgBox("This permission comes from a group and cannot be changed on the user.  Please change the permission on the group or remove the user from the group if you would like to change it.");
			}
		}
	}NxCatchAll(__FUNCTION__);

}

// (j.gruber 2010-04-13 16:45) - PLID 23982
void CUserGroupSecurityDlg::OnBnClickedPermsClear()
{
	try {
		if (m_bIsGroup) {
			//we are clearing all permissions on this template
			//make sure they want to do this first
			// (j.gruber 2010-08-17 09:44) - PLID 40138 - change template to group
			if (IDNO == MsgBox(MB_YESNO, "This will clear all permissions from this group and remove these permissions from the users in this permission group if they do not exist in other groups the user is in. \nAre you sure you want to continue?")) {
				return;
			}
		}
		else {
			if (IDNO == MsgBox(MB_YESNO, "This will clear all permissions from this user and reset the user to only having the permissions of the groups it is in. \nAre you sure you want to continue?")) {
				return;
			}
		}		
		//load the change template with all permissions set to 0
		m_mapPermChanges.RemoveAll();

		if (m_bIsGroup) {
			CString strTableName;
			CMap<EBuiltInObjectIDs, EBuiltInObjectIDs, DWORD, DWORD> mapTemplates;
			LoadPermissionsIntoMap(strTableName, m_lPersonID, &mapTemplates, -1, TRUE);
			LoadBlankPermissionMap(&m_mapPermChanges, &mapTemplates);		

			if (m_mapPermChanges.GetSize() == 0) {
				// (j.gruber 2010-08-17 09:44) - PLID 40138 - change template to group
				MsgBox("The current group has no permissions.");
				return;
			}
		}
		else {
			LoadBlankPermissionMap(&m_mapPermChanges);	
		}
		
		if (SavePermissions(TRUE, FALSE)) {
			return;	
		}	
		CNxDialog::OnOK();
	}NxCatchAll(__FUNCTION__);
}

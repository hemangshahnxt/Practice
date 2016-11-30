//PermissionUtils.cpp
#include "stdafx.h"
#include "nxsecurity.h"
#include "audittrail.h"
#include "PermissionUtils.h"


// (d.thompson 2013-03-26) - PLID 55847 - Refactored out the GeneratePermissionUpdateString() functions into CGeneratePermissionUpdateBatch


// (j.gruber 2010-04-13 16:46) - PLID 23982
DWORD GetPermisson(const CBuiltInObject* pObj, const ESecurityPermissionType esptPermType, const ESecurityPermissionType esptPermTypeWithPass, DWORD dwPermissionsMap1, DWORD dwPermissionsMap2)
{
	// Determine which, if either, of the "perm type" or the "perm type with pass" permissions are available
	BOOL bPermExists, bPermWithPassExists;
	{
		if (pObj->m_AvailPermissions.nPermissions & esptPermType) {
			bPermExists = TRUE;
		} else {
			bPermExists = FALSE;
		}
		if (pObj->m_AvailPermissions.nPermissions & esptPermTypeWithPass) {
			bPermWithPassExists = TRUE;
		} else {
			bPermWithPassExists = FALSE;
		}
	}

	// Now if either permission is available, we're going to be checking
	if (bPermExists || bPermWithPassExists) {
			
		// Now if the permission exists, check the permission against the template
		{
			// Do it for the "perm type" permission
			//Perm type first because it is "higher"
			if (bPermExists) {
				if ((dwPermissionsMap1|dwPermissionsMap2) & esptPermType) {
					return esptPermType;
				}			
			}
			
			// check the "perm type with pass" permission
			if (bPermWithPassExists) {
				if ((dwPermissionsMap1|dwPermissionsMap2) & esptPermTypeWithPass) {
					return esptPermTypeWithPass;
				}
			}

			//if we got here, neither map has the permissions
			return 0;
		}		
	} else {
		// No permission of this type available, so return 0
		return 0;
	}
}

// (j.gruber 2010-04-13 16:46) - PLID 23982
//(e.lally 2010-10-14) PLID 40912 - Added bIncludeSavedUserPerms. When set it means that we are also considering the permission to be set if the current user's saved permissions has it.
//	When unset, it means that the mapChange and mapOtherTemplates reflect the new permissions. Otherwise we don't know if the change was unset or meant we should remove the permission.
DWORD GetPermisson(const CBuiltInObject* pObj, const ESecurityPermissionType esptPermType, const ESecurityPermissionType esptPermTypeWithPass, DWORD dwPermissionsMapChange, DWORD dwPermissionsMapCurUser, DWORD dwPermissionsMapOtherTemplates, bool bIncludeSavedUserPerms, BOOL &bChange)
{
	// Determine which, if either, of the "perm type" or the "perm type with pass" permissions are available
	BOOL bPermExists, bPermWithPassExists;
	{
		if (pObj->m_AvailPermissions.nPermissions & esptPermType) {
			bPermExists = TRUE;
		} else {
			bPermExists = FALSE;
		}
		if (pObj->m_AvailPermissions.nPermissions & esptPermTypeWithPass) {
			bPermWithPassExists = TRUE;
		} else {
			bPermWithPassExists = FALSE;
		}
	}

	// Now if either permission is available, we're going to be checking
	if (bPermExists || bPermWithPassExists) {
		DWORD dwConditionalUserPerms = 0;
		if(bIncludeSavedUserPerms){
			//(e.lally 2010-10-14) PLID 40912 - We also want to check the 
			//	current user (external only) permissions
			dwConditionalUserPerms = dwPermissionsMapCurUser;
		}
		// Now if the permission exists, check the permission against the template
		{
			// Do it for the "perm type" permission
			//Perm type first because it is "higher"
			if (bPermExists) {
				if ((dwPermissionsMapChange|dwPermissionsMapOtherTemplates) & esptPermType) {
					//see if we need to output a message
					if (((dwPermissionsMapChange|dwPermissionsMapOtherTemplates) & esptPermType) != (dwPermissionsMapCurUser & esptPermType)) {
						//the user permission will be changing
						bChange = TRUE;
					}

					return esptPermType;
				}
				//(e.lally 2010-10-14) PLID 40912 - Check the conditional user permissions set above
				if ((dwConditionalUserPerms) & esptPermType) {
					return esptPermType;
				}
			}
			
			// check the "perm type with pass" permission
			if (bPermWithPassExists) {
				if ((dwPermissionsMapChange|dwPermissionsMapOtherTemplates) & esptPermTypeWithPass) {
					if (((dwPermissionsMapChange|dwPermissionsMapOtherTemplates) & esptPermTypeWithPass) != (dwPermissionsMapCurUser&esptPermTypeWithPass)) {
						//the user permission will be changing
						bChange = TRUE;
					}
					return esptPermTypeWithPass;
				}
				//(e.lally 2010-10-14) PLID 40912 - Check the conditional user permissions set above
				if ((dwConditionalUserPerms) & esptPermTypeWithPass) {
					return esptPermTypeWithPass;
				}
			}

			//if we got here, neither the change or templates map has the permissions
			//and conditionally the saved user permissions doesn't either
			return 0;
		}		
	} else {
		// No permission of this type available, so return 0
		return 0;
	}
}


// (j.gruber 2010-04-13 16:46) - PLID 38085 - generate what has changed
void SaveChangeString(BOOL &bChange, CString strFirstSection, CString strType, CString &strChangeString) {

	if (bChange) {

		strChangeString += strFirstSection + ": " + strType + "\r\n";
		bChange = FALSE;
	}
}

// (j.gruber 2010-04-13 16:46) - PLID 23982
CString CreateAllPermissionsTable() {

	//first do all the built in IDs

	POSITION pos = GetFirstBuiltInObjectPosition();
	CString strSql = BeginSqlBatch();

	CString strTableName;
	strTableName.Format("#TempPermissions%lu", GetTickCount());

	AddStatementToSqlBatch(strSql, "CREATE TABLE %s (ObjectID INT NOT NULL PRIMARY KEY) ", strTableName);
	while (pos)
	{
		
		const CBuiltInObject* pObj = GetNextBuiltInObject(pos);

		AddStatementToSqlBatch(strSql, "INSERT INTO %s (ObjectID) VALUES (%li)", strTableName, pObj->m_eBuiltInID);
	}

	//now do the userdefined IDs
	ADODB::_RecordsetPtr rsUserDefined = CreateParamRecordset("SELECT ID FROM SecurityObjectT");
	while (! rsUserDefined->eof) {
		AddStatementToSqlBatch(strSql, "INSERT INTO %s (ObjectID) VALUES (%li)", strTableName, AdoFldLong(rsUserDefined, "ID"));

		rsUserDefined->MoveNext();
	}

	
	// (a.walling 2012-02-09 17:13) - PLID 48115 - Use NxAdo::PushMaxRecordsWarningLimit and NxAdo::PushPerformanceWarningLimit
	NxAdo::PushMaxRecordsWarningLimit pmr(1000000);
	ExecuteSqlBatch(strSql);

	return strTableName;

}

// (j.gruber 2010-04-13 16:46) - PLID 23982
void LoadPermissionsIntoMap(CString &strTableName, long nUserGroupID, CMap<EBuiltInObjectIDs,EBuiltInObjectIDs,DWORD,DWORD> *pMapTemplates, long nUserGroupIDToExclude, BOOL bAddUserPerms	){

	try {

		CString strSelect, strFrom;

		if (strTableName.IsEmpty()) {
			strTableName = CreateAllPermissionsTable();
		}

		//first setup the user
		strSelect.Format("SELECT %s.ObjectID, UserPermsT.Permissions as UserPerms,  ", strTableName);
		strFrom.Format(" FROM %s LEFT JOIN (SELECT * FROM PermissionT WHERE UserGroupID = %li) UserPermsT ON %s.ObjectID = UserPermsT.ObjectID ", strTableName, nUserGroupID, strTableName);
		CString strWhere;
		if (nUserGroupIDToExclude != -1) {
			strWhere.Format(" AND GroupID <> %li", nUserGroupIDToExclude);
		}

		//first get all the templates they are in
		BOOL bHasTemplates = FALSE;
		//(e.lally 2010-10-14) PLID 40912 - Always check for group detail permissions so we can distinguish which user permissions are outside of the groups they belong to
		//if (!bAddUserPerms) 
		{
			ADODB::_RecordsetPtr rsGroups = CreateRecordset("SELECT GroupID FROM UserGroupDetailsT WHERE UserID = %li %s ", nUserGroupID, strWhere);
			if (!rsGroups->eof) {
				while (!rsGroups->eof) {

					bHasTemplates = TRUE;

					CString strTemp;
					long nGroupID = AdoFldLong(rsGroups,"GroupID");
					strTemp.Format(" COALESCE(PermT%li.Permissions, 0) | ", nGroupID);
					strSelect += strTemp;
					strTemp.Format(" LEFT JOIN (SELECT * FROM PermissionT WHERE UserGroupID = %li) PermT%li ON %s.ObjectID = PermT%li.ObjectID", nGroupID, nGroupID, strTableName, nGroupID);
					strFrom += strTemp;

					rsGroups->MoveNext();
				}

				//take off the last |
				strSelect.TrimRight("| ");

				strSelect += " as TemplatePerms ";
			}
			else {
				//trim the comma
				//(e.lally 2010-10-14) PLID 40912 - Instead of trimming, include the default value 0 
				//	for no details. This applies when the nUserGroupID was a group ID (and therefore can't belong to other groups)
				//	or the user did not belong to any groups.
				//strSelect.TrimRight(", ");
				strSelect += " 0 as TemplatePerms ";
			}
		}
		/*
		//(e.lally 2010-10-14) PLID 40912 - No longer needed since we always look at the group details.
		else {
			//trim the comma
			strSelect.TrimRight(", ");
		}			
		*/			


		//Now run the recordset
		ADODB::_RecordsetPtr rs = CreateRecordsetStd(strSelect + strFrom);

		//now go through and fill our map
		//first clear the map		
		pMapTemplates->RemoveAll();
				
		while (!rs->eof) {

			EBuiltInObjectIDs nObjectID = (EBuiltInObjectIDs)AdoFldLong(rs, "ObjectID");
			DWORD dwPerms = 0;
			if (bAddUserPerms) {
				//(e.lally 2010-10-14) PLID 40912 - The effective permissions are just the user or group permissions
				//that are not included in the template permissions (user's group details). Since groups do not have
				//other group details it will always be it's own permissions.
				DWORD dwUserGroupPerms = AdoFldLong(rs, "UserPerms", 0);
				DWORD dwTemplatePerms = AdoFldLong(rs, "TemplatePerms", 0);
				//permissions for the user/group but not in their templates
				dwPerms = (dwUserGroupPerms & (~dwTemplatePerms));
			}
			else {
				if (bHasTemplates) {
					dwPerms = AdoFldLong(rs, "TemplatePerms", 0);
				}
			}

			pMapTemplates->SetAt(nObjectID, dwPerms);			
			

			rs->MoveNext();
		}			
	
	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2010-04-13 16:46) - PLID 23982
void LoadBlankPermissionMap(CMap<EBuiltInObjectIDs,EBuiltInObjectIDs,DWORD,DWORD> *pMapToBlank, CMap<EBuiltInObjectIDs,EBuiltInObjectIDs,DWORD,DWORD> *pMapUsedOjects/* = NULL*/) {

	POSITION pos = GetFirstBuiltInObjectPosition();
	
	if (pMapUsedOjects != NULL) {
		while (pos)
		{			
			const CBuiltInObject* pObj = GetNextBuiltInObject(pos);

			DWORD perms;
			if (pMapUsedOjects->Lookup(pObj->m_eBuiltInID, perms)) {
				if (perms != 0) {
					pMapToBlank->SetAt(pObj->m_eBuiltInID, 0);
				}
			}
		}

	}
	else {
		while (pos)
		{			
			const CBuiltInObject* pObj = GetNextBuiltInObject(pos);

			pMapToBlank->SetAt(pObj->m_eBuiltInID, 0);
		}

	}


	//now do the userdefined IDs
	ADODB::_RecordsetPtr rsUserDefined = CreateParamRecordset("SELECT ID FROM SecurityObjectT");
	if (pMapUsedOjects != NULL) {

		while (! rsUserDefined->eof) {
		
			EBuiltInObjectIDs ebioID = (EBuiltInObjectIDs)AdoFldLong(rsUserDefined, "ID");

			DWORD dwPerms;

			if (pMapUsedOjects->Lookup(ebioID, dwPerms)) {

				if (dwPerms != 0) {
					pMapToBlank->SetAt(ebioID, 0);
				}
			}
			
			rsUserDefined->MoveNext();
		}


	}
	else {

		while (! rsUserDefined->eof) {
		
			EBuiltInObjectIDs ebioID = (EBuiltInObjectIDs)AdoFldLong(rsUserDefined, "ID");
			pMapToBlank->SetAt(ebioID, 0);
			
			rsUserDefined->MoveNext();
		}
	}
}


//pulled directly from UserGroupSecurity.cpp
void AdjustViewPerms(IN const EBuiltInObjectIDs ebio, IN OUT DWORD &dwPerms)
{
	const CBuiltInObject *pObj = NULL;
	if(ebio < 0)
		pObj = GetBuiltInObject(ebio);
	else {
		pObj = GetUserDefinedObject(ebio);
	}
	ASSERT(pObj);
	if (pObj) {
		
		// (a.walling 2007-11-07 10:18) - PLID 27998 - VS2008 - Need to specify a type
		const long nsptAllPermsAndPassExceptView = ((SPT_VRWCD01234|SPT_VRWCD01234_ONLYWITHPASS) & ~(sptView|sptViewWithPass));

		// If the object has ANY permissions other than view or view w/pass available
		if (pObj->m_AvailPermissions.nPermissions & nsptAllPermsAndPassExceptView)
		{
			// If the object has view permissions available
			if (pObj->m_AvailPermissions.nPermissions & sptView)
			{
				//DRT 4/1/03 - Due to change with the default permissions stuff, we removed
				//		the code that always disabled the view with pass permission if an object
				//		had something else in addition.  So this assert will often end asserting.
				//ASSERT((pObj->m_AvailPermissions.nPermissions & sptViewWithPass) == 0);

				// If the dwPerms has any no-pass permissions other than view
				if (dwPerms & nsptAllPermsAndPassExceptView) {
					// Set the view perm
					dwPerms |= sptView;
				} else {
					// Unset the view perm
					dwPerms &= ~sptView;
				}
			}
		}
	}
}

// (j.gruber 2010-04-13 16:38) - PLID 37947 - used in checking summary screen changes
BOOL CheckPermissionChanged(const CBuiltInObject *pbio, ESecurityPermissionType esptPermType, ESecurityPermissionType esptPermTypeWithPass, DWORD dwUserPerms, DWORD dwTempPerms) {


	// Determine which, if either, of the "perm type" or the "perm type with pass" permissions are available
	BOOL bPermExists, bPermWithPassExists;
	{
		if (pbio->m_AvailPermissions.nPermissions & esptPermType) {
			bPermExists = TRUE;
		} else {
			bPermExists = FALSE;
		}
		if (pbio->m_AvailPermissions.nPermissions & esptPermTypeWithPass) {
			bPermWithPassExists = TRUE;
		} else {
			bPermWithPassExists = FALSE;
		}
	}

	// Now if either permission is available, we're going to be checking
	if (bPermExists || bPermWithPassExists) {

		// Now if the permission exists, check the permission against the template
		{
			// Do it for the "perm type" permission
			//Perm type first because it is "higher"
			if (bPermExists) {
				if ((dwUserPerms) & esptPermType) {
					//the user has this permission, see if the tempalte doesn't
					if (((dwTempPerms) & esptPermType) != (dwUserPerms & esptPermType)) {
						return TRUE;		
					}

					return FALSE;
				}			
			}
			
			// check the "perm type with pass" permission
			if (bPermWithPassExists) {
				if ((dwUserPerms) & esptPermTypeWithPass) {
					//the user has this permission, see if the tempalte doesn't
					if (((dwTempPerms) & esptPermTypeWithPass) != (dwUserPerms & esptPermTypeWithPass)) {
						return TRUE;				
					}

					return FALSE;
				}		
			}

			//if we got here, neither map has the permissions
			return FALSE;
		}		
	} else {
		// No permission of this type available, so return
		return FALSE;
	}
}
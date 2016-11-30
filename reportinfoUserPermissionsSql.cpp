//ReportInfoUserPermissionsSql.cpp


#include "stdafx.h"
#include "ReportInfoUserPermissionsSql.h"
#include "PermissionUtils.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


using namespace ADODB;
//loops through the list of Built In Objects and returns the Name of the ID passed in
CString GetBioName(EBuiltInObjectIDs nID) {

	const CBuiltInObject* pObj = NULL;
	pObj = GetBuiltInObject(nID);
	if (pObj) {
		return pObj->m_strDisplayName;
	}
	else {
		return "";
	}
}

long GetGrandParentID (EBuiltInObjectIDs nParentID) {

	
	const CBuiltInObject* pObj = NULL;

	pObj = GetBuiltInObject(nParentID);

	if (pObj) {
		if (pObj->m_eParentID == bioInvalidID) {
			return bioInvalidID;
		}
		else {
			return pObj->m_eParentID;
		}	
	}
	else {
		return bioInvalidID;
	}	
}


// (j.gruber 2010-04-12 09:49) - PLID 37949 - redid entire report
// (j.gruber 2010-04-12 15:52) - PLID 37947 - added bool to only show extra perms which is actually overriding the nProvider variable
_RecordsetPtr GetUserPermissionsRecordset(long nSubLevel, long nSubReport, CString strExtraFilter, BOOL bForReportVerify, long nShowOnlyExtraPerms) {

	//first load our recordset that we are going to be adding to
	_RecordsetPtr rsUserPerms(__uuidof(Recordset));
	rsUserPerms->CursorLocation = adUseClient;

	// (a.walling 2009-08-11 13:25) - PLID 35178 - Use the snapshot connection
	rsUserPerms->Open((LPCTSTR)"SELECT ID AS NodeID, DisplayName as NodeName, ID AS ParentID, "
		" DisplayName AS ParentName, ID AS GrandParentID, DisplayName as GrandParentName, ID as UserID, "
		" DisplayName AS UserName, DisplayName as PermissionName, Id as PermissionValue, "
		" ObjectValue as GroupVal "
		" FROM SecurityObjectT WHERE 1 = 0", (LPDISPATCH)GetRemoteDataSnapshot(), adOpenKeyset, adLockBatchOptimistic,adOptionUnspecified);
	rsUserPerms->PutRefActiveConnection(NULL);

	if (bForReportVerify) {
		//skip everything
		return rsUserPerms;
	}
			
	FieldsPtr flds;
	flds = rsUserPerms->Fields;

	CString strUserIDs;
	strUserIDs = strExtraFilter;

	strUserIDs.Replace("}", "");
	strUserIDs.Replace("{", "");

	//First load the admin users, because we aren't doing anything else with the
	// (j.gruber 2010-04-12 15:55) - PLID 37947 - check if we are only showing extras
	if (nShowOnlyExtraPerms != 1) {
		LoadAdminUsers(GetRemoteDataSnapshot(), rsUserPerms, strUserIDs);
	}

	//now load the templates for all the users into the recordset	
	// (j.gruber 2010-04-12 15:55) - PLID 37947 - check if we are only showing extras
	if (nShowOnlyExtraPerms != 1) {
		LoadUserTemplates(GetRemoteDataSnapshot(), rsUserPerms, strUserIDs);
	}

	//now load all the extra permissions and the templates
	LoadUserPermissions(GetRemoteDataSnapshot(), rsUserPerms, strUserIDs);

	return rsUserPerms;
}

// (j.gruber 2010-04-12 09:57) - PLID 37949
void LoadAdminUsers(ADODB::_Connection* lpCon, ADODB::_RecordsetPtr &rsUserPerms, CString strUserIDs) {

	_RecordsetPtr rsAdmins = CreateRecordset(lpCon, "SELECT * FROM (SELECT PersonID, Username FROM UsersT WHERE PersonID > 0 AND Administrator = 1) PermsByUserName %s ", 
		strUserIDs.IsEmpty() ? "" : " WHERE " + strUserIDs);
	FieldsPtr flds = rsUserPerms->Fields;

	while (! rsAdmins->eof) {

		rsUserPerms->AddNew();
		flds->Item["NodeID"]->Value = (long)bioInvalidID;
		flds->Item["NodeName"]->Value = _variant_t("Administrator User - Has All Permissions");
		flds->Item["ParentID"]->Value = (long)bioInvalidID;
		flds->Item["ParentName"]->Value = _variant_t("");
		flds->Item["GrandParentID"]->Value = (long)bioInvalidID;
		flds->Item["GrandParentName"]->Value = _variant_t("");
		flds->Item["UserID"]->Value = AdoFldLong(rsAdmins, "PersonID");
		flds->Item["UserName"]->Value = _variant_t(AdoFldString(rsAdmins, "UserName", ""));
		flds->Item["PermissionName"]->Value = _variant_t("");
		flds->Item["PermissionValue"]->Value = (long)0;
		flds->Item["GroupVal"]->Value = (long)1;		
		rsUserPerms->Update();

		rsAdmins->MoveNext();
	}
}

// (j.gruber 2010-04-12 09:57) - PLID 37949
void LoadUserTemplates(ADODB::_Connection* lpCon, ADODB::_RecordsetPtr &rsUserPerms, CString strUserIDs) {

	_RecordsetPtr rsTemplates = CreateRecordset(lpCon, "SELECT * FROM (SELECT UsersT.PersonID as PersonID, Username, UserGroupsT.PersonID as GroupID, UserGroupsT.Name as GroupName FROM  "
		" UserGroupDetailsT LEFT JOIN UsersT ON UserGroupDetailsT.UserID = UsersT.PersonID "
		" LEFT JOIN UserGroupsT ON UserGroupDetailsT.GroupID = UserGroupsT.PersonID "
		" WHERE UsersT.PersonID > 0 AND Administrator = 0) PermsByUserName %s", 
		strUserIDs.IsEmpty() ? "" : " WHERE " + strUserIDs);
	FieldsPtr flds = rsUserPerms->Fields;

	while (! rsTemplates->eof) {

		rsUserPerms->AddNew();
		flds->Item["NodeID"]->Value = (long)bioInvalidID;
		flds->Item["NodeName"]->Value = _variant_t(AdoFldString(rsTemplates, "GroupName", ""));
		flds->Item["ParentID"]->Value = (long)bioInvalidID;
		flds->Item["ParentName"]->Value = _variant_t("Member of Group(s):");
		flds->Item["GrandParentID"]->Value = (long)bioInvalidID;
		flds->Item["GrandParentName"]->Value = _variant_t("");
		flds->Item["UserID"]->Value = AdoFldLong(rsTemplates, "PersonID");
		flds->Item["UserName"]->Value = _variant_t(AdoFldString(rsTemplates, "UserName", ""));
		flds->Item["PermissionName"]->Value = _variant_t(AdoFldString(rsTemplates, "GroupName", ""));
		flds->Item["PermissionValue"]->Value = (long)0;
		flds->Item["GroupVal"]->Value = (long)2;		
		rsUserPerms->Update();

		rsTemplates->MoveNext();
	}
}


// (j.gruber 2010-04-12 09:57) - PLID 37949
void LoadUserPermissions(ADODB::_Connection* lpCon, ADODB::_RecordsetPtr &rsUserPerms, CString strUserIDs) {

	_RecordsetPtr rsUsers = CreateRecordset(lpCon, "SELECT * FROM (SELECT UsersT.PersonID as PersonID, Username, 0 as IsTemplate "
		" FROM UsersT "
		" WHERE PersonID > 0 AND Administrator = 0 "
		" UNION "
		" SELECT PersonID, Name, 1 as IsTemplate FROM UserGroupsT UsersT WHERE 1=1) PermsByUsername %s", 
		strUserIDs.IsEmpty() ? "" : " WHERE " + strUserIDs);	

	CMap<EBuiltInObjectIDs,EBuiltInObjectIDs,DWORD,DWORD>  mapUser;
	CMap<EBuiltInObjectIDs,EBuiltInObjectIDs,DWORD,DWORD>  mapTemplates;
	CString strTableName;

	while (! rsUsers->eof) {

		long nUserID = AdoFldLong(rsUsers, "PersonID");
		CString strUserName = AdoFldString(rsUsers, "UserName");
		long nIsTemplate = AdoFldLong(rsUsers, "IsTemplate");
		long nGroupVal;

		if (nIsTemplate) {
			LoadPermissionsIntoMap(strTableName, nUserID, &mapUser, -1, TRUE);
			LoadBlankPermissionMap(&mapTemplates);
			nGroupVal = 3;
		}
		else {

			//we need to load the permissions for this user and their templates
			LoadPermissionsIntoMap(strTableName, nUserID, &mapUser, -1, TRUE);
			LoadPermissionsIntoMap(strTableName, nUserID, &mapTemplates, -1, FALSE);
			nGroupVal = 4;
		}

		//now we have to loop through the maps
		//we can do this because our maps load every possible permission
		POSITION pos = mapUser.GetStartPosition();
		while (pos) {
			EBuiltInObjectIDs ebioUser;
			DWORD dwUserPerms;

			mapUser.GetNextAssoc(pos, ebioUser, dwUserPerms);

			//now look up our template value
			DWORD dwTempPerms;
			if (mapTemplates.Lookup(ebioUser, dwTempPerms)) {

				//do our user permissions match the template permissions?
				if (dwUserPerms != dwTempPerms) {

					//they didn't match exactly, so we know we have some user perms that the tempaltes don't have
					//so we need to output them
					AddPermsToRecordset(rsUserPerms, ebioUser, dwUserPerms, dwTempPerms, nUserID, strUserName, nGroupVal);
				}
			}
		}
		
		rsUsers->MoveNext();
	}
}

void CheckPermission(ADODB::_RecordsetPtr &rsUserPerms, const CBuiltInObject *pbio, CString strDescription, const ESecurityPermissionType esptPermType, const ESecurityPermissionType esptPermTypeWithPass, DWORD dwUserPerms, DWORD dwTempPerms, long nUserID, CString strUserName, long nGroupVal) {

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

		EBuiltInObjectIDs ebioParentID = pbio->m_eParentID;
		EBuiltInObjectIDs ebioGParentID = (EBuiltInObjectIDs)GetGrandParentID(ebioParentID);
		CString strParentName = GetBioName(ebioParentID);
		CString strGParentName = GetBioName(ebioGParentID);
		FieldsPtr flds = rsUserPerms->Fields;

		//take out internal
		bool bAdd = false;
		if(pbio->m_eBuiltInID == bioInternalOnly || pbio->m_eParentID == bioInternalOnly) {
			if(IsNexTechInternal())
				bAdd = true;
		}
		else {
			bAdd = true;
		}

			
		if (bAdd)  {
			// Now if the permission exists, check the permission against the template
			{
				// Do it for the "perm type" permission
				//Perm type first because it is "higher"
				if (bPermExists) {
					if ((dwUserPerms) & esptPermType) {
						//the user has this permission, see if the tempalte doesn't
						if (((dwTempPerms) & esptPermType) != (dwUserPerms & esptPermType)) {
							//the user has the permisison, but the template doesn't, so we need to add it to our recordset
							rsUserPerms->AddNew();
							flds->Item["NodeID"]->Value = (long)pbio->m_eBuiltInID;
							flds->Item["NodeName"]->Value = _variant_t(pbio->m_strDisplayName);
							flds->Item["ParentID"]->Value = (long)pbio->m_eParentID;
							flds->Item["ParentName"]->Value = _variant_t(strParentName);
							flds->Item["GrandParentID"]->Value = (long)ebioGParentID;
							flds->Item["GrandParentName"]->Value = _variant_t(strGParentName);
							flds->Item["UserID"]->Value = (long)nUserID;
							flds->Item["UserName"]->Value = _variant_t(strUserName);
							flds->Item["PermissionName"]->Value = _variant_t(strDescription);
							flds->Item["PermissionValue"]->Value = (long)1;
							flds->Item["GroupVal"]->Value = nGroupVal;						
							rsUserPerms->Update();						
						}

						return;
					}			
				}
				
				// check the "perm type with pass" permission
				if (bPermWithPassExists) {
					if ((dwUserPerms) & esptPermTypeWithPass) {
						//the user has this permission, see if the tempalte doesn't
						if (((dwTempPerms) & esptPermTypeWithPass) != (dwUserPerms & esptPermTypeWithPass)) {
							//the user has the permisison, but the template doesn't, so we need to add it to our recordset
							rsUserPerms->AddNew();
							flds->Item["NodeID"]->Value = (long)pbio->m_eBuiltInID;
							flds->Item["NodeName"]->Value = _variant_t(pbio->m_strDisplayName);
							flds->Item["ParentID"]->Value = (long)pbio->m_eParentID;
							flds->Item["ParentName"]->Value = _variant_t(strParentName);
							flds->Item["GrandParentID"]->Value = (long)ebioGParentID;
							flds->Item["GrandParentName"]->Value = _variant_t(strGParentName);
							flds->Item["UserID"]->Value = (long)nUserID;
							flds->Item["UserName"]->Value = _variant_t(strUserName);
							flds->Item["PermissionName"]->Value = _variant_t(strDescription + " With Pass");
							flds->Item["PermissionValue"]->Value = (long)1;
							flds->Item["GroupVal"]->Value = nGroupVal;						
							rsUserPerms->Update();						
						}

						return;
					}		
				}
			}

			//if we got here, neither map has the permissions
			return;
		}		
	} else {
		// No permission of this type available, so return
		return;
	}

}

void AddPermsToRecordset(_RecordsetPtr &rsUserPerms, EBuiltInObjectIDs ebio, DWORD dwUserPerms, DWORD dwTempPerms, long nUserID, CString strUserName, long nGroupVal) {

	//figure out which permissions are different then the templates
	const CBuiltInObject *pbio;
	if (ebio < 0) {
		pbio = GetBuiltInObject(ebio);
	}
	else {
		pbio = GetUserDefinedObject(ebio);
	}

	CheckPermission(rsUserPerms, pbio, "Delete", sptDelete, sptDeleteWithPass, dwUserPerms, dwTempPerms, nUserID, strUserName, nGroupVal);
	CheckPermission(rsUserPerms, pbio, "Create", sptCreate, sptCreateWithPass, dwUserPerms, dwTempPerms, nUserID, strUserName, nGroupVal);
	CheckPermission(rsUserPerms, pbio, "Read", sptRead, sptReadWithPass, dwUserPerms, dwTempPerms, nUserID, strUserName, nGroupVal);
	CheckPermission(rsUserPerms, pbio, "Write", sptWrite, sptWriteWithPass, dwUserPerms, dwTempPerms, nUserID, strUserName, nGroupVal);
	if ((pbio->m_AvailPermissions.nPermissions & (~(sptView|sptViewWithPass))) == 0) {
		CheckPermission(rsUserPerms, pbio, "Access", sptView, sptViewWithPass, dwUserPerms, dwTempPerms, nUserID, strUserName, nGroupVal);
	}
	CheckPermission(rsUserPerms, pbio, pbio->m_strDynamicPermissionNames[0], sptDynamic0, sptDynamic0WithPass, dwUserPerms, dwTempPerms, nUserID, strUserName, nGroupVal);
	CheckPermission(rsUserPerms, pbio, pbio->m_strDynamicPermissionNames[1], sptDynamic1, sptDynamic1WithPass, dwUserPerms, dwTempPerms, nUserID, strUserName, nGroupVal);
	CheckPermission(rsUserPerms, pbio, pbio->m_strDynamicPermissionNames[2], sptDynamic2, sptDynamic2WithPass, dwUserPerms, dwTempPerms, nUserID, strUserName, nGroupVal);
	CheckPermission(rsUserPerms, pbio, pbio->m_strDynamicPermissionNames[3], sptDynamic3, sptDynamic3WithPass, dwUserPerms, dwTempPerms, nUserID, strUserName, nGroupVal);
	CheckPermission(rsUserPerms, pbio, pbio->m_strDynamicPermissionNames[4], sptDynamic4, sptDynamic4WithPass, dwUserPerms, dwTempPerms, nUserID, strUserName, nGroupVal);
}

//(e.lally 2008-04-07) PLID 9989 - Add parameter to flag when this is for a report verification
/*_RecordsetPtr GetUserPermissionsRecordset(long nSubLevel, long nSubReport, CString strExtraFilter, BOOL bForReportVerify) {

	//build a dynamic recordset of the data we need

	//first get the recordset of the items that are in the data
	CString strDataSql = "SELECT ObjectID, UserGroupID, Permissions FROM PermissionT ";
	//(e.lally 2008-04-07) PLID 9989 - check if we are verifying reports and add a filter to return no records
		//for a performance boost.
	if(bForReportVerify != FALSE){
		strDataSql += " WHERE 1=0 ";
	}

	// (a.walling 2009-08-11 13:25) - PLID 35178 - Use the snapshot connection
	_RecordsetPtr rsData = CreateRecordsetStd(GetRemoteDataSnapshot(), strDataSql);
	CMap<CString, LPCTSTR, long, long> pMap;
	CString strObjectID, strUserGroupID;
	long nPermissions;
	while (! rsData->eof) {
		strObjectID = AsString(AdoFldLong(rsData, "ObjectID"));
		strUserGroupID = AsString(AdoFldLong(rsData, "UserGroupID"));
		nPermissions = AdoFldLong(rsData, "Permissions");

		pMap.SetAt(strObjectID + "-" + strUserGroupID, nPermissions);

		rsData->MoveNext();		
	}
	rsData->Close();

	//now go through and get all the other permissions
	_RecordsetPtr rsUserPerms(__uuidof(Recordset));
	rsUserPerms->CursorLocation = adUseClient;

	// (a.walling 2009-08-11 13:25) - PLID 35178 - Use the snapshot connection
	rsUserPerms->Open((LPCTSTR)"SELECT ID AS NodeID, DisplayName as NodeName, ID AS ParentID, "
		" DisplayName AS ParentName, ID AS GrandParentID, DisplayName as GrandParentName, ID as UserID, "
		" DisplayName AS UserName, DisplayName as PermissionName, Id as PermissionValue FROM SecurityObjectT WHERE 1 = 0", (LPDISPATCH)GetRemoteDataSnapshot(), adOpenKeyset, adLockBatchOptimistic,adOptionUnspecified);
	rsUserPerms->PutRefActiveConnection(NULL);
			
	FieldsPtr flds;
	flds = rsUserPerms->Fields;

	//start out with a recordset of users and groups
	strExtraFilter.Replace("{", "");
	strExtraFilter.Replace("}", "");

	CString strUsersSql;
	strUsersSql.Format("SELECT PersonID, UserName, Administrator FROM "
		" (SELECT PersonID, UserName, Administrator FROM UsersT WHERE PersonID > 0 " 
		 " UNION   "
		 " SELECT PersonID, Name, 0 As Administrator FROM UserGroupsT) PermsByUsername %s ", 
		 strExtraFilter.IsEmpty() ? "" : "WHERE " + strExtraFilter);

	//(e.lally 2008-04-07) PLID 9989 - check if we are verifying reports and add a filter to return no records
		//for a performance boost.
	if(bForReportVerify != FALSE){
		if(strExtraFilter.IsEmpty()){
			strUsersSql += " WHERE 1=0 ";
		}
		else{
			ASSERT(FALSE);
			//This should not be possible, but if it were, we need to uncomment this
			//strUsersSql += " AND (1=0) ";
		}
	}

	// (a.walling 2009-08-11 13:25) - PLID 35178 - Use the snapshot connection
	_RecordsetPtr rsUsers = CreateRecordsetStd(GetRemoteDataSnapshot(), strUsersSql);
	CString strUserName;
	long nUserID;
	BOOL bIsAdmin;
	while (! rsUsers->eof) {

		strUserName = AdoFldString(rsUsers, "UserName");
		nUserID = AdoFldLong(rsUsers, "PersonID");
		bIsAdmin = AdoFldLong(rsUsers, "Administrator");

		//add all the built in IDs
		AddBuiltInObjects(rsUserPerms, &pMap, bIsAdmin, strUserName, nUserID);

		//now add all the user defined objects
		// (a.walling 2009-12-14 15:18) - PLID 35178 - Use the snapshot connection
		AddUserDefinedObjects(GetRemoteDataSnapshot(), rsUserPerms, &pMap, bIsAdmin, strUserName, nUserID);

		rsUsers->MoveNext();
	}
	return rsUserPerms;
			
			
}


void AddUserDefinedObjects(_Connection* lpCon, _RecordsetPtr &rsUserPerms, CMap<CString, LPCTSTR, long, long> *pMap, BOOL bIsAdmin, CString strUserName, long nUserID) {
	_ConnectionPtr pCon(lpCon);

	long nNodeID, nParentID, nGrandParentID;
	CString strNodeName, strParentName, strGrandParentName, strPermName;
	FieldsPtr flds;
	flds = rsUserPerms->Fields;
	//loop through each BuiltInObject to fill in that portion of the recordset
	// (a.walling 2009-12-14 15:18) - PLID 35178 - Use the passed-in connection
	_RecordsetPtr rsUserDefs = CreateRecordset(pCon, "SELECT ID, BuiltInID, DisplayName, AvailablePermissions "
		"FROM SecurityObjectT");

	CString strDisplayName;
	long nID, nBuiltInID, nAvailPerms;
	while (! rsUserDefs->eof)
	{
		strNodeName = "";
		strParentName = "";
		strGrandParentName = "";
		nGrandParentID = bioInvalidID;
		nNodeID = bioInvalidID;
		nParentID = bioInvalidID;

		nID = AdoFldLong(rsUserDefs, "ID");
		nBuiltInID = AdoFldLong(rsUserDefs, "BuiltInID");
		nAvailPerms = AdoFldLong(rsUserDefs, "AvailablePermissions");	
		strDisplayName = AdoFldString(rsUserDefs, "DisplayName");

		//DRT 5/11/2004 - We must be able to hide this id for only internal use
		bool bAdd = false;
		if(nBuiltInID == bioInternalOnly) {
			if(IsNexTechInternal())
				bAdd = true;
		}
		else {
			bAdd = true;
		}

		nNodeID = nID;
		strNodeName = strDisplayName;
		nParentID = nBuiltInID;
		if (nParentID != bioInvalidID) {
			nGrandParentID = GetGrandParentID((EBuiltInObjectIDs)nParentID);
			strParentName = GetBioName((EBuiltInObjectIDs)nParentID);

			if (nGrandParentID != bioInvalidID) {
				strGrandParentName = GetBioName((EBuiltInObjectIDs)nGrandParentID);
			}
		}
		else {
			nParentID = bioInvalidID;
			nGrandParentID = bioInvalidID;
			strParentName = "";
			strGrandParentName = "";
		}
			
		const CBuiltInObject* pObj = NULL;
		pObj = GetUserDefinedObject((EBuiltInObjectIDs)nNodeID);

		//make an array of items that we need to add
		CPtrArray pAryItems;
		char* szNames[] = { "Delete", "Delete with password", "Create", "Create with password", "Write", "Write with password", "Read", "Read with password", "Access", "Access with password" };
		BOOL bHasNonViewPerms = FALSE;
		ESecurityPermissionType obj;
		int i = 0;
		//now that we have the Bio Ids, we need to get the permission name and then all the values for each one
		for (obj = sptDeleteWithPass; obj >= sptViewWithPass; i+=2, obj = (ESecurityPermissionType)((DWORD)obj>>2))
		{
			if ((obj != sptViewWithPass) || (!bHasNonViewPerms)) {
				if (pObj->m_AvailPermissions.nPermissions & (obj>>1))
				{
					PermType *perm = new PermType;
					perm->strPermName = szNames[i];

					//if they are an administrator we can skip the lookup
					if (bIsAdmin) {
						perm->nValue = TRUE;
					}
					else {
						//lookup this objectID, userID from the map
						long nPerm;
						BOOL bResult;
						bResult = pMap->Lookup(AsString((long)pObj->m_eBuiltInID) + "-" + AsString((long)nUserID), nPerm);
						long nAvailPerms = (long)obj>>1;
						if (bResult) {
							if (nAvailPerms & nPerm) {
								perm->nValue = TRUE;
							}
							else {
								perm->nValue = FALSE;
							}
						}
						else {
							perm->nValue = FALSE;
						}
					}
					pAryItems.Add(((PermType*)perm));
					bHasNonViewPerms = TRUE;
				}
				if (pObj->m_AvailPermissions.nPermissions & obj)
				{
					PermType *perm = new PermType;
					perm->strPermName = szNames[i+1];
					//if they are an administrator we can skip the lookup
					if (bIsAdmin) {
						perm->nValue = TRUE;
					}
					else {
						long nPerm;
						BOOL bResult;
						bResult = pMap->Lookup(AsString((long)pObj->m_eBuiltInID) + "-" + AsString((long)nUserID), nPerm);
						long nAvailPerms = (long)obj;
						if (bResult) {
							if (nAvailPerms & nPerm) {
								perm->nValue = TRUE;
							}
							else {
								perm->nValue = FALSE;
							}
						}
						else {
							perm->nValue = FALSE;
						}
					}
					pAryItems.Add(((PermType*)perm));
					bHasNonViewPerms = TRUE;
				}
			}
		}

		for (i=0, obj = sptDynamic0; obj >= sptDynamic4; i++, obj = (ESecurityPermissionType)((DWORD)obj >> 2))
		{
			if (pObj->m_AvailPermissions.nPermissions & obj)
			{
				PermType *perm = new PermType;
				perm->strPermName = pObj->m_strDynamicPermissionNames[i];
				if (bIsAdmin) {
					perm->nValue = TRUE;
				}
				else {
					long nPerm;
					BOOL bResult;
					bResult = pMap->Lookup(AsString((long)pObj->m_eBuiltInID) + "-" + AsString((long)nUserID), nPerm);
					long nAvailPerms = (long)obj;
					if (bResult) {
						if (nAvailPerms & nPerm) {
							perm->nValue = TRUE;
						}
						else {
							perm->nValue = FALSE;
						}
					}
					else {
						perm->nValue = FALSE;
					}
				}
				pAryItems.Add(perm);
				PermType *perm2 = new PermType;
				perm2->strPermName = pObj->m_strDynamicPermissionNames[i] + " with password";
				if (bIsAdmin) {
					perm2->nValue = TRUE;
				}
				else {
					long nPerm;
					BOOL bResult = pMap->Lookup(AsString((long)pObj->m_eBuiltInID) + "-" + AsString((long)nUserID), nPerm);
					long nAvailPerms = (long)obj << 1;
					if (bResult) {
						if (nAvailPerms & nPerm) {
							perm2->nValue = TRUE;
						}
						else {
							perm2->nValue = FALSE;
						}
					}
					else {
						perm2->nValue = FALSE;
					}	
				}
				pAryItems.Add(perm2);
			}
		}
		
	

		for (int j = 0; j < pAryItems.GetSize(); j++) {
			//loop through the permissions loop and generate the recordset					
			if (nNodeID != bioInvalidID) {
				rsUserPerms->AddNew();
				flds->Item["NodeID"]->Value = nNodeID;
				flds->Item["NodeName"]->Value = _variant_t(strNodeName);
				flds->Item["ParentID"]->Value = nParentID;
				flds->Item["ParentName"]->Value = _variant_t(strParentName);
				flds->Item["GrandParentID"]->Value = nGrandParentID;
				flds->Item["GrandParentName"]->Value = _variant_t(strGrandParentName);
				flds->Item["UserID"]->Value = nUserID;
				flds->Item["UserName"]->Value = _variant_t(strUserName);
				flds->Item["PermissionName"]->Value = _variant_t(((PermType*)pAryItems.GetAt(j))->strPermName);
				flds->Item["PermissionValue"]->Value = ((PermType*)pAryItems.GetAt(j))->nValue;
				rsUserPerms->Update();
			}
		}

		//remove everything from the array and delete it
		for (int k = 0; k < pAryItems.GetSize(); k++) {
			PermType *pPerm = ((PermType*)pAryItems.GetAt(k));
			delete pPerm;
		}
		pAryItems.RemoveAll();
		
		rsUserDefs->MoveNext();
		
	}



}

void AddBuiltInObjects(_RecordsetPtr &rsUserPerms, CMap<CString, LPCTSTR, long, long> *pMap, BOOL bIsAdmin, CString strUserName, long nUserID) {

	long nNodeID, nParentID, nGrandParentID;
	CString strNodeName, strParentName, strGrandParentName, strPermName;
	FieldsPtr flds;
	flds = rsUserPerms->Fields;
	//loop through each BuiltInObject to fill in that portion of the recordset
	POSITION pos = GetFirstBuiltInObjectPosition();
	while (pos)
	{

		strNodeName = "";
		strParentName = "";
		strGrandParentName = "";
		nGrandParentID = bioInvalidID;
		nNodeID = bioInvalidID;
		nParentID = bioInvalidID;
		const CBuiltInObject* pObj = GetNextBuiltInObject(pos);

		//DRT 5/11/2004 - We must be able to hide this id for only internal use
		bool bAdd = false;
		if(pObj->m_eBuiltInID == bioInternalOnly || pObj->m_eParentID == bioInternalOnly) {
			if(IsNexTechInternal())
				bAdd = true;
		}
		else {
			bAdd = true;
		}

		if(bAdd) {
			nNodeID = pObj->m_eBuiltInID;
			strNodeName = pObj->m_strDisplayName;
			nParentID = pObj->m_eParentID;
			if (nParentID != bioInvalidID) {
				nGrandParentID = GetGrandParentID(pObj->m_eParentID);
				strParentName = GetBioName(pObj->m_eParentID);
				if (nGrandParentID != bioInvalidID) {
					strGrandParentName = GetBioName((EBuiltInObjectIDs)nGrandParentID);
				}
			}
			else {
				nParentID = bioInvalidID;
				nGrandParentID = bioInvalidID;
				strParentName = "";
				strGrandParentName = "";
			}
			

		}
		else {
			nNodeID = bioInvalidID;
		}

		//make an array of items that we need to add
	
		CPtrArray pAryItems;
		char* szNames[] = { "Delete", "Delete with password", "Create", "Create with password", "Write", "Write with password", "Read", "Read with password", "Access", "Access with password" };
		BOOL bHasNonViewPerms = FALSE;
		ESecurityPermissionType obj;
		int i = 0;
		//now that we have the Bio Ids, we need to get the permission name and then all the values for each one
		for (obj = sptDeleteWithPass; obj >= sptViewWithPass; i+=2, obj = (ESecurityPermissionType)((DWORD)obj>>2))
		{
			if ((obj != sptViewWithPass) || (!bHasNonViewPerms)) {
				if (pObj->m_AvailPermissions.nPermissions & (obj>>1))
				{
					PermType *perm = new PermType;
					perm->strPermName = szNames[i];

					//if they are an administrator we can skip the lookup
					if (bIsAdmin) {
						perm->nValue = TRUE;
					}
					else {
						//lookup this objectID, userID from the map
						long nPerm;
						BOOL bResult;
						bResult = pMap->Lookup(AsString((long)pObj->m_eBuiltInID) + "-" + AsString((long)nUserID), nPerm);
						long nAvailPerms = (long)obj>>1;
						if (bResult) {
							if (nAvailPerms & nPerm) {
								perm->nValue = TRUE;
							}
							else {
								perm->nValue = FALSE;
							}
						}
						else {
							perm->nValue = FALSE;
						}
					}
					pAryItems.Add(((PermType*)perm));
					bHasNonViewPerms = TRUE;
				}
				if (pObj->m_AvailPermissions.nPermissions & obj)
				{
					PermType *perm = new PermType;
					perm->strPermName = szNames[i+1];
					//if they are an administrator we can skip the lookup
					if (bIsAdmin) {
						perm->nValue = TRUE;
					}
					else {
						long nPerm;
						BOOL bResult;
						bResult = pMap->Lookup(AsString((long)pObj->m_eBuiltInID) + "-" + AsString((long)nUserID), nPerm);
						long nAvailPerms = (long)obj;
						if (bResult) {
							if (nAvailPerms & nPerm) {
								perm->nValue = TRUE;
							}
							else {
								perm->nValue = FALSE;
							}
						}
						else {
							perm->nValue = FALSE;
						}
					}
					pAryItems.Add(((PermType*)perm));
					bHasNonViewPerms = TRUE;
				}
			}
		}

		for (i=0, obj = sptDynamic0; obj >= sptDynamic4; i++, obj = (ESecurityPermissionType)((DWORD)obj >> 2))
		{
			if (pObj->m_AvailPermissions.nPermissions & obj)
			{
				PermType *perm = new PermType;
				perm->strPermName = pObj->m_strDynamicPermissionNames[i];
				if (bIsAdmin) {
					perm->nValue = TRUE;
				}
				else {
					long nPerm;
					BOOL bResult;
					bResult = pMap->Lookup(AsString((long)pObj->m_eBuiltInID) + "-" + AsString((long)nUserID), nPerm);
					long nAvailPerms = (long)obj;
					if (bResult) {
						if (nAvailPerms & nPerm) {
							perm->nValue = TRUE;
						}
						else {
							perm->nValue = FALSE;
						}
					}
					else {
						perm->nValue = FALSE;
					}
				}
				pAryItems.Add(perm);
				PermType *perm2 = new PermType;
				perm2->strPermName = pObj->m_strDynamicPermissionNames[i] + " with password";
				if (bIsAdmin) {
					perm2->nValue = TRUE;
				}
				else {
					long nPerm;
					BOOL bResult = pMap->Lookup(AsString((long)pObj->m_eBuiltInID) + "-" + AsString((long)nUserID), nPerm);
					long nAvailPerms = (long)obj << 1;
					if (bResult) {
						if (nAvailPerms & nPerm) {
							perm2->nValue = TRUE;
						}
						else {
							perm2->nValue = FALSE;
						}
					}
					else {
						perm2->nValue = FALSE;
					}	
				}
				pAryItems.Add(perm2);
			}
		}
		
	

		for (int j = 0; j < pAryItems.GetSize(); j++) {
			//loop through the permissions loop and generate the recordset					
			if (nNodeID != bioInvalidID) {
				rsUserPerms->AddNew();
				flds->Item["NodeID"]->Value = nNodeID;
				flds->Item["NodeName"]->Value = _variant_t(strNodeName);
				flds->Item["ParentID"]->Value = nParentID;
				flds->Item["ParentName"]->Value = _variant_t(strParentName);
				flds->Item["GrandParentID"]->Value = nGrandParentID;
				flds->Item["GrandParentName"]->Value = _variant_t(strGrandParentName);
				flds->Item["UserID"]->Value = nUserID;
				flds->Item["UserName"]->Value = _variant_t(strUserName);
				flds->Item["PermissionName"]->Value = _variant_t(((PermType*)pAryItems.GetAt(j))->strPermName);
				flds->Item["PermissionValue"]->Value = ((PermType*)pAryItems.GetAt(j))->nValue;
				rsUserPerms->Update();
			}
		}

		//remove everything from the list
		//remove everything from the array and delete it
		for (int k = 0; k < pAryItems.GetSize(); k++) {
			PermType *pPerm = ((PermType*)pAryItems.GetAt(k));
			delete pPerm;
		}
		pAryItems.RemoveAll();
				
		
	}
	
}*/
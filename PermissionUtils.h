//PermissionUtils.h

#pragma once

// (d.thompson 2013-03-26) - PLID 55847 - Refactored out the GeneratePermissionUpdateString() functions into CGeneratePermissionUpdateBatch

// (j.gruber 2010-04-13 16:46) - PLID 23982
DWORD GetPermisson(const CBuiltInObject* pObj, const ESecurityPermissionType esptPermType, const ESecurityPermissionType esptPermTypeWithPass, DWORD dwPermissionsMap1, DWORD dwPermissionsMap2);

//(e.lally 2010-10-14) PLID 40912 - Added bool bIncludeSavedUserPerms
DWORD GetPermisson(const CBuiltInObject* pObj, const ESecurityPermissionType esptPermType, const ESecurityPermissionType esptPermTypeWithPass, DWORD dwPermissionsMap1, DWORD dwPermissionsMap2, DWORD dwPermissionMap3, bool bIncludeSavedUserPerms, BOOL &bChange);

void LoadPermissionsIntoMap(CString &strTableName, long nUserGroupID, CMap<EBuiltInObjectIDs,EBuiltInObjectIDs,DWORD,DWORD> *pMapTemplates, long nUserGroupIDToExclude, BOOL bAddUserPerms);

CString CreateAllPermissionsTable();
void SaveChangeString(BOOL &bChange, CString strFirstSection, CString strType, CString &strChangeString);


void LoadBlankPermissionMap(CMap<EBuiltInObjectIDs,EBuiltInObjectIDs,DWORD,DWORD> *pMapToBlank, CMap<EBuiltInObjectIDs,EBuiltInObjectIDs,DWORD,DWORD> *pMapFilledIn = NULL);

// (j.gruber 2010-04-13 16:38) - PLID 37947 - used in checking summary screen changes
BOOL CheckPermissionChanged(const CBuiltInObject *pbio, ESecurityPermissionType esptPermType, ESecurityPermissionType esptPermTypeWithPass, DWORD dwUserPerms, DWORD dwTempPerms);

//this just moved from UserGroupSecurityDlg
void AdjustViewPerms(IN const EBuiltInObjectIDs ebio, IN OUT DWORD &dwPerms);


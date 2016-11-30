#include "stdafx.h"
#include "GeneratePermissionUpdateBatch.h"
#include "PermissionUtils.h"
#include "AuditTrail.h"

// (d.thompson 2013-03-26) - PLID 55847 - Call to return a CSqlFragment of all the batch data generated.
CSqlFragment CGeneratePermissionUpdateBatch::ToSqlFragment()
{
	//All declarations go in here so they only happen once per batch
	CSqlFragment sqlReturn(
		"DECLARE @UserGroupID int;\r\n"
		"DECLARE @x XML;\r\n"
		"CREATE TABLE #tmpPermissionInsertT (UserGroupID int NOT NULL, ObjectID int NOT NULL, Permissions int NOT NULL)\r\n"
	);

	//Append the working values.  This includes all the actual permissions we want to update
	sqlReturn += m_sqlWorking;

	// (d.thompson 2013-03-25) - PLID 55847 - And now that we've actually got all the data in our temp table, write some statements
	//	to push it to the live database tables.  Keep in mind this class is used in batch, so we could have multiple user
	//	groups worth of data filled into #tmpPermissionInsertT
	//1)  Update all the permissions that already exist to their new values in the #tmp table.   This should include a 0 for anything we're removing
	//2)  Insert new records for any permissions we're adding that didn't exist before.
	//3)  Delete all the 0's.  Our structure seems to want no 0-valued records in data (I'm not entirely sure why offhand).
	//Then remove the temp table, this should be operable in a batch of itself
	sqlReturn += CSqlFragment(
		"UPDATE PermissionT SET Permissions = T.Permissions "
		"FROM PermissionT P "
		"INNER JOIN #tmpPermissionInsertT T ON P.UserGroupID = T.UserGroupID AND P.ObjectID = T.ObjectID;\r\n"

		"INSERT INTO PermissionT (UserGroupID, ObjectID, Permissions) "
		"SELECT T.UserGroupID, T.ObjectID, T.Permissions "
		"FROM #tmpPermissionInsertT T "
		"LEFT JOIN PermissionT P ON T.UserGroupID = P.UserGroupID AND T.ObjectID = P.ObjectID "
		"WHERE P.Permissions IS NULL;\r\n"

		"DELETE FROM PermissionT WHERE UserGroupID IN (SELECT UserGroupID FROM #tmpPermissionInsertT) AND Permissions = 0 "
	);

	//And that's it!
	return sqlReturn;
}

// (d.thompson 2033-03-25) - PLID 55847 - Start of an XML data set
void CGeneratePermissionUpdateBatch::GeneratePermissionUpdateString_StartXml(CString &strCurrentXml)
{
	strCurrentXml = "<records>";
}

// (d.thompson 2013-03-25) - PLID 55847 - End of an XML data set
void CGeneratePermissionUpdateBatch::GeneratePermissionUpdateString_EndXml(CString &strCurrentXml)
{
	strCurrentXml += "</records>";
}

// (d.thompson 2013-03-25) - PLID 55847 - Given the current XML and new BIO / Perms, generate XML data and add to the current xml set
void CGeneratePermissionUpdateBatch::GeneratePermissionUpdateString_AddToXml(CString &strCurrentXml, long nBuiltInID, DWORD dwPerms)
{
	// (d.thompson 2013-03-25) - PLID 55847 - Reworked this to just load XML data for the content we want to update
	//	then do set-based operations on them.
	CString strNew = FormatString("<item bio=\"%li\" perms=\"%li\" />", nBuiltInID, dwPerms);
	GrowFastConcat(strCurrentXml, strNew.GetLength(), strNew);
}

// (d.thompson 2013-03-25) - PLID 55847 - Given a fully valid XML, create a sql fragment that will insert into the #temp 
//	table.
CSqlFragment CGeneratePermissionUpdateBatch::GeneratePermissionUpdateStringFromXML(long nUserGroupID, CString &strXml)
{
	CSqlFragment sql;

	// (d.thompson 2013-03-25) - PLID 55847 - Now create a #temp table with the xml
	sql += CSqlFragment(
		"SET @UserGroupID = {INT};\r\n"
		"SET @x = {STRING};\r\n"

		"INSERT INTO #tmpPermissionInsertT (UserGroupID, ObjectID, Permissions)\r\n"
		"select @UserGroupID, T.c.value('@bio', 'int') AS BuiltInObject, "
		"T.c.value('@perms', 'int') AS Perms "
		"FROM @x.nodes('/records/item') T(c);\r\n"
		, nUserGroupID, strXml);

	return sql;
}

// (j.gruber 2010-04-13 16:46) - PLID 23982 this iteration just loops through and creates our string the is nothing to check here
//this is used for saving just the template since we don't have to check anything for that.
// (d.thompson 2013-03-25) - PLID 55847 - Optimized to CSqlFragment parameterized queries, redesigned into CGeneratePermissionUpdateBatch.  
//	Previously known as GeneratePermissionUpdateString
void CGeneratePermissionUpdateBatch::AddPermissionUpdate(long nUserGroupID, CMap<EBuiltInObjectIDs,EBuiltInObjectIDs,DWORD,DWORD> *pMapCurrentTemplateChanges) 
{
	// (d.thompson 2013-03-25) - PLID 55847 - Refactored to generate XML based data then pass it to set based
	//	operations for the updates.  Worlds faster.  See detailed comments in the other GeneratePermissionUpdateString(bool...) override.
	CString strXml;
	GeneratePermissionUpdateString_StartXml(strXml);

	//do non-userdefined permissions first
	POSITION pos = GetFirstBuiltInObjectPosition();
	while (pos) {
		const CBuiltInObject* pObj = GetNextBuiltInObject(pos);

		DWORD dwPerms;
		
		if (pMapCurrentTemplateChanges->Lookup(pObj->m_eBuiltInID, dwPerms) ) {

			//the old saving code did this, so we will too
			AdjustViewPerms(pObj->m_eBuiltInID, dwPerms);

			// (d.thompson 2013-03-25) - PLID 55847 - Reworked this to just load XML data for the content we want to update
			//	then do set-based operations on them.
			GeneratePermissionUpdateString_AddToXml(strXml, pObj->m_eBuiltInID, dwPerms);
		}
	}

	//now do the userdefined permissions
	ADODB::_RecordsetPtr rsUserDefined = CreateParamRecordset("SELECT ID FROM SecurityObjectT");
	while (!rsUserDefined->eof) {

		EBuiltInObjectIDs nID = (EBuiltInObjectIDs)AdoFldLong(rsUserDefined, "ID");

		const CBuiltInObject* pObj = GetUserDefinedObject(nID);

		DWORD dwPerms;
		if (pMapCurrentTemplateChanges->Lookup(pObj->m_eBuiltInID, dwPerms)) {

			//the old saving code did this, so we will too
			AdjustViewPerms(pObj->m_eBuiltInID, dwPerms);

			// (d.thompson 2013-03-25) - PLID 55847 - Reworked this to just load XML data for the content we want to update
			//	then do set-based operations on them.
			GeneratePermissionUpdateString_AddToXml(strXml, pObj->m_eBuiltInID, dwPerms);
		}
	
		rsUserDefined->MoveNext();				
	}

	GeneratePermissionUpdateString_EndXml(strXml);
	m_sqlWorking += GeneratePermissionUpdateStringFromXML(nUserGroupID, strXml);
}

// (j.gruber 2010-04-13 16:46) - PLID 23982
//(e.lally 2010-10-14) PLID 40912 - Added bIncludeSavedUserPerms so we can pass it into the GetPermission function calls.
// (d.thompson 2013-03-25) - PLID 55847 - Optimized to CSqlFragment parameterized queries, redesigned into CGeneratePermissionUpdateBatch.  
//	Previously known as GeneratePermissionUpdateString
void CGeneratePermissionUpdateBatch::AddPermissionUpdate(bool bIncludeSavedUserPerms, long nUserGroupID, CMap<EBuiltInObjectIDs,EBuiltInObjectIDs,DWORD,DWORD> *pMapCurrentTemplateChanges, CMap<EBuiltInObjectIDs,EBuiltInObjectIDs,DWORD,DWORD> *pMapUser, CMap<EBuiltInObjectIDs,EBuiltInObjectIDs,DWORD,DWORD> *pMapOtherTemplates, CString strUserName, long &nAuditTransID, CString &strChangeString) 
{
	//here's what we need do here, loop through every permission
	// loop through each permission type ie: read, write, write with permission, etc
	//if the given permission is valid, then compare map1 to map2
	//check the permission first and then the permission with password
	// if map1 values | map2 value & permission then they have permission
	//else check withpass permission


	// (d.thompson 2013-03-25) - PLID 55847 - Optimized execution.  The previous implementation wrote a 10 line procedure
	//	that took in what we wanted the new permission to be, then aggregated all of those into a single batch and fired
	//	them off.  Unfortunately the batch could easily surpass 400 iterations, and each iteration had numerous parameters.
	//Instead, I decided to write "here's the permissions we want to have" to a #tmp table, then write an UPDATE, an INSERT,
	//	and a DELETE statement for each.  We'll use XML to most efficiently manage the very large data set.
	CString strXml;
	GeneratePermissionUpdateString_StartXml(strXml);
	
	//do non-userdefined permissions first
	POSITION pos = GetFirstBuiltInObjectPosition();
	while (pos) {
		const CBuiltInObject* pObj = GetNextBuiltInObject(pos);

		DWORD nPermissionsMap1;
		
		if (pMapCurrentTemplateChanges->Lookup(pObj->m_eBuiltInID, nPermissionsMap1) ) {

			DWORD nPermissionsMap2;
			if (pMapUser->Lookup(pObj->m_eBuiltInID, nPermissionsMap2)) {

				DWORD nPermissionsMap3;
				if (pMapOtherTemplates->Lookup(pObj->m_eBuiltInID, nPermissionsMap3)) {

					BOOL bChange = FALSE;					

					DWORD dwDelete = GetPermisson(pObj, sptDelete, sptDeleteWithPass, nPermissionsMap1, nPermissionsMap2, nPermissionsMap3, bIncludeSavedUserPerms, bChange);
					SaveChangeString(bChange, pObj->m_strDisplayName, "Delete", strChangeString);

					DWORD dwCreate = GetPermisson(pObj, sptCreate, sptCreateWithPass, nPermissionsMap1, nPermissionsMap2, nPermissionsMap3, bIncludeSavedUserPerms, bChange);
					SaveChangeString(bChange, pObj->m_strDisplayName, "Create", strChangeString);

					DWORD dwRead = GetPermisson(pObj, sptRead, sptReadWithPass, nPermissionsMap1, nPermissionsMap2, nPermissionsMap3, bIncludeSavedUserPerms, bChange);
					SaveChangeString(bChange, pObj->m_strDisplayName, "Read", strChangeString);

					DWORD dwWrite = GetPermisson(pObj, sptWrite, sptWriteWithPass, nPermissionsMap1, nPermissionsMap2, nPermissionsMap3, bIncludeSavedUserPerms, bChange);
					SaveChangeString(bChange, pObj->m_strDisplayName, "Write", strChangeString);

					DWORD dwAccess = 0;
					if ((pObj->m_AvailPermissions.nPermissions & (~(sptView|sptViewWithPass))) == 0) {
						dwAccess = GetPermisson(pObj, sptView, sptViewWithPass, nPermissionsMap1, nPermissionsMap2, nPermissionsMap3, bIncludeSavedUserPerms, bChange);
						SaveChangeString(bChange, pObj->m_strDisplayName, "View", strChangeString);
					}

					DWORD dwDynamic0 = GetPermisson(pObj, sptDynamic0, sptDynamic0WithPass, nPermissionsMap1, nPermissionsMap2, nPermissionsMap3, bIncludeSavedUserPerms, bChange);
					SaveChangeString(bChange, pObj->m_strDisplayName, pObj->m_strDynamicPermissionNames[0], strChangeString);

					DWORD dwDynamic1 = GetPermisson(pObj, sptDynamic1, sptDynamic1WithPass, nPermissionsMap1, nPermissionsMap2, nPermissionsMap3, bIncludeSavedUserPerms, bChange);
					SaveChangeString(bChange, pObj->m_strDisplayName, pObj->m_strDynamicPermissionNames[1], strChangeString);

					DWORD dwDynamic2 = GetPermisson(pObj, sptDynamic2, sptDynamic2WithPass, nPermissionsMap1, nPermissionsMap2, nPermissionsMap3, bIncludeSavedUserPerms, bChange);
					SaveChangeString(bChange, pObj->m_strDisplayName, pObj->m_strDynamicPermissionNames[2], strChangeString);

					DWORD dwDynamic3 = GetPermisson(pObj, sptDynamic3, sptDynamic3WithPass, nPermissionsMap1, nPermissionsMap2, nPermissionsMap3, bIncludeSavedUserPerms, bChange);
					SaveChangeString(bChange, pObj->m_strDisplayName, pObj->m_strDynamicPermissionNames[3], strChangeString);

					DWORD dwDynamic4 = GetPermisson(pObj, sptDynamic4, sptDynamic4WithPass, nPermissionsMap1, nPermissionsMap2, nPermissionsMap3, bIncludeSavedUserPerms, bChange);
					SaveChangeString(bChange, pObj->m_strDisplayName, pObj->m_strDynamicPermissionNames[4], strChangeString);

					DWORD dwPerms = dwDelete|dwCreate|dwRead|dwWrite|dwAccess|dwDynamic0|dwDynamic1|dwDynamic2|dwDynamic3|dwDynamic4;

					//the old saving code did this, so we will too
					AdjustViewPerms(pObj->m_eBuiltInID, dwPerms);

					// (d.thompson 2013-03-25) - PLID 55847 - Reworked this to just load XML data for the content we want to update
					//	then do set-based operations on them.
					GeneratePermissionUpdateString_AddToXml(strXml, pObj->m_eBuiltInID, dwPerms);

					if (dwPerms != nPermissionsMap2) {
						if (nAuditTransID == -1) {
							nAuditTransID = BeginAuditTransaction();
						}

						AuditEvent(-1, strUserName, nAuditTransID, aeiUserPermissionChanged, nUserGroupID, "", pObj->m_strDisplayName, aepMedium, aetChanged);
					}
				}					
			}
		}
	}

	//now do the userdefined permissions
	ADODB::_RecordsetPtr rsUserDefined = CreateParamRecordset("SELECT ID FROM SecurityObjectT");
	while (!rsUserDefined->eof) {

		EBuiltInObjectIDs nID = (EBuiltInObjectIDs)AdoFldLong(rsUserDefined, "ID");

		const CBuiltInObject* pObj = GetUserDefinedObject(nID);

		DWORD nPermissionsMap1;
		if (pMapCurrentTemplateChanges->Lookup(pObj->m_eBuiltInID, nPermissionsMap1)) {

			DWORD nPermissionsMap2;
			if (pMapUser->Lookup(pObj->m_eBuiltInID, nPermissionsMap2)) {

				DWORD nPermissionsMap3;
				if (pMapOtherTemplates->Lookup(pObj->m_eBuiltInID, nPermissionsMap3)) {

					BOOL bChange = FALSE;

					DWORD dwDelete = GetPermisson(pObj, sptDelete, sptDeleteWithPass, nPermissionsMap1, nPermissionsMap2, nPermissionsMap3, bIncludeSavedUserPerms, bChange);
					SaveChangeString(bChange, pObj->m_strDisplayName, "Delete", strChangeString);

					DWORD dwCreate = GetPermisson(pObj, sptCreate, sptCreateWithPass, nPermissionsMap1, nPermissionsMap2, nPermissionsMap3, bIncludeSavedUserPerms, bChange);
					SaveChangeString(bChange, pObj->m_strDisplayName, "Create", strChangeString);

					DWORD dwRead = GetPermisson(pObj, sptRead, sptReadWithPass, nPermissionsMap1, nPermissionsMap2, nPermissionsMap3, bIncludeSavedUserPerms, bChange);
					SaveChangeString(bChange, pObj->m_strDisplayName, "Read", strChangeString);

					DWORD dwWrite = GetPermisson(pObj, sptWrite, sptWriteWithPass, nPermissionsMap1, nPermissionsMap2, nPermissionsMap3, bIncludeSavedUserPerms, bChange);
					SaveChangeString(bChange, pObj->m_strDisplayName, "Write", strChangeString);
				
					DWORD dwAccess = 0;
					if ((pObj->m_AvailPermissions.nPermissions & (~(sptView|sptViewWithPass))) == 0) {
						dwAccess = GetPermisson(pObj, sptView, sptViewWithPass, nPermissionsMap1, nPermissionsMap2, nPermissionsMap3, bIncludeSavedUserPerms, bChange);
						SaveChangeString(bChange, pObj->m_strDisplayName, "View", strChangeString);
					}

					DWORD dwDynamic0 = GetPermisson(pObj, sptDynamic0, sptDynamic0WithPass, nPermissionsMap1, nPermissionsMap2, nPermissionsMap3, bIncludeSavedUserPerms, bChange);
					SaveChangeString(bChange, pObj->m_strDisplayName, pObj->m_strDynamicPermissionNames[0], strChangeString);

					DWORD dwDynamic1 = GetPermisson(pObj, sptDynamic1, sptDynamic1WithPass, nPermissionsMap1, nPermissionsMap2, nPermissionsMap3, bIncludeSavedUserPerms, bChange);
					SaveChangeString(bChange, pObj->m_strDisplayName, pObj->m_strDynamicPermissionNames[1], strChangeString);

					DWORD dwDynamic2 = GetPermisson(pObj, sptDynamic2, sptDynamic2WithPass, nPermissionsMap1, nPermissionsMap2, nPermissionsMap3, bIncludeSavedUserPerms, bChange);
					SaveChangeString(bChange, pObj->m_strDisplayName, pObj->m_strDynamicPermissionNames[2], strChangeString);

					DWORD dwDynamic3 = GetPermisson(pObj, sptDynamic3, sptDynamic3WithPass, nPermissionsMap1, nPermissionsMap2, nPermissionsMap3, bIncludeSavedUserPerms, bChange);
					SaveChangeString(bChange, pObj->m_strDisplayName, pObj->m_strDynamicPermissionNames[3], strChangeString);

					DWORD dwDynamic4 = GetPermisson(pObj, sptDynamic4, sptDynamic4WithPass, nPermissionsMap1, nPermissionsMap2, nPermissionsMap3, bIncludeSavedUserPerms, bChange);
					SaveChangeString(bChange, pObj->m_strDisplayName, pObj->m_strDynamicPermissionNames[4], strChangeString);
					
					DWORD dwPerms = dwDelete|dwCreate|dwRead|dwWrite|dwAccess|dwDynamic0|dwDynamic1|dwDynamic2|dwDynamic3|dwDynamic4;

					//the old saving code did this, so we will too
					AdjustViewPerms(pObj->m_eBuiltInID, dwPerms);

					// (d.thompson 2013-03-25) - PLID 55847 - Reworked this to just load XML data for the content we want to update
					//	then do set-based operations on them.
					GeneratePermissionUpdateString_AddToXml(strXml, pObj->m_eBuiltInID, dwPerms);


					if (dwPerms != nPermissionsMap2) {
						if (nAuditTransID == -1) {
							nAuditTransID = BeginAuditTransaction();
						}

						AuditEvent(-1, strUserName, nAuditTransID, aeiUserPermissionChanged, nUserGroupID, "", pObj->m_strDisplayName, aepMedium, aetChanged);
					}
				}
			}
		}
	
		rsUserDefined->MoveNext();				
	}

	// (d.thompson 2013-03-25) - PLID 55847 - Now actually generate the sql we want to execute
	GeneratePermissionUpdateString_EndXml(strXml);
	m_sqlWorking += GeneratePermissionUpdateStringFromXML(nUserGroupID, strXml);
}

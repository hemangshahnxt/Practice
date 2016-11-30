#pragma once

// (d.thompson 2013-03-25) - PLID 55847 - Optimized methodology for generating the sql statements for permission updates.  Previously
//	2 global functions with different overrides in PermissionUtils.h

/*	Expected Use
	This is designed to be used in batch.  You'd generally do something like this for batch functionality, or just a single Add... 
	for non-batched use.

	CGeneratePermissionUpdateBatch batch;
	while(!rs->eof) {
		batch.AddPermissionUpdate(...);
	}
	CSqlFragment sql = batch.ToSqlFragment();
	if(!sql.IsEmpty()) {
		ExecuteParamSql(sql);
	}
*/

/*	Design
	This class works to update user permissions.  This can happen because you edited a users permissions, you changed
	the groups a user belongs to, or you actually modified one of the groups (which cascades its changes down to all
	users in that group.

	The general idea is...
	 - Create a temp table holding UserGroupID, BuiltInObjectID, NewPermissions
	 - Generate an XML string (we're generally looking at a lot of data) of those values.  This can be hundreds of records.
	 - Insert the XML into a #temp table.
	 - Keep repeating for each user that is being updated.
	 - Now generate a single UPDATE, a single INSERT, and a single DELETE to move all the data we placed in the #temp table
		into the real Permissions table.
	 - Send this whole thing back as a CSqlFragment
*/

class CGeneratePermissionUpdateBatch {
public:
	//(e.lally 2010-10-14) PLID 40912 - Added bool bIncludeSavedUserPerms
	// (d.thompson 2013-03-25) - PLID 55847 - Optimized to CSqlFragment parameterized queries
	void AddPermissionUpdate(bool bIncludeSavedUserPerms, long nUserGroupID, CMap<EBuiltInObjectIDs,EBuiltInObjectIDs,DWORD,DWORD> *pMapCurrentTemplateChanges, CMap<EBuiltInObjectIDs,EBuiltInObjectIDs,DWORD,DWORD> *pMapUser, CMap<EBuiltInObjectIDs,EBuiltInObjectIDs,DWORD,DWORD> *pMapOtherTemplates, CString strUserName, long &nAuditTransID, CString &strChangeString);

	// (d.thompson 2013-03-25) - PLID 55847 - Optimized to CSqlFragment parameterized queries
	void AddPermissionUpdate(long nUserGroupID, CMap<EBuiltInObjectIDs,EBuiltInObjectIDs,DWORD,DWORD> *pMapCurrentTemplateChanges);

	// (d.thompson 2013-03-26) - PLID 55847 - Get your batch back for execution!
	CSqlFragment ToSqlFragment();

protected:
	CSqlFragment GeneratePermissionUpdateStringFromXML(long nUserGroupID, CString &strXml);
	void GeneratePermissionUpdateString_AddToXml(CString &strCurrentXml, long nBuiltInID, DWORD dwPerms);
	void GeneratePermissionUpdateString_EndXml(CString &strCurrentXml);
	void GeneratePermissionUpdateString_StartXml(CString &strCurrentXml);

	//The sql fragment we've built to this point.  Keep adding to it.
	CSqlFragment m_sqlWorking;

};

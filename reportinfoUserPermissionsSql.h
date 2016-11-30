//ReportInfoUserPermissionsSql.h

#ifndef REPORT_INFO_USER_PERMISSIONS_SQL_H
#define REPORT_INFO_USER_PERMISSIONS_SQL_H

#pragma once



	// (j.gruber 2010-04-12 15:52) - PLID 37947 - added bool to only show extra perms
	ADODB::_RecordsetPtr GetUserPermissionsRecordset(long nSublevel, long nSubReportNum, CString strExtraText, BOOL bForReportVerify, long nShowOnlyExtraPerms);
	long GetGrandParentID (long nParentID);
	CString GetBioName(EBuiltInObjectIDs nID);

	struct PermType {
		CString strPermName;
		long nValue;
	};
	void AddBuiltInObjects(ADODB::_RecordsetPtr &rs, CMap<CString, LPCTSTR, long, long> *pMap, BOOL bIsAdmin, CString strUserName, long nUserID);
	
	// (a.walling 2009-12-14 15:18) - PLID 35178 - Pass in a connection
	void AddUserDefinedObjects(ADODB::_Connection* lpCon, ADODB::_RecordsetPtr &rs, CMap<CString, LPCTSTR, long, long> *pMap, BOOL bIsAdmin, CString strUserName, long nUserID);
	CString GetBioName(EBuiltInObjectIDs nID);
	long GetGrandParentID (EBuiltInObjectIDs nParentID);


	// (j.gruber 2010-04-12 13:03) - PLID 37949
	void LoadAdminUsers(ADODB::_Connection* lpCon, ADODB::_RecordsetPtr &rsUserPerms, CString strUserIDs);
	void LoadUserTemplates(ADODB::_Connection* lpCon, ADODB::_RecordsetPtr &rsUserPerms, CString strUserIDs);
	void LoadUserPermissions(ADODB::_Connection* lpCon, ADODB::_RecordsetPtr &rsUserPerms, CString strUserIDs);
	void CheckPermission(ADODB::_RecordsetPtr &rsUserPerms, const CBuiltInObject *pbio, CString strDescription, const ESecurityPermissionType esptPermType, const ESecurityPermissionType esptPermTypeWithPass, DWORD dwUserPerms, DWORD dwTempPerms, long nUserID, CString strUserName, long nGroupVal);
	void AddPermsToRecordset(ADODB::_RecordsetPtr &rsUserPerms, EBuiltInObjectIDs ebio, DWORD dwUserPerms, DWORD dwTempPerms, long nUserID, CString strUserName, long nGroupVal);
#endif

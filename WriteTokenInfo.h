#pragma once

// (a.walling 2008-06-10 15:17) - PLID 22049 - Information about write token status
// (j.armen 2013-05-14 11:35) - PLID 56680 - Track Internal / External status instead of a user token
struct CWriteTokenInfo {
	CWriteTokenInfo() {
		bIsOldRevision = FALSE;
		bIsVerified = FALSE;
		bIsDeleted = FALSE;
		nHeldByUserID = -1;
		bIsExternal = FALSE;
	}

	CWriteTokenInfo& operator=(const CWriteTokenInfo& source) {
		strHeldByUserName = source.strHeldByUserName;
		strDeviceInfo = source.strDeviceInfo;
		bIsOldRevision = source.bIsOldRevision;
		dtHeld = source.dtHeld;
		bIsVerified = source.bIsVerified;
		bIsDeleted = source.bIsDeleted;
		nHeldByUserID = source.nHeldByUserID;
		bIsExternal = source.bIsExternal;
		return *this;
	}

	BOOL bIsVerified;
	BOOL bIsOldRevision;
	BOOL bIsDeleted; // (a.walling 2008-08-13 14:14) - PLID 22049 - Also check for deleted!
	long nHeldByUserID;
	CString strHeldByUserName;
	CString strDeviceInfo;
	COleDateTime dtHeld;
	BOOL bIsExternal;
};
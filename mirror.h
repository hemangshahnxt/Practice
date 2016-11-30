#ifndef MIRROR_UTILS_H
#define MIRROR_UTILS_H

#pragma once



#define  NEXTECH_COLOR 0xEBCEC5	//0xD79D8C
#define MIRROR_COLOR 0xCDCDEC	//0x9C9CDA

#define MIRROR_INDEX_PRIMARY_THUMBNAIL		-2

namespace Mirror
{
	enum MirrorResult
	{
		Skip = 0,
		Stop,
		Update,
		Addnew
	};

	// (c.haag 2009-03-31 10:36) - PLID 33630 - Return values for ConnectToCanfieldSDK
	enum ECanfieldSDKInitResult
	{
		// We successfully connected to the Canfield SDK, and are ready to read and write
		eCSDK_Success = 0L,

		// The caller requested an asynchronous connection, and a message will be posted to
		// all notification windows when done
		eCSDK_Connecting_WillPostMessage,

		// We did not attempt a connection because the link is disabled
		eCSDK_Failure_Link_Disabled,

		// We did not attempt a connection because the SDK is disabled
		eCSDK_Failure_Link_SDK_Disabled,

		// We failed to connect to the Canfield SDK because it's not available
		eCSDK_Failure_NotAvailable,

		// A timeout occured attempting to connect to the Canfield SDK
		eCSDK_Failure_TimeoutExpired,

		// An unexpected error occured attempting to connect to the Canfield SDK
		eCSDK_Failure_Error,
	};

	extern BOOL g_bForceDisable;

	BOOL IsMirror60();
	// (c.haag 2009-03-31 10:34) - PLID 33630 - IsMirror61 is deprecated
	// (c.haag 2009-03-31 10:37) - PLID 33630 - This function will try to establish a link between
	// NexTech Practice and Canfield Mirror. The caller must review the return value after the 
	// function returns before executing Mirror functionality.
	ECanfieldSDKInitResult InitCanfieldSDK(BOOL bSynchronous, HWND hWndAsyncNotify = NULL);
	BOOL IsMirrorEnabled();
	BOOL EnsureMirrorData();
	void CloseMirrorLink();
	void ResetMirrorDataPath();
	bool BrowseMirrorPath();
	CString GetMirrorDataPath();
	LPDISPATCH GetMirrorData();
	CString GetMirrorImagePath();
	HRESULT FillList61(NXDATALISTLib::_DNxDataListPtr& dlList);
	HRESULT FillList61Incremental(NXDATALISTLib::_DNxDataListPtr& dlList, long nStep, CString& strRecnum);
	HRESULT CancelFillList61();
	void SetMirrorDataPath(const CString &path);
	void SetMirrorImagePath(const CString &path);
	BOOL GetHighRes();
	void SetHighRes(BOOL bHighRes);
	//BOOL GetPrimaryImageOnly();
	//void SetPrimaryImageOnly(BOOL bPrimaryImageOnly);
	BOOL GetLinkMirrorMRNToUserDefinedID();
	BOOL GetLinkMirrorSSNToUserDefinedID();
	void SetLinkMirrorMRNToUserDefinedID(BOOL bLinkMirrorMRNToUserDefinedID);
	void SetLinkMirrorSSNToUserDefinedID(BOOL bLinkMirrorSSNToUserDefinedID);	
	MirrorResult Export(int nID, CString &recnum, BOOL bAssumeOneMatchingNameLinks = FALSE);
	MirrorResult Import(const CString &recnum, long &userID /*out*/, BOOL bAssumeOneMatchingNameLinks = FALSE);
	// (c.haag 2008-02-06 12:42) - PLID 28622 - We require a return value so that the
	// Link Common Patients dialog can abort the linking process if an error occurs.
	// The CanfieldLink library returns S_FAILED if LinkPatient fails, or 0 (S_OK) on
	// success. We will do the same here.
	HRESULT Link(int nID, const CString &recnum);
	void Unlink(int nID);
	long GetPatientCount();
	bool GetMirrorName(const CString &mirrorID, CString &first, CString &last);
	CString GetPatientMirrorID(unsigned long dwPatientID);
	long GetFirstValidImageIndex(const CString &strMirrorId);
	long GetLastValidImageIndex(const CString &strMirrorId, const CStringArray* apAllPatientThumbnailIDs, const CStringArray* apAllPatientMailSentPathNames);
	void Troubleshoot();
	BOOL HasInvalidRECNUMs();
	BOOL RepairImageTable(int& nRepairedRecords);
	BOOL RepairM2000Table(int& nRepairedRecords);
	void Run(long nPatientID = -1); // (a.walling 2008-07-07 17:39) - PLID 29900 - Pass in a patientID
	void ShowApplication(BOOL bShow);
	BOOL Get61Version(long& nMajor, long& nMinor);
	BOOL HasMirrorLinkLicense();
	// (c.haag 2009-03-31 13:36) - PLID 33630 - Returns TRUE if Practice is trying (not necessarily successfully)
	// to use the Canfield SDK functionality. The link's enabled/disabled and some error states are irrelevant here.
	BOOL IsUsingCanfieldSDK();

	// (c.haag 2008-06-19 09:33) - PLID 28886 - Added nQualityOverride
	// (c.haag 2009-04-01 16:06) - PLID 33630 - Removed bForceSynchronousOpen and default parameters
	HBITMAP LoadMirrorImage(const CString &recnum, IN OUT long &nIndex, OUT long &nCount, long nQualityOverride);
	// (c.haag 2010-02-22 17:34) - PLID 37364 - Overload that takes in an array of patient Mirror thumbnail ID's
	// and MailSent.PathName's. Also, from now on, nIndex exists in the range that only includes Mirror thumbnails
	// that were not imported by the Mirror image import app.
	HBITMAP LoadMirrorImage(const CString &recnum, IN OUT long &nIndex, OUT long &nCount, long nQualityOverride, 
		const CStringArray* apAllPatientThumbnailIDs, const CStringArray* apAllPatientMailSentPathNames);
	// (c.haag 2010-02-22 17:33) - PLID 37364 - Overload for not providing a list of Mirror thumbnail ID's or MailSent.PathNames
	long GetImageCount(const CString &strMirrorId);
	// (c.haag 2009-04-01 16:06) - PLID 33630 - Removed bForceSynchronousOpen
	// (c.haag 2010-02-22 17:34) - PLID 37364 - Overload that takes in an array of patient Mirror thumbnail ID's
	// and MailSent.PathName's. Also, from now on, this function returns the count of Mirror images that were not
	// imported by the Mirror image import app.
	long GetImageCount(const CString &strMirrorId, const CStringArray* apAllPatientThumbnailIDs, const CStringArray* apAllPatientMailSentPathNames);

	// (c.haag 2009-03-31 14:58) - PLID 33630 - I moved the contained code
	// from GetImageCount to this function. This should never be called from outside the namespace.
	long GetImageCount_PreCanfieldSDK(const CString &strMirrorId);
	// (c.haag 2009-03-31 15:05) - PLID 33630 - Moved from LoadMirrorImage into its own function.
	// This should never be called from outside the namespace.
	HBITMAP LoadMirrorImage_PreCanfieldSDK(const CString &recnum, long &nIndex, long &nCount, long nQualityOverride);
	// (c.haag 2010-02-24 10:32) - PLID 37364 - Moved Mirror 6.0 (pre-SDK) logic into its own function
	long GetFirstValidImageIndex_PreCanfieldSDK(const CString &strMirrorId);
	// (c.haag 2010-02-24 10:32) - PLID 37364 - Moved Mirror 6.0 (pre-SDK) logic into its own function
	long GetLastValidImageIndex_PreCanfieldSDK(const CString &strMirrorId);
}

#endif
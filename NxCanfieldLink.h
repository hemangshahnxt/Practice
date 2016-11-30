// NxCanfieldLink.h : main header file for the NXCANFIELDLINK DLL
//

#if !defined(AFX_NXCANFIELDLINK_H__75950613_4C86_44C6_9C80_DF090292D96C__INCLUDED_)
#define AFX_NXCANFIELDLINK_H__75950613_4C86_44C6_9C80_DF090292D96C__INCLUDED_

// If we're compiling this from the NxCanfieldLink.DLL,
// we need to do some extra things.
#ifdef _AFXDLL

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#define NX_DLL_API	__declspec(dllexport)
#include "resource.h"		// main symbols
#include "NxPropManager.h"

/////////////////////////////////////////////////////////////////////////////
// CNxCanfieldLinkApp
// See NxCanfieldLink.cpp for the implementation of this class
//
class CNxCanfieldLinkApp : public CWinApp
{
public:
	CNxCanfieldLinkApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNxCanfieldLinkApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CNxCanfieldLinkApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
#else 
#define NX_DLL_API	__declspec(dllimport)
#endif


#define NXD_MIRRORSEARCH_MIDDLE_NAME	0x00000001
#define NXD_MIRRORSEARCH_SSN			0x00000002

// Possible return values:

namespace CanfieldLink
{
typedef enum {
	eError = 0,		// An error occured and the export failed
	eAddNew = 1,	// Add a new patient
	eUpdate = 2,	// Update an existing patient
	eSkip = 3,		// Skip this patient (do nothing)
	eStop = 4,		// Stop exporting (do nothing)
} ECanfieldResult;

typedef enum {
	eStatusLinkNotEstablished = 0,
	eStatusTryingToLink = 1,
	eStatusLinkEstablished = 2,
	eStatusIncompatibleSDK = 3,
	// (b.cardillo 2007-04-30 14:07) - PLID 25839 - This enum value was added by c.haag in mainstream code 
	// for  the purposes of 25520, but it's not used in that capacity yet.  Since we now also need it for 
	// this pl item, we're keeping the same enum value, name, and meaning.
	eStatusLinkTimedOut = 4,
} ELinkStatus;

// One-time link establishment
// (c.haag 2009-03-31 11:44) - PLID 33760 - We now return the global status, and 
// take in an asynchronous notification window, and a notification message ID.
NX_DLL_API ELinkStatus EnsureLink(const CString& strSubRegistryKey,
				   const CString& strNetworkLibraryString,
				   const CString& strSQLServerName,
				   const CString& strPassword,
				   long nConnectionTimeout,
				   long nCommandTimeout,
				   BOOL bAsyncInitialize,
				   CNxPropManager* pPropManager,
				   HWND hWndAsyncNotify,
				   UINT uMsgNotify);

NX_DLL_API HRESULT DisconnectLink();

// Version
NX_DLL_API void GetCanfieldSDKVersion(long& nMajor, long& nMinor);

// Importing and exporting patients with detection of duplicate
// entries and automatic new patient creation in both databases.
NX_DLL_API ECanfieldResult ExportPatient(long nNxID, CString& szRecNum /* OUT */, BOOL bAssumeOneMatchingNameLinks);
NX_DLL_API ECanfieldResult ImportPatient(long& nNxID /* OUT */, LPCTSTR szRecNum, BOOL bAssumeOneMatchingNameLinks);

// Direct linking
NX_DLL_API HRESULT LinkPatient(long nNxID, const CString& szRecNum);
NX_DLL_API HRESULT UnlinkPatient(long nNxID);

// Properties
NX_DLL_API BOOL GetLinkMirrorSSNToUserDefinedID();
NX_DLL_API BOOL GetLinkMirrorMRNToUserDefinedID();
NX_DLL_API void SetLinkMirrorSSNToUserDefinedID(BOOL bLink);
NX_DLL_API void SetLinkMirrorMRNToUserDefinedID(BOOL bLink);

// Other properties
NX_DLL_API HRESULT GetPatientName(const CString& strRemoteID, CString& strFirst, CString& strMiddle, CString& strLast);

// Images
NX_DLL_API HBITMAP LoadImage(const CString &strRemoteID, long &nIndex, long &nCount, int iQuality);
NX_DLL_API HBITMAP LoadPrimaryImage(const CString &strRemoteID, int iQuality);
NX_DLL_API unsigned long GetImageCount(const CString &strRemoteID);
// (c.haag 2010-02-22 12:16) - PLID 37364 - Populates an array of existing thumbnail ID's for a given Mirror patient ID.
// Returns TRUE on success, otherwise FALSE.
NX_DLL_API BOOL GetThumbnailIDs(const CString& strRemoteID, OUT CStringArray& astrThumbnailIDs);

// Patient count
NX_DLL_API unsigned long GetPatientCount();

// NexTech customized function to fill a datalist with Mirror information.
NX_DLL_API HRESULT FillList(NXDATALISTLib::_DNxDataListPtr& dlList);
NX_DLL_API HRESULT FillListIncremental(NXDATALISTLib::_DNxDataListPtr& dlList, long nStep, CString& strRecNum);

// NexTech customized function to stop filling a datalist with Mirror information.
NX_DLL_API HRESULT CancelFillList();

// NexTech login functionality
NX_DLL_API void SetCurrentUserName(const CString& strCurrentUserName);
NX_DLL_API void SetCurrentLocationName(const CString& strCurrentLocationName);

// Application access

// Opens Mirror to a certain patient. This is independent of the state
// set by ShowApplication
NX_DLL_API HRESULT OpenChart(const CString& strRemoteID);

// Sets the default visibility of the Mirror app and immediately takes effect
NX_DLL_API HRESULT ShowApplication(BOOL bShow);

// Returns the status of the link
NX_DLL_API ELinkStatus GetLinkStatus();

}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NXCANFIELDLINK_H__75950613_4C86_44C6_9C80_DF090292D96C__INCLUDED_)

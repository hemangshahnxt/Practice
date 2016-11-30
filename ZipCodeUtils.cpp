// ZipcodeUtils.cpp: implementation of the ZipcodeUtils namespace.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ZipcodeUtils.h"
#include "RegUtils.h"
#include "NxMessageDef.h"
#include "math.h"
#include "FileUtils.h"

// (a.walling 2009-10-13 10:01) - PLID 35930
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;	// (d.lange 2010-04-29 10:05) - PLID 38249 - needed _RecordsetPtr available
// (a.walling 2006-12-06 09:31) - PLID 7069 - ZipcodeUtils namespace for interacting with our new Zip Code data file.
//		see the header file for a description of the file format and structures.
namespace ZipcodeUtils
{
	// (a.walling 2006-12-06 09:26) - PLID 7069
	// return the first record that matches the zip, and whether Alias should be null or not
	void FindByZipcode(OUT ZipcodeInfo &zi, IN const CString &strZip, IN OPTIONAL const bool bAliasIsNull /* = true */)
	{
		CString strTargetZip = strZip;
		strTargetZip.MakeUpper();
		const char* szTargetZip = (LPCTSTR)strTargetZip;
		CString strTargetIx = strTargetZip.Left(4);
		const char* szTargetIx = (LPCTSTR)strTargetIx;

		CString str;
		DWORD dwTargetPos = 0;

		CFile* pFile = NULL;

		try {
			pFile = OpenZipcodeFile();
			// (d.singleton 2011-10-07 12:53) - PLID 18875 - if you have the wrong install path and you go to enter a zip code you get 2 network error messages. it would be much better to have this say check your install path and make sure you have a zipcode.mdb there
			if(!pFile)
			{
				AfxMessageBox("The city and state related to this zip code could not be loaded, please check the install path for accuracy and make sure the ZipCodeList.dat file exists,  or call NexTech Technical Support for help:  1-866-654-4396");
				zi = ZipcodeInfo::ziInvalid(); 
				return;
			}
			ZipcodeFileHeader zfhFileHeader;
			ReadZipcodeFileHeader(zfhFileHeader, pFile);

			dwTargetPos = LookupIndex(pFile, zfhFileHeader, szTargetIx);
		} NxCatchAllCall("Error scanning Zip index.", { CloseZipcodeFile(pFile); zi = ZipcodeInfo::ziInvalid(); return; } );
		
		if (dwTargetPos == 0) {
			CloseZipcodeFile(pFile);
			zi = ZipcodeInfo::ziInvalid();
			return;
		}

		bool bFoundZip = false;

		try {
			long nBytesRead;

			//TES 11/7/2007 - PLID 27979 - VS2008 - Files now report their length and position as ULONGLONGs
#if _MSC_VER > 1300
			ULONGLONG nLength = pFile->GetLength();
			ULONGLONG nPos = pFile->Seek(dwTargetPos, CFile::begin);
#else
			DWORD nLength = pFile->GetLength();
			DWORD nPos = pFile->Seek(dwTargetPos, CFile::begin);
#endif

			ASSERT(nPos == dwTargetPos);
			
			ZipcodeInfo buffer;			
			nBytesRead= pFile->Read(&buffer, sizeof(ZipcodeInfo));

			while ( (nBytesRead == sizeof(ZipcodeInfo)) && (nPos < nLength) && (strncmp(szTargetIx, buffer.Zip, 4) == 0) )
			{
				nPos += nBytesRead;

				if (strcmp(szTargetZip, buffer.Zip) == 0) 
				{
					if (bAliasIsNull) { // only want to return if alias is null (which is stored here as bAlias = FALSE)
						if (buffer.bAlias == FALSE) {
							bFoundZip = true;
							break;
						}
					} else {
						bFoundZip = true;
						break;
					}
				}

				nBytesRead = pFile->Read(&buffer, sizeof(ZipcodeInfo));
			} 

			CloseZipcodeFile(pFile);

			if (bFoundZip) {
				zi = buffer;
				return;
			} else {
				zi = ZipcodeInfo::ziInvalid();
				return;
			}
		} NxCatchAll("Error scanning Zip codes.");

		CloseZipcodeFile(pFile);

		zi = ZipcodeInfo::ziInvalid();
		return;
	}

	// (a.walling 2006-12-06 09:26) - PLID 7069
	// return the first zip found that matches the ID and Zip passed
	void FindByIDAndZipcode(OUT ZipcodeInfo &zi, IN const long nID, IN const CString &strZip)
	{
		long nTargetID = nID;
		CString strTargetZip = strZip;
		strTargetZip.MakeUpper();
		const char* szTargetZip = (LPCTSTR)strTargetZip;
		CString strTargetIx = strTargetZip.Left(4);
		const char* szTargetIx = (LPCTSTR)strTargetIx;

		CString str;
		DWORD dwTargetPos = 0;

		CFile* pFile = NULL;

		try {
			pFile = OpenZipcodeFile();						
			// (d.singleton 2011-10-07 12:53) - PLID 18875 - if you have the wrong install path and you go to enter a zip code you get 2 network error messages. it would be much better to have this say check your install path and make sure you have a zipcode.mdb there
			if(!pFile)
			{
				AfxMessageBox("The city and state related to this zip code could not be loaded, please check the install path for accuracy and make sure the ZipCodeList.dat file exists,  or call NexTech Technical Support for help:  1-866-654-4396");
				zi = ZipcodeInfo::ziInvalid(); 
				return;
			}
			ZipcodeFileHeader zfhFileHeader;
			ReadZipcodeFileHeader(zfhFileHeader, pFile);

			dwTargetPos = LookupIndex(pFile, zfhFileHeader, szTargetIx);
		} NxCatchAllCall("Error scanning Zip index.", { CloseZipcodeFile(pFile); zi = ZipcodeInfo::ziInvalid(); return; } );
		
		if (dwTargetPos == 0) {
			CloseZipcodeFile(pFile);
			zi = ZipcodeInfo::ziInvalid();
			return;
		}

		bool bFoundZip = false;

		try {
			long nBytesRead;

			//TES 11/7/2007 - PLID 27979 - VS2008 - Files now report their length and position as ULONGLONGs
#if _MSC_VER > 1300
			ULONGLONG nLength = pFile->GetLength();
			ULONGLONG nPos = pFile->Seek(dwTargetPos, CFile::begin);
#else
			DWORD nLength = pFile->GetLength();
			DWORD nPos = pFile->Seek(dwTargetPos, CFile::begin);
#endif

			ASSERT(nPos == dwTargetPos);
			
			ZipcodeInfo buffer;
			nBytesRead= pFile->Read(&buffer, sizeof(ZipcodeInfo));

			while ( (nBytesRead == sizeof(ZipcodeInfo)) && (nPos < nLength) && (strncmp(szTargetIx, buffer.Zip, 4) == 0))
			{
				nPos += nBytesRead;

				if ( (strcmp(szTargetZip, buffer.Zip) == 0) && 
					 (nTargetID == buffer.nID) )
				{
					bFoundZip = true;
					break;
				}

				nBytesRead= pFile->Read(&buffer, sizeof(ZipcodeInfo));
			}

			CloseZipcodeFile(pFile);

			if (bFoundZip) {
				zi = buffer;
				return;
			} else {
				zi = ZipcodeInfo::ziInvalid();
				return;
			}
		} NxCatchAll("Error scanning Zip codes.");

		CloseZipcodeFile(pFile);

		zi = ZipcodeInfo::ziInvalid();
		return;
	}

	// (a.walling 2006-12-06 09:26) - PLID 7069
	// return the count of records with different city names for this zip
	long GetZipcodeCount(IN const CString &strZip)
	{
		CString strTargetZip = strZip;
		strTargetZip.MakeUpper();
		const char* szTargetZip = (LPCTSTR)strTargetZip;
		CString strTargetIx = strTargetZip.Left(4);
		const char* szTargetIx = (LPCTSTR)strTargetIx;

		CString str;
		DWORD dwTargetPos = 0;

		CFile* pFile = NULL;

		try {
			pFile = OpenZipcodeFile();
			if(!pFile)
			{			
				CloseZipcodeFile(pFile);
				return 0;
			}
			ZipcodeFileHeader zfhFileHeader;
			ReadZipcodeFileHeader(zfhFileHeader, pFile);

			dwTargetPos = LookupIndex(pFile, zfhFileHeader, szTargetIx);
		} NxCatchAllCall("Error scanning Zip index.", { CloseZipcodeFile(pFile); return 0; } );
		
		if (dwTargetPos == 0) {
			CloseZipcodeFile(pFile);
			return 0;
		}

		bool bFoundZip = false;
		long nZipCount = 0;

		try {
			long nBytesRead;

			//TES 11/7/2007 - PLID 27979 - VS2008 - Files now report their length and position as ULONGLONGs
#if _MSC_VER > 1300
			ULONGLONG nLength = pFile->GetLength();
			ULONGLONG nPos = pFile->Seek(dwTargetPos, CFile::begin);
#else
			DWORD nLength = pFile->GetLength();
			DWORD nPos = pFile->Seek(dwTargetPos, CFile::begin);
#endif

			ASSERT(nPos == dwTargetPos);
			
			ZipcodeInfo buffer;
			nBytesRead= pFile->Read(&buffer, sizeof(ZipcodeInfo));

			while ( (nBytesRead == sizeof(ZipcodeInfo)) && (nPos < nLength) && (strncmp(szTargetIx, buffer.Zip, 4) == 0))
			{
				nPos += nBytesRead;

				if ( strcmp(szTargetZip, buffer.Zip) == 0 )
				{
					bFoundZip = true;
					nZipCount++;
				}

				nBytesRead= pFile->Read(&buffer, sizeof(ZipcodeInfo));
			} 

			CloseZipcodeFile(pFile);

			if (bFoundZip) {
				return nZipCount;
			} else {
				return 0;
			}
		} NxCatchAll("Error scanning Zip codes.");

		CloseZipcodeFile(pFile);

		return 0;
	}

	// (a.walling 2006-12-06 09:26) - PLID 7069
	// return a set of zip codes matching the given criteria, checking for the stop event if passed to cancel the thread (may also be used in non-threading situations, just pass INVALID_HANDLE_VALUE for stop event)
	// Make sure to call ClearZipSet when finished with the ZipSet
	ZipcodeSet* ListAll(BOOL &bFailure, IN OPTIONAL const HANDLE hStopEvent /* = INVALID_HANDLE_VALUE */, IN OPTIONAL const EZipcodeField ezfField /* = zfNone */, IN OPTIONAL const char* lpszCriteria /* = NULL */)
	{
		// disable the conversion from 'double' to 'long', possible loss of data warning.
		// Trying to keep this function as tight as possible.
#pragma warning ( push )
#pragma warning ( disable : 4244 )
		DWORD dwTargetPos = 0;

		CFile* pFile = NULL;
		ZipcodeSet* zsRecords = new ZipcodeSet;
		ZipcodeInfo* buffer = NULL;
		// (d.singleton 2011-11-18 10:03) - PLID 18875 - set bFailure to FALSE by default
		bFailure = FALSE;

		try {
			pFile = OpenZipcodeFile();
			// (d.singleton 2011-10-07 12:53) - PLID 18875 - if you have the wrong install path and you go to enter a zip code you get 2 network error messages. it would be much better to have this say check your install path and make sure you have a zipcode.mdb there
			if(!pFile)
			{
				bFailure = TRUE;
				return zsRecords;
			}
			ZipcodeFileHeader zfhFileHeader;
			ReadZipcodeFileHeader(zfhFileHeader, pFile);

			dwTargetPos = zfhFileHeader.dwHeaderSize;
		} NxCatchAllCall("Error scanning Zip index.", { CloseZipcodeFile(pFile); delete zsRecords; return NULL; } ); // ensure we clean up data
		
		if (dwTargetPos == 0) {
			CloseZipcodeFile(pFile);
			return zsRecords;
		}

		bool bFilter = false;
		if (ezfField != zfNone) { // they are searching for something
			bFilter = true;
			if (strlen(lpszCriteria) > 32) {
				// way too big
				CloseZipcodeFile(pFile);
				return zsRecords;
			}
		}

		try {
			long nRecordsRead = 0;
			long nBytesRead;
			bool bCheckForEvent = hStopEvent != INVALID_HANDLE_VALUE;

			long nPattern, nLength, nLeading = 3; // for area

			if (bFilter) {
				switch(ezfField) {
					case zfAreaCode: // area;
						nPattern = atoi(lpszCriteria);
						// (a.walling 2007-11-07 11:42) - PLID 27998 - VS2008 - Specify type for overload disambiguation
						nLength = (log10(double(nPattern)) + 1);
						break;
					case zfZip: // zip;
					case zfCity: // city
					default:
						// nothing to do here
						break;
				}

				// this is for matching area codes (numbers) to the search string. Numbers don't count leading zeroes.
				const char* pZero = lpszCriteria;
				while ((*pZero == '0') && (*pZero != '\0')) {
					nLeading--;
					pZero++;
				}
			}


			DWORD dwLength = pFile->GetLength();
			DWORD dwPos = pFile->Seek(dwTargetPos, CFile::begin);

			ASSERT(dwPos == dwTargetPos);

//			DWORD dwZipSize = dwLength - dwPos; // the size (in bytes) of all the zipinfos is the length minus the header
			zsRecords->SetSize(0, 2048); // set the grow by size
			
			bool bAdd = false;
			long nTemp;
			do {
				if (buffer == NULL) {
					buffer = new ZipcodeInfo;
				}

				nBytesRead= pFile->Read(buffer, sizeof(ZipcodeInfo));

				if (bFilter) {
					bAdd = false;

					switch(ezfField) {
						case zfZip: // zip;
							if(BeginsWith(buffer->Zip, lpszCriteria) == TRUE)
								bAdd = true;
							break;
						case zfAreaCode: // area;
							// (a.walling 2007-11-07 11:42) - PLID 27998 - VS2008 - Specify type to disambiguate overload
							nTemp = buffer->nArea / ((pow(double(10), nLeading - nLength)));

							if (nTemp == nPattern)
								bAdd = true;
							break;
						case zfCity: // city
							if (BeginsWith(buffer->City, lpszCriteria) == TRUE)
								bAdd = true;
							break;
					}

					if (bAdd) {
						zsRecords->Add(buffer);
						buffer = NULL;
					} else {
						//delete buffer;
						//rather than delete the buffer, we leave it as NOT NULL so we avoid redundant new/delete operations.
					}
				} else {
					zsRecords->Add(buffer);	
					buffer = NULL;
				}
		

				dwPos += nBytesRead;
				nRecordsRead++;

				if ( bCheckForEvent && (nRecordsRead % 20) == 0)
				{
					// let's check the event
					if (WaitForSingleObject(hStopEvent, 0) == WAIT_OBJECT_0) {
						// event was signalled!! get out of here..
						CloseZipcodeFile(pFile);
						ClearZipcodeSet(zsRecords);
						return NULL;
					}
				}
			} while ( (nBytesRead == sizeof(ZipcodeInfo)) && (dwPos < dwLength) );

			if (buffer != NULL) {
				delete buffer; // we keep buffer around to be reused, if the last record did not meet the search criteria
					// it would still be pointing to some unused memory, so delete it.
			}

			CloseZipcodeFile(pFile);

			// do one final check for a signalled event so we can clean up memory if necessary
			if ( bCheckForEvent )
			{
				if (WaitForSingleObject(hStopEvent, 0) == WAIT_OBJECT_0) {
					// event was signalled!! get out of here..
					CloseZipcodeFile(pFile);
					ClearZipcodeSet(zsRecords);
					return NULL;
				}
			}

			return zsRecords;
		} NxCatchAllCall("Error scanning Zip codes.", { delete buffer; ClearZipcodeSet(zsRecords); } );

		CloseZipcodeFile(pFile);

		return zsRecords;

#pragma warning ( pop )
	}

	// (a.walling 2006-12-12 14:25) - PLID 23838 - Return the total number of records in the data file.
	//  this is useful when listing 'all' in the ZipCodes admin dialog, since if the client is using the
	//  canadian database, they will be loading approx 850,000 records!
	long GetTotalRecords()
	{
		CFile* pFile = OpenZipcodeFile();
		// (d.singleton 2011-10-07 15:06) - PLID 18875 make sure its a valid file
		if(!pFile)
		{
			return -1;
		}

		ZipcodeFileHeader zfhFileHeader;
		ReadZipcodeFileHeader(zfhFileHeader, pFile);

		CloseZipcodeFile(pFile);

		return zfhFileHeader.dwRecordCount;

		return -1;
	}

	// (a.walling 2006-12-06 09:26) - PLID 7069
	// frees all memory from the ZipSet
	void ClearZipcodeSet(IN OUT ZipcodeSet* zs)
	{
		if (zs != NULL) {
			for (int i = 0; i < zs->GetSize(); i++) {
				delete zs->GetAt(i);
			}

			delete zs;
		}
	}

	////////////////// threading functions below

	// (a.walling 2006-12-06 09:26) - PLID 7069
	// helper function to parse ZipLoadingCriteria and pass params to ListAll
	UINT LoadZipcodeAsyncThread(IN LPVOID p)
	{
		ZipcodeLoadingCriteria *pLoadCriteria = NULL;
		try {
			 pLoadCriteria = (ZipcodeLoadingCriteria*)p;

			 ZipcodeSet* pZs = ListAll(pLoadCriteria->bFailure, pLoadCriteria->hStopLoadingEvent, pLoadCriteria->ezfField, pLoadCriteria->lpszCriteria); // add search terms here

			 if (pZs == NULL) {
				// even if nothing was found, we get an empty ZipSet / CArray. So this means it was cancelled.
				// so delete our pLoadCriteria memory ourselves.
				
				ClearZipcodeThreadData(pLoadCriteria);
				pLoadCriteria = NULL;
			 }

			 if (pLoadCriteria && pLoadCriteria->pWnd) {
				 if (pLoadCriteria->pWnd->GetSafeHwnd())
				 {
					pLoadCriteria->pWnd->PostMessage(NXM_ZIPCODES_LOADED, (WPARAM)pZs, (LPARAM)pLoadCriteria);
				 }
			 }
		// (a.walling 2007-07-20 11:21) - PLID 26762 - Use threaded exception handling
		} NxCatchAllCallThread("Error in LoadZipAsyncThread.", { ClearZipcodeThreadData(pLoadCriteria); });
		// if there was an exception, perhaps starting the thread, ensure we clean up the parameter data

		return 0;
	}

	// (a.walling 2006-12-11 10:04) - PLID 23754
	// return a ZipLoadingCriteria struct initialized with the params.
	ZipcodeLoadingCriteria* CreateZipcodeThreadData(IN CWnd* pMsgWnd, IN const HANDLE hStopLoadingEvent, IN OPTIONAL const EZipcodeField ezfField /* = zfNone */, IN OPTIONAL const CString &strCriteria /* = "" */)
	{
		ZipcodeLoadingCriteria* pzlcLoad = NULL;
		char *lpszCriteria;

		try {
			pzlcLoad = new ZipcodeLoadingCriteria;
			memset(pzlcLoad, 0, sizeof(ZipcodeLoadingCriteria));

			pzlcLoad->pWnd = pMsgWnd;
			pzlcLoad->hStopLoadingEvent = hStopLoadingEvent;

			// create the string data on the heap
			if ((ezfField != zfNone) && !strCriteria.IsEmpty()) {
				lpszCriteria = new char[strCriteria.GetLength() + 1];	// add one for the null term

				strcpy(lpszCriteria, (LPCTSTR)strCriteria);

				pzlcLoad->ezfField = ezfField;
				pzlcLoad->lpszCriteria = lpszCriteria;
			} else {
				// thanks to our memset, these should already be set to NULL, but there is no harm in being safe

				pzlcLoad->ezfField = zfNone;
				pzlcLoad->lpszCriteria = NULL;
			}

			return pzlcLoad;
		} NxCatchAllCall("Error creating zip thread data", { ClearZipcodeThreadData(pzlcLoad); } ); // ensure we clean up data

		return NULL;
	}

	// (a.walling 2006-12-11 10:12) - PLID 23754
	// clears the ZipLoadingCriteria structure
	void ClearZipcodeThreadData(IN OUT ZipcodeLoadingCriteria* pzlcLoad)
	{
		try {
			if (pzlcLoad != NULL) {
				delete [] pzlcLoad->lpszCriteria;	// deleting NULL is ok
				delete pzlcLoad;
			}
		} NxCatchAll("Error cleaning up zip thread data");
	}
	

	////////////////// 'begins-with' function for searching
	// (a.walling 2006-12-06 09:26) - PLID 7069
	// fast string comparision function to determine if str begins with prefix
	inline BOOL BeginsWith(IN const char *str, IN const char *prefix)
	{
		try {
			char nandStr, nandPrefix;
			while ( (*prefix != '\0') && (*str != '\0') ) { // prevent overrunning str
				nandStr = *str & 0xFFFFFFDF;			// fast uppercase
				nandPrefix = *prefix & 0xFFFFFFDF;
				if ( (nandPrefix >= 'A') && (nandPrefix <= 'Z') ) {
					if (nandPrefix != nandStr)			// of course, uppercase only makes sense with alpha chars.
						return FALSE;
				} else {
					if (*prefix != *str) {
						return FALSE;
					}
				}
				prefix++;
				str++;
			}

			if ( (*str == '\0') && (*prefix != '\0') )
				return FALSE;

			return TRUE;
		} NxCatchAll("Error in ZipUtils::BeginsWith()");

		return FALSE;
	}


	////////////////// internal functions below

	// (a.walling 2006-12-06 09:26) - PLID 7069
	// returns invalid ZipInfo (fill ZipInfo with zero, set ID to -1)
	/* static */
	ZipcodeInfo ZipcodeInfo::ziInvalid()
	{
		ZipcodeInfo zi;

		memset(&zi, 0, sizeof(ZipcodeInfo));

		zi.nID = -1;

		return zi;
	}

	// (a.walling 2006-12-06 09:26) - PLID 7069
	// true if the ZipInfo is valid.
	bool ZipcodeInfo::IsValid()
	{
		return (nID != -1);
	}

	// (a.walling 2006-12-06 09:26) - PLID 7069
	// open the zip code data file, allocate the CFile on the heap, and return it.
	CFile* OpenZipcodeFile()
	{
		CFile* file = NULL;
		try {
			CString strPath = NxRegUtils::ReadString(GetRegistryBase() + "InstallPath");
			// (d.lange 2010-04-22 10:41) - PLID 38249 - Grab the filename from ConfigRT
			CString strDefault = "***File Unspecified***";
			CString strFile = GetRemotePropertyText("InputZipCodeFile", strDefault, 0, "<None>", true);
			long nCanadaLocations = 0;
			if(strFile.Compare(strDefault) == 0) {	// This should only be true the very first time zipcodes are searched
				// We want to see if there is any active managed location that is located in Canada, if we have at least one we want
				// to set the preference to "ZipCodeList_Canada.dat"
				_RecordsetPtr prs = CreateParamRecordset("SELECT COUNT(*) AS CanadaLocations FROM LocationsT "
														  "WHERE Managed = 1 AND Active = 1 AND "
														  "State = 'AB' OR "
														  "State = 'BC' OR "
														  "State = 'MB' OR "
														  "State = 'NB' OR "
														  "State = 'NL' OR "
														  "State = 'NT' OR "
														  "State = 'NS' OR "
														  "State = 'NU' OR "
														  "State = 'ON' OR "
														  "State = 'PE' OR "
														  "State = 'QC' OR "
														  "State = 'SK' OR "
														  "State = 'YT'");
				if(!prs->eof) {
					nCanadaLocations = AdoFldLong(prs, "CanadaLocations");
				}

				if(nCanadaLocations > 0) {
					//Set to ZipCodeList_Canada.dat
					SetRemotePropertyText("InputZipCodeFile", "ZipCodeList_Canada.dat", 0, "<None>");
					strFile = GetRemotePropertyText("InputZipCodeFile", "", 0, "<None>", true);
				} else{
					//Set to ZipCodeList.dat
					SetRemotePropertyText("InputZipCodeFile", "ZipCodeList.dat", 0, "<None>");
					strFile = GetRemotePropertyText("InputZipCodeFile", "", 0, "<None>", true);
				}
			}

			strPath = strPath ^ strFile;
			//strPath = strPath ^ "ZipcodeList.dat";

			// (d.singleton 2011-10-07 12:54) - PLID 18875 - check to see if install path is valid and zip code file exists
			if(!FileUtils::DoesFileOrDirExist(strPath))
			{
				return NULL;
			}
			
			file = new CFile;
			
			file->Open(strPath, CFile::modeRead | CFile::shareDenyWrite);

			return file;
		} NxCatchAllCall("Failed to load zip code file", { delete file; return NULL; } ) // ensure we clean up data
	}

	// (a.walling 2006-12-06 09:26) - PLID 7069
	// reads the header info of the given ZipInfo file and returns it in a ZipFileHeader struct
	void ReadZipcodeFileHeader(OUT ZipcodeFileHeader &zfh, IN CFile* pFile)
	{
		long nBytesRead;
		//TES 11/7/2007 - PLID 27979 - VS2008 - Files now report their length and position as ULONGLONGs
#if _MSC_VER > 1300
			ULONGLONG nOriginalPos = pFile->GetPosition();
#else
			DWORD nOriginalPos = pFile->GetPosition();
#endif
		
		pFile->SeekToBegin();

		ZipcodeFileHeader zfhFileHeader;

		nBytesRead = pFile->Read(&zfhFileHeader, sizeof(ZipcodeFileHeader)); // read the header
		if (nBytesRead != sizeof(ZipcodeFileHeader)) {
			AfxThrowNxException("Zip code file is corrupt.");
		}
		if ( strcmp("NxZip", zfhFileHeader.FileID) != 0 ) {
			AfxThrowNxException("Zip code file is corrupt.");
		}
		if (zfhFileHeader.dwVersion != NXZIPCODE_VERSION) {
			AfxThrowNxException("Zip code file is not the correct version.");
		}
		if (zfhFileHeader.dwRecordCount == 0) {
			AfxThrowNxException("Zip code file is empty or corrupt.");
		}

		pFile->Seek(nOriginalPos, CFile::begin);

		zfh = zfhFileHeader;
		return;
	}

	// (a.walling 2006-12-06 09:26) - PLID 7069
	// looks up given index (4 digit zip) and returns the file position to seek to.
	DWORD LookupIndex(IN CFile *pFile, IN const ZipcodeFileHeader &zfhFileHeader, IN const char* szTargetIx)
	{
		ZipcodeIxHeader buffer;
		//TES 11/7/2007 - PLID 27979 - VS2008 - Files now report their length and position as ULONGLONGs
#if _MSC_VER > 1300
		ULONGLONG nOriginalPos = pFile->GetPosition();
		ULONGLONG nPos = pFile->Seek(sizeof(ZipcodeFileHeader), CFile::begin);
		ULONGLONG nTargetPos = 0;
#else
		DWORD nOriginalPos = pFile->GetPosition();
		DWORD nPos = pFile->Seek(sizeof(ZipcodeFileHeader), CFile::begin);
		DWORD nTargetPos = 0;
#endif
		
		

		bool bFound = false;
		do {
			long nBytesRead = pFile->Read(&buffer, sizeof(ZipcodeIxHeader));

			nPos += nBytesRead;

			if (strcmp(szTargetIx, buffer.IxZip) == 0) {
				bFound = true;
				break;
			}
		} while (nPos < zfhFileHeader.dwHeaderSize);

		if (bFound) {
			nTargetPos = buffer.dwZipPos;
		}

		pFile->Seek(nOriginalPos, CFile::begin);

		//TES 11/7/2007 - PLID 27979 - VS2008 - Files now report their length and position as ULONGLONGs.  However,
		// there's no point going to great lengths to fully support that, this file should never be more than 4 GB.
		// If it is, just throw an exception.
		if(nTargetPos > ULONG_MAX) {
			AfxThrowNxException("LookupIndex() found index outside of supported range (0-4 GB)");
		}
		return (DWORD)nTargetPos;
	}

	// (a.walling 2006-12-06 09:26) - PLID 7069
	// close the zip code file and deallocate the memory
	void CloseZipcodeFile(IN OUT CFile *pFile)
	{
		if (pFile != NULL) {
			pFile->Close();
			delete pFile;
		}
	}
}
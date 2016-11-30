// CityZipUtils.cpp: implementation of the CityZipUtils namespace.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CityZipUtils.h"
#include "RegUtils.h"
#include "NxMessageDef.h"
#include "math.h"

// (j.gruber 2009-10-06 13:10) - PLID 35607 - ZipcodeUtils namespace for interacting with our new Zip Code data file.
//		see the header file for a description of the file format and structures.
namespace CityZipUtils
{
	void FindByCity(OUT CityZipInfo &zi, IN const CString &strCity, IN OPTIONAL const bool bAliasIsNull /* = true */)
	{
		CString strTargetCity = strCity;
		strTargetCity.MakeUpper();
		const char* szTargetCity = (LPCTSTR)strTargetCity;
		CString strTargetIx = strTargetCity.Left(4);
		const char* szTargetIx = (LPCTSTR)strTargetIx;

		CString str;
		DWORD dwTargetPos = 0;

		CFile* pFile = NULL;

		try {
			pFile = OpenCityFile();
			CityFileHeader zfhFileHeader;
			ReadCityFileHeader(zfhFileHeader, pFile);

			dwTargetPos = LookupCityIndex(pFile, zfhFileHeader, szTargetIx);
		} NxCatchAllCall("Error scanning City index.", { CloseCityFile(pFile); zi = CityZipInfo::ciInvalid(); return; } );
		
		if (dwTargetPos == 0) {
			CloseCityFile(pFile);
			zi = CityZipInfo::ciInvalid();
			return;
		}

		bool bFoundCity = false;

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
			
			CityZipInfo buffer;			
			nBytesRead= pFile->Read(&buffer, sizeof(CityZipInfo));

			while ( (nBytesRead == sizeof(CityZipInfo)) && (nPos < nLength) && (strncmp(szTargetIx, buffer.City, 4) == 0) )
			{
				nPos += nBytesRead;

				if (strcmp(szTargetCity, buffer.City) == 0) 
				{
					if (bAliasIsNull) { // only want to return if alias is null (which is stored here as bAlias = FALSE)
						if (buffer.bAlias == FALSE) {
							bFoundCity = true;
							break;
						}
					} else {
						bFoundCity = true;
						break;
					}
				}

				nBytesRead = pFile->Read(&buffer, sizeof(CityZipInfo));
			} 

			CloseCityFile(pFile);

			if (bFoundCity) {
				zi = buffer;
				return;
			} else {
				zi = CityZipInfo::ciInvalid();
				return;
			}
		} NxCatchAll("Error scanning Cities.");

		CloseCityFile(pFile);

		zi = CityZipInfo::ciInvalid();
		return;
	}

	// (a.walling 2006-12-06 09:26) - PLID 7069
	// return the first zip found that matches the ID and Zip passed
	void FindByIDAndCity(OUT CityZipInfo &zi, IN const long nID, IN const CString &strCity)
	{
		long nTargetID = nID;
		CString strTargetCity = strCity;
		strTargetCity.MakeUpper();
		const char* szTargetCity = (LPCTSTR)strTargetCity;
		CString strTargetIx = strTargetCity.Left(4);
		const char* szTargetIx = (LPCTSTR)strTargetIx;

		CString str;
		DWORD dwTargetPos = 0;

		CFile* pFile = NULL;

		try {
			pFile = OpenCityFile();
			CityFileHeader zfhFileHeader;
			ReadCityFileHeader(zfhFileHeader, pFile);

			dwTargetPos = LookupCityIndex(pFile, zfhFileHeader, szTargetIx);
		} NxCatchAllCall("Error scanning City index.", { CloseCityFile(pFile); zi = CityZipInfo::ciInvalid(); return; } );
		
		if (dwTargetPos == 0) {
			CloseCityFile(pFile);
			zi = CityZipInfo::ciInvalid();
			return;
		}

		bool bFoundCity = false;

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
			
			CityZipInfo buffer;
			nBytesRead= pFile->Read(&buffer, sizeof(CityZipInfo));

			while ( (nBytesRead == sizeof(CityZipInfo)) && (nPos < nLength) && (strncmp(szTargetIx, buffer.City, 4) == 0))
			{
				nPos += nBytesRead;

				if ( (strcmp(szTargetCity, buffer.City) == 0) && 
					 (nTargetID == buffer.nID) )
				{
					bFoundCity = true;
					break;
				}

				nBytesRead= pFile->Read(&buffer, sizeof(CityZipInfo));
			}

			CloseCityFile(pFile);

			if (bFoundCity) {
				zi = buffer;
				return;
			} else {
				zi = CityZipInfo::ciInvalid();
				return;
			}
		} NxCatchAll("Error scanning Cities.");

		CloseCityFile(pFile);

		zi = CityZipInfo::ciInvalid();
		return;
	}

	// (a.walling 2006-12-06 09:26) - PLID 7069
	// return the count of records with different city names for this zip
	long GetCityCount(IN const CString &strCity)
	{
		CString strTargetCity = strCity;
		strTargetCity.MakeUpper();
		const char* szTargetCity = (LPCTSTR)strTargetCity;
		CString strTargetIx = strTargetCity.Left(4);
		const char* szTargetIx = (LPCTSTR)strTargetIx;

		CString str;
		DWORD dwTargetPos = 0;

		CFile* pFile = NULL;

		try {
			pFile = OpenCityFile();
			CityFileHeader zfhFileHeader;
			ReadCityFileHeader(zfhFileHeader, pFile);

			dwTargetPos = LookupCityIndex(pFile, zfhFileHeader, szTargetIx);
		} NxCatchAllCall("Error scanning City index.", { CloseCityFile(pFile); return 0; } );
		
		if (dwTargetPos == 0) {
			CloseCityFile(pFile);
			return 0;
		}

		bool bFoundCity = false;
		long nCityCount = 0;

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
			
			CityZipInfo buffer;
			nBytesRead= pFile->Read(&buffer, sizeof(CityZipInfo));

			while ( (nBytesRead == sizeof(CityZipInfo)) && (nPos < nLength) && (strncmp(szTargetIx, buffer.City, 4) == 0))
			{
				nPos += nBytesRead;

				if ( strcmp(szTargetCity, buffer.City) == 0 )
				{
					bFoundCity = true;
					nCityCount++;
				}

				nBytesRead= pFile->Read(&buffer, sizeof(CityZipInfo));
			} 

			CloseCityFile(pFile);

			if (bFoundCity) {
				return nCityCount;
			} else {
				return 0;
			}
		} NxCatchAll("Error scanning Cities.");

		CloseCityFile(pFile);

		return 0;
	}

	// return a set of zip codes matching the given criteria, checking for the stop event if passed to cancel the thread (may also be used in non-threading situations, just pass INVALID_HANDLE_VALUE for stop event)
	// Make sure to call ClearZipSet when finished with the ZipSet
	CityZipSet* ListAll(IN OPTIONAL const HANDLE hStopEvent /* = INVALID_HANDLE_VALUE */, IN OPTIONAL const ECityZipField ezfField /* = zfNone */, IN OPTIONAL const char* lpszCriteria /* = NULL */)
	{
		// disable the conversion from 'double' to 'long', possible loss of data warning.
		// Trying to keep this function as tight as possible.
#pragma warning ( push )
#pragma warning ( disable : 4244 )
		DWORD dwTargetPos = 0;

		CFile* pFile = NULL;
		CityZipSet* zsRecords = new CityZipSet;
		CityZipInfo* buffer = NULL;

		try {
			pFile = OpenCityFile();
			CityFileHeader zfhFileHeader;
			ReadCityFileHeader(zfhFileHeader, pFile);

			dwTargetPos = zfhFileHeader.dwHeaderSize;
		} NxCatchAllCall("Error scanning city index.", { CloseCityFile(pFile); delete zsRecords; return NULL; } ); // ensure we clean up data
		
		if (dwTargetPos == 0) {
			CloseCityFile(pFile);
			return zsRecords;
		}

		bool bFilter = false;
		if (ezfField != zfNone) { // they are searching for something
			bFilter = true;
			if (strlen(lpszCriteria) > 40) {
				// way too big
				CloseCityFile(pFile);
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
					buffer = new CityZipInfo;
				}

				nBytesRead= pFile->Read(buffer, sizeof(CityZipInfo));

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
						CloseCityFile(pFile);
						ClearCityZipSet(zsRecords);
						return NULL;
					}
				}
			} while ( (nBytesRead == sizeof(CityZipInfo)) && (dwPos < dwLength) );

			if (buffer != NULL) {
				delete buffer; // we keep buffer around to be reused, if the last record did not meet the search criteria
					// it would still be pointing to some unused memory, so delete it.
			}

			CloseCityFile(pFile);

			// do one final check for a signalled event so we can clean up memory if necessary
			if ( bCheckForEvent )
			{
				if (WaitForSingleObject(hStopEvent, 0) == WAIT_OBJECT_0) {
					// event was signalled!! get out of here..
					CloseCityFile(pFile);
					ClearCityZipSet(zsRecords);
					return NULL;
				}
			}

			return zsRecords;
		} NxCatchAllCall("Error scanning City codes.", { delete buffer; ClearCityZipSet(zsRecords); } );

		CloseCityFile(pFile);

		return zsRecords;

#pragma warning ( pop )
	}

	// Return the total number of records in the data file.
	//  this is useful when listing 'all' in the ZipCodes admin dialog, since if the client is using the
	//  canadian database, they will be loading approx 850,000 records!
	long GetTotalRecords()
	{
		CFile* pFile = OpenCityFile();

		CityFileHeader zfhFileHeader;
		ReadCityFileHeader(zfhFileHeader, pFile);

		CloseCityFile(pFile);

		return zfhFileHeader.dwRecordCount;

		return -1;
	}

	// frees all memory from the ZipSet
	void ClearCityZipSet(IN OUT CityZipSet* zs)
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
	UINT LoadCityZipAsyncThread(IN LPVOID p)
	{
		CityZipLoadingCriteria *pLoadCriteria = NULL;
		try {
			 pLoadCriteria = (CityZipLoadingCriteria*)p;

			 CityZipSet* pZs = ListAll(pLoadCriteria->hStopLoadingEvent, pLoadCriteria->ezfField, pLoadCriteria->lpszCriteria); // add search terms here

			 if (pZs == NULL) {
				// even if nothing was found, we get an empty ZipSet / CArray. So this means it was cancelled.
				// so delete our pLoadCriteria memory ourselves.
				ClearCityZipThreadData(pLoadCriteria);
				pLoadCriteria = NULL;
			 }

			 if (pLoadCriteria && pLoadCriteria->pWnd) {
				 if (pLoadCriteria->pWnd->GetSafeHwnd())
				 {
					pLoadCriteria->pWnd->PostMessage(NXM_ZIPCODES_LOADED, (WPARAM)pZs, (LPARAM)pLoadCriteria);
				 }
			 }
		// (a.walling 2007-07-20 11:21) - PLID 26762 - Use threaded exception handling
		} NxCatchAllCallThread("Error in LoadCityZipAsyncThread.", { ClearCityZipThreadData(pLoadCriteria); });
		// if there was an exception, perhaps starting the thread, ensure we clean up the parameter data

		return 0;
	}

	
	// return a CityZipLoadingCriteria struct initialized with the params.
	CityZipLoadingCriteria* CreateCityZipThreadData(IN CWnd* pMsgWnd, IN const HANDLE hStopLoadingEvent, IN OPTIONAL const ECityZipField ezfField /* = zfNone */, IN OPTIONAL const CString &strCriteria /* = "" */)
	{
		CityZipLoadingCriteria* pzlcLoad = NULL;
		char *lpszCriteria;

		try {
			pzlcLoad = new CityZipLoadingCriteria;
			memset(pzlcLoad, 0, sizeof(CityZipLoadingCriteria));

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
		} NxCatchAllCall("Error creating city thread data", { ClearCityZipThreadData(pzlcLoad); } ); // ensure we clean up data

		return NULL;
	}

	// clears the CityZipLoadingCriteria structure
	void ClearCityZipThreadData(IN OUT CityZipLoadingCriteria* pzlcLoad)
	{
		try {
			if (pzlcLoad != NULL) {
				delete [] pzlcLoad->lpszCriteria;	// deleting NULL is ok
				delete pzlcLoad;
			}
		} NxCatchAll("Error cleaning up city zip thread data");
	}
	

	////////////////// 'begins-with' function for searching
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

	// (j.gruber 2009-10-05 16:55) - PLID 35607 - functions modified for looking up by city

	CityZipInfo CityZipInfo::ciInvalid()
	{
		CityZipInfo zi;

		memset(&zi, 0, sizeof(CityZipInfo));

		zi.nID = -1;

		return zi;
	}

	// (a.walling 2006-12-06 09:26) - PLID 7069
	// true if the ZipInfo is valid.
	bool CityZipInfo::IsValid()
	{
		return (nID != -1);
	}

	CFile* OpenCityFile()
	{
		CFile* file = NULL;
		try {
			CString strPath = NxRegUtils::ReadString(GetRegistryBase() + "InstallPath");
			strPath = strPath ^ "CityCodeList.dat";

			file = new CFile;
			file->Open(strPath, CFile::modeRead | CFile::shareDenyWrite);

			return file;
		} NxCatchAllCall("Failed to load city file", { delete file; return NULL; } ) // ensure we clean up data
	}

	void CloseCityFile(IN OUT CFile *pFile)
	{
		if (pFile != NULL) {
			pFile->Close();
			delete pFile;
		}
	}

	// reads the header info of the given ZipInfo file and returns it in a ZipFileHeader struct
	void ReadCityFileHeader(OUT CityFileHeader &zfh, IN CFile* pFile)
	{
		long nBytesRead;
		//TES 11/7/2007 - PLID 27979 - VS2008 - Files now report their length and position as ULONGLONGs
#if _MSC_VER > 1300
			ULONGLONG nOriginalPos = pFile->GetPosition();
#else
			DWORD nOriginalPos = pFile->GetPosition();
#endif
		
		pFile->SeekToBegin();

		CityFileHeader zfhFileHeader;

		nBytesRead = pFile->Read(&zfhFileHeader, sizeof(CityFileHeader)); // read the header
		if (nBytesRead != sizeof(CityFileHeader)) {
			AfxThrowNxException("City file is corrupt.");
		}
		if ( strcmp("NxCity", zfhFileHeader.FileID) != 0 ) {
			AfxThrowNxException("City file is corrupt.");
		}
		if (zfhFileHeader.dwVersion != NXCITYZIP_VERSION) {
			AfxThrowNxException("City file is not the correct version.");
		}
		if (zfhFileHeader.dwRecordCount == 0) {
			AfxThrowNxException("City file is empty or corrupt.");
		}

		pFile->Seek(nOriginalPos, CFile::begin);

		zfh = zfhFileHeader;
		return;
	}

	
	DWORD LookupCityIndex(IN CFile *pFile, IN const CityFileHeader &zfhFileHeader, IN const char* szTargetIx)
	{
		CityIxHeader buffer;
		//TES 11/7/2007 - PLID 27979 - VS2008 - Files now report their length and position as ULONGLONGs
#if _MSC_VER > 1300
		ULONGLONG nOriginalPos = pFile->GetPosition();
		ULONGLONG nPos = pFile->Seek(sizeof(CityFileHeader), CFile::begin);
		ULONGLONG nTargetPos = 0;
#else
		DWORD nOriginalPos = pFile->GetPosition();
		DWORD nPos = pFile->Seek(sizeof(CityFileHeader), CFile::begin);
		DWORD nTargetPos = 0;
#endif
		
		

		bool bFound = false;
		do {
			long nBytesRead = pFile->Read(&buffer, sizeof(CityIxHeader));

			nPos += nBytesRead;

			if (strcmp(szTargetIx, buffer.IxCity) == 0) {
				bFound = true;
				break;
			}
		} while (nPos < zfhFileHeader.dwHeaderSize);

		if (bFound) {
			nTargetPos = buffer.dwCityPos;
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

}


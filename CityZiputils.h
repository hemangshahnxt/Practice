// CityZipUtils.h: interface for the CityZipUtils namespace.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CITYZIPUTILS_H__C6A2570B_AAC7_4750_B234_342844082FE0__INCLUDED_)
#define AFX_CITYZIPUTILS_H__C6A2570B_AAC7_4750_B234_342844082FE0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define NXCITYZIP_VERSION 0

/* CityCodeList.dat file structure:
CityZipFileHeader {
	"NxCity\0"		-- file identification header
	NXCITY_VERSION	-- file version number
	0x00C0FFEE		-- position in file where header ends and zip code records begin (size of header)
	0x000BEAD5		-- the number of records in the file
  }
CityZipIxHeader {		*Number of city indexes. 
	"MELB\0"			-- first 4 characters of the city.
	0xDEADBEEF		-- position in file where city index begins 
  }
CityZipInfo {			*Number of zip code records. The first record is pointed to by CityZipHeader.dwZipPos.
	37730			-- ID
	"45342\0"			-- Zip code
	'OH\0'			-- State abbrev
	937				-- Area code
	0x00000000		-- BOOL (The alias would be a * in the MDB file for this to be true)
	"Miamisburg\0"	-- City
  }
 */

// (a.walling 2006-12-06 09:18) - PLID 7069 - ZipUtils namespace for interacting with our new Zip Code data file.
namespace CityZipUtils
{
	// (j.gruber 2009-10-05 16:39) - PLID 35607 - new file for cities
	//needed to change state from 3 to 4 because austraila has 3 character states
	//also changed the city length to 40
	struct CityZipInfo {
		long nID;
		char Zip[12];  // including null
		char State[4]; // including null
		long nArea;
		BOOL bAlias;
		char City[40]; // including null

		static CityZipInfo ciInvalid(); // returns invalid CityInfo (fill CityInfo with zero, set ID to -1)
		bool IsValid();				// true if the CityInfo is valid.
	};

	struct CityIxHeader {
		char IxCity[5];			// index (4 digit zip)
		DWORD dwCityPos;		// position in the file where the index (4 digit zip) begins.
	};

	struct CityFileHeader {
		char FileID[7];		// = "NxCity\0";
		DWORD dwVersion;	// = NXZIP_VERSION
		DWORD dwHeaderSize;	// size of the header (ie, zip code records start at this position)
		DWORD dwRecordCount;// number of records in the file.
	};
	
	// Enum for search field
	enum ECityZipField {
		zfNone = -1L,		// thankfully, C++ enums take on the type of their values.
		zfZip =  0L,
		zfCity,
		zfAreaCode,
	};

	// thread info structure
	struct CityZipLoadingCriteria {
		ECityZipField	ezfField;			// lookup field, only checks first char. Zipcode, AreaCode, City
		char*			lpszCriteria;		// criteria to search for
		CWnd*			pWnd;				// window to post messages to when complete or cancelled
		HANDLE			hStopLoadingEvent;	// event to check when thread needs to be cancelled
	};

	// Multiple-Zip return type definition
	typedef CArray<CityZipInfo*, CityZipInfo*> CityZipSet;

	// useful Zip functions
	CityZipSet* ListAll(IN OPTIONAL const HANDLE hStopEvent = INVALID_HANDLE_VALUE, IN OPTIONAL const ECityZipField ezfField = zfNone, IN OPTIONAL const char* lpszCriteria = NULL); // return a set of zip codes matching the given criteria, checking for the stop event if passed to cancel the thread (may also be used in non-threading situations, just pass INVALID_HANDLE_VALUE for stop event)

	long GetTotalRecords(); // return the total number of records in the data file
	
	void ClearCityZipSet(IN OUT CityZipSet* zs); // frees all memory from the CityZipSet.

	CityZipLoadingCriteria* CreateZipcodeThreadData(IN const CWnd* pMsgWnd, IN const HANDLE hStopLoadingEvent, IN OPTIONAL const ECityZipField ezfField = zfNone, IN OPTIONAL const CString &strCriteria = ""); // return a ZipLoadingCriteria struct initialized with the params, and memory appropriately allocated on the heap for the field and criteria strings.
	void ClearCityZipThreadData(IN OUT CityZipLoadingCriteria* pzlcLoad); // clears the ZipLoadingCriteria structure

	// internal functions
	UINT LoadCityZipAsyncThread(IN LPVOID p); // helper function to parse ZipLoadingCriteria and pass params to ListAll
	inline BOOL BeginsWith(IN const char *str, IN const char *prefix); // fast string comparision function to determine if str begins with prefix

	// (j.gruber 2009-10-05 15:53) - PLID 35607 - city lookup functions
	long GetCityCount(IN const CString &strCity);
	void FindByIDAndCity(OUT CityZipInfo &zi, IN const long nID, IN const CString &strCity);
	void FindByCity(OUT CityZipInfo &zi, IN const CString &strZip, IN OPTIONAL const bool bAliasIsNull  = true );
	void CloseCityFile(IN OUT CFile *pFile);
	CFile* OpenCityFile();
	void ReadCityFileHeader(OUT CityFileHeader &zfh, IN CFile* pFile);
	DWORD LookupCityIndex(IN CFile *pFile, IN const CityFileHeader &zfhFileHeader, IN const char* szTargetIx);
}	


#endif // !defined(AFX_CITYZIPUTILS_H__C6A2570B_AAC7_4750_B234_342844082FE0__INCLUDED_)

// ZipcodeUtils.h: interface for the ZipcodeUtils namespace.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ZIPUTILS_H__C6A2570B_AAC7_4750_B234_342844082FE0__INCLUDED_)
#define AFX_ZIPUTILS_H__C6A2570B_AAC7_4750_B234_342844082FE0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define NXZIPCODE_VERSION 0

/* ZipcodeList.dat file structure:
ZipcodeFileHeader {
	"NxZip\0"		-- file identification header
	NXZIP_VERSION	-- file version number
	0x00C0FFEE		-- position in file where header ends and zip code records begin (size of header)
	0x000BEAD5		-- the number of records in the file
  }
ZipcodeIxHeader {		*Number of zip indexes. Currently 6840.
	"4534\0"			-- first 4 digits of the zip code.
	0xDEADBEEF		-- position in file where zip code index begins (for example, 45340)	
  }
ZipcodeInfo {			*Number of zip code records. The first record is pointed to by ZipFileHeader.dwZipPos.
	37730			-- ID
	"45342\0"			-- Zip code
	'OH\0'			-- State abbrev
	937				-- Area code
	0x00000000		-- BOOL (The alias would be a * in the MDB file for this to be true)
	"Miamisburg\0"	-- City
  }
 */

// (a.walling 2006-12-06 09:18) - PLID 7069 - ZipUtils namespace for interacting with our new Zip Code data file.
namespace ZipcodeUtils
{
// (a.walling 2010-09-24 09:29) - PLID 40784 - Initially changed packing, but that would lead to incompatibilities, so I just left it alone in the end.
//#pragma pack(push, no_zip_packing, 1)
	// Zip information structure
	struct ZipcodeInfo {
		ZipcodeInfo()
			: nID(0)
			, nArea(0)
			, bAlias(FALSE)
		{
			::ZeroMemory(Zip, sizeof(Zip));
			::ZeroMemory(State, sizeof(State));
			::ZeroMemory(City, sizeof(City));
		};

		long nID;
		char Zip[12];  // including null
		char State[3]; // including null
		long nArea;
		BOOL bAlias;
		char City[32]; // including null

		static ZipcodeInfo ziInvalid(); // returns invalid ZipInfo (fill ZipInfo with zero, set ID to -1)
		bool IsValid();				// true if the ZipInfo is valid.
	};

	// Zip file structures
	struct ZipcodeIxHeader {
		ZipcodeIxHeader()
			: dwZipPos(0)
		{
			::ZeroMemory(IxZip, sizeof(IxZip));
		};

		char IxZip[5];			// index (4 digit zip)
		DWORD dwZipPos;		// position in the file where the index (4 digit zip) begins.
	};

	struct ZipcodeFileHeader {
		ZipcodeFileHeader()
			: dwVersion(0)
			, dwHeaderSize(0)
			, dwRecordCount(0)
		{
			::ZeroMemory(FileID, sizeof(FileID));
		};

		char FileID[6];		// = "NxZip\0";
		DWORD dwVersion;	// = NXZIP_VERSION
		DWORD dwHeaderSize;	// size of the header (ie, zip code records start at this position)
		DWORD dwRecordCount;// number of records in the file.
	};
//#pragma pack(pop, no_zip_packing)
	
	// Enum for search field
	enum EZipcodeField {
		zfNone = -1L,		// thankfully, C++ enums take on the type of their values.
		zfZip =  0L,
		zfCity,
		zfAreaCode,
	};

	// thread info structure
	struct ZipcodeLoadingCriteria {
		EZipcodeField	ezfField;			// lookup field, only checks first char. Zipcode, AreaCode, City
		char*			lpszCriteria;		// criteria to search for
		CWnd*			pWnd;				// window to post messages to when complete or cancelled
		HANDLE			hStopLoadingEvent;	// event to check when thread needs to be cancelled
		BOOL			bFailure;			// (d.singleton 2011-11-22 10:34) - PLID 18875 check to see if ZipCodeList.dat exists or not
	};

	// Multiple-Zip return type definition
	typedef CArray<ZipcodeInfo*, ZipcodeInfo*> ZipcodeSet;

	// useful Zip functions
	void FindByIDAndZipcode(OUT ZipcodeInfo &zi, IN const long nID, IN const CString &strZip); // return the first zip found that matches the ID and Zip
	void FindByZipcode(OUT ZipcodeInfo &zi, IN const CString &strZip, IN OPTIONAL const bool bAliasIsNull = true); // return the first record that matches the zip, and whether Alias should be null or not
	long GetZipcodeCount(IN const CString &strZip); // return the count of records with different city names for this zip
	// (d.singleton 2011-11-18 10:02) - PLID 18875 - Added bFailure to check if ZipCodeList.dat exist in the install path,  if it gets set to true you can handle accordingly
	ZipcodeSet* ListAll( BOOL &bFailure, IN OPTIONAL const HANDLE hStopEvent = INVALID_HANDLE_VALUE, IN OPTIONAL const EZipcodeField ezfField = zfNone, IN OPTIONAL const char* lpszCriteria = NULL); // return a set of zip codes matching the given criteria, checking for the stop event if passed to cancel the thread (may also be used in non-threading situations, just pass INVALID_HANDLE_VALUE for stop event)

	long GetTotalRecords(); // return the total number of records in the data file
	
	void ClearZipcodeSet(IN OUT ZipcodeSet* zs); // frees all memory from the ZipSet.

	ZipcodeLoadingCriteria* CreateZipcodeThreadData(IN const CWnd* pMsgWnd, IN const HANDLE hStopLoadingEvent, IN OPTIONAL const EZipcodeField ezfField = zfNone, IN OPTIONAL const CString &strCriteria = ""); // return a ZipLoadingCriteria struct initialized with the params, and memory appropriately allocated on the heap for the field and criteria strings.
	void ClearZipcodeThreadData(IN OUT ZipcodeLoadingCriteria* pzlcLoad); // clears the ZipLoadingCriteria structure

	// internal functions
	UINT LoadZipcodeAsyncThread(IN LPVOID p); // helper function to parse ZipLoadingCriteria and pass params to ListAll
	inline BOOL BeginsWith(IN const char *str, IN const char *prefix); // fast string comparision function to determine if str begins with prefix

	// Zip internal file utility functions
	CFile* OpenZipcodeFile(); // open the zip code data file, allocate the CFile on the heap, and return it.
	void CloseZipcodeFile(IN OUT CFile *pFile); // close the zip code file and deallocate the memory
	void ReadZipcodeFileHeader(OUT ZipcodeFileHeader &zfh, IN CFile* pFile); // reads the header info of the given ZipInfo file and returns it in a ZipFileHeader struct
	DWORD LookupIndex(IN CFile *pFile, IN const ZipcodeFileHeader &zfhFileHeader, IN const char* szTargetIx); // looks up given index (4 digit zip) and returns the file position to seek to.
}	

#endif // !defined(AFX_ZIPUTILS_H__C6A2570B_AAC7_4750_B234_342844082FE0__INCLUDED_)

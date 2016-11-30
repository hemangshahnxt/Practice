#ifndef __NXRAPI_H__
#define __NXRAPI_H__

#pragma once



#ifndef RAPI_H
//
// The Windows CE WIN32_FIND_DATA structure differs from the
// Windows WIN32_FIND_DATA stucture so we copy the Windows CE
// definition to here so that both sides match.
//
typedef struct _CE_FIND_DATA {
    DWORD    dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD    nFileSizeHigh;
    DWORD    nFileSizeLow;
    DWORD    dwOID;
    WCHAR    cFileName[MAX_PATH];
} CE_FIND_DATA, *LPCE_FIND_DATA;

typedef CE_FIND_DATA** LPLPCE_FIND_DATA;

typedef struct _RAPIINIT
{
    DWORD cbSize;
    HANDLE heRapiInit;
    HRESULT hrRapiInit;
} RAPIINIT;

//
// These are flags for CeFindAllFiles
//
#define FAF_ATTRIBUTES      ((DWORD) 0x01)
#define FAF_CREATION_TIME   ((DWORD) 0x02)
#define FAF_LASTACCESS_TIME ((DWORD) 0x04)
#define FAF_LASTWRITE_TIME  ((DWORD) 0x08)
#define FAF_SIZE_HIGH       ((DWORD) 0x10)
#define FAF_SIZE_LOW        ((DWORD) 0x20)
#define FAF_OID             ((DWORD) 0x40)
#define FAF_NAME            ((DWORD) 0x80)
#define FAF_FLAG_COUNT      ((UINT)  8)
#define FAF_ATTRIB_CHILDREN ((DWORD)			0x01000)
#define FAF_ATTRIB_NO_HIDDEN ((DWORD)			0x02000)
#define FAF_FOLDERS_ONLY    ((DWORD)			0x04000)
#define FAF_NO_HIDDEN_SYS_ROMMODULES ((DWORD)	0x08000)
#define FAF_GETTARGET	    ((DWORD)			0x10000)

#define FAD_OID             ((WORD) 0x01)
#define FAD_FLAGS           ((WORD) 0x02)
#define FAD_NAME            ((WORD) 0x04)
#define FAD_TYPE            ((WORD) 0x08)
#define FAD_NUM_RECORDS     ((WORD) 0x10)
#define FAD_NUM_SORT_ORDER  ((WORD) 0x20)
#define FAD_SIZE            ((WORD) 0x40)
#define FAD_LAST_MODIFIED   ((WORD) 0x80)
#define FAD_SORT_SPECS      ((WORD) 0x100)
#define FAD_FLAG_COUNT      ((UINT) 9)

#ifndef FILE_ATTRIBUTE_INROM
#define FILE_ATTRIBUTE_INROM        0x00000040
#endif
#ifndef FILE_ATTRIBUTE_ROMSTATICREF
#define FILE_ATTRIBUTE_ROMSTATICREF 0x00001000
#endif
#ifndef FILE_ATTRIBUTE_ROMMODULE
#define FILE_ATTRIBUTE_ROMMODULE    0x00002000
#endif

#endif

// CNxRAPI class for
class CNxRAPI
{
protected:
	HMODULE m_hInst; // Instance of the RAPI module
public:
	CNxRAPI();
	virtual ~CNxRAPI();

	// True if this is a valid RAPI object
	BOOL IsValid();

	// Safe initialization of the RAPI
	HRESULT RapiInit();

	// Custom functions
	HRESULT CopyPCToCE(const CString& strFileNamePC, const CString& strFileNameCE);
	HRESULT CopyCEToPC(const CString& strFileNamePC, const CString& strFileNameCE);

	// Standard RAPI functions
	typedef HANDLE (FAR PASCAL * pfnFunc0)(LPCWSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
	typedef BOOL   (FAR PASCAL * pfnFunc1)(HANDLE, LPCVOID, DWORD, LPDWORD, LPOVERLAPPED);
	typedef BOOL   (FAR PASCAL * pfnFunc2)(HANDLE);	
	typedef HANDLE (FAR PASCAL * pfnFunc3)(LPCWSTR, LPCE_FIND_DATA);
	typedef DWORD  (FAR PASCAL * pfnFunc4)(HANDLE, LPDWORD);
	typedef BOOL   (FAR PASCAL * pfnFunc5)(HANDLE, LPVOID, DWORD, LPDWORD, LPOVERLAPPED);
	typedef BOOL   (FAR PASCAL * pfnFunc6)(HANDLE, LPCE_FIND_DATA);
	typedef BOOL   (FAR PASCAL * pfnFunc7)(LPCWSTR, LPSECURITY_ATTRIBUTES);
	typedef BOOL   (FAR PASCAL * pfnFunc8)(LPCWSTR, LPCWSTR, LPSECURITY_ATTRIBUTES, LPSECURITY_ATTRIBUTES, BOOL, DWORD, LPVOID, LPWSTR, LPSTARTUPINFO, LPPROCESS_INFORMATION); 
	typedef VOID   (FAR PASCAL * pfnFunc9)(LPSYSTEM_INFO);
	typedef BOOL   (FAR PASCAL * pfnFunc10)(LPCWSTR, DWORD, LPDWORD, LPLPCE_FIND_DATA);
	typedef VOID   (FAR PASCAL * pfnFunc11)(LPVOID);
	typedef HRESULT(FAR PASCAL * pfnFunc12)(RAPIINIT*);
	typedef BOOL   (FAR PASCAL * pfnFunc13)(LPCWSTR);
	typedef HRESULT(FAR PASCAL * pfnFunc14)(void);
	typedef DWORD  (FAR PASCAL * pfnFunc15)(void);

	pfnFunc0 CeCreateFile;
	pfnFunc1 CeWriteFile;
	pfnFunc2 CeCloseHandle;
	pfnFunc3 CeFindFirstFile;
	pfnFunc4 CeGetFileSize;
	pfnFunc5 CeReadFile;
	pfnFunc6 CeFindNextFile;
	pfnFunc7 CeCreateDirectory;
	pfnFunc8 CeCreateProcess; 
	pfnFunc9 CeGetSystemInfo;
	pfnFunc10 CeFindAllFiles;
	pfnFunc11 CeRapiFreeBuffer;
	pfnFunc12  CeRapiInitEx;
	pfnFunc13 CeDeleteFile;
	pfnFunc14 CeRapiGetError;
	pfnFunc15 CeGetLastError;
	FARPROC  CeRapiUninit;
};

#endif

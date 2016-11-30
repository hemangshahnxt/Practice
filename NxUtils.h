#ifndef NX_UTILS_H
#define NX_UTILS_H

#pragma once



// (a.walling 2008-10-02 09:34) - PLID 28040 - NxUtils -- new, clean shared utils library.
// Goals are to keep this lightweight and used only where the advantages of DLLs are useful.
// Specifically, shared resources. That is why GetPracPath and Logging utilites are here.
// Eventually it would be ideal to move the Property Manager here, or into another shared lib.

#if _MSC_VER > 1300

#ifndef BUILDNXUTILS
#ifdef _DEBUG
	#pragma comment(lib, "NxUtilsD.lib")
#else
	#pragma comment(lib, "NxUtils.lib")
#endif
#endif

// (j.armen 2011-10-26 14:40) - PLID 45795 - PracPath::PathType is the enumeration that 
// determines which path you get when setting/getting prac paths
namespace PracPath
{
	enum PathType
	{
		PracticeEXEPath,// The path where practice.exe is located (i.e. C:\Pracstation)
		PracticePath,	// The path of the shared working directory (i.e. C:\Pracstation)
		SessionPath,	// The path of the practice session's working directory (i.e. (C:\Pracstation\Sessions\Default\{GUID})
		ConfigRT,		// The path used for ConfigRT (i.e. C:\Pracstation\Practice.mde)
	};
};

// (j.armen 2011-10-26 14:42) - PLID 45795 - Functions for getting and setting prac paths
AFX_EXT_API CString GetPracPath(PracPath::PathType ePathType);
AFX_EXT_API void SetPracPath(PracPath::PathType ePathType, CString strPracPath);
AFX_EXT_API CString GetCurrentDirectory();

// Generic logging
AFX_EXT_API int Log(LPCTSTR strFormat, ...);
AFX_EXT_API int Log(CException *e, LPCTSTR strFormat, ...);
AFX_EXT_API int LogFlat(LPCTSTR strFormat, ...); // Removes all carriage returns and line feeds before logging
AFX_EXT_API int LogAppend(LPCTSTR strFormat, ...);
AFX_EXT_API int LogMany(LPCTSTR strFormat, ...);
AFX_EXT_API int LogIndent();
AFX_EXT_API int LogIndent(LPCTSTR strFormat, ...);
AFX_EXT_API int LogUnindent();
AFX_EXT_API int LogUnindent(LPCTSTR strFormat, ...);
AFX_EXT_API CFile *GetLogFile();
AFX_EXT_API CString GetLogFilePathName();
AFX_EXT_API void SetLogFilePathName(const CString &strPathName);

#endif

#endif
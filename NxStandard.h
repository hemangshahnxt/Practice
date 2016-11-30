#ifndef NX_STANDARD_UTILITES_H
#define NX_STANDARD_UTILITES_H
#pragma once

// (a.walling 2008-10-02 10:35) - PLID 28040 - NxUtils contains anything that actually needs
// the shared memory among modules that DLLs provide
#if _MSC_VER > 1300
#include "NxUtils.h"
#endif

// (a.walling 2008-09-19 09:53) - PLID 28040 - NxStandard has been consolidated into Practice

#define NXM_CANCEL				WM_USER + 3

BOOL ReadWindowPlacement(LPWINDOWPLACEMENT pwp);
void WriteWindowPlacement(LPWINDOWPLACEMENT pwp);

// These functions are used for manipulating text edit fields
int NewSelPos(int OrgStart, const CString &CurText, CString &FmtText);
int NextFmtPos(int FmtPos, LPCTSTR fmtText);
CString MakeFormatText(LPCTSTR fmtText) ;
void FormatItemText(CWnd *wndTextBox, const CString & strNewVal, LPCTSTR fmtText);
void FormatText(const CString & strInText, CString & strOutText, LPCTSTR fmtText);
void UnformatItemText(CWnd *wndTextBox, CString & strNewVal);

// For dealing with currency
COleVariant UnformatCurrency(COleVariant var);


// Convenient message capabilities (you may use MAKEINTRESOURCE 
// for the strFmt parameter if you want to use resource strings)

// (j.jones 2016-02-25 09:11) - PLID 22663 - converted all existing MsgBox versions
// to not take parameters, and added a templated override to safely handle ... cases
// that do not have any parameters
int MsgBoxStd(const char* strText);
template<typename... Args>
int MsgBox(const char* strFmt, Args&&... args)
{
	// Get the correct format string
	CString strFormat;
	if (HIWORD((LPCTSTR)strFmt) == 0) {
		// A resource ID was given
		VERIFY(strFormat.LoadString(LOWORD((LPCTSTR)strFmt)) != 0);
	}
	else {
		// An actual string was given
		strFormat = strFmt;
	}

	constexpr size_t numargs = sizeof...(Args);

	if (numargs > 0) {
		strFormat = FormatString(strFormat, args...);
	}
	
	return MsgBoxStd(strFormat);
}

int MsgBoxStd(UINT nType, const char* strText);
// (j.jones 2016-02-25 09:08) - PLID 22663 - override to safely handle cases where ... contains no arguments
template<typename... Args>
int MsgBox(UINT nType, const char* strFmt, Args&&... args)
{
	// Get the correct format string
	CString strFormat;
	if (HIWORD((LPCTSTR)strFmt) == 0) {
		// A resource ID was given
		VERIFY(strFormat.LoadString(LOWORD((LPCTSTR)strFmt)) != 0);
	}
	else {
		// An actual string was given
		strFormat = strFmt;
	}

	constexpr size_t numargs = sizeof...(Args);

	if (numargs > 0) {
		strFormat = FormatString(strFormat, args...);
	}
	
	return MsgBoxStd(nType, strFormat);
}

// (j.jones 2016-02-25 11:36) - PLID 22663 - removed the version for UINT nIDHelp, nothing used it
//int MsgBox(UINT nType, UINT nIDHelp, LPCTSTR strText);

// (j.jones 2016-02-25 11:36) - PLID 22663 - removed the version for CException, nothing used it
//int MsgBox(CException *e, LPCTSTR strText);

// (a.walling 2011-12-21 16:10) - PLID 46648 - Dialogs must set a parent!
int InputBox(CWnd* pParent, const CString &strPrompt, CString &strResult, const CString &strOther, bool bPassword = false, bool bBrowseBtn = false, LPCTSTR strCancelBtnText = NULL, BOOL bIsNumeric = FALSE);
// (a.walling 2011-12-21 16:10) - PLID 46648 - Dialogs must set a parent!
int InputBoxLimited(CWnd* pParent, const CString &strPrompt, CString &strResult, const CString &strOther, unsigned long nMaxChars, bool bPassword /* = false */, bool bBrowseBtn /* = false */, LPCTSTR strCancelBtnText /* = NULL */);
// (c.haag 2007-09-11 16:15) - PLID 27353 - This is exactly like InputBoxLimited but you can pass in a parent window
int InputBoxLimitedWithParent(CWnd* pParentWnd, const CString &strPrompt, CString &strResult, const CString &strOther, unsigned long nMaxChars, bool bPassword /* = false */, bool bBrowseBtn /* = false */, LPCTSTR strCancelBtnText /* = NULL */);
// (z.manning 2015-08-13 16:52) - PLID 67248 - Input box with an input mask
int InputBoxMasked(CWnd *pwndParent, const CString &strPrompt, const CString &strInputMask, OUT CString &strResult
	, const CString &strCueBanner = "");

// Convenient conversion functions
void SetUppersInString(CWnd *wndTextBox, CString & strInText);
CString CStringOfVariant(COleVariant var);

// Security functions
// (j.jones 2009-04-30 14:27) - PLID 33853 - these are obsolete now, we use AES decryption,
// but we use this for the obsolete "calc secret password" concept and a couple other places
CString EncryptString_Deprecated(const CString &strInputString, bool bDecrypt = false);
CString EncryptString_Deprecated(const CString &strInputString, LPCTSTR strDirection);

// For obtaining version and other file information
bool GetFileSystemTime(const CString &strFile, COleDateTime &dtAns);
bool SetFileSystemTime(const CString &strFile, const COleDateTime &dt);
long CalcVersion(UINT nYear, UINT nMonth, UINT nDay, UINT nExtra = 0);
long CalcVersion(const COleDateTime &dt);
UINT GetVerMonth(long nVer);
UINT GetVerDay(long nVer);
UINT GetVerYear(long nVer);
UINT GetVerExtra(long nVer);
CString GetFileVersionString(long nVer);

// String routines
CString Exclude(const CString &inString, const CString &exChar);
CString Include(const CString &inStr, const CString &inChar);

// Various utilities for various purposes
bool MoveFileAtStartup(LPCTSTR strSource, LPCTSTR strDest);
bool IsLocked(const CString &strFile, CFileException *e = NULL);
bool DoesExist(LPCTSTR strFile);
bool IsReadOnly(LPCTSTR strFile);
void SetReadOnly(LPCTSTR strFile, bool bReadOnly = true);
bool IsFileReadable(const CString &strFile, CFileException *e = NULL);
CString GetTempFileName(const CString &strFile, char chCharacter = 't', const LPCTSTR strPath = NULL);
CString GetFileExtension(const CString &strFile);
CString GetFilePath(const CString &strFile);
CString GetFileName(const CString &strFile);
bool CreatePath(const CString &strPath);
bool IsKeyDown(int nVirtualKey);
bool IsSuperKeyDown();

// To help determine whether to use Office 2000 or Office 97
bool IsJet40();
bool IsAccess2k();

// Quick way of allowing the user to browse to a path and/or a file
bool Browse(HWND hWndParent, const CString &strInitPath, CString &strReturnPath, bool bIncludeFiles = false);

// Registry handling specific to NexTech
HKEY GetNexTechKey();
bool GetNexTechProfileInt(const CString &strValueName, DWORD &nValue);
bool GetNexTechProfileString(const CString &strValueName, CString &strValue);
bool WriteNexTechProfileInt(const CString &strValueName, const DWORD nValue);
bool WriteNexTechProfileString(const CString &strValueName, const CString &strValue);

#endif
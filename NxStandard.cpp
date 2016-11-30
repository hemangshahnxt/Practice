// NxStandard.cpp : Defines the initialization routines for the DLL.
//

// (a.walling 2008-09-19 09:53) - PLID 28040 - NxStandard has been consolidated into Practice

#include "stdafx.h"
#include "NxStandard.h"
#include "NxInputDlg.h"
#include "NxProgressDlg.h"
#include "fileutils.h"
#include <afxtempl.h>
#include "PathStringUtils.h"	//DRT 10/13/2008 - PLID 31662 - Needed because this file uses the ^ operator frequently

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Helpers for saving/restoring window state
TCHAR BASED_CODE szSection[] = _T("Settings");
TCHAR BASED_CODE szWindowPos[] = _T("WindowPos");
TCHAR szFormat[] = _T("%u,%u,%d,%d,%d,%d,%d,%d,%d,%d");

//const CString szBoxFmt = _T("(%1bPatient%0b: [pName)   ](%1bPurpose%0b: [rPurpose)   ](%1bNotes%0b: [rNotes)]");

// Format specifications ///////////////////////////////////////////////////////////////
// '%nb' - sets to font #n
// '...[STRING...]' - displays data referred to by STRING
//                    only displays "..." if data is returned from STRING
//                    if STRING is an unused string constant STRING returns "STRING"
//
// ex.  =  "([pName)   ]([rPurpose)   ]([rNotes)]"
//  would display as	 "(John Doe)   (Bleph)   (He's the man)"
//		OR						"(Bleph)"
//		OR						"(John Doe)   (He's the man)"
//		OR						"(Bleph)   (He's the man)"
//  etc based on what information is available

//const CString szBoxFmt = _T("%0b([rMoveUpConfirmed)   ]%0b(%1b[pName%0b)   ]%0b(%3b[rPurpose%0b)   ]%0b([rNotes)]");
//const CString szBoxFmt = _T("%0b([rMoveUpConfirmed)   ]%1b[pName   ]%3b[rPurpType   ]%3b[rPurpose   ]%0b[rNotes]");
const CString szBoxFmt = _T("%1b[pName    ]%0b([rMoveUpConfirmed)   ]%0b([rShowState)   ]%3b[rPurpType   ]%3b[rPurpose   ]%0b[rNotes   ]%0bID:[ExtraInfo0  ]%0bH:[ExtraInfo1   ]%0bW:[ExtraInfo2   ]%0bBD:[ExtraInfo3   ]%0b[ExtraInfo4   ]%0b[ExtraInfo5]");
const char strBoxFmt[255] = "%1b[pName   ]%0b([rMoveUpConfirmed)   ]%0b([rShowState)   ]%3b[rPurpType   ]%3b[rPurpose   ]%0b[rNotes   ]%0bID:[ExtraInfo0  ]%0bH:[ExtraInfo1   ]%0bW:[ExtraInfo2   ]%0bBD:[ExtraInfo3   ]%0b[ExtraInfo4   ]%0b[ExtraInfo5]";
////////////////////////////////////////////////////////////////////////////////////////

CString GlobalPracPath = _T("");
CString GlobalPracFolderPath = _T("");
bool GlobalPracPathSet = false;

BOOL ReadWindowPlacement(LPWINDOWPLACEMENT pwp)
{
	CString strBuffer = AfxGetApp()->GetProfileString(szSection, szWindowPos);
	if (strBuffer.IsEmpty())
		return FALSE;

	WINDOWPLACEMENT wp;
	int nRead = _stscanf(strBuffer, szFormat,
		&wp.flags, &wp.showCmd,
		&wp.ptMinPosition.x, &wp.ptMinPosition.y,
		&wp.ptMaxPosition.x, &wp.ptMaxPosition.y,
		&wp.rcNormalPosition.left, &wp.rcNormalPosition.top,
		&wp.rcNormalPosition.right, &wp.rcNormalPosition.bottom);

	if (nRead != 10)
		return FALSE;

	wp.length = sizeof wp;
	*pwp = wp;
	return TRUE;
}

// write a window placement to settings section of app's ini file
void WriteWindowPlacement(LPWINDOWPLACEMENT pwp)
{
	TCHAR szBuffer[sizeof("-32767")*8 + sizeof("65535")*2];

	wsprintf(szBuffer, szFormat,
		pwp->flags, pwp->showCmd,
		pwp->ptMinPosition.x, pwp->ptMinPosition.y,
		pwp->ptMaxPosition.x, pwp->ptMaxPosition.y,
		pwp->rcNormalPosition.left, pwp->rcNormalPosition.top,
		pwp->rcNormalPosition.right, pwp->rcNormalPosition.bottom);
	AfxGetApp()->WriteProfileString(szSection, szWindowPos, szBuffer);
}

//////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////
// Global text box formatting functions
int NewSelPos(int OrgStart, const CString &CurText, CString &FmtText)
{
	int i, Ans;
	CString tmpChar;

	if (OrgStart > CurText.GetLength()) OrgStart = CurText.GetLength();

	/*JMM 5/28/03 - This seems like a really weird thing to do, but let me explain...
	There is a case (albeit obscure) where this line fires when it shouldn't,
	so in order to let this line of code work except where it shouldn't I put in the 
	statement before the && (OrgStart != CurText.GetLength())
	This makes it so that if the first if doesn't fire, this if statement will still fire, 
	otherwise this if statement won't fire.
	I haven't been able to figure out what Chris was trying to fix with this statement, he doesn't remember either, 
	all these functions should really be re-written since they are very confusing and rather sketchy, there should be a much easier and clearer way to write them besides this way, 
	also, this function should really just be part of FormatItemText, whenever we have time to re-write that function from scratch.
	Just in case you were wondering, I tested this in phone number, ss#, and date fields to make sure it doesn't screw anything else up.
*/
	// CAH 2/8
	if (OrgStart != CurText.GetLength() && OrgStart > FmtText.GetLength()) OrgStart = FmtText.GetLength();
	//
	if (OrgStart < 0) OrgStart = CurText.GetLength();

	for (Ans=0, i=0; i<OrgStart; i++) {
		// (a.walling 2008-10-03 17:29) - PLID 31589 - ASSERTion if you try to pass a signed char to any ::is* functions.
		if (isdigit(unsigned char(CurText[i]))) {
			tmpChar = FmtText.Mid(Ans, 1);
			while ((tmpChar != "#" && tmpChar != "n") && Ans < FmtText.GetLength()){
				Ans++;
				tmpChar = FmtText.Mid(Ans, 1);
			}
			Ans++;
		}
	}
	return Ans;
}

int NextFmtPos(int FmtPos, LPCTSTR fmtText)
{
	//DRT 10/13/2008 - PLID 31662 - vs2008
	unsigned int Ans;
	for (Ans=FmtPos+1; (fmtText[Ans] != 'n') && (fmtText[Ans] != '#') && (Ans < strlen(fmtText)); Ans++);
	return Ans;
}

CString MakeFormatText(LPCTSTR fmtText) 
{
	CString Ans;//("");
	Ans.Empty();
	int len = strlen(fmtText);
	int i;
	for (i=0; i<len; i++) {
		if (fmtText[i] == 'n')
			Ans += ' ';
		else
			Ans += fmtText[i];
	}
	return Ans;
}

void FormatItemText(CWnd *wndTextBox, const CString & strNewVal, LPCTSTR fmtText)
{
	int tmpS, tmpE, newPos;
	static bool IsRunning = false;
	bool WasRunning;
	if (IsRunning) return;

	// Put numeric digits into appropriate places in format string
	CString strNewText;
	FormatText(strNewVal, strNewText, fmtText);

	// Get pointer to edit box
	CEdit *tmpWnd = (CEdit *)wndTextBox;
	tmpWnd->GetSel(tmpS, tmpE);
	
	// Write string back to the text box
	CString CurText;
	WasRunning = IsRunning;
	IsRunning = true;
	tmpWnd->GetWindowText(CurText);
	strNewText.TrimRight();
	strNewText.TrimLeft();
	if (strNewText != CurText) {
		tmpWnd->SetWindowText(strNewText);
		
		newPos = NewSelPos(tmpS, CurText, CString(fmtText));
		tmpWnd->SetSel(newPos, newPos+tmpE-tmpS);
	}
	IsRunning = WasRunning;
}

void FormatText(const CString & strInText, CString & strOutText, LPCTSTR fmtText)
{
	strOutText = MakeFormatText(fmtText);
	
	// Put numeric digits into appropriate places in format string
	int CurPos, FmtPos = -1;
	for (CurPos=0; CurPos<strInText.GetLength(); CurPos++)  {
		// (a.walling 2008-10-03 17:29) - PLID 31589 - ASSERTion if you try to pass a signed char to any ::is* functions.
		if (isdigit(unsigned char(strInText[CurPos]))) {
			FmtPos = NextFmtPos(FmtPos, fmtText);
			if (FmtPos > (strOutText.GetLength()-1)) break;
			strOutText.SetAt(FmtPos, strInText[CurPos]);
		}
	}
}

void UnformatItemText(CWnd *wndTextBox, CString & strNewVal)
{
	CString tmpStr;
	wndTextBox->GetWindowText(tmpStr);
	strNewVal = "";
	for (int i=0; i<tmpStr.GetLength(); i++) {
		// (a.walling 2008-10-03 17:29) - PLID 31589 - ASSERTion if you try to pass a signed char to any ::is* functions.
		if (isdigit(unsigned char(tmpStr[i])))
			strNewVal += tmpStr[i];
	}
}

COleVariant UnformatCurrency(COleVariant var)
{
	// The opposite of the format currency
	COleCurrency cy;
	if(GetUserDefaultLCID() == MAKELCID(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US), SORT_DEFAULT))
		cy.ParseCurrency(var.pcVal, 0, MAKELCID(MAKELANGID(LANG_ENGLISH,
			SUBLANG_ENGLISH_US), SORT_DEFAULT));
	else
		cy.ParseCurrency(var.pcVal);
	var = cy;
	return var;
}

///////////////////////////////////////////////////////////////
// For providing the user with information
//////////

// (j.jones 2016-02-25 09:11) - PLID 22663 - converted all existing MsgBox versions
// to not take parameters, and added a templated override in the .h file to safely
// handle ... cases that do not have any parameters
int MsgBoxStd(const char* strText)
{
	// Output string
	return AfxMessageBox(strText, MB_OK | MB_ICONINFORMATION);
}

int MsgBoxStd(UINT nType, const char* strText)
{
	// Output string
	return AfxMessageBox(strText, nType);
}

// (j.jones 2016-02-25 11:36) - PLID 22663 - removed the version for UINT nIDHelp, nothing used it
// (j.jones 2016-02-25 11:36) - PLID 22663 - removed the version for CException, nothing used it

///////////////////////////////////////////////////////////////////////

// New way (bob 10-16-1998)
void SetUppersInString(CWnd *wndTextBox, CString & strInText)
{
	// Get rid of leading spaces
	strInText.TrimLeft();
	
	// Loop through each character.  If there is a 
	// space, the next character should be capitalized
	CString strTemp;
	bool bSetUpper = true;
	int nSize = strInText.GetLength();
	for (int i=0; i<nSize; i++) {
		if (strInText[i] == ' ') {
			// Capitalize the next character
			bSetUpper = true;
		} else if (bSetUpper) {
			strInText.SetAt(i, toupper(strInText.GetAt(i)));
			bSetUpper = false;
		}
	}
}

CString CStringOfVariant(COleVariant var)
{
	CString tmpStr;

	switch (var.vt) {
	case VT_BSTR:
	case VT_BSTRT: 
	case VT_LPSTR:
	case VT_LPWSTR:{
		COleVariant tmpVar;
		if (VariantChangeType(tmpVar, var, 0, VT_BSTR) != S_OK)
			tmpStr = CString("");
		else tmpStr = tmpVar.pcVal; }
		break;
	case VT_DATE: {
		COleDateTime tmpDate;
		tmpDate = var.date;
		tmpStr = tmpDate.Format("%m/%d/%Y");}
		break;
	case VT_CY: {
		COleCurrency tmpCur;
		tmpCur = var.cyVal;
		tmpStr = CString("$") + tmpCur.Format();
		// Add 0 if only one character after decimal place
		if (tmpStr[ tmpStr.GetLength() - 2] == '.')
			tmpStr += '0';
		else if (tmpStr.Find('.') == -1) // Put .00 extention if none
			tmpStr += CString(".00"); }
		break;
	case VT_BOOL:
		if (var.iVal) tmpStr = CString("Yes");
		else tmpStr = CString("No");
		break;
	case VT_NULL:
	case VT_ERROR:
	case VT_VOID:
	case VT_EMPTY:
		tmpStr = "";
		break;
	case VT_R4:
		tmpStr.Format("%f", var.fltVal);
		break;
	case VT_R8:
		tmpStr.Format("%lf", var.dblVal);
		break;
	default:
		tmpStr.Format("%li", var.lVal);
		break;
	}
	return tmpStr;
}

// (j.jones 2009-04-30 14:27) - PLID 33853 - these are obsolete now, we use AES decryption,
// but we use this for the obsolete "calc secret password" concept and a couple other places
CString EncryptString_Deprecated(const CString &strInputString, bool bDecrypt /* = false */)
{
	long i, len;
	CString strAns;

	// Loop for the length of the passed string
	len = strInputString.GetLength();

	// (a.walling 2008-10-02 09:27) - PLID 31567 - Must explicitly cast as char to prevent
	// ambiguity for CString's overloaded += operator
	if (bDecrypt) {
		// If the user asks for decryption, decrypt
		for (i = 0; i < len; i++) {
			strAns += (char)((strInputString.GetAt(i) ^ 77) - 125);
		}
	} else {
		// The user wants the default, encrypt
		for (i = 0; i < len; i++) {
		   strAns += (char)((strInputString.GetAt(i) + 125) ^ 77);
		}
	}
	
	return strAns;
}

CString EncryptString_Deprecated(const CString &strInputString, LPCTSTR strDirection)
{
	if (stricmp(strDirection, "ENCRYPT") == 0) {
		// Encryption was specified, so call the encryption operation
		return EncryptString_Deprecated(strInputString, false);
	} else if (stricmp(strDirection, "DECRYPT") == 0) {
		// Decryption was specified, so call the decryption operation
		return EncryptString_Deprecated(strInputString, true);
	} else {
		// Neither was specified so return blank (failure)
		return "";
	}
}

bool GetFileSystemTime(const CString &strFile, COleDateTime &dtAns)
{
	// First try to open the file
	HANDLE h = CreateFile(strFile, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (h == INVALID_HANDLE_VALUE) {
		return false;
	}

	// Then check the time of the file
	FILETIME ft;
	if (!GetFileTime(h, NULL, NULL, &ft)) {
		CloseHandle(h);
		return false;
	}
	CloseHandle(h);

	// If we made it to here, the file time has been successfully retrieved
	// so now we need to convert it to a system time and then COleDateTime
	SYSTEMTIME st;
	FileTimeToSystemTime(&ft, &st);
	dtAns = st;

	// And then return success
	return true;
}

bool SetFileSystemTime(const CString &strFile, const COleDateTime &dt)
{
	// First convert the COleDateTime to system time
	SYSTEMTIME st;
	if (!dt.GetAsSystemTime(st)) {
		return false;
	}

	// Then convert from system time to file time
	FILETIME ft;
	if (!SystemTimeToFileTime(&st, &ft)) {
		return false;
	}

	// Try to open the file
	HANDLE h = CreateFile(strFile, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (h == INVALID_HANDLE_VALUE) {
		return false;
	}

	// Then set the time of the file
	if (!SetFileTime(h, NULL, NULL, &ft)) {
		DWORD errCode = GetLastError();
		CloseHandle(h);
		SetLastError(errCode);
		return false;
	}
	CloseHandle(h);

	// If we made it to here, we have succeeded
	return true;
}
long CalcVersion(UINT nYear, UINT nMonth, UINT nDay, UINT nExtra /* = 0 */)
{
	CString str;
	// Format of the version number YYYYMMDDE
	str.Format("%04i%02i%02i%01i", nYear, nMonth, nDay, nExtra);
	return atol(str);
}

long CalcVersion(const COleDateTime &dt)
{
	return CalcVersion(dt.GetYear(), dt.GetMonth(), dt.GetDay(), 0);
}

UINT GetVerMonth(long nVer)
{
	CString str;
	str.Format("%li", nVer);
	if (str.GetLength() == 9) {
		// This is good
		return atol(str.Mid(4, 2));
	} else {
		// This is bad
		return 0;
	}
}

UINT GetVerDay(long nVer)
{
	CString str;
	str.Format("%li", nVer);
	if (str.GetLength() == 9) {
		// This is good
		return atol(str.Mid(6, 2));
	} else {
		// This is bad
		return 0;
	}
}

UINT GetVerYear(long nVer)
{
	CString str;
	str.Format("%li", nVer);
	if (str.GetLength() == 9) {
		// This is good
		return atol(str.Left(4));
	} else {
		// This is bad
		return 0;
	}
}

UINT GetVerExtra(long nVer)
{
	CString str;
	str.Format("%li", nVer);
	if (str.GetLength() == 9) {
		// This is good
		return atol(str.Right(1));
	} else {
		// This is bad
		return 0;
	}
}

CString GetFileVersionString(long nVer)
{
	CString strVer;
	strVer.Format("%02li-%02li-%04li.%01li", GetVerMonth(nVer), GetVerDay(nVer), GetVerYear(nVer), GetVerExtra(nVer));
	return strVer;
}

CString Exclude(const CString &inString, const CString &exChar)
{
	CString outStr = "";
	CString strLocalIn = inString;

	for (int h = 0; h < exChar.GetLength(); h++)
	{
		if (h > 0) {
			strLocalIn = outStr;
			outStr = "";
		}
		for (int i = 0; i < strLocalIn.GetLength(); i++) 
		{
			if (strLocalIn.GetAt(i) != exChar.GetAt(h))
				outStr += strLocalIn.GetAt(i);
		}
	}

	return outStr;

}

CString Include(const CString &inStr, const CString &inChar)
{
	CString str;
	CString outStr = "";

	for (int i = 0; i < inStr.GetLength(); i++) 
	{
		for (int j = 0; j < inChar.GetLength(); j++) {
			if (inStr.GetAt(i) == inChar.GetAt(j))
				outStr += inChar.GetAt(j);
		}

	}

	return outStr;
}

bool DoesExist(LPCTSTR strFile)
{
	// (a.walling 2012-10-16 07:44) - PLID 52689 - Delegate to FileUtils
	return FileUtils::DoesFileOrDirExist(strFile);

	// (c.haag 2003-10-06 12:40) - Before we proceed with the CFileFind, we should make
	// sure our network credentials are accepted first, if applicable. If we have a local
	// machine path, the user is not prompted.

	// (c.haag 2003-10-07 12:24) - Taking it out until we decide where it really belongs.
	/*switch (DoNetworkLogin(GetFilePath(strFile)))
	{
	case ERROR_SESSION_CREDENTIAL_CONFLICT:
	case ERROR_ACCESS_DENIED:
		return false;
	default:
		// Any time we don't get an explicit credential conflict, we allow CFileFind
		// to determine the reachability of the file.
		break;
	}*/

	// Only search if strFile is not NULL
	//if (strFile) {
	//	// First check to see if strFile exists, and if not, check to see if strFile\*.* exists
	//	if (INVALID_FILE_ATTRIBUTES == ::GetFileAttributes(strFile)) {
	//		CFileFind f;
	//		if (f.FindFile(CString(strFile) ^ "*.*")) {
	//			return true;
	//		} else {
	//			return false;
	//		}
	//	} else {
	//		return true;
	//	}
	//}

	//DRT 9/23/03 - This code does not work.  Here is the list of problems.  1)  Even if you are successfully logging
	//		into the network, you are returning false, which tells the program that the file doesn't exist.  Maybe it 
	//		does!  2)  You are making an assumption that a file not existing means it must be on a blocked network
	//		resource.  That is definitely not the case, it could just be an invalid file, or one that isn't created yet
	//		- often we do a DoesExist() to make sure the file we are about to create doesn't exist.  In all those cases, 
	//		it leaps up with a network message.  Creating a new custom report is a good example of this.
	//	I think the real solution needs to be able to monitor the file finder and see if it returns some sort of 
	//		"error_network_disconnect" or something message, if that is possible.

	// (c.haag 2003-08-05 12:49) - It didn't work; try logging in
//	if (ERROR_CANCELLED == DoNetworkLogin(GetFilePath(strFile)))
//		return FALSE;

	// If we made it to here we have failed (NULL strFile also indicates failure)
	return false;
}

// Check to see if the given file is read-only or not
bool IsReadOnly(LPCTSTR strFile)
{
	// (a.walling 2008-10-06 16:02) - PLID 28040 - Use fileutils
	return FileUtils::IsReadOnly(strFile);
}

void SetReadOnly(LPCTSTR strFile, bool bReadOnly /* = true */)
{
	// (a.walling 2008-10-06 16:02) - PLID 28040 - Use fileutils
	FileUtils::SetReadOnly(strFile, bReadOnly);
}

// Returns true iff the file exists and can be opened non-exclusively in read mode
// If false is returned and e is not null, *e will be filled with the offending exception
bool IsFileReadable(const CString &strFile, CFileException *e /* = NULL */)
{
	if (DoesExist(strFile)) {
		CFile fil;
		// First make sure the source file is accessible
		if (fil.Open(strFile, CFile::modeRead | CFile::shareDenyNone, e)) {
			fil.Close();
			return true;
		}
	}
	return false;
}

bool bgCheckedJet40 = false;
bool bIsJet40 = false;

bool IsJet40()
{
	if (!bgCheckedJet40) {
		// HKEY_CLASSES_ROOT\TypeLib\{00025E01-0000-0000-C000-000000000046}\5.0\0\win32

		HKEY hKey = NULL;

		// Try to open the registry key for the dao dll
		long nResult = RegOpenKeyEx(HKEY_CLASSES_ROOT, 
			"TypeLib\\{00025E01-0000-0000-C000-000000000046}\\5.0\\0\\win32",
			0, KEY_READ, &hKey);

		if (nResult == ERROR_SUCCESS && hKey) {
			unsigned long nType;
			unsigned long nLen = MAX_PATH;
			CString str;

			// Try to get the path
			nResult = RegQueryValueEx(hKey, NULL, NULL, &nType, (LPBYTE)str.GetBuffer(MAX_PATH), &nLen);
			str.ReleaseBuffer();
			if (nResult == ERROR_MORE_DATA) {
				nResult = RegQueryValueEx(hKey, NULL, NULL, &nType, (LPBYTE)str.GetBuffer(nLen), &nLen);
				str.ReleaseBuffer();
			}
			
			// Now we're done with the key
			RegCloseKey(hKey);

			// So make sure the path exists
			if (nResult == ERROR_SUCCESS) {
				bIsJet40 = DoesExist(str);
				bgCheckedJet40 = true;
				return bIsJet40;
			}
		}
		
		// If we made it to here, something went wrong
		bIsJet40 = false;
		bgCheckedJet40 = true;
	}

	return bIsJet40;
}

bool bgCheckedAccess2k = false;
bool bIsAccess2k = false;

bool IsAccess2k()
{
	if (!bgCheckedAccess2k) {
		// HKEY_LOCAL_MACHINE\Software\Microsoft\Office\9.0\Access
		HKEY hKey = NULL;
		long nResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
			"Software\\Microsoft\\Office\\9.0\\Access",
			0, KEY_READ, &hKey);

		if (nResult == ERROR_SUCCESS) {
			// Re-open the more valuable key
			RegCloseKey(hKey);
			nResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
			"Software\\Microsoft\\Office\\9.0\\Access\\InstallRoot",
			0, KEY_READ, &hKey);

			if (nResult == ERROR_SUCCESS) {
				unsigned long nType;
				unsigned long nLen = MAX_PATH;
				CString str;

				// Try to get the path
				nResult = RegQueryValueEx(hKey, "Path", NULL, &nType, (LPBYTE)str.GetBuffer(MAX_PATH), &nLen);
				str.ReleaseBuffer();
				if (nResult == ERROR_MORE_DATA) {
					nResult = RegQueryValueEx(hKey, "Path", NULL, &nType, (LPBYTE)str.GetBuffer(nLen), &nLen);
					str.ReleaseBuffer();
				}
				
				// Now we're done with the key
				RegCloseKey(hKey);
			
				if (nResult == ERROR_SUCCESS) {
					// First make sure the program is installed
					str = str ^ "MSACCESS.EXE";
					if (DoesExist(str)) {
						// Now make sure it has an appropriate date for Access 2000
						HANDLE hFile;
						hFile = CreateFile(str, GENERIC_READ, FILE_SHARE_READ|
							FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
						if (((long)hFile != -1) && ((long)hFile != ERROR_ALREADY_EXISTS)) {
							FILETIME ft;
							if (GetFileTime(hFile, NULL, NULL, &ft)) {
								CloseHandle(hFile);
								SYSTEMTIME stComp;
								if (COleDateTime(1999, 1, 1, 1, 1, 1).GetAsSystemTime(stComp)) {
									FILETIME ftComp;
									if (SystemTimeToFileTime(&stComp, &ftComp)) {
										if (CompareFileTime(&ft, &ftComp) >= 0) {
											bIsAccess2k = true;
											bgCheckedAccess2k = true;
											return bIsAccess2k;
										}
									}
								}
							} else {
								CloseHandle(hFile);
							}
						}
					}
				}
			}
		}
#ifdef _DEBUG
		DWORD nErr = GetLastError();
#endif
		
		bIsAccess2k = false;
		bgCheckedAccess2k = true;
	}

	return bIsAccess2k;
}

int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	if (uMsg == BFFM_INITIALIZED) {
		::SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
	}
	return 0;
}

bool Browse(HWND hWndParent, const CString &strInitPath, CString &strReturnPath, bool bIncludeFiles /* = false */)
{
	BROWSEINFO bi;
	char strPath[MAX_PATH];
	char szInitPath[MAX_PATH];

	// Prepare structure
	memset(&bi, 0, sizeof(bi));
	memset(strPath, 0, MAX_PATH);
	sprintf(szInitPath, "%s", strInitPath);

	bi.hwndOwner = hWndParent;
	bi.pszDisplayName = strPath;
	bi.lpszTitle = "Select Path";
	bi.ulFlags = BIF_RETURNONLYFSDIRS | (bIncludeFiles?BIF_BROWSEINCLUDEFILES:0);
	bi.lpfn = BrowseCallbackProc;
	bi.lParam = (long)szInitPath;
	
	// Open dialog
	//CoInitialize(NULL);
	LPITEMIDLIST pIdList = SHBrowseForFolder(&bi);
	SHGetPathFromIDList(pIdList, strPath);
	LPMALLOC pM = NULL;
	SHGetMalloc(&pM);
	if (pM) pM->Free(pIdList);
	//CoUninitialize();

	// Return results
	if (strPath[0]) {
		strReturnPath.Format("%s", strPath);
		return true;
	} else {
		return false;
	}
}

bool MoveFileAtStartup(LPCTSTR strSource, LPCTSTR strDest)
{
	// This will work under NT
	if (MoveFileEx(strSource, strDest, MOVEFILE_DELAY_UNTIL_REBOOT)) {
		// It worked (probably because we are under NT)
		return true;
	} else {
		// It didn't work so we have to do special processing for Win95/98
		CString strLocalSource(strSource), strLocalDest;
		strLocalDest = GetFilePath(strDest);
		char *p1, *p2;
		
		// Get short file paths
		p1 = strLocalSource.GetBuffer(MAX_PATH);
		p2 = strLocalDest.GetBuffer(MAX_PATH);
		GetShortPathName(p1, p1, MAX_PATH);
		GetShortPathName(p2, p2, MAX_PATH);
		strLocalSource.ReleaseBuffer();
		strLocalDest.ReleaseBuffer();

		strLocalDest = strLocalDest ^ GetFileName(strDest);

		CString strWinDir;
		int nLen = GetWindowsDirectory(strWinDir.GetBuffer(MAX_PATH), MAX_PATH);
		strWinDir.ReleaseBuffer();
		if (nLen == 0) {
			// Failed to get the windows path
			return false;
		}
		if (strDest) { // destination not null
			if (!WritePrivateProfileString("rename", strLocalDest, strLocalSource, strWinDir ^ "WinInit.INI")) {
				// Couldn't write to file so failure
				return false;
			}
		} else {
			if (!WritePrivateProfileString("rename", "NUL", strLocalSource, strWinDir ^ "WinInit.INI")) {
				// Couldn't write to file so failure
				return false;
			}
		}
		// Made it through all that so we must have succeeded
		return true;
	}
}

bool IsLocked(const CString &strFile, CFileException *e /* = NULL */)
{
	if (DoesExist(strFile)) {
		CFile fil;
		// First make sure the source file is accessible
		if (!fil.Open(strFile, CFile::modeReadWrite | CFile::shareExclusive, e)) {
			return true;
		}
		fil.Close();
	}
	return false;
}

// (c.haag 2003-07-16 12:44) - We're phasing out NxStandard, so
// we're not going to expend the effort of adding fileutils to it.
// Gets the given file's modified date/time
// Throws CFileException on failure
// The file must already be opened with at least Read access
void GetFileModifiedTime(CFile &f, OUT CTime &tmModified)
{
	FILETIME lastWriteTime;
	if (GetFileTime(reinterpret_cast<HANDLE>(f.m_hFile), NULL, NULL, &lastWriteTime)) {
		// Success
		tmModified = lastWriteTime;
	} else {
		// Failure
		CFileException::ThrowOsError((LONG)::GetLastError());
	}
}

// (c.haag 2003-07-16 12:44) - We're phasing out NxStandard, so
// we're not going to expend the effort of adding fileutils to it.
// Copied from FileUtils.cpp
// Gets the given file's modified date/time, handling the opening and closing of the file for you
// Throws CFileException on failure
CTime GetFileModifiedTime(LPCTSTR strFullPath)
{
	CTime tmModified;
	CFile fIn(strFullPath, CFile::modeRead|CFile::shareDenyNone);
	GetFileModifiedTime(fIn, tmModified);
	return tmModified;
}

void CleanTempFiles(const CString &strTitle)
{
	CTime tm;
	CTime tmMin = CTime::GetCurrentTime() - CTimeSpan(1,0,0,0);

	for (long i=1; i<=99; i++) {
		CString strAns;
		strAns.Format("%s%li", strTitle, i);
		if (DoesExist(strAns)) {
			// (c.haag 2003-07-16 12:41) - The temp file exists. See if it was modified more
			// than 24 hours ago. If it was, then try to delete it.
			tm = GetFileModifiedTime(strAns); 
			if (tm < tmMin)
				DeleteFile(strAns);
		}
	}
}

CString GetTempFileName(const CString &strFile, char chCharacter /* = 'b' */, const LPCTSTR strPath /* = NULL */)
{
	CString strAns;
	CString strTitle;
	long pos;
	// Get the position of the character right after the last backslash (or 0 if no backslash)
	pos = strFile.ReverseFind('\\') + 1;
	strTitle = strFile.Mid(pos);
	
	// Get rid of the normal file extension
	pos = strTitle.ReverseFind('.');
	if (pos == -1) { // '.' not found
		// No extension
		pos = strTitle.GetLength();
	}
	
	// Get the correct destination path
	CString strFilePath;
	if (strPath) {
		strFilePath = strPath;
	} else {
		strFilePath = GetFilePath(strFile);
	}

	// Set Title to D:\\BackupPath\\Filename.b
	strTitle = strFilePath ^ strTitle.Left(pos) + "." + CString(chCharacter);
	
	// Now loop until you find a filename that doesn't exist
	for (long i=1; i<=99; i++) {
		strAns.Format("%s%li", strTitle, i);
		if (!DoesExist(strAns)) {
			return strAns;
		}
	}

	// If we made it here, we will start deleting temp files that are no longer in use.
	CleanTempFiles(strTitle);

	// Now loop until you find a filename that doesn't exist
	//DRT 10/13/2008 - PLID 31662 - vs2008
	for (long i=1; i<=99; i++) {
		strAns.Format("%s%li", strTitle, i);
		if (!DoesExist(strAns)) {
			return strAns;
		}
	}

	// If we made it to here we couldn't find an appropriate file to backup to
	return "";
}

CString GetFileExtension(const CString &strFile)
{
	// (a.walling 2008-10-06 16:02) - PLID 28040 - Use fileutils
	return FileUtils::GetFileExtension(strFile);
}

CString GetFilePath(const CString &strFile)
{
	// (a.walling 2008-10-06 16:02) - PLID 28040 - Use fileutils
	return FileUtils::GetFilePath(strFile);
}

CString GetFileName(const CString &strFile)
{
	// (a.walling 2008-10-06 16:02) - PLID 28040 - Use fileutils
	return FileUtils::GetFileName(strFile);
}

bool CreatePath(const CString &strPath)
{
	// (a.walling 2008-10-06 16:02) - PLID 28040 - Use fileutils
	return FileUtils::CreatePath(strPath);
}

bool IsKeyDown(int nVirtualKey)
{
    if (GetKeyState(nVirtualKey) & 0x80) {
        return true;
    } else {
        return false;
    }
}

bool IsSuperKeyDown()
{
	// TODO: Maybe change this to include control and 
	// space and num-lock, etc. like Practice.mde security
	if (IsKeyDown(VK_SHIFT)) {
		return true;
	} else {
		return false;
	}
}


HKEY GetNexTechKey()
{
	HKEY hSoftKey = NULL;
	HKEY hCompanyKey = NULL;

	// First open the registry
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("software"), 0, KEY_WRITE|KEY_READ,
		&hSoftKey) != ERROR_SUCCESS) {
		if (RegOpenKeyEx(HKEY_CURRENT_USER, _T("software"), 0, KEY_WRITE|KEY_READ,
			&hSoftKey) != ERROR_SUCCESS) {
			return NULL;
		}
		// This should not really happen.  If it does, just scare the user a little bit
		AfxMessageBox("Registry key had to be obtained from the current user");
	}

	DWORD dw;
	// Open the NexTech subkey
	RegCreateKeyEx(hSoftKey, "NexTech", 0, REG_NONE,
		REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL,
		&hCompanyKey, &dw);

	// Close the original one but leave the NexTech Key open
	if (hSoftKey != NULL)
		RegCloseKey(hSoftKey);

	// Return the open key
	return hCompanyKey;
}

bool GetNexTechProfileInt(const CString &strValueName, DWORD &nValue)
{
	// Get the key
	HKEY hNexTech = GetNexTechKey();
	long nResult = NULL;
	
	// Try to get the data from the value
	if (hNexTech) {
		unsigned long nType = REG_NONE;
		unsigned long nSize = sizeof(DWORD);
		nResult = RegQueryValueEx(hNexTech, strValueName, NULL, &nType, (LPBYTE)&nValue, &nSize);
		RegCloseKey(hNexTech);
	}

	// Return success value
	if (nResult == ERROR_SUCCESS) {
		return true;
	} else {
		return false;
	}
}

bool GetNexTechProfileString(const CString &strValueName, CString &strValue)
{
	// Get the key
	HKEY hNexTech = GetNexTechKey();
	long nResult = NULL;
	
	// Try to get the data from the value
	if (hNexTech) {
		unsigned long nType = REG_NONE;
		unsigned long nSize = MAX_PATH;
		nResult = RegQueryValueEx(hNexTech, strValueName, NULL, &nType, (LPBYTE)strValue.GetBuffer(MAX_PATH), &nSize);
		strValue.ReleaseBuffer();
		if (nResult == ERROR_MORE_DATA) {
			// If our buffer wasn't big enough, use the right sized buffer
			nResult = RegQueryValueEx(hNexTech, strValueName, NULL, &nType, (LPBYTE)strValue.GetBuffer(nSize), &nSize);
			strValue.ReleaseBuffer();
		}
		RegCloseKey(hNexTech);
	}

	// Return success value
	if (nResult == ERROR_SUCCESS) {
		return true;
	} else {
		return false;
	}
}

bool WriteNexTechProfileInt(const CString &strValueName, const DWORD nValue)
{
	// Get the key
	HKEY hNexTech = GetNexTechKey();
	long nResult = NULL;
	
	// Try to get the data from the value
	if (hNexTech) {
		nResult = RegSetValueEx(hNexTech, strValueName, 0, REG_DWORD, (CONST BYTE *)&nValue, sizeof(DWORD));
		RegCloseKey(hNexTech);
	}

	// Return success value
	if (nResult == ERROR_SUCCESS) {
		return true;
	} else {
		return false;
	}
}

bool WriteNexTechProfileString(const CString &strValueName, const CString &strValue)
{
	// Get the key
	HKEY hNexTech = GetNexTechKey();
	long nResult = NULL;
	
	// Try to get the data from the value
	if (hNexTech) {
		unsigned long nSize = strValue.GetLength() + 1;
		const char * pString = strValue.operator LPCTSTR();
		nResult = RegSetValueEx(hNexTech, strValueName, 0, REG_SZ, (CONST BYTE *)pString, nSize);
		RegCloseKey(hNexTech);
	}

	// Return success value
	if (nResult == ERROR_SUCCESS) {
		return true;
	} else {
		return false;
	}
}

// (a.walling 2011-12-21 16:10) - PLID 46648 - Dialogs must set a parent!
int InputBox(CWnd* pParent, const CString &strPrompt, CString &strResult, const CString &strOther, bool bPassword /* = false */, bool bBrowseBtn /* = false */, LPCTSTR strCancelBtnText /* = NULL */, BOOL bIsNumeric /* = FALSE*/)
{
	// Make sure we look to our dll for the resources
	//HINSTANCE hOld = AfxGetResourceHandle();
	//AfxSetResourceHandle((HINSTANCE)::GetModuleHandle(DLL_MODULE_NAME));
	
	// Open the dialog
	CNxInputDlg dlg(pParent, strPrompt, strResult, strOther, bPassword, bBrowseBtn, strCancelBtnText, -1, bIsNumeric);
	int nResult = dlg.DoModal();

	// Okay, we can now look back to the exe for future resources
	//AfxSetResourceHandle(hOld);

	// Return the correct result
	return nResult;
}

// (a.walling 2011-12-21 16:10) - PLID 46648 - Dialogs must set a parent!
int InputBoxLimited(CWnd* pParent, const CString &strPrompt, CString &strResult, const CString &strOther, unsigned long nMaxChars, bool bPassword /* = false */, bool bBrowseBtn /* = false */, LPCTSTR strCancelBtnText /* = NULL */)
{
	// Make sure we look to our dll for the resources
	//HINSTANCE hOld = AfxGetResourceHandle();
	//AfxSetResourceHandle((HINSTANCE)::GetModuleHandle(DLL_MODULE_NAME));
	
	// Open the dialog
	CNxInputDlg dlg(pParent, strPrompt, strResult, strOther, bPassword, bBrowseBtn, strCancelBtnText, nMaxChars);
	int nResult = dlg.DoModal();

	// Okay, we can now look back to the exe for future resources
	//AfxSetResourceHandle(hOld);

	// Return the correct result
	return nResult;
}

int InputBoxLimitedWithParent(CWnd* pParentWnd, const CString &strPrompt, CString &strResult, const CString &strOther, unsigned long nMaxChars, bool bPassword /* = false */, bool bBrowseBtn /* = false */, LPCTSTR strCancelBtnText /* = NULL */)
{
	// (c.haag 2007-09-11 16:15) - PLID 27353 - This is exactly like InputBoxLimited but you can pass in a parent window

	// Make sure we look to our dll for the resources
	//HINSTANCE hOld = AfxGetResourceHandle();
	//AfxSetResourceHandle((HINSTANCE)::GetModuleHandle(DLL_MODULE_NAME));
	
	// Open the dialog
	CNxInputDlg dlg(pParentWnd, strPrompt, strResult, strOther, bPassword, bBrowseBtn, strCancelBtnText, nMaxChars, FALSE);
	int nResult = dlg.DoModal();

	// Okay, we can now look back to the exe for future resources
	//AfxSetResourceHandle(hOld);

	// Return the correct result
	return nResult;
}

// (z.manning 2015-08-13 16:52) - PLID 67248 - Input box with an input mask
int InputBoxMasked(CWnd *pwndParent, const CString &strPrompt, const CString &strInputMask, OUT CString &strResult
	, const CString &strCueBanner /* = "" */)
{
	CNxInputDlg dlg(pwndParent, strPrompt, strResult, "");
	dlg.m_strInputMask = strInputMask;
	dlg.m_strCueBanner = strCueBanner;
	int nResult = dlg.DoModal();
	return nResult;
}


// (a.walling 2008-10-02 10:49) - PLID 28040 - These are all in NxUtils now...
#if _MSC_VER <= 1300
CFile g_fLogFile;
bool bKeepTryingLog = true;

CFile *GetLogFile()
{
	if (g_fLogFile.m_hFile != CFile::hFileNull) {
		return &g_fLogFile;
	}

	CFileException e;
	// Prepare flags
	int nOpenFlags = CFile::modeCreate | CFile::modeNoTruncate | 
		CFile::modeWrite | CFile::shareDenyWrite;
	
	// Try to open the file
	if (g_fLogFile.Open(GetLogFilePathName(), nOpenFlags, &e)) {
		g_fLogFile.SeekToEnd();
		LogAppend("\r\n");
		LogIndent("Begin Logging");
		return &g_fLogFile;
	} else {
		if (bKeepTryingLog) {
			// Only fail once
			bKeepTryingLog = false;
			//CString str;
			//char strErr[2048];
			//e.GetErrorMessage(strErr, 2048);
			//str.Format("Could not open log file! Error follows:\n\n%s", strErr);
			// (a.walling 2008-09-18 17:13) - PLID 28040 - Just report this error
			e.ReportError();
			//AddError(1, str);
		}
		return NULL;
	}
}

CString strLogFilePathName;

void SetLogFilePathName(const CString &strPathName)
{
	strLogFilePathName = strPathName;
}

CString GetLogFilePathName()
{
	if (strLogFilePathName.IsEmpty()) {
		strLogFilePathName = GetPracPath(true) ^ "NxLog.log";
	}

	return strLogFilePathName;
}

int nLogIndentCount = 0;
int nLogFirstCount = 0;

int Log(LPCTSTR strFormat, ...)
{
	CFile *pLog = GetLogFile();
	if (pLog && strFormat) {
		// Convert the ... to and argumented string
		CString strString;
		va_list argList;
		va_start(argList, strFormat);
		strString.FormatV(strFormat, argList);
		va_end(argList);

		// Format the string for output
		CString strOutput;
		strOutput.Format("\r\n%s:%5li: %*.0s%s", COleDateTime::GetCurrentTime().Format("%c"), GetTickCount() - nLogFirstCount, 
			((nLogIndentCount>0)?nLogIndentCount:0)*2, " ", strString);

		// Output the string
		int nLen = strOutput.GetLength();
		pLog->Write(strOutput, nLen);

		// Return the number of characters output
		return nLen;
	}
	
	// Failure return 0
	return 0;
}

int Log(CException *e, LPCTSTR strFormat, ...)
{
	if (e) {
		CFile *pLog = GetLogFile();
		if (pLog && strFormat) {
			// Convert the ... to and argumented string
			CString strString;
			va_list argList;
			va_start(argList, strFormat);
			strString.FormatV(strFormat, argList);
			va_end(argList);

			// Get the exceptions text description
			char strExceptionDesc[4096];
			e->GetErrorMessage(strExceptionDesc, 4096);

			// Now call the default logging function
			return LogFlat("%s %s", strString, strExceptionDesc);
		}
	}
	
	// Failure return 0
	return 0;
}

int LogFlat(LPCTSTR strFormat, ...)
{
	CFile *pLog = GetLogFile();
	if (pLog && strFormat) {
		// Convert the ... to and argumented string
		CString strString;
		va_list argList;
		va_start(argList, strFormat);
		strString.FormatV(strFormat, argList);
		va_end(argList);

		// For error logging we strip carriage returns and line feeds
		strString.Replace('\r', ' ');
		strString.Replace('\n', ' ');
		strString.Replace('\t', ' ');
		while (strString.Replace("--", "-"));
		while (strString.Replace("  ", " "));

		// Call the default logging function
		// (b.cardillo 2007-08-23 16:51) - PLID 27165 - We were passing strString straight in as the 
		// format string, which was causing serious problems in cases where the string contained 
		// legitimate percent characters.  So now we pass strString in as a parameter to the simple 
		// "%s" format string.
		return Log("%s", strString);
	}
	
	// Failure return 0
	return 0;
}

int LogAppend(LPCTSTR strFormat, ...)
{
	CFile *pLog = GetLogFile();
	if (pLog && strFormat) {
		// Convert the ... to and argumented string
		CString strString;
		va_list argList;
		va_start(argList, strFormat);
		strString.FormatV(strFormat, argList);
		va_end(argList);

		// Output the string
		int nLen = strString.GetLength();
		pLog->Write(strString, nLen);

		// Return the number of characters output
		return nLen;
	}
	
	// Failure return 0
	return 0;
}

int LogMany(LPCTSTR strFormat, ...)
{
	CFile *pLog = GetLogFile();
	if (pLog && strFormat) {
		// Convert the ... to and argumented string
		CString strString;
		va_list argList;
		va_start(argList, strFormat);
		strString.FormatV(strFormat, argList);
		va_end(argList);

		// For error logging we strip carriage returns and line feeds
		bool bContinue = true;
		int nPosStart;
		int nTotalLen = 0;
		while (strString.GetLength() > 0) {
			// Find the position of the first carriage return or line feed
			nPosStart = strString.FindOneOf("\r\n") + 1;
			while ((strString.GetLength() > nPosStart) && 
					((strString.GetAt(nPosStart) == '\r') || 
					 (strString.GetAt(nPosStart) == '\n'))) {
				// Keep moving forward one until you find the first charater of the next string
				nPosStart++;
			}

			if (nPosStart <= 0) {
				nPosStart = strString.GetLength();
			}
			
			// Now nPosStart is the position of the first character of the next string
			// So log the text up to that point
			// (b.cardillo 2007-08-23 16:51) - PLID 27165 - We were passing strString straight in as the 
			// format string, which was causing serious problems in cases where the string contained 
			// legitimate percent characters.  So now we pass strString in as a parameter to the simple 
			// "%s" format string.
			nTotalLen += LogFlat("%s", strString.Left(nPosStart));
			
			// Then delete all characters up to that point
			strString.Delete(0, nPosStart);
		}

		// Call the default logging function
		return nTotalLen;
	}
	
	// Failure return 0
	return 0;
}

int LogIndent()
{
	nLogIndentCount++;
	return 0;
}

int LogIndent(LPCTSTR strFormat, ...)
{
	CFile *pLog = GetLogFile();
	if (pLog && strFormat) {
		// Convert the ... to and argumented string
		CString strString;
		va_list argList;
		va_start(argList, strFormat);
		strString.FormatV(strFormat, argList);
		va_end(argList);

		// Call the default logging function
		// (b.cardillo 2007-08-23 16:51) - PLID 27165 - We were passing strString straight in as the 
		// format string, which was causing serious problems in cases where the string contained 
		// legitimate percent characters.  So now we pass strString in as a parameter to the simple 
		// "%s" format string.
		int nLen = Log("%s", strString);

		// Indent now that we've output the main text
		nLogIndentCount++;

		// Return the number of characters output
		return nLen;
	}
	
	// Failure return 0
	return 0;
}

int LogUnindent()
{
	nLogIndentCount--;
	return 0;
}

int LogUnindent(LPCTSTR strFormat, ...)
{
	CFile *pLog = GetLogFile();
	if (pLog && strFormat) {
		// Unindent first then output the concluding text
		nLogIndentCount--;

		// Convert the ... to and argumented string
		CString strString;
		va_list argList;
		va_start(argList, strFormat);
		strString.FormatV(strFormat, argList);
		va_end(argList);

		// Call the default logging function
		// (b.cardillo 2007-08-23 16:51) - PLID 27165 - We were passing strString straight in as the 
		// format string, which was causing serious problems in cases where the string contained 
		// legitimate percent characters.  So now we pass strString in as a parameter to the simple 
		// "%s" format string.
		int nLen = Log("%s", strString);

		// Return the number of characters output
		return nLen;
	}
	
	// Failure return 0
	return 0;
}
#endif
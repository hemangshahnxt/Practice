#include "stdafx.h"
#include "SecurityUtils.h"
#include <Sddl.h>
#include "FileUtils.h"
#include <Aclapi.h>

// (a.walling 2009-10-13 10:01) - PLID 35930
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (b.cardillo 2009-03-26 14:40) - This whole utility source file was created originally for use 
// by plid 14887, but I created it separately since it could be useful in general.

// (b.cardillo 2011-01-06 20:34) - A few new utility functions necessary for PLID 33748.

// Returns the unfriendly string associated with the specified SID (e.g. "S-1-5-21-2263713762-513851276-1464104247-1008")
CString SecurityUtils::GetSidRawString(PSID pSID)
{
	CString strRawSIDString;
	{
		LPTSTR pstrName = NULL;
		if (ConvertSidToStringSid(pSID, &pstrName)) {
			strRawSIDString = pstrName;
		}
		LocalFree(pstrName);
	}
	return strRawSIDString;
}

// Takes a relative or absolute path and returns the remote machine name the path references, automatically 
// expanding network drive if necessary.  If the path is local, empty string is returned.
// TODO: This is implemented in a way that could be used more generally than just this local source file, so
// ultimately it should be moved somewhere more generic, like FileUtils.  But right now FileUtils doesn't 
// have a dependency on mpr.lib, and this function does.  Someday we'll need to move it.
CString GuessSystemNameFromPath(const CString &strFilePath)
{
	CString strAbsolutePath = FileUtils::GetAbsolutePath(strFilePath);
	if (strAbsolutePath.GetLength() > 2) {
		// Detect whether it's a drive letter or unc based path
		if (strAbsolutePath.GetAt(1) == _T(':')) {
			// The only valid drive letters are a-z && A-Z.
			TCHAR c = strAbsolutePath.GetAt(0);
			if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
				CString strDrive = strAbsolutePath.Left(2);
				DWORD dwLength = 0;
				if (WNetGetConnection(strDrive, NULL, &dwLength) == ERROR_MORE_DATA) {
					// Good, allocate the string of the correct size and call it again
					LPTSTR pstrBase = new TCHAR[dwLength];
					if (WNetGetConnection(strDrive, pstrBase, &dwLength) == NO_ERROR) {
						CString strBase = pstrBase;
						delete []pstrBase;
						// Ok, now we just need to extract the machine name from the unc base
						if (strBase.GetLength() > 2 && strBase.GetAt(0) == _T('\\') && strBase.GetAt(1) == _T('\\')) {
							long nEnd = strBase.Find(_T('\\'), 2);
							if (nEnd == -1) {
								nEnd = strBase.GetLength();
							}
							return strBase.Mid(2, nEnd - 2);
						} else {
							// We got some crazy kind of string back as the mapped path
							return _T("");
						}
					} else {
						// Weird, we got the size, but then failed after allocating that size
						delete []pstrBase;
						return _T("");
					}
				} else {
					// Couldn't get info about the drive letter, so it's probably local
					return _T("");
				}
			} else {
				// Not a valid drive letter
				return _T("");
			}
		} else if (strAbsolutePath.GetAt(0) == _T('\\') && strAbsolutePath.GetAt(1) == _T('\\')) {
			// Unc already
			long nEnd = strAbsolutePath.Find(_T('\\'), 2);
			if (nEnd == -1) {
				nEnd = strAbsolutePath.GetLength();
			}
			return strAbsolutePath.Mid(2, nEnd - 2);
		} else {
			// Unrecognizable path, doesn't start with "X:" or "\\"
			return _T("");
		}
	} else {
		// Invalid path given
		return _T("");
	}
}

// Returns the friendly name of the account that owns the specified file; if the friendly name cannot be 
// determined, returns the raw SID string.
CString SecurityUtils::GetFileOwner(const CString &strFilePath)
{
	DWORD dwSizeNeeded = 0;
    if (GetFileSecurity(strFilePath, OWNER_SECURITY_INFORMATION, 0, 0, &dwSizeNeeded) == FALSE && 
		GetLastError() == ERROR_INSUFFICIENT_BUFFER && 
		dwSizeNeeded != 0)
	{
        // Call it again with a buffer of appropriate size
		LPBYTE lpSecurityBuffer = new BYTE[dwSizeNeeded];
        if (GetFileSecurity(strFilePath, OWNER_SECURITY_INFORMATION, lpSecurityBuffer, dwSizeNeeded, &dwSizeNeeded)) {
			// Get the the owner SID from the descriptor
			PSID pSID = NULL;
			BOOL bOwnerDefaulted = FALSE;
			if (GetSecurityDescriptorOwner(lpSecurityBuffer, &pSID, &bOwnerDefaulted)) {
				CString strAns = GetSidAccountName(pSID, _T(""));
				if (!strAns.IsEmpty()) {
					// Good, we got it.  Fall through
				} else {
					// Try again this time guessing the system name from the path
					CString strSystemName = GuessSystemNameFromPath(strFilePath);
					if (!strSystemName.IsEmpty()) {
						strAns = GetSidAccountName(pSID, strSystemName);
						if (!strAns.IsEmpty()) {
							// Good, we got it this time! Fall through.
						} else {
							// Still failed, so just use the raw SID string
							strAns = GetSidRawString(pSID);
						}
					} else {
						// Couldn't guess the system name, so don't bother trying again (we'd just get the 
						// same failure).  So use the raw SID string.
						strAns = GetSidRawString(pSID);
					}
				}
				// We have our answer
				delete []lpSecurityBuffer;
				return strAns;
			} else {
				delete []lpSecurityBuffer;
				return _T("");
			}
		} else {
            delete []lpSecurityBuffer;
            return _T("");
        }
	} else {
		return _T("");
	}
}

// Returns the friendly name of the account that owns the specified NT object; if the friendly name cannot be 
// determined, returns the raw SID string.
CString SecurityUtils::GetObjectOwnerName(const CString &strObjectName)
{
    // Pretty simple, just ask Windows for the SID of the object's owner, then return the friendly name
	PSID psidOwner;
	PSECURITY_DESCRIPTOR psd;
	// Ask windows for the SID of the object's owner
	if (GetNamedSecurityInfo(CString(strObjectName).GetBuffer(0), SE_KERNEL_OBJECT, OWNER_SECURITY_INFORMATION, &psidOwner, NULL, NULL, NULL, &psd) == ERROR_SUCCESS) {
		// Try to get the actual friendly username; we don't bother considering a remote machine name because we're talking 
		// about an NT object, which is inherently local to this machine.  It seems conceivable to me that the user doesn't  
		// actually exist on this machine (and is operating under more general group permission at the moment); I'm not sure 
		// that's possible but even if it is I don't expect it to come up very much at all.  If it becomes an issue we'll 
		// have to find a way to guess the machine name, find another way, or more likely just give a slightly different 
		// message to the user.
		CString strAns = GetSidAccountName(psidOwner, _T(""));
		if (strAns.IsEmpty()) {
			strAns = GetSidRawString(psidOwner);
		}
		// Free the psd (which is home to the psidOwner, so we don't need to free that)
		LocalFree(psd);
		return strAns;
	} else {
		return _T("");
	}
}

// Handy utility for creating a copy of a SID.  Call LocalFree() to free the SID when you're finished.
PSID SecurityUtils::CopySid(PSID pSid)
{
     DWORD dwLength = GetLengthSid(pSid);
     PSID pSidAns = (PSID)LocalAlloc(LPTR, dwLength);
	 if (::CopySid(dwLength, pSidAns, pSid)) {
		 return pSidAns;
	 } else {
         LocalFree(pSidAns);
         return NULL;
     }
}

// Returns a user account SID associated with the specified token.  Call LocalFree() to free the SID when you're finished.
BOOL SecurityUtils::GetTokenUserSid(HANDLE token, PSID *ppSID)
{
    *ppSID = NULL;
    DWORD cb = 0;
	if (GetTokenInformation(token, TokenUser, NULL, 0, &cb) == FALSE && GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
		PTOKEN_USER ptokUser = (PTOKEN_USER)LocalAlloc(LPTR, cb);
		if (GetTokenInformation(token, TokenUser, ptokUser, cb, &cb)) {
			*ppSID = CopySid(ptokUser->User.Sid);
			LocalFree(ptokUser);
			return TRUE;
		} else {
			LocalFree(ptokUser);
			return FALSE;
		}
	} else {
		return FALSE;
	}
}

// Returns a user account SID associated with the specified process.  Call LocalFree() to free the SID when you're finished.
BOOL SecurityUtils::GetProcessUserSid(HANDLE hProcess, PSID *ppSID)
{
    *ppSID = NULL;
    HANDLE procToken = NULL;
    if (OpenProcessToken(hProcess, TOKEN_QUERY, &procToken)) {
	    BOOL ret = GetTokenUserSid(procToken, ppSID);
        CloseHandle(procToken);
		return ret;
	} else {
		return FALSE;
	}
}

// Returns a pointer to a new SECURITY_DESCRIPTOR object with the specified parameters.  Call LocalFree() to destroy it.
PSECURITY_DESCRIPTOR SecurityUtils::CreateSecurityDescriptor(BOOL bCurUserIsOwner, DWORD grfEveryoneAccessPermissions)
{
    // Create a well-known SID for the Everyone group.
	PSID pEveryoneSID;
    SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;
    if (AllocateAndInitializeSid(&SIDAuthWorld, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, &pEveryoneSID)) {
		// Initialize an EXPLICIT_ACCESS structure for an ACE.
		// The ACE will allow Everyone read access to the key.
		EXPLICIT_ACCESS eaEveryoneReadWrite;
		ZeroMemory(&eaEveryoneReadWrite, sizeof(EXPLICIT_ACCESS));
		eaEveryoneReadWrite.grfAccessPermissions = grfEveryoneAccessPermissions;
		eaEveryoneReadWrite.grfAccessMode = SET_ACCESS;
		eaEveryoneReadWrite.grfInheritance = NO_INHERITANCE;
		eaEveryoneReadWrite.Trustee.TrusteeForm = TRUSTEE_IS_SID;
		eaEveryoneReadWrite.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
		eaEveryoneReadWrite.Trustee.ptstrName  = (LPTSTR)pEveryoneSID;

		// Create a new ACL that contains the new ACE.
	    PACL pACL = NULL;
		DWORD dwRes = SetEntriesInAcl(1, &eaEveryoneReadWrite, NULL, &pACL);
		if (ERROR_SUCCESS == dwRes) {
			// Create a security descriptor.  
			PSECURITY_DESCRIPTOR pSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH); 
			if (pSD != NULL) {
				// Initialize the security descriptor
				if (InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION)) {  
					// Optionally set the owner
					PSID pUserSid;
					if (bCurUserIsOwner) {
						if (GetProcessUserSid(GetCurrentProcess(), &pUserSid)) {
							if (SetSecurityDescriptorOwner(pSD, pUserSid, FALSE)) {
								// Fall through to the rest of the function
							} else {
								// SetSecurityDescriptorDacl() failed
								LocalFree(pUserSid);
								LocalFree(pSD);
								LocalFree(pACL);
								FreeSid(pEveryoneSID);
								return NULL;
							}
						} else {
							// SetSecurityDescriptorDacl() failed
							LocalFree(pSD);
							LocalFree(pACL);
							FreeSid(pEveryoneSID);
							return NULL;
						}
					} else {
						pUserSid = NULL;
					}
					// Add the ACL to the security descriptor. 
					{
						if (SetSecurityDescriptorDacl(pSD, TRUE, pACL, FALSE)) {  
							// Success!
							FreeSid(pEveryoneSID);
							return pSD;
						} else {
							// SetSecurityDescriptorDacl() failed
							if (pUserSid != NULL) {
								LocalFree(pUserSid);
							}
							LocalFree(pSD);
							LocalFree(pACL);
							FreeSid(pEveryoneSID);
							return NULL;
						}
					}
				} else {
					// InitializeSecurityDescriptor() failed
					LocalFree(pSD);
			        LocalFree(pACL);
			        FreeSid(pEveryoneSID);
					return NULL;
				} 
			} else { 
				// LocalAlloc() failed
		        LocalFree(pACL);
		        FreeSid(pEveryoneSID);
				return NULL;
			} 
		} else {
			// SetEntriesInAcl() failed
	        FreeSid(pEveryoneSID);
			return NULL;
		}
	} else {
        // AllocateAndInitializeSid() failed
        return NULL;
    }
}

// Creates a security attributes object with the specified parameters.  Call DestroySecurityAttributes() to destroy it.
LPSECURITY_ATTRIBUTES SecurityUtils::CreateSecurityAttributes(BOOL bCurUserIsOwner, DWORD grfEveryoneAccessPermissions)
{
	// Instantiate and initialize the sa
	LPSECURITY_ATTRIBUTES psa = (LPSECURITY_ATTRIBUTES)LocalAlloc(LPTR, sizeof(SECURITY_ATTRIBUTES));
	psa->nLength = sizeof(SECURITY_ATTRIBUTES);
	psa->bInheritHandle = FALSE;

	// Create the descriptor and set our psa to point to it
	psa->lpSecurityDescriptor = CreateSecurityDescriptor(bCurUserIsOwner, grfEveryoneAccessPermissions);

	// Return success or failure
	if (psa->lpSecurityDescriptor != NULL) {
		return psa;
	} else {
		DWORD dwLastError = GetLastError();
		LocalFree(psa);
		SetLastError(dwLastError);
		return NULL;
	}
}

// Destroys a security attributes object created by CreateSecurityAttributes()
BOOL SecurityUtils::DestroySecurityAttributes(LPSECURITY_ATTRIBUTES psa)
{
	// Destroy the descriptor
	if (psa != NULL) {
		// See if there's a descriptor to destroy
		if (psa->lpSecurityDescriptor != NULL) {
			// Determine the pSidOwner to destroy
			PSID pSidOwner = NULL;
			{
				BOOL bOwnerDefaulted = TRUE;
				if (GetSecurityDescriptorOwner(psa->lpSecurityDescriptor, &pSidOwner, &bOwnerDefaulted)) {
					if (pSidOwner != NULL && bOwnerDefaulted == FALSE) {
						// Got it, we know what to destroy
					} else {
						// None to destroy
						pSidOwner = FALSE;
					}
				} else {
					// Bad security descriptor?
					return FALSE;
				}
			}
			
			// Determine the pACL to destroy
		    PACL pACL = NULL;
			{
				BOOL bDaclPresent, bDaclDefaulted;
				if (GetSecurityDescriptorDacl(psa->lpSecurityDescriptor, &bDaclPresent, &pACL, &bDaclDefaulted)) {  
					if (bDaclPresent && !bDaclDefaulted) {
						if (pACL != NULL) {
							// Got it, we know what to destroy
						} else {
							// None to destroy
							pACL = NULL;
						}
					} else {
						// None to destroy
						pACL = NULL;
					}
				} else {
					// Bad security descriptor?
					return FALSE;
				}
			}

			// Ok, now we know what to destroy, so destroy it
			if (pSidOwner != NULL) {
				LocalFree(pSidOwner);
			}
			if (pACL != NULL) {
				LocalFree(pACL);
			}
			// And destroy the descriptor
			LocalFree(psa->lpSecurityDescriptor);
			psa->lpSecurityDescriptor = NULL;
		}

		// And finally destroy the security attributes object itself
		LocalFree(psa);
		return TRUE;
	} else {
		// Bad pointer passed
		return FALSE;
	}
}

// Returns a friendly username corresponding to the user account of the given SID.  This can be slow, especially for 
// remote accounts, so consider displaying GetSidRawString() first and then calling this function asynchronously.
CString SecurityUtils::GetSidAccountName(PSID pSid, OPTIONAL const CString &strSystemName)
{
	DWORD dwNameLen = 0;
	DWORD dwDomainNameLen = 0;
	SID_NAME_USE sidNameUse = SidTypeUser;
	if (LookupAccountSid(strSystemName, pSid, NULL, &dwNameLen, NULL, &dwDomainNameLen, &sidNameUse) == FALSE && 
		GetLastError() == ERROR_INSUFFICIENT_BUFFER && 
		dwNameLen != 0 && 
		dwDomainNameLen != 0 && 
		sidNameUse == SidTypeUser) 
	{
		LPTSTR pstrName = new TCHAR[dwNameLen];
		LPTSTR pstrDomainName = new TCHAR[dwDomainNameLen];
		if (LookupAccountSid(strSystemName, pSid, pstrName, &dwNameLen, pstrDomainName, &dwDomainNameLen, &sidNameUse)) {
			CString strAns = pstrDomainName + CString(_T("\\")) + pstrName;
			delete []pstrName;
			delete []pstrDomainName;
			return strAns;
		} else {
			delete []pstrName;
			delete []pstrDomainName;
			return _T("");
		}
	} else {
		return _T("");
	}
}

// Creates a file at the specified path the same way CreateFile() does, applying a 
// security descriptor according to the "Sec" parameters.
HANDLE SecurityUtils::CreateSecureFile(LPCTSTR strFilePathName, DWORD dwDesiredAccess, DWORD dwShareMode, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile, BOOL bSec_CurUserIsObjectOwner, DWORD grfSec_EveryoneAccessPermissions)
{
	// Create the self-owned public security attributes object
	LPSECURITY_ATTRIBUTES psa = CreateSecurityAttributes(bSec_CurUserIsObjectOwner, grfSec_EveryoneAccessPermissions);
	if (psa != NULL) {
		// Good, now create the mutex with the self-owned public security attributes
		HANDLE hAns = CreateFile(strFilePathName, dwDesiredAccess, dwShareMode, psa, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
		DWORD dwLastError = GetLastError();
		// Destroy our security attributes object, now that it's been copied into the mutex
		DestroySecurityAttributes(psa);
		SetLastError(dwLastError);
		// Return the result
		return hAns;
	} else {
		// Couldn't create self-owned security attributes
		return INVALID_HANDLE_VALUE;
	}
}

// Creates a mutex of the specified name the same way CreateMutex() does, applying a 
// security descriptor according to the "Sec" parameters.
HANDLE SecurityUtils::CreateSecureMutex(LPCTSTR strMutexName, BOOL bEnterMutexOnCreate, BOOL bSec_CurUserIsObjectOwner, DWORD grfSec_EveryoneAccessPermissions)
{
	// Create the self-owned public security attributes object
	LPSECURITY_ATTRIBUTES psa = CreateSecurityAttributes(bSec_CurUserIsObjectOwner, grfSec_EveryoneAccessPermissions);
	if (psa != NULL) {
		// Good, now create the mutex with the self-owned public security attributes
		HANDLE hMut = CreateMutex(psa, bEnterMutexOnCreate, strMutexName);
		DWORD dwLastError = GetLastError();
		// Destroy our security attributes object, now that it's been copied into the mutex
		DestroySecurityAttributes(psa);
		SetLastError(dwLastError);
		// Return the result
		return hMut;
	} else {
		// Couldn't create self-owned security attributes
		return NULL;
	}
}


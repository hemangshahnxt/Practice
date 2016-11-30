#include "stdafx.h"
#include "NxSecurity.h"
#include "GlobalUtils.h"
#include "AuditTrail.h"
#include "ChangePasswordDlg.h"
#include "DateTimeUtils.h"
#include "NxAPI.h"
#include "NxAPIManager.h"

// (a.walling 2009-10-13 10:01) - PLID 35930
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CArray<CBuiltInObject*,CBuiltInObject*> gc_aryUserDefinedObjects;;
extern const CBuiltInObject gc_aryBuiltInObjects[];
extern const long gc_nBuiltInObjectCount;

const CPermissions permissionsNone(0);

using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37025 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



class CMapObjectValueToPermissions : public CMap<long, long, CPermissions*, CPermissions*> 
{
public:
	~CMapObjectValueToPermissions()
	{
		POSITION p = GetStartPosition();
		while (p) {
			long nVal;
			CPermissions *pPerms;
			GetNextAssoc(p, nVal, pPerms);
			delete pPerms;
		}
	}

};

struct CSecurityObjectPermissions
{
	// The permissions at the built-in-object level
	CPermissions m_BuiltInPermissions;
  
	// If there are any more detailed permissions, they'll be found in this map
	CMapObjectValueToPermissions m_mapDetailedPermissions;
};

class CMapBuiltInIDToObjectPermissions : public CMap<EBuiltInObjectIDs, EBuiltInObjectIDs, CSecurityObjectPermissions *, CSecurityObjectPermissions *>
{
public:
	~CMapBuiltInIDToObjectPermissions()
	{
		POSITION p = GetStartPosition();
		while (p) {
			EBuiltInObjectIDs bioid;
			CSecurityObjectPermissions *psop;
			GetNextAssoc(p, bioid, psop);
			delete psop;
		}
	}
};

class CMapUserHandleToPermissionMap : public CMap</*user*/ HANDLE, /*user*/ HANDLE, CMapBuiltInIDToObjectPermissions *, CMapBuiltInIDToObjectPermissions *> 
{
public:
	~CMapUserHandleToPermissionMap()
	{
		POSITION p = GetStartPosition();
		while (p) {
			HANDLE h;
			CMapBuiltInIDToObjectPermissions *pop;
			GetNextAssoc(p, h, pop);
			delete pop;
		}
	}
};


CMapUserHandleToPermissionMap m_mapUserPermissionCache;

CBuiltInObject::CBuiltInObject(EBuiltInObjectIDs eBuiltInID, const CPermissions &AvailablePermissions, const CString &strDisplayName, const CString &strDescription, LPCTSTR strDynamicPermissionNames /*= NULL*/, EBuiltInObjectIDs eParentID /*= bioInvalidID*/, long nObjectValue /*= 0*/)
{
	m_eBuiltInID = eBuiltInID;
	m_strDisplayName = strDisplayName;
	m_strDescription = strDescription;
	m_AvailPermissions = AvailablePermissions;
	m_eParentID = eParentID;
	m_nObjectValue = nObjectValue;

/*		DRT 3/31/03 - This special case causes major problems in the default permissions setup.  And really it's not the
				best place to put it.  Any special handling is now done in the code that checks permissions, to make sure
				that ViewWithPass does not pop up a message box.
	// Special case, for now anything that asks for sptView AND any other permissions 
	// besides sptView|sptViewWithPass, we are going to disable the sptViewWithPass
	// This is just a temporary work-around because probably the sptViewWithPass will 
	// eventually be removed as an option and then objects with only sptView|sptViewWithPass 
	// available should be switched to sptView|sptRead|sptReadWithPass.
//	if ((m_AvailPermissions.nPermissions & sptView) && (m_AvailPermissions.nPermissions & ~(sptView|sptViewWithPass))) {
//		m_AvailPermissions.nPermissions &= ~sptViewWithPass;
//	}
*/

	// Parse the null-delimited external permission names, adding each one to the array
	if (strDynamicPermissionNames) {
		LPCTSTR pCurName = strDynamicPermissionNames;
		for (long i=0; (i<NX_SECURITY_DYNAMIC_PERMISSION_COUNT) && (*pCurName != '\0'); i++) {

			// Assert that if an i-th permission is being named, then the sptDynamic-i permission must be available
			#ifdef _DEBUG
				switch (i) {
				case 0: ASSERT(m_AvailPermissions.bDynamic0 || m_AvailPermissions.bDynamic0WithPass); break;
				case 1: ASSERT(m_AvailPermissions.bDynamic1 || m_AvailPermissions.bDynamic1WithPass); break;
				case 2: ASSERT(m_AvailPermissions.bDynamic2 || m_AvailPermissions.bDynamic2WithPass); break;
				case 3: ASSERT(m_AvailPermissions.bDynamic3 || m_AvailPermissions.bDynamic3WithPass); break;
				case 4: ASSERT(m_AvailPermissions.bDynamic4 || m_AvailPermissions.bDynamic4WithPass); break;
				}
			#endif

			// Store the name
			m_strDynamicPermissionNames[i] = pCurName;
			// Move to the next name in the null-delimited string of strings
			pCurName += strlen(pCurName) + 1;
		}
	}

	// Assert that there is a name for each available dynamic permission
	#ifdef _DEBUG
		if (m_AvailPermissions.bDynamic0 || m_AvailPermissions.bDynamic0WithPass) ASSERT(m_strDynamicPermissionNames[0].IsEmpty() == FALSE);
		if (m_AvailPermissions.bDynamic1 || m_AvailPermissions.bDynamic1WithPass) ASSERT(m_strDynamicPermissionNames[1].IsEmpty() == FALSE);
		if (m_AvailPermissions.bDynamic2 || m_AvailPermissions.bDynamic2WithPass) ASSERT(m_strDynamicPermissionNames[2].IsEmpty() == FALSE);
		if (m_AvailPermissions.bDynamic3 || m_AvailPermissions.bDynamic3WithPass) ASSERT(m_strDynamicPermissionNames[3].IsEmpty() == FALSE);
		if (m_AvailPermissions.bDynamic4 || m_AvailPermissions.bDynamic4WithPass) ASSERT(m_strDynamicPermissionNames[4].IsEmpty() == FALSE);
	#endif
}

void LoadSingleUserDefinedPermission(_RecordsetPtr& rs);

// (b.cardillo 2005-07-08 12:48) - I don't understand why the GetUserDefinedObject() function 
// even exists.  Looks like when someone totally undermined the behavior of 
// gc_aryUserDefinedObjects (made it non-const!) which made GetBuiltInObject() stop working 
// properly, he or she created GetUserDefinedObject() to supercede GetBuiltInObject().  But 
// that's why I don't get it.  Why didn't he or she just rewrite GetBuiltInObject() to make 
// it handle the new gc_aryUserDefinedObjects behavior?  It seems to me the restructuring of 
// gc_aryUserDefinedObjects was done haphazardly (and half-assedly?)
const CBuiltInObject *GetUserDefinedObject(EBuiltInObjectIDs eBuiltInObjectID)
{
	// (c.haag 2003-10-27 17:49) - gc_aryUserDefinedObjects is dynamic. For instance,
	// template rule permissions can be changed on the fly.
	for (long i=0; i<gc_aryUserDefinedObjects.GetSize(); i++) {
		if (gc_aryUserDefinedObjects[i]->m_eBuiltInID == eBuiltInObjectID)
			return gc_aryUserDefinedObjects[i];
	}
	return NULL;
}

const CBuiltInObject *GetBuiltInObject(EBuiltInObjectIDs eBuiltInObjectID)
{
	static CMap<EBuiltInObjectIDs, EBuiltInObjectIDs, const CBuiltInObject *, const CBuiltInObject *> mapBuiltInObjects;
	static bool bBuiltInObjectMapIsSet = false;

	if (!bBuiltInObjectMapIsSet) {
		for (long i=0; i<gc_nBuiltInObjectCount; i++) {
			mapBuiltInObjects[gc_aryBuiltInObjects[i].m_eBuiltInID] = &(gc_aryBuiltInObjects[i]);
		}
		bBuiltInObjectMapIsSet = true;
	}

	return mapBuiltInObjects[eBuiltInObjectID];
}

POSITION GetFirstBuiltInObjectPosition()
{
	// The first object position is 1, because we'll interpret it as the index into the array
	if (gc_nBuiltInObjectCount > 0) {
		return (POSITION)1;
	} else {
		return 0;
	}
}

const CBuiltInObject *GetNextBuiltInObject(IN OUT POSITION &pInOut)
{
	long n = (long)pInOut;
	if (n >= 0) {
		if (n < gc_nBuiltInObjectCount) {
			// This is the common case, just increment p and return the object that p had been referencing
			pInOut = (POSITION)(n + 1);
			return &(gc_aryBuiltInObjects[n-1]);
		} else if (n == gc_nBuiltInObjectCount) {
			// This is the last one, so set p to NULL because there is no "next one"
			pInOut = NULL;
			// But still return this one
			return &(gc_aryBuiltInObjects[n-1]);
		} else {
			// p is out of range, this should be impossible unless someone called this with their own made-up p
			ASSERT(FALSE);
			pInOut = NULL;
			return NULL;
		}
	} else {
		// p is out of range, this should be impossible unless someone called this with their own made-up p
		ASSERT(FALSE);
		pInOut = NULL;
		return NULL;
	}
}

// Returns 0 if valid, 1 if bad username, 2 if bad password
// (j.jones 2008-11-19 11:08) - PLID 28578 - added pbIsPasswordVerified as a parameter
int AuthenticateUser(IN const CString &strUsername, IN const CString &strPassword, IN long nLocationID, OPTIONAL OUT long *pnUserID = NULL, OPTIONAL OUT CString *pstrTruePassword = NULL, OPTIONAL OUT BOOL *pbIsPasswordVerified = NULL)
{
	CString strTruePass;
	COleDateTime dtPwExpires;	
	BOOL bIsPasswordVerified = FALSE;
	BOOL bPasswordExpireNextLogin = FALSE;
	long nUserID = -1;
	long nFailedLogins = -1;
	// (j.jones 2008-11-19 10:51) - PLID 28578 - added bIsPasswordVerified as a parameter
	if (!LoadUserInfo(strUsername, nLocationID, &strTruePass, &bIsPasswordVerified, NULL, &nUserID, &dtPwExpires, &bPasswordExpireNextLogin, &nFailedLogins)) {
	/*	DRT 2/3/03 - This code can ONLY be executed if there is no username of Developer in the data, but someone got it as the strUsername field, 
		which, I assume, at one point we had the ability to just type in your username (ala Windows Login), and that this case would let you in
		without a password if you did so... I see no reason to keep it here.

		if (strUsername == DEVELOPER_USERNAME) { // Allow true user or developer
			strTruePass = GetSecretPassword();
		} else {
	*/
			// Couldn't find user
			return 1;
	//	}
	}

	//TES 4/30/2009 - PLID 28573 - If the user is already over the limit for failed logins, then don't bother going 
	// any farther.
	// (d.thompson 2012-08-07) - PLID 51969 - Changed default to Enabled
	if(GetRemotePropertyInt("LockAccountsOnFailedLogin", 1, 0, "<None>")) {
		long nMaxFailedLogins = GetRemotePropertyInt("FailedLoginsAllowed", 5, 0, "<None>");
		if(nFailedLogins >= nMaxFailedLogins) {
			return 5;
		}
	}

	if(pnUserID) {
		*pnUserID = nUserID;
	}

	if(pbIsPasswordVerified) {
		*pbIsPasswordVerified = bIsPasswordVerified;
	}

	// (b.savon 2015-12-18 10:07) - PLID 67705 - Initialize the API here
	// (b.savon 2016-01-13 11:26) - PLID 67718 - Supplemental
	SetCurrentUserID(nUserID);
	SetCurrentUserName(strUsername);
	SetCurrentUserPassword(strPassword);
	SetCurrentLocationID(nLocationID);
	if (theApp.InitializeAPI(strPassword) == FALSE) {
		return 2;
	}
	else {

		// (b.savon 2016-01-13 11:26) - PLID 67718 - Supplemental
		NexTech_Accessor::_LoginResultPtr loginResult = theApp.GetAPIObject()->GetLoginResult();

		if (loginResult->GetLoginFailed() == VARIANT_TRUE) {

			SetCurrentUserID(-1);
			SetCurrentUserName("");
			SetCurrentUserPassword("");
			SetCurrentLocationID(-1);

			if (loginResult->GetLockedOut() == VARIANT_TRUE) {
				return 5;
			}
			else {
				return 2;
			}
		}
		else {
			//Login passed
			strTruePass = strPassword;
		}

		// (c.haag 2004-03-04 17:21) - Check if the user's password is expired. If so, we prompt them to enter a new one.
		// MSC 3-24-2004 - The built in Nextech TechSupport username should never have it's password expire
		ASSERT(pnUserID);
		// (j.jones 2008-11-20 09:11) - PLID 23227 - check if the user's password is set to expire at next login, which is right now
		if (strUsername != BUILT_IN_USER_NEXTECH_TECHSUPPORT_USERNAME && pnUserID
			&& (
				(dtPwExpires.GetStatus() == COleDateTime::valid && COleDateTime::GetCurrentTime() > dtPwExpires)
				|| bPasswordExpireNextLogin
				)
			)
		{
			// (b.savon 2015-12-16 09:29) - PLID 67718
			CChangePasswordDlg dlg(NULL);
			dlg.SetUserID(*pnUserID);
			dlg.SetUserName(strUsername);
			dlg.SetLocationID(nLocationID);
			MsgBox("Your password has expired. Please enter a new password in the next window.");
			if (IDCANCEL == dlg.DoModal())
				return 3;
			strTruePass = dlg.GetNewPassword();

			// (j.jones 2008-11-20 10:15) - PLID 14462 - the change password ability will update
			// PasswordPivotDate and PWExpireNextLogin
		}


		// Found user and password is acceptable; pass back the true password if requested, and return success
		if (pstrTruePassword) {
			*pstrTruePassword = strTruePass;
		}

		return 0;
	}
}


// Generate the list of PersonIDs whose permissions we will load for 
// this user (the user's ID, plus all the groups he's a member of)
CString GenerateUserGroupIDList(long nUserID)
{
	// Start out with at least the user's id
	CString strUserGroupIDList;
	strUserGroupIDList.Format("%li", nUserID);
	
	// Open the recordset of all groups this user is in
	_RecordsetPtr prs = CreateRecordset(
		"SELECT GroupID FROM UserGroupDetailsT WHERE UserID = %li", nUserID);
	FieldsPtr pflds = prs->GetFields();
	FieldPtr fldGroupID = pflds->GetItem("GroupID");
	
	// Loop through the user groups
	while (!prs->eof) {
		// Add this group id to the list of PersonIDs
		strUserGroupIDList += ", " + AsString(AdoFldLong(fldGroupID));
		// Move to the next group of which this user is a member
		prs->MoveNext();
	}

	// Return the list
	return strUserGroupIDList;
}

void FillBlankObjectPermissions(CMapBuiltInIDToObjectPermissions *pUserPermissions)
{
	// It's probably empty right now, but we might as well be sure
	pUserPermissions->RemoveAll();

	for (long i=0; i<gc_nBuiltInObjectCount; i++) {
		// Get an easy reference to the built in object
		const CBuiltInObject &bio = gc_aryBuiltInObjects[i];

		// Create a new security object permissions object and set the built in permissions component of it
		CSecurityObjectPermissions *pObjectPerms = new CSecurityObjectPermissions;
		pObjectPerms->m_BuiltInPermissions = permissionsNone;

		for(long j=0; j<gc_aryUserDefinedObjects.GetSize(); j++) {
			const CBuiltInObject *bioUser = gc_aryUserDefinedObjects.GetAt(j);
			if(bioUser->m_eParentID == bio.m_eBuiltInID) {
				CPermissions *pNone = new CPermissions;
				pNone->nPermissions = permissionsNone.nPermissions;
				pObjectPerms->m_mapDetailedPermissions.SetAt(bioUser->m_eBuiltInID, pNone);
			}
		}

		// Then put the new security object permissions into this user's map of security objects
		pUserPermissions->SetAt(bio.m_eBuiltInID, pObjectPerms);
	}
}

static long l_nNextUserHandle = 0;

// (j.jones 2008-11-19 11:08) - PLID 28578 - added pbIsPasswordVerified as a parameter
HANDLE OpenUserHandle(const CString &strUsername, const CString &strPassword, IN long nLocationID, OPTIONAL OUT int *pnFailureReason /*= NULL*/, OPTIONAL OUT long *pnUserID /*= NULL*/, OPTIONAL OUT CString *pstrTruePassword /*= NULL*/, OPTIONAL OUT BOOL *pbIsPasswordVerified /*= NULL*/)
{
	// WE need user id
	long nUserID;
	// Authenticate the user, getting the user's id, and optionally the locationid
	// (j.jones 2008-11-19 11:08) - PLID 28578 - added pbIsPasswordVerified as a parameter
	int nAuthFailure = AuthenticateUser(strUsername, strPassword, nLocationID, &nUserID, pstrTruePassword, pbIsPasswordVerified);
	// Handle the result (0 indicates success, non-zero is failure of some kind)
	if (nAuthFailure != 0) {
		// User could not be authenticated
		if (pnFailureReason) {
			*pnFailureReason = nAuthFailure;
		}
		return NULL;
	}

	// Successfully authenticated
	if (pnUserID) {
		*pnUserID = nUserID;
	}

	// Generate what will be the user handle that we'll be returning in the end
	HANDLE hAns = (HANDLE)InterlockedIncrement(&l_nNextUserHandle);

	// Create the permissions map that we'll then go ahead and fill up with permissions
	CMapBuiltInIDToObjectPermissions *pUserPermissions = new CMapBuiltInIDToObjectPermissions;

	// Make sure there's a blank entry for each built-in permission
	FillBlankObjectPermissions(pUserPermissions);

	{
		//if administrator user, all permissions are enabled

		BOOL bIsAdministrator = FALSE;

		if(strUsername == BUILT_IN_USER_NEXTECH_TECHSUPPORT_USERNAME) {
			bIsAdministrator = TRUE;
		}
		else {
			// (j.jones 2010-01-27 09:18) - PLID 23982 - parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT Administrator FROM UsersT WHERE Administrator = 1 AND PersonID = {INT}", nUserID);
			bIsAdministrator = !rs->eof;
			rs->Close();
		}

		if(bIsAdministrator) {

			//DRT 5/19/2006 - PLID 20693 - We are now caching the fact that the user is an administrator.
			SetCurrentUserAdministratorStatus(TRUE);

			//get all the permissions and set them to be all enabled

			for(int i = 0; i < gc_nBuiltInObjectCount; i++) {

				const CBuiltInObject &bio = gc_aryBuiltInObjects[i];

				CSecurityObjectPermissions *pObjectPerms = NULL;

				long nObjectID = bio.m_eBuiltInID;

				if(pUserPermissions->Lookup((EBuiltInObjectIDs)nObjectID, pObjectPerms)) {

					pObjectPerms->m_BuiltInPermissions.nPermissions |= bio.m_AvailPermissions;
				}
				else {
					ASSERT(FALSE);
				}
			}

			for(i = 0; i < gc_aryUserDefinedObjects.GetSize(); i++) {

				const CBuiltInObject *bio = gc_aryUserDefinedObjects[i];

				CSecurityObjectPermissions *pObjectPerms = NULL;

				long nObjectID = bio->m_eParentID;

				if(pUserPermissions->Lookup((EBuiltInObjectIDs)nObjectID, pObjectPerms)) {

					CPermissions *pPerms = NULL;

					if(pObjectPerms->m_mapDetailedPermissions.Lookup(bio->m_eBuiltInID, pPerms)) {

						pPerms->nPermissions |= bio->m_AvailPermissions;
					}
					else {
						ASSERT(FALSE);
					}
				}
				else {
					ASSERT(FALSE);
				}
			}

		}
		else {
			//if not administrator, load from data
			// (a.walling 2007-05-04 09:58) - PLID 4850 - Need to set the admin status if switching from admin to non-admin
			SetCurrentUserAdministratorStatus(FALSE);

			// Loop through the recordset of user permissions, overwriting the defaults whenever we have permissions from the data

			// (j.jones 2010-01-27 09:18) - PLID 23982 - parameterized
			_RecordsetPtr prs = CreateParamRecordset(
				"SELECT ObjectID, BuiltInID AS ParentBuiltInID, ObjectValue, Permissions "
				"FROM PermissionT LEFT JOIN SecurityObjectT ON PermissionT.ObjectID = SecurityObjectT.ID "
				"INNER JOIN UsersT ON PermissionT.UserGroupID = UsersT.PersonID "
				"WHERE UserGroupID = {INT}", nUserID);

			FieldsPtr pflds = prs->GetFields();
			FieldPtr fldObjectID = pflds->GetItem("ObjectID");
			FieldPtr fldParentBuiltInID = pflds->GetItem("ParentBuiltInID");
			FieldPtr fldObjectValue = pflds->GetItem("ObjectValue");
			FieldPtr fldPermissions = pflds->GetItem("Permissions");

			if(prs->eof) {
				// this user has no permissions!
				if (pnFailureReason) {
					*pnFailureReason = 4;
				}
				return NULL;
			}

			// Do the loop
			while (!prs->eof) {
				
				// Get the object id from the recordset
				long nObjectID = AdoFldLong(fldObjectID);

				// Get the security object permissions object associated with this object id
				CSecurityObjectPermissions *pObjectPerms = NULL;
				BOOL bGotSecurityObject = FALSE;
				{
					if (nObjectID > 0) {
						// It's a user-defined security object
						bGotSecurityObject = pUserPermissions->Lookup((EBuiltInObjectIDs)AdoFldLong(fldParentBuiltInID), pObjectPerms);
					} else {
						// It's a built-in security object
						bGotSecurityObject = pUserPermissions->Lookup((EBuiltInObjectIDs)nObjectID, pObjectPerms);
					}
				}

				if (bGotSecurityObject) {	
					// Good, we got the permissions object as we expected

					// Get the object value from the recordset (this is the "sub-object", or "user-defined object")
					_variant_t varObjectValue = fldObjectValue->GetValue();

					// We expect that if it's a user-defined then we DO have an ObjectValue, and if it's built-in object then we DON'T (it shouldn't be possible to have an nObjectID of 0)
					ASSERT(((nObjectID > 0) && (varObjectValue.vt != VT_NULL)) || ((nObjectID < 0) && (varObjectValue.vt == VT_NULL)));

					if (varObjectValue.vt == VT_NULL) {
						// The object value is null, which means there is no "sub-object", 
						// this is the permissions record for the built-in object itself
						pObjectPerms->m_BuiltInPermissions.nPermissions |= AdoFldLong(fldPermissions);
					} else {
						// The object value is not null, which means these are the permissions for a user-defined object
						CPermissions *pPerms = NULL;

						if(pObjectPerms->m_mapDetailedPermissions.Lookup(nObjectID, pPerms)) {
							pPerms->nPermissions |= AdoFldLong(fldPermissions);
						}
						else {
							ASSERT(FALSE);
						}
					}
				} else {
					// It should be impossible for a built-in permission to exist in the database without it also existing in the map
					ASSERT(FALSE);
				}

				// Move to the next permission record for this user
				prs->MoveNext();
			}
		}
	}

	// We're all done, our permissions object is completely set up, now just put it in our user map and jobdone
	m_mapUserPermissionCache.SetAt(hAns, pUserPermissions);
	return hAns;
}

void CloseUserHandle(HANDLE hUser)
{
	CMapBuiltInIDToObjectPermissions *pmapBuiltIns = NULL;
	if (m_mapUserPermissionCache.Lookup(hUser, pmapBuiltIns)) {
		// It would have had to be set to a non-null pointer in the first place
		ASSERT(pmapBuiltIns);

		// Remove the mapping
		m_mapUserPermissionCache.RemoveKey(hUser);

		// Delete the object
		delete pmapBuiltIns;
	} else {
		// Invalid handle
		ASSERT(FALSE);
	}

	// (a.walling 2007-05-07 10:17) - PLID 4850 - Set the current user handle if needed
	if (GetCurrentUserHandle() == hUser) {
		extern void SetCurrentUserHandle(HANDLE hUserHandle);
		SetCurrentUserHandle(NULL);
	}
}


HANDLE LogInUser(LPCTSTR strUsername, LPCTSTR strPassword, long nLocationID, long nInactivityMinutes /*= -1*/, LPCTSTR strLocationName /*= NULL*/, int *pnFailureReason /*= NULL*/)
{
	// Get the old handle
	HANDLE hOldUser = GetCurrentUserHandle();

	// Open a new handle to this same user
	long nUserID;
	int nFailure;
	CString strTruePass;
	BOOL bIsPasswordVerified = FALSE;
	// (j.jones 2008-11-19 11:08) - PLID 28578 - added bIsPasswordVerified as a parameter to OpenUserHandle
	HANDLE hUser = OpenUserHandle(strUsername, strPassword, nLocationID, &nFailure, &nUserID, &strTruePass, &bIsPasswordVerified);
	if (hUser) {
		// We successfully opened the handle, so set the user info
		// (j.jones 2008-11-19 10:42) - PLID 28578 - added bIsPasswordVerified as a parameter to SetCurrentUser
		if (strLocationName) {
			SetCurrentUser(hUser, nUserID, strUsername, strTruePass, bIsPasswordVerified, nLocationID, nInactivityMinutes, strLocationName);
		} else {
			SetCurrentUser(hUser, nUserID, strUsername, strTruePass, bIsPasswordVerified, nLocationID, nInactivityMinutes);
		}
		
		// And then close the old handle
		if (hOldUser) {
			CloseUserHandle(hOldUser);
		}		


		// Return the new handle
		if (pnFailureReason) *pnFailureReason = 0;
		return hUser;
	} else {
		// Failure, return NULL
		if (pnFailureReason) *pnFailureReason = nFailure;
		return NULL;
	}
}

// (j.jones 2010-01-14 16:08) - PLID 36887 - added support for dynamic names
void AddUserDefinedPermission(long ObjectID, long AvailablePermissions, CString DisplayName, CString Description, long ParentID, long nCurrentPermissions,
							  CString strDynamicName0 /*= ""*/, CString strDynamicName1 /*= ""*/, CString strDynamicName2 /*= ""*/,
							  CString strDynamicName3 /*= ""*/, CString strDynamicName4 /*= ""*/)
{
	// (z.manning 2011-05-19 10:34) - PLID 43767 - Moved the logic of this function into 2 different functions
	long nNewSecurityObjectID = AddUserDefinedPermissionToData(ObjectID, AvailablePermissions, DisplayName, Description, ParentID, nCurrentPermissions, strDynamicName0, strDynamicName1, strDynamicName2, strDynamicName3, strDynamicName4);
	AddUserDefinedPermissionToMemory(nNewSecurityObjectID, ObjectID, AvailablePermissions, DisplayName, Description, ParentID, nCurrentPermissions, strDynamicName0, strDynamicName1, strDynamicName2, strDynamicName3, strDynamicName4);
}

// (z.manning 2011-05-19 08:43) - PID 43767 - I split up the logic of AddUserDefinedPermission into 2 different functions
// Returns the ID of the newly created security object
long AddUserDefinedPermissionToData(long ObjectID, long AvailablePermissions, CString DisplayName, CString Description, long ParentID, long nCurrentPermissions,
							  CString strDynamicName0 /*= ""*/, CString strDynamicName1 /*= ""*/, CString strDynamicName2 /*= ""*/,
							  CString strDynamicName3 /*= ""*/, CString strDynamicName4 /*= ""*/)
{
	// (c.haag 2006-05-22 13:56) - PLID 20609 - THIS FUNCTION MUST NOT BE CALLED AS A RESULT
	// OF GETTING A TABLE CHECKER

	//JMJ - 6/5/03 - In an ideal situation, only insert statements would be called in this function. However,
	//since SecurityObjectT does not have a relationship with your pseudo-linked table
	//(such as, SecurityObjectT.ObjectValue does not have a relationship with ResourceT.ID, or ProvidersT.PersonID),
	//I chose to alter this code to safely account for the potential for bad data, and repair as needed.
	//Note: I never actually got this situation to happen through Practice, but it COULD happen if a developer
	//ever used NxQuery on someone's data to manually remove a Resource, Provider, anything that is permissioned.

	//check to make sure the SecurityObject we are about to create doesn't already exist
	if(ReturnsRecordsParam("SELECT ID FROM SecurityObjectT WHERE BuiltInID = {INT} AND ObjectValue = {INT}",ParentID,ObjectID)) {
		//Okay, this record exists, but we are adding new so therefore this record is outdated and invalid, and would not exist
		//if we had a relationship between SecurityObjectT and the table we just created a record in.
		//This will clear out data and the current user's array.
		DeleteUserDefinedPermission(ParentID, ObjectID, FALSE);
	}
	
	// (z.manning 2013-10-18 11:13) - PLID 59082 - Call the shared function
	long nSecurityObjectID = AddUserDefinedPermissionToData(GetRemoteData(), ObjectID, AvailablePermissions, DisplayName, Description
		, ParentID, nCurrentPermissions, FALSE, strDynamicName0, strDynamicName1, strDynamicName2, strDynamicName3, strDynamicName4);
	return nSecurityObjectID;
}

// (z.manning 2011-05-19 08:43) - PID 43767 - I split up the logic of AddUserDefinedPermission into 2 different functions
void AddUserDefinedPermissionToMemory(long nNewSecurityObjectID, long ObjectID, long AvailablePermissions, CString DisplayName, CString Description, long ParentID, long nCurrentPermissions,
							  CString strDynamicName0 /*= ""*/, CString strDynamicName1 /*= ""*/, CString strDynamicName2 /*= ""*/,
							  CString strDynamicName3 /*= ""*/, CString strDynamicName4 /*= ""*/)
{
	//now add to the global array
	//TODO: This only adds to the current user's global array. Ponder this.
	
	// (j.jones 2010-01-14 16:08) - PLID 36887 - added support for dynamic values
	CString strDynamicNames[5];
	char* szDynamicNames = NULL;
	if(!strDynamicName0.IsEmpty() || !strDynamicName1.IsEmpty() || !strDynamicName2.IsEmpty()
		|| !strDynamicName3.IsEmpty() || !strDynamicName4.IsEmpty()) {

		//initialize the list
		int i=0;
		for(i=0; i < 5; i++) {
			strDynamicNames[i] = "";
		}

		if(!strDynamicName0.IsEmpty()) {
			strDynamicNames[0] = strDynamicName0;
		}
		if(!strDynamicName1.IsEmpty()) {
			strDynamicNames[1] = strDynamicName1;
		}
		if(!strDynamicName2.IsEmpty()) {
			strDynamicNames[2] = strDynamicName3;
		}
		if(!strDynamicName3.IsEmpty()) {
			strDynamicNames[3] = strDynamicName3;
		}
		if(!strDynamicName4.IsEmpty()) {
			strDynamicNames[4] = strDynamicName4;
		}

		char* offset;
		long nSize = 0;
		for(i=0; i < 5; i++) {
			nSize += strDynamicNames[i].GetLength() + 1;
		}
		offset = szDynamicNames = new char[nSize];
		for(i=0; i < 5; i++)
		{
			strcpy(offset, strDynamicNames[i]);
			offset += strDynamicNames[i].GetLength() + 1;
		}
	}

	CBuiltInObject *pObj = new CBuiltInObject((EBuiltInObjectIDs)nNewSecurityObjectID, AvailablePermissions, DisplayName, Description, szDynamicNames, (EBuiltInObjectIDs)ParentID, ObjectID);
	gc_aryUserDefinedObjects.Add(pObj);

	if(szDynamicNames) {
		delete szDynamicNames;
	}

	//re-login to update your permissions
	//TODO: There is code elsewhere to handle when another user attempts to access this new permission,
	//however we should consider a way to automatically have them re-log in. This should not happen though until
	//we find a way to auto-update their global array first.
	LogInUser(GetCurrentUserName(), GetCurrentUserPassword(), GetCurrentLocationID());
	GetMainFrame()->UpdateToolBarButtons(TRUE);
	// (c.haag 2006-05-22 13:56) - PLID 20609 - Make sure other users update their cache
	CClient::RefreshTable(NetUtils::UserDefinedSecurityObject, nNewSecurityObjectID);
}

void UpdateUserDefinedPermissionName(long BuiltInID, long ObjectID, CString newDisplayName)
{
	// (z.manning 2011-05-19 11:00) - PLID 43767 - Split this logic up into 2 functions
	long nSecurityObjectID = UpdateUserDefinedPermissionNameInData(BuiltInID, ObjectID, newDisplayName);
	UpdateUserDefinedPermissionNameInMemory(nSecurityObjectID, newDisplayName);
}

// (z.manning 2011-05-19 08:43) - PID 43767 - I split up the logic of UpdateUserDefinedPermissionName into 2 different functions
long UpdateUserDefinedPermissionNameInData(long BuiltInID, long ObjectID, CString newDisplayName)
{
	// (c.haag 2006-05-22 13:56) - PLID 20609 - THIS FUNCTION MUST NOT BE CALLED AS A RESULT
	// OF GETTING A TABLE CHECKER

	long nSecurityObjectID = 0;

	_RecordsetPtr rs = CreateParamRecordset("SELECT ID FROM SecurityObjectT WHERE BuiltInID = {INT} AND ObjectValue = {INT}"
		, BuiltInID, ObjectID);
	if(!rs->eof) {
		nSecurityObjectID = AdoFldLong(rs, "ID");
	}
	rs->Close();

	ExecuteParamSql("UPDATE SecurityObjectT SET DisplayName = {STRING} WHERE ID = {INT}", newDisplayName, nSecurityObjectID);

	return nSecurityObjectID;
}

// (z.manning 2011-05-19 08:43) - PID 43767 - I split up the logic of UpdateUserDefinedPermissionName into 2 different functions
void UpdateUserDefinedPermissionNameInMemory(long nSecurityObjectID, CString newDisplayName)
{
	for(int i=0; i<gc_aryUserDefinedObjects.GetSize();i++) {
		if(gc_aryUserDefinedObjects.GetAt(i)->m_eBuiltInID == nSecurityObjectID) {
			gc_aryUserDefinedObjects.GetAt(i)->m_strDisplayName = newDisplayName;
		}
	}

	//re-login to update your permissions
	LogInUser(GetCurrentUserName(), GetCurrentUserPassword(), GetCurrentLocationID());
	GetMainFrame()->UpdateToolBarButtons(TRUE);

	// (c.haag 2006-05-22 13:56) - PLID 20609 - Make sure other users update their cache
	CClient::RefreshTable(NetUtils::UserDefinedSecurityObject, nSecurityObjectID);
}

void DeleteUserDefinedPermission(long BuiltInID, long ObjectID, BOOL bLogInUser /*= TRUE*/)
{
	// (c.haag 2006-05-22 13:56) - PLID 20609 - THIS FUNCTION MUST NOT BE CALLED AS A RESULT
	// OF GETTING A TABLE CHECKER

	// (z.manning 2011-05-19 10:39) - PLID 43767 - Moved the logic of this function into 2 separate functions
	long nSecurityObjectID = DeleteUserDefinedPermissionFromData(BuiltInID, ObjectID);
	DeleteUserDefinedPermissionFromMemory(nSecurityObjectID, bLogInUser);
}

// (z.manning 2011-05-19 08:43) - PID 43767 - I split up the logic of DeleteUserDefinedPermission into 2 different functions
// Returns the security object ID of the object being deleted
long DeleteUserDefinedPermissionFromData(long BuiltInID, long ObjectID)
{
	// (z.manning 2013-10-17 17:03) - PLID 59082 - Call the shared function
	return DeleteUserDefinedPermissionFromData(GetRemoteData(), BuiltInID, ObjectID);
}

// (z.manning 2011-05-19 08:43) - PID 43767 - I split up the logic of DeleteUserDefinedPermission into 2 different functions
void DeleteUserDefinedPermissionFromMemory(long nSecurityObjectID, BOOL bLogInUser /*= TRUE*/)
{
	for(int i=0; i<gc_aryUserDefinedObjects.GetSize();i++) {
		if(gc_aryUserDefinedObjects.GetAt(i)->m_eBuiltInID == nSecurityObjectID) {
			delete gc_aryUserDefinedObjects.GetAt(i);
			gc_aryUserDefinedObjects.RemoveAt(i);
			break;
		}
	}

	if(bLogInUser) {
		//re-login to update your permissions
		LogInUser(GetCurrentUserName(), GetCurrentUserPassword(), GetCurrentLocationID());
		GetMainFrame()->UpdateToolBarButtons(TRUE);
	}
	// (c.haag 2006-05-22 13:56) - PLID 20609 - Make sure other users update their cache
	CClient::RefreshTable(NetUtils::UserDefinedSecurityObject, nSecurityObjectID);
}

void LoadUserDefinedPermissions(long nSecurityObjectID)
{
	try {
		// (c.haag 2006-05-22 13:56) - PLID 20609 - If we get here, it means that we got a
		// SecurityObjectT table checker message. We need to make sure that gc_aryUserDefinedObjects
		// is up to date.
		if (-1 == nSecurityObjectID) {
			// A value of -1 means we don't know what SecurityObjectT record(s) updated, so just
			// update them all
			ClearUserDefinedPermissionList();
			LoadUserDefinedPermissions();
		} else {
			// Update just one SecurityObjectT record
			CString strWhere;
			strWhere.Format("ID = %d", nSecurityObjectID);
			LoadUserDefinedPermissions(strWhere);
		}
	}
	NxCatchAll("Error updating user-defined permissions");
}

void LoadUserDefinedPermissions(CString strWhereClause)
{
	try {
		_RecordsetPtr rs = CreateRecordset("SELECT ID, BuiltInID, ObjectValue, DisplayName, Description, AvailablePermissions, DynamicName0, DynamicName1, DynamicName2, DynamicName3, DynamicName4 "
			"FROM SecurityObjectT WHERE (%s)", strWhereClause);
		while (!rs->eof) {
			LoadSingleUserDefinedPermission(rs);
			rs->MoveNext();
		}
	}
	NxCatchAll("Error updating user-defined permissions");
}


long GetDetailedPermissionValue(IN EBuiltInObjectIDs eBuiltInID, IN long nObjectValue) {
	//
	// (c.haag 2006-05-09 13:49) - PLID 20502 - Look in our cache first
	//
	for (int i=0; i < gc_aryUserDefinedObjects.GetSize(); i++) {
		if (gc_aryUserDefinedObjects[i]->m_eParentID == eBuiltInID &&
			gc_aryUserDefinedObjects[i]->m_nObjectValue == nObjectValue)
		{
			return gc_aryUserDefinedObjects[i]->m_eBuiltInID;
		}
	}
	//
	// (c.haag 2006-05-09 13:50) - This is a failsafe for the event the value is not cached
	//
	_RecordsetPtr rs = CreateRecordset("SELECT ID FROM SecurityObjectT WHERE ObjectValue = %li AND BuiltInID = %li",nObjectValue, eBuiltInID);
	if(!rs->eof) {
		return AdoFldLong(rs, "ID",0);
	}
	rs->Close();

	return 0;
}

void GetPermissions(IN HANDLE hUser, IN EBuiltInObjectIDs eBuiltInID, IN BOOL bUseObjectValue, IN long nObjectValue, OUT CPermissions &permsOut)
{
	CMapBuiltInIDToObjectPermissions *pmapBuiltIns = NULL;
	if (m_mapUserPermissionCache.Lookup(hUser, pmapBuiltIns)) {
		// Got the built-in permission map, use it to get the full security object permissions for the specified built-in permission
		CSecurityObjectPermissions *psop = NULL;
		if (pmapBuiltIns->Lookup(eBuiltInID, psop)) {
			// Got the full security object permissions
			if (bUseObjectValue) {
				// A more detailed permission is being requested, see if it exists for this user
				//if (psop->m_mapDetailedPermissions) {
				CPermissions *pPerms = NULL;
				if (psop->m_mapDetailedPermissions.Lookup(GetDetailedPermissionValue(eBuiltInID,nObjectValue), pPerms)) {
					// Found the more detailed entry
					permsOut.nPermissions = pPerms->nPermissions;
				} else {
					//JJ - 6/5/03 - This should only happen in one case - where a user-defined item was added on
					//another machine and the current user doesn't have it in their map yet, and then tried to
					//access this newly permissioned item. In this case, we will have to load it from data.
					//This should not be a huge performance hit as it should only happen in rare cases,
					//and only until they restart Practice.

					//TODO: Consider adding this to the user's permission map, and to the global user defined permission array,
					//so that this case would only happen one time

					if (IsCurrentUserAdministrator()) {
						//we need to give admins all available permissions for this object,
						//but that means we need to query to find out what they are
						_RecordsetPtr rs = CreateParamRecordset("SELECT AvailablePermissions FROM SecurityObjectT WHERE BuiltInID = {INT} AND ObjectValue = {INT}", (long)eBuiltInID, nObjectValue);
						if (!rs->eof) {
							permsOut = AdoFldLong(rs, "AvailablePermissions", psop->m_BuiltInPermissions);
						}
						else {
							// A more detailed entry WAS NOT found, so it should be inherited from its parent, the built-in one
							permsOut = psop->m_BuiltInPermissions;
						}
						rs->Close();
					}
					else {
						_RecordsetPtr rs = CreateParamRecordset("SELECT Permissions FROM PermissionT INNER JOIN SecurityObjectT ON PermissionT.ObjectID = SecurityObjectT.ID WHERE BuiltInID = {INT} AND ObjectValue = {INT} AND UserGroupID = {INT}", (long)eBuiltInID, nObjectValue, GetCurrentUserID());
						if (!rs->eof) {
							permsOut = AdoFldLong(rs, "Permissions", psop->m_BuiltInPermissions);
						}
						else {
							// A more detailed entry WAS NOT found, so it should be inherited from its parent, the built-in one
							permsOut = psop->m_BuiltInPermissions;
						}
						rs->Close();
					}
				}
			} else {
				// A more detailed permission IS NOT being requested, so just use the parent
				permsOut = psop->m_BuiltInPermissions;
			}
		} else {
			// Should be impossible to get here because by simply logging in, a user gets all built-in permissions cached
			ASSERT(FALSE);
			permsOut = permissionsNone;
		}
	} else {
		// Should be impossible with a valid user handle because by logging in, all the built-in permissions should get mapped
		ASSERT(FALSE);
		permsOut = permissionsNone;
	}
}

// Given a user handle, a reference to the built-in object, and the value of the user-defined 
// object if there is one, returns a CPermissions struct that contains the various permissions 
// the user has for that object
// NOTE: nObjectValue is ignored iff bUseObjectValue is FALSE
CPermissions GetPermissions(HANDLE hUser, EBuiltInObjectIDs eBuiltInID, BOOL bUseObjectValue /*= FALSE*/, long nObjectValue /*= 0*/)
{
	CPermissions perms;
	GetPermissions(hUser, eBuiltInID, bUseObjectValue, nObjectValue, perms);
	return perms;
}

// (z.manning 2009-06-10 14:43) - PLID 34585 - Check to see if given user has any permissions
bool DoesUserHaveAnyPermissions(HANDLE hUser)
{
	// (z.manning 2009-06-10 15:25) - PLID 34585 - Find the permission object map for the given user.
	CMapBuiltInIDToObjectPermissions *pmapPermissionObjects = NULL;
	if(m_mapUserPermissionCache.Lookup(hUser, pmapPermissionObjects))
	{
		// (z.manning 2009-06-10 15:25) - PLID 34585 - Now iterate through all permission objects.
		POSITION posMapPermObjects = pmapPermissionObjects->GetStartPosition();
		while(posMapPermObjects != NULL)
		{
			EBuiltInObjectIDs eObjectID;
			CSecurityObjectPermissions *pPermObject;
			pmapPermissionObjects->GetNextAssoc(posMapPermObjects, eObjectID, pPermObject);

			// (z.manning 2009-06-10 15:25) - PLID 34585 - If the built in permission for this object
			// are non-zero then the user has a permission.
			if(pPermObject->m_BuiltInPermissions.nPermissions != 0) {
				return true;
			}

			// (z.manning 2009-06-10 15:25) - PLID 34585 - Also check the detailed permissions
			POSITION posMapPermDetails = pPermObject->m_mapDetailedPermissions.GetStartPosition();
			while(posMapPermDetails != NULL)
			{
				long nObjectValue;
				CPermissions *pPermissions;
				pPermObject->m_mapDetailedPermissions.GetNextAssoc(posMapPermDetails, nObjectValue, pPermissions);

				if(pPermissions->nPermissions != 0) {
					return true;
				}
			}
		}
	}

	return false;
}

void LoadSingleUserDefinedPermission(_RecordsetPtr& rs)
{
	long ID = AdoFldLong(rs, "ID");
	long BuiltInID = AdoFldLong(rs, "BuiltInID");
	long ObjectValue = AdoFldLong(rs, "ObjectValue");
	CString DisplayName = AdoFldString(rs, "DisplayName","");
	CString Description = AdoFldString(rs, "Description","");
	long AvailablePermissions = AdoFldLong(rs, "AvailablePermissions");
	CString strDynamicNames[5];
	char* szDynamicNames = NULL;
	BOOL bBuildDynamicNameString = FALSE;

	// Remove the value from memory
	for(int i=0; i<gc_aryUserDefinedObjects.GetSize();i++) {
		if(gc_aryUserDefinedObjects.GetAt(i)->m_eBuiltInID == ID) {
			delete gc_aryUserDefinedObjects.GetAt(i);
			gc_aryUserDefinedObjects.RemoveAt(i);
			break;
		}
	}

	// (c.haag 2003-08-01 10:13) - Build the list of dynamic security object names in case we have
	// dynamic permissions
	for (i=0; i < 5; i++)
	{
		CString strField;
		strField.Format("DynamicName%d", i);
		strDynamicNames[i] = VarString(rs->Fields->Item[(LPCTSTR)strField]->Value, "");
		if (strDynamicNames[i].GetLength())
			bBuildDynamicNameString = TRUE;
	}
	if (bBuildDynamicNameString)
	{
		char* offset;
		long nSize = 0;
		for (i=0; i < 5; i++)
			nSize += strDynamicNames[i].GetLength() + 1;
		offset = szDynamicNames = new char[nSize];
		for (i=0; i < 5; i++)
		{
			strcpy(offset, strDynamicNames[i]);
			offset += strDynamicNames[i].GetLength() + 1;
		}
	}

	CBuiltInObject *pObj = new CBuiltInObject((EBuiltInObjectIDs)ID, AvailablePermissions, DisplayName, Description, szDynamicNames, (EBuiltInObjectIDs)BuiltInID, ObjectValue);
	gc_aryUserDefinedObjects.Add(pObj);

	if (szDynamicNames) delete szDynamicNames;
}

void LoadUserDefinedPermissions() {

	try {
		//the BuiltInObject list is hard coded and thereore automatically loaded,
		//but the UserDefinedObject list loads from data, so we must load it at the beginning of the program!
		
		_RecordsetPtr rs = CreateRecordset("SELECT ID, BuiltInID, ObjectValue, DisplayName, Description, AvailablePermissions, DynamicName0, DynamicName1, DynamicName2, DynamicName3, DynamicName4 "
		"FROM SecurityObjectT");
		while(!rs->eof) {		
			LoadSingleUserDefinedPermission(rs);
			rs->MoveNext();
		}
		rs->Close();

	}NxCatchAll("Error loading user-defined permissions.");

}

//Helper functions for EnsureDefaultPermissions - do not call these anywhere else
bool EnsureSinglePermission(const CBuiltInObject bio, CMap<EBuiltInObjectIDs, EBuiltInObjectIDs, unsigned long, unsigned long> *pmap);

//helper function to remove all objects which exist in the database PermissionStateT table, but do not exist
//as objects in code any longer
void EnsureRemovedObjectsInData(CMap<EBuiltInObjectIDs, EBuiltInObjectIDs, unsigned long, unsigned long> *mapPerms);

void EnsureDefaultPermissions() {

	//DRT 3/25/03 - More conversations with Bob regarding the permissions setup.  Here's some more info:
	//		We've missed a vital part all along.  Setting the defaults in the manner we have been trying
	//		to set them does not handle templates (or, if we add them in the future, groups).  So here's
	//		our proposed system now:
	//		We have saved a list of all objects and their available permissions to the data in 
	//		PermissionStateT.  We loop through all records in that table and put them in a map (for
	//		efficiency, only open 1 recordset).  Afterwards, we loop through every object ID and compare
	//		it with the available permissions in the map.  If they are the same, nothing happens.  If they
	//		are different, we update PermissionStateT with the newer value, and call SetDefaultPermissions
	//		with a mask of the fields that are different.  SetDefaultPermissions handles writing the default
	//		set to the database for all users.    If the item does not even exist in the map, it is added
	//		to the PermissionStateT table, and again SetDefaultPermissions is called to handle all user
	//		based permission setting.

	CMap<EBuiltInObjectIDs, EBuiltInObjectIDs, unsigned long, unsigned long> mapPerms;

	try {
		_RecordsetPtr prs = CreateRecordset("SELECT ObjectID, AvailablePerms FROM PermissionStateT");

		while(!prs->eof) {
			long nObjID = AdoFldLong(prs, "ObjectID");
			unsigned long nAvailPerms = AdoFldLong(prs, "AvailablePerms");

			//load the id and the available into the map
			mapPerms.SetAt((EBuiltInObjectIDs)nObjID, nAvailPerms);
			prs->MoveNext();
		}
		prs->Close();
	} NxCatchAll("Error determining permissions.");

	//There is a very remote possibility that we added an object at one point, it got into someone's data, 
	//and then it was removed from our array of CBuiltInObjects.  In that case, we have to remove it from 
	//the data altogether.
	EnsureRemovedObjectsInData(&mapPerms);

	CArray<EBuiltInObjectIDs, EBuiltInObjectIDs> aryMissed;

	// Now that we've got the item out of the database, loop through all our permissions and compare.  If it's 
	//		different, we need to add the default (call GetDefaultPermission()) for all users/templates/etc.
	for(int i = 0; i < gc_nBuiltInObjectCount; i++) {

		const CBuiltInObject &bio = gc_aryBuiltInObjects[i];

		//if this object has different permissions from the map (data), add it to our array to look at later
		unsigned long nPerms = 0;
		BOOL bFound = mapPerms.Lookup(bio.m_eBuiltInID, nPerms);
		if(bFound) {
			if(bio.m_AvailPermissions.nPermissions != nPerms) {
				//different permissions than expected
				aryMissed.Add(bio.m_eBuiltInID);
			}
		}
		else {
			//not even found, add it too
			aryMissed.Add(bio.m_eBuiltInID);
		}
	}

	//all finished looping, now 1 last time we go around and try to run the ones which failed,
	//just in case the thing they were waiting on is now done
	bool bOnePassed = true;
	unsigned long nPerm = 0;
	while(bOnePassed) {
		bOnePassed = false;

		for(int i = 0; i < aryMissed.GetSize(); i++) {
			const CBuiltInObject *bioMissed = GetBuiltInObject(aryMissed.GetAt(i));

			//if this returns false, we failed to write anything
			//if this returns true, something was written (though it may not be everything)
			bool bEnsured = EnsureSinglePermission(*bioMissed, &mapPerms);

			if(bEnsured) {
				//this time around it succeeded in writing something
				unsigned long nPermsInMap = 0;
				BOOL bFound = mapPerms.Lookup(bioMissed->m_eBuiltInID, nPermsInMap);
				if(bFound) {
					if(bioMissed->m_AvailPermissions.nPermissions == nPermsInMap) {
						//data matches what code says it should be

						//remove this item from our array, it has been updated correctly + fully
						aryMissed.RemoveAt(i);
						i--;

						//one succeeded, so we'll need to loop again - another item may have relied on this one
						bOnePassed = true;
					}
					else {
						//something was written to the data, but it wasn't everything.  Mark this as a success, but
						//leave it in the array.  The next pass through may use this success to finish another.
						bOnePassed = true;
					}
				}
				else {
					//not found in the map, but returned true?  This really shouldn't be possible
					ASSERT(false);
				}
			}
			else {
				//still failed, maybe next time
			}
		}
	}

	//at this point, if aryMissed contains anything, throw an exception about unhandled permissions
	if(aryMissed.GetSize() > 0) {
		AfxThrowNxException("There are %li unhandled default permissions.", aryMissed.GetSize());
	}
}

//DRT 4/1/03 - helper function to remove all objects which exist in the database PermissionStateT table, but do not exist
//as objects in code any longer
void EnsureRemovedObjectsInData(CMap<EBuiltInObjectIDs, EBuiltInObjectIDs, unsigned long, unsigned long> *mapPerms) {

	//loop through the map
	POSITION pos = mapPerms->GetStartPosition();
	while(pos) {
		EBuiltInObjectIDs nObj;
		unsigned long nPerms = 0;
		mapPerms->GetNextAssoc(pos, nObj, nPerms);
		bool bSuccess = false;

		//loop through the CBuiltInObject array and make sure it exists somewhere
		for (long i=0; i<gc_nBuiltInObjectCount && !bSuccess; i++) {
			if(gc_aryBuiltInObjects[i].m_eBuiltInID == nObj) {
				//success
				bSuccess = true;
			}
		}

		if(!bSuccess) {
			//this item was never found - remove it from the database and any permissions for it
			BEGIN_TRANS("EnsureDataObjects") {
				CString strExecute;
				strExecute.Format("DELETE FROM PermissionStateT WHERE ObjectID = %li;  DELETE FROM PermissionT WHERE ObjectID = %li;", nObj, nObj);
				ExecuteSqlStd(strExecute);
				bSuccess = true;
			} END_TRANS_CATCH_ALL("EnsureDataObjects");
		}

		if(!bSuccess) {
			//we've still failed, something probably failed in our transaction
			AfxThrowNxException("Unable to remove unused builtin object.");
		}
	}
}

//DRT 3/28/03 - Helper function for EnsureDefaultPermissions - handles looking up the item in the map, 
//		comparing it's old permissions, and calling the proper functions to insert anything into the data.
//bio:			the built in object we are ensuring
//pmap:			a pointer to the map which contains the current state of the database in regards
//				to the known permissions for each object
//Return value:	True if anything was written.  This may be either a partially written permission or a fully
//				written one.  False if nothing at all was written.
bool EnsureSinglePermission(const CBuiltInObject bio, CMap<EBuiltInObjectIDs, EBuiltInObjectIDs, unsigned long, unsigned long> *pmap) {

	bool bSuccess = false;
	unsigned long nPerm = 0;

	//see if it exists in the map
	BOOL bFound = pmap->Lookup(bio.m_eBuiltInID, nPerm);

	if(!bFound) {
		//item does not exist in the map at all

		// if it does not exist in the map, then it must be a brand new permissions object.  Look up the default value
		//per user, and save that to the data.

		BEGIN_TRANS("SetDefaultPermissions") {
			unsigned long nAdded = (bio.m_AvailPermissions.nPermissions ^ nPerm) & bio.m_AvailPermissions.nPermissions;

			//get the permissions that successfully set
			unsigned long nSet = SetDefaultPermission(bio.m_eBuiltInID, true, nAdded);

			//and then add into our table with their permissions
			//DRT 6/17/03 - Added " || bio.m_AvailPermissions.nPermissions == 0"
			//		This is to handle a special case where we are putting in 
			//		'placeholder' bios, generally used to be headers to better
			//		group things.  So, in the case that our available permissions
			//		are 0 (remember, this section of code is new items only), then
			//		we want to insert the available permissions (0) into the table.
			if(nSet || bio.m_AvailPermissions.nPermissions == 0) {
				//something was set successfully - this code is only executed if it does not exist in the map (data) - so always
				//an insert
				ExecuteSql("INSERT INTO PermissionStateT (ObjectID, AvailablePerms) values (%li, %li)", (long)bio.m_eBuiltInID, nSet);

				//update the map to be the same as the data
				pmap->SetAt(bio.m_eBuiltInID, nSet);

				bSuccess = true;	//success on something
			}
			else {
				//there was a failure - generally this means the permissions we compare against are 
				//not yet setup.  perhaps they are added later in this loop.  Return false to save
				//the ID, we'll try again shortly
				bSuccess = false;
			}
		} END_TRANS_CATCH_ALL("SetDefaultPermissions");
	}
	else {
		//item is in the map, compare the permissions
		if(bio.m_AvailPermissions.nPermissions != nPerm) {
			//permissions do not match - they had the object previously, but they do not have the appropriate permissions for it

			// it exists in the map, but it's got different available permissions.  That means something new has been added
			//		to this object.

			//xor the 2 together, then and that with the current permission - this gives us a mask of all
			//new permissions (AND the other for removed permissions)

			unsigned long nRemoved = (bio.m_AvailPermissions.nPermissions ^ nPerm) & nPerm;
			unsigned long nAdded = (bio.m_AvailPermissions.nPermissions ^ nPerm) & bio.m_AvailPermissions.nPermissions;

			if (nRemoved != 0){
				unsigned long nNewPerms = 0;
				nNewPerms = (nPerm & (~nRemoved));

				BEGIN_TRANS("RemoveDefaultPermissions") {
					//this can only succeed or fail, there is no partial
					bSuccess = RemoveUserPermission(bio.m_eBuiltInID, nRemoved);

					//update the map appropriately
					if(bSuccess) {
						bSuccess = false;	//for return value in case this execute fails and we rollback
						ExecuteSql("UPDATE PermissionStateT SET AvailablePerms = %li WHERE ObjectID = %li", nNewPerms, bio.m_eBuiltInID);
						pmap->SetAt(bio.m_eBuiltInID, nNewPerms);
						bSuccess = true;	//return value
					}
				} END_TRANS_CATCH_ALL("RemoveDefaultPermissions");
			}

			if (nAdded == 0 && nRemoved == 0) {
				AfxThrowNxException("Cannot update default permissions because there are no permissions available to add or remove.");
			}

			if(nAdded != 0) {
				//only need to execute this if something was added - we might have gotten here by a straight removal

				//DRT 3/27/03 - BeginTrans() before the SetDefault.  The set default will return 1 of 3
				//		ways.  1)  A non-zero bitmask.  This means something was written to the data.  2) 0.
				//		This means nothing was written.  Some criteria is missing (such as one 
				//		of the permissions to be compared against doesn't exist in the database), so
				//		we didn't even try writing.  3)  An exception.  In this case we wrote some amount
				//		of data, but an error condition arose, and we need to undo that and prompt the user
				//		with that exception.

				BEGIN_TRANS("SetDefaultPermissions") {
					unsigned long nSet = SetDefaultPermission(bio.m_eBuiltInID, false, nAdded);

					//update our State table only if we succeeded
					if(nSet) {
						//something succeeded - nPerm is what we had before, nSet is what we have now in addition
						ExecuteSql("UPDATE PermissionStateT SET AvailablePerms = %li WHERE ObjectID = %li", (nPerm | nSet), (long)bio.m_eBuiltInID);

						//update the map to match data
						pmap->SetAt(bio.m_eBuiltInID, (nPerm | nSet));

						bSuccess = true;	//success on something
					}
					else {
						//there was a failure - generally this means the permissions we compare against are 
						//not yet setup.  perhaps they are added later in this loop.  Return false to save
						//the ID, we'll try again shortly
						bSuccess = false;
					}
				} END_TRANS_CATCH_ALL("SetDefaultPermissions");
			}
		}
		else {
			//permissions match, nothing needs to happen
			bSuccess = true;
		}
	}

	return bSuccess;
}

//DRT 3/27/03 - Loops through all users, templates, etc and removes the permission for sptType
//		from each user if they have it.
//		Note that sptType is not a strict SecurityPermissionType object, you can pass in sptView | sptWrite
//		and it will remove permission to both of those parts of the object
bool RemoveUserPermission(EBuiltInObjectIDs eBuiltInID, const long sptType) {

	try {
		//Write the whole thing as 1 quick update statement - very fancy!
		ExecuteSql("declare @sptType int; "
			"SET @sptType = %li; "
			"UPDATE PermissionT SET Permissions = Permissions & (~@sptType) WHERE ObjectID = %li;", (long)sptType, eBuiltInID);

		return true;

	} NxCatchAll("Error removing default permissions.");

	return false;
}

//DRT 3/27/03 - Parameters:
//eBuiltInID - The ID of the built in object we're inserting permissions for
//dla - an array of things to compare each users current permissions against
//nAttemptedPerms - the permissions to set (or OR with, in the case of updates) if the comparison succeeds
//bInsertExpected - Are we expecting to Insert new data?  Generally set for new permissions, not updates.
//					If we are attempting to Insert and there is already something in that place, an exception 
//					will be thrown
//nAlreadyAppliedPerms - Any permissions we've already set on this object.  This will be irrelevant most of the
//					time, but in new install, long-time-since-upgrade, etc cases, this allows us to set the 
//					permissions all in 1 swoop.
//
//		Return Conditions:
//true - True is returned if everything was set correctly as it was intended.
//false - False is returned if we wrote absolutely no data.  This is generally caused when a permission is asked
//			to be compared against, but does not yet exist in our State table (maybe run later).
//Exception - In this case some things have been written, but not all of them when the error arose.  We throw an 
///			exception, which is caught above in EnsureDefaultPermissions().  If an exception is caught, we rollback
//			any changes which have occurred so far.
bool SetUserData(EBuiltInObjectIDs eBuiltInID, DoubleListArray* dla, unsigned long nAttemptedPerms, bool bInsertExpected) {

	CString strSelect, strFrom, strTemp;
	CString strTestWhere;

	strSelect = "SELECT PersonID, SelfPermQ.Permissions AS SelfPerms, ";
	strFrom.Format("FROM (SELECT PersonID FROM UsersT UNION SELECT PersonID FROM UserGroupsT) SubQ "
			"LEFT JOIN (SELECT * FROM PermissionT WHERE ObjectID = %li) SelfPermQ ON SubQ.PersonID = SelfPermQ.UserGroupID ", (long)eBuiltInID);

	//loop through the array to generate our SQL Query
	for(int i = 0; i < dla->GetSize(); i++) {
		for(int j = 0; j < dla->GetAt(i)->GetSize(); j++) {
			strTemp.Format("ComparePermQ_%li_%li.Permissions AS Perms_%li_%li, ", i, j, i, j);
			strSelect += strTemp;

			strTemp.Format("LEFT JOIN (SELECT * FROM PermissionT WHERE ObjectID = %li) ComparePermQ_%li_%li ON SubQ.PersonID = ComparePermQ_%li_%li.UserGroupID ", 
				dla->GetAt(i)->GetAt(j).eBuiltInID, i, j, i, j);
			strFrom += strTemp;

			//for testing available permissions
			//TES 10/8/2009 - PLID 35887 - If sptType is sptDynamic0WithPass (1 << 31), then a.) SQL will treat the
			// numeric literal as a NUMERIC type, and throw an exception, and b.) even after converting to an int,
			// the & will return 1 << 31, which is LESS than 0.  So, I added a convert, and changed the operator
			// from > 0 to <> 0.
			strTemp.Format("(ObjectID = %li AND (AvailablePerms & convert(int,%li)) <> 0) AND ", 
				(long)dla->GetAt(i)->GetAt(j).eBuiltInID, (long)dla->GetAt(i)->GetAt(j).sptType);
			strTestWhere += strTemp;
		}
	}

	////////////
	// Setup a recordset to ensure all items we're comparing against are correctly set in the data
	if(dla->GetSize() > 0) {
		//only need to execute this if we're comparing against something
		strTestWhere.TrimRight(" AND ");

		CString strTest = "SELECT ObjectID, AvailablePerms FROM PermissionStateT WHERE ";
		_RecordsetPtr prsTest = CreateRecordsetStd(strTest + " " + strTestWhere);

		FieldsPtr fieldsTest = prsTest->Fields;
		CMap<EBuiltInObjectIDs, EBuiltInObjectIDs, unsigned long, unsigned long> mapTest;

		while(!prsTest->eof) {
			long nObjectID = AdoFldLong(fieldsTest, "ObjectID");
			unsigned long nPerms = AdoFldLong(fieldsTest, "AvailablePerms");
			mapTest.SetAt((EBuiltInObjectIDs)nObjectID, nPerms);

			prsTest->MoveNext();
		}

		//now loop through all our items and make sure they all exist in the map
		for(i = 0; i < dla->GetSize(); i++) {
			for(int j = 0; j < dla->GetAt(i)->GetSize(); j++) {
				unsigned long nPerms = 0;
				BOOL bFound = mapTest.Lookup(dla->GetAt(i)->GetAt(j).eBuiltInID, nPerms);
				if(!bFound) {
					//item is not correctly in the database!  We must return false, this update
					//is not yet ready to run
					return false;
				}
			}
		}
	}
	// Done
	////////////


	//trim any commas off the end of the select
	strSelect.TrimRight(", ");

	CString strQuery = strSelect + " " + strFrom;

	//because we're in a transaction, we can't really loop through all the items and do an insert, 
	//there's a good chance of running into an error condition (having more than 1 connection pending
	//at a time).  So we'll save everything in this string, and then just run that when we're done.
	CString strExecuteSql;

	//loop through a list of all users
	_RecordsetPtr prs = CreateRecordsetStd(strQuery, adOpenForwardOnly, adLockReadOnly, adCmdText);
	FieldsPtr fields = prs->Fields;
	bool bThrowException = false;

	long AuditID = BeginNewAuditEvent();

	while(!prs->eof) {
		long nPersonID = AdoFldLong(fields, "PersonID");
		bool bOneFailed = false;

		//for each user, loop through all our arrays until one matches (or we run out of arrays)
		for(int i = 0; i < dla->GetSize(); i++) {
			bOneFailed = false;

			//now see if this user meets all the criteria of every item in this sub-array
			//quit out of the loop if we find something that doesn't meet the criteria
			for(int j = 0; j < dla->GetAt(i)->GetSize() && !bOneFailed; j++) {
				//setup our variables
				unsigned long nPerms = 0;
				_variant_t var;
				CString strPerm;
				strPerm.Format("Perms_%li_%li", i, j);
				var = fields->Item[_bstr_t(strPerm)]->Value;

				if(var.vt == VT_I4) {
					//they have a field for this permission
					nPerms = (unsigned long)var.lVal;
				}

				if(nPerms & dla->GetAt(i)->GetAt(j).sptType) {
					//they meet our criteria
				}
				else {
					//they don't meet our criteria
					bOneFailed = true;
				}
			}

			//by this point, we've gone through every item in the list, if bOneFailed is false
			//we have success, and don't need to continue looping.  Otherwise we must keep on.
			if(!bOneFailed)
				break;	//jump out of the loop
		}

		//we're out of the loop.  If bOneFailed is false, we successfully completed the loop
		//otherwise, there was some kind of error and this user did not meet the criteria of 
		//any allowed options
		if(!bOneFailed) {

			//the user is approved for data insertion.  Here are the rules:
			// - If bInsertExpected is true, then we expect this is a new permission, and will need
			//inserted for all users.  But just to be safe, we already got the SelfPerms out in the 
			//above recordset.  If the user already has something written there, and bInsertExpected
			//is true, then we must throw an exception - something has gone horribly wrong if one 
			//user has data for a brand new permission
			// - In the case bInsertExpected is false, then we are updating an existing permission with
			//something new.  We check our SelfPerms from the recordset, and bitwise-OR that value with
			//the nAttemptedPerms that was passed in.  If there is nothing in the recordset, then we
			//must instead do an insert.  This is not an exception case, because when a user has no 
			//permission for an object, the record in PermissionT for that Person + Object is removed, 
			//so if they meet the criteria for setting a default, we must insert to satisfy that default.

			_variant_t var;
			var = fields->Item["SelfPerms"]->Value;

			if(var.vt == VT_I4) {
				//the user has permissions already set.  If bInsertExpected is true, this is an exception case
				if(bInsertExpected) {
					// (d.thompson 2015-08-03 18:12) - PLID 66747
					//HI THERE!  If you're receiving this message, it's because your permission objects have gotten into an invalid state.  In short:  We track
					//	one table (PermissionStateT) to tell us what the permissions should look like, and another table (PermissionT) to track what they actually
					//	look like.  Generally you only get this on development databases, and it's typically because someone added a new permission while using
					//	a database that other users were on an older executable, and while the system attempted to auto-update back and forth, someone manually
					//	granted that permission.
					//
					//So here's how to fix it:
					//	Query PermissionStateT for the eBuiltInID.  You should get no results - that's because the database doesn't know anything about this
					//		new permission.
					//	Query PermissionT for the same eBuiltInID.  You should get results!  This is the problem.  Somehow a user has some kind of permission
					//		for this objectID, yet the system thinks it's a brand new permission.
					//If you're on a development database, you can just delete the data from PermissionT.  If this is a client system, record the users who
					//	have the permission and give that to the office to reconcile manually, then delete the data from PermissionT.
					AfxThrowNxException("Attempted to insert default permissions for object ID %li a user who already has those permissions.", eBuiltInID);
					return false;
				}
				else {
					//we did not expect an insert, so bitwise or var.lVal with nAttemptedPerms
					unsigned long nPerms = 0;
					nPerms = (unsigned long)var.lVal;

					nPerms |= nAttemptedPerms;

					strTemp.Format("UPDATE PermissionT SET Permissions = %li WHERE UserGroupID = %li AND ObjectID = %li; ", 
						nPerms, nPersonID, (long)eBuiltInID);

					strExecuteSql += strTemp;
				}
			}
			else {
				//this user has no permissions currently.  regardless of what bInsertExpected is, we're just 
				//going to insert nAttemptedPerms
				strTemp.Format("INSERT INTO PermissionT (UserGroupID, ObjectID, Permissions) values (%li, %li, %li); ", 
					nPersonID, (long)eBuiltInID, nAttemptedPerms);

				strExecuteSql += strTemp;
			}

			const CBuiltInObject* pObj = NULL;
			pObj = GetBuiltInObject(eBuiltInID);
			_RecordsetPtr rsUsers = CreateRecordset("SELECT UserName FROM UsersT WHERE PersonID = %li",nPersonID);
			//if it is a group rather than a user, we don't need to audit
			if(!rsUsers->eof && pObj) {
				AuditEvent(-1, AdoFldString(rsUsers, "UserName",""),AuditID,aeiUserPermissionChanged,nPersonID,"",pObj->m_strDisplayName,aepMedium,aetChanged);
			}
			rsUsers->Close();
		}

		prs->MoveNext();
	}

	//we're done looping through, so run all our executes at the same time
	if(!strExecuteSql.IsEmpty()) {
		NxAdo::PushMaxRecordsWarningLimit pmr(100000);
		ExecuteSql(strExecuteSql);
	}

	//success!
	return true;

}

void ClearUserDefinedPermissionList() {

	for(int i = gc_aryUserDefinedObjects.GetSize()-1; i>=0; i--) {
		CBuiltInObject *bio = gc_aryUserDefinedObjects.GetAt(i);
		if(bio) {
			delete bio;
			gc_aryUserDefinedObjects.RemoveAt(i);
		}
		
	}
	gc_aryUserDefinedObjects.RemoveAll();
}
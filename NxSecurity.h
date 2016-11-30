#ifndef PRACTICE_NX_SECURITY_H
#define PRACTICE_NX_SECURITY_H

#pragma once

#include "SharedPermissionUtils.h" // (z.manning 2013-10-18 08:42) - PLID 59082


// NOTE: The bit position assigned to each variable below MUST NEVER CHANGE!
// This is absolutely critical because the database contains data that references these positions.
class CPermissions
{
public:
	CPermissions()
	{
		nPermissions = 0;
	}

	CPermissions(unsigned long nPermissions)
	{
		this->nPermissions = nPermissions;
	}

	operator unsigned long() const
	{
		return nPermissions;
	}

	CPermissions &operator |=(const CPermissions &permsOrWith)
	{
		nPermissions |= permsOrWith.nPermissions;
		return *this;
	}

public:
	union {
		struct {
			unsigned long bView				: 1;	// least signigicant bit
			unsigned long bViewWithPass		: 1;
			unsigned long bRead				: 1;
			unsigned long bReadWithPass		: 1;
			unsigned long bWrite			: 1;
			unsigned long bWriteWithPass	: 1;
			unsigned long bCreate			: 1;
			unsigned long bCreateWithPass	: 1;
			unsigned long bDelete			: 1;
			unsigned long bDeleteWithPass	: 1;
			
			unsigned long unused			: 12;	// do not use, should always be 0
			
			unsigned long bDynamic4			: 1;
			unsigned long bDynamic4WithPass	: 1;
			unsigned long bDynamic3			: 1;
			unsigned long bDynamic3WithPass	: 1;
			unsigned long bDynamic2			: 1;
			unsigned long bDynamic2WithPass	: 1;
			unsigned long bDynamic1			: 1;
			unsigned long bDynamic1WithPass	: 1;
			unsigned long bDynamic0			: 1;
			unsigned long bDynamic0WithPass	: 1;
		};
		unsigned long nPermissions; // this is automatically the bit-wise OR of the above values
	};
};

#define NX_SECURITY_DYNAMIC_PERMISSION_COUNT	5


// Some handy macros for simplified access to the permissions types

// The permissions plus the WithPass version of that same permission
const unsigned int SPT_V__________ANDPASS = (sptView|sptViewWithPass);
const unsigned int SPT__R_________ANDPASS = (sptRead|sptReadWithPass);
const unsigned int SPT___W________ANDPASS = (sptWrite|sptWriteWithPass);
const unsigned int SPT____C_______ANDPASS = (sptCreate|sptCreateWithPass);
const unsigned int SPT_____D______ANDPASS = (sptDelete|sptDeleteWithPass);
const unsigned int SPT______0_____ANDPASS = (sptDynamic0|sptDynamic0WithPass);
const unsigned int SPT_______1____ANDPASS = (sptDynamic1|sptDynamic1WithPass);
const unsigned int SPT________2___ANDPASS = (sptDynamic2|sptDynamic2WithPass);
const unsigned int SPT_________3__ANDPASS = (sptDynamic3|sptDynamic3WithPass);
const unsigned int SPT__________4_ANDPASS = (sptDynamic4|sptDynamic4WithPass);

// The permission by itself without the WithPass
const unsigned int SPT_V_________ = sptView;
const unsigned int SPT__R________ = sptRead;
const unsigned int SPT___W_______ = sptWrite;
const unsigned int SPT____C______ = sptCreate;
const unsigned int SPT_____D_____ = sptDelete;
const unsigned int SPT______0____ = sptDynamic0;
const unsigned int SPT_______1___ = sptDynamic1;
const unsigned int SPT________2__ = sptDynamic2;
const unsigned int SPT_________3_ = sptDynamic3;
const unsigned int SPT__________4 = sptDynamic4;

// The permission by itself but only the WithPass version of it
const unsigned int SPT_V__________ONLYWITHPASS = (sptViewWithPass);
const unsigned int SPT__R_________ONLYWITHPASS = (sptReadWithPass);
const unsigned int SPT___W________ONLYWITHPASS = (sptWriteWithPass);
const unsigned int SPT____C_______ONLYWITHPASS = (sptCreateWithPass);
const unsigned int SPT_____D______ONLYWITHPASS = (sptDeleteWithPass);
const unsigned int SPT______0_____ONLYWITHPASS = (sptDynamic0WithPass);
const unsigned int SPT_______1____ONLYWITHPASS = (sptDynamic1WithPass);
const unsigned int SPT________2___ONLYWITHPASS = (sptDynamic2WithPass);
const unsigned int SPT_________3__ONLYWITHPASS = (sptDynamic3WithPass);
const unsigned int SPT__________4_ONLYWITHPASS = (sptDynamic4WithPass);






// Allows easy definition of all useful combinations in a single place
// (a.walling 2007-09-21 09:05) - PLID 27468 - Added WC_0 definition
#define SPT_DEFINE_ALL_COMBINATIONS(suffix) \
const unsigned int SPT_VR________##suffix = (SPT_V_________##suffix|SPT__R________##suffix); \
const unsigned int SPT__RW_______##suffix = (SPT__R________##suffix|SPT___W_______##suffix); \
const unsigned int SPT___WC______##suffix = (SPT___W_______##suffix|SPT____C______##suffix); \
const unsigned int SPT___WC_0____##suffix = (SPT___WC______##suffix|SPT______0____##suffix); \
const unsigned int SPT____CD_____##suffix = (SPT____C______##suffix|SPT_____D_____##suffix); \
const unsigned int SPT______01___##suffix = (SPT______0____##suffix|SPT_______1___##suffix); \
const unsigned int SPT______012__##suffix = (SPT______01___##suffix|SPT________2__##suffix); \
const unsigned int SPT________23_##suffix = (SPT________2__##suffix|SPT_________3_##suffix); \
const unsigned int SPT______0123_##suffix = (SPT______01___##suffix|SPT________23_##suffix); \
const unsigned int SPT___W_D_____##suffix = (SPT___W_______##suffix|SPT_____D_____##suffix); \
const unsigned int SPT_VRW_______##suffix = (SPT_V_________##suffix|SPT__RW_______##suffix); \
const unsigned int SPT_V_W_______##suffix = (SPT_V_________##suffix|SPT___W_______##suffix); \
const unsigned int SPT__RWC______##suffix = (SPT__R________##suffix|SPT___WC______##suffix); \
const unsigned int SPT___WCD_____##suffix = (SPT___W_______##suffix|SPT____CD_____##suffix); \
const unsigned int SPT________234##suffix = (SPT__________4##suffix|SPT________23_##suffix); \
const unsigned int SPT_VRWC______##suffix = (SPT_VRW_______##suffix|SPT____C______##suffix); \
const unsigned int SPT_VRW_D_____##suffix = (SPT_VRW_______##suffix|SPT_____D_____##suffix); \
const unsigned int SPT_VR_CD_____##suffix = (SPT_VR________##suffix|SPT____CD_____##suffix); \
const unsigned int SPT__RWCD_____##suffix = (SPT__RW_______##suffix|SPT____CD_____##suffix); \
const unsigned int SPT_VRWCD_____##suffix = (SPT_VRW_______##suffix|SPT____CD_____##suffix); \
const unsigned int SPT__RWCD0____##suffix = (SPT__RWCD_____##suffix|SPT______0____##suffix); \
/* (a.walling 2008-06-17 17:08) - PLID 30356 - New permission for write access */\
const unsigned int SPT__RWCD01___##suffix = (SPT__RWCD_____##suffix|SPT______01___##suffix); \
/* (d.thompson 2009-06-03) - PLID 29909 - Patient EMR needed this*/\
const unsigned int SPT_VRWC_012__##suffix = (SPT_VRWC______##suffix|SPT______012__##suffix); \
const unsigned int SPT_V_WCD_____##suffix = (SPT_V_________##suffix|SPT___WCD_____##suffix); \
const unsigned int SPT_VRW__01___##suffix = (SPT_VRW_______##suffix|SPT______01___##suffix); \
const unsigned int SPT_V_W__01___##suffix = (SPT_V_W_______##suffix|SPT______01___##suffix); \
/* (j.jones 2010-07-30 17:21) - PLID 39917 - a dynamic perm. for bioInsuranceCo needed this*/\
const unsigned int SPT_V_WCD0____##suffix = (SPT_V_WCD_____##suffix|SPT______0____##suffix); \
const unsigned int SPT_V_WCD01___##suffix = (SPT_V_WCD_____##suffix|SPT______01___##suffix); \
const unsigned int SPT_VRWCD0____##suffix = (SPT_VRWCD_____##suffix|SPT______0____##suffix); \
const unsigned int SPT_VRWC_0____##suffix = (SPT_VRWC______##suffix|SPT______0____##suffix); \
const unsigned int SPT_VRWC_01___##suffix = (SPT_VRWC______##suffix|SPT______01___##suffix); \
const unsigned int SPT_VRWCD01___##suffix = (SPT_VRWCD_____##suffix|SPT______01___##suffix); \
const unsigned int SPT_VRWCD012__##suffix = (SPT_VRWCD_____##suffix|SPT______012__##suffix); \
const unsigned int SPT_V_WCD012__##suffix = (SPT_V_WCD_____##suffix|SPT______012__##suffix); \
const unsigned int SPT_V_WCD0123_##suffix = (SPT_V_WCD_____##suffix|SPT______0123_##suffix); \
const unsigned int SPT_VRWCD01234##suffix = (SPT_VRWCD01___##suffix|SPT________234##suffix); \
/*TES 9/19/2007 - PLID 27434  - This had never been used before, needed now for bioPatientTracking.*/\
const unsigned int SPT_VRW__0____##suffix = (SPT_VRW_______##suffix|SPT______0____##suffix); \
/*TES 9/13/2010 - PLID 39845 - Tracking now has a Delete option*/\
const unsigned int SPT_VRW_D0____##suffix = (SPT_VRW_D_____##suffix|SPT______0____##suffix); \
/*// (j.jones 2009-08-03 10:28) - PLID 33036 - added yet another combination*/ \
const unsigned int SPT__R___01___##suffix = (SPT__R________##suffix|SPT______01___##suffix); \
/*// (z.manning 2009-08-11 15:09) - PLID 24277 - And another (for EMN locking permission)*/ \
const unsigned int SPT_VRWC_0123_##suffix = (SPT_VRWC______##suffix|SPT______0123_##suffix); \
/*//(e.lally 2011-06-17) PLID 43963 - here's a combination we haven't seen before (patient note priority, category)*/ \
const unsigned int SPT_VRWCD0123_##suffix = (SPT_VRWCD_____##suffix|SPT______0123_##suffix); \
/* // (c.haag 2009-11-13 11:06) - PLID 36180 - And another */ \
const unsigned int SPT_VR__D_____##suffix = (SPT_VR________##suffix|SPT_____D_____##suffix); \
/* // (f.dinatale 2010-10-07) - PLID 33753 - This hasn't been used before, needed for bioPatientSSNMasking */ \
const unsigned int SPT__R___0____##suffix = (SPT__R________##suffix|SPT______0____##suffix); \
/*// (j.gruber 2010-10-26 13:52) - PLID 40416 - and another*/ \
const unsigned int SPT___W__0____##suffix = (SPT___W_______##suffix|SPT______0____##suffix); \
/*(c.haag 2010-12-10 10:22) - PLID 38633 */ \
const unsigned int SPT_VRWCD01_3_##suffix = (SPT_VRWCD01___##suffix|SPT_________3_##suffix); \
/* //(e.lally 2011-08-09) PLID 37287 */ \
const unsigned int SPT__RWCD012__##suffix = (SPT__RWCD_____##suffix|SPT______012__##suffix); \
/*TES 8/10/2011 - PLID 44966 */ \
const unsigned int SPT_VRW_D01___##suffix = (SPT_VRW_D_____##suffix|SPT______01___##suffix); \
/* (j.jones 2011-09-21 11:21) - PLID 45462 - W/D/0 */ \
const unsigned int SPT___W_D0____##suffix = (SPT___W_D_____##suffix|SPT______0____##suffix); \
/* (e.lally 2011-11-18) PLID 46539 - W/C/D/0 */ \
const unsigned int SPT___WCD0_____##suffix = (SPT___WCD_____##suffix|SPT______0____##suffix); \
/* (a.wilson 2012-3-22) PLID 48472 - R/C/0/1 - needed for recall system permissions bioRecallSystem */ \
const unsigned int SPT__R_C_01____##suffix = (SPT__R___01___##suffix|SPT____C______##suffix); \
/* (a.wilson 2012-5-23) PLID 48537 - R/W/0 - needed for patient referral sources permissions bioPatientReferralSources */ \
const unsigned int SPT__RW__0____##suffix = (SPT__RW_______##suffix|SPT______0____##suffix); \

#pragma warning (push)
#pragma warning (disable: 4003)

// Use the above macro to define all useful combinations for each suffix
SPT_DEFINE_ALL_COMBINATIONS();
SPT_DEFINE_ALL_COMBINATIONS(_ANDPASS);
SPT_DEFINE_ALL_COMBINATIONS(_ONLYWITHPASS);

#pragma warning (pop)




///////////////////////////////////////////////////////////////////////////////////////
/// Functions used for checking a user's permissions
///////////////////

// Does all the work necessary to officially log a given user into Practice; this can be called easily later if you want to switch users, or simply reload the permissions for the existing user
HANDLE LogInUser(LPCTSTR strUsername, LPCTSTR strPassword, long nLocationID, long nInactivityMinutes = -1, LPCTSTR strLocationName = NULL, int *pnFailureReason = NULL);

// Given a username and correct password, returns a valid handle that can be passed into other security functions
// (j.jones 2008-11-19 11:08) - PLID 28578 - added pbIsPasswordVerified as a parameter
HANDLE OpenUserHandle(const CString &strUsername, const CString &strPassword, IN long nLocationID, OPTIONAL OUT int *pnFailureReason = NULL, OPTIONAL OUT long *pnUserID = NULL, OPTIONAL OUT CString *pstrTruePassword = NULL, OPTIONAL OUT BOOL *pbIsPasswordVerified = NULL);

// Release memory associated with this user
void CloseUserHandle(HANDLE hUser);

//functions for ensuring users have the correct default permissions setup
struct UserDataListItem {
	EBuiltInObjectIDs eBuiltInID;
	ESecurityPermissionType sptType;
};
typedef CArray<UserDataListItem, UserDataListItem> UDLIArray;
typedef CArray<UDLIArray*, UDLIArray*> DoubleListArray;
//

//Functions for ensuring permissions are set correctly
void EnsureDefaultPermissions();
unsigned long SetDefaultPermission(EBuiltInObjectIDs eBuiltInID, bool bIsNew, unsigned long nAddedPerms);
bool SetUserData(EBuiltInObjectIDs eBuiltInID, DoubleListArray* dla, unsigned long nAttemptedPerms, bool bInsertExpected);
bool RemoveUserPermission(EBuiltInObjectIDs eBuiltInID, const long sptType);

//Given a permission ID, and a ObjectID, pull the correct Permission ID from SecurityObjectT
long GetDetailedPermissionValue(IN EBuiltInObjectIDs eBuiltInID, IN long nObjectValue);

// Given a user handle, a built-in object, and optionally an object value, returns the permissions that user has
void GetPermissions(IN HANDLE hUser, IN EBuiltInObjectIDs eBuiltInID, IN BOOL bUseObjectValue, IN long nObjectValue, OUT CPermissions &permsOut);
CPermissions GetPermissions(HANDLE hUser, EBuiltInObjectIDs eBuiltInID, BOOL bUseObjectValue = FALSE, OPTIONAL long nObjectValue = 0);

// (z.manning 2009-06-10 14:43) - PLID 34585 - Check to see if given user has any permissions
bool DoesUserHaveAnyPermissions(HANDLE hUser);

//when they add a new user-defined permission (new resource, provider, etc.) this adds to the array
// (j.jones 2010-01-14 16:08) - PLID 36887 - added support for dynamic names
void AddUserDefinedPermission(long ObjectID, long AvailablePermissions, CString DisplayName, CString Description, long ParentID, long nCurrentPermissions,
							  CString strDynamicName0 = "", CString strDynamicName1 = "", CString strDynamicName2 = "",
							  CString strDynamicName3 = "", CString strDynamicName4 = "");
// (z.manning 2011-05-19 08:43) - PID 43767 - I split up the logic of AddUserDefinedPermission into 2 different functions
long AddUserDefinedPermissionToData(long ObjectID, long AvailablePermissions, CString DisplayName, CString Description, long ParentID, long nCurrentPermissions,
							  CString strDynamicName0 = "", CString strDynamicName1 = "", CString strDynamicName2 = "",
							  CString strDynamicName3 = "", CString strDynamicName4 = "");
// (z.manning 2011-05-19 08:43) - PID 43767 - I split up the logic of AddUserDefinedPermission into 2 different functions
void AddUserDefinedPermissionToMemory(long nNewSecurityObjectID, long ObjectID, long AvailablePermissions, CString DisplayName, CString Description, long ParentID, long nCurrentPermissions,
							  CString strDynamicName0 = "", CString strDynamicName1 = "", CString strDynamicName2 = "",
							  CString strDynamicName3 = "", CString strDynamicName4 = "");

void UpdateUserDefinedPermissionName(long BuiltInID, long ObjectID, CString newDisplayName);
// (z.manning 2011-05-19 08:43) - PID 43767 - I split up the logic of UpdateUserDefinedPermissionName into 2 different functions
long UpdateUserDefinedPermissionNameInData(long BuiltInID, long ObjectID, CString newDisplayName);
void UpdateUserDefinedPermissionNameInMemory(long nSecurityObjectID, CString newDisplayName);

void DeleteUserDefinedPermission(long BuiltInID, long ObjectID, BOOL bLogInUser = TRUE);
// (z.manning 2011-05-19 08:43) - PID 43767 - I split up the logic of DeleteUserDefinedPermission into 2 different functions
long DeleteUserDefinedPermissionFromData(long BuiltInID, long ObjectID);
void DeleteUserDefinedPermissionFromMemory(long nSecurityObjectID, BOOL bLogInUser = TRUE);

void LoadUserDefinedPermissions();
void LoadUserDefinedPermissions(long nSecurityObjectID);
void LoadUserDefinedPermissions(CString strWhereClause);

void ClearUserDefinedPermissionList();

///////////////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////////////
/// Functions used for iterating the built-in objects themselves
///////////////////

class CBuiltInObject
{
public:
	CBuiltInObject(EBuiltInObjectIDs eBuiltInID, const CPermissions &AvailablePermissions, const CString &strDisplayName, const CString &strDescription, LPCTSTR strDynamicPermissionNames = NULL, EBuiltInObjectIDs eParentID = bioInvalidID, long nObjectValue = 0);

public:
	EBuiltInObjectIDs m_eBuiltInID; // Used to uniquely identify the object
	CPermissions m_AvailPermissions; // The permissions that this object allows
	
	CString m_strDisplayName; // The short name for this object
	
	CString m_strDescription; // The long description of this object
	
	CString m_strDynamicPermissionNames[NX_SECURITY_DYNAMIC_PERMISSION_COUNT]; // If the object supports any dynamic permissions, then the names of those permissions are stored here
	
	EBuiltInObjectIDs m_eParentID; // For use by the user interface only, in case a hierarchical display of objects is desired

	long m_nObjectValue; // The object value
};

// Returns the pointer to the official built-in object identified by the given eBuiltInObjectID
const CBuiltInObject *GetBuiltInObject(EBuiltInObjectIDs eBuiltInObjectID);

// Returns the pointer to the user-defined object identified by the given eBuiltInObjectID
const CBuiltInObject *GetUserDefinedObject(EBuiltInObjectIDs eBuiltInObjectID);

// Allows iterating through the complete list of built-in objects
POSITION GetFirstBuiltInObjectPosition();
const CBuiltInObject *GetNextBuiltInObject(IN OUT POSITION &pInOut);

///////////////////////////////////////////////////////////////////////////////////////


#endif
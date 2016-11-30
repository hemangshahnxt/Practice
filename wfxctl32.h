// Machine generated IDispatch wrapper class(es) created with ClassWizard
/////////////////////////////////////////////////////////////////////////////
// ISDKPhoneBook wrapper class

#ifndef WFXCTL32_H
#define WFXCTL32_H

#pragma once



class ISDKPhoneBook : public COleDispatchDriver
{
public:
	ISDKPhoneBook() {}		// Calls COleDispatchDriver default constructor
	ISDKPhoneBook(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	ISDKPhoneBook(const ISDKPhoneBook& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

// Attributes
public:
	LPDISPATCH GetApplication();
	void SetApplication(LPDISPATCH);
	short GetError();
	void SetError(short);

// Operations
public:
	CString GetFolderListFirst(short standardFolder, LPCTSTR folderID);
	CString GetFolderListNext();
	CString GetUserGroupFirst(short standardFolder, LPCTSTR folderID);
	CString GetUserGroupNext();
	short SetFolderDisplayName(LPCTSTR sFolder, LPCTSTR sName);
	CString GetFolderDisplayName(LPCTSTR folderID);
	short SetGroupDisplayName(LPCTSTR folderID, LPCTSTR displayname);
	CString GetGroupDisplayName(LPCTSTR groupID);
	BOOL CompareFolderEntryIDs(LPCTSTR ID1, LPCTSTR ID2);
	BOOL CompareUserGroupEntryIDs(LPCTSTR ID1, LPCTSTR ID2);
	short SetUserDisplayName(LPCTSTR userID, LPCTSTR displayname);
	CString GetUserDisplayName(LPCTSTR userID);
	short SetUserTitle(LPCTSTR userID, LPCTSTR title);
	CString GetUserTitle(LPCTSTR userID);
	short SetUserFirstName(LPCTSTR userID, LPCTSTR firstName);
	CString GetUserFirstName(LPCTSTR userID);
	short SetUserLastName(LPCTSTR userID, LPCTSTR lastName);
	CString GetUserLastName(LPCTSTR userID);
	short SetUserCompany(LPCTSTR userID, LPCTSTR company);
	CString GetUserCompany(LPCTSTR userID);
	short SetUserAddress1(LPCTSTR userID, LPCTSTR address1);
	CString GetUserAddress1(LPCTSTR userID);
	short SetUserAddress2(LPCTSTR userID, LPCTSTR address2);
	CString GetUserAddress2(LPCTSTR userID);
	short SetUserCity(LPCTSTR userID, LPCTSTR city);
	CString GetUserCity(LPCTSTR userID);
	short SetUserState(LPCTSTR userID, LPCTSTR state);
	CString GetUserState(LPCTSTR userID);
	short SetUserCountry(LPCTSTR userID, LPCTSTR country);
	CString GetUserCountry(LPCTSTR userID);
	short SetUserPostalCode(LPCTSTR userID, LPCTSTR code);
	CString GetUserPostalCode(LPCTSTR userID);
	short GetType(LPCTSTR eventID);
	short SetUserFaxCountry(LPCTSTR userID, LPCTSTR number);
	CString GetUserFaxCountry(LPCTSTR userID);
	short SetUserFaxArea(LPCTSTR userID, LPCTSTR number);
	CString GetUserFaxArea(LPCTSTR userID);
	short SetUserFaxLocal(LPCTSTR userID, LPCTSTR number);
	CString GetUserFaxLocal(LPCTSTR userID);
	short SetUserFaxExtension(LPCTSTR userID, LPCTSTR number);
	CString GetUserFaxExtension(LPCTSTR userID);
	short SetUserFaxAltCountry(LPCTSTR userID, LPCTSTR number);
	CString GetUserFaxAltCountry(LPCTSTR userID);
	short SetUserFaxAltArea(LPCTSTR userID, LPCTSTR number);
	CString GetUserFaxAltArea(LPCTSTR userID);
	short SetUserFaxAltLocal(LPCTSTR userID, LPCTSTR number);
	CString GetUserFaxAltLocal(LPCTSTR userID);
	short SetUserFaxAltExtension(LPCTSTR userID, LPCTSTR number);
	CString GetUserFaxAltExtension(LPCTSTR userID);
	short SetUserCSID(LPCTSTR userID, LPCTSTR csid);
	CString GetUserCSID(LPCTSTR userID);
	short SetUserVoiceCountry(LPCTSTR userID, LPCTSTR number);
	CString GetUserVoiceCountry(LPCTSTR userID);
	short SetUserVoiceArea(LPCTSTR userID, LPCTSTR number);
	CString GetUserVoiceArea(LPCTSTR userID);
	short SetUserVoiceLocal(LPCTSTR userID, LPCTSTR number);
	CString GetUserVoiceLocal(LPCTSTR userID);
	short SetUserVoiceExtension(LPCTSTR userID, LPCTSTR number);
	CString GetUserVoiceExtension(LPCTSTR userID);
	short SetUserCellCountry(LPCTSTR userID, LPCTSTR number);
	CString GetUserCellCountry(LPCTSTR userID);
	short SetUserCellArea(LPCTSTR userID, LPCTSTR number);
	CString GetUserCellArea(LPCTSTR userID);
	short SetUserCellLocal(LPCTSTR userID, LPCTSTR number);
	CString GetUserCellLocal(LPCTSTR userID);
	short SetUserCellExtension(LPCTSTR userID, LPCTSTR number);
	CString GetUserCellExtension(LPCTSTR userID);
	CString AddFolder(short standardFolder, LPCTSTR folderID, LPCTSTR folderName, LPCTSTR comment);
	CString AddUser(short standardFolder, LPCTSTR folderID, LPCTSTR firstName, LPCTSTR lastName, LPCTSTR company);
	CString AddGroup(short standardFolder, LPCTSTR folderID, LPCTSTR groupName);
	short DeleteFolder(LPCTSTR folderID);
	short DeleteUser(LPCTSTR userID);
	short DeleteGroup(LPCTSTR groupID);
};

// enum definitions for use with the SDK
typedef enum  {
    SDKERROR_NOERROR = 0,
    SDKERROR_NOENTRYID,         // A required entry ID was not specified.
    SDKERROR_BADENTRYID,        // Could not get/set entry ID.
    SDKERROR_BADDATASTRUCT,     // Could not create internal data layer structure.
    SDKERROR_BADFILESPEC,       // Bad file name or file not found.
    SDKERROR_CANTSETFOLDER,     // Could not change to specified folder.
    SDKERROR_CANTGETCOUNTER,    // Could not invoke the object counter.
    SDKERROR_CANTENUMERATE,     // Internal enumeration error.
    SDKERROR_CANTREMOVE,        // Error removing the specified object.
    SDKERROR_CANTADD,           // Error adding the specified object.
    SDKERROR_WRONGEVENTTYPE,    // Wrong type of event for this operation.
} TSDKError;

typedef enum  {
    STANDARDFOLDER_NONE = 0,            	// Don't specify standard folder.
    STANDARDFOLDER_WINFAX_ROOT,	// Root.
    STANDARDFOLDER_WINFAX_LOG,	// Winfax's LOGs.
    STANDARDFOLDER_WINFAX_RECEIVELOG,
    STANDARDFOLDER_WINFAX_SENDLOG,
    STANDARDFOLDER_WINFAX_WASTEBASKET,
    STANDARDFOLDER_WINFAX_OUTBOX,
    STANDARDFOLDER_MAPI,                // MAPI
} TStandardFolder;

typedef enum  {
     EVENTTYPE_UNKNOWN = 0,
     EVENTTYPE_LGFOLDER,
     EVENTTYPE_FAX,
     EVENTTYPE_VOICE,
     EVENTTYPE_PBFOLDER,
     EVENTTYPE_USER,
     EVENTTYPE_GROUP,
} TEventType;

#endif //WFXCTL32_H
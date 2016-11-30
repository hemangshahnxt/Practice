// Machine generated IDispatch wrapper class(es) created with ClassWizard

#include "stdafx.h"
#include "wfxctl32.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



/////////////////////////////////////////////////////////////////////////////
// ISDKPhoneBook properties

LPDISPATCH ISDKPhoneBook::GetApplication()
{
	LPDISPATCH result;
	GetProperty(0x1, VT_DISPATCH, (void*)&result);
	return result;
}

void ISDKPhoneBook::SetApplication(LPDISPATCH propVal)
{
	SetProperty(0x1, VT_DISPATCH, propVal);
}

short ISDKPhoneBook::GetError()
{
	short result;
	GetProperty(0x2, VT_I2, (void*)&result);
	return result;
}

void ISDKPhoneBook::SetError(short propVal)
{
	SetProperty(0x2, VT_I2, propVal);
}

/////////////////////////////////////////////////////////////////////////////
// ISDKPhoneBook operations

CString ISDKPhoneBook::GetFolderListFirst(short standardFolder, LPCTSTR folderID)
{
	CString result;
	static BYTE parms[] =
		VTS_I2 VTS_BSTR;
	InvokeHelper(0x3, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms,
		standardFolder, folderID);
	return result;
}

CString ISDKPhoneBook::GetFolderListNext()
{
	CString result;
	InvokeHelper(0x4, DISPATCH_METHOD, VT_BSTR, (void*)&result, NULL);
	return result;
}

CString ISDKPhoneBook::GetUserGroupFirst(short standardFolder, LPCTSTR folderID)
{
	CString result;
	static BYTE parms[] =
		VTS_I2 VTS_BSTR;
	InvokeHelper(0x5, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms,
		standardFolder, folderID);
	return result;
}

CString ISDKPhoneBook::GetUserGroupNext()
{
	CString result;
	InvokeHelper(0x6, DISPATCH_METHOD, VT_BSTR, (void*)&result, NULL);
	return result;
}

short ISDKPhoneBook::SetFolderDisplayName(LPCTSTR sFolder, LPCTSTR sName)
{
	short result;
	static BYTE parms[] =
		VTS_BSTR VTS_BSTR;
	InvokeHelper(0x7, DISPATCH_METHOD, VT_I2, (void*)&result, parms,
		sFolder, sName);
	return result;
}

CString ISDKPhoneBook::GetFolderDisplayName(LPCTSTR folderID)
{
	CString result;
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x8, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms,
		folderID);
	return result;
}

short ISDKPhoneBook::SetGroupDisplayName(LPCTSTR folderID, LPCTSTR displayname)
{
	short result;
	static BYTE parms[] =
		VTS_BSTR VTS_BSTR;
	InvokeHelper(0x9, DISPATCH_METHOD, VT_I2, (void*)&result, parms,
		folderID, displayname);
	return result;
}

CString ISDKPhoneBook::GetGroupDisplayName(LPCTSTR groupID)
{
	CString result;
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0xa, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms,
		groupID);
	return result;
}

BOOL ISDKPhoneBook::CompareFolderEntryIDs(LPCTSTR ID1, LPCTSTR ID2)
{
	BOOL result;
	static BYTE parms[] =
		VTS_BSTR VTS_BSTR;
	InvokeHelper(0xb, DISPATCH_METHOD, VT_BOOL, (void*)&result, parms,
		ID1, ID2);
	return result;
}

BOOL ISDKPhoneBook::CompareUserGroupEntryIDs(LPCTSTR ID1, LPCTSTR ID2)
{
	BOOL result;
	static BYTE parms[] =
		VTS_BSTR VTS_BSTR;
	InvokeHelper(0xc, DISPATCH_METHOD, VT_BOOL, (void*)&result, parms,
		ID1, ID2);
	return result;
}

short ISDKPhoneBook::SetUserDisplayName(LPCTSTR userID, LPCTSTR displayname)
{
	short result;
	static BYTE parms[] =
		VTS_BSTR VTS_BSTR;
	InvokeHelper(0xd, DISPATCH_METHOD, VT_I2, (void*)&result, parms,
		userID, displayname);
	return result;
}

CString ISDKPhoneBook::GetUserDisplayName(LPCTSTR userID)
{
	CString result;
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0xe, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms,
		userID);
	return result;
}

short ISDKPhoneBook::SetUserTitle(LPCTSTR userID, LPCTSTR title)
{
	short result;
	static BYTE parms[] =
		VTS_BSTR VTS_BSTR;
	InvokeHelper(0xf, DISPATCH_METHOD, VT_I2, (void*)&result, parms,
		userID, title);
	return result;
}

CString ISDKPhoneBook::GetUserTitle(LPCTSTR userID)
{
	CString result;
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x10, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms,
		userID);
	return result;
}

short ISDKPhoneBook::SetUserFirstName(LPCTSTR userID, LPCTSTR firstName)
{
	short result;
	static BYTE parms[] =
		VTS_BSTR VTS_BSTR;
	InvokeHelper(0x11, DISPATCH_METHOD, VT_I2, (void*)&result, parms,
		userID, firstName);
	return result;
}

CString ISDKPhoneBook::GetUserFirstName(LPCTSTR userID)
{
	CString result;
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x12, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms,
		userID);
	return result;
}

short ISDKPhoneBook::SetUserLastName(LPCTSTR userID, LPCTSTR lastName)
{
	short result;
	static BYTE parms[] =
		VTS_BSTR VTS_BSTR;
	InvokeHelper(0x13, DISPATCH_METHOD, VT_I2, (void*)&result, parms,
		userID, lastName);
	return result;
}

CString ISDKPhoneBook::GetUserLastName(LPCTSTR userID)
{
	CString result;
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x14, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms,
		userID);
	return result;
}

short ISDKPhoneBook::SetUserCompany(LPCTSTR userID, LPCTSTR company)
{
	short result;
	static BYTE parms[] =
		VTS_BSTR VTS_BSTR;
	InvokeHelper(0x15, DISPATCH_METHOD, VT_I2, (void*)&result, parms,
		userID, company);
	return result;
}

CString ISDKPhoneBook::GetUserCompany(LPCTSTR userID)
{
	CString result;
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x16, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms,
		userID);
	return result;
}

short ISDKPhoneBook::SetUserAddress1(LPCTSTR userID, LPCTSTR address1)
{
	short result;
	static BYTE parms[] =
		VTS_BSTR VTS_BSTR;
	InvokeHelper(0x17, DISPATCH_METHOD, VT_I2, (void*)&result, parms,
		userID, address1);
	return result;
}

CString ISDKPhoneBook::GetUserAddress1(LPCTSTR userID)
{
	CString result;
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x18, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms,
		userID);
	return result;
}

short ISDKPhoneBook::SetUserAddress2(LPCTSTR userID, LPCTSTR address2)
{
	short result;
	static BYTE parms[] =
		VTS_BSTR VTS_BSTR;
	InvokeHelper(0x19, DISPATCH_METHOD, VT_I2, (void*)&result, parms,
		userID, address2);
	return result;
}

CString ISDKPhoneBook::GetUserAddress2(LPCTSTR userID)
{
	CString result;
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x1a, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms,
		userID);
	return result;
}

short ISDKPhoneBook::SetUserCity(LPCTSTR userID, LPCTSTR city)
{
	short result;
	static BYTE parms[] =
		VTS_BSTR VTS_BSTR;
	InvokeHelper(0x1b, DISPATCH_METHOD, VT_I2, (void*)&result, parms,
		userID, city);
	return result;
}

CString ISDKPhoneBook::GetUserCity(LPCTSTR userID)
{
	CString result;
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x1c, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms,
		userID);
	return result;
}

short ISDKPhoneBook::SetUserState(LPCTSTR userID, LPCTSTR state)
{
	short result;
	static BYTE parms[] =
		VTS_BSTR VTS_BSTR;
	InvokeHelper(0x1d, DISPATCH_METHOD, VT_I2, (void*)&result, parms,
		userID, state);
	return result;
}

CString ISDKPhoneBook::GetUserState(LPCTSTR userID)
{
	CString result;
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x1e, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms,
		userID);
	return result;
}

short ISDKPhoneBook::SetUserCountry(LPCTSTR userID, LPCTSTR country)
{
	short result;
	static BYTE parms[] =
		VTS_BSTR VTS_BSTR;
	InvokeHelper(0x1f, DISPATCH_METHOD, VT_I2, (void*)&result, parms,
		userID, country);
	return result;
}

CString ISDKPhoneBook::GetUserCountry(LPCTSTR userID)
{
	CString result;
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x20, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms,
		userID);
	return result;
}

short ISDKPhoneBook::SetUserPostalCode(LPCTSTR userID, LPCTSTR code)
{
	short result;
	static BYTE parms[] =
		VTS_BSTR VTS_BSTR;
	InvokeHelper(0x21, DISPATCH_METHOD, VT_I2, (void*)&result, parms,
		userID, code);
	return result;
}

CString ISDKPhoneBook::GetUserPostalCode(LPCTSTR userID)
{
	CString result;
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x22, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms,
		userID);
	return result;
}

short ISDKPhoneBook::GetType(LPCTSTR eventID)
{
	short result;
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x23, DISPATCH_METHOD, VT_I2, (void*)&result, parms,
		eventID);
	return result;
}

short ISDKPhoneBook::SetUserFaxCountry(LPCTSTR userID, LPCTSTR number)
{
	short result;
	static BYTE parms[] =
		VTS_BSTR VTS_BSTR;
	InvokeHelper(0x24, DISPATCH_METHOD, VT_I2, (void*)&result, parms,
		userID, number);
	return result;
}

CString ISDKPhoneBook::GetUserFaxCountry(LPCTSTR userID)
{
	CString result;
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x25, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms,
		userID);
	return result;
}

short ISDKPhoneBook::SetUserFaxArea(LPCTSTR userID, LPCTSTR number)
{
	short result;
	static BYTE parms[] =
		VTS_BSTR VTS_BSTR;
	InvokeHelper(0x26, DISPATCH_METHOD, VT_I2, (void*)&result, parms,
		userID, number);
	return result;
}

CString ISDKPhoneBook::GetUserFaxArea(LPCTSTR userID)
{
	CString result;
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x27, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms,
		userID);
	return result;
}

short ISDKPhoneBook::SetUserFaxLocal(LPCTSTR userID, LPCTSTR number)
{
	short result;
	static BYTE parms[] =
		VTS_BSTR VTS_BSTR;
	InvokeHelper(0x28, DISPATCH_METHOD, VT_I2, (void*)&result, parms,
		userID, number);
	return result;
}

CString ISDKPhoneBook::GetUserFaxLocal(LPCTSTR userID)
{
	CString result;
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x29, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms,
		userID);
	return result;
}

short ISDKPhoneBook::SetUserFaxExtension(LPCTSTR userID, LPCTSTR number)
{
	short result;
	static BYTE parms[] =
		VTS_BSTR VTS_BSTR;
	InvokeHelper(0x2a, DISPATCH_METHOD, VT_I2, (void*)&result, parms,
		userID, number);
	return result;
}

CString ISDKPhoneBook::GetUserFaxExtension(LPCTSTR userID)
{
	CString result;
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x2b, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms,
		userID);
	return result;
}

short ISDKPhoneBook::SetUserFaxAltCountry(LPCTSTR userID, LPCTSTR number)
{
	short result;
	static BYTE parms[] =
		VTS_BSTR VTS_BSTR;
	InvokeHelper(0x2c, DISPATCH_METHOD, VT_I2, (void*)&result, parms,
		userID, number);
	return result;
}

CString ISDKPhoneBook::GetUserFaxAltCountry(LPCTSTR userID)
{
	CString result;
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x2d, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms,
		userID);
	return result;
}

short ISDKPhoneBook::SetUserFaxAltArea(LPCTSTR userID, LPCTSTR number)
{
	short result;
	static BYTE parms[] =
		VTS_BSTR VTS_BSTR;
	InvokeHelper(0x2e, DISPATCH_METHOD, VT_I2, (void*)&result, parms,
		userID, number);
	return result;
}

CString ISDKPhoneBook::GetUserFaxAltArea(LPCTSTR userID)
{
	CString result;
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x2f, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms,
		userID);
	return result;
}

short ISDKPhoneBook::SetUserFaxAltLocal(LPCTSTR userID, LPCTSTR number)
{
	short result;
	static BYTE parms[] =
		VTS_BSTR VTS_BSTR;
	InvokeHelper(0x30, DISPATCH_METHOD, VT_I2, (void*)&result, parms,
		userID, number);
	return result;
}

CString ISDKPhoneBook::GetUserFaxAltLocal(LPCTSTR userID)
{
	CString result;
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x31, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms,
		userID);
	return result;
}

short ISDKPhoneBook::SetUserFaxAltExtension(LPCTSTR userID, LPCTSTR number)
{
	short result;
	static BYTE parms[] =
		VTS_BSTR VTS_BSTR;
	InvokeHelper(0x32, DISPATCH_METHOD, VT_I2, (void*)&result, parms,
		userID, number);
	return result;
}

CString ISDKPhoneBook::GetUserFaxAltExtension(LPCTSTR userID)
{
	CString result;
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x33, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms,
		userID);
	return result;
}

short ISDKPhoneBook::SetUserCSID(LPCTSTR userID, LPCTSTR csid)
{
	short result;
	static BYTE parms[] =
		VTS_BSTR VTS_BSTR;
	InvokeHelper(0x34, DISPATCH_METHOD, VT_I2, (void*)&result, parms,
		userID, csid);
	return result;
}

CString ISDKPhoneBook::GetUserCSID(LPCTSTR userID)
{
	CString result;
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x35, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms,
		userID);
	return result;
}

short ISDKPhoneBook::SetUserVoiceCountry(LPCTSTR userID, LPCTSTR number)
{
	short result;
	static BYTE parms[] =
		VTS_BSTR VTS_BSTR;
	InvokeHelper(0x36, DISPATCH_METHOD, VT_I2, (void*)&result, parms,
		userID, number);
	return result;
}

CString ISDKPhoneBook::GetUserVoiceCountry(LPCTSTR userID)
{
	CString result;
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x37, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms,
		userID);
	return result;
}

short ISDKPhoneBook::SetUserVoiceArea(LPCTSTR userID, LPCTSTR number)
{
	short result;
	static BYTE parms[] =
		VTS_BSTR VTS_BSTR;
	InvokeHelper(0x38, DISPATCH_METHOD, VT_I2, (void*)&result, parms,
		userID, number);
	return result;
}

CString ISDKPhoneBook::GetUserVoiceArea(LPCTSTR userID)
{
	CString result;
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x39, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms,
		userID);
	return result;
}

short ISDKPhoneBook::SetUserVoiceLocal(LPCTSTR userID, LPCTSTR number)
{
	short result;
	static BYTE parms[] =
		VTS_BSTR VTS_BSTR;
	InvokeHelper(0x3a, DISPATCH_METHOD, VT_I2, (void*)&result, parms,
		userID, number);
	return result;
}

CString ISDKPhoneBook::GetUserVoiceLocal(LPCTSTR userID)
{
	CString result;
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x3b, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms,
		userID);
	return result;
}

short ISDKPhoneBook::SetUserVoiceExtension(LPCTSTR userID, LPCTSTR number)
{
	short result;
	static BYTE parms[] =
		VTS_BSTR VTS_BSTR;
	InvokeHelper(0x3c, DISPATCH_METHOD, VT_I2, (void*)&result, parms,
		userID, number);
	return result;
}

CString ISDKPhoneBook::GetUserVoiceExtension(LPCTSTR userID)
{
	CString result;
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x3d, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms,
		userID);
	return result;
}

short ISDKPhoneBook::SetUserCellCountry(LPCTSTR userID, LPCTSTR number)
{
	short result;
	static BYTE parms[] =
		VTS_BSTR VTS_BSTR;
	InvokeHelper(0x3e, DISPATCH_METHOD, VT_I2, (void*)&result, parms,
		userID, number);
	return result;
}

CString ISDKPhoneBook::GetUserCellCountry(LPCTSTR userID)
{
	CString result;
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x3f, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms,
		userID);
	return result;
}

short ISDKPhoneBook::SetUserCellArea(LPCTSTR userID, LPCTSTR number)
{
	short result;
	static BYTE parms[] =
		VTS_BSTR VTS_BSTR;
	InvokeHelper(0x40, DISPATCH_METHOD, VT_I2, (void*)&result, parms,
		userID, number);
	return result;
}

CString ISDKPhoneBook::GetUserCellArea(LPCTSTR userID)
{
	CString result;
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x41, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms,
		userID);
	return result;
}

short ISDKPhoneBook::SetUserCellLocal(LPCTSTR userID, LPCTSTR number)
{
	short result;
	static BYTE parms[] =
		VTS_BSTR VTS_BSTR;
	InvokeHelper(0x42, DISPATCH_METHOD, VT_I2, (void*)&result, parms,
		userID, number);
	return result;
}

CString ISDKPhoneBook::GetUserCellLocal(LPCTSTR userID)
{
	CString result;
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x43, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms,
		userID);
	return result;
}

short ISDKPhoneBook::SetUserCellExtension(LPCTSTR userID, LPCTSTR number)
{
	short result;
	static BYTE parms[] =
		VTS_BSTR VTS_BSTR;
	InvokeHelper(0x44, DISPATCH_METHOD, VT_I2, (void*)&result, parms,
		userID, number);
	return result;
}

CString ISDKPhoneBook::GetUserCellExtension(LPCTSTR userID)
{
	CString result;
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x45, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms,
		userID);
	return result;
}

CString ISDKPhoneBook::AddFolder(short standardFolder, LPCTSTR folderID, LPCTSTR folderName, LPCTSTR comment)
{
	CString result;
	static BYTE parms[] =
		VTS_I2 VTS_BSTR VTS_BSTR VTS_BSTR;
	InvokeHelper(0x46, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms,
		standardFolder, folderID, folderName, comment);
	return result;
}

CString ISDKPhoneBook::AddUser(short standardFolder, LPCTSTR folderID, LPCTSTR firstName, LPCTSTR lastName, LPCTSTR company)
{
	CString result;
	static BYTE parms[] =
		VTS_I2 VTS_BSTR VTS_BSTR VTS_BSTR VTS_BSTR;
	InvokeHelper(0x47, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms,
		standardFolder, folderID, firstName, lastName, company);
	return result;
}

CString ISDKPhoneBook::AddGroup(short standardFolder, LPCTSTR folderID, LPCTSTR groupName)
{
	CString result;
	static BYTE parms[] =
		VTS_I2 VTS_BSTR VTS_BSTR;
	InvokeHelper(0x48, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms,
		standardFolder, folderID, groupName);
	return result;
}

short ISDKPhoneBook::DeleteFolder(LPCTSTR folderID)
{
	short result;
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x49, DISPATCH_METHOD, VT_I2, (void*)&result, parms,
		folderID);
	return result;
}

short ISDKPhoneBook::DeleteUser(LPCTSTR userID)
{
	short result;
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x4a, DISPATCH_METHOD, VT_I2, (void*)&result, parms,
		userID);
	return result;
}

short ISDKPhoneBook::DeleteGroup(LPCTSTR groupID)
{
	short result;
	static BYTE parms[] =
		VTS_BSTR;
	InvokeHelper(0x4b, DISPATCH_METHOD, VT_I2, (void*)&result, parms,
		groupID);
	return result;
}

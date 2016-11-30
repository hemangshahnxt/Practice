// Upgrade.cpp: implementation of the CUpgrade class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "Upgrade.h"
#include "RegUtils.h"

#define _WIN32_MSI 110	//USE 1.1 installer API

#include <Msi.h>
#include <Msiquery.h>
#include <stdio.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

DWORD parseVersion(char *buffer)
{
	long dot;
	DWORD version = 0;
	char *p = buffer;

	//major
	dot = strchr(p, '.') - p;
	if (dot == -1)
		return version;
	version = atoi(p) << 24;
	p += dot + 1;

	//minor
	dot = strchr(p, '.') - p;
	if (dot == -1)
		return version;
	version += atoi(p) << 16;
	p += dot + 1;

	//build
	dot = strchr(p, '.') - p;
	if (dot == -1)
		return version;
	version += atoi(p);

	return version; 
}

DWORD GetPracticeLastVersion()
{
	//enumerate all version of Nextech Practice Installed
	//return the highest version
	//in almost all cases, only 1 copy will be installed
	DWORD size = 12;

	DWORD result;
	char productCode[39];
	char buffer[12];
	long i = 0;
	DWORD version;
	DWORD maxVersion = 0;

	do
	{
		result = MsiEnumRelatedProducts("{D98C20EB-4CF3-435F-B0C0-4C2967B4607C}", 0, i, productCode);

		if (result == ERROR_SUCCESS)
		{
			size = 12;
			result = MsiGetProductInfo(productCode, INSTALLPROPERTY_VERSIONSTRING, buffer, &size);

			if (result == ERROR_SUCCESS)
			{
				version = parseVersion(buffer);

				if (version > maxVersion)
					maxVersion = version;
			}

			i++;
		}
	}	while (result == ERROR_SUCCESS || result == ERROR_UNKNOWN_PRODUCT);//ERROR_UNKNOWN_PRODUCT is possible for a previous install that didn't uninstall 

	return maxVersion;
}

CString GetNextPath()
{
	//DRT 8/10/2005 - PLID 15688 - This is always located in <root>\Setup\NextPath, no matter what registry
	//	key we are using.  We therefore have to strip off the registry type if it's in use.
	CString strBase = GetRegistryBase();
	CString strSub = GetSubRegistryKey();

	//if this is empty, we're on the base level
	if(!strSub.IsEmpty()) {
		//strip off the subkey
		strBase = strBase.Left(strBase.GetLength() - strSub.GetLength() - 1);
	}

	return strBase ^ "Setup\\NextPath";
}

DWORD GetPracticeNewVersion(const char *path)
{
	char buffer[12];
	unsigned long size = 12;
	MSIHANDLE msi, view, record;

	if (ERROR_SUCCESS != MsiOpenDatabase(path, MSIDBOPEN_READONLY, &msi))
		return 0;
	if (ERROR_SUCCESS != MsiDatabaseOpenView(msi, 
		"SELECT Value FROM Property WHERE Property = 'ProductVersion'", &view))
		return 0;
	if (ERROR_SUCCESS != MsiViewExecute(view, 0))
		return 0;
	if (ERROR_SUCCESS != MsiViewFetch(view, &record))
		return 0;
	if (ERROR_SUCCESS != MsiRecordGetString(record, 1, buffer, &size))
		return 0;

	if (ERROR_SUCCESS != MsiCloseHandle(view))
		return 0;
	if (ERROR_SUCCESS != MsiCloseHandle(record))
		return 0;
	if (ERROR_SUCCESS != MsiCloseHandle(msi))
		return 0;

	return parseVersion(buffer);
}

DWORD GetPracticeNewVersion()
{
	CString path;
	path = NxRegUtils::ReadString(GetNextPath());
	path = path ^ "practice.msi";

	return GetPracticeNewVersion(path);
}

bool CheckUpgrade()
{
	DWORD currentVer = GetPracticeLastVersion(),
		  nextVer = GetPracticeNewVersion();

	//if current version = 0, we could not get a version, so don't prompt
	if (nextVer > currentVer && currentVer
		&& IDYES == AfxMessageBox("A new version of Practice is available.\n"
		"Would you like to run setup?", MB_YESNO))
	{	CString path;
		STARTUPINFO si;
		PROCESS_INFORMATION processInfo;

		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);


		path = NxRegUtils::ReadString(GetNextPath());
		path = path ^ "setup.exe";

		if (!CreateProcess(NULL, (char *)(LPCSTR)path, NULL, NULL, false, 0, NULL, NULL, &si, &processInfo))
			return false;

		return true;
	}
	return false;
}
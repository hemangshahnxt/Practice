// regsvr.cpp : Program to invoke OLE self-registration on a DLL.
//
// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992-1998 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include "stdafx.h"
#include <ole2.h>
#include "RegSvr32.h"

const char _szDllRegSvr[] = "DllRegisterServer";
const char _szDllUnregSvr[] = "DllUnregisterServer";

int RegSvr32(LPCSTR pszDllName)
{
	int iReturn = 0;
	HRESULT (STDAPICALLTYPE * lpDllEntryPoint)(void);

	LPCSTR pszDllEntryPoint = _szDllRegSvr;

	// Initialize OLE.
	if (FAILED(OleInitialize(NULL))) {
		return REG_FAIL_OLE;
	}

	SetErrorMode(SEM_FAILCRITICALERRORS);       // Make sure LoadLib fails.

	// Load the library.
	HINSTANCE hLib = LoadLibraryEx(pszDllName, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);

	if (hLib < (HINSTANCE)HINSTANCE_ERROR) {
		iReturn = REG_FAIL_LOAD;
		goto CleanupOle;
	}

	// Find the entry point.
	(FARPROC&)lpDllEntryPoint = GetProcAddress(hLib, pszDllEntryPoint);

	if (lpDllEntryPoint == NULL) {
		iReturn = REG_FAIL_ENTRY;
		goto CleanupLibrary;
	}

	// Call the entry point.
	if (FAILED((*lpDllEntryPoint)())) {
		iReturn = REG_FAIL_REG;
		goto CleanupLibrary;
	}

CleanupLibrary:
	FreeLibrary(hLib);

CleanupOle:
	OleUninitialize();

	return iReturn;
}

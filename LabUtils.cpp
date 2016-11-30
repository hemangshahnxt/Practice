#include "stdafx.h"
#include "LabUtils.h"
#include "GlobalLabUtils.h"
#include "PrintUtils.h"
#include "NxCatch.h"

namespace LabUtils
{
	// Loads the lab label print settings and displays the print dialog
	void ShowLabLabelPrintSettings()
	{
		try {
			// (z.manning 2010-05-14 10:38) - PLID 37876 - Load the previous settings
			// (a.walling 2010-06-09 08:23) - PLID 37876 - Initialize HGLOBALS to NULL
			HGLOBAL hDevMode = NULL, hDevNames = NULL;
			// (z.manning 2010-11-22 10:41) - PLID 40486 - We now load from HKCU instead of the HKLM registry section.
			// Retrieve the label printer settings based on the 
			// System Property Specificity preference
			LabUtils::GetLabLabelPrintSettings(hDevNames, hDevMode);

			// (z.manning 2010-05-14 10:38) - PLID 37876 - Now popup the print settings dialog set to the old
			// settings. (If there weren't any previous settings it will use default print settings.)
			CPrintDialog dlgPrint(TRUE);
			dlgPrint.m_pd.hDevMode = (HANDLE)hDevMode;
			dlgPrint.m_pd.hDevNames = (HANDLE)hDevNames;

			int nResult = IDCANCEL;
			try
			{
				nResult = dlgPrint.DoModal();
			}
			// (z.manning 2010-05-14 10:08) - PLID 37876 - Based on code in CPracticeApp::DefaultPrintSetup
			NxCatchAllCallIgnore({
				MessageBox(GetActiveWindow(), "Could not load print settings. Please review your printer configuration by clicking Start -> Control Panel -> Printers.", NULL, MB_OK | MB_ICONERROR);
			});

			if (nResult == IDOK) {
				// (z.manning 2010-05-14 10:39) - PLID 37876 - Save the new settings
				// (z.manning 2010-11-22 12:16) - PLID 40486 - Now per user
				// Now stores the lab label printer settings based on the System Property Specificity preference.
				BOOL bSuccess = LabUtils::SaveLabLabelPrintSettings(dlgPrint.m_pd.hDevNames, dlgPrint.m_pd.hDevMode);
				if (!bSuccess) {
					// (z.manning 2010-09-28 16:15) - PLID 40722 - We now actually tell the user if this failed for any reason.
					// (Most likely cause is lack of registry permissions.)
					MessageBox(GetActiveWindow(), "Failed to save label printer settings. Please ensure you have permissions to modify the registry on this machine.", NULL, MB_OK | MB_ICONERROR);
				}
			}

			if (hDevMode != NULL) {
				GlobalFree(hDevMode);
				hDevMode = NULL;
			}
			if (hDevNames != NULL) {
				GlobalFree(hDevNames);
				hDevNames = NULL;
			}
		} NxCatchAll(__FUNCTION__);
	}

	// Stores the lab label printer settings based on the System Property Specificity preference.
	// - Server: Stored in the registry on the client or terminal server
	// - Client: Stored in ConfigRT based on the local machine name
	BOOL SaveLabLabelPrintSettings(HGLOBAL &hDevNames, HGLOBAL &hDevMode)
	{
		if (hDevNames == NULL || hDevMode == NULL) {
			return FALSE;
		}

		ESystemPropertySpecificity eSystemPropertySpecificity = (ESystemPropertySpecificity)GetRemotePropertyInt("SystemPropertySpecificity", (long)ESystemPropertySpecificity::Server, 0, "<None>", true);
		if (eSystemPropertySpecificity == ESystemPropertySpecificity::Client) {
			// Store the DevNames object in data
			DWORD dwDevNamesSize = GlobalSize(hDevNames);
			_variant_t varDevNames = DevObjectToVariant(hDevNames);

			// Store the DevMode object in data
			DWORD dwDevModeSize = GlobalSize(hDevMode);
			_variant_t varDevMode = DevObjectToVariant(hDevMode);

			if (varDevNames.vt == VT_NULL || varDevNames.vt == VT_EMPTY) {
				return FALSE;
			}

			if (varDevMode.vt == VT_NULL || varDevMode.vt == VT_EMPTY) {
				return FALSE;
			}

			// Write the DevNames data and data size to ConfigRT
			SetRemotePropertyImage("LabLabelPrintSettingsDevNames", varDevNames, 0, g_propManager.GetSystemName());
			SetRemotePropertyInt("LabLabelPrintSettingsDevNames", dwDevNamesSize, 0, g_propManager.GetSystemName());

			// Write the DevMode data and data size to ConfigRT
			SetRemotePropertyImage("LabLabelPrintSettingsDevMode", varDevMode, 0, g_propManager.GetSystemName());
			SetRemotePropertyInt("LabLabelPrintSettingsDevMode", dwDevModeSize, 0, g_propManager.GetSystemName());

			return TRUE;
		}

		// Store the label printer settings in the registry
		return ::SaveDevSettingsToRegistryHKCU(LAB_LABEL_PRINT_SETTINGS_REGISTRY_LOCATION, hDevNames, hDevMode);
	}

	// Retrieves the label printer settings based on the System Property Specificity preference.
	// - Server: Stored in the registry on the client or terminal server
	// - Client: Stored in ConfigRT based on the local machine name
	void GetLabLabelPrintSettings(OUT HGLOBAL &hDevNames, OUT HGLOBAL &hDevMode)
	{
		// First free the existing global memory if any exists
		if (hDevMode) {
			GlobalFree(hDevMode);
			hDevMode = NULL;
		}

		if (hDevNames) {
			GlobalFree(hDevNames);
			hDevNames = NULL;
		}

		ESystemPropertySpecificity eSystemPropertySpecificity = (ESystemPropertySpecificity)GetRemotePropertyInt("SystemPropertySpecificity", (long)ESystemPropertySpecificity::Server, 0, "<None>", true);
		if (eSystemPropertySpecificity == ESystemPropertySpecificity::Client) {
			// Load DEVNAMES object from data
			_variant_t varDevNames = GetRemotePropertyImage("LabLabelPrintSettingsDevNames", 0, g_propManager.GetSystemName(), true);
			DWORD dwDevNamesSize = static_cast<DWORD>(GetRemotePropertyInt("LabLabelPrintSettingsDevNames", 0, 0, g_propManager.GetSystemName(), true));

			if (dwDevNamesSize > sizeof(DEVNAMES)) {
				COleSafeArray saDevNames;
				_variant_t varSafeDevNames(varDevNames);
				saDevNames.Attach(varSafeDevNames.Detach());

				DEVNAMES *pSrcDevNames = NULL;
				saDevNames.AccessData((LPVOID *)&pSrcDevNames);

				// Allocate a global handle for the data
				hDevNames = GlobalAlloc(GHND, dwDevNamesSize);
				DEVNAMES *pDstDevNames = reinterpret_cast<DEVNAMES *>(GlobalLock(hDevNames));

				// Copy the DEVNAMES object
				memcpy(pDstDevNames, pSrcDevNames, dwDevNamesSize);

				// Unlock the global handle
				::GlobalUnlock(hDevNames);

				// Decrement the lock count and invalidate the pointer
				saDevNames.UnaccessData();
			}

			// Load DEVMODE object from data
			_variant_t varDevMode = GetRemotePropertyImage("LabLabelPrintSettingsDevMode", 0, g_propManager.GetSystemName(), true);
			DWORD dwDevModeSize = static_cast<DWORD>(GetRemotePropertyInt("LabLabelPrintSettingsDevMode", 0, 0, g_propManager.GetSystemName(), true));

			if (dwDevModeSize > sizeof(DEVMODE)) {
				COleSafeArray saDevMode;
				_variant_t varSafeDevMode(varDevMode);
				saDevMode.Attach(varSafeDevMode.Detach());

				DEVMODE *pSrcDevMode = NULL;
				saDevMode.AccessData((LPVOID *)&pSrcDevMode);

				// Allocate a global handle for the data
				hDevMode = GlobalAlloc(GHND, dwDevModeSize);
				DEVMODE *pDstDevMode = (DEVMODE *)GlobalLock(hDevMode);

				// Copy the DEVMODE object
				memcpy(pDstDevMode, pSrcDevMode, dwDevModeSize);

				// Unlock the global handle
				::GlobalUnlock(hDevMode);

				// Decrement the lock count and invalidate the pointer
				saDevMode.UnaccessData();
			}
		}
		else {
			// When the System Property Specificity preference is set to Terminal Server, retrieve 
			// the label printer settings from the registry
			GetLabLabelPrintSettingsFromRegistry(hDevNames, hDevMode);
		}
	}

	// Returns a _variant_t based on the provided HGLOBAL
	_variant_t DevObjectToVariant(const HGLOBAL &hDevObject)
	{
		COleSafeArray saDevObject;
		DWORD dwDevObjectSize = GlobalSize(hDevObject);
		BYTE *pDevObject = reinterpret_cast<BYTE *>(GlobalLock(hDevObject));

		if (dwDevObjectSize != 0) {
			// Create the array of the right size
			saDevObject.CreateOneDim(VT_UI1, dwDevObjectSize, pDevObject);
		}
		else {
			// Setup the bounds and create the empty array
			SAFEARRAYBOUND rgsabound;
			rgsabound.cElements = 0;
			rgsabound.lLbound = 0;
			saDevObject.Create(VT_UI1, 1, &rgsabound);
		}

		::GlobalUnlock(hDevObject);
		return _variant_t(saDevObject.Detach(), false);
	}
}
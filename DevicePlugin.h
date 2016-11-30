#pragma once

// (b.savon 2011-9-1) - PLID 45288 - Device Plugin Settings
#include "DevicePluginSettings.h"

// (a.walling 2014-04-24 12:00) - VS2013 - MSXML Import
#include <NxDataUtilitiesLib/NxMsxml6Import.h>

#include "DevicePluginUtils.h"

// (j.jones 2010-05-11 10:19) - PLID 38591 - created

//This class will take in a given .nxdp plugin file, load it, and encapsulate it into a pointer.
//Deleting the pointer will unload the plugin.

//Process takes in strFolderName which tells us where the files are that we need to process,
//It then constructs the XML document and returns TRUE if it succeeded.
//If a problem occurred, it returns FALSE and fills strErrorDescription.
//BOOL Process(const CString IN strFolderName, IN OUT MSXML2::IXMLDOMDocument2Ptr Doc, OUT CString &strErrorDescription);
typedef BOOL (*DEVICE_PLUGIN_PROCESS_FUNC)(const CString, MSXML2::IXMLDOMDocument2Ptr, CString&);

//GetPluginDescription returns the official full plugin description that we display to clients in the interface
//LPCTSTR GetPluginDescription();
typedef LPCTSTR (*DEVICE_PLUGIN_GETDESC_FUNC)();

//GetLaunchDeviceSettings returns a LaunchDeviceSettings enum (cast as an int) to tell the caller whether
//the LaunchDevice() functionality is supported, and if so, what parameters are required
//int GetLaunchDeviceSettings();
typedef int (*DEVICE_PLUGIN_GETLAUNCHDEVICESETTINGS_FUNC)();

//LaunchDevice takes in a dock key (like "Main"), a PatientID (PersonID),
//and optionally a path to the device exe and/or a path to an output folder
//If the return value is FALSE, strErrorDescription should explain why.
//BOOL LaunchDevice(const CString IN strDockKey, const long IN nPatientID, OPTIONAL IN CString strExePath, OPTIONAL IN CString strOutputFolderPath, OUT CString &strErrorDescription);
typedef BOOL (*DEVICE_PLUGIN_LAUNCHDEVICE_FUNC)(const CString, const long, CString, CString, CString&);

class CDevicePlugin
{
public:
	CDevicePlugin(CString strPath, CString strFileName);
	~CDevicePlugin(void);

	//Process takes in strFolderName which tells us where the files are that we need to process,
	//It then constructs the XML document and returns TRUE if it succeeded.
	//If a problem occurred, it returns FALSE and fills strErrorDescription.
	BOOL Process(const CString IN strFolderName, IN OUT MSXML2::IXMLDOMDocument2Ptr Doc, OUT CString &strErrorDescription);

	//GetPluginDescription returns the official full plugin description that we display to clients in the interface
	CString GetPluginDescription();

	//GetLaunchDeviceSettings returns a LaunchDeviceSettings enum (cast as an int) to tell the caller whether
	//the LaunchDevice() functionality is supported, and if so, what parameters are required
	DevicePluginUtils::LaunchDeviceSettings GetLaunchDeviceSettings();

	//LaunchDevice takes in a dock key (like "Main"), a PatientID (PersonID),
	//and optionally a path to the device exe and/or a path to an output folder
	//If the return value is FALSE, strErrorDescription should explain why.
	BOOL LaunchDevice(const CString IN strDockKey, const long IN nPatientID,
		OPTIONAL IN CString strExePath, OPTIONAL IN CString strOutputFolderPath,
		OUT CString &strErrorDescription);

	BOOL m_bLoaded;			//TRUE if the module successfully loaded, FALSE otherwise
	CString m_strLoadError;	//only filled if m_bLoaded == FALSE, will tell us why it failed to load

	// (b.savon 2011-9-1) - PLID 45288 - Device Plugin Settings
	CDevicePluginSettings m_cDevicePluginSettings;

protected:

	HINSTANCE m_hPlugin;	//the handle to the loaded plugin file

	DEVICE_PLUGIN_PROCESS_FUNC m_pProcess;					//takes in a path to scan, generates XML
	DEVICE_PLUGIN_GETDESC_FUNC m_pGetPluginDescription;		//returns the plugin description
	DEVICE_PLUGIN_GETLAUNCHDEVICESETTINGS_FUNC m_pGetLaunchDeviceSettings;	//returns a LaunchDeviceSettings enum
	DEVICE_PLUGIN_LAUNCHDEVICE_FUNC m_pLaunchDevice;		//Launches the device application
};

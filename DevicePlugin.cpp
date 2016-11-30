#include "stdafx.h"
#include "DevicePlugin.h"
#include "PathStringUtils.h"

// (j.jones 2010-05-11 10:19) - PLID 38591 - created

//This class will take in a given .nxdp plugin file, load it, and encapsulate it into a pointer.
//Deleting the pointer will unload the plugin.

CDevicePlugin::CDevicePlugin(CString strPath, CString strFileName)
{
	m_bLoaded = FALSE;

	if(strPath.IsEmpty() || strFileName.IsEmpty()) {
		m_strLoadError.Format("Invalid file path (%s) and file name (%s)", strPath, strFileName);
		return;
	}

	m_hPlugin = LoadLibrary(strPath ^ strFileName);	
	ASSERT(m_hPlugin);
	if(!m_hPlugin) {
		m_strLoadError.Format("Error loading device plugin file (Error Code %li).", GetLastError());
		return;
	}

	m_pProcess = (DEVICE_PLUGIN_PROCESS_FUNC)GetProcAddress(m_hPlugin, "Process");
	ASSERT(m_pProcess);
	if(!m_pProcess) {
		m_strLoadError.Format("Error binding to Process() (Error Code %li)", GetLastError());
		return;
	}
	
	m_pGetPluginDescription = (DEVICE_PLUGIN_GETDESC_FUNC)GetProcAddress(m_hPlugin, "GetPluginDescription");
	ASSERT(m_pGetPluginDescription);
	if(!m_pGetPluginDescription) {
		m_strLoadError.Format("Error binding to GetPluginDescription() (Error Code %li)", GetLastError());
		return;
	}

	m_pGetLaunchDeviceSettings = (DEVICE_PLUGIN_GETLAUNCHDEVICESETTINGS_FUNC)GetProcAddress(m_hPlugin, "GetLaunchDeviceSettings");
	ASSERT(m_pGetLaunchDeviceSettings);
	if(!m_pGetLaunchDeviceSettings) {
		m_strLoadError.Format("Error binding to GetLaunchDeviceSettings() (Error Code %li)", GetLastError());
		return;
	}

	m_pLaunchDevice = (DEVICE_PLUGIN_LAUNCHDEVICE_FUNC)GetProcAddress(m_hPlugin, "LaunchDevice");
	ASSERT(m_pLaunchDevice);
	if(!m_pLaunchDevice) {
		m_strLoadError.Format("Error binding to LaunchDevice() (Error Code %li)", GetLastError());
		return;
	}

	m_bLoaded = TRUE;
	m_strLoadError = "";

	// (b.savon 2011-9-1) - PLID 45288 - Device Plugin Settings
	int nLaunchSettings = m_pGetLaunchDeviceSettings();
	m_cDevicePluginSettings.SetLaunchDeviceSettings( nLaunchSettings );
}

CDevicePlugin::~CDevicePlugin()
{
	if(m_hPlugin) {
		FreeLibrary(m_hPlugin);
		m_hPlugin = NULL;

		m_bLoaded = FALSE;
		m_strLoadError = "Unloaded in ~CDevicePlugin";
	}
}

//Process takes in strFolderName which tells us where the files are that we need to process,
//It then constructs the XML document and returns TRUE if it succeeded.
//If a problem occurred, it returns FALSE and fills strErrorDescription.
BOOL CDevicePlugin::Process(const CString IN strFolderName, IN OUT MSXML2::IXMLDOMDocument2Ptr Doc, OUT CString &strErrorDescription)
{
	//this function cleanly encapsulates the plugin's Process function, and exposes its parameter list

	if(m_bLoaded && m_pProcess) {
		return m_pProcess(strFolderName, Doc, strErrorDescription);
	}
	else {
		strErrorDescription = "Plugin not loaded properly.";
		return FALSE;
	}
}

//GetPluginDescription returns the official full plugin description that we display to clients in the interface
CString CDevicePlugin::GetPluginDescription()
{
	//this function encapsulates the plugin's GetPluginDescription function
	if(m_bLoaded && m_pGetPluginDescription) {
		return (CString)m_pGetPluginDescription();
	}
	else {
		return "";
	}
}

// (b.savon 2011-9-1) - PLID 45288 - This function is ***DEPRECATED***  
//	Use the pPlugin->DeviceSettings.XYZ() for all future references to settings
//	of the device plugin.
//
//	We need to bitwise AND the GetLaunchDeviceSettings because the Enum has
//	4 values (1-4).  This requires the 3 LSBs of an INT.  Thus, we need to bitwise
//	AND the result with 7 (0b111) so that we get the launch settings of result.
//	For example:  GetLaunchDeviceSettings could return 11 (0b1011).  Since, the Enum
//				  defintion has Supported_Recursion = 8 as a flag, this doesn't have
//				  any bearing on the LaunchSettings for the device.  Instead of taking
//				  11 as the value returned from GetLaunchDeviceSettings, we are only
//				  interested in the last 3 bits. (The following example uses 8 bits for
//				  simplicity.
//
//					0b00001011 (11)
//				  & 0b00000111 (7)
//				  ------------
//					0b00000011 (3) 
//				  Thus, the launch settings for this device, depicted by the Enum definition
//				  in DevicePluginUtils::enum LaunchDeviceSettings is: 
//				  ldsSupported_NeedFolderOnly = 3
//
//	Bits 4-31 are now reserved for further flag settings and will be described in
//	another class -> CDeviceSettings

//GetLaunchDeviceSettings returns a LaunchDeviceSettings enum (cast as an int) to tell the caller whether
//the LaunchDevice() functionality is supported, and if so, what parameters are required
DevicePluginUtils::LaunchDeviceSettings CDevicePlugin::GetLaunchDeviceSettings()
{
	//this function encapsulates the plugin's GetLaunchDeviceSettings function
	if(m_bLoaded && m_pGetLaunchDeviceSettings) {
		return (DevicePluginUtils::LaunchDeviceSettings)(m_pGetLaunchDeviceSettings()&7);
	}
	else {
		return (DevicePluginUtils::LaunchDeviceSettings)(DevicePluginUtils::ldsNotSupported);
	}
}

//LaunchDevice takes in a dock key (like "Main"), a PatientID (PersonID),
//and optionally a path to the device exe and/or a path to an output folder
//If the return value is FALSE, strErrorDescription should explain why.
BOOL CDevicePlugin::LaunchDevice(const CString IN strDockKey, const long IN nPatientID,
											   OPTIONAL IN CString strExePath, OPTIONAL IN CString strOutputFolderPath,
											   OUT CString &strErrorDescription)
{
	//this function encapsulates the plugin's LaunchDevice function
	if(m_bLoaded && m_pLaunchDevice) {
		return m_pLaunchDevice(strDockKey, nPatientID, strExePath, strOutputFolderPath, strErrorDescription);
	}
	else {
		return FALSE;
	}
}
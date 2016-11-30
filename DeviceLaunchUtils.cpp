//DeviceLaunchUtils.cpp

// (j.gruber 2013-04-04 15:18) - PLID 56012 - created
#include "StdAfx.h"
#include "DevicePluginUtils.h"
#include "PathStringUtils.h"
#include "GlobalDataUtils.h"
#include "FileUtils.h"
#include "DeviceLaunchUtils.h"


namespace DeviceLaunchUtils {

	/* This Function generates a menu based on the plugin array and will add it to the menu you pass in 
	Parameters: 
		CArray<DevicePlugin*, DevicePlugin*> &aryLoadedPlugins - an array of the plugins available to this instance of Practice, some may be launchable, others not
		CMenu *pMenu - a pointer to the menu you would like the launchable devices added to
		long &nIndex IN OUT - the index of where in the menu you would like to add the next device, incremented as devices are added
		BOOL bUseIndex - whether to use the index given, or just add to the end of the menu
		long nFlags - any flags you would like to pass into the menu, only used when adding indexes
		BOOL bUseSeparator  - whether to use a separator between the plugins, when there are multiple plugins
	*/
	void GenerateDeviceMenu(CArray<DevicePlugin*, DevicePlugin*> &aryLoadedPlugins, CMenu *pMenu, long &nIndex, BOOL bUseIndex, long nFlags, BOOL bUseSeparator /*= TRUE*/)
	{
		try {
			// (d.lange 2010-11-03 12:26) - PLID 41211 - add menu items for every device plugin thats enabled and has the ability to send to devic		
			if (g_pLicense->CheckForLicense(CLicense::lcDeviceImport, CLicense::cflrSilent)) 
			{
				// (r.gonet 2016-05-20 3:39) - NX-100691 - Use the client's machine name.
				ADODB::_RecordsetPtr rs = CreateParamRecordset("SELECT Enable, PluginOverrideName, PluginFileName, DeviceExecutablePath, DeviceAdditionalPath FROM DevicePluginConfigT "
													"WHERE Enable = 1 AND ComputerName = {STRING}; \r\n", 
					g_propManager.GetSystemName());

				CString strPath = NxRegUtils::ReadString(GetRegistryBase() + "InstallPath") ^ "DevicePlugins\\";
				BOOL bMultiPlugins = FALSE;
				while(!rs->eof) {
					if(DoesExist(strPath)) {
						CDevicePlugin *pPlugin = new CDevicePlugin(strPath, AdoFldString(rs, "PluginFileName"));
						if(pPlugin) {
							// (j.gruber 2013-04-02 13:00) - PLID 53032 - use our new function						
							if(IsLaunchableDevice(pPlugin)) {
								if (bUseSeparator) {
									if(!bMultiPlugins) {
										if (bUseIndex) {
											pMenu->InsertMenu(nIndex++, MF_BYPOSITION|MF_SEPARATOR);
										}
										else {
											pMenu->AppendMenu(MF_SEPARATOR, 0, "");
										}
									}
								}

								CString strOverrideName = AdoFldString(rs, "PluginOverrideName", "");

								if (bUseIndex) {
									pMenu->InsertMenu(nIndex++, nFlags, (long)pPlugin, FormatString("Send to &%s"
										, (!strOverrideName.IsEmpty() ? strOverrideName : pPlugin->GetPluginDescription())));
								}
								else {
									pMenu->AppendMenu(MF_BYPOSITION|MF_ENABLED, (long)pPlugin, FormatString("Send to &%s"
										, (!strOverrideName.IsEmpty() ? strOverrideName : pPlugin->GetPluginDescription())));
								}
								DevicePlugin *dPluginInfo = new DevicePlugin;
								
								dPluginInfo->strExePath = AdoFldString(rs, "DeviceExecutablePath");
								dPluginInfo->strAdditionalPath = AdoFldString(rs, "DeviceAdditionalPath", "");
								dPluginInfo->pdPlugin = pPlugin;
								dPluginInfo->nPlugin = (long)pPlugin;
								dPluginInfo->strOverrideName = strOverrideName;

								aryLoadedPlugins.Add(dPluginInfo);
								bMultiPlugins = TRUE;
							}else {
								delete pPlugin;
							}
						}
					}
					rs->MoveNext();
				}
				rs->Close();
			}

		}NxCatchAll(__FUNCTION__);
	}

	/* (j.gruber 2013-04-02 13:00) - PLID 56032 - lets have a function for this to make it easy to add future ones
	//ldsSupported_NeedExeOnly and ldsSupported_NeedFolderOnly were always used in pre-consolidated code, I added ldsSupported_NeedExeAndFolder since
	//it should've always been there, but never was, and ldsSupported_NeedURLOnly since it was new*/
	BOOL IsLaunchableDevice(CDevicePlugin *pPlugin)
	{
		long nDeviceSettings = pPlugin->GetLaunchDeviceSettings();
		if (nDeviceSettings == DevicePluginUtils::ldsSupported_NeedExeOnly ||
			nDeviceSettings == DevicePluginUtils::ldsSupported_NeedFolderOnly ||			
			// (j.gruber 2013-04-22 12:37) - PLID 56012 - Don doesn't want this in until a plugin actualy uses it.
			//nDeviceSettings == DevicePluginUtils::ldsSupported_NeedExeAndFolder ||
			nDeviceSettings == DevicePluginUtils::ldsSupported_NeedURLOnly) {
				return TRUE;
		}
		else {
			return FALSE;
		}
	
	}

	/*Call this function to actually launch the device plugin
	Parameters:
		CArray<DevicePlugin*, DevicePlugin*> &aryLoadedPlugins - array of device plugins
		int nCmdId - the selection of the menu the user chose
		long nPatientID - the patient you want to send to the device
	Return Value:  Whether the nCmdID is actually one of the loaded plugins, NOT whether the device launched successfully 
	*/
	BOOL LaunchDevice(CArray<DevicePlugin*, DevicePlugin*> &aryLoadedPlugins, int nCmdId, long nPatientID)
	{
		try {
			CDevicePlugin *pResultPlugin = (CDevicePlugin*)nCmdId;
			if(pResultPlugin) {
				//Iterate through the array of loaded plugins to find a match and call the 'Launch Device' function
				for(int i = 0; i < aryLoadedPlugins.GetSize(); i++) {
					// (j.gruber 2013-04-22 10:19) - PLID 56012 - One instance checked that the plugin name matched, I don't see a reason to put that constraint here, since the other places didn't have it
					if(aryLoadedPlugins[i]->nPlugin == (long)pResultPlugin) {
						// (j.gruber 2013-04-04 13:38) - PLID 56032 - reconfigure this 
						long nDeviceSettings = aryLoadedPlugins[i]->pdPlugin->GetLaunchDeviceSettings();
						BOOL bCheckExePath = FALSE;
						BOOL bCheckAdditionalPath = FALSE;
						BOOL bIsUrl = FALSE;
						switch (nDeviceSettings) {

							case DevicePluginUtils::ldsSupported_NeedExeOnly:						
								bCheckExePath = TRUE;							
							break;
							
							case DevicePluginUtils::ldsSupported_NeedFolderOnly:
								bCheckAdditionalPath = TRUE;
							break;

							case DevicePluginUtils::ldsSupported_NeedExeAndFolder:
								bCheckExePath = TRUE;
								bCheckAdditionalPath = TRUE;
							break;

							case DevicePluginUtils::ldsSupported_NeedURLOnly:
								bCheckExePath = TRUE;						
								bIsUrl = TRUE;
							break;

						}

						if (bCheckExePath) {
							//Let's check to see if the executable path is null
							if(aryLoadedPlugins[i]->strExePath.IsEmpty()) {
								AfxMessageBox(FormatString("The Device %s Path has not been configured for the Device Plugin: \r\n\r\n%s\r\n\r\n"
												"Please configure this path and try again.",
												bIsUrl ? "URL" : "Executable",
												(!aryLoadedPlugins[i]->strOverrideName.IsEmpty() ? aryLoadedPlugins[i]->strOverrideName : aryLoadedPlugins[i]->pdPlugin->GetPluginDescription())));
								// (d.lange 2010-01-11 17:49) - PLID 41211 - Now returns TRUE since it did find the loaded plugin, its just not configured properly
								return TRUE;						
							}
						}

						if (bCheckAdditionalPath) {
							//Let's check to see if the additional path is null
							if(aryLoadedPlugins[i]->strAdditionalPath.IsEmpty()) {
								AfxMessageBox(FormatString("The Device Additional Path has not been configured for the Device Plugin: \r\n\r\n%s\r\n\r\n"
												"Please configure this path and try again.", 
												(!aryLoadedPlugins[i]->strOverrideName.IsEmpty() ? aryLoadedPlugins[i]->strOverrideName : aryLoadedPlugins[i]->pdPlugin->GetPluginDescription())));
								return TRUE;
							}
						}

						CString strErrorDesc = "";
						// (b.savon 2012-02-07 15:45) - PLID 48019 - If this is the Topcon Synergy WebLink plugin
						// then let's send the current username in the 'AdditionalExportPath' field as a custom plugin launch parameter
						CString strPlugin = aryLoadedPlugins[i]->pdPlugin->GetPluginDescription();
						BOOL bTopconSynergyWebLink = strPlugin.CompareNoCase("Topcon Synergy WebLink") == 0;
						//Here is where we actually do the magic
						aryLoadedPlugins[i]->pdPlugin->LaunchDevice(GetSubRegistryKey(), nPatientID, aryLoadedPlugins[i]->strExePath, bTopconSynergyWebLink ? GetCurrentUserName() : aryLoadedPlugins[i]->strAdditionalPath, strErrorDesc);
						return TRUE;
					}
				}
			}			
		}NxCatchAll(__FUNCTION__);
		return FALSE;
	}

	/*Call this function to detroy the memory from the array
	Parameters:
		CArray<DevicePlugin*, DevicePlugin*> &aryLoadedPlugins - the array of plugins to destroy
	*/
	void DestroyLoadedDevicePlugins(CArray<DevicePlugin*, DevicePlugin*> &aryLoadedPlugins)
	{
		try {
			//Let's iterate through the array of loaded plugins and unload them
			for(int i = 0; i < aryLoadedPlugins.GetSize(); i++) {
				DevicePlugin *pLoadedInfo = aryLoadedPlugins[i];
				CDevicePlugin *pLoadedPlugin = pLoadedInfo->pdPlugin;
				delete pLoadedPlugin;
				delete pLoadedInfo;
			}
		}NxCatchAll(__FUNCTION__);
	}

	/*This function will go through all the devices enabled for this user and return true if any are capable of launching a device*/
	BOOL HasLaunchableDevice()
	{
		try {
			BOOL bDevices = FALSE;

			// (c.haag 2010-06-30 15:27) - PLID 39424 - Added license checking
			if (!g_pLicense->CheckForLicense(CLicense::lcDeviceImport, CLicense::cflrSilent)) {
				return FALSE;
			}		

			// (r.gonet 2016-05-20 3:39) - NX-100691 - Use the client's machine name.
			ADODB::_RecordsetPtr rs = CreateParamRecordset("SELECT Enable, PluginFileName, DeviceExecutablePath, DeviceAdditionalPath FROM DevicePluginConfigT "
													"WHERE Enable = 1 AND ComputerName = {STRING}; \r\n", 
				g_propManager.GetSystemName());
			CDevicePlugin *pPlugin;
			CString strPath = NxRegUtils::ReadString(GetRegistryBase() + "InstallPath") ^ "DevicePlugins\\";
			while(!rs->eof) {
				if(DoesExist(strPath)) {
					pPlugin = new CDevicePlugin(strPath, AdoFldString(rs, "PluginFileName"));
					if(pPlugin) {					
						if(IsLaunchableDevice(pPlugin)) {	// (d.lange 2010-10-22 17:45) - PLID 41078 - Check for plugin that enables the additional path
							//We found at least one so lets return true!
							bDevices = TRUE;
							delete pPlugin;
							break;
						}
					}
					delete pPlugin;
				}
				rs->MoveNext();
			}
			rs->Close();

			return bDevices;
		}NxCatchAll(__FUNCTION__);

		return FALSE;

	}
};
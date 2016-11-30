//DeviceLaunchUtils.h
// (j.gruber 2013-04-04 15:18) - PLID 56012 - created

#ifndef _DEVICELAUNCHUTILS_H
#define _DEVICELAUNCHUTILS_H

#pragma once

#include "DevicePlugin.h"

namespace DeviceLaunchUtils {

	struct DevicePlugin{
		CString strExePath;
		CString strOverrideName;
		CString strAdditionalPath;
		long nPlugin;
		CDevicePlugin *pdPlugin;
	};

	void GenerateDeviceMenu(CArray<DevicePlugin*, DevicePlugin*> &aryLoadedPlugins, CMenu *pMenu, long &nIndex, BOOL bUseIndex, long nFlags, BOOL bUseSeparator = TRUE);
	BOOL IsLaunchableDevice(CDevicePlugin *pPlugin);
	BOOL LaunchDevice(CArray<DevicePlugin*, DevicePlugin*> &aryLoadedPlugins, int nCmdId, long nPatientID);
	void DestroyLoadedDevicePlugins(CArray<DevicePlugin*, DevicePlugin*> &aryLoadedPlugins);
	BOOL HasLaunchableDevice();
}


#endif
// (b.savon 2011-9-1) - PLID 45288 - Device Plugin Settings

#include "stdafx.h"
#include "DevicePluginSettings.h"
#include "DevicePluginUtils.h"

using namespace DevicePluginUtils;

CDevicePluginSettings::CDevicePluginSettings(int nLaunchDeviceSettings)
{
	m_nLaunchDeviceSettings = nLaunchDeviceSettings;
}

CDevicePluginSettings::CDevicePluginSettings( void )
{
}

void CDevicePluginSettings::SetLaunchDeviceSettings( int nLaunchDeviceSettings )
{
	m_nLaunchDeviceSettings = nLaunchDeviceSettings;
}

BOOL CDevicePluginSettings::LaunchNotSupported(){	
	return ((m_nLaunchDeviceSettings & 7) == ldsNotSupported) ? TRUE : FALSE;	
}

BOOL CDevicePluginSettings::LaunchParamsOnly(){	
	return ((m_nLaunchDeviceSettings & 7) == ldsSupported_NoParams) ? TRUE : FALSE;
}

BOOL CDevicePluginSettings::LaunchExeOnly(){	
	return ((m_nLaunchDeviceSettings & 7) == ldsSupported_NeedExeOnly) ? TRUE : FALSE;
}

BOOL CDevicePluginSettings::LaunchFolderOnly(){	
	return ((m_nLaunchDeviceSettings & 7) == ldsSupported_NeedFolderOnly) ? TRUE : FALSE ;
}

BOOL CDevicePluginSettings::LaunchExeAndFolder(){	
	return ((m_nLaunchDeviceSettings & 7) == ldsSupported_NeedExeAndFolder) ? TRUE : FALSE;	
}

// (j.gruber 2013-04-04 16:07) - PLID 56032
BOOL CDevicePluginSettings::LaunchLinkOnly(){	
	return ((m_nLaunchDeviceSettings & 7) == ldsSupported_NeedURLOnly) ? TRUE : FALSE;	
}

BOOL CDevicePluginSettings::SupportsRecursion(){	
	return ((m_nLaunchDeviceSettings & 8) == ldsSupported_Recursion) ? TRUE : FALSE;
}
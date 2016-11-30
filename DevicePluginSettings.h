// (b.savon 2011-9-1) - PLID 45288 - Device Plugin Settings

#pragma once

class CDevicePluginSettings
{
private:
	int m_nLaunchDeviceSettings;

	//	Define Additional Flags Here
	BOOL m_bSupportsRecursion;

public:
	CDevicePluginSettings( void );
	CDevicePluginSettings(int nLaunchDeviceSettings);

	void SetLaunchDeviceSettings( int nLaunchDeviceSettings );

	BOOL LaunchNotSupported();
	BOOL LaunchParamsOnly();
	BOOL LaunchExeOnly();
	BOOL LaunchFolderOnly();
	BOOL LaunchExeAndFolder();
	BOOL LaunchLinkOnly(); // (j.gruber 2013-04-04 16:07) - PLID 56032

	//	Define Additional Flag Functions Here
	BOOL SupportsRecursion();
};

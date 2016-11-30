#pragma once

// (j.jones 2009-03-10 08:58) - PLID 32405 - LoadDialerSettings will load
// all the settings and put them into the passed-in variables
// (j.jones 2009-08-14 10:38) - PLID 34914 - added OHIP per-provider setting
// (s.dhole 2011-01-17 12:00) - PLID  42145  added strNXOhipModemLocation, bTeraTerm
// (b.spivey, June 26th, 2014) - PLID 62603 - Added bEnableAutoDial. 
void LoadDialerSettings(CString &strPhoneNumber, CString &strUserName, CString &strPassword,
						long &nCommPort, CString &strDownloadPath, CString &strTTLocation,
						BOOL &bDeleteReports, BOOL &bChangePassword, CString &strNewPassword,
						BOOL &bShowTerminalWindow, BOOL &bSeparateAccountsPerProviders,
						CString &strNXOhipModemLocation, BOOL &bTeraTerm, bool &bEnableAutoDial);

// (j.jones 2009-09-15 14:01) - PLID 33955 - added helper functions to help migrate from
// loading from the registry to loading from ConfigRT
long LoadDialerSettingLong(CString strConfigRTName, CString strRegistryLocation,
						   long nDefaultValue, CString strUserName, BOOL &bLoadedFromRegistry);
CString LoadDialerSettingString(CString strConfigRTName, CString strRegistryLocation,
						   CString strDefaultValue, CString strUserName, BOOL &bLoadedFromRegistry);

// (j.jones 2009-08-14 14:49) - PLID 34194 - added function to load provider login info from data
void LoadProviderLoginInfo(long nProviderID, CString &strUserName, CString &strPassword, BOOL &bChangePassword, CString &strNewPassword);

// (j.jones 2009-04-03 12:47) - PLID 33846 - generate a new password
// (j.jones 2009-08-14 11:27) - PLID 34914 - added optional provider ID parameter
CString CalcNewOHIPPassword(CString strOldPassword, long nCalculateForProviderID);

// (j.jones 2009-04-03 12:47) - PLID 33846 - check password validity
// (j.jones 2009-08-14 11:27) - PLID 34914 - renamed to be different from the per-provider ability
BOOL HasOHIPPasswordBeenUsed_Workstation(CString strNewPassword, CString strOldPassword);

// (j.jones 2009-08-14 11:25) - PLID 34914 - added a per-provider option
BOOL HasOHIPPasswordBeenUsed_Provider(long nProviderID, CString strNewPassword, CString strOldPassword);

// (j.jones 2009-03-09 16:43) - PLID 33419 - GenerateDownloadScriptTemplate will
// generate our default template file with {NX$} placeholders for configurable data,
// which we will later replace with our dialer settings.
// The return value is the full file location of the generated template.
CString GenerateDownloadScriptTemplate();

// (j.jones 2009-03-09 17:06) - PLID 33419 - PrepareDownloadScriptFromTemplate will
// open a template file that has {NX$} placeholders, replace them with our configured
// data, and save the result as a temp file for TeraTerm to use as its macro.
// The return value is the full file location of the ready-to-use macro file.
CString PrepareDownloadScriptFromTemplate(CString strTemplatePath,
							CString strPhoneNumber, CString strUserName, CString strPassword,
							long nCommPort, CString strDownloadPath, CString strTTLocation,
							BOOL bDeleteReports, BOOL bChangePassword, CString strNewPassword,
							BOOL bShowTerminalWindow);
// (s.dhole 2011-01-17 17:50) - PLID 42145 CommadString 
CString PrepareModemUploadCommandString(CString strClaimFilePath,
							CString strPhoneNumber, CString strUserName, CString strPassword,
							long nCommPort, BOOL bChangePassword, CString strNewPassword,
							BOOL bShowTerminalWindow);
// (s.dhole 2011-01-17 17:50) - PLID 42145 CommadString 
CString PrepareModemDownloadCommandString(CString strPhoneNumber, CString strUserName, CString strPassword,
							long nCommPort, CString strDownloadPath, 
							BOOL bDeleteReports, BOOL bChangePassword, CString strNewPassword,
							BOOL bShowTerminalWindow);
// (j.jones 2009-03-10 09:23) - PLID 33419 - DownloadReportsFromOHIP will prepare the
// script file, launch teraterm, and download the reports.
// The return value is -1 if a problem occurred, 0 if no reports downloaded, 1 if reports were downloaded.
// (j.jones 2009-08-14 11:37) - PLID 34914 - takes in a provider ID, optionally
long DownloadReportsFromOHIP(long nProviderID);

// (j.jones 2009-03-10 09:32) - PLID 33419 - LaunchTeraTerm will launch the teraterm program
// with our passed-in macro file
DWORD LaunchTeraTerm(CString strExecutablePath, CString strMacroFile);

// (s.dhole 2011-01-17 14:31) - PLID  42145 Call NXmodem 
DWORD LaunchNXModem(CString  strExecutablePath, CString  strCommandString);

// (j.jones 2009-03-10 09:39) - PLID 33419 - UpdateOldOHIPPasswordWithNew will be called after
// a connection that changes the password succeeds, and will clear the change password flag,
// clear the new password field, and save the new password as the current password
// (j.jones 2009-04-03 14:38) - PLID 33846 - added strOldPassword
// (j.jones 2009-08-14 11:27) - PLID 34914 - renamed to be different from the per-provider ability
void UpdateOldOHIPPasswordWithNew_Workstation(CString strNewPassword, CString strOldPassword);

// (j.jones 2009-08-14 11:25) - PLID 34914 - added a per-provider option
void UpdateOldOHIPPasswordWithNew_Provider(long nProviderID, CString strNewPassword, CString strOldPassword);

// (j.jones 2009-03-10 12:48) - PLID 33418 - GenerateUploadScriptTemplate will
// generate our default template file with {NX$} placeholders for configurable data,
// which we will later replace with our dialer settings.
// The return value is the full file location of the generated template.
CString GenerateUploadScriptTemplate();

// (j.jones 2009-03-10 12:48) - PLID 33418 - PrepareUploadScriptFromTemplate will
// open a template file that has {NX$} placeholders, replace them with our configured
// data, and save the result as a temp file for TeraTerm to use as its macro.
// The return value is the full file location of the ready-to-use macro file.
CString PrepareUploadScriptFromTemplate(CString strTemplatePath, CString strClaimFilePath,
							CString strPhoneNumber, CString strUserName, CString strPassword,
							long nCommPort, CString strTTLocation,
							BOOL bChangePassword, CString strNewPassword,
							BOOL bShowTerminalWindow);

// (j.jones 2009-03-10 12:48) - PLID 33419 - SendClaimFileToOHIP will prepare the
// script file, launch teraterm, and send the claim file in strClaimFilePath
// The return value is FALSE if a problem occurred, TRUE if the submission succeeded.
// (j.jones 2009-08-14 11:37) - PLID 34914 - takes in a provider ID, optionally
BOOL SendClaimFileToOHIP(CString strClaimFilePath, long nProviderID);
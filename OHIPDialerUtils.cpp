#include "stdafx.h"
#include "OHIPDialerUtils.h"
#include "NxStandard.h"
#include "RegUtils.h"

//TO OVERRIDE A DOWNLOAD TEMPLATE:
//INSERT INTO ConfigRT (Username, Name, Number, TextParam) VALUES ('<None>', 'OHIPDownloadScriptPath_NexTechOverride', 0, 'C:\Pracstation\OVERRIDE_teraterm_download_template.ttl')

//TO REMOVE A DOWNLOAD OVERRIDE:
//DELETE FROM ConfigRT WHERE Name = 'OHIPDownloadScriptPath_NexTechOverride'

//TO OVERRIDE AN UPLOAD TEMPLATE:
//INSERT INTO ConfigRT (Username, Name, Number, TextParam) VALUES ('<None>', 'OHIPUploadScriptPath_NexTechOverride', 0, 'C:\Pracstation\OVERRIDE_teraterm_upload_template.ttl')

//TO REMOVE AN UPLOAD OVERRIDE:
//DELETE FROM ConfigRT WHERE Name = 'OHIPUploadScriptPath_NexTechOverride'

// (j.jones 2009-03-10 08:58) - PLID 32405 - LoadDialerSettings will load
// all the settings and put them into the passed-in variables
// (j.jones 2009-08-14 10:38) - PLID 34914 - added OHIP per-provider setting
// (s.dhole 2011-01-17 12:00) - PLID 42145 Added parameter for Teraterm  falg and NXModemlocation
// (b.spivey, June 26th, 2014) - PLID 62603 - Added EnableAutoDial to determine if any of these settings matter.
void LoadDialerSettings(CString &strPhoneNumber, CString &strUserName, CString &strPassword,
						long &nCommPort, CString &strDownloadPath, CString &strTTLocation,
						BOOL &bDeleteReports, BOOL &bChangePassword, CString &strNewPassword,
						BOOL &bShowTerminalWindow, BOOL &bSeparateAccountsPerProviders, 
						CString &strNXOhipModemLocation, BOOL &bTeraTerm, bool &bEnableAutoDial)
{
	//throw exceptions to the caller

	// (j.jones 2009-09-15 13:43) - PLID 33955 - This function originally loaded data
	// from the local PC's registry, and now we have migrated the settings to ConfigRT,
	// some are per-workstation, and some are global. To achieve a smooth transition,
	// we will try to load from the local registry if there is no data in the ConfigRT fields.
	// Saving occurs only in COHIPDialerSetupDlg::Save(), which will only save to ConfigRT.

	// (j.jones 2009-04-03 12:33) - PLID 33846 - cache ConfigRT preferences 
	// (s.dhole 2011-01-17 14:06) - PLID 42145- Add OHIP_UseTeraTerm
	// (j.armen 2011-10-25 15:17) - PLID 46134 - GetPracPath is referencing ConfigRT
	// (b.spivey, June 26th, 2014) - PLID 62603 - Cache. 
	g_propManager.CachePropertiesInBulk("OHIPDialerUtils_LoadDialerSettings_1", propNumber,
		"(Username = '<None>' OR Username = '%s' OR Username = '%s') AND ("
		"Name = 'OHIPAutoChangePasswords' OR "
		// (j.jones 2009-08-14 10:38) - PLID 34914 - added OHIP per-provider setting
		"Name = 'OHIP_SeparateAccountsPerProviders' OR "
		// (j.jones 2009-09-15 13:45) - PLID 33955 - added all old registry settings into ConfigRT
		"Name = 'OHIP_CommPort' OR "
		"Name = 'OHIP_DeleteReports' OR "
		"Name = 'OHIP_ChangePassword' OR "
		"Name = 'OHIP_ShowTerminalWindow' OR "
		"Name = 'OHIP_UseTeraTerm' OR "
		"Name = 'OHIP_AutoDialerEnabled' "
		")",
		_Q(GetCurrentUserName()), _Q(g_propManager.GetSystemName() + '.' + GetPracPath(PracPath::ConfigRT)));

	// (s.dhole 2011-01-17 13:06) - PLID 
	// (j.jones 2009-09-15 13:45) - PLID 33955 - added all old registry settings into ConfigRT
	// (s.dhole 2011-01-17 14:06) - PLID 42145- Add OHIP_NXModemLocation
	// (j.armen 2011-10-25 15:19) - PLID 46134 - GetPracPath is referencing ConfigRT
	g_propManager.CachePropertiesInBulk("OHIPDialerUtils_LoadDialerSettings_2", propText,
		"(Username = '<None>' OR Username = '%s' OR Username = '%s') AND ("
		"Name = 'OHIP_DialupNumber' OR "
		"Name = 'OHIP_UserName' OR "
		"Name = 'OHIP_Password' OR "
		"Name = 'OHIP_TTLocation' OR "
		"Name = 'OHIP_NewPassword' OR "
		"Name = 'OHIPLastPassword1' OR "
		"Name = 'OHIPLastPassword2' OR "
		"Name = 'OHIPLastPassword3' OR "
		"Name = 'OHIPLastPassword4' OR "
		"Name = 'OHIPLastPassword5' OR "
		// (j.jones 2009-09-15 13:48) - PLID 33955 - this wasn't cached before, now it is
		"Name = 'OHIPReportManager_ScanReportPath' OR "
		"Name = 'OHIP_NXModemLocation' "
		")",
		_Q(GetCurrentUserName()), _Q(g_propManager.GetSystemName() + '.' + GetPracPath(PracPath::ConfigRT)));

	// (j.jones 2009-09-15 14:45) - PLID 33955 - this is now global, not per-pc
	g_propManager.CachePropertiesInBulk("OHIPDialerUtils_LoadDialerSettings_3", propDateTime,
		"(Username = '<None>') AND ("
		"Name = 'OHIPPassword_LastChangedDate' "
		")");

	// (j.jones 2009-09-15 13:50) - PLID 33955 - load all the settings from ConfigRT, and if
	// nothing is there, load from the registry
	BOOL bLoadedFromRegistry = FALSE;
	strPhoneNumber = LoadDialerSettingString("OHIP_DialupNumber", "HKEY_LOCAL_MACHINE\\SOFTWARE\\Nextech\\OHIPAutoDialer\\DialupNumber",
		"1-613-544-3883", "<None>", bLoadedFromRegistry);	
	strUserName = LoadDialerSettingString("OHIP_UserName", "HKEY_LOCAL_MACHINE\\SOFTWARE\\Nextech\\OHIPAutoDialer\\UserName",
		"", "<None>", bLoadedFromRegistry);
	strPassword = LoadDialerSettingString("OHIP_Password", "HKEY_LOCAL_MACHINE\\SOFTWARE\\Nextech\\OHIPAutoDialer\\Password",
		"", "<None>", bLoadedFromRegistry);

	// (j.jones 2009-09-15 14:41) - PLID 33955 - If the password ended up being loaded from the registry,
	// it means this workstation was previously tracking it and we just now migrated it to ConfigRT.
	// That also means that this workstation was responsible for tracking the last passwords used,
	// which were per-PC, and are now global. So update accordingly.
	if(bLoadedFromRegistry) {
		CString strLastPassword1 = GetPropertyText("OHIPLastPassword1", "", 0, FALSE);
		CString strLastPassword2 = GetPropertyText("OHIPLastPassword2", "", 0, FALSE);
		CString strLastPassword3 = GetPropertyText("OHIPLastPassword3", "", 0, FALSE);
		CString strLastPassword4 = GetPropertyText("OHIPLastPassword4", "", 0, FALSE);
		CString strLastPassword5 = GetPropertyText("OHIPLastPassword5", "", 0, FALSE);
		SetRemotePropertyText("OHIPLastPassword1", strLastPassword1, 0, "<None>");
		SetRemotePropertyText("OHIPLastPassword2", strLastPassword2, 0, "<None>");
		SetRemotePropertyText("OHIPLastPassword3", strLastPassword3, 0, "<None>");
		SetRemotePropertyText("OHIPLastPassword4", strLastPassword4, 0, "<None>");
		SetRemotePropertyText("OHIPLastPassword5", strLastPassword5, 0, "<None>");

		COleDateTime dtLast = GetPropertyDateTime("OHIPPassword_LastChangedDate", &COleDateTime(1901, 1, 1, 1, 1, 1), 0, TRUE);
		SetRemotePropertyDateTime("OHIPPassword_LastChangedDate", dtLast, 0, "<None>");
	}

	// (j.armen 2011-10-25 15:30) - PLID 46134 - GetPracPath is referencing ConfigRT
	nCommPort = LoadDialerSettingLong("OHIP_CommPort", "HKEY_LOCAL_MACHINE\\SOFTWARE\\Nextech\\OHIPAutoDialer\\CommPort",
		3, g_propManager.GetSystemName() + '.' + GetPracPath(PracPath::ConfigRT), bLoadedFromRegistry);
	strTTLocation = LoadDialerSettingString("OHIP_TTLocation", "HKEY_LOCAL_MACHINE\\SOFTWARE\\Nextech\\OHIPAutoDialer\\TTLocation",
		"C:\\Program Files\\teraterm\\ttpmacro.exe", g_propManager.GetSystemName() + '.' + GetPracPath(PracPath::ConfigRT), bLoadedFromRegistry);
	// (s.dhole 2011-01-17 14:06) - PLID 42145
	strNXOhipModemLocation = LoadDialerSettingString("OHIP_NXModemLocation", "HKEY_LOCAL_MACHINE\\SOFTWARE\\Nextech\\OHIPAutoDialer\\NXModemLocation",
		GetPracPath(PracPath::PracticePath) ^	"NXModem\\NXModem.exe", g_propManager.GetSystemName() + '.' + GetPracPath(PracPath::ConfigRT), bLoadedFromRegistry);
	long nDeleteReports = LoadDialerSettingLong("OHIP_DeleteReports", "HKEY_LOCAL_MACHINE\\SOFTWARE\\Nextech\\OHIPAutoDialer\\DeleteReports",
		0, "<None>", bLoadedFromRegistry);
	bDeleteReports = (nDeleteReports == 1);
	long nChangePassword = LoadDialerSettingLong("OHIP_ChangePassword", "HKEY_LOCAL_MACHINE\\SOFTWARE\\Nextech\\OHIPAutoDialer\\ChangePassword",
		0, "<None>", bLoadedFromRegistry);
	bChangePassword = (nChangePassword == 1);
	strNewPassword = LoadDialerSettingString("OHIP_NewPassword", "HKEY_LOCAL_MACHINE\\SOFTWARE\\Nextech\\OHIPAutoDialer\\NewPassword",
		"", "<None>", bLoadedFromRegistry);
	long nShowTerminalWindow = LoadDialerSettingLong("OHIP_ShowTerminalWindow", "HKEY_LOCAL_MACHINE\\SOFTWARE\\Nextech\\OHIPAutoDialer\\ShowTerminalWindow",
		0, "<None>", bLoadedFromRegistry);
	bShowTerminalWindow = (nShowTerminalWindow == 1);
	// (s.dhole 2011-01-17 14:06) - PLID 42145
	bTeraTerm =GetPropertyInt( "OHIP_UseTeraTerm",TRUE,0,TRUE);

	bEnableAutoDial = !!GetPropertyInt("OHIP_AutoDialerEnabled");

	/***ALL NEW OHIP DIALER SETTINGS SHOULD LOAD FROM CONFIGRT ONLY***/

	//this is a change from the OHIP dialer - we want our download path to be the OHIP report manager path,
	//which defaults to the shared path, and is stored per machine
	strDownloadPath = GetPropertyText("OHIPReportManager_ScanReportPath", GetSharedPath(), 0, true);

	// (j.jones 2009-04-03 12:31) - PLID 33846 - enforce changing the password routinely,
	// but don't bother if we're already changing the password
	// (the preference to change this routinely is global, but the tracking information is local)
	if(!bChangePassword && GetRemotePropertyInt("OHIPAutoChangePasswords", 0, 0, "<None>", TRUE) != 0) {
		
		//how long ago did we change the password?
		COleDateTime dtInvalid;
		dtInvalid.SetStatus(COleDateTime::invalid);

		//defaults to invalid date time
		// (j.jones 2009-09-15 14:45) - PLID 33955 - this now global, not per-PC
		COleDateTime dtLast = GetRemotePropertyDateTime("OHIPPassword_LastChangedDate", &COleDateTime(1901, 1, 1, 1, 1, 1), 0, "<None>", TRUE);
		if(dtLast.GetStatus() == COleDateTime::invalid) {
			//invalid date time, definitely change the password
			bChangePassword = TRUE;
		}
		else {
			COleDateTime dtNow = COleDateTime::GetCurrentTime();
			COleDateTimeSpan dtSpan = dtNow - dtLast;
			if(dtSpan.GetDays() >= 25) {
				//it's been 25 days or more, so change it now
				bChangePassword = TRUE;
			}
		}

		if(bChangePassword) {
			//if we're changing the password, we must auto-generate a new one

			//this function will return a valid password that hasn't been used in the past 5 passwords
			strNewPassword = CalcNewOHIPPassword(strPassword, -1);

			//save these settings
			// (j.jones 2009-09-15 14:30) - PLID 33955 - we save to ConfigRT now
			SetRemotePropertyInt("OHIP_ChangePassword", 1, 0, "<None>");
			SetRemotePropertyText("OHIP_NewPassword", strNewPassword, 0, "<None>");
		}
	}

	// (j.jones 2009-08-14 10:38) - PLID 34914 - added OHIP per-provider setting
	bSeparateAccountsPerProviders = GetRemotePropertyInt("OHIP_SeparateAccountsPerProviders", 0, 0, "<None>", TRUE) == 1;
}

// (j.jones 2009-09-15 14:01) - PLID 33955 - added helper function to help migrate from
// loading from the registry to loading from ConfigRT
long LoadDialerSettingLong(CString strConfigRTName, CString strRegistryLocation,
						   long nDefaultValue, CString strUserName, BOOL &bLoadedFromRegistry)
{
	//first load the ConfigRT value, using a random default value that indicates an empty field
	long nRandomDefault = (long)GetTickCount();
	long nValue = GetRemotePropertyInt(strConfigRTName, nRandomDefault, 0, strUserName, false);
	
	//if the returned value equals the default ConfigRT value, try the registry
	if(nValue == nRandomDefault) {

		//is it in the registry?
		if(NxRegUtils::DoesValueExist(strRegistryLocation)) {
			nValue = NxRegUtils::ReadLong(strRegistryLocation, nDefaultValue);
			bLoadedFromRegistry = TRUE;
		}
		else {
			//use the default value
			nValue = nDefaultValue;
		}
	}

	//if we loaded from the registry, save the value in ConfigRT
	if(bLoadedFromRegistry) {
		SetRemotePropertyInt(strConfigRTName, nValue, 0, strUserName);

		//remove the registry key after we've done this
		NxRegUtils::DeleteValue(strRegistryLocation);
	}

	return nValue;
}

// (j.jones 2009-09-15 14:01) - PLID 33955 - added helper function to help migrate from
// loading from the registry to loading from ConfigRT
CString LoadDialerSettingString(CString strConfigRTName, CString strRegistryLocation,
						   CString strDefaultValue, CString strUserName, BOOL &bLoadedFromRegistry)
{
	//first load the ConfigRT value, using a default value that indicates an empty field
	CString strRandomDefault = AsString((long)GetTickCount());
	CString strValue = GetRemotePropertyText(strConfigRTName, strRandomDefault, 0, strUserName, false);
	
	//if the returned value equals the default ConfigRT value, try the registry
	if(strValue == strRandomDefault) {

		//is it in the registry?
		if(NxRegUtils::DoesValueExist(strRegistryLocation)) {
			strValue = NxRegUtils::ReadString(strRegistryLocation, strDefaultValue);
			bLoadedFromRegistry = TRUE;
		}
		else {
			//use the default value
			strValue = strDefaultValue;
		}
	}

	//if we loaded from the registry, save the value in ConfigRT
	if(bLoadedFromRegistry) {
		SetRemotePropertyText(strConfigRTName, strValue, 0, strUserName);

		//remove the registry key after we've done this
		NxRegUtils::DeleteValue(strRegistryLocation);
	}

	return strValue;
}

// (j.jones 2009-08-14 14:49) - PLID 34194 - added function to load provider login info from data
void LoadProviderLoginInfo(long nProviderID, CString &strUserName, CString &strPassword, BOOL &bChangePassword, CString &strNewPassword)
{
	//send exceptions to the caller

	ADODB::_RecordsetPtr rs = CreateParamRecordset("SELECT OHIPUserName, OHIPPassword, OHIPChangePassword, "
		"OHIPNewPassword, OHIPPasswordLastChangedDate "
		"FROM ProvidersT WHERE PersonID = {INT}", nProviderID);
	if(rs->eof) {
		ThrowNxException("LoadProviderLoginInfo called with invalid provider ID %li!", nProviderID);
	}
	else {
		strUserName = AdoFldString(rs, "OHIPUserName", "");
		strPassword = AdoFldString(rs, "OHIPPassword", "");
		bChangePassword = AdoFldBool(rs, "OHIPChangePassword", FALSE);
		strNewPassword = AdoFldString(rs, "OHIPNewPassword", "");

		// (j.jones 2009-08-14 14:30) - PLID 34914 - enforce changing the password routinely,
		// but don't bother if we're already changing the password
		// (the preference to change this routinely is global, but the tracking information is per-provider)
		if(!bChangePassword && GetRemotePropertyInt("OHIPAutoChangePasswords", 0, 0, "<None>", TRUE) != 0) {
			
			//how long ago did we change the password?
			COleDateTime dtInvalid;
			dtInvalid.SetStatus(COleDateTime::invalid);

			//defaults to invalid date time
			COleDateTime dtOld = COleDateTime(1901, 1, 1, 1, 1, 1);
			COleDateTime dtLast = AdoFldDateTime(rs, "OHIPPasswordLastChangedDate", dtOld);
			if(dtLast.GetStatus() == COleDateTime::invalid) {
				//invalid date time, definitely change the password
				bChangePassword = TRUE;
			}
			else {
				COleDateTime dtNow = COleDateTime::GetCurrentTime();
				COleDateTimeSpan dtSpan = dtNow - dtLast;
				if(dtSpan.GetDays() >= 25) {
					//it's been 25 days or more, so change it now
					bChangePassword = TRUE;
				}
			}

			if(bChangePassword) {
				//if we're changing the password, we must auto-generate a new one

				//this function will return a valid password that hasn't been used in the past 5 passwords
				strNewPassword = CalcNewOHIPPassword(strPassword, nProviderID);
			}
		}
	}
}

// (j.jones 2009-04-03 12:47) - PLID 33846 - generate a new password
// (j.jones 2009-08-14 11:27) - PLID 34914 - added optional provider ID parameter
CString CalcNewOHIPPassword(CString strOldPassword, long nCalculateForProviderID)
{
	CString strNewPassword;

	//this is the same syllabic alphabet we use in the generation of the NexTech superuser password
	const LPCTSTR carySyllableAlphabet[] = { 
		"ba", "be", "bi", "bo", "bu", "ca", "ce", "ci", "co", "cu", "da", "de", "di", "do", "du", "fa", "fe", "fi", "fo", "fu", "ga", 
		"ge", "gi", "go", "gu", "ha", "he", "hi", "ho", "hu", "ja", "je", "ji", "jo", "ju", "ka", "ke", "ki", "ko", "ku", "la", "le", 
		"li", "lo", "lu", "ma", "me", "mi", "mo", "mu", "na", "ne", "ni", "no", "nu", "pa", "pe", "pi", "po", "pu", "qua", "que", 
		"qui", "quo", "ra", "re", "ri", "ro", "ru", "sa", "se", "si", "so", "su", "ta", "te", "ti", "to", "tu", "va", "ve", "vi", "vo", 
		"vu", "wa", "we", "wi", "wo", "wu", "xa", "xe", "xi", "xo", "xu", "ya", "ye", "yi", "yo", "yu", "za", "ze", "zi", "zo", "zu" 
	};

	srand((unsigned)time(NULL));

	BOOL bContinue = TRUE;
	while(bContinue) {
		
		//first generate a password with random syllables
		int nRandomInt = rand();

		int nRandomIndex = nRandomInt % 100;
		CString strRandomSyllable = carySyllableAlphabet[nRandomIndex];

		if(!strNewPassword.IsEmpty()) {
			//confirm we won't put two of the same letters together
			if(strNewPassword.Right(1) == strRandomSyllable.Left(1)) {
				//can't do that, so try again
				continue;
			}
		}			
		strNewPassword += strRandomSyllable;

		if(strNewPassword.GetLength() > 15) {
			//we overshot the mark and made the password too long,
			//but this should really be impossible, so ASSERT
			ASSERT(FALSE);

			//gotta try again
			strNewPassword = "";
			continue;
		}
		else if(strNewPassword.GetLength() >= 8) {
			//great, we have a password of a valid length, now double-check it has no duplicate characters
			BOOL bValid = TRUE;
			CString strNewPasswordCaps = strNewPassword;
			strNewPasswordCaps.MakeUpper();
			for(int i=0; i<strNewPasswordCaps.GetLength() - 1 && bValid; i++) {
				char chThis = strNewPasswordCaps.GetAt(i);
				char chNext = strNewPasswordCaps.GetAt(i+1);
				if(chThis == chNext) {
					bValid = FALSE;
				}
			}

			if(!bValid) {
				//if it fails this test, find out how we did this, since previous
				//code should have prevented this from occurring
				ASSERT(FALSE);

				//gotta try again
				strNewPassword = "";
				continue;
			}

			//now, has it been used before?
			// (j.jones 2009-08-14 11:27) - PLID 34914 - compare per provider if one was given to us
			if(nCalculateForProviderID == -1) {
				if(HasOHIPPasswordBeenUsed_Workstation(strNewPassword, strOldPassword)) {
					//gotta try again
					strNewPassword = "";
					continue;
				}
			}
			else {
				if(HasOHIPPasswordBeenUsed_Provider(nCalculateForProviderID, strNewPassword, strOldPassword)) {
					//gotta try again
					strNewPassword = "";
					continue;
				}
			}

			//if we get here, we have a valid password!
			bContinue = FALSE;
		}
	}

	ASSERT(strNewPassword.GetLength() >= 8 && strNewPassword.GetLength() < 15);

	return strNewPassword;
}

// (j.jones 2009-04-03 12:47) - PLID 33846 - check password validity
// (j.jones 2009-08-14 11:27) - PLID 34914 - renamed to be different from the per-provider ability
BOOL HasOHIPPasswordBeenUsed_Workstation(CString strNewPassword, CString strOldPassword)
{
	// (j.jones 2009-09-15 14:36) - PLID 33955 - these are now global, not per-workstation
	g_propManager.CachePropertiesInBulk("OHIPDialerUtils_HasPasswordBeenUsed", propText,
		"(Username = '<None>') AND ("
		"Name = 'OHIPLastPassword1' OR "
		"Name = 'OHIPLastPassword2' OR "
		"Name = 'OHIPLastPassword3' OR "
		"Name = 'OHIPLastPassword4' OR "
		"Name = 'OHIPLastPassword5' "
		")");

	//get our last 5 passwords
	CString strLastPassword1 = GetRemotePropertyText("OHIPLastPassword1", "", 0, "<None>", TRUE);
	CString strLastPassword2 = GetRemotePropertyText("OHIPLastPassword2", "", 0, "<None>", TRUE);
	CString strLastPassword3 = GetRemotePropertyText("OHIPLastPassword3", "", 0, "<None>", TRUE);
	CString strLastPassword4 = GetRemotePropertyText("OHIPLastPassword4", "", 0, "<None>", TRUE);
	CString strLastPassword5 = GetRemotePropertyText("OHIPLastPassword5", "", 0, "<None>", TRUE);

	//if the password has been used, return TRUE

	if(strNewPassword.CompareNoCase(strLastPassword1) == 0) {
		return TRUE;
	}
	else if(strNewPassword.CompareNoCase(strLastPassword2) == 0) {
		return TRUE;
	}
	else if(strNewPassword.CompareNoCase(strLastPassword3) == 0) {
		return TRUE;
	}
	else if(strNewPassword.CompareNoCase(strLastPassword4) == 0) {
		return TRUE;
	}
	else if(strNewPassword.CompareNoCase(strLastPassword5) == 0) {
		return TRUE;
	}

	return FALSE;
}

// (j.jones 2009-08-14 11:25) - PLID 34914 - added a per-provider option
BOOL HasOHIPPasswordBeenUsed_Provider(long nProviderID, CString strNewPassword, CString strOldPassword)
{
	//throw exceptions to the caller

	ADODB::_RecordsetPtr rs = CreateParamRecordset("SELECT OHIPLastPassword1, OHIPLastPassword2, "
		"OHIPLastPassword3, OHIPLastPassword4, OHIPLastPassword5 "
		"FROM ProvidersT WHERE PersonID = {INT}", nProviderID);

	if(rs->eof) {
		ThrowNxException("HasOHIPPasswordBeenUsed_Provider called for an invalid provider ID! (%li)", nProviderID);
	}
	else {

		//get our last 5 passwords
		CString strLastPassword1 = AdoFldString(rs, "OHIPLastPassword1", "");
		CString strLastPassword2 = AdoFldString(rs, "OHIPLastPassword2", "");
		CString strLastPassword3 = AdoFldString(rs, "OHIPLastPassword3", "");
		CString strLastPassword4 = AdoFldString(rs, "OHIPLastPassword4", "");
		CString strLastPassword5 = AdoFldString(rs, "OHIPLastPassword5", "");

		//if the password has been used, return TRUE

		if(strNewPassword.CompareNoCase(strLastPassword1) == 0) {
			return TRUE;
		}
		else if(strNewPassword.CompareNoCase(strLastPassword2) == 0) {
			return TRUE;
		}
		else if(strNewPassword.CompareNoCase(strLastPassword3) == 0) {
			return TRUE;
		}
		else if(strNewPassword.CompareNoCase(strLastPassword4) == 0) {
			return TRUE;
		}
		else if(strNewPassword.CompareNoCase(strLastPassword5) == 0) {
			return TRUE;
		}

		return FALSE;
	}
}

// (j.jones 2009-03-09 16:43) - PLID 33419 - GenerateDownloadScriptTemplate will
// generate our default template file with {NX$} placeholders for configurable data,
// which we will later replace with our dialer settings.
// The return value is the full file location of the generated template.
CString GenerateDownloadScriptTemplate()
{
	//throw exceptions to the caller

	CString strPath = GetNxTempPath() ^ "teraterm_download_template.ttl";

	CFile fOutputFile;
	if(!fOutputFile.Open(strPath, CFile::modeCreate|CFile::modeWrite|CFile::shareExclusive)) {
		AfxMessageBox("The default download script template could not be created. Contact Nextech for assistance.");
		return "";
	}

	// Find the resource
	HRSRC hResInfo = FindResource(NULL, MAKEINTRESOURCE(IDR_TERATERM_DOWNLOAD_TEMPLATE), "TXT");
	if (hResInfo) {
		// Get the size of the resource data
		int nSize = (int)SizeofResource(NULL, hResInfo);
		ASSERT(nSize > 0);
		if (nSize > 0) {
			// Load the resource
			HGLOBAL hgRes = LoadResource(NULL, hResInfo);
			if (hgRes) {
				// Lock the resource
				LPVOID pResData = LockResource(hgRes);
				if (pResData) {
					// Write the data to the file
					fOutputFile.Write(pResData, nSize);

					UnlockResource(hgRes);
					FreeResource(hgRes);
				} else {
					FreeResource(hgRes);
					ThrowNxException("GenerateDownloadScriptTemplate: Could not lock resource! (1)");
				}
			} else {
				ThrowNxException("GenerateDownloadScriptTemplate: Could not lock resource! (2)");
			}
		} else {
			//no content!
			ThrowNxException("GenerateDownloadScriptTemplate: Resource is 0-length!");
		}
	} else {
		ThrowNxException("SetRtfTextFromResource: Could not find resource!");
	}

	fOutputFile.Close();

	return strPath;
}

// (s.dhole 2011-01-17 14:12) - PLID 42145 Build Download String command for NXModem
// this will return String for Download Command string 
CString PrepareModemDownloadCommandString(CString strPhoneNumber, CString strUserName, CString strPassword,
							long nCommPort, CString strDownloadPath, 
							BOOL bDeleteReports, BOOL bChangePassword, CString strNewPassword,
							BOOL bShowTerminalWindow)
{

	CString strScriptFile  ;
		//if changing the password, we add -np to the username
	if(bChangePassword) {
		strUserName += " -np";
	}

	if (strPhoneNumber.IsEmpty() ){
		strScriptFile ="- ";
	}
	else{
		strPhoneNumber.Remove('-') ;
		strScriptFile =strPhoneNumber  + " ";
	}

	if (strUserName.IsEmpty() ){
		strScriptFile +="- ";
	}
	else{
		strScriptFile +="\"" + strUserName + "\" ";
	}
	
	if (strPassword.IsEmpty() ){
		strScriptFile +="- ";
	}
	else{
		strScriptFile += strPassword + " ";
	}
	
	if (nCommPort<0 ){
		strScriptFile +="- ";
	}
	else{
		strScriptFile += FormatString( "%li ", nCommPort);
	}

	if (strDownloadPath.IsEmpty() ){
		strScriptFile +="\"-\" ";
	}
	else{
		strScriptFile +="\"" + strDownloadPath + "\" ";
	}

	if (bDeleteReports!=TRUE ){
		strScriptFile += "0 ";
	}
	else{
		strScriptFile += "1 ";
	}
	
	if (bChangePassword!=TRUE ){
		strScriptFile +="0 ";
	}
	else{
		strScriptFile +="1 ";
	}
	
	if (bChangePassword==TRUE && (!strNewPassword.IsEmpty() )){  
		strScriptFile += strNewPassword + " ";
	}
	else if (bChangePassword==TRUE &&  strNewPassword.IsEmpty() ){
		strScriptFile +="- ";
	}
	else{
		strScriptFile +="- ";
	}

	if (bShowTerminalWindow!=TRUE ){
		strScriptFile +="0 ";
	}
	else{
		strScriptFile += "1 ";
	}

	//upload path
	strScriptFile += "- ";
	
	// Action
	strScriptFile += "DOWNLOAD";
	
	//all done, return the path to the macro file	
	return strScriptFile;
}

// (j.jones 2009-03-09 17:06) - PLID 33419 - PrepareDownloadScriptFromTemplate will
// open a template file that has {NX$} placeholders, replace them with our configured
// data, and save the result as a temp file for TeraTerm to use as its macro.
// The return value is the full file location of the ready-to-use macro file.
CString PrepareDownloadScriptFromTemplate(CString strTemplatePath,
							CString strPhoneNumber, CString strUserName, CString strPassword,
							long nCommPort, CString strDownloadPath, CString strTTLocation,
							BOOL bDeleteReports, BOOL bChangePassword, CString strNewPassword,
							BOOL bShowTerminalWindow)
{
	//throw exceptions to the caller

	//open up our script template, and write a new script file with the current settings
	CFile fInput, fOutput;

	if(!fInput.Open(strTemplatePath, CFile::modeRead | CFile::shareCompat)) {
		AfxMessageBox("The input script template could not be found or opened. Downloading reports will be cancelled.");
		return "";
	}

	CString strScriptFile = GetNxTempPath() ^ "OHIP_Download_Script.ttl";

	if(!fOutput.Open(strScriptFile,CFile::modeCreate|CFile::modeWrite | CFile::shareCompat)) {
		AfxMessageBox("The output script template could not be created. Downloading reports will be cancelled.");
		fInput.Close();
		return "";
	}

	//if changing the password, we add -np to the username
	if(bChangePassword) {
		strUserName += " -np";
	}

	//now read in each string of the file, replace our templated text, and save to the new file

	const int LEN_16_KB = 16384;
	CString strIn;	//input string
	long iFileSize = fInput.Read(strIn.GetBuffer(LEN_16_KB), LEN_16_KB);
	strIn.ReleaseBuffer(iFileSize);

	CString strLastData = "";	//stores data between reads

	//these booleans track whether our updates succeeded
	BOOL bSetShowWindow = FALSE;
	BOOL bSetCommPort = FALSE;
	BOOL bSetDirectory = FALSE;
	BOOL bSetPhoneNum = FALSE;
	BOOL bSetUserName = FALSE;
	BOOL bSetPassword = FALSE;
	BOOL bSetDeleteReports = FALSE;
	BOOL bSetChangePass = FALSE;
	BOOL bSetNewPassword = FALSE;

	while(iFileSize > 0) {

		while(strIn.GetLength() > 0) {

			//replace our templated items

			int nCountReplaced = 0;

			//if the "show terminal window" is not checked, hide TeraTerm with a -1,
			//hide the log window with 6, and hide the macro window with 2
			//"show -1" will hide the macro window, but I decided not to use it
			CString strShowCommands = "";
			if(!bShowTerminalWindow) {
				strShowCommands = "showtt -1\r\n"
					"showtt 6\r\n"
					"showtt 2\r\n";
					//"show -1\r\n";
			}

			nCountReplaced = strIn.Replace("{NX$SHOWTT}", strShowCommands);
			if(nCountReplaced > 0) {
				bSetShowWindow = TRUE;
			}

			CString strPort;
			strPort.Format("%li", nCommPort);
			nCountReplaced = strIn.Replace("{NX$COMMPORT}", strPort);
			if(nCountReplaced > 0) {
				bSetCommPort = TRUE;
			}

			nCountReplaced = strIn.Replace("{NX$DIRECTORY}", strDownloadPath);
			if(nCountReplaced > 0) {
				bSetDirectory = TRUE;
			}

			nCountReplaced = strIn.Replace("{NX$PHONENUM}", strPhoneNumber);
			if(nCountReplaced > 0) {
				bSetPhoneNum = TRUE;
			}

			nCountReplaced = strIn.Replace("{NX$USERNAME}", strUserName);
			if(nCountReplaced > 0) {
				bSetUserName = TRUE;
			}

			nCountReplaced = strIn.Replace("{NX$PASSWORD}", strPassword);
			if(nCountReplaced > 0) {
				bSetPassword = TRUE;
			}

			//if we do not want to download reports, the :deletereports section
			//of the template will go straight to logoff, otherwise it will
			//delete all reports
			CString strSkipDeleteReports = "";
			if(!bDeleteReports) {
				strSkipDeleteReports = "goto logoff";
			}

			nCountReplaced = strIn.Replace("{NX$SKIPDELETEREPORTS}", strSkipDeleteReports);
			if(nCountReplaced > 0) {
				bSetDeleteReports = TRUE;
			}

			// (j.jones 2008-12-31 09:24) - PLID 32501 - supported changing the password
			nCountReplaced = strIn.Replace("{NX$CHANGEPASS}", bChangePassword ? "1" : "0");
			if(nCountReplaced > 0) {
				bSetChangePass = TRUE;
			}

			nCountReplaced = strIn.Replace("{NX$NEWPASSWORD}", strNewPassword);
			if(nCountReplaced > 0) {
				bSetNewPassword = TRUE;
			}

			//write to the output file
			fOutput.Write(strIn,strIn.GetLength());

			iFileSize = fInput.Read(strIn.GetBuffer(LEN_16_KB), LEN_16_KB);
			strIn.ReleaseBuffer(iFileSize);

			strIn = strLastData += strIn;
		}
	}

	fInput.Close();
	fOutput.Close();

	//validate that we replaced all our templated fields

	if(!bSetShowWindow) {
		AfxMessageBox("The Show Window flag could not be found in the script template.\n"
			"Please contact NexTech for assistance.");
		return "";
	}

	if(!bSetCommPort) {
		AfxMessageBox("The Comm Port could not be found in the script template.\n"
			"Please contact NexTech for assistance.");
		return "";
	}

	if(!bSetDirectory) {
		AfxMessageBox("The Directory could not be found in the script template.\n"
			"Please contact NexTech for assistance.");
		return "";
	}

	if(!bSetPhoneNum) {
		AfxMessageBox("The Phone Number could not be found in the script template.\n"
			"Please contact NexTech for assistance.");
		return "";
	}

	if(!bSetUserName) {
		AfxMessageBox("The User Name could not be found in the script template.\n"
			"Please contact NexTech for assistance.");
		return "";
	}

	if(!bSetPassword) {
		AfxMessageBox("The Password could not be found in the script template.\n"
			"Please contact NexTech for assistance.");
		return "";
	}

	if(!bSetDeleteReports) {
		AfxMessageBox("The 'Skip Delete Reports' field could not be found in the script template.\n"
			"Please contact NexTech for assistance.");
		return "";
	}

	// (j.jones 2008-12-31 09:24) - PLID 32501 - supported changing the password
	if(!bSetChangePass) {
		AfxMessageBox("The Change Password flag could not be found in the script template.\n"
			"Please contact NexTech for assistance.");
		return "";
	}

	if(!bSetNewPassword) {
		AfxMessageBox("The New Password could not be found in the script template.\n"
			"Please contact NexTech for assistance.");
		return "";
	}

	//all done, return the path to the macro file	
	return strScriptFile;
}

// (j.jones 2009-03-10 09:23) - PLID 33419 - DownloadReportsFromOHIP will prepare the
// script file, launch teraterm, and download the reports.
// The return value is -1 if a problem occurred, 0 if no reports downloaded, 1 if reports were downloaded.
// (j.jones 2009-08-14 11:37) - PLID 34914 - takes in a provider ID, optionally
long DownloadReportsFromOHIP(long nProviderID)
{
	//throw exceptions to the caller

	CString strPhoneNumber, strUserName, strPassword, strNewPassword;
	CString strDownloadPath, strTTLocation,strNXOhipModemLocation;
	long nCommPort = 3;
	CString strModemAppName="TeraTerm";
	BOOL bDeleteReports = FALSE, bChangePassword = FALSE, bShowTerminalWindow = FALSE;
	BOOL bSeparateAccountsPerProviders = FALSE,bTeraTerm =TRUE;

	bool bEnableAutoDialer = true; 
	
	// (j.jones 2009-08-14 10:38) - PLID 34914 - added OHIP per-provider setting
	// (s.dhole 2011-01-17 12:37) - PLID  42145 Added parameter for Teraterm  falg and NXModemlocation
	LoadDialerSettings(strPhoneNumber, strUserName, strPassword, nCommPort, strDownloadPath, strTTLocation,
		bDeleteReports, bChangePassword, strNewPassword, bShowTerminalWindow, bSeparateAccountsPerProviders,strNXOhipModemLocation, bTeraTerm, bEnableAutoDialer);

	// (b.spivey, June 26th, 2014) - PLID 62603 - Don't do anything at all. The auto dialer is turned off for a reason. 
	if (!bEnableAutoDialer) {
		AfxMessageBox("The Dialer setting is disabled. \r\n"
			"Please edit your Dialer Settings and confirm that you no longer use the dialer method for sending or receiving OHIP claims.");
		return -1;
	}

	// (j.jones 2009-08-14 11:32) - PLID 34914 - if per-provider, make sure we have a provider ID
	if(bSeparateAccountsPerProviders) {
		if(nProviderID == -1) {
			//we should never have allowed this
			ThrowNxException("DownloadReportsFromOHIP called without a provider ID!");
		}

		//load the provider information
		LoadProviderLoginInfo(nProviderID, strUserName, strPassword, bChangePassword, strNewPassword);
	}

	if(strPhoneNumber.IsEmpty() || strUserName.IsEmpty() || strPassword.IsEmpty()) {
		AfxMessageBox("You must enter valid login information to download reports.\n"
			"Please edit your Dialer Settings and confirm the connection information is correct.");
		return -1;
	}

	// (s.dhole 2011-01-17 14:06) - PLID 42145 Show Diffrent msg base on EXE call
	if ((bTeraTerm==TRUE) && (strTTLocation.IsEmpty() || !DoesExist(strTTLocation))) {
			AfxMessageBox("You must enter a valid location for the TeraTerm product.\n"
				"Please edit your Dialer Settings and confirm the TeraTerm path is correct.");
			return -1;
		}
	else{
		if(strNXOhipModemLocation.IsEmpty() || !DoesExist(strNXOhipModemLocation)) {
			AfxMessageBox("You must enter a valid location for the NXModem product.\n"
				"Please edit your Dialer Settings and confirm the NXModem path is correct.");
			return -1;
		}
	}


	if(strDownloadPath.IsEmpty() || !DoesExist(strDownloadPath)) {
		AfxMessageBox("You must enter a valid location for the download path.\n"
			"Please edit your Dialer Settings and confirm that the path to download reports is correct.");
		return -1;
	}

	if(nCommPort <= 0 || nCommPort > 256) {
		AfxMessageBox("You must enter valid comm port number (1 - 256).\n"
			"Please edit your Dialer Settings and confirm that the modem's comm port is correct.");
		return -1;
	}

	CString strTemplatePath = "";

	//check the secret override path for a download template
	CString strOverridePath = GetRemotePropertyText("OHIPDownloadScriptPath_NexTechOverride", "", 0, "<None>", false);
	if(!strOverridePath.IsEmpty() && DoesExist(strOverridePath)) {
		//if someone at NexTech has set this override path, use it,
		//and we won't generate a stock template to use
		strTemplatePath = strOverridePath;
	}

	
	
	DWORD dwReturn=-999;
	// (s.dhole 2011-01-17 14:06) - PLID 42145 Call Diffrent EXe base upon uer setting
	if (bTeraTerm==TRUE){
		//generate our default template
		if(strTemplatePath.IsEmpty()) {
			strTemplatePath = GenerateDownloadScriptTemplate();	
		}

		if(strTemplatePath.IsEmpty()) {
			//if this failed, we should have had a message as to why it failed
			return -1;
		}
			//now generate the script file from that template
		CString strMacroFile = PrepareDownloadScriptFromTemplate(strTemplatePath,
									strPhoneNumber, strUserName, strPassword, nCommPort,
									strDownloadPath, strTTLocation, bDeleteReports,
									bChangePassword, strNewPassword, bShowTerminalWindow);

		if(strMacroFile.IsEmpty()) {
			//if this failed, we should have had a message as to why it failed
			return -1;
		}

		dwReturn = LaunchTeraTerm(strTTLocation, strMacroFile);
	}
	else
	{
		strModemAppName="NxModem";
			//now generate the script file from that template
		CString strCommandString = PrepareModemDownloadCommandString(
									strPhoneNumber, strUserName, strPassword, nCommPort,
									strDownloadPath, bDeleteReports,
									bChangePassword, strNewPassword, bShowTerminalWindow);
		dwReturn = LaunchNXModem(strNXOhipModemLocation, strCommandString);
	}
	CString strShortMessage, strLongMessage;

	// (j.jones 2008-12-31 09:34) - PLID 32501 - track which return messages indicate
	// that we at least logged in successfully
	BOOL bSuccessfulLogin = FALSE;

	long nReturnValue = -1;	//-1 means a problem occurred

	if(dwReturn == 0) {
		//could be an error, could be the user closed the app before it finished
		strShortMessage = "Report download cancelled.";
		strLongMessage = "The TeraTerm application either could not be started, or closed prematurely.\n"
			"Please try downloading again.";
	}
	else if(dwReturn == 1) {
		//successful download
		strShortMessage = "Reports downloaded successfully.";
		strLongMessage = "All reports downloaded successfully. The Reports Manager will now open and import these new reports into Practice.";
		bSuccessfulLogin = TRUE;

		//1 means we downloaded reports
		nReturnValue = 1;
	}
	else if(dwReturn == 2) {
		//no files available
		strShortMessage = "No reports available.";
		strLongMessage = "No reports are available to download. Your mailbox is empty.";
		bSuccessfulLogin = TRUE;

		//0 means no reports are available
		nReturnValue = 0;
	}
	else if(dwReturn == -1) {
		//timeout
		// (j.jones 2008-12-31 10:10) - PLID 32501 - this is now the timeout that occurs AFTER login
		strShortMessage = "Timeout expired. Please try again.";
		strLongMessage = "A timeout occurred during the download process. Please try to download reports again.\n"
			"If you get this message frequently, please contact NexTech for assistance.";
		bSuccessfulLogin = TRUE;
	}
	else if(dwReturn == -2) {
		//no answer
		strShortMessage = "No answer.";
		strLongMessage = "The phone number you dialed did not answer. Please check the number entered in the dialer settings and try downloading again.";
	}
	else if(dwReturn == -3) {
		//line busy
		strShortMessage = "Line busy.";
		strLongMessage = "The phone number you dialed was busy. Please try downloading again.";
	}
	else if(dwReturn == -4) {
		//invalid password
		strShortMessage = "Invalid password.";
		strLongMessage = "The login name and password entered was invalid. Please correct this in the dialer settings and try again.";
	}
	else if(dwReturn == -5) {
		//report download failed
		strShortMessage = "Download failed.";
		strLongMessage = "There are reports to download, but the download process did not complete. Please try downloading again.\n"
			"If you get this message frequently, please contact NexTech for assistance.";
		bSuccessfulLogin = TRUE;
	}
	else if(dwReturn == -6) {
		//modem failed to connect
		strShortMessage = "Could not access modem.";
		strLongMessage = "TeraTerm could not access the modem specified in your Comm Port setting.\n"
			"Please check your Comm Port in your dialer settings, and ensure there is a modem configured for that port.";
	}
	// (j.jones 2008-12-30 16:09) - PLID 32501 - supported password expiration detection
	else if(dwReturn == -7) {
		//password expired
		strShortMessage = "User Password expired.";
		strLongMessage = "OHIP has indicated that your user password has expired.\n"
			"You must edit your dialer settings, enable the 'Change Password' and select a new password to use, then connect again.";
	}
	else if(dwReturn == -8) {
		//entered an old password as a new one
		strShortMessage = "New password already used.";
		strLongMessage = "OHIP has indicated that the new password you entered has already been used.\n"
			"You must edit your dialer settings and enter a new, unique password in the 'Change Password' box.";
	}
	else if(dwReturn == -9) {
		//pre-login timeout
		// (j.jones 2008-12-31 10:10) - PLID 32501 - this is the timeout that occurs BEFORE login
		strShortMessage = "Timeout expired. Please try again.";
		strLongMessage = "A timeout occurred during the download process. Please try to download reports again.\n"
			"If you get this message frequently, please contact NexTech for assistance.";
	}
	else {
		//unknown
		strShortMessage.Format("An unknown problem occurred. (Exit Code: %li)", (long)dwReturn);
		strLongMessage.Format("An unknown problem occurred while trying to download reports.\n"
			"Please contact NexTech for assistance. (Exit Code: %li)", (long)dwReturn);
	}

	// (j.jones 2008-12-31 09:34) - PLID 32501 - if we did log in, and we were
	// changing the password, update the saved password
	if(bSuccessfulLogin && bChangePassword) {
		// (j.jones 2009-08-14 11:32) - PLID 34914 - support per provider
		if(!bSeparateAccountsPerProviders) {
			UpdateOldOHIPPasswordWithNew_Workstation(strNewPassword, strPassword);
		}
		else {
			UpdateOldOHIPPasswordWithNew_Provider(nProviderID, strNewPassword, strPassword);
		}
	}
	
	//we don't have a progress bar to send the "short message", so right now we're not using it
	//SetDlgItemText(IDC_EDIT_PROGRESS, strShortMessage);
	// (s.dhole 2011-01-17 14:06) - PLID 42145 Show Diffrent msg base on EXE call
	strLongMessage.Replace("TeraTerm",strModemAppName);
	AfxMessageBox(strLongMessage);

	return nReturnValue;

}

// (s.dhole 2011-01-17 14:32) - PLID 42145 Call NxModem EXE

DWORD LaunchNXModem(CString  strExecutablePath, CString  strCommandString)
{
	//throw exceptions to caller

	DWORD dwResult = 0;

	if(strExecutablePath.IsEmpty() || !DoesExist(strExecutablePath)) {
		CString strWarn;
		strWarn.Format("The NXModem application cannot be launched because the program file:\n"
			"%s\n"
			"does not exist.", strExecutablePath);
		AfxMessageBox(strWarn);
		return 0;
	}

	if(strCommandString.IsEmpty() ) {
		CString strWarn;
		strWarn.Format("The NXModem application cannot be launched because Command line string is empty.");
		AfxMessageBox(strWarn);
		return 0;
	}

	//the command line value for CreateProcess wants the exe name only (no path),
	//plus the command line executable parameters, because CreateProcess likes
	//to make things difficult, then laughs at your failure

	CString strExeFileName;
	int nSlash = strExecutablePath.ReverseFind('\\');
	if(nSlash == -1) {
		strExeFileName = strExecutablePath;
	}
	else {
		strExeFileName = strExecutablePath.Right(strExecutablePath.GetLength()-nSlash-1);
	}
	CString strCommandLine;
	strCommandLine.Format("%s %s", strExeFileName, strCommandString);

	//launch TeraTerm in a process, so we can get the return value
	HANDLE hProcess;
	{
		STARTUPINFO si;
		si.cb = sizeof(STARTUPINFO);
		si.lpReserved = NULL;
		si.lpDesktop = NULL;
		si.lpTitle = NULL;
		si.dwFlags = STARTF_FORCEONFEEDBACK;
		si.cbReserved2 = 0;
		si.lpReserved2 = NULL;
		PROCESS_INFORMATION pi;
					
		BOOL bCreated;
		{
			Log("Launching NXModem with the path '%s' and command line '%s'.", strExecutablePath, strCommandLine);

			bCreated = CreateProcess(strExecutablePath, strCommandLine.GetBuffer(0), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
			DWORD dwErr = GetLastError();
			strCommandLine.ReleaseBuffer();
			if(bCreated == FALSE && dwErr == ERROR_FILE_NOT_FOUND) {
				//could not launch the process
				dwResult = 0;
			}
		}
		if (bCreated) {
			hProcess = pi.hProcess;
			CloseHandle(pi.hThread);
		} else {
			hProcess = NULL;
		}
	}


	//wait for TeraTerm to close, then send the exit code
	if (hProcess) {

		Log("Process created, waiting on response code.");

		WaitForSingleObject(hProcess, INFINITE);
		GetExitCodeProcess(hProcess, &dwResult);
		
		if(dwResult == STILL_ACTIVE)
		{
		  //TeraTerm did not terminate, so let's kill it
		  ::TerminateProcess(hProcess, 0);
		}

		Log("Process response code: %li", (long)dwResult);
		
		CloseHandle(hProcess);
		hProcess = NULL;
	} else {
		dwResult = 0;
	}

	return dwResult;
}



// (j.jones 2009-03-10 09:32) - PLID 33419 - LaunchTeraTerm will launch the teraterm program
// with our passed-in macro file
DWORD LaunchTeraTerm(CString strExecutablePath, CString strMacroFile)
{
	//throw exceptions to caller

	DWORD dwResult = 0;

	if(strExecutablePath.IsEmpty() || !DoesExist(strExecutablePath)) {
		CString strWarn;
		strWarn.Format("The TeraTerm application cannot be launched because the program file:\n"
			"%s\n"
			"does not exist.", strExecutablePath);
		AfxMessageBox(strWarn);
		return 0;
	}

	if(strMacroFile.IsEmpty() || !DoesExist(strMacroFile)) {
		CString strWarn;
		strWarn.Format("The TeraTerm application cannot be launched because the macro file:\n"
			"%s\n"
			"does not exist.", strMacroFile);
		AfxMessageBox(strWarn);
		return 0;
	}

	//the command line value for CreateProcess wants the exe name only (no path),
	//plus the command line executable parameters, because CreateProcess likes
	//to make things difficult, then laughs at your failure

	CString strExeFileName;
	int nSlash = strExecutablePath.ReverseFind('\\');
	if(nSlash == -1) {
		strExeFileName = strExecutablePath;
	}
	else {
		strExeFileName = strExecutablePath.Right(strExecutablePath.GetLength()-nSlash-1);
	}

	CString strCommandLine;
	strCommandLine.Format("%s \"%s\"", strExeFileName, strMacroFile);

	//launch TeraTerm in a process, so we can get the return value
	HANDLE hProcess;
	{
		STARTUPINFO si;
		si.cb = sizeof(STARTUPINFO);
		si.lpReserved = NULL;
		si.lpDesktop = NULL;
		si.lpTitle = NULL;
		si.dwFlags = STARTF_FORCEONFEEDBACK;
		si.cbReserved2 = 0;
		si.lpReserved2 = NULL;
		PROCESS_INFORMATION pi;
					
		BOOL bCreated;
		{
			Log("Launching TeraTerm with the path '%s' and command line '%s'.", strExecutablePath, strCommandLine);

			bCreated = CreateProcess(strExecutablePath, strCommandLine.GetBuffer(0), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
			DWORD dwErr = GetLastError();
			strCommandLine.ReleaseBuffer();
			if(bCreated == FALSE && dwErr == ERROR_FILE_NOT_FOUND) {
				//could not launch the process
				dwResult = 0;
			}
		}
		if (bCreated) {
			hProcess = pi.hProcess;
			CloseHandle(pi.hThread);
		} else {
			hProcess = NULL;
		}
	}
	
	//wait for TeraTerm to close, then send the exit code
	if (hProcess) {

		Log("Process created, waiting on response code.");

		WaitForSingleObject(hProcess, INFINITE);
		GetExitCodeProcess(hProcess, &dwResult);
		
		if(dwResult == STILL_ACTIVE)
		{
		  //TeraTerm did not terminate, so let's kill it
		  ::TerminateProcess(hProcess, 0);
		}

		Log("Process response code: %li", (long)dwResult);
		
		CloseHandle(hProcess);
		hProcess = NULL;
	} else {
		dwResult = 0;
	}

	return dwResult;
}

// (j.jones 2009-03-10 09:39) - PLID 33419 - UpdateOldOHIPPasswordWithNew will be called after
// a connection that changes the password succeeds, and will clear the change password flag,
// clear the new password field, and save the new password as the current password
// (j.jones 2009-04-03 14:38) - PLID 33846 - added strOldPassword
void UpdateOldOHIPPasswordWithNew_Workstation(CString strNewPassword, CString strOldPassword)
{
	try {

		//save the new password
		// (j.jones 2009-09-15 14:33) - PLID 33955 - this is all in ConfigRT now
		SetRemotePropertyText("OHIP_Password", strNewPassword, 0, "<None>");

		//turn off the change password settings
		SetRemotePropertyInt("OHIP_ChangePassword", 0, 0, "<None>");
		SetRemotePropertyText("OHIP_NewPassword", "", 0, "<None>");

		// (j.jones 2009-04-03 14:36) - PLID 33846 - now we also need to update that we changed the password,
		// and each of our last 5 passwords

		// (j.jones 2009-09-15 14:36) - PLID 33955 - these are now global, not per-workstation
		g_propManager.CachePropertiesInBulk("OHIPDialerUtils_UpdateOldOHIPPasswordWithNew", propText,
			"(Username = '<None>') AND ("
			"Name = 'OHIPLastPassword1' OR "
			"Name = 'OHIPLastPassword2' OR "
			"Name = 'OHIPLastPassword3' OR "
			"Name = 'OHIPLastPassword4' "
			")");
		
		//get our last 4 passwords
		CString strLastPassword1 = GetRemotePropertyText("OHIPLastPassword1", "", 0, "<None>", TRUE);
		CString strLastPassword2 = GetRemotePropertyText("OHIPLastPassword2", "", 0, "<None>", TRUE);
		CString strLastPassword3 = GetRemotePropertyText("OHIPLastPassword3", "", 0, "<None>", TRUE);
		CString strLastPassword4 = GetRemotePropertyText("OHIPLastPassword4", "", 0, "<None>", TRUE);

		SetRemotePropertyText("OHIPLastPassword5", strLastPassword4, 0, "<None>");
		SetRemotePropertyText("OHIPLastPassword4", strLastPassword3, 0, "<None>");
		SetRemotePropertyText("OHIPLastPassword3", strLastPassword2, 0, "<None>");
		SetRemotePropertyText("OHIPLastPassword2", strLastPassword1, 0, "<None>");
		SetRemotePropertyText("OHIPLastPassword1", strOldPassword, 0, "<None>");

		//and now set our date/time
		SetRemotePropertyDateTime("OHIPPassword_LastChangedDate", COleDateTime::GetCurrentTime(), 0, "<None>");

	}NxCatchAll("Error in UpdateOldOHIPPasswordWithNew_Workstation");
}

// (j.jones 2009-08-14 11:25) - PLID 34914 - added a per-provider option
void UpdateOldOHIPPasswordWithNew_Provider(long nProviderID, CString strNewPassword, CString strOldPassword)
{
	try {

		if(nProviderID == -1) {
			ThrowNxException("UpdateOldOHIPPasswordWithNew_Provider called with no ProviderID!");
		}

		//we need to propagate the old passwords, set the new password,
		//and clear the change password info.
		//(impressively, this logic works in one execute!)
		ExecuteParamSql("UPDATE ProvidersT SET "
			"OHIPLastPassword5 = OHIPLastPassword4, "
			"OHIPLastPassword4 = OHIPLastPassword3, "
			"OHIPLastPassword3 = OHIPLastPassword2, "
			"OHIPLastPassword2 = OHIPLastPassword1, "
			"OHIPLastPassword1 = {STRING}, "
			"OHIPPassword = {STRING}, "
			"OHIPChangePassword = 0, OHIPNewPassword = '', "
			"OHIPPasswordLastChangedDate = GetDate() "
			"WHERE PersonID = {INT}", strOldPassword, strNewPassword, nProviderID);

	}NxCatchAll("Error in UpdateOldOHIPPasswordWithNew_Provider");
}

// (j.jones 2009-03-10 12:48) - PLID 33418 - GenerateUploadScriptTemplate will
// generate our default template file with {NX$} placeholders for configurable data,
// which we will later replace with our dialer settings.
// The return value is the full file location of the generated template.
CString GenerateUploadScriptTemplate()
{
	//throw exceptions to the caller

	CString strPath = GetNxTempPath() ^ "teraterm_upload_template.ttl";

	CFile fOutputFile;
	if(!fOutputFile.Open(strPath, CFile::modeCreate|CFile::modeWrite|CFile::shareExclusive)) {
		AfxMessageBox("The default upload script template could not be created. Contact Nextech for assistance.");
		return "";
	}

	// Find the resource
	HRSRC hResInfo = FindResource(NULL, MAKEINTRESOURCE(IDR_TERATERM_UPLOAD_TEMPLATE), "TXT");
	if (hResInfo) {
		// Get the size of the resource data
		int nSize = (int)SizeofResource(NULL, hResInfo);
		ASSERT(nSize > 0);
		if (nSize > 0) {
			// Load the resource
			HGLOBAL hgRes = LoadResource(NULL, hResInfo);
			if (hgRes) {
				// Lock the resource
				LPVOID pResData = LockResource(hgRes);
				if (pResData) {
					// Write the data to the file
					fOutputFile.Write(pResData, nSize);

					UnlockResource(hgRes);
					FreeResource(hgRes);
				} else {
					FreeResource(hgRes);
					ThrowNxException("GenerateUploadScriptTemplate: Could not lock resource! (1)");
				}
			} else {
				ThrowNxException("GenerateUploadScriptTemplate: Could not lock resource! (2)");
			}
		} else {
			//no content!
			ThrowNxException("GenerateUploadScriptTemplate: Resource is 0-length!");
		}
	} else {
		ThrowNxException("GenerateUploadScriptTemplate: Could not find resource!");
	}

	fOutputFile.Close();

	return strPath;
}

// (s.dhole 2011-01-17 17:29) - PLID  42145 Build Upload File Command string

CString PrepareModemUploadCommandString( CString strClaimFilePath,
							CString strPhoneNumber, CString strUserName, CString strPassword,
							long nCommPort, BOOL bChangePassword, CString strNewPassword,
							BOOL bShowTerminalWindow)
{
	CString strScriptFile  ;
		//if changing the password, we add -np to the username
	if(bChangePassword) {
		strUserName += " -np";
	}

	if (strPhoneNumber.IsEmpty() ){
		strScriptFile ="- " ;
	}
	else{
		strPhoneNumber.Remove('-') ;
		strScriptFile =strPhoneNumber  + " ";
	}

	if (strUserName.IsEmpty() ){
		strScriptFile += "- " ;
	}
	else{
		strScriptFile +="\"" + strUserName + "\" ";
	}
	
	if (strPassword.IsEmpty()){
		strScriptFile += "- ";
	}
	else
	{
		strScriptFile += strPassword + " ";
	}
	
	if (nCommPort<0 ){
		strScriptFile += "- " ;
	}
	else{
		strScriptFile += FormatString( "%li ", nCommPort)   ;
	}

	//Download path
		strScriptFile += "- ";

	//Delete Report
		strScriptFile += "0 ";

	if (bChangePassword!=TRUE ){
		strScriptFile += "0 ";
	}
	else{
		strScriptFile += "1 " ;
	}
	
	if (bChangePassword==TRUE && (!strNewPassword.IsEmpty() )){  
		strScriptFile += strNewPassword + " "   ;
	}
	else if (bChangePassword==TRUE &&  strNewPassword.IsEmpty() ){
		strScriptFile += "- "   ;
	}
	else{
		strScriptFile += "- "   ;
	}

	if (bShowTerminalWindow!=TRUE ){
		strScriptFile += "0 "   ;
	}
	else{
		strScriptFile += "1 "   ;
	}
	//upload path
	if (strClaimFilePath.IsEmpty() ){
		strScriptFile +="\"-\" ";
	}
	else{
		strScriptFile += "\"" + strClaimFilePath + "\" ";
	}
	// Action
	strScriptFile +="UPLOAD";
	
	//all done, return the path to the macro file	
	return strScriptFile;
}
// (j.jones 2009-03-10 12:48) - PLID 33418 - PrepareUploadScriptFromTemplate will
// open a template file that has {NX$} placeholders, replace them with our configured
// data, and save the result as a temp file for TeraTerm to use as its macro.
// The return value is the full file location of the ready-to-use macro file.
CString PrepareUploadScriptFromTemplate(CString strTemplatePath, CString strClaimFilePath,
							CString strPhoneNumber, CString strUserName, CString strPassword,
							long nCommPort, CString strTTLocation,
							BOOL bChangePassword, CString strNewPassword,
							BOOL bShowTerminalWindow)
{
	//throw exceptions to the caller

	//open up our script template, and write a new script file with the current settings
	CFile fInput, fOutput;

	if(!fInput.Open(strTemplatePath, CFile::modeRead | CFile::shareCompat)) {
		AfxMessageBox("The input script template could not be found or opened. Submitting claims will be cancelled.\n"
			"You can send these claims again by clicking the \"Manually Send Claim File\" button in the dialer setup.");
		return "";
	}

	CString strScriptFile = GetNxTempPath() ^ "OHIP_Upload_Script.ttl";

	if(!fOutput.Open(strScriptFile,CFile::modeCreate|CFile::modeWrite | CFile::shareCompat)) {
		AfxMessageBox("The output script template could not be created. Submitting claims will be cancelled.\n"
			"You can send these claims again by clicking the \"Manually Send Claim File\" button in the dialer setup.");
		fInput.Close();
		return "";
	}

	//if changing the password, we add -np to the username
	if(bChangePassword) {
		strUserName += " -np";
	}

	//split the file path into filename and path
	CString strDirectory;
	CString strFileName;
	int nSlash = strClaimFilePath.ReverseFind('\\');
	if(nSlash == -1) {
		// (j.armen 2011-10-25 15:44) - PLID 46134 - EBilling is located in the practice path
		strFileName = strClaimFilePath;
		// (v.maida 2016-05-19 17:01) - NX-100684 - Updated to used the shared path for Azure.
		strDirectory = GetEnvironmentDirectory();
	}
	else {
		strFileName = strClaimFilePath.Right(strClaimFilePath.GetLength()-nSlash-1);
		strDirectory = strClaimFilePath.Left(nSlash);
	}

	//now read in each string of the file, replace our templated text, and save to the new file

	const int LEN_16_KB = 16384;
	CString strIn;	//input string
	long iFileSize = fInput.Read(strIn.GetBuffer(LEN_16_KB), LEN_16_KB);
	strIn.ReleaseBuffer(iFileSize);

	CString strLastData = "";	//stores data between reads

	//these booleans track whether our updates succeeded
	BOOL bSetShowWindow = FALSE;
	BOOL bSetCommPort = FALSE;
	BOOL bSetDirectory = FALSE;
	BOOL bSetPhoneNum = FALSE;
	BOOL bSetUserName = FALSE;
	BOOL bSetPassword = FALSE;
	BOOL bSetFileName = FALSE;
	BOOL bSetFilePath = FALSE;
	BOOL bChangePass = FALSE;
	BOOL bNewPassword = FALSE;

	while(iFileSize > 0) {

		while(strIn.GetLength() > 0) {

			//replace our templated items

			int nCountReplaced = 0;

			//if the "show terminal window" is not checked, hide TeraTerm with a -1,
			//hide the log window with 6, and hide the macro window with 2
			//"show -1" will hide the macro window, but I decided not to use it
			CString strShowCommands = "";
			if(!bShowTerminalWindow) {
				strShowCommands = "showtt -1\r\n"
					"showtt 6\r\n"
					"showtt 2\r\n";
					//"show -1\r\n";
			}

			nCountReplaced = strIn.Replace("{NX$SHOWTT}", strShowCommands);
			if(nCountReplaced > 0) {
				bSetShowWindow = TRUE;
			}

			CString strPort;
			strPort.Format("%li", nCommPort);
			nCountReplaced = strIn.Replace("{NX$COMMPORT}", strPort);
			if(nCountReplaced > 0) {
				bSetCommPort = TRUE;
			}

			nCountReplaced = strIn.Replace("{NX$DIRECTORY}", strDirectory);
			if(nCountReplaced > 0) {
				bSetDirectory = TRUE;
			}

			nCountReplaced = strIn.Replace("{NX$PHONENUM}", strPhoneNumber);
			if(nCountReplaced > 0) {
				bSetPhoneNum = TRUE;
			}

			nCountReplaced = strIn.Replace("{NX$USERNAME}", strUserName);
			if(nCountReplaced > 0) {
				bSetUserName = TRUE;
			}

			nCountReplaced = strIn.Replace("{NX$PASSWORD}", strPassword);
			if(nCountReplaced > 0) {
				bSetPassword = TRUE;
			}

			// (j.jones 2009-02-09 11:43) - PLID 32996 - the filename cannot be greater
			// than eight characters, though it doesn't seem that OHIP cares very much
			// what name shows up here, so just silently trim it
			CString strFileNameTrimmed = strFileName;
			if(strFileNameTrimmed.GetLength() > 8) {
				strFileNameTrimmed = strFileNameTrimmed.Left(8);
			}
			nCountReplaced = strIn.Replace("{NX$FILENAME}", strFileNameTrimmed);
			if(nCountReplaced > 0) {
				bSetFileName = TRUE;
			}

			nCountReplaced = strIn.Replace("{NX$FILEPATH}", strClaimFilePath);
			if(nCountReplaced > 0) {
				bSetFilePath = TRUE;
			}

			// (j.jones 2008-12-31 09:24) - PLID 32501 - supported changing the password
			nCountReplaced = strIn.Replace("{NX$CHANGEPASS}", bChangePassword ? "1" : "0");
			if(nCountReplaced > 0) {
				bChangePass = TRUE;
			}

			nCountReplaced = strIn.Replace("{NX$NEWPASSWORD}", strNewPassword);
			if(nCountReplaced > 0) {
				bNewPassword = TRUE;
			}

			//write to the output file
			fOutput.Write(strIn,strIn.GetLength());

			iFileSize = fInput.Read(strIn.GetBuffer(LEN_16_KB), LEN_16_KB);
			strIn.ReleaseBuffer(iFileSize);

			strIn = strLastData += strIn;
		}
	}

	fInput.Close();
	fOutput.Close();

	//validate that we replaced all our templated fields

	if(!bSetShowWindow) {
		AfxMessageBox("The Show Window flag could not be found in the script template. Please contact NexTech for assistance.\n"
			"Submitting claims will be cancelled. You can send these claims again by clicking the "
			"\"Manually Send Claim File\" button in the dialer setup.");
		return "";
	}

	if(!bSetCommPort) {
		AfxMessageBox("The Comm Port could not be found in the script template. Please contact NexTech for assistance.\n"
			"Submitting claims will be cancelled. You can send these claims again by clicking the "
			"\"Manually Send Claim File\" button in the dialer setup.");
		return "";
	}

	if(!bSetDirectory) {
		AfxMessageBox("The Directory could not be found in the script template. Please contact NexTech for assistance.\n"
			"Submitting claims will be cancelled. You can send these claims again by clicking the "
			"\"Manually Send Claim File\" button in the dialer setup.");
		return "";
	}

	if(!bSetPhoneNum) {
		AfxMessageBox("The Phone Number could not be found in the script template. Please contact NexTech for assistance.\n"
			"Submitting claims will be cancelled. You can send these claims again by clicking the "
			"\"Manually Send Claim File\" button in the dialer setup.");
		return "";
	}

	if(!bSetUserName) {
		AfxMessageBox("The User Name could not be found in the script template. Please contact NexTech for assistance.\n"
			"Submitting claims will be cancelled. You can send these claims again by clicking the "
			"\"Manually Send Claim File\" button in the dialer setup.");
		return "";
	}

	if(!bSetFileName) {
		AfxMessageBox("The FileName could not be found in the script template. Please contact NexTech for assistance.\n"
			"Submitting claims will be cancelled. You can send these claims again by clicking the "
			"\"Manually Send Claim File\" button in the dialer setup.");
		return "";
	}

	if(!bSetFilePath) {
		AfxMessageBox("The FilePath could not be found in the script template. Please contact NexTech for assistance.\n"
			"Submitting claims will be cancelled. You can send these claims again by clicking the "
			"\"Manually Send Claim File\" button in the dialer setup.");
		return "";
	}

	// (j.jones 2008-12-31 09:24) - PLID 32501 - supported changing the password
	if(!bChangePass) {
		AfxMessageBox("The Change Password flag could not be found in the script template. Please contact NexTech for assistance.\n"
			"Submitting claims will be cancelled. You can send these claims again by clicking the "
			"\"Manually Send Claim File\" button in the dialer setup.");
		return "";
	}

	if(!bNewPassword) {
		AfxMessageBox("The New Password could not be found in the script template. Please contact NexTech for assistance.\n"
			"Submitting claims will be cancelled. You can send these claims again by clicking the "
			"\"Manually Send Claim File\" button in the dialer setup.");
		return "";
	}

	return strScriptFile;
}

// (j.jones 2009-03-10 12:48) - PLID 33419 - SendClaimFileToOHIP will prepare the
// script file, launch teraterm, and send the claim file in strClaimFilePath
// The return value is FALSE if a problem occurred, TRUE if the submission succeeded.
// (j.jones 2009-08-14 11:37) - PLID 34914 - takes in a provider ID, optionally
BOOL SendClaimFileToOHIP(CString strClaimFilePath, long nProviderID)
{
	//throw exceptions to the caller

	CString strPhoneNumber, strUserName, strPassword, strNewPassword;
	CString strDownloadPath, strTTLocation,strNXOhipModemLocation;
	long nCommPort = 3;
	CString strModemAppName="TeraTerm"; 	// (s.dhole 2011-01-17 14:06) - PLID 42145
	BOOL bDeleteReports = FALSE, bChangePassword = FALSE, bShowTerminalWindow = FALSE;
	BOOL bSeparateAccountsPerProviders = FALSE,bTeraTerm=TRUE;

	bool bEnableAutoDialer = false;

	// (j.jones 2009-08-14 10:38) - PLID 34914 - added OHIP per-provider setting	
	// (s.dhole 2011-01-17 12:37) - PLID 42145 Added parameter for Teraterm  flag and NXModemlocation
	// (b.spivey, June 26th, 2014) - PLID 62603 - Added bEnableAutoDialer
	LoadDialerSettings(strPhoneNumber, strUserName, strPassword, nCommPort, strDownloadPath, strTTLocation,
		bDeleteReports, bChangePassword, strNewPassword, bShowTerminalWindow, bSeparateAccountsPerProviders,strNXOhipModemLocation, bTeraTerm, bEnableAutoDialer);

	// (j.jones 2009-08-14 11:32) - PLID 34914 - if per-provider, make sure we have a provider ID
	if(bSeparateAccountsPerProviders) {
		if(nProviderID == -1) {
			//we should never have allowed this
			ThrowNxException("SendClaimFileToOHIP called without a provider ID!");
		}

		//load the provider information
		LoadProviderLoginInfo(nProviderID, strUserName, strPassword, bChangePassword, strNewPassword);
	}

	if(strPhoneNumber.IsEmpty() || strUserName.IsEmpty() || strPassword.IsEmpty()) {
		AfxMessageBox("You must enter valid login information to submit claims.\n"
			"Please edit your Dialer Settings and confirm the connection information is correct.\n\n"
			"You can send these claims again by clicking the \"Manually Send Claim File\" button in the dialer setup.");
		return FALSE;
	}
	
	// (s.dhole 2011-02-11 09:20) - PLID  42145 this location is required for terterm for script file, not used in  NexModem
	if  (bTeraTerm==TRUE)
	{
		if(strTTLocation.IsEmpty() || !DoesExist(strTTLocation)) {
			AfxMessageBox("You must enter a valid location for the TeraTerm product.\n"
				"Please edit your Dialer Settings and confirm the TeraTerm path is correct.\n\n"
				"You can send these claims again by clicking the \"Manually Send Claim File\" button in the dialer setup.");
			return FALSE;
		}
	}

	if(strClaimFilePath.IsEmpty() || !DoesExist(strClaimFilePath)) {
		AfxMessageBox("The claim file path is invalid. Contact NexTech for assistance.\n"
			"Please edit your Dialer Settings and confirm that the path to download reports is correct.\n\n"
			"You can send these claims again by clicking the \"Manually Send Claim File\" button in the dialer setup.");
		return FALSE;
	}

	if(nCommPort <= 0 || nCommPort > 256) {
		AfxMessageBox("You must enter valid comm port number (1 - 256).\n"
			"Please edit your Dialer Settings and confirm that the modem's comm port is correct.\n\n"
			"You can send these claims again by clicking the \"Manually Send Claim File\" button in the dialer setup.");
		return FALSE;
	}



	DWORD dwReturn=-999;
	// (s.dhole 2011-01-17 14:06) - PLID 42145 Call Diffrent EXe base upon uer setting
	if (bTeraTerm==TRUE){

		CString strTemplatePath = "";
		// (s.dhole 2011-02-11 09:20) - PLID  42145 this location is required for terterm for script file n, not for NexModem
		//check the secret override path for a upload template
		CString strOverridePath = GetRemotePropertyText("OHIPUploadScriptPath_NexTechOverride", "", 0, "<None>", false);
		if(!strOverridePath.IsEmpty() && DoesExist(strOverridePath)) {
			//if someone at NexTech has set this override path, use it,
			//and we won't generate a stock template to use
			strTemplatePath = strOverridePath;
		}

		//generate our default template
		if(strTemplatePath.IsEmpty()) {
			strTemplatePath = GenerateUploadScriptTemplate();	
		}

		if(strTemplatePath.IsEmpty()) {
			//if this failed, we should have had a message as to why it failed
			return FALSE;
		}
		//now generate the script file from that template
		CString strMacroFile = PrepareUploadScriptFromTemplate(strTemplatePath, strClaimFilePath,
									strPhoneNumber, strUserName, strPassword, nCommPort,
									strTTLocation, bChangePassword, strNewPassword, bShowTerminalWindow);

		if(strMacroFile.IsEmpty()) {
			//if this failed, we should have had a message as to why it failed
			return FALSE;
		}
		dwReturn = LaunchTeraTerm(strTTLocation, strMacroFile);
	}
	else{
		strModemAppName = "NXModem";
		//now generate the script file from that template
		CString strCommandString = PrepareModemUploadCommandString( strClaimFilePath,
									strPhoneNumber, strUserName, strPassword, nCommPort,
									bChangePassword, strNewPassword, bShowTerminalWindow);
		dwReturn = LaunchNXModem(strNXOhipModemLocation, strCommandString);
	}

	CString strShortMessage, strLongMessage;

	// (j.jones 2008-12-31 09:34) - PLID 32501 - track which return messages indicate
	// that we at least logged in successfully
	BOOL bSuccessfulLogin = FALSE;

	BOOL bReturnFlag = FALSE;

	if(dwReturn == 0) {
		//could be an error, could be the user closed the app before it finished
		strShortMessage = "File send cancelled.";
		strLongMessage = "The TeraTerm application either could not be started, or closed prematurely.\n"
			"Please try uploading again.\n\n"
			"You can send these claims again by clicking the \"Manually Send Claim File\" button in the dialer setup.";
	}
	else if(dwReturn == 1) {
		//successful upload
		strShortMessage = "File successfully sent.";
		strLongMessage = "Your file has successfully been sent to OHIP.";
		bSuccessfulLogin = TRUE;

		//only this action will return TRUE from this function
		bReturnFlag = TRUE;
	}
	else if(dwReturn == -1) {
		//timeout
		// (j.jones 2008-12-31 10:10) - PLID 32501 - this is now the timeout that occurs AFTER login
		strShortMessage = "Timeout expired. Please try again.";
		strLongMessage = "A timeout occurred during the send process. Please try to send your file again.\n"
			"If you get this message frequently, please contact NexTech for assistance.\n\n"
			"You can send these claims again by clicking the \"Manually Send Claim File\" button in the dialer setup.";
		bSuccessfulLogin = TRUE;
	}
	else if(dwReturn == -2) {
		//no answer
		strShortMessage = "No answer.";
		strLongMessage = "The phone number you dialed did not answer. Please check the number entered in the dialer settings and try sending your file again.\n\n"
			"You can send these claims again by clicking the \"Manually Send Claim File\" button in the dialer setup.";
	}
	else if(dwReturn == -3) {
		//line busy
		strShortMessage = "Line busy.";
		strLongMessage = "The phone number you dialed was busy. Please try sending your file again.\n\n"
			"You can send these claims again by clicking the \"Manually Send Claim File\" button in the dialer setup.";
	}
	else if(dwReturn == -4) {
		//invalid password
		strShortMessage = "Invalid password.";
		strLongMessage = "The login name and password entered was invalid. Please correct this in the dialer settings and try again.\n\n"
			"You can send these claims again by clicking the \"Manually Send Claim File\" button in the dialer setup.";
	}
	else if(dwReturn == -5) {
		//report upload failed
		strShortMessage = "File send failed.";
		strLongMessage = "The file send process did not complete. Please try sending the file again.\n"
			"If you get this message frequently, please contact NexTech for assistance.\n\n"
			"You can send these claims again by clicking the \"Manually Send Claim File\" button in the dialer setup.";
		bSuccessfulLogin = TRUE;
	}
	else if(dwReturn == -6) {
		//modem failed to connect
		strShortMessage = "Could not access modem.";
		strLongMessage = "TeraTerm could not access the modem specified in your Comm Port setting.\n"
			"Please check your Comm Port in your dialer settings, and ensure there is a modem configured for that port.\n\n"
			"You can send these claims again by clicking the \"Manually Send Claim File\" button in the dialer setup.";
	}
	// (j.jones 2008-12-30 16:09) - PLID 32501 - supported password expiration detection
	else if(dwReturn == -7) {
		//password expired
		strShortMessage = "User Password expired.";
		strLongMessage = "OHIP has indicated that your user password has expired.\n"
			"You must edit your dialer settings, enable the 'Change Password' and select a new password to use, then connect again.\n\n"
			"You can send these claims again by clicking the \"Manually Send Claim File\" button in the dialer setup.";
	}
	else if(dwReturn == -8) {
		//entered an old password as a new one
		strShortMessage = "New password already used.";
		strLongMessage = "OHIP has indicated that the new password you entered has already been used.\n"
			"You must edit your dialer settings and enter a new, unique password in the 'Change Password' box.\n\n"
			"You can send these claims again by clicking the \"Manually Send Claim File\" button in the dialer setup.";
	}
	else if(dwReturn == -9) {
		//pre-login timeout
		// (j.jones 2008-12-31 10:10) - PLID 32501 - this is the timeout that occurs BEFORE login
		strShortMessage = "Timeout expired. Please try again.";
		strLongMessage = "A timeout occurred during the send process. Please try to send your file again.\n"
			"If you get this message frequently, please contact NexTech for assistance.\n\n"
			"You can send these claims again by clicking the \"Manually Send Claim File\" button in the dialer setup.";
	}
	else {
		//unknown
		strShortMessage.Format("An unknown problem occurred. (Exit Code: %li)", (long)dwReturn);
		strLongMessage.Format("An unknown problem occurred while trying to send the file.\n"
			"Please contact NexTech for assistance. (Exit Code: %li)\n\n"
			"You can send these claims again by clicking the \"Manually Send Claim File\" button in the dialer setup.", (long)dwReturn);
	}

	// (j.jones 2008-12-31 09:34) - PLID 32501 - if we did log in, and we were
	// changing the password, update the saved password
	if(bSuccessfulLogin && bChangePassword) {
		// (j.jones 2009-08-14 11:32) - PLID 34914 - support per provider
		if(!bSeparateAccountsPerProviders) {
			UpdateOldOHIPPasswordWithNew_Workstation(strNewPassword, strPassword);
		}
		else {
			UpdateOldOHIPPasswordWithNew_Provider(nProviderID, strNewPassword, strPassword);
		}
	}
	
	//we don't have a progress bar to send the "short message", so right now we're not using it
	//SetDlgItemText(IDC_EDIT_PROGRESS, strShortMessage);
	// (s.dhole 2011-01-17 14:06) - PLID 42145 Show Diffrent msg base on EXE call
	strLongMessage.Replace("TeraTerm",strModemAppName);
	AfxMessageBox(strLongMessage);

	return bReturnFlag;
}
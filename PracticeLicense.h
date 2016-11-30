#pragma once

#include <NxNetworkLib\License.h>

class CPracticeLicenseSocket;

// (z.manning 2015-05-19 16:48) - PLID 65971 - Created. Has Practice-specific license functionality as
// the core license code was moved to NxNetworkLib
class CPracticeLicense :
	public CLicense
{
public:
	// (j.jones 2015-12-16 09:44) - PLID 67723 - removed official server name, now the license server calculates it
	CPracticeLicense(const char *id);

	BOOL Initialize(); // (a.walling 2007-03-26 13:58) - PLID 24434 - Return false if cannot initialize

	//Attempts to silently download a new update code, and apply it.
	BOOL TryToUpdateLicense();

	virtual BOOL CheckSchedulerAccess_Enterprise(const ECheckForLicenseReason ecflr) override;
	//TES 12/23/2008 - PLID 32145 - You can now pass in a (fairly brief) feature description, and a link to a help
	// manual page that documents that feature, that information will be incorporated into the message they get if
	// they don't have access, and therefore isn't necessary if you pass in cflrSilent.
	BOOL CheckSchedulerAccess_Enterprise(const ECheckForLicenseReason ecflr, CString strFeatureDescription);
	BOOL CheckSchedulerAccess_Enterprise(const ECheckForLicenseReason ecflr, CString strFeatureDescription, CString strManualBookmark);

	//Asks the user for an update code, and applies it if one is received.
	virtual void PromptForUpdateCode() override;
	//TES 3/13/2006 - If this prompt is during startup, set bNeedRestart to FALSE, then the user will not be advised to restart.
	void PromptForUpdateCode(BOOL bNeedRestart);

	virtual bool LicenseConnectionLostAndRegained() override;

	// (a.walling 2006-12-18 10:17) - PLID 23932 - Handling EMR Provider licensing
	BOOL RequestEMRProvider(IN const long nProviderID);	// request that this provider be licensed.
	// (j.luckoski 2013-01-21 14:45) - PLID 49892 - Added licensing for ERX prescribers
	BOOL RequestERXLicPrescriber(IN const long nPrescriberID);
	BOOL RequestERXMidPrescriber(IN const long nPrescriberID);
	BOOL RequestERXStaffPrescriber(IN const long nPrescriberID);
	// (z.manning 2013-01-31 13:44) - PLID 54954
	BOOL RequestNuanceUser(IN const long nUserID);
	// (z.manning 2015-06-17 16:47) - PLID 66287
	BOOL RequestPortalProvider(IN const long nProviderID);

	// (j.jones 2010-03-24 15:27) - PLID 37670 - Added function to verify that the Practice.exe date
	// is not greater than the support expiration date. May possibly update the license and if so,
	// will return a new dtExpires.
	BOOL IsPracticeExeDatePermitted(COleDateTime &dtExpires);

protected:
	// (z.manning 2015-05-21 09:35) - PLID 65964 - The concept of a "requested folder" only makes sense in Practice currently.
	// Although other places that do licensing (Preferences and NxWeb) used to set this, it was never actually doing anything.
	// So I'm moving this here out of CLicense.
	CString m_strRequestedFolder;

	//This will prompt the user to enter a license key, and an update code.  Returns true if a license key was entered,
	//otherwise false.
	//If pLm is not null, then it will be filled in if a license key was entered.  If no update
	//code was entered, then it will be set to unlicensed, but it will still be filled in.
	// (z.manning 2009-04-02 10:23) - PLID 33813 - I changed the return type of this funtion so that we
	// can have more than 2 possible return values.
	virtual EPromptForNewLicenseReturnValues PromptForNewLicense(const CString &strMessage, OPTIONAL LicensedModule *pLm = NULL, OPTIONAL const ELicensedComponent elc = lcPatient) override;

	//If false, the license code will have reported the reason for failure.  If true, strCode will be the downloaded code.
	bool TryToDownloadCode(OUT CString &strCode, BOOL bSilent = FALSE);
};


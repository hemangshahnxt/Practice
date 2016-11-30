#include "stdafx.h"
#include "PracticeLicense.h"
#include "globalutils.h"
#include "ConnectionDlg.h"
#include "DontShowDlg.h"
#include "SupportExpiredDlg.h"
#include "LicenseKeyDlg.h"
#include "LicenseUpdateCodeDlg.h"
#include "VersionInfo.h"
#include "EnterpriseSchedulerBlockedDlg.h"
#include "PracticeLicenseSocket.h"
#include "LicenseConnectionFailureDlg.h"

// (z.manning 2015-05-19 16:48) - PLID 65971 - Created. Has Practice-specific license functionality as
// the core license code was moved to NxNetworkLib



// (z.manning 2015-06-10 14:25) - PLID 66361 - Pass in true to show message boxes
// (j.jones 2015-12-16 09:44) - PLID 67723 - removed official server name, now the license server calculates it
CPracticeLicense::CPracticeLicense(const char *id)
	: CLicense(GetSubRegistryKey(), TRUE)
{
	ASSERT((int)strlen(id) <= m_maxPath && strlen(id) > 0);
	m_strRequestedFolder = id;

	// (z.manning 2015-05-20 14:37) - PLID 65971 - Destroy the just created license socket and use
	// a practice license socket instead.
	if (m_psktLicense != NULL) {
		delete m_psktLicense;
		m_psktLicense = NULL;
	}
	m_psktLicense = new CPracticeLicenseSocket(this);
}

//DRT 6/15/2006 - PLID 21038 - 1 time initialization pulled out
//	of the constructor.
BOOL CPracticeLicense::Initialize()
{
	// (j.jones 2016-03-03 09:41) - PLID 68479 - if this is a subscription license,
	// we must ensure that the local license server has connectivity to the
	// Nextech License Activation Server
	if(g_pLicense->GetSubscriptionLocalHosted() > 0) {

		CLicenseConnectionFailureDlg dlg(NULL, m_strSubkey, this);
		//this will show the warning if they do not have connectivity,
		//shows nothing if everything is ok
		dlg.CheckShowDialog();

		// (j.jones 2016-03-03 13:55) - PLID 68478 - if logins have been denied,
		// return now, they have already been warned as to why they can't log in
		if (dlg.m_bPreventLogin) {
			return FALSE;
		}
	}

	//TES 10/11/2005 - First, check the support expiration date.
	COleDateTime dtExpires = GetExpirationDate();

	if (m_bExpirationLoaded) { // if this is false, then the connection/request has failed.
		if (dtExpires.GetStatus() != COleDateTime::valid) {
			// (a.walling 2007-03-22 10:22) - PLID 24434
			// Practice expiration invalid! that means that their support expires info is corrupt or missing.
			// Prompt for the update code.
			PromptForUpdateCode(FALSE);

			dtExpires = GetExpirationDate();
			if (dtExpires.GetStatus() == COleDateTime::valid) {
				// good, just continue below
			}
			else {
				CString strOutput;
				strOutput.Format("Licensing information has been initialized, but the support expiration date could not be loaded or is invalid! Please contact Technical Support.");
				OnMessageBox(strOutput);
				return FALSE;
			}
		}

		// (j.jones 2010-03-24 15:06) - PLID 37670 - prevent entering the program
		// if the exe date is greater than the tech support expiration date
	/*	if (!IsPracticeExeDatePermitted(dtExpires)) {
			//we will have already warned the user & logged this
			return FALSE;
		}*/

		//We only continue if we got a valid expiration date; if we didn't, it means the request failed.

		// (b.cardillo 2005-11-11 16:08) - PLID 18294 - We now remind them about their tech 
		// support expiration BEFORE it expires.  So that all the logic is in one place, we just 
		// pass in the expiration date, and the CSupportExpiredDlg does all the checks necessary 
		// and prompts the user as appropriate.
		CSupportExpiredDlg dlg(NULL);
		dlg.m_pLicense = this;
		dlg.Show(dtExpires);

		RequestWorkstation(m_strRequestedFolder);

		// (j.armen 2011-10-27 13:50) - PLID 45796 - If we don't have a license, then we failed
		return m_bGotLicenseForFolder ? TRUE : FALSE;

	}
	else {
		if (m_nLicenseKey > 0) {
			CString strOutput;
			strOutput.Format("A license has been found, but licensing information could not be loaded!");
			OnMessageBox(strOutput);
		}
		else {
			CString strOutput;
			strOutput.Format("License information could not be initialized! Please contact Technical Support.");
			OnMessageBox(strOutput);
		}

		return FALSE;
	}
}

// (z.manning 2009-04-02 10:23) - PLID 33813 - I changed the return type of this funtion so that we
// can have more than 2 possible return values.
CPracticeLicense::EPromptForNewLicenseReturnValues CPracticeLicense::PromptForNewLicense(const CString &strMessage, OPTIONAL LicensedModule *pLm /*= NULL*/, OPTIONAL const ELicensedComponent elc /*= lcPatient*/)
{
	m_bLicenseKeyLoaded = false;
	CLicenseKeyDlg dlg(NULL);
	dlg.m_strMessage = strMessage;
	// (a.walling 2011-04-13 12:20) - PLID 40262
	dlg.m_strOfficialServer = GetOfficialServerName();

	{
		CString strServer = CPracticeLicense::GetSqlServerNameForLicense(m_strSubkey);

		if (strServer.IsEmpty()) {
			// no override
			strServer = GetSqlServerName();
		}

		dlg.m_strServer = strServer;
	}

	if (dlg.DoModal() != IDOK) return pfnlrvCancel;

	m_nLicenseKey = dlg.m_nLicenseKey;

	//Try to activate
	bool bActivated = false;
	UINT response = IDNO;
	CString strOutput;
	strOutput.Format("You must now connect to the internet to activate this license key.  If you do not activate it, "
		"you will be permitted to log into Practice for 48 hours, after which you will no longer be able to access Practice.\n\n"
		"If you need to dial-up to connect to the internet, please do so now, and click Yes to activate your license.  Click No if you plan to activate your license at a later time.");
	response = OnMessageBox(strOutput, MB_YESNO);

	EPromptForNewLicenseReturnValues eReturn;
	if (IDYES == response) {
		eReturn = pfnlrvOk;
		TryToActivateLicense();
	}
	else {
		eReturn = pfnlrvOkNoActivate;
		ApplyLicenseKey();
	}

	if (m_bLicenseKeyLoaded) {
		PromptForUpdateCode();
	}

	//Now, fill in the license module we got, if appropriate.
	if (pLm) CheckServerForModule(elc, *pLm, false);
	return eReturn;
}

void CPracticeLicense::PromptForUpdateCode()
{
	PromptForUpdateCode(TRUE);
}

void CPracticeLicense::PromptForUpdateCode(BOOL bNeedRestart)
{
	CLicenseUpdateCodeDlg dlg(NULL);
	CString strCode;
	int nReturn = dlg.DoModal();
	if (nReturn == IDOK) {
		strCode = dlg.m_strUpdateCode;
	}
	else if (nReturn == ID_DOWNLOAD_CODE) {
		TryToDownloadCode(strCode);
	}

	if (!strCode.IsEmpty()) {

		_LICENSE_UPDATE_CODE_VERSIONED *pLuc = (_LICENSE_UPDATE_CODE_VERSIONED*)new BYTE[sizeof(_LICENSE_UPDATE_CODE_VERSIONED) + strCode.GetLength() + 1];
		strcpy(pLuc->szCode, strCode);
		// (z.manning 2013-08-28 15:23) - PLID 55816 - Include the version
		pLuc->dwVersion = LICENSE_VERSION;
		void *pOutBuffer = NULL;
		unsigned long nOutSize = 0;
		if (!SyncPacketWait(PACKET_TYPE_LICENSE_UPDATE_CODE_VERSIONED, PACKET_TYPE_UPDATE_CODE_RESPONSE, pLuc, sizeof(_LICENSE_UPDATE_CODE_VERSIONED) + strCode.GetLength() + 1, pOutBuffer, nOutSize)) {
			CString strOutput;
			strOutput.Format("Failed to connect to the license server.  The update code was not applied.");
			OnMessageBox(strOutput);
		}
		else {
			CString strOutput;
			_LICENSE_UPDATE_CODE_RESPONSE *pLucr = (_LICENSE_UPDATE_CODE_RESPONSE*)pOutBuffer;
			switch (pLucr->elrResponse) {
			case lrSuccess:
				strOutput.Format("Update code successfully applied!%s", bNeedRestart ? "\nPlease restart Practice for these changes to take effect." : "");
				OnMessageBox(strOutput);
				ClearCache();
				if (!m_bGotLicenseForFolder) {
					RequestWorkstation(m_strRequestedFolder);
				}
				break;
			case lrFailureBadUpdateCode:
				strOutput.Format("The update code you entered was not valid.");
				OnMessageBox(strOutput);
				PromptForUpdateCode();
				break;
			default:
				strOutput.Format("The license server reported an unexpected error.  The update code was not applied.");
				OnMessageBox(strOutput);
				break;
			}
		}
		delete[] pLuc;
	}
}

bool CPracticeLicense::TryToDownloadCode(OUT CString &strCode, BOOL bSilent /*= FALSE*/)
{
	try {
		//DRT 4/10/2007 - PLID 25557 - Call the global function instead of copying the details ourself.
		NxSocketUtils::HSERVER hLicenseActivationServer = ConnectToLicenseActivationServer(m_strSubkey);

		if (!hLicenseActivationServer) {
			if (!bSilent){
				CString strOutput;
				strOutput.Format("Practice could not connect to the NexTech License Activation Server.  Please check your connection to the internet, and any security software which may be preventing this access.  If the problem still persists, please call NexTech Technical Support at (888) 417-8464.");
				OnMessageBox(strOutput);
			}
		}
		//TES 9/12/2007 - PLID 25873 - Pass in the current date (our current date, which could be different from
		// NxLicenseActivationServer's current date, for example if we're in New Zealand).
		_UPDATE_CODE_REQUEST_VERSIONED_DATED ucrvd;
		ucrvd.dwKey = GetLicenseKey();
		ucrvd.dwFlags = 0;
		ucrvd.dwVersion = LICENSE_VERSION;
		COleDateTime dtNow = COleDateTime::GetCurrentTime();
		ucrvd.wYear = dtNow.GetYear();
		ucrvd.bMonth = dtNow.GetMonth();
		ucrvd.bDay = dtNow.GetDay();

		void *pOutBuffer = NULL;
		unsigned long nOutSize = 0;
		bool bReturn = false;
		if (!NxSocketUtils::SyncPacketWait(hLicenseActivationServer, PACKET_TYPE_UPDATE_CODE_REQUEST_VERSIONED_DATED, PACKET_TYPE_UPDATE_CODE_REQUEST_RESPONSE_VERSIONED, &ucrvd, sizeof(_UPDATE_CODE_REQUEST_VERSIONED_DATED), pOutBuffer, nOutSize)) {
			if (!bSilent){
				CString strOutput;
				strOutput.Format("Practice did not receive a response from the NexTech License Activation Server.  Please check your connection to the internet, and any security software which may be preventing this access.  If the problem still persists, please call NexTech Technical Support at (888) 417-8464.");
				OnMessageBox(strOutput);
			}
		}
		else {
			CString strOutput;
			_UPDATE_CODE_REQUEST_RESPONSE_VERSIONED *pUcrrv = (_UPDATE_CODE_REQUEST_RESPONSE_VERSIONED*)pOutBuffer;
			switch (pUcrrv->elarResponse) {
			case larInternalError:
				if (!bSilent){
					strOutput.Format("The License Activation Server reported an internal error.  Please contact NexTech Technical Support at (888) 417-8464.");
					OnMessageBox(strOutput);
				}
				break;
			case larNotFound:
				if (!bSilent) PromptForNewLicense("Your license key is not registered as a valid key in NexTech's database.  Please enter a valid key.  If you do not know your license key, or if you believe your current key is correct, please click Cancel and contact NexTech Technical Support at (888) 417-8464.");
				break;
			case larSuccess:
				strCode = pUcrrv->szCode;
				bReturn = true;
				break;
			default:
				ASSERT(FALSE);
				ThrowNxException("CPracticeLicense::TryToDownloadCode: Activation Server returned an unexpected value %li", pUcrrv->elarResponse);
				break;
			}
		}
		NxSocketUtils::Disconnect(hLicenseActivationServer);
		return bReturn;

	}
	catch (CException *e) {
		if (!bSilent) HandleException(e, "Error in CPracticeLicense::TryToDownloadCode()", __LINE__, __FILE__);
		else e->Delete();
	}
	catch (_com_error e) {
		if (!bSilent) HandleException(e, "Error in CPracticeLicense::TryToDownloadCode()", __LINE__, __FILE__);
	}
	catch (...) {
		if (!bSilent) HandleException(NULL, "Error in CPracticeLicense::TryToDownloadCode()", __LINE__, __FILE__);
	}
	return false;
}

BOOL CPracticeLicense::TryToUpdateLicense()
{
	//TES 1/11/2007 - PLID 24189 - We are now attempting to update the license, so log this attempt, whether or not its
	// successful (if they don't have internet access, for example, we don't want to be trying and failing every single time
	// they start Practice).
	// (j.jones 2016-03-08 13:50) - PLID 68478 - the ConfigRT value of LastLicenseUpdateAttempt
	// is now encrypted to prevent tampering, this function will update the encrypted date to
	// be the current time
	UpdateLastLicenseUpdateAttemptToCurrentTime();

	CString strCode;
	TryToDownloadCode(strCode, TRUE);
	BOOL bSuccess = FALSE;

	if (!strCode.IsEmpty()) {

		_LICENSE_UPDATE_CODE_VERSIONED *pLuc = (_LICENSE_UPDATE_CODE_VERSIONED*)new BYTE[sizeof(_LICENSE_UPDATE_CODE_VERSIONED) + strCode.GetLength() + 1];
		strcpy(pLuc->szCode, strCode);
		// (z.manning 2013-08-28 15:15) - PLID 55816 - Include the version
		pLuc->dwVersion = LICENSE_VERSION;
		void *pOutBuffer = NULL;
		unsigned long nOutSize = 0;
		if (SyncPacketWait(PACKET_TYPE_LICENSE_UPDATE_CODE_VERSIONED, PACKET_TYPE_UPDATE_CODE_RESPONSE, pLuc, sizeof(_LICENSE_UPDATE_CODE_VERSIONED) + strCode.GetLength() + 1, pOutBuffer, nOutSize)) {
			_LICENSE_UPDATE_CODE_RESPONSE *pLucr = (_LICENSE_UPDATE_CODE_RESPONSE*)pOutBuffer;
			switch (pLucr->elrResponse) {
			case lrSuccess:
				bSuccess = TRUE;
				ClearCache();
				if (!m_bGotLicenseForFolder) {
					RequestWorkstation(m_strRequestedFolder);
				}
				break;
			default:
				break;
			}
		}
		delete[] pLuc;
	}

	return bSuccess;
}

// (a.walling 2006-12-18 10:17) - PLID 23932 - Handling EMR Provider licensing
// request that this provider be licensed.
BOOL CPracticeLicense::RequestEMRProvider(IN const long nProviderID)
{
	LoadEMRProviders();
	if (m_bEMRProvidersLoaded) {
		if (m_nEMRProvidersAllowed) {
			bool bTmp;
			COleDateTime dtTmp;
			int nEMRProvidersUsed;
			CString strEMRProviders, strProviderID;
			strProviderID.Format("%li", nProviderID);
			CString strOutput;
			ELicenseResponse elr = CheckServerForLicense(lfEMRProviders, true, strProviderID, bTmp, m_nEMRProvidersAllowed, nEMRProvidersUsed,
				dtTmp, strEMRProviders);
			switch (elr) {
			case lrSuccess:
				PipeStringToDWordArray(strEMRProviders, m_dwaEMRProvidersUsed);
				return TRUE;
				break;
			case lrFailureBadPacket:
			case lrFailureUnknownError:
				// (a.walling 2009-10-16 08:56) - PLID 36619
				strOutput.Format("An unexpected error%s occurred while attempting to access the license information for %s.  Please contact Nextech for assistance if this problem persists.", strEMRProviders.GetLength() ? " (Description: " + strEMRProviders + ")" : "", GetDescriptionFromFeature(lfEMRProviders));
				OnMessageBox(strOutput);
				return FALSE;
				break;
			case lrFailureDataNotFound:
				strOutput.Format("The license server has reported failure to connect to your database%s.  Please check the connection settings on your server.", strEMRProviders.GetLength() ? " (Last error: " + strEMRProviders + ")" : "");
				OnMessageBox(strOutput);
				return FALSE;
				break;
			case lrFailureLicenseInvalid:
			{
				CString strName = GetExistingContactName(nProviderID);
				strOutput.Format("The provider %s (%li) has been de-activated.  They can no longer be the main provider of an EMN.", strName, nProviderID);
				OnMessageBox(strOutput);
				return FALSE;
			}
			break;
			case lrFailureLicenseExpired:
				ASSERT(FALSE);
			case lrFailureLicenseUsageExpired:
			{
				CString strMsg;
				strMsg.Format("The number of providers licensed for EMR purchased by your office is %i.", m_nEMRProvidersAllowed);
				switch (m_nEMRProvidersAllowed) {
				case 0:
					// Nothing to add
					break;
				case 1:
					strMsg += "  This license has been allocated to another provider.";
					break;
				case 2:
					strMsg += "  Both licenses have been allocated to other providers.";
					break;
				default:
					strMsg += "  All of these licenses have been allocated to other providers.";
					break;
				}
				strOutput = strMsg + "  A new license could not be obtained for this provider.\n\n"
					"If you feel you have received this message in error or you "
					"wish to purchase additional licenses, please contact NexTech "
					"at 1-888-417-8464.";
				OnMessageBox(strOutput);
			}
			return FALSE;
			break;
			case lrFailureLicenseNotFound:
				PromptForNewLicense(LICENSE_EMPTY_MESSAGE);
				return FALSE;
				break;
			case lrFailureDataInvalid:
			{
				CString strMessage = LICENSE_CORRUPT_MESSAGE;
				if (strEMRProviders.GetLength()) {
					strMessage = "Your license information has been corrupted and cannot be read (Error description: " + strEMRProviders + ").  In order to continue using Practice, you must re-enter your license key below.  If you do not know your license key, please call Nextech Technical Support, at (888) 417-8464.";
				}
				PromptForNewLicense(strMessage);
			}
			return FALSE;
			break;
			}
		}
		else {
			PromptForUpdateCode();
		}
	}

	return FALSE;
}

// (z.manning 2013-01-31 14:48) - PLID 54954
BOOL CPracticeLicense::RequestNuanceUser(IN const long nUserID)
{
	LoadNuanceUsers();
	if (m_bNuanceUsersLoaded)
	{
		if (m_nNuanceUsersAllowed)
		{
			bool bTmp;
			COleDateTime dtTmp;
			int nNuanceUsersUsed;
			CString strNuanceUsers, strUserID;
			strUserID.Format("%li", nUserID);
			CString strOutput;
			ELicenseResponse elr = CheckServerForLicense(lfNuanceUsers, true, strUserID, bTmp, m_nNuanceUsersAllowed, nNuanceUsersUsed,
				dtTmp, strNuanceUsers);
			switch (elr) {
			case lrSuccess:
				PipeStringToDWordArray(strNuanceUsers, m_dwaNuanceUsersUsed);
				return TRUE;
				break;
			case lrFailureBadPacket:
			case lrFailureUnknownError:
				strOutput.Format("An unexpected error%s occurred while attempting to access the license information for %s.  "
					"Please contact Nextech for assistance if this problem persists."
					, strNuanceUsers.GetLength() ? " (Description: " + strNuanceUsers + ")" : ""
					, GetDescriptionFromFeature(lfNuanceUsers));
				OnMessageBox(strOutput);
				return FALSE;
				break;
			case lrFailureDataNotFound:
				strOutput.Format("The license server has reported failure to connect to your database%s.  "
					"Please check the connection settings on your server."
					, strNuanceUsers.GetLength() ? " (Last error: " + strNuanceUsers + ")" : "");
				OnMessageBox(strOutput);
				return FALSE;
				break;
			case lrFailureLicenseInvalid:
			{
				CString strName = GetExistingContactName(nUserID);
				strOutput.Format("The user %s (%li) has been de-activated.  He/She can no longer use dictation."
					, strName, nUserID);
				OnMessageBox(strOutput);
				return FALSE;
			}
			break;
			case lrFailureLicenseExpired:
				ASSERT(FALSE);
			case lrFailureLicenseUsageExpired:
			{
				CString strMsg;
				strMsg.Format("The number of users licensed for dictation purchased by your office is %i.", m_nNuanceUsersAllowed);
				switch (m_nNuanceUsersAllowed) {
				case 0:
					// Nothing to add
					break;
				case 1:
					strMsg += "  This license has been allocated to another user.";
					break;
				case 2:
					strMsg += "  Both licenses have been allocated to other users.";
					break;
				default:
					strMsg += "  All of these licenses have been allocated to other users.";
					break;
				}
				strOutput = strMsg + "  A new license could not be obtained for this user.\n\n"
					"If you feel you have received this message in error or you "
					"wish to purchase additional licenses, please contact NexTech "
					"at 1-888-417-8464.";
				OnMessageBox(strOutput);
			}
			return FALSE;
			break;
			case lrFailureLicenseNotFound:
				PromptForNewLicense(LICENSE_EMPTY_MESSAGE);
				return FALSE;
				break;
			case lrFailureDataInvalid:
			{
				CString strMessage = LICENSE_CORRUPT_MESSAGE;
				if (strNuanceUsers.GetLength()) {
					strMessage = "Your license information has been corrupted and cannot be read (Error description: "
						+ strNuanceUsers + ").  In order to continue using Practice, you must re-enter your license key below.  "
						"If you do not know your license key, please call Nextech Technical Support, at (888) 417-8464.";
				}
				PromptForNewLicense(strMessage);
			}
			return FALSE;
			break;
			}
		}
		else {
			PromptForUpdateCode();
		}
	}

	return FALSE;
}

// (z.manning 2015-06-17 16:49) - PLID 66287
BOOL CPracticeLicense::RequestPortalProvider(IN const long nProviderID)
{
	LoadPortalProviders();
	if (m_bPortalProvidersLoaded)
	{
		if (m_nPortalProvidersAllowed)
		{
			bool bTmp;
			COleDateTime dtTmp;
			int nPortalProvidersUsed;
			CString strPortalProviders, strProviderID;
			strProviderID.Format("%li", nProviderID);
			CString strOutput;
			ELicenseResponse elr = CheckServerForLicense(lfPortalProviders, true, strProviderID, bTmp, m_nPortalProvidersAllowed, nPortalProvidersUsed,
				dtTmp, strPortalProviders);
			switch (elr) {
			case lrSuccess:
				PipeStringToDWordArray(strPortalProviders, m_dwaPortalProvidersUsed);
				return TRUE;
				break;
			case lrFailureBadPacket:
			case lrFailureUnknownError:
				strOutput.Format("An unexpected error%s occurred while attempting to access the license information for %s.  "
					"Please contact Nextech for assistance if this problem persists."
					, strPortalProviders.GetLength() ? " (Description: " + strPortalProviders + ")" : ""
					, GetDescriptionFromFeature(lfPortalProviders));
				OnMessageBox(strOutput);
				return FALSE;
				break;
			case lrFailureDataNotFound:
				strOutput.Format("The license server has reported failure to connect to your database%s.  "
					"Please check the connection settings on your server."
					, strPortalProviders.GetLength() ? " (Last error: " + strPortalProviders + ")" : "");
				OnMessageBox(strOutput);
				return FALSE;
				break;
			case lrFailureLicenseInvalid:
			{
				CString strName = GetExistingContactName(nProviderID);
				strOutput.Format("The provider %s (%li) has been de-activated.  He/She can no longer use dictation."
					, strName, nProviderID);
				OnMessageBox(strOutput);
				return FALSE;
			}
			break;
			case lrFailureLicenseExpired:
				ASSERT(FALSE);
			case lrFailureLicenseUsageExpired:
			{
				CString strMsg;
				strMsg.Format("The number of providers licensed for dictation purchased by your office is %i.", m_nPortalProvidersAllowed);
				switch (m_nPortalProvidersAllowed) {
				case 0:
					// Nothing to add
					break;
				case 1:
					strMsg += "  This license has been allocated to another provider.";
					break;
				case 2:
					strMsg += "  Both licenses have been allocated to other providers.";
					break;
				default:
					strMsg += "  All of these licenses have been allocated to other providers.";
					break;
				}
				strOutput = strMsg + "  A new license could not be obtained for this provider.\n\n"
					"If you feel you have received this message in error or you "
					"wish to purchase additional licenses, please contact NexTech "
					"at 1-888-417-8464.";
				OnMessageBox(strOutput);
			}
			return FALSE;
			break;
			case lrFailureLicenseNotFound:
				PromptForNewLicense(LICENSE_EMPTY_MESSAGE);
				return FALSE;
				break;
			case lrFailureDataInvalid:
			{
				CString strMessage = LICENSE_CORRUPT_MESSAGE;
				if (strPortalProviders.GetLength()) {
					strMessage = "Your license information has been corrupted and cannot be read (Error description: "
						+ strPortalProviders + ").  In order to continue using Practice, you must re-enter your license key below.  "
						"If you do not know your license key, please call Nextech Technical Support, at (888) 417-8464.";
				}
				PromptForNewLicense(strMessage);
			}
			return FALSE;
			break;
			}
		}
		else {
			PromptForUpdateCode();
		}
	}

	return FALSE;
}

// (j.luckoski 01-28-13) - PLID 49892 - ERX Prescriber License Objects
BOOL CPracticeLicense::RequestERXLicPrescriber(IN const long nPrescriberID)
{
	LoadERXLicPrescribers();
	if (m_bERXLicPrescribersLoaded) {
		if (m_nERXLicPrescribersAllowed) {
			bool bTmp;
			COleDateTime dtTmp;
			int nERXLicPrescribersUsed;
			CString strERXLicPrescribers, strPrescriberID;
			strPrescriberID.Format("%li", nPrescriberID);
			CString strOutput;
			ELicenseResponse elr = CheckServerForLicense(lfERXLicPrescribers, true, strPrescriberID, bTmp, m_nERXLicPrescribersAllowed, nERXLicPrescribersUsed,
				dtTmp, strERXLicPrescribers);
			switch (elr) {
			case lrSuccess:
				PipeStringToDWordArray(strERXLicPrescribers, m_dwaERXLicPrescribersUsed);
				return TRUE;
				break;
			case lrFailureBadPacket:
			case lrFailureUnknownError:
				// (a.walling 2009-10-16 08:56) - PLID 36619
				strOutput.Format("An unexpected error%s occurred while attempting to access the license information for %s.  Please contact Nextech for assistance if this problem persists.", strERXLicPrescribers.GetLength() ? " (Description: " + strERXLicPrescribers + ")" : "", GetDescriptionFromFeature(lfERXLicPrescribers));
				OnMessageBox(strOutput);
				return FALSE;
				break;
			case lrFailureDataNotFound:
				strOutput.Format("The license server has reported failure to connect to your database%s.  Please check the connection settings on your server.", strERXLicPrescribers.GetLength() ? " (Last error: " + strERXLicPrescribers + ")" : "");
				OnMessageBox(strOutput);
				return FALSE;
				break;
			case lrFailureLicenseInvalid:
			{
				CString strName = GetExistingContactName(nPrescriberID);
				// (j.luckoski 2013-02-13 16:13) - PLID 49892 - Fixed grammar
				strOutput.Format("The prescriber %s (%li) has been de-activated.  He/she can no longer prescribe medications.", strName, nPrescriberID);
				OnMessageBox(strOutput);
				return FALSE;
			}
			break;
			case lrFailureLicenseExpired:
				ASSERT(FALSE);
			case lrFailureLicenseUsageExpired:
			{
				CString strMsg;
				strMsg.Format("The number of licensed prescribers purchased by your office is %i.", m_nERXLicPrescribersAllowed);
				switch (m_nERXLicPrescribersAllowed) {
				case 0:
					// Nothing to add
					break;
				case 1:
					strMsg += "  This license has been allocated to another prescriber.";
					break;
				case 2:
					strMsg += "  Both licenses have been allocated to other prescribers.";
					break;
				default:
					strMsg += "  All of these licenses have been allocated to other prescribers.";
					break;
				}
				strOutput = strMsg + "  A new license could not be obtained for this prescriber.\n\n"
					"If you feel you have received this message in error or you "
					"wish to purchase additional licenses, please contact NexTech "
					"at 1-888-417-8464.";
				OnMessageBox(strOutput);
			}
			return FALSE;
			break;
			case lrFailureLicenseNotFound:
				PromptForNewLicense(LICENSE_EMPTY_MESSAGE);
				return FALSE;
				break;
			case lrFailureDataInvalid:
			{
				CString strMessage = LICENSE_CORRUPT_MESSAGE;
				if (strERXLicPrescribers.GetLength()) {
					strMessage = "Your license information has been corrupted and cannot be read (Error description: " + strERXLicPrescribers + ").  In order to continue using Practice, you must re-enter your license key below.  If you do not know your license key, please call Nextech Technical Support, at (888) 417-8464.";
				}
				PromptForNewLicense(strMessage);
			}
			return FALSE;
			break;
			}
		}
		else {
			PromptForUpdateCode();
		}
	}

	return FALSE;
}

BOOL CPracticeLicense::RequestERXMidPrescriber(IN const long nPrescriberID)
{
	LoadERXMidPrescribers();
	if (m_bERXMidPrescribersLoaded) {
		if (m_nERXMidPrescribersAllowed) {
			bool bTmp;
			COleDateTime dtTmp;
			int nERXMidPrescribersUsed;
			CString strERXMidPrescribers, strPrescriberID;
			strPrescriberID.Format("%li", nPrescriberID);
			CString strOutput;
			ELicenseResponse elr = CheckServerForLicense(lfERXMidPrescribers, true, strPrescriberID, bTmp, m_nERXMidPrescribersAllowed, nERXMidPrescribersUsed,
				dtTmp, strERXMidPrescribers);
			switch (elr) {
			case lrSuccess:
				PipeStringToDWordArray(strERXMidPrescribers, m_dwaERXMidPrescribersUsed);
				return TRUE;
				break;
			case lrFailureBadPacket:
			case lrFailureUnknownError:
				// (a.walling 2009-10-16 08:56) - PLID 36619
				strOutput.Format("An unexpected error%s occurred while attempting to access the license information for %s.  Please contact Nextech for assistance if this problem persists.", strERXMidPrescribers.GetLength() ? " (Description: " + strERXMidPrescribers + ")" : "", GetDescriptionFromFeature(lfERXMidPrescribers));
				OnMessageBox(strOutput);
				return FALSE;
				break;
			case lrFailureDataNotFound:
				strOutput.Format("The license server has reported failure to connect to your database%s.  Please check the connection settings on your server.", strERXMidPrescribers.GetLength() ? " (Last error: " + strERXMidPrescribers + ")" : "");
				OnMessageBox(strOutput);
				return FALSE;
				break;
			case lrFailureLicenseInvalid:
			{
				CString strName = GetExistingContactName(nPrescriberID);
				// (j.luckoski 2013-02-13 16:13) - PLID 49892 - Fixed grammar
				strOutput.Format("The prescriber %s (%li) has been de-activated.  He/she can no longer prescribe medications.", strName, nPrescriberID);
				OnMessageBox(strOutput);
				return FALSE;
			}
			break;
			case lrFailureLicenseExpired:
				ASSERT(FALSE);
			case lrFailureLicenseUsageExpired:
			{
				CString strMsg;
				strMsg.Format("The number of midlevel prescribers purchased by your office is %i.", m_nERXMidPrescribersAllowed);
				switch (m_nERXMidPrescribersAllowed) {
				case 0:
					// Nothing to add
					break;
				case 1:
					strMsg += "  This license has been allocated to another prescriber.";
					break;
				case 2:
					strMsg += "  Both licenses have been allocated to other prescribers.";
					break;
				default:
					strMsg += "  All of these licenses have been allocated to other prescribers.";
					break;
				}
				strOutput = strMsg + "  A new license could not be obtained for this prescriber.\n\n"
					"If you feel you have received this message in error or you "
					"wish to purchase additional licenses, please contact NexTech "
					"at 1-888-417-8464.";
				OnMessageBox(strOutput);
			}
			return FALSE;
			break;
			case lrFailureLicenseNotFound:
				PromptForNewLicense(LICENSE_EMPTY_MESSAGE);
				return FALSE;
				break;
			case lrFailureDataInvalid:
			{
				CString strMessage = LICENSE_CORRUPT_MESSAGE;
				if (strERXMidPrescribers.GetLength()) {
					strMessage = "Your license information has been corrupted and cannot be read (Error description: " + strERXMidPrescribers + ").  In order to continue using Practice, you must re-enter your license key below.  If you do not know your license key, please call Nextech Technical Support, at (888) 417-8464.";
				}
				PromptForNewLicense(strMessage);
			}
			return FALSE;
			break;
			}
		}
		else {
			PromptForUpdateCode();
		}
	}

	return FALSE;
}

BOOL CPracticeLicense::RequestERXStaffPrescriber(IN const long nPrescriberID)
{
	LoadERXStaffPrescribers();
	if (m_bERXStaffPrescribersLoaded) {
		if (m_nERXStaffPrescribersAllowed) {
			bool bTmp;
			COleDateTime dtTmp;
			int nERXStaffPrescribersUsed;
			CString strERXStaffPres, strPrescriberID;
			strPrescriberID.Format("%li", nPrescriberID);
			CString strOutput;
			ELicenseResponse elr = CheckServerForLicense(lfERXStaffPrescribers, true, strPrescriberID, bTmp, m_nERXStaffPrescribersAllowed, nERXStaffPrescribersUsed,
				dtTmp, strERXStaffPres);
			switch (elr) {
			case lrSuccess:
				PipeStringToDWordArray(strERXStaffPres, m_dwaERXStaffPrescribersUsed);
				return TRUE;
				break;
			case lrFailureBadPacket:
			case lrFailureUnknownError:
				// (a.walling 2009-10-16 08:56) - PLID 36619
				strOutput.Format("An unexpected error%s occurred while attempting to access the license information for %s.  Please contact Nextech for assistance if this problem persists.", strERXStaffPres.GetLength() ? " (Description: " + strERXStaffPres + ")" : "", GetDescriptionFromFeature(lfERXStaffPrescribers));
				OnMessageBox(strOutput);
				return FALSE;
				break;
			case lrFailureDataNotFound:
				strOutput.Format("The license server has reported failure to connect to your database%s.  Please check the connection settings on your server.", strERXStaffPres.GetLength() ? " (Last error: " + strERXStaffPres + ")" : "");
				OnMessageBox(strOutput);
				return FALSE;
				break;
			case lrFailureLicenseInvalid:
			{
				CString strName = GetExistingContactName(nPrescriberID);
				// (j.luckoski 2013-02-13 16:13) - PLID 49892 - Fixed grammar
				strOutput.Format("The prescriber %s (%li) has been de-activated.  He/she can no longer prescribe medications.", strName, nPrescriberID);
				OnMessageBox(strOutput);
				return FALSE;
			}
			break;
			case lrFailureLicenseExpired:
				ASSERT(FALSE);
			case lrFailureLicenseUsageExpired:
			{
				CString strMsg;
				strMsg.Format("The number of nurse/staff prescribers purchased by your office is %i.", m_nERXStaffPrescribersAllowed);
				switch (m_nERXStaffPrescribersAllowed) {
				case 0:
					// Nothing to add
					break;
				case 1:
					strMsg += "  This license has been allocated to another prescriber.";
					break;
				case 2:
					strMsg += "  Both licenses have been allocated to other prescribers.";
					break;
				default:
					strMsg += "  All of these licenses have been allocated to other prescribers.";
					break;
				}
				strOutput = strMsg + "  A new license could not be obtained for this prescriber.\n\n"
					"If you feel you have received this message in error or you "
					"wish to purchase additional licenses, please contact NexTech "
					"at 1-888-417-8464.";
				OnMessageBox(strOutput);
			}
			return FALSE;
			break;
			case lrFailureLicenseNotFound:
				PromptForNewLicense(LICENSE_EMPTY_MESSAGE);
				return FALSE;
				break;
			case lrFailureDataInvalid:
			{
				CString strMessage = LICENSE_CORRUPT_MESSAGE;
				if (strERXStaffPres.GetLength()) {
					strMessage = "Your license information has been corrupted and cannot be read (Error description: " + strERXStaffPres + ").  In order to continue using Practice, you must re-enter your license key below.  If you do not know your license key, please call Nextech Technical Support, at (888) 417-8464.";
				}
				PromptForNewLicense(strMessage);
			}
			return FALSE;
			break;
			}
		}
		else {
			PromptForUpdateCode();
		}
	}

	return FALSE;
}

BOOL CPracticeLicense::CheckSchedulerAccess_Enterprise(const ECheckForLicenseReason ecflr)
{
	return CheckSchedulerAccess_Enterprise(ecflr, "", "");
}

BOOL CPracticeLicense::CheckSchedulerAccess_Enterprise(const ECheckForLicenseReason ecflr, CString strFeatureDescription)
{
	return CheckSchedulerAccess_Enterprise(ecflr, strFeatureDescription, "");
}

//TES 12/10/2008 - PLID 32145 - Called to determine if the user can access areas that are barred to "Scheduler Standard"
// users.
BOOL CPracticeLicense::CheckSchedulerAccess_Enterprise(const ECheckForLicenseReason ecflr, CString strFeatureDescription, CString strManualBookmark)
{
	//TES 12/10/2008 - PLID 32145 - Do they have the standard module?
	DWORD dwSchedModules = GetSchedulerLevel(cflrJustCheckingLicenseNoUI);
	if (dwSchedModules & SL_STANDARD) {
		//TES 12/10/2008 - PLID 32145 - If they have standard, then they're not allowed in, tell them why if we're not
		// being silent.
		if (ecflr != cflrSilent) {
			//TES 12/23/2008 - PLID 32145 - We now have a designated dialog for this, pass in any descriptions we've been
			// given and show it to the user.
			CEnterpriseSchedulerBlockedDlg dlg(NULL);
			dlg.m_strFeatureDescription = strFeatureDescription;
			dlg.m_strManualBookmark = strManualBookmark;
			dlg.DoModal();
		}
		return FALSE;
	}
	else {
		//TES 12/10/2008 - PLID 32145 - Check the license normally.
		return (GetSchedulerLevel(ecflr) != 0);
	}
	return TRUE;
}

bool CPracticeLicense::LicenseConnectionLostAndRegained()
{
	CLicense::LicenseConnectionLostAndRegained();

	RequestWorkstation(m_strRequestedFolder);

	//Let our calling function know what's going on.  If we were unable
	//	to get a license for this login, we will have to quit the 
	//	program.
	return m_bGotLicenseForFolder;
}

// (j.jones 2010-03-24 15:27) - PLID 37670 - Added function to verify that the Practice.exe date
// is not greater than the support expiration date. May possibly update the license and if so,
// will return a new dtExpires.
BOOL CPracticeLicense::IsPracticeExeDatePermitted(COleDateTime &dtExpires)
{
	//this function is used to prevent entering the program
	//if the exe date is greater than the tech support expiration date

	// Get this exe's path
	// (j.armen 2011-10-24 15:53) - PLID 46137 - We have the practice exe path stored, so let's use it
	CString strExePath = GetPracPath(PracPath::PracticeEXEPath);

	COleDateTime dtExeDate;
	dtExeDate.SetStatus(COleDateTime::invalid);

	//find the file
	CFileFind ff;
	if (!strExePath.IsEmpty() && ff.FindFile(strExePath, 0)) {
		// Get the file modified time
		ff.FindNextFile();
		CTime tmpTime;
		ff.GetLastWriteTime(tmpTime);

		COleDateTime dt(tmpTime.GetYear(), tmpTime.GetMonth(), tmpTime.GetDay(), 0, 0, 0);
		dtExeDate = dt;
	}

	if (dtExeDate.GetStatus() == COleDateTime::invalid) {
		CString strOutput;
		strOutput.Format("Licensing information has been initialized, but the date of the Practice.exe file is invalid! Please contact Technical Support.");
		OnMessageBox(strOutput);
		return FALSE;
	}
	else if (dtExeDate > dtExpires) {

		//first prompt for the update code.
		PromptForUpdateCode(FALSE);

		//now we have to re-verify the date is valid
		dtExpires = GetExpirationDate();
		if (dtExpires.GetStatus() == COleDateTime::invalid) {
			CString strOutput;
			strOutput.Format("Licensing information has been initialized, but the support expiration date could not be loaded or is invalid! Please contact Technical Support.");
			OnMessageBox(strOutput);
			return FALSE;
		}

		//still expired?
		if (dtExeDate > dtExpires) {

			CString strOutput;
			strOutput.Format("This version of Practice (%s) was created after your support expiration date (%s), and cannot be used. Please contact Technical Support.",
				FormatDateTimeForInterface(dtExeDate, NULL, dtoDate), FormatDateTimeForInterface(dtExpires, NULL, dtoDate));
			OnMessageBox(strOutput);
			return FALSE;
		}
	}

	return TRUE;
}

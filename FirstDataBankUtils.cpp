#include "stdafx.h"
#include "NxPracticeSharedLib\SharedFirstDataBankUtils.h"
#include "FirstDataBankUtils.h"
#include "InternationalUtils.h"


using namespace FirstDataBank;

//TES 11/12/2009 - PLID 36241 - All FirstDataBank importing is now done in NxServer, which made 90% of the FirstDataBankUtils code invalid, 
// so it's been taken out.

//TES 10/1/2009 - PLID 34252 - Checks FirstDataBank data for the given NDC.  Returns true, and sets bContinueEditing
// to false, if the NDC number exists and has the same name as strPracticeDrugName.  If it returns false, it will 
// ask the user if they want to continue editing, and set bContinueEditing accordingly.  Throws exceptions.
using namespace ADODB;
bool FirstDataBank::ValidateNDCNumber(const CString &strNDC, const CString &strPracticeDrugName, OUT bool &bContinueEditing)
{
	//TES 10/1/2009 - PLID 34252 - Does this NDC number exist at all?
	// (a.walling 2009-11-09 15:36) - PLID 36144 - Reference FirstDataBank database
	//TES 11/18/2009 - PLID 36355 - Check against RNDC14.LN60, not RNDC14.LN
	// (j.jones 2010-01-26 16:37) - PLID 37078 - it needs to compare to RNDC14.LN60 *or* RMIID1_MED.MED_MEDID_DESC
	_RecordsetPtr rsRealNDC = CreateParamRecordset("SELECT TOP 1 LN60 AS Name FROM FirstDataBank..RNDC14 "
		"WHERE FirstDataBank..RNDC14.NDC = {STRING} "
		"UNION SELECT TOP 1 FirstDataBank..RMIID1_MED.MED_MEDID_DESC AS Name FROM FirstDataBank..RMIID1_MED "
		"INNER JOIN FirstDataBank..RMINDC1_NDC_MEDID ON FirstDataBank..RMIID1_MED.MEDID = FirstDataBank..RMINDC1_NDC_MEDID.MEDID "
		"WHERE FirstDataBank..RMINDC1_NDC_MEDID.NDC = {STRING}", strNDC, strNDC);
	if(rsRealNDC->eof) {
		if(IDYES == MsgBox(MB_YESNO, "Warning: The NDC Number you have entered could not be found in the FirstDataBank database.  It may be invalid.\r\n"
			"Would you like to return to editing the field?")) {
			bContinueEditing = true;
		}
		else {
			bContinueEditing = false;
		}
		return false;
	}
	else {
		//TES 10/1/2009 - PLID 34252 - Does it have the same name as the drug being edited?
		//TES 11/18/2009 - PLID 36355 - Check against RNDC14.LN60, not RNDC14.LN
		// (j.jones 2010-01-26 16:37) - PLID 37078 - it needs to compare to RNDC14.LN60 *or* RMIID1_MED.MED_MEDID_DESC
		//save the first name we find, LN60, to use to warn the user
		CString strFirstRealName = AdoFldString(rsRealNDC, "Name", "");
		while(!rsRealNDC->eof) {
			CString strCompareName = AdoFldString(rsRealNDC, "Name", "");
			if(strCompareName.CompareNoCase(strPracticeDrugName) == 0) {
				//we found a match, so we can continue
				bContinueEditing = false;
				return true;
			}

			rsRealNDC->MoveNext();
		}
		
		//if we're still here, we found no match, warn using the first name we found, likely LN60
		if(IDYES == MsgBox(MB_YESNO, "Warning: The NDC Number you have entered is associated with the drug '%s' in the FirstDataBank database.\r\n"
			"Would you like to return to editing the field?", strFirstRealName)) {
			bContinueEditing = true;
		}
		else {
			bContinueEditing = false;
		}
		return false;
	}
	bContinueEditing = false;
	return true;
}

// (a.walling 2009-11-16 12:54) - PLID 36239 - Ensure the FirstDataBank database is available, with UI messages
bool FirstDataBank::EnsureDatabase(CWnd* pMessageParent, bool bNotifyNxServer)
{
	// We will only check once per session if we succeed.
	static bool g_bFirstDataBankValid = false;
	if (g_bFirstDataBankValid) {
		return g_bFirstDataBankValid;
	}

	try {
		if (!IsDatabaseValid()) {
			if (pMessageParent) {
				CString strMessage = "The FirstDataBank database is not currently available. ";
				
				if (bNotifyNxServer) {
					strMessage += "The NexTech Network Server will be notified to prepare the database automatically. The database should be ready in a few minutes.";
				}

				strMessage += "\r\n\r\nPlease contact Technical Support if this issue persists.";

				pMessageParent->MessageBox(strMessage, NULL, MB_ICONEXCLAMATION);
			}
			//TES 4/3/2009 - PLID 33376 - A setting changed, notify NxServer.
			bool bNotified = CClient::Send(PACKET_TYPE_ENSURE_FIRSTDATABANK, NULL, 0);

			if (!bNotified) {
				if (pMessageParent) {
					pMessageParent->MessageBox("Could not communicate with the NexTech Network Server!", NULL, MB_ICONERROR);
				}
			}

			return false;
		} else {
			g_bFirstDataBankValid = true;
			return true;
		}
	} NxCatchAll("Could not validate the FirstDataBank database");

	return false;
}

// (a.walling 2009-11-16 13:00) - PLID 36239 - Is the FirstDataBank database valid?
bool FirstDataBank::IsDatabaseValid()
{
	// (a.walling 2012-06-28 18:24) - PLID 49394 - return false if we have an error at least
	try {
		// (a.walling 2010-10-20 09:41) - PLID 34813 - Don't catch and ignore SQL _com_errors!
		_RecordsetPtr prsFDB = CreateRecordsetStd("SELECT dbid FROM master..sysdatabases WHERE name = 'FirstDataBank'");
		if (prsFDB->eof) {
			// does not exist
			return false;
		}

		// (z.manning 2011-06-17 12:07) - PLID 43815 - I reworked this a bit to prevent a possible exception.
		// (s.dhole 2013-03-25 15:43) - PLID 55855  change validation sql so we can verify new table
		prsFDB = CreateRecordsetStd(
			"	SELECT TABLE_NAME \r\n"
			"	FROM FirstDataBank.INFORMATION_SCHEMA.COLUMNS \r\n"
			"	WHERE TABLE_NAME = 'NexFDBVersion' \r\n"
			"		AND COLUMN_NAME = 'CurrentVersion' \r\n"
		);

		// (a.walling 2012-04-03 15:56) - PLID 49394 - This was not checking for eof, and hence would not send the packet to update due to the exception
		if (prsFDB->eof) {
			// does not exist
			return false;
		}
		// (s.dhole 2013-03-21 11:35) - PLID 55855 check version 
		prsFDB = CreateRecordsetStd(
			"SELECT  CurrentVersion  FROM FirstDataBank..NexFDBVersion"
		);

		if (!prsFDB->eof && AdoFldString(prsFDB, "CurrentVersion","").CompareNoCase(FirstDataBankUtils::GetFirstDataBankSupportedVersion())==0) 
		{

			return true;
		} else {
			return false;
		}
	} NxCatchAll("Error checking for FirstDataBank database");

	return false;
}

//TES 11/18/2009 - PLID 36360 - This function returns either strDEA_Practice, or a DEA Schedule pulled from the matching drug in the 
// FirstDataBank database, if that DEA is stricter.
CString FirstDataBank::CalcDEASchedule(const CString &strDEA_Practice, long nMedicationID)
{
	if(g_pLicense && g_pLicense->CheckForLicense(CLicense::lcFirstDataBank, CLicense::cflrSilent) && EnsureDatabase(NULL, true)) {
		//TES 11/18/2009 - PLID 36360 - Pull the DEA field from the matching drug (if any) in our drug database).
		_RecordsetPtr rsDEA = CreateParamRecordset("SELECT FirstDataBank..RNDC14.DEA "
			"FROM DrugList INNER JOIN FirstDataBank..RNDC14 ON DrugList.NDCNumber = FirstDataBank..RNDC14.NDC "
			"WHERE DrugList.ID = {INT}", nMedicationID);
		if(!rsDEA->eof) {
			CString strDEA_FDB = AdoFldString(rsDEA, "DEA", "");
			//TES 11/18/2009 - PLID 36360 - We want to use the FDB DEA Schedule if the Practice DEA Schedule is blank, or if it represents a 
			// stricter classification than Practice's DEA Schedule
			if(strDEA_Practice.IsEmpty() ||
				(strDEA_FDB == "1") ||
				(strDEA_FDB == "2" && strDEA_Practice != "I") ||
				(strDEA_FDB == "3" && strDEA_Practice != "I" && strDEA_Practice != "II") ||
				(strDEA_FDB == "4" && strDEA_Practice != "I" && strDEA_Practice != "II" && strDEA_Practice != "III") ||
				(strDEA_FDB == "5" && strDEA_Practice != "I" && strDEA_Practice != "II" && strDEA_Practice != "III" && strDEA_Practice != "IV")) {
					//TES 11/18/2009 - PLID 36360 - Return the schedule as a Roman numeral, as that's how we store it in Practice.
					if(strDEA_FDB == "1") return "I";
					else if(strDEA_FDB == "2") return "II";
					else if(strDEA_FDB == "3") return "III";
					else if(strDEA_FDB == "4") return "IV";
					else if(strDEA_FDB == "5") return "V";
					//TES 11/18/2009 - PLID 36360 - If the FDB DEA Schedule is not 1-5, don't use it.
			}
		}
	}
	return strDEA_Practice;
}

// (j.jones 2010-01-21 17:03) - PLID 37004 - added ability to calculate the NDC number,
// using the NewCropID as the FirstDataBank ID
// (j.jones 2010-01-26 17:40) - PLID 37078 - moved to FirstDataBankUtils
CString FirstDataBank::ChooseNDCNumberFromFirstDataBank(long nFirstDataBankID)
{
	try {

		if(nFirstDataBankID <= 0) {
			//we were given an invalid ID
			return "";
		}

		// (j.jones 2010-05-17 11:29) - PLID 38690 - fixed the sort on REPNDC, we meant for this to find non-replaced NDCs first
		// but we had sorted incorrectly so that non-replaced NDCs came last

		//match by MEDID only, as we've confirmed that the strength & name is identical for all
		//of the same MEDID (for reference, NewCrop gives us names matching RMIID1_MED.MED_MEDID_DESC, not matching RNDC14.LN60)
		//also, we ideally want non-replaced, non-obsolete NDCs, but we will use them if that's all that is available,
		//perhaps this is an old medication (these are sorted to be returned to us last)
		_RecordsetPtr rsNDCLookup = CreateParamRecordset("SELECT TOP 1 FirstDataBank..RMINDC1_NDC_MEDID.NDC "
			"FROM FirstDataBank..RMINDC1_NDC_MEDID "
			"INNER JOIN FirstDataBank..RMIID1_MED ON FirstDataBank..RMINDC1_NDC_MEDID.MEDID = FirstDataBank..RMIID1_MED.MEDID "
			"INNER JOIN FirstDataBank..RNDC14 ON FirstDataBank..RMINDC1_NDC_MEDID.NDC = FirstDataBank..RNDC14.NDC "
			"WHERE FirstDataBank..RMINDC1_NDC_MEDID.MEDID = {INT} "
			//we want to order such that non-replaced, non-obsolete NDCs come first,
			//then ordered by newest NDC to oldest
			"ORDER BY "
			//REPNDC identifies a replacement NDC, we prefer those that have no replacements
			"Convert(bit, CASE WHEN FirstDataBank..RNDC14.REPNDC = '' OR FirstDataBank..RNDC14.REPNDC Is Null THEN 0 ELSE 1 END) ASC, "
			//OBSDTEC identifies when this NDC became/becomes obsolete
			"Convert(bit, CASE WHEN FirstDataBank..RNDC14.OBSDTEC = '00000000' OR Convert(datetime, FirstDataBank..RNDC14.OBSDTEC) >= GetDate() THEN 0 ELSE 1 END) ASC, "
			//DADDNDC is the date the NDC was added
			"Convert(datetime, FirstDataBank..RNDC14.DADDNC) DESC", nFirstDataBankID);
		if(rsNDCLookup->eof) {
			//if there are no results, we're done
			return "";
		}
		else {
			//use the first NDC, doesn't matter how many matches there are
			return AdoFldString(rsNDCLookup, "NDC", "");
		}
		rsNDCLookup->Close();

	}NxCatchAll(__FUNCTION__);

	//return a blank value
	return "";
}
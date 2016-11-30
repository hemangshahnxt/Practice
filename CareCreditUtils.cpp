#include "stdafx.h"
#include "CareCreditUtils.h"
#include "RegUtils.h"
#include "CareCreditMarketingDlg.h"
#include "AuditTrail.h"
#include "FileUtils.h"
#include "CareCreditSelectDlg.h"
#include "globalstringutils.h"


// (a.walling 2010-01-21 16:43) - PLID 37021 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



#define REPLACE_QUOTES(str) str.Replace('\"', '\'')

void CareCreditParameters::ReplaceQuotes()
{
	REPLACE_QUOTES(m_strPartner);
	REPLACE_QUOTES(m_strLast);
	REPLACE_QUOTES(m_strMiddle);
	REPLACE_QUOTES(m_strFirst);
	REPLACE_QUOTES(m_strBirthDate);
	REPLACE_QUOTES(m_strSSN);
	REPLACE_QUOTES(m_strAddress);
	REPLACE_QUOTES(m_strCity);
	REPLACE_QUOTES(m_strState);
	REPLACE_QUOTES(m_strZip);
	REPLACE_QUOTES(m_strHomePhone);
	REPLACE_QUOTES(m_strNetIncome);
	REPLACE_QUOTES(m_strCreditLimit);
	REPLACE_QUOTES(m_strResidenceType);
}

#define ADD_QUOTES(str)	("\"" + str + "\"")

CString CareCreditParameters::GenerateCommandLineParameters()
{
	// (z.manning, 01/03/2007) - PLID 24056 - Make sure none of the fields have a quotation mark or the
	// parameter listing will be messed up.
	ReplaceQuotes();

	// (z.manning, 01/02/2007) - PLID 24056 - Basically just need a space delimited list of all the paramaters
	// in quotation marks in the order specificed by CareCredit's documentation.
	return 
		"\"NONE\" \"1.0\" " + // NONE meaning not using an ini file and 1.0 is the version number.
		ADD_QUOTES(m_strPartner) + " " + 
		ADD_QUOTES(m_strLast) + " " +
		ADD_QUOTES(m_strMiddle) + " " +
		ADD_QUOTES(m_strFirst) + " " +
		ADD_QUOTES(m_strBirthDate) + " " +
		ADD_QUOTES(m_strSSN) + " " +
		ADD_QUOTES(m_strAddress) + " " +
		ADD_QUOTES(m_strCity) + " " +
		ADD_QUOTES(m_strState) + " " +
		ADD_QUOTES(m_strZip) + " " +
		ADD_QUOTES(m_strHomePhone) + " " +
		ADD_QUOTES(m_strNetIncome) + " " +
		ADD_QUOTES(m_strCreditLimit) + " " +
		ADD_QUOTES(m_strResidenceType);
}

#define ENSURE_CARE_CREDIT_FIELD(strFieldName, strField, nLength, bStrip) \
	if(bStrip) { \
		StripNonAlphanumericChars(strField); \
	} \
	if(strField.GetLength() > nLength) { \
		/* (z.manning, 02/28/2007) - PLID 24350 - Keep track of any fields that get truncated so we can warn the user. */ \
		arystrTruncatedFields.Add(strFieldName); \
		strField = strField.Left(nLength); \
	} \
	if(strField.IsEmpty()) { \
		strField = " "; \
	}

void CareCreditParameters::EnsureValidData(CStringArray &arystrTruncatedFields)
{		
	// (z.manning, 01/02/2007) - PLID 24056 - Mostly, we need to ensure we don't exceed the max length
	// and then make all blank parameters a space (this function is based off of CareCredit's API documentation).
	ENSURE_CARE_CREDIT_FIELD("Partner", m_strPartner, 20, TRUE);
	ENSURE_CARE_CREDIT_FIELD("Last Name", m_strLast, 25, TRUE);
	ENSURE_CARE_CREDIT_FIELD("Middle Name", m_strMiddle, 1, TRUE);
	ENSURE_CARE_CREDIT_FIELD("First Name", m_strFirst, 20, TRUE);
	ENSURE_CARE_CREDIT_FIELD("Birth Date", m_strBirthDate, 10, FALSE);
	ENSURE_CARE_CREDIT_FIELD("Address", m_strAddress, 25, TRUE);
	ENSURE_CARE_CREDIT_FIELD("City", m_strCity, 16, TRUE);
	ENSURE_CARE_CREDIT_FIELD("Net Income", m_strNetIncome, 6, TRUE);
	ENSURE_CARE_CREDIT_FIELD("Credit Limit", m_strCreditLimit, 6, TRUE);
	ENSURE_CARE_CREDIT_FIELD("Residence Type", m_strResidenceType, 1, TRUE);
	
	// (z.manning, 01/02/2007) - PLID 24056 - SSN must be ###-##-####, so unless we have a 9 digit number, ignore it.
	StripNonNumericChars(m_strSSN);
	if(m_strSSN.GetLength() == 9 && IsNumeric(m_strSSN)) {
		m_strSSN.Insert(5, "-");
		m_strSSN.Insert(3, "-");
	}
	else {
		// (z.manning, 02/28/2007) - PLID 24350 - Keep track of any fields that get truncated so we can warn the user.
		if(!m_strSSN.IsEmpty()) {
			arystrTruncatedFields.Add("Social Security");
		}
		m_strSSN.Empty();
		ENSURE_CARE_CREDIT_FIELD("Social Security", m_strSSN, 11, FALSE);
	}

	// (z.manning, 01/02/2007) - PLID 24056 - State must be 2 chars, so if it's not, let's ignore.
	if(m_strState.GetLength() > 2) {
		// (z.manning, 02/28/2007) - PLID 24350 - Keep track of any fields that get truncated so we can warn the user.
		if(!m_strState.IsEmpty()) {
			arystrTruncatedFields.Add("State");
		}
		m_strState.Empty();
	}
	ENSURE_CARE_CREDIT_FIELD("State", m_strState, 2, TRUE);

	// (z.manning, 01/02/2007) - PLID 24056 - Zip code must be numeric with a max of 9 digits.
	// DO NOT m_strip all non-numeric characters in case it's one of those crazy Canadian zip codes that have letters.
	m_strZip.Remove('-');
	if(!IsNumeric(m_strZip) || m_strZip.GetLength() > 9) {
		// (z.manning, 02/28/2007) - PLID 24350 - Keep track of any fields that get truncated so we can warn the user.
		if(!m_strZip.IsEmpty()) {
			arystrTruncatedFields.Add("Zip Code");
		}
		m_strZip.Empty();
	}
	ENSURE_CARE_CREDIT_FIELD("Zip Code", m_strZip, 9, TRUE);

	// (z.manning, 01/02/2007) - PLID 24056 - HomePhone must be in the format: ###-###-####.
	StripNonNumericChars(m_strHomePhone);
	if(m_strHomePhone.GetLength() == 10 && IsNumeric(m_strHomePhone)) {
		m_strHomePhone.Insert(6, "-");
		m_strHomePhone.Insert(3, "-");
	}
	else {
		// (z.manning, 02/28/2007) - PLID 24350 - Keep track of any fields that get truncated so we can warn the user.
		if(!m_strHomePhone.IsEmpty()) {
			arystrTruncatedFields.Add("Home Phone");
		}
		m_strHomePhone.Empty();
		ENSURE_CARE_CREDIT_FIELD("Home Phone", m_strHomePhone, 12, FALSE);
	}
}


namespace NxCareCredit
{
	// (j.camacho 2013-09-03 16:06) - PLID 58024 check for carecredit accounts before opening carecredit dialog.
	bool FindCareCreditAccount(CString&  strStoredMerchantID, CString& strStoredPassword)
	{
		try
		{
			//Pull record data from 
			ADODB::_RecordsetPtr rsDetails = CreateParamRecordset("SELECT CareCreditId, AccountName, MerchantNumber, Password FROM CareCreditUserst");
			
			//If the record is empty then direct towards the setup dialog
			if(!rsDetails->eof) 
			{
				if(rsDetails->RecordCount ==1)
				{
					strStoredMerchantID= AdoFldString(rsDetails,"MerchantNumber");
					strStoredPassword = AdoFldString(rsDetails,"Password");
					return true;
				}
				else
				{
					//there is more than one record
					//create dialog and request which one to use
					// (j.camacho 2013-08-16 15:07) - PLID 58024 - Multiple accounts found, ask which one to use.
					CareCreditSelectDlg SelectDLG;
					if(SelectDLG.DoModal() == IDOK)
					{
						//positive selection of an account 
						//grab selected ID from the dialog
						strStoredMerchantID= SelectDLG.GetMerchantNumber();
						strStoredPassword = SelectDLG.GetMerchantPassword();
						if(strStoredMerchantID !="" && strStoredPassword != "")
						{
							return true; //password and merchant id were found
						}
						else
						{
							return false; //password and merchant id were NOT found
						}
					}
					else
					{
						//cancel from selection 
					return false;
					}
				}
			}
			else
			{
				//direct to the setup
				AfxMessageBox("Please setup your CareCredit accounts.\r\nThe 'CareCredit Settings...' option can be found under the Tools menu", NULL, MB_ICONINFORMATION|MB_OK);						
			}
		}NxCatchAll(__FUNCTION__);
		//failed
		return false;
}

	// (j.camacho 2013-07-11 12:42) - PLID 57470 - made changes to this function to allow for the carecreditAPI
	void OpenCCWare(long nPatientID)
	{
		// (z.manning, 01/09/2007) - We do not check for licensing here nor do we the places that call this funtion.
		// The reason being-- the "marketing" dialog checks for licensing and removies the "OK" button from
		// the dialog. And as you can see below, if the marketing dialog does not return ok, then we don't open CCWare.
		// (z.manning, 01/04/2007) - PLID 24056 - Need to show the marketing dialog and return if they cancel out of it.
		CCareCreditMarketingDlg dlgMarketing(NULL);
		if(dlgMarketing.DoModal() != IDOK) {
			return;
		}

		// (z.manning, 01/11/2007) - PLID 24062 - Make sure they have permissions for CareCredit.
		if(!CheckCurrentUserPermissions(bioCareCreditIntegration, sptView)) {
			return;
		}	

		// (z.manning, 01/02/2007) - PLID 24056 - Get the current patient's information and load it into a struct.
		ADODB::_RecordsetPtr prsPatient = CreateRecordset(
			"SELECT Last, Middle, First, BirthDate, SocialSecurity, Address1, Address2, City, State, Zip, HomePhone, Workphone "
			"FROM PersonT "
			"WHERE PersonT.ID = %li "
			, nPatientID );
		if(prsPatient->eof) {
			AfxThrowNxException( FormatString("Could not find patient with ID: %li", nPatientID) );
		}
		ADODB::FieldsPtr flds = prsPatient->Fields;

		COleDateTime dtBirthDate, dtInvalid;
		dtInvalid.SetStatus(COleDateTime::invalid);
		dtBirthDate = AdoFldDateTime(flds, "BirthDate", dtInvalid);
		
		CareCreditParameters ccp;
		ccp.m_strPartner = "NexTech";// this is our partner name with CareCredit
		ccp.m_strLast = AdoFldString(flds, "Last", "");
		ccp.m_strMiddle = AdoFldString(flds, "Middle", "");
		ccp.m_strFirst = AdoFldString(flds, "First", "");
		if(dtBirthDate.GetStatus() == COleDateTime::valid) {
			ccp.m_strBirthDate = FormatDateForCareCredit(dtBirthDate);
		}
		ccp.m_strSSN = AdoFldString(flds, "SocialSecurity", "");
		ccp.m_strAddress = AdoFldString(flds, "Address1", "");
		ccp.m_strAddress.Remove('#'); // (j.camacho 2013-09-05 17:44) - PLID 57470 cleaning address
		CString strAddress2 = AdoFldString(flds, "Address2", "");
		strAddress2.Remove('#'); // (j.camacho 2013-09-05 17:44) - PLID 57470 cleaning address
		// (j.camacho 2013-07-11 12:45) - PLID 57511 - moved this to after checking for API since we can store this in the ccp object now
		if(!strAddress2.IsEmpty()) {
			// (z.manning, 01/02/2007) - PLID 24056 - They only support one address line, so if this patient
			// has an address2, let's just put it on the main address line.
			ccp.m_strAddress += " " + strAddress2;
		}
		else if(!strAddress2.IsEmpty())
		{
			ccp.m_strAddress2 += strAddress2;
		}
		ccp.m_strCity = AdoFldString(flds, "City", "");
		ccp.m_strState = AdoFldString(flds, "State", "");
		ccp.m_strZip = AdoFldString(flds, "Zip", "");
		ccp.m_strHomePhone = AdoFldString(flds, "HomePhone", "");
		ccp.m_strWorkPhone = AdoFldString(flds, "WorkPhone", "");

		// (z.manning, 01/02/2007) - PLID 24056 - We don't store this info.
		ccp.m_strNetIncome = "";
		ccp.m_strCreditLimit = "";
		ccp.m_strResidenceType = "";

		// (z.manning, 01/02/2007) - PLID 24056 - Make sure the data is valid (e.g. fields not too long).
		CStringArray arystrTruncatedFields;
		ccp.EnsureValidData(arystrTruncatedFields);

		// (z.manning, 01/10/2007) - PLID 24057 - Since the CareCredit licensing is handled slightly unsually,
		// let's check the licensing here for the sole purpose of decrementing the count (if they're not licensed
		// they should not have made it this far).
		BOOL bLicensed = g_pLicense->CheckForLicense(CLicense::lcCareCredit, CLicense::cflrUse);
		ASSERT(bLicensed);

		// (z.manning, 02/28/2007) - PLID 24350 - If any fields were truncated, warn the user.
		if(arystrTruncatedFields.GetSize() > 0) {
			CString strMsg = "The following fields were too long for CareCredit and have been truncated:\r\n";
			for(int i = 0; i < arystrTruncatedFields.GetSize(); i++) {
				strMsg += "-- " + arystrTruncatedFields.GetAt(i) + "\r\n";
			}
			strMsg += "\r\nPlease review these fields before submitting an application through CareCredit.";
			AfxMessageBox(strMsg);
		}
	
		// (z.manning, 01/02/2007) - PLID 24056 - All right, let's make the command and execute the CCWare application.
		// (a.walling 2009-08-10 16:17) - PLID 35153 - Use NULL instead of "open" when calling ShellExecute(Ex). Usually equivalent; see PL notes.
		// (j.camacho 2013-07-10 16:25) - PLID 57470 - Changed the CareCredit link to be CC API based for upcoming move away from the application 
		// (j.camacho 2013-07-11 15:37) - PLID 57511 - Check if using API
		CString strStoredMerchantID, strStoredPassword;
		CareCreditLogin ccLogin;		
		// (j.camacho 2013-08-12 16:17) - PLID 58024 check for the client account. If one carecredit account just go automatically.
		if(FindCareCreditAccount(strStoredMerchantID, strStoredPassword))
		{
			ccLogin.m_strMerchantId = strStoredMerchantID;
			ccLogin.m_strPassword =  strStoredPassword;
			if((int)ShellExecute(NULL, "open", CCwebAPIURL(ccp,ccLogin) , NULL, NULL, SW_SHOWNORMAL) < 32) 
			{
				CString strError;
				FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, strError.GetBuffer(MAX_PATH), MAX_PATH, NULL);
				strError.ReleaseBuffer();
				AfxMessageBox("Failed to execute using the CareCredit API due to the following error:\r\n\r\n" + strError);
			}
			else {
				// (z.manning, 01/11/2007) - PLID 24219 - Ok, we opened it successfully, let's audit as much.
				AuditEvent(nPatientID, GetExistingPatientName(nPatientID), BeginNewAuditEvent(), aeiCareCreditOpened, nPatientID, "", "", aepMedium, aetChanged);
			}
		}	
		else {
			//Failure to successfully find a carecredit account (could also be a cancel from the dialog)
			TRACE("Cancel CareCredit Attempt.");
		}
	}

	// (j.camacho 2013-07-10 17:34) - PLID 57470 - build URL for CareCredit API
	CString CCwebAPIURL(CareCreditParameters ccp,CareCreditLogin cclogin)
	{
		CString m_strURL;
		//base URL
		m_strURL="https://www.carecredit.com/validate/index.html?";
	
		//make login object
		m_strURL+="partner=NexTech"; // test data		//partner Partner name
		m_strURL+="&storeid=S023"; //test data			//storeid Store ID
		m_strURL+="&mrchnbr="+cclogin.m_strMerchantId;  //mrchnbr merchant number
		m_strURL+="&pwd="+cclogin.m_strPassword ; 		//pwd password
		m_strURL+="&ln="+ccp.m_strLast;					//ln patient last name
		m_strURL+="&mi="+ccp.m_strMiddle;				//mi patient middle name
		m_strURL+="&fn="+ccp.m_strFirst;				//fn patient first name 
		m_strURL+="&db="+ccp.m_strBirthDate;			//db patient date of birth
		m_strURL+="&ssn="+ccp.m_strSSN;					//ssn patient SSN
		m_strURL+="&ad="+ccp.m_strAddress;				//ad patient address
		m_strURL+="&ad2="+ccp.m_strAddress2;			//ad2 patient address 2
		m_strURL+="&cty="+ccp.m_strCity;					//cty patient city
		m_strURL+="&st="+ccp.m_strState;				//st patient state
		m_strURL+="&zp="+ccp.m_strZip;					//Zp patient zipcode
		m_strURL+="&hp="+ccp.m_strHomePhone;			//hp patient home phone
		m_strURL+="&bp="+ccp.m_strWorkPhone;			//bp patient business hone
		m_strURL+="&res_type="+ccp.m_strResidenceType;	//res_type
		m_strURL+="&inc="+ccp.m_strNetIncome;			//inc Net Income
		m_strURL+="&CreditLimit="+ccp.m_strCreditLimit;//CreditLimit

		return m_strURL;
	}

	CString FormatDateForCareCredit(COleDateTime dtDate)
	{
		if(dtDate.GetStatus() == COleDateTime::invalid) {
			return " ";
		}

		// (z.manning, 01/02/2007) - PLID 24056 - Birthdate should be in format: MM/DD/YYYY
		return dtDate.Format("%m/%d/%Y");
	}

	CareCreditLicenseStatus GetCareCreditLicenseStatus()
	{
		COleDateTime dtCareCreditExpiration;
		dtCareCreditExpiration.SetStatus(COleDateTime::invalid);
		BOOL bLicensed = g_pLicense->CheckForLicense(CLicense::lcCareCredit, CLicense::cflrSilent, FALSE, NULL, NULL, NULL, &dtCareCreditExpiration);

		if(dtCareCreditExpiration.GetStatus() == COleDateTime::valid 
			&& COleDateTime::GetCurrentTime() > dtCareCreditExpiration)
		{
			// (z.manning, 01/10/2007) - PLID 24057 - It's expired, which likely means we set it this we because
			// we're no longer linking with CareCredit.
			return cclsExpired;
		}

		if(bLicensed) {
			return cclsLicensed;
		}

		return cclsUnlicensed;
	}
}; //End namespeace NxCareCredit
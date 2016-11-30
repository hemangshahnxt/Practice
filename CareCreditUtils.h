//GlobalAuditUtils.h
#if !defined CARE_CREDIT_UTILS_H
#define CARE_CREDIT_UTILS_H

#pragma once

class CareCreditParameters
{
public:
	CString m_strPartner;
	CString m_strLast;
	CString m_strMiddle;
	CString m_strFirst;
	CString m_strBirthDate;
	CString m_strSSN;
	CString m_strAddress;
	CString m_strAddress2;// (j.camacho 2013-07-11 12:45) - PLID 57470 - This allows for the API to use the second address part
	CString m_strCity;
	CString m_strState;
	CString m_strZip;
	CString m_strHomePhone;
	CString m_strWorkPhone;// (j.camacho 2013-09-04 16:37) - PLID 57470 - This allows for the API to use the business phone part
	CString m_strNetIncome;
	CString m_strCreditLimit;
	CString m_strResidenceType;

public:
	void ReplaceQuotes();
	CString GenerateCommandLineParameters();
	// (z.manning, 02/28/2007) - PLID 24350 - Added a parameter to track the fields that get cut off.
	void EnsureValidData(CStringArray &arystrTruncatedFields);
};


enum CareCreditLicenseStatus
{
	cclsExpired = 0,
	cclsUnlicensed,
	cclsLicensed,
};

// (j.camacho 2013-07-12 15:57) - PLID 57470 - Login information object
class CareCreditLogin
{
public:
	int m_intUserId;
	CString m_strPartnerName; //Static Nextech Credential
	CString m_strStoreId; //Static Nextech Credential
	CString m_strMerchantId; // Dynamic client merchant id
	CString m_strPassword; //Dynamic client password (actually their zipcode)
};


namespace NxCareCredit
{	
	void OpenCCWare(long nPatientID);
	CString FormatDateForCareCredit(COleDateTime dtDate);
	CareCreditLicenseStatus GetCareCreditLicenseStatus();	
	CString CCwebAPIURL(CareCreditParameters ccp, CareCreditLogin cclogin);// (j.camacho 2013-07-11 09:41) - PLID 57470
};



#endif // !defined CARE_CREDIT_UTILS_H
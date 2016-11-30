// OHIPERemitParser.h: interface for the COHIPERemitParser class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OHIPEREMITPARSER_H__4BE263DB_74B0_4EFE_98A0_7F2FDBF23A96__INCLUDED_)
#define AFX_OHIPEREMITPARSER_H__4BE263DB_74B0_4EFE_98A0_7F2FDBF23A96__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// (j.jones 2008-06-16 12:21) - PLID 30413 - created

// (j.jones 2009-09-25 10:32) - PLID 34453 - added accounting transactions
// (j.jones 2012-10-04 16:32) - PLID 52929 - turned into a class so it could have a constructor/destructor
class OHIPEOBAccountingTransaction {

public:
	COleCurrency cyAmount;			//the amount of this transaction
	CString strReasonCode;			//the reason code
	CString strReasonDesc;			//the reason description
	CString strMessage;				//the message given
	COleDateTime dtDate;			//the date for the transaction

	// (j.jones 2012-10-04 16:31) - PLID 52929 - moved all initialization to a constructor
	OHIPEOBAccountingTransaction::OHIPEOBAccountingTransaction() {
		cyAmount = COleCurrency(0,0);
		dtDate.SetStatus(COleDateTime::invalid);
	}
	OHIPEOBAccountingTransaction::~OHIPEOBAccountingTransaction() {
	}
};

//the EOB Adjustment Info stores adjustment dollars, and is an optional part of a line item
// (j.jones 2012-10-04 16:32) - PLID 52929 - turned into a class so it could have a constructor/destructor
// (j.jones 2016-04-22 14:12) - NX-100161 - OHIP does not have adjustments, this code is terribly misleading
//class OHIPEOBAdjustmentInfo

//the OHIP EOB Line Item Info data element stores remittance data for an individual charge
//it is many-to-one with EOBClaimInfo
// (j.jones 2012-10-04 16:32) - PLID 52929 - turned into a class so it could have a constructor/destructor
class OHIPEOBLineItemInfo {

public:
	COleDateTime dtServiceDate;		//the date of the charge
	BOOL bChargeDatePresent;		//did we get a date?
	CString strServiceCode;			//the service code of the charge
	long nNumberOfServices;			//the number of services

	COleCurrency cyLineItemChargeAmt;	//the total charge amount
	COleCurrency cyLineItemPaymentAmt;	//the total payment amount

	CString strExplanatoryCode;		//the explanation code for the payment
	// (j.jones 2008-12-15 17:11) - PLID 32329 - added the explanation description for the payment,
	// although it is not used elsewhere right now
	CString strExplanatoryDescription;	//the explanation description

	long nChargeID;					//the internal charge ID this corresponds to
	long nProviderID;				//the internal provider ID on the charge

	// (j.jones 2008-12-15 16:50) - PLID 32329 - supported detecting duplicate charges
	BOOL bDuplicateCharge;			//was this a duplicate charge?

	// (j.jones 2016-04-22 14:12) - NX-100161 - OHIP does not have adjustments, this code is terribly misleading
	//CArray<OHIPEOBAdjustmentInfo*, OHIPEOBAdjustmentInfo*> paryOHIPEOBAdjustmentInfo;	//stores an array of adjustments

	// (j.jones 2012-10-04 16:31) - PLID 52929 - moved all initialization to a constructor
	OHIPEOBLineItemInfo::OHIPEOBLineItemInfo() {
		cyLineItemChargeAmt = COleCurrency(0,0);
		cyLineItemPaymentAmt = COleCurrency(0,0);
		nNumberOfServices = 0;
		dtServiceDate.SetStatus(COleDateTime::invalid);
		bChargeDatePresent = FALSE;
		nChargeID = -1;
		nProviderID = -1;
		bDuplicateCharge = FALSE;
	}
};

//the OHIP EOB Claim Info data element stores remittance data for an individual claim
//it is many-to-one with EOBInfo
// (j.jones 2012-10-04 16:32) - PLID 52929 - turned into a class so it could have a constructor/destructor
class OHIPEOBClaimInfo {

public:
	CArray<OHIPEOBLineItemInfo*, OHIPEOBLineItemInfo*> paryOHIPEOBLineItemInfo;	//stores an array of line items included in this claim

	CString strClaimNumber;			//the claim number reported by MOH
	CString strProvNumber;			//the provider number
	CString strAccountingNumber;	//the accounting number from Health Claim Header 1
	CString strHealthRegistrationNumber;	//the health registration number	
	CString strVersionCode;			//the Version code as on Health Encounter Claim Header 1
	CString strPaymentProgram;		//the Payment Program as on Health Encounter Claim Header 1
	CString strLocationCode;		//the Location Code as on Health Encounter Claim Header 1

	long nPatientID;				//the internal patient ID that the claim is for
	long nUserDefinedID;			//the patient UserDefined ID that the claim is for
	long nBillID;					//the internal bill ID of the claim
	long nInsuredPartyID;			//the internal insured party ID of the claim
	// (j.jones 2010-02-08 11:58) - PLID 37182 - added RespTypeID
	long nRespTypeID;				//the internal RespTypeID for the insured party
	// (j.jones 2010-09-02 11:15) - PLID 38787 - added SubmitAsPrimary
	BOOL bSubmitAsPrimary;
	long nInsuranceCoID;			//the internal insurance company ID for the insured party
	CString strInsuranceCoName;		//the internal insurance company name for the insured party

	// (j.jones 2012-10-04 16:31) - PLID 52929 - moved all initialization to a constructor
	OHIPEOBClaimInfo::OHIPEOBClaimInfo() {
		nBillID = -1;
		nPatientID = -1;
		nInsuredPartyID = -1;
		// (j.jones 2010-02-08 11:58) - PLID 37182 - added RespTypeID
		nRespTypeID = -1;
		// (j.jones 2010-09-02 11:15) - PLID 38787 - added SubmitAsPrimary
		bSubmitAsPrimary = FALSE;
		nInsuranceCoID = -1;
	}

	// (j.jones 2012-10-04 16:31) - PLID 52929 - added a destructor
	OHIPEOBClaimInfo::~OHIPEOBClaimInfo() {
		if(paryOHIPEOBLineItemInfo.GetSize() > 0) {
			for(int q=paryOHIPEOBLineItemInfo.GetSize()-1;q>=0;q--) {
				OHIPEOBLineItemInfo *pLineItemInfo = (OHIPEOBLineItemInfo*)paryOHIPEOBLineItemInfo.GetAt(q);
				delete pLineItemInfo;
				paryOHIPEOBLineItemInfo.RemoveAt(q);
			}
		}
	}
};

//the EOB Info data element stores remittance data for an entire EOB,
//and all its claims and charges therein
// (j.jones 2012-10-04 16:32) - PLID 52929 - turned into a class so it could have a constructor/destructor
class OHIPEOBInfo {

public:

	long nIndex;				//the internal index of this EOB (0-based)

	CArray<OHIPEOBClaimInfo*, OHIPEOBClaimInfo*> paryEOBClaimInfo;	//stores an array of claims included in this EOB

	// (j.jones 2012-10-04 16:30) - PLID 52929 - Added a count of claims skipped because they were not found,
	// and the total that will be paid on the non-skipped claims.
	// These are only used when the OHIPIgnoreMissingPatients preference is enabled.
	long nCountSkippedClaims;
	COleCurrency cyTotalPaidAfterSkipping;

	CString strGroupNumber;		//the group number for the office
	CString strProvNumber;		//the provider number

	COleDateTime dtPaymentDate;	//the date of the payment
	CString strPayeeName;		//the person/entity being paid
	CString strCheckNumber;		//the check number for the payment (blank if direct deposit)
	
	COleCurrency cyTotalPaymentAmt;	//the total payment amount of this EOB

	CStringArray arystrMessages;	//one entry for each Message Facility line

	long nLikelyInsuranceCoID;	//the insurance company we believe this EOB is from	

	// (j.jones 2009-09-25 10:32) - PLID 34453 - added accounting transactions
	CArray<OHIPEOBAccountingTransaction*, OHIPEOBAccountingTransaction*> paryOHIPEOBAccountingTransaction;	//stores an array of transactions

	// (j.jones 2012-10-04 16:31) - PLID 52929 - moved all initialization to a constructor
	OHIPEOBInfo::OHIPEOBInfo() {
		nIndex = -1;
		nLikelyInsuranceCoID = -1;
		cyTotalPaymentAmt = COleCurrency(0,0);
		// (j.jones 2010-04-09 12:04) - PLID 31309 - default the date to be today's date
		dtPaymentDate = COleDateTime::GetCurrentTime();
		nCountSkippedClaims = 0;
		cyTotalPaidAfterSkipping = COleCurrency(0,0);
	}

	// (j.jones 2012-10-04 16:31) - PLID 52929 - added a destructor
	OHIPEOBInfo::~OHIPEOBInfo() {
		if(paryEOBClaimInfo.GetSize() > 0) {
			for(int i=paryEOBClaimInfo.GetSize()-1;i>=0;i--) {
				OHIPEOBClaimInfo *pClaimInfo = (OHIPEOBClaimInfo*)paryEOBClaimInfo.GetAt(i);
				delete pClaimInfo;
				paryEOBClaimInfo.RemoveAt(i);
			}
		}

		arystrMessages.RemoveAll();

		// (j.jones 2009-09-25 10:32) - PLID 34453 - added accounting transactions
		if(paryOHIPEOBAccountingTransaction.GetSize() > 0) {
			for(int i=paryOHIPEOBAccountingTransaction.GetSize()-1;i>=0;i--) {
				delete (OHIPEOBAccountingTransaction*)paryOHIPEOBAccountingTransaction.GetAt(i);					
				paryOHIPEOBAccountingTransaction.RemoveAt(i);
			}
		}
	}
};

class COHIPERemitParser  
{
public:
	COHIPERemitParser();
	virtual ~COHIPERemitParser();

	// (j.jones 2012-05-25 13:45) - PLID 44367 - we now track a CWnd parent
	CWnd *m_pParentWnd;

	CArray<OHIPEOBInfo*, OHIPEOBInfo*> m_paryOHIPEOBInfo;
	long m_CountOfEOBs;
	void ClearEOB();

	CProgressCtrl *m_ptrProgressBar;
	CNxEdit *m_ptrProgressStatus;

	// (j.jones 2010-09-22 07:34) - PLID 40621 - track whether the file is complete
	BOOL m_bParsingFailed;

	// (j.jones 2010-03-15 10:46) - PLID 32184 - added ability to enable/disable PeekAndPump
	BOOL m_bEnablePeekAndPump;
	
	// (j.jones 2008-06-17 17:31) - PLID 21921 - these functions help us tie the remittance data to actual bills/charges/inscos in our system
	void CalcInternalIDs(OHIPEOBInfo *ptrOHIPEOBInfo);	//tries to associate reported claims and charges with internal bill, patient, and charge IDs
	void CalcInsuranceIDs(OHIPEOBInfo *ptrEOBInfo);			//tries to determine the Insurance Co ID from the EOB, and insured party IDs

	// (j.jones 2008-12-15 12:20) - PLID 32322 - created CalcBillAndPatientID and moved its code from CalcInternalIDs,
	// so we can call it while parsing and output the patient name in the EOB.txt
	// Returns TRUE if we found a matching bill and patient, otherwise FALSE.
	// (b.spivey, October 01, 2012) - PLID 52666 - We need to send in the OHIP Health Reg #
	BOOL CalcBillAndPatientID(const IN CString strAccountingNumber, IN CString strHealthRegistrationNumber,
		OUT long &nBillID, OUT long &nPatientID, OUT CString &strPatientName);

	// (j.jones 2011-03-18 15:11) - PLID 42905 - given a patient ID, find all of that patient's
	// charges that we are about to post to, and add to the list
	void GetChargeIDsByPatientID(long nPatientID, CArray<long,long> &aryUsedCharges);

	// (j.jones 2012-05-25 13:53) - PLID 44367 - pass in a parent
	BOOL ParseFile(CWnd *pParentWnd);

	//stores the file name for later manipulation
	CString m_strFileName;
	CString m_strOutputFile;

	// (b.spivey, October 9th, 2014) PLID 62701 - get the stored file location. 
	CString GetStoredParsedFilePath();

private:

	// (j.jones 2012-10-04 15:28) - PLID 52929 - added cache for the OHIPIgnoreMissingPatients preference
	BOOL m_bIgnoreMissingPatients;

	CFile m_InputFile, m_OutputFile;
	// (j.jones 2012-10-04 15:28) - PLID 52929 - take in the current claim pointer (optional), we might
	// skip outputting any data for the current claim
	void OutputData(CString &OutputString, CString strNewData, OPTIONAL IN OHIPEOBClaimInfo *pCurrentClaimInfo);
	CString ParseElement(CString strLine, long nStart, long nLength, BOOL bDoNotTrim = FALSE);
	COleCurrency ParseOHIPCurrency(CString strAmount, CString strSign);

	CString m_strStoredParsedFile; 

	//returns the current EOB Info object we are building (in OHIP there should only be one)
	OHIPEOBInfo* GetCurrentEOBInfo();
	//returns the current Claim Info object we are building (last claim added to our EOB)
	OHIPEOBClaimInfo* GetCurrentClaimInfo();
	//returns the current Line Item Info object we are building (last line item added to the last claim)
	OHIPEOBLineItemInfo* GetCurrentLineItemInfo();

	//creates and intializes a new EOB info object, adds to our tracked list, and returns the pointer
	OHIPEOBInfo* GenerateNewEOBInfo();
	//creates and intializes a new EOB claim info object, adds to our tracked list, and returns the pointer
	OHIPEOBClaimInfo* GenerateNewEOBClaimInfo();
	//creates and intializes a new EOB line item info object, adds to our tracked list, and returns the pointer
	OHIPEOBLineItemInfo* GenerateNewEOBLineItemInfo();
	//creates and intializes a new EOB adjustment info object, adds to our tracked list, and returns the pointer
	// (j.jones 2009-04-03 11:22) - PLID 33851 - feature removed
	//OHIPEOBAdjustmentInfo* GenerateNewEOBAdjustmentInfo();
	// (j.jones 2009-09-25 10:32) - PLID 34453 - added accounting transactions
	OHIPEOBAccountingTransaction* GenerateNewEOBAccountingTransaction();

	// (j.jones 2008-12-15 16:10) - PLID 32329 - given an explanatory code, return its description
	CString GetExplanatoryDescriptionFromCode(CString strExplanatoryCode);

	void FileHeader_1(CString strLine);		//Health care provider information (once per file)
	void AddressRecord1_2(CString strLine);	//Name and address Line 1 of billing agent or health care provider (once per file)
	void AddressRecord2_3(CString strLine);	//Name and address Line 2 and 3 of billing agent or health care provider (once per file)
	void ClaimHeader_4(CString strLine);		//Common control information for each claim
	void ClaimItem_5(CString strLine);		//Detailed information for each item of service within a claim (code, date, amounts)
	void BalanceForward_6(CString strLine);	//This record is present only if the previous month's remittance was NEGATIVE.
	void AccountingTransaction_7(CString strLine);	//This record is present only if an accounting transaction is posted to the remittance advice (advance, reduction, advance repayment)
	void MessageFacility_8(CString strLine);	//A facility for the MOH to send messages to health care providers. May or may not be present. May have up to 99,999 occurrences.

	// (j.jones 2010-03-15 10:44) - PLID 32184 - added a local PeekAndPump function that
	// can optionally disable PeekAndPump usage for the import process	
	void PeekAndPump_OHIPERemitParser();

	// (j.jones 2011-03-21 14:53) - PLID 42917 - this function backs up the remit file
	// and EOB.txt to the server's NexTech\ConvertedEOBs path, and also ensures that files
	// > 365 days old are deleted
	void CopyConvertedEOBToServer();

	// (b.spivey, October 01, 2012) - PLID 52666 - holds the custom field ID at the time of parsing, no need to cache.
	long m_nHealthNumCustomField;
};

#endif // !defined(AFX_OHIPEREMITPARSER_H__4BE263DB_74B0_4EFE_98A0_7F2FDBF23A96__INCLUDED_)

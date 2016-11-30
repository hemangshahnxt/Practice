// ANSI835Parser.h: interface for the CANSI835Parser class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ANSI835PARSER_H__E37A6E3C_DEF8_4DB6_A695_7164CF0768D3__INCLUDED_)
#define AFX_ANSI835PARSER_H__E37A6E3C_DEF8_4DB6_A695_7164CF0768D3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//Note: these structs are defined in reverse hierarchy

//the EOB Adjustment Info stores adjustment dollars, and can be a part of a claim or a line item
struct EOBAdjustmentInfo {

	COleCurrency cyAdjustmentAmt;	//the amount of this adjustment, might not be what we actually post
	BOOL bPatResp;					//determines if the adjustment is simply an indicator of patient resp.
	CString strReason;				//the reason for the adjustment

	// (j.jones 2006-11-16 10:32) - PLID 23551 - added group code and reason code
	CString strGroupCode;			//the group code for the adjustment
	CString strReasonCode;			//the reason code for the adustment

	// (j.jones 2012-04-20 08:57) - PLID 49846 - track the takeback adjustment ID,
	// and also flag if it's ok to post without finding an adjustment
	// (for example, if it was ignored and never posted to begin with)
	long nReversedAdjustmentID;
	BOOL bMissingReversedAdjustmentOK;
};

//the EOB Line Item Info data element stores remittance data for an individual charge
//it is many-to-one with EOBClaimInfo
struct EOBLineItemInfo {

	CString strTraceNumber;	// (j.jones 2010-10-12 09:27) - PLID 40892 - 5010-only, this is the REF*6R Line Item Control Number,
	// which should be our internal charge ID

	CString strServiceID;	//presumably the CPT Code
	CString strModifier1;	//the first CPT Modifier
	CString strModifier2;	//the second CPT Modifier
	CString strModifier3;	//the third CPT Modifier
	CString strModifier4;	//the fourth CPT Modifier	
	long nChargeID;			//the internal charge ID for this line item
	long nProviderID;		// (j.jones 2008-06-18 14:21) - PLID 21921 - the internal provider ID on the charge
	COleCurrency cyLineItemChargeAmt;	//the total charge amount
	COleCurrency cyLineItemPaymentAmt;	//the total payment amount
	COleDateTime dtChargeDate;			//the date of the charge
	BOOL bChargeDatePresent;			//do we have a valid date?
	// (j.jones 2011-01-07 16:43) - PLID 41980 - track the allowable
	COleCurrency cyChargeAllowedAmt;

	// (j.armen 2012-02-20 09:15) - PLID 34344 - Create a typed CArray instead of a generic ptr array
	CArray<EOBAdjustmentInfo*, EOBAdjustmentInfo*> arypEOBAdjustmentInfo;	//stores an array of adjustments

	// (j.jones 2011-04-04 15:48) - PLID 42571 - added nInsuredPartyID and bIsSecondaryResp
	long nInsuredPartyID;			//the internal insured party ID we will post under
	BOOL bIsSecondaryResp;			//true only if the RespTypeT.CategoryPlacement = 2

	// (j.jones 2012-04-20 08:57) - PLID 49846 - track the takeback payment ID
	long nReversedPaymentID;

	// (j.jones 2012-05-01 14:13) - PLID 47477 - finally made a constructor for this
	EOBLineItemInfo::EOBLineItemInfo() {
		// (j.jones 2010-10-12 09:27) - PLID 40892 - make sure that strTraceNumber starts out empty
		strTraceNumber = "";
		nChargeID = -1;
		nProviderID = -1;
		// (j.jones 2011-04-04 15:48) - PLID 42571 - added nInsuredPartyID and bIsSecondaryResp
		nInsuredPartyID = -1;
		bIsSecondaryResp = FALSE;
		bChargeDatePresent = FALSE;
		cyLineItemPaymentAmt = COleCurrency(0, 0);
		// (j.jones 2011-01-07 16:47) - PLID 41980 - initialize the allowed amount to invalid
		cyChargeAllowedAmt.SetStatus(COleCurrency::invalid);
		// (j.jones 2012-04-20 08:57) - PLID 49846 - initialize the takeback payment ID
		nReversedPaymentID = -1;
	}
};

//the EOB Claim Info data element stores remittance data for an individual claim
//it is many-to-one with EOBInfo
class EOBClaimInfo {

public:
	// (j.armen 2012-02-20 09:15) - PLID 34344 - Create a typed CArray instead of a generic ptr array
	CArray<EOBLineItemInfo*, EOBLineItemInfo*> arypEOBLineItemInfo;	//stores an array of line items included in this claim

	CString strPatientID;			//the patient ID on the claim
	long nUserDefinedID; // (s.dhole 2013-06-24 16:20) - PLID 42275 added userdefinedid
	CString strPatientFirst;		//the patient's first name
	CString strPatientMiddle;		//the patient's middle name
	CString strPatientLast;			//the patient's last name
	// (j.jones 2011-03-16 16:25) - PLID 42866 - an EOB might try to correct an insurance ID,
	// and if they do, we want to track the old and the new separately, so I split this into 
	// to separate fields
	CString strOriginalPatientInsuranceID;	//the listed insurance ID for the patient
	CString strCorrectedPatientInsuranceID;	//the "corrected" insurance ID for the patient
	CString strGroupOrPolicyNum;	//the given group or policy number
	long nPatientID;				//the internal patient ID that the claim is for
	long nBillID;					//the internal bill ID of the claim	
	COleCurrency cyClaimChargeAmt;	//the total charges of the claim
	COleCurrency cyClaimPaymentAmt;	//the total payment for the claim
	COleCurrency cyClaimPatientResp;	//the patient resp. of the claim
	COleCurrency cyInsuranceResp;	//the insurance resp. of the claim (for this insurance co.)
	COleDateTime dtBillDate;		//the date of the bill
	BOOL bBillDatePresent;			//do we have a valid date?
	BOOL bDuplicateClaim;			//was this a duplicate claim?
	// (j.jones 2011-01-07 16:47) - PLID 41980 - track the allowed amount
	COleCurrency cyClaimAllowedAmt;
	// (j.jones 2013-07-19 08:57) - PLID 57616 - track the bill's insured party ID and other insured party ID,
	// as they were prior to us posting and swapping responsibilities
	long nBillInsuredPartyID;
	long nBillOthrInsuredPartyID;

	// (j.jones 2011-02-09 11:11) - PLID 42391 - track if this is a reversal
	// (j.jones 2012-04-23 12:07) - PLID 49846 - now we track if this is a reversed claim
	// or if this is a re-posted claim, and we track the pointer to its sibling
	BOOL bIsReversedClaim;	//true if this claim is a takeback
	BOOL bIsRepostedClaim;	//true if this claim is the new posting for the reversed claim
	EOBClaimInfo *pRepostedSibling; //if bIsReversedClaim is true, this points to the reposted claim
	EOBClaimInfo *pReversedSibling; //if bIsRepostedClaim is true, this points to the reversed claim

	// (j.armen 2012-02-20 09:15) - PLID 34344 - Create a typed CArray instead of a generic ptr array
	CArray<EOBAdjustmentInfo*, EOBAdjustmentInfo*> arypEOBAdjustmentInfo;	//stores an array of adjustments

	// (j.jones 2012-05-01 13:45) - PLID 47477 - track the "Processed As" status,
	// 1 for Primary, 2 for Secondary, 3 for Tertiary, -1 for any other response we don't track
	long nProcessedAs;
	// (j.jones 2013-07-10 10:29) - PLID 57263 - track an InsuredPartyT exclusion query
	// to optionally exclude primary insured parties when ProcessedAs is > 1
	// (j.jones 2016-04-13 16:13) - NX-100184 - removed this from the claim object
	//CSqlFragment sqlExcludeRespTypeFilter;

	// (j.dinatale 2012-11-05 11:57) - PLID 50792 - need to keep track if we are unbatching the current claim
	// (j.dinatale 2012-12-19 11:16) - PLID 54256 - renamed to reflect N89
	BOOL bHasMA18orN89;

	// (b.spivey, October 30, 2012) - PLID 49943 - Track CPL07 from the EOB
	CString strOriginalRefNum;

	// (j.dinatale 2012-12-19 14:41) - PLID 54256 - need the check date so that way we can use it later
	COleDateTime dtCheckDate; //the date the check (or EFT) was sent

	// (j.jones 2012-05-01 13:47) - PLID 47477 - finally made a constructor for this
	EOBClaimInfo::EOBClaimInfo() {
		nBillID = -1;
		nPatientID = -1;
		// (j.jones 2014-08-26 16:13) - PLID 63397 - initialize the user defined ID
		nUserDefinedID = -1;
		bBillDatePresent = FALSE;
		bDuplicateClaim = FALSE;
		// (j.jones 2011-01-07 16:47) - PLID 41980 - initialize the allowed amount to invalid
		cyClaimAllowedAmt.SetStatus(COleCurrency::invalid);
		// (j.jones 2011-02-09 11:11) - PLID 42391 - inititalize the reversal flag to FALSE
		// (j.jones 2012-04-23 12:07) - PLID 49846 - we now track more variables for reversals, initialize them all
		bIsReversedClaim = FALSE;
		bIsRepostedClaim = FALSE;
		pRepostedSibling = NULL;
		pReversedSibling = NULL;
		// (j.jones 2012-05-01 13:45) - PLID 47477 - initialize nProcessedAs to -1
		nProcessedAs = -1;
		// (j.dinatale 2012-11-05 11:57) - PLID 50792 - need to keep track if we are unbatching the current claim
		bHasMA18orN89 = FALSE;
		// (b.spivey, October 30, 2012) - PLID 49943 - Init the original reference number.
		strOriginalRefNum = "";
		// (j.dinatale 2012-12-19 14:41) - PLID 54256 - need the check date so that way we can use it later
		dtCheckDate = g_cdtInvalid;
		nBillInsuredPartyID = -1;
		nBillOthrInsuredPartyID = -1;
	}
};

//the EOB Info data element stores remittance data for an entire EOB,
//and all its claims and charges therein
struct EOBInfo {

	long nIndex;				//the internal index of this EOB (0-based)

	// (j.armen 2012-02-20 09:15) - PLID 34344 - Create a typed CArray instead of a generic ptr array
	CArray<EOBClaimInfo*, EOBClaimInfo*> arypEOBClaimInfo;	//stores an array of claims included in this EOB

	CString strBatchNumber;	//the batch number of the claims this EOB is associated with	
	COleCurrency cyTotalChargeAmt;	//the total charge amount of this EOB
	COleCurrency cyTotalPaymentAmt;	//the total payment amount of this EOB
	long nLikelyInsuranceCoID;	//the insurance company we believe this EOB is from
	long nHCFAGroupID;			//the HCFA Group that contains the company this EOB is from
	CString strInsuranceCoName;	//the name of the Ins Co. on the EOB
	CString strProviderName;	//the name of the provider on the EOB
	CString strPayeeName;		//the name of the payee on the EOB
	CString strPayerID;			//the Payer ID for th Ins Co.

	CString strCheckNumber; //the check (or EFT number) or the payment
	COleDateTime dtCheckDate; //the date the check (or EFT) was sent
	CString strPayBankRoutingNo; //the check's routing #
	CString strPayBankName;	//the name of the bank issuing the check
	CString strPayAccount;	//the account number of the payer

	//JMJ - they refer to the Payee in a generic sense, I am not sure if EOBs get split
	//among multiple providers or not. It's not a huge deal if we don't support it
	//but we should try to support it if we get confirmation that it is used.
	//long nProviderID;
};

// (j.jones 2015-07-06 10:42) - PLID 66359 - remark & reason codes
// are now initialized only once per session, since they only change
// when the software is upgraded
static bool g_bAdjustmentCodesInitialized = false;
static boost::container::flat_map<CString, CString> g_mapReasonCodes;
static boost::container::flat_map<CString, CString> g_mapRemarkCodes;
void InitializeAdjustmentCodes();

class CANSI835Parser
{
public:

	// (j.jones 2011-03-14 15:48) - PLID 42806 - added m_bIs5010File
	BOOL m_bIs5010File;

	// (j.armen 2012-02-20 09:15) - PLID 34344 - Create a typed CArray instead of a generic ptr array
	CArray<EOBInfo*, EOBInfo*> m_arypEOBInfo;
	long m_CountOfEOBs;
	CProgressCtrl *m_ptrProgressBar;
	CNxEdit *m_ptrProgressStatus;

	// (j.jones 2010-03-15 10:46) - PLID 32184 - added ability to enable/disable PeekAndPump
	BOOL m_bEnablePeekAndPump;

	// (j.jones 2010-02-09 09:27) - PLID 37174 - renamed to ClearAllEOBs
	void ClearAllEOBs();
	// (j.jones 2010-02-09 09:27) - PLID 37174 - added ClearEOB
	void ClearEOB(EOBInfo *ptrEOBInfoToClear);
	// (j.jones 2010-02-09 12:03) - PLID 37254 - added ClearClaimInfo
	void ClearClaimInfo(EOBInfo *ptrEOBInfo, EOBClaimInfo *ptrClaimInfoToClear);

	// (j.jones 2006-12-19 15:16) - PLID 23913 - stored the file name for later manipulation
	CString m_strFileName;
	// (j.jones 2011-03-21 14:58) - PLID 42917 - the output file is now a member variable
	CString m_strOutputFile;

	// (b.spivey, October 9th, 2014) PLID 62701 
	CString m_strStoredParsedFile = ""; 

	char m_chSegmentTerminator;
	char m_chElementTerminator;
	char m_chCompositeSeparator;

	//adds an adjustment to either a charge or claim
	void AddAdjustment(COleCurrency cyAdj, BOOL bPatResp, CString strGroupCode, CString strReasonCode, CString strReason);

	//these functions help us tie the remittance data to actual bills/charges/inscos in our system
	void CalcInternalIDs(EOBInfo *ptrEOBInfo);	//tries to associate reported claims and charges with internal bill and charge IDs
	void CalcBillID(EOBClaimInfo *pClaimInfo, long nHCFAGroupID);	//tries to determine the bill ID from a given claim info
	void CalcInsCoID(EOBInfo *ptrEOBInfo);	//tries to determine the Insurance Co ID from the EOB

	// (j.jones 2008-11-24 15:04) - PLID 32075 - CheckAdjustmentCodesToIgnore will check the setup in ERemitIgnoredAdjCodesT,
	// and make sure that all adjustments with matching group & reason codes are updated to be $0.00
	// (j.jones 2016-04-26 14:08) - NX-100327 - this no longer reduces all secondary adjustments to $0.00, that is done upon posting
	void CheckAdjustmentCodesToIgnore();

	// (j.jones 2008-11-25 16:32) - PLID 32133 - CheckWarnPaymentOverage will report to the user how much of the payment
	// may be converted to adjustments. This has to be called after the EOB is loaded onto the screen because claim level
	// adjustments and other changes may alter the payment amounts that were parsed from the EOB.
	// (j.jones 2012-02-13 10:34) - PLID 48084 - this now just warns of paying past zero, they would be payments now, not adjustments
	void CheckWarnPaymentOverage();

	//this function is used when you have multiple insurance results and need to narrow down by payer ID or InsCoName
	// (j.armen 2012-02-20 09:18) - PLID 48237 - for Parameratization, pass in the CSqlFragment version of the last recordset sql
	BOOL NarrowInsuranceSelection(EOBInfo *ptrEOBInfo, CSqlFragment sqlLastRecordsetSql, CString strPendingLog);

	// (j.jones 2011-03-18 15:11) - PLID 42905 - given a patient ID, find all of that patient's
	// charges that we are about to post to, and add to the list
	void GetChargeIDsByPatientID(long nPatientID, CArray<long, long> &aryUsedCharges);

	// (j.jones 2012-05-25 13:53) - PLID 44367 - pass in a parent
	BOOL ParseFile(CWnd *pParentWnd);

	CString ParseSection(CString &strIn, char chDelimiter);
	CString ParseSegment(CString &strIn);
	CString ParseElement(CString &strIn);
	CString ParseComposite(CString &strIn);

	void OutputData(CString &OutputString, CString strNewData);

	//stores the last data output to the file, mainly to keep track of newlines
	CString m_strLastOutput;

	//stores what loop we are believed to be on
	CString m_strLoopNumber;

	COleCurrency m_cyPendingClaimTotalCharge;
	COleCurrency m_cyPendingClaimTotalPayment;

	//these functions make up the 835 file
	BOOL ANSI_ST(CString &strIn);
	void ANSI_SE(CString &strIn);

	void ANSI_BPR(CString &strIn);
	void ANSI_TRN(CString &strIn);
	void ANSI_CUR(CString &strIn);
	void ANSI_REF(CString &strIn);
	void ANSI_DTM(CString &strIn);
	void ANSI_N1(CString &strIn);
	void ANSI_N3(CString &strIn);
	void ANSI_N4(CString &strIn);
	void ANSI_PER(CString &strIn);
	void ANSI_LX(CString &strIn);
	void ANSI_TS3(CString &strIn);
	void ANSI_TS2(CString &strIn);
	void ANSI_CLP(CString &strIn);
	void ANSI_CAS(CString &strIn);
	void ANSI_NM1(CString &strIn);
	void ANSI_MIA(CString &strIn);
	void ANSI_MOA(CString &strIn);
	void ANSI_AMT(CString &strIn);
	void ANSI_QTY(CString &strIn);
	void ANSI_SVC(CString &strIn);
	void ANSI_LQ(CString &strIn);
	void ANSI_PLB(CString &strIn);
	void ANSI_RDM(CString& strIn); // (c.haag 2010-10-26 16:33) - PLID 40349 - ANSI 5010

	//these are the interchange headers
	void ANSI_ISA(CString &strIn);
	void ANSI_IEA(CString &strIn);
	void ANSI_TA1(CString &strIn);
	void ANSI_GS(CString &strIn);
	void ANSI_GE(CString &strIn);

	CString GetRemarkCode(CString strCode);
	CString GetReasonCode(CString strCode);

	// (j.jones 2015-07-06 10:39) - PLID 66359 - renamed to GetAdjustmentPLBCode to clarify
	// that this is not the same list as the conventional adjustment reason codes that
	// are stored in data
	CString GetAdjustmentPLBCode(CString strCode);

	CFile m_InputFile, m_OutputFile;

	// (j.jones 2009-10-01 14:40) - PLID 35711 - we need to cache possible prepended patient ID codes
	CStringArray m_arystrPrependedPatientIDCodes;

	// (j.jones 2010-02-09 09:20) - PLID 37174 - cache the contents of ERemitEOBFilteredIDsT
	CStringArray m_arystrEOBFilteredIDs;
	// (j.jones 2010-02-09 10:44) - PLID 37254 - cache the contents of ERemitClaimFilteredIDsT
	CStringArray m_arystrClaimFilteredIDs;

	// (j.jones 2010-02-09 09:27) - PLID 37174 - track whether we should output/track info for the current EOB
	BOOL m_bSkipCurrentEOB;

	// (j.jones 2010-02-09 10:44) - PLID 37254 - track whether we should output/track info for the current claim
	BOOL m_bSkipCurrentClaim;

	// (j.jones 2012-05-25 13:45) - PLID 44367 - we now track a CWnd parent
	CWnd *m_pParentWnd;

	CANSI835Parser();
	virtual ~CANSI835Parser();

	EOBInfo* GetCurrentEOBInfo();

	// (j.dinatale 2012-11-05 17:32) - PLID 50792 - need the last claim info
	EOBClaimInfo* GetLastClaimInfo();

	// (j.jones 2010-03-15 10:44) - PLID 32184 - added a local PeekAndPump function that
	// can optionally disable PeekAndPump usage for the import process	
	void PeekAndPump_ANSI835Parser();

	// (j.jones 2011-03-21 14:53) - PLID 42917 - this function backs up the remit file
	// and EOB.txt to the server's NexTech\ConvertedEOBs path, and also ensures that files
	// > 30 days old are deleted
	void CopyConvertedEOBToServer();

	CString GetStoredParsedFilePath();

	// (j.jones 2011-09-28 14:00) - PLID 45486 - returns true if the cyClaimPaymentAmt
	// does not match the total of payments for each charge
	// (j.jones 2011-10-03 10:34) - PLID 45785 - moved into the parser class, from the EOBDlg
	// (j.armen 2012-02-20 09:19) - PLID 34344 - the EOBClaimInfo does not change here, so declare it as const
	BOOL HasInvalidPayTotals(const EOBClaimInfo *pClaim);

	// (j.jones 2011-09-28 14:00) - PLID 45486 - actually calculates the amt. paid for each charge
	// in the claim, which may or may not match the cyClaimPaymentAmt
	// (j.jones 2011-10-03 10:34) - PLID 45785 - moved into the parser class, from the EOBDlg
	// (j.armen 2012-02-20 09:19) - PLID 34344 - the EOBClaimInfo does not change here, so declare it as const
	COleCurrency GetPaymentTotalsForCharges(const EOBClaimInfo *pClaim);

	// (j.jones 2012-04-23 12:20) - PLID 49846 - Most remit files with reversals will list the reversal,
	// then the re-posted claim immediately following. If so, we link them together during the parsing.
	// But if they aren't listed this way, we need to link them together at the end of posting.
	// This function will handle linking reversed & re-posted claims that are not already linked.
	void VerifyReversedClaimLinks();
};

#endif // !defined(AFX_ANSI835PARSER_H__E37A6E3C_DEF8_4DB6_A695_7164CF0768D3__INCLUDED_)

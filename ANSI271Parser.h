// ANSI271Parser.h: interface for the CANSI271Parser class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ANSI271PARSER_H__E37A6E3C_DEF8_4DB6_A695_7164CF0768D3__INCLUDED_)
#define AFX_ANSI271PARSER_H__E37A6E3C_DEF8_4DB6_A695_7164CF0768D3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "EEligibility.h"
#include "GlobalFinancialUtils.h"

// (j.jones 2007-05-01 16:48) - PLID 25868 - created the ANSI 271 Eligibility Response importer

// (j.jones 2010-03-24 16:47) - PLID 37870 - added a class for Benefit Details, these correspond
// to EB loops, and there will be many per response
class BenefitDetail {

public:
	// (j.jones 2010-03-26 13:34) - PLID 37905 - changed BenefitType, CoverageLevel, and ServiceType
	// to track IDs from EligibilityDataReferenceT, instead of full text
	long nBenefitTypeRefID;
	long nCoverageLevelRefID;
	long nServiceTypeRefID;
	// (j.jones 2016-05-16 09:07) - NX-100357 - added the benefit type qualifier
	CString strBenefitTypeQualifier;
	CString strInsuranceType;
	CString strCoverageDesc;
	CString strTimePeriod;
	_variant_t varAmount;		//currency, possibly none
	_variant_t varPercentage;	//double, possibly none
	CString strQuantityType;
	_variant_t varQuantity;		//double, possibly none
	_variant_t varAuthorized;	//boolean, possibly unknown
	_variant_t varInNetwork;	//boolean, possibly unknown
	CString strServiceCode;
	CString strServiceCodeRangeEnd; // (c.haag 2010-10-20 13:14) - PLID 41017 - Added for ANSI 5010 (EB13 - 8)
	CString strModifiers;
	CString strExtraMessage;

	// (j.jones 2016-05-16 09:14) - NX-100357 - a benefit response may report
	// an insured party, if so track the discrete values
	CString strInsuranceCompanyName;
	CString strInsuranceCompanyPayerID;
	CString strInsuranceCompanyAddress1;
	CString strInsuranceCompanyAddress2;
	CString strInsuranceCompanyCity;
	CString strInsuranceCompanyState;
	CString strInsuranceCompanyZip;
	CString strInsuranceCompanyContactName;
	CString strInsuranceCompanyPhone;
	CString strInsuredPartyPolicyNumber;
	CString strInsuredPartyGroupNumber;

	// (j.jones 2010-04-19 14:36) - PLID 38202 - bSuppressFromOutput is not saved
	// to data, it's only used such that if a given benefit detail was suppressed
	// from the notepad file, extra messages appended from non EB loops will also
	// be suppressed
	BOOL bSuppressFromOutput;

	BenefitDetail() {
		nBenefitTypeRefID = -1;
		nCoverageLevelRefID = -1;
		nServiceTypeRefID = -1;
		varAmount = g_cvarNull;
		varPercentage = g_cvarNull;
		varQuantity = g_cvarNull;
		varAuthorized = g_cvarNull;
		varInNetwork = g_cvarNull;
		bSuppressFromOutput = FALSE;
	}

	// (c.haag 2010-11-03 10:25) - PLID 40350 - Copy constructor 
	BenefitDetail(const BenefitDetail* p)
	{
		nBenefitTypeRefID = p->nBenefitTypeRefID;
		nCoverageLevelRefID = p->nCoverageLevelRefID;
		nServiceTypeRefID = p->nServiceTypeRefID;
		// (j.jones 2016-05-16 09:07) - NX-100357 - added the benefit type qualifier
		strBenefitTypeQualifier = p->strBenefitTypeQualifier;
		strInsuranceType = p->strInsuranceType;
		strCoverageDesc = p->strCoverageDesc;
		strTimePeriod = p->strTimePeriod;
		varAmount = p->varAmount;
		varPercentage = p->varPercentage;
		strQuantityType = p->strQuantityType;
		varQuantity = p->varQuantity;
		varAuthorized = p->varAuthorized;
		varInNetwork = p->varInNetwork;
		strServiceCode = p->strServiceCode;
		strServiceCodeRangeEnd = p->strServiceCodeRangeEnd;
		strModifiers = p->strModifiers;
		strExtraMessage = p->strExtraMessage;

		// (j.jones 2016-05-16 09:14) - NX-100357 - a benefit response may report
		// an insured party, if so track the discrete values
		strInsuranceCompanyName = p->strInsuranceCompanyName;
		strInsuranceCompanyPayerID = p->strInsuranceCompanyPayerID;
		strInsuranceCompanyAddress1 = p->strInsuranceCompanyAddress1;
		strInsuranceCompanyAddress2 = p->strInsuranceCompanyAddress2;
		strInsuranceCompanyCity = p->strInsuranceCompanyCity;
		strInsuranceCompanyState = p->strInsuranceCompanyState;
		strInsuranceCompanyZip = p->strInsuranceCompanyZip;
		strInsuranceCompanyContactName = p->strInsuranceCompanyContactName;
		strInsuranceCompanyPhone = p->strInsuranceCompanyPhone;
		strInsuredPartyPolicyNumber = p->strInsuredPartyPolicyNumber;
		strInsuredPartyGroupNumber = p->strInsuredPartyGroupNumber;
	}
};

class EligibilityResponseDetailInfo {

	public:
	long nEligibilityRequestID;
	long nPatientID;
	long nInsuredPartyID;
	// (j.jones 2009-12-17 14:54) - PLID 36641 - added field for whether the detail
	// was confirmed, denied, or invalid
	EligibilityResponseConfirmationStatusEnum ercsStatus;

	CString strResponseDetailInfo;	//a string description of the response information for just one detail

	// (j.jones 2010-03-24 17:14) - PLID 37870 - added an array of the benefit information returned,
	// one entry per EB loop received
	CArray<BenefitDetail*, BenefitDetail*> arypBenefitDetails;

	EligibilityResponseDetailInfo() {
		nEligibilityRequestID = -1;
		nPatientID = -1;
		nInsuredPartyID = -1;
		// (j.jones 2009-12-17 14:54) - PLID 36641 - assume confirmed unless a rejection
		// is given, the AAA segment only shows up when it fails
		ercsStatus = ercsConfirmed;
	}
};

struct EligibilityResponseMasterInfo {

	CString strResponseMasterInfo;	//a string description of the top level response information, that applies to all details

	// (j.jones 2009-12-17 16:07) - PLID 36641 - added field for whether the master response
	// was confirmed, denied, or invalid
	EligibilityResponseConfirmationStatusEnum ercsStatus;

	CArray<EligibilityResponseDetailInfo*, EligibilityResponseDetailInfo*> arypResponses;	//stores all the EligibilityResponseDetail objects in this main response

	EligibilityResponseMasterInfo() {
		// (j.jones 2009-12-17 16:07) - PLID 36641 - assume confirmed unless a rejection
		// is given, the AAA segment only shows up when it fails
		ercsStatus = ercsConfirmed;
	}
};

enum HLLevel {

	hllInvalid = -1,
	hllSource = 1,
	hllReceiver = 2,
	hllSubscriber = 3,
	hllPatient = 4,
};

class CANSI271Parser  
{
public:

	CANSI271Parser();
	virtual ~CANSI271Parser();

	CArray<EligibilityResponseMasterInfo*, EligibilityResponseMasterInfo*> m_paryResponses;
	
	// (j.jones 2010-07-07 10:24) - PLID 39534 - added aryRequestIDsUpdated, which needs to
	// return an array of unique request IDs that we just imported to
	//this also fills an array of the new responses we just imported
	BOOL ParseFile(OUT std::vector<long> &aryRequestIDsUpdated, OUT std::vector<long> &aryResponseIDsReturned);

	// (j.jones 2010-07-02 13:29) - PLID 39499 - launches the parser from a text response
	// rather than browsing for a file
	// (j.jones 2010-07-07 10:14) - PLID 39499 - added aryRequestIDsUpdated, which needs to
	// return an array of unique request IDs that we just imported to
	// (j.jones 2010-11-05 09:31) - PLID 41341 - the array of responses is now the array of requests
	// that received responses, response text is tracked inside the request
	// (r.goldschmidt 2015-11-12 12:55) - PLID 65363 - add bNotepadWasOpened so we know if notepad was opened	
	//this also fills an array of the new responses we just imported
	BOOL CANSI271Parser::ParseRealTimeResponses(CArray<EligibilityInfo*, EligibilityInfo*> &aryRequestsWithResponses,
		OUT std::vector<long> &aryRequestIDsUpdated, OUT std::vector<long> &aryResponseIDsReturned, OUT bool &bNotepadWasOpened);

	//if we don't have an eligibility request ID from a trace, but we do have an
	//insured party ID, calculate the likely request ID
	void CalculateRequestID(EligibilityResponseDetailInfo *pDetail);

protected:

	void ClearResponseInfo();

	EligibilityResponseMasterInfo* GetCurrentResponseMasterInfo();
	EligibilityResponseDetailInfo* GetCurrentResponseDetailInfo();
	// (j.jones 2010-03-25 13:15) - PLID 37870 - added function to return the current benefit detail
	BenefitDetail* GetCurrentResponseBenefitDetailInfo();

	// (j.jones 2010-07-02 12:46) - PLID 39499 - moved the parsing into a new ParseContent function,
	// used by both ParseFile and ParseTextResponse
	// (j.jones 2010-11-05 09:49) - PLID 41341 - pass in our request ID, we only know it if
	// we are in a realtime import, as that is the only time the parsing would have only one response
	void ParseContent(IN OUT CString &strIn, BOOL &bIsValidFile, long nRealTimeRequestID = -1);

	//store the file name for later manipulation
	CString m_strFileName;

	char m_chSegmentTerminator;
	char m_chElementTerminator;
	char m_chCompositeSeparator;

	CString ParseSection(CString &strIn, char chDelimiter);
	CString ParseSegment(CString &strIn);
	CString ParseElement(CString &strIn);
	CString ParseComposite(CString &strIn);

	CString FixCapitalization(CString strCapitalized);
	CString ConvertDateFormat(CString strCCYYMMDD);
	CString ConvertTimeFormat(CString strHHMMSS);

	void OutputData(CString &OutputString, CString strNewData);

	//appends the text to either the current master response text, or the current detail response text,
	//based on the m_hllCurHLLevel value
	void StoreOutputInMemory(CString strText);

	//stores the last data output to the file, mainly to keep track of newlines
	CString m_strLastOutput;

	//stores what loop we are believed to be on
	CString m_strLoopNumber;

	// (j.jones 2016-05-16 09:24) - NX-100357 - tracks if the last NM1 loop
	// was an insurance company
	bool m_bLastNM1IsInsuranceCompany;

	// (j.jones 2016-05-17 15:38) - NX-100668 - track when we are in a 2120 loop
	bool m_bCurLoop2120;

	//for each loop we may indent all of its contents a given amount,
	//this is the indented data
	CString m_strOutputIndent;

	//these functions make up the 271 file
	BOOL ANSI_ST(CString &strIn);
	void ANSI_BHT(CString &strIn);
	void ANSI_HL(CString &strIn);
	void ANSI_AAA(CString &strIn);
	void ANSI_NM1(CString &strIn);
	void ANSI_REF(CString &strIn);
	void ANSI_PER(CString &strIn);
	void ANSI_TRN(CString &strIn);
	void ANSI_N3(CString &strIn);
	void ANSI_N4(CString &strIn);
	void ANSI_DMG(CString &strIn);
	void ANSI_INS(CString &strIn);
	void ANSI_HI(CString& strIn); // (c.haag 2010-10-20 15:33) - PLID 40350 - ANSI 5010 segment HI
	void ANSI_DTP(CString &strIn);
	void ANSI_MPI(CString &strIn); // (c.haag 2010-10-20 15:33) - PLID 40350 - ANSI 5010 segment MPI
	void ANSI_EB(CString &strIn);
	void ANSI_HSD(CString &strIn);
	void ANSI_MSG(CString &strIn);
	void ANSI_III(CString &strIn);		
	void ANSI_PRV(CString &strIn);
	void ANSI_LS(CString &strIn);
	void ANSI_LE(CString &strIn);
	void ANSI_SE(CString &strIn);

	//these are the interchange headers
	void ANSI_ISA(CString &strIn);
	void ANSI_IEA(CString &strIn);
	void ANSI_TA1(CString &strIn);
	void ANSI_GS(CString &strIn);
	void ANSI_GE(CString &strIn);

	CFile m_OutputFile;

	//compares the trace string to EligibilityRequestsT.ID, and sets it in the detail if it exists
	void CalculateIDFromTrace1(EligibilityResponseDetailInfo *pDetail, CString strTrace1);
	
	//compares the trace string to PatientsT.PersonID and InsuredPartyT.PersonID, and sets them in the detail if they exist
	void CalculateIDFromTrace2(EligibilityResponseDetailInfo *pDetail, CString strTrace2);

	HLLevel m_hllCurHLLevel;	//used to determine whether a current segment applies to a provider, patient, etc.

	// (j.jones 2010-03-26 13:34) - PLID 37905 - added maps for BenefitType, CoverageLevel, and ServiceType,
	// so we can cache the contents of EligibilityDataReferenceT upon first use
	CMap<CString, LPCTSTR, long, long> m_mapBenefitTypeQualifiersToIDs;
	CMap<CString, LPCTSTR, CString, LPCTSTR> m_mapBenefitTypeQualifiersToDescriptions;
	CMap<CString, LPCTSTR, long, long> m_mapCoverageLevelQualifiersToIDs;
	CMap<CString, LPCTSTR, CString, LPCTSTR> m_mapCoverageLevelQualifiersToDescriptions;
	CMap<CString, LPCTSTR, long, long> m_mapServiceTypeQualifiersToIDs;
	CMap<CString, LPCTSTR, CString, LPCTSTR> m_mapServiceTypeQualifiersToDescriptions;
	BOOL m_bBenefitTypesFilled, m_bCoverageLevelsFilled, m_bServiceTypesFilled;

	// (j.jones 2010-04-19 10:49) - PLID 38202 - cache the eligibility filter data,
	// so we can exclude outputting certain data if requestd
	BOOL m_bSuppressExcludedCoverage;
	CMap<CString, LPCTSTR, BOOL, BOOL> m_mapBenefitTypeQualifiersToExcluded;
	CMap<CString, LPCTSTR, BOOL, BOOL> m_mapServiceTypeQualifiersToExcluded;
	CMap<CString, LPCTSTR, BOOL, BOOL> m_mapCoverageLevelQualifiersToExcluded;

	// (j.jones 2010-03-26 13:34) - PLID 37905 - each of these functions will load the corresponding map on first use
	// (j.jones 2010-04-19 10:56) - PLID 38202 - added return value for whether the type is excluded from the output
	void GetBenefitTypeFromMap(CString strQualifier, long &nBenefitTypeRefID, CString &strDescription, BOOL &bExcludeFromOutput);
	void GetCoverageLevelFromMap(CString strQualifier, long &nCoverageLevelRefID, CString &strDescription, BOOL &bExcludeFromOutput);
	void GetServiceTypeFromMap(CString strQualifier, long &nServiceTypeRefID, CString &strDescription, BOOL &bExcludeFromOutput);

	// (j.jones 2010-07-02 13:52) - PLID 39499 - all actual importing to Practice data is now done inside this class,
	// and we now fill aryRequestIDsUpdated with a list of unique request IDs that we just imported to
	//this also fills an array of the new responses we just imported
	BOOL ImportParsedContentToData(BOOL &bMustOpenNotepadFile, BOOL bIsRealTimeImport, OUT std::vector<long> &aryRequestIDsUpdated, OUT std::vector<long> &aryResponseIDsReturned);
};

#endif // !defined(AFX_ANSI271PARSER_H__E37A6E3C_DEF8_4DB6_A695_7164CF0768D3__INCLUDED_)
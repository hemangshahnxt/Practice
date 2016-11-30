#pragma once

// (j.jones 2014-07-24 10:47) - PLID 62579 - created

enum TransactionActionCode {

	tacInvalid = -1,
	tacAdd = 1,			//A - indicates the assessment result is for the originating claim add
	tacChange,			//C - indicates the assessment result is for a change transaction
	tacDelete,			//D - indicates the assessment result is for a delete transaction
	tacReassessment,	//R - indicates the assessment result is for a re-assessment transaction
};

enum TransactionReassessReason {

	trrNone = -1,		//having no reason is normal
	trrHSCChange = 1,	//SRLE - HSC retro-active change
	trrProviderChange,	//RTRO - Service Provider retro-active change
	trrRecipientChange,	//CHEL - Service Recipient retro-active change
	trrAnotherService,	//ARUL - Affected by another Service - etc.
	trrPRRQ,			//PRRQ - this is not in the specs, we have no idea what it means, but we've found it in a file before
	trrMASC,			//MASC - this is not in the specs, we have no idea what it means, but we've found it in a file before
};

enum AssessmentResultAction_Main {

	aramInvalid = -1,
	aramRefused = 1,	//R - Indicates transaction refused. The Explanation Codes indicate the reason for refusal. Refused add transactions must be re-submitted as a new claim with a new claim number once the correct information is determined. If a change or delete transaction is refused, the claim with Alberta Health is left unchanged.
	aramHeld,			//H - Indicates transaction is currently being held for review by Alberta Health. Final outcome will be provided on a subsequent assessment result detail.
	aramApplied,		//A - Indicates transaction was applied and an assessed amount has been determined. The amount could be a reduced amount or could be 0.00. In these cases, the Explanation Codes indicate the reason for reduction. An applied claim can later be re-assessed, deleted, or changed.
};

enum AssessmentResultAction_Additional {

	araaReversal = 1,	//"R" indicates the record is a reversal of a previous assessment result for the claim.
	araaCurrent,		//"space" indicates the current assessment result.
};

struct AssessmentExplanation {

	CString strCode;
	CString strDescription;
};

//declares all the local memory objects that the parser will
//fill for later use by E-Remittance
namespace AlbertaAssessments {

	struct ChargeInfo {

		//fields provided by the Alberta Assessment file
		CString strClaimNum;
		CString strTransactionTagNumber;
		TransactionActionCode eTransactionActionCode;
		TransactionReassessReason eTransactionReassessReason;
		AssessmentResultAction_Main eAssessmentResultAction_Main;
		AssessmentResultAction_Additional eAssessmentResultAction_Additional; //TES 8/5/2014 - PLID 62580 - Renamed from strAssess... to eAssess...
		CString strChartNumber;
		CString strServiceRecipientULI;
		CString strServiceRecipientRegistrationNo;
		CString strStatementofAssessmentReferenceNumber;
		COleDateTime dtExpectedPaymentDate;
		COleDateTime dtAssessmentDate;
		COleCurrency cyFinalAssessedAmount;
		COleCurrency cyClaimedAmount;
		CString strClaimedAmountIndicator;
		std::list<AssessmentExplanation> aryExplanationCodes;
		CString strEMSAFStatus;
		CString strFeeModifiersUsed;
		CString strFRReferenceNumber;
		CString strBusinessArrangementNumber;
		CString strServiceProviderULI;
		COleDateTime dtServiceDate;
		CString strServiceCode;
		CString strPayToCode;		

		//fields calculated by us
		//TES 8/5/2014 - PLID 62580 - Added a bunch more fields in this section
		long nPatientID;
		CString strPatientFullName;
		long nUserDefinedID;
		long nProviderID;
		CString strProviderFullName;
		long nChargeID;
		long nBillID;
		long nInsuredPartyID;
		long nRespTypeID;
		BOOL bSubmitAsPrimary;
		long nInsuranceCoID;
		CString strInsuranceCoName;
		long nReversedPaymentID;

		ChargeInfo() {
			eTransactionActionCode = tacInvalid;
			eTransactionReassessReason = trrNone;
			eAssessmentResultAction_Main = aramInvalid;
			eAssessmentResultAction_Additional = araaCurrent;
			nPatientID = -1;
			nUserDefinedID = -1;
			nProviderID = -1;
			nChargeID = -1;
			nBillID = -1;
			dtExpectedPaymentDate = g_cdtNull;
			dtAssessmentDate = g_cdtNull;
			cyFinalAssessedAmount = g_ccyNull;
			cyClaimedAmount = g_ccyNull;
			dtServiceDate = g_cdtNull;
			nInsuredPartyID = -1;
			nRespTypeID = -1;
			bSubmitAsPrimary = FALSE;
			nInsuranceCoID = -1;
			nReversedPaymentID = -1;
		}
	};

	//TES 8/5/2014 - PLID 62580 - Changed this to a list of pointers, so it can be updated more easily
	typedef boost::shared_ptr<ChargeInfo> ChargeInfoPtr;
	typedef std::list<ChargeInfoPtr> ChargeList;
	typedef boost::shared_ptr<ChargeList> ChargeListPtr;
};

class CAlbertaAssessmentParser
{
public:
	CAlbertaAssessmentParser();
	~CAlbertaAssessmentParser();

	bool ParseFile(CWnd *pParentWnd);

	//tracks the date of the assessment file
	COleDateTime m_dtFileDate;

	//each charge has an expected payment date, presumably they
	//would be all the same, but this is the earliest of those dates
	COleDateTime m_dtFirstExpectedPaymentDate;

	//tracks a calculated total of all the amounts to be paid
	COleCurrency m_cyTotalAssessedAmount;

	//tracks all our parsed charge information for use by E-Remittance
	AlbertaAssessments::ChargeListPtr m_pChargeList;

	//TES 8/5/2014 - PLID 62580 - You can now specify a filename
	CString m_strFileName;
	CString m_strOutputFile;
	//TES 8/5/2014 - PLID 62580 - Track insurance information
	long m_nLikelyInsuranceCoID;
	//TES 8/5/2014 - PLID 62580 - Added
	void CalcInsuranceIDs();
	void ClearAllEOBs();
	void GetChargeIDsByPatientID(long nPatientID, CArray<long, long> &aryUsedCharges);

	//TES 9/15/2014 - PLID 62777 - Used to access charges based on the INT stored in the EOB datalist
	AlbertaAssessments::ChargeInfoPtr GetChargeFromRawPointer(AlbertaAssessments::ChargeInfo* pRawPointer);

	// (b.spivey, October 9th, 2014) PLID 62701 - get the stored file location. 
	CString GetStoredParsedFilePath();

private:

	CWnd *m_pParentWnd;

	// (j.jones 2014-07-24 10:48) - PLID 62579 - moved these functions from the EbillingFormDlg to this class
	CString FormatDate(CString s);

	// (j.jones 2014-07-24 17:01) - PLID 62579 - changed this to just fill our pRecord information
	//TES 8/5/2014 - PLID 62580 - Pass in a pointer
	void FindProviderByULI(AlbertaAssessments::ChargeInfoPtr pRecord);

	// (j.jones 2014-07-24 14:55) - PLID 62579 - changed this to just fill our pRecord information
	//TES 8/5/2014 - PLID 62580 - Pass in a pointer
	void FindPatient(AlbertaAssessments::ChargeInfoPtr pRecord);

	CString FormatMoney(CString s);

	// (j.jones 2014-07-25 09:45) - PLID 62579 - this now returns a friendly string and updates
	// the record to use the proper enumeration
	//TES 8/5/2014 - PLID 62580 - Pass in a pointer
	CString AssessmentResultMain(CString strResult, AlbertaAssessments::ChargeInfoPtr pRecord);
	
	// (j.jones 2014-07-25 09:50) - PLID 62579 - added correct handling, this also returns a
	// friendly string and updates the record to use the proper enumeration
	//TES 8/5/2014 - PLID 62580 - Pass in a pointer
	CString AssessmentResultAdditional(CString strResult, AlbertaAssessments::ChargeInfoPtr pRecord);

	// (j.jones 2014-07-25 09:55) - PLID 62579 - added enum translation for TransactionActionCode,
	// returns a friendly string, updates the record with the proper enum
	//TES 8/5/2014 - PLID 62580 - Pass in a pointer
	CString TransactionActionCode(CString strResult, AlbertaAssessments::ChargeInfoPtr pRecord);

	// (j.jones 2014-07-25 10:05) - PLID 62579 - added a map to reduce calls to the database
	std::map<CString, CString> m_mapExplanationCodeToDesc;
	// (j.jones 2014-07-25 10:16) - PLID 62579 - this now fills the explanation reason(s) in the record,
	// and returns a clean concatenated string for display purposes
	//TES 8/5/2014 - PLID 62580 - Pass in a pointer
	CString FindAlbertaExplanationCodes(CString s, AlbertaAssessments::ChargeInfoPtr pRecord);
	
	// (j.jones 2013-06-18 09:02) - PLID 57191 - gets the translation of a reassess reason
	// (j.jones 2014-07-25 09:45) - PLID 62579 - this now returns a friendly string and updates
	// the record to use the proper enumeration
	//TES 8/5/2014 - PLID 62580 - Pass in a pointer
	CString FindAlbertaReassessReason(CString strCode, AlbertaAssessments::ChargeInfoPtr pRecord);

	// (j.jones 2014-07-25 09:07) - PLID 62579 - added the ability to find the charge
	//TES 8/5/2014 - PLID 62580 - Pass in a pointer
	void FindCharge(AlbertaAssessments::ChargeInfoPtr pRecord);

	// (j.jones 2014-10-08 16:48) - PLID 62579 - this function backs up the remit file
	// and EOB.txt to the server's NexTech\ConvertedEOBs path, and also ensures that files
	// > 365 days old are deleted
	void CopyConvertedEOBToServer();

	// (b.spivey, October 9th, 2014) PLID 62701 
	CString m_strStoredParsedFile = "";

};
//{{AFX_INCLUDES()
#include "progressbar.h"
//}}AFX_INCLUDES
#if !defined(AFX_EELIGIBILITY_H__3BB7AE19_661D_4F08_961C_4A29C7C14ABB__INCLUDED_)
#define AFX_EELIGIBILITY_H__3BB7AE19_661D_4F08_961C_4A29C7C14ABB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EEligibility.h : header file
//

#include "HCFASetupInfo.h"

// (j.jones 2007-04-26 10:40) - PLID 25867 - created E-Eligibility

// (j.jones 2010-10-21 12:53) - PLID 40914 - added an enum for realtime eligibility clearinghouse,
// this is stored in data in a preference
enum EligibilityRealTime_ClearingHouse {
	
	ertcTrizetto = 0,
	ertcECP = 1,
};

enum InsuranceTypeCode;
enum ANSIVersion;

//this struct will hold key information for batched requests
struct EligibilityInfo {

	long nID;					//the EligibilityRequestsT ID
	long nPatientID;			//the PersonID for this patient	
	long nInsuranceCoID;		//the Insurance Company ID
	long nProviderID;			//the ProviderID for this request
	long nInsuredPartyID;		//the Insured Party ID for this patient/request
	long nLocationID;			//the Location ID for this request
	BOOL bPatientIsInsured;		//TRUE if InsuredPartyT.RelationToPatient = 'Self'
	// (j.jones 2009-01-16 08:59) - PLID 32754 - added strRelationToPatient
	CString strRelationToPatient;
	CString strInsuranceCoName; //the Insurance Company name
	CString strPayerID;			//the Insurance Company payer ID
	// (j.jones 2008-09-09 11:10) - PLID 18695 - converted NSF Code to InsType
	InsuranceTypeCode eInsType;	//the Insurance Company's InsType
	long nHCFASetupID;			//the Insurance Company's HCFA group ID
	CString strTaxonomyCode;	//the Provider's TaxonomyCode

	// (j.jones 2010-07-06 13:05) - PLID 39499 - track patient, provider, and location names, only used for reporting errors
	CString strPatientName;		//the patient's name
	CString strProviderName;	//the provider's name
	CString strLocationName;	//the location's name

	// (j.jones 2010-07-02 09:19) - PLID 39486 - when sending realtime, track the 270 request to be sent
	CString strRealTime270Request;

	// (j.jones 2010-11-05 09:09) - PLID 41341 - if we send this in real-time, track the 271 response
	// in this memory object - this is the only time it would be filled
	CString strRealTime271Response;

	// (j.jones 2014-03-13 10:27) - PLID 61363 - track the insurance ICD-10 setting and diag codes to export
	bool bUseICD10Codes;
	std::vector<CString> aryDiagCodes;
};

/////////////////////////////////////////////////////////////////////////////
// CEEligibility dialog

class CEEligibility : public CNxDialog
{
// Construction
public:
	CEEligibility(CWnd* pParent);   // standard constructor

	// (c.haag 2010-10-14 11:15) - PLID 40352 - cache the toggle for ANSI 4010/5010
	ANSIVersion m_avANSIVersion;
	//TES 7/1/2011 - PLID 40938 - Cache everything in EbillingFormatsT
	CString m_strFilenameElig, m_strPER05Qual_1000A, m_strPER06ID_1000A, m_strISA01Qual, m_strISA02, m_strISA03Qual, m_strISA04,
		m_strSubmitterISA05Qual, m_strSubmitterISA06ID, m_strReceiverISA07Qual, m_strReceiverISA08ID, m_strSubmitterGS02ID,
		m_strReceiverGS03ID;
	BOOL m_bUse1000APER;
	bool m_bEbillingFormatRecordExists;


	int m_FormatID;				//the ANSI settings we are using, EbillingFormatsT.ID

	// (j.jones 2010-07-02 09:16) - PLID 39486 - If enabled, we will not create
	// a text file, and instead just auto-send/receive via SOAP calls.
	// (j.jones 2010-10-21 12:34) - PLID 40914 - renamed to reflect that this is not Trizetto-only
	BOOL m_bUseRealTimeElig;	

	// (j.jones 2010-07-02 09:16) - PLID 39486 - If empty, we will send
	// all batched, selected requests. If filled, we only send the request
	// IDs listed in this array.
	CArray<long, long> m_aryRequestIDsToExport;
	//(s.dhole 09/11/2012) PLID 52414
	BOOL m_bCEligibilityRequestDetailDlg;
	// (j.jones 2008-05-07 15:32) - PLID 29854 - added nxiconbutton for modernization
// Dialog Data
	//{{AFX_DATA(CEEligibility)
	enum { IDD = IDD_EELIGIBILITY };
	CNxIconButton	m_btnCancel;
	CProgressBar	m_Progress;
	CNxStatic	m_nxstaticCurrEligEvent;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEEligibility)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// (j.jones 2007-06-27 16:55) - PLID 26480 - logs the export in MailSent
	void UpdateEligibilityHistory();

	//ExportData is the main function in the process from which all other functions are called.
	void ExportData();

	//ExportEligibility generates a file with all loaded eligibility requests
	//the return value is a defined error code
	int ExportEligibility();

	// (j.jones 2015-11-13 08:45) - PLID 67578 - Sends one request and returns the response.
	// Returns false if a warning prevented the send from being attempted.
	// Returns true if a send was attempted. It may or may not have succeeded.
	bool SendRealTimeEligibility(long nTimeoutSeconds, long nTimeoutRetryCount,
		OUT CArray<EligibilityInfo*, EligibilityInfo*> &arySOAPReceived,
		OUT CArray<EligibilityInfo*, EligibilityInfo*> &arySOAPFailed,
		OUT CArray<EligibilityInfo*, EligibilityInfo*> &arySOAPEmptyOrInvalid,
		OUT CArray<EligibilityInfo*, EligibilityInfo*> &arySOAPTimeout);

	//ResolveErrors takes in the return value of a function
	//and deals with it accordingly
	void ResolveErrors(int Error);

	//LoadRequestInfo loads key information from the batched requests into EligibilityInfo structs
	//and places them in the m_aryEligibilityInfo array.
	int LoadRequestInfo();

	CPtrArray m_aryEligibilityInfo;			//the array of EligibilityInfo structs
	EligibilityInfo *m_pEligibilityInfo;	//current request being processed

	// (j.jones 2010-10-21 12:53) - PLID 40914 - track the realtime eligibility clearinghouse
	EligibilityRealTime_ClearingHouse m_ertcClearinghouse;

	// (j.jones 2015-11-18 12:41) - PLID 67578 - generates a list of timed out patients
	CString GenerateTimeoutWarning(CArray<EligibilityInfo*, EligibilityInfo*> &aryTimeouts, bool bPromptForRetry);

	//The following functions represent individual 'loops' of a generated ANSI eligibility file.
	//Each function will generate its own line and write it to the output file.

	int ANSI_Header();	//Header
	int ANSI_2000A();	//Information Source Level
	int ANSI_2100A();	//Information Source Name
	int ANSI_2000B();	//Information Receiver Level
	int ANSI_2100B();	//Information Receiver Name
	int ANSI_2000C();	//Subscriber Level
	int ANSI_2100C();	//Subscriber Name
	int ANSI_2110C();	//Subscriber Eligibility Or Benefit Inquiry Information
	int ANSI_2000D();	//Dependent Level
	int ANSI_2100D();	//Dependent Name
	int ANSI_2110D();	//Dependent Eligibility Or Benefit Inquiry Information
	int ANSI_Trailer();	//Trailer

	int ANSI_InterchangeHeader();	//Interchange Control Header
	int ANSI_InterchangeTrailer();	//Interchange Control Trailer
	int ANSI_FunctionalGroupHeader();	//Functional Group Header
	int ANSI_FunctionalGroupTrailer();	//Functional Group Trailer
	
	// (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
	CString NormalizeName(CString str);

	//StripNonNum simply takes in a string, and returns only the numbers from that string
	CString StripNonNum(CString str);

	//StripPunctuation really shouldn't be used unless we have definitive evidence that a field needs it.
	//It works like StripNonNum. The returned string will only contain a-z, A-Z, 0-9, and space.
	//Normally, illegal characters are replaces with spaces, but if bReplaceWithSpaces is false,
	//then the character is removed altogether
	CString StripPunctuation(CString str, BOOL bReplaceWithSpaces = TRUE);

	//StripANSIPunctuation, for now, only strips out * and ~ but may strip out all punctuation later.
	CString StripANSIPunctuation(CString str);

	//ParseANSIField takes in a string (data), sizes appropriately to the Min and Max lengths,
	//determines whether it is justified left or right, and filles spaces with a given character
	//(usually a space or zero). Also, there is the optional bForceFill. We assume that if
	//an element is empty, then that is okay because the element will be unused. However there
	//are a few elements (notably in the transaction headers) that are required to be a
	//certain length even when they are blank. So bForceFill is set to TRUE when a minimum length
	//is required.
	CString ParseANSIField(CString data, long Min, long Max, BOOL bForceFill = FALSE, char Justify = 'L', char FillChar = ' ');

	//increments the count of segments, appends a ~, and writes OutputString to the file.
	void EndANSISegment(CString OutputString);

	CFile m_OutputFile;			//the output file containing generated claims
	CString m_ExportName;		//the name of the export file
	CString m_strBatchNumber;	//string version of BatchID, padded with zeros

	// (j.jones 2010-07-02 09:19) - PLID 39486 - track a string of one entire request,
	// used for SOAP calls only
	CString m_strCurrentRequestText;

	long m_ANSISegmentCount;	//count of segments outputted by EndSegment()
	long m_ANSIHLCount;			//count of Hierarchical Levels (unlimited)
	long m_ANSICurrPayerHL;		//current payer HL number
	long m_ANSICurrProviderHL;	//current provider HL number
	long m_ANSICurrSubscriberHL;	//current subscriber HL number
	long m_ANSICurrPatientHL;	//current patient HL number

	long m_PrevInsCo;			//the previous InsCo ID - assists in determing when to restart loops
	long m_PrevProvider;		//the previous provider ID - assists in determing when to restart loops	
	long m_PrevInsParty;		//the previous Insured Party ID - assists in determing when to restart loops
	long m_PrevPatient;			//the previous Patient ID - assists in determing when to restart loops

	//load these only once, as they is checked in every call of ParseField
	BOOL m_bCapitalizeEbilling;
	BOOL m_bStripANSIPunctuation;

	float m_ProgIncrement;	//the increment of the progress bar

	CHCFASetupInfo m_HCFAInfo;	//the information from the HCFASetupT table

	// Generated message map functions
	//{{AFX_MSG(CEEligibility)
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EELIGIBILITY_H__3BB7AE19_661D_4F08_961C_4A29C7C14ABB__INCLUDED_)

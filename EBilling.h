//{{AFX_INCLUDES()
#include "progressbar.h"
//}}AFX_INCLUDES
#if !defined(AFX_EBILLING_H__CC3BDF98_0DE4_4B2B_951B_8C3C6DCFD48B__INCLUDED_)
#define AFX_EBILLING_H__CC3BDF98_0DE4_4B2B_951B_8C3C6DCFD48B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Ebilling.h : header file
//

#include "HCFASetupInfo.h"
#include "UB92SetupInfo.h"
#include "GlobalFinancialUtils.h"
#include "NxApi.h"

//to better clarify the different format STYLES
enum FormatStyle // (a.walling 2014-03-19 10:05) - PLID 61346 - Now an enum
{
	//NSF = 1	// (j.jones 2008-05-09 09:37) - PLID 29986 - completely removed NSF from the program, we are not changing the indices though
	  IMAGE = 2
	, ANSI = 3
	, OHIP = 4
	, ALBERTA = 5	// (j.jones 2010-07-09 14:37) - PLID 29971 - added support for Alberta HLINK
}; 

//to better clarify the different claim types
//#define HCFA		1	//enum ANSIClaimType actProf = 1,	//Professional = HCFA form
//TES 3/13/2007 - PLID 24993 - Changed from UB92 to UB
//#define UB			2	//enum ANSIClaimType actInst = 2,	//Institutional = UB form	

// (j.jones 2010-10-13 10:42) - PLID 40913 - moved these into the header file
enum ErrorCode // (a.walling 2014-03-19 10:05) - PLID 61346 - Now an enum
{
	  Success = 0
	, Error_Missing_Info = 1
	, Error_Other = 2
	// (j.jones 2007-10-05 14:40) - PLID 25867 - added Cancel_Silent
	, Cancel_Silent = 3
};

// (j.jones 2008-11-12 12:33) - PLID 31740 - added structs for ANSI CAS segments
struct ANSI_CAS_Detail {

	CString strReasonCode;
	COleCurrency cyAmount;
};

struct ANSI_CAS_Info {

	CString strGroupCode;
	CArray<ANSI_CAS_Detail*, ANSI_CAS_Detail*> aryDetails;
};

// (j.jones 2010-03-02 13:45) - PLID 37584 - added enum for PriorAuthType,
// do not change these, these are stored in data
enum PriorAuthType {
	patPriorAuthNumber = 1,
	patReferralNumber = 2,
};


// (j.jones 2013-08-16 09:49) - PLID 58063 - moved ConditionDateType to GlobalFinancialUtils

// (j.jones 2010-10-18 15:54) - PLID 40346 - added enum for claim type,
//not stored in data
enum ANSIClaimType {

	actProf = 1,	//Professional = HCFA form
	actInst = 2,	//Institutional = UB form
	//actDental = 3,	//Dental = ADA form
};

// (j.jones 2014-08-25 10:01) - PLID 54213 - added enum for SBR01 qualifiers
enum SBR01Qualifier {

	sbrP = 1,	//P - Primary
	sbrS,		//S - Secondary
	sbrT,		//T - Tertiary
	sbrA,		//A - Payer Responsibility Four
	sbrB,		//B - Payer Responsibility Five
	sbrC,		//C - Payer Responsibility Six
	sbrD,		//D - Payer Responsibility Seven
	sbrE,		//E - Payer Responsibility Eight
	sbrF,		//F - Payer Responsibility Nine
	sbrG,		//G - Payer Responsibility Ten
	sbrH,		//H - Payer Responsibility Eleven
	sbrU,		//U - Unknown
};

// (j.jones 2012-03-21 14:48) - PLID 48870 - added a struct for Other Insurance Info
// (j.jones 2014-08-25 08:40) - PLID 54213 - added bIsOnBill, HasPaid, and SBR01Qual
struct OtherInsuranceInfo {

	long nInsuredPartyID;
	CString strHCFAPayerID;
	CString strUBPayerID;
	CString strTPLCode;				//InsuranceCoT.TPLCode for the OthrInsuredPartyID
	long nANSI_Hide2330AREF;		//fills from the HCFA or UB group for the "Other" insurance, instead of relying on the ANSI_Hide2330AREF value in m_HCFAInfo/m_UB92Info
	bool bIsOnBill;					//true if this is the "Other" insurance on the bill, false if this is a payer not selected in the bill	
	bool bHasPaid;					//true if the insured party has any payment or adjustment applied to this claim
	SBR01Qualifier sbr01Qual;		//the qualifier to use in Loop 2320 SBR01
};

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
// using namespace ADODB;

//this struct will hold key information for batched HCFAs
struct EBillingInfo {

	long HCFAID;				//the HCFA ID
	long BillID;				//the Bill ID
	long PatientID;				//the PatientID
	long UserDefinedID;			//the UserDefinedID for the patient	
	long ProviderID;			//the ID of the provider to be used on this form (Gen1 or Bill) (will be ClaimProviderID from Contacts if one is in use)
	// (j.jones 2008-04-02 16:52) - PLID 28995 - added ANSI RenderingProviderID and Box24J ProviderID
	long ANSI_RenderingProviderID;	//the ANSI_2310B override provider ID of the rendering provider from G1 or the Bill, is usually same as ProviderID
	long Box24J_ProviderID;		//the Box24J override provider ID of the rendering provider from G1 or the Bill, is usually same as ProviderID
	// (j.jones 2011-08-23 08:53) - PLID 44984 - added UB 2310B and 2310C provider ID
	long UB_2310B_ProviderID;	//the UB Box 77 provider, could be -1
	long UB_2310C_ProviderID;	//the UB Box 78 provider, could be -1
	long InsuredPartyID;		//the insured party ID for the HCFA
	long OthrInsuredPartyID;	//the other insured party ID for the HCFA
	long InsuranceCoID;			//the InsuranceCoID for the primary insured party on the HCFA
	long BillLocation;			//the Billing Location (LineItemT.LocationID)
	// (j.jones 2008-05-06 14:57) - PLID 29937 - added the Bill Location's NPI
	CString strBillLocationNPI;	//the Billing Location's NPI
	long nPOSID;				//the Place Of Service ID (BillsT.Location)
	// (j.jones 2013-04-24 17:27) - PLID 55564 - added the POS designation code (11, 22, etc.)
	CString strPOSCode;			//the Place Of Service code (PlaceOfServiceCodesT.PlaceCodes for ChargesT.ServiceLocationID)
	// (j.jones 2013-04-25 10:02) - PLID 55564 - added boolean to track whether we should send the patient address as the POS
	BOOL bSendPatientAddressAsPOS;	//TRUE if strPOSCode is 12 and the Claim_SendPatAddressWhenPOS12 preference is on
	long HCFASetupID;			//the HCFASetupID of the InsuranceCo - for referencing HCFASetupT
	long UB92SetupID;			//the UB92SetupID of the InsuranceCo - for referencing UB92SetupT
	long Box33Setup;			//the Box33Setup status for the InsuranceCo
	// (j.jones 2007-05-10 12:01) - PLID 25948 - needed the Box 82/76 setup for referring physician	
	long Box82Setup;			//the Box82/76Setup status for the InsuranceCo
	// (j.jones 2011-08-19 10:00) - PLID 44984 - added Box77 and 78
	long Box77Setup;			//the Box77Setup status for the InsuranceCo
	long Box78Setup;			//the Box83/78Setup status for the InsuranceCo	
	// (j.jones 2008-09-09 10:01) - PLID 18695 - converted NSF Code to InsType
	InsuranceTypeCode eInsType;	//the InsType for the InsuranceCo
	BOOL AnesthesiaTimes;		//whether or not Anesthesia times are enabled
	BOOL OnlyAnesthesiaMinutes;	//whether or not we should sent minutes and not times
	BOOL IsPrimary;				//whether or not the Insured Party we're sending to is the patients primary insurance
	// (j.jones 2008-12-11 13:28) - PLID 32401 - added RefPhyID and SupervisingProviderID, to reduce recordsets later
	long nRefPhyID;					//BillsT.RefPhyID
	long nSupervisingProviderID;	//BillsT.SupervisingProviderID
	// (j.jones 2009-08-05 10:13) - PLID 34467 - added payer IDs
	// (j.jones 2009-12-16 15:54) - PLID 36237 - split out into HCFA and UB payer IDs
	CString strHCFAPayerID;			//for the InsuredPartyID of the bill
	CString strUBPayerID;			//for the InsuredPartyID of the bill
	// (j.jones 2010-08-30 17:12) - PLID 15025 - added TPL info
	long nSendTPLCode;				//HCFASetupT/UB92SetupT.SendTPLCode
	// (j.jones 2013-06-20 12:27) - PLID 57245 - OriginalReferenceNumber is now cached
	CString strOriginalReferenceNumber;
	// (j.jones 2014-08-22 13:58) - PLID 63446 - tracks the earliest date of service on the claim
	COleDateTime dtFirstDateOfService;
	// (j.jones 2014-08-25 09:07) - PLID 54213 - track the qualifier to use in Loop 2000B SBR01, for the InsuredPartyID of the bill
	SBR01Qualifier sbr2000B_SBR01;

	// (j.jones 2012-03-21 15:12) - PLID 48870 - moved all data for the "Other" insured party into a pointer
	boost::optional<OtherInsuranceInfo> pOtherInsuranceInfo;

	// (a.walling 2014-03-19 10:05) - PLID 61346 - All diag codes for the bill
	std::vector<Nx::DiagCode> billDiags;

	// (a.walling 2014-03-19 10:05) - PLID 61346 - returns blank code if out of bounds
	Nx::DiagCode GetSafeBillDiag(size_t index)
	{
		if (index >= billDiags.size()) {
			return Nx::DiagCode();
		} else {
			return billDiags[index];
		}
	}
};

/////////////////////////////////////////////////////////////////////////////
// CEbilling dialog

class CEbilling : public CNxDialog
{
// Construction
public:
	//Box33Pin can be calculated for the primary or secondary
	CString Box33Pin(BOOL bCalcForSecondary = FALSE);
	CString Box17a();

	
	// (j.jones 2010-10-20 15:42) - PLID 40936 - moved Box17aANSI to GlobalFinancialUtils
	// (j.jones 2010-04-14 09:19) - PLID 38194 - moved Box33PinANSI to GlobalFinancialUtils	
	// (j.jones 2010-04-14 09:19) - PLID 38194 - moved Box8283NumANSI to GlobalFinancialUtils

	// (j.jones 2008-05-02 09:48) - PLID 27478 - now we are given an array so the calling function
	// can track which qualifiers we used
	// (j.jones 2010-10-13 11:18) - PLID 40913 - split into 4010 and 5010 versions
	void ANSI_4010_Output2010AAProviderIDs(long nProviderID, long nLocationID, CStringArray &arystrQualifiers);
	void ANSI_5010_Output2010AAProviderIDs(long nProviderID, long nLocationID, CStringArray &arystrQualifiers);

	//ResolveErrors takes in the return value of a function
	//and deals with it accordingly
	void ResolveErrors(int Error);

	//ExportToANSI processes all the commands necessary to send an ANSI X12 4010 or 5010 file
	//the return value is an Ebilling defined error code
	int ExportToANSI();

	//ExportToHCFAImage processes all the commands necessary to generate a HCFA Image
	//the return value is an Ebilling defined error code
	int ExportToHCFAImage();

	//ExportToOHIP processes all the commands necessary to generate an OHIP file
	//the return value is an Ebilling defined error code
	int ExportToOHIP();

	// (j.jones 2010-07-12 11:14) - PLID 29971 - supported Alberta HLINK
	int ExportToAlbertaHLINK();
	
	bool m_bAlbertaSubmitterPrefixCached;
	CString m_strAlbertaSubmitterPrefix;

	// (j.dinatale 2012-12-28 11:25) - PLID 54365 - need to be able to get a providers submitter prefix
	CString GetProviderSubmitterPrefix(long nProviderID);

	//the process of exporting a HCFA image is quite large, and isn't as easily
	//broken up as the ANSI format is. The following three functions are called from within
	//the ExportToHCFAImage function (they wouldn't compile when they were all in one function!)

	//HCFABoxes1to11 takes in a recordset opened in ExportToHCFAImage, generates boxes 1 through 11,
	//and returns an Ebilling defined error code
	// (j.jones 2011-04-20 11:18) - PLID 43329 - this function now takes in a list of Charge IDs to export,
	// so we don't have to calculate which charges to export twice
	int HCFABoxes1to11(ADODB::_RecordsetPtr &rsBills, IN CArray<long, long> &aryChargesToExport);

	//HCFABoxes12to23 takes in a recordset opened in ExportToHCFAImage, generates boxes 12 through 23,
	//and returns an Ebilling defined error code
	int HCFABoxes12to23(ADODB::_RecordsetPtr &rsBills);

	//HCFABoxes24to33 takes in a recordset opened in ExportToHCFAImage, generates boxes 24 through 33,
	//and returns an Ebilling defined error code
	int HCFABoxes24to33(ADODB::_RecordsetPtr &rsBills, long nCurPage, long nPages);

	// (j.jones 2007-05-15 09:11) - PLID 25953 - these "IntermediateNPI_" functions are for the interim
	// image that is the legacy layout but with NPIs appended in the lower right
	int IntermediateNPI_HCFABoxes1to11(ADODB::_RecordsetPtr &rsBills);
	int IntermediateNPI_HCFABoxes12to23(ADODB::_RecordsetPtr &rsBills);
	int IntermediateNPI_HCFABoxes24to33(ADODB::_RecordsetPtr &rsBills, long nCurPage, long nPages);

	// (j.jones 2006-11-09 15:42) - PLID 22668 - these "Legacy_" functions are for legacy support of the
	// pre-2007 HCFA Image, without NPI numbers and qualifiers
	// (j.jones 2007-05-15 09:11) - PLID 25953 - renamed from "Old_" to "Legacy_"
	int Legacy_HCFABoxes1to11(ADODB::_RecordsetPtr &rsBills);
	int Legacy_HCFABoxes12to23(ADODB::_RecordsetPtr &rsBills);
	int Legacy_HCFABoxes24to33(ADODB::_RecordsetPtr &rsBills, long nCurPage, long nPages);

	// (j.jones 2008-06-18 09:34) - PLID 30403 - CalculateBoxes29and30 will calculate the values
	// for Box 29 and Box 30 based on the settings to ShowPays, IgnoreAdjustments, ExcludeAdjFromBal,
	// and DoNotFillBox29. I made it a modular function because it is identical for all three image types.
	void CalculateBoxes29and30(const COleCurrency cyCharges, COleCurrency &cyApplies, COleCurrency &cyTotal, CString strChargeIDs);

	//LoadClaimInfo loads key information from the batched HCFAs into EBillingInfo structs
	//and places them in the m_aryEBillingInfo array.
	// (j.jones 2007-02-22 12:56) - PLID 24089 - changed return value to support sending errors back
	int LoadClaimInfo();

	//ZipFile will zip up an exported file prior to sending it, and return the success code
	bool ZipFile();

	//UpdateClaimHistory takes in the name of the Ebilling service and
	//updates the history of all claims that have been exported
	// (j.jones 2009-03-16 16:52) - PLID 32321 - added boolean to determine when OHIP
	// (j.jones 2011-07-13 10:44) - PLID 44542 - changed the boolean to reflect what it is really used for
	// (j.jones 2015-11-19 14:11) - PLID 67600 - removed the parameters, they are now calculated values
	void UpdateClaimHistory();

	// (a.walling 2016-01-11 16:03) - PLID 67828 - Normalize names, keep some punctuation
	CString NormalizeName(CString str);

	//StripNonNum simply takes in a string, and returns only the numbers from that string
	CString StripNonNum(CString str);

	// (j.jones 2013-05-06 17:48) - PLID 55100 - added function to strip spaces and hyphens only
	CString StripSpacesAndHyphens(CString str);

	//StripPunctuation really shouldn't be used unless we have definitive evidence that a field needs it.
	//The need for this is far more common in ANSI than any other format.
	//It works like StripNonNum. The returned string will only contain a-z, A-Z, 0-9, and space.
	//Normally, illegal characters are replaces with spaces, but if bReplaceWithSpaces is false,
	//then the character is removed altogether
	CString StripPunctuation(CString str, BOOL bReplaceWithSpaces = TRUE);

	//StripANSIPunctuation, for now, only strips out * and ~ but may strip out all punctuation later.
	CString StripANSIPunctuation(CString str);

	// (j.jones 2012-05-25 16:22) - PLID 50657 - FormatANSICurrency will first remove the dollar sign
	// and commas from the string. Then it will check the m_bTruncateCurrency setting, and if enabled,
	// will trim leading and trailing zeros.
	CString FormatANSICurrency(const CString strCurrency);

	//We use the same code so often, we encapsulated it in this function that will get
	//a string field (strField) from a recordset (rs) and a blank string if it fails
	CString GetFieldFromRecordset(ADODB::_RecordsetPtr rs, CString strField);

	//ParseField is the core function used when generating claims. It takes in a string (data),
	//the size that the string needs to be (Size), whether it should be justified left or right
	//(Justify - 'L' or 'R'), and the character to fill spaces with (FillChar - ' ' or  '0').
	//The return value is the finished string, formatted appropriately.
	CString ParseField(CString data, long Size, char Justify, char FillChar);

	//ParseANSIField is a variant of ParseField, with custom ANSI stuff. The difference
	//is that there are two sizes - "Min and Max". Obviously, they will help determine
	//the minimum size and maximum size. Most of the logic in this function that is different
	//is in regards to this change. Also, there is the optional bForceFill. We assume that if
	//an element is empty, then that is okay because the element will be unused. However there
	//are a few elements (notably in the transaction headers) that are required to be a
	//certain length even when they are blank. So bForceFill is set to TRUE when a minimum length
	//is required.
	// (j.jones 2012-10-30 16:49) - PLID 53364 - added override to disable punctuation stripping
	CString ParseANSIField(CString data, long Min, long Max,
		BOOL bForceFill = FALSE, char Justify = 'L', char FillChar = ' ',
		BOOL bKeepAllPunctuation = FALSE);

	//increments the count of segments, appends a ~, and writes OutputString to the file.
	void EndANSISegment(CString OutputString);

	//ExportData is the main function in Ebilling from which all other functions are called.
	void ExportData();

	//load these only once, as they is checked in every call of ParseField
	BOOL m_bCapitalizeEbilling;
	BOOL m_bStripANSIPunctuation;
	// (j.jones 2007-05-02 10:32) - PLID 25855 - cache the FourDigitRevCode value
	BOOL m_bFourDigitRevenueCode;
	// (j.jones 2008-02-15 12:31) - PLID 28943 - cache the SendSubscriberPerClaim setting
	BOOL m_bSendSubscriberPerClaim;
	// (j.jones 2008-02-19 11:40) - PLID 29004 - cache the Send2330BAddress setting
	BOOL m_bSend2330BAddress;
	// (j.jones 2008-09-09 16:45) - PLID 26482 - cache the HidePatAmtPaid setting
	BOOL m_bHidePatAmtPaid;
	// (j.jones 2009-10-01 14:02) - PLID 35711 - cache the PrepentPatientID data
	BOOL m_bPrependPatientID;
	CString m_strPrependPatientIDCode;
	// (j.jones 2010-04-16 11:14) - PLID 38225 - added ability to disable sending REF*XX in provider loops
	BOOL m_bDisableREFXX;
	bool m_bDeleteExportDirectory = false;

	// (b.spivey, August 27th, 2014) - PLID 63492 - Special claims file for previewing a bill. 
	CEbilling(CWnd* pParent, bool bFileExportOnly = false, long nBillID = -1);   // standard constructor

	// (j.jones 2008-05-09 09:37) - PLID 29986 - completely removed NSF from the program,
	// all of its functions are removed

	// (j.jones 2010-10-13 10:44) - PLID 40913 - renamed all ANSI 4010 functions to state 4010 in the function header,
	// the body for each of these functions is now in EbillingANSI4010.cpp

	//The following functions, ANSI_4010_1000A() to ANSI_4010_2440(), represent individual 'loops' of a generated ANSI claim file.
	//Each function will generate its own line and write it to the output file.

	int ANSI_4010_Header();	//Header
	int ANSI_4010_1000A();	//Submitter Name
	int ANSI_4010_1000B();	//Receiver Name
	int ANSI_4010_2000A();	//Billing/Pay-To Provider Hierarchical Level
	int ANSI_4010_2010AA();	//Billing Provider Name
	int ANSI_4010_2010AB();	//Pay-To Provider Name
	int ANSI_4010_2000B();	//Subscriber Hierachical Level
	int ANSI_4010_2010BA();	//Subscriber Name
	int ANSI_4010_2010BB();	//Payer Name
	//int ANSI_4010_2010BC();//Responsible Party Name
	//int ANSI_4010_2010BD();//Credit/Debit Card Holder Name
	int ANSI_4010_2000C();	//Patient Hierarchical Level
	int ANSI_4010_2010CA();	//Patient Name

	//Claim Information (HCFA/Professional claim)
	// (j.jones 2011-03-07 14:09) - PLID 42660 - the CLIA number is now calculated by our caller and is passed in
	// (j.jones 2011-04-05 14:31) - PLID 42372 - now we pass in a struct	
	// (j.jones 2011-04-20 10:45) - PLID 41490 - we now pass in info. on skipping diagnosis codes
	int ANSI_4010_2300_Prof(ECLIANumber eCLIANumber);
	int ANSI_4010_2300_Inst();	//Claim Information (UB92/Institutional claim)
	//int ANSI_4010_2305();	//Home Health Care Plan Information
	// (j.jones 2011-03-07 14:17) - PLID 42260 - added ability to force this function to load
	// the 2310B IDs of the rendering provider
	int ANSI_4010_2310A_Prof(BOOL bUseRenderingProvider);	//Referring Provider Name
	int ANSI_4010_2310A_Inst();	//Attending Physician Name
	int ANSI_4010_2310B_Prof();	//Rendering Provider Name
	// (j.jones 2011-08-19 10:00) - PLID 44984 - supported 2310B_Inst
	int ANSI_4010_2310B_Inst();	//Operating Physician Name
	//int ANSI_4010_2310C_Prof();	//Purchased Service Provider Name
	// (j.jones 2011-08-19 10:00) - PLID 44984 - supported 2310C_Inst
	int ANSI_4010_2310C_Inst();	//Other Provider Name
	int ANSI_4010_2310D_Prof();	//Service Facility Location
	// (j.jones 2008-12-11 13:20) - PLID 32401 - supported 2310E
	int ANSI_4010_2310E_Prof();	//Supervising Provider Name
	int ANSI_4010_2310E_Inst();	//Service Facility Name	
	int ANSI_4010_2320();	//Other Subscriber Information
	int ANSI_4010_2330A();	//Other Subscriber Name
	int ANSI_4010_2330B();	//Other Payer Name
	//int ANSI_4010_2330C();	//Other Payer Patient Information
	//int ANSI_4010_2330D();	//Other Payer Referring Provider
	//int ANSI_4010_2330E();	//Other Payer Rendering Provider
	//int ANSI_4010_2330F();	//Other Payer Purchased Service Provider
	//int ANSI_4010_2330G();	//Other Payer Service Facility Location
	//int ANSI_4010_2330H();	//Other Payer Supervising Provider
	
	//Service Line (HCFA/Professional claim)
	// (j.jones 2008-05-23 10:48) - PLID 29084 - added parameters for allowables
	// (j.jones 2011-04-20 10:45) - PLID 41490 - we now pass in info. on skipping diagnosis codes
	int ANSI_4010_2400_Prof(ADODB::_RecordsetPtr &rsCharges, BOOL &bSentAllowedAmount, COleCurrency &cyAllowableSent);
	
	int ANSI_4010_2400_Inst(ADODB::_RecordsetPtr &rsCharges);	//Service Line (UB92/Institutional claim)
	// (j.jones 2008-05-28 14:14) - PLID 30176 - added ANSI_2410
	// (j.jones 2009-08-12 16:48) - PLID 35096 - added support for unit information and prescription number
	int ANSI_4010_2410(CString strNDCCode, COleCurrency cyDrugUnitPrice, CString strUnitType, double dblUnitQty, CString strPrescriptionNumber);	//Drug Identification	
	int ANSI_4010_2420A(long ProviderID);	//Rendering Provider Name
	//int ANSI_4010_2420B();	//Purchased Service Provider Name
	//int ANSI_4010_2420C();	//Service Facility Location
	//int ANSI_4010_2420D();	//Supervising Provider Name
	//int ANSI_4010_2420E();	//Ordering Provider Name
	//int ANSI_4010_2420F();	//Referring Provider Name
	//int ANSI_4010_2420G();	//Other Payer Prior Authorization Or Referral Number
	// (j.jones 2008-05-23 10:48) - PLID 29084 - added parameters for allowables
	// (j.jones 2010-03-31 15:17) - PLID 37918 - added parameter for total count of charges being submitted
	int ANSI_4010_2430(ADODB::_RecordsetPtr &rsCharges, BOOL bSentAllowedAmount, COleCurrency cyAllowableSent, long nCountOfCharges);	//Line Adjudication Information
	//int ANSI_2440();	//Form Identification Code
	int ANSI_4010_Trailer();	//Trailer

	int ANSI_4010_InterchangeHeader();	//Interchange Control Header
	int ANSI_4010_InterchangeTrailer();	//Interchange Control Trailer
	int ANSI_4010_FunctionalGroupHeader();	//Functional Group Header
	int ANSI_4010_FunctionalGroupTrailer();	//Functional Group Trailer

	// (j.jones 2010-10-13 11:23) - PLID 32848 - created ANSI 5010 functions,
	// the body for each of these functions is in EbillingANSI5010.cpp

	//The following functions, ANSI_5010_1000A() to ANSI_5010_2440(), represent individual 'loops' of a generated ANSI claim file.
	//Each function will generate its own line and write it to the output file.

	int ANSI_5010_Header();	//Header
	int ANSI_5010_1000A();	//Submitter Name
	int ANSI_5010_1000B();	//Receiver Name
	int ANSI_5010_2000A();	//Billing/Pay-To Provider Hierarchical Level
	int ANSI_5010_2010AA();	//Billing Provider Name
	// (j.jones 2010-11-01 15:33) - PLID 40919 - we now pass in rsAddressOverride, which contains
	// the 2010AB override info (will often be eof)
	int ANSI_5010_2010AB(ADODB::_RecordsetPtr rsAddressOverride);	//Pay-To Address
	//int ANSI_5010_2010AC();	//Pay-To Plan Name
	int ANSI_5010_2000B();	//Subscriber Hierachical Level
	int ANSI_5010_2010BA();	//Subscriber Name
	int ANSI_5010_2010BB();	//Payer Name
	int ANSI_5010_2000C();	//Patient Hierarchical Level
	int ANSI_5010_2010CA();	//Patient Name

	//Claim Information (HCFA/Professional claim)
	// (j.jones 2011-03-07 14:09) - PLID 42660 - the CLIA number is now calculated by our caller and is passed in
	// (j.jones 2011-04-05 14:31) - PLID 42372 - now we pass in a struct
	int ANSI_5010_2300_Prof(ECLIANumber eCLIANumber);

	int ANSI_5010_2300_Inst();	//Claim Information (UB92/Institutional claim)
	//int ANSI_5010_2305();	//Home Health Care Plan Information
	// (j.jones 2011-03-07 14:17) - PLID 42260 - added ability to force this function to load
	// the 2310B IDs of the rendering provider
	int ANSI_5010_2310A_Prof(BOOL bUseRenderingProvider);	//Referring Provider Name
	int ANSI_5010_2310A_Inst();	//Attending Physician Name
	int ANSI_5010_2310B_Prof();	//Rendering Provider Name
	// (j.jones 2011-08-23 09:17) - PLID 44984 - supported 2310B_Inst
	int ANSI_5010_2310B_Inst();	//Operating Physician Name
	// (j.jones 2011-08-23 09:17) - PLID 44984 - supported 2310C_Inst
	int ANSI_5010_2310C_Inst();	//Other Operating Physician Name
	//int ANSI_5010_2310D_Inst();	//Rendering Provider Name
	int ANSI_5010_2310C_Prof();	//Service Facility Location
	int ANSI_5010_2310D_Prof();	//Supervising Provider Name
	//int ANSI_5010_2310E_Prof();	//Ambulance Pickup Location
	//int ANSI_5010_2310F_Prof();	//Ambulance Drop-Off Location
	int ANSI_5010_2310E_Inst();	//Service Facility Name	

	// (j.jones 2012-03-21 13:46) - PLID 48870 - these functions now require a OtherInsuranceInfo struct
	int ANSI_5010_2320(OtherInsuranceInfo oInfo);	//Other Subscriber Information
	int ANSI_5010_2330A(OtherInsuranceInfo oInfo);	//Other Subscriber Name
	int ANSI_5010_2330B(OtherInsuranceInfo oInfo);	//Other Payer Name

	//int ANSI_5010_2330C(OtherInsuranceInfo oInfo);	//Other Payer Referring Provider
	//int ANSI_5010_2330D(OtherInsuranceInfo oInfo);	//Other Payer Rendering Provider
	//int ANSI_5010_2330E(OtherInsuranceInfo oInfo);	//Other Payer Service Facility Location
	//int ANSI_5010_2330F(OtherInsuranceInfo oInfo);	//Other Payer Supervising Provider
	//int ANSI_5010_2330G(OtherInsuranceInfo oInfo);	//Other Payer Billing Provider
	
	//Service Line (HCFA/Professional claim)
	// (j.jones 2008-05-23 10:48) - PLID 29084 - added parameters for allowables
	// (j.jones 2011-04-20 10:45) - PLID 41490 - we now pass in info. on skipping diagnosis codes
	int ANSI_5010_2400_Prof(ADODB::_RecordsetPtr &rsCharges, BOOL &bSentAllowedAmount, COleCurrency &cyAllowableSent);

	int ANSI_5010_2400_Inst(ADODB::_RecordsetPtr &rsCharges);	//Service Line (UB92/Institutional claim)
	// (j.jones 2008-05-28 14:14) - PLID 30176 - added ANSI_2410
	// (j.jones 2009-08-12 16:48) - PLID 35096 - added support for unit information and prescription number
	// (j.jones 2010-10-18 09:11) - PLID 32848 - Price is obsolete in 5010, so I removed the parameter
	int ANSI_5010_2410(CString strNDCCode, CString strUnitType, double dblUnitQty, CString strPrescriptionNumber);	//Drug Identification	
	int ANSI_5010_2420A(long ProviderID);	//Rendering Provider Name
	//int ANSI_5010_2420B();	//Purchased Service Provider Name
	//int ANSI_5010_2420C();	//Service Facility Location
	// (j.jones 2014-04-23 14:51) - PLID 61834 - supported 2420D
	int ANSI_5010_2420D_Prof(long nPersonID);	//Supervising Provider Name
	// (j.jones 2013-06-03 11:56) - PLID 54091 - supported 2420E
	// (j.jones 2014-04-23 14:51) - PLID 61834 - added PersonID
	int ANSI_5010_2420E_Prof(long nPersonID);	//Ordering Provider Name
	// (j.jones 2014-04-23 14:51) - PLID 61834 - supported 2420F
	int ANSI_5010_2420F_Prof(long nPersonID);	//Referring Provider Name
	//int ANSI_5010_2310G_Prof();	//Ambulance Pickup Location
	//int ANSI_5010_2310H_Prof();	//Ambulance Drop-Off Location
	
	// (j.jones 2008-05-23 10:48) - PLID 29084 - added parameters for allowables
	// (j.jones 2010-03-31 15:17) - PLID 37918 - added parameter for total count of charges being submitted
	// (j.jones 2012-03-21 14:18) - PLID 48870 - this now requires an OtherInsuranceInfo struct
	int ANSI_5010_2430(OtherInsuranceInfo oInfo, ADODB::_RecordsetPtr &rsCharges, BOOL bSentAllowedAmount, COleCurrency cyAllowableSent, long nCountOfCharges);	//Line Adjudication Information
	
	//int ANSI_2440();	//Form Identification Code
	int ANSI_5010_Trailer();	//Trailer

	int ANSI_5010_InterchangeHeader();	//Interchange Control Header
	int ANSI_5010_InterchangeTrailer();	//Interchange Control Trailer
	int ANSI_5010_FunctionalGroupHeader();	//Functional Group Header
	int ANSI_5010_FunctionalGroupTrailer();	//Functional Group Trailer

	//These values are used to keep track of the current number of providers, subscribers, patients, claims, and services
	//They are incremented BEFORE a new one of each is added, i.e., when you add a new provider loop, you increment m_ANSIProviderCount, 
	//then add in the record.  This way, when lower loops need to look at the current ID of the parent loop, it can just grab the ID and
	//doesn't need to worry about subtracting from it to get the real ID.  
	//The variables are currently in the order of the loops, so when a new loop begins, all counters below it MUST be reset to 0.
	//The variables are all incrememnted in their specific loops (2000A, 2000B, 2000C, 2300, 2400), at the very beginning

	long	m_ANSIHLCount,						//count of Hierarchical Levels					(unlimited)
			m_ANSICurrProviderHL,				//current Provider HL number					(unlimited)
			m_ANSICurrSubscriberHL,				//current Subscriber HL number					(unlimited)
			m_ANSICurrPatientParent,			//current Patient's parent HL number			(unlimited)
			m_ANSIClaimCount,					//count of ANSI claim loops in this file		(100 per subscriber/patient max)
			m_ANSIServiceCount,					//count of ANSI service loops in this file		(50 per claim max)
			m_ANSICurrentPatientNumber;			//will either be the current subscriber count or the current patient count

	//functions for the OHIP export
	int OHIP_Batch_Header();
	int OHIP_Claim_Header1(CString strPlanName);
	int OHIP_Claim_Header2();
	int OHIP_Item_Record(ADODB::_RecordsetPtr &rsCharges);
	int OHIP_Batch_Trailer();

	//totals for the OHIP export
	long m_OHIP_ClaimHeader1_Count;				//count of Claim Header-1 records
	long m_OHIP_ClaimHeader2_Count;				//count of Claim Header-2 records
	long m_OHIP_Item_Record_Count;				//count of Item records

	// (j.jones 2010-07-12 08:43) - PLID 29971 - supported Alberta HLINK

	enum EAlbertaSegment {

		asCIB1 = 0,
		asCPD1,
		asCST1,
		asCTX1,
	};

	int Alberta_Header();
	// (j.jones 2011-07-21 12:25) - PLID 44662 - added more parameters for the purposes of potentially changing
	// the sequence number based on the action code
	// (j.jones 2011-09-20 09:11) - PLID 44934 - added strNote, only used when the segment is CST1
	// (j.dinatale 2013-01-18 16:57) - PLID 54419 - need to keep track of this now
	int Alberta_ClaimTransaction(long nChargeID, IN OUT long &nClaimHistoryDetailID,
								CString strServiceRecipientULI, IN OUT CString &strSequenceNumber,
								IN OUT ANSI_ClaimTypeCode &eActionCode, IN OUT BOOL &bHasChosenNewSequenceNumber,
								EAlbertaSegment asSegmentType, ADODB::_RecordsetPtr &rsCharges, CString &strNote, CString &strAlbertaTransactionNumber);
	int Alberta_ClaimSegment_CIB1(CString strServiceRecipientULI, CString &OutputString, ADODB::_RecordsetPtr &rsCharges);
	int Alberta_ClaimSegment_CPD1(CString &OutputString, ADODB::_RecordsetPtr &rsCharges);
	// (j.jones 2011-09-20 09:07) - PLID 44934 - this now takes in a note that may be spanned across multiple CST1 segments
	// which can be repeated 500 times
	int Alberta_ClaimSegment_CST1(CString &OutputString, CString &strNote);
	int Alberta_ClaimSegment_CTX1(CString &OutputString);
	int Alberta_Trailer();

	long m_nAlbertaTotalTransactionCounts; //count of unique transactions (charges)
	long m_nAlbertaCurrentSegmentCount;	//current transaction segment
	long m_nAlbertaTotalSegmentCounts;	//total count of transaction segments

	//used for the ANSI export, determines if it is an indiv. provider or a group
	BOOL m_bIsGroup;

	//used to differentiate ANSI Professional or Institutional
	// (j.jones 2010-10-18 15:54) - PLID 40346 - added enum for claim type
	ANSIClaimType m_actClaimType;

	//Totals for the ANSI format
	long	m_ANSISegmentCount;					//count of segments outputted by EndSegment()

	long	m_BatchCount,						//the number of claims to be exported
			m_PrevProvider,						//the previous provider ID - assists in determing when to start a new batch	
			m_PrevInsCo,						//the previous InsCo ID - assists in determing when to start a new batch	
			m_PrevInsParty,						//the previous Insured Party - same usage as above, but only ANSI
			m_PrevLocation,						//the previous Location - same usage as the rest
			
			m_nCurrPage;						//for HCFA Image only, determines which page we are printing

	CPtrArray m_aryEBillingInfo;				//the array of EBillingInfo structs

	CHCFASetupInfo m_HCFAInfo;					//the information from the HCFASetupT table
	CUB92SetupInfo m_UB92Info;					//the information from the UB92SetupT table

	CFile m_OutputFile;							//the output file containing generated claims

	CString m_ExportName,						//the name of the export file
			m_ZipName,							//the name of the ZipFile			
			m_Contact,							//the office's ebilling contact
			m_SiteID,							//the Site ID
			m_VendorID;							//the Vendor ID

	CString m_strLast2010AAQual;				//the qualifier used for the provider ID in 2010AA

	// (j.jones 2012-01-06 15:06) - PLID 47351 - store the 2010AA NM102/NM103 values,
	// so we know if we sent a 1 or 2 for person or location, and the location name (*used in 5010 only)
	CString m_strLast2010AA_NM102;
	CString m_strLast2010AA_NM103;

	long m_nFilterByLocationID; //the location we are filtering on, if any
	long m_nFilterByProviderID;	//the provider we are filtering on, if any
	int m_FormatID;	//the format we are exporting
	long m_BatchID;	//the BatchID from ConfigRT
	int m_FormatStyle; //the style we are exporting (Image, ANSI, OHIP)
	CString m_strBatchNumber;	//string version of BatchID, padded with zeros

	bool m_bIsClearinghouseIntegrationEnabled = false;
	NexTech_Accessor::_ClearinghouseLoginPtr m_pClearinghouseLogin = nullptr;

	ADODB::_RecordsetPtr m_rsBill, m_rsDoc;	//member recordsets to make the export more efficient
	ADODB::FieldsPtr m_BillFields;				//pointer to the bill recordset fields

	COleCurrency m_TotalClaimCharges,	//the running total of charges for the current claim
				 m_TotalClaimPaid,		//the running total of payments for the current claim
				 m_TotalBatchCharges,	//the running total of charges for the current batch
				 m_TotalFileCharges;	//the running total of charges for the file				

	EBillingInfo *m_pEBillingInfo; //current claim being processed

	float m_ProgIncrement;	//the increment of the progress bar
	
	void OutputHCFAImageAnesthesiaTimes(long nBillID, BOOL bOnlyAnesthesiaMinutes);

	// (j.jones 2008-05-07 15:07) - PLID 29854 - added nxiconbutton for modernization
// Dialog Data
	//{{AFX_DATA(CEbilling)
	enum { IDD = IDD_EBILLING };
	CNxIconButton	m_btnCancel;
	CProgressBar	m_Progress;
	// (j.jones 2009-06-16 17:40) - PLID 34643 - removed xceedzip control
	//CXceedZip	m_FileZip;
	CNxStatic	m_nxstaticCurrEvent;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEbilling)
	public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// (j.jones 2008-11-12 12:33) - PLID 31740 - added abilities to group CAS segments together,
	// to protect against accidentally sending duplicate group codes
	// (j.jones 2009-08-28 16:55) - PLID 35006 - added a boolean to allow zero
	void AddCASSegmentToMemory(CString strGroupCode, CString strReasonCode, COleCurrency cyAmount, CArray<ANSI_CAS_Info*, ANSI_CAS_Info*> &aryCASSegments, BOOL bAllowZeroAmt = FALSE);
	// (j.jones 2010-10-13 11:18) - PLID 40913 - split into 4010 and 5010 versions
	void ANSI_4010_OutputCASSegments(CArray<ANSI_CAS_Info*, ANSI_CAS_Info*> &aryCASSegments);
	void ANSI_5010_OutputCASSegments(CArray<ANSI_CAS_Info*, ANSI_CAS_Info*> &aryCASSegments);

	// (j.jones 2008-05-02 09:57) - PLID 27478 - simple CStringArray search utility
	BOOL IsQualifierInArray(CStringArray &arystrQualifiers, CString strQualifier);

	// (j.jones 2008-05-06 11:04) - PLID 29937 - cached all EbillingFormatsT options
	// that weren't previously cached
	CString m_strEbillingFormatName, m_strEbillingFormatContact, m_strEbillingFormatFilename,
		m_strISA01Qual, m_strISA02, m_strISA03Qual, m_strISA04, m_strReceiverISA08ID,
		m_strReceiverGS03ID, m_strReceiver1000BID, m_strSubmitterISA06ID, m_strSubmitterGS02ID,
		m_strSubmitter1000AID, m_strReceiverISA07Qual, m_strReceiver1000BQual,
		m_strSubmitterISA05Qual, m_strSubmitter1000AQual, m_strAddnl_2010AA_Qual, 
		m_strAddnl_2010AA, m_strPER05Qual_1000A, m_strPER06ID_1000A;

	// (j.jones 2009-08-04 11:34) - PLID 14573 - renamed to PrependTHINPayerNSF to PrependPayerNSF,
	// (a.wilson 2014-06-27 10:51) - PLID 62518
	// and removed m_bUseTHINPayerIDs
	BOOL m_bEbillingFormatZipped, m_bUse2420A,
		m_bUseSV106, m_bUse2010AB, m_bDontSubmitSecondary, m_bExport2330BPER,
		m_bExportAll2010AAIDs, m_bTruncateCurrency,	m_bUseSSN, m_bPrependPayerNSF,
		m_bUse_Addnl_2010AA, m_bSeparateBatchesByInsCo, m_bUse1000APER, m_bDontSendSecondaryOnDiagnosisMismatch;
	
	// (j.jones 2012-01-06 15:16) - PLID 47351 - this is Hide2310D in data, but I renamed it to be
	// 5010 compliant and make more sense with respect to our other new 2310C-hiding ability
	BOOL m_bHide2310C_WhenType11Or12;

	// (j.jones 2012-01-06 15:13) - PLID 47351 - added option to not send place of service if the same
	// location as 2010AA, HCFA only, 5010 only
	BOOL m_bHide2310C_WhenBillLocation;

	// (b.spivey, August 27th, 2014) - PLID 63492 - a bool for previewing a claims file, and long designating which claims file. 
	bool m_bFileExportOnly;
	bool m_bHumanReadableFormat;
	long m_nBillID; 

	// (j.jones 2010-10-13 13:29) - PLID 40913 - cache the toggle for ANSI 4010/5010
	ANSIVersion m_avANSIVersion;

	// (a.walling 2014-03-17 14:31) - PLID 61202 - Returns whether ICD10 should be used based on the m_pEBillingInfo's InsuredParty and BillID and m_FormatStyle+m_avANSIVersion
	bool ShouldUseICD10() const;

	// (a.walling 2014-03-19 10:05) - PLID 61346 - Only skip unlinked diags when using a HCFA export with the SkipUnlinkedDiagsOnClaims preference enabled
	bool ShouldSkipUnlinkedDiags() const;

	// (a.wilson 2014-06-30 09:46) - PLID 62518 - determine whether the secondary and primary insurance have the same diagnosis code sets.
	bool DiagnosisCodeSetMismatch(bool bInsuredPartyShouldUseICD10) const;

	// (j.jones 2014-08-25 08:45) - PLID 54213 - given an array of OtherInsuranceInfo objects,
	// try to remove unnecessary duplicate payers, and update the SBR01 qualifier for those
	// that remain
	void ScrubOtherInsuredPartyList(CArray<OtherInsuranceInfo, OtherInsuranceInfo> &aryOtherInsuredPartyInfo);

	// (j.jones 2014-08-25 09:09) - PLID 54213 - returns "P" for Primary, "S" for secondary,
	// or additional SBR01 qualifiers based on the RespTypeT.Priority
	SBR01Qualifier CalculateSBR01Qualifier(bool bIs2000B, bool bSendingToPrimary, long nANSI_EnablePaymentInfo, long nPriority);

	// (j.jones 2014-08-25 10:16) - PLID 54213 - converts the SBR01Qualifier enum to a string
	CString OutputSBR01Qualifier(SBR01Qualifier sbr);

	// Gets the message to display in the progress window when claims are being processed.
	CString GetExportEventMessage();

	/// <summary>
	/// Attempts to upload the exported claim file to the Nextech Claim Service. Reports any errors
	/// to the user in a message box. Silent upon success.
	/// </summary>
	/// <returns>True if successfully uploaded the claim file. False if some failure occurred.</returns>
	bool UploadClaimFile();

	// Generated message map functions
	//{{AFX_MSG(CEbilling)
	virtual BOOL OnInitDialog();
	CString GetExportFilePath();
	afx_msg void OnListingFileFileZip(LPCTSTR sFilename, LPCTSTR sComment, long lSize, long lCompressedSize, short nCompressionRatio, long xAttributes, long lCRC, DATE dtLastModified, DATE dtLastAccessed, DATE dtCreated, long xMethod, BOOL bEncrypted, long lDiskNumber, BOOL bExcluded, long xReason);
	virtual void OnCancel();
	afx_msg void OnTimer(UINT nIDEvent);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EBILLING_H__CC3BDF98_0DE4_4B2B_951B_8C3C6DCFD48B__INCLUDED_)

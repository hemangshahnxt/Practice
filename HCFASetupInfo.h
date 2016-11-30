// HCFASetupInfo.h: interface for the CHCFASetupInfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HCFASETUPINFO_H__897A4646_E17F_416B_B2D1_0E3B98988CDF__INCLUDED_)
#define AFX_HCFASETUPINFO_H__897A4646_E17F_416B_B2D1_0E3B98988CDF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

enum ANSI_Hide2310AOptions;	// (j.jones 2014-04-29 08:49) - PLID 61840 - declared in GlobalFinancialUtils.h

class CHCFASetupInfo  
{
public:
	CHCFASetupInfo();
	CHCFASetupInfo(long HCFASetupGroupID);
	virtual ~CHCFASetupInfo();

	void Initialize();
	void LoadData(long HCFASetupGroupID);

	long ID;
	CString Name;
	long Address;
	long Box3;
	long Box9b;
	long Box33Setup;
	long Box17a;
	long Box33;
	CString Box11;
	long Box11Rule;
	long Box11a;
	long Box25;
	long Box31;
	long Box31Show;
	CString Box33GRP;
	long ExportFA8;  //obsolete, but still loaded for now
	long GRPAsEMC;
	long Box11D;
	long DefBatch;
	long DefBatchSecondary;
	long Box33Order;
	long ShowPays;
	long IgnoreAdjustments;
	long Box3E;
	long Box11aE;
	long Box9bE;
	long Box24To;
	long Box24From;
	long Box16;
	long Box18;
	long Box14;
	long Box15;
	long Box12;
	long Box3Wide;
	long Box9bWide;
	long Box11aWide;
	long Box31Wide;
	long Box24FWide;
	long Box24TWide;
	long Box16Wide;
	long Box18Wide;
	long Box14Wide;
	long Box15Wide;
	long Box12Wide;
	long HideBox4;
	long HideBox7 ;
	long HideBox9;
	long HideBox11;
	long UseEmp;
	long Box33Num;
	long HidePhoneBox33;
	long Box12Accepted;
	long Box13Accepted;
	CString DefaultPriorAuthNum;
	long DocAddress;
	long ShowDiagDecimals;
	long ShowICD9Desc;
	long Box24C;
	// (j.jones 2007-03-26 15:45) - PLID 24538 - I removed this, but by client demand I reinstated it
	long LeftAlignInsAddress;
	long Box19RefPhyID;
	long Box19Override;
	CString Box19;
	long HideBox30;
	long ShowSecInsAdd;
	long Box17Order;
	long ANSI_SendBox19;
	long ANSI_SendRefPhyIn2300;
	long ANSI_UseAnesthMinutesAsQty;
	long ShowAnesthesiaTimes;
	long Box12UseDate;
	long Box31UseDate;
	CString strCorrespondenceNote;
	long ShowRespName;
	// (j.jones 2009-08-03 14:58) - PLID 33827 - removed Use2310B
	//BOOL Use2310B;
	BOOL Use2310BPRVSegment;
	long Use2000APRVSegment;
	long ShowSecPINNumber;
	long ShowSec24JNumber;
	long ExcludeAdjFromBal;
	long UseBox23InBox17a;
	long UseOverrideInBox31;
	long PrintPinBox1a;
	long Use1aIn9a;
	CString Box17aQual;
	CString Box33bQual;
	long ShowAnesthMinutesOnly;
	long Use1aIn9aAlways;
	long SecondaryFillBox11;
	long NM109_IDType;
	long ExtraREF_IDType;
	long LocationNPIUsage;
	long ANSI_EnablePaymentInfo;
	long ANSI_AdjustmentLoop;
	long ANSI_2300Contract;
	long ANSI_2320Allowed;
	long ANSI_2320Approved;
	long ANSI_2400Contract;
	long ANSI_2400Allowed;
	// (j.jones 2007-02-21 12:24) - PLID 24776 - added QualifierSpace
	long QualifierSpace;
	// (j.jones 2007-03-28 13:31) - PLID 25392 - added ShowWhichCodesCommas
	long ShowWhichCodesCommas;
	// (j.jones 2007-03-29 11:49) - PLID 25409 - added ANSI_SecondaryAllowableCalc
	// (j.jones 2009-08-28 17:45) - PLID 32993 - removed this setting
	//long ANSI_SecondaryAllowableCalc;
	// (j.jones 2007-04-09 16:14) - PLID 25537 - added HideBox11a/b/c
	long HideBox11a_Birthdate;
	long HideBox11a_Gender;
	long HideBox11b;
	long HideBox11c;
	// (j.jones 2007-04-09 17:46) - PLID 25539 - added Hide24ATo
	long Hide24ATo;
	// (j.jones 2007-05-11 11:39) - PLID 25932 - revised Box 32 hide options
	long HideBox32NameAdd;
	long HideBox32NPIID;
	// (j.jones 2007-06-15 10:08) - PLID 26309 - added group/reason code defaults
	CString ANSI_DefaultRemAdjGroupCode;
	CString ANSI_DefaultRemAdjReasonCode;

	// (j.jones 2007-07-12 14:28) - PLID 26636 - added ability to hide 24J NPI
	long HideBox24JNPI;

	// (j.jones 2008-02-06 17:15) - PLID 28843 - added SecondaryInsCodeUsage and SecondaryInsCode
	BOOL bSecondaryInsCodeUsage;
	CString strSecondaryInsCode;

	// (j.jones 2008-02-25 09:06) - PLID 29077 - added ANSI_SecondaryApprovedCalc
	long ANSI_SecondaryApprovedCalc;

	// (j.jones 2008-04-01 16:17) - PLID 29486 - added ANSI_Hide2330AREF
	long ANSI_Hide2330AREF;

	// (j.jones 2008-06-10 14:50) - PLID 25834 - added ValidateRefPhy
	long ValidateRefPhy;

	// (j.jones 2008-06-18 09:05) - PLID 30403 - added DoNotFillBox29
	long DoNotFillBox29;

	// (j.jones 2008-06-23 12:23) - PLID 30434 - added Eligibility_2100C_REF_Option
	long Eligibility_2100C_REF_Option;

	// (j.jones 2010-05-25 15:43) - PLID 38868 - added Eligibility_2100D_REF
	long Eligibility_2100D_REF;

	// (j.jones 2008-10-06 10:17) - PLID 31580 - added ANSI_SendCorrespSegment
	long ANSI_SendCorrespSegment;

	// (j.jones 2008-10-06 13:03) - PLID 31578 - added ANSI_SendBox19Segment
	long ANSI_SendBox19Segment;

	// (j.jones 2008-11-12 10:58) - PLID 31912 - added bANSI_SendCASPR
	BOOL bANSI_SendCASPR;

	// (j.jones 2009-01-06 09:09) - PLID 32614 - added ability to hide 17b
	long HideBox17b;

	// (j.jones 2009-03-06 11:18) - PLID 33354 - added PullCurrentAccInfo
	long PullCurrentAccInfo;

	// (j.jones 2009-03-11 10:36) - PLID 33446 - added default insurance adjustment group/reason codes
	CString ANSI_DefaultInsAdjGroupCode;
	CString ANSI_DefaultInsAdjReasonCode;

	// (j.jones 2009-11-24 15:45) - PLID 36411 - added PriorAuthQualifier
	CString PriorAuthQualifier;

	// (j.jones 2010-03-31 15:15) - PLID 37918 - added ANSI_Hide2430_SVD06_OneCharge
	long ANSI_Hide2430_SVD06_OneCharge;

	// (j.jones 2010-08-31 09:58) - PLID 15025 - added ANSI_SendTPLNumber
	long ANSI_SendTPLNumber;

	// (j.jones 2010-08-31 10:37) - PLID 40303 - added TPLIn9a
	long TPLIn9a;

	// (j.jones 2011-06-15 12:51) - PLID 42181 - added EligNPIUsage
	long EligNPIUsage;

	// (j.jones 2012-03-23 15:33) - PLID 49176 - added PRV2000ACode
	long PRV2000ACode;

	// (j.jones 2012-05-14 14:01) - PLID 50338 - added ANSI_SBR04
	// (j.jones 2012-10-04 10:07) - PLID 53018 - removed ANSI_SBR04 and added
	// ANSI_2000B_SBR03, ANSI_2000B_SBR04, and ANSI_2320_SBR04
	long ANSI_2000B_SBR03;
	long ANSI_2000B_SBR04;
	long ANSI_2320_SBR04;

	// (j.jones 2013-04-11 09:01) - PLID 56166 - added ANSI_HideRefPhyFields
	long ANSI_HideRefPhyFields;

	// (j.jones 2013-06-10 16:26) - PLID 56255 - added UseRefPhyGroupNPI
	long UseRefPhyGroupNPI;

	// (j.jones 2013-06-20 11:39) - PLID 57245 - added OrigRefNo_2300 and OrigRefNo_2330B
	long OrigRefNo_2300;
	long OrigRefNo_2330B;

	// (j.jones 2013-07-08 15:08) - PLID 57469 - added EligTitleInLast
	long EligTitleInLast;

	// (j.jones 2013-07-17 16:06) - PLID 41823 - added DontBatchSecondary
	long DontBatchSecondary;

	// (j.jones 2014-01-22 09:49) - PLID 60034 - added OrderingProvider
	long OrderingProvider;

	// (j.jones 2014-04-23 13:47) - PLID 61840 - added Hide 2310A setting
	ANSI_Hide2310AOptions Hide2310A;

	// (b.spivey January 26, 2015) - PLID 64452 - added SendN3N4PERsegment
	bool SendN3N4PERSegment;
};

#endif // !defined(AFX_HCFASETUPINFO_H__897A4646_E17F_416B_B2D1_0E3B98988CDF__INCLUDED_)

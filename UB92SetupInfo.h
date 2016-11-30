// UB92SetupInfo.h: interface for the CUB92SetupInfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UB92SETUPINFO_H__290B5758_00BB_476E_8185_85281E87701A__INCLUDED_)
#define AFX_UB92SETUPINFO_H__290B5758_00BB_476E_8185_85281E87701A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "UB04Utils.h"

class CUB92SetupInfo  
{
public:
	CUB92SetupInfo();
	CUB92SetupInfo(long UB92SetupGroupID);
	virtual ~CUB92SetupInfo();

	void Initialize();
	void LoadData(long UB92SetupGroupID);

	long ID;
	CString Name;
	long Box38;
	long DefBatch;
	long DefBatchSecondary;
	long DateFmt;
	long Box82Setup;
	long Box83Setup;
	long Box8283Format;
	long Box82Num;
	long Box83Num;
	long Box64Show;
	long Box76Show;
	long Box85Show;
	long GroupRev;
	CString Box4;
	CString Box8;
	CString Box10;
	CString Box18;
	CString Box19;
	CString Box20;
	CString Box22;
	CString Box79;
	CString Box42Line23;
	long ShowPhone;
	long ShowTotals;
	long ShowInsAdd;
	long InsAddr50;
	CString Box51Default;
	long Box1Loc;
	long PunctuateChargeLines;
	long PunctuateDiagCodes;
	CString Box32;
	long Fill42With44;
	long ShowApplies;
	long Box6;
	long Box14;
	long Box17;
	long Box32_36;
	long Box45;
	long Box80_81;
	long Box86;
	long Box80Number;
	long ShowCompanyAsInsurer;
	long Box53Accepted;
	long Box5ID;
	long NM109_IDType;
	long ExtraREF_IDType;
	long ANSI_EnablePaymentInfo;
	long ANSI_AdjustmentLoop;
	long ANSI_2300Contract;
	long ANSI_2320Allowed;

	//TES 3/19/2007 - PLID 25235 - UB04-only fields.
	CString UB04Box76Qual;
	long UB04Box77Setup;
	CString UB04Box77Qual;
	long UB04Box77Num;
	CString UB04Box78Qual;

	// (j.jones 2007-03-20 12:34) - PLID 25278 - supported UB04 Box 56 NPI (UB04-only)
	long UB04Box56NPI;

	// (j.jones 2007-03-21 10:05) - PLID 25279 - supported UB04 Box 81 Taxonomy Code
	CString UB04Box81aQual;
	long UB04Box81a;
	CString UB04Box81bQual;
	long UB04Box81b;
	CString UB04Box81cQual;
	long UB04Box81c;

	//TES 3/21/2007 - PLID 25295 - Two new checkboxes to fill the "Other Prv ID" in Boxes 51 or 57
	long UB04Box51Show;
	long UB04Box57Show;

	// (j.jones 2007-03-29 11:49) - PLID 25409 - added ANSI_SecondaryAllowableCalc
	// (j.jones 2009-08-28 17:45) - PLID 32993 - removed this setting
	//long ANSI_SecondaryAllowableCalc;

	// (a.walling 2007-06-05 09:18) - PLID 26219 - Zero fill page numbers as 001
	long UB04Box42Line23;

	// (a.walling 2007-06-05 11:40) - PLID 26223 - Use three lines for address in 80
	long UB04Box80UseThree;

	// (a.walling 2007-06-05 13:36) - PLID 26228 - ID of the category to expand group codes for
	// (a.walling 2007-08-17 14:05) - PLID 27092 - make that an array of IDs to expand
	CArray<long, long> arUB04GroupRevExpand;

	// (j.jones 2007-06-15 10:08) - PLID 26309 - added group/reason code defaults
	CString ANSI_DefaultRemAdjGroupCode;
	CString ANSI_DefaultRemAdjReasonCode;
	
	// (a.walling 2007-06-20 13:25) - PLID 26414 - Whether to use ICD-9-CM Procedure Codes
	long UB04UseICD9ProcedureCodes;

	// (j.jones 2007-07-12 08:29) - PLID 26621 - added Box 2 setup
	long UB04Box2Setup;

	// (j.jones 2007-07-12 10:35) - PLID 26625 - added new Box 1 setup (UB04 only)
	long UB04Box1Setup;

	// (j.jones 2008-04-01 16:17) - PLID 29486 - added ANSI_Hide2330AREF
	long ANSI_Hide2330AREF;

	// (j.jones 2008-05-21 10:34) - PLID 30129 - added UB04 Box 66
	CString UB04Box66;

	// (j.jones 2008-05-23 09:05) - PLID 29939 - added ANSI_SendRefPhyIn2300;
	long ANSI_SendRefPhyIn2300;

	// (j.jones 2008-11-12 10:58) - PLID 31912 - added bANSI_SendCASPR
	BOOL bANSI_SendCASPR;

	// (j.jones 2009-03-11 10:36) - PLID 33446 - added default insurance adjustment group/reason codes
	CString ANSI_DefaultInsAdjGroupCode;
	CString ANSI_DefaultInsAdjReasonCode;

	// (j.jones 2009-11-24 15:45) - PLID 36411 - added PriorAuthQualifier
	CString PriorAuthQualifier;

	// (j.jones 2009-12-22 10:21) - PLID 27131 - added UB04UseBox31Date
	long UB04UseBox31Date;

	// (j.jones 2010-03-31 15:15) - PLID 37918 - added ANSI_Hide2430_SVD06_OneCharge
	long ANSI_Hide2430_SVD06_OneCharge;

	// (j.jones 2010-04-19 12:03) - PLID 38265 - added ANSI_StatementDateRange
	long ANSI_StatementDateRange;

	// (j.jones 2010-08-31 09:58) - PLID 15025 - added ANSI_SendTPLNumber
	long ANSI_SendTPLNumber;

	// (j.jones 2012-03-26 14:44) - PLID 49175 - added PRV2000ACode
	long PRV2000ACode;

	// (j.jones 2012-03-26 14:44) - PLID 49175 - added Use2000APRVSegment
	long Use2000APRVSegment;

	// (j.jones 2012-05-14 14:01) - PLID 50338 - added ANSI_SBR04
	// (j.jones 2012-10-04 10:07) - PLID 53018 - removed ANSI_SBR04 and added
	// ANSI_2000B_SBR03, ANSI_2000B_SBR04, and ANSI_2320_SBR04
	long ANSI_2000B_SBR03;
	long ANSI_2000B_SBR04;
	long ANSI_2320_SBR04;

	// (j.jones 2012-05-24 10:24) - PLID 50597 - added InsRelANSI
	long InsRelANSI;

	// (j.jones 2012-09-05 13:57) - PLID 52191 - added Box74Qual
	long Box74Qual;

	// (j.jones 2013-04-11 09:01) - PLID 56166 - added ANSI_HideRefPhyFields
	long ANSI_HideRefPhyFields;

	// (j.jones 2013-06-20 11:39) - PLID 57245 - added OrigRefNo_2300 and OrigRefNo_2330B
	long OrigRefNo_2300;
	long OrigRefNo_2330B;

	// (j.jones 2013-07-17 16:06) - PLID 41823 - added DontBatchSecondary
	long DontBatchSecondary;
	
	// (b.spivey July 8, 2015) PLID 66516 - fill settings for box 12 and 16
	long FillBox16;
	long FillBox12_13; 

	// (j.jones 2016-05-06 8:53) - NX-100514 - added UB04ClaimInfo, an XML struct
	UB04::ClaimInfo ub04ClaimInfo;
};

#endif // !defined(AFX_UB92SETUPINFO_H__290B5758_00BB_476E_8185_85281E87701A__INCLUDED_)

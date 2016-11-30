// UB92SetupInfo.cpp: implementation of the CUB92SetupInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UB92SetupInfo.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

using namespace ADODB;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUB92SetupInfo::CUB92SetupInfo()
{
	Initialize();
}

CUB92SetupInfo::CUB92SetupInfo(long UB92SetupGroupID)
{
	LoadData(UB92SetupGroupID);
}

CUB92SetupInfo::~CUB92SetupInfo()
{

}

void CUB92SetupInfo::Initialize() {
	//this function initializes the data with defaults
	//techincally this shouldn't be needed, as the Load should, in theory, never fail,
	//and this class should never be used without calling Load().

	Name = "";
	Box38 = 0;
	DefBatch = 0;
	DefBatchSecondary = 0;
	DateFmt = 0;
	Box82Setup = 1;
	Box83Setup = 3;
	Box8283Format = 1;
	Box82Num = 1;
	Box83Num = 1;
	Box64Show = 1;
	Box76Show = 1;
	Box85Show = 1;
	GroupRev = 0;
	Box4 = "";
	Box8 = "";
	Box10 = "";
	Box18 = "";
	Box19 = "";
	Box20 = "";
	Box22 = "";
	Box79 = "";
	ShowPhone = 0;
	ShowTotals = 0;
	ShowInsAdd = 0;
	InsAddr50 = 0;
	Box51Default = "";
	Box1Loc = 0;
	PunctuateChargeLines = 0;
	PunctuateDiagCodes = 0;
	Box32 = "";
	Fill42With44 = 0;
	ShowApplies = 0;
	Box6 = 4;
	Box14 = 4;
	Box17 = 4;
	Box32_36 = 4;
	Box45 = 4;
	Box80_81 = 4;
	Box86 = 4;
	Box80Number = 0;
	ShowCompanyAsInsurer = 0;
	Box53Accepted = 1;	// (j.jones 2010-07-27 09:13) - PLID 39853 - changed the default to 1
	Box5ID = 0;
	NM109_IDType = 0;
	// (j.jones 2008-09-09 13:12) - PLID 27450 - changed the default to 2
	ExtraREF_IDType = 2;
	ANSI_EnablePaymentInfo = 1;
	ANSI_AdjustmentLoop = 1;
	ANSI_2300Contract = 0;
	ANSI_2320Allowed = 0;

	UB04Box76Qual = "";
	UB04Box77Setup = 1;
	UB04Box77Qual = "";
	UB04Box77Num = 1;
	UB04Box78Qual = "";

	// (j.jones 2007-03-20 12:34) - PLID 25278 - supported UB04 Box 56 NPI (UB04-only)
	UB04Box56NPI = 1;

	// (j.jones 2007-03-21 10:05) - PLID 25279 - supported UB04 Box 81 Taxonomy Code
	UB04Box81aQual = "B6";
	UB04Box81a = 1;
	UB04Box81bQual = "B6";
	UB04Box81b = -1;
	UB04Box81cQual = "B6";
	UB04Box81c = -1;

	//TES 3/21/2007 - PLID 25295 - Two new checkboxes to fill the "Other Prv ID" in Boxes 51 or 57
	UB04Box51Show = 1;
	UB04Box57Show = 0;

	// (j.jones 2007-03-29 11:50) - PLID 25409 - added ANSI_SecondaryAllowableCalc
	// (j.jones 2009-08-28 17:45) - PLID 32993 - removed this setting
	//ANSI_SecondaryAllowableCalc = 3;

	// (a.walling 2007-06-05 09:19) - PLID 26219 - Fill '001' in line 23 of box 42
	UB04Box42Line23 = 0;

	// (a.walling 2007-08-13 10:02) - PLID 26219 - Custom text for Box42Line23
	Box42Line23 = "";

	// (a.walling 2007-06-05 11:40) - PLID 26223 - Use three lines for address in 80
	UB04Box80UseThree = 0;

	// (a.walling 2007-06-05 13:37) - PLID 26228 - Revenue group to expand, -1 if none
	// (a.walling 2007-08-17 14:08) - PLID 27092 - We use an array now
	//UB04GroupRevExpand = -1;
	arUB04GroupRevExpand.RemoveAll();

	// (j.jones 2007-06-15 10:08) - PLID 26309 - added group/reason code defaults
	ANSI_DefaultRemAdjGroupCode = "OA";
	ANSI_DefaultRemAdjReasonCode = "2";
	
	// (a.walling 2007-06-20 13:25) - PLID 26414 - Whether to use ICD-9-CM Procedure Codes
	UB04UseICD9ProcedureCodes = 0;

	// (j.jones 2007-07-12 08:29) - PLID 26621 - added Box 2 setup
	UB04Box2Setup = -1;

	// (j.jones 2007-07-12 10:35) - PLID 26625 - added new Box 1 setup (UB04 only)
	UB04Box1Setup = 1;

	// (j.jones 2008-04-01 16:17) - PLID 29486 - added ANSI_Hide2330AREF
	ANSI_Hide2330AREF = 0;

	// (j.jones 2008-05-21 10:34) - PLID 30129 - added UB04 Box 66
	UB04Box66 = "";

	// (j.jones 2008-05-23 09:05) - PLID 29939 - added ANSI_SendRefPhyIn2300;
	ANSI_SendRefPhyIn2300 = 0;

	// (j.jones 2008-11-12 10:58) - PLID 31912 - added bANSI_SendCASPR
	bANSI_SendCASPR = TRUE;

	// (j.jones 2009-03-11 10:36) - PLID 33446 - added default insurance adjustment group/reason codes
	ANSI_DefaultInsAdjGroupCode = "CO";
	ANSI_DefaultInsAdjReasonCode = "45";

	// (j.jones 2009-11-24 15:45) - PLID 36411 - added PriorAuthQualifier
	PriorAuthQualifier = "G1";
	
	// (j.jones 2009-12-22 10:21) - PLID 27131 - added UB04UseBox31Date
	UB04UseBox31Date = 0;

	// (j.jones 2010-03-31 15:15) - PLID 37918 - added ANSI_Hide2430_SVD06_OneCharge
	ANSI_Hide2430_SVD06_OneCharge = 0;

	// (j.jones 2010-04-19 12:03) - PLID 38265 - added ANSI_StatementDateRange
	ANSI_StatementDateRange = 1;

	// (j.jones 2010-08-31 09:58) - PLID 15025 - added ANSI_SendTPLNumber
	ANSI_SendTPLNumber = 1;

	// (j.jones 2012-03-26 14:44) - PLID 49175 - added PRV2000ACode
	PRV2000ACode = 0;

	// (j.jones 2012-03-26 14:44) - PLID 49175 - added Use2000APRVSegment
	Use2000APRVSegment = 1;

	// (j.jones 2012-05-14 14:01) - PLID 50338 - added ANSI_SBR04
	// (j.jones 2012-10-04 10:07) - PLID 53018 - removed ANSI_SBR04 and added
	// ANSI_2000B_SBR03, ANSI_2000B_SBR04, and ANSI_2320_SBR04
	ANSI_2000B_SBR03 = 0;
	ANSI_2000B_SBR04 = 1;
	ANSI_2320_SBR04 = 1;

	// (j.jones 2012-05-24 10:24) - PLID 50597 - added InsRelANSI
	InsRelANSI = 1;

	// (j.jones 2012-09-05 13:57) - PLID 52191 - added Box74Qual
	Box74Qual = 0;

	// (j.jones 2013-04-11 09:01) - PLID 56166 - added ANSI_HideRefPhyFields
	ANSI_HideRefPhyFields = 1;

	// (j.jones 2013-06-20 11:39) - PLID 57245 - added OrigRefNo_2300 and OrigRefNo_2330B
	OrigRefNo_2300 = 1;
	OrigRefNo_2330B = 1;

	// (j.jones 2013-07-17 16:06) - PLID 41823 - added DontBatchSecondary
	DontBatchSecondary = 0;

	// (b.spivey July 8, 2015) PLID 66516 - fill settings for box 12 and 16
	FillBox12_13 = 1;
	FillBox16 = 1;

	// (j.jones 2016-05-06 8:53) - NX-100514 - added UB04ClaimInfo, an XML struct
	ub04ClaimInfo = UB04::ClaimInfo();
}

void CUB92SetupInfo::LoadData(long UB92SetupGroupID) {

	//initialize now - in case a variant fails, we don't need to put in the default
	Initialize();

	// (j.jones 2010-09-23 15:52) - PLID 40653 - the group & reason codes are IDs now, but we load the actual code here,
	// which means we need to join tables
	_RecordsetPtr rs = CreateParamRecordset("SELECT UB92SetupT.*, "
		"AdjustmentGroupCodesT.Code AS ANSI_DefaultAdjGroupCode, "
		"AdjustmentReasonCodesT.Code AS ANSI_DefaultAdjReasonCode, "
		"AdjustmentInsGroupCodesT.Code AS ANSI_DefaultInsAdjGroupCode, "
		"AdjustmentInsReasonCodesT.Code AS ANSI_DefaultInsAdjReasonCode "
		"FROM UB92SetupT "
		"LEFT JOIN AdjustmentCodesT AS AdjustmentGroupCodesT ON UB92SetupT.ANSI_DefaultAdjGroupCodeID = AdjustmentGroupCodesT.ID "
		"LEFT JOIN AdjustmentCodesT AS AdjustmentReasonCodesT ON UB92SetupT.ANSI_DefaultAdjReasonCodeID = AdjustmentReasonCodesT.ID "
		"LEFT JOIN AdjustmentCodesT AS AdjustmentInsGroupCodesT ON UB92SetupT.ANSI_DefaultInsAdjGroupCodeID = AdjustmentInsGroupCodesT.ID "
		"LEFT JOIN AdjustmentCodesT AS AdjustmentInsReasonCodesT ON UB92SetupT.ANSI_DefaultInsAdjReasonCodeID = AdjustmentInsReasonCodesT.ID "
		"WHERE UB92SetupT.ID = {INT} \r\n"
		"SELECT UBCategoryID FROM UB04MultiGroupRevExpandT WHERE UB92SetupID = {INT}",
		UB92SetupGroupID, UB92SetupGroupID);

	if(rs->eof) {
		rs->Close();
		//this should never happen, but just incase, we have already called Initialize(), so we're safe.
		return;
	}

	_variant_t var;

	//if the variant fails, don't worry, we already called Initialize()
	var = rs->Fields->Item["Name"]->Value;
	if(var.vt == VT_BSTR)
		Name = CString(var.bstrVal);

	var = rs->Fields->Item["Box38"]->Value;
	if(var.vt == VT_I4)
		Box38 = var.lVal;

	var = rs->Fields->Item["DefBatch"]->Value;
	if(var.vt == VT_I4)
		DefBatch = var.lVal;

	var = rs->Fields->Item["DefBatchSecondary"]->Value;
	if(var.vt == VT_I4)
		DefBatchSecondary = var.lVal;

	var = rs->Fields->Item["DateFmt"]->Value;
	if(var.vt == VT_I4)
		DateFmt = var.lVal;

	var = rs->Fields->Item["Box82Setup"]->Value;
	if(var.vt == VT_I4)
		Box82Setup = var.lVal;

	var = rs->Fields->Item["Box83Setup"]->Value;
	if(var.vt == VT_I4)
		Box83Setup = var.lVal;

	var = rs->Fields->Item["Box8283Format"]->Value;
	if(var.vt == VT_I4)
		Box8283Format = var.lVal;

	var = rs->Fields->Item["Box82Num"]->Value;
	if(var.vt == VT_I4)
		Box82Num = var.lVal;

	var = rs->Fields->Item["Box83Num"]->Value;
	if(var.vt == VT_I4)
		Box83Num = var.lVal;

	var = rs->Fields->Item["Box64Show"]->Value;
	if(var.vt == VT_I4)
		Box64Show = var.lVal;

	var = rs->Fields->Item["Box76Show"]->Value;
	if(var.vt == VT_I4)
		Box76Show = var.lVal;

	var = rs->Fields->Item["Box85Show"]->Value;
	if(var.vt == VT_I4)
		Box85Show = var.lVal;

	var = rs->Fields->Item["GroupRev"]->Value;
	if(var.vt == VT_I4)
		GroupRev = var.lVal;

	var = rs->Fields->Item["Box4"]->Value;
	if(var.vt == VT_BSTR)
		Box4 = CString(var.bstrVal);

	var = rs->Fields->Item["Box8"]->Value;
	if(var.vt == VT_BSTR)
		Box8 = CString(var.bstrVal);

	var = rs->Fields->Item["Box10"]->Value;
	if(var.vt == VT_BSTR)
		Box10 = CString(var.bstrVal);

	var = rs->Fields->Item["Box18"]->Value;
	if(var.vt == VT_BSTR)
		Box18 = CString(var.bstrVal);

	var = rs->Fields->Item["Box19"]->Value;
	if(var.vt == VT_BSTR)
		Box19 = CString(var.bstrVal);

	var = rs->Fields->Item["Box20"]->Value;
	if(var.vt == VT_BSTR)
		Box20 = CString(var.bstrVal);

	var = rs->Fields->Item["Box22"]->Value;
	if(var.vt == VT_BSTR)
		Box22 = CString(var.bstrVal);

	var = rs->Fields->Item["Box79"]->Value;
	if(var.vt == VT_BSTR)
		Box79 = CString(var.bstrVal);

	var = rs->Fields->Item["ShowPhone"]->Value;
	if(var.vt == VT_I4)
		ShowPhone = var.lVal;

	var = rs->Fields->Item["ShowTotals"]->Value;
	if(var.vt == VT_I4)
		ShowTotals = var.lVal;

	var = rs->Fields->Item["ShowInsAdd"]->Value;
	if(var.vt == VT_I4)
		ShowInsAdd = var.lVal;

	var = rs->Fields->Item["InsAddr50"]->Value;
	if(var.vt == VT_I4)
		InsAddr50 = var.lVal;

	var = rs->Fields->Item["Box51Default"]->Value;
	if(var.vt == VT_BSTR)
		Box51Default = CString(var.bstrVal);

	var = rs->Fields->Item["Box1Loc"]->Value;
	if(var.vt == VT_I4)
		Box1Loc = var.lVal;

	var = rs->Fields->Item["PunctuateChargeLines"]->Value;
	if(var.vt == VT_I4)
		PunctuateChargeLines = var.lVal;

	var = rs->Fields->Item["PunctuateDiagCodes"]->Value;
	if(var.vt == VT_I4)
		PunctuateDiagCodes = var.lVal;

	var = rs->Fields->Item["Box32"]->Value;
	if(var.vt == VT_BSTR)
		Box32 = CString(var.bstrVal);

	var = rs->Fields->Item["Fill42With44"]->Value;
	if(var.vt == VT_I4)
		Fill42With44 = var.lVal;

	var = rs->Fields->Item["ShowApplies"]->Value;
	if(var.vt == VT_I4)
		ShowApplies = var.lVal;

	var = rs->Fields->Item["Box6"]->Value;
	if(var.vt == VT_I4)
		Box6 = var.lVal;

	var = rs->Fields->Item["Box14"]->Value;
	if(var.vt == VT_I4)
		Box14 = var.lVal;

	var = rs->Fields->Item["Box17"]->Value;
	if(var.vt == VT_I4)	
		Box17 = var.lVal;

	var = rs->Fields->Item["Box32_36"]->Value;
	if(var.vt == VT_I4)
		Box32_36 = var.lVal;

	var = rs->Fields->Item["Box45"]->Value;
	if(var.vt == VT_I4)
		Box45 = var.lVal;

	var = rs->Fields->Item["Box80_81"]->Value;
	if(var.vt == VT_I4)
		Box80_81 = var.lVal;

	var = rs->Fields->Item["Box86"]->Value;
	if(var.vt == VT_I4)
		Box86 = var.lVal;

	var = rs->Fields->Item["Box80Number"]->Value;
	if(var.vt == VT_I4)
		Box80Number = var.lVal;

	var = rs->Fields->Item["ShowCompanyAsInsurer"]->Value;
	if(var.vt == VT_I4)
		ShowCompanyAsInsurer = var.lVal;

	var = rs->Fields->Item["Box53Accepted"]->Value;
	if(var.vt == VT_I4)
		Box53Accepted = var.lVal;

	var = rs->Fields->Item["Box5ID"]->Value;
	if(var.vt == VT_I4)
		Box5ID = var.lVal;

	// (j.jones 2006-11-14 08:46) - PLID 23446 - Added options to control
	// ANSI NPI behavior
	NM109_IDType = AdoFldLong(rs, "NM109_IDType", 0);
	// (j.jones 2008-09-09 13:12) - PLID 27450 - changed the default to 2
	ExtraREF_IDType = AdoFldLong(rs, "ExtraREF_IDType", 2);

	// (j.jones 2006-11-27 17:34) - PLID 23652 - added secondary ANSI controls
	ANSI_EnablePaymentInfo = AdoFldLong(rs, "ANSI_EnablePaymentInfo", 1);
	ANSI_AdjustmentLoop = AdoFldLong(rs, "ANSI_AdjustmentLoop", 1);
	ANSI_2300Contract = AdoFldLong(rs, "ANSI_2300Contract", 0);
	ANSI_2320Allowed = AdoFldLong(rs, "ANSI_2320Allowed", 0);

	//TES 3/19/2007 - PLID 25235 - UB04-specific fields.
	UB04Box76Qual = AdoFldString(rs, "UB04Box76Qual","");
	UB04Box77Setup = AdoFldLong(rs, "UB04Box77Setup", 1);
	UB04Box77Qual = AdoFldString(rs, "UB04Box77Qual","");
	UB04Box77Num = AdoFldLong(rs, "UB04Box77Num", 1);
	UB04Box78Qual = AdoFldString(rs, "UB04Box78Qual","");

	// (j.jones 2007-03-20 12:34) - PLID 25278 - supported UB04 Box 56 NPI (UB04-only)
	UB04Box56NPI = AdoFldLong(rs, "UB04Box56NPI", 1);

	// (j.jones 2007-03-21 10:05) - PLID 25279 - supported UB04 Box 81 Taxonomy Code
	UB04Box81aQual = AdoFldString(rs, "UB04Box81aQual", "B6");
	UB04Box81a = AdoFldLong(rs, "UB04Box81a", 1);
	UB04Box81bQual = AdoFldString(rs, "UB04Box81bQual", "B6");
	UB04Box81b = AdoFldLong(rs, "UB04Box81b", -1);
	UB04Box81cQual = AdoFldString(rs, "UB04Box81cQual", "B6");
	UB04Box81c = AdoFldLong(rs, "UB04Box81c", -1);

	// (j.jones 2013-04-05 15:56) - PLID 40960 - Referring Physicians don't have taxonomy codes anymore,
	// revert to "do not fill" if the value is 3
	if(UB04Box81a == 3) {
		UB04Box81a = -1;
	}
	if(UB04Box81b == 3) {
		UB04Box81b = -1;
	}
	if(UB04Box81c == 3) {
		UB04Box81c = -1;
	}

	//TES 3/21/2007 - PLID 25295 - Two new checkboxes to fill the "Other Prv ID" in Boxes 51 or 57
	UB04Box51Show = AdoFldLong(rs, "UB04Box51Show", 1);
	UB04Box57Show = AdoFldLong(rs, "UB04Box57Show", 0);

	// (j.jones 2007-03-29 11:51) - PLID 25409 - added ANSI_SecondaryAllowableCalc
	// (j.jones 2009-08-28 17:45) - PLID 32993 - removed this setting
	//ANSI_SecondaryAllowableCalc = AdoFldLong(rs, "ANSI_SecondaryAllowableCalc", 3);

	// (a.walling 2007-06-05 09:21) - PLID 26219 - Fill box 42 line 23 with 001
	UB04Box42Line23 = AdoFldLong(rs, "UB04Box42Line23", 0);

	// (a.walling 2007-08-13 10:03) - PLID 26219
	Box42Line23 = AdoFldString(rs, "Box42Line23", "");

	// (a.walling 2007-06-05 11:41) - PLID 26223 - Use 3 lines for box 80 address
	UB04Box80UseThree = AdoFldLong(rs, "UB04Box80UseThree", 0);

	// (j.jones 2007-06-15 10:08) - PLID 26309 - added group/reason code defaults
	// (j.jones 2010-09-23 15:52) - PLID 40653 - the group & reason codes are IDs now, but we load the actual code here
	ANSI_DefaultRemAdjGroupCode = AdoFldString(rs, "ANSI_DefaultAdjGroupCode", "OA");
	ANSI_DefaultRemAdjReasonCode = AdoFldString(rs, "ANSI_DefaultAdjReasonCode", "2");

	// (a.walling 2007-06-20 13:26) - PLID 26414 - Load the ICD-9-CM Procedure Code option
	UB04UseICD9ProcedureCodes = AdoFldLong(rs, "UB04UseICD9ProcedureCodes", FALSE);

	// (j.jones 2007-07-12 08:30) - PLID 26621 - added Box 2 setup
	UB04Box2Setup = AdoFldLong(rs, "UB04Box2Setup", -1);

	// (j.jones 2007-07-12 10:41) - PLID 26625 - added new Box 1 setup (UB04 only)
	UB04Box1Setup = AdoFldLong(rs, "UB04Box1Setup", 1);

	// (j.jones 2008-04-01 16:17) - PLID 29486 - added ANSI_Hide2330AREF
	ANSI_Hide2330AREF = AdoFldLong(rs, "ANSI_Hide2330AREF", 0);

	// (j.jones 2008-05-21 10:34) - PLID 30129 - added UB04 Box 66
	UB04Box66 = AdoFldString(rs, "UB04Box66", "");;

	// (j.jones 2008-05-23 09:05) - PLID 29939 - added ANSI_SendRefPhyIn2300;
	ANSI_SendRefPhyIn2300 = AdoFldLong(rs, "ANSI_SendRefPhyIn2300", 0);

	// (j.jones 2008-11-12 10:58) - PLID 31912 - added bANSI_SendCASPR
	bANSI_SendCASPR = AdoFldBool(rs, "ANSI_SendCASPR", TRUE);

	// (j.jones 2009-03-11 10:36) - PLID 33446 - added default insurance adjustment group/reason codes
	// (j.jones 2010-09-23 15:52) - PLID 40653 - the group & reason codes are IDs now, but we load the actual code here
	ANSI_DefaultInsAdjGroupCode = AdoFldString(rs, "ANSI_DefaultInsAdjGroupCode", "CO");
	ANSI_DefaultInsAdjReasonCode = AdoFldString(rs, "ANSI_DefaultInsAdjReasonCode", "45");

	// (j.jones 2009-11-24 15:45) - PLID 36411 - added PriorAuthQualifier
	PriorAuthQualifier = AdoFldString(rs, "PriorAuthQualifier", "G1");

	// (j.jones 2009-12-22 10:21) - PLID 27131 - added UB04UseBox31Date
	UB04UseBox31Date = AdoFldLong(rs, "UB04UseBox31Date", 0);

	// (j.jones 2010-03-31 15:15) - PLID 37918 - added ANSI_Hide2430_SVD06_OneCharge
	ANSI_Hide2430_SVD06_OneCharge = AdoFldLong(rs, "ANSI_Hide2430_SVD06_OneCharge", 0);

	// (j.jones 2010-04-19 12:03) - PLID 38265 - added ANSI_StatementDateRange
	ANSI_StatementDateRange = AdoFldLong(rs, "ANSI_StatementDateRange", 1);

	// (j.jones 2010-08-31 09:58) - PLID 15025 - added ANSI_SendTPLNumber
	ANSI_SendTPLNumber = AdoFldLong(rs, "ANSI_SendTPLNumber", 1);

	// (j.jones 2012-03-26 14:44) - PLID 49175 - added PRV2000ACode
	PRV2000ACode = AdoFldLong(rs, "PRV2000ACode", 0);

	// (j.jones 2012-03-26 14:44) - PLID 49175 - added Use2000APRVSegment
	Use2000APRVSegment = AdoFldLong(rs, "Use2000APRVSegment", 1);

	// (j.jones 2012-05-14 14:01) - PLID 50338 - added ANSI_SBR04
	// (j.jones 2012-10-04 10:07) - PLID 53018 - removed ANSI_SBR04 and added
	// ANSI_2000B_SBR03, ANSI_2000B_SBR04, and ANSI_2320_SBR04
	ANSI_2000B_SBR03 = AdoFldLong(rs, "ANSI_2000B_SBR03", 0);
	ANSI_2000B_SBR04 = AdoFldLong(rs, "ANSI_2000B_SBR04", 1);
	ANSI_2320_SBR04 = AdoFldLong(rs, "ANSI_2320_SBR04", 1);

	// (j.jones 2012-05-24 10:24) - PLID 50597 - added InsRelANSI
	InsRelANSI = AdoFldLong(rs, "InsRelANSI", 1);

	// (j.jones 2012-09-05 13:57) - PLID 52191 - added Box74Qual
	Box74Qual = AdoFldLong(rs, "Box74Qual", 0);

	// (j.jones 2013-04-11 09:01) - PLID 56166 - added ANSI_HideRefPhyFields
	ANSI_HideRefPhyFields = AdoFldLong(rs, "ANSI_HideRefPhyFields", 1);

	// (j.jones 2013-06-20 11:39) - PLID 57245 - added OrigRefNo_2300 and OrigRefNo_2330B
	OrigRefNo_2300 = AdoFldLong(rs, "OrigRefNo_2300", 1);
	OrigRefNo_2330B = AdoFldLong(rs, "OrigRefNo_2330B", 1);

	// (j.jones 2013-07-17 16:06) - PLID 41823 - added DontBatchSecondary
	DontBatchSecondary = AdoFldLong(rs, "DontBatchSecondary", 0);

	// (b.spivey July 8, 2015) PLID 66516 - fill settings for box 12 and 16
	FillBox12_13 = AdoFldBool(rs, "FillBox12_13", 1);
	FillBox16 = AdoFldBool(rs, "FillBox16", 1);

	// (j.jones 2016-05-06 8:53) - NX-100514 - added UB04ClaimInfo, an XML struct
	ub04ClaimInfo = UB04::ClaimInfo::FromXml(AdoFldString(rs, "UB04ClaimInfo", ""));

	// (a.walling 2007-06-05 15:00) - PLID 26228 - load the expansion group
	// (a.walling 2007-08-17 14:03) - PLID 27092 - make that... groupS
	rs = rs->NextRecordset(NULL);
	while (!rs->eof) {
		arUB04GroupRevExpand.Add(AdoFldLong(rs, "UBCategoryID"));
		rs->MoveNext();
	}

	rs->Close();
}
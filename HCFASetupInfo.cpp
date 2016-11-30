// HCFASetupInfo.cpp: implementation of the CHCFASetupInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "HCFASetupInfo.h"
#include "GlobalFinancialUtils.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

using namespace ADODB;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHCFASetupInfo::CHCFASetupInfo()
{
	Initialize();
}

CHCFASetupInfo::CHCFASetupInfo(long HCFASetupGroupID)
{
	LoadData(HCFASetupGroupID);
}

CHCFASetupInfo::~CHCFASetupInfo()
{

}

void CHCFASetupInfo::Initialize() {
	//this function initializes the data with defaults
	//techincally this shouldn't be needed, as the Load should, in theory, never fail,
	//and this class should never be used without calling Load().

	Name = "";
	Address = 1;
	Box3 = 2;
	Box9b = 2;
	Box33Setup = 1;
	Box17a = 3;
	Box33 = 2;
	Box11 = "";
	Box11Rule = 2;
	Box11a = 2;
	Box25 = 2;
	Box31 = 2;
	Box31Show = 1;
	Box33GRP = "";
	ExportFA8 = 0;  //obsolete, but still loaded for now
	GRPAsEMC = 0;
	Box11D = 0;
	DefBatch = 0;
	DefBatchSecondary = 0;
	Box33Order = 1;
	ShowPays = 1;
	IgnoreAdjustments = 0;
	Box3E = 1;
	Box11aE = 1;
	Box9bE = 1;
	Box24To = 2;
	Box24From = 2;
	Box16 = 2;
	Box18 = 2;
	Box14 = 2;
	Box15 = 2;
	Box12 = 2;
	Box3Wide = 1;
	Box9bWide = 1;
	Box11aWide = 1;
	Box31Wide = 0;
	Box24FWide = 1;
	Box24TWide = 1;
	Box16Wide = 1;
	Box18Wide = 1;
	Box14Wide = 1;
	Box15Wide = 1;
	Box12Wide = 0;
	HideBox4 = 0;
	HideBox7 = 0;
	HideBox9 = 0;
	HideBox11 = 0;
	UseEmp = 0;
	Box33Num = 0;	
	HidePhoneBox33 = 0;
	// (j.jones 2010-07-23 10:44) - PLID 39794 - this now defaults to 3
	Box12Accepted = 3;
	// (j.jones 2010-07-22 11:48) - PLID 39779 - this defaults to 3
	Box13Accepted = 3;
	DefaultPriorAuthNum = "";
	DocAddress = 1;
	ShowDiagDecimals = 0;
	ShowICD9Desc = 0;
	Box24C = 0;
	// (j.jones 2007-03-26 15:45) - PLID 24538 - I removed this, but by client demand I reinstated it
	LeftAlignInsAddress = 0;
	Box19RefPhyID = 0;
	Box19Override = 0;
	Box19 = "";
	HideBox30 = 0;
	ShowSecInsAdd = 0;
	Box17Order = 1;
	ANSI_SendBox19 = 0;
	ANSI_SendRefPhyIn2300 = 0;
	ANSI_UseAnesthMinutesAsQty = 0;
	ShowAnesthesiaTimes = 0;
	Box12UseDate = 0;
	Box31UseDate = 0;
	strCorrespondenceNote = "";
	ShowRespName = 0;
	// (j.jones 2009-08-03 14:58) - PLID 33827 - removed Use2310B
	//Use2310B = FALSE;
	Use2310BPRVSegment = FALSE;
	Use2000APRVSegment = 1;
	ShowSecPINNumber = 0;
	ShowSec24JNumber = 0;
	ExcludeAdjFromBal = 0;
	UseBox23InBox17a = 0;
	UseOverrideInBox31 = 0;
	PrintPinBox1a = 0;
	Use1aIn9a = 0;
	Box17aQual = "";
	Box33bQual = "";
	ShowAnesthMinutesOnly = 0;
	Use1aIn9aAlways = 0;
	SecondaryFillBox11 = 0;
	NM109_IDType = 0;
	// (j.jones 2008-09-09 13:12) - PLID 27450 - changed the default to 2
	ExtraREF_IDType = 2;
	LocationNPIUsage = 1;
	ANSI_EnablePaymentInfo = 1;
	ANSI_AdjustmentLoop = 1;
	ANSI_2300Contract = 0;
	ANSI_2320Allowed = 0;
	ANSI_2320Approved = 0;
	ANSI_2400Contract = 0;
	ANSI_2400Allowed = 0;
	// (j.jones 2007-02-21 12:24) - PLID 24776 - added QualifierSpace
	QualifierSpace = 0;
	// (j.jones 2007-03-28 13:32) - PLID 25392 - added ShowWhichCodesCommas
	ShowWhichCodesCommas = 0;
	// (j.jones 2007-03-29 11:50) - PLID 25409 - added ANSI_SecondaryAllowableCalc
	// (j.jones 2009-08-28 17:45) - PLID 32993 - removed this setting
	//ANSI_SecondaryAllowableCalc = 3;
	// (j.jones 2007-04-09 16:15) - PLID 25537 - added HideBox11a/b/c
	HideBox11a_Birthdate = 0;
	HideBox11a_Gender = 0;
	HideBox11b = 0;
	HideBox11c = 0;
	// (j.jones 2007-04-09 17:47) - PLID 25539 - added Hide24ATo
	Hide24ATo = 0;
	// (j.jones 2007-05-11 11:39) - PLID 25932 - revised Box 32 hide options
	HideBox32NameAdd = 0;
	HideBox32NPIID = 0;

	// (j.jones 2007-06-15 10:08) - PLID 26309 - added group/reason code defaults
	ANSI_DefaultRemAdjGroupCode = "OA";
	ANSI_DefaultRemAdjReasonCode = "2";

	// (j.jones 2007-07-12 14:44) - PLID 26636 - added ability to hide 24J NPI
	HideBox24JNPI = 0;

	// (j.jones 2008-02-06 17:15) - PLID 28843 - added SecondaryInsCodeUsage and SecondaryInsCode
	bSecondaryInsCodeUsage = 0;
	strSecondaryInsCode = "";

	// (j.jones 2008-02-25 09:06) - PLID 29077 - added ANSI_SecondaryApprovedCalc
	ANSI_SecondaryApprovedCalc = 3;

	// (j.jones 2008-04-01 16:17) - PLID 29486 - added ANSI_Hide2330AREF
	ANSI_Hide2330AREF = 0;

	// (j.jones 2008-06-10 14:50) - PLID 25834 - added ValidateRefPhy
	ValidateRefPhy = 0;

	// (j.jones 2008-06-18 09:05) - PLID 30403 - added DoNotFillBox29
	DoNotFillBox29 = 0;

	// (j.jones 2008-06-23 12:23) - PLID 30434 - added Eligibility_2100C_REF_Option
	Eligibility_2100C_REF_Option = 1;

	// (j.jones 2010-05-25 15:43) - PLID 38868 - added Eligibility_2100D_REF
	Eligibility_2100D_REF = 2;

	// (j.jones 2008-10-06 10:17) - PLID 31580 - added ANSI_SendCorrespSegment
	ANSI_SendCorrespSegment = 1;

	// (j.jones 2008-10-06 13:03) - PLID 31578 - added ANSI_SendBox19Segment
	ANSI_SendBox19Segment = 1;

	// (j.jones 2008-11-12 10:58) - PLID 31912 - added bANSI_SendCASPR
	bANSI_SendCASPR = TRUE;

	// (j.jones 2009-01-06 09:09) - PLID 32614 - added ability to hide 17b
	HideBox17b = 0;

	// (j.jones 2009-03-06 11:18) - PLID 33354 - added PullCurrentAccInfo
	PullCurrentAccInfo = 1;

	// (j.jones 2009-03-11 10:36) - PLID 33446 - added default insurance adjustment group/reason codes
	ANSI_DefaultInsAdjGroupCode = "CO";
	ANSI_DefaultInsAdjReasonCode = "45";

	// (j.jones 2009-11-24 15:45) - PLID 36411 - added PriorAuthQualifier
	PriorAuthQualifier = "G1";

	// (j.jones 2010-03-31 15:15) - PLID 37918 - added ANSI_Hide2430_SVD06_OneCharge
	ANSI_Hide2430_SVD06_OneCharge = 0;

	// (j.jones 2010-08-31 09:58) - PLID 15025 - added ANSI_SendTPLNumber
	ANSI_SendTPLNumber = 1;

	// (j.jones 2010-08-31 10:37) - PLID 40303 - added TPLIn9a
	TPLIn9a = 0;

	// (j.jones 2011-06-15 12:51) - PLID 42181 - added EligNPIUsage
	EligNPIUsage = 1;

	// (j.jones 2012-03-23 15:33) - PLID 49176 - added PRV2000ACode
	PRV2000ACode = 0;

	// (j.jones 2012-05-14 14:01) - PLID 50338 - added ANSI_SBR04
	// (j.jones 2012-10-04 10:07) - PLID 53018 - removed ANSI_SBR04 and added
	// ANSI_2000B_SBR03, ANSI_2000B_SBR04, and ANSI_2320_SBR04
	ANSI_2000B_SBR03 = 0;
	ANSI_2000B_SBR04 = 1;
	ANSI_2320_SBR04 = 1;

	// (j.jones 2013-04-11 09:01) - PLID 56166 - added ANSI_HideRefPhyFields
	ANSI_HideRefPhyFields = 1;

	// (j.jones 2013-06-10 16:26) - PLID 56255 - added UseRefPhyGroupNPI
	UseRefPhyGroupNPI = 0;

	// (j.jones 2013-06-20 11:39) - PLID 57245 - added OrigRefNo_2300 and OrigRefNo_2330B
	OrigRefNo_2300 = 1;
	OrigRefNo_2330B = 1;

	// (j.jones 2013-07-08 15:08) - PLID 57469 - added EligTitleInLast
	EligTitleInLast = 0;

	// (j.jones 2013-07-17 16:06) - PLID 41823 - added DontBatchSecondary
	DontBatchSecondary = 0;

	// (j.jones 2014-01-22 09:49) - PLID 60034 - added OrderingProvider
	OrderingProvider = 0;

	// (j.jones 2014-04-23 13:47) - PLID 61840 - added Hide 2310A setting
	Hide2310A = hide2310A_Never;
}

void CHCFASetupInfo::LoadData(long HCFASetupGroupID) {

	//initialize now - in case a variant fails, we don't need to put in the default
	Initialize();

	// (j.jones 2010-09-23 15:52) - PLID 40653 - the group & reason codes are IDs now, but we load the actual code here,
	// which means we need to join tables
	_RecordsetPtr rs = CreateParamRecordset("SELECT HCFASetupT.*, "
		"AdjustmentGroupCodesT.Code AS ANSI_DefaultAdjGroupCode, "
		"AdjustmentReasonCodesT.Code AS ANSI_DefaultAdjReasonCode, "
		"AdjustmentInsGroupCodesT.Code AS ANSI_DefaultInsAdjGroupCode, "
		"AdjustmentInsReasonCodesT.Code AS ANSI_DefaultInsAdjReasonCode "
		"FROM HCFASetupT "
		"LEFT JOIN AdjustmentCodesT AS AdjustmentGroupCodesT ON HCFASetupT.ANSI_DefaultAdjGroupCodeID = AdjustmentGroupCodesT.ID "
		"LEFT JOIN AdjustmentCodesT AS AdjustmentReasonCodesT ON HCFASetupT.ANSI_DefaultAdjReasonCodeID = AdjustmentReasonCodesT.ID "
		"LEFT JOIN AdjustmentCodesT AS AdjustmentInsGroupCodesT ON HCFASetupT.ANSI_DefaultInsAdjGroupCodeID = AdjustmentInsGroupCodesT.ID "
		"LEFT JOIN AdjustmentCodesT AS AdjustmentInsReasonCodesT ON HCFASetupT.ANSI_DefaultInsAdjReasonCodeID = AdjustmentInsReasonCodesT.ID "
		"WHERE HCFASetupT.ID = {INT}",HCFASetupGroupID);

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

	var = rs->Fields->Item["Address"]->Value;
	if(var.vt == VT_I4)
		Address = var.lVal;

	var = rs->Fields->Item["Box3"]->Value;
	if(var.vt == VT_I4)
		Box3 = var.lVal;

	var = rs->Fields->Item["Box9b"]->Value;
	if(var.vt == VT_I4)
		Box9b = var.lVal;

	var = rs->Fields->Item["Box33Setup"]->Value;
	if(var.vt == VT_I4)
		Box33Setup = var.lVal;

	var = rs->Fields->Item["Box17a"]->Value;
	if(var.vt == VT_I4)
		Box17a = var.lVal;

	var = rs->Fields->Item["Box33"]->Value;
	if(var.vt == VT_I4)
		Box33 = var.lVal;
	
	var = rs->Fields->Item["Box11"]->Value;
	if(var.vt == VT_BSTR)
		Box11 = CString(var.bstrVal);

	var = rs->Fields->Item["Box11Rule"]->Value;
	if(var.vt == VT_I4)
		Box11Rule = var.lVal;

	var = rs->Fields->Item["Box11a"]->Value;
	if(var.vt == VT_I4)
		Box11a = var.lVal;

	var = rs->Fields->Item["Box25"]->Value;
	if(var.vt == VT_I4)
		Box25 = var.lVal;

	var = rs->Fields->Item["Box31"]->Value;
	if(var.vt == VT_I4)
		Box31 = var.lVal;

	var = rs->Fields->Item["Box31Show"]->Value;
	if(var.vt == VT_I4)
		Box31Show = var.lVal;

	var = rs->Fields->Item["Box33GRP"]->Value;
	if(var.vt == VT_BSTR)
		Box33GRP = CString(var.bstrVal);

	//obsolete, but still loaded for now
	var = rs->Fields->Item["ExportFA8"]->Value;
	if(var.vt == VT_I4)
		ExportFA8 = var.lVal;

	var = rs->Fields->Item["GRPAsEMC"]->Value;
	if(var.vt == VT_I4)
		GRPAsEMC = var.lVal;

	var = rs->Fields->Item["Box11D"]->Value;
	if(var.vt == VT_I4)
		Box11D = var.lVal;

	var = rs->Fields->Item["DefBatch"]->Value;
	if(var.vt == VT_I4)
		DefBatch = var.lVal;

	var = rs->Fields->Item["DefBatchSecondary"]->Value;
	if(var.vt == VT_I4)
		DefBatchSecondary = var.lVal;

	var = rs->Fields->Item["Box33Order"]->Value;
	if(var.vt == VT_I4)
		Box33Order = var.lVal;

	var = rs->Fields->Item["Box17Order"]->Value;
	if(var.vt == VT_I4)
		Box17Order = var.lVal;

	var = rs->Fields->Item["ShowPays"]->Value;
	if(var.vt == VT_I4)
		ShowPays = var.lVal;

	var = rs->Fields->Item["IgnoreAdjustments"]->Value;
	if(var.vt == VT_I4)
		IgnoreAdjustments = var.lVal;

	var = rs->Fields->Item["Box3E"]->Value;
	if(var.vt == VT_I4)
		Box3E = var.lVal;

	var = rs->Fields->Item["Box11aE"]->Value;
	if(var.vt == VT_I4)
		Box11aE = var.lVal;

	var = rs->Fields->Item["Box9bE"]->Value;
	if(var.vt == VT_I4)
		Box9bE = var.lVal;

	var = rs->Fields->Item["Box24To"]->Value;
	if(var.vt == VT_I4)
		Box24To = var.lVal;

	var = rs->Fields->Item["Box24From"]->Value;
	if(var.vt == VT_I4)
		Box24From = var.lVal;

	var = rs->Fields->Item["Box16"]->Value;
	if(var.vt == VT_I4)
		Box16 = var.lVal;

	var = rs->Fields->Item["Box18"]->Value;
	if(var.vt == VT_I4)
		Box18 = var.lVal;

	var = rs->Fields->Item["Box14"]->Value;
	if(var.vt == VT_I4)
		Box14 = var.lVal;

	var = rs->Fields->Item["Box15"]->Value;
	if(var.vt == VT_I4)
		Box15 = var.lVal;

	var = rs->Fields->Item["Box12"]->Value;
	if(var.vt == VT_I4)
		Box12 = var.lVal;

	var = rs->Fields->Item["Box3Wide"]->Value;
	if(var.vt == VT_I4)
		Box3Wide = var.lVal;

	var = rs->Fields->Item["Box9bWide"]->Value;
	if(var.vt == VT_I4)
		Box9bWide = var.lVal;

	var = rs->Fields->Item["Box11aWide"]->Value;
	if(var.vt == VT_I4)
		Box11aWide = var.lVal;

	var = rs->Fields->Item["Box31Wide"]->Value;
	if(var.vt == VT_I4)
		Box31Wide = var.lVal;

	var = rs->Fields->Item["Box24FWide"]->Value;
	if(var.vt == VT_I4)
		Box24FWide = var.lVal;

	var = rs->Fields->Item["Box24TWide"]->Value;
	if(var.vt == VT_I4)
		Box24TWide = var.lVal;

	var = rs->Fields->Item["Box16Wide"]->Value;
	if(var.vt == VT_I4)
		Box16Wide = var.lVal;

	var = rs->Fields->Item["Box18Wide"]->Value;
	if(var.vt == VT_I4)
		Box18Wide = var.lVal;

	var = rs->Fields->Item["Box14Wide"]->Value;
	if(var.vt == VT_I4)
		Box14Wide = var.lVal;

	var = rs->Fields->Item["Box15Wide"]->Value;
	if(var.vt == VT_I4)
		Box15Wide = var.lVal;

	var = rs->Fields->Item["Box12Wide"]->Value;
	if(var.vt == VT_I4)
		Box12Wide = var.lVal;

	var = rs->Fields->Item["HideBox4"]->Value;
	if(var.vt == VT_I4)
		HideBox4 = var.lVal;

	var = rs->Fields->Item["HideBox7"]->Value;
	if(var.vt == VT_I4)
		HideBox7 = var.lVal;

	var = rs->Fields->Item["HideBox9"]->Value;
	if(var.vt == VT_I4)
		HideBox9 = var.lVal;

	var = rs->Fields->Item["HideBox11"]->Value;
	if(var.vt == VT_I4)
		HideBox11 = var.lVal;

	var = rs->Fields->Item["UseEmp"]->Value;
	if(var.vt == VT_I4)
		UseEmp = var.lVal;

	var = rs->Fields->Item["Box33Num"]->Value;
	if(var.vt == VT_I4)
		Box33Num = var.lVal;	

	//Even though this is breaking the format above, this is a cleaner way to get the info we need
	FieldsPtr pflds = rs->GetFields();
	HidePhoneBox33 = AdoFldLong(pflds, "HidePhoneBox33", 0);

	// (j.jones 2010-07-23 10:44) - PLID 39794 - this now defaults to 3
	Box12Accepted = AdoFldLong(pflds, "Box12Accepted", 3);

	// (j.jones 2010-07-22 11:48) - PLID 39779 - this defaults to 3
	Box13Accepted = AdoFldLong(pflds, "Box13Accepted", 3);

	DefaultPriorAuthNum = AdoFldString(pflds, "DefaultPriorAuthNum", "");

	DocAddress = AdoFldLong(pflds, "DocAddress", 1);

	ShowDiagDecimals = AdoFldLong(pflds, "ShowDiagDecimals", 0);

	ShowICD9Desc = AdoFldLong(pflds, "ShowICD9Desc", 0);

	Box24C = AdoFldLong(pflds, "Box24C", 0);

	// (j.jones 2007-03-26 15:45) - PLID 24538 - I removed this, but by client demand I reinstated it
	LeftAlignInsAddress = AdoFldLong(pflds, "LeftAlignInsAddress", 0);

	Box19RefPhyID = AdoFldLong(pflds, "Box19RefPhyID", 0);

	Box19Override = AdoFldLong(pflds, "Box19Override", 0);
	Box19 = AdoFldString(pflds, "Box19", "");

	HideBox30 = AdoFldLong(pflds, "HideBox30", 0);

	ShowSecInsAdd = AdoFldLong(pflds, "ShowSecInsAdd", 0);

	ANSI_SendBox19 = AdoFldLong(pflds, "ANSI_SendBox19", 0);
	ANSI_SendRefPhyIn2300 = AdoFldLong(pflds, "ANSI_SendRefPhyIn2300", 0);
	ANSI_UseAnesthMinutesAsQty = AdoFldLong(pflds, "ANSI_UseAnesthMinutesAsQty", 0);

	ShowAnesthesiaTimes = AdoFldLong(pflds, "ShowAnesthesiaTimes", 0);

	Box12UseDate = AdoFldLong(pflds, "Box12UseDate", 0);
	Box31UseDate = AdoFldLong(pflds, "Box31UseDate", 0);

	strCorrespondenceNote = AdoFldString(pflds, "ANSI_Correspondence_Note", "");

	ShowRespName = AdoFldLong(pflds, "ShowRespName", 0);

	// (j.jones 2009-08-03 14:58) - PLID 33827 - removed Use2310B
	//Use2310B = AdoFldBool(pflds, "Use2310B", FALSE);
	Use2310BPRVSegment = AdoFldBool(pflds, "Use2310BPRVSegment", FALSE);

	Use2000APRVSegment = AdoFldLong(pflds, "Use2000APRVSegment", 1);

	ShowSecPINNumber = AdoFldLong(pflds, "ShowSecPINNumber", 1);
	// (j.jones 2006-09-14 10:01) - I did not rename this field yet because
	// we really have not yet reworked the HCFA image to support the new HCFA form
	ShowSec24JNumber = AdoFldLong(pflds, "ShowSec24KNumber", 1);
	
	ExcludeAdjFromBal = AdoFldLong(pflds, "ExcludeAdjFromBal", 1);

	UseBox23InBox17a = AdoFldLong(pflds, "UseBox23InBox17a", 0);

	UseOverrideInBox31 = AdoFldLong(pflds, "UseOverrideInBox31", 0);

	PrintPinBox1a = AdoFldLong(pflds, "PrintPinBox1a", 0);

	Use1aIn9a = AdoFldLong(pflds, "Use1aIn9a", 0);
	Use1aIn9aAlways = AdoFldLong(pflds, "Use1aIn9aAlways", 0);

	Box17aQual = AdoFldString(pflds, "Box17aQual", "");
	Box33bQual = AdoFldString(pflds, "Box33bQual", "");

	ShowAnesthMinutesOnly = AdoFldLong(pflds, "ShowAnesthMinutesOnly", 0);

	SecondaryFillBox11 = AdoFldLong(pflds, "SecondaryFillBox11", 0);

	// (j.jones 2006-11-14 08:46) - PLID 23413 - Added options to control
	// ANSI NPI behavior
	NM109_IDType = AdoFldLong(rs, "NM109_IDType", 0);
	// (j.jones 2008-09-09 13:12) - PLID 27450 - changed the default to 2
	ExtraREF_IDType = AdoFldLong(rs, "ExtraREF_IDType", 2);

	// (j.jones 2006-11-15 16:36) - PLID 23563 - added ability to control
	// which NPI to use in Box 33 when using Bill Location
	LocationNPIUsage = AdoFldLong(rs, "LocationNPIUsage", 0);

	// (j.jones 2006-11-24 15:02) - PLID 23415 - added secondary ANSI controls
	ANSI_EnablePaymentInfo = AdoFldLong(rs, "ANSI_EnablePaymentInfo", 1);
	ANSI_AdjustmentLoop = AdoFldLong(rs, "ANSI_AdjustmentLoop", 1);
	ANSI_2300Contract = AdoFldLong(rs, "ANSI_2300Contract", 0);
	ANSI_2320Allowed = AdoFldLong(rs, "ANSI_2320Allowed", 0);
	ANSI_2320Approved = AdoFldLong(rs, "ANSI_2320Approved", 0);
	ANSI_2400Contract = AdoFldLong(rs, "ANSI_2400Contract", 0);

	// (j.jones 2007-02-15 11:50) - PLID 24762 - added option for allowed amount in 2400
	ANSI_2400Allowed = AdoFldLong(rs, "ANSI_2400Allowed", 0);

	// (j.jones 2007-02-21 12:24) - PLID 24776 - added QualifierSpace
	QualifierSpace = AdoFldLong(rs, "QualifierSpace", 0);

	// (j.jones 2007-03-28 13:32) - PLID 25392 - added ShowWhichCodesCommas
	ShowWhichCodesCommas = AdoFldLong(rs, "ShowWhichCodesCommas", 0);

	// (j.jones 2007-03-29 11:51) - PLID 25409 - added ANSI_SecondaryAllowableCalc
	// (j.jones 2009-08-28 17:45) - PLID 32993 - removed this setting
	//ANSI_SecondaryAllowableCalc = AdoFldLong(rs, "ANSI_SecondaryAllowableCalc", 3);

	// (j.jones 2007-04-09 16:16) - PLID 25537 - added HideBox11a/b/c
	HideBox11a_Birthdate = AdoFldLong(rs, "HideBox11a_Birthdate", 0);
	HideBox11a_Gender = AdoFldLong(rs, "HideBox11a_Gender", 0);
	HideBox11b = AdoFldLong(rs, "HideBox11b", 0);
	HideBox11c = AdoFldLong(rs, "HideBox11c", 0);

	// (j.jones 2007-04-09 17:47) - PLID 25539 - added Hide24ATo
	Hide24ATo = AdoFldLong(rs, "Hide24ATo", 0);

	// (j.jones 2007-05-11 11:39) - PLID 25932 - revised Box 32 hide options
	HideBox32NameAdd = AdoFldLong(rs, "HideBox32NameAdd", 0);
	HideBox32NPIID = AdoFldLong(rs, "HideBox32NPIID", 0);

	// (j.jones 2007-06-15 10:08) - PLID 26309 - added group/reason code defaults
	// (j.jones 2010-09-23 15:52) - PLID 40653 - the group & reason codes are IDs now, but we load the actual code here
	ANSI_DefaultRemAdjGroupCode = AdoFldString(rs, "ANSI_DefaultAdjGroupCode", "OA");
	ANSI_DefaultRemAdjReasonCode = AdoFldString(rs, "ANSI_DefaultAdjReasonCode", "2");

	// (j.jones 2007-07-12 14:44) - PLID 26636 - added ability to hide 24J NPI
	HideBox24JNPI = AdoFldLong(rs, "HideBox24JNPI", 0);

	// (j.jones 2008-02-06 17:15) - PLID 28843 - added SecondaryInsCodeUsage and SecondaryInsCode
	bSecondaryInsCodeUsage = AdoFldBool(rs, "SecondaryInsCodeUsage", FALSE);
	strSecondaryInsCode = AdoFldString(rs, "SecondaryInsCode", "");

	// (j.jones 2008-02-25 09:06) - PLID 29077 - added ANSI_SecondaryApprovedCalc
	ANSI_SecondaryApprovedCalc = AdoFldLong(rs, "ANSI_SecondaryApprovedCalc", 3);

	// (j.jones 2008-04-01 16:17) - PLID 29486 - added ANSI_Hide2330AREF
	ANSI_Hide2330AREF = AdoFldLong(rs, "ANSI_Hide2330AREF", 0);

	// (j.jones 2008-06-10 14:50) - PLID 25834 - added ValidateRefPhy
	ValidateRefPhy = AdoFldLong(rs, "ValidateRefPhy", 0);

	// (j.jones 2008-06-18 09:05) - PLID 30403 - added DoNotFillBox29
	DoNotFillBox29 = AdoFldLong(rs, "DoNotFillBox29", 0);

	// (j.jones 2008-06-23 12:23) - PLID 30434 - added Eligibility_2100C_REF_Option
	Eligibility_2100C_REF_Option = AdoFldLong(rs, "Eligibility_2100C_REF_Option", 1);

	// (j.jones 2010-05-25 15:43) - PLID 38868 - added Eligibility_2100D_REF
	Eligibility_2100D_REF = AdoFldLong(rs, "Eligibility_2100D_REF", 2);

	// (j.jones 2008-10-06 10:17) - PLID 31580 - added ANSI_SendCorrespSegment
	ANSI_SendCorrespSegment = AdoFldLong(rs, "ANSI_SendCorrespSegment", 1);

	// (j.jones 2008-10-06 13:03) - PLID 31578 - added ANSI_SendBox19Segment
	ANSI_SendBox19Segment = AdoFldLong(rs, "ANSI_SendBox19Segment", 1);

	// (j.jones 2008-11-12 10:58) - PLID 31912 - added bANSI_SendCASPR
	bANSI_SendCASPR = AdoFldBool(rs, "ANSI_SendCASPR", TRUE);

	// (j.jones 2009-01-06 09:09) - PLID 32614 - added ability to hide 17b
	HideBox17b = AdoFldLong(rs, "HideBox17b", 0);

	// (j.jones 2009-03-06 11:18) - PLID 33354 - added PullCurrentAccInfo
	PullCurrentAccInfo = AdoFldLong(rs, "PullCurrentAccInfo", 1);

	// (j.jones 2009-03-11 10:36) - PLID 33446 - added default insurance adjustment group/reason codes
	// (j.jones 2010-09-23 15:52) - PLID 40653 - the group & reason codes are IDs now, but we load the actual code here
	ANSI_DefaultInsAdjGroupCode = AdoFldString(rs, "ANSI_DefaultInsAdjGroupCode", "CO");
	ANSI_DefaultInsAdjReasonCode = AdoFldString(rs, "ANSI_DefaultInsAdjReasonCode", "45");

	// (j.jones 2009-11-24 15:45) - PLID 36411 - added PriorAuthQualifier
	PriorAuthQualifier = AdoFldString(rs, "PriorAuthQualifier", "G1");

	// (j.jones 2010-03-31 15:15) - PLID 37918 - added ANSI_Hide2430_SVD06_OneCharge
	ANSI_Hide2430_SVD06_OneCharge = AdoFldLong(rs, "ANSI_Hide2430_SVD06_OneCharge", 0);

	// (j.jones 2010-08-31 09:58) - PLID 15025 - added ANSI_SendTPLNumber
	ANSI_SendTPLNumber = AdoFldLong(rs, "ANSI_SendTPLNumber", 1);

	// (j.jones 2010-08-31 10:37) - PLID 40303 - added TPLIn9a
	TPLIn9a = AdoFldLong(rs, "TPLIn9a", 0);

	// (j.jones 2011-06-15 12:51) - PLID 42181 - added EligNPIUsage
	EligNPIUsage = AdoFldLong(rs, "EligNPIUsage", 1);

	// (j.jones 2012-03-23 15:33) - PLID 49176 - added PRV2000ACode
	PRV2000ACode = AdoFldLong(rs, "PRV2000ACode", 0);

	// (j.jones 2012-05-14 14:01) - PLID 50338 - added ANSI_SBR04
	// (j.jones 2012-10-04 10:07) - PLID 53018 - removed ANSI_SBR04 and added
	// ANSI_2000B_SBR03, ANSI_2000B_SBR04, and ANSI_2320_SBR04
	ANSI_2000B_SBR03 = AdoFldLong(rs, "ANSI_2000B_SBR03", 0);
	ANSI_2000B_SBR04 = AdoFldLong(rs, "ANSI_2000B_SBR04", 1);
	ANSI_2320_SBR04 = AdoFldLong(rs, "ANSI_2320_SBR04", 1);

	// (j.jones 2013-04-11 09:01) - PLID 56166 - added ANSI_HideRefPhyFields
	ANSI_HideRefPhyFields = AdoFldLong(rs, "ANSI_HideRefPhyFields", 1);

	// (j.jones 2013-06-10 16:26) - PLID 56255 - added UseRefPhyGroupNPI
	UseRefPhyGroupNPI = AdoFldLong(rs, "UseRefPhyGroupNPI", 0);

	// (j.jones 2013-06-20 11:39) - PLID 57245 - added OrigRefNo_2300 and OrigRefNo_2330B
	OrigRefNo_2300 = AdoFldLong(rs, "OrigRefNo_2300", 1);
	OrigRefNo_2330B = AdoFldLong(rs, "OrigRefNo_2330B", 1);

	// (j.jones 2013-07-08 15:08) - PLID 57469 - added EligTitleInLast
	EligTitleInLast = AdoFldLong(rs, "EligTitleInLast", 0);

	// (j.jones 2013-07-17 16:06) - PLID 41823 - added DontBatchSecondary
	DontBatchSecondary = AdoFldLong(rs, "DontBatchSecondary", 0);

	// (j.jones 2014-01-22 09:49) - PLID 60034 - added OrderingProvider
	OrderingProvider = AdoFldLong(rs, "OrderingProvider");

	// (j.jones 2014-04-23 13:47) - PLID 61840 - added Hide 2310A setting
	Hide2310A = (ANSI_Hide2310AOptions)AdoFldLong(rs, "Hide2310A", (long)hide2310A_Never);

	// (b.spivey January 26, 2015) - PLID 64452 - added SendN3N4PERsegment
	SendN3N4PERSegment = !!AdoFldBool(rs, "SendN3N4PERSegment", FALSE);

	rs->Close();
}
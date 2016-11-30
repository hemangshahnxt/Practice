// EbillingValidationDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Ebilling.h"
#include "EbillingValidationDlg.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "GlobalUtils.h"
#include "InsuranceDlg.h"
#include "GlobalFinancialUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;

/////////////////////////////////////////////////////////////////////////////
// CEbillingValidationDlg dialog

// (j.jones 2014-04-25 11:30) - PLID 61912 - added tablechecker for the charge claim provider setup
CEbillingValidationDlg::CEbillingValidationDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEbillingValidationDlg::IDD, pParent),
	m_ClaimProviderChecker(NetUtils::ChargeLevelProviderConfigT)
{
	//{{AFX_DATA_INIT(CEbillingValidationDlg)
		m_strInvalidClaims = "";
		m_LocationID = -1;
		m_ClaimType = -1;
		m_bElectronic = TRUE;
		m_bErrorListDisplayed = FALSE;
		m_nProviderID = -1;
		// (j.jones 2012-02-01 11:02) - PLID 40880 - the default is now 5010
		m_avANSIVersion = av5010;
	//}}AFX_DATA_INIT
}


void CEbillingValidationDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEbillingValidationDlg)
	DDX_Control(pDX, IDC_CURRENT_CLAIM_PROGRESS, m_progress);
	DDX_Control(pDX, IDC_CURRENT_CLAIM_LABEL, m_nxstaticCurrentClaimLabel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEbillingValidationDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEbillingValidationDlg)
	ON_WM_SHOWWINDOW()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEbillingValidationDlg message handlers

BOOL CEbillingValidationDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	// (j.jones 2006-04-25 11:30) - load all the properties at once into the NxPropManager cache
	// Note: this must remain identical to the cache in EbillingValidationConfigDlg.cpp, so ensure
	// that both files have the exact same cache list, whenever this list changes.
	// (j.jones 2008-12-03 14:11) - PLID 32305 - we now have separate preferences for OHIP versus regular claims,
	// so cache only what we need
	if(m_FormatStyle == OHIP) {
		//load the OHIP preferences
		g_propManager.CachePropertiesInBulk("OHIPValidationPrefs-Number", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'OHIPValidateMOHOfficeCode' OR "
			"Name = 'OHIPValidateGroupNumber' OR "
			"Name = 'OHIPValidateSpecialty' OR "
			"Name = 'OHIPValidateProviderNPI' OR "
			"Name = 'OHIPValidateLocationNPI' OR "
			"Name = 'OHIPValidatePOSNPI' OR "
			"Name = 'OHIPValidateRefPhySelected' OR "
			"Name = 'OHIPValidateRefPhyNPI' OR "
			"Name = 'OHIPValidateServiceCode' OR "
			"Name = 'OHIPValidatePatientHealthNumber' OR "
			"Name = 'OHIPValidatePatientVersionCode' OR "
			"Name = 'OHIPValidateRegistrationNumber' OR "
			"Name = 'OHIPValidatePatientName' OR "
			"Name = 'OHIPValidatePatientBirthdate' OR "
			"Name = 'OHIPValidatePatientGender' OR "
			"Name = 'OHIPValidatePatientProvince' OR "
			"Name = 'OHIPValidateDiagCode' OR "
			"Name = 'OHIP_HealthNumberCustomField' OR "
			"Name = 'OHIP_VersionCodeCustomField' OR "
			"Name = 'OHIP_RegistrationNumberCustomField' OR "
			// (j.jones 2008-12-10 11:22) - PLID 32312 - added patient health number verification
			"Name = 'OHIPValidatePatientHealthNumberVerify' OR "
			// (j.jones 2009-08-14 13:47) - PLID 35235 - added potential provider filter
			"Name = 'OHIP_SeparateAccountsPerProviders' OR "
			// (j.jones 2010-02-02 09:16) - PLID 33060 - added option to send G2 ref. phy. in a claim
			"Name = 'OHIPUseG2RefPhy' OR "
			// (j.jones 2011-09-23 16:07) - PLID 39377 - added validation for duplicate same-day ebilling claims
			"Name = 'OHIPValidateExportedTwiceInSameDay' "
			")",
			_Q(GetCurrentUserName()));

		g_propManager.CachePropertiesInBulk("OHIPValidationPrefs-Text", propText,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'OHIP_GroupNumber' OR "
			"Name = 'OHIP_MOHOfficeCode' "
			")",
			_Q(GetCurrentUserName()));
	}
	// (j.jones 2010-11-01 17:22) - PLID 39594 - supported validating Alberta H-Link claims
	else if(m_FormatStyle == ALBERTA) {
		//load the Alberta preferences

		g_propManager.CachePropertiesInBulk("AlbertaValidationPrefs-Number", propNumber,
				"(Username = '<None>' OR Username = '%s') AND ("
				"Name = 'AlbertaValidatePOSNPI' OR "
				"Name = 'AlbertaValidateRefPhySelected' OR "
				"Name = 'AlbertaValidateRefPhyNPI' OR "
				"Name = 'AlbertaValidatePatientBirthdate' OR "
				"Name = 'AlbertaValidatePatientName' OR "
				"Name = 'AlbertaValidatePatientGender' OR "
				"Name = 'AlbertaValidateProviderNPI' OR "
				"Name = 'AlbertaValidateProviderTaxonomyCode' OR "
				"Name = 'AlbertaValidateProviderBAID' OR "
				"Name = 'AlbertaValidateSubmitterPrefix' OR "
				"Name = 'AlbertaValidatePatientHealthNumber' OR "
				"Name = 'AlbertaValidatePatientRegNumber' OR "
				"Name = 'Alberta_PatientULICustomField' OR "
				"Name = 'Alberta_PatientRegNumberCustomField' OR "
				"Name = 'Alberta_ProviderBAIDCustomField' OR "
				// (j.jones 2011-09-23 16:07) - PLID 39377 - added validation for duplicate same-day ebilling claims
				"Name = 'AlbertaValidateExportedTwiceInSameDay' OR "
				"Name = 'Alberta_SubmitterPrefixCustomField' " // (j.dinatale 2012-12-28 11:30) - PLID 54382
				")",
				_Q(GetCurrentUserName()));

		g_propManager.CachePropertiesInBulk("AlbertaValidationPrefs-Text", propText,
				"(Username = '<None>' OR Username = '%s') AND ("
				"Name = 'Alberta_SubmitterPrefix'"
				")",
				_Q(GetCurrentUserName()));
	}
	else {
		//good old US of A
		g_propManager.CachePropertiesInBulk("EbillingValidationPrefs", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'EbillingValidatePOSCode' OR "
			"Name = 'EbillingValidatePrimaryPayerID' OR "
			"Name = 'EbillingValidatePrimaryNSFCode' OR "
			"Name = 'EbillingValidatePrimaryName' OR "
			"Name = 'EbillingValidatePrimaryAddress' OR "
			"Name = 'EbillingValidatePrimaryBirthdate' OR "
			"Name = 'EbillingValidatePrimaryInsuranceID' OR "
			"Name = 'EbillingValidatePrimaryPolicyGroupNum' OR "
			"Name = 'EbillingValidateProvider' OR "
			// (j.jones 2011-03-15 10:55) - PLID 42788 - EbillingValidateProviderIDs is now obsolete,
			// but is used to determine the default for EbillingValidateProviderOtherIDs and
			// EbillingValidateProviderBox24JID
			"Name = 'EbillingValidateProviderIDs' OR "
			"Name = 'EbillingValidateProviderOtherIDs' OR "
			"Name = 'EbillingValidateProviderBox24JID' OR "
			"Name = 'EbillingValidateProviderTaxonomyCode' OR "
			"Name = 'EbillingValidateRefPhyID' OR "
			"Name = 'EbillingValidateHospFrom' OR "
			"Name = 'EbillingValidateHospTo' OR "
			"Name = 'EbillingValidateDiagCode1' OR "
			"Name = 'EbillingValidateDiagPointers' OR "
			"Name = 'EbillingValidateInvalidModifiers' OR "
			"Name = 'EbillingValidateSecondaryPayerID' OR "
			"Name = 'EbillingValidateSecondaryNSFCode' OR "
			"Name = 'EbillingValidateSecondaryPhoneNumber' OR "
			"Name = 'EbillingValidateSecondaryContactName' OR "
			"Name = 'EbillingValidateSecondaryName' OR "
			"Name = 'EbillingValidateSecondaryAddress' OR "
			"Name = 'EbillingValidateSecondaryBirthdate' OR "
			"Name = 'EbillingValidateSecondaryInsuranceID' OR "
			"Name = 'EbillingValidateSecondaryPolicyGroupNum' OR "
			"Name = 'EbillingValidatePatientAddress' OR "
			"Name = 'EbillingValidatePatientBirthdate' OR "
			"Name = 'EbillingValidatePOSNPI' OR "
			"Name = 'EbillingValidateNPIChecksum' OR "	// (a.walling 2008-05-20 09:30) - PLID 27812
			"Name = 'EbillingValidateProviderNPI' OR "	// (j.jones 2008-05-21 15:04) - PLID 29280 - added provider and ref. phy. NPI options
			"Name = 'EbillingValidateRefPhyNPI' OR "
			"Name = 'EbillingValidatePrimaryGender' OR "	// (j.jones 2009-08-10 10:55) - PLID 32886 - added gender validations
			"Name = 'EbillingValidateSecondaryGender' OR "
			"Name = 'EbillingValidatePatientGender' OR "
			// (j.jones 2009-08-14 13:47) - PLID 35235 - added potential provider filter, include in this cache anyways
			"Name = 'OHIP_SeparateAccountsPerProviders' OR "
			// (j.jones 2010-01-07 09:18) - PLID 36786 - added ability to disable the Self name checking
			"Name = 'EbillingValidatePatientNameWhenSelf' OR "
			// (j.jones 2010-04-16 11:58) - PLID 38225 - added ability to disable sending REF*XX in provider loops
			"Name = 'ANSIDisableREFXX' OR "
			// (j.jones 2010-07-23 11:44) - PLID 39797 - added assignment of benefits validation
			"Name = 'EbillingValidateAssignmentOfBenefits' OR "
			// (j.jones 2011-09-23 16:07) - PLID 39377 - added validation for duplicate same-day ebilling claims
			"Name = 'EbillingValidateExportedTwiceInSameDay' "
			// (j.jones 2012-05-01 09:59) - PLID 48530 - added prior auth. number validation
			"OR Name = 'EbillingValidatePriorAuthNumber' "
			// (j.jones 2012-07-25 09:24) - PLID 51764 - added office visit global period validation
			"OR Name = 'EbillingValidateOfficeVisitGlobalPeriods' "
			"OR Name = 'GlobalPeriod_OnlySurgicalCodes' "
			"OR Name = 'GlobalPeriod_IgnoreModifier78' "	// (j.jones 2012-07-26 14:26) - PLID 51827
			"OR Name = 'EbillingValidateAccidentTypeForCurrentAccidentDate' " // (a.wilson 2013-03-26 16:37) - PLID 51773
			"OR Name = 'Claim_SendPatAddressWhenPOS12'	" // (j.jones 2013-04-25 11:18) - PLID 55564
			"OR Name = 'EbillingValidateRequiresICD9' " // (d.singleton 2014-03-06 15:37) - PLID 61235
			"OR Name = 'EbillingValidateRequiresICD10' " // (d.singleton 2014-03-06 15:38) - PLID 61236
			"OR Name = 'SkipUnlinkedDiagsOnClaims' " // (d.singleton 2014-03-06 15:38) - PLID 61236
			")",
			_Q(GetCurrentUserName()));
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// (j.jones 2009-08-14 13:48) - PLID 35235 - added optional provider filter
int CEbillingValidationDlg::DoModal(long HCFAID, long FormatID, long FormatStyle, long LocationID, long nProviderID, long ClaimType, BOOL bElectronic) 
{
	m_HCFAID = HCFAID;
	m_FormatID = FormatID;
	m_FormatStyle = FormatStyle;
	m_LocationID = LocationID;
	m_nProviderID = nProviderID;
	m_ClaimType = ClaimType;
	m_bElectronic = bElectronic;
	return CNxDialog::DoModal();
}


void CEbillingValidationDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CNxDialog::OnShowWindow(bShow, nStatus);

	SetTimer(0,10,NULL);
}

BOOL CEbillingValidationDlg::ValidateOne(long HCFAID)
{
	try {

		//TODO: start checking for a lot more possible errors. If we don't know
		//that it would specifically cause a rejection, then set bWarnings = TRUE.
		//Then the validation icon will be a question mark, not a red X.

		// (j.jones 2006-12-11 16:57) - PLID 23811 - for OHIP, use an entirely different
		// validation function
		if(m_FormatStyle == OHIP) {
			return ValidateOneOHIP(HCFAID);
		}
		// (j.jones 2010-11-01 17:22) - PLID 39594 - supported validating Alberta H-Link claims
		else if(m_FormatStyle == ALBERTA) {
			return ValidateOneAlberta(HCFAID);
		}

		// (j.jones 2010-04-16 11:59) - PLID 38225 - added ability to disable sending REF*XX in provider loops
		BOOL bDisableREFXX = GetRemotePropertyInt("ANSIDisableREFXX", 1, 0, "<None>", true) == 1;

		_variant_t var;
		BOOL bPassed = TRUE;
		BOOL bWarnings = FALSE;

		BOOL bHasInsurance = TRUE;

		CString str, strErrors = "";

		long nBillID = -1;
		long nProviderID_2010AA = -1;
		long nProviderID_2310B = -1;
		long nProviderID_Box33 = -1;
		long nProviderID_Box24J = -1;
		long nBillingProviderID = -1;
		long nRenderingProviderID = -1;
		long HCFASetupGroupID = -1, UB92SetupGroupID = -1;
		long HCFAProvType = -1;
		long Box33Num = -1;
		long DocAddress = -1;
		long nBox13Accepted = 3;
		long nUBBox53Accepted = 1;
		HCFABox13Over hb13Value = hb13_UseDefault;
		long InsuranceCoID = -1;
		long nInsuredPartyID = -1;
		long nPOSID = -1;
		long nLocationID = -1;
		long nPatientID = -1;
		long Box33 = 2;
		long LocationNPIUsage = 1;
		CString strBox33bQual = "";
		CString strInsCoName = "", strHCFAPayerID = "", strUBPayerID = "";
		InsuranceTypeCode eInsType = itcInvalid;
		long nBox82Setup = -1, nBox83Setup = -1, nUB04Box77Setup = -1;
		long nBox82Num = -1, nBox83Num = -1, nUB04Box77Num = -1;
		long nHCFABox25 = 2;
		CString strUB04Box76Qual = "", strUB04Box77Qual = "", strUB04Box78Qual = "";
		long nUB_2310B_ProviderID = -1;
		long nUB_2310C_ProviderID = -1;
		long nValidateRefPhy = 0;
		CString strRelationToPatient = "", strInsuredName = "";
		CString strDefaultBox33GRP = "";
		long nExtraREF_IDType = 2;
		CString strLocationEIN = "";
		ANSI_ClaimTypeCode eClaimTypeCode = ctcOriginal;
		CString strOriginalRefNum = "";
		CString strMedicaidResubmissionNum = "";

		BOOL bIsUB92 = FALSE;

		// (j.jones 2010-04-14 11:59) - PLID 27457 - load the ANSI Format settings
		BOOL bSend2010AB = FALSE;
		BOOL bSend2420A = FALSE;
		BOOL bDontSubmitSecondary = FALSE;
		BOOL bExport2330BPER = FALSE;
		BOOL bIsGroup = FALSE;
		BOOL bUse_Addnl_2010AA = FALSE;
		BOOL bHide2310C_WhenType11Or12 = FALSE;
		CString strAddnl_2010AA_Qual = "";
		CString strAddnl_2010AA = "";
		if(m_FormatID != -1) {
			_RecordsetPtr rsFormats = CreateParamRecordset("SELECT Use2420A, Use2010AB, DontSubmitSecondary, Export2330BPER, IsGroup, "
				"Use_Addnl_2010AA, Addnl_2010AA_Qual, Addnl_2010AA, ANSIVersion, Hide2310D "
				"FROM EbillingFormatsT WHERE ID = {INT}", m_FormatID);
			if(!rsFormats->eof) {
				bSend2010AB = AdoFldBool(rsFormats, "Use2010AB", FALSE);
				bSend2420A = AdoFldBool(rsFormats, "Use2420A", FALSE);
				bDontSubmitSecondary = AdoFldBool(rsFormats, "DontSubmitSecondary", FALSE);
				bExport2330BPER = AdoFldBool(rsFormats, "Export2330BPER", FALSE);
				bIsGroup = AdoFldBool(rsFormats, "IsGroup", FALSE);
				bUse_Addnl_2010AA = AdoFldBool(rsFormats, "Use_Addnl_2010AA", FALSE);
				strAddnl_2010AA_Qual = AdoFldString(rsFormats, "Addnl_2010AA_Qual", "");
				strAddnl_2010AA = AdoFldString(rsFormats, "Addnl_2010AA", "");
				// (j.jones 2010-10-19 17:33) - PLID 40936 - added ANSIVersion
				// (j.jones 2012-02-01 11:02) - PLID 40880 - the default is now 5010 (although this field is not nullable)
				m_avANSIVersion = (ANSIVersion)AdoFldLong(rsFormats, "ANSIVersion", (long)av5010);
				// (j.jones 2013-04-25 11:38) - PLID 55564 - loaded the Hide2310C field, this is actually
				// Hide2310D in data, but it's 2310C in 5010, I'd rather have the variable be easy to read
				bHide2310C_WhenType11Or12 = AdoFldBool(rsFormats, "Hide2310D", FALSE);
			}
			rsFormats->Close();
		}

		BOOL bSkipPOSValidation = FALSE;
		BOOL bSendPatientAddressAsPOS = FALSE;

		//output the header, and while doing so, check our basic information on the claim

		// (j.jones 2012-07-25 09:24) - PLID 51764 - Include global period information in the query,
		// so that we can later know whether the patient is under an active global period.
		// Check the preference to see if we are only tracking global periods for
		// surgical codes only, if it is disabled when we would look at all codes.
		long nSurgicalCodesOnly = GetRemotePropertyInt("GlobalPeriod_OnlySurgicalCodes", 1, 0, "<None>", true);

		// (j.jones 2012-07-26 15:19) - PLID 51827 - added a preference to NOT track global periods
		// if the charge uses modifier 78
		long nIgnoreModifier78 = GetRemotePropertyInt("GlobalPeriod_IgnoreModifier78", 1, 0, "<None>", true);

		// (j.jones 2007-04-25 08:55) - PLID 25277 - added UB04 qualifiers
		// (j.jones 2008-06-10 15:11) - PLID 25834 - added ValidateRefPhy
		// (j.jones 2008-12-11 16:13) - PLID 32414 - added SupervisingProviderNPI info.
		// (j.jones 2009-08-05 10:08) - PLID 34467 - supported payer IDs per location / insurance
		// (j.jones 2009-09-11 11:51) - PLID 34540 - added RelationToPatient and InsuredName
		// (j.jones 2009-12-17 09:53) - PLID 36237 - supported UB payer IDs
		// (j.jones 2010-10-20 13:46) - PLID 40936 - loaded location zip codes, claim provider
		// (j.jones 2010-11-10 14:24) - PLID 41396 - loaded all possible provider IDs, using claim provider information
		// (j.jones 2011-08-16 17:22) - PLID 44805 - we will filter out "original" and "void" charges
		// (j.jones 2011-08-23 12:31) - PLID 45141 - added UB 2310B and 2310C provider ID
		// (j.jones 2011-09-16 16:01) - PLID 39789 - added checks for claim type / original ref. num		
		// (j.jones 2011-09-23 16:14) - PLID 39377 - added AlreadyExportedToday
		// (j.jones 2012-05-01 10:02) - PLID 48530 - added validation for prior auth. number
		// (j.jones 2012-08-06 16:28) - PLID 51916 - supported the new payer ID data structure, always pulling from EbillingInsCoIDs
		// (r.wilson 10/2/2012) plid 52970 - Replace hardcoded SendType with enumerated value
		// (a.wilson 2013-03-26 15:23) - PLID 51773 - ensure they have accident type for addicent dates for HCFA.
		// (j.jones 2013-04-25 11:12) - PLID 55564 - we now get the POS designation code (11, 22, etc.) and the patient's zip code
		// I also added TOP 1, because this query runs on all charges, but never calls MoveNext.
		// Use of TOP 1 makes it clearer what the intentional use is here.
		// (d.singleton 2014-03-06 16:08) - PLID 61236 - if ins company requires icd10 codes and none are present on bill need to have validation error in ebilling
		// (d.singleton 2014-03-06 16:08) - PLID 61235 - if ins company requires icd9 codes and none are present on bill need to have validation error in ebilling
		// (j.jones 2014-07-17 14:11) - PLID 62931 - added a warning for 439 - Accident date used without a valid condition relation type
		// (j.jones 2016-04-11 17:13) - NX-100070 - added BillsT.AccidentDate
		_RecordsetPtr rsClaim = CreateParamRecordset("SELECT TOP 1 BillsT.ID AS BillID, BillsT.FormType, "
			"PersonT.[Last] + ', ' + PersonT.[First] + ' ' + PersonT.Middle AS PatientName, "
			"InsuredPersonT.[Last] + ', ' + InsuredPersonT.[First] + ' ' + InsuredPersonT.Middle AS InsuredName, "
			"BillsT.Description, BillsT.Date, PersonT.ID AS PersonID, InsuranceCoT.PersonID AS InsCoID, "
			"InsuredPartyT.PersonID AS InsuredPartyID, InsuranceCoT.Name AS InsCoName, "
			"PlaceOfServiceCodesT.PlaceCodes AS POSCode, PlaceOfServiceT.Name AS POSName, PlaceOfServiceT.Zip AS POSZIP, "
			"PersonT.Zip AS PatientZip, PlaceOfServiceT.ID AS POSID, LineItemT.LocationID, "
			"HCFASetupT.ID AS HCFASetupGroupID, UB92SetupT.ID AS UB92SetupGroupID, HCFASetupT.Box33GRP AS DefaultBox33GRP, "
			"CASE WHEN HCFALocPayerIDsT.ID Is Not Null THEN HCFALocPayerIDsT.EbillingID ELSE HCFAPayerIDsT.EBillingID END AS HCFAPayerID, "
			"CASE WHEN UBLocPayerIDsT.ID Is Not Null THEN UBLocPayerIDsT.EbillingID ELSE UBPayerIDsT.EBillingID END AS UBPayerID, "
			"HCFASetupT.ExtraREF_IDType AS HCFA_ExtraREF_IDType, UB92SetupT.ExtraREF_IDType AS UB_ExtraREF_IDType, "
			"InsuranceCoT.InsType, InsuranceCoT.UseReferrals, BillsT.PriorAuthNum, HCFASetupT.DefaultPriorAuthNum, "
			"HCFASetupT.Box33, HCFASetupT.Box25, HCFASetupT.Box33Setup, HCFASetupT.Box33Num, HCFASetupT.DocAddress, HCFASetupT.Box33bQual, "
			"BillsT.ConditionDate, BillsT.ConditionDateType, BillsT.AccidentDate, BillsT.RelatedToEmp, BillsT.RelatedToAutoAcc, BillsT.RelatedToOther, "
			"HCFASetupT.PullCurrentAccInfo, InsuredPartyT.DateOfCurAcc, InsuredPartyT.AccidentType, "
			"HCFASetupT.Box13Accepted, BillsT.HCFABox13Over, UB92SetupT.Box53Accepted, "
			"HCFASetupT.LocationNPIUsage, "
			"HCFASetupT.ValidateRefPhy, "
			"UB92SetupT.Box82Num, UB92SetupT.Box82Setup, "
			"UB92SetupT.Box83Num, UB92SetupT.Box83Setup, "
			"UB92SetupT.UB04Box77Num, UB92SetupT.UB04Box77Setup, "
			"UB92SetupT.UB04Box76Qual, UB92SetupT.UB04Box77Qual, UB92SetupT.UB04Box78Qual, "
			"BillsT.SupervisingProviderID, SupervisingProvidersT.NPI AS SupervisingProviderNPI, "
			"SupervisingProviderPersonT.Last + ', ' + SupervisingProviderPersonT.First +  ' ' + SupervisingProviderPersonT.Middle AS SupervisingProviderName, "
			"InsuredPartyT.RelationToPatient, "
			"LocationsT.EIN AS LocEIN, LocationsT.Zip AS LocZIP, "
			"BillsT.ANSI_ClaimTypeCode, BillsT.OriginalRefNo, BillsT.MedicaidResubmission, "
			"ConverT(bit, CASE WHEN AlreadyExportedTodayQ.BillID Is Null THEN 0 ELSE 1 END) AS AlreadyExportedToday, ICD10Count.Count AS ICD10Codes, ICD9Count.Count AS ICD9Codes, "
			"ICD9WhichCodesCount.Count AS ICD9WhichCodesCount, ICD10WhichCodesCount.Count AS ICD10WhichCodesCount, "

			//find who the 2010AA billing provider is
			"CASE WHEN ChargesT.ClaimProviderID Is Not Null THEN ChargesT.ClaimProviderID ELSE "
			"(CASE WHEN BillsT.FormType = 2 "
			"THEN (CASE WHEN UB92SetupT.Box82Setup = 2 THEN PatientProvidersT.ClaimProviderID "
			"	WHEN UB92SetupT.Box82Setup = 3 THEN ReferringPhysT.PersonID "
			"	ELSE ChargeProvidersT.ClaimProviderID END) "
			"WHEN BillsT.FormType = 1 "
			"	THEN (CASE WHEN HCFAClaimProvidersT.ANSI_2010AA_ProviderID Is Not Null "
			"		THEN HCFAClaimProvidersT.ANSI_2010AA_ProviderID "
			"		ELSE (CASE WHEN HCFASetupT.Box33Setup = 2 THEN PatientProvidersT.ClaimProviderID "
			"			ELSE ChargeProvidersT.ClaimProviderID END) "
			"		END) "
			"ELSE ChargeProvidersT.ClaimProviderID END) END AS ProviderID_2010AA, "

			//now find out who 2310B rendering provider is
			"CASE WHEN ChargesT.ClaimProviderID Is Not Null THEN ChargesT.ClaimProviderID ELSE "
			"(CASE WHEN BillsT.FormType = 2 "
			"THEN (CASE WHEN UB92SetupT.Box82Setup = 2 THEN PatientProvidersT.ClaimProviderID "
			"	WHEN UB92SetupT.Box82Setup = 3 THEN ReferringPhysT.PersonID "
			"	ELSE ChargeProvidersT.ClaimProviderID END) "
			"WHEN BillsT.FormType = 1 "
			"	THEN (CASE WHEN HCFAClaimProvidersT.ANSI_2310B_ProviderID Is Not Null "
			"		THEN HCFAClaimProvidersT.ANSI_2310B_ProviderID "
			"		ELSE (CASE WHEN HCFASetupT.Box33Setup = 2 THEN PatientProvidersT.ClaimProviderID "
			"			ELSE ChargeProvidersT.ClaimProviderID END) "
			"		END) "
			"ELSE ChargeProvidersT.ClaimProviderID END) END AS ProviderID_2310B, "

			//now find out who the Box33 billing provider is
			"CASE WHEN ChargesT.ClaimProviderID Is Not Null THEN ChargesT.ClaimProviderID ELSE "
			"(CASE WHEN BillsT.FormType = 1 "
			"	THEN (CASE WHEN HCFAClaimProvidersT.Box33_ProviderID Is Not Null "
			"		THEN HCFAClaimProvidersT.Box33_ProviderID "
			"		ELSE (CASE WHEN HCFASetupT.Box33Setup = 2 THEN PatientProvidersT.ClaimProviderID "
			"			ELSE ChargeProvidersT.ClaimProviderID END) "
			"		END) "
			"ELSE ChargeProvidersT.ClaimProviderID END) END AS ProviderID_Box33, "

			//now find out who the Box24J rendering provider is
			"CASE WHEN ChargesT.ClaimProviderID Is Not Null THEN ChargesT.ClaimProviderID ELSE "
			"(CASE WHEN BillsT.FormType = 2 "
			"THEN (CASE WHEN UB92SetupT.Box82Setup = 2 THEN PatientProvidersT.ClaimProviderID "
			"	WHEN UB92SetupT.Box82Setup = 3 THEN ReferringPhysT.PersonID "
			"	ELSE ChargeProvidersT.ClaimProviderID END) "
			"WHEN BillsT.FormType = 1 "
			"	THEN (CASE WHEN HCFAClaimProvidersT.Box24J_ProviderID Is Not Null "
			"		THEN HCFAClaimProvidersT.Box24J_ProviderID "
			"		ELSE (CASE WHEN HCFASetupT.Box33Setup = 2 THEN PatientProvidersT.ClaimProviderID "
			"			ELSE ChargeProvidersT.ClaimProviderID END) "
			"		END) "
			"ELSE ChargeProvidersT.ClaimProviderID END) END AS ProviderID_Box24J, "

			//now find out who the ANSI UB 2310B provider is (Box 77)				
			"CASE WHEN BillsT.FormType = 2 THEN "
			"	CASE WHEN UB92SetupT.UB04Box77Setup = 1 THEN "
			"		CASE WHEN ChargesT.ClaimProviderID Is Not Null THEN ChargesT.ClaimProviderID ELSE ChargeProvidersT.ClaimProviderID END "
			"	WHEN UB92SetupT.UB04Box77Setup = 2 THEN PatientProvidersT.ClaimProviderID "
			"	WHEN UB92SetupT.UB04Box77Setup = 3 THEN ReferringPhysT.PersonID "
			"	ELSE -1 END "
			"ELSE -1 END AS UB_2310B_ProviderID, "

			//now find out who the ANSI UB 2310C provider is (Box 78)				
			"CASE WHEN BillsT.FormType = 2 THEN "
			"	CASE WHEN UB92SetupT.Box83Setup = 1 THEN "
			"		CASE WHEN ChargesT.ClaimProviderID Is Not Null THEN ChargesT.ClaimProviderID ELSE ChargeProvidersT.ClaimProviderID END "
			"	WHEN UB92SetupT.Box83Setup = 2 THEN PatientProvidersT.ClaimProviderID "
			"	WHEN UB92SetupT.Box83Setup = 3 THEN ReferringPhysT.PersonID "
			"	ELSE -1 END "
			"ELSE -1 END AS UB_2310C_ProviderID "			

			"FROM HCFATrackT "
			"INNER JOIN BillsT ON HCFATrackT.BillID = BillsT.ID "
			"INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"LEFT JOIN PatientsT ON BillsT.PatientID = PatientsT.PersonID "
			"LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			"LEFT JOIN InsuredPartyT ON BillsT.InsuredPartyID = InsuredPartyT.PersonID "
			"LEFT JOIN PersonT InsuredPersonT ON InsuredPartyT.PersonID = InsuredPersonT.ID "
			"LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
			"LEFT JOIN LocationsT AS PlaceOfServiceT ON BillsT.Location = PlaceOfServiceT.ID "
			"LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
			"LEFT JOIN HCFASetupT ON InsuranceCoT.HCFASetupGroupID = HCFASetupT.ID "
			"LEFT JOIN UB92SetupT ON InsuranceCoT.UB92SetupGroupID = UB92SetupT.ID "
			"LEFT JOIN ProvidersT SupervisingProvidersT ON BillsT.SupervisingProviderID = SupervisingProvidersT.PersonID "
			"LEFT JOIN PersonT SupervisingProviderPersonT ON SupervisingProvidersT.PersonID = SupervisingProviderPersonT.ID "
			"LEFT JOIN PlaceOfServiceCodesT ON ChargesT.ServiceLocationID = PlaceOfServiceCodesT.ID "
			"LEFT JOIN InsuranceLocationPayerIDsT ON InsuranceCoT.PersonID = InsuranceLocationPayerIDsT.InsuranceCoID AND LineItemT.LocationID = InsuranceLocationPayerIDsT.LocationID "
			"LEFT JOIN EbillingInsCoIDs AS HCFALocPayerIDsT ON InsuranceLocationPayerIDsT.ClaimPayerID = HCFALocPayerIDsT.ID "
			"LEFT JOIN EbillingInsCoIDs AS UBLocPayerIDsT ON InsuranceLocationPayerIDsT.UBPayerID = UBLocPayerIDsT.ID "
			"LEFT JOIN EbillingInsCoIDs AS UBPayerIDsT ON InsuranceCoT.UBPayerID = UBPayerIDsT.ID "
			"LEFT JOIN EbillingInsCoIDs AS HCFAPayerIDsT ON InsuranceCoT.HCFAPayerID = HCFAPayerIDsT.ID "
			"LEFT JOIN ProvidersT ChargeProvidersT ON ChargesT.DoctorsProviders = ChargeProvidersT.PersonID "
			"LEFT JOIN ProvidersT PatientProvidersT ON PatientsT.MainPhysician = PatientProvidersT.PersonID "
			"LEFT JOIN ReferringPhysT ON BillsT.RefPhyID = ReferringPhysT.PersonID "
			"LEFT JOIN HCFAClaimProvidersT ON InsuranceCoT.PersonID = HCFAClaimProvidersT.InsuranceCoID AND (CASE WHEN HCFASetupT.Box33Setup = 2 THEN PatientProvidersT.PersonID ELSE ChargeProvidersT.PersonID END) = HCFAClaimProvidersT.ProviderID "
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
			"LEFT JOIN (SELECT BillID, SUM(CASE WHEN ICD10DiagID IS NULL THEN 0 ELSE 1 END) AS Count FROM BillDiagCodeT GROUP BY BillID) ICD10Count ON BillsT.ID = ICD10Count.BillID "
			"LEFT JOIN (SELECT BillID, SUM(CASE WHEN ICD9DiagID IS NULL THEN 0 ELSE 1 END) AS Count FROM BillDiagCodeT GROUP BY BillID) ICD9Count ON BillsT.ID = ICD9Count.BillID "
			"LEFT JOIN (SELECT ChargeID, COUNT(BillDiagCodeID) AS Count FROM ChargeWhichCodesT "
			"	INNER JOIN BillDiagCodeT ON ChargeWhichCodesT.BillDiagCodeID = BillDiagCodeT.ID AND ICD10DiagID IS NOT NULL GROUP BY ChargeID) ICD10WhichCodesCount ON ChargesT.ID = ICD10WhichCodesCount.ChargeID "
			"LEFT JOIN (SELECT ChargeID, COUNT(BillDiagCodeID) AS Count FROM ChargeWhichCodesT "
			"	INNER JOIN BillDiagCodeT ON ChargeWhichCodesT.BillDiagCodeID = BillDiagCodeT.ID AND ICD9DiagID IS NOT NULL GROUP BY ChargeID) ICD9WhichCodesCount ON ChargesT.ID = ICD9WhichCodesCount.ChargeID "
			"LEFT JOIN (SELECT BillID FROM ClaimHistoryT "
			//(r.wilson 10/2/2012) plid 52970 - below old SendType value was 0
			"	WHERE SendType = {INT} AND dbo.AsDateNoTime(Date) = dbo.AsDateNoTime(GetDate()) "
			"	GROUP BY BillID) AS AlreadyExportedTodayQ ON BillsT.ID = AlreadyExportedTodayQ.BillID "
			"WHERE BillsT.Deleted = 0 AND LineItemT.Deleted = 0 AND Batched = 1 "
			"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
			"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
			"AND HCFATrackT.ID = {INT}", ClaimSendType::Electronic ,HCFAID);

		if(rsClaim->eof) {

			//Bill - this is NOT a warning that can be disabled

			str.Format("\tERROR - No Valid Bill Was Found (Claim ID: %li)\r\n\r\n"
				"\t\tEither all the charges have been deleted off of this bill,\r\n"
				"\t\tall the charges have been unbatched from this bill,\r\n"
				"\t\tor this bill no longer exists.",HCFAID);
			strErrors += str;
			bPassed = FALSE;

			// (j.jones 2009-03-02 11:07) - PLID 33286 - return now, we can't check anything else
			m_strInvalidClaims += strErrors;

			ExecuteParamSql("UPDATE HCFATrackT SET ValidationState = 2 WHERE ID = {INT}", HCFAID);

			return FALSE;
		}
		else {

			nLocationID = AdoFldLong(rsClaim, "LocationID",-1);

			nPatientID = AdoFldLong(rsClaim, "PersonID",-1);

			//Patient - this is NOT a warning that can be disabled
			if(nPatientID == -1) {				
				str.Format("\tERROR - No Patient Was Found (Claim ID: %li)\r\n\r\n"
					"\t\tThere is no patient associated with this claim.\r\n"
					"\t\tYou will need to recreate this claim.\r\n\r\n\r\n",HCFAID);
				strErrors += str;
				bPassed = FALSE;
			}

			//it is highly unlikely that the above two errors will ever occur. If they do, we can't get the patient name.
			//but now we can, so generate this patient's report header.
			//In the event there are no errors, strErrors is cleared out at the end of this function

			CString strPatientName = AdoFldString(rsClaim, "PatientName","");
			CString strDescription = AdoFldString(rsClaim, "Description","");
			strLocationEIN = AdoFldString(rsClaim, "LocEIN","");

			str.Format("____________________________________________________________________________________\r\n\r\n"
				"Patient:  %s\t\r\n\r\nBill Desc: %s     Bill Date: %s\r\n\r\n",
				strPatientName, strDescription, FormatDateTimeForInterface(AdoFldDateTime(rsClaim, "Date")));
			strErrors += str;

			nBillID = AdoFldLong(rsClaim, "BillID",-1);

			// (j.jones 2008-02-12 14:53) - PLID 28847 - included a silent call to CanCreateInsuranceClaim
			// (j.jones 2010-10-21 15:06) - PLID 41051 - CanCreateInsuranceClaim changed to issue its own warnings,
			// but they are silent during this call
			if(!CanCreateInsuranceClaim(nBillID, TRUE)) {

				str.Format("\tERROR - Claim Has No Batched Charges\r\n\r\n"
					"\t\tNo charges on this bill are marked as batched.\r\n"
					"\t\tThis could be due to having no insurance responsibility on this claim,\r\n"
					"\t\tand your preferences configured such that patient-only claims\r\n"
					"\t\tshould not be exported.\r\n\r\n\r\n");
				strErrors += str;
				bPassed = FALSE;

				m_strInvalidClaims += strErrors;

				ExecuteParamSql("UPDATE HCFATrackT SET ValidationState = 2 WHERE ID = {INT}", HCFAID);

				return FALSE;
			}

			//Insurance Company - this is NOT a warning that can be disabled
			InsuranceCoID = AdoFldLong(rsClaim, "InsCoID",-1);
			nInsuredPartyID = AdoFldLong(rsClaim, "InsuredPartyID",-1);
			if(InsuranceCoID == -1) {				
				str.Format("\tERROR - No Insurance Company Selected\r\n\r\n"
					"\t\tThere is no Insurance Company selected on this claim.\r\n"
					"\t\tCheck the patient's account and ensure that an\r\n"
					"\t\tinsurance company is selected on the Insurance tab of\r\n"
					"\t\tthe Bill. If no insurance companies are available,\r\n"
					"\t\tyou must add one in the patient's Insurance tab.\r\n\r\n\r\n");
				strErrors += str;
				bHasInsurance = FALSE;
				bPassed = FALSE;
			}

			//store some variables for later
			if(bHasInsurance) {
				strInsCoName = AdoFldString(rsClaim, "InsCoName", "");
				// (j.jones 2009-12-17 09:53) - PLID 36237 - supported UB payer IDs
				strHCFAPayerID = AdoFldString(rsClaim, "HCFAPayerID", "");
				strHCFAPayerID.TrimLeft();
				strHCFAPayerID.TrimRight();
				strUBPayerID = AdoFldString(rsClaim, "UBPayerID", "");
				strUBPayerID.TrimLeft();
				strUBPayerID.TrimRight();
				// (j.jones 2009-08-04 12:34) - PLID 14573 - removed THIN ID
				//strTHINID = AdoFldString(rs, "THIN", "");
				// (j.jones 2008-09-09 11:20) - PLID 18695 - supported insurance type code
				eInsType = (InsuranceTypeCode)AdoFldLong(rsClaim, "InsType", (long)itcInvalid);
				Box33 = AdoFldLong(rsClaim, "Box33",2);
				nHCFABox25 = AdoFldLong(rsClaim, "Box25",2);
				nBox82Num = AdoFldLong(rsClaim, "Box82Num",-1);
				nBox83Num = AdoFldLong(rsClaim, "Box83Num",-1);
				nUB04Box77Num = AdoFldLong(rsClaim, "UB04Box77Num",-1);

				// (j.jones 2011-08-23 12:31) - PLID 45141 - added UB 2310B and 2310C provider ID
				nUB_2310B_ProviderID = AdoFldLong(rsClaim, "UB_2310B_ProviderID", -1);
				nUB_2310C_ProviderID = AdoFldLong(rsClaim, "UB_2310C_ProviderID", -1);

				LocationNPIUsage = AdoFldLong(rsClaim, "LocationNPIUsage",-1);
				// (j.jones 2008-06-10 15:11) - PLID 25834 - added ValidateRefPhy
				nValidateRefPhy = AdoFldLong(rsClaim, "ValidateRefPhy",0);
				HCFAProvType = AdoFldLong(rsClaim, "Box33Setup",-1);
				// (j.jones 2010-10-20 13:50) - PLID 40936 - loaded Box33Num
				Box33Num = AdoFldLong(rsClaim, "Box33Num",-1);
				DocAddress = AdoFldLong(rsClaim, "DocAddress", 1);
				// (j.jones 2010-07-23 12:05) - PLID 39797 - added Box13Accepted and HCFABox13Over
				nBox13Accepted = AdoFldLong(rsClaim, "Box13Accepted",3);
				hb13Value = (HCFABox13Over)AdoFldLong(rsClaim, "HCFABox13Over", (long)hb13_UseDefault);
				// (j.jones 2010-07-27 11:49) - PLID 39784 - added UBBox53Accepted
				nUBBox53Accepted = AdoFldLong(rsClaim, "Box53Accepted",1);
				nBox82Setup = AdoFldLong(rsClaim, "Box82Setup",-1);
				nBox83Setup = AdoFldLong(rsClaim, "Box83Setup",-1);
				nUB04Box77Setup = AdoFldLong(rsClaim, "UB04Box77Setup",-1);
				strBox33bQual = AdoFldString(rsClaim, "Box33bQual", "");
				strBox33bQual.TrimRight();
				strUB04Box76Qual = AdoFldString(rsClaim, "UB04Box76Qual", "");
				strUB04Box76Qual.TrimRight();
				strUB04Box77Qual = AdoFldString(rsClaim, "UB04Box77Qual", "");
				strUB04Box77Qual.TrimRight();
				strUB04Box78Qual = AdoFldString(rsClaim, "UB04Box78Qual", "");
				strUB04Box78Qual.TrimRight();
				// (j.jones 2009-09-11 11:51) - PLID 34540 - added RelationToPatient and InsuredName
				strRelationToPatient = AdoFldString(rsClaim, "RelationToPatient", "");
				strRelationToPatient.TrimRight();
				strInsuredName = AdoFldString(rsClaim, "InsuredName", "");

				if(AdoFldLong(rsClaim, "FormType",1) == 2) {
					bIsUB92 = TRUE;
				}

				// (j.jones 2010-04-14 12:22) - PLID 27457 - load the nExtraREF_IDType field
				if(bIsUB92) {
					nExtraREF_IDType = AdoFldLong(rsClaim, "UB_ExtraREF_IDType",2);
				}
				else {
					nExtraREF_IDType = AdoFldLong(rsClaim, "HCFA_ExtraREF_IDType",2);
				}

				// (j.jones 2010-11-10 14:33) - PLID 41396 - load all possible provider IDs
				nProviderID_2010AA = AdoFldLong(rsClaim, "ProviderID_2010AA",-1);
				nProviderID_2310B = AdoFldLong(rsClaim, "ProviderID_2310B",-1);
				nProviderID_Box33 = AdoFldLong(rsClaim, "ProviderID_Box33",-1);
				nProviderID_Box24J = AdoFldLong(rsClaim, "ProviderID_Box24J",-1);

				//assign these to the proper IDs for this claim style
				if(m_bElectronic && m_FormatStyle == ANSI) {
					nBillingProviderID = nProviderID_2010AA;
					nRenderingProviderID = nProviderID_2310B;
				}
				else {
					//use the paper settings
					nBillingProviderID = nProviderID_Box33;
					nRenderingProviderID = nProviderID_Box24J;
				}

				if(bIsUB92) {
					//we do not support a different rendering provider ID
					//for UBs, so ensure we didn't load one
					nRenderingProviderID = nBillingProviderID;
				}

				// (j.jones 2011-09-16 16:01) - PLID 39789 - added checks for claim type / original ref. num
				eClaimTypeCode = (ANSI_ClaimTypeCode)AdoFldLong(rsClaim, "ANSI_ClaimTypeCode", (long)ctcOriginal);

				if(m_bElectronic && m_FormatStyle == ANSI && !bIsUB92) {
					if(eClaimTypeCode != ctcOriginal) {
						//if the claim type is not original, then they need to see a reference number,
						//which is either OriginalRefNo or MedicaidResubmission, it does not matter which one

						strOriginalRefNum = AdoFldString(rsClaim, "OriginalRefNo", "");
						strMedicaidResubmissionNum = AdoFldString(rsClaim, "MedicaidResubmission", "");

						strOriginalRefNum.TrimLeft(); strOriginalRefNum.TrimRight();
						strMedicaidResubmissionNum.TrimLeft(); strMedicaidResubmissionNum.TrimRight();

						if(strOriginalRefNum.IsEmpty() && strMedicaidResubmissionNum.IsEmpty()) {

							// (j.jones 2016-05-24 14:21) - NX-100704 - this logic is now in a global function
							CString strTypeCode = GetClaimTypeCode_HCFA(eClaimTypeCode);

							//warn that they have no reference number - this is not a skippable validation
							// (j.jones 2013-08-07 10:09) - PLID 58042 - the label in the bill is now just "Resubmission Code"
							str.Format("\tWARNING - No Reference Number Found\r\n\r\n"
								"\t\tThe Claim Type on the insurance tab of this bill is set to '%s',\r\n"
								"\t\tbut there is no reference number filled in.\r\n"
								"\t\tTo fix this, go to the insurance tab of this bill and fill in either\r\n"
								"\t\tthe Original Reference Number or Resubmission Code with the ID\r\n"
								"\t\tof the claim you are changing. Typically, this is the Bill ID.\r\n\r\n\r\n", strTypeCode);
							strErrors += str;
							bWarnings = TRUE;
						}
					}
				}

				// (j.jones 2016-05-24 15:37) - NX-100706 - added a UB variation to the claim type check
				if (bIsUB92) {
					if (eClaimTypeCode != ctcOriginal) {
						//if the claim type is not original, then they need to see a reference number,
						//which is only OriginalRefNo for UB paper claims, so yell at them about that field

						strOriginalRefNum = AdoFldString(rsClaim, "OriginalRefNo", "");

						strOriginalRefNum.TrimLeft(); strOriginalRefNum.TrimRight();

						if (strOriginalRefNum.IsEmpty()) {

							CString strTypeCode = GetClaimTypeCode_UB(eClaimTypeCode);

							//warn that they have no reference number - this is not a skippable validation
							str.Format("\tWARNING - No Reference Number Found\r\n\r\n"
								"\t\tThe Claim Type on the insurance tab of this bill is set to '%s',\r\n"
								"\t\tbut there is no reference number filled in.\r\n"
								"\t\tTo fix this, go to the insurance tab of this bill and fill in either\r\n"
								"\t\tthe Original Reference Number with the ID of the claim you are changing.\r\n"
								"\t\tTypically, this is the Bill ID.\r\n\r\n\r\n", strTypeCode);
							strErrors += str;
							bWarnings = TRUE;
						}
					}
				}

				// (j.jones 2011-09-23 16:07) - PLID 39377 - added validation for duplicate same-day ebilling claims
				if(m_bElectronic && AdoFldBool(rsClaim, "AlreadyExportedToday", FALSE)
					&& GetRemotePropertyInt("EbillingValidateExportedTwiceInSameDay",1,0,"<None>",TRUE) == 1) {

					str.Format("\tWARNING - Claim Exported Twice In The Same Day\r\n\r\n"
						"\t\tThis bill has already been exported electronically today.\r\n"
						"\t\tTo avoid a duplicate claim rejection, you should not export\r\n"
						"\t\tthis bill unless you failed to submit the previous export\r\n"
						"\t\tto your clearinghouse.\r\n\r\n\r\n");
					strErrors += str;
					bWarnings = TRUE;
				}
			}

			// (j.jones 2013-04-25 11:18) - PLID 55564 - get the place of service code
			CString strPOSCode = AdoFldString(rsClaim, "POSCode", "");
			if(m_bElectronic && m_FormatStyle == ANSI && bHide2310C_WhenType11Or12
				&& (strPOSCode == "11" || strPOSCode == "12")) {

				// They want to hide the POS when the place of service code is 11 or 12, and it is.
				// So do not bother validating it.
				bSkipPOSValidation = TRUE;
			}

			//we do not export POS for ANSI UB claims, so don't validate
			if(m_bElectronic && m_FormatStyle == ANSI && bIsUB92) {
				bSkipPOSValidation = TRUE;
			}

			nPOSID = AdoFldLong(rsClaim, "POSID",-1);

			if(!bSkipPOSValidation) {
				//Place Of Service - this is NOT a warning that can be disabled
				//This should actually be impossible in our data structure.
				if(nPOSID == -1) {				
					str.Format("\tERROR - No Place Of Service Selected\r\n\r\n"
						"\t\tThere is no Place Of Service selected on this claim.\r\n"
						"\t\tCheck the patient's account and ensure that a place\r\n"
						"\t\tof service is selected on the lower left-hand side\r\n"
						"\t\tof the Bill. In addition, you should check the\r\n"
						"\t\tPlace Of Service designation on the Bill as well.\r\n\r\n\r\n");
					strErrors += str;
					bPassed = FALSE;
				}
				else if(m_bElectronic && m_FormatStyle == ANSI && m_avANSIVersion == av5010) {
					// (j.jones 2010-10-20 11:59) - PLID 40936 - if a place of service is selected,
					// verify its zip code is 9 digits			
					// (j.jones 2013-04-25 10:02) - PLID 55564 - we send the patient address as the POS
					//if strPOSCode is 12 and the Claim_SendPatAddressWhenPOS12 preference is on
					CString strRealZip = "";
					if(strPOSCode == "12" && GetRemotePropertyInt("Claim_SendPatAddressWhenPOS12", 1, 0, "<None>", true) == 1) {
						bSendPatientAddressAsPOS = TRUE;
						strRealZip = AdoFldString(rsClaim, "PatientZip", "");
					}
					else {
						strRealZip = AdoFldString(rsClaim, "POSZIP", "");
					}
					strRealZip.TrimLeft(); strRealZip.TrimRight();
					CString strZipToCheck = strRealZip;
					StripNonNumericChars(strZipToCheck);
					if(strRealZip.IsEmpty()) {
						// (j.jones 2013-04-25 11:27) - PLID 55564 - if sending the patient's address, ensure
						// the warning says so
						if(bSendPatientAddressAsPOS) {
							str = "\tERROR - Missing Patient Zip Code\r\n\r\n"
								"\t\tThe Place Of Service Designation selected on this claim is 12,\r\n"
								"\t\tand your preferences are configured to send the patient's address\r\n"
								"\t\tas the Place Of Service when the designation of 12 is used.\r\n"
								"\t\tThis patient has no zip code entered. Place Of Service Zip Codes must be 9 digits in length.\r\n"
								"\t\tPlease edit the patient's address in General 1 to correct this Zip Code.\r\n\r\n\r\n";
						}
						else {
							str.Format("\tERROR - Missing Place Of Service Zip Code\r\n\r\n"
								"\t\tThe Place Of Service selected on this claim, '%s',\r\n"
								"\t\thas no zip code entered. Place Of Service Zip Codes must be 9 digits in length.\r\n"
								"\t\tPlease go to the Locations tab of the Administrator module to correct this Zip Code.\r\n\r\n\r\n",
								AdoFldString(rsClaim, "POSName", ""));
						}
						strErrors += str;
						bPassed = FALSE;
					}
					else if(strZipToCheck.GetLength() != 9) {
						// (j.jones 2013-04-25 11:27) - PLID 55564 - if sending the patient's address, ensure
						// the warning says so
						if(bSendPatientAddressAsPOS) {
							str.Format("\tERROR - Invalid Patient Zip Code\r\n\r\n"
								"\t\tThe Place Of Service Designation selected on this claim is 12,\r\n"
								"\t\tand your preferences are configured to send the patient's address\r\n"
								"\t\tas the Place Of Service when the designation of 12 is used.\r\n"
								"\t\tThis patient has a zip code of '%s'. Place Of Service Zip Codes must be 9 digits in length.\r\n"
								"\t\tPlease edit the patient's address in General 1 to correct this Zip Code.\r\n\r\n\r\n",
								strRealZip);
						}
						else {
							str.Format("\tERROR - Invalid Place Of Service Zip Code\r\n\r\n"
								"\t\tThe Place Of Service selected on this claim, '%s',\r\n"
								"\t\thas a zip code of '%s'. Place Of Service Zip Codes must be 9 digits in length.\r\n"
								"\t\tPlease go to the Locations tab of the Administrator module to correct this Zip Code.\r\n\r\n\r\n",
								AdoFldString(rsClaim, "POSName", ""), strRealZip);
						}
						strErrors += str;
						bPassed = FALSE;
					}
				}
			}

			//Insurance Company must be in a HCFA Group - this is NOT a warning that can be disabled
			if(bHasInsurance) {

				HCFASetupGroupID = AdoFldLong(rsClaim, "HCFASetupGroupID",-1);
				if(HCFASetupGroupID == -1 && !bIsUB92) {
					str.Format("\tERROR - Insurance Company is not in a HCFA Group\r\n\r\n"
						"\t\tThe Insurance Company selected on this Bill is not configured\r\n"
						"\t\tin a HCFA Group. You must configure each Insurance Company in\r\n"
						"\t\tthe HCFA Tab of the Administrator module. If you need\r\n"
						"\t\tassistance in this tab, look in the Nextech Help for\r\n"
						"\t\tmore information on configuring HCFA Groups.\r\n\r\n\r\n");
					strErrors += str;
					bPassed = FALSE;
				}
				strDefaultBox33GRP = AdoFldString(rsClaim, "DefaultBox33GRP", "");

				UB92SetupGroupID = AdoFldLong(rsClaim, "UB92SetupGroupID",-1);
				if(UB92SetupGroupID == -1 && bIsUB92) {
					//TES 3/13/2007 - PLID 24993 - Changed from UB92 to UB
					str.Format("\tERROR - Insurance Company is not in a UB Group\r\n\r\n"
						"\t\tThe Insurance Company selected on this Bill is not configured\r\n"
						"\t\tin a UB Group. You must configure each Insurance Company in\r\n"
						"\t\tthe UB Tab of the Administrator module. If you need\r\n"
						"\t\tassistance in this tab, look in the Nextech Help for\r\n"
						"\t\tmore information on configuring UB Groups.\r\n\r\n\r\n");
					strErrors += str;
					bPassed = FALSE;
				}

				// (j.jones 2009-09-11 11:51) - PLID 34540 - if ANSI, check and see if the RelationToPatient
				// is Self but the patient and insured names are not identical
				// (j.jones 2010-01-07 09:18) - PLID 36786 - this is now disableable
				// (j.jones 2013-02-26 09:40) - PLID 55325 - corrected the wording to state that this is the main insured party selected
				// on the bill, it may or may not be the patient's Primary insurance
				BOOL bEbillingValidatePatientNameWhenSelf = GetRemotePropertyInt("EbillingValidatePatientNameWhenSelf",1,0,"<None>",TRUE) == 1;
				if(bEbillingValidatePatientNameWhenSelf && m_FormatStyle == ANSI
					&& strRelationToPatient == "Self" && strPatientName.CompareNoCase(strInsuredName) != 0) {
					str.Format("\tERROR - Relationship to Insured is 'Self', and Insured name does not match\r\n\r\n"
						"\t\tThe insured party selected on the bill has an insured name of '%s'.\r\n"
						"\t\tThis does not match the patient name, but the Patient Relationship to the Insured Party is 'Self'.\r\n"
						"\t\tA claim generated for this insured party will indicate to the insurance company that the\r\n"
						"\t\tinsured party is the patient, and not submit the patient information.\r\n"
						"\t\tTo fix this, go to the patient's insurance tab and make sure the insured name is correct and\r\n"
						"\t\tthat the Patient Relationship to the Insured Party field is accurate for ALL of their\r\n"
						"\t\tinsurance companies.\r\n\r\n\r\n", strInsuredName);
					strErrors += str;
					bPassed = FALSE;
				}

				// (j.jones 2012-05-01 09:59) - PLID 48530 - added prior auth. number validation, only checked if
				// the insurance company prompts for insurance referrals
				BOOL bEbillingValidatePriorAuthNumber = GetRemotePropertyInt("EbillingValidatePriorAuthNumber",1,0,"<None>",TRUE) == 1;
				if(bEbillingValidatePriorAuthNumber && VarBool(rsClaim->Fields->Item["UseReferrals"]->Value, FALSE)) {
					//the validation is enabled and this insurance company requires referrals, so check the prior authorization number
					CString strPriorAuthNum = VarString(rsClaim->Fields->Item["PriorAuthNum"]->Value, "");

					//the HCFA has a default override that is used if the bill's number is blank
					//(no code that uses this number trims the bill value before checking for emptiness, so we won't trim here as well)
					if(!bIsUB92 && strPriorAuthNum.IsEmpty()) {
						strPriorAuthNum = VarString(rsClaim->Fields->Item["DefaultPriorAuthNum"]->Value, "");										
					}

					//now trim, and warn if empty
					strPriorAuthNum.TrimLeft(); strPriorAuthNum.TrimRight();
					if(strPriorAuthNum.IsEmpty()) {
						str.Format("\tERROR - No Prior Authorization Number Found\r\n\r\n"
							"\t\tThis patient's bill does not have a Prior Authorization Number entered,\r\n"
							"\t\tand the insurance company '%s' requires insurance referrals.\r\n"
							"\t\tTo fix this, edit the patient's bill and enter a Prior Authorization Number\r\n"
							"\t\tin the Insurance tab.\r\n\r\n\r\n",
							strInsCoName);
						strErrors += str;
						bPassed = FALSE;
					}
				}

				// (j.jones 2014-07-17 14:11) - PLID 62931 - Added a warning for 439 - Accident Date used without a valid condition relation type,
				// unlike the validation below this, this is not something that can be disabled, because this is required on a claim.
				// This only applies to the new paper HCFA, or ANSI 5010 HCFA claims.
				bool bRelatedToAuto = (AdoFldBool(rsClaim, "RelatedToAutoAcc") == 1);
				bool bRelatedToOther = (AdoFldBool(rsClaim, "RelatedToOther") == 1);
				if (!bIsUB92 && ((!m_bElectronic && UseNewHCFAForm(nInsuredPartyID, nBillID)) || (m_bElectronic && m_FormatStyle == ANSI && m_avANSIVersion == av5010))) {
					// (j.jones 2016-04-08 09:00) - NX-100070 - this is now BillsT.AccidentDate
					COleDateTime dtAccidentDate = AdoFldDateTime(rsClaim, "AccidentDate", g_cdtNull);
					ConditionDateType cdtConditionDateType = (ConditionDateType)VarLong(rsClaim->Fields->Item["ConditionDateType"]->Value);
					if (cdtConditionDateType == cdtAccident439 && dtAccidentDate.GetStatus() == COleDateTime::valid
						&& !bRelatedToAuto && !bRelatedToOther) {
						//they entered a date and selected 439 - Accident Date, but did not select Auto Accident or Other
						//as the condition relation, which will likely cause the claim to reject

						str.Format("\tERROR - No Valid Condition Relation for Accident Date\r\n\r\n"
							"\t\tThis patient's bill has an Accident Date filled in, but does not have \r\n"
							"\t\ta Condition Relation of Auto Accident or Other selected.\r\n"
							"\t\tSubmitting a claim with an Accident Date without Auto Accident or Other\r\n"
							"\t\tselected may cause the claim to be rejected.\r\n"
							"\t\tTo fix this, edit the patient's bill and select Auto Accident or Other\r\n"
							"\t\tas the Condition Relation in the Insurance tab.\r\n\r\n\r\n");
						strErrors += str;
						bPassed = FALSE;
					}
				}

				// (a.wilson 2013-03-26 15:32) - PLID 51773 - warn if they don't have an accident type but have a date of current accident.
				bool bEbillingValidateAccTypeForAccDate = (GetRemotePropertyInt("EbillingValidateAccidentTypeForCurrentAccidentDate", 1, 0, "<None>", true) == 1);
				if (!bIsUB92 && bEbillingValidateAccTypeForAccDate) {	//HCFA Only Issue
					bool bPullFromInsurance = (AdoFldLong(rsClaim, "PullCurrentAccInfo", 1) == 2);
					if (bPullFromInsurance) {
						COleDateTime dtCurrentAccident = AdoFldDateTime(rsClaim, "DateOfCurAcc", g_cdtNull);
						InsuredPartyAccidentType ipat = (InsuredPartyAccidentType)AdoFldLong(rsClaim, "AccidentType");
						//if the date is filled with no accident type then we have an error.
						if (dtCurrentAccident.GetStatus() != COleDateTime::null && ipat == ipatNone)
						{
							//error
							str.Format("\tERROR - No Insured Party Condition Relation for Date of Current Illness / Injury\r\n\r\n"
								"\t\tThe insured party on the bill does not have a Condition Relation selected,\r\n"
								"\t\tbut the insured party does have a Current Accident Date filled in.\r\n"
								"\t\tTo fix this, edit the patient's insured party and select a Condition Relation.\r\n\r\n\r\n");
							strErrors += str;
							bPassed = FALSE;
						}
					} else {
						// (j.jones 2013-08-16 09:37) - PLID 58063 - this field is no longer controlled by ConditionDateType
						COleDateTime dtCurrentAccident = AdoFldDateTime(rsClaim, "ConditionDate", g_cdtNull);
						//the other two condition types were loaded earlier
						bool bRelatedToEmp = (AdoFldBool(rsClaim, "RelatedToEmp") == 1);
						//if the date is filled with no accident type then we have an error.
						if (dtCurrentAccident.GetStatus() != COleDateTime::null && !bRelatedToEmp && !bRelatedToAuto && !bRelatedToOther)
						{
							//error
							str.Format("\tERROR - No Condition Relation for Date of Current Illness / Injury\r\n\r\n"
								"\t\tThis patient's bill does not have a Condition Relation selected,\r\n"
								"\t\tbut the bill does have a Date of Current Illness / Injury filled in.\r\n"
								"\t\tTo fix this, edit the patient's bill and select a Condition Relation\r\n"
								"\t\tin the Insurance tab.\r\n\r\n\r\n");
							strErrors += str;
							bPassed = FALSE;
						}
					}
				}

				// (d.singleton 2014-03-07 08:18) - PLID 61235 - if ins company requires icd9 codes and none are present on bill need to have validation error in ebilling
				bool bRequiresICD9 = (GetRemotePropertyInt("EbillingValidateRequiresICD9", 1, 0, "<None>", true) == 1);
				if(bRequiresICD9) {
					//first need to find out if they require icd9
					if(!ShouldUseICD10(nInsuredPartyID, nBillID)) {
						//they require icd9, do any icd9 codes exist on bill
						long nICD9Count;
						if(GetRemotePropertyInt("SkipUnlinkedDiagsOnClaims", 0, 0, "<None>", true) == 1) {
							nICD9Count = AdoFldLong(rsClaim, "ICD9WhichCodesCount", -1);
						}
						else {							
							nICD9Count = AdoFldLong(rsClaim, "ICD9Codes", -1);
						}
						if(nICD9Count < 1) {
							//no codes, error
							str.Format("\tERROR - No ICD-9 codes Found\r\n\r\n"
								"\t\tThe Insurance Company selected on this Bill:\r\n"
								"\t\t'%s'\r\n"
								"\t\trequires the use of ICD-9 codes.\r\n"
								"\t\tTo fix this, edit the patient's bill associated with this claim\r\n"
								"\t\tand add the needed ICD-9 codes.\r\n\r\n\r\n", strInsCoName);
							strErrors += str;
							bPassed = FALSE;
						}
					}
				}
				
				// (d.singleton 2014-03-07 08:35) - PLID 61236 - if ins company requires icd10 codes and none are present on bill need to have validation error in ebilling
				bool bRequiresICD10 = (GetRemotePropertyInt("EbillingValidateRequiresICD10", 1, 0, "<None>", true) == 1);
				if(bRequiresICD10) {
					//first need to find out if they require icd10
					if(ShouldUseICD10(nInsuredPartyID, nBillID)) {
						//they require icd10, do any icd10 codes exist on bill
						long nICD10Count;
						if(GetRemotePropertyInt("SkipUnlinkedDiagsOnClaims", 0, 0, "<None>", true) == 1) {
							nICD10Count = AdoFldLong(rsClaim, "ICD10WhichCodesCount", -1);
						}
						else {
							nICD10Count = AdoFldLong(rsClaim, "ICD10Codes", -1);
						}
						if(nICD10Count < 1) {
							//no codes, error
							str.Format("\tERROR - No ICD-10 codes Found\r\n\r\n"
								"\t\tThe Insurance Company selected on this Bill:\r\n"
								"\t\t'%s'\r\n"
								"\t\trequires the use of ICD-10 codes.\r\n"
								"\t\tTo fix this, edit the patient's bill associated with this claim\r\n"
								"\t\tand add the needed ICD-10 codes.\r\n\r\n\r\n", strInsCoName);
							strErrors += str;
							bPassed = FALSE;
						}
					}
				}
			}

			// (j.jones 2008-12-11 16:13) - PLID 32414 - added a check for the Supervising Provider's NPI, using the existing preferences
			// (j.jones 2009-03-02 10:43) - PLID 33286 - moved this to correctly be inside the !rs->eof block
			long nSupervisingProviderID = AdoFldLong(rsClaim, "SupervisingProviderID", -1);
			//only validate if we have a supervising provider
			if(nSupervisingProviderID != -1 && m_bElectronic && m_FormatStyle == ANSI && !bIsUB92) {
				CString strSupervisingProviderNPI = AdoFldString(rsClaim, "SupervisingProviderNPI", "");
				CString strSupervisingProviderName = AdoFldString(rsClaim, "SupervisingProviderName", "");
				//is the NPI blank?
				if(GetRemotePropertyInt("EbillingValidateProviderNPI",1,0,"<None>",TRUE) && strSupervisingProviderNPI.IsEmpty()) {

					str.Format("\tERROR - No Supervising Provider NPI Found\r\n\r\n"
						"\t\tThe Supervising Provider on this patient's bill, '%s', does not have a\r\n"
						"\t\tNPI number set up properly. To fix this, go to the contacts module for this provider\r\n"
						"\t\tand fill in the NPI field.\r\n\r\n\r\n",
						strSupervisingProviderName);
					strErrors += str;
					bPassed = FALSE;
				}
				//if not blank, is it valid?
				else if (GetRemotePropertyInt("EbillingValidateNPIChecksum",1,0,"<None>",TRUE) && !IsValidNPI(strSupervisingProviderNPI)) {
					str.Format("\tERROR - Supervising Provider NPI is Invalid\r\n\r\n"
						"\t\tThe Supervising Provider on this patient's bill, '%s', does not have a\r\n"
						"\t\tvalid NPI number (%s). To fix this, go to the contacts module for\r\n"
						"\t\tthis provider and check your records to confirm the accuracy of the NPI field.\r\n\r\n\r\n",
						strSupervisingProviderName, strSupervisingProviderNPI);
					strErrors += str;
					bPassed = FALSE;
				}
			}
		}		

		// (j.jones 2010-10-20 13:39) - PLID 40936 - in 5010 we need to verify that the 2010AA zip code
		// is 9 digits, but quite a lot can control what is output in 2010AA
		// (j.jones 2011-11-16 16:42) - PLID 46489 - this is no longer a Box33 override, it's a unique 2010AA override
		if(m_bElectronic && m_FormatStyle == ANSI && m_avANSIVersion == av5010) {
			
			CString strRealZip = "";
			CString strLoadedFrom = "";

			if(!bIsUB92) {
				_RecordsetPtr rs2010AAOver = CreateParamRecordset("SELECT PayTo2010AA_Zip "
					"FROM EbillingSetupT WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT} "
					"AND PayTo2010AA_Zip Is Not Null AND PayTo2010AA_Zip <> ''",
					nProviderID_2010AA, nLocationID, HCFASetupGroupID);
				if(!rs2010AAOver->eof) {
					strRealZip = AdoFldString(rs2010AAOver, "PayTo2010AA_Zip", "");
					strRealZip.TrimLeft(); strRealZip.TrimRight();
					if(!strRealZip.IsEmpty()) {
						strLoadedFrom = "the HCFA Setup's Adv. ID Setup override";
					}
				}
				rs2010AAOver->Close();
			}
			else {
				_RecordsetPtr rs2010AAOver = CreateParamRecordset("SELECT PayTo2010AA_Zip "
					"FROM UB92EbillingSetupT WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT} "
					"AND PayTo2010AA_Zip Is Not Null AND PayTo2010AA_Zip <> ''",
					nProviderID_2010AA, nLocationID, UB92SetupGroupID);
				if(!rs2010AAOver->eof) {
					strRealZip = AdoFldString(rs2010AAOver, "PayTo2010AA_Zip", "");
					strRealZip.TrimLeft(); strRealZip.TrimRight();
					if(!strRealZip.IsEmpty()) {
						strLoadedFrom = "the HCFA Setup's Adv. ID Setup override";
					}
				}
				rs2010AAOver->Close();
			}

			if(strRealZip.IsEmpty()) {
				if(bIsGroup || bIsUB92 || HCFAProvType == 4 || (!bIsUB92 && HCFAProvType != 3 && DocAddress == 1)) {
					//in all these cases, we send the location zip code
					strRealZip = AdoFldString(rsClaim, "LocZIP", "");
					strLoadedFrom = "the bill's location";
				}
				else if(!bIsUB92 && HCFAProvType == 3 && Box33Num != -1) {
					//this is the Box33 Override
					_RecordsetPtr rsZip = CreateParamRecordset("SELECT Zip FROM PersonT WHERE PersonT.ID = {INT}", Box33Num);
					if(!rsZip->eof) {
						strRealZip = AdoFldString(rsZip, "Zip", "");
					}
					strLoadedFrom = "the Box 33 override configured in the HCFA Setup";
					rsZip->Close();
				}
				else {
					//use the 2010AA provider
					_RecordsetPtr rsZip = CreateParamRecordset("SELECT Zip FROM PersonT WHERE PersonT.ID = {INT}", nProviderID_2010AA);
					if(!rsZip->eof) {
						strRealZip = AdoFldString(rsZip, "Zip", "");
					}
					strLoadedFrom = "the Box 33 provider configured in the HCFA Setup";
					rsZip->Close();
				}
			}
			
			strRealZip.TrimLeft(); strRealZip.TrimRight();
			CString strZipToCheck = strRealZip;
			StripNonNumericChars(strZipToCheck);
			if(strRealZip.IsEmpty()) {
				str.Format("\tERROR - Missing Billing Address Zip Code\r\n\r\n"
					"\t\tLoop 2010AA, Billing Provider, is configured to load from %s, \r\n"
					"\t\tand has no zip code entered. Zip Codes for your billing address must be 9 digits in length.\r\n\r\n\r\n",
					strLoadedFrom);
				strErrors += str;
				bPassed = FALSE;
			}
			else if(strZipToCheck.GetLength() != 9) {
				str.Format("\tERROR - Invalid Billing Address Zip Code\r\n\r\n"
					"\t\tLoop 2010AA, Billing Provider, is configured to load from %s, \r\n"
					"\t\tand has a zip code of '%s'. Zip Codes for your billing address must be 9 digits in length.\r\n\r\n\r\n",
					strLoadedFrom, strRealZip);
				strErrors += str;
				bPassed = FALSE;
			}
		}

		rsClaim->Close();

		// (j.jones 2008-05-09 09:49) - PLID 29986 - removed Envoy/NSF from the system completely
		// (j.jones 2009-08-04 11:34) - PLID 14573 - removed THIN from the system

		//check presence of PayerID and InsType (already stored)
		if(bHasInsurance) {

			//Payer ID (not used on HCFA Images)
			// (j.jones 2009-12-17 09:53) - PLID 36237 - supported UB payer IDs
			if(bIsUB92 && strUBPayerID.IsEmpty() || strUBPayerID == " ") {
				//If ANSI only
				if (m_bElectronic && m_FormatStyle == ANSI
					&& GetRemotePropertyInt("EbillingValidatePrimaryPayerID",1,0,"<None>",TRUE)) {
					str.Format("No UB Payer ID Selected For This Insurance Company\r\n\r\n"
						"\t\tThe Insurance Company selected on this Bill:\r\n"
						"\t\t'%s'\r\n"
						"\t\tdoes not have a valid UB Payer ID.\r\n"
						"\t\tEvery Insurance Company you send electronic claims to must have a\r\n"
						"\t\tUB Payer ID selected in the 'UB Payer ID' box in the Edit Insurance List.\r\n"
						"\t\tYou can get there by clicking 'Edit Insurance List' in the\r\n"
						"\t\tInsurance Tab of the Patients Module, or the UB Tab of the\r\n"
						"\t\tAdministrator Module.\r\n", strInsCoName);
					
					bPassed = FALSE;
					str = "\tERROR - " + str + "\t\tThe official Payer ID Lists can be given to you by your clearinghouse.\r\n\r\n\r\n";

					strErrors += str;
				}
			}
			else if(!bIsUB92 && strHCFAPayerID.IsEmpty() || strHCFAPayerID == " ") {
				//If ANSI only
				if (m_bElectronic && m_FormatStyle == ANSI
					&& GetRemotePropertyInt("EbillingValidatePrimaryPayerID",1,0,"<None>",TRUE)) {
					str.Format("No HCFA Payer ID Selected For This Insurance Company\r\n\r\n"
						"\t\tThe Insurance Company selected on this Bill:\r\n"
						"\t\t'%s'\r\n"
						"\t\tdoes not have a valid HCFA Payer ID.\r\n"
						"\t\tEvery Insurance Company you send electronic claims to must have a\r\n"
						"\t\tHCFA Payer ID selected in the 'HCFA Payer ID' box in the Edit Insurance List.\r\n"
						"\t\tYou can get there by clicking 'Edit Insurance List' in the\r\n"
						"\t\tInsurance Tab of the Patients Module, or the HCFA Tab of the\r\n"
						"\t\tAdministrator Module.\r\n", strInsCoName);
					
					bPassed = FALSE;
					str = "\tERROR - " + str + "\t\tThe official Payer ID Lists can be given to you by your clearinghouse.\r\n\r\n\r\n";

					strErrors += str;
				}
			}

			//InsType Code (ANSI)
			// (j.jones 2008-09-09 11:22) - PLID 18695 - the value changed to InsType, the preference stayed the same
			if(eInsType == itcInvalid) {
				//If ANSI
				if (m_bElectronic && m_FormatStyle == ANSI && GetRemotePropertyInt("EbillingValidatePrimaryNSFCode",1,0,"<None>",TRUE)) {
					str.Format("\tERROR - No Insurance Type Selected For This Insurance Company\r\n\r\n"
						"\t\tThe Insurance Company selected on this Bill:\r\n"
						"\t\t'%s'\r\n"
						"\t\tdoes not have a valid Insurance Type.\r\n"
						"\t\tEvery Insurance Company you send electronic claims to must have a,\r\n"
						"\t\tInsurance Type Code selected in the 'Insurance Type' box in the Edit Insurance List.\r\n"
						"\t\tYou can get there by clicking 'Edit Insurance List' in the Insurance\r\n"
						"\t\tTab of the Patients Module, or the HCFA Tab of the Administrator Module.\r\n\r\n\r\n",strInsCoName);
					strErrors += str;
					bPassed = FALSE;
				}
			}

			//Check for Insured Party demographics

			_RecordsetPtr rsInsured = CreateParamRecordset("SELECT PersonT.First, PersonT.Last, PersonT.Address1, PersonT.Address2, "
				"PersonT.City, PersonT.State, PersonT.Zip, PersonT.BirthDate, PersonT.Gender, "
				"InsuredPartyT.PolicyGroupNum, InsuredPartyT.IDForInsurance "
				"FROM HCFATrackT "
				"INNER JOIN BillsT ON HCFATrackT.BillID = BillsT.ID "
				"INNER JOIN InsuredPartyT ON BillsT.InsuredPartyID = InsuredPartyT.PersonID "
				"INNER JOIN PersonT ON InsuredPartyT.PersonID = PersonT.ID "
				"WHERE BillsT.Deleted = 0 AND HCFATrackT.ID = {INT}", HCFAID);

			if(!rsInsured->eof) {
				//Check for First/Last Name
				BOOL bFirst = TRUE, bLast = TRUE;
				var = rsInsured->Fields->Item["First"]->Value;
				if(var.vt != VT_BSTR)
					bFirst = FALSE;
				else {
					CString strFirst = CString(var.bstrVal);
					strFirst.TrimRight();
					if(strFirst == "")
						bFirst = FALSE;
				}
				var = rsInsured->Fields->Item["Last"]->Value;
				if(var.vt != VT_BSTR)
					bLast = FALSE;
				else {
					CString strLast = CString(var.bstrVal);
					strLast.TrimRight();
					if(strLast == "")
						bLast = FALSE;
				}
				// (j.jones 2013-02-26 09:40) - PLID 55325 - corrected the wording to state that this is the main insured party selected
				// on the bill, it may or may not be the patient's Primary insurance
				if((!bFirst || !bLast) && GetRemotePropertyInt("EbillingValidatePrimaryName",1,0,"<None>",TRUE)) {
					str.Format("\tERROR - No Insured Party Name Found\r\n\r\n"
						"\t\tThe insured party selected on the bill does not have a\r\n"
						"\t\tcomplete name filled in. To fix this, go to the patient's\r\n"
						"\t\tinsurance tab and make sure the information filled in on the\r\n"
						"\t\tleft side of the screen is complete and correct for ALL of their\r\n"
						"\t\tinsurance companies. To auto-fill this information, click\r\n"
						"\t\tthe 'Copy Patient Info' button.\r\n\r\n\r\n");
					strErrors += str;
					bPassed = FALSE;
				}

				//Check for Address
				BOOL bAddress1 = TRUE, bAddress2 = TRUE, bCity = TRUE, bState = TRUE, bZip = TRUE;
				var = rsInsured->Fields->Item["Address1"]->Value;
				if(var.vt != VT_BSTR)
					bAddress1 = FALSE;
				else {
					CString strAddress1 = CString(var.bstrVal);
					strAddress1.TrimRight();
					if(strAddress1 == "")
						bAddress1 = FALSE;
				}
				var = rsInsured->Fields->Item["Address2"]->Value;
				if(var.vt != VT_BSTR)
					bAddress2 = FALSE;
				else {
					CString strAddress2 = CString(var.bstrVal);
					strAddress2.TrimRight();
					if(strAddress2 == "")
						bAddress2 = FALSE;
				}
				var = rsInsured->Fields->Item["City"]->Value;
				if(var.vt != VT_BSTR)
					bCity = FALSE;
				else {
					CString strCity = CString(var.bstrVal);
					strCity.TrimRight();
					if(strCity == "")
						bCity = FALSE;
				}
				var = rsInsured->Fields->Item["State"]->Value;
				if(var.vt != VT_BSTR)
					bState = FALSE;
				else {
					CString strState = CString(var.bstrVal);
					strState.TrimRight();
					if(strState == "")
						bState = FALSE;
				}
				var = rsInsured->Fields->Item["Zip"]->Value;
				if(var.vt != VT_BSTR)
					bZip = FALSE;
				else {
					CString strZip = CString(var.bstrVal);
					strZip.TrimRight();
					if(strZip == "")
						bZip = FALSE;
				}
				// (j.jones 2013-02-26 09:40) - PLID 55325 - corrected the wording to state that this is the main insured party selected
				// on the bill, it may or may not be the patient's Primary insurance
				if((!bAddress1 || !bCity || !bState || !bZip) && GetRemotePropertyInt("EbillingValidatePrimaryAddress",1,0,"<None>",TRUE)) {
					str.Format("\tWARNING - No Insured Party Address Found\r\n\r\n"
						"\t\tThe insured party selected on the bill does not have a\r\n"
						"\t\tcomplete address filled in. To fix this, go to the patient's\r\n"
						"\t\tinsurance tab and make sure the information filled in on the\r\n"
						"\t\tleft side of the screen is complete and correct for ALL of their\r\n"
						"\t\tinsurance companies. To auto-fill this information, click\r\n"
						"\t\tthe 'Copy Patient Info' button.\r\n\r\n\r\n");
					strErrors += str;
					bWarnings = TRUE;
				}
				if(bAddress2 && !bAddress1) {  //this is NOT a warning that can be disabled
					str.Format("\tERROR - Insured Party Address Formatted Improperly\r\n\r\n"
						"\t\tThe insured party selected on the bill \r\n"
						"\t\thas data in their Address 2 field but not in their Address 1 field.\r\n"
						"\t\tTo fix this, go to the patient's insurance tab and\r\n"
						"\t\tmake sure that the Address 1 field is filled in.\r\n\r\n\r\n");
					strErrors += str;
					bPassed = FALSE;
				}

				//Check for Birthdate
				BOOL bBirthDate = TRUE;
				var = rsInsured->Fields->Item["BirthDate"]->Value;
				if(var.vt != VT_DATE)
					bBirthDate = FALSE;
				// (j.jones 2013-02-26 09:40) - PLID 55325 - corrected the wording to state that this is the main insured party selected
				// on the bill, it may or may not be the patient's Primary insurance
				if(!bBirthDate && GetRemotePropertyInt("EbillingValidatePrimaryBirthdate",1,0,"<None>",TRUE)) {
					str.Format("\tWARNING - No Insured Party Birthdate Found\r\n\r\n"
						"\t\tThe insured party selected on the bill does not have a\r\n"
						"\t\tbirthdate filled in. To fix this, go to the patient's\r\n"
						"\t\tinsurance tab and make sure the information filled in on the\r\n"
						"\t\tleft side of the screen is complete and correct for ALL of their\r\n"
						"\t\tinsurance companies. To auto-fill this information, click\r\n"
						"\t\tthe 'Copy Patient Info' button.\r\n\r\n\r\n");
					strErrors += str;
					bWarnings = TRUE;
				}

				// (j.jones 2009-08-10 10:55) - PLID 32886 - added gender validation,
				// validate Male or Female only, anything else is considered invalid
				// (j.jones 2013-02-26 09:40) - PLID 55325 - corrected the wording to state that this is the main insured party selected
				// on the bill, it may or may not be the patient's Primary insurance
				if(GetRemotePropertyInt("EbillingValidatePrimaryGender",1,0,"<None>",TRUE)) {
					int iGender = AdoFldByte(rsInsured, "Gender", 0);
					if(iGender != 1 && iGender != 2) {
						str.Format("\tWARNING - No Insured Party Gender Found\r\n\r\n"
							"\t\tThe insured party selected on the bill does not have a\r\n"
							"\t\tgender filled in. To fix this, go to the patient's\r\n"
							"\t\tinsurance tab and make sure the information filled in on the\r\n"
							"\t\tleft side of the screen is complete and correct for ALL of their\r\n"
							"\t\tinsurance companies. To auto-fill this information, click\r\n"
							"\t\tthe 'Copy Patient Info' button.\r\n\r\n\r\n");
						strErrors += str;
						bWarnings = TRUE;
					}
				}

				CString strPolicyGroupNumber, strIDForInsurance;
				strPolicyGroupNumber = AdoFldString(rsInsured, "PolicyGroupNum","");
				strIDForInsurance = AdoFldString(rsInsured, "IDForInsurance","");

				// (j.jones 2013-02-26 09:40) - PLID 55325 - corrected the wording to state that this is the insured party selected
				// on the bill, it may or may not be the patient's Primary insurance
				if(strIDForInsurance == "" && GetRemotePropertyInt("EbillingValidatePrimaryInsuranceID",1,0,"<None>",TRUE)) {
					str.Format("\tWARNING - No Insurance ID Number Found\r\n\r\n"
						"\t\tThe insured party selected on the bill does not have a\r\n"
						"\t\tinsurance ID number filled in. To fix this, go to the patient's\r\n"
						"\t\tinsurance tab and make sure the information filled in on the\r\n"
						"\t\tlower right of the screen is complete and correct.\r\n\r\n\r\n");
					strErrors += str;
					bWarnings = TRUE;
				}

				if(GetRemotePropertyInt("EbillingValidatePrimaryPolicyGroupNum", GetRemotePropertyInt("EbillingValidatePolicyGroupNum",1,0,"<None>",TRUE),0,"<None>",TRUE)
					&& strPolicyGroupNumber == "") {
					// (j.jones 2013-02-26 09:40) - PLID 55325 - corrected the wording to state that this is the insured party selected
					// on the bill, it may or may not be the patient's Primary insurance
					str.Format("\tWARNING - No Policy Group Number Found\r\n\r\n"
						"\t\tThe insured party selected on the bill does not have a\r\n"
						"\t\tpolicy group number filled in. To fix this, go to the patient's\r\n"
						"\t\tinsurance tab and make sure the information filled in on the\r\n"
						"\t\tlower right of the screen is complete and correct.\r\n\r\n\r\n");
					strErrors += str;
					bWarnings = TRUE;
				}
			}
			else {
				//this is NOT a warning that can be disabled
				str.Format("\tERROR - No Insured Party Found\r\n\r\n"
						"\t\tThe patient on this bill does not have an insured party.\r\n"
						"\t\tTo fix this, go to the patient's insurance tab and make sure a\r\n"
						"\t\tcompany is selected, and there is information filled in on the\r\n"
						"\t\tleft side of the screen. To auto-fill this information, click\r\n"
						"\t\tthe 'Copy Patient Info' button.\r\n\r\n\r\n");
				strErrors += str;
				bPassed = FALSE;
			}
			rsInsured->Close();

			//check the payer ID for secondary insurance for ANSI and NSF
			//check the NSF code for NSF format
			//also, for ANSI, check for the presence of a phone number

			//JJ - 5/23/2003 - I turned this off for NSF because too many people complained about it, as it was unnecessary.
			//if (m_FormatStyle != IMAGE) {
			if (m_FormatStyle == ANSI && !bDontSubmitSecondary) {
				// (j.jones 2011-08-16 17:22) - PLID 44805 - we will filter out "original" and "void" charges
				// (j.jones 2012-08-06 16:29) - PLID 51916 - supported the new payer ID data structure, always pulling from EbillingInsCoIDs
				_RecordsetPtr rsSecInsured = CreateParamRecordset("SELECT TOP 1 InsuranceCoT.HCFASetupGroupID, "
					"CASE WHEN HCFALocPayerIDsT.ID Is Not Null THEN HCFALocPayerIDsT.EbillingID ELSE HCFAPayerIDsT.EBillingID END AS HCFAPayerID, "
					"CASE WHEN UBLocPayerIDsT.ID Is Not Null THEN UBLocPayerIDsT.EbillingID ELSE UBPayerIDsT.EBillingID END AS UBPayerID, "
					"InsuranceCoT.InsType, "
					"InsuranceCoT.Name, InsContactT.First AS ContactFirst, InsContactT.Last AS ContactLast, "
					"InsContactT.WorkPhone, InsContactT.Fax, InsuredPartyPersonT.Address1, InsuredPartyPersonT.Address2, "
					"InsuredPartyPersonT.First, InsuredPartyPersonT.Last, InsuredPartyPersonT.City, InsuredPartyPersonT.State, InsuredPartyPersonT.Zip, InsuredPartyPersonT.BirthDate, InsuredPartyPersonT.Gender, "
					"InsuredPartyT.PolicyGroupNum, InsuredPartyT.IDForInsurance "
					"FROM HCFATrackT "
					"INNER JOIN BillsT ON HCFATrackT.BillID = BillsT.ID "
					"INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
					"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
					"INNER JOIN InsuredPartyT ON BillsT.OthrInsuredPartyID = InsuredPartyT.PersonID "
					"INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
					"INNER JOIN PersonT ON InsuranceCoT.PersonID = PersonT.ID "
					"INNER JOIN PersonT InsuredPartyPersonT ON InsuredPartyT.PersonID = InsuredPartyPersonT.ID "
					"LEFT JOIN InsuranceContactsT ON InsuredPartyT.InsuranceContactID = InsuranceContactsT.PersonID "
					"LEFT JOIN PersonT InsContactT ON InsuranceContactsT.PersonID = InsContactT.ID "
					"LEFT JOIN InsuranceLocationPayerIDsT ON InsuranceCoT.PersonID = InsuranceLocationPayerIDsT.InsuranceCoID AND LineItemT.LocationID = InsuranceLocationPayerIDsT.LocationID "
					"LEFT JOIN EbillingInsCoIDs AS HCFALocPayerIDsT ON InsuranceLocationPayerIDsT.ClaimPayerID = HCFALocPayerIDsT.ID "
					"LEFT JOIN EbillingInsCoIDs AS UBLocPayerIDsT ON InsuranceLocationPayerIDsT.UBPayerID = UBLocPayerIDsT.ID "
					"LEFT JOIN EbillingInsCoIDs AS UBPayerIDsT ON InsuranceCoT.UBPayerID = UBPayerIDsT.ID "
					"LEFT JOIN EbillingInsCoIDs AS HCFAPayerIDsT ON InsuranceCoT.HCFAPayerID = HCFAPayerIDsT.ID "
					"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
					"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
					"WHERE BillsT.Deleted = 0 AND LineItemT.Deleted = 0 AND ChargesT.Batched = 1 AND HCFATrackT.ID = {INT} "
					"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
					"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null", HCFAID);

				if(!rsSecInsured->eof) {

					CString strInsCoName = CString(rsSecInsured->Fields->Item["Name"]->Value.bstrVal);

					//check for secondary Payer ID
					// (j.jones 2012-08-06 16:32) - PLID 51916 - fixed to use the HCFA or UB payer ID
					if(bIsUB92) {
						var = rsSecInsured->Fields->Item["UBPayerID"]->Value;
					}
					else {
						var = rsSecInsured->Fields->Item["HCFAPayerID"]->Value;
					}
					if(GetRemotePropertyInt("EbillingValidateSecondaryPayerID",1,0,"<None>",TRUE)
						&& (var.vt != VT_BSTR || CString(var.bstrVal).GetLength() == 0 || CString(var.bstrVal) == " ")) {
						
							str.Format("No Payer ID Selected For the Secondary Insurance Company\r\n\r\n"
								"\t\tThe Secondary Insurance Company selected on this Bill:\r\n"
								"\t\t'%s'\r\n"
								"\t\tdoes not have a valid Payer ID.\r\n"
								"\t\tEvery Insurance Company you send electronic claims to must have a\r\n"
								"\t\tPayer ID selected in the 'Payer ID' box in the Edit Insurance List.\r\n"
								"\t\tYou can get there by clicking 'Edit Insurance List' in the\r\n"
								"\t\tInsurance Tab of the Patients Module, or the HCFA Tab of the\r\n"
								"\t\tAdministrator Module.\r\n",strInsCoName);					
							
							bPassed = FALSE;
							str = "\tERROR - " + str + "\t\tThe official Payer ID Lists can be given to you by your clearinghouse.\r\n\r\n\r\n";

							strErrors += str;
					}

					//NSF Code (NSF or ANSI)
					InsuranceTypeCode eInsType = (InsuranceTypeCode)AdoFldLong(rsSecInsured, "InsType", (long)itcInvalid);
					// (j.jones 2008-09-09 11:22) - PLID 18695 - the value changed to InsType, the preference stayed the same
					if(eInsType == itcInvalid) {
						//If ANSI
						if (m_bElectronic && m_FormatStyle == ANSI && GetRemotePropertyInt("EbillingValidateSecondaryNSFCode",1,0,"<None>",TRUE)) {
							str.Format("\tERROR - No Insurance Type Selected For the Secondary Insurance Company\r\n\r\n"
								"\t\tThe Secondary Insurance Company selected on this Bill:\r\n"
								"\t\t'%s'\r\n"
								"\t\tdoes not have a valid Insurance Type.\r\n"
								"\t\tEvery Insurance Company you send electronic claims to must have a\r\n"
								"\t\tInsurance Type Code selected in the 'Insurance Type' box in the Edit Insurance List.\r\n"
								"\t\tYou can get there by clicking 'Edit Insurance List' in the Insurance\r\n"
								"\t\tTab of the Patients Module, or the HCFA Tab of the Administrator Module.\r\n\r\n\r\n",strInsCoName);
							strErrors += str;
							bPassed = FALSE;
						}
					}

					//if ANSI, check for a phone number
					if(m_FormatStyle == ANSI) {

						//only check the secondary insco phone number if we are exporting that record and validating that item
						// (j.jones 2012-10-22 13:33) - PLID 53297 - this setting is obsolete in 5010, because this content is never exported in 5010
						if(bExport2330BPER && m_avANSIVersion == av4010) {
							_variant_t varWork = rsSecInsured->Fields->Item["WorkPhone"]->Value;
							_variant_t varFax = rsSecInsured->Fields->Item["Fax"]->Value;
							if((varWork.vt != VT_BSTR || CString(varWork.bstrVal).GetLength() == 0 || CString(varWork.bstrVal) == " ") &&
								(varFax.vt != VT_BSTR || CString(varFax.bstrVal).GetLength() == 0 || CString(varFax.bstrVal) == " ")) {
								
								if(GetRemotePropertyInt("EbillingValidateSecondaryPhoneNumber",1,0,"<None>",TRUE)) {

									str.Format("\tERROR - No Phone Number Available For the Secondary Insurance Company\r\n\r\n"
										"\t\tThe Secondary Insurance Company selected on this Bill:\r\n"
										"\t\t'%s'\r\n"
										"\t\tdoes not have any valid Phone Numbers.\r\n"
										"\t\tEvery Secondary Insurance Company you send electronic claims to must have a\r\n"
										"\t\tcontact number (either a phone number or fax number) selected in the\r\n"
										"\t\tcontact information section of the Edit Insurance List.\r\n"
										"\t\tYou can get there by clicking 'Edit Insurance List' in the\r\n"
										"\t\tInsurance Tab of the Patients Module, or the HCFA Tab of the\r\n"
										"\t\tAdministrator Module.\r\n\r\n\r\n",strInsCoName);						
									
									bPassed = FALSE;
									strErrors += str;
								}
							}

							_variant_t varFirst = rsSecInsured->Fields->Item["ContactFirst"]->Value;
							_variant_t varLast = rsSecInsured->Fields->Item["ContactLast"]->Value;
							if((varFirst.vt != VT_BSTR || CString(varFirst.bstrVal).GetLength() == 0 || CString(varFirst.bstrVal) == " ") &&
								(varLast.vt != VT_BSTR || CString(varLast.bstrVal).GetLength() == 0 || CString(varLast.bstrVal) == " ")) {

								if(GetRemotePropertyInt("EbillingValidateSecondaryContactName",1,0,"<None>",TRUE)) {
								
									str.Format("\tERROR - No Contact Name Entered For the Secondary Insurance Company\r\n\r\n"
										"\t\tThe Secondary Insurance Company selected on this Bill:\r\n"
										"\t\t'%s'\r\n"
										"\t\tdoes not have a valid Contact Name.\r\n"
										"\t\tEvery Secondary Insurance Company you send electronic claims to must have a\r\n"
										"\t\tcontact name (enter 'Customer Service' if no name is available) entered in the\r\n"
										"\t\tcontact information section of the Edit Insurance List.\r\n"
										"\t\tYou can get there by clicking 'Edit Insurance List' in the\r\n"
										"\t\tInsurance Tab of the Patients Module, or the HCFA Tab of the\r\n"
										"\t\tAdministrator Module.\r\n\r\n\r\n",strInsCoName);						
									
									bPassed = FALSE;
									strErrors += str;
								}
							}
						}

						//Check for First/Last Name
						BOOL bFirst = TRUE, bLast = TRUE;
						var = rsSecInsured->Fields->Item["First"]->Value;
						if(var.vt != VT_BSTR)
							bFirst = FALSE;
						else {
							CString strFirst = CString(var.bstrVal);
							strFirst.TrimRight();
							if(strFirst == "")
								bFirst = FALSE;
						}
						var = rsSecInsured->Fields->Item["Last"]->Value;
						if(var.vt != VT_BSTR)
							bLast = FALSE;
						else {
							CString strLast = CString(var.bstrVal);
							strLast.TrimRight();
							if(strLast == "")
								bLast = FALSE;
						}
						// (j.jones 2013-02-26 09:40) - PLID 55325 - corrected the wording to state that this is the secondary insured party selected
						// on the bill, it may or may not be the patient's Secondary insurance
						if((!bFirst || !bLast) && GetRemotePropertyInt("EbillingValidateSecondaryName",1,0,"<None>",TRUE)) {
							str.Format("\tERROR - No Secondary Insured Party Name Found\r\n\r\n"
								"\t\tThe secondary insured party selected on the bill does not have a\r\n"
								"\t\tcomplete name filled in. To fix this, go to the patient's\r\n"
								"\t\tinsurance tab and make sure the information filled in on the\r\n"
								"\t\tleft side of the screen is complete and correct for ALL of their\r\n"
								"\t\tinsurance companies. To auto-fill this information, click\r\n"
								"\t\tthe 'Copy Patient Info' button.\r\n\r\n\r\n");
							strErrors += str;
							bPassed = FALSE;
						}

						//Check for Address
						BOOL bAddress1 = TRUE, bAddress2 = TRUE, bCity = TRUE, bState = TRUE, bZip = TRUE;
						var = rsSecInsured->Fields->Item["Address1"]->Value;
						if(var.vt != VT_BSTR)
							bAddress1 = FALSE;
						else {
							CString strAddress1 = CString(var.bstrVal);
							strAddress1.TrimRight();
							if(strAddress1 == "")
								bAddress1 = FALSE;
						}
						var = rsSecInsured->Fields->Item["Address2"]->Value;
						if(var.vt != VT_BSTR)
							bAddress2 = FALSE;
						else {
							CString strAddress2 = CString(var.bstrVal);
							strAddress2.TrimRight();
							if(strAddress2 == "")
								bAddress2 = FALSE;
						}
						var = rsSecInsured->Fields->Item["City"]->Value;
						if(var.vt != VT_BSTR)
							bCity = FALSE;
						else {
							CString strCity = CString(var.bstrVal);
							strCity.TrimRight();
							if(strCity == "")
								bCity = FALSE;
						}
						var = rsSecInsured->Fields->Item["State"]->Value;
						if(var.vt != VT_BSTR)
							bState = FALSE;
						else {
							CString strState = CString(var.bstrVal);
							strState.TrimRight();
							if(strState == "")
								bState = FALSE;
						}
						var = rsSecInsured->Fields->Item["Zip"]->Value;
						if(var.vt != VT_BSTR)
							bZip = FALSE;
						else {
							CString strZip = CString(var.bstrVal);
							strZip.TrimRight();
							if(strZip == "")
								bZip = FALSE;
						}
						// (j.jones 2013-02-26 09:40) - PLID 55325 - corrected the wording to state that this is the secondary insured party selected
						// on the bill, it may or may not be the patient's Secondary insurance
						if((!bAddress1 || !bCity || !bState || !bZip) && GetRemotePropertyInt("EbillingValidateSecondaryAddress",1,0,"<None>",TRUE)) {
							str.Format("\tWARNING - No Secondary Insured Party Address Found\r\n\r\n"
								"\t\tThe secondary insured party selected on the bill does not have a\r\n"
								"\t\tcomplete address filled in. To fix this, go to the patient's\r\n"
								"\t\tinsurance tab and make sure the information filled in on the\r\n"
								"\t\tleft side of the screen is complete and correct for ALL of their\r\n"
								"\t\tinsurance companies. To auto-fill this information, click\r\n"
								"\t\tthe 'Copy Patient Info' button.\r\n\r\n\r\n");
							strErrors += str;
							bWarnings = TRUE;
						}
						if(bAddress2 && !bAddress1) {	//this is NOT a warning that can be disabled
							str.Format("\tERROR - Secondary Insured Party Address Formatted Improperly\r\n\r\n"
								"\t\tThe secondary insured party selected on the bill \r\n"
								"\t\thas data in their Address 2 field but not in their Address 1 field.\r\n"
								"\t\tTo fix this, go to the patient's insurance tab and\r\n"
								"\t\tmake sure that the Address 1 field is filled in.\r\n\r\n\r\n");
							strErrors += str;
							bPassed = FALSE;
						}

						//Check for Birthdate
						BOOL bBirthDate = TRUE;
						var = rsSecInsured->Fields->Item["BirthDate"]->Value;
						if(var.vt != VT_DATE)
							bBirthDate = FALSE;
						// (j.jones 2013-02-26 09:40) - PLID 55325 - corrected the wording to state that this is the secondary insured party selected
						// on the bill, it may or may not be the patient's Secondary insurance
						if(!bBirthDate && GetRemotePropertyInt("EbillingValidateSecondaryBirthdate",1,0,"<None>",TRUE)) {
							str.Format("\tWARNING - No Secondary Insured Party Birthdate Found\r\n\r\n"
								"\t\tThe secondary insured party selected on the bill ('%s')\r\n"
								"\t\tdoes not have a birthdate filled in. To fix this, go to the patient's\r\n"
								"\t\tinsurance tab and make sure the information filled in on the\r\n"
								"\t\tleft side of the screen is complete and correct for ALL of their\r\n"
								"\t\tinsurance companies. To auto-fill this information, click\r\n"
								"\t\tthe 'Copy Patient Info' button.\r\n\r\n\r\n",strInsCoName);
							strErrors += str;
							bWarnings = TRUE;
						}

						// (j.jones 2009-08-10 10:55) - PLID 32886 - added gender validation,
						// validate Male or Female only, anything else is considered invalid
						// (j.jones 2013-02-26 09:40) - PLID 55325 - corrected the wording to state that this is the secondary insured party selected
						// on the bill, it may or may not be the patient's Secondary insurance
						if(GetRemotePropertyInt("EbillingValidateSecondaryGender",1,0,"<None>",TRUE)) {
							int iGender = AdoFldByte(rsSecInsured, "Gender", 0);
							if(iGender != 1 && iGender != 2) {
								str.Format("\tWARNING - No Secondary Insured Party Gender Found\r\n\r\n"
									"\t\tThe secondary insured party selected on the bill does not have a\r\n"
									"\t\tgender filled in. To fix this, go to the patient's\r\n"
									"\t\tinsurance tab and make sure the information filled in on the\r\n"
									"\t\tleft side of the screen is complete and correct for ALL of their\r\n"
									"\t\tinsurance companies. To auto-fill this information, click\r\n"
									"\t\tthe 'Copy Patient Info' button.\r\n\r\n\r\n");
								strErrors += str;
								bWarnings = TRUE;
							}
						}

						CString strPolicyGroupNumber, strIDForInsurance;
						strPolicyGroupNumber = AdoFldString(rsSecInsured, "PolicyGroupNum","");
						strIDForInsurance = AdoFldString(rsSecInsured, "IDForInsurance","");

						// (j.jones 2013-02-26 09:40) - PLID 55325 - corrected the wording to state that this is the secondary insured party selected
						// on the bill, it may or may not be the patient's Secondary insurance
						if(strIDForInsurance == "" && GetRemotePropertyInt("EbillingValidateSecondaryInsuranceID",1,0,"<None>",TRUE)) {
							str.Format("\tWARNING - No Insurance ID Number Found for the Secondary Insured Party\r\n\r\n"
								"\t\tThe secondary insured party selected on the bill does not have a\r\n"
								"\t\tinsurance ID number filled in. To fix this, go to the patient's\r\n"
								"\t\tinsurance tab and make sure the information filled in on the\r\n"
								"\t\tlower right of the screen is complete and correct.\r\n\r\n\r\n");
							strErrors += str;
							bWarnings = TRUE;
						}

						// (j.jones 2013-02-26 09:40) - PLID 55325 - corrected the wording to state that this is the secondary insured party selected
						// on the bill, it may or may not be the patient's Secondary insurance
						if(GetRemotePropertyInt("EbillingValidateSecondaryPolicyGroupNum", GetRemotePropertyInt("EbillingValidatePolicyGroupNum",1,0,"<None>",TRUE),0,"<None>",TRUE)
							&& strPolicyGroupNumber == "") {
							str.Format("\tWARNING - No Policy Group Number Found for the Secondary Insured Party\r\n\r\n"
								"\t\tThe secondary insured party selected on the bill does not have a\r\n"
								"\t\tpolicy group number filled in. To fix this, go to the patient's\r\n"
								"\t\tinsurance tab and make sure the information filled in on the\r\n"
								"\t\tlower right of the screen is complete and correct.\r\n\r\n\r\n");
							strErrors += str;
							bWarnings = TRUE;
						}
					}				
				}
				rsSecInsured->Close();
			}
		}
		
		//Provider
		BOOL bHasProvider = FALSE;
		// (j.jones 2010-11-10 14:51) - PLID 41396 - we can eliminate several recordsets here as
		// we already loaded the provider IDs (use the billing provider for this calculation)
		if(nBillingProviderID != -1) {
			bHasProvider = TRUE;
		}

		if(!bHasProvider && GetRemotePropertyInt("EbillingValidateProvider",1,0,"<None>",TRUE)) {
			str = "\tERROR - No Provider Selected For This Claim\r\n\r\n";
			if((!bIsUB92 && HCFAProvType == 2) || (bIsUB92 && nBox82Setup == 2))
				str += "\t\tThis Insurance Company's HCFA Group is set up to use the patient's\r\n"
				"\t\tdefault provider from General 1. You must select a provider for this\r\n"
				"\t\tpatient in the General 1 Tab of the Patients Module.\r\n\r\n\r\n";
			else if((!bIsUB92 && HCFAProvType == 1) || (bIsUB92 && nBox82Setup == 1))
				str += "\t\tThis Insurance Company's HCFA Group is set up to use the bill\r\n"
				"\t\tprovider, and one or more charges on this claim has no provider\r\n"
				"\t\tassociated with it. You must select a provider for every charge\r\n"
				"\t\ton this Bill.\r\n\r\n\r\n";
			else if(!bIsUB92 && HCFAProvType == 3) {
				str += "\t\tThis Insurance Company's HCFA Group is set up to use a custom\r\n"
				"\t\tprovider, however you still need a provider selected on the bill,\r\n"
				"\t\tand one or more charges on this claim has no provider associated with it.\r\n"
				"\t\tYou must select a provider for every charge on this Bill.\r\n\r\n\r\n";
			}
			else if(!bIsUB92 && HCFAProvType == 4) {
				str += "\t\tThis Insurance Company's HCFA Group is set up to use the bill location\r\n"
				"\t\tas the provider, however you still need a provider selected on the bill, and\r\n"
				"\t\tone or more charges on this claim has no provider associated with it.\r\n"
				"\t\tYou must select a provider for every charge on this Bill.\r\n\r\n\r\n";
			}
			else if(bIsUB92 && nBox82Setup == 3) {
				str += "\t\tThis Insurance Company's HCFA Group is set up to use the referring physician\r\n"
				"\t\tas the provider, and the bill does not have a referring physician selected.\r\n"
				"\t\tYou must select a referring physician on the Insurance tab of this Bill.\r\n\r\n\r\n";
			}
			else {
				str += "\t\tNo provider configuration was found for this Insurance Company's\r\n"
				"\t\tHCFA Group. As a result, the bill provider is used, and one or more\r\n"
				"\t\tcharges on this claim has no provider associated with it. You must \r\n"
				"\t\tselect a provider for every charge on this Bill.\r\n\r\n\r\n";
			}

			if(bIsUB92) {
				//TES 3/13/2007 - PLID 24993 - Changed from UB92 to UB
				str.Replace("HCFA", "UB");
				//TES 3/13/2007 - PLID 24993 - UB92 Box 82 = UB04 Box 76
				if(GetUBFormType() == eUB04) {
					str.Replace("Box 33", "Box 76");
				}
				else {
					str.Replace("Box 33", "Box 82");
				}
			}

			strErrors += str;
			bPassed = FALSE;
		}

		//check individual charge data
		// (j.jones 2011-08-16 17:22) - PLID 44805 - we will filter out "original" and "void" charges
		// (j.jones 2012-07-25 09:52) - PLID 51764 - load the charge's pay group category
		_RecordsetPtr rsCharges = CreateParamRecordset("SELECT HospFrom, HospTo, PlaceCodes, "
			"CPTModifier, CPTModifier2, CPTModifier3, CPTModifier4, "
			"CPTCodeT.Code, LineItemT.Description, "
			"ServicePayGroupsT.Category AS PayGroupCategory, LineItemT.Date, LineItemT.InputDate "
			"FROM HCFATrackT "
			"INNER JOIN BillsT ON HCFATrackT.BillID = BillsT.ID "
			"INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"INNER JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
			"LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
			"LEFT JOIN ServicePayGroupsT ON ServiceT.PayGroupID = ServicePayGroupsT.ID "
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
			"LEFT JOIN PlaceOfServiceCodesT ON ChargesT.ServiceLocationID = PlaceOfServiceCodesT.ID "
			"WHERE BillsT.Deleted = 0 AND LineItemT.Deleted = 0 AND Batched = 1 "
			"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
			"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
			"AND HCFATrackT.ID = {INT}", HCFAID);

		BOOL bPOSCode = TRUE;
		BOOL bPOSCodeIs21 = FALSE;
		BOOL bHasHospitalizationDateFrom = FALSE;
		BOOL bHasHospitalizationDateTo = FALSE;

		if(!rsCharges->eof) {
			//override the booleans to TRUE if we don't want to validate them
			var = rsCharges->Fields->Item["HospFrom"]->Value;
			if(var.vt == VT_DATE || !GetRemotePropertyInt("EbillingValidateHospFrom",1,0,"<None>",TRUE))
				bHasHospitalizationDateFrom = TRUE;
			var = rsCharges->Fields->Item["HospTo"]->Value;
			if(var.vt == VT_DATE || !GetRemotePropertyInt("EbillingValidateHospTo",1,0,"<None>",TRUE))
				bHasHospitalizationDateTo = TRUE;
		}

		while(!rsCharges->eof) {
			
			//Check for Place Of Service Code
			var = rsCharges->Fields->Item["PlaceCodes"]->Value;
			if(var.vt != VT_BSTR)
				bPOSCode = FALSE;
			else {
				CString strPOS = CString(var.bstrVal);
				strPOS.TrimRight();
				if(strPOS == "" || strPOS == "NULL")
					bPOSCode = FALSE;
				else if(strPOS == "21")
					bPOSCodeIs21 = TRUE;
			}

			//Check for Modifier length problems

			CString strMod1, strMod2, strMod3, strMod4;
			strMod1 = AdoFldString(rsCharges, "CPTModifier","");
			strMod2 = AdoFldString(rsCharges, "CPTModifier2","");
			strMod3 = AdoFldString(rsCharges, "CPTModifier3","");
			strMod4 = AdoFldString(rsCharges, "CPTModifier4","");

			//only validate if the setting says to
			if(GetRemotePropertyInt("EbillingValidateInvalidModifiers",1,0,"<None>",TRUE)) {
				if(strMod1.GetLength() > 2) {
					str.Format("\tERROR - Invalid Modifier 1 Selected\r\n\r\n"
							"\t\tThe Modifier 1 value (%s) on the bill is invalid,\r\n"
							"\t\tas it cannot be more than two characters long.\r\n"
							"\t\tTo fix this, open up the bill and select a valid modifier.\r\n\r\n\r\n",strMod1);
					strErrors += str;
					bPassed = FALSE;
				}
				else if(strMod2.GetLength() > 2) {
					str.Format("\tERROR - Invalid Modifier 2 Selected\r\n\r\n"
							"\t\tThe Modifier 2 value (%s) on the bill is invalid,\r\n"
							"\t\tas it cannot be more than two characters long.\r\n"
							"\t\tTo fix this, open up the bill and select a valid modifier.\r\n\r\n\r\n",strMod2);
					strErrors += str;
					bPassed = FALSE;
				}
				else if(strMod3.GetLength() > 2) {
					str.Format("\tERROR - Invalid Modifier 3 Selected\r\n\r\n"
							"\t\tThe Modifier 3 value (%s) on the bill is invalid,\r\n"
							"\t\tas it cannot be more than two characters long.\r\n"
							"\t\tTo fix this, open up the bill and select a valid modifier.\r\n\r\n\r\n",strMod3);
					strErrors += str;
					bPassed = FALSE;
				}
				else if(strMod4.GetLength() > 2) {
					str.Format("\tERROR - Invalid Modifier 4 Selected\r\n\r\n"
							"\t\tThe Modifier 4 value (%s) on the bill is invalid,\r\n"
							"\t\tas it cannot be more than two characters long.\r\n"
							"\t\tTo fix this, open up the bill and select a valid modifier.\r\n\r\n\r\n",strMod4);
					strErrors += str;
					bPassed = FALSE;
				}
			}

			// (j.jones 2012-07-25 09:24) - PLID 51764 - added office visit global period validation,
			// it applies to all US claims, paper or ANSI, HCFA or UB
			if(GetRemotePropertyInt("EbillingValidateOfficeVisitGlobalPeriods",1,0,"<None>",TRUE)) {
				//If the charge is categorized as an office visit, and there is an active global period,
				//then we must warn. However, we do not warn if this charge has a modifier of 24, 25, or 57,
				//which explicitly permits the use of office visit codes under global periods.

				//get the pay group category
				PayGroupCategory::Category ePayGroupCategory = (PayGroupCategory::Category)VarLong(rsCharges->Fields->Item["PayGroupCategory"]->Value, (long)PayGroupCategory::NoCategory);

				//only check global periods if this charge is an office visit that does not have a 24 modifier
				// (j.jones 2013-06-18 15:26) - PLID 57147 - modifiers 25 and 57 also disable the warning
				if(ePayGroupCategory == PayGroupCategory::OfficeVisit
					&& !ContainsModifier("24", strMod1, strMod2, strMod3, strMod4)
					&& !ContainsModifier("25", strMod1, strMod2, strMod3, strMod4)
					&& !ContainsModifier("57", strMod1, strMod2, strMod3, strMod4)) {

					COleDateTime dtDate = VarDateTime(rsCharges->Fields->Item["Date"]->Value);
					COleDateTime dtInputDate = VarDateTime(rsCharges->Fields->Item["InputDate"]->Value);

					//This is similar logic as CheckWarnGlobalPeriod(), except that if the global period
					//started on the same date as the charge we're checking, we won't warn if this charge
					//was input first. This will avoid warning in cases where they entered an office visit
					//on one bill and then added a global period-starting charge on the same or successive
					//bill with the same service date. It wouldn't have warned in billing, and should not
					//warn here either.
					//We're also only going to report the most recent global period that applies. Billing
					//would have warned about all global periods. This will report one to conserve space
					//in the warnings export.

					// (j.jones 2012-07-26 15:14) - PLID 51827 - optionally exclude modifier 78
					_RecordsetPtr rsGlobalPeriod = CreateParamRecordset("SELECT TOP 1 CPTCodeT.Code, "
						"ServiceT.Name, LineItemT.Date, CPTCodeT.GlobalPeriod, "
						"DATEADD(day,GlobalPeriod,LineItemT.Date) AS ExpDate "
						"FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
						"INNER JOIN CPTCodeT ON ChargesT.ServiceID = CPTCodeT.ID "
						"INNER JOIN ServiceT ON CPTCodeT.ID = ServiceT.ID "
						"LEFT JOIN ServicePayGroupsT ON ServiceT.PayGroupID = ServicePayGroupsT.ID "
						"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalLineItemsQ ON LineItemT.ID = LineItemCorrections_OriginalLineItemsQ.OriginalLineItemID "
						"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingLineItemsQ ON LineItemT.ID = LineItemCorrections_VoidingLineItemsQ.VoidingLineItemID "
						"WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 10 AND GlobalPeriod Is Not Null AND CPTCodeT.GlobalPeriod <> 0 AND "
						//look to see if the global period's date ends after this service date
						"dbo.AsDateNoTime(DATEADD(day,GlobalPeriod,LineItemT.Date)) > dbo.AsDateNoTime({OLEDATETIME}) "
						//then look to see if the global period's date starts before this service date
						"AND (dbo.AsDateNoTime(LineItemT.Date) < dbo.AsDateNoTime({OLEDATETIME}) OR "
						//or, in the case where the global period started on the same date,
						//look to see if the input date is prior to this charge
						"	(dbo.AsDateNoTime(LineItemT.Date) = dbo.AsDateNoTime({OLEDATETIME}) AND LineItemT.InputDate < {OLEDATETIME})) "
						"AND LineItemT.PatientID = {INT} "
						"AND LineItemCorrections_OriginalLineItemsQ.OriginalLineItemID Is Null "
						"AND LineItemCorrections_VoidingLineItemsQ.VoidingLineItemID Is Null "
						"AND ({INT} <> 1 OR ServicePayGroupsT.Category = {CONST}) "
						"AND ({INT} <> 1 OR (Coalesce(ChargesT.CPTModifier, '') <> '78' AND Coalesce(ChargesT.CPTModifier2, '') <> '78' AND Coalesce(ChargesT.CPTModifier3, '') <> '78' AND Coalesce(ChargesT.CPTModifier4, '') <> '78')) "
						"ORDER BY DATEADD(day,GlobalPeriod,LineItemT.Date) DESC",
						dtDate, dtDate, dtDate, dtInputDate,
						nPatientID, nSurgicalCodesOnly, PayGroupCategory::SurgicalCode, nIgnoreModifier78);
					if(!rsGlobalPeriod->eof) {
						CString strGlobalPeriodCode = VarString(rsGlobalPeriod->Fields->Item["Code"]->Value, "");
						CString strGlobalPeriodName = VarString(rsGlobalPeriod->Fields->Item["Name"]->Value, "");
						_variant_t varGlobalPeriodDate = rsGlobalPeriod->Fields->Item["Date"]->Value;
						_variant_t varGlobalPeriodExpDate = rsGlobalPeriod->Fields->Item["ExpDate"]->Value;
						long nGlobalPeriodDays = VarLong(rsGlobalPeriod->Fields->Item["GlobalPeriod"]->Value);

						CString strCode = VarString(rsCharges->Fields->Item["Code"]->Value, "");
						CString strChargeDesc = VarString(rsCharges->Fields->Item["Description"]->Value, "");
						CString strChargeInfo = strCode;
						if(!strChargeInfo.IsEmpty() && !strChargeDesc.IsEmpty()) {
							strChargeInfo += " - ";
						}
						strChargeInfo += strChargeDesc;

						CString strGlobalPeriodDesc;
						strGlobalPeriodDesc.Format("\n\nService Code: (%s) %s, Service Date: %s, Global Period: %li days, Expires: %s",
							strGlobalPeriodCode, strGlobalPeriodName, FormatDateTimeForInterface(VarDateTime(varGlobalPeriodDate), NULL, dtoDate),
							nGlobalPeriodDays, FormatDateTimeForInterface(VarDateTime(varGlobalPeriodExpDate), NULL, dtoDate));

						//warn about this global period

						// (j.jones 2013-06-18 15:26) - PLID 57147 - modifiers 25 and 57 also disable the warning
						str.Format("\tWARNING - Office Visit Billed Within A Global Period\r\n\r\n"
							"\t\tThis bill has an office visit charge for %s.\r\n\r\n"
							"\t\tHowever, the patient is under an active global period for:\r\n"
							"\t\t- %s\r\n\r\n"
							"\t\tIf this office visit code is intentional, it should have a modifier of 24, 25, or 57\r\n"
							"\t\tadded to the charge in order to avoid this warning.\r\n\r\n\r\n",
							strChargeInfo, strGlobalPeriodDesc);
						strErrors += str;
						bWarnings = TRUE;
					}
					rsGlobalPeriod->Close();
				}
			}
			
			//move to the next charge
			rsCharges->MoveNext();

		}	//end of charge loop
		rsCharges->Close();

		//Check for Place Of Service Code
		if(GetRemotePropertyInt("EbillingValidatePOSCode",1,0,"<None>",TRUE)
			&& !bPOSCode) {
			str.Format("\tERROR - No Place Of Service Code Selected\r\n\r\n"
					"\t\tThe Place Of Service on the bill does not have a corresponding\r\n"
					"\t\tcode selected with it (11,22,3, etc.) To fix this, open up the\r\n"
					"\t\tbill and select a code in the lower right hand corner. To\r\n"
					"\t\tpermanently fix this, you can assign a default code to a place\r\n"
					"\t\tof service by selecting one in the Locations tab of the\r\n"
					"\t\tAdministrator module.\r\n\r\n\r\n");
			strErrors += str;
			bPassed = FALSE;
		}
		else if(bPOSCodeIs21 && (!bHasHospitalizationDateFrom || !bHasHospitalizationDateTo)) {
			CString strHospMissing;
			if(!bHasHospitalizationDateFrom && !bHasHospitalizationDateTo)
				strHospMissing = "'Hospitalization From' and 'To' dates are";
			else if(!bHasHospitalizationDateFrom && bHasHospitalizationDateTo)
				strHospMissing = "'Hospitalization From' date is";
			else if(bHasHospitalizationDateFrom && !bHasHospitalizationDateTo)
				strHospMissing = "'Hospitalization To' date is";

			str.Format("\tERROR - Hospitalization Dates Empty\r\n\r\n"
					"\t\tThe Place Of Service on the bill is 21 (Inpatient Hospital)\r\n"
					"\t\tbut the %s empty.\r\n"
					"\t\tTo fix this, open up the bill and correct the Hospitalization\r\n"
					"\t\tdates on the Insurance tab of the bill.\r\n\r\n\r\n",strHospMissing);
			strErrors += str;
			bPassed = FALSE;
		}

		//Check for Patient demographics

		_RecordsetPtr rsPatient = CreateParamRecordset("SELECT PersonT.First, PersonT.Last, PersonT.Address1, PersonT.Address2, "
					"PersonT.City, PersonT.State, PersonT.Zip, PersonT.BirthDate, PersonT.Gender "
					"FROM HCFATrackT "
					"INNER JOIN BillsT ON HCFATrackT.BillID = BillsT.ID "
					"INNER JOIN PatientsT ON BillsT.PatientID = PatientsT.PersonID "
					"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
					"WHERE BillsT.Deleted = 0 AND HCFATrackT.ID = {INT}",HCFAID);

		if(!rsPatient->eof) {
			//Check for First/Last Name
			BOOL bFirst = TRUE, bLast = TRUE;
			var = rsPatient->Fields->Item["First"]->Value;
			if(var.vt != VT_BSTR)
				bFirst = FALSE;
			else {
				CString strFirst = CString(var.bstrVal);
				strFirst.TrimRight();
				if(strFirst == "")
					bFirst = FALSE;
			}
			var = rsPatient->Fields->Item["Last"]->Value;
			if(var.vt != VT_BSTR)
				bLast = FALSE;
			else {
				CString strLast = CString(var.bstrVal);
				strLast.TrimRight();
				if(strLast == "")
					bLast = FALSE;
			}
			if(!bFirst || !bLast) {
				str.Format("\tERROR - No Patient Name Found\r\n\r\n"
					"\t\tThis patient does not have a complete name filled in.\r\n"
					"\t\tTo fix this, go to the patient's General 1 tab and\r\n"
					"\t\tmake sure all the information is complete and correct.\r\n\r\n\r\n");
				strErrors += str;
				bPassed = FALSE;
			}

			//Check for Address
			BOOL bAddress1 = TRUE, bAddress2 = TRUE, bCity = TRUE, bState = TRUE, bZip = TRUE;
			var = rsPatient->Fields->Item["Address1"]->Value;
			if(var.vt != VT_BSTR)
				bAddress1 = FALSE;
			else {
				CString strAddress1 = CString(var.bstrVal);
				strAddress1.TrimRight();
				if(strAddress1 == "")
					bAddress1 = FALSE;
			}
			var = rsPatient->Fields->Item["Address2"]->Value;
			if(var.vt != VT_BSTR)
				bAddress2 = FALSE;
			else {
				CString strAddress2 = CString(var.bstrVal);
				strAddress2.TrimRight();
				if(strAddress2 == "")
					bAddress2 = FALSE;
			}
			var = rsPatient->Fields->Item["City"]->Value;
			if(var.vt != VT_BSTR)
				bCity = FALSE;
			else {
				CString strCity = CString(var.bstrVal);
				strCity.TrimRight();
				if(strCity == "")
					bCity = FALSE;
			}
			var = rsPatient->Fields->Item["State"]->Value;
			if(var.vt != VT_BSTR)
				bState = FALSE;
			else {
				CString strState = CString(var.bstrVal);
				strState.TrimRight();
				if(strState == "")
					bState = FALSE;
			}
			var = rsPatient->Fields->Item["Zip"]->Value;
			if(var.vt != VT_BSTR)
				bZip = FALSE;
			else {
				CString strZip = CString(var.bstrVal);
				strZip.TrimRight();
				if(strZip == "")
					bZip = FALSE;
			}
			if((!bAddress1 || !bCity || !bState || !bZip) && GetRemotePropertyInt("EbillingValidatePatientAddress",1,0,"<None>",TRUE)) {
				str.Format("\tWARNING - No Patient Address Found\r\n\r\n"
					"\t\tThis patient does not have a complete address filled in.\r\n"
					"\t\tTo fix this, go to the patient's General 1 tab and\r\n"
					"\t\tmake sure all the information is complete and correct.\r\n\r\n\r\n");
				strErrors += str;
				bWarnings = TRUE;
			}
			if(bAddress2 && !bAddress1) {	//this is NOT a warning that can be disabled
				str.Format("\tERROR - Patient Address Formatted Improperly\r\n\r\n"
					"\t\tThis patient has data in their Address 2 field but not in\r\n"
					"\t\ttheir Address 1 field. To fix this, go to the patient's\r\n"
					"\t\tGeneral 1 tab and make sure that the Address 1 field is filled in.\r\n\r\n\r\n");
				strErrors += str;
				bPassed = FALSE;
			}

			//Check for Birthdate
			BOOL bBirthDate = TRUE;
			var = rsPatient->Fields->Item["BirthDate"]->Value;
			if(var.vt != VT_DATE)
				bBirthDate = FALSE;
			if(!bBirthDate && GetRemotePropertyInt("EbillingValidatePatientBirthdate",1,0,"<None>",TRUE)) {
				str.Format("\tWARNING - No Patient BirthDate Found\r\n\r\n"
					"\t\tThis patient does not have a birthdate filled in.\r\n"
					"\t\tTo fix this, go to the patient's General 1 tab and\r\n"
					"\t\tmake sure all the information is complete and correct.\r\n\r\n\r\n");
				strErrors += str;
				bWarnings = TRUE;
			}

			// (j.jones 2009-08-10 10:55) - PLID 32886 - added gender validation,
			// validate Male or Female only, anything else is considered invalid
			if(GetRemotePropertyInt("EbillingValidatePatientGender",1,0,"<None>",TRUE)) {
				int iGender = AdoFldByte(rsPatient, "Gender", 0);
				if(iGender != 1 && iGender != 2) {
					str.Format("\tWARNING - No Patient Gender Found\r\n\r\n"
						"\t\tThis patient does not have a gender filled in.\r\n"
						"\t\tTo fix this, go to the patient's General 1 tab and\r\n"
						"\t\tmake sure all the information is complete and correct.\r\n\r\n\r\n");
					strErrors += str;
					bWarnings = TRUE;
				}
			}
		}
		rsPatient->Close();

		// (j.jones 2011-08-16 17:22) - PLID 44805 - we will filter out "original" and "void" charges
		// (j.jones 2012-01-04 09:30) - PLID 47301 - added NOCType and LineNote, as all NOC codes require a note
		// (r.gonet 03/26/2012) - PLID 49043 - Added RequireRefPhys flag and the bill's referring physician 
		// (j.jones 2013-07-15 16:17) - PLID 57566 - NOCType is now in ServiceT, it's only auto-calculated
		// for non-CPT codes if the ItemCode is filled on the charge
		// (d.singleton 2014-03-10 15:05) - PLID 61308 - refactor older ebilling validations to work with new diag code data structure
		// (j.jones 2014-04-25 09:40) - PLID 61912 - added Referring, Ordering, Supervising providers on charges
		// (j.jones 2014-04-25 09:44) - PLID 61835 - for Referring, Ordering, Supervising providers, this pulls a boolean
		// to confirm whether they are actually configured as that provider type
		// (j.jones 2014-04-25 10:25) - PLID 61919 - fixed bug where warnings were duplicated for each diag code on the bill
		_RecordsetPtr rsCharges2 = CreateParamRecordset("SELECT "
			"(SELECT TOP 1 DiagCodes.ID FROM BillDiagCodeT "
			"	INNER JOIN DiagCodes ON BillDiagCodeT.{CONST_STRING} = DiagCodes.ID "
			"	WHERE BillDiagCodeT.BillID = BillsT.ID ORDER BY BillDiagCodeT.OrderIndex) AS Diag1ID, "
			""
			"ChargeWhichCodesFlatV.WhichCodes, ChargesT.ServiceID, ChargesT.ItemCode AS Code, "
			"ChargesT.DoctorsProviders, LineItemT.LocationID, "
			"CPTCodeT.RequireRefPhys, BillsT.RefPhyID, "
			""
			"ChargesT.ReferringProviderID, ReferringProvPersonT.FullName AS ReferringProvName, "
			"Convert(bit, CASE WHEN ReferringProvProvidersT.PersonID Is Not Null THEN 1 ELSE 0 END) AS IsReferringProvAProvider, "
			"Coalesce(ReferringProvProvidersT.ReferringProvider, ReferringProvReferringPhysT.ReferringProvider) AS IsReferringProvOK, "
			""
			"ChargesT.OrderingProviderID, OrderingProvPersonT.FullName AS OrderingProvName, "
			"Convert(bit, CASE WHEN OrderingProvProvidersT.PersonID Is Not Null THEN 1 ELSE 0 END) AS IsOrderingProvAProvider, "
			"Coalesce(OrderingProvProvidersT.OrderingProvider, OrderingProvReferringPhysT.OrderingProvider) AS IsOrderingProvOK, "
			""
			"ChargesT.SupervisingProviderID, SupervisingProvPersonT.FullName AS SupervisingProvName, "
			"Convert(bit, CASE WHEN SupervisingProvProvidersT.PersonID Is Not Null THEN 1 ELSE 0 END) AS IsSupervisingProvAProvider, "
			"Coalesce(SupervisingProvProvidersT.SupervisingProvider, SupervisingProvReferringPhysT.SupervisingProvider) AS IsSupervisingProvOK, "
			""
			"LineNotesQ.LineNote, "
			//a CPT code is an NOC code if it has a keyword in this list, or NOCType forces it on (or off)
			"Convert(bit, CASE WHEN ServiceT.NOCType = 1 THEN 1 WHEN ServiceT.NOCType = 0 THEN 0 "
			"	WHEN (CPTCodeT.ID Is Not Null OR ChargesT.ItemCode <> '') "
			"		AND ("
			"			ServiceT.Name LIKE '%Not Otherwise Classified%' "
			"			OR ServiceT.Name LIKE '%Not Otherwise%' "
			"			OR ServiceT.Name LIKE '%Unlisted%' "
			"			OR ServiceT.Name LIKE '%Not listed%' "
			"			OR ServiceT.Name LIKE '%Unspecified%' "
			"			OR ServiceT.Name LIKE '%Unclassified%' "
			"			OR ServiceT.Name LIKE '%Not otherwise specified%' "
			"			OR ServiceT.Name LIKE '%Non-specified%' "
			"			OR ServiceT.Name LIKE '%Not elsewhere specified%' "
			"			OR ServiceT.Name LIKE '%Not elsewhere%' "
			"			OR ServiceT.Name LIKE '%nos' "
			"			OR ServiceT.Name LIKE '%nos %' "
			"			OR ServiceT.Name LIKE '%nos;%' "
			"			OR ServiceT.Name LIKE '%nos,%' "
			"			OR ServiceT.Name LIKE '%noc' "
			"			OR ServiceT.Name LIKE '%noc %' "
			"			OR ServiceT.Name LIKE '%noc;%' "
			"			OR ServiceT.Name LIKE '%noc,%' "
			"		)"
			"	THEN 1 ELSE 0 END) AS NOCCode "
			"FROM BillsT "
			"INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"INNER JOIN HCFATrackT ON BillsT.ID = HCFATrackT.BillID "
			"LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
			"LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
			"LEFT JOIN (SELECT Min(Note) AS LineNote, LineItemID FROM Notes "
			"	WHERE LineItemID Is Not Null AND SendOnClaim = 1 "
			"	GROUP BY LineItemID) AS LineNotesQ ON ChargesT.ID = LineNotesQ.LineItemID "
			"LEFT JOIN ChargeWhichCodesFlatV ON ChargesT.ID = ChargeWhichCodesFlatV.ChargeID "
			""
			"LEFT JOIN PersonT ReferringProvPersonT ON ChargesT.ReferringProviderID = ReferringProvPersonT.ID "
			"LEFT JOIN ProvidersT ReferringProvProvidersT ON ReferringProvPersonT.ID = ReferringProvProvidersT.PersonID "
			"LEFT JOIN ReferringPhysT ReferringProvReferringPhysT ON ReferringProvPersonT.ID = ReferringProvReferringPhysT.PersonID "

			"LEFT JOIN PersonT OrderingProvPersonT ON ChargesT.OrderingProviderID = OrderingProvPersonT.ID "
			"LEFT JOIN ProvidersT OrderingProvProvidersT ON OrderingProvPersonT.ID = OrderingProvProvidersT.PersonID "
			"LEFT JOIN ReferringPhysT OrderingProvReferringPhysT ON OrderingProvPersonT.ID = OrderingProvReferringPhysT.PersonID "
			""
			"LEFT JOIN PersonT SupervisingProvPersonT ON ChargesT.SupervisingProviderID = SupervisingProvPersonT.ID "
			"LEFT JOIN ProvidersT SupervisingProvProvidersT ON SupervisingProvPersonT.ID = SupervisingProvProvidersT.PersonID "
			"LEFT JOIN ReferringPhysT SupervisingProvReferringPhysT ON SupervisingProvPersonT.ID = SupervisingProvReferringPhysT.PersonID "
			""
			"WHERE LineItemT.Deleted = 0 AND ChargesT.Batched = 1 "
			"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
			"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
			"AND HCFATrackT.ID = {INT}", ShouldUseICD10(nInsuredPartyID, nBillID) ? "ICD10DiagID" : "ICD9DiagID", HCFAID);

		BOOL bDiag1Missing = FALSE;
		BOOL bWhichCodesMissing = FALSE;
		while(!rsCharges2->eof) {

			CString strCode = AdoFldString(rsCharges2, "Code", "");

			//loop through the charges, we want to see if whichcodes is empty for any charge
			var = rsCharges2->Fields->Item["Diag1ID"]->Value;
			if(var.vt == VT_NULL)
				bDiag1Missing = TRUE;
			var = rsCharges2->Fields->Item["WhichCodes"]->Value;
			CString strWhichCodes;
			strWhichCodes = VarString(var,"");
			if(strWhichCodes == "") {
				bWhichCodesMissing = TRUE;
			}

			// (a.wilson 2014-07-16 09:55) - PLID 62882 - retain these in code but comment them out since they are obsolete with ICD10.
			//else {
			//	//ANSI only: if the length is greater than 1 character and there is no comma,
			//	//or there is a decimal point, then we know the user chose a
			//	//ICD-9 code and not a DiagCode pointer. Bad, bad user!
			//	//this is NOT a warning that can be disabled
			//	if(m_FormatStyle == ANSI && ((strWhichCodes.GetLength() > 1 && strWhichCodes.Find(",") == -1) || strWhichCodes.Find(".") != -1)) {

			//		// (j.jones 2006-11-08 17:49) - PLID 23381 - rephrased "HCFA" to "claim"
			//		str.Format("\tERROR - Diagnosis Code Pointer Invalid\r\n\r\n"
			//			"\t\tA charge on the bill associated with this\r\n"
			//			"\t\tclaim has an invalid Diagnosis Code pointer filled in,\r\n"
			//			"\t\tas denoted in the 'DiagCs' column of each charge line\r\n"
			//			"\t\ton the bill. The value is '%s', and it should be a number\r\n"
			//			"\t\tcorresponding with an index of a diagnosis code on the bill.\r\n"
			//			"\t\t(Examples: '1', '1,2', etc.) To fix this, open up the\r\n"
			//			"\t\tpatient's bill and ensure 'DiagCs' column for each charge\r\n"
			//			"\t\tis filled in.\r\n\r\n\r\n",strWhichCodes);
			//		strErrors += str;
			//		bPassed = FALSE;
			//	}
			//	// (j.jones 2009-03-25 17:04) - PLID 33670 - if image, or not paper,
			//	// warn if they have a whichcodes index that is > 4
			//	// (j.jones 2013-08-09 14:43) - PLID 57299 - now this only applies to image or the old HCFA
			//	// (j.jones 2014-04-21 10:11) - PLID 61736 - Fixed the if statement because the format style is always
			//	// IMAGE if a paper HCFA. Now the validation is for all paper claims using the old form,
			//	// and electronic claims only if it's the HCFA image.
			//	else if(!bIsUB92 && ((!m_bElectronic && !UseNewHCFAForm(nInsuredPartyID, nBillID)) || (m_bElectronic && m_FormatStyle == IMAGE))) {

			//		BOOL bValid = TRUE;
			//		//if the whichcodes is one character, and it is greater than 4, it's invalid
			//		if(strWhichCodes.GetLength() == 1 && atoi(strWhichCodes) > 4) {
			//			bValid = FALSE;
			//		}
			//		//if the whichcodes is multiple characters, and not a diagnosis code,
			//		//make sure each index is not > 4
			//		else if(strWhichCodes.GetLength() > 1 && strWhichCodes.Find(",") != -1
			//			&& strWhichCodes.Find(".") == -1) {

			//			CString strToTest = strWhichCodes;

			//			int nComma = strToTest.Find(",");
			//			while(nComma != -1 && bValid) {

			//				CString strIndex = strToTest.Left(nComma);
			//				if(atoi(strIndex) > 4) {
			//					bValid = FALSE;
			//				}

			//				strToTest = strToTest.Right(strToTest.GetLength() - nComma - 1);
			//				
			//				nComma = strToTest.Find(",");
			//			}

			//			//check the final value
			//			if(atoi(strToTest) > 4) {
			//				bValid = FALSE;
			//			}
			//		}

			//		if(!bValid) {
			//			//this is NOT a warning that can be disabled
			//			str.Format("\tERROR - Diagnosis Code Pointer Invalid\r\n\r\n"
			//				"\t\tA charge on the bill associated with this claim has an invalid\r\n"
			//				"\t\tDiagnosis Code pointer filled in, as denoted in the 'DiagCs'\r\n"
			//				"\t\tcolumn of each charge line on the bill.\r\n"
			//				"\t\tThe value is '%s', and it is referencing a diagnosis\r\n"
			//				"\t\tcode that is not within the top four codes on the bill.\r\n"
			//				"\t\tOlder paper HCFA claims and HCFA Images do not support more than 4 diagnosis codes.\r\n"
			//				"\t\tTo fix this, open up the patient's bill and ensure 'DiagCs' column\r\n"
			//				"\t\tfor each charge does not reference any code index higher than 4.\r\n\r\n\r\n", strWhichCodes);
			//			strErrors += str;
			//			bPassed = FALSE;
			//		}
			//	}
			//}

			// (j.jones 2012-01-04 09:32) - PLID 47301 - If the code is an NOC code, but has no note entered,
			// give an error, as all NOC codes require notes.
			// ANSI 5010 only, not able to be turned off.
			if(m_bElectronic && m_FormatStyle == ANSI && m_avANSIVersion == av5010) {
				BOOL bNOCCode = VarBool(rsCharges2->Fields->Item["NOCCode"]->Value);
				CString strNote = AdoFldString(rsCharges2, "LineNote", "");
				strNote.Replace("\r\n"," ");
				//incase a \r or \n is on its own, remove it
				strNote.Replace("\r","");
				strNote.Replace("\n","");
				strNote.TrimLeft();
				strNote.TrimRight();

				//this will warn for each offending charge, meaning you can potentially
				//be warned multiple times on one claim
				if(bNOCCode && strNote.IsEmpty()) {

					str.Format("\tERROR - \"Not Otherwise Classified Code\" Missing A Claim Note.\r\n\r\n"
						"\t\tThe charge for code %s on this claim is a \"Not Otherwise Classified Code\" (NOC),\r\n"
						"\t\tand requires that a claim note be submitted to explain why the code was used.\r\n"
						"\t\tYou can enter a claim note by expanding the bill in the patient's Billing Tab\r\n"
						"\t\tand clicking the note icon for this charge. Enter a note for the charge and\r\n"
						"\t\tcheck the Claim box in order to submit it on the claim.\r\n"
						"\t\tIf this code is not supposed to be reported as a NOC code, you can edit the\r\n"
						"\t\tservice code in the Billing tab of the Administrator module, or the product in Inventory,\r\n"
						"\t\tand change the NOC Code dropdown to \"No\". A note will no longer be required.\r\n\r\n\r\n", strCode);
					strErrors += str;
					bPassed = FALSE;
				}
			}

			// (r.gonet 03/26/2012) - PLID 49043 - A CPT Code may require a referring physician, make sure we have one associated with
			//  this bill.
			// (j.luckoski 2012-04-04 10:58) - PLID 49419 - Make varbool always return a FALSE value if field is null
			// to prevent issue with inventory items on bills.
			BOOL bRequireRefPhys = VarBool(rsCharges2->Fields->Item["RequireRefPhys"]->Value, FALSE); // cannot be null, defaults to 0
			long nRefPhyID = VarLong(rsCharges2->Fields->Item["RefPhyID"]->Value, -1); // can be null, -1 (means same as null), or a FK to ReferringPhysT
			if(bRequireRefPhys && nRefPhyID == -1){
				
				// This CPT code requires the bill to have a referring physician but this bill lacks one, give an error
				str.Format("\tERROR - Code Requires Referring Physician.\r\n\r\n"
						"\t\tThe charge for code %s on this claim requires a referring physician on the claim,\r\n"
						"\t\tbut the referring physician is missing.\r\n"
						"\t\tYou can fix this by opening up the bill, going to the Insurance tab, and then \r\n"
						"\t\tselecting a referring physician from the dropdown.\r\n"
						"\t\tIf this code is not supposed to require a referring physician, you can edit the\r\n"
						"\t\tservice code in the Billing tab of the Administrator module and then uncheck the\r\n"
						"\t\tReq. Ref. Phys. check box. A referring physician will then no longer be required.\r\n\r\n\r\n", strCode);
					strErrors += str;
					bPassed = FALSE;
			} else {
				// Either this CPT code doesn't need a referring physician on the bill or this bill has one, so we're good.
			}

			// (j.jones 2014-04-25 10:36) - PLID 61912 - added validation that Referring, Ordering, Supervising providers
			// are selected on charges when the setup requires it
			// (j.jones 2014-04-25 10:39) - PLID 61835 - added validation that selected Referring, Ordering, Supervising
			// providers are actually configured as those provider types in contacts		
			// both of these apply only to ANSI 5010 HCFA claims or paper "new" HCFA claims
			// neither of these warnings can be turned off.
			if(!bIsUB92 && ((!m_bElectronic && UseNewHCFAForm(nInsuredPartyID, nBillID)) || (m_bElectronic && m_FormatStyle == ANSI && m_avANSIVersion == av5010))) {

				long nReferringProviderID = VarLong(rsCharges2->Fields->Item["ReferringProviderID"]->Value, -1);
				CString strReferringProviderName = VarString(rsCharges2->Fields->Item["ReferringProvName"]->Value, "");

				long nOrderingProviderID = VarLong(rsCharges2->Fields->Item["OrderingProviderID"]->Value, -1);
				CString strOrderingProviderName = VarString(rsCharges2->Fields->Item["OrderingProvName"]->Value, "");				

				long nSupervisingProviderID = VarLong(rsCharges2->Fields->Item["SupervisingProviderID"]->Value, -1);
				CString strSupervisingProviderName = VarString(rsCharges2->Fields->Item["SupervisingProvName"]->Value, "");

				// (j.jones 2014-04-25 11:30) - PLID 61912 - added validation that Referring, Ordering, Supervising providers
				// are selected on charges when the setup requires it
				{
					long nServiceID = VarLong(rsCharges2->Fields->Item["ServiceID"]->Value);
					long nChargeProviderID = VarLong(rsCharges2->Fields->Item["DoctorsProviders"]->Value, -1);
					long nLocationID = VarLong(rsCharges2->Fields->Item["LocationID"]->Value);

					// CalculateChargeClaimProviders caches its data, the bForceReload boolean tells it whether to wipe the cache.
					// If the boolean is false but we haven't called this function before, the data is still loaded.
					bool bForceReload = m_ClaimProviderChecker.Changed();

					ChargeClaimProviderSettings pSettings = CalculateChargeClaimProviders(nServiceID, nChargeProviderID, nLocationID, nInsuredPartyID, bForceReload);
					
					if (nReferringProviderID == -1 && pSettings.eReferringProviderOption != ChargeLevelProviderConfigOption::NoSelection) {
						str.Format("\tERROR - No Referring Provider Selected\r\n\r\n"
							"\t\tThe charge for code %s on this claim has no Referring Provider\r\n"
							"\t\tselected, but this code is configured to require a Referring Provider.\r\n"
							"\t\tYou can fix this by opening up the bill and adding a Referring Provider\r\n"
							"\t\tto this charge.\r\n\r\n\r\n",
							strCode);
						strErrors += str;
						bPassed = FALSE;
					}

					if (nOrderingProviderID == -1 && pSettings.eOrderingProviderOption != ChargeLevelProviderConfigOption::NoSelection) {
						str.Format("\tERROR - No Ordering Provider Selected\r\n\r\n"
							"\t\tThe charge for code %s on this claim has no Ordering Provider\r\n"
							"\t\tselected, but this code is configured to require a Ordering Provider.\r\n"
							"\t\tYou can fix this by opening up the bill and adding a Ordering Provider\r\n"
							"\t\tto this charge.\r\n\r\n\r\n",
							strCode);
						strErrors += str;
						bPassed = FALSE;
					}

					if (nSupervisingProviderID == -1 && pSettings.eSupervisingProviderOption != ChargeLevelProviderConfigOption::NoSelection) {
						str.Format("\tERROR - No Supervising Provider Selected\r\n\r\n"
							"\t\tThe charge for code %s on this claim has no Supervising Provider\r\n"
							"\t\tselected, but this code is configured to require a Supervising Provider.\r\n"
							"\t\tYou can fix this by opening up the bill and adding a Supervising Provider\r\n"
							"\t\tto this charge.\r\n\r\n\r\n",
							strCode);
						strErrors += str;
						bPassed = FALSE;
					}
				}

				// (j.jones 2014-04-25 10:39) - PLID 61835 - added validation that selected Referring, Ordering, Supervising
				// providers are actually configured as those provider types in contacts
				{
					bool bIsReferringProvOK = VarBool(rsCharges2->Fields->Item["IsReferringProvOK"]->Value, FALSE) ? true : false;
					bool bIsOrderingProvOK = VarBool(rsCharges2->Fields->Item["IsOrderingProvOK"]->Value, FALSE) ? true : false;
					bool bIsSupervisingProvOK = VarBool(rsCharges2->Fields->Item["IsSupervisingProvOK"]->Value, FALSE) ? true : false;

					if(nReferringProviderID != -1 && !bIsReferringProvOK) {

						CString strReferringProviderType = VarBool(rsCharges2->Fields->Item["IsReferringProvAProvider"]->Value, FALSE) ? "provider" : "referring physician";

						str.Format("\tWARNING - Invalid Referring Provider Selected\r\n\r\n"
							"\t\tThe charge for code %s on this claim has %s\r\n"
							"\t\t%s selected as the Referring Provider,\r\n"
							"\t\tbut this %s is not configured as a Referring Provider\r\n"
							"\t\tin the Contacts module.\r\n"
							"\t\tYou can fix this by editing this %s’s record in the\r\n"
							"\t\tContacts module and enabling the Referring Provider checkbox.\r\n\r\n\r\n", 
							strCode, strReferringProviderType,
							strReferringProviderName,
							strReferringProviderType,
							strReferringProviderType);
						strErrors += str;
						bWarnings = TRUE;
					}

					if(nOrderingProviderID != -1 && !bIsOrderingProvOK) {

						CString strOrderingProviderType = VarBool(rsCharges2->Fields->Item["IsOrderingProvAProvider"]->Value, FALSE) ? "provider" : "referring physician";

						str.Format("\tWARNING - Invalid Ordering Provider Selected\r\n\r\n"
							"\t\tThe charge for code %s on this claim has %s\r\n"
							"\t\t%s selected as the Ordering Provider,\r\n"
							"\t\tbut this %s is not configured as a Ordering Provider\r\n"
							"\t\tin the Contacts module.\r\n"
							"\t\tYou can fix this by editing this %s’s record in the\r\n"
							"\t\tContacts module and enabling the Ordering Provider checkbox.\r\n\r\n\r\n", 
							strCode, strOrderingProviderType,
							strOrderingProviderName,
							strOrderingProviderType,
							strOrderingProviderType);
						strErrors += str;
						bWarnings = TRUE;
					}

					if(nSupervisingProviderID != -1 && !bIsSupervisingProvOK) {
				
						CString strSupervisingProviderType = VarBool(rsCharges2->Fields->Item["IsSupervisingProvAProvider"]->Value, FALSE) ? "provider" : "referring physician";

						str.Format("\tWARNING - Invalid Supervising Provider Selected\r\n\r\n"
							"\t\tThe charge for code %s on this claim has %s\r\n"
							"\t\t%s selected as the Supervising Provider,\r\n"
							"\t\tbut this %s is not configured as a Supervising Provider\r\n"
							"\t\tin the Contacts module.\r\n"
							"\t\tYou can fix this by editing this %s’s record in the\r\n"
							"\t\tContacts module and enabling the Supervising Provider checkbox.\r\n\r\n\r\n", 
							strCode, strSupervisingProviderType,
							strSupervisingProviderName,
							strSupervisingProviderType,
							strSupervisingProviderType);
						strErrors += str;
						bWarnings = TRUE;
					}
				}
			}

			rsCharges2->MoveNext();
		}
		rsCharges2->Close();

		if(bDiag1Missing && GetRemotePropertyInt("EbillingValidateDiagCode1",1,0,"<None>",TRUE)) {

			// (j.jones 2006-11-08 17:49) - PLID 23381 - rephrased "HCFA" to "claim"
			// (r.gonet 03/31/2014) - PLID 61191 - Rephrased. We no longer have ICD-9 lists. Now an ICD-9/ICD-10 search plus diagnosis "boxes" that get populated automatically.
			str.Format("\tWARNING - No Diagnosis 1 Code Found\r\n\r\n"
				"\t\tThe bill associated with this claim does not have a\r\n"
				"\t\tDiagnosis 1 code filled in. To fix this, open up the patient's bill\r\n"
				"\t\tand select an appropriate diagnosis code.\r\n"
				"\t\tAlso make sure to update the 'DiagCs' column for each charge to\r\n"
				"\t\tassociate the new diagnosis code with each charge.\r\n\r\n\r\n");
			strErrors += str;
			bWarnings = TRUE;
		}

		if(bWhichCodesMissing && GetRemotePropertyInt("EbillingValidateDiagPointers",1,0,"<None>",TRUE)) {

			// (j.jones 2006-11-08 17:49) - PLID 23381 - rephrased "HCFA" to "claim"
			str.Format("\tWARNING - No Diagnosis Code Pointers Found\r\n\r\n"
				"\t\tAt least one charge on the bill associated with this\r\n"
				"\t\tclaim does not have a Diagnosis Code pointer filled in,\r\n"
				"\t\tas denoted in the 'DiagCs' column of each charge line\r\n"
				"\t\ton the bill. To fix this, open up the patient's bill\r\n"
				"\t\tand ensure 'DiagCs' column for each charge is filled in.\r\n\r\n\r\n");
			strErrors += str;
			bWarnings = TRUE;
		}
		

		//Check for ProviderID, and Taxonomy Code

		// (j.jones 2007-05-10 10:46) - PLID 25948 - supported ref. phy. for UB forms
		if(bIsUB92 && nBox82Setup == 3) { //referring physician
			//BillsT.RefPhyID
			// (b.eyers 2015-04-16) - PLID 41027 - Parameterize 
			_RecordsetPtr rsRefPhy = CreateParamRecordset("SELECT ReferringPhysT.PersonID, ReferringPhysT.NPI, PersonT.Last, PersonT.First, "
					"PersonT.SocialSecurity, ReferringPhysT.FedEmployerID AS RefPhyEIN "
					"FROM HCFATrackT "
					"INNER JOIN BillsT ON HCFATrackT.BillID = BillsT.ID "
					"LEFT JOIN ReferringPhysT ON BillsT.RefPhyID = ReferringPhysT.PersonID "
					"LEFT JOIN PersonT ON ReferringPhysT.PersonID = PersonT.ID "
					"WHERE BillsT.Deleted = 0 AND PersonT.ID Is Not Null AND HCFATrackT.ID = {INT}",HCFAID);
			if(!rsRefPhy->eof) {

				long nRefPhyID = AdoFldLong(rsRefPhy, "PersonID",-1);

				//Provider ID

				// (j.jones 2007-03-21 16:28) - PLID 25281 - this formerly only checked
				// the NPI when sending a paper HCFA, but should in fact check the NPI
				// at all times, for all forms

				//first validate the NPI number
				CString strNPI = AdoFldString(rsRefPhy, "NPI","");
				strNPI.TrimRight();
				if(strNPI.IsEmpty()) {

					// (j.jones 2008-05-21 15:05) - PLID 29280 - checking the provider NPI is now independent of
					// the regular provider IDs, and the checksum validation is a separate option as well
					
					if(GetRemotePropertyInt("EbillingValidateProviderNPI",1,0,"<None>",TRUE)) {

						str.Format("\tERROR - No Provider NPI Found\r\n\r\n"
							"\t\tThis Insurance Company's UB Group is set up to use the referring physician\r\n"
							"\t\tfrom the bill. The bill's referring physician, %s %s,\r\n"
							"\t\tdoes not have an NPI set up properly.\r\n"
							"\t\tTo fix this, go to the contacts module for this referring physician and\r\n"
							"\t\tfill in the NPI field.\r\n\r\n\r\n",
							AdoFldString(rsRefPhy, "First",""),AdoFldString(rsRefPhy, "Last",""));
						strErrors += str;
						bPassed = FALSE;
					}

				} else if (GetRemotePropertyInt("EbillingValidateNPIChecksum",1,0,"<None>",TRUE) && !IsValidNPI(strNPI)) {
					// (a.walling 2008-05-20 09:06) - PLID 27812 - Verify the NPI integrity if desired
					str.Format("\tERROR - Provider NPI is Invalid\r\n\r\n"
						"\t\tThis Insurance Company's UB Group is set up to use the referring physician\r\n"
						"\t\tfrom the bill. The bill's referring physician, %s %s,\r\n"
						"\t\tdoes not have a valid NPI number (%s).\r\n"
						"\t\tTo fix this, go to the contacts module for this referring physician and\r\n"
						"\t\tcheck your records to confirm the accuracy of the NPI field.\r\n\r\n\r\n",
						AdoFldString(rsRefPhy, "First",""),AdoFldString(rsRefPhy, "Last",""), strNPI);
					strErrors += str;
					bPassed = FALSE;
				}

				// (j.jones 2010-07-27 11:50) - PLID 39784 - supported this on UB claims
				if(GetRemotePropertyInt("EbillingValidateAssignmentOfBenefits",1,0,"<None>",TRUE) == 1) {

					//get the accept assignment value
					//the ebilling export only sets accepted to false if no active providers accept
					BOOL bAccepted = TRUE;
					
					//we only set accepted to false if no active providers accept, so simply search for whether
					//any provider accepts (which they would do if a InsuranceAcceptedT record was missing)
					_RecordsetPtr rsAcc = CreateParamRecordset("SELECT TOP 1 PersonID "
						"FROM ProvidersT "
						"INNER JOIN PersonT ON ProvidersT.PersonID = PersonT.ID "
						"LEFT JOIN (SELECT ProviderID, Accepted FROM InsuranceAcceptedT WHERE InsuranceCoID = {INT}) AS InsuranceAcceptedQ "
						"	ON ProvidersT.PersonID = InsuranceAcceptedQ.ProviderID "
						"WHERE PersonT.Archived = 0 "
						"AND (InsuranceAcceptedQ.Accepted = 1 OR InsuranceAcceptedQ.Accepted Is Null)", InsuranceCoID);
					if(rsAcc->eof) {
						//empty, which means no active providers accept assignment
						bAccepted = FALSE;
					}

					//now, using the accepted value, calculate whether we fill Box 53
					BOOL bFillBox53 = TRUE;
					if(nUBBox53Accepted == 0) {
						//fill if accepted
						bFillBox53 = bAccepted;
					}
					else if(nUBBox53Accepted == 1) {
						//always yes
						bFillBox53 = TRUE;
					}
					else if(nUBBox53Accepted == 2) {
						//always no
						bFillBox53 = FALSE;
					}

					//check the bill override - the field is called HCFA Box 13 but it applies to UBs too
					if(hb13Value == hb13_No || (hb13Value == hb13_UseDefault && !bFillBox53)) {
						bFillBox53 = FALSE;
					}
					else {
						bFillBox53 = TRUE;
					}

					//warn if not filled
					if(!bFillBox53) {
						str.Format("\tWARNING - Assignment of Benefits not set - Patient will be paid.\r\n\r\n"
								"\t\tThe Assignment of Benefits indicator (UB Box 53) is configured to be not filled in\r\n"
								"\t\ton this claim. If this is not filled, the patient will receive a check from the\r\n"
								"\t\tinsurance company instead of the office.\r\n"
								"\t\tIf this is not intended, check your Box 53 setting in the UB Setup. \r\n"
								"\t\tAlso ensure that the Assignment Of Benefits override is not used on the Insurance \r\n"
								"\t\ttab of this Bill.\r\n\r\n\r\n");
						if(GetUBFormType() != eUB04) {
							str.Replace("UB04", "UB92");
						}
						strErrors += str;
						bWarnings = TRUE;
					}
				}

				//check Provider IDs, unless the preference says not to
				// (j.jones 2011-03-15 10:58) - PLID 42788 - this now follows EbillingValidateProviderOtherIDs
				if(GetRemotePropertyInt("EbillingValidateProviderOtherIDs",GetRemotePropertyInt("EbillingValidateProviderIDs",1,0,"<None>",FALSE),0,"<None>",TRUE)) {

					// (j.jones 2010-04-14 14:56) - PLID 27457 - If ANSI, call ValidateAllANSIProviderREFIDs,
					// but for a referring physician that will only check the Additional REF IDs,
					// so we will still need the traditional ID loading after this.
					if(m_bElectronic && m_FormatStyle == ANSI) {

						CString strProviderFirst = AdoFldString(rsRefPhy, "First","");
						CString strProviderLast = AdoFldString(rsRefPhy, "Last","");
						CString strProvSSN = AdoFldString(rsRefPhy, "SocialSecurity","");
						CString strProvEIN = AdoFldString(rsRefPhy, "RefPhyEIN","");

						//validate all possible ANSI REF IDs for this provider
						ValidateAllANSIProviderREFIDs(bIsUB92, TRUE, nRefPhyID, TRUE, TRUE,
							InsuranceCoID, nLocationID, nInsuredPartyID,
							HCFASetupGroupID, strDefaultBox33GRP, HCFAProvType,
							UB92SetupGroupID, strUB04Box76Qual, nBox82Num, nBox82Setup,
							strErrors, bPassed, bWarnings, bDisableREFXX,
							strProviderLast, strProviderFirst,
							bSend2010AB, bSend2420A,
							nExtraREF_IDType, bIsGroup, nHCFABox25,
							strLocationEIN, strProvEIN, strProvSSN,
							bUse_Addnl_2010AA, strAddnl_2010AA_Qual, strAddnl_2010AA);
					}

					// (j.jones 2010-04-14 11:10) - PLID 27457 - do not use the ANSI functions here
					BOOL bProviderFound = TRUE;
					// (a.walling 2008-05-20 10:24) - PLID 27812 - Validate the NPI if appropriate
					BOOL bValidNPI = TRUE;
					CString strIDName;
					if(nBox82Num != -1) {
						// (j.jones 2007-04-25 09:48) - PLID 25277 - if UB04 or paper, we don't need the "limited" check
						if(!m_bElectronic || GetUBFormType() == eUB04) {
							bProviderFound = CheckBox82(UB92SetupGroupID, nBox82Num, nBox82Setup, AdoFldLong(rsRefPhy, "PersonID",-1), -1, InsuranceCoID, strIDName, bValidNPI);
						}
						else {
							bProviderFound = CheckBox82ANSILimited(UB92SetupGroupID, nBox82Num, nBox82Setup, AdoFldLong(rsRefPhy, "PersonID",-1), -1, strIDName, bValidNPI);
						}
					}

					if(!bProviderFound && m_FormatStyle == ANSI) {

						//check and see if an override is in place for the 2010AA field

						// (j.jones 2008-04-29 16:51) - PLID 29619 - we allow blank overrides, so I changed this check
						// to validate that an override is enabled, the program disallows saving bad data in the override
						if(ReturnsRecords("SELECT ANSI_2010AA, ANSI_2010AA_Qual FROM UB92EbillingSetupT WHERE Use_2010AA = 1 AND SetupGroupID = %li AND ProviderID = %li",UB92SetupGroupID,AdoFldLong(rsRefPhy, "PersonID",-1))) {
							bProviderFound = TRUE;
						}
					}

					if(!bProviderFound) {
						if(strIDName != "Invalid") {
							str.Format("\tERROR - No Provider ID Found\r\n\r\n"
								"\t\tThis Insurance Company's UB Group is set up to use the referring physician\r\n"
								"\t\tfrom the bill. The bill's referring physician, %s %s,\r\n"
								"\t\tdoes not have an insurance ID set up properly. The current setting for the UB group\r\n"
								"\t\tis to use the referring physician's %s from the Contacts module.\r\n"
								"\t\tTo fix this, go to the contacts module for this referring physician and fill in a value,\r\n"
								"\t\tor change the Box 76 Number on this insurance company's UB group to another value.\r\n\r\n\r\n",AdoFldString(rsRefPhy, "First",""),AdoFldString(rsRefPhy, "Last",""),strIDName);
							if(GetUBFormType() != eUB04) {
								str.Replace("Box 76 Number", "Box 82 Number");
							}
						}
						else {
							str.Format("\tERROR - No Valid Provider ID Found\r\n\r\n"
								"\t\tThis Insurance Company's UB Group is set up to use the referring physician\r\n"
								"\t\tfrom the bill. The bill's referring physician, %s %s,\r\n"
								"\t\tdoes not have an insurance ID set up properly. The current setting for the UB group\r\n"
								"\t\tis not using a standard provider ID. To fix this, change the\r\n"
								"\t\tBox 76 Number on this insurance company's UB group to another value.\r\n\r\n\r\n",AdoFldString(rsRefPhy, "First",""),AdoFldString(rsRefPhy, "Last",""));
							if(GetUBFormType() != eUB04) {
								str.Replace("Box 76 Number", "Box 82 Number");
							}
						}
						strErrors += str;
						bPassed = FALSE;
					} else if (!bValidNPI) {
						// (a.walling 2008-05-20 10:19) - PLID 27812
						str.Format("\tERROR - Provider NPI is Invalid\r\n\r\n"
							"\t\tThis Insurance Company's UB Group is set up to use the referring physician\r\n"
							"\t\tfrom the bill. The bill's referring physician, %s %s,\r\n"
							"\t\tdoes not have a valid NPI. To fix this, go to the contacts module\r\n"
							"\t\tfor the referring physician and verify the NPI's accuracy, or change the\r\n"
							"\t\tBox 76 Number on this insurance company's UB group to another value.\r\n\r\n\r\n",AdoFldString(rsRefPhy, "First",""),AdoFldString(rsRefPhy, "Last",""));
						if(GetUBFormType() != eUB04) {
							str.Replace("Box 76 Number", "Box 82 Number");
						}

						strErrors += str;
						bPassed = FALSE;
					}

					//qualifier is validated later
				}

				// (j.jones 2013-09-05 13:56) - PLID 58451 - referring physicians don't have
				// taxonomy codes anymore
				/*
				if(m_FormatStyle == ANSI && m_bElectronic && GetRemotePropertyInt("EbillingValidateProviderTaxonomyCode",1,0,"<None>",TRUE)) {
					//Taxonomy Code
					CString strTaxonomy = AdoFldString(rsRefPhy, "TaxonomyCode","");
					strTaxonomy.TrimRight();
					if(strTaxonomy == "") {
						str.Format("\tERROR - No Referring Physician Taxonomy Code Found\r\n\r\n"
								"\t\tThis Insurance Company's UB Group is set up to use the referring physician\r\n"
								"\t\tfrom the bill. The bill's referring physician, %s %s,\r\n"
								"\t\tdoes not have a Taxonomy Code set up properly. To fix this, go to the contacts\r\n"
								"\t\tmodule for this referring physician and fill in a value for the Taxonomy Code.\r\n\r\n\r\n",AdoFldString(rsRefPhy, "First",""),AdoFldString(rsRefPhy, "Last",""));
						strErrors += str;
						bPassed = FALSE;
					}
				}
				*/
			}
			rsRefPhy->Close();
		}
		else {
			//either bill provider or G1 provider

			// (j.jones 2006-12-12 12:00) - PLID 23835 - here we need to check the data for the Claim Provider
			// selected on the actual provider's Contacts tab

			// (j.jones 2010-11-11 08:30) - PLID 41396 - reworked all of this to handle the correct providers,
			// using the claim provider information

			//first check the billing provider information
			_RecordsetPtr rsBillProv = CreateParamRecordset("SELECT NPI, Last, First, "
				"SocialSecurity, [Fed Employer ID] AS ProvEIN, TaxonomyCode "
				"FROM PersonT "
				"INNER JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID "
				"WHERE PersonT.ID = {INT}", nBillingProviderID);
			if(!rsBillProv->eof) {

				// (j.jones 2007-03-21 16:28) - PLID 25281 - this formerly only checked
				// the NPI when sending a paper HCFA, but should in fact check the NPI
				// at all times, for all forms

				//first validate the NPI number
				CString strNPI = AdoFldString(rsBillProv, "NPI","");
				strNPI.TrimRight();

				if(strNPI.IsEmpty()) {

					// (j.jones 2008-05-21 15:05) - PLID 29280 - checking the provider NPI is now independent of
					// the regular provider IDs, and the checksum validation is a separate option as well
					
					if(GetRemotePropertyInt("EbillingValidateProviderNPI",1,0,"<None>",TRUE)) {

						// (j.jones 2010-11-11 08:36) - PLID 41396 - output the error based off
						// their settings, if they are normally set to use the G1 provider, say so,
						// even though the claim providers can override that

						if((!bIsUB92 && HCFAProvType == 2) || (bIsUB92 && nBox82Setup == 2)) {
							//general 1 provider
							str.Format("\tERROR - No Provider NPI Found\r\n\r\n"
								"\t\tThis Insurance Company's HCFA Group is set up to use the patient's\r\n"
								"\t\tdefault provider from General 1. The provider's assigned claim\r\n"
								"\t\tprovider, '%s %s', for this patient's account does not have a\r\n"
								"\t\tnumber set up properly.\r\n"
								"\t\tTo fix this, go to the contacts module for this provider and\r\n"
								"\t\tfill in the NPI field.\r\n\r\n\r\n",
								AdoFldString(rsBillProv, "First",""),AdoFldString(rsBillProv, "Last",""));

							if(bIsUB92) {
								str.Replace("HCFA", "UB");
							}
						}
						else {
							//bill provider
							str.Format("\tERROR - No Provider NPI Found\r\n\r\n"
								"\t\tThe provider's claim provider,'%s %s',\r\n"
								"\t\ton this patient's bill does not have a NPI number\r\n"
								"\t\tset up properly. To fix this, go to the contacts\r\n"
								"\t\tmodule for this provider and fill in the NPI field.\r\n\r\n\r\n",
								AdoFldString(rsBillProv, "First",""),AdoFldString(rsBillProv, "Last",""));
						}
						strErrors += str;
						bPassed = FALSE;
					}

				} else if (GetRemotePropertyInt("EbillingValidateNPIChecksum",1,0,"<None>",TRUE) && !IsValidNPI(strNPI)) {

					// (j.jones 2010-11-11 08:36) - PLID 41396 - output the error based off
					// their settings, if they are normally set to use the G1 provider, say so,
					// even though the claim providers can override that

					if((!bIsUB92 && HCFAProvType == 2) || (bIsUB92 && nBox82Setup == 2)) {
						//general 1 provider
						// (a.walling 2008-05-20 09:06) - PLID 27812 - Verify the NPI integrity if desired
						str.Format("\tERROR - Provider NPI is invalid\r\n\r\n"
							"\t\tThis Insurance Company's HCFA Group is set up to use the patient's\r\n"
							"\t\tdefault provider from General 1. The provider's assigned claim\r\n"
							"\t\tprovider, '%s %s', for this patient's account does not have a\r\n"
							"\t\tvalid NPI number (%s).\r\n"
							"\t\tTo fix this, go to the contacts module for this provider and\r\n"
							"\t\tcheck your records to confirm the accuracy of the NPI field.\r\n\r\n\r\n",
							AdoFldString(rsBillProv, "First",""),AdoFldString(rsBillProv, "Last",""), strNPI);

						if(bIsUB92) {
							str.Replace("HCFA", "UB");
						}
					}
					else {
						//bill provider
						// (a.walling 2008-05-20 09:06) - PLID 27812 - Verify the NPI integrity if desired
						str.Format("\tERROR - Provider NPI is Invalid\r\n\r\n"
							"\t\tThe provider's claim provider,'%s %s',\r\n"
							"\t\ton this patient's bill has an invalid NPI number (%s).\r\n"
							"\t\tTo fix this, go to the contacts module for this\r\n"
							"\t\tprovider and check your records to confirm the\r\n"
							"\t\taccuracy of the NPI field.\r\n\r\n\r\n",
							AdoFldString(rsBillProv, "First",""),AdoFldString(rsBillProv, "Last",""), strNPI);
					}
					strErrors += str;
					bPassed = FALSE;
				}

				// (j.jones 2010-07-23 11:44) - PLID 39797 - added assignment of benefits validation,
				// applies to all HCFA formats only
				// (j.jones 2010-07-27 11:50) - PLID 39784 - this now applies to UB claims
				if(GetRemotePropertyInt("EbillingValidateAssignmentOfBenefits",1,0,"<None>",TRUE) == 1) {

					//get the accept assignment value
					// (j.jones 2010-07-23 15:02) - PLID 39783 - accept assignment is calculated in GetAcceptAssignment_ByInsCo now
					BOOL bAccepted = GetAcceptAssignment_ByInsCo(InsuranceCoID, nBillingProviderID);

					//now, using the accepted value, calculate whether we fill Box 13
					if(!bIsUB92) {
						BOOL bFillBox13 = TRUE;
						if(nBox13Accepted == 0) {
							//never fill
							bFillBox13 = FALSE;
						}
						else if(nBox13Accepted == 1) {
							//fill if accepted
							bFillBox13 = bAccepted;
						}
						else if(nBox13Accepted == 2) {
							//fill if not accepted
							bFillBox13 = !bAccepted;
						}
						else if(nBox13Accepted == 3) {
							//always fill
							bFillBox13 = TRUE;
						}

						//does the bill have Box 13 overridden?
						if(hb13Value == hb13_No || (hb13Value == hb13_UseDefault && !bFillBox13)) {
							bFillBox13 = FALSE;
						}
						else {
							bFillBox13 = TRUE;
						}

						//warn if not filled
						if(!bFillBox13) {
							str.Format("\tWARNING - Assignment of Benefits not set - Patient will be paid.\r\n\r\n"
								"\t\tThe Assignment of Benefits indicator (HCFA Box 13) is configured to be not filled in\r\n"
								"\t\ton this claim. If this is not filled, the patient will receive a check from the\r\n"
								"\t\tinsurance company instead of the office.\r\n"
								"\t\tIf this is not intended, check your Fill Box 13 setting in the HCFA Setup, \r\n"
								"\t\tConfigure Fields To Show Or Hide. Also ensure that the Assignment Of Benefits \r\n"
								"\t\toverride is not used on the Insurance tab of this Bill.\r\n\r\n\r\n");
							strErrors += str;
							bWarnings = TRUE;
						}
					}
					else {
						// (j.jones 2010-07-27 11:51) - PLID 39784 - supported this setting for UB claims

						BOOL bFillBox53 = TRUE;
						if(nUBBox53Accepted == 0) {
							//fill if accepted
							bFillBox53 = bAccepted;
						}
						else if(nUBBox53Accepted == 1) {
							//always yes
							bFillBox53 = TRUE;
						}
						else if(nUBBox53Accepted == 2) {
							//always no
							bFillBox53 = FALSE;
						}

						//check the bill override - the field is called HCFA Box 13 but it applies to UBs too
						if(hb13Value == hb13_No || (hb13Value == hb13_UseDefault && !bFillBox53)) {
							bFillBox53 = FALSE;
						}
						else {
							bFillBox53 = TRUE;
						}

						//warn if not filled
						if(!bFillBox53) {
							str.Format("\tWARNING - Assignment of Benefits not set - Patient will be paid.\r\n\r\n"
								"\t\tThe Assignment of Benefits indicator (UB Box 53) is configured to be not filled in\r\n"
								"\t\ton this claim. If this is not filled, the patient will receive a check from the\r\n"
								"\t\tinsurance company instead of the office.\r\n"
								"\t\tIf this is not intended, check your Box 53 setting in the UB Setup. \r\n"
								"\t\tAlso ensure that the Assignment Of Benefits override is not used on the Insurance \r\n"
								"\t\ttab of this Bill.\r\n\r\n\r\n");
							if(GetUBFormType() != eUB04) {
								str.Replace("UB04", "UB92");
							}
							strErrors += str;
							bWarnings = TRUE;
						}
					}
				}

				//check Provider IDs, unless the preference says not to
				// (j.jones 2011-03-15 10:58) - PLID 42788 - this now follows EbillingValidateProviderBox24JID
				if(GetRemotePropertyInt("EbillingValidateProviderBox24JID",GetRemotePropertyInt("EbillingValidateProviderIDs",1,0,"<None>",FALSE),0,"<None>",TRUE)) {

					//check for the Box24J Qualifier (this should not be disabled by a preference)
					// (j.jones 2010-11-11 10:11) - PLID 41396 - validate 24J only if the billing and rendering
					// providers are the same, otherwise we will do it later in the rendering provider code
					if((nBillingProviderID == nRenderingProviderID) && !bIsUB92 && (!m_bElectronic || m_FormatStyle == IMAGE)) {
						_RecordsetPtr rs24J = CreateParamRecordset("SELECT Box24JNumber FROM InsuranceBox24J "
							"WHERE Box24JNumber <> '' AND RTRIM(Box24IQualifier) = '' "
							"AND ProviderID = {INT} AND InsCoID = {INT}", nBillingProviderID, InsuranceCoID);
						// (j.jones 2013-05-15 14:42) - PLID 56711 - check for NOT eof, because this query
						// is looking for the presence of bad data
						if(!rs24J->eof) {

							// (j.jones 2010-11-11 08:36) - PLID 41396 - output the error based off
							// their settings, if they are normally set to use the G1 provider, say so,
							// even though the claim providers can override that

							if((!bIsUB92 && HCFAProvType == 2) || (bIsUB92 && nBox82Setup == 2)) {
								//general 1 provider
								str.Format("\tERROR - No Box 24J ID Qualifier Found\r\n\r\n"
									"\t\tThis Insurance Company's HCFA Group is set up to use the patient's\r\n"
									"\t\tdefault provider from General 1. The provider's assigned claim\r\n"
									"\t\tprovider, '%s %s', for this patient's account has a Box24J ID\r\n"
									"\t\tconfigured for the insurance company, but does not have an ID\r\n"
									"\t\tqualifier set up properly.\r\n"
									"\t\tTo fix this, go to the 'Edit Insurance List', available in the\r\n"
									"\t\tInsurance Tab of the Patients Module, or the HCFA Tab of the\r\n"
									"\t\tAdministrator Module, edit the Box24J setup for the selected\r\n"
									"\t\tinsurance company and provider, and enter in a 2-character\r\n"
									"\t\tqualifier to identify the type of ID.\r\n\r\n\r\n",
									AdoFldString(rsBillProv, "First",""),AdoFldString(rsBillProv, "Last",""));
							}
							else {
								//bill provider
								str.Format("\tERROR - No Box 24J ID Qualifier Found\r\n\r\n"
									"\t\tThe provider's claim provider,'%s %s',\r\n"
									"\t\ton this patient's bill, has a Box24J ID configured for the\r\n"
									"\t\tinsurance company, but does not have an ID qualifier set up properly.\r\n"
									"\t\tTo fix this, go to the 'Edit Insurance List', available in the\r\n"
									"\t\tInsurance Tab of the Patients Module, or the HCFA Tab of the\r\n"
									"\t\tAdministrator Module, edit the Box24J setup for the selected\r\n"
									"\t\tinsurance company and provider, and enter in a 2-character\r\n"
									"\t\tqualifier to identify the type of ID.\r\n\r\n\r\n",
									AdoFldString(rsBillProv, "First",""),AdoFldString(rsBillProv, "Last",""));
							}

							strErrors += str;
							bPassed = FALSE;
						}
						rs24J->Close();
					}
				}

				// (j.jones 2011-03-15 10:58) - PLID 42788 - this now follows EbillingValidateProviderOtherIDs
				if(GetRemotePropertyInt("EbillingValidateProviderOtherIDs",GetRemotePropertyInt("EbillingValidateProviderIDs",1,0,"<None>",FALSE),0,"<None>",TRUE)) {

					// (j.jones 2010-04-14 11:10) - PLID 27457 - If ANSI, use the modular functions
					// to load the appropriate ID and Qualifier, then yell if something is wrong.
					// Not not attempt to validate the NPI in this field.
					if(m_bElectronic && m_FormatStyle == ANSI) {

						CString strProviderFirst = AdoFldString(rsBillProv, "First","");
						CString strProviderLast = AdoFldString(rsBillProv, "Last","");
						CString strProvSSN = AdoFldString(rsBillProv, "SocialSecurity","");
						CString strProvEIN = AdoFldString(rsBillProv, "ProvEIN","");

						//validate all possible ANSI REF IDs for this provider
						// (j.jones 2010-11-11 10:05) - PLID 41396 - in the unlikely case that the billing
						// and rendering providers are different, we won't validate the rendering provider just yet
						ValidateAllANSIProviderREFIDs(bIsUB92, FALSE, nBillingProviderID, TRUE, nBillingProviderID == nRenderingProviderID,
							InsuranceCoID, nLocationID, nInsuredPartyID,
							HCFASetupGroupID, strDefaultBox33GRP, HCFAProvType,
							UB92SetupGroupID, strUB04Box76Qual, nBox82Num, nBox82Setup,
							strErrors, bPassed, bWarnings, bDisableREFXX,
							strProviderLast, strProviderFirst,
							bSend2010AB, bSend2420A,
							nExtraREF_IDType, bIsGroup, nHCFABox25,
							strLocationEIN, strProvEIN, strProvSSN,
							bUse_Addnl_2010AA, strAddnl_2010AA_Qual, strAddnl_2010AA);
					}
					else {

						BOOL bProviderFound = TRUE;
						// (a.walling 2008-05-20 10:24) - PLID 27812 - Validate the NPI if appropriate
						BOOL bValidNPI = TRUE;
						CString strIDName;
						if(bIsUB92) {
							if(nBox82Num != -1) {
								// (j.jones 2007-04-25 09:48) - PLID 25277 - if UB04 or paper, we don't need the "limited" check
								if(!m_bElectronic || GetUBFormType() == eUB04) {
									bProviderFound = CheckBox82(UB92SetupGroupID, nBox82Num, nBox82Setup, nBillingProviderID, nLocationID, InsuranceCoID, strIDName, bValidNPI);
								}
								else {
									bProviderFound = CheckBox82ANSILimited(UB92SetupGroupID, nBox82Num, nBox82Setup, nBillingProviderID, nLocationID, strIDName, bValidNPI);
								}
							}
						}
						else {
							if(Box33 != -1) {
								bProviderFound = CheckBox33Pin(HCFASetupGroupID, Box33, nBillingProviderID, nLocationID, InsuranceCoID, strIDName, bValidNPI);
							}
						}

						CString strProvType = "";
						if (!bProviderFound || !bValidNPI) {
							if(!bIsUB92 && HCFAProvType == 3) {
								strProvType = "This Insurance Company's HCFA Group is set up to use a custom\r\n"
								"\t\tprovider, however for some data the bill provider is being used.\r\n\t\t";
							}
							else if(!bIsUB92 && HCFAProvType == 4) {
								strProvType = "This Insurance Company's HCFA Group is set up to use the bill\r\n"
								"\t\tlocation as the provider, however for some data the bill provider is being used.\r\n\t\t";
							}
							else if(bIsUB92 && nBox82Setup == 3) {
								//TES 3/13/2007 - PLID 24993 - Changed from UB92 to UB
								strProvType = "This Insurance Company's UB Group is set up to use the\r\n"
								"\t\treferring physician as the provider, which is not allowed for electronic billing.\r\n"
								"\t\tInstead, the bill provider is being used.\r\n\t\t";
							}
						}

						if(!bProviderFound) {
							if(strIDName != "Invalid") {

								if((!bIsUB92 && HCFAProvType == 2) || (bIsUB92 && nBox82Setup == 2)) {
									//general 1 provider
									str.Format("\tERROR - No Provider ID Found\r\n\r\n"
										"\t\tThis Insurance Company's HCFA Group is set up to use the patient's\r\n"
										"\t\tdefault provider from General 1. The provider's claim provider,\r\n"
										"\t\t'%s %s', for this patient's account does not have an\r\n"
										"\t\tinsurance ID set up properly. The current setting for the HCFA group\r\n"
										"\t\tis to use the provider's %s from the Contacts module.\r\n"
										"\t\tTo fix this, go to the contacts module\r\n"
										"\t\tfor this provider and fill in a value, or change the Box 33 PIN on\r\n"
										"\t\tthis insurance company's HCFA group to another value.\r\n\r\n\r\n",AdoFldString(rsBillProv, "First",""),AdoFldString(rsBillProv, "Last",""),strIDName);
								}
								else {
									//bill provider
									str.Format("\tERROR - No Provider ID Found\r\n\r\n"
										"\t\t%sThe provider's claim provider,'%s %s', on this\r\n"
										"\t\tpatient's bill does not have an insurance ID set up properly.\r\n"
										"\t\tThe current setting for the HCFA group is to use the provider's\r\n"
										"\t\t%s from the Contacts module.\r\n"
										"\t\tTo fix this, go to the contacts module for this provider and fill in\r\n"
										"\t\ta value, or change the Box 33 PIN on this insurance company's HCFA\r\n"
										"\t\tgroup to another value.\r\n\r\n\r\n",strProvType,AdoFldString(rsBillProv, "First",""),AdoFldString(rsBillProv, "Last",""),strIDName);
								}
								if(bIsUB92 && m_FormatStyle == ANSI) {
									//TES 3/13/2007 - PLID 24993 - Changed from UB92 to UB
									str.Replace("HCFA", "UB");
									//TES 3/13/2007 - PLID 24993 - UB92 Box 82 = UB04 Box 76
									if(GetUBFormType() == eUB04) {
										str.Replace("Box 33 PIN", "Box 76 Number");
									}
									else {
										str.Replace("Box 33 PIN", "Box 82 Number");
									}
								}
							}
							else {
								if((!bIsUB92 && HCFAProvType == 2) || (bIsUB92 && nBox82Setup == 2)) {
									//general 1 provider
									str.Format("\tERROR - No Valid Provider ID Found\r\n\r\n"
										"\t\tThe provider's claim provider, '%s %s',\r\n"
										"\t\tfor this patient's account does not have an insurance ID set\r\n"
										"\t\tup properly. The current setting for the HCFA group is not\r\n"
										"\t\tusing a standard provider ID. To fix this, change the\r\n"
										"\t\tBox 33 PIN on this insurance company's HCFA group to another value.\r\n\r\n\r\n",AdoFldString(rsBillProv, "First",""),AdoFldString(rsBillProv, "Last",""));
								}
								else {
									//bill provider
									str.Format("\tERROR - No Valid Provider ID Found\r\n\r\n"
										"\t\t%sThe provider's claim provider,'%s %s',\r\n"
										"\t\ton this patient's bill does not have an insurance ID\r\n"
										"\t\tset up properly. The current setting for the HCFA group\r\n"
										"\t\tis not using a standard provider ID. To fix this, change the\r\n"
										"\t\tBox 33 PIN on this insurance company's HCFA group to another value.\r\n\r\n\r\n",strProvType,AdoFldString(rsBillProv, "First",""),AdoFldString(rsBillProv, "Last",""));
								}
								if(bIsUB92 && m_FormatStyle == ANSI) {
									//TES 3/13/2007 - PLID 24993 - Changed from UB92 to UB
									str.Replace("HCFA", "UB");
									//TES 3/13/2007 - PLID 24993 - UB92 Box 82 = UB04 Box 76
									if(GetUBFormType() == eUB04) {
										str.Replace("Box 33 PIN", "Box 76 Number");
									}
									else {
										str.Replace("Box 33 PIN", "Box 82 Number");
									}
								}
							}
							strErrors += str;
							bPassed = FALSE;
						} else if (!bValidNPI) {
							// (a.walling 2008-05-20 10:19) - PLID 27812

							if((!bIsUB92 && HCFAProvType == 2) || (bIsUB92 && nBox82Setup == 2)) {
								//general 1 provider
								str.Format("\tERROR - No Valid Provider ID Found\r\n\r\n"
									"\t\tThe provider's claim provider, '%s %s',\r\n"
									"\t\tfor this patient's account does not have a\r\n"
									"\t\tvalid NPI. To fix this, go to the contacts module\r\n"
									"\t\tfor the provider and verify its accuracy, or change\r\n"
									"\t\tthe Box 33 PIN on this insurance company's HCFA\r\n"
									"\t\tgroup to another value.\r\n\r\n\r\n",AdoFldString(rsBillProv, "First",""),AdoFldString(rsBillProv, "Last",""));
							}
							else {
								//bill provider
								str.Format("\tERROR - No Valid Provider ID Found\r\n\r\n"
									"\t\t%sThe provider's claim provider,'%s %s',\r\n"
									"\t\ton this patient's bill does not have a\r\n"
									"\t\tvalid NPI. To fix this, go to the contacts module\r\n"
									"\t\tfor the provider and verify its accuracy, or change\r\n"
									"\t\tthe Box 33 PIN on this insurance company's HCFA\r\n"
									"\t\tgroup to another value.\r\n\r\n\r\n",strProvType,AdoFldString(rsBillProv, "First",""),AdoFldString(rsBillProv, "Last",""));
							}
							
							if(bIsUB92) {
								//TES 3/13/2007 - PLID 24993 - Changed from UB92 to UB
								str.Replace("HCFA", "UB");
								//TES 3/13/2007 - PLID 24993 - UB92 Box 82 = UB04 Box 76
								if(GetUBFormType() == eUB04) {
									str.Replace("Box 33 PIN", "Box 76 Number");
								}
								else {
									str.Replace("Box 33 PIN", "Box 82 Number");
								}
							}
							strErrors += str;
							bPassed = FALSE;
						}

						//now validate the qualifier (paper claims or ANSI claims)
						if(!bIsUB92 && Box33 != -1 && (!m_bElectronic || m_FormatStyle == ANSI) && strBox33bQual.IsEmpty()) {
							str.Format("\tERROR - No Provider ID Qualifier Found\r\n\r\n"
								"\t\tThe HCFA Group for this insurance company does not have a Box 33b\r\n"
								"\t\tinsurance ID qualifier set up properly. To fix this, go to the\r\n"
								"\t\tHCFA Setup tab in the Administrator module and enter in a 2-character\r\n"
								"\t\tqualifier to the left of the Box33b dropdown list.\r\n\r\n\r\n");
							strErrors += str;
							bPassed = FALSE;
						}
					}
				}

				if(m_FormatStyle == ANSI && m_bElectronic && GetRemotePropertyInt("EbillingValidateProviderTaxonomyCode",1,0,"<None>",TRUE)) {
					//Taxonomy Code
					CString strTaxonomy = AdoFldString(rsBillProv, "TaxonomyCode","");
					strTaxonomy.TrimRight();

					// (j.jones 2013-09-05 13:56) - PLID 58451 - we have overrides for taxonomy codes now,
					// but we can only suppress the warning if every taxonomy code is filled
					if(strTaxonomy.IsEmpty() && m_avANSIVersion == av5010) {
						if(!bIsUB92) {
							_RecordsetPtr rsTaxonomyOver1 = CreateParamRecordset("SELECT "
								"ANSI_2000A_Taxonomy, ANSI_2310B_Taxonomy, ANSI_2420A_Taxonomy "
								"FROM EbillingSetupT "
								"WHERE SetupGroupID = {INT} AND ProviderID = {INT} AND LocationID = {INT}",
								HCFASetupGroupID, nBillingProviderID, nLocationID);
							if(!rsTaxonomyOver1->eof) {
								CString str2000ATaxonomy = AdoFldString(rsTaxonomyOver1, "ANSI_2000A_Taxonomy", "");
								str2000ATaxonomy.TrimLeft();
								str2000ATaxonomy.TrimRight();

								//if we have a 2000A override, look for the rendering provider overrides next
								if(!str2000ATaxonomy.IsEmpty()) {
									_RecordsetPtr rsTaxonomyOver2 = CreateParamRecordset("SELECT Export2310BRecord, "
										"ANSI_2310B_Taxonomy, ANSI_2420A_Taxonomy "
										"FROM EbillingSetupT "
										"WHERE SetupGroupID = {INT} AND ProviderID = {INT} AND LocationID = {INT}",
										HCFASetupGroupID, nRenderingProviderID, nLocationID);
									if(!rsTaxonomyOver2->eof) {
										BOOL bExport2310BRecord = AdoFldBool(rsTaxonomyOver2, "Export2310BRecord", FALSE);
										CString str2310BTaxonomy = AdoFldString(rsTaxonomyOver2, "ANSI_2310B_Taxonomy", "");
										str2310BTaxonomy.TrimLeft();
										str2310BTaxonomy.TrimRight();
										CString str2420ATaxonomy = AdoFldString(rsTaxonomyOver2, "ANSI_2420A_Taxonomy", "");
										str2420ATaxonomy.TrimLeft();
										str2420ATaxonomy.TrimRight();

										//if we have a 2310B override, or if not a group and not sending 2310B,
										//then 2310B is ok,
										//and if we have a 2420A override, or if not sending 2420A,
										//then 2420A is ok
										if((!str2310BTaxonomy.IsEmpty() || (!bIsGroup && !bExport2310BRecord))
											&& (!str2420ATaxonomy.IsEmpty() || !bSend2420A)) {

											//either we're not sending more taxonomy codes,
											//or an override exists, either way we won't send a blank one

											//fill it with an arbritrary code to make it non-empty
											strTaxonomy = str2000ATaxonomy;
										}
									}
									rsTaxonomyOver2->Close();
								}
							}
							rsTaxonomyOver1->Close();
						}
						else {
							//for UB, the same provider is used in all loops
							_RecordsetPtr rsTaxonomyOver = CreateParamRecordset("SELECT "
								"ANSI_2000A_Taxonomy, ANSI_2310A_Taxonomy "
								"FROM UB92EbillingSetupT "
								"WHERE ANSI_2310A_Taxonomy <> '' AND SetupGroupID = {INT} AND ProviderID = {INT} AND LocationID = {INT}",
								UB92SetupGroupID, nBillingProviderID, nLocationID);
							if(!rsTaxonomyOver->eof) {
								CString str2000ATaxonomy = AdoFldString(rsTaxonomyOver, "ANSI_2000A_Taxonomy", "");
								str2000ATaxonomy.TrimLeft();
								str2000ATaxonomy.TrimRight();
								CString str2310ATaxonomy = AdoFldString(rsTaxonomyOver, "ANSI_2310A_Taxonomy", "");
								str2310ATaxonomy.TrimLeft();
								str2310ATaxonomy.TrimRight();

								//if we have a 2000A override, then 2000A is ok (always sent)
								//and if we have a 2310A override, or if 2310A is a ref. phy. (never has taxonomy)
								//then 2310A is ok
								if(!str2000ATaxonomy.IsEmpty()
									&& (!str2310ATaxonomy.IsEmpty() || nBox82Setup == 3)) {

									//an override exists for each code we are sending,
									//fill it with an arbritrary code to make it non-empty
									strTaxonomy = str2000ATaxonomy;
								}
							}
							rsTaxonomyOver->Close();
						}
					}

				

					if(strTaxonomy.IsEmpty()) {
						if((!bIsUB92 && HCFAProvType == 2) || (bIsUB92 && nBox82Setup == 2)) {
							//general 1 provider
							str.Format("\tERROR - No Provider ID Found\r\n\r\n"
								"\t\tThis Insurance Company's HCFA Group is set up to use the patient's\r\n"
								"\t\tdefault provider from General 1. The provider's claim provider,\r\n"
								"\t\t'%s %s', for this patient's account does not have a\r\n"
								"\t\tTaxonomy Code set up properly. To fix this, go to the contacts\r\n"
								"\t\tmodule for this provider and fill in a value for the Taxonomy Code.\r\n\r\n\r\n",AdoFldString(rsBillProv, "First",""),AdoFldString(rsBillProv, "Last",""));
						}
						else {
							//bill provider
							str.Format("\tERROR - No Provider Taxonomy Code Found\r\n\r\n"
								"\t\tThe provider's claim provider,'%s %s', on this patient's bill\r\n"
								"\t\tdoes not have a Taxonomy Code set up properly. To fix this, go to the contacts\r\n"
								"\t\tmodule for this provider and fill in a value for the Taxonomy Code.\r\n\r\n\r\n",AdoFldString(rsBillProv, "First",""),AdoFldString(rsBillProv, "Last",""));
						}						

						if(bIsUB92) {
							str.Replace("HCFA", "UB");
						}

						strErrors += str;
						bPassed = FALSE;
					}
				}
			}
			rsBillProv->Close();

			//now check the rendering provider information, HCFA only

			// (j.jones 2010-11-11 09:21) - PLID 41396 - only validate the IDs
			// if the rendering provider is different from the billing provider
			// (this is extremely rare)

			// (j.jones 2011-03-15 10:58) - PLID 42788 - this now follows EbillingValidateProviderOtherIDs and/or EbillingValidateProviderBox24JID
			BOOL bValidateProviderOtherIDs = GetRemotePropertyInt("EbillingValidateProviderOtherIDs",GetRemotePropertyInt("EbillingValidateProviderIDs",1,0,"<None>",FALSE),0,"<None>",TRUE) == 1;
			BOOL bValidateProviderBox24JID = GetRemotePropertyInt("EbillingValidateProviderBox24JID",GetRemotePropertyInt("EbillingValidateProviderIDs",1,0,"<None>",FALSE),0,"<None>",TRUE) == 1;
			BOOL bValidateProviderNPI = GetRemotePropertyInt("EbillingValidateProviderNPI",1,0,"<None>",TRUE) == 1;
			BOOL bValidateProviderNPIChecksum = GetRemotePropertyInt("EbillingValidateNPIChecksum",1,0,"<None>",TRUE) == 1;
			if((bValidateProviderOtherIDs || bValidateProviderBox24JID || bValidateProviderNPI || bValidateProviderNPIChecksum)
				&& nRenderingProviderID != nBillingProviderID
				&& !bIsUB92) {

				_RecordsetPtr rsRenderProv = CreateParamRecordset("SELECT NPI, Last, First, "
					"SocialSecurity, [Fed Employer ID] AS ProvEIN, TaxonomyCode "
					"FROM PersonT "
					"INNER JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID "
					"WHERE PersonT.ID = {INT}", nRenderingProviderID);
				if(!rsRenderProv->eof) {
					
					//check for the Box24J Qualifier (this should not be disabled by a preference)
					if(bValidateProviderBox24JID && (!m_bElectronic || m_FormatStyle == IMAGE)) {
						_RecordsetPtr rs24J = CreateParamRecordset("SELECT Box24JNumber FROM InsuranceBox24J "
							"WHERE Box24JNumber <> '' AND RTRIM(Box24IQualifier) = '' "
							"AND ProviderID = {INT} AND InsCoID = {INT}", nRenderingProviderID, InsuranceCoID);
						// (j.jones 2013-05-15 14:42) - PLID 56711 - check for NOT eof, because this query
						// is looking for the presence of bad data
						if(!rs24J->eof) {

							// (j.jones 2010-11-11 08:36) - PLID 41396 - output the error based off
							// their settings, if they are normally set to use the G1 provider, say so,
							// even though the claim providers can override that

							if(HCFAProvType == 2) {
								//general 1 provider
								str.Format("\tERROR - No Box 24J ID Qualifier Found\r\n\r\n"
									"\t\tThis Insurance Company's HCFA Group is set up to use the patient's\r\n"
									"\t\tdefault provider from General 1. The provider's assigned claim\r\n"
									"\t\tprovider, '%s %s', for this patient's account has a Box24J ID\r\n"
									"\t\tconfigured for the insurance company, but does not have an ID\r\n"
									"\t\tqualifier set up properly.\r\n"
									"\t\tTo fix this, go to the 'Edit Insurance List', available in the\r\n"
									"\t\tInsurance Tab of the Patients Module, or the HCFA Tab of the\r\n"
									"\t\tAdministrator Module, edit the Box24J setup for the selected\r\n"
									"\t\tinsurance company and provider, and enter in a 2-character\r\n"
									"\t\tqualifier to identify the type of ID.\r\n\r\n\r\n",
									AdoFldString(rsRenderProv, "First",""),AdoFldString(rsRenderProv, "Last",""));
							}
							else {
								//bill provider
								str.Format("\tERROR - No Box 24J ID Qualifier Found\r\n\r\n"
									"\t\tThe provider's claim provider,'%s %s',\r\n"
									"\t\ton this patient's bill, has a Box24J ID configured for the\r\n"
									"\t\tinsurance company, but does not have an ID qualifier set up properly.\r\n"
									"\t\tTo fix this, go to the 'Edit Insurance List', available in the\r\n"
									"\t\tInsurance Tab of the Patients Module, or the HCFA Tab of the\r\n"
									"\t\tAdministrator Module, edit the Box24J setup for the selected\r\n"
									"\t\tinsurance company and provider, and enter in a 2-character\r\n"
									"\t\tqualifier to identify the type of ID.\r\n\r\n\r\n",
									AdoFldString(rsRenderProv, "First",""),AdoFldString(rsRenderProv, "Last",""));
							}

							strErrors += str;
							bPassed = FALSE;
						}
						rs24J->Close();
					}

					if(bValidateProviderOtherIDs && m_bElectronic && m_FormatStyle == ANSI) {

						CString strProviderFirst = AdoFldString(rsRenderProv, "First","");
						CString strProviderLast = AdoFldString(rsRenderProv, "Last","");
						CString strProvSSN = AdoFldString(rsRenderProv, "SocialSecurity","");
						CString strProvEIN = AdoFldString(rsRenderProv, "ProvEIN","");

						//validate all possible ANSI REF IDs for this provider
						ValidateAllANSIProviderREFIDs(bIsUB92, FALSE, nRenderingProviderID, FALSE, TRUE,
							InsuranceCoID, nLocationID, nInsuredPartyID,
							HCFASetupGroupID, strDefaultBox33GRP, HCFAProvType,
							UB92SetupGroupID, strUB04Box76Qual, nBox82Num, nBox82Setup,
							strErrors, bPassed, bWarnings, bDisableREFXX,
							strProviderLast, strProviderFirst,
							bSend2010AB, bSend2420A,
							nExtraREF_IDType, bIsGroup, nHCFABox25,
							strLocationEIN, strProvEIN, strProvSSN,
							bUse_Addnl_2010AA, strAddnl_2010AA_Qual, strAddnl_2010AA);
					}

					CString strNPI = AdoFldString(rsRenderProv, "NPI","");
					strNPI.TrimRight();

					if(strNPI.IsEmpty()) {

						// (j.jones 2008-05-21 15:05) - PLID 29280 - checking the provider NPI is now independent of
						// the regular provider IDs, and the checksum validation is a separate option as well
						
						if(bValidateProviderNPI) {

							// (j.jones 2010-11-11 08:36) - PLID 41396 - output the error based off
							// their settings, if they are normally set to use the G1 provider, say so,
							// even though the claim providers can override that

							if(HCFAProvType == 2) {
								//general 1 provider
								str.Format("\tERROR - No Provider NPI Found\r\n\r\n"
									"\t\tThis Insurance Company's HCFA Group is set up to use the patient's\r\n"
									"\t\tdefault provider from General 1. The provider's assigned claim\r\n"
									"\t\tprovider, '%s %s', for this patient's account does not have a\r\n"
									"\t\tnumber set up properly.\r\n"
									"\t\tTo fix this, go to the contacts module for this provider and\r\n"
									"\t\tfill in the NPI field.\r\n\r\n\r\n",
									AdoFldString(rsRenderProv, "First",""),AdoFldString(rsRenderProv, "Last",""));
							}
							else {
								//bill provider
								str.Format("\tERROR - No Provider NPI Found\r\n\r\n"
									"\t\tThe provider's claim provider,'%s %s',\r\n"
									"\t\ton this patient's bill does not have a NPI number\r\n"
									"\t\tset up properly. To fix this, go to the contacts\r\n"
									"\t\tmodule for this provider and fill in the NPI field.\r\n\r\n\r\n",
									AdoFldString(rsRenderProv, "First",""),AdoFldString(rsRenderProv, "Last",""));
							}
							strErrors += str;
							bPassed = FALSE;
						}

					} else if (bValidateProviderNPIChecksum && !IsValidNPI(strNPI)) {

						// (j.jones 2010-11-11 08:36) - PLID 41396 - output the error based off
						// their settings, if they are normally set to use the G1 provider, say so,
						// even though the claim providers can override that

						if(HCFAProvType == 2) {
							//general 1 provider
							// (a.walling 2008-05-20 09:06) - PLID 27812 - Verify the NPI integrity if desired
							str.Format("\tERROR - Provider NPI is invalid\r\n\r\n"
								"\t\tThis Insurance Company's HCFA Group is set up to use the patient's\r\n"
								"\t\tdefault provider from General 1. The provider's assigned claim\r\n"
								"\t\tprovider, '%s %s', for this patient's account does not have a\r\n"
								"\t\tvalid NPI number (%s).\r\n"
								"\t\tTo fix this, go to the contacts module for this provider and\r\n"
								"\t\tcheck your records to confirm the accuracy of the NPI field.\r\n\r\n\r\n",
								AdoFldString(rsRenderProv, "First",""),AdoFldString(rsRenderProv, "Last",""), strNPI);
						}
						else {
							//bill provider
							// (a.walling 2008-05-20 09:06) - PLID 27812 - Verify the NPI integrity if desired
							str.Format("\tERROR - Provider NPI is Invalid\r\n\r\n"
								"\t\tThe provider's claim provider,'%s %s',\r\n"
								"\t\ton this patient's bill has an invalid NPI number (%s).\r\n"
								"\t\tTo fix this, go to the contacts module for this\r\n"
								"\t\tprovider and check your records to confirm the\r\n"
								"\t\taccuracy of the NPI field.\r\n\r\n\r\n",
								AdoFldString(rsRenderProv, "First",""),AdoFldString(rsRenderProv, "Last",""), strNPI);
						}
						strErrors += str;
						bPassed = FALSE;
					}
				}
				rsRenderProv->Close();
			}
		}

		//Check for a Referring Physician blank ID

		//only check if we are supposed to validate the ID

		// (j.jones 2008-05-21 15:17) - PLID 29280 - we now have separate preferences for validating NPI or Ref Phy ID

		BOOL bEbillingValidateRefPhyNPI = GetRemotePropertyInt("EbillingValidateRefPhyNPI",1,0,"<None>",TRUE) == 1;
		BOOL bEbillingValidateRefPhyID = GetRemotePropertyInt("EbillingValidateRefPhyID",1,0,"<None>",TRUE) == 1;

		if(!bIsUB92 && (bEbillingValidateRefPhyNPI || bEbillingValidateRefPhyID || nValidateRefPhy == 1 || m_avANSIVersion == av5010)) {

			long Box17a = 3;
			CString strBox17aQual = "";
			// (j.jones 2013-06-10 17:17) - PLID 56255 - supported GroupNPI
			long nUseRefPhyGroupNPI = 0;
			if(bEbillingValidateRefPhyID || m_avANSIVersion == av5010) {
				_RecordsetPtr rs = CreateParamRecordset("SELECT Box17a, Box17aQual, UseRefPhyGroupNPI FROM HCFASetupT WHERE ID = {INT}",HCFASetupGroupID);
				if(!rs->eof) {
					Box17a = AdoFldLong(rs, "Box17a",3);
					strBox17aQual = AdoFldString(rs, "Box17aQual","");
					strBox17aQual.TrimRight();
					nUseRefPhyGroupNPI = AdoFldLong(rs, "UseRefPhyGroupNPI", 0);
				}
				rs->Close();
			}

			long RefPhyID = -1;
			_RecordsetPtr rsRefPhyID = CreateParamRecordset("SELECT RefPhyID FROM BillsT INNER JOIN HCFATrackT ON BillsT.ID = HCFATrackT.BillID WHERE HCFATrackT.ID = {INT}",HCFAID);
			if(!rsRefPhyID->eof) {
				RefPhyID = AdoFldLong(rsRefPhyID, "RefPhyID",-1);
			}
			rsRefPhyID->Close();

			if(RefPhyID != -1) { //a ref phy exists

				_RecordsetPtr rsRefPhy;

				if(bEbillingValidateRefPhyNPI || bEbillingValidateRefPhyID || m_avANSIVersion == av5010) {

					// (j.jones 2013-06-10 17:17) - PLID 56255 - supported GroupNPI
					rsRefPhy = CreateParamRecordset("SELECT PersonT.Last, PersonT.First, "
						"ReferringPhysT.NPI, ReferringPhysT.GroupNPI "
						"FROM PersonT "
						"INNER JOIN ReferringPhysT ON PersonT.ID = ReferringPhysT.PersonID "
						"WHERE ID = {INT}",RefPhyID);

				}

				if(bEbillingValidateRefPhyNPI || bEbillingValidateRefPhyID) {
					if(!rsRefPhy->eof) {

						if(bEbillingValidateRefPhyNPI) {

							// (j.jones 2007-03-21 16:28) - PLID 25281 - this formerly only checked
							// the NPI when sending a paper HCFA, but should in fact check the NPI
							// at all times (this block is only called for HCFAs)

							CString strNPI = AdoFldString(rsRefPhy, "NPI","");

							// (j.jones 2013-06-10 17:17) - PLID 56255 - supported GroupNPI							
							bool bUsedGroupNPI = false;
							if(m_avANSIVersion == av5010 && !bIsUB92 && nUseRefPhyGroupNPI == 1) {
								CString strGroupNPI = AdoFldString(rsRefPhy, "GroupNPI","");
								strGroupNPI.TrimLeft(); strGroupNPI.TrimRight();
								if(!strGroupNPI.IsEmpty()) {
									strNPI = strGroupNPI;
									bUsedGroupNPI = true;
								}
							}

							//first validate the NPI number
							
							strNPI.TrimRight();
							if(strNPI.IsEmpty()) {
								str.Format("\tERROR - No Referring Physician NPI Found\r\n\r\n"
									"\t\tThe referring physician '%s %s' on this patient's bill does not have a\r\n"
									"\t\tNPI number set up properly. To fix this, go to the contacts module for\r\n"
									"\t\tthis referring physician and fill in the NPI field.\r\n\r\n\r\n",
									AdoFldString(rsRefPhy, "First",""),AdoFldString(rsRefPhy, "Last",""));
								strErrors += str;
								bPassed = FALSE;
							} else if (GetRemotePropertyInt("EbillingValidateNPIChecksum",1,0,"<None>",TRUE) && !IsValidNPI(strNPI)) {
								// (a.walling 2008-05-20 09:06) - PLID 27812 - Verify the NPI integrity if desired
								// (j.jones 2013-06-10 17:29) - PLID 56255 - supported GroupNPI
								if(bUsedGroupNPI) {
									str.Format("\tERROR - Referring Physician Group NPI is Invalid\r\n\r\n"
										"\t\tThe referring physician '%s %s' on this patient's bill does not have a\r\n"
										"\t\tvalid Group NPI number (%s). To fix this, go to the contacts module for\r\n"
										"\t\tthis referring physician and check your records to confirm the accuracy\r\n"
										"\t\tof the Group NPI field.\r\n\r\n\r\n",
										AdoFldString(rsRefPhy, "First",""),AdoFldString(rsRefPhy, "Last",""), strNPI);
								}
								else {
									str.Format("\tERROR - Referring Physician NPI is Invalid\r\n\r\n"
										"\t\tThe referring physician '%s %s' on this patient's bill does not have a\r\n"
										"\t\tvalid NPI number (%s). To fix this, go to the contacts module for\r\n"
										"\t\tthis referring physician and check your records to confirm the accuracy\r\n"
										"\t\tof the NPI field.\r\n\r\n\r\n",
										AdoFldString(rsRefPhy, "First",""),AdoFldString(rsRefPhy, "Last",""), strNPI);
								}
								strErrors += str;
								bPassed = FALSE;
							}
						}

						if(bEbillingValidateRefPhyID) {

							//only check the Box17a number if something is actually selected
							if(Box17a != -1) {

								BOOL bRefPhyFound = TRUE;
								// (a.walling 2008-05-20 10:24) - PLID 27812 - Validate the NPI if appropriate
								BOOL bValidNPI = TRUE;
								CString strIDName;
								bRefPhyFound = CheckBox17a(HCFASetupGroupID, Box17a, RefPhyID, strIDName, bValidNPI);

								if(!bRefPhyFound) {
									str.Format("\tERROR - No Referring Physician ID Found\r\n\r\n"
										"\t\tThe referring physician '%s %s' on this patient's bill does not have an\r\n"
										"\t\tinsurance ID set up properly. The current setting for the HCFA group\r\n"
										"\t\tis to use the referring physician's %s from the Contacts module.\r\n"
										"\t\tTo fix this, go to the contacts module for this referring physician and fill in\r\n"
										"\t\ta value, or change the Box 17a selection on this insurance company's HCFA\r\n"
										"\t\tgroup to another value.\r\n\r\n\r\n",AdoFldString(rsRefPhy, "First",""),AdoFldString(rsRefPhy, "Last",""),strIDName);
									strErrors += str;
									bPassed = FALSE;
								} else if (!bValidNPI) {
									// (a.walling 2008-05-20 10:37) - PLID 27812 - Check the NPI
									str.Format("\tERROR - Referring Physician NPI is Invalid\r\n\r\n"
										"\t\tThe referring physician '%s %s' on this patient's bill does not have a\r\n"
										"\t\tvalid NPI. To fix this, go to the contacts module for this referring physician\r\n"
										"\t\tand verify the accuracy of the NPI, or change the Box 17a selection on this\r\n"
										"\t\tinsurance company's HCFA group to another value.\r\n\r\n\r\n",AdoFldString(rsRefPhy, "First",""),AdoFldString(rsRefPhy, "Last",""));
									strErrors += str;
									bPassed = FALSE;
								}

								//now validate the qualifier (paper claims or ANSI claims)
								// (b.spivey February 19, 2015) - PLID 56651 - This isn't right, but because we fill in defaults it's not an error.
								if((!m_bElectronic || m_FormatStyle == ANSI) && strBox17aQual.IsEmpty()) {
									str.Format("\tWARNING - No Referring Physician ID Qualifier Found\r\n\r\n"
										"\t\tThe HCFA Group for this insurance company does not have a Box 17a\r\n"
										"\t\tinsurance ID qualifier set up properly. To fix this, go to the HCFA Setup\r\n"
										"\t\ttab in the Administrator module and enter in a 2-character qualifier to\r\n"
										"\t\tthe left of the Box17a dropdown list. Nextech Practice will fill this \r\n"
										"\t\tfield in with the value G2. \r\n\r\n\r\n");
									strErrors += str;
									bWarnings = TRUE;
								}
							}
						}
					}
				}

				// (j.jones 2010-10-20 15:30) - PLID 40936 - if 5010, validate that the REF qualifier is valid
				//referring physicians are not currently sent on UBs
				if(m_avANSIVersion == av5010 && !bIsUB92) {

					CString strQualifier = "", strID = "";
					EBilling_Box17aANSI(Box17a, strBox17aQual,strQualifier,strID,RefPhyID);

					strQualifier.TrimLeft();
					strQualifier.TrimRight();
					strID.TrimLeft();
					strID.TrimRight();

					//both can be blank, but you can't have one without the other
					if(strQualifier.IsEmpty() != strID.IsEmpty()) {
					
						CString str;
						str.Format("\tERROR - Referring Physician ID is incomplete\r\n\r\n"
							"\t\tThe referring physician '%s %s' on this patient's bill\r\n"
							"\t\tdoes not have a complete ID configured in Box 17 of the HCFA Setup.\r\n"
							"\t\tAll IDs sent in the ANSI format require both the qualifier and an ID\r\n"
							"\t\tor both blank to not send an REF ID.\r\n"
							"\t\tPlease review your setup and correct this issue before submitting.\r\n\r\n\r\n",
							AdoFldString(rsRefPhy, "First",""),AdoFldString(rsRefPhy, "Last",""));
						strErrors += str;
						bPassed = FALSE;
					}	
					else if(strQualifier.CompareNoCase("XX") == 0) {
						//if REF*XX is disabled, make this a warning, otherwise it is an error
						if(bDisableREFXX) {
							//they won't actually be sending the REF*XX if they export
							CString str;
							str.Format("\tWARNING - Referring Physician ID cannot use XX qualifier\r\n\r\n"
								"\t\tThe referring physician '%s %s' on this patient's bill\r\n"
								"\t\thas an invalid ID qualifier of XX configured in Box 17 of the HCFA Setup.\r\n"
								"\t\tSending an XX in the REF segment is no longer a valid qualifier.\r\n"
								"\t\tYour preferences are configured to not send this invalid REF*XX segment,\r\n"
								"\t\tbut you should review your setup and correct this issue before submitting.\r\n\r\n\r\n",
								AdoFldString(rsRefPhy, "First",""),AdoFldString(rsRefPhy, "Last",""));
							strErrors += str;
							bWarnings = TRUE;
						}
						else {
							//they will be sending the REF*XX if they export
							CString str;
							str.Format("\tERROR - Referring Physician ID cannot use XX qualifier\r\n\r\n"
								"\t\tThe referring physician '%s %s' on this patient's bill\r\n"
								"\t\thas an invalid ID qualifier of XX configured in Box 17 of the HCFA Setup.\r\n"
								"\t\tSending an XX in the REF segment is no longer a valid qualifier.\r\n"
								"\t\tPlease review your setup and correct this issue before submitting.\r\n\r\n\r\n",
								AdoFldString(rsRefPhy, "First",""),AdoFldString(rsRefPhy, "Last",""));
							strErrors += str;
							bPassed = FALSE;
						}
					}
					//validate against a list of permissible qualifiers aside from XX
					else if(!strQualifier.IsEmpty()) {

						//the only valid qualifiers in 5010 are:
						//0B, 1G, G2, LU
						if(strQualifier.CompareNoCase("0B") != 0
							&& strQualifier.CompareNoCase("1G") != 0
							&& strQualifier.CompareNoCase("G2") != 0
							&& strQualifier.CompareNoCase("LU") != 0) {

							CString str;
							str.Format("\tERROR - Referring Physician ID cannot use a %s qualifier\r\n\r\n"
								"\t\tThe referring physician '%s %s' on this patient's bill\r\n"
								"\t\thas an invalid ID qualifier of %s configured in Box 17 of the HCFA Setup.\r\n"
								"\t\tSending an %s in the REF segment is no longer a valid qualifier.\r\n"
								"\t\tPlease review your setup and correct this issue before submitting.\r\n\r\n\r\n",
								strQualifier,
								AdoFldString(rsRefPhy, "First",""),AdoFldString(rsRefPhy, "Last",""), strQualifier,
								strQualifier);
							strErrors += str;
							bPassed = FALSE;
						}
					}

				}
			}
			// (j.jones 2008-06-10 15:09) - PLID 25834 - warn if the setting says so, if no ref. phy exists
			else if(nValidateRefPhy == 1) {
				str.Format("\tERROR - No Referring Physician Found\r\n\r\n"
					"\t\tThis bill does not have a referring physician assigned to it.\r\n"
					"\t\tTo fix this, edit the bill and select a referring physician on the insurance tab.\r\n\r\n\r\n");
				strErrors += str;
				bPassed = FALSE;
			}
		}

		//check for the Facility ID Qualifier (this should not be disabled by a preference)
		// (j.jones 2013-04-25 11:48) - PLID 55564 - skip this if we skip POS validation, or if we're sending patient address
		if(!bSkipPOSValidation && !bSendPatientAddressAsPOS && !bIsUB92 && (!m_bElectronic || m_FormatStyle == ANSI)) { //(paper claims or ANSI claims)
			if(ReturnsRecords("SELECT * FROM InsuranceFacilityID WHERE FacilityID <> '' AND RTRIM(Qualifier) = '' "
				"AND LocationID = %li AND InsCoID = %li", nPOSID, InsuranceCoID)) {

				str.Format("\tERROR - No Facility ID Qualifier Found\r\n\r\n"
					"\t\tThe insurance company on this bill has a Facility ID configured for the\r\n"
					"\t\tPlace Of Service, but does not have an ID qualifier set up properly.\r\n"
					"\t\tTo fix this, go to the 'Edit Insurance List', available in the\r\n"
					"\t\tInsurance Tab of the Patients Module, or the HCFA Tab of the\r\n"
					"\t\tAdministrator Module, edit the Facility ID for the selected\r\n"
					"\t\tinsurance company and place of service, and enter in a 2-character\r\n"
					"\t\tqualifier to identify the type of ID.\r\n\r\n\r\n");
				strErrors += str;
				bPassed = FALSE;
			}
		}

		// (j.jones 2007-03-21 16:28) - PLID 25281 - this formerly only checked
		// the NPI when sending a paper HCFA, but should in fact check the NPI
		// for electronic HCFAs as well. Just not UB92.

		//check for the Facility ID NPI number

		// (j.jones 2007-03-27 11:33) - PLID 25364 - added option to disable this check,
		// as the POS NPI is not always required
		// (j.jones 2013-04-25 11:48) - PLID 55564 - skip this if we skip POS validation, or if we're sending patient address
		if(!bSkipPOSValidation && !bSendPatientAddressAsPOS && !bIsUB92 && GetRemotePropertyInt("EbillingValidatePOSNPI",1,0,"<None>",TRUE)) {
			// (a.walling 2008-05-20 09:17) - PLID 27812 - Verify the NPI integrity if desired
			// (b.eyers 2015-04-16) - PLID 41027 - Parameterize 
			_RecordsetPtr prsNPI = CreateParamRecordset("SELECT RTRIM(NPI) AS NPI FROM LocationsT WHERE ID = {INT}", nPOSID);
			if(prsNPI->eof) {
				// (j.jones 2009-03-02 11:05) - PLID 33286 - we would have already warned that no POS exists,
				// so do not throw an exception
				ASSERT(FALSE);
				//ThrowNxException("Could not access info for POS location %li", nPOSID);
			} else {				
				CString strPOSNPI = AdoFldString(prsNPI, "NPI", "");

				if (strPOSNPI.IsEmpty()) {
					str.Format("\tERROR - No Facility ID NPI Found\r\n\r\n"
						"\t\tThe Place Of Service on this bill does not have an NPI number entered.\r\n"
						"\t\tTo fix this, go to the Locations Tab in the Administrator Module and\r\n"
						"\t\tenter in the NPI number for this location.\r\n\r\n\r\n");
					strErrors += str;
					bPassed = FALSE;
				} else if (GetRemotePropertyInt("EbillingValidateNPIChecksum",1,0,"<None>",TRUE) && !IsValidNPI(strPOSNPI)) {
					// (a.walling 2008-05-20 09:17) - PLID 27812 - Verify the NPI integrity if desired
					str.Format("\tERROR - Facility ID NPI is Invalid\r\n\r\n"
						"\t\tThe Place Of Service on this bill does not have a valid NPI number (%s).\r\n"
						"\t\tTo fix this, go to the Locations Tab in the Administrator Module and\r\n"
						"\t\tcheck your records to confirm the accuracy of the NPI for this location.\r\n\r\n\r\n", strPOSNPI);
					strErrors += str;
					bPassed = FALSE;
				}
			}
		}

		// (j.jones 2007-03-21 16:40) - PLID 25281 - they would be using a Location NPI
		// if an ANSI UB form, if any ANSI file sent as a Group, or any HCFA sent with LocationNPIUsage = 1

		if((m_FormatStyle == ANSI && (bIsUB92 || bIsGroup))
			|| (!bIsUB92 && LocationNPIUsage == 1)) {

			//check for the Bill Location NPI number
			// (b.eyers 2015-04-16) - PLID 41027 - Parameterize 
			_RecordsetPtr prsNPI = CreateParamRecordset("SELECT RTRIM(NPI) AS NPI FROM LocationsT WHERE ID = {INT}", nLocationID);
			if(prsNPI->eof) {
				// (j.jones 2009-03-02 11:05) - PLID 33286 - we would have already warned that no POS exists,
				// so do not throw an exception
				ASSERT(FALSE);
				//ThrowNxException("Could not access info for bill location %li", nPOSID);
			} else {
				CString strBillNPI = AdoFldString(prsNPI, "NPI", "");

				if (strBillNPI.IsEmpty()) {
					str.Format("\tERROR - No Bill Location NPI Found\r\n\r\n"
						"\t\tThe Location on this bill does not have an NPI number entered.\r\n"
						"\t\tTo fix this, go to the Locations Tab in the Administrator Module and\r\n"
						"\t\tenter in the NPI number for this location.\r\n\r\n\r\n");
					strErrors += str;
					bPassed = FALSE;
				} else if (GetRemotePropertyInt("EbillingValidateNPIChecksum",1,0,"<None>",TRUE) && !IsValidNPI(strBillNPI)) {
					// (a.walling 2008-05-20 09:17) - PLID 27812 - Verify the NPI integrity if desired
					str.Format("\tERROR - Bill Location NPI is Invalid\r\n\r\n"
						"\t\tThe Location on this bill does not have a valid NPI number (%s).\r\n"
						"\t\tTo fix this, go to the Locations Tab in the Administrator Module and\r\n"
						"\t\tcheck your records to confirm the accuracy of the NPI for this location.\r\n\r\n\r\n", strBillNPI);
					strErrors += str;
					bPassed = FALSE;
				}
			}
		}

		// (j.jones 2007-04-25 09:25) - PLID 25277 - Validate the UB04 qualifiers for boxes
		// 76, 77, and 78, but only if "Do Not Fill" is not selected. Don't bother with the overhead
		// to check and see if each ID field will be filled in per claim, because if they have
		// an ID setup and no qualifier, it's a general setup failure that needs corrected immediately.
		if(bIsUB92 && GetUBFormType() == eUB04) {

			//Box 76
			if(strUB04Box76Qual.IsEmpty() && nBox82Setup != -1 && nBox82Num != -1) {
				str.Format("\tERROR - No Box 76 Provider ID Qualifier Found\r\n\r\n"
					"\t\tThe UB Group for this insurance company '%s' does not have a\r\n"
					"\t\tBox 76 insurance ID qualifier set up properly. To fix this, go to the\r\n"
					"\t\tUB Setup tab in the Administrator module and enter in a 2-character\r\n"
					"\t\tqualifier to the left of the Box 76 ID dropdown list.\r\n\r\n\r\n", strInsCoName);
				strErrors += str;
				bPassed = FALSE;
			}

			//Box 77
			if(strUB04Box77Qual.IsEmpty() && nUB04Box77Setup != -1 && nUB04Box77Num != -1) {
				str.Format("\tERROR - No Box 77 Provider ID Qualifier Found\r\n\r\n"
					"\t\tThe UB Group for this insurance company '%s' does not have a\r\n"
					"\t\tBox 77 insurance ID qualifier set up properly. To fix this, go to the\r\n"
					"\t\tUB Setup tab in the Administrator module and enter in a 2-character\r\n"
					"\t\tqualifier to the left of the Box 77 ID dropdown list.\r\n\r\n\r\n", strInsCoName);
				strErrors += str;
				bPassed = FALSE;
			}

			//Box 78
			if(strUB04Box78Qual.IsEmpty() && nBox83Setup != -1 && nBox83Num != -1) {
				str.Format("\tERROR - No Box 78 Provider ID Qualifier Found\r\n\r\n"
					"\t\tThe UB Group for this insurance company '%s' does not have a\r\n"
					"\t\tBox 78 insurance ID qualifier set up properly. To fix this, go to the\r\n"
					"\t\tUB Setup tab in the Administrator module and enter in a 2-character\r\n"
					"\t\tqualifier to the left of the Box 78 ID dropdown list.\r\n\r\n\r\n", strInsCoName);
				strErrors += str;
				bPassed = FALSE;
			}
		}

		// (j.jones 2011-08-23 11:59) - PLID 45141 - supported ANSI validation of UB loops 2310B & C, using Box 77/78 data
		if(bIsUB92 && m_bElectronic && m_FormatStyle == ANSI) {
			//2310B (UB04 only)
			if(nUB_2310B_ProviderID != -1 && GetUBFormType()) {
				CString strID = "", strQualifier = "", strLoadedFrom = "Box 77 settings";

				_RecordsetPtr rs = CreateParamRecordset("SELECT First, Last FROM PersonT WHERE ID = {INT}", nUB_2310B_ProviderID);
				if(!rs->eof) {
					CString strProviderFirst = AdoFldString(rs, "First","");
					CString strProviderLast = AdoFldString(rs, "Last","");
					
					//the 2310A override is used for 2310B as well, if an override is set
					EBilling_Calculate2310A_REF(strQualifier, strID, strLoadedFrom,
						nUB_2310B_ProviderID, UB92SetupGroupID, strUB04Box77Qual, nUB04Box77Num, nUB04Box77Setup, 
						InsuranceCoID, nInsuredPartyID, nLocationID);

					CheckANSIProviderREFID(strQualifier, strID, strLoadedFrom, "2310B",
						TRUE, strErrors, bPassed, bWarnings, bDisableREFXX,
						strProviderLast, strProviderFirst, FALSE);
				}
				rs->Close();
			}

			//2310C
			if(nUB_2310C_ProviderID != -1) {
				CString strID = "", strQualifier = "", strLoadedFrom = "Box 78 settings";
				if(GetUBFormType() != eUB04) {
					strLoadedFrom = "Box 83 settings";
				}
				
				_RecordsetPtr rs = CreateParamRecordset("SELECT First, Last FROM PersonT WHERE ID = {INT}", nUB_2310C_ProviderID);
				if(!rs->eof) {
					CString strProviderFirst = AdoFldString(rs, "First","");
					CString strProviderLast = AdoFldString(rs, "Last","");
					
					//the 2310A override is used for 2310C as well, if an override is set
					EBilling_Calculate2310A_REF(strQualifier, strID, strLoadedFrom,
						nUB_2310C_ProviderID, UB92SetupGroupID, strUB04Box78Qual, nBox83Num, nBox83Setup, 
						InsuranceCoID, nInsuredPartyID, nLocationID);

					CheckANSIProviderREFID(strQualifier, strID, strLoadedFrom, "2310C",
						TRUE, strErrors, bPassed, bWarnings, bDisableREFXX,
						strProviderLast, strProviderFirst, FALSE);
				}
				rs->Close();
			}
		}
	
		if(bPassed && !bWarnings)
			strErrors = "";

		m_strInvalidClaims += strErrors;

		int Val;
		if(bPassed && !bWarnings)
			//the claim has no errors, and no warnings
			Val = 1;
		else if(bPassed && bWarnings)
			//the claim had no critical errors, but there are warnings
			Val = 3;
		else if(!bPassed)
			//the claim failed with definite errors
			Val = 2;
		else
			//this should not be possible
			Val = 0;

		ExecuteParamSql("UPDATE HCFATrackT SET ValidationState = {INT} WHERE ID = {INT}", Val, HCFAID); // (b.eyers 2015-04-16) - PLID 41027 - Parameterize 

		return (bPassed && !bWarnings);

	}NxCatchAll("Error validating claim.");

	ExecuteParamSql("UPDATE HCFATrackT SET ValidationState = 0 WHERE ID = {INT}", HCFAID); // (b.eyers 2015-04-16) - PLID 41027 - Parameterize 

	return FALSE;
}

BOOL CEbillingValidationDlg::ValidateAll(int Type)
{
	try {

		SetDlgItemText(IDC_CURRENT_CLAIM_LABEL,"Validating Claims");

		BOOL bNoErrors = TRUE;
		CString strLabel;
		long i = 1;
		long count = 0;
		float increment = 0.0;

		// (b.eyers 2015-04-16) - PLID 41027
		CSqlFragment sqlSQLCount, sqlSQLData;
		//CString strSQLCount, strSQLData;

		CString strLocationFilter = "";
		if(m_LocationID != -1) {
			strLocationFilter.Format(" AND LineItemT.LocationID = %li",m_LocationID);
		}

		CString strProviderFilter = "";
		if(m_nProviderID != -1) {
			//filter on any bill that contains the selected provider, even if they may
			//also contain other providers
			strProviderFilter.Format(" AND ChargesT.DoctorsProviders = %li", m_nProviderID);
		}

		// (j.jones 2009-03-02 11:16) - PLID 33286 - ensure that we only include bills that have at least one batched, undeleted charge
		// (j.jones 2011-08-16 17:22) - PLID 44805 - we will filter out "original" and "void" charges
		// (j.jones 2011-09-19 16:00) - PLID 44805 - when rewriting this filter, I had lost the FormType filter - now it's back!
		// (j.jones 2014-07-09 09:28) - PLID 62568 - bills with an "on hold" status are hidden here
		// (b.eyers 2015-04-16) - PLID 41027 - Parameterize 
		CSqlFragment sqlWhere;
		sqlWhere.Create(" AND BillID IN (SELECT BillID FROM ChargesT "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
			"LEFT JOIN BillStatusT ON BillsT.StatusID = BillStatusT.ID "
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
			"WHERE LineItemT.Deleted = 0 AND ChargesT.Batched = 1 "
			"AND BillsT.FormType = {INT} "
			"AND Coalesce(BillStatusT.Type, -1) != {INT} "
			"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
			"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
			"{CONST_STRING} {CONST_STRING})", m_ClaimType, EBillStatusType::OnHold, strLocationFilter, strProviderFilter);
		//CString strWhere = "";
		//strWhere.Format(" AND BillID IN (SELECT BillID FROM ChargesT "
		//	"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
		//	"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
		//	"LEFT JOIN BillStatusT ON BillsT.StatusID = BillStatusT.ID "
		//	"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
		//	"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
		//	"WHERE LineItemT.Deleted = 0 AND ChargesT.Batched = 1 "
		//	"AND BillsT.FormType = %li "
		//	"AND Coalesce(BillStatusT.Type, -1) != %li "
		//	"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
		//	"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
		//	"%s %s)", m_ClaimType, EBillStatusType::OnHold, strLocationFilter, strProviderFilter);

		if(Type == 1) {
			//all unselected claims
			/*strSQLCount.Format("SELECT Count(ID) AS CountOfID FROM HCFATrackT WHERE HCFATrackT.Batch = %li AND HCFATrackT.CurrentSelect = 0 %s",m_bElectronic ? 2 : 1, strWhere);
			strSQLData.Format("SELECT ID FROM HCFATrackT WHERE HCFATrackT.Batch = %li AND HCFATrackT.CurrentSelect = 0 %s",m_bElectronic ? 2 : 1, strWhere);*/
			sqlSQLCount.Create("SELECT Count(ID) AS CountOfID FROM HCFATrackT WHERE HCFATrackT.Batch = {INT} AND HCFATrackT.CurrentSelect = 0 {SQL}", m_bElectronic ? 2 : 1, sqlWhere);
			sqlSQLData.Create("SELECT ID FROM HCFATrackT WHERE HCFATrackT.Batch = {INT} AND HCFATrackT.CurrentSelect = 0 {SQL}", m_bElectronic ? 2 : 1, sqlWhere);
		}
		else if(Type == 2) {
			//all selected claims
			/*strSQLCount.Format("SELECT Count(ID) AS CountOfID FROM HCFATrackT WHERE HCFATrackT.Batch = %li AND HCFATrackT.CurrentSelect = 1 %s",m_bElectronic ? 2 : 1, strWhere);
			strSQLData.Format("SELECT ID FROM HCFATrackT WHERE HCFATrackT.Batch = %li AND HCFATrackT.CurrentSelect = 1 %s",m_bElectronic ? 2 : 1, strWhere);*/
			sqlSQLCount.Create("SELECT Count(ID) AS CountOfID FROM HCFATrackT WHERE HCFATrackT.Batch = {INT} AND HCFATrackT.CurrentSelect = 1 {SQL}", m_bElectronic ? 2 : 1, sqlWhere);
			sqlSQLData.Create("SELECT ID FROM HCFATrackT WHERE HCFATrackT.Batch = {INT} AND HCFATrackT.CurrentSelect = 1 {SQL}", m_bElectronic ? 2 : 1, sqlWhere);
		}
		else {
			//all claims
			/*strSQLCount.Format("SELECT Count(ID) AS CountOfID FROM HCFATrackT WHERE HCFATrackT.Batch = %li %s",m_bElectronic ? 2 : 1, strWhere);
			strSQLData.Format("SELECT ID FROM HCFATrackT WHERE HCFATrackT.Batch = %li %s",m_bElectronic ? 2 : 1, strWhere);*/
			sqlSQLCount.Create("SELECT Count(ID) AS CountOfID FROM HCFATrackT WHERE HCFATrackT.Batch = {INT} {SQL}", m_bElectronic ? 2 : 1, sqlWhere);
			sqlSQLData.Create("SELECT ID FROM HCFATrackT WHERE HCFATrackT.Batch = {INT} {SQL}", m_bElectronic ? 2 : 1, sqlWhere);
		}

		m_progress.SetValue(0);

		_RecordsetPtr rs = CreateParamRecordset(sqlSQLCount);

		if(!rs->eof) {
			count = AdoFldLong(rs, "CountOfID",0);
			if(count > 0)
				increment = (float)100.0 / (float)count;
		}
		rs->Close();

		rs = CreateParamRecordset(sqlSQLData);

		while(!rs->eof) {
			strLabel.Format("Validating Claim %li of %li",i,count);
			SetDlgItemText(IDC_CURRENT_CLAIM_LABEL,strLabel);

			if(!ValidateOne(rs->Fields->Item["ID"]->Value.lVal))
				bNoErrors = FALSE;
			
			rs->MoveNext();
			i++;

			if(m_progress.GetValue()+increment > 100.0)
				m_progress.SetValue(100.0);
			else
				m_progress.SetValue(m_progress.GetValue() + increment);
		}
		rs->Close();

		//if any claim had any errors, we will return false
		return bNoErrors;

	} NxCatchAll("Error validating claims.");

	return FALSE;
}

void CEbillingValidationDlg::ShowErrors() {

	try {
		CString pathname,OutputString;
		CFile	OutputFile;

		//for now, let's NOT prompt them to save the file, that way it will auto-open

		/*
		CFileDialog SaveAs(FALSE,NULL,"ebillingerrors.txt");
		SaveAs.m_ofn.lpstrInitialDir = GetPracPath(true);
		SaveAs.m_ofn.lpstrFilter = "Text Files\0*.txt\0All Files\0*.*\0\0";
		if (SaveAs.DoModal() == IDCANCEL) return;
		pathname = SaveAs.GetPathName();
		*/

		// (j.armen 2011-10-25 13:01) - PLID 46134 - Ebilling is using the practice path
		// (v.maida 2016-05-19 17:01) - NX-100684 - Updated to used the shared path for Azure.
		if(m_bElectronic) {
			pathname = GetEnvironmentDirectory() ^ "Ebilling\\ebillingerrors.txt";
		}
		else {
			pathname = GetEnvironmentDirectory() ^ "\\paperclaimerrors.txt";
		}

		if(!OutputFile.Open(pathname,CFile::modeCreate|CFile::modeWrite|CFile::shareExclusive)) {
			// (v.maida 2016-05-19 17:01) - NX-100684 - Updated to used the shared path for Azure. Also, changed to FileUtils::CreatePath() so that 
			// non-existent intermediate paths are created.
			if(m_bElectronic) {
				FileUtils::CreatePath(GetEnvironmentDirectory() ^ "Ebilling\\");
			}
			else {
				FileUtils::CreatePath(GetEnvironmentDirectory());
			}

			if(!OutputFile.Open(pathname,CFile::modeCreate|CFile::modeWrite|CFile::shareExclusive)) {
				AfxMessageBox("The claim error file could not be created. Contact Nextech for assistance.");
			}
		};

		CString strDate, strUserName;
		COleDateTime dt;
		dt = COleDateTime::GetCurrentTime();
		strDate = FormatDateTimeForInterface(dt, DTF_STRIP_SECONDS, dtoDateTime);
		strUserName = GetCurrentUserName();

		OutputString.Format("Electronic Batch Claim Errors\r\n\r\nUser: %s\tDate/Time: %s \r\n\r\n",strUserName,strDate);

		if(!m_bElectronic) {
			OutputString.Replace("Electronic","Paper");
		}

		OutputFile.Write(OutputString,OutputString.GetLength());

		OutputString = "The following items are either invalid or have warnings.\r\n\r\n"
			"'ERROR' means the claim will definitely be rejected, and this claim must be corrected.\r\n"
			"'WARNING' means the claim may be rejected, but it is possible that it may pass for some companies.\r\n\r\n"
			"Keep in mind that when a claim passes this level of error checking, it may still be rejected for\r\n"
			"other reasons not on this report.";
		
		if(m_bElectronic) {
			// (a.walling 2008-05-20 10:41) - PLID 27812 - The 'report.This' part always bothered me; I'm adding a space.
			OutputString += " This is a tool to help you diagnose problems with your batches\r\n"
				"and should not be a replacement for confirmation reports provided to you by your electronic\r\n"
				"claims provider.";
		}

		OutputString += "\r\n\r\n\r\n";
				
		OutputFile.Write(OutputString,OutputString.GetLength());

		OutputString = m_strInvalidClaims;

		OutputFile.Write(OutputString,OutputString.GetLength());

		OutputFile.Close();

		// (a.walling 2009-08-10 16:17) - PLID 35153 - Use NULL instead of "open" when calling ShellExecute(Ex). Usually equivalent; see PL notes.
		int nResult = (int)ShellExecute ((HWND)this, NULL, "notepad.exe", ("'" + pathname + "'"), NULL, SW_SHOW);

	} NxCatchAll("Error in writing 'HCFA Errors' file.");
}

void CEbillingValidationDlg::OnTimer(UINT nIDEvent) 
{
	if(nIDEvent == 0) {

		KillTimer(nIDEvent);

		m_progress.SetValue(0);
		m_progress.SetMax(100);
		
		if(m_HCFAID == -1) {
			if(ValidateAll(1)) {
				m_progress.SetValue(100);
				AfxMessageBox("All unselected claims have passed NexTech claim validation!\n\n"
					"However, this does not necessarily mean they will pass claim validation by "
					"your insurance company. You must still check your reports when sending claims.");
				CNxDialog::OnOK();
				return;
			}
		}
		else if(m_HCFAID == -2) {
			if(ValidateAll(2)) {
				m_progress.SetValue(100);
				AfxMessageBox("All selected claims have passed NexTech claim validation!\n\n"
					"However, this does not necessarily mean they will pass claim validation by "
					"your insurance company. You must still check your reports when sending claims.");
				CNxDialog::OnOK();
				return;
			}
		}
		else if(m_HCFAID == -3) {
			if(ValidateAll(3)) {
				m_progress.SetValue(100);
				AfxMessageBox("All claims have passed NexTech claim validation!\n\n"
					"However, this does not necessarily mean they will pass claim validation by "
					"your insurance company. You must still check your reports when sending claims.");
				CNxDialog::OnOK();
				return;
			}
		}
		else {
			SetDlgItemText(IDC_CURRENT_CLAIM_LABEL,"Validating Claim");
			if(ValidateOne(m_HCFAID)) {
				m_progress.SetValue(100);
				AfxMessageBox("This claim has passed NexTech claim validation!\n\n"
					"However, this does not necessarily mean it will pass claim validation by "
					"your insurance company. You must still check your reports when sending claims.");
				CNxDialog::OnOK();
				return;
			}
		}
		
		m_progress.SetValue(100);

		//output the error file
		ShowErrors();

		// (j.jones 2007-03-01 09:06) - PLID 25015 - tracked a boolean to determine if any errors/warnings were shown
		m_bErrorListDisplayed = TRUE;

		CNxDialog::OnOK();

	}
	
	CNxDialog::OnTimer(nIDEvent);
}

// (a.walling 2008-05-20 10:12) - PLID 27812 - Added out parameter for valid NPI
BOOL CEbillingValidationDlg::CheckBox33Pin(long HCFASetupGroupID, long Box33, long ProviderID, long LocationID, long InsuranceCoID, CString &strIDName, BOOL &bValidNPI)
{
	try {

		//PIN

		_RecordsetPtr rsTemp;
		_variant_t var;

		CString str;
		bValidNPI = TRUE;

		// (b.eyers 2015-04-16) - PLID 41027 - Parameterize 
		rsTemp = CreateParamRecordset("SELECT PIN FROM AdvHCFAPinT WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT}",ProviderID,LocationID,HCFASetupGroupID);
		if(!rsTemp->eof) {
			var = rsTemp->Fields->Item["PIN"]->Value;
			if(var.vt == VT_BSTR) {				
				strIDName = "AdvHCFAPin";
				CString strValueToCheck = AdoFldString(rsTemp, "PIN","");
				strValueToCheck.TrimLeft();
				strValueToCheck.TrimRight();
				return (strValueToCheck != "");
			}
		}
		rsTemp->Close();

		// (b.eyers 2015-04-16) - PLID 41027 - Parameterize 
		_RecordsetPtr rsDoc = CreateParamRecordset("SELECT * FROM PersonT INNER JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID WHERE ID = {INT}",ProviderID);

		if(!rsDoc->eof) {

			switch(Box33) {
			case 1: { //NPI
				//ProvidersT.NPI
				strIDName = "NPI";
				CString strValueToCheck = AdoFldString(rsDoc, "NPI","");
				strValueToCheck.TrimLeft();
				strValueToCheck.TrimRight();
				// (a.walling 2008-05-20 09:23) - PLID 27812 - Verify the NPI integrity if desired
				bValidNPI = !GetRemotePropertyInt("EbillingValidateNPIChecksum",1,0,"<None>",TRUE) || (GetRemotePropertyInt("EbillingValidateNPIChecksum",1,0,"<None>",TRUE) && IsValidNPI(strValueToCheck));
				return (strValueToCheck != "");
				break;
			}
			case 2: { //Social Security Number
				//PersonT.SocialSecurity
				strIDName = "Social Security Number";
				CString strValueToCheck = AdoFldString(rsDoc, "SocialSecurity","");
				strValueToCheck.TrimLeft();
				strValueToCheck.TrimRight();
				return (strValueToCheck != "");
				break;
			}
			case 3: { //Federal ID Number
				//ProvidersT.[Fed Employer ID]
				strIDName = "Federal Employer ID";
				CString strValueToCheck = AdoFldString(rsDoc, "Fed Employer ID","");
				strValueToCheck.TrimLeft();
				strValueToCheck.TrimRight();
				return (strValueToCheck != "");
				break;
			}
			case 4: { //DEA Number
				//ProvidersT.[DEA Number]
				strIDName = "DEA Number";
				CString strValueToCheck = AdoFldString(rsDoc, "DEA Number","");
				strValueToCheck.TrimLeft();
				strValueToCheck.TrimRight();
				return (strValueToCheck != "");
				break;
			}
			case 5: { //BCBS Number
				//ProvidersT.[BCBS Number]
				strIDName = "BCBS Number";
				CString strValueToCheck = AdoFldString(rsDoc, "BCBS Number","");
				strValueToCheck.TrimLeft();
				strValueToCheck.TrimRight();
				return (strValueToCheck != "");
				break;
			}
			case 6: { //Medicare Number
				//ProvidersT.[Medicare Number]
				strIDName = "Medicare Number";
				CString strValueToCheck = AdoFldString(rsDoc, "Medicare Number","");
				strValueToCheck.TrimLeft();
				strValueToCheck.TrimRight();
				return (strValueToCheck != "");
				break;
			}
			case 7: { //Medicaid Number
				//ProvidersT.[Medicaid Number]
				strIDName = "Medicaid Number";
				CString strValueToCheck = AdoFldString(rsDoc, "Medicaid Number","");
				strValueToCheck.TrimLeft();
				strValueToCheck.TrimRight();
				return (strValueToCheck != "");
				break;
			}
			case 8: { //Workers Comp Number
				//ProvidersT.[Workers Comp Number]
				strIDName = "Workers Comp Number";
				CString strValueToCheck = AdoFldString(rsDoc, "Workers Comp Number","");
				strValueToCheck.TrimLeft();
				strValueToCheck.TrimRight();
				return (strValueToCheck != "");
				break;
			}
			case 9: { //Other ID
				//ProvidersT.[Other ID Number]
				strIDName = "Other ID Number";
				CString strValueToCheck = AdoFldString(rsDoc, "Other ID Number","");
				strValueToCheck.TrimLeft();
				strValueToCheck.TrimRight();
				return (strValueToCheck != "");
				break;
			}
			case 11: { //UPIN
				//ProvidersT.UPIN
				strIDName = "UPIN";
				CString strValueToCheck = AdoFldString(rsDoc, "UPIN","");
				strValueToCheck.TrimLeft();
				strValueToCheck.TrimRight();
				return (strValueToCheck != "");
				break;
			}
			case 12: { //Box24J
				//InsuranceBox24J.Box24JNumber
				str = "";
				// (b.eyers 2015-04-16) - PLID 41027 - Parameterize 
				_RecordsetPtr rsBox24J = CreateParamRecordset("SELECT Box24JNumber FROM InsuranceBox24J WHERE ProviderID = {INT} AND InsCoID = {INT}",ProviderID, InsuranceCoID);
				if(!rsBox24J->eof && rsBox24J->Fields->Item["Box24JNumber"]->Value.vt == VT_BSTR) {
					str = CString(rsBox24J->Fields->Item["Box24JNumber"]->Value.bstrVal);
				}
				else
					str = "";
				rsBox24J->Close();
				strIDName = "Box24J Number";
				str.TrimLeft();
				str.TrimRight();
				return (str != "");
				break;
			}
			case 13: {	//GRP
				// (j.jones 2007-03-20 10:09) - PLID 25273 - ensured we validated the GRP

				str = "";

				//first check the Insurance Co's group 
				// (b.eyers 2015-04-16) - PLID 41027 - Parameterize 
				_RecordsetPtr rsTemp = CreateParamRecordset("SELECT GRP FROM InsuranceGroups WHERE InsCoID = {INT} AND ProviderID = {INT}", InsuranceCoID, ProviderID);
				if(!rsTemp->eof && rsTemp->Fields->Item["GRP"]->Value.vt == VT_BSTR)
					str = AdoFldString(rsTemp, "GRP","");
				else
					str = "";
				rsTemp->Close();

				//then check the HCFA group's default
				// (j.jones 2007-03-12 15:04) - PLID 25175 - changed to handle the secondary loading
				// (b.eyers 2015-04-16) - PLID 41027 - Parameterize 
				rsTemp = CreateParamRecordset("SELECT Box33GRP FROM HCFASetupT WHERE ID = {INT}", HCFASetupGroupID);
				if(!rsTemp->eof && rsTemp->Fields->Item["Box33GRP"]->Value.vt == VT_BSTR) {
					CString strBox33GRP = AdoFldString(rsTemp, "Box33GRP","");
					if(strBox33GRP.GetLength() != 0)
						str = strBox33GRP;
				}
				rsTemp->Close();

				//last check for the override
				// (b.eyers 2015-04-16) - PLID 41027 - Parameterize 
				rsTemp = CreateParamRecordset("SELECT GRP FROM AdvHCFAPinT WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT}", ProviderID, LocationID, HCFASetupGroupID);
				if(!rsTemp->eof) {
					var = rsTemp->Fields->Item["GRP"]->Value;
					if(var.vt == VT_BSTR) {
						CString strTemp = VarString(var);
						strTemp.TrimLeft();
						strTemp.TrimRight();
						if(!strTemp.IsEmpty())
							str = strTemp;
					}
				}
				rsTemp->Close();

				strIDName = "GRP Number";
				str.TrimLeft();
				str.TrimRight();
				return (str != "");
				break;
			}
			// (j.jones 2007-04-25 09:00) - PLID 25764 - supported Taxonomy Code
			case 14: { //Provider Taxonomy Code
				//ProvidersT.TaxonomyCode
				strIDName = "Taxonomy Code";
				CString strValueToCheck = AdoFldString(rsDoc, "TaxonomyCode","");
				strValueToCheck.TrimLeft();
				strValueToCheck.TrimRight();
				return (strValueToCheck != "");
				break;
			}
			case 16: { //Custom ID 1
				//Custom1.TextParam
				// (b.eyers 2015-04-16) - PLID 41027 - Parameterize 
				_RecordsetPtr rsCustom = CreateParamRecordset("SELECT TextParam FROM CustomFieldDataT WHERE FieldID = 6 AND PersonID = {INT}",ProviderID);
				str = "";
				if(!rsCustom->eof) {
					var = rsCustom->Fields->Item["TextParam"]->Value;
					if(var.vt == VT_BSTR)
						str = CString(var.bstrVal);
				}
				rsCustom->Close();
				strIDName = "Custom ID 1";
				str.TrimLeft();
				str.TrimRight();
				return (str != "");
				break;
			}
			case 17: { //Custom ID 2
				//Custom2.TextParam
				// (b.eyers 2015-04-16) - PLID 41027 - Parameterize 
				_RecordsetPtr rsCustom = CreateParamRecordset("SELECT TextParam FROM CustomFieldDataT WHERE FieldID = 7 AND PersonID = {INT}",ProviderID);
				str = "";
				if(!rsCustom->eof) {
					var = rsCustom->Fields->Item["TextParam"]->Value;
					if(var.vt == VT_BSTR)
						str = CString(var.bstrVal);
				}
				rsCustom->Close();
				strIDName = "Custom ID 2";
				str.TrimLeft();
				str.TrimRight();
				return (str != "");
				break;
			}
			case 18: { //Custom ID 3
				//Custom3.TextParam
				// (b.eyers 2015-04-16) - PLID 41027 - Parameterize 
				_RecordsetPtr rsCustom = CreateParamRecordset("SELECT TextParam FROM CustomFieldDataT WHERE FieldID = 8 AND PersonID = {INT}",ProviderID);
				str = "";
				if(!rsCustom->eof) {
					var = rsCustom->Fields->Item["TextParam"]->Value;
					if(var.vt == VT_BSTR)
						str = CString(var.bstrVal);
				}
				rsCustom->Close();
				// (j.jones 2006-11-07 11:18) - PLID 23366 - fixed this, it used to say Custom ID 1
				strIDName = "Custom ID 3";
				str.TrimLeft();
				str.TrimRight();
				return (str != "");
				break;
			}
			case 19: { //Custom ID 4
				//Custom4.TextParam
				// (b.eyers 2015-04-16) - PLID 41027 - Parameterize 
				_RecordsetPtr rsCustom = CreateParamRecordset("SELECT TextParam FROM CustomFieldDataT WHERE FieldID = 9 AND PersonID = {INT}",ProviderID);
				str = "";
				if(!rsCustom->eof) {
					var = rsCustom->Fields->Item["TextParam"]->Value;
					if(var.vt == VT_BSTR)
						str = CString(var.bstrVal);
				}
				rsCustom->Close();
				strIDName = "Custom ID 4";
				str.TrimLeft();
				str.TrimRight();
				return (str != "");
				break;
			}
			}
		}
		rsDoc->Close();

		return TRUE;

	}NxCatchAll("Error validating Box 33 PIN.");

	return FALSE;
}

// (j.jones 2007-04-25 09:13) - PLID 25277 - used to only validate valid ANSI IDs that we can auto-generate
// qualifiers for - for ANSI UB92s and not UB04s
// (j.jones 2007-05-10 11:05) - PLID 25948 - handle referring physicians
// (a.walling 2008-05-20 10:12) - PLID 27812 - Added out parameter for valid NPI
BOOL CEbillingValidationDlg::CheckBox82ANSILimited(long UB92SetupGroupID, long nBox82Num, long nBox82Setup, long ProviderID, long LocationID, CString &strIDName, BOOL &bValidNPI)
{
	try {

		//PIN

		_RecordsetPtr rsDoc;
		_variant_t var;
		bValidNPI = TRUE;

		// (j.jones 2007-05-10 11:05) - PLID 25948 - handle referring physicians
		// (b.eyers 2015-04-16) - PLID 41027 - Parameterize 
		if(nBox82Setup == 3)
			rsDoc = CreateParamRecordset("SELECT * FROM ReferringPhysT INNER JOIN PersonT ON ReferringPhysT.PersonID = PersonT.ID WHERE PersonID = {INT}", ProviderID);
		else
			rsDoc = CreateParamRecordset("SELECT * FROM ProvidersT INNER JOIN PersonT ON ProvidersT.PersonID = PersonT.ID WHERE PersonID = {INT}",ProviderID);
		
		if(rsDoc->eof) {
			strIDName = "";
			return FALSE;
		}

		switch(nBox82Num) {
		case 1: { //UPIN
			//ProvidersT.UPIN
			strIDName = "UPIN";
			CString strValueToCheck = AdoFldString(rsDoc, "UPIN","");
			strValueToCheck.TrimLeft();
			strValueToCheck.TrimRight();
			return (strValueToCheck != "");
			break;
		}		
		case 2: { //Social Security Number
			strIDName = "Social Security Number";
			CString strValueToCheck = AdoFldString(rsDoc, "SocialSecurity","");
			strValueToCheck.TrimLeft();
			strValueToCheck.TrimRight();
			return (strValueToCheck != "");
			break;
		}
		case 3: { //Federal ID Number
			//ProvidersT.[Fed Employer ID]
			strIDName = "Federal Employer ID";
			CString strValueToCheck = "";
			if(nBox82Setup == 3) //handle referring physicians
				strValueToCheck = AdoFldString(rsDoc, "FedEmployerID","");
			else
				strValueToCheck = AdoFldString(rsDoc, "Fed Employer ID","");

			strValueToCheck.TrimLeft();
			strValueToCheck.TrimRight();
			return (strValueToCheck != "");
			break;
		}
		case 4: { //Medicare Number
			//ProvidersT.[Medicare Number]
			strIDName = "Medicare Number";
			CString strValueToCheck = "";
			if(nBox82Setup == 3) //handle referring physicians
				strValueToCheck = AdoFldString(rsDoc, "MedicareNumber","");
			else
				strValueToCheck = AdoFldString(rsDoc, "Medicare Number","");

			strValueToCheck.TrimLeft();
			strValueToCheck.TrimRight();
			return (strValueToCheck != "");
			break;
		}
		case 5: { //Medicaid Number
			//ProvidersT.[Medicaid Number]
			strIDName = "Medicaid Number";
			CString strValueToCheck = "";
			if(nBox82Setup == 3) //handle referring physicians
				strValueToCheck = AdoFldString(rsDoc, "MedicaidNumber","");
			else
				strValueToCheck = AdoFldString(rsDoc, "Medicaid Number","");

			strValueToCheck.TrimLeft();
			strValueToCheck.TrimRight();
			return (strValueToCheck != "");
			break;
		}
		case 6: { //BCBS Number
			//ProvidersT.[BCBS Number]
			strIDName = "BCBS Number";
			CString strValueToCheck = "";
			if(nBox82Setup == 3) //handle referring physicians
				strValueToCheck = AdoFldString(rsDoc, "BlueShieldID","");
			else
				strValueToCheck = AdoFldString(rsDoc, "BCBS Number","");

			strValueToCheck.TrimLeft();
			strValueToCheck.TrimRight();
			return (strValueToCheck != "");
			break;
		}
		case 7: { //DEA Number
			//ProvidersT.[DEA Number]
			strIDName = "DEA Number";
			CString strValueToCheck = "";
			if(nBox82Setup == 3) //handle referring physicians
				strValueToCheck = AdoFldString(rsDoc, "DEANumber","");
			else
				strValueToCheck = AdoFldString(rsDoc, "DEA Number","");

			strValueToCheck.TrimLeft();
			strValueToCheck.TrimRight();
			return (strValueToCheck != "");
			break;
		}			
		case 12: { //NPI
			//ProvidersT.NPI
			strIDName = "NPI";
			CString strValueToCheck = AdoFldString(rsDoc, "NPI","");
			strValueToCheck.TrimLeft();
			strValueToCheck.TrimRight();
			// (a.walling 2008-05-20 09:24) - PLID 27812 - Verify the NPI integrity if desired
			bValidNPI = !GetRemotePropertyInt("EbillingValidateNPIChecksum",1,0,"<None>",TRUE) || (GetRemotePropertyInt("EbillingValidateNPIChecksum",1,0,"<None>",TRUE) && IsValidNPI(strValueToCheck));
			return (strValueToCheck != "");
			break;
		}
		// (j.jones 2008-05-21 15:57) - PLID 30139 - removed the default case, because
		// it is valid to have no selection now
		}

		rsDoc->Close();

		return TRUE;

	}NxCatchAll("Error validating Box 82.");

	return FALSE;
}

// (j.jones 2007-04-25 09:31) - PLID 25277 - CheckBox82 now allows all IDs
// (a.walling 2008-05-20 10:12) - PLID 27812 - Added out parameter for valid NPI
BOOL CEbillingValidationDlg::CheckBox82(long UB92SetupGroupID, long nBox82Num, long nBox82Setup, long ProviderID, long LocationID, long InsuranceCoID, CString &strIDName, BOOL &bValidNPI)
{
	try {

		//PIN

		_RecordsetPtr rsDoc;
		_variant_t var;
		bValidNPI = TRUE;

		// (b.eyers 2015-04-16) - PLID 41027 - Parameterize 
		if(nBox82Setup == 3) //handle referring physicians
			rsDoc = CreateParamRecordset("SELECT * FROM ReferringPhysT INNER JOIN PersonT ON ReferringPhysT.PersonID = PersonT.ID WHERE PersonID = {INT}", ProviderID);
		else
			rsDoc = CreateParamRecordset("SELECT * FROM ProvidersT INNER JOIN PersonT ON ProvidersT.PersonID = PersonT.ID WHERE PersonID = {INT}", ProviderID);
		
		if(rsDoc->eof) {
			strIDName = "";
			return FALSE;
		}

		switch(nBox82Num) {
		case 1: { //UPIN
			//ProvidersT.UPIN
			strIDName = "UPIN";
			CString strValueToCheck = AdoFldString(rsDoc, "UPIN","");
			strValueToCheck.TrimLeft();
			strValueToCheck.TrimRight();
			return (strValueToCheck != "");
			break;
		}
		case 2: { //Social Security Number
			strIDName = "Social Security Number";
			CString strValueToCheck = AdoFldString(rsDoc, "SocialSecurity","");
			strValueToCheck.TrimLeft();
			strValueToCheck.TrimRight();
			return (strValueToCheck != "");
			break;
		}
		case 3: { //Federal ID Number
			//ProvidersT.[Fed Employer ID]
			strIDName = "Federal Employer ID";
			CString strValueToCheck = "";
			if(nBox82Setup == 3) //handle referring physicians
				strValueToCheck = AdoFldString(rsDoc, "FedEmployerID","");
			else
				strValueToCheck = AdoFldString(rsDoc, "Fed Employer ID","");

			strValueToCheck.TrimLeft();
			strValueToCheck.TrimRight();
			return (strValueToCheck != "");
			break;
		}
		case 4: { //Medicare Number
			//ProvidersT.[Medicare Number]
			strIDName = "Medicare Number";
			CString strValueToCheck = "";
			if(nBox82Setup == 3) //handle referring physicians
				strValueToCheck = AdoFldString(rsDoc, "MedicareNumber","");
			else
				strValueToCheck = AdoFldString(rsDoc, "Medicare Number","");

			strValueToCheck.TrimLeft();
			strValueToCheck.TrimRight();
			return (strValueToCheck != "");
			break;
		}
		case 5: { //Medicaid Number
			//ProvidersT.[Medicaid Number]
			strIDName = "Medicaid Number";
			CString strValueToCheck = "";
			if(nBox82Setup == 3) //handle referring physicians
				strValueToCheck = AdoFldString(rsDoc, "MedicaidNumber","");
			else
				strValueToCheck = AdoFldString(rsDoc, "Medicaid Number","");

			strValueToCheck.TrimLeft();
			strValueToCheck.TrimRight();
			return (strValueToCheck != "");
			break;
		}
		case 6: { //BCBS Number
			//ProvidersT.[BCBS Number]
			strIDName = "BCBS Number";
			CString strValueToCheck = "";
			if(nBox82Setup == 3) //handle referring physicians
				strValueToCheck = AdoFldString(rsDoc, "BlueShieldID","");
			else
				strValueToCheck = AdoFldString(rsDoc, "BCBS Number","");

			strValueToCheck.TrimLeft();
			strValueToCheck.TrimRight();
			return (strValueToCheck != "");
			break;
		}
		case 7: { //DEA Number
			//ProvidersT.[DEA Number]
			strIDName = "DEA Number";
			CString strValueToCheck = "";
			if(nBox82Setup == 3) //handle referring physicians
				strValueToCheck = AdoFldString(rsDoc, "DEANumber","");
			else
				strValueToCheck = AdoFldString(rsDoc, "DEA Number","");

			strValueToCheck.TrimLeft();
			strValueToCheck.TrimRight();
			return (strValueToCheck != "");
			break;
		}
		case 8: {	//Workers Comp.
			//ProvidersT.[Workers Comp Number]
			strIDName = "Workers Comp. Number";
			CString strValueToCheck = "";
			if(nBox82Setup == 3) //handle referring physicians
				strValueToCheck = AdoFldString(rsDoc, "WorkersCompNumber","");
			else
				strValueToCheck = AdoFldString(rsDoc, "Workers Comp Number","");

			strValueToCheck.TrimLeft();
			strValueToCheck.TrimRight();
			return (strValueToCheck != "");
			break;
		}
		case 9: {		//Other ID
			//ProvidersT.[Other ID Number]
			strIDName = "Other ID Number";
			CString strValueToCheck = "";
			if(nBox82Setup == 3) //handle referring physicians
				strValueToCheck = AdoFldString(rsDoc, "OtherIDNumber","");
			else
				// (j.jones 2009-03-25 10:39) - PLID 33659 - this had brackets around it, which failed
				strValueToCheck = AdoFldString(rsDoc, "Other ID Number","");

			strValueToCheck.TrimLeft();
			strValueToCheck.TrimRight();
			return (strValueToCheck != "");
			break;
		}
		case 10: {	//Box 51

			strIDName = "Other Prv. ID";

			CString strSQL = "";
			CString str = "";

			//first check the UB group's default
			// (b.eyers 2015-04-16) - PLID 41027 - Parameterize 
			_RecordsetPtr rsTemp = CreateParamRecordset("SELECT Box51Default FROM UB92SetupT WHERE ID = {INT}", UB92SetupGroupID);
			if(!rsTemp->eof && rsTemp->Fields->Item["Box51Default"]->Value.vt == VT_BSTR) {
				CString strBox51 = AdoFldString(rsTemp, "Box51Default","");
				strBox51.TrimLeft();
				strBox51.TrimRight();
				if(!strBox51.IsEmpty()) {
					return TRUE;
				}
			}
			rsTemp->Close();

			//next load from the insurance list
			// (b.eyers 2015-04-16) - PLID 41027 - Parameterize 
			rsTemp = CreateParamRecordset("SELECT Box51Info FROM InsuranceBox51 WHERE InsCoID = {INT} AND ProviderID = {INT}", InsuranceCoID, ProviderID);
			if(!rsTemp->eof && rsTemp->Fields->Item["Box51Info"]->Value.vt == VT_BSTR)
				str = AdoFldString(rsTemp, "Box51Info","");
			else
				str = "";
			rsTemp->Close();

			str.TrimLeft();
			str.TrimRight();
			return (str != "");
			break;
		}
		case 11: {	//Group Number

			CString strSQL = "";
			CString str = "";

			//check the Insurance Co's ID 
			// (b.eyers 2015-04-16) - PLID 41027 - Parameterize 
			_RecordsetPtr rsTemp = CreateParamRecordset("SELECT GRP FROM InsuranceGroups WHERE InsCoID = {INT} AND ProviderID = {INT}", InsuranceCoID, ProviderID);
			if(!rsTemp->eof && rsTemp->Fields->Item["GRP"]->Value.vt == VT_BSTR)
				str = AdoFldString(rsTemp, "GRP","");
			else
				str = "";
			rsTemp->Close();

			str.TrimLeft();
			str.TrimRight();

			strIDName = "GRP Number";
			return (str != "");
			break;
		}
		case 12: { //NPI
			//ProvidersT.NPI
			strIDName = "NPI";
			CString strValueToCheck = AdoFldString(rsDoc, "NPI","");
			strValueToCheck.TrimLeft();
			strValueToCheck.TrimRight();
			// (a.walling 2008-05-20 09:24) - PLID 27812 - Verify the NPI integrity if desired			
			bValidNPI = !GetRemotePropertyInt("EbillingValidateNPIChecksum",1,0,"<None>",TRUE) || (GetRemotePropertyInt("EbillingValidateNPIChecksum",1,0,"<None>",TRUE) && IsValidNPI(strValueToCheck));
			return (strValueToCheck != "");
			break;
		}
		// (j.jones 2008-05-21 15:57) - PLID 30139 - removed the default case, because
		// it is valid to have no selection now
		}

		rsDoc->Close();

		return TRUE;

	}NxCatchAll("Error validating Box 82.");

	return FALSE;
}

// (a.walling 2008-05-20 10:12) - PLID 27812 - Added out parameter for valid NPI
BOOL CEbillingValidationDlg::CheckBox17a(long HCFASetupGroupID, long Box17a, long RefPhyID, CString &strIDName, BOOL &bValidNPI)
{
	//this is the ID specified by the setup of Box17a

	try {

		CString str = "";
		_variant_t var;
		bValidNPI = TRUE;

		// (b.eyers 2015-04-16) - PLID 41027 - Parameterize 
		_RecordsetPtr rsDoc = CreateParamRecordset("SELECT * FROM ReferringPhysT WHERE PersonID = {INT}",RefPhyID);
		
		if(rsDoc->eof) {
			strIDName = "";
			return FALSE;
		}

		switch(Box17a) {
			case 1: { //NPI
			//ReferringPhysT.NPI
			strIDName = "Referring Phy. ID";
			CString strValueToCheck = AdoFldString(rsDoc, "NPI","");
			strValueToCheck.TrimLeft();
			strValueToCheck.TrimRight();
			// (a.walling 2008-05-20 09:24) - PLID 27812 - Verify the NPI integrity if desired
			bValidNPI = !GetRemotePropertyInt("EbillingValidateNPIChecksum",1,0,"<None>",TRUE) || (GetRemotePropertyInt("EbillingValidateNPIChecksum",1,0,"<None>",TRUE) && IsValidNPI(strValueToCheck));
			return (strValueToCheck != "");
			break;
		}
		case 2: { //Referring ID
			//ReferringPhysT.ReferringPhyID
			strIDName = "Referring Phy. ID";
			CString strValueToCheck = AdoFldString(rsDoc, "ReferringPhyID","");
			strValueToCheck.TrimLeft();
			strValueToCheck.TrimRight();
			return (strValueToCheck != "");
			break;
		}
		case 3: { //Referring UPIN
			//ReferringPhysT.UPIN
			strIDName = "UPIN";
			CString strValueToCheck = AdoFldString(rsDoc, "UPIN","");
			strValueToCheck.TrimLeft();
			strValueToCheck.TrimRight();
			return (strValueToCheck != "");
			break;
		}
		case 4: { //Referring Blue Shield
			//ReferringPhysT.BlueShieldID
			strIDName = "Blue Shield ID";
			CString strValueToCheck = AdoFldString(rsDoc, "BlueShieldID","");
			strValueToCheck.TrimLeft();
			strValueToCheck.TrimRight();
			return (strValueToCheck != "");
			break;
		}
		case 5: { //Referring FedEmployerID
			//ReferringPhysT.FedEmployerID
			strIDName = "Federal Employer ID";
			CString strValueToCheck = AdoFldString(rsDoc, "FedEmployerID","");
			strValueToCheck.TrimLeft();
			strValueToCheck.TrimRight();
			return (strValueToCheck != "");
			break;
		}
		case 6: { //Referring DEANumber
			//ReferringPhysT.DEANumber
			strIDName = "DEA Number";
			CString strValueToCheck = AdoFldString(rsDoc, "DEANumber","");
			strValueToCheck.TrimLeft();
			strValueToCheck.TrimRight();
			return (strValueToCheck != "");
			break;
		}
		case 7: { //Referring MedicareNumber
			//ReferringPhysT.MedicareNumber
			strIDName = "Medicare Number";
			CString strValueToCheck = AdoFldString(rsDoc, "MedicareNumber","");
			strValueToCheck.TrimLeft();
			strValueToCheck.TrimRight();
			return (strValueToCheck != "");
			break;
		}
		case 8: { //Referring MedicaidNumber
			//ReferringPhysT.MedicaidNumber
			strIDName = "Medicaid Number";
			CString strValueToCheck = AdoFldString(rsDoc, "MedicaidNumber","");
			strValueToCheck.TrimLeft();
			strValueToCheck.TrimRight();
			return (strValueToCheck != "");
			break;
		}
		case 9: { //Referring WorkersCompNumber
			//ReferringPhysT.WorkersCompNumber
			strIDName = "Workers Comp. Number";
			CString strValueToCheck = AdoFldString(rsDoc, "WorkersCompNumber","");
			strValueToCheck.TrimLeft();
			strValueToCheck.TrimRight();
			return (strValueToCheck != "");
			break;
		}
		case 10: { //Referring OtherIDNumber
			//ReferringPhysT.OtherIDNumber
			strIDName = "Other ID Number";
			CString strValueToCheck = AdoFldString(rsDoc, "OtherIDNumber","");
			strValueToCheck.TrimLeft();
			strValueToCheck.TrimRight();
			return (strValueToCheck != "");
			break;
		}
		case 11: { //Referring License Number
			//ReferringPhysT.License
			strIDName = "License Number";
			CString strValueToCheck = AdoFldString(rsDoc, "License","");
			strValueToCheck.TrimLeft();
			strValueToCheck.TrimRight();
			return (strValueToCheck != "");
			break;
		}
		// (j.jones 2007-04-24 11:56) - PLID 25764 - supported Taxonomy Code
		// (j.jones 2012-03-07 15:58) - PLID  48676 - removed Taxonomy Code
		/*
		case 12: { //Referring Taxonomy Code
			//ReferringPhysT.TaxonomyCode
			strIDName = "Taxonomy Code";
			CString strValueToCheck = AdoFldString(rsDoc, "TaxonomyCode","");
			strValueToCheck.TrimLeft();
			strValueToCheck.TrimRight();
			return (strValueToCheck != "");
			break;
		}
		*/
		case 16: { //Custom ID 1
			//Custom1.TextParam
			// (b.eyers 2015-04-16) - PLID 41027 - Parameterize 
			_RecordsetPtr rsCustom = CreateParamRecordset("SELECT TextParam FROM CustomFieldDataT WHERE PersonID = {INT} AND FieldID = 6",RefPhyID);
			str = "";
			if(!rsCustom->eof) {
				var = rsCustom->Fields->Item["TextParam"]->Value;
				if(var.vt == VT_BSTR)
					str = CString(var.bstrVal);
			}
			rsCustom->Close();
			strIDName = "Custom ID 1";
			str.TrimLeft();
			str.TrimRight();
			return (str != "");
			break;
		}
		case 17: { //Custom ID 2
			//Custom2.TextParam
			// (b.eyers 2015-04-16) - PLID 41027 - Parameterize 
			_RecordsetPtr rsCustom = CreateParamRecordset("SELECT TextParam FROM CustomFieldDataT WHERE PersonID = {INT} AND FieldID = 7",RefPhyID);
			str = "";
			if(!rsCustom->eof) {
				var = rsCustom->Fields->Item["TextParam"]->Value;
				if(var.vt == VT_BSTR)
					str = CString(var.bstrVal);
			}
			rsCustom->Close();
			strIDName = "Custom ID 2";
			str.TrimLeft();
			str.TrimRight();
			return (str != "");
			break;
		}
		case 18: { //Custom ID 3
			//Custom3.TextParam
			// (b.eyers 2015-04-16) - PLID 41027 - Parameterize 
			_RecordsetPtr rsCustom = CreateParamRecordset("SELECT TextParam FROM CustomFieldDataT WHERE PersonID = {INT} AND FieldID = 8",RefPhyID);
			str = "";
			if(!rsCustom->eof) {
				var = rsCustom->Fields->Item["TextParam"]->Value;
				if(var.vt == VT_BSTR)
					str = CString(var.bstrVal);
			}
			rsCustom->Close();
			strIDName = "Custom ID 3";
			str.TrimLeft();
			str.TrimRight();
			return (str != "");
			break;
		}
		case 19: { //Custom ID 4
			//Custom4.TextParam
			// (b.eyers 2015-04-16) - PLID 41027 - Parameterize 
			_RecordsetPtr rsCustom = CreateParamRecordset("SELECT TextParam FROM CustomFieldDataT WHERE PersonID = {INT} AND FieldID = 9",RefPhyID);
			str = "";
			if(!rsCustom->eof) {
				var = rsCustom->Fields->Item["TextParam"]->Value;
				if(var.vt == VT_BSTR)
					str = CString(var.bstrVal);
			}
			rsCustom->Close();
			strIDName = "Custom ID 4";
			str.TrimLeft();
			str.TrimRight();
			return (str != "");
			break;
		}
		}

		rsDoc->Close();

		return TRUE;

	}NxCatchAll("Error validating Box 17a.");

	return FALSE;
}

// (j.jones 2006-12-11 17:00) - PLID 23811 - added OHIP-specific validation function
BOOL CEbillingValidationDlg::ValidateOneOHIP(long nClaimID)
{
	try {

		_RecordsetPtr rs;
		_variant_t var;
		BOOL bPassed = TRUE;
		BOOL bWarnings = FALSE;

		BOOL bHasInsurance = TRUE;

		CString str, strErrors = "";

		long InsuranceCoID = -1;		
		long nPOSID = -1;
		CString strInsCoName = "";

		long nBillID = -1;
		long nPatientID = -1;
		long nFormType = 1;

		BOOL bIsRMB = FALSE;

		//output the header, and while doing so, check our basic information on the claim
		// (j.jones 2008-12-03 17:16) - PLID 32305 - added much more info for the new validations
		// (j.jones 2010-02-02 09:03) - PLID 33060 - pull the General 2 ref. phy. NPI as well
		// (j.jones 2011-08-16 17:22) - PLID 44805 - we will filter out "original" and "void" charges
		// (j.jones 2011-09-23 16:14) - PLID 39377 - added AlreadyExportedToday
		//(r.wilson 10/2/2012) plid 52970 - Replace hardcoded SendTypes with enumerated values
		// (d.singleton 2014-03-17 15:12) - PLID 61308 - refactor older ebilling validations to work with new diag code data structure
		rs = CreateParamRecordset("SELECT BillsT.ID AS BillID, BillsT.FormType, "
			"PersonT.[Last] AS PatLast, PersonT.[First] AS PatFirst, "
			"PersonT.[Last] + ', ' + PersonT.[First] + ' ' + PersonT.Middle AS PatientName, "
			"PersonT.BirthDate, PersonT.Gender, PersonT.State, "
			"BillsT.Description, BillsT.Date, PersonT.ID AS PersonID, InsuranceCoT.PersonID AS InsCoID, "
			"InsuranceCoT.Name AS InsCoName, PlaceOfServiceT.ID AS POSID, PlaceOfServiceT.NPI AS POSNPI, "
			"LocationsT.NPI AS LocNPI, InsurancePlansT.PlanName, "
			"ReferringPhysT.PersonID AS BillRefPhyID, PatReferringPhysT.PersonID AS G2RefPhyID, "
			"ReferringPhysT.NPI AS BillRefPhyNPI, PatReferringPhysT.NPI AS G2RefPhyNPI, "
			"ReferringPhysPersonT.[Last] + ', ' + ReferringPhysPersonT.[First] + ' ' + ReferringPhysPersonT.Middle AS BillRefPhyName, "
			"PatReferringPhysPersonT.[Last] + ', ' + PatReferringPhysPersonT.[First] + ' ' + PatReferringPhysPersonT.Middle AS G2RefPhyName, "
			"Convert(bit, CASE WHEN DATEADD(month, 6, LineItemT.Date) < GetDate() THEN 1 ELSE 0 END) AS IsOldCharge, "
			"Convert(bit, CASE WHEN LineItemT.Date > GetDate() THEN 1 ELSE 0 END) AS IsFutureCharge, "
			"ChargesT.ItemCode, BillDiagCodeT.ICD9DiagID AS Diag1ID, "
			"ConverT(bit, CASE WHEN AlreadyExportedTodayQ.BillID Is Null THEN 0 ELSE 1 END) AS AlreadyExportedToday "
			"FROM HCFATrackT "
			"INNER JOIN BillsT ON HCFATrackT.BillID = BillsT.ID "
			"INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"LEFT JOIN BillDiagCodeT ON BillsT.ID = BillDiagCodeT.BillID AND BillDiagCodeT.OrderIndex = 1 "
			"LEFT JOIN PatientsT ON BillsT.PatientID = PatientsT.PersonID "
			"LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			"LEFT JOIN InsuredPartyT ON BillsT.InsuredPartyID = InsuredPartyT.PersonID "
			"LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
			"LEFT JOIN InsurancePlansT ON InsuredPartyT.InsPlan = InsurancePlansT.ID "
			"LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
			"LEFT JOIN LocationsT AS PlaceOfServiceT ON BillsT.Location = PlaceOfServiceT.ID "
			"LEFT JOIN ReferringPhysT ON BillsT.RefPhyID = ReferringPhysT.PersonID "
			"LEFT JOIN PersonT ReferringPhysPersonT ON ReferringPhysT.PersonID = ReferringPhysPersonT.ID "
			"LEFT JOIN ReferringPhysT PatReferringPhysT ON PatientsT.DefaultReferringPhyID = PatReferringPhysT.PersonID "
			"LEFT JOIN PersonT PatReferringPhysPersonT ON PatReferringPhysT.PersonID = PatReferringPhysPersonT.ID "
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
			"LEFT JOIN (SELECT BillID FROM ClaimHistoryT "
			//(r.wilson 10/2/2012) plid 52970 - below the old SendType value was 0
			"	WHERE SendType = {INT} AND dbo.AsDateNoTime(Date) = dbo.AsDateNoTime(GetDate()) "
			"	GROUP BY BillID) AS AlreadyExportedTodayQ ON BillsT.ID = AlreadyExportedTodayQ.BillID "
			"WHERE BillsT.Deleted = 0 AND LineItemT.Deleted = 0 AND ChargesT.Batched = 1 "
			"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
			"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
			"AND HCFATrackT.ID = {INT}", ClaimSendType::Electronic,nClaimID);

		if(rs->eof) {

			//Bill - this is NOT a warning that can be disabled

			str.Format("\tERROR - No Valid Bill Was Found (Claim ID: %li)\r\n\r\n"
				"\t\tEither all the charges have been deleted off of this bill,\r\n"
				"\t\tall the charges have been unbatched from this bill,\r\n"
				"\t\tor this bill no longer exists.", nClaimID);
			strErrors += str;
			bPassed = FALSE;

			// (j.jones 2009-03-02 11:07) - PLID 33286 - return now, we can't check anything else
			m_strInvalidClaims += strErrors;

			ExecuteParamSql("UPDATE HCFATrackT SET ValidationState = 2 WHERE ID = {INT}", nClaimID);

			return FALSE;
		}
		else {

			nBillID = AdoFldLong(rs, "BillID",-1);

			nPatientID = AdoFldLong(rs, "PersonID",-1);

			//Patient - this is NOT a warning that can be disabled
			if(nPatientID == -1) {				
				str.Format("\tERROR - No Patient Was Found (Claim ID: %li)\r\n\r\n"
					"\t\tThere is no patient associated with this claim.\r\n"
					"\t\tYou will need to recreate this claim.\r\n\r\n\r\n", nClaimID);
				strErrors += str;
				bPassed = FALSE;
			}

			//it is highly unlikely that the above two errors will ever occur. If they do, we can't get the patient name.
			//but now we can, so generate this patient's report header.
			//In the event there are no errors, strErrors is cleared out at the end of this function

			CString strPatientName = AdoFldString(rs, "PatientName","");
			CString strDescription = AdoFldString(rs, "Description","");

			str.Format("____________________________________________________________________________________\r\n\r\n"
				"Patient:  %s\t\r\n\r\nBill Desc: %s     Bill Date: %s\r\n\r\n",
				strPatientName, strDescription, FormatDateTimeForInterface(AdoFldDateTime(rs, "Date")));
			strErrors += str;

			//Insurance Company - this is NOT a warning that can be disabled
			InsuranceCoID = AdoFldLong(rs, "InsCoID",-1);			
			if(InsuranceCoID == -1) {				
				str.Format("\tERROR - No Insurance Company Selected\r\n\r\n"
					"\t\tThere is no Insurance Company selected on this claim.\r\n"
					"\t\tCheck the patient's account and ensure that an\r\n"
					"\t\tinsurance company is selected on the Insurance tab of\r\n"
					"\t\tthe Bill. If no insurance companies are available,\r\n"
					"\t\tyou must add one in the patient's Insurance tab.\r\n\r\n\r\n");
				strErrors += str;
				bHasInsurance = FALSE;
				bPassed = FALSE;
			}

			// (j.jones 2011-09-23 16:07) - PLID 39377 - added validation for duplicate same-day ebilling claims
			if(AdoFldBool(rs, "AlreadyExportedToday", FALSE)
				&& GetRemotePropertyInt("OHIPValidateExportedTwiceInSameDay",1,0,"<None>",TRUE) == 1) {

				str.Format("\tWARNING - Claim Exported Twice In The Same Day\r\n\r\n"
					"\t\tThis bill has already been exported electronically today.\r\n"
					"\t\tTo avoid a duplicate claim rejection, you should not export\r\n"
					"\t\tthis bill unless you failed to submit the previous export\r\n"
					"\t\tto OHIP.\r\n\r\n\r\n");
				strErrors += str;
				bWarnings = TRUE;
			}

			//store some variables for later
			if(bHasInsurance) {
				strInsCoName = AdoFldString(rs, "InsCoName", "");
			}

			// (j.jones 2008-12-03 16:41) - PLID 32305 - check the POS validation preference
			if(GetRemotePropertyInt("OHIPValidatePOSNPI",1,0,"<None>",TRUE) == 1) {
				nPOSID = AdoFldLong(rs, "POSID",-1);
				if(nPOSID == -1) {				
					str.Format("\tWARNING - No Place Of Service Selected\r\n\r\n"
						"\t\tThere is no Place Of Service selected on this claim.\r\n"
						"\t\tCheck the patient's account and ensure that a Place Of\r\n"
						"\t\tservice is selected on the lower left-hand side of the Bill.\r\n\r\n\r\n");
					strErrors += str;
					bWarnings = TRUE;
				}
				else {

					//warn if the Place Of Service has no NPI number
					//(e.lally 2009-03-25) PLID 33634 - updated message to reflect dynamic label.
					CString strPOSNPI = AdoFldString(rs, "POSNPI","");
					strPOSNPI.TrimRight();
					if(strPOSNPI.IsEmpty()) {
						str.Format("\tWARNING - No Facility Number Found\r\n\r\n"
							"\t\tThe place of service selected for this bill has no Facility Number filled in.\r\n"
							"\t\tThis number is pulled from the Facility Number field in the Locations tab\r\n"
							"\t\tof the Administrator module.\r\n"
							"\t\tPlease ensure that all locations in the Administrator module have a 4-digit number\r\n"
							"\t\tfilled in the Facility Number field.\r\n\r\n\r\n");
						strErrors += str;
						bWarnings = TRUE;
					}
					//check that it is 4 characters long - they are routinely numbers but since the format
					//allows for non-numerics, we won't force them to be numeric here, but we WILL use the word
					//digits to imply that they should be numbers
					//(e.lally 2009-03-25) PLID 33634 - updated message to reflect dynamic label.
					else if(strPOSNPI.GetLength() != 4) {
						//this is an error, because it's an invalid code
						str.Format("\tERROR - Facility Number Invalid\r\n\r\n"
							"\t\tThe place of service selected for this bill has a Facility Number of '%s'\r\n"
							"\t\twhich is not 4 digits long.\r\n"
							"\t\tThis number is pulled from the Facility Number field in the Locations tab\r\n"
							"\t\tof the Administrator module.\r\n"
							"\t\tPlease ensure that all locations in the Administrator module have a 4-digit number\r\n"
							"\t\tfilled in the Facility Number field.\r\n\r\n\r\n", strPOSNPI);
						strErrors += str;
						bPassed = FALSE;
					}
				}
			}

			// (j.jones 2008-12-03 16:41) - PLID 32305 - check the Location NPI validation preference
			if(GetRemotePropertyInt("OHIPValidateLocationNPI",1,0,"<None>",TRUE) == 1) {
				//warn if the Bill Location has no NPI number			
				CString strLocNPI = AdoFldString(rs, "LocNPI","");
				strLocNPI.TrimRight();
				if(strLocNPI.IsEmpty()) {
					//(e.lally 2009-03-25) PLID 33634 - updated message to reflect dynamic label. (Admin field only labels
					//	facility number.)
					str.Format("\tWARNING - No Location Code Found\r\n\r\n"
						"\t\tThe location selected for this bill has no Location Code filled in.\r\n"
						"\t\tThis number is pulled from the Facility Number field in the Locations tab\r\n"
						"\t\tof the Administrator module.\r\n"
						"\t\tPlease ensure that all locations in the Administrator module have a 4-digit number\r\n"
						"\t\tfilled in the Facility Number field.\r\n\r\n\r\n");
					strErrors += str;
					bWarnings = TRUE;
				}
				//check that it is 4 characters long - they are routinely numbers but since the format
				//allows for non-numerics, we won't force them to be numeric here, but we WILL use the word
				//digits to imply that they should be numbers
				else if(strLocNPI.GetLength() != 4) {
					//this is an error, because it's an invalid code
					//(e.lally 2009-03-25) PLID 33634 - updated message to reflect dynamic label. (Admin field only labels
					//	facility number.)
					str.Format("\tERROR - Location Code Invalid\r\n\r\n"
						"\t\tThe location selected for this bill has a Location Code of '%s'\r\n"
						"\t\twhich is not 4 digits long.\r\n"
						"\t\tThis number is pulled from the Facility Number field in the Locations tab\r\n"
						"\t\tof the Administrator module.\r\n"
						"\t\tPlease ensure that all locations in the Administrator module have a 4-digit number\r\n"
						"\t\tfilled in the Facility Number field.\r\n\r\n\r\n", strLocNPI);
					strErrors += str;
					bPassed = FALSE;
				}
			}
			
			// (j.jones 2008-12-03 16:45) - PLID 32305 - I removed this validation, but we do need to check
			// to see if the PlanName is RMB
			CString strPlanName = AdoFldString(rs, "PlanName","");
			strPlanName.TrimLeft();
			strPlanName.TrimRight();
			strPlanName.MakeUpper();
			bIsRMB = strPlanName == "RMB";
			/*
			//ensure that the patient has a plan name selected and that it is HCP, WCB, or RMB			
			if(strPlanName != "HCP" && strPlanName != "WCB" && strPlanName != "RMB") {
				str.Format("\tERROR - Invalid Insurance Plan Name\r\n\r\n"
					"\t\tThe insured party for this claim does not have a proper plan name selected.\r\n"
					"\t\tEach insured party must have a plan name of 'HCP', 'WCB', or 'RMB'.\r\n"
					"\t\tOn the patient's insurance tab, select a plan name for their insured party.\r\n"
					"\t\tIf a valid plan name is not available, click on 'Edit Insurance List' and\r\n"
					"\t\tensure that the insurance company has these three plan types as options.\r\n\r\n\r\n");
				strErrors += str;
				bPassed = FALSE;
			}
			*/

			nFormType = AdoFldLong(rs, "FormType",1);

			// (j.jones 2008-12-03 16:41) - PLID 32305 - added referring physician validation
			long nBillRefPhyID = AdoFldLong(rs, "BillRefPhyID", -1);
			long nG2RefPhyID = AdoFldLong(rs, "G2RefPhyID",-1);

			// (j.jones 2010-02-02 09:30) - PLID 33060 - if there is no referring physician selected on the bill,
			// there is a preference to use the G2 ref. phy. instead
			// (d.thompson 2010-03-23) - PLID 37850 - m.clark set this to enabled by default
			BOOL bUseG2RefPhy = (GetRemotePropertyInt("OHIPUseG2RefPhy", 1, 0, "<None>", true) == 1);

			if(GetRemotePropertyInt("OHIPValidateRefPhySelected",1,0,"<None>",TRUE) == 1) {
				//warn if there is no referring physician selected				
				if(nBillRefPhyID == -1 && !bUseG2RefPhy) {
					str.Format("\tERROR - No Referring Physician Selected\r\n\r\n"
						"\t\tThis bill has no referring physician selected.\r\n"
						"\t\tPlease ensure that each bill has a referring physician selected in the Insurance tab of the bill.\r\n\r\n\r\n");
					strErrors += str;
					bPassed = FALSE;
				}
				else if(nG2RefPhyID == -1 && bUseG2RefPhy) {
					// (j.jones 2010-02-02 09:30) - PLID 33060 - warn when there isn't even a G2 physician
					str.Format("\tERROR - No Default Referring Physician Selected\r\n\r\n"
						"\t\tNo referring physician is selected on this bill. Your preferences state that in this\r\n"
						"\t\tcase the referring physician should be pulled from General 2, but this patient has no\r\n"
						"\t\tdefault referring physician selected in the General 2 tab.\r\n"
						"\t\tPlease ensure that either the bill or the patient has a referring physician selected.\r\n\r\n\r\n");
					strErrors += str;
					bPassed = FALSE;
				}
			}

			// (j.jones 2008-12-03 16:41) - PLID 32305 - added referring physician NPI validation
			// (j.jones 2010-02-02 09:28) - PLID 33060 - this has changed slightly as now the claim
			// can be configured to send the G2 NPI if no referring physician is on the bill
			if(GetRemotePropertyInt("OHIPValidateRefPhyNPI",1,0,"<None>",TRUE) == 1) {
				if(nBillRefPhyID != -1) {				
					//warn if there is no ref phy NPI
					CString strRefPhyNPI = AdoFldString(rs, "BillRefPhyNPI","");
					CString strRefPhyName = AdoFldString(rs, "BillRefPhyName","");
					strRefPhyNPI.TrimLeft();
					strRefPhyNPI.TrimRight();
					if(strRefPhyNPI.IsEmpty()) {	
						//(e.lally 2009-03-25) PLID 33634 - updated message to reflect dynamic label.
						str.Format("\tERROR - No Referring Physician Number Found\r\n\r\n"
							"\t\tThe referring physician on this bill, '%s', has no ID number filled in.\r\n"
							"\t\tThis number is pulled from the Ref Phys Number field in the Contacts module.\r\n"
							"\t\tPlease ensure that all referring physicians in the Contacts module have 6-digit IDs\r\n"
							"\t\tfilled in the Ref Phys Number field.\r\n\r\n\r\n", strRefPhyName);
						strErrors += str;
						bPassed = FALSE;
					}
					//check that it is 6 digits long - numbers only
					else if(strRefPhyNPI.GetLength() != 6 || (atoi(strRefPhyNPI) == 0 && strRefPhyNPI != "000000")) {
						//(e.lally 2009-03-25) PLID 33634 - updated message to reflect dynamic label.
						str.Format("\tERROR - Referring Physician Number Invalid\r\n\r\n"
							"\t\tThe referring physician on this bill, '%s', has an invalid ID number of '%s'\r\n"
							"\t\twhich is not 6 digits long.\r\n"
							"\t\tThis number is pulled from the Ref Phys Number field in the Contacts module.\r\n"
							"\t\tPlease ensure that all referring physicians in the Contacts module have 6-digit IDs\r\n"
							"\t\tfilled in the Ref Phys Number field.\r\n\r\n\r\n", strRefPhyName, strRefPhyNPI);
						strErrors += str;
						bPassed = FALSE;
					}
				}
				// (j.jones 2010-02-02 09:30) - PLID 33060 - if there is no referring physician selected on the bill,
				// there is a preference to use the G2 ref. phy. instead, so if they did that, validate that NPI
				else if(bUseG2RefPhy && nG2RefPhyID != -1) {
					CString strRefPhyNPI = AdoFldString(rs, "G2RefPhyNPI","");
					CString strRefPhyName = AdoFldString(rs, "G2RefPhyName","");
					strRefPhyNPI.TrimLeft();
					strRefPhyNPI.TrimRight();
					if(strRefPhyNPI.IsEmpty()) {	
						str.Format("\tERROR - No Referring Physician Number Found\r\n\r\n"
							"\t\tNo referring physician is selected on this bill. Your preferences state that in this\r\n"
							"\t\tcase the referring physician should be pulled from General 2, however that default\r\n"
							"\t\treferring physician, '%s', has no ID number filled in.\r\n"
							"\t\tThis number is pulled from the Ref Phys Number field in the Contacts module.\r\n"
							"\t\tPlease ensure that all referring physicians in the Contacts module have 6-digit IDs\r\n"
							"\t\tfilled in the Ref Phys Number field.\r\n\r\n\r\n", strRefPhyName);
						strErrors += str;
						bPassed = FALSE;
					}
					//check that it is 6 digits long - numbers only
					else if(strRefPhyNPI.GetLength() != 6 || (atoi(strRefPhyNPI) == 0 && strRefPhyNPI != "000000")) {
						//(e.lally 2009-03-25) PLID 33634 - updated message to reflect dynamic label.
						str.Format("\tERROR - Referring Physician Number Invalid\r\n\r\n"
							"\t\tNo referring physician is selected on this bill. Your preferences state that in this\r\n"
							"\t\tcase the referring physician should be pulled from General 2, however that default\r\n"
							"\t\treferring physician, '%s', has an invalid ID number of '%s'\r\n"
							"\t\twhich is not 6 digits long.\r\n"
							"\t\tThis number is pulled from the Ref Phys Number field in the Contacts module.\r\n"
							"\t\tPlease ensure that all referring physicians in the Contacts module have 6-digit IDs\r\n"
							"\t\tfilled in the Ref Phys Number field.\r\n\r\n\r\n", strRefPhyName, strRefPhyNPI);
						strErrors += str;
						bPassed = FALSE;
					}
				}
			}

			// (j.jones 2008-12-03 16:41) - PLID 32305 - added patient birthdate validation
			if(GetRemotePropertyInt("OHIPValidatePatientBirthdate",1,0,"<None>",TRUE) == 1) {
				BOOL bBirthDate = TRUE;
				var = rs->Fields->Item["BirthDate"]->Value;
				if(var.vt != VT_DATE) {
					bBirthDate = FALSE;
				}
				if(!bBirthDate) {
					str.Format("\tWARNING - No Patient Birthdate Found\r\n\r\n"
						"\t\tThe patient on this bill does not have a birthdate filled in.\r\n"
						"\t\tPlease ensure that each patient has a Birthdate filled in on their General 1 tab.\r\n\r\n\r\n");
					strErrors += str;
					bWarnings = TRUE;
				}
			}

			// (j.jones 2008-12-03 16:41) - PLID 32305 - added patient name validation
			if(GetRemotePropertyInt("OHIPValidatePatientName",1,0,"<None>",TRUE) == 1) {
				CString strFirst = AdoFldString(rs, "PatFirst", "");
				CString strLast = AdoFldString(rs, "PatLast", "");
				strFirst.TrimLeft();
				strFirst.TrimRight();
				strLast.TrimLeft();
				strLast.TrimRight();
				if(strFirst.IsEmpty() || strLast.IsEmpty()) {
					//this field is an error if not RMB, a warning otherwise
					if(bIsRMB) {
						str.Format("\tERROR - No Patient Name Found\r\n\r\n"
							"\t\tThe patient on this bill is missing either a first or last name.\r\n"
							"\t\tPlease ensure that each patient has their full name filled in on their General 1 tab.\r\n"
							"\t\tBecause this is a RMB claim, this field is required.\r\n\r\n\r\n");
						strErrors += str;
						bPassed = FALSE;
					}
					else {
						str.Format("\tWARNING - No Patient Name Found\r\n\r\n"
							"\t\tThe patient on this bill is missing either a first or last name.\r\n"
							"\t\tPlease ensure that each patient has their full name filled in on their General 1 tab.\r\n"
							"\t\tBecause this is not a RMB claim, this field is not required.\r\n\r\n\r\n");
						strErrors += str;
						bWarnings = TRUE;
					}
				}
			}

			// (j.jones 2008-12-03 16:41) - PLID 32305 - added patient gender validation
			if(GetRemotePropertyInt("OHIPValidatePatientGender",1,0,"<None>",TRUE) == 1) {
				//warn if there is no gender
				short nGender = AdoFldByte(rs, "Gender",0);				
				if(nGender != 1 && nGender != 2) {
					//this field is an error if not RMB, a warning otherwise
					if(bIsRMB) {
						str.Format("\tERROR - No Patient Gender Found\r\n\r\n"
							"\t\tThe patient on this bill has no Gender entered on their account.\r\n"
							"\t\tPlease ensure that each patient has a Gender filled in on their General 1 tab.\r\n"
							"\t\tBecause this is a RMB claim, this field is required.\r\n\r\n\r\n");
						strErrors += str;
						bPassed = FALSE;
					}
					else {
						str.Format("\tWARNING - No Patient Gender Found\r\n\r\n"
							"\t\tThe patient on this bill has no Gender entered on their account.\r\n"
							"\t\tPlease ensure that each patient has a Gender filled in on their General 1 tab.\r\n"
							"\t\tBecause this is not a RMB claim, this field is not required.\r\n\r\n\r\n");
						strErrors += str;
						bWarnings = TRUE;
					}
				}
			}

			// (j.jones 2008-12-03 16:41) - PLID 32305 - added patient province validation
			if(GetRemotePropertyInt("OHIPValidatePatientProvince",1,0,"<None>",TRUE) == 1) {
				//warn if there is no province
				CString strState = AdoFldString(rs, "State","");
				strState.TrimRight();
				if(strState.IsEmpty()) {
					//this field is an error if not RMB, a warning otherwise
					if(bIsRMB) {
						str.Format("\tERROR - No Patient Province Found\r\n\r\n"
							"\t\tThe patient on this bill has no Province in the address on their account.\r\n"
							"\t\tPlease ensure that each patient has a Province entered in the address on their General 1 tab.\r\n"
							"\t\tBecause this is a RMB claim, this field is required.\r\n\r\n\r\n");
						strErrors += str;
						bPassed = FALSE;
					}
					else {
						str.Format("\tWARNING - No Patient Province Found\r\n\r\n"
							"\t\tThe patient on this bill has no Province in the address on their account.\r\n"
							"\t\tPlease ensure that each patient has a Province entered in the address on their General 1 tab.\r\n"
							"\t\tBecause this is not a RMB claim, this field is not required.\r\n\r\n\r\n");
						strErrors += str;
						bWarnings = TRUE;
					}
				}
			}

			// (j.jones 2008-12-03 16:41) - PLID 32305 - added diag code validation
			if(GetRemotePropertyInt("OHIPValidateDiagCode",1,0,"<None>",TRUE) == 1) {
				//warn if there is no diagnosis code
				long nDiag1ID = AdoFldLong(rs, "Diag1ID", -1);
				if(nDiag1ID == -1) {
					str.Format("\tERROR - No Diagnosis Code Selected\r\n\r\n"
						"\t\tThis bill has no Diagnosis Code 1 selected.\r\n"
						"\t\tPlease ensure that each bill has a diagnosis code selected in the Diag Code 1 field.\r\n\r\n\r\n");
					strErrors += str;
					bPassed = FALSE;
				}
			}

			//now loop through each charge
			while(!rs->eof) {

				// (j.jones 2008-12-03 17:28) - PLID 32305 - this code needs to be in the format
				// ANNNA (alpha - 3 numbers - alpha), where the latter A can only be the letters A, B, or C
				if(GetRemotePropertyInt("OHIPValidateServiceCode",1,0,"<None>",TRUE) == 1) {
					CString strCode = AdoFldString(rs, "ItemCode", "");
					BOOL bValid = TRUE;
					if(strCode.GetLength() != 5) {
						bValid = FALSE;
					}
					else {
						//validate each character
						CString strCodeToCheck = strCode;
						strCodeToCheck.MakeUpper();
						if(strCodeToCheck.GetAt(0) < 'A' || strCodeToCheck.GetAt(0) > 'Z') {
							//invalid code
							bValid = FALSE;
						}
						if(bValid && (strCodeToCheck.GetAt(4) < 'A' || strCodeToCheck.GetAt(4) > 'C')) {
							//invalid code
							bValid = FALSE;
						}
						//check the middle three digits
						if(bValid && (strCodeToCheck.GetAt(1) < '0' || strCodeToCheck.GetAt(1) > '9')) {
							//invalid code
							bValid = FALSE;
						}
						if(bValid && (strCodeToCheck.GetAt(2) < '0' || strCodeToCheck.GetAt(2) > '9')) {
							//invalid code
							bValid = FALSE;
						}
						if(bValid && (strCodeToCheck.GetAt(3) < '0' || strCodeToCheck.GetAt(3) > '9')) {
							//invalid code
							bValid = FALSE;
						}
					}
					if(!bValid) {
						str.Format("\tERROR - Invalid Service Code\r\n\r\n"
							"\t\tThis bill has a charge with the invalid service code '%s'.\r\n"
							"\t\tValid service codes are in the format ANNNA (a letter, three numbers, and a letter).\r\n"
							"\t\tYou must remove this invalid code from this bill and re-add a charge with the corrected\r\n"
							"\t\tservice code in order to pass validation.\r\n\r\n\r\n", strCode);
						strErrors += str;
						bPassed = FALSE;
					}
				}

				// (j.jones 2008-12-03 17:19) - PLID 32305 - this validation was pre-existing,
				// but I moved it to be inside this recordset to save a database access
				BOOL bWarnedOldCharge = FALSE;
				BOOL bWarnedFutureCharge = FALSE;
				//ensure that the service date on a charge is not over 6 months old, or in the future
				if(!bWarnedOldCharge && AdoFldBool(rs, "IsOldCharge", FALSE)) {				
					
					str.Format("\tERROR - Outdated Charge Date\r\n\r\n"
						"\t\tThis bill includes a charge with service date greater than 6 months old.\r\n\r\n\r\n");
					strErrors += str;
					bPassed = FALSE;

					bWarnedOldCharge = TRUE;
				}

				if(!bWarnedFutureCharge && AdoFldBool(rs, "IsFutureCharge", FALSE)) {				
					str.Format("\tERROR - Future Charge Date\r\n\r\n"
						"\t\tThis bill includes a charge with service date in the future.\r\n\r\n\r\n");
					strErrors += str;
					bPassed = FALSE;

					bWarnedFutureCharge = TRUE;
				}

				rs->MoveNext();
			}
		}
		rs->Close();

		// (j.jones 2008-12-03 16:41) - PLID 32305 - check the Group Number validation preference
		if(GetRemotePropertyInt("OHIPValidateGroupNumber",1,0,"<None>",TRUE) == 1) {

			//now grab the group number
			CString strGroup = GetRemotePropertyText("OHIP_GroupNumber", "0000", 0, "<None>", true);
			strGroup.Replace(" ", "");

			// (j.jones 2009-02-09 12:17) - PLID 32997 - if they have a group number, validate that
			// it is 4 characters, regardless of whether it's used in this export, because it is
			// used in the filename
			if(!strGroup.IsEmpty() && strGroup != "0000" && strGroup.GetLength() != 4) {
				str.Format("\tERROR - Group Number Invalid\r\n\r\n"
					"\t\tThere is an invalid group number entered in the OHIP Properties.\r\n"
					"\t\tValid group numbers must be 4 digits in length.\r\n"
					"\t\tTo fix this, go to the OHIP Properties screen and enter in your\r\n"
					"\t\toffice's 4 digit group number.\r\n\r\n\r\n");
				strErrors += str;
				bPassed = FALSE;
			}
			else if(strGroup.IsEmpty() || strGroup == "0000") {

				//if more than one provider exists in the batch (selected or unselected),
				//warn if there is no group number
				//or if that the group number is "0000" (which means no group)
				//(yes, this is a validation not specific to this claim)
				// (j.jones 2011-08-16 17:22) - PLID 44805 - we will filter out "original" and "void" charges
				rs = CreateParamRecordset("SELECT DoctorsProviders FROM ChargesT "
					"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
					"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
					"INNER JOIN HCFATrackT ON BillsT.ID = HCFATrackT.BillID "
					"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
					"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
					"WHERE ChargesT.Batched = 1 AND LineItemT.Deleted = 0 "
					"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
					"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
					"AND BillsT.FormType = {INT} "
					"AND DoctorsProviders <> -1 "
					"AND HCFATrackT.Batch = {INT} "
					"GROUP BY DoctorsProviders", nFormType, m_bElectronic ? 2 : 1);

				//technically this validation cannot even be run if not electronic, but
				//might as well be accurate

				if(rs->GetRecordCount() > 1) {

					//there is more than one provider in the selected batched claims

					strGroup.Replace(" ", "");
					str.Format("\tERROR - No Group Number Found\r\n\r\n"
						"\t\tThe selected batched claims include bills with multiple providers,\r\n"
						"\t\tand no valid group number is entered in the OHIP Properties.\r\n"
						"\t\tTo fix this, go to the OHIP Properties screen and enter in your\r\n"
						"\t\toffice's group number.\r\n\r\n\r\n");
					strErrors += str;
					bPassed = FALSE;
				}
				rs->Close();
			}
		}

		// (j.jones 2008-12-03 16:41) - PLID 32305 - check the Provider NPI validation preference
		// (j.jones 2008-12-03 16:41) - PLID 32305 - check the Specialty validation preference
		BOOL bCheckNPI = GetRemotePropertyInt("OHIPValidateProviderNPI",1,0,"<None>",TRUE) == 1;
		BOOL bCheckSpecialty = GetRemotePropertyInt("OHIPValidateSpecialty",1,0,"<None>",TRUE) == 1;
		if(bCheckNPI || bCheckSpecialty) {

			//warn if any Provider's NPI is empty or invalid
			// (j.jones 2009-06-26 09:36) - PLID 34292 - Specialty is now in ProvidersT
			// (j.jones 2011-08-16 17:22) - PLID 44805 - we will filter out "original" and "void" charges
			rs = CreateParamRecordset("SELECT NPI, OHIPSpecialty, Last + ', ' + First + ' ' + Middle AS ProvName "
				"FROM ProvidersT "
				"INNER JOIN PersonT ON ProvidersT.PersonID = PersonT.ID "
				"WHERE PersonID IN ("
				"SELECT Coalesce(ChargesT.ClaimProviderID, ProvidersT.ClaimProviderID) AS ClaimProviderID FROM ProvidersT " 
				"INNER JOIN ChargesT ON ProvidersT.PersonID = ChargesT.DoctorsProviders "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				"WHERE ChargesT.Batched = 1 AND LineItemT.Deleted = 0 "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"AND ChargesT.BillID = {INT})", nBillID);
			while(!rs->eof) {

				CString strNPI = AdoFldString(rs, "NPI","");
				CString strSpecialty = AdoFldString(rs, "OHIPSpecialty","");
				CString strProvName = AdoFldString(rs, "ProvName","");

				if(bCheckNPI) {
					strNPI.TrimRight("");
					if(strNPI.IsEmpty()) {			
						//(e.lally 2009-03-25) PLID 33634 - updated message to reflect dynamic label.
						str.Format("\tERROR - No Provider Number Found\r\n\r\n"
							"\t\tA provider on this bill, '%s', has no ID number filled in.\r\n"
							"\t\tThis number is pulled from the Provider Number field in the Contacts module.\r\n"
							"\t\tPlease ensure that all providers in the Contacts module have 6-digit IDs filled\r\n"
							"\t\tin the Provider Number field.\r\n\r\n\r\n", strProvName);
						strErrors += str;
						bPassed = FALSE;
					}
					//check that it is 6 digits long - numbers only
					else if(strNPI.GetLength() != 6 || (atoi(strNPI) == 0 && strNPI != "000000")) {
						//(e.lally 2009-03-25) PLID 33634 - updated message to reflect dynamic label.
						str.Format("\tERROR - Provider Number Invalid\r\n\r\n"
							"\t\tA provider on this bill, '%s', has an invalid ID number of '%s'\r\n"
							"\t\twhich is not 6 digits long.\r\n"
							"\t\tThis number is pulled from the Provider Number field in the Contacts module.\r\n"
							"\t\tPlease ensure that all providers in the Contacts module have 6-digit IDs filled\r\n"
							"\t\tin the Provider Number field.\r\n\r\n\r\n", strProvName, strNPI);
						strErrors += str;
						bPassed = FALSE;
					}
				}

				// (j.jones 2008-12-03 16:41) - PLID 32305 - check the Specialty validation preference
				//warn if the Specialty Code is empty
				if(bCheckSpecialty) {
					// (j.jones 2009-06-26 09:36) - PLID 34292 - Specialty is now in ProvidersT
					strSpecialty.TrimRight();
					if(strSpecialty.IsEmpty()) {
						str.Format("\tERROR - No Specialty Code Found\r\n\r\n"
							"\t\tA provider on this bill, '%s', has no Specialty code filled in.\r\n"
							"\t\tThis number is pulled from the OHIP Specialty field in the Contacts module.\r\n"
							"\t\tPlease ensure that all providers in the Contacts module have 2-digit codes filled\r\n"
							"\t\tin the OHIP Specialty field.\r\n\r\n\r\n", strProvName);
						strErrors += str;
						bPassed = FALSE;
					}
					//check that it is no more than 2 digits long - numbers only
					//We don't restrict to *only* 2 digits, because if they entered 8, it would export as 08, and that's fine
					else if(strSpecialty.GetLength() > 2 || (atoi(strSpecialty) == 0 && strSpecialty != "00")) {
						str.Format("\tERROR - Specialty Code Invalid\r\n\r\n"
							"\t\tA provider on this bill, '%s', has an invalid Specialty code of '%s'\r\n"
							"\t\twhich must be no more than 2 digits.\r\n"
							"\t\tThis number is pulled from the OHIP Specialty field in the Contacts module.\r\n"
							"\t\tPlease ensure that all providers in the Contacts module have 2-digit codes filled\r\n"
							"\t\tin the OHIP Specialty field.\r\n\r\n\r\n", strProvName, strSpecialty);
						strErrors += str;
						bPassed = FALSE;
					}
				}

				rs->MoveNext();
			}
			rs->Close();
		}
		
		// (j.jones 2008-12-03 16:41) - PLID 32305 - check the MOH Office Code validation preference
		if(GetRemotePropertyInt("OHIPValidateMOHOfficeCode",1,0,"<None>",TRUE) == 1) {
			//warn if the MOH Office Code is empty (from OHIP Properties)
			CString strMOH = GetRemotePropertyText("OHIP_MOHOfficeCode", "N", 0, "<None>", true);
			strMOH.TrimRight();
			if(strMOH.IsEmpty()) {
				str.Format("\tERROR - No MOH Office Code Found\r\n\r\n"
					"\t\tNo MOH Office Code is entered in the OHIP Properties.\r\n\r\n\r\n");
				strErrors += str;
				bPassed = FALSE;
			}
			else if(strMOH.GetLength() != 1) {
				str.Format("\tERROR - MOH Office Code Invalid\r\n\r\n"
					"\t\tThe MOH Office Code that is entered in the OHIP Properties, '%s', is invalid.\r\n"
					"\t\tA MOH Office Code must be only one character in length.\r\n\r\n\r\n", strMOH);
				strErrors += str;
				bPassed = FALSE;
			}
		}
		
		// (j.jones 2008-12-03 16:41) - PLID 32305 - check the Patient Health Number validation preference
		// (j.jones 2009-04-15 15:16) - PLID 33997 - don't bother checking for a health number if RMB
		if(!bIsRMB && GetRemotePropertyInt("OHIPValidatePatientHealthNumber",1,0,"<None>",TRUE) == 1) {
			//check that the patient has a value for Health Number,
			//as specified in OHIP Properties (it is a user-selectable custom field)

			CString strHealthNumber = "";
			long nHealthNumberCustomField = GetRemotePropertyInt("OHIP_HealthNumberCustomField", 1, 0, "<None>", true);
			//this field will correspond to a patient's custom text field
			_RecordsetPtr rsCustom = CreateParamRecordset("SELECT TextParam FROM CustomFieldDataT "
				"WHERE FieldID = {INT} AND PersonID = {INT}", nHealthNumberCustomField, nPatientID);
			if(!rsCustom->eof) {
				strHealthNumber = AdoFldString(rsCustom, "TextParam","");
				strHealthNumber.Replace("-","");
				strHealthNumber.Replace(" ","");
			}
			rsCustom->Close();

			strHealthNumber.TrimLeft();
			strHealthNumber.TrimRight();
			if(strHealthNumber.IsEmpty()) {
				str.Format("\tERROR - No Health Number Found\r\n\r\n"
					"\t\tA Health Number could not be found for this patient.\r\n"
					"\t\tEnsure that a Health Number custom field is selected in the OHIP Properties,\r\n"
					"\t\tand that this patient has a value entered on their account for the selected\r\n"
					"\t\tcustom field.\r\n\r\n\r\n");
				strErrors += str;
				bPassed = FALSE;
			}
			//like the location codes, this number is almost always a number, but the claim allows
			//alphanumerics, so we will warn using the word digits, but we won't actually enforce digits only
			else if(strHealthNumber.GetLength() != 10) {
				str.Format("\tERROR - Health Number Invalid\r\n\r\n"
					"\t\tThe Health Number for this patient, '%s', is not 10 digits in length.\r\n"
					"\t\tYou must correct the Health Number for this patient in the appropriate\r\n"
					"\t\tcustom field.\r\n\r\n\r\n", strHealthNumber);
				strErrors += str;
				bPassed = FALSE;
			}
			// (j.jones 2008-12-10 11:22) - PLID 32312 - added patient health number verification
			else if(GetRemotePropertyInt("OHIPValidatePatientHealthNumberVerify",1,0,"<None>",TRUE) == 1) {
				//We won't attempt to verify the validity of the health number unless it is present
				//and 10 characters long. Only now do we require it to be a number.
				if(!ValidateOHIPPatientHealthNumber(strHealthNumber)) {
					str.Format("\tERROR - Health Number Invalid\r\n\r\n"
						"\t\tThe Health Number for this patient, '%s', is an invalid Health Number.\r\n"
						"\t\tYou must correct the Health Number for this patient in the appropriate\r\n"
						"\t\tcustom field.\r\n\r\n\r\n", strHealthNumber);
					strErrors += str;
					bPassed = FALSE;
				}
			}
		}

		// (j.jones 2008-12-03 16:41) - PLID 32305 - check the Patient Health Number validation preference, which defaults to off
		// (j.jones 2009-04-15 15:16) - PLID 33997 - don't bother checking for a version code if RMB
		if(!bIsRMB && GetRemotePropertyInt("OHIPValidatePatientVersionCode",0,0,"<None>",TRUE) == 1) {
			//check that the patient has a value for Version Code
			//as specified in OHIP Properties (it is a user-selectable custom field)

			CString strVersionCode = "";
			long nVersionCodeCustomField = GetRemotePropertyInt("OHIP_VersionCodeCustomField", 2, 0, "<None>", true);
			//this field will correspond to a patient's custom text field
			_RecordsetPtr rsCustom = CreateParamRecordset("SELECT TextParam FROM CustomFieldDataT "
				"WHERE FieldID = {INT} AND PersonID = {INT}", nVersionCodeCustomField, nPatientID);
			if(!rsCustom->eof) {
				strVersionCode = AdoFldString(rsCustom, "TextParam","");
			}
			rsCustom->Close();

			strVersionCode.TrimRight();
			// (j.jones 2010-02-01 13:26) - PLID 34934 - It is possible, albeit not common,
			// for an empty or 1-character version code to be valid. So in these cases, warn,
			// such that they are reminded to confirm this, but it isn't an outright error.
			if(strVersionCode.IsEmpty()) {
				str.Format("\tWARNING - No Version Code Found\r\n\r\n"
					"\t\tA Version Code has not been entered for this patient.\r\n"
					"\t\tPlease verify whether this patient needs a Version Code,\r\n"
					"\t\tand enter it in the appropriate custom field.\r\n\r\n\r\n");
				strErrors += str;
				bWarnings = TRUE;
			}
			else if(strVersionCode.GetLength() == 1) {
				str.Format("\tWARNING - Version Code Is Only One Character\r\n\r\n"
					"\t\tThe Version Code for this patient, '%s', is only one character in length.\r\n"
					"\t\tPlease ensure that this Version Code is correct for this patient in the\r\n"
					"\t\tappropriate custom field.\r\n\r\n\r\n", strVersionCode);
				strErrors += str;
				bWarnings = TRUE;
			}
			//do not use the word 'digits' in this warning, it is almost always text
			else if(strVersionCode.GetLength() > 2) {
				str.Format("\tERROR - Version Code Is Too Long\r\n\r\n"
					"\t\tThe Version Code for this patient, '%s', is over 2 characters in length.\r\n"
					"\t\tYou must correct the Version Code for this patient in the appropriate\r\n"
					"\t\tcustom field.\r\n\r\n\r\n", strVersionCode);
				strErrors += str;
				bPassed = FALSE;
			}
		}

		// (j.jones 2008-12-03 17:43) - PLID 32305 - check the Patient Registration Number validation preference
		// (j.jones 2009-04-15 15:16) - PLID 33997 - only check for this if it is RMB
		if(bIsRMB && GetRemotePropertyInt("OHIPValidateRegistrationNumber",1,0,"<None>",TRUE) == 1) {
			//check that the patient has a value for Registration Number,
			//as specified in OHIP Properties (it is a user-selectable custom field)

			CString strRegistrationNumber = "";
			long nRegistrationNumberCustomField = GetRemotePropertyInt("OHIP_RegistrationNumberCustomField", 1, 0, "<None>", true);
			//this field will correspond to a patient's custom text field
			_RecordsetPtr rsCustom = CreateParamRecordset("SELECT TextParam FROM CustomFieldDataT "
				"WHERE FieldID = {INT} AND PersonID = {INT}", nRegistrationNumberCustomField, nPatientID);
			if(!rsCustom->eof) {
				strRegistrationNumber = AdoFldString(rsCustom, "TextParam","");
			}
			rsCustom->Close();

			strRegistrationNumber.TrimRight();
			if(strRegistrationNumber.IsEmpty()) {
				//this field is an error if not RMB				
				//if(bIsRMB) {
					str.Format("\tERROR - No Registration Number Found\r\n\r\n"
						"\t\tA Registration Number could not be found for this patient.\r\n"
						"\t\tEnsure that a Registration Number custom field is selected in the OHIP Properties,\r\n"
						"\t\tand that this patient has a value entered on their account for the selected\r\n"
						"\t\tcustom field.\r\n"
						"\t\tBecause this is a RMB claim, this field is required.\r\n\r\n\r\n");
					strErrors += str;
					bPassed = FALSE;
				//}
				// (j.jones 2009-04-15 15:16) - PLID 33997 - don't even bother checking the registration
				// number at all if not RMB
				/*
				else {
					str.Format("\tWARNING - No Registration Number Found\r\n\r\n"
						"\t\tA Registration Number could not be found for this patient.\r\n"
						"\t\tEnsure that a Registration Number custom field is selected in the OHIP Properties,\r\n"
						"\t\tand that this patient has a value entered on their account for the selected\r\n"
						"\t\tcustom field.\r\n"
						"\t\tBecause this is not a RMB claim, this field is not required.\r\n\r\n\r\n");
					strErrors += str;
					bWarnings = TRUE;
				}
				*/
			}
		}
	
		if(bPassed && !bWarnings){
			strErrors = "";
		}

		m_strInvalidClaims += strErrors;

		int Val;
		if(bPassed && !bWarnings)
			//the claim has no errors, and no warnings
			Val = 1;
		else if(bPassed && bWarnings)
			//the claim had no critical errors, but there are warnings
			Val = 3;
		else if(!bPassed)
			//the claim failed with definite errors
			Val = 2;
		else
			//this should not be possible
			Val = 0;

		ExecuteParamSql("UPDATE HCFATrackT SET ValidationState = {INT} WHERE ID = {INT}",Val,nClaimID);

		return (bPassed && !bWarnings);

	}NxCatchAll("Error validating claim.");

	ExecuteParamSql("UPDATE HCFATrackT SET ValidationState = 0 WHERE ID = {INT}",nClaimID);

	return FALSE;
}

// (j.jones 2008-12-10 11:53) - PLID 32312 - OHIP Health Numbers are 10 digit numbers,
// where the last digit is a check digit, so let's validate that check digit
BOOL CEbillingValidationDlg::ValidateOHIPPatientHealthNumber(CString strHealthNumber) {

	//pass any exceptions to the caller

	//remove all non-numerics
	StripNonNumericChars(strHealthNumber);

	//if the result is not 10 characters, quit now
	if(strHealthNumber.GetLength() != 10) {
		//must be 10 numbers
		return FALSE;
	}

	//Now perform the OHIP MOD 10 Check Digit routine.
	//For this, weu take the in the first 9 digits, double digits 1,3,5,7,9.
	//If the result is 2 digits you add them together, i.e. 9 x 2 is 18, add that together and you get 9.
	//Add all the results together, then take the rightmost number, subtract from ten, and that's the check digit.
	
	//An example given is 9876543217. 7 is the check digit.
	//Taking the remaining numbers, they calculate out to be:
	//(1+8) + 8 + (1 + 4) + 6 + (1 + 0 ) + 4 + 6 + 2 + 2 = 43.
	//Take the 3, subtract if from 10, you get 7. 7 is the check digit.

	long nTotal = 0;
	int i=0;
	for(i=0; i<9; i++) {
		long nNumber = atoi(CString(strHealthNumber.GetAt(i)));
		//double the numbers at the odd indices (1,3,5,7,9)
		if((i+1)%2 == 1) {
			nNumber *= 2;
			//if greater than or equal to 10, subtract 9,
			//which is the same thing as adding the 1 and 0 together, etc.
			if(nNumber >= 10) {
				nNumber -= 9;
			}
		}
		//add the number to our total
		nTotal += nNumber;
	}

	//now get the rightmost digit of our total
	CString strTotal;
	strTotal.Format("%li", nTotal);
	long nRightMostTotalNum = atoi(strTotal.Right(1));
	
	//subtract that number from 10, result must be 1 digit
	long nEstimatedCheckDigit = 10 - nRightMostTotalNum;
	if(nEstimatedCheckDigit == 10) {
		nEstimatedCheckDigit = 0;
	}

	//get the actual check digit
	long nActualCheckDigit = atoi(strHealthNumber.Right(1));

	//return their equality
	return nEstimatedCheckDigit == nActualCheckDigit;
}

// (j.jones 2010-04-14 11:39) - PLID 27457 - call one function to validate all ANSI loops
// for a given provider ID
// (j.jones 2010-04-16 12:00) - PLID 38225 - added bWarnings and bDisableREFXX
// (j.jones 2010-11-11 09:43) - PLID 41396 - added a parameter to validate billing and/or rendering provider IDs
void CEbillingValidationDlg::ValidateAllANSIProviderREFIDs(BOOL bIsUBClaim, BOOL bIsRefPhy,
						long nProviderID, BOOL bValidateBillingProvIDs, BOOL bValidateRenderingProvIDs,
						long InsuranceCoID, long nLocationID, long nInsuredPartyID,
						long HCFASetupGroupID, CString strDefaultBox33GRP, long nBox33Setup,
						long UB92SetupGroupID, CString strUB04Box76Qual, long nBox82Num, long nBox82Setup,
						CString &strErrors, BOOL &bPassed, BOOL &bWarnings, BOOL bDisableREFXX,
						CString strProviderLast, CString strProviderFirst,
						BOOL bSend2010AB, BOOL bSend2420A,
						long nExtraREF_IDType, BOOL bIsGroup, long nHCFABox25,
						CString strLocationEIN, CString strProvEIN, CString strProvSSN,
						BOOL bUse_Addnl_2010AA, CString strAddnl_2010AA_Qual, CString strAddnl_2010AA)
{
	if(nProviderID == -1) {
		return;
	}

	//validate all possible IDs

	CString strID = "", strQualifier = "", strLoadedFrom = "";

	if(!bIsRefPhy) {

		// (j.jones 2010-11-11 09:55) - PLID 41396 - only validate these if the bValidateBillingProvIDs
		// field is enabled
		if(bValidateBillingProvIDs) { 

			//2010AA
			strID = "", strQualifier = "", strLoadedFrom = "";

			// (j.jones 2012-01-06 08:47) - PLID 47336 - in 5010, there is only one REF segment for UB claims,
			// and the only override is the Adv. Ebilling Setup option, so just validate that
			if(m_avANSIVersion == av5010 && bIsUBClaim) {
				_RecordsetPtr rsOver = CreateParamRecordset("SELECT ANSI_2010AA AS IDNum, ANSI_2010AA_Qual AS Qual "
					"FROM UB92EbillingSetupT "
					"WHERE Use_2010AA = 1 AND SetupGroupID = {INT} AND ProviderID = {INT} AND LocationID = {INT}",
					UB92SetupGroupID, nProviderID, nLocationID);
				if(!rsOver->eof) {
					//this allows blank values,  so we must accept whatever is in the data, albeit with spaces trimmed
					strID = AdoFldString(rsOver, "IDNum", "");
					strID.TrimLeft();
					strID.TrimRight();
					strQualifier = AdoFldString(rsOver, "Qual", "");
					strQualifier.TrimLeft();
					strQualifier.TrimRight();
					strLoadedFrom = "Adv. Ebilling Setup 2010AA Override";
				}
				rsOver->Close();
			}
			else {
				//all other variations need the full 2010AA REF override check
				EBilling_Calculate2010_REF(FALSE, strQualifier, strID, strLoadedFrom, bIsUBClaim,
					nProviderID, InsuranceCoID, nLocationID, nInsuredPartyID,
					HCFASetupGroupID, strDefaultBox33GRP,
					UB92SetupGroupID, strUB04Box76Qual, nBox82Num, nBox82Setup);
			}

			CheckANSIProviderREFID(strQualifier, strID, strLoadedFrom, "2010AA",
				bIsUBClaim, strErrors, bPassed, bWarnings, bDisableREFXX,
				strProviderLast, strProviderFirst, TRUE);

			// (j.jones 2010-10-19 17:40) - PLID 40936 - 2010AB has no IDs in 5010, so only validate when 4010
			if(bSend2010AB && m_avANSIVersion == av4010) {
				//2010AB
				strID = "", strQualifier = "", strLoadedFrom = "";
				EBilling_Calculate2010_REF(TRUE, strQualifier, strID, strLoadedFrom, bIsUBClaim,
					nProviderID, InsuranceCoID, nLocationID, nInsuredPartyID,
					HCFASetupGroupID, strDefaultBox33GRP,
					UB92SetupGroupID, strUB04Box76Qual, nBox82Num, nBox82Setup);

				CheckANSIProviderREFID(strQualifier, strID, strLoadedFrom, "2010AB",
					bIsUBClaim, strErrors, bPassed, bWarnings, bDisableREFXX,
					strProviderLast, strProviderFirst, FALSE);
			}
		}

		// (j.jones 2010-11-11 09:55) - PLID 41396 - only validate these if the bValidateRenderingProvIDs
		// field is enabled
		if(bValidateRenderingProvIDs) { 

			if(!bIsUBClaim) {
				//2310B
				strID = "", strQualifier = "", strLoadedFrom = "";
				EBilling_Calculate2310B_REF(strQualifier, strID, strLoadedFrom,
					nProviderID, HCFASetupGroupID, InsuranceCoID, nInsuredPartyID, nLocationID);

				CheckANSIProviderREFID(strQualifier, strID, strLoadedFrom, "2310B",
					bIsUBClaim, strErrors, bPassed, bWarnings, bDisableREFXX,
					strProviderLast, strProviderFirst, FALSE);

				if(bSend2420A) {
					//2420A
					strID = "", strQualifier = "", strLoadedFrom = "";
					EBilling_Calculate2420A_REF(strQualifier, strID, strLoadedFrom,
						nProviderID, HCFASetupGroupID, InsuranceCoID, nInsuredPartyID, nLocationID);

					CheckANSIProviderREFID(strQualifier, strID, strLoadedFrom, "2420A",
						bIsUBClaim, strErrors, bPassed, bWarnings, bDisableREFXX,
						strProviderLast, strProviderFirst, FALSE);
				}
			}
			else {
				//2310A
				strID = "", strQualifier = "", strLoadedFrom = "";
				EBilling_Calculate2310A_REF(strQualifier, strID, strLoadedFrom,
					nProviderID, UB92SetupGroupID, strUB04Box76Qual, nBox82Num, nBox82Setup, 
					InsuranceCoID, nInsuredPartyID, nLocationID);

				CheckANSIProviderREFID(strQualifier, strID, strLoadedFrom, "2310A",
					bIsUBClaim, strErrors, bPassed, bWarnings, bDisableREFXX,
					strProviderLast, strProviderFirst, FALSE);
			}
		}
	}

	//next, validate the "Additional REF" setup from the Adv. Ebilling Setup
	//0 - EIN/SSN, 1 - NPI, 2 - none
	if(nExtraREF_IDType != 2) {
		
		strID = "", strQualifier = "";

		if(nExtraREF_IDType == 0) {
			//EIN/SSN

			//"EI" for EIN, "SY" for SSN - from HCFASetupT.Box25

			//If a group, show the location's EIN, ID of EI")

			// (j.jones 2007-07-10 15:36) - PLID 26458 - supported Bill Location (m_HCFAInfo.Box33Setup is 4)
			if(bIsGroup || bIsUBClaim || nBox33Setup == 4) {
				//a group
				StripNonNumericChars(strLocationEIN);
				if(strLocationEIN.IsEmpty()) {
					CString str;
					str.Format("\tERROR - Additional REF Segment is incomplete\r\n\r\n"
						"\t\tThe HCFA Setup is configured to send an Additional REF Segment\r\n"
						"\t\twith the Location's EIN, but no EIN has been configured for\r\n"
						"\t\tthis bill's location.\r\n"
						"\t\tPlease correct the Location's EIN or review your Adv. Ebilling Setup\r\n"
						"\t\tfor the HCFA group before submitting.\r\n\r\n\r\n");
					if(bIsUBClaim) {
						str.Replace("HCFA", "UB");
					}
					strErrors += str;
					bPassed = FALSE;
				}
			}
			
			//we'll always send the doctor in 2310B / 2310A on UB
			if(!bIsUBClaim && nHCFABox25 == 1) {
				StripNonNumericChars(strProvSSN);
				if(strProvSSN.IsEmpty()) {
					CString str;
					str.Format("\tERROR - Additional REF Segment is incomplete\r\n\r\n"
						"\t\tThe HCFA Setup is configured to send an Additional REF Segment\r\n"
						"\t\twith the provider's Social Security Number, this field is empty\r\n"
						"\t\tfor the provider '%s %s'.\r\n"
						"\t\tPlease correct the provider's Social Security Number or review\r\n"
						"\t\tyour Adv. Ebilling Setup for the HCFA group before submitting.\r\n\r\n\r\n", strProviderFirst, strProviderLast);					
					strErrors += str;
					bPassed = FALSE;
				}
			}
			else {
				StripNonNumericChars(strProvEIN);
				if(strProvEIN.IsEmpty()) {
					CString str;
					//mention both EIN and Federal Tax ID, because the setup says EIN
					//but the contacts module says Tax ID
					str.Format("\tERROR - Additional REF Segment is incomplete\r\n\r\n"
						"\t\tThe HCFA Setup is configured to send an Additional REF Segment\r\n"
						"\t\twith the provider's EIN, this field is empty for the provider '%s %s'.\r\n"
						"\t\tPlease correct the provider's Federal Tax ID or review your\r\n"
						"\t\tAdv. Ebilling Setup for the HCFA group before submitting.\r\n\r\n\r\n", strProviderFirst, strProviderLast);
					if(bIsUBClaim) {
						str.Replace("HCFA", "UB");

						//this can optionally be a referring physician on the UB claim
						if(bIsRefPhy) {
							str.Replace("provider", "referring physician");
						}
					}
					strErrors += str;
					bPassed = FALSE;
				}
			}
		}
		else if(nExtraREF_IDType == 1) {
			//NPI
			// (j.jones 2010-04-15 17:10) - PLID 38149 - if this option is selected at all, warn,
			// because they should not be using this anymore

			// (j.jones 2010-04-16 11:59) - PLID 38225 - if REF*XX is disabled, make this a warning,
			// otherwise it is an error
			if(bDisableREFXX) {
				//they won't actually be sending the REF*XX if they export

				CString str;
				str.Format("\tWARNING - Additional REF Segment cannot be NPI\r\n\r\n"
					"\t\tThe HCFA Setup is configured to send an Additional REF Segment\r\n"
					"\t\twith an NPI, but sending an NPI in the REF segment is no longer valid.\r\n"
					"\t\tYour preferences are configured to not send this invalid REF*XX segment,\r\n"
					"\t\tbut you should change the Additional REF Segment to 'Do Not Add' or 'EIN/SSN'\r\n"
					"\t\tin your Adv. Ebilling Setup for the HCFA group before submitting.\r\n\r\n\r\n");

				if(bIsUBClaim) {
					str.Replace("HCFA", "UB");
				}

				strErrors += str;
				bWarnings = TRUE;
			}
			else {
				//they will be sending the REF*XX if they export

				CString str;
				str.Format("\tERROR - Additional REF Segment cannot be NPI\r\n\r\n"
					"\t\tThe HCFA Setup is configured to send an Additional REF Segment\r\n"
					"\t\twith an NPI, but sending an NPI in the REF segment is no longer valid.\r\n"
					"\t\tPlease change the Additional REF Segment to 'Do Not Add' or 'EIN/SSN'\r\n"
					"\t\tin your Adv. Ebilling Setup for the HCFA group before submitting.\r\n\r\n\r\n");

				if(bIsUBClaim) {
					str.Replace("HCFA", "UB");
				}

				strErrors += str;
				bPassed = FALSE;
			}
		}
	}

	//now validate the ANSI Properties override, if we have one

	// (j.jones 2010-11-11 09:55) - PLID 41396 - only validate this if the bValidateBillingProvIDs
	// field is enabled
	if(bValidateBillingProvIDs) { 

		// (j.jones 2010-09-01 08:44) - PLID 40338 - fixed a poor if statement
		if(bUse_Addnl_2010AA && (strAddnl_2010AA_Qual.IsEmpty() || strAddnl_2010AA.IsEmpty())) {

			CString str;
			str.Format("\tERROR - Additional 2010AA ID is incomplete\r\n\r\n"
				"\t\tThe ANSI Properties setup is configured to send an\r\n"
				"\t\tadditional 2010AA REF ID, but is incomplete.\r\n"
				"\t\tThis field must have both a qualifier and an ID filled in.\r\n"
				"\t\tPlease review your ANSI Properties and correct this issue\r\n"
				"\t\tbefore submitting.\r\n\r\n\r\n");
			strErrors += str;
			bPassed = FALSE;
		}
		else if(bUse_Addnl_2010AA && strAddnl_2010AA_Qual.CompareNoCase("XX") == 0) {
			// (j.jones 2010-04-15 17:20) - PLID 38149 - warn if XX

			// (j.jones 2010-04-16 11:59) - PLID 38225 - if REF*XX is disabled, make this a warning,
			// otherwise it is an error
			if(bDisableREFXX) {
				//they won't actually be sending the REF*XX if they export

				CString str;
				str.Format("\tWARNING - Additional 2010AA ID cannot have an XX qualifier\r\n\r\n"
					"\t\tThe ANSI Properties setup is configured to send an XX qualifier in the\r\n"
					"\t\tadditional 2010AA REF ID, but sending an XX in the REF segment is no\r\n"
					"\t\tlonger valid.\r\n"
					"\t\tYour preferences are configured to not send this invalid REF*XX segment,\r\n"
					"\t\tbut you should review your ANSI Properties and correct this issue\r\n"
					"\t\tbefore submitting.\r\n\r\n\r\n");

				strErrors += str;
				bWarnings = TRUE;
			}
			else {
				//they will be sending the REF*XX if they export

				CString str;
				str.Format("\tERROR - Additional 2010AA ID cannot have an XX qualifier\r\n\r\n"
					"\t\tThe ANSI Properties setup is configured to send an XX qualifier in the\r\n"
					"\t\tadditional 2010AA REF ID, but sending an XX in the REF segment is no\r\n"
					"\t\tlonger valid.\r\n"
					"\t\tPlease review your ANSI Properties and correct this issue\r\n"
					"\t\tbefore submitting.\r\n\r\n\r\n");

				strErrors += str;
				bPassed = FALSE;
			}
		}
		// (j.jones 2010-10-20 11:21) - PLID 40396 - if 5010 only, we validate against a list
		// of permissible qualifiers aside from XX
		else if(m_avANSIVersion == av5010 && bUse_Addnl_2010AA && !strAddnl_2010AA_Qual.IsEmpty()) {

			//the only valid qualifiers in 5010 are:
			//0B, 1G, G2, LU
			//EI, SY, and G5 are also allowed in in 2010AA, which is all this setting applies to
			if(strAddnl_2010AA_Qual.CompareNoCase("0B") != 0
				&& strAddnl_2010AA_Qual.CompareNoCase("1G") != 0
				&& strAddnl_2010AA_Qual.CompareNoCase("G2") != 0
				&& strAddnl_2010AA_Qual.CompareNoCase("LU") != 0
				&& strAddnl_2010AA_Qual.CompareNoCase("EI") != 0
				&& strAddnl_2010AA_Qual.CompareNoCase("SY") != 0
				&& strAddnl_2010AA_Qual.CompareNoCase("G5") != 0
				) {

				CString str;
				str.Format("\tERROR - Additional 2010AA ID cannot be a %s qualifier\r\n\r\n"
					"\t\tThe ANSI Properties setup is configured to send a %s qualifier in the\r\n"
					"\t\tadditional 2010AA REF ID, but sending a %s in the REF segment is no\r\n"
					"\t\tlonger valid.\r\n"
					"\t\tPlease review your ANSI Properties and correct this issue\r\n"
					"\t\tbefore submitting.\r\n\r\n\r\n", strAddnl_2010AA_Qual, strAddnl_2010AA_Qual, strAddnl_2010AA_Qual);
				if(bIsUBClaim) {
					str.Replace("HCFA", "UB");
				}
				strErrors += str;
				bPassed = FALSE;
			}
		}
	}
}

// (j.jones 2010-04-14 11:23) - PLID 27457 - this function will calculate
// if a loaded ANSI provider REF ID is invalid, and if so, warn about it
// (j.jones 2010-04-16 12:00) - PLID 38225 - added bWarnings and bDisableREFXX
// (j.jones 2010-10-20 11:13) - PLID 40396 - added bIs2010AA, which permits EI, SY, and G5 as valid qualifiers in 5010,
// otherwise they are illegal in 5010
void CEbillingValidationDlg::CheckANSIProviderREFID(CString strQualifier, CString strID, CString strLoadedFrom, CString strLoopID,
													   BOOL bIsUBClaim, CString &strErrors, BOOL &bPassed, BOOL &bWarnings, BOOL bDisableREFXX,
													   CString strProviderLast, CString strProviderFirst, BOOL bIs2010AA)
{
	strQualifier.TrimLeft();
	strQualifier.TrimRight();
	strID.TrimLeft();
	strID.TrimRight();

	CString strHCFA;
	strHCFA.Format("%s", bIsUBClaim ? "UB" : "HCFA");

	//both can be blank, but you can't have one without the other
	if(strQualifier.IsEmpty() != strID.IsEmpty()) {
	
		CString str;
		// (j.jones 2010-11-03 13:20) - PLID 41309 - be specific and state if the qualifier or ID is missing
		if(strQualifier.IsEmpty()) {
			str.Format("\tERROR - Provider ID is missing a qualifier for Loop %s\r\n\r\n"
				"\t\tThe provider's claim provider, '%s %s',\r\n"
				"\t\tfor this patient's account does not have a complete ID configured for ANSI Loop %s.\r\n"
				"\t\tAll IDs sent in the ANSI format require either both a qualifier and an ID filled in,\r\n"
				"\t\tor both blank to not send an REF ID.\r\n"
				"\t\tThe ID for Loop %s is currently configured to be loaded\r\n"
				"\t\tfrom the %s in the %s Setup of the Administrator module.\r\n"
				"\t\tThere is an ID of %s configured to be sent, but no qualifier is configured to be sent with it.\r\n"
				"\t\tPlease review your setup and correct this issue before submitting.\r\n\r\n\r\n",
				strLoopID, strProviderFirst, strProviderLast, strLoopID, strLoopID, strLoadedFrom, strHCFA, strID);
		}
		else if(strID.IsEmpty()) {
			//the odds of having a qualifier and no ID are very, very slim, but it's possible
			str.Format("\tERROR - Provider ID has a qualifier but no ID for Loop %s\r\n\r\n"
				"\t\tThe provider's claim provider, '%s %s',\r\n"
				"\t\tfor this patient's account does not have a complete ID configured for ANSI Loop %s.\r\n"
				"\t\tAll IDs sent in the ANSI format require either both a qualifier and an ID filled in,\r\n"
				"\t\tor both blank to not send an REF ID.\r\n"
				"\t\tThe ID for Loop %s is currently configured to be loaded\r\n"
				"\t\tfrom the %s in the %s Setup of the Administrator module.\r\n"
				"\t\tThere is a qualifier of %s configured to be sent, but no ID is configured to be sent with it.\r\n"
				"\t\tPlease review your setup and correct this issue before submitting.\r\n\r\n\r\n",
				strLoopID, strProviderFirst, strProviderLast, strLoopID, strLoopID, strLoadedFrom, strHCFA, strQualifier);
		}
		if(bIsUBClaim) {
			str.Replace("HCFA", "UB");
		}
		strErrors += str;
		bPassed = FALSE;
	}	
	else if(strQualifier.CompareNoCase("XX") == 0) {
		// (j.jones 2010-04-15 17:20) - PLID 38149 - warn if XX

		// (j.jones 2010-04-16 11:59) - PLID 38225 - if REF*XX is disabled, make this a warning,
		// otherwise it is an error
		if(bDisableREFXX) {
			//they won't actually be sending the REF*XX if they export

			CString str;
			str.Format("\tWARNING - Provider ID cannot use XX qualifier in Loop %s\r\n\r\n"
				"\t\tThe provider's claim provider, '%s %s',\r\n"
				"\t\tfor this patient's account has an invalid ID qualifier of XX\r\n"
				"\t\tconfigured for ANSI Loop %s. Sending an XX in the REF segment\r\n"
				"\t\tis no longer a valid qualifier.\r\n"
				"\t\tThe ID for Loop %s is currently configured to be loaded\r\n"
				"\t\tfrom the %s in the %s Setup\r\n"
				"\t\tof the Administrator module.\r\n"
				"\t\tYour preferences are configured to not send this invalid REF*XX segment,\r\n"
				"\t\tbut you should review your setup and correct this issue before submitting.\r\n\r\n\r\n",
				strLoopID, strProviderFirst, strProviderLast, strLoopID, strLoopID, strLoadedFrom, strHCFA);
			if(bIsUBClaim) {
				str.Replace("HCFA", "UB");
			}
			strErrors += str;
			bWarnings = TRUE;
		}
		else {
			//they will be sending the REF*XX if they export
			CString str;
			str.Format("\tERROR - Provider ID cannot use XX qualifier in Loop %s\r\n\r\n"
				"\t\tThe provider's claim provider, '%s %s',\r\n"
				"\t\tfor this patient's account has an invalid ID qualifier of XX\r\n"
				"\t\tconfigured for ANSI Loop %s. Sending an XX in the REF segment\r\n"
				"\t\tis no longer a valid qualifier.\r\n"
				"\t\tThe ID for Loop %s is currently configured to be loaded\r\n"
				"\t\tfrom the %s in the %s Setup\r\n"
				"\t\tof the Administrator module.\r\n"
				"\t\tPlease review your setup and correct this issue before submitting.\r\n\r\n\r\n",
				strLoopID, strProviderFirst, strProviderLast, strLoopID, strLoopID, strLoadedFrom, strHCFA);
			if(bIsUBClaim) {
				str.Replace("HCFA", "UB");
			}
			strErrors += str;
			bPassed = FALSE;
		}
	}
	// (j.jones 2010-10-20 11:21) - PLID 40396 - if 5010 only, we validate against a list
	// of permissible qualifiers aside from XX
	else if(m_avANSIVersion == av5010 && !strQualifier.IsEmpty()) {

		//the only valid qualifiers in 5010 are:
		//0B, 1G, G2, LU
		//EI, SY, and G5 are also allowed but only in 2010AA

		// (j.jones 2012-01-06 09:22) - PLID 47336 - cleaned up this code for validating the 2010AA-only items
		BOOL bIsValidQualifier = (strQualifier.CompareNoCase("0B") == 0 || strQualifier.CompareNoCase("1G") == 0
								|| strQualifier.CompareNoCase("G2") == 0 || strQualifier.CompareNoCase("LU") == 0);
		BOOL bIsValidFor2010AAOnly = (strQualifier.CompareNoCase("EI") == 0 || strQualifier.CompareNoCase("SY") == 0 || strQualifier.CompareNoCase("G5") == 0);

		if(bIsValidQualifier || (bIs2010AA && bIsValidFor2010AAOnly)) {
			//this qualifier is either valid for all REF segments,
			//or it is valid for 2010AA and this is the 2010AA loop
			return;
		}

		//if we're still here, the qualifier is invalid
		CString str;
		str.Format("\tERROR - Provider ID cannot use %s qualifier in Loop %s\r\n\r\n"
			"\t\tThe provider's claim provider, '%s %s',\r\n"
			"\t\tfor this patient's account has an invalid ID qualifier of %s\r\n"
			"\t\tconfigured for ANSI Loop %s. Sending an %s in the REF segment\r\n"
			"\t\tis no longer a valid qualifier.\r\n"
			"\t\tThe ID for Loop %s is currently configured to be loaded\r\n"
			"\t\tfrom the %s in the %s Setup\r\n"
			"\t\tof the Administrator module.\r\n"
			"\t\tPlease review your setup and correct this issue before submitting.\r\n\r\n\r\n",
			strQualifier, strLoopID,
			strProviderFirst, strProviderLast, strQualifier,
			strLoopID, strQualifier,
			strLoopID, strLoadedFrom, strHCFA);
		if(bIsUBClaim) {
			str.Replace("HCFA", "UB");
		}
		strErrors += str;
		bPassed = FALSE;
	}
}

// (j.jones 2010-11-01 17:51) - PLID 39594 - added Alberta-specific validation function
BOOL CEbillingValidationDlg::ValidateOneAlberta(long nClaimID)
{
	try {

		_RecordsetPtr rs;
		_variant_t var;
		BOOL bPassed = TRUE;
		BOOL bWarnings = FALSE;

		BOOL bHasInsurance = TRUE;

		CString str, strErrors = "";

		long InsuranceCoID = -1;		
		long nPOSID = -1;
		CString strInsCoName = "";

		long nBillID = -1;
		long nPatientID = -1;

		//output the header, and while doing so, check our basic information on the claim
		// (j.jones 2011-08-16 17:22) - PLID 44805 - we will filter out "original" and "void" charges
		// (j.jones 2011-09-23 16:14) - PLID 39377 - added AlreadyExportedToday
		//(r.wilson 10/2/2012) plid 52970 - Replace hardcoded SendType with enumerated value
		rs = CreateParamRecordset("SELECT TOP 1 BillsT.ID AS BillID, "
			"PersonT.[Last] AS PatLast, PersonT.[First] AS PatFirst, "
			"PersonT.[Last] + ', ' + PersonT.[First] + ' ' + PersonT.Middle AS PatientName, "
			"PersonT.BirthDate, PersonT.Gender, PersonT.State, "
			"BillsT.Description, BillsT.Date, PersonT.ID AS PersonID, InsuranceCoT.PersonID AS InsCoID, "
			"InsuranceCoT.Name AS InsCoName, PlaceOfServiceT.ID AS POSID, PlaceOfServiceT.NPI AS POSNPI, "
			"LocationsT.NPI AS LocNPI, InsurancePlansT.PlanName, "
			"ReferringPhysT.PersonID AS RefPhyID, ReferringPhysT.NPI AS RefPhyNPI, "
			"ReferringPhysPersonT.[Last] + ', ' + ReferringPhysPersonT.[First] + ' ' + ReferringPhysPersonT.Middle AS RefPhyName, "
			"ConverT(bit, CASE WHEN AlreadyExportedTodayQ.BillID Is Null THEN 0 ELSE 1 END) AS AlreadyExportedToday "
			"FROM HCFATrackT "
			"INNER JOIN BillsT ON HCFATrackT.BillID = BillsT.ID "
			"INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"LEFT JOIN PatientsT ON BillsT.PatientID = PatientsT.PersonID "
			"LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			"LEFT JOIN InsuredPartyT ON BillsT.InsuredPartyID = InsuredPartyT.PersonID "
			"LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
			"LEFT JOIN InsurancePlansT ON InsuredPartyT.InsPlan = InsurancePlansT.ID "
			"LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
			"LEFT JOIN LocationsT AS PlaceOfServiceT ON BillsT.Location = PlaceOfServiceT.ID "
			"LEFT JOIN ReferringPhysT ON BillsT.RefPhyID = ReferringPhysT.PersonID "
			"LEFT JOIN PersonT ReferringPhysPersonT ON ReferringPhysT.PersonID = ReferringPhysPersonT.ID "
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
			"LEFT JOIN (SELECT BillID FROM ClaimHistoryT "
			//(r.wilson 10/2/2012) plid 52970 - below the old SendType value was 0
			"	WHERE SendType = {INT} AND dbo.AsDateNoTime(Date) = dbo.AsDateNoTime(GetDate()) "
			"	GROUP BY BillID) AS AlreadyExportedTodayQ ON BillsT.ID = AlreadyExportedTodayQ.BillID "
			"WHERE BillsT.Deleted = 0 AND LineItemT.Deleted = 0 AND ChargesT.Batched = 1 "
			"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
			"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
			"AND HCFATrackT.ID = {INT}", ClaimSendType::Electronic ,nClaimID);

		if(rs->eof) {

			//Bill - this is NOT a warning that can be disabled

			str.Format("\tERROR - No Valid Bill Was Found (Claim ID: %li)\r\n\r\n"
				"\t\tEither all the charges have been deleted off of this bill,\r\n"
				"\t\tall the charges have been unbatched from this bill,\r\n"
				"\t\tor this bill no longer exists.", nClaimID);
			strErrors += str;
			bPassed = FALSE;

			//return now, we can't check anything else
			m_strInvalidClaims += strErrors;

			ExecuteParamSql("UPDATE HCFATrackT SET ValidationState = 2 WHERE ID = {INT}", nClaimID);

			return FALSE;
		}
		else {

			nBillID = AdoFldLong(rs, "BillID",-1);

			nPatientID = AdoFldLong(rs, "PersonID",-1);

			//Patient - this is NOT a warning that can be disabled
			if(nPatientID == -1) {				
				str.Format("\tERROR - No Patient Was Found (Claim ID: %li)\r\n\r\n"
					"\t\tThere is no patient associated with this claim.\r\n"
					"\t\tYou will need to recreate this claim.\r\n\r\n\r\n", nClaimID);
				strErrors += str;
				bPassed = FALSE;
			}

			//it is highly unlikely that the above two errors will ever occur. If they do, we can't get the patient name.
			//but now we can, so generate this patient's report header.
			//In the event there are no errors, strErrors is cleared out at the end of this function

			CString strPatientName = AdoFldString(rs, "PatientName","");
			CString strDescription = AdoFldString(rs, "Description","");

			str.Format("____________________________________________________________________________________\r\n\r\n"
				"Patient:  %s\t\r\n\r\nBill Desc: %s     Bill Date: %s\r\n\r\n",
				strPatientName, strDescription, FormatDateTimeForInterface(AdoFldDateTime(rs, "Date")));
			strErrors += str;

			//Insurance Company - this is NOT a warning that can be disabled
			InsuranceCoID = AdoFldLong(rs, "InsCoID",-1);			
			if(InsuranceCoID == -1) {				
				str.Format("\tERROR - No Insurance Company Selected\r\n\r\n"
					"\t\tThere is no Insurance Company selected on this claim.\r\n"
					"\t\tCheck the patient's account and ensure that an\r\n"
					"\t\tinsurance company is selected on the Insurance tab of\r\n"
					"\t\tthe Bill. If no insurance companies are available,\r\n"
					"\t\tyou must add one in the patient's Insurance tab.\r\n\r\n\r\n");
				strErrors += str;
				bHasInsurance = FALSE;
				bPassed = FALSE;
			}

			//store some variables for later
			if(bHasInsurance) {
				strInsCoName = AdoFldString(rs, "InsCoName", "");
			}

			//check the POS validation preference
			if(GetRemotePropertyInt("AlbertaValidatePOSNPI",1,0,"<None>",TRUE) == 1) {
				nPOSID = AdoFldLong(rs, "POSID",-1);
				if(nPOSID == -1) {				
					str.Format("\tWARNING - No Place Of Service Selected\r\n\r\n"
						"\t\tThere is no Place Of Service selected on this claim.\r\n"
						"\t\tCheck the patient's account and ensure that a Place Of\r\n"
						"\t\tservice is selected on the lower left-hand side of the Bill.\r\n\r\n\r\n");
					strErrors += str;
					bWarnings = TRUE;
				}
				else {

					//warn if the Place Of Service has no NPI number
					CString strPOSNPI = AdoFldString(rs, "POSNPI","");
					strPOSNPI.TrimRight();
					if(strPOSNPI.IsEmpty()) {
						str.Format("\tWARNING - No Facility Number Found\r\n\r\n"
							"\t\tThe place of service selected for this bill has no Facility Number filled in.\r\n"
							"\t\tThis number is pulled from the Facility Number field in the Locations tab\r\n"
							"\t\tof the Administrator module.\r\n"
							"\t\tPlease ensure that all locations in the Administrator module have a value\r\n"
							"\t\tfilled in the Facility Number field.\r\n\r\n\r\n");
						strErrors += str;
						bWarnings = TRUE;
					}					
				}
			}

			// (j.jones 2011-09-23 16:07) - PLID 39377 - added validation for duplicate same-day ebilling claims
			if(AdoFldBool(rs, "AlreadyExportedToday", FALSE)
				&& GetRemotePropertyInt("AlbertaValidateExportedTwiceInSameDay",1,0,"<None>",TRUE) == 1) {

				str.Format("\tWARNING - Claim Exported Twice In The Same Day\r\n\r\n"
					"\t\tThis bill has already been exported electronically today.\r\n"
					"\t\tTo avoid a duplicate claim rejection, you should not export\r\n"
					"\t\tthis bill unless you failed to submit the previous export\r\n"
					"\t\tto Alberta HLINK.\r\n\r\n\r\n");
				strErrors += str;
				bWarnings = TRUE;
			}

			long nRefPhyID = AdoFldLong(rs, "RefPhyID", -1);

			if(GetRemotePropertyInt("AlbertaValidateRefPhySelected",1,0,"<None>",TRUE) == 1) {
				//warn if there is no referring physician selected				
				if(nRefPhyID == -1) {
					str.Format("\tERROR - No Referring Physician Selected\r\n\r\n"
						"\t\tThis bill has no referring physician selected.\r\n"
						"\t\tPlease ensure that each bill has a referring physician selected in the Insurance tab of the bill.\r\n\r\n\r\n");
					strErrors += str;
					bPassed = FALSE;
				}
			}
			if(GetRemotePropertyInt("AlbertaValidateRefPhyNPI",1,0,"<None>",TRUE) == 1) {
				if(nRefPhyID != -1) {				
					//warn if there is no ref phy NPI
					CString strRefPhyNPI = AdoFldString(rs, "RefPhyNPI","");
					CString strRefPhyName = AdoFldString(rs, "RefPhyName","");
					strRefPhyNPI.TrimLeft();
					strRefPhyNPI.TrimRight();
					if(strRefPhyNPI.IsEmpty()) {
						str.Format("\tERROR - No Referring Physician Number Found\r\n\r\n"
							"\t\tThe referring physician on this bill, '%s', has no ID number filled in.\r\n"
							"\t\tThis number is pulled from the Ref Phys Number field in the Contacts module.\r\n"
							"\t\tPlease ensure that all referring physicians in the Contacts module have IDs\r\n"
							"\t\tfilled in the Ref Phys Number field.\r\n\r\n\r\n", strRefPhyName);
						strErrors += str;
						bPassed = FALSE;
					}
				}				
			}

			if(GetRemotePropertyInt("AlbertaValidatePatientBirthdate",1,0,"<None>",TRUE) == 1) {
				BOOL bBirthDate = TRUE;
				var = rs->Fields->Item["BirthDate"]->Value;
				if(var.vt != VT_DATE) {
					bBirthDate = FALSE;
				}
				if(!bBirthDate) {
					str.Format("\tWARNING - No Patient Birthdate Found\r\n\r\n"
						"\t\tThe patient on this bill does not have a birthdate filled in.\r\n"
						"\t\tPlease ensure that each patient has a Birthdate filled in on their General 1 tab.\r\n\r\n\r\n");
					strErrors += str;
					bWarnings = TRUE;
				}
			}

			if(GetRemotePropertyInt("AlbertaValidatePatientName",1,0,"<None>",TRUE) == 1) {
				CString strFirst = AdoFldString(rs, "PatFirst", "");
				CString strLast = AdoFldString(rs, "PatLast", "");
				strFirst.TrimLeft();
				strFirst.TrimRight();
				strLast.TrimLeft();
				strLast.TrimRight();
				if(strFirst.IsEmpty() || strLast.IsEmpty()) {
					str.Format("\tWARNING - No Patient Name Found\r\n\r\n"
						"\t\tThe patient on this bill is missing either a first or last name.\r\n"
						"\t\tPlease ensure that each patient has their full name filled in on their General 1 tab.\r\n\r\n\r\n");
					strErrors += str;
					bWarnings = TRUE;
				}
			}

			if(GetRemotePropertyInt("AlbertaValidatePatientGender",1,0,"<None>",TRUE) == 1) {
				//warn if there is no gender
				short nGender = AdoFldByte(rs, "Gender",0);				
				if(nGender != 1 && nGender != 2) {
					str.Format("\tWARNING - No Patient Gender Found\r\n\r\n"
						"\t\tThe patient on this bill has no Gender entered on their account.\r\n"
						"\t\tPlease ensure that each patient has a Gender filled in on their General 1 tab.\r\n"
						"\t\tBecause this is not a RMB claim, this field is not required.\r\n\r\n\r\n");
					strErrors += str;
					bWarnings = TRUE;
				}
			}
		}
		
		// (b.spivey, June 21, 2013) - PLID 54961 - Check the plan name. if it's AB or empty we should throw an error. 
		CString strPlanName = AdoFldString(rs, "PlanName", "");

		rs->Close();

		BOOL bCheckNPI = GetRemotePropertyInt("AlbertaValidateProviderNPI",1,0,"<None>",TRUE) == 1;
		BOOL bCheckTaxonomyCode = GetRemotePropertyInt("AlbertaValidateProviderTaxonomyCode",1,0,"<None>",TRUE) == 1;
		BOOL bCheckBAID = GetRemotePropertyInt("AlbertaValidateProviderBAID",1,0,"<None>",TRUE) == 1;
		long nProviderBAIDCustomField = GetRemotePropertyInt("Alberta_ProviderBAIDCustomField", 6, 0, "<None>", true);

		if(bCheckNPI || bCheckBAID || bCheckTaxonomyCode) {

			//warn for each claim provider on the bill
			// (j.jones 2011-08-16 17:22) - PLID 44805 - we will filter out "original" and "void" charges
			rs = CreateParamRecordset("SELECT ProvidersT.PersonID, ProvidersT.TaxonomyCode, "
				"ProvidersT.NPI, Last + ', ' + First + ' ' + Middle AS ProvName "
				"FROM ProvidersT "
				"INNER JOIN PersonT ON ProvidersT.PersonID = PersonT.ID "
				"WHERE PersonID IN ("
				"SELECT Coalesce(ChargesT.ClaimProviderID, ProvidersT.ClaimProviderID) AS ClaimProviderID FROM ProvidersT " 
				"INNER JOIN ChargesT ON ProvidersT.PersonID = ChargesT.DoctorsProviders "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				"WHERE ChargesT.Batched = 1 AND LineItemT.Deleted = 0 "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"AND ChargesT.BillID = {INT})", nBillID);
			while(!rs->eof) {

				long nProviderID = AdoFldLong(rs, "PersonID", -1);
				CString strNPI = AdoFldString(rs, "NPI","");
				CString strTaxonomyCode = AdoFldString(rs, "TaxonomyCode","");
				CString strProvName = AdoFldString(rs, "ProvName","");

				if(bCheckNPI) {
					strNPI.TrimRight("");
					if(strNPI.IsEmpty()) {			
						str.Format("\tERROR - No Provider Number Found\r\n\r\n"
							"\t\tA provider on this bill, '%s', has no ID number filled in.\r\n"
							"\t\tThis number is pulled from the Provider Number field in the Contacts module.\r\n"
							"\t\tPlease ensure that all providers in the Contacts module have IDs filled\r\n"
							"\t\tin the Provider Number field.\r\n\r\n\r\n", strProvName);
						strErrors += str;
						bPassed = FALSE;
					}
				}

				if(bCheckTaxonomyCode) {
					strTaxonomyCode.TrimRight("");
					if(strTaxonomyCode.IsEmpty()) {			
						str.Format("\tERROR - No Provider Skill Code Found\r\n\r\n"
							"\t\tA provider on this bill, '%s', has no Skill Code filled in.\r\n"
							"\t\tThis number is pulled from the Skill Code field in the Contacts module.\r\n"
							"\t\tPlease ensure that all providers in the Contacts module have IDs filled\r\n"
							"\t\tin the Skill Code field.\r\n\r\n\r\n", strProvName);
						strErrors += str;
						bPassed = FALSE;
					}
				}

				if(bCheckBAID) {

					CString strBusinessArrangement = "";

					//this field will correspond to a provider's custom text field
					_RecordsetPtr rsCustom = CreateParamRecordset("SELECT TextParam FROM CustomFieldDataT "
						"WHERE FieldID = {INT} AND PersonID = {INT}", nProviderBAIDCustomField, nProviderID);
					if(!rsCustom->eof) {
						strBusinessArrangement = AdoFldString(rsCustom, "TextParam","");
						//this is numeric, so strip hyphens and spaces only incase they entered them,
						//anything else is their own data entry problem
						strBusinessArrangement.Replace("-","");
						strBusinessArrangement.Replace(" ","");
					}
					rsCustom->Close();

					strBusinessArrangement.TrimRight();
					if(strBusinessArrangement.IsEmpty()) {
						str.Format("\tERROR - No Provider Business Arrangement ID Found\r\n\r\n"
							"\t\tA provider on this bill, '%s', has no Provider Business Arrangement ID filled in.\r\n"
							"\t\tEnsure that a Provider Business Arrangement ID custom field is selected in the Alberta HLINK Properties,\r\n"
							"\t\tand that this provider has a value entered on their account for the selected custom field.\r\n\r\n\r\n", strProvName);
						strErrors += str;
						bPassed = FALSE;
					}
				}

				rs->MoveNext();
			}
			rs->Close();
		}
		
		// (j.dinatale 2012-12-31 13:04) - PLID 54382 - need to use the util function to get our submitter prefix
		if(GetRemotePropertyInt("AlbertaValidateSubmitterPrefix",1,0,"<None>",TRUE) == 1) {
			//warn if the Submitter Prefix is empty (from Alberta Properties)
			CString strSubmitterPrefix = GetAlbertaProviderSubmitterPrefix(m_nProviderID);

			if(strSubmitterPrefix.IsEmpty()) {
				str.Format("\tERROR - No Submitter Prefix Found\r\n\r\n"
					"\t\tNo Submitter Prefix is entered in the Alberta HLINK Properties.\r\n\r\n\r\n");
				strErrors += str;
				bPassed = FALSE;
			}else{
				if(strSubmitterPrefix.GetLength() != 3){
					str.Format("\tWARNING - Submitter Prefix Incorrect Length\r\n\r\n"
						"\t\tThe Submitter Prefix for the current provider is %li character(s) long. \r\n"
						"\t\tEnsure that the Submitter Prefix is a 3 characters long.\r\n\r\n\r\n", strSubmitterPrefix.GetLength());
					strErrors += str;
				}
			}
		}

		if(GetRemotePropertyInt("AlbertaValidatePatientHealthNumber",1,0,"<None>",TRUE) == 1) {
			//check that the patient has a value for Patient Health Number
			//as specified in Alberta Properties (it is a user-selectable custom field)

			CString strHealthNumber = "";
			long nPatientULICustomField = GetRemotePropertyInt("Alberta_PatientULICustomField", 1, 0, "<None>", true);
			//this field will correspond to a patient's custom text field
			_RecordsetPtr rsCustom = CreateParamRecordset("SELECT TextParam FROM CustomFieldDataT "
				"WHERE FieldID = {INT} AND PersonID = {INT}", nPatientULICustomField, nPatientID);
			if(!rsCustom->eof) {
				strHealthNumber = AdoFldString(rsCustom, "TextParam","");
				//this is numeric, so strip hyphens and spaces only incase they entered them,
				//anything else is their own data entry problem
				strHealthNumber.Replace("-","");
				strHealthNumber.Replace(" ","");
			}
			rsCustom->Close();

			strHealthNumber.TrimLeft();
			strHealthNumber.TrimRight();
			if(strHealthNumber.IsEmpty()) {
				str.Format("\tERROR - No Patient Health Number Found\r\n\r\n"
					"\t\tA Patient Health Number could not be found for this patient.\r\n"
					"\t\tEnsure that a Patient Health Number custom field is selected in the Alberta HLINK Properties,\r\n"
					"\t\tand that this patient has a value entered on their account for the selected custom field.\r\n\r\n\r\n");
				strErrors += str;
				bPassed = FALSE;
			}
		}
				
		// (b.spivey, July 15, 2013) - PLID 54961 - Misunderstood the requirements. Check for not empty and a 
		//		collection of random canadian territories (jacked from Ebilling!) to determine if we need to 
		//		check for the registration number. 
		if((!strPlanName.IsEmpty() && 
					(strPlanName == "WCB"	//Worker's Compensation
						|| strPlanName == "AB"	//Alberta
						|| strPlanName == "BC"	//British Columbia
						|| strPlanName == "MB"	//Manitoba
						|| strPlanName == "NB"	//New Brunswick
						|| strPlanName == "NL"	//Newfoundland and Labrador
						|| strPlanName == "NF"	//Newfoundland (pre-2002)	// (j.jones 2011-07-21 08:58) - PLID 44660 - NF is an accepted abbreviation, despite being obsolete
						|| strPlanName == "NT"	//Northwest Territories
						|| strPlanName == "NS"	//Nova Scotia
						|| strPlanName == "NU"	//Nunavut
						|| strPlanName == "ON"	//Ontario
						|| strPlanName == "PE"	//Prince Edward Island
						|| strPlanName == "SK"	//Saskatchewan
						|| strPlanName == "YT"	//Yukon
					)
			) && GetRemotePropertyInt("AlbertaValidatePatientRegNumber",1,0,"<None>",TRUE) == 1) {
			//check that the patient has a value for Registration Number
			//as specified in Alberta Properties (it is a user-selectable custom field)

			CString strRegistrationNumber = "";
			long nPatientRegNumberCustomField = GetRemotePropertyInt("Alberta_PatientRegNumberCustomField", 2, 0, "<None>", true);
			//this field will correspond to a patient's custom text field
			_RecordsetPtr rsCustom = CreateParamRecordset("SELECT TextParam FROM CustomFieldDataT "
				"WHERE FieldID = {INT} AND PersonID = {INT}", nPatientRegNumberCustomField, nPatientID);
			if(!rsCustom->eof) {
				strRegistrationNumber = AdoFldString(rsCustom, "TextParam","");
				//can be alphanumeric, so do not try to modify this value
			}
			rsCustom->Close();

			strRegistrationNumber.TrimLeft();
			strRegistrationNumber.TrimRight();
			if(strRegistrationNumber.IsEmpty()) {
				str.Format("\tERROR - No Patient Registration Number Found\r\n\r\n"
					"\t\tA Patient Registration Number could not be found for this patient.\r\n"
					"\t\tEnsure that a Registration Number custom field is selected in the Alberta HLINK Properties,\r\n"
					"\t\tand that this patient has a value entered on their account for the selected custom field.\r\n\r\n\r\n");
				strErrors += str;
				bPassed = FALSE;
			}
		}
	
		if(bPassed && !bWarnings){
			strErrors = "";
		}

		m_strInvalidClaims += strErrors;

		int Val;
		if(bPassed && !bWarnings)
			//the claim has no errors, and no warnings
			Val = 1;
		else if(bPassed && bWarnings)
			//the claim had no critical errors, but there are warnings
			Val = 3;
		else if(!bPassed)
			//the claim failed with definite errors
			Val = 2;
		else
			//this should not be possible
			Val = 0;

		ExecuteParamSql("UPDATE HCFATrackT SET ValidationState = {INT} WHERE ID = {INT}",Val,nClaimID);

		return (bPassed && !bWarnings);

	}NxCatchAll(__FUNCTION__);

	ExecuteParamSql("UPDATE HCFATrackT SET ValidationState = 0 WHERE ID = {INT}",nClaimID);

	return FALSE;
}
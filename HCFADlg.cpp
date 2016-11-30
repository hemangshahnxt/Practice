// HCFADlg.cpp : implementation file
//

#include "stdafx.h"
#include "HCFADlg.h"
#include "FormCheck.h"
#include "FormEdit.h"
#include "FormFormat.h"
#include "FormDisplayDlg.h"
#include "globalfinancialutils.h"
#include "printaligndlg.h"
#include "PrintWaitDlg.h"
#include "InsuranceDlg.h"
#include "InternationalUtils.h"
#include "FormQuery.h"
#include "FormLayer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define SAVE_HISTORICAL_INFORMATION
#define NEW_STRUCTURE_LOAD

#define SCROLL_WIDTH	16
#define BUTTON_WIDTH	100
#define BUTTON_HEIGHT	48
#define TEXT_HEIGHT		18

#define IDC_PHYSICIAN_LEFT	120
#define IDC_PHYSICIAN_RIGHT	121
#define IDC_BILL_DOWN		122
#define IDC_BILL_UP			123

// Someone PLEASE find a better way than static globals!!!

// HCFA stuff...probably should have made
// into member variables.

// (j.jones 2014-05-01 10:25) - PLID 61839 - A page is now defined by
// more than just a provider ID, it's also broken up by the Box 17
// provider as well.
class PageInfo {
public:
	long nProviderID;					//the claim provider (Box 33) for this page
	long nBox17PersonID;				//the referring provider (Box 17) for this page
	CString strBox17Qualifier;			//defines the Box 17 person qualifier (this is *not* the ID qualifier in Box 17a)
		
	PageInfo(long nProviderIDToUse, long nBox17PersonIDToUse, CString strBox17QualifierToUse)
	{
		nProviderID = nProviderIDToUse;
		nBox17PersonID = nBox17PersonIDToUse;
		strBox17Qualifier = strBox17QualifierToUse;
	}
};
typedef shared_ptr<PageInfo> PageInfoPtr;

// (j.jones 2014-05-01 10:25) - PLID 61839 - this is now an array of structs

static std::vector<PageInfoPtr> aryPageInfo;
static unsigned long pageinfo_index;// = 0;

static int firstcharge;// = 0;
static long fee_group_id;// = -1;
static long g_nBox33Setup = 1;
static long billid;
static long nTotalCharges;
static long g_nInsuredPartyID = -1;
static long g_DocumentID;
// (j.jones 2013-08-07 10:13) - PLID 57299 - added flag for using the new 02/2012 HCFA
static bool g_bUseNewHCFAForm = true;
// (j.jones 2008-04-07 08:51) - PLID 28995 - added map of Box24J provider IDs, declared here
// so that it is in the same scope as adwProviderList
static CMap<long, long, long, long> mapBox24JProviderIDs;

CFormDisplayDlg *m_pframe;

// For printing
static long nPrintedCharges = 0;
static long nPrintedProviders = 0;
static long oldfirstcharge, oldpageinfo_index;
static long PatientID;
static CFormDisplayDlg* g_pFrame;
static long nPageIncrement = 6;

static COleVariant g_varDefaultGRP;

using namespace ADODB;

/////////////////////////////////////////////////////////////////////////////
// CHCFADlg dialog

void OnHCFACommand(WPARAM wParam, LPARAM lParam, CDWordArray& dwChangedForms, int FormControlsID, CFormDisplayDlg* dlg);
// (j.jones 2014-05-05 15:43) - PLID 61993 - this now takes in a Box17 person ID & qualifer override,
// which is -1 if we're just supposed to use the bill's referring physician
void LoadProviderInfo(long nProviderID, long nLocationID, long nBox17PersonID, CString strBox17PersonQualifier, bool bBox17_IsAProvider);
// (j.jones 2014-05-05 15:43) - PLID 61993 - this now takes in a Box17 person ID & qualifer override,
// which is -1 if we're just supposed to use the bill's referring physician
void BuildFormsT_Form22(long nProviderID, long nLocationID, long nBox17PersonID, CString strBox17PersonQualifier, bool bBox17_IsAProvider);
BOOL PreHCFAPrint(CDWordArray& dwChangedForms, CFormDisplayDlg* dlg);

// (j.jones 2014-05-01 10:48) - PLID 61839 - modular function to get the current page's provider ID
long GetCurrentPageProviderID()
{
	long nProviderID = -1;
	if (pageinfo_index >= 0 && pageinfo_index < aryPageInfo.size()) {
		nProviderID = aryPageInfo[pageinfo_index]->nProviderID;
	}

	return nProviderID;
}

// (j.jones 2014-05-02 10:04) - PLID 61839 - added one function for calculating Form 2, the charges list
// (j.jones 2014-05-06 08:57) - PLID 61993 - this takes in the page index, if -1 it won't filter at all,
// if >= 0 it will filter only on charges for that page
// (j.jones 2014-09-30 10:06) - PLID 63793 - changed to take in the BillID, and streamlined loading Box 24I/J fields
CString GenerateChargesQuery(long nBillID, CString strServiceDateTo, CString strBox24C, CString strBox33bNum, unsigned long nPageInfoIndex = -1)
{
	CString strProviderType = "ChargesT.DoctorsProviders";
	if (g_nBox33Setup == 2) {
		//use the General 1 Main Physician
		strProviderType = "PatientsT.MainPhysician";
	}

	// (j.jones 2007-04-04 15:02) - PLID 25473 - streamlined the query such that
	// we either pull directly from InsuranceBox24J, or use an override	
	CString strBox24I = "InsuranceBox24J.Box24IQualifier";
	CString strBox24JID = "InsuranceBox24J.Box24JNumber";

	// (j.jones 2007-08-08 10:25) - PLID 25395 - added Box24JNPI to the AdvHCFAPinT
	// (j.jones 2008-04-07 09:01) - PLID 28995 - changed the table we pull from
	CString strBox24JNPI = "Box24JProvidersT.NPI";

	CSqlFragment sqlChargeFilter("");
	if (nPageInfoIndex != -1) {

		if (nPageInfoIndex < 0 || nPageInfoIndex >= aryPageInfo.size()) {
			ThrowNxException("GenerateChargesQuery: nPageInfoIndex %li is invalid for page count of %li.", nPageInfoIndex, aryPageInfo.size());
		}

		PageInfoPtr pInfo = aryPageInfo[nPageInfoIndex];

		// (j.jones 2014-05-06 08:57) - PLID 61993 - we now filter by both claim provider and Box 17 provider,
		// it is possible the Box 17 provider can be null
		CSqlFragment sqlBox17IDFilter("Is Null");
		CSqlFragment sqlBox17QualFilter("Is Null");
		if (pInfo->nBox17PersonID != -1) {
			sqlBox17IDFilter = CSqlFragment(" = {INT}", pInfo->nBox17PersonID);
			sqlBox17QualFilter = CSqlFragment(" = {STRING}", pInfo->strBox17Qualifier);
		}

		sqlChargeFilter = CSqlFragment("AND Coalesce(ChargesT.ClaimProviderID, HCFAClaimProvidersT.Box33_ProviderID, ClaimProvidersT.PersonID, -1) = {INT} "
			"AND Coalesce(ChargesT.ReferringProviderID, ChargesT.OrderingProviderID, ChargesT.SupervisingProviderID) {SQL} "
			"AND (CASE WHEN ChargesT.ReferringProviderID Is Not Null THEN 'DN' "
			"	WHEN ChargesT.OrderingProviderID Is Not Null THEN 'DK' "
			"	WHEN ChargesT.SupervisingProviderID Is Not Null THEN 'DQ' "
			"ELSE NULL END) {SQL} ",
			pInfo->nProviderID, sqlBox17IDFilter, sqlBox17QualFilter);

		long nBox24JProviderID = -1;
		mapBox24JProviderIDs.Lookup(pInfo->nProviderID, nBox24JProviderID);
		if (nBox24JProviderID == -1) {
			//this shouldn't be -1, but if it is, use the provider ID
			nBox24JProviderID = pInfo->nProviderID;
		}

		//Load the override for Box24J
		// (j.jones 2008-04-07 09:11) - PLID 28995 - changed to filter by the nBox24JProviderID
		_RecordsetPtr nxrs = CreateParamRecordset("SELECT Box24JQual, Box24J, Box24JNPI FROM ChargesT INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"INNER JOIN AdvHCFAPinT ON LineItemT.LocationID = AdvHCFAPinT.LocationID WHERE AdvHCFAPinT.SetupGroupID = {INT} AND BillsT.ID = {INT} AND AdvHCFAPinT.ProviderID = {INT}", fee_group_id, nBillID, nBox24JProviderID);
		if (!nxrs->eof) {
			CString strOverride24I = AdoFldString(nxrs, "Box24JQual", "");
			if (!strOverride24I.IsEmpty()) {
				strBox24I.Format("'%s'", _Q(strOverride24I));
			}
			CString strOverride24J = AdoFldString(nxrs, "Box24J", "");
			if (!strOverride24J.IsEmpty()) {
				strBox24JID.Format("'%s'", _Q(strOverride24J));
			}
			CString strOverride24JNPI = AdoFldString(nxrs, "Box24JNPI", "");
			if (!strOverride24JNPI.IsEmpty()) {
				strBox24JNPI.Format("'%s'", _Q(strOverride24JNPI));
			}
		}
		nxrs->Close();
	}

	// (j.jones 2013-08-09 12:40) - PLID 57955 - Moved this code to a modular function.
	// This function will calculate all diagnosis codes that should show on this bill,
	// following the SkipUnlinkedDiagsOnClaims preference if it is enabled.
	// This function now also filters by provider.
	// (a.walling 2014-03-06 08:47) - PLID 61216 - insured party ID for determining ICD10 state
	CString strTop12DiagCodeFields, strTop4DiagCodeDescFields, strWhichCodes, strICDIndicator;
	GenerateHCFADiagCodeFieldsForBill(g_bUseNewHCFAForm, billid, g_nInsuredPartyID, sqlChargeFilter,
		strTop12DiagCodeFields, strTop4DiagCodeDescFields, strWhichCodes, strICDIndicator);

	CString strQuery;

	//DRT 4/10/2006 - PLID 11734 - Removed ProcCode
	// (j.jones 2008-04-04 09:16) - PLID 28995 - supported the HCFAClaimProvidersT setup
	// (j.jones 2008-05-28 15:08) - PLID 30177 - supported NDC codes, only if there is no anesthesia note
	// (j.jones 2008-08-01 10:17) - PLID 30917 - HCFAClaimProvidersT is now per insurance company
	// (j.gruber 2009-03-18 10:26) - PLID 33574 - changed discount structure
	// (j.jones 2010-04-07 17:29) - PLID 15224 - removed ChargesT.EMG, obsolete field
	// (j.jones 2010-11-09 14:42) - PLID 41387 - supported ChargesT.ClaimProviderID
	// (j.jones 2011-04-20 11:28) - PLID 43330 - supported overriding the diagnosis codes
	// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" charges & applies
	// (j.jones 2011-06-01 08:59) - PLID 43775 - supported ChargesT.ClaimProviderID in the Box24JProvidersT join
	// (j.jones 2013-08-09 12:35) - PLID 57955 - supported 12 diag codes, and reworked how they inserted into this query
	// (a.walling 2014-03-06 09:21) - PLID 61217 - All DiagCodes and info are passed into us; no need to join into anything at this point.
	// (j.jones 2014-05-01 14:04) - PLID 61993 - added Referring/Ordering/Supervising provider logic, such that if a charge
	// has more than one specialty provider, they override each other in this order
	// (j.jones 2014-08-01 09:55) - PLID 63103 - supported lab test codes on charges, will override anesthesia and NDC notes
	strQuery.Format("SELECT GetDate() AS TodaysDate, BillsT.Date AS BillDate, ChargesT.ID, ChargesT.BillID, ChargesT.ItemCode, ChargesT.ItemSubCode, ChargesT.Category, ChargesT.SubCategory, LineItemT.Description, ChargesT.CPTModifier, ChargesT.CPTModifier2, ChargesT.CPTModifier3, ChargesT.CPTModifier4, "
		"%s, %s, %s AS WhichCodes, %s AS ICDInd, "
		"ChargesT.TaxRate, ChargesT.TaxRate2, ChargesT.Quantity, LineItemT.Amount, ChargesT.OthrBillFee, LineItemT.LocationID, "

		"Coalesce(ChargesT.ClaimProviderID, HCFAClaimProvidersT.Box33_ProviderID, ClaimProvidersT.PersonID) AS DoctorsProviders, "
		"Coalesce(ChargesT.ClaimProviderID, HCFAClaimProvidersT.Box24J_ProviderID, ClaimProvidersT.PersonID) AS Box24JProviderID, "

		"Coalesce(ChargesT.ReferringProviderID, ChargesT.OrderingProviderID, ChargesT.SupervisingProviderID) AS ChargeBox17PersonID, "
		"(CASE WHEN ChargesT.ReferringProviderID Is Not Null THEN 'DN' "
		"	WHEN ChargesT.OrderingProviderID Is Not Null THEN 'DK' "
		"	WHEN ChargesT.SupervisingProviderID Is Not Null THEN 'DQ' "
		"ELSE 'DN' END) AS ChargeBox17Qual, "
		"Convert(bit, CASE WHEN Box17ProvidersT.PersonID Is Not Null THEN 1 ELSE 0 END) AS ChargeBox17IsAProvider, "
		""
		"ChargesT.ServiceDateFrom AS [Service Date From], %s AS [Service Date To], "
		"PlaceOfServiceCodesT.PlaceCodes AS [Service Location], ChargesT.EPSDT, ChargesT.COB, LineItemT.Date, LineItemT.InputDate, LineItemT.InputName, "
		"(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) as PercentOff, "
		"(SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) as Discount, "
		"(CASE WHEN ChargeRespT.Amount Is Not Null THEN ChargeRespT.Amount ELSE 0 END) AS InsResp, "
		"(CASE WHEN ChargeRespT.Amount Is Not Null THEN InsuredPartyT.RespTypeID ELSE 0 END) AS InsType, "
		"dbo.GetChargeTotal(ChargesT.ID) AS ChargeTotal, ServiceT.Anesthesia, "

		"CASE WHEN IsNull(ChargeLabTestCodesQ.LabTestCodes, '') <> '' THEN ChargeLabTestCodesQ.LabTestCodes "
		"	WHEN ServiceT.Anesthesia <> 0 THEN '%s' "
		"	ELSE ChargesT.NDCCode END AS SupplementalNote, "

		"SumOfAmount AS ApplyTotal, SumOfPayAmount AS PaymentTotal, "
		"%s AS Box24IQual, %s AS Box24JID, %s AS Box24JNPI, "
		"%s AS Box24C, %s AS Box33bNum, '' AS Box33aNPI "
		"FROM BillsT "
		"INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
		"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
		"INNER JOIN PatientsT ON BillsT.PatientID = PatientsT.PersonID "
		"LEFT JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ID "
		"LEFT JOIN InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID "
		"LEFT JOIN (SELECT Sum(Round(AppliesT.Amount,2)) AS SumOfAmount, DestID "
		"	FROM AppliesT "
		"	INNER JOIN LineItemT ON AppliesT.SourceID = LineItemT.ID "
		"	LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
		"	LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
		"	WHERE Deleted = 0 "
		"	AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
		"	AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
		"	GROUP BY DestID) AS AppliesQ ON ChargesT.ID = AppliesQ.DestID "
		"LEFT JOIN (SELECT DestID, Sum(AppliesT.Amount) AS SumOfPayAmount "
		"	FROM AppliesT "
		"	INNER JOIN LineItemT ON AppliesT.SourceID = LineItemT.ID "
		"	LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
		"	LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
		"	WHERE LineItemT.Type = 1 "
		"	AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
		"	AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
		"	GROUP BY DestID) AS PayAppliesQ ON ChargesT.ID = PayAppliesQ.DestID "
		"LEFT JOIN PlaceOfServiceCodesT ON ChargesT.ServiceLocationID = PlaceOfServiceCodesT.ID "
		"LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "

		"LEFT JOIN (SELECT ChargesT.ID, "
		"	STUFF((SELECT ',' + BillLabTestCodesT.Code "
		"	FROM BillLabTestCodesT "
		"	INNER JOIN ChargeLabTestCodesT ON BillLabTestCodesT.ID = ChargeLabTestCodesT.BillLabTestCodeID "
		"	WHERE ChargeLabTestCodesT.ChargeID = ChargesT.ID "
		"	FOR XML PATH('')), 1, 1, '') AS LabTestCodes "
		"	FROM ChargesT "
		") AS ChargeLabTestCodesQ ON ChargesT.ID = ChargeLabTestCodesQ.ID "

		"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
		"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "

		"LEFT JOIN ProvidersT ActualProvidersT ON %s = ActualProvidersT.PersonID "
		"LEFT JOIN ProvidersT ClaimProvidersT ON ActualProvidersT.ClaimProviderID = ClaimProvidersT.PersonID "
		"LEFT JOIN InsuredPartyT BillInsuredPartyT ON BillsT.InsuredPartyID = BillInsuredPartyT.PersonID "
		"LEFT JOIN HCFAClaimProvidersT AS HCFAClaimProvidersT ON ActualProvidersT.PersonID = HCFAClaimProvidersT.ProviderID AND BillInsuredPartyT.InsuranceCoID = HCFAClaimProvidersT.InsuranceCoID "
		"LEFT JOIN ProvidersT Box24JProvidersT ON Coalesce(ChargesT.ClaimProviderID, HCFAClaimProvidersT.Box24J_ProviderID, ClaimProvidersT.PersonID) = Box24JProvidersT.PersonID "

		"LEFT JOIN InsuranceBox24J ON InsuranceBox24J.InsCoID = BillInsuredPartyT.InsuranceCoID AND InsuranceBox24J.ProviderID = Box24JProvidersT.PersonID "
		"LEFT JOIN ProvidersT Box17ProvidersT ON Box17ProvidersT.PersonID = Coalesce(ChargesT.ReferringProviderID, ChargesT.OrderingProviderID, ChargesT.SupervisingProviderID) "
		"WHERE BillsT.ID = %li "
		"%s "
		"AND LineItemT.Deleted = 0 AND Batched = 1 "
		"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
		"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null ",
		strTop12DiagCodeFields, strTop4DiagCodeDescFields, strWhichCodes, strICDIndicator,
		strServiceDateTo, _Q(CalculateAnesthesiaNoteForHCFA(billid)),
		strBox24I, strBox24JID, strBox24JNPI,
		strBox24C, strBox33bNum,
		strProviderType,
		billid,
		sqlChargeFilter.Flatten());
	
	return strQuery;
}

// (j.jones 2007-01-17 12:26) - PLID 24263 - I reworked the absurdity of the group number behavior

CString CalcGRPNumber()
{
	try {
		
		CString strGroupId = "";

		// (j.jones 2014-05-01 10:48) - PLID 61839 - get the provider ID out of the current page info
		long nProviderID = GetCurrentPageProviderID();

		if (nProviderID != -1) {
			_RecordsetPtr rs = CreateParamRecordset("SELECT GRP FROM AdvHCFAPinT WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT}", nProviderID, GetBillLocation(billid), fee_group_id);
			if (!rs->eof) {
				strGroupId = VarString(rs->Fields->Item["GRP"]->Value, "");
			}
			rs->Close();
		}

		if(!strGroupId.IsEmpty()) {
			return strGroupId;
		}

		//if that's blank, use the default group
		strGroupId = VarString(g_varDefaultGRP,"");

		if (strGroupId == "-1")
			strGroupId = "";

		if(!strGroupId.IsEmpty()) {
			return strGroupId;
		}

		//and if that's blank, then use the normal GRP setup

		//////////////////////////////////////////////////////////////////
		// Override GRP number through InsuranceGroups table.
		if (g_nInsuredPartyID != -1 && nProviderID != -1) {
			_RecordsetPtr rs = CreateParamRecordset("SELECT InsuranceGroups.GRP "
				"FROM InsuredPartyT "
				"INNER JOIN InsuranceGroups ON InsuredPartyT.InsuranceCoID = InsuranceGroups.InsCoID "
				"WHERE InsuredPartyT.PersonID = {INT} AND InsuranceGroups.ProviderID = {INT}",
				g_nInsuredPartyID, nProviderID);
			if (!rs->eof) {
				// We can only do so if there is a record to pull from
				strGroupId = VarString(rs->Fields->Item["GRP"]->Value, "");
			}
			rs->Close();
		}

		return strGroupId;
	}
	NxCatchAll("Error in CalcGRPNumber");

	return "";
}
	   
void ReloadGRPNumber(CFormDisplayDlg* dlg)
{
	try {

		CString strGroupId = CalcGRPNumber();

		// At this point we don't care how we got it, but we can be sure 
		// that strGroupId contains the correct group ID so go ahead and use it
		COleVariant var = _bstr_t(strGroupId);
		dlg->ChangeParameter(1, "GRPNumber", var);
		dlg->ChangeParameter(6, "GRPNumber", var);
		dlg->ChangeParameter(34, "GRPNumber", var);
	}
	NxCatchAll("Error in ReloadGRPNumber");
}

/////////////////////////////////////////////////////
// This function, given the bill ID, provider index,
// and first visible charge (all static globals),
// will find the Document ID in the historical forms
// table, and assign that ID to the form display
// dialog.
//
// The form display dialog will internally use this
// to generate field values based on user-written
// historical data.
//
// This is called on OnInitDialog and OnHCFACommand.
//
void RequeryHistoricalHCFAData()
{
	try {
		
		if (pageinfo_index < 0 || pageinfo_index >= aryPageInfo.size()) {
			ThrowNxException("pageinfo_index %li is invalid for page count of %li.", pageinfo_index, aryPageInfo.size());
		}

		PageInfoPtr pInfo = aryPageInfo[pageinfo_index];

		_RecordsetPtr rs;
		// (j.jones 2014-05-02 11:41) - PLID 61839 - added Box17 info, because pages can now be broken up by the Box17 person and qualifier
		if (pInfo->nBox17PersonID != -1) {
			rs = CreateParamRecordset("SELECT FirstDocumentID FROM HCFADocumentsT "
				"WHERE BillID = {INT} AND FirstCharge = {INT} AND ProviderIndex = {INT} AND FormType = 0 "
				"AND Box17PersonID = {INT} AND Box17Qual = {STRING}",
				billid, firstcharge, pInfo->nProviderID, pInfo->nBox17PersonID, pInfo->strBox17Qualifier);
		}
		else {
			//typical case
			//if there is no special Box17 person, and we're using the normal bill referring physician,
			//the Box17PersonID is null, and the qualifier is empty
			rs = CreateParamRecordset("SELECT FirstDocumentID FROM HCFADocumentsT "
				"WHERE BillID = {INT} AND FirstCharge = {INT} AND ProviderIndex = {INT} AND FormType = 0 "
				"AND Box17PersonID Is Null",
				billid, firstcharge, pInfo->nProviderID);
		}

		if (!rs->eof) {
			//-1 is an acceptable value
			g_DocumentID = VarLong(rs->Fields->Item["FirstDocumentID"]->Value, -1);
		}
		else {
			g_DocumentID = -1;
		}
		rs->Close();

		g_pFrame->SetDocumentID(g_DocumentID);
	}
	NxCatchAll("Error in requerying historical data");
}

CHCFADlg::CHCFADlg(CWnd* pParent /*=NULL*/)
	: CDialog(CHCFADlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CHCFADlg)
	m_CapOnPrint = FALSE;
	m_bPrintWarnings = TRUE;
	//}}AFX_DATA_INIT
	m_pframe = 0;
	m_pleftbutton = 0;
	m_prightbutton = 0;
	m_pupbutton = 0;
	m_pdownbutton = 0;
	m_CapOnPrint = GetRemotePropertyInt("CapitalizeHCFA",0);
	//m_CapOnPrint = TRUE;
	//m_pOnCommand = NULL;
	m_pPrePrintFunc = NULL;
	m_ShowWindowOnInit = TRUE;

	g_nInsuredPartyID = -1;
	m_OthrInsuredPartyID = -1;
	m_ID = -1;

	m_bShowSecondaryInBox11 = FALSE;

	m_pOnCommand = &OnHCFACommand;
	//m_pPrePrintFunc = &PreHCFAPrint;

	g_bUseNewHCFAForm = true;
}

CHCFADlg::~CHCFADlg()
{
	if (m_pframe)
		delete m_pframe;
	if (m_pleftbutton)
		delete m_pleftbutton;
	if (m_prightbutton)
		delete m_prightbutton;
	if (m_pupbutton)
		delete m_pupbutton;
	if (m_pdownbutton)
		delete m_pdownbutton;
}

void CHCFADlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHCFADlg)
	DDX_Check(pDX, IDC_CAP_ON_PRINT, m_CapOnPrint);
	// (j.jones 2007-06-25 10:14) - PLID 25663 - changed the buttons to NxIconButtons
	DDX_Control(pDX, IDC_RESTORE, m_btnRestoreDefaults);
	DDX_Control(pDX, IDC_ALIGN, m_btnAlignForm);
	DDX_Control(pDX, IDC_CHECK, m_btnSave);
	DDX_Control(pDX, IDC_PRINT, m_btnPrint);
	DDX_Control(pDX, IDC_X, m_btnClose);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CHCFADlg, CDialog)
	//{{AFX_MSG_MAP(CHCFADlg)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_CAP_ON_PRINT, OnClickCapitalizeOnPrint)
	ON_BN_CLICKED(IDC_RESTORE, OnRestore)
	ON_BN_CLICKED(IDC_CHECK, OnCheck)
	ON_BN_CLICKED(IDC_PRINT, OnPrint)
	ON_BN_CLICKED(IDC_ALIGN, OnAlign)
	ON_WM_VSCROLL()
	ON_BN_CLICKED(IDC_X, OnCancel)
	ON_BN_CLICKED(IDC_RADIO_NO_BATCH, OnRadioNoBatch)
	ON_BN_CLICKED(IDC_RADIO_ELECTRONIC, OnRadioElectronic)
	ON_BN_CLICKED(IDC_RADIO_PAPER, OnRadioPaper)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHCFADlg message handlers

BOOL CHCFADlg::OnInitDialog() 
{
	CWaitCursor pWait;

	CDialog::OnInitDialog();

	CString			where;
	RECT			rect;
	_RecordsetPtr	tmpRS;

	CStringArray astrParams;
	VarAry avarParams;

	EnsureRemoteData();

	ShowScrollBar(SB_VERT, TRUE);

	firstcharge = 0;
	pageinfo_index = 0;

	// (j.dinatale 2010-07-23) - PLID 39692 - Set the Device context that this dialog holds to null to begin with
	m_pPrintDC = NULL;

	// (j.dinatale 2010-07-28) - PLID 39803 - initialize the update flag to true, so that way if its not specified that we want to update, we assume that it wants to be done
	m_bUpdateClaimsTables = true;

	////////////////////////////////////////////////
	/// HCFA SPECIFIC STUFF ////////////////////////
	////////////////////////////////////////////////
	try {
		_RecordsetPtr rs;

		// (j.jones 2010-03-15 15:19) - PLID 37719 - removed unnecessary log
		//LogDetail("HCFADlg: Getting insurance IDs");

		//////////////////////////////////////
		// Set some global statics
		PatientID = m_PatientID;
		billid = m_ID;
		m_CapOnPrint = GetRemotePropertyInt("CapitalizeHCFA",0);
		((CButton*)GetDlgItem(IDC_CAP_ON_PRINT))->SetCheck(m_CapOnPrint);
		//m_CapOnPrint = TRUE;

		//load the HCFAInfo class
		m_HCFAInfo.Initialize();

		///////////////////////////////////////////
		// Get the Insured Party and Other
		// Insured Party ID's. Also get the
		// bill name for use in a later
		// SQL statement.
		// (j.jones 2013-08-07 10:19) - PLID 57299 - parameterized and improved
		rs = CreateParamRecordset("SELECT BillsT.Description, "
			"BillsT.InsuredPartyID, BillsT.OthrInsuredPartyID, InsuranceCoT.HCFASetupGroupID "
			"FROM BillsT "
			"LEFT JOIN InsuredPartyT ON BillsT.InsuredPartyID = InsuredPartyT.PersonID "
			"LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
			"WHERE BillsT.ID = {INT}", m_ID);
		if(rs->eof) {
			ThrowNxException("Failed to find bill ID %li!", m_ID);
		}
		else {
			g_nInsuredPartyID = VarLong(rs->Fields->Item["InsuredPartyID"]->Value, -1);
			m_OthrInsuredPartyID = VarLong(rs->Fields->Item["OthrInsuredPartyID"]->Value, -1);
			m_strBillName = VarString(rs->Fields->Item["Description"]->Value, "");

			// (j.jones 2013-08-07 10:18) - PLID 57299 - Determine whether we need to use the new HCFA.
			// If they permit the old form on older service dates, this function handles that logic too.
			g_bUseNewHCFAForm = UseNewHCFAForm(g_nInsuredPartyID, m_ID);

			fee_group_id = VarLong(rs->Fields->Item["HCFASetupGroupID"]->Value, -1);
			if(fee_group_id != -1) {
				m_HCFAInfo.LoadData(fee_group_id);
			}
		}
		rs->Close();

		//////////////////////////////////////////////////
		// Set up defaults in HCFA boxes
		BuildHCFAChargesT(); // Put all the charges in a temporary table for the HCFA to read from	

		// From this point adwProviderList is initialized

		// (j.jones 2007-01-17 14:06) - PLID 24263 - initially generates the group number
		BuildGRPNumber(&astrParams, &avarParams);

		SetHCFADateFormats(); // Set all the date boxes to 2 or 4 digit years through HCFA ID Setup information

		//Do NOT load box 25 default value until RequeryHistoricalData has been called

		////////////////////////////////////////////////
		// Find out if this is paper or electronic
		// batched and check the checkbox in the corner.
		FindBatch();
	
	}NxCatchAll("Error in HCFADlg::OnInitDialog 1");
	////////////////////////////////////

	try {

		////////////////////////////////////////////////////
		// Allocate the main form display of the HCFA dialog
		// (it contains several layers, and each layer contains
		// several boxes)
		// (j.jones 2010-03-15 15:19) - PLID 37719 - removed unnecessary log
		//LogDetail("HCFADlg: Creating frame");
		m_pframe = new CFormDisplayDlg(this);
		m_pframe->color = 0x000000FF;
		g_pFrame = m_pframe;
		RequeryHistoricalHCFAData();

		//load the default Box25 value
		LoadBox25Default();

		////////////////////////////////////////////////////////////
		// Allocate buttons
		m_pleftbutton	= new CButton;
		m_prightbutton	= new CButton;
		m_pupbutton		= new CButton;
		m_pdownbutton	= new CButton;

		if (!m_pframe->Create(IDD_FORMS_DIALOG, this))
			AfxMessageBox ("Failed!");
		ModifyStyle (0, WS_CAPTION | WS_SYSMENU);
		SetControlPositions();

		// (j.jones 2008-05-09 11:40) - PLID 29953 - added button styles for modernization
		m_btnRestoreDefaults.AutoSet(NXB_MODIFY);
		m_btnAlignForm.AutoSet(NXB_MODIFY);
		m_btnSave.AutoSet(NXB_OK);
		m_btnPrint.AutoSet(NXB_PRINT);
		m_btnClose.AutoSet(NXB_CLOSE);

		// (v.maida 2016-06-06 15:55) - NX-100631 - Set a larger default size so that the dialog is easier to read if it's restored down to a smaller size.
		SetWindowPos(NULL, 0, 0, 994, 738, 0);

		if (m_ShowWindowOnInit) 
			ShowWindow(SW_SHOWMAXIMIZED);
		else
			m_pframe->m_ShowPrintDialog = FALSE;


		////////////////////////////////////////////////////////////////////////////
		// Load all of the layers of the form...each layer has several controls.
		// Refer to FormsT for SQL's corresponding to the numbers in m_pFrame->Load.
		// Inputs include the form layer ID, the where clause of the SQL, an index
		// to the first free control in the form as a whole, and an optional array
		// of parameters and parameter names (which are saved in the form layer).
		//
		int i = 187;//can be any number above largest control id

		ShowSignature();

		ShowAddress();

		_variant_t var;
		CString str;
		_RecordsetPtr rs;

		CString strPatientID = "PatientsT.UserDefinedID";

		if(GetRemotePropertyInt("SearchByBillID", 0, 0, "<None>", TRUE) == 1) {
			//use patient ID / bill ID as the patient ID
			strPatientID.Format("(Convert(nvarchar, PatientsT.UserDefinedID) + '/' + Convert(nvarchar, '%li'))",m_ID);

			if(GetRemotePropertyInt("HCFAIDAppendRespType", 0, 0, "<None>", TRUE) == 1) {
				//use patient ID / bill ID-RespTypeID as the patient ID (but only if it is secondary or higher)
				int RespTypeID = GetInsuranceTypeFromID(g_nInsuredPartyID);
				if(RespTypeID > 1)
					strPatientID.Format("(Convert(nvarchar, PatientsT.UserDefinedID) + '/' + Convert(nvarchar, '%li') + '-' + Convert(nvarchar, '%li'))",m_ID,RespTypeID);
			}
		}

		// (j.jones 2013-08-07 10:29) - PLID 57299 - change the form based on the new/old HCFA flag
		{
			CString strHCFAVersionBatch;

			if(g_bUseNewHCFAForm) {
				//changes for the 02/2012 form

				//Box 1 labels
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET Source = 'TRICARE' WHERE ID = 58");
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET Source = '(ID#/DoD#)' WHERE ID = 67");
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET Source = '(ID#)' WHERE ID IN (69, 70, 71)");

				//Box 8
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET Source = '8. RESERVED FOR NUCC USE', Width = 140 WHERE ID = 126");
				//hide the labels and checkboxes by changing the form to -1
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET FormID = -1 WHERE ID IN (127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140)");
				//show the edit boxes
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET FormID = 22 WHERE ID IN (8688, 8689, 8690)");

				//Box 9b
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET Source = 'b. RESERVED FOR NUCC USE' WHERE ID = 145");
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET Source = 'Box9b', X = 22, Width = 175, FormID = 22, Format = 512 WHERE ID = 146");
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET FormID = -1 WHERE ID IN (147, 148, 149, 150, 151)");

				//Box 9c
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET Source = 'c. RESERVED FOR NUCC USE' WHERE ID = 152");
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET Source = 'Box9c', Width = 175, FormID = 22 WHERE ID = 153");

				//Box 10d
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET Source = '10d. CLAIM CODES' WHERE ID = 194");

				//Box 11b
				//becomes a 2 part qualifier / ID field
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET Source = 'b. OTHER CLAIM ID' WHERE ID = 165");
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET Source = 'Box11bQual', FormID = 22 WHERE ID = 8691");
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET Source = 'Box11b', X = 508, Width = 210 WHERE ID = 166");

				//Box 11d
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET Source = 'If yes, complete items 9, 9a, and 9d.' WHERE ID = 174");

				//Box 14
				//hide three labels, combine into just one
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET Source = '14. DATE OF CURRENT ILLNESS, INJURY, or PREGNANCY (LMP)', Width = 250 WHERE ID = 208");
				//change the form to -1 to hide the fields
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET FormID = -1 WHERE ID IN (209, 210, 211)");
				//show the qualifier
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET FormID = 22 WHERE ID IN (8708, 8709)");

				//Box 15
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET Source = '15. OTHER DATE' WHERE ID = 232");
				//change the form to -1 to hide the field
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET FormID = -1 WHERE ID = 233");
				//show the qualifier
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET FormID = 22 WHERE ID IN (8710, 8711)");

				//Box 17				
				//show the qualifier and move the ref. phy. name
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET FormID = 22 WHERE ID = 8712");
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET X = 50, Width = 210 WHERE ID = 237");

				//Box 19
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET Source = '19. ADDITIONAL CLAIM INFORMATION' WHERE ID = 239");

				//Box 21 - Diagnosis Codes
				//slightly abbreviated label to ensure room for the ICD Indicator
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET Source = '21. DIAGNOSIS OR NATURE OF ILLNESS OR INJURY  Relate A-L to line 24E', Width = 330 WHERE ID = 240");
				//Show the ICD indicator
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET FormID = 2 WHERE ID IN (8713, 8714)");
				//Show codes 5 through 12
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET FormID = 2 WHERE ID IN (8692, 8693, 8694, 8695, 8696, 8697, 8698, 8699)");
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET FormID = 2 WHERE ID IN (8700, 8701, 8702, 8703, 8704, 8705, 8706, 8707)");
				//Move codes 1 - 4 to the new locations
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET Source = 'A.', X = 23, Y = 625, Width = 12 WHERE ID = 249");
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET X = 38, Y = 621, Height = 14, Width = 70 WHERE ID = 243");
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET Source = 'B.', X = 146, Y = 625, Width = 12 WHERE ID = 250");
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET X = 161, Y = 621, Height = 14, Width = 70 WHERE ID = 244");
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET Source = 'C.', X = 270, Y = 625, Width = 12 WHERE ID = 251");
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET X = 285, Y = 621, Height = 14, Width = 70 WHERE ID = 245");
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET Source = 'D.', X = 397, Y = 625, Width = 12 WHERE ID = 252");
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET X = 410, Y = 621, Height = 14, Width = 70 WHERE ID = 246");
				//Hide descriptions for codes 1 - 4
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET FormID = -1 WHERE ID IN (4399, 4400, 4401, 4402)");

				//Box 22
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET Source = '22. RESUBMISSION' WHERE ID = 370");

				//Box 30
				//technically "Rsvd for NUCC Use" but it won't fit in the box
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET Source = '30. Rsvd for NUCC' WHERE ID = 430");
			}
			else {
				//changes for the 08/2005 form

				//Box 1 labels
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET Source = 'CHAMPUS' WHERE ID = 58");
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET Source = '(Sponsor''s SSN)' WHERE ID = 67");
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET Source = '(SSN or ID)' WHERE ID = 69");
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET Source = '(SSN)' WHERE ID = 70");
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET Source = '(ID)' WHERE ID = 71");

				//Box 8
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET Source = '8. PATIENT STATUS', Width = 95 WHERE ID = 126");
				//show the labels and checkboxes by reverting to the correct form
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET FormID = 1 WHERE ID IN (127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140)");
				//hide the edit boxes
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET FormID = -1 WHERE ID IN (8688, 8689, 8690)");

				//Box 9b
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET Source = 'b. OTHER INSURED''S DATE OF BIRTH' WHERE ID = 145");
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET Source = 'OthrInsBD', X = 30, Width = 95, FormID = 21, Format = 36875 WHERE ID = 146");
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET FormID = 21 WHERE ID IN (147, 148, 149, 150, 151)");

				//Box 9c
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET Source = 'c. EMPLOYER''S NAME OR SCHOOL NAME' WHERE ID = 152");
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET Source = 'OthrInsEmp', Width = 175, FormID = 21 WHERE ID = 153");

				//Box 10d
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET Source = '10d. RESERVED FOR LOCAL USE' WHERE ID = 194");

				//Box 11b
				//becomes just one employer field
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET Source = 'b. EMPLOYER''S NAME OR SCHOOL NAME' WHERE ID = 165");
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET Source = '', FormID = -1 WHERE ID = 8691");
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET Source = 'InsEmp', X = 486, Width = 232 WHERE ID = 166");

				//Box 11d
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET Source = 'If yes, return to and complete item 9 a-d.' WHERE ID = 174");

				//Box 14
				//restore the main label and show the three hidden ones
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET Source = '14. DATE OF CURRENT:', Width = 95 WHERE ID = 208");
				//revert to the correct form (from -1) to show the fields
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET FormID = 22 WHERE ID IN (209, 210, 211)");
				//hide the qualifier
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET FormID = -1 WHERE ID IN (8708, 8709)");

				//Box 15
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET Source = '15. IF PATIENT HAS HAD SAME OR SIMILAR ILLNESS,' WHERE ID = 232");
				//revert to the correct form (from -1) to show the field
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET FormID = 22 WHERE ID = 233");
				//hide the qualifier
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET FormID = -1 WHERE ID IN (8710, 8711)");

				//Box 17
				//hide the qualifier and move the ref. phy. name
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET FormID = -1 WHERE ID = 8712");
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET X = 22, Width = 238 WHERE ID = 237");

				//Box 19
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET Source = '19. RESERVED FOR LOCAL USE' WHERE ID = 239");

				//Box 21 - Diagnosis Codes
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET Source = '21. DIAGNOSIS OR NATURE OF ILLNESS OR INJURY. (RELATE ITEMS 1,2,3, OR 4 TO ITEM 24E BY LINE)', Width = 420 WHERE ID = 240");
				//Hide the ICD indicator
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET FormID = -1 WHERE ID IN (8713, 8714)");
				//Hide codes 5 through 12
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET FormID = -1 WHERE ID IN (8692, 8693, 8694, 8695, 8696, 8697, 8698, 8699)");
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET FormID = -1 WHERE ID IN (8700, 8701, 8702, 8703, 8704, 8705, 8706, 8707)");
				//Move codes 1 - 4 to the old locations
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET Source = '1.', X = 28, Y = 628, Width = 10 WHERE ID = 249");
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET X = 53, Y = 624, Height = 16, Width = 50 WHERE ID = 243");
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET Source = '2.', X = 28, Y = 655, Width = 10 WHERE ID = 250");
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET X = 53, Y = 651, Height = 16, Width = 50 WHERE ID = 244");
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET Source = '3.', X = 291, Y = 628, Width = 10 WHERE ID = 251");
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET X = 315, Y = 624, Height = 16, Width = 50 WHERE ID = 245");
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET Source = '4.', X = 291, Y = 655, Width = 10 WHERE ID = 252");
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET X = 315, Y = 651, Height = 16, Width = 50 WHERE ID = 246");
				//Show descriptions for codes 1 - 4
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET FormID = 2 WHERE ID IN (4399, 4400, 4401, 4402)");


				//Box 22
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET Source = '22. MEDICAID RESUBMISSION' WHERE ID = 370");

				//Box 30
				AddStatementToSqlBatch(strHCFAVersionBatch, "UPDATE FormControlsT SET Source = '30. BALANCE DUE' WHERE ID = 430");
			}

			if(!strHCFAVersionBatch.IsEmpty()) {
				ExecuteSqlBatch(strHCFAVersionBatch);
			}
		}

		// (j.jones 2007-02-28 17:36) - PLID 23297 - converted to use batched statements per form
		// (doesn't really affect Form 1 right now but will help in the future
		{
			CString strForm1Batch = BeginSqlBatch();

			//0 - the Print Date (today), 1 - the Bill Date
			if(m_HCFAInfo.Box31UseDate == 0) {
				AddStatementToSqlBatch(strForm1Batch, "UPDATE FormControlsT SET Source = 'TodaysDate', FormID = 2 WHERE ID = 398");
			}
			else {
				AddStatementToSqlBatch(strForm1Batch, "UPDATE FormControlsT SET Source = 'BillDate', FormID = 2 WHERE ID = 398");
			}

			if(!strForm1Batch.IsEmpty()) {
				ExecuteSqlBatch(strForm1Batch);
			}

			// (j.jones 2006-12-01 13:01) - PLID 22110 - supported the ClaimProviderID override
			// (j.jones 2008-04-04 09:16) - PLID 28995 - this query doesn't even need providers, so I removed the references
			g_aryFormQueries[1].sql.Format("SELECT %s AS [Patient ID], PersonT.[Last] + ', ' + PersonT.[First] + ' ' + PersonT.[Middle] AS PatName, PersonT.[Address1] + ' ' + PersonT.[Address2] AS PatAdd, PersonT.City AS PatCity, PersonT.State AS PatState, PersonT.Zip AS PatZip, "
					"PersonT.HomePhone AS PatPhone, PersonT.BirthDate AS PatBD, PersonT.Gender AS PatGender, PersonT.SocialSecurity AS PatSSN, PatientsT.MaritalStatus AS PatMarStatus, PatientsT.Employment AS PatEmployment, "
					"LocationsT.Name AS PracName, LocationsT.Address1 AS PracAdd1, LocationsT.Address2 AS PracAdd2, LocationsT.[Phone] AS DocPhone, 'Signature On File' AS SignatureOnFile, GetDate() AS TodaysDate, [GRPNumber] AS GRP "
					"FROM PatientsT "
					"LEFT JOIN LocationsT ON PatientsT.InLocation = LocationsT.ID "
					"LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID ", strPatientID);

			where.Format (" WHERE PatientsT.PersonID = %li", m_PatientID);
			m_pframe->Load (1, where, "", &i, fee_group_id, &astrParams, &avarParams); // Patient information
		}

		// (j.jones 2007-02-28 17:36) - PLID 23297 - converted to use batched statements per form
		{
			CString strForm2Batch = BeginSqlBatch();

			// (j.jones 2007-07-12 15:33) - PLID 26636 - added ability to hide Box 24J NPI
			if(m_HCFAInfo.HideBox24JNPI == 0) {
				AddStatementToSqlBatch(strForm2Batch, "UPDATE FormControlsT SET Source = 'Box24JNPI' WHERE ID IN (5444, 5445, 5446, 5447, 5448, 5449)");
			}
			else {
				AddStatementToSqlBatch(strForm2Batch, "UPDATE FormControlsT SET Source = '' WHERE ID IN (5444, 5445, 5446, 5447, 5448, 5449)");
			}

			// (j.jones 2013-08-09 15:53) - PLID 57299 - don't bother doing this if on the new form, as these diagnosis
			// descriptions are hidden entirely
			if(!g_bUseNewHCFAForm) {
				if(m_HCFAInfo.ShowICD9Desc) {
					AddStatementToSqlBatch(strForm2Batch, "UPDATE FormControlsT SET Source = 'DiagCode1Desc' WHERE ID = 4399");
					AddStatementToSqlBatch(strForm2Batch, "UPDATE FormControlsT SET Source = 'DiagCode2Desc' WHERE ID = 4400");
					AddStatementToSqlBatch(strForm2Batch, "UPDATE FormControlsT SET Source = 'DiagCode3Desc' WHERE ID = 4401");
					AddStatementToSqlBatch(strForm2Batch, "UPDATE FormControlsT SET Source = 'DiagCode4Desc' WHERE ID = 4402");
				}
				else {
					AddStatementToSqlBatch(strForm2Batch, "UPDATE FormControlsT SET Source = '' WHERE ID >= 4399 AND ID <= 4402");
				}
			}

			if(!strForm2Batch.IsEmpty())
				ExecuteSqlBatch(strForm2Batch);

			CString strOrderBy = " ORDER BY LineID";

			// (j.jones 2007-02-28 17:42) - 23297 - this is outdated, and was never commented out fully,
			// potentially causing two unnecessary recordsets
			/*
			BOOL bAnesthesia = FALSE;

			//if an anesthesia code, and they are using anesthesia billing, and the POS doesn't use a flat fee, put anesthesia codes last
			if(m_HCFAInfo.ShowAnesthesiaTimes == 1
				&& ReturnsRecords("SELECT ID FROM LocationsT WHERE AnesthesiaFeeBillType <> 1 AND "
				"ID IN (SELECT Location FROM BillsT WHERE ID = %li)", billid)
				&& ReturnsRecords("SELECT ID FROM ServiceT WHERE Anesthesia = 1 AND UseAnesthesiaBilling = 1 AND "
				"ID IN (SELECT ServiceID FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"WHERE Deleted = 0 AND BillID = %li)", billid)) {

				bAnesthesia = TRUE;

				// (j.jones 2006-09-18 16:06) - PLID 22455 - no longer necessary
				//strOrderBy = " ORDER BY Anesthesia, LineID";
			}
			*/

			m_pframe->Load (2, "", strOrderBy, &i, fee_group_id); // Diagnosis codes			
		}

		m_pframe->Load (3, "", "", &i, fee_group_id); // Total Charge (box 28)
		m_pframe->Load (4, "", "", &i, fee_group_id); // Amount Paid (box 29

		// (j.jones 2007-02-28 17:36) - PLID 23297 - converted to use batched statements per form
		// (doesn't really affect Form 5 right now but will help in the future
		{
			CString strForm5Batch = BeginSqlBatch();

			//based on our setting, hide the total balance
			if(m_HCFAInfo.HideBox30) {
				AddStatementToSqlBatch(strForm5Batch, "UPDATE FormControlsT SET Source = '' WHERE ID = 433");
			}
			else {
				AddStatementToSqlBatch(strForm5Batch, "UPDATE FormControlsT SET Source = 'TotalBalance' WHERE ID = 433");
			}

			if(!strForm5Batch.IsEmpty())
				ExecuteSqlBatch(strForm5Batch);

			m_pframe->Load (5, "", "", &i, fee_group_id); // Balance (box 30)
		}

		{
			/////////////////////////////////////////////
			// Initialize parameter arrays for form 6
			astrParams.Add("ShowSignature");
			var.vt = VT_BOOL;

			if (m_HCFAInfo.Box31Show == 0) {
				var.vt = VT_BOOL;
				var.boolVal = false;
			}
			else {
				var.vt = VT_BOOL;
				var.boolVal = true;
			}

			avarParams.Add(var);

			m_pframe->Load (6, "", "", &i, fee_group_id, &astrParams, &avarParams); // Box 31
		}

		// (j.jones 2007-02-28 17:36) - PLID 23297 - converted to use batched statements per form
		{
			CString strForms20_21Batch = BeginSqlBatch();

			//Determine if we need to 
			BOOL bHideBox11Check = FALSE;
			BOOL bHideBox4 = FALSE;
			BOOL bHideBox7 = FALSE;
			BOOL bHideBox9 = FALSE;
			BOOL bHideBox11 = FALSE;
			// (j.jones 2007-04-09 16:48) - PLID 25337 - added HideBox11a/b/c, which split each box
			// out from a total "hide all of Box 11" option
			BOOL bHideBox11a_Birthdate = FALSE;
			BOOL bHideBox11a_Gender = FALSE;
			BOOL bHideBox11b = FALSE;
			BOOL bHideBox11c = FALSE;

			if (m_HCFAInfo.HideBox4 == 1)
				bHideBox4 = TRUE;

			if (m_HCFAInfo.HideBox7 == 1)
				bHideBox7 = TRUE;

			if (m_HCFAInfo.HideBox9 == 1)
				bHideBox9 = TRUE;

			if (m_HCFAInfo.HideBox11 == 1)
				bHideBox11 = TRUE;
			
			if (m_HCFAInfo.HideBox11a_Birthdate == 1)
				bHideBox11a_Birthdate = TRUE;

			if (m_HCFAInfo.HideBox11a_Gender == 1)
				bHideBox11a_Gender = TRUE;

			if (m_HCFAInfo.HideBox11b == 1)
				bHideBox11b = TRUE;

			if (m_HCFAInfo.HideBox11c == 1)
				bHideBox11c = TRUE;

			//hide/show Box 4, 7, 9, 11

			//Box 4
			if(bHideBox4) {
				AddStatementToSqlBatch(strForms20_21Batch, "UPDATE FormControlsT SET Source = '' WHERE ID = 92");
			}
			else {
				//check to see if they need the employer in Box 4
				if (m_HCFAInfo.UseEmp == 1)
					AddStatementToSqlBatch(strForms20_21Batch, "UPDATE FormControlsT SET Source = 'InsEmp' WHERE ID = 92");
				else
					AddStatementToSqlBatch(strForms20_21Batch, "UPDATE FormControlsT SET Source = 'InsName' WHERE ID = 92");
			}

			//Box 7
			if(bHideBox7) {
				AddStatementToSqlBatch(strForms20_21Batch, "UPDATE FormControlsT SET Source = '' WHERE ID = 96");
				AddStatementToSqlBatch(strForms20_21Batch, "UPDATE FormControlsT SET Source = '' WHERE ID = 119");
				AddStatementToSqlBatch(strForms20_21Batch, "UPDATE FormControlsT SET Source = '' WHERE ID = 123");
				AddStatementToSqlBatch(strForms20_21Batch, "UPDATE FormControlsT SET Source = '' WHERE ID = 121");
				AddStatementToSqlBatch(strForms20_21Batch, "UPDATE FormControlsT SET Source = '' WHERE ID = 125");
			}
			else {
				AddStatementToSqlBatch(strForms20_21Batch, "UPDATE FormControlsT SET Source = 'InsAdd' WHERE ID = 96");
				AddStatementToSqlBatch(strForms20_21Batch, "UPDATE FormControlsT SET Source = 'InsCity' WHERE ID = 119");
				AddStatementToSqlBatch(strForms20_21Batch, "UPDATE FormControlsT SET Source = 'InsState' WHERE ID = 123");
				AddStatementToSqlBatch(strForms20_21Batch, "UPDATE FormControlsT SET Source = 'InsZip' WHERE ID = 121");
				AddStatementToSqlBatch(strForms20_21Batch, "UPDATE FormControlsT SET Source = 'InsPhone' WHERE ID = 125");
			}

			// (j.jones 2006-10-31 15:49) - PLID 21845 - check the secondary insurance's HCFA group,
			// and see if we need to use the secondary insurance's information in Box 11. If so,
			// this will override the Hide Box 11 option
			m_bShowSecondaryInBox11 = FALSE;
			if(m_OthrInsuredPartyID != -1) {
				if(ReturnsRecords("SELECT ID FROM HCFASetupT WHERE SecondaryFillBox11 = 1 AND "
					"ID IN (SELECT HCFASetupGroupID FROM InsuranceCoT INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID "
					"WHERE InsuredPartyT.PersonID = %li)", m_OthrInsuredPartyID)) {
					//there is a secondary insurance and its HCFA group says to overwrite Box 11
					m_bShowSecondaryInBox11 = TRUE;
				}
			}

			//Box 11
			if(m_bShowSecondaryInBox11) {
				//override the "primary" insurance info with the "secondary" insurance info
				AddStatementToSqlBatch(strForms20_21Batch, "UPDATE FormControlsT SET Source = 'OthrInsIDForInsurance', FormID = 21 WHERE ID = 157");
				AddStatementToSqlBatch(strForms20_21Batch, "UPDATE FormControlsT SET Source = 'OthrInsBD', FormID = 21 WHERE ID = 159");
				AddStatementToSqlBatch(strForms20_21Batch, "UPDATE FormControlsT SET Source = 'OthrInsGender', FormID = 21 WHERE ID = 161 OR ID = 162");
				// (j.jones 2013-08-09 09:26) - PLID 57299 - if the new HCFA, don't change Box 11b
				if(!g_bUseNewHCFAForm) {
					AddStatementToSqlBatch(strForms20_21Batch, "UPDATE FormControlsT SET Source = 'OthrInsEmp', FormID = 21 WHERE ID = 166");
				}
				AddStatementToSqlBatch(strForms20_21Batch, "UPDATE FormControlsT SET Source = 'OthrInsPlan', FormID = 21 WHERE ID = 168");
			}
			else {
				// (j.jones 2007-04-09 16:53) - PLID 25537 - now the components of Box 11 are hidden separately
				// (Box 11d is handled further down)

				if(bHideBox11)
					AddStatementToSqlBatch(strForms20_21Batch, "UPDATE FormControlsT SET Source = '' WHERE ID = 157");
				else
					AddStatementToSqlBatch(strForms20_21Batch, "UPDATE FormControlsT SET Source = 'InsFECA', FormID = 20 WHERE ID = 157");

				if(bHideBox11a_Birthdate)
					AddStatementToSqlBatch(strForms20_21Batch, "UPDATE FormControlsT SET Source = '' WHERE ID = 159");
				else
					AddStatementToSqlBatch(strForms20_21Batch, "UPDATE FormControlsT SET Source = 'InsBD', FormID = 20 WHERE ID = 159");

				if(bHideBox11a_Gender)
					AddStatementToSqlBatch(strForms20_21Batch, "UPDATE FormControlsT SET Source = '' WHERE ID = 161 OR ID = 162");
				else
					AddStatementToSqlBatch(strForms20_21Batch, "UPDATE FormControlsT SET Source = 'InsGender', FormID = 20 WHERE ID = 161 OR ID = 162");

                // (a.levy 2014-01-14 08:56) - PLID - 60233 - corrected sql syntax error.
				if(bHideBox11b) {
					AddStatementToSqlBatch(strForms20_21Batch, "UPDATE FormControlsT SET Source = '' WHERE ID IN (166, 8691)");
				}
				else {
					// (j.jones 2013-08-09 09:26) - PLID 57299 - this box is now different for the new HCFA
					if(g_bUseNewHCFAForm) {					
						AddStatementToSqlBatch(strForms20_21Batch, "UPDATE FormControlsT SET Source = 'Box11bQual', FormID = 22 WHERE ID = 8691");
						AddStatementToSqlBatch(strForms20_21Batch, "UPDATE FormControlsT SET Source = 'Box11b', FormID = 22 WHERE ID = 166");
					}
					else {					
						AddStatementToSqlBatch(strForms20_21Batch, "UPDATE FormControlsT SET Source = '', FormID = -1 WHERE ID = 8691");
						AddStatementToSqlBatch(strForms20_21Batch, "UPDATE FormControlsT SET Source = 'InsEmp', FormID = 20 WHERE ID = 166");
					}
				}

				if(bHideBox11c)
					AddStatementToSqlBatch(strForms20_21Batch, "UPDATE FormControlsT SET Source = '' WHERE ID = 168");
				else
					AddStatementToSqlBatch(strForms20_21Batch, "UPDATE FormControlsT SET Source = 'InsPlan', FormID = 20 WHERE ID = 168");
			}

			//Box11D: 0 = fill normally, 1 = fill if no secondary, 2 = never fill
			if(m_HCFAInfo.Box11D == 2) {
				AddStatementToSqlBatch(strForms20_21Batch, "UPDATE FormControlsT SET Source = '' WHERE ID = 170 OR ID = 171");
			}
			else {
				AddStatementToSqlBatch(strForms20_21Batch, "UPDATE FormControlsT SET Source = 'Box11d' WHERE ID = 170 OR ID = 171");
			}

			//Box 9
			if(bHideBox9) {
				AddStatementToSqlBatch(strForms20_21Batch, "UPDATE FormControlsT SET Source = '' WHERE ID = 142");
				// (j.jones 2010-08-31 16:54) - PLID 40303 - like Use1aIn9a, TPLIn9a also overrides HideBox9
				if(!m_HCFAInfo.Use1aIn9a && m_HCFAInfo.TPLIn9a == 0)
					AddStatementToSqlBatch(strForms20_21Batch, "UPDATE FormControlsT SET Source = '', FormID = 21 WHERE ID = 144");
					//the "else" to this will be done later
				AddStatementToSqlBatch(strForms20_21Batch, "UPDATE FormControlsT SET Source = '' WHERE ID = 146");
				AddStatementToSqlBatch(strForms20_21Batch, "UPDATE FormControlsT SET Source = '' WHERE ID = 148 OR ID = 149");
				AddStatementToSqlBatch(strForms20_21Batch, "UPDATE FormControlsT SET Source = '' WHERE ID = 153");
				AddStatementToSqlBatch(strForms20_21Batch, "UPDATE FormControlsT SET Source = '' WHERE ID = 155");
			}
			else {
				AddStatementToSqlBatch(strForms20_21Batch, "UPDATE FormControlsT SET Source = 'OthrInsName' WHERE ID = 142");
				AddStatementToSqlBatch(strForms20_21Batch, "UPDATE FormControlsT SET Source = 'OthrInsIDForInsurance', FormID = 21 WHERE ID = 144");
				AddStatementToSqlBatch(strForms20_21Batch, "UPDATE FormControlsT SET Source = 'OthrInsGender' WHERE ID = 148 OR ID = 149");
				// (j.jones 2013-08-09 09:26) - PLID 57299 - 9b and 9c are now different for the new HCFA
				if(g_bUseNewHCFAForm) {					
					AddStatementToSqlBatch(strForms20_21Batch, "UPDATE FormControlsT SET Source = 'Box9b', FormID = 22 WHERE ID = 146");
					AddStatementToSqlBatch(strForms20_21Batch, "UPDATE FormControlsT SET Source = 'Box9c', FormID = 22 WHERE ID = 153");
				}
				else {					
					AddStatementToSqlBatch(strForms20_21Batch, "UPDATE FormControlsT SET Source = 'OthrInsBD', FormID = 21 WHERE ID = 146");
					AddStatementToSqlBatch(strForms20_21Batch, "UPDATE FormControlsT SET Source = 'OthrInsEmp', FormID = 21 WHERE ID = 153");
				}
				AddStatementToSqlBatch(strForms20_21Batch, "UPDATE FormControlsT SET Source = 'OthrInsPlan' WHERE ID = 155");
			}

			if (m_HCFAInfo.Use1aIn9a && g_nInsuredPartyID != -1) {
				//ensure the primary policy ID is used instead of secondary

				// (j.jones 2006-10-27 09:51) - PLID 23113 - to support the possibility of remapping
				// this field even when a secondary insurance doesn't exist, we'll just point the field
				// to Box 1a. Otherwise, use the normal field. The "else" statement here doesn't conflict
				// with the bHideBox9 stuff above because it would only be hidden if Use1aIn9a is not checked
				if(m_OthrInsuredPartyID != -1 || m_HCFAInfo.Use1aIn9aAlways)
					AddStatementToSqlBatch(strForms20_21Batch, "UPDATE FormControlsT SET Source = 'InsID', FormID = 20 WHERE ID = 144");
				else
					AddStatementToSqlBatch(strForms20_21Batch, "UPDATE FormControlsT SET Source = 'OthrInsIDForInsurance', FormID = 21 WHERE ID = 144");
			}
			// (j.jones 2010-08-31 16:54) - PLID 40303 - supported TPL in 9a, which is mutually exclusive of Use1aIn9a
			if(!m_HCFAInfo.Use1aIn9a && m_HCFAInfo.TPLIn9a == 1 && m_OthrInsuredPartyID != -1) {
				AddStatementToSqlBatch(strForms20_21Batch, "UPDATE FormControlsT SET Source = 'OthrInsTPLCode', FormID = 21 WHERE ID = 144");
			}

			//be sure to make our changes prior to LoadDefaultBox11Value!
			if(!strForms20_21Batch.IsEmpty())
				ExecuteSqlBatch(strForms20_21Batch);

			LoadDefaultBox11Value(20, g_nInsuredPartyID);
			LoadDefaultBox11Value(21, m_OthrInsuredPartyID);

			where.Format(" WHERE PatientID = %li AND InsuredPartyT.PersonID = %d", m_PatientID, g_nInsuredPartyID);
			m_pframe->Load (20, where, "", &i, fee_group_id); // Load the insurance information on the right side of the form

			where.Format (" WHERE PatientID = %li AND InsuredPartyT.PersonID = %d", m_PatientID, m_OthrInsuredPartyID);
			m_pframe->Load (21, where, "", &i, fee_group_id); // Load the "other" insurance information on the left side of the form
		}

		// (j.jones 2007-02-28 17:36) - PLID 23297 - converted to use batched statements per form
		{
			CString strForms33_34Batch = BeginSqlBatch();

			bool bHideBox32NameAdd = false;
			//check to see if we should hide box 32

			// (j.jones 2007-05-11 12:41) - PLID 25932 - reworked this to have options to 
			// hide just the name and address, and the IDs
			// (j.gruber 2007-06-28 16:49) - PLID 26154 - fixed the query for if POS is null
			// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" charges
			if(m_HCFAInfo.HideBox32NameAdd != 0) {
				//if HideBox32NameAdd = 1, they want to hide the name/add in box 32 if the POS is 11,
				//so check to see if any of the charges associated with this bill have a POS of 11
				//if HideBox32NameAdd = 2, they want to hide the name/add in box 32 if the POS is NOT 11,
				//so check to see if any of the charges associated with this bill do not have a POS of 11
				//if HideBox32NameAdd = 3, they want to always hide the name/add in box 32
				if(m_HCFAInfo.HideBox32NameAdd == 3 || (m_HCFAInfo.HideBox32NameAdd == 1
					&& ReturnsRecordsParam("SELECT PlaceOfServiceCodesT.PlaceCodes FROM ChargesT "
						"INNER JOIN LINEITEMT ON ChargesT.ID = LineItemT.ID "
						"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
						"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
						"LEFT JOIN PlaceOfServiceCodesT ON ChargesT.ServiceLocationID = PlaceOfServiceCodesT.ID "
						"WHERE BillID = {INT} AND Deleted = 0 AND Batched = 1 "
						"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
						"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
						"AND PlaceCodes = '11'", billid)) ||
					
					(m_HCFAInfo.HideBox32NameAdd == 2
						&& ReturnsRecordsParam("SELECT PlaceOfServiceCodesT.PlaceCodes FROM ChargesT "
							"INNER JOIN LINEITEMT ON ChargesT.ID = LineItemT.ID "
							"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
							"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
							"LEFT JOIN PlaceOfServiceCodesT ON ChargesT.ServiceLocationID = PlaceOfServiceCodesT.ID "
							"WHERE BillID = {INT} AND Deleted = 0 AND Batched = 1 "
							"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
							"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
							"AND (PlaceCodes <> '11' OR PlaceCodes IS NULL) ", billid))) {

					bHideBox32NameAdd = true;
				}
				else{
					bHideBox32NameAdd = false;
				}
			}
			else{
				bHideBox32NameAdd = false;
			}

			bool bHideBox32NPIID = false;
			//check to see if we should hide box 32

			// (j.jones 2007-05-11 12:02) - PLID 25932 - reworked this to have options to 
			// hide just the name and address, and the IDs
			// (j.gruber 2007-06-28 16:49) - PLID 26154 - fixed the query for if POS is null
			// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" charges
			if(m_HCFAInfo.HideBox32NPIID != 0){
				//if HideBox32NPIID = 1, they want to hide the IDs in box 32 if the POS is 11,
				//so check to see if any of the charges associated with this bill have a POS of 11
				//if HideBox32NPIID = 2, they want to hide the IDs in box 32 if the POS is NOT 11,
				//so check to see if any of the charges associated with this bill do not have a POS of 11
				//if HideBox32NPIID = 3, they want to always hide the IDs in box 32
				if(m_HCFAInfo.HideBox32NPIID == 3 || (m_HCFAInfo.HideBox32NPIID == 1
					&& ReturnsRecordsParam("SELECT PlaceOfServiceCodesT.PlaceCodes FROM ChargesT "
						"INNER JOIN LINEITEMT ON ChargesT.ID = LineItemT.ID "
						"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
						"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
						"LEFT JOIN PlaceOfServiceCodesT ON ChargesT.ServiceLocationID = PlaceOfServiceCodesT.ID "
						"WHERE BillID = {INT} AND Deleted = 0 AND Batched = 1 "
						"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
						"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
						"AND PlaceCodes = '11'", billid)) ||
					
					(m_HCFAInfo.HideBox32NPIID == 2
						&& ReturnsRecordsParam("SELECT PlaceOfServiceCodesT.PlaceCodes FROM ChargesT "
							"INNER JOIN LINEITEMT ON ChargesT.ID = LineItemT.ID "
							"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
							"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
							"LEFT JOIN PlaceOfServiceCodesT ON ChargesT.ServiceLocationID = PlaceOfServiceCodesT.ID "
							"WHERE BillID = {INT} AND Deleted = 0 AND Batched = 1 "
							"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
							"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
							"AND (PlaceCodes <> '11' OR PlaceCodes IS NULL) ", billid))) {

					bHideBox32NPIID = true;
				}
				else{
					bHideBox32NPIID = false;
				}
			}
			else{
				bHideBox32NPIID = false;
			}

			if(bHideBox32NameAdd){
				//hide the info in box 32
				AddStatementToSqlBatch(strForms33_34Batch, "UPDATE FormControlsT SET Source = '' WHERE ID IN (401,402,404)");
			}
			else {
				//show the info in box 32
				AddStatementToSqlBatch(strForms33_34Batch, "UPDATE FormControlsT SET Source = 'HospName' WHERE ID = 401");
				AddStatementToSqlBatch(strForms33_34Batch, "UPDATE FormControlsT SET Source = 'HospAdd' WHERE ID = 402");
				AddStatementToSqlBatch(strForms33_34Batch, "UPDATE FormControlsT SET Source = 'HospCityStateZip' WHERE ID = 404");
			}	

			//see if they want the IDs shown or not
			if(bHideBox32NPIID) {
				AddStatementToSqlBatch(strForms33_34Batch, "UPDATE FormControlsT SET Source = '' WHERE ID IN (5427,3549)");
			}
			else{
				//show the IDs in box 32
				AddStatementToSqlBatch(strForms33_34Batch, "UPDATE FormControlsT SET Source = 'HospNPI' WHERE ID = 5427");
				AddStatementToSqlBatch(strForms33_34Batch, "UPDATE FormControlsT SET Source = 'FacilityID' WHERE ID = 3549");
			}
			
			//check to see if we should hide the phone number in Box 33
			if(m_HCFAInfo.HidePhoneBox33 == 1){ //Hide phone
				AddStatementToSqlBatch(strForms33_34Batch, "UPDATE FormControlsT SET Source = '' WHERE ID = 409");
			}
			else {
				AddStatementToSqlBatch(strForms33_34Batch, "UPDATE FormControlsT SET Source = 'DocPhone' WHERE ID = 409");
			}

			if(!strForms33_34Batch.IsEmpty())
				ExecuteSqlBatch(strForms33_34Batch);
			
			m_pframe->Load (33, "", "", &i, fee_group_id); // HCFA Address (Box 33)
			m_pframe->Load (34, "", "", &i, fee_group_id, &astrParams, &avarParams); // PIN number (Box 33)
		}

		// This code looks to see if Box11d is not checked as true. If so, then
		// we have to check Box11d as false.
		// (j.jones 2010-03-15 15:19) - PLID 37719 - removed unnecessary log
		//LogDetail("HCFADlg: Manually checking box 11d");
		// (j.armen 2014-03-27 16:28) - PLID 60784 - control array is now a vector
		for (unsigned int j = 0; j < m_pframe->m_ControlArray.size(); j++) {
			FormControl* pCtrl = m_pframe->m_ControlArray[j].get();
			if (pCtrl->id == 170) {
				FormCheck* pCheck = (FormCheck*)pCtrl;
				if (!pCheck->GetCheck()) {
					for (unsigned int k = 0; k < m_pframe->m_ControlArray.size(); k++) {
						pCtrl = m_pframe->m_ControlArray[k].get();
						if (pCtrl->id == 171) {
							pCheck = (FormCheck*)pCtrl;
							if(m_HCFAInfo.Box11D == 0)
								pCheck->SetCheck(1);
							else
								pCheck->SetCheck(0);
							break;
						}
					}
				}
				break;
			}
		}

		////////////////////////////////////////////////////////
		// Form 22
		where.Format (" WHERE BillsT.[ID] = %i;", m_ID);	
		m_pframe->Load (22, where, "", &i, fee_group_id); // Load the bill information

		//////////////////////////////////////////////////
		//Add buttons
		// (j.jones 2010-03-15 15:19) - PLID 37719 - removed unnecessary log
		//LogDetail("HCFADlg: Adding buttons");

#define scale * 19 / 20
		rect.left	= 509 scale;
		rect.right	= 662 scale;
		rect.top	= 1052 scale;
		rect.bottom = 1067 scale;
		if (!m_pleftbutton->Create ("<", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, rect, m_pframe, IDC_PHYSICIAN_LEFT))
			AfxMessageBox ("Failed");
		rect.left	= 662 scale;
		rect.right	= 816 scale;
		if (!m_prightbutton->Create (">", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, rect, m_pframe, IDC_PHYSICIAN_RIGHT))
			AfxMessageBox ("Failed");
		rect.left	= 6 scale;
		rect.right	= 20 scale;
		rect.top	= 702 scale;
		rect.bottom = 817 scale;
		if (!m_pupbutton->Create ("/\\", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, rect, m_pframe, IDC_BILL_UP))
			AfxMessageBox ("Failed");
		rect.top	= 817 scale;
		rect.bottom = 932 scale;
		if (!m_pdownbutton->Create ("\\/", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, rect, m_pframe, IDC_BILL_DOWN))
			AfxMessageBox ("Failed");

		//set printer settings
		PRINT_X_SCALE			= 15.1;
		PRINT_Y_SCALE			= 15.1;
		PRINT_X_OFFSET	= -100;
		PRINT_Y_OFFSET	= 200;//150;
		// Added by Chris
		m_pframe->m_pOnCommand = m_pOnCommand;
		m_pframe->m_pPrePrintFunc = m_pPrePrintFunc;
		
		m_pframe->TrackChanges(0, 0);

	}NxCatchAll("Error in HCFADlg::OnInitDialog 2");

	return TRUE;
}

BEGIN_EVENTSINK_MAP(CHCFADlg, CDialog)
    //{{AFX_EVENTSINK_MAP(CHCFADlg)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CHCFADlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	SetControlPositions();
}

void CHCFADlg::SetControlPositions(void)
{	
	RECT rect;

	if (m_pframe)
	{	GetClientRect(&rect);
		rect.top += BUTTON_HEIGHT + TEXT_HEIGHT;
		m_pframe->MoveWindow(&rect, false);

		ScrollBottomPos = 1250 + rect.top - rect.bottom;

		if(GetSystemMetrics(SM_CXFULLSCREEN) <= 800) {
			SetDlgItemText(IDC_CAP_ON_PRINT,"Capitalize");
		}

		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_DISABLENOSCROLL|SIF_PAGE|SIF_RANGE;
		si.nMin = 0;
		si.nMax = ScrollBottomPos;
		si.nPage = SCROLL_POS_PER_PAGE;
		SetScrollInfo(SB_VERT, &si, TRUE);

		// (j.jones 2007-06-22 12:27) - PLID 25665 - added info text label control
		CRect rcInfo = rect;
		rcInfo.bottom = rcInfo.top;
		rcInfo.top = rcInfo.top - TEXT_HEIGHT;
		GetDlgItem (IDC_INFO_TEXT)->MoveWindow(&rcInfo, false);

		rect.left = rect.right;

		rect.bottom = rect.top - TEXT_HEIGHT;
		rect.top -= (BUTTON_HEIGHT + TEXT_HEIGHT);
		rect.left = rect.right - BUTTON_WIDTH;
		GetDlgItem (IDC_X)->MoveWindow(&rect, false);

		rect.right = rect.left;
		rect.left -= (long)(BUTTON_WIDTH * 1.6);
		GetDlgItem (IDC_CHECK)->MoveWindow(&rect, false);

		rect.right = rect.left;
		rect.left -= BUTTON_WIDTH;
		GetDlgItem (IDC_PRINT)->MoveWindow(&rect, false);

		rect.right = rect.left;
		rect.left -= BUTTON_WIDTH;
		GetDlgItem (IDC_ALIGN)->MoveWindow(&rect, false);

		rect.right = rect.left;
		rect.left -= BUTTON_WIDTH;
		GetDlgItem (IDC_RESTORE)->MoveWindow(&rect, false);

		rect.right = rect.left;
		rect.left -= BUTTON_WIDTH;
		//GetDlgItem (IDC_RELOAD)->MoveWindow(&rect, false);
		GetDlgItem (IDC_RELOAD)->MoveWindow(0,0,0,0, false);

		Invalidate();
	}
}

extern int FONT_SIZE;
extern int MINI_FONT_SIZE;

int CHCFADlg::DoModal(int billID) 
{
	m_ID = billID;
	return CDialog::DoModal();
}

int SizeOfFormChargesT()
{
	COleVariant var;
	CString str, strSQL;
	int nCharges = 0;

     // (a.levy 2013-10-18 11:12) - PLID 59043 -Changed to CreateRecordSetStd  --CreateRecordSetStd wont escape the % sign and try to format it with garbage
	_RecordsetPtr rs = CreateRecordsetStd(g_aryFormQueries[2].sql);
	while(!rs->eof) {
		nCharges++;
		rs->MoveNext();
	}

	rs->Close();

	return nCharges; 
}

void CHCFADlg::Save(BOOL boSaveAll, CDialog* pdlgWait, int& iPage)
{
	try {

		_RecordsetPtr nxrs;
		CString str, str1, strBox33bNum = "''";

		CString strServiceDateTo = "ChargesT.ServiceDateTo";

		// (j.jones 2007-04-10 08:40) - PLID 25539 - added ability to hide Box 24A Service Date To
		if(m_HCFAInfo.Hide24ATo == 1) {
			strServiceDateTo = "NULL";
		}

		// (j.jones 2014-05-01 10:48) - PLID 61839 - get the provider ID out of the current page info
		long nProviderID = GetCurrentPageProviderID();

		// (j.jones 2007-02-21 12:36) - PLID 24776 - supported QualifierSpace
		nxrs = CreateParamRecordset("SELECT CASE WHEN PIN IS NULL THEN NULL ELSE (CASE WHEN Box33bQual IS NULL THEN '' ELSE Box33bQual END) + {STRING} + PIN END AS Box33b "
			"FROM AdvHCFAPinT WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT}",
			m_HCFAInfo.QualifierSpace == 1 ? " " : "", nProviderID, GetBillLocation(billid), fee_group_id);
		if(!nxrs->eof) {
			_variant_t v = nxrs->Fields->Item["Box33b"]->Value;
			if(v.vt == VT_BSTR)
				strBox33bNum = "'" + _Q(CString(v.bstrVal)) + "'";
		}
		nxrs->Close();

		// (j.jones 2010-04-08 14:24) - PLID 38094 - ChargesT.IsEmergency determines whether
		// we send the setting per charge, or use the HCFA Setup value.
		CString strBox24C = "''";
		strBox24C.Format("CASE WHEN (ChargesT.IsEmergency = %li OR (ChargesT.IsEmergency = %li AND %li = 2)) THEN 'N' "
			"WHEN (ChargesT.IsEmergency = %li OR (ChargesT.IsEmergency = %li AND %li = 1)) THEN 'Y' "
			"ELSE '' END",
			(long)cietNo, (long)cietUseDefault, m_HCFAInfo.Box24C, 
			(long)cietYes, (long)cietUseDefault, m_HCFAInfo.Box24C);
		
		for (pageinfo_index = 0; pageinfo_index < aryPageInfo.size(); pageinfo_index++) {

			PageInfoPtr pageInfo = aryPageInfo[pageinfo_index];

			////////////////////////////////////////////////////////////
			// Rebuild the charge list with the first provider

			// (j.jones 2014-05-02 10:07) - PLID 61839 - moved the query to a modular function
			// (j.jones 2014-09-30 10:06) - PLID 63793 - changed to take in the BillID to streamline loading Box 24I/J fields
			g_aryFormQueries[2].sql = GenerateChargesQuery(billid, strServiceDateTo, strBox24C, strBox33bNum, pageinfo_index);

			////////////////////////////////////////////////////////////
			// Fill total charge amount for each line
			nTotalCharges = SizeOfFormChargesT();

			for (firstcharge=0; firstcharge < nTotalCharges; firstcharge+=nPageIncrement) {

				if (pdlgWait) {
					str.Format("Saving HCFA page %d...", ++iPage);
					pdlgWait->GetDlgItem(IDC_LABEL)->SetWindowText(str);
					pdlgWait->RedrawWindow();
				}

				SaveHistoricalData(boSaveAll);
			}
		}

	}NxCatchAll("Error saving HCFA form.");
}

// (j.jones 2013-08-07 10:23) - PLID 57299 - removed GetHCFAGroupID()
// (j.jones 2014-05-05 15:43) - PLID 61993 - this now takes in a Box17 person ID & qualifer override,
// which is -1 if we're just supposed to use the bill's referring physician
void BuildFormsT_Form22(long nProviderID, long nLocationID, long nBox17PersonID, CString strBox17PersonQualifier, bool bBox17_IsAProvider)
{
	CString strSQL, str;

	try {

		CString strPriorAuthNum = "";
		CString strDefBox19 = "";
		long Box17Order = 1;
		long UseBox23InBox17a = 0;
		long nQualifierSpace = 0;
		long nHideBox17b = 0;
		long nPullCurrentAccInfo = 1;
		long nUseRefPhyGroupNPI = 0;
		// (j.jones 2007-02-21 12:26) - PLID 24776 - added QualifierSpace
		// (j.jones 2009-01-06 09:40) - PLID 32614 - added HideBox17b, and parameterized
		// (j.jones 2009-03-06 12:27) - PLID 33354 - added PullCurrentAccInfo
		// (j.jones 2013-06-10 17:06) - PLID 56255 - added UseRefPhyGroupNPI
		_RecordsetPtr rs = CreateParamRecordset("SELECT DefaultPriorAuthNum, Box19, Box17Order, UseBox23InBox17a, "
			"QualifierSpace, HideBox17b, PullCurrentAccInfo, UseRefPhyGroupNPI "
			"FROM HCFASetupT WHERE ID = {INT}", fee_group_id);
		if (!rs->eof) {
			strPriorAuthNum = AdoFldString(rs, "DefaultPriorAuthNum","");
			strDefBox19 = AdoFldString(rs, "Box19","");
			Box17Order = AdoFldLong(rs, "Box17Order",1);
			UseBox23InBox17a = AdoFldLong(rs, "UseBox23InBox17a",0);
			nQualifierSpace = AdoFldLong(rs, "QualifierSpace",0);
			nHideBox17b = AdoFldLong(rs, "HideBox17b",0);
			nPullCurrentAccInfo = AdoFldLong(rs, "PullCurrentAccInfo", 1);
			nUseRefPhyGroupNPI = AdoFldLong(rs, "UseRefPhyGroupNPI", 0);
		}
		rs->Close();

		// (j.jones 2007-02-28 17:36) - PLID 23297 - converted to use batched statements per form
		CString strForm22Batch = BeginSqlBatch();
		
		//re-set the ref. phy info.
		// (j.jones 2009-01-06 09:39) - PLID 32614 - supported hiding box 17b, the NPI
		if(nHideBox17b == 1) {
			AddStatementToSqlBatch(strForm22Batch, "UPDATE FormControlsT SET Source = '', FormID = 22 WHERE ID = 5420");
		}
		else {
			AddStatementToSqlBatch(strForm22Batch, "UPDATE FormControlsT SET Source = 'Box17bNPI', FormID = 22 WHERE ID = 5420");
		}
		AddStatementToSqlBatch(strForm22Batch, "UPDATE FormControlsT SET Source = 'Box17aData', FormID = 22 WHERE ID = 238");

		if(Box17Order == 1)
			AddStatementToSqlBatch(strForm22Batch, "UPDATE FormControlsT SET Source = 'RefNameLFM', FormID = 22 WHERE ID = 237");
		else
			AddStatementToSqlBatch(strForm22Batch, "UPDATE FormControlsT SET Source = 'RefNameFML', FormID = 22 WHERE ID = 237");
		
		//if UseBox23InBox17a = 0, then PriorAuthNum is in 23, normal RefPhyID in 17a
		if(UseBox23InBox17a == 0) {
			AddStatementToSqlBatch(strForm22Batch, "UPDATE FormControlsT SET Source = 'Box17aData', FormID = 22 WHERE ID = 238");
			AddStatementToSqlBatch(strForm22Batch, "UPDATE FormControlsT SET Source = 'PriorAuthNum' WHERE ID = 376");
		}
		//if UseBox23InBox17a = 1, then PriorAuthNum is in 23 and also in 17a
		else if(UseBox23InBox17a == 1) {
			AddStatementToSqlBatch(strForm22Batch, "UPDATE FormControlsT SET Source = 'PriorAuthNum', FormID = 22 WHERE ID = 238");
			AddStatementToSqlBatch(strForm22Batch, "UPDATE FormControlsT SET Source = 'PriorAuthNum' WHERE ID = 376");
		}
		//if UseBox23InBox17a = 2, then PriorAuthNum is in 17a and not in 23
		else if(UseBox23InBox17a == 2) {
			AddStatementToSqlBatch(strForm22Batch, "UPDATE FormControlsT SET Source = 'PriorAuthNum', FormID = 22 WHERE ID = 238");
			AddStatementToSqlBatch(strForm22Batch, "UPDATE FormControlsT SET Source = '' WHERE ID = 376");
		}

		// (j.jones 2004-07-14 18:00) - don't forget to change the control in the loaded arrays to be form 22,
		//because it may be already loaded as form 33 for one doctor
		if(m_pframe) {
			for (unsigned int j = 0; j < m_pframe->m_ControlArray.size(); j++) {
				FormControl* pCtrl = m_pframe->m_ControlArray[j].get();

				// (j.jones 2006-11-08 14:45) - PLID 23378 - change the controls for 238 and 5420
				if (pCtrl->id == 238 || pCtrl->id == 5420) {
					pCtrl->form = 22;
				}
			}
		}

		long nBox19RefPhyID = 0;
		long nBox19Override = 0;
		long nBox17a = 0;		
		CString strBox17aUse = "''";
		rs = CreateParamRecordset("SELECT Box19RefPhyID, Box19Override, Box17a, Box17aQual FROM HCFASetupT WHERE ID = {INT}", fee_group_id);
		if (!rs->eof) {
			nBox19RefPhyID = rs->Fields->Item["Box19RefPhyID"]->Value.lVal;
			nBox19Override = rs->Fields->Item["Box19Override"]->Value.lVal;
			nBox17a = rs->Fields->Item["Box17a"]->Value.lVal;
			CString strBox17aQual = AdoFldString(rs, "Box17aQual", "");

			// (j.jones 2007-03-15 15:21) - PLID 25225 - ensure no qualifier is used if Box17a is not filled
			// by a referring physician or by a PriorAuthNum
			if(UseBox23InBox17a == 0 && nBox17a != -1) {				
				strBox17aUse.Format("CASE WHEN Coalesce(Box17DoctorQ.ReferringPhyID,'') = '' THEN '' ELSE '%s' END", _Q(strBox17aQual));
			}
			else if(UseBox23InBox17a == 1 || UseBox23InBox17a == 2) {
				strBox17aUse.Format("CASE WHEN PriorAuthNum = '' THEN '' ELSE '%s' END", _Q(strBox17aQual));
			}
		}
		rs->Close();

		//decide what to fill in Box 19
		CString strBox19 = "BillsT.HCFABlock19";

		if(nBox19RefPhyID == 1) {
			strBox19 = "CASE WHEN BillsT.HCFABlock19 = ' ' OR BillsT.HCFABlock19 = '' OR BillsT.HCFABlock19 IS Null THEN Box17DoctorQ.ReferringPhyID ELSE BillsT.HCFABlock19 END";
		}
		else if(nBox19Override == 1) {
			strBox19.Format("CASE WHEN BillsT.HCFABlock19 = ' ' OR BillsT.HCFABlock19 = '' OR BillsT.HCFABlock19 IS Null THEN '%s' ELSE BillsT.HCFABlock19 END",_Q(strDefBox19));
		}

		// (j.jones 2013-04-26 13:36) - PLID 56453 - Get the POS code in this query,
		// since it is already finding a field tied to the first charge.
		CString strPOSCode = "";
		rs = CreateParamRecordset("SELECT TOP 1 AdvHCFAPinQ.Box19, PlaceOfServiceCodesT.PlaceCodes "
			"FROM ChargesT "
			"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"LEFT JOIN (SELECT Box19, LocationID FROM AdvHCFAPinT WHERE SetupGroupID = {INT} AND ProviderID = {INT}) AS AdvHCFAPinQ ON LineItemT.LocationID = AdvHCFAPinQ.LocationID "
			"LEFT JOIN PlaceOfServiceCodesT ON ChargesT.ServiceLocationID = PlaceOfServiceCodesT.ID "
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "		
			"WHERE BillsT.ID = {INT} AND LineItemT.Deleted = 0 AND ChargesT.Batched = 1 "
			"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
			"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null", fee_group_id, nProviderID, billid);
		if(!rs->eof) {
			strPOSCode = AdoFldString(rs, "PlaceCodes", "");
			_variant_t var = rs->Fields->Item["Box19"]->Value;
			if(var.vt == VT_BSTR && VarString(var).GetLength() > 0)
				strBox19.Format("'%s'",_Q(VarString(var)));
		}
		rs->Close();

		// (j.jones 2013-04-26 15:04) - PLID 56453 - we now send the patient address as the POS
		//if its POS code is 12 and the Claim_SendPatAddressWhenPOS12 preference is on
		BOOL bUsePatientAddressASPOS = FALSE;
		if(strPOSCode == "12" && GetRemotePropertyInt("Claim_SendPatAddressWhenPOS12", 1, 0, "<None>", true) == 1) {
			bUsePatientAddressASPOS = TRUE;
		}

		CString strFacilityID = "";
		if(!bUsePatientAddressASPOS) {
			//the facility ID needs the qualifier and ID as one long number
			// (j.jones 2007-02-21 12:31) - PLID 24776 - supported putting a space between the qualifier and the ID
			rs = CreateParamRecordset("SELECT Qualifier + {STRING} + FacilityID AS FacilityID FROM InsuranceFacilityID "
				"INNER JOIN InsuredPartyT ON InsuranceFacilityID.InsCoID = InsuredPartyT.InsuranceCoID "
				"INNER JOIN BillsT ON InsuredPartyT.PersonID = BillsT.InsuredPartyID "
				"WHERE InsuranceFacilityID.LocationID = BillsT.Location AND BillsT.ID = {INT}", nQualifierSpace == 1 ? " " : "", billid);
			if(!rs->eof) {
				_variant_t var = rs->Fields->Item["FacilityID"]->Value;
				if(var.vt != VT_NULL && var.vt != VT_EMPTY)
					strFacilityID = CString(var.bstrVal);
			}
			rs->Close();
		}

		// (j.jones 2009-02-23 11:12) - PLID 33167 - get the rendering provider ID, which is the Box 24J provider ID,
		// to pass into the CLIA lookup
		long nBox24JProviderID = -1;
		mapBox24JProviderIDs.Lookup(nProviderID, nBox24JProviderID);
		if(nBox24JProviderID == -1) {
			//this shouldn't be -1, but if it is, use the provider ID
			nBox24JProviderID = nProviderID;
		}

		// (j.jones 2009-02-06 16:29) - PLID 32951 - this function now requires the bill location
		// (j.jones 2009-02-23 10:54) - PLID 33167 - now we require the billing provider ID and rendering provider ID
		// (j.jones 2011-04-05 13:39) - PLID 42372 - this now returns a struct of information
		ECLIANumber eCLIANumber = UseCLIANumber(billid, g_nInsuredPartyID, nProviderID, nBox24JProviderID, nLocationID);
		if(eCLIANumber.bUseCLIANumber) {
			if(eCLIANumber.bUseCLIAInHCFABox23) {
				strPriorAuthNum = eCLIANumber.strCLIANumber;
			}

			if(eCLIANumber.bUseCLIAInHCFABox32) {
				strFacilityID = eCLIANumber.strCLIANumber;
			}

			// (j.jones 2014-05-05 15:43) - PLID 61993 - if given a Box17 Person ID, do not
			// follow the CLIA Box 17 setting
			if(nBox17PersonID == -1 && eCLIANumber.bCLIAUseBillProvInHCFABox17) {
				
				//switch the Referring Phys. data to the provider data

				// (j.jones 2006-11-06 10:39) - PLID 23355 - pull Box 33a into Box17b, pull Box33b into Box17a

				//first, Box17 name

				if(Box17Order == 1)
					AddStatementToSqlBatch(strForm22Batch, "UPDATE FormControlsT SET Source = 'DocNameLFM', FormID = 22 WHERE ID = 237");
				else
					AddStatementToSqlBatch(strForm22Batch, "UPDATE FormControlsT SET Source = 'DocNameFML', FormID = 22 WHERE ID = 237");

				//next Box33a -> Box17b

				CString strIDType = "UPIN"; //set in BuildHCFABox33, which is called first
				_RecordsetPtr rs = CreateRecordsetStd("SELECT Source FROM FormControlsT WHERE ID = 420");
				if(!rs->eof) {
					strIDType = AdoFldString(rs, "Source","UPIN");
				}
				rs->Close();
				// (j.jones 2009-01-06 09:39) - PLID 32614 - supported hiding box 17b, the NPI
				if(nHideBox17b == 1) {
					AddStatementToSqlBatch(strForm22Batch, "UPDATE FormControlsT SET Source = '', FormID = 34 WHERE ID = 5420");
				}
				else {
					AddStatementToSqlBatch(strForm22Batch, "UPDATE FormControlsT SET Source = '%s', FormID = 34 WHERE ID = 5420",strIDType);
				}

				//last, Box33b -> Box17a
				rs = CreateParamRecordset("SELECT Box33, Box33bQual FROM HCFASetupT WHERE ID = {INT}", fee_group_id);
				long Box33 = 1;
				CString strBox33bQual = "";
				if (!rs->eof) {
					Box33 = rs->Fields->Item["Box33"]->Value.lVal;
					// (j.jones 2007-03-15 15:34) - PLID 25225 - is using CLIA, set the Box17Qualifier
					// to definitely use the 33b qualifier - would be a bad setup if there was no 33b number
					strBox17aUse.Format("'%s'", _Q(AdoFldString(rs, "Box33bQual","")));
				}
				rs->Close();
				
				CString strBox17aSource;

				switch (Box33) {
					case 1: strBox17aSource = "NPI"; break;
					case 2: strBox17aSource = "SocialSecurity"; break;
					case 3: strBox17aSource = "[Fed Employer ID]"; break;
					case 4: strBox17aSource = "[DEA Number]"; break;
					case 5: strBox17aSource = "[BCBS Number]"; break;
					case 6: strBox17aSource = "[Medicare Number]"; break;
					case 7: strBox17aSource = "[Medicaid Number]"; break;
					case 8: strBox17aSource = "[Workers Comp Number]"; break;
					case 9: strBox17aSource = "[Other ID Number]"; break;
					case 11: strBox17aSource = "UPIN"; break;
					case 12: strBox17aSource = "Box24J"; break;
					// (j.jones 2007-01-16 15:24) - PLID 24273 - added support for old GRP number
					case 13: strBox17aSource = "[GRPNumber]"; break;
					// (j.jones 2007-04-24 12:15) - PLID 25764 - supported Taxonomy Code
					case 14: strBox17aSource = "TaxonomyCode"; break;
					case 16: strBox17aSource = "CustomData1.TextParam"; break;
					case 17: strBox17aSource = "CustomData2.TextParam"; break;
					case 18: strBox17aSource = "CustomData3.TextParam"; break;
					case 19: strBox17aSource = "CustomData4.TextParam"; break;
					default:
						strBox17aSource = "''";
						strBox17aUse = "''";
						break;
				}

				AddStatementToSqlBatch(strForm22Batch, "UPDATE FormControlsT SET Source = '%s', FormID = 34 WHERE ID = 238",strBox17aSource);

				// (j.jones 2004-07-14 18:00) - don't forget to change the control in the loaded arrays to be form 34,
				//because it may be already loaded as form 22 for one doctor
				if(m_pframe) {
					for (unsigned int j = 0; j < m_pframe->m_ControlArray.size(); j++) {
						FormControl* pCtrl = m_pframe->m_ControlArray[j].get();

						// (j.jones 2006-11-08 14:45) - PLID 23378 - change the controls for 238 and 5420
						if (pCtrl->id == 238 || pCtrl->id == 5420) {
							pCtrl->form = 34;
						}
					}
				}				
			}
		}

		//and now execute our batched statements
		if(!strForm22Batch.IsEmpty()) {
			ExecuteSqlBatch(strForm22Batch);
		}

		CString strPOSFields;
		// (j.jones 2013-04-26 15:04) - PLID 56453 - we now send the patient address as the POS
		//if its POS code is 12 and the Claim_SendPatAddressWhenPOS12 preference is on
		if(bUsePatientAddressASPOS) {
			//use the patient's address for the POS fields
			strPOSFields = "'Patient Home' AS HospName, PatientPersonT.Address1 + ' ' + PatientPersonT.Address2 AS HospAdd, "
				"PatientPersonT.City + ', ' + PatientPersonT.State + ' ' + PatientPersonT.Zip AS HospCityStateZip, '' AS HospNPI, '' AS FacilityID, ";
		}
		else {
			//use the POS address for the POS fields
			strPOSFields.Format("LocationsT.Name AS HospName, LocationsT.Address1 + ' ' + LocationsT.Address2 AS HospAdd, "
				"LocationsT.City + ', ' + LocationsT.State + ' ' + LocationsT.Zip AS HospCityStateZip, LocationsT.NPI AS HospNPI, '%s' AS FacilityID, ", _Q(strFacilityID));
		}

		// (j.jones 2014-05-05 15:39) - PLID 61993 - Box 17 now loads the provided Box17PersonID if given,
		// else continues the old Referring Physician Box 17 logic if the provided ID is -1
		CString strBox17PersonJoin = "LEFT JOIN ReferringPhysT AS Box17DoctorQ ON BillsT.RefPhyID = Box17DoctorQ.PersonID";
		CString strBox17AQualifier = "";
		if (nBox17PersonID != -1) {
			if (bBox17_IsAProvider) {
				strBox17PersonJoin.Format("CROSS JOIN (SELECT ProvidersT.*, '' AS GroupNPI FROM ProvidersT WHERE PersonID = %li) AS Box17DoctorQ", nBox17PersonID);
			}
			else {
				strBox17PersonJoin.Format("CROSS JOIN (SELECT ReferringPhysT.* FROM ReferringPhysT WHERE PersonID = %li) AS Box17DoctorQ", nBox17PersonID);
			}
			strBox17AQualifier = strBox17PersonQualifier;
		}
		if (strBox17AQualifier == "") {
			strBox17AQualifier = "DN";
		}

		// (j.jones 2006-12-01 13:01) - PLID 22110 - supported the ClaimProviderID override
		// (j.jones 2008-04-04 09:16) - PLID 28995 - this query takes in a provider ID, which is already the correct claim provider ID
		// (j.jones 2009-03-06 12:27) - PLID 33354 - added ability to load the current accident info. from the insured party
		// (j.jones 2013-06-10 17:08) - PLID 56255 - if they want to use the Ref. Phys. Group NPI, try to use it,
		// but revert to the regular NPI if the group NPI is blank
		// (j.jones 2013-08-14 14:57) - PLID 57299 - added Box8, Box9b, Box9c, Box11bQual, Box11b, Box17Qual, Box14Qual, Box15Qual
		// (j.jones 2013-08-16 14:15) - PLID 58069 - added many more condition type options for Box15 qualifier
		// (j.jones 2014-07-17 11:51) - PLID 62928 - added support for 439 - accident in Box 15
		// (r.gonet 2016-04-12) - NX-100073 - Switched out the old FirstConditionDate for a case expression which multiplexes the new additional claim dates with ConditionDateType to get the right one.
		strSQL.Format("SELECT "
			"CASE WHEN %li = 2 THEN Convert(bit, CASE WHEN InsuredPartyT.AccidentType = %li THEN 1 ELSE 0 END) ELSE BillsT.RelatedToEmp END AS CondEmp, "
			"CASE WHEN %li = 2 THEN Convert(bit, CASE WHEN InsuredPartyT.AccidentType = %li THEN 1 ELSE 0 END) ELSE BillsT.RelatedToAutoAcc END AS CondAuto, "
			"CASE WHEN %li = 2 THEN Convert(bit, CASE WHEN InsuredPartyT.AccidentType = %li THEN 1 ELSE 0 END) ELSE BillsT.RelatedToOther END AS CondOthr, "
			"CASE WHEN %li = 2 THEN InsuredPartyT.AccidentState ELSE BillsT.State END AS [Auto Acc State], "
			"CASE WHEN %li = 2 THEN InsuredPartyT.DateOfCurAcc ELSE BillsT.ConditionDate END AS CurrentIll, "
			"CASE WHEN (CASE WHEN %li = 2 THEN InsuredPartyT.DateOfCurAcc ELSE BillsT.ConditionDate END) Is Null THEN '' ELSE '431' END AS Box14Qual, "
			"CASE "
			"	WHEN BillsT.ConditionDateType = %li AND BillsT.FirstVisitOrConsultationDate IS NOT NULL THEN '444' "
			"	WHEN BillsT.ConditionDateType = %li AND BillsT.InitialTreatmentDate IS NOT NULL THEN '454' "
			"	WHEN BillsT.ConditionDateType = %li AND BillsT.LastSeenDate IS NOT NULL THEN '304' "
			"	WHEN BillsT.ConditionDateType = %li AND BillsT.AcuteManifestationDate IS NOT NULL THEN '453' "
			"	WHEN BillsT.ConditionDateType = %li AND BillsT.LastXRayDate IS NOT NULL THEN '455' "
			"	WHEN BillsT.ConditionDateType = %li AND BillsT.HearingAndPrescriptionDate IS NOT NULL THEN '471' "
			"	WHEN BillsT.ConditionDateType = %li AND BillsT.AssumedCareDate IS NOT NULL THEN '090' "
			"	WHEN BillsT.ConditionDateType = %li AND BillsT.RelinquishedCareDate IS NOT NULL THEN '091' "
			"	WHEN BillsT.ConditionDateType = %li AND BillsT.AccidentDate IS NOT NULL THEN '439' "
			"	ELSE '' "
			"END AS Box15Qual, "
			"CASE BillsT.ConditionDateType "
			"	WHEN %li THEN BillsT.FirstVisitOrConsultationDate "
			"	WHEN %li THEN BillsT.InitialTreatmentDate "
			"	WHEN %li THEN BillsT.LastSeenDate "
			"	WHEN %li THEN BillsT.AcuteManifestationDate "
			"	WHEN %li THEN BillsT.LastXRayDate "
			"	WHEN %li THEN BillsT.HearingAndPrescriptionDate "
			"	WHEN %li THEN BillsT.AssumedCareDate "
			"	WHEN %li THEN BillsT.RelinquishedCareDate "
			"	WHEN %li THEN BillsT.AccidentDate "
			"	ELSE NULL "
			"END AS FirstSimIll, "
			"BillsT.NoWorkFrom, BillsT.NoWorkTo, "
			"BillsT.HospFrom, BillsT.HospTo, BillsT.OutsideLab, BillsT.OutsideLabCharges AS OutLabCharges, BillsT.MedicaidResubmission, BillsT.OriginalRefNo, "
			"CASE WHEN BillsT.PriorAuthNum Is Null OR BillsT.PriorAuthNum = '' THEN '%s' ELSE BillsT.PriorAuthNum END AS PriorAuthNum, "
			"Box17PersonQ.[Last] + ', ' + Box17PersonQ.[First] + ' ' + Box17PersonQ.[Middle] AS RefNameLFM, "
			"Box17PersonQ.[First] + ' ' + Box17PersonQ.[Middle] + ' ' + Box17PersonQ.[Last] AS RefNameFML, "
			"CASE WHEN Box17DoctorQ.PersonID Is Null THEN '' ELSE '%s' END AS Box17Qual, "
			"%s AS Box17aQual, Box17DoctorQ.ReferringPhyID AS Box17aData, "
			"CASE WHEN %li = 1 AND LTRIM(RTRIM(Box17DoctorQ.GroupNPI)) <> '' THEN Box17DoctorQ.GroupNPI ELSE Box17DoctorQ.NPI END AS Box17bNPI, "
			"%s AS HCFABlock19, BillsT.HCFABox8 AS Box8, BillsT.HCFABox9b AS Box9b, BillsT.HCFABox9c AS Box9c, BillsT.HCFABox10D, BillsT.HCFABox11bQual AS Box11bQual, BillsT.HCFABox11b AS Box11b, "
			"%s "
			"Box17DoctorQ.NPI, Box17DoctorQ.UPIN AS RefID, "
			"DoctorsT.[First] + ' ' + DoctorsT.Middle + ' ' + DoctorsT.[Last] + (CASE WHEN (DoctorsT.Title <> '') THEN (', ' + DoctorsT.Title) ELSE ('') END) AS DocNameFML, "
			"DoctorsT.[Last] + (CASE WHEN (DoctorsT.Title <> '') THEN (', ' + DoctorsT.Title) ELSE ('') END) + ', ' + DoctorsT.[First] + ' ' + DoctorsT.Middle AS DocNameLFM "
			"FROM BillsT "
			"INNER JOIN PersonT PatientPersonT ON BillsT.PatientID = PatientPersonT.ID "
			"INNER JOIN InsuredPartyT ON BillsT.InsuredPartyID = InsuredPartyT.PersonID "

			"%s "
			"LEFT JOIN PersonT Box17PersonQ ON Box17DoctorQ.PersonID = Box17PersonQ.ID "

			"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 6) AS CustomData1 ON Box17DoctorQ.PersonID = CustomData1.PersonID "
			"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 7) AS CustomData2 ON Box17DoctorQ.PersonID = CustomData2.PersonID "
			"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 8) AS CustomData3 ON Box17DoctorQ.PersonID = CustomData3.PersonID "
			"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 9) AS CustomData4 ON Box17DoctorQ.PersonID = CustomData4.PersonID "
			"LEFT JOIN LocationsT ON BillsT.Location = LocationsT.ID "
			"CROSS JOIN (SELECT * FROM PersonT WHERE PersonT.ID = %li) AS DoctorsT ",
			nPullCurrentAccInfo, ipatEmployment,
			nPullCurrentAccInfo, ipatAutoAcc,
			nPullCurrentAccInfo, ipatOtherAcc,
			nPullCurrentAccInfo, nPullCurrentAccInfo, nPullCurrentAccInfo,
			ConditionDateType::cdtFirstVisitOrConsultation444,
			ConditionDateType::cdtInitialTreatmentDate454,
			ConditionDateType::cdtLastSeenDate304,
			ConditionDateType::cdtAcuteManifestation453,
			ConditionDateType::cdtLastXray455,
			ConditionDateType::cdtHearingAndPrescription471,
			ConditionDateType::cdtAssumedCare090,
			ConditionDateType::cdtRelinquishedCare91,
			ConditionDateType::cdtAccident439,
			ConditionDateType::cdtFirstVisitOrConsultation444,
			ConditionDateType::cdtInitialTreatmentDate454,
			ConditionDateType::cdtLastSeenDate304,
			ConditionDateType::cdtAcuteManifestation453,
			ConditionDateType::cdtLastXray455,
			ConditionDateType::cdtHearingAndPrescription471,
			ConditionDateType::cdtAssumedCare090,
			ConditionDateType::cdtRelinquishedCare91,
			ConditionDateType::cdtAccident439,
			_Q(strPriorAuthNum), strBox17AQualifier, strBox17aUse, nUseRefPhyGroupNPI, strBox19, strPOSFields,
			strBox17PersonJoin,
			nProviderID);

		//Find out what to fill in HCFA form 17a

		// (j.jones 2014-05-06 08:40) - PLID 61993 - now we may be pulling from ProvidersT
		CString str;
		switch (nBox17a) {
		case 1:
			str = "Box17DoctorQ.NPI";
			strSQL.Replace("Box17DoctorQ.ReferringPhyID", str);
			break;
		case 2:
			str = "Box17DoctorQ.ReferringPhyID";
			if (nBox17PersonID != -1 && bBox17_IsAProvider) {
				//providers do not have this field
				str = "''";
			}
			//no need
			strSQL.Replace("Box17DoctorQ.ReferringPhyID", str);
			break;
		case 3:
			str = "Box17DoctorQ.UPIN";
			strSQL.Replace("Box17DoctorQ.ReferringPhyID", str);
			break;
		case 4:
			str = "Box17DoctorQ.BlueShieldID";
			if (nBox17PersonID != -1 && bBox17_IsAProvider) {
				//the provider field is a different name
				str = "Box17DoctorQ.[BCBS Number]";
			}
			strSQL.Replace("Box17DoctorQ.ReferringPhyID", str);
			break;
		case 5:
			str = "Box17DoctorQ.FedEmployerID";
			if (nBox17PersonID != -1 && bBox17_IsAProvider) {
				//the provider field is a different name
				str = "Box17DoctorQ.[Fed Employer ID]";
			}
			strSQL.Replace("Box17DoctorQ.ReferringPhyID", str);
			break;
		case 6:
			str = "Box17DoctorQ.DEANumber";
			if (nBox17PersonID != -1 && bBox17_IsAProvider) {
				//the provider field is a different name
				str = "Box17DoctorQ.[DEA Number]";
			}
			strSQL.Replace("Box17DoctorQ.ReferringPhyID", str);
			break;
		case 7:
			str = "Box17DoctorQ.MedicareNumber";
			if (nBox17PersonID != -1 && bBox17_IsAProvider) {
				//the provider field is a different name
				str = "Box17DoctorQ.[Medicare Number]";
			}
			strSQL.Replace("Box17DoctorQ.ReferringPhyID", str);
			break;
		case 8:
			str = "Box17DoctorQ.MedicaidNumber";
			if (nBox17PersonID != -1 && bBox17_IsAProvider) {
				//the provider field is a different name
				str = "Box17DoctorQ.[Medicaid Number]";
			}
			strSQL.Replace("Box17DoctorQ.ReferringPhyID", str);
			break;
		case 9:
			str = "Box17DoctorQ.WorkersCompNumber";
			if (nBox17PersonID != -1 && bBox17_IsAProvider) {
				//the provider field is a different name
				str = "Box17DoctorQ.[Workers Comp Number]";
			}
			strSQL.Replace("Box17DoctorQ.ReferringPhyID", str);
			break;
		case 10:
			str = "Box17DoctorQ.OtherIDNumber";
			if (nBox17PersonID != -1 && bBox17_IsAProvider) {
				//the provider field is a different name
				str = "Box17DoctorQ.[Other ID Number]";
			}
			strSQL.Replace("Box17DoctorQ.ReferringPhyID", str);
			break;
		case 11:
			str = "Box17DoctorQ.License";
			if (nBox17PersonID != -1 && bBox17_IsAProvider) {
				//the provider field is a different name
				str = "Box17DoctorQ.[DEA Number]";
			}
			strSQL.Replace("Box17DoctorQ.ReferringPhyID", str);
			break;
		// (j.jones 2007-04-24 12:17) - PLID 25764 - supported Taxonomy Code
		case 12:
			str = "Box17DoctorQ.TaxonomyCode";
			strSQL.Replace("Box17DoctorQ.ReferringPhyID", str);
			break;
		case 16:
			str = "CustomData1.TextParam";
			strSQL.Replace("Box17DoctorQ.ReferringPhyID", str);
			break;
		case 17:
			str = "CustomData2.TextParam";
			strSQL.Replace("Box17DoctorQ.ReferringPhyID", str);
			break;
		case 18:
			str = "CustomData3.TextParam";
			strSQL.Replace("Box17DoctorQ.ReferringPhyID", str);
			break;
		case 19:
			str = "CustomData4.TextParam";
			strSQL.Replace("Box17DoctorQ.ReferringPhyID", str);
			break;
		default:
			str = "''";
			strSQL.Replace("Box17DoctorQ.ReferringPhyID", str);
		}
		g_aryFormQueries[22].sql = strSQL;
		g_aryFormQueries[23].sql = strSQL;
	}
	NxCatchAll("HCFADlg::BuildFormsT_Form22()");
}

void CHCFADlg::BuildFormsT_Form33()
{
	CString strSQL, str;
	// (j.jones 2010-10-21 14:47) - PLID 41051 - initialize these values
	long nProviderID = -1, nLocationID = -1, nBox17PersonID = -1;
	CString strBox17Qualifier = "DN";
	bool bBox17IsAProvider = false;

	try {
		// (j.jones 2010-04-20 10:28) - PLID 30852 - converted to CreateRecordsetStd
		_RecordsetPtr rs = CreateRecordsetStd(g_aryFormQueries[2].sql);
		if(!rs->eof) {
			nProviderID = AdoFldLong(rs, "DoctorsProviders", -1);
			nLocationID = AdoFldLong(rs, "LocationID", -1);
			// (j.jones 2014-05-01 14:04) - PLID 61993 - Added Referring/Ordering/Supervising provider ID, 
			// if one is in use, the proper ID will be loaded. If one is not in use, this will be -1
			// which means we follow standard Box17 logic and show the referring physician.
			nBox17PersonID = AdoFldLong(rs, "ChargeBox17PersonID", -1);
			strBox17Qualifier = AdoFldString(rs, "ChargeBox17Qual", "DN");
			bBox17IsAProvider = AdoFldBool(rs, "ChargeBox17IsAProvider", FALSE) ? true : false;
		}
		rs->Close();

		LoadProviderInfo(nProviderID, nLocationID, nBox17PersonID, strBox17Qualifier, bBox17IsAProvider);
	}
	NxCatchAll("HCFADlg::BuildFormsT_Form33()");
}

void CHCFADlg::BuildGRPNumber(CStringArray* pastrParams, VarAry* pavarParams)
{
	try {
		_RecordsetPtr rs;
		COleVariant varNull;
		CString str, strBox33Source;

		varNull.vt = VT_NULL;

		///////////////////////////////////////////////////////////////////
		// Fill in GRP number through insurance group in HCFA setup.
		// Later, this may be overridden by the InsuranceGroups table.
		pastrParams->Add("GRPNumber");
		str = m_HCFAInfo.Box33GRP;
		if (str.GetLength() != 0) {			
			g_varDefaultGRP = m_HCFAInfo.Box33GRP;
		}
		else {
			g_varDefaultGRP = varNull;
		}
		
		pavarParams->Add(CalcGRPNumber());
	}
	NxCatchAll("HCFADlg::BuildGRPNumber()");
}

void CHCFADlg::BuildHCFAChargesT()
{
	_RecordsetPtr rs;
	_RecordsetPtr rs2;
	CString str;
	CString strBox24C = "''";
	CString strServiceDateTo = "ChargesT.ServiceDateTo";
	try {

		// (j.jones 2007-04-10 08:29) - PLID 25539 - added ability to hide Box 24A Service Date To
		if(m_HCFAInfo.Hide24ATo == 1) {
			strServiceDateTo = "NULL";
		}

		// Pull the box 33 setup from the HCFA ID Setup table
		g_nBox33Setup = m_HCFAInfo.Box33Setup;

		// (j.jones 2010-04-08 14:24) - PLID 38094 - ChargesT.IsEmergency determines whether
		// we send the setting per charge, or use the HCFA Setup value.
		strBox24C.Format("CASE WHEN (ChargesT.IsEmergency = %li OR (ChargesT.IsEmergency = %li AND %li = 2)) THEN 'N' "
			"WHEN (ChargesT.IsEmergency = %li OR (ChargesT.IsEmergency = %li AND %li = 1)) THEN 'Y' "
			"ELSE '' END",
			(long)cietNo, (long)cietUseDefault, m_HCFAInfo.Box24C, 
			(long)cietYes, (long)cietUseDefault, m_HCFAInfo.Box24C);
	
		// If we are using the provider in General 1 or an overridden provider,
		// set that provider ID to all the charges in the HCFA charges table

		// (j.jones 2014-05-02 10:07) - PLID 61839 - moved the query to a modular function
		// (j.jones 2014-09-30 10:06) - PLID 63793 - changed to take in the BillID to streamline loading Box 24I/J fields
		g_aryFormQueries[2].sql = GenerateChargesQuery(billid, strServiceDateTo, strBox24C, "''", -1);

	}NxCatchAll("HCFADlg::BuildHCFAChargesT() Error in Loading FormCharges");

	try{
		
		m_pPrePrintFunc = &PreHCFAPrint;
		
		// Now build a list of different charge providers. Assume every charge
		// has a provider in FormCharges.
		aryPageInfo.clear();
		// (j.jones 2008-04-07 08:54) - PLID 28995 - clear the Box24J IDs
		mapBox24JProviderIDs.RemoveAll();
        /*(a.levy 2013-10-17 09:18) - PLID - 59060 - CreateRecordset to CreateRecordsetStd  - When users enter a % sign in Diagnosis code
           CreateRecordset tries to format it instead of escaping resulting in CRT error - invalid parameter. CreateRecordSetStd no format.
        */
		rs = CreateRecordsetStd(g_aryFormQueries[2].sql);
		while(!rs->eof) {
			long nProviderID = AdoFldLong(rs, "DoctorsProviders",-1);
			// (j.jones 2008-04-07 08:54) - PLID 28995 - we now map Box24J IDs
			long nBox24JProviderID = AdoFldLong(rs, "Box24JProviderID",-1);
			// (j.jones 2014-05-01 14:04) - PLID 61993 - Added Referring/Ordering/Supervising provider ID, 
			// if one is in use, the proper ID will be loaded. If one is not in use, this will be -1
			// which means we follow standard Box17 logic and show the referring physician.
			long nBox17PersonID = AdoFldLong(rs, "ChargeBox17PersonID", -1);
			CString strBox17Qualifier = AdoFldString(rs, "ChargeBox17Qual", "DN");

			bool bFound = false;

			//see if a page exists for this combination
			for each (PageInfoPtr pInfo in aryPageInfo)
			{
				if (pInfo->nProviderID == nProviderID
					&& pInfo->nBox17PersonID == nBox17PersonID
					&& pInfo->strBox17Qualifier == strBox17Qualifier) {
					
					//we already have a page for this combination
					bFound = true;
				}
			}

			if (!bFound) {
				//add a new page
				PageInfoPtr pNew = PageInfoPtr(new PageInfo(nProviderID, nBox17PersonID, strBox17Qualifier));
				aryPageInfo.push_back(pNew);

				mapBox24JProviderIDs.SetAt(nProviderID, nBox24JProviderID);
			}

			rs->MoveNext();
		}
		rs->Close();

		BuildFormsT_Form33();

	}NxCatchAll("HCFADlg::BuildHCFAChargesT() Error in building form.");

	try {
				
		// Rebuild the charge list with the first provider
		pageinfo_index = 0;
		if (aryPageInfo.size() > 0) {

			// (j.jones 2014-05-02 10:07) - PLID 61839 - moved the query to a modular function
			// (j.jones 2014-09-30 10:06) - PLID 63793 - changed to take in the BillID to streamline loading Box 24I/J fields
			g_aryFormQueries[2].sql = GenerateChargesQuery(m_ID, strServiceDateTo, strBox24C, "''", pageinfo_index);
		}

		// Change the controls in the HCFA form so that the charge lines go to the first 6
		// charges in the HCFA list
		UpdateCharges(firstcharge/* = 0*/);

		// Fill total charge amount for each line
		nTotalCharges = SizeOfFormChargesT();
	}
	NxCatchAll("HCFADlg::BuildHCFAChargesT() Error in rebuilding charge list.");
}

void CHCFADlg::SetHCFADateFormats()
{
	try {
		_RecordsetPtr rs;
		int i = 0, j = 0;
		int boxids[][7] = {
			{ 85, -1 },
			{ 146, -1 },
			{ 159, -1 },
			{ 200, -1 },
			{ 212, -1 },
			{ 234, -1 },
			{ 355, 356, -1 },
			{ 360, 361, -1 },
			{ 265, 275, 284, 293, 303, 312, -1 },
			{ 266, 276, 285, 294, 304, 313, -1 },
			{ 398, -1 }
		};
		CString strBoxNames[] = {
			"Box3", "Box9b", "Box11a", "Box12", "Box14", "Box15", "Box16",
				"Box18", "Box24From", "Box24To", "Box31"
		};
		CString strWideBoxNames[] = {
			"Box3Wide", "Box9bWide", "Box11aWide", "Box12Wide", "Box14Wide", "Box15Wide", "Box16Wide",
				"Box18Wide", "Box24FWide", "Box24TWide", "Box31Wide"
		};

		// Assign the date formats to be 2 or 4 digit
		rs = CreateParamRecordset("SELECT * FROM HCFASetupT WHERE ID = {INT}", fee_group_id);

		// (j.jones 2007-03-01 16:03) - PLID 23297 - converted to use batched statements
		CString strDatesBatch = BeginSqlBatch();

		// If no group in HCFA ID setup, default to 4 digits for all dates
		if (rs->eof) {
			for (i=0; i < 11; i++) {
				j=0;
				while (boxids[i][j] != -1) {
					AddStatementToSqlBatch(strDatesBatch, "UPDATE FormControlsT SET Format = 4096 WHERE ID = %li", boxids[i][j]);
					j++;
				}
			}
		}
		else {

			for (i=0; i < 11; i++) {

				//first get the 2/4 digit setup right
				switch (rs->Fields->Item[_bstr_t(strBoxNames[i])]->Value.lVal) {
				case 2:
					j=0;
					while (boxids[i][j] != -1) {
						AddStatementToSqlBatch(strDatesBatch, "UPDATE FormControlsT SET Format = 4107 WHERE ID = %li", boxids[i][j]);
						j++;
					}
					break;
				case 4:
					j=0;
					while (boxids[i][j] != -1) {
						AddStatementToSqlBatch(strDatesBatch, "UPDATE FormControlsT SET Format = 4096 WHERE ID = %li", boxids[i][j]);
						j++;
					}
					break;
				}

				//then get the widedate setting
				switch (rs->Fields->Item[_bstr_t(strWideBoxNames[i])]->Value.lVal) {
				case 1:
					j=0;
					while (boxids[i][j] != -1) {
						AddStatementToSqlBatch(strDatesBatch, "UPDATE FormControlsT SET Format = Format + 32768 WHERE ID = %li", boxids[i][j]);
						j++;
					}
					break;
				}
			}
		}
		rs->Close();

		//now execute in one batch
		if(!strDatesBatch.IsEmpty()) {
			ExecuteSqlBatch(strDatesBatch);
		}
	}
	NxCatchAll("SetHCFADateFormat");
}

void UpdateCharges(int &firstcharge)
{
	CString str,temp;

	try {

		nPageIncrement = 6; //will be less if anesthesia

		CString strOrderBy = " ORDER BY LineID";

		int chargecount = SizeOfFormChargesT();

		while (firstcharge >= chargecount)
			firstcharge -= nPageIncrement;

		// (j.jones 2007-03-01 16:03) - PLID 23297 - converted to use batched statements
		{
			CString strPageBatch = BeginSqlBatch();

			// (j.jones 2007-02-23 10:19) - PLID 24887 - changed these queries to support the new charge-line fields on the new HCFA form
			AddStatementToSqlBatch(strPageBatch, "UPDATE FormControlsT SET FormControlsT.[Value] = %d WHERE (((FormControlsT.ID)>=265) AND ((FormControlsT.ID)<=274)) OR (FormControlsT.ID IN (1997, 2003, 2009, 2015, 5444, 5504, 5510, 5516, 5522))", firstcharge);
			AddStatementToSqlBatch(strPageBatch, "UPDATE FormControlsT SET FormControlsT.[Value] = %d WHERE (((FormControlsT.ID)>=275) AND ((FormControlsT.ID)<=283)) OR (FormControlsT.ID IN (1998, 2004, 2010, 2016, 5445, 5505, 5511, 5517, 5523))", firstcharge+1);
			AddStatementToSqlBatch(strPageBatch, "UPDATE FormControlsT SET FormControlsT.[Value] = %d WHERE (((FormControlsT.ID)>=284) AND ((FormControlsT.ID)<=292)) OR (FormControlsT.ID IN (1999, 2005, 2011, 2017, 5446, 5506, 5512, 5518, 5524))", firstcharge+2);
			AddStatementToSqlBatch(strPageBatch, "UPDATE FormControlsT SET FormControlsT.[Value] = %d WHERE (((FormControlsT.ID)>=293) AND ((FormControlsT.ID)<=301)) OR (FormControlsT.ID IN (2000, 2006, 2012, 2018, 5447, 5507, 5513, 5519, 5525))", firstcharge+3);
			AddStatementToSqlBatch(strPageBatch, "UPDATE FormControlsT SET FormControlsT.[Value] = %d WHERE (((FormControlsT.ID)>=303) AND ((FormControlsT.ID)<=311)) OR (FormControlsT.ID IN (2001, 2007, 2013, 2019, 5448, 5508, 5514, 5520, 5526))", firstcharge+4);
			AddStatementToSqlBatch(strPageBatch, "UPDATE FormControlsT SET FormControlsT.[Value] = %d WHERE (((FormControlsT.ID)>=312) AND ((FormControlsT.ID)<=320)) OR (FormControlsT.ID IN (2002, 2008, 2014, 2020, 5449, 5509, 5515, 5521, 5527))", nPageIncrement == 6 ? firstcharge+5 : 5000);

			// (j.jones 2008-04-08 09:23) - PLID 28765 - changed the ExecuteSqlBatch call to ExecuteSqlStd,
			// so we could override the nMaxRecordsAffected number, as this code routinely updates 102 records
			// (a.walling 2012-02-09 17:13) - PLID 48115 - Use NxAdo::PushMaxRecordsWarningLimit and NxAdo::PushPerformanceWarningLimit
			NxAdo::PushMaxRecordsWarningLimit pmr(200);
			long nRecordsAffected = 0;
			ExecuteSqlStd("BEGIN TRAN \r\n" + strPageBatch + "COMMIT TRAN \r\n", &nRecordsAffected, adCmdText);
		}

		// (j.jones 2006-09-18 16:04) - PLID 22455 - this is outdated, we no longer show anesthesia times directly on the charge line
		/*
		//ensure we are initially always showing ItemCodes and CPT Modifiers
		ExecuteSql("UPDATE FormControlsT SET Source = 'ItemCode' WHERE ID IN (269, 279, 288, 297, 307, 316)");
		ExecuteSql("UPDATE FormControlsT SET Source = 'CPTModifier' WHERE ID IN (274, 283, 292, 301, 311, 320)");

		if(bAnesthesia) {
			//now try to figure out which row needs to show the times
			if(chargecount - firstcharge <= nPageIncrement) {
				long nItemCodeIDToUpdate = -1;
				long nModifierIDToUpdate = -1;

				switch((chargecount - firstcharge) % nPageIncrement) {
					case 1:
						nItemCodeIDToUpdate = 279;
						nModifierIDToUpdate = 283;
						break;
					case 2:
						nItemCodeIDToUpdate = 288;
						nModifierIDToUpdate = 292;
						break;
					case 3:
						nItemCodeIDToUpdate = 297;
						nModifierIDToUpdate = 301;
						break;
					case 4:
						nItemCodeIDToUpdate = 307;
						nModifierIDToUpdate = 311;
						break;
					case 0:
						nItemCodeIDToUpdate = 316;
						nModifierIDToUpdate = 320;
						break;
				}

				ExecuteSql("UPDATE FormControlsT SET Source = 'AnesthStartTime', Value = 0 WHERE ID = %li", nItemCodeIDToUpdate);
				ExecuteSql("UPDATE FormControlsT SET Source = 'AnesthEndTime', Value = 0 WHERE ID = %li", nModifierIDToUpdate);
			}
		}
		*/

		//JJ - yes this looks godawful but it is FAR better than what was used before. We used to have the TOP function which is not
		//only bad logic, but it NEVER worked right. These loops look nasty but they don't cause a speed hit (that anyone can
		//notice) and they get the job done. The commented out mess is the remnants of what used to be here.

		if(firstcharge>0) {
			str = "";
			int i = 0;
             // (a.levy 2013-10-18 11:12) - PLID 59043 
			_RecordsetPtr rs = CreateRecordsetStd(g_aryFormQueries[2].sql + strOrderBy);
			if(!rs->eof) {
				str = " AND (";
				//loop up to this page
				for(i=0;i<(nPageIncrement*((long)(firstcharge/nPageIncrement)));i++) {
					rs->MoveNext();
				}
				//loop through the charges on this page
				for(i=0;i<nPageIncrement;i++) {
					if(i>0)
						str += " OR ";
					temp.Format("ChargesT.ID = %li",rs->Fields->Item["ID"]->Value.lVal);
					str += temp;
					rs->MoveNext();
					if(rs->eof)
						break;
				}
				str += ")";
				temp = g_aryFormQueries[2].sql + str;
			}
			else {
				//this should not be possible, assert for debugging purposes,
				//try to figure out what is wrong with the SQL we just ran
				ASSERT(FALSE);
			}
		}
		else {
			str = "";
            // (a.levy 2013-10-18 11:12) - PLID 59043 -Changed to CreateRecordSetStd  --CreateRecordSetStd won't escape the % sign and try to format it with garbage
			_RecordsetPtr rs = CreateRecordsetStd(g_aryFormQueries[2].sql + strOrderBy);
			if(!rs->eof) {
				str = " AND (";
				//loop through the charges on this page
				for(int i=0;i<nPageIncrement;i++) {
					if(i>0)
						str += " OR ";
					temp.Format("ChargesT.ID = %li",rs->Fields->Item["ID"]->Value.lVal);
					str += temp;
					rs->MoveNext();
					if(rs->eof)
						break;
				}
				str += ")";
				temp = g_aryFormQueries[2].sql + str;
			}
			else {
				//this should not be possible, assert for debugging purposes,
				//try to figure out what is wrong with the SQL we just ran
				ASSERT(FALSE);
			}
		}

		BOOL bShowPayments = TRUE;
		BOOL bExcludeAdjustments = FALSE;
		BOOL bExcludeAdjFromBal = FALSE;
		BOOL bDoNotFillBox29 = FALSE;
		// (j.jones 2008-06-18 09:06) - PLID 30403 - added ability to not fill Box 29
		_RecordsetPtr rs = CreateParamRecordset("SELECT ShowPays, IgnoreAdjustments, ExcludeAdjFromBal, DoNotFillBox29 FROM HCFASetupT WHERE ID = {INT}", fee_group_id);
		if(!rs->eof) {
			if(rs->Fields->Item["ShowPays"]->Value.lVal == 0)
				bShowPayments = FALSE;

			if(rs->Fields->Item["IgnoreAdjustments"]->Value.lVal == 1)
				bExcludeAdjustments = TRUE;

			if(rs->Fields->Item["ExcludeAdjFromBal"]->Value.lVal == 1)
				bExcludeAdjFromBal = TRUE;

			if(AdoFldLong(rs, "DoNotFillBox29") == 1) {
				bDoNotFillBox29 = TRUE;
			}
		}
		rs->Close();


		str.Format("SELECT ChargeTotal, (CASE WHEN ApplyTotal Is Null THEN 0 ELSE ApplyTotal END) AS ApplyTotal FROM (%s) AS FormChargesT",temp);
		g_aryFormQueries[3].sql.Format("SELECT Convert(money,Sum(ChargeTotal)) AS TotalCharges FROM (SELECT TOP %li HCFAChargeTopQ.ChargeTotal, HCFAChargeTopQ.ApplyTotal "
				"FROM (%s) AS HCFAChargeTopQ) AS HCFACharge6Q", nPageIncrement, str);

		if(bShowPayments) {

			// (j.jones 2008-06-18 09:06) - PLID 30403 - added ability to not fill Box 29
			if(bDoNotFillBox29) {
				//query 4, the amount paid, should be $0.00
				g_aryFormQueries[4].sql = "SELECT Convert(money,'$0.00') AS TotalApplies ";
			}
			else if(bExcludeAdjustments) {
				//query 4, the amount paid, should be only payments
				g_aryFormQueries[4].sql.Format("SELECT Sum(PaymentTotal) AS TotalApplies FROM (SELECT TOP %li HCFAChargeTopQ.ChargeTotal, HCFAChargeTopQ.PaymentTotal "
						"FROM (SELECT ChargeTotal, (CASE WHEN PaymentTotal Is Null THEN 0 ELSE PaymentTotal END) AS PaymentTotal FROM (%s) AS FormChargesT) AS HCFAChargeTopQ) AS HCFACharge6Q",nPageIncrement,temp);				
			}
			else {
				//query 4, the amount paid, should be all applies
				g_aryFormQueries[4].sql.Format("SELECT Sum(ApplyTotal) AS TotalApplies FROM (SELECT TOP %li HCFAChargeTopQ.ChargeTotal, HCFAChargeTopQ.ApplyTotal "
						"FROM (%s) AS HCFAChargeTopQ) AS HCFACharge6Q", nPageIncrement, str);
			}

			//ExcludeAdjFromBal is now separated logic
			if(bExcludeAdjFromBal) {
				//query 5, the balance, should be the charge amount - total payment amount
				g_aryFormQueries[5].sql.Format("SELECT Sum(Convert(money,(ChargeTotal)-(PaymentTotal))) AS TotalBalance FROM (SELECT TOP %li HCFAChargeTopQ.ChargeTotal, HCFAChargeTopQ.PaymentTotal "
						"FROM (SELECT ChargeTotal, (CASE WHEN PaymentTotal Is Null THEN 0 ELSE PaymentTotal END) AS PaymentTotal FROM (%s) AS FormChargesT) AS HCFAChargeTopQ) AS HCFACharge6Q", nPageIncrement, temp);
			}
			else {
				//query 5, the balance, should be the charge amount - total apply amount
				g_aryFormQueries[5].sql.Format("SELECT Sum(Convert(money,(ChargeTotal)-(ApplyTotal))) AS TotalBalance FROM (SELECT TOP %li HCFAChargeTopQ.ChargeTotal, HCFAChargeTopQ.ApplyTotal "
						"FROM (%s) AS HCFAChargeTopQ) AS HCFACharge6Q", nPageIncrement, str);
			}
		}
		else {
			//query 4, the amount paid, should be $0.00
			g_aryFormQueries[4].sql = "SELECT Convert(money,'$0.00') AS TotalApplies ";
			//query 5, the balance, should be the total charge amount
			g_aryFormQueries[5].sql.Format("SELECT Sum(Convert(money,(ChargeTotal))) AS TotalBalance FROM (SELECT TOP %li HCFAChargeTopQ.ChargeTotal, HCFAChargeTopQ.ApplyTotal "
					"FROM (%s) AS HCFAChargeTopQ) AS HCFACharge6Q", nPageIncrement, str);
		}
		g_aryFormQueries[6].sql.Replace("FROM FormChargesT","FROM ("+ g_aryFormQueries[2].sql +") AS FormChargesT");
	//}
	}NxCatchAll("Error in updating charges");
}

//////////////////////////////////////////////////////////////////////
// This event is fired when the user clicks on a button on the HCFA
// form.
//
// Parameters:
// wParam:
//
// lParam: Taken from the message loop
//
// dwChangedForms: An array that contains a list of ID's of form layers
// that need to be refreshed because of changes in the data. The number
// does not correspond to the ID in FormsT, but the zero-based index in
// which it was added on m_pLayer->Load in OnInitDialog().
//
// firstcharge: The first visible charge on the HCFA
//

void OnHCFACommand(WPARAM wParam, LPARAM lParam, CDWordArray& dwChangedForms, int FormControlsID, CFormDisplayDlg* dlg) 
{
	
	CString str;
	_RecordsetPtr rs;
	// (j.jones 2010-10-21 14:47) - PLID 41051 - initialize these values
	long nProviderID = -1, nLocationID = -1, nBox17PersonID = -1;
	CString strBox17Qualifier = "DN";
	bool bBox17IsAProvider = false;

	if (pageinfo_index == -1)
		pageinfo_index = 0;

	switch (wParam) {
	case IDC_BILL_UP:
		//////////////////////////////////////////
		// "Scroll up" the charge listing
		if (firstcharge > 0) firstcharge -= nPageIncrement;
		break;
	case IDC_BILL_DOWN:
		//////////////////////////////////////////
		// "Scroll down" the charge listing
		firstcharge += nPageIncrement;
		break;
	case IDC_PHYSICIAN_LEFT: {
		try {
		//////////////////////////////////////////
		// Go to the previous provider
			if (pageinfo_index <= 0) {
				return;
			}
			pageinfo_index--;
		
		//////////////////////////////////////////
		// Force the charge listing to reset to
		// show the first 6 charges
		firstcharge = 0;

		_RecordsetPtr nxrs;
		CString str;
		CString strServiceDateTo = "ChargesT.ServiceDateTo";
		CString strBox24C = "''";
		nxrs = CreateParamRecordset("SELECT Box24C, Hide24ATo FROM HCFASetupT WHERE ID = {INT}", fee_group_id);
		if(!nxrs->eof) {
			_variant_t v = nxrs->Fields->Item["Box24C"]->Value;
			if(v.vt == VT_I4){
				long nBox24C = VarLong(v);
				// (j.jones 2010-04-08 14:24) - PLID 38094 - ChargesT.IsEmergency determines whether
				// we send the setting per charge, or use the HCFA Setup value.
				strBox24C.Format("CASE WHEN (ChargesT.IsEmergency = %li OR (ChargesT.IsEmergency = %li AND %li = 2)) THEN 'N' "
					"WHEN (ChargesT.IsEmergency = %li OR (ChargesT.IsEmergency = %li AND %li = 1)) THEN 'Y' "
					"ELSE '' END",
					(long)cietNo, (long)cietUseDefault, nBox24C, 
					(long)cietYes, (long)cietUseDefault, nBox24C);
			}
			else {
				// (j.jones 2010-04-20 11:02) - PLID 38094 - ChargesT.IsEmergency determines whether
				// we send the setting per charge, or use the HCFA Setup value (which we don't have in this case)
				strBox24C.Format("CASE WHEN ChargesT.IsEmergency = %li THEN 'N' "
					"WHEN ChargesT.IsEmergency = %li THEN 'Y' "
					"ELSE '' END",
					(long)cietNo, (long)cietYes);
			}

			// (j.jones 2007-04-10 08:40) - PLID 25539 - added ability to hide Box 24A Service Date To
			long Hide24ATo = AdoFldLong(nxrs, "Hide24ATo",0);
			if(Hide24ATo == 1) {
				strServiceDateTo = "NULL";
			}
		}
		else {
			// (j.jones 2010-04-08 14:24) - PLID 38094 - ChargesT.IsEmergency determines whether
			// we send the setting per charge, or use the HCFA Setup value (which we don't have in this case)
			strBox24C.Format("CASE WHEN ChargesT.IsEmergency = %li THEN 'N' "
				"WHEN ChargesT.IsEmergency = %li THEN 'Y' "
				"ELSE '' END",
				(long)cietNo, (long)cietYes);
		}
		nxrs->Close();
		//////////////////////////////////////////
		// Assign form layers to be updated
		//dwChangedForms.Add(2);	// HCFA total
		//dwChangedForms.Add(3);	// Total applied
		//dwChangedForms.Add(4);	// HCFA balance
		//dwChangedForms.Add(5);	// Box 28
		//dwChangedForms.Add(8);	// Box 33 (Address)
		//dwChangedForms.Add(9);	// Box 33 (PIN#)

		////////////////////////////////////////////////////////////
		// Rebuild the charge list with the previous provider
		if (aryPageInfo.size() > 0) {
			
			// (j.jones 2014-05-02 10:07) - PLID 61839 - moved the query to a modular function
			// (j.jones 2014-09-30 10:06) - PLID 63793 - changed to take in the BillID to streamline loading Box 24I/J fields
			g_aryFormQueries[2].sql = GenerateChargesQuery(billid, strServiceDateTo, strBox24C, "''", pageinfo_index);

			// (j.jones 2010-04-20 10:28) - PLID 30852 - converted to CreateRecordsetStd
			_RecordsetPtr rs = CreateRecordsetStd(g_aryFormQueries[2].sql);
			if(!rs->eof) {
				nProviderID = AdoFldLong(rs, "DoctorsProviders", -1);
				nLocationID = AdoFldLong(rs, "LocationID",-1);
				// (j.jones 2014-05-01 14:04) - PLID 61993 - Added Referring/Ordering/Supervising provider ID, 
				// if one is in use, the proper ID will be loaded. If one is not in use, this will be -1
				// which means we follow standard Box17 logic and show the referring physician.
				nBox17PersonID = AdoFldLong(rs, "ChargeBox17PersonID", -1);
				strBox17Qualifier = AdoFldString(rs, "ChargeBox17Qual", "DN");
				bBox17IsAProvider = AdoFldBool(rs, "ChargeBox17IsAProvider", FALSE) ? true : false;
			}
			rs->Close();

			LoadProviderInfo(nProviderID, nLocationID, nBox17PersonID, strBox17Qualifier, bBox17IsAProvider);
			
			////////////////////////////////////////////////////////////
			// Fill total charge amount for each line
			nTotalCharges = SizeOfFormChargesT();
		}
		}NxCatchAll("Error in OnHcfaCommand - Physician Left");
		break;
	}
	case IDC_PHYSICIAN_RIGHT: {
		try {
			if (pageinfo_index == -1) {
				pageinfo_index = 0;
			}
		
			//////////////////////////////////////////
			// Go to the previous provider
			if (pageinfo_index == aryPageInfo.size() - 1) {
				return;
			}
			pageinfo_index++;

		//////////////////////////////////////////
		// Force the charge listing to reset to
		// show the first 6 charges
		firstcharge = 0;

		_RecordsetPtr nxrs;
		CString str;

		// (j.jones 2014-05-01 10:48) - PLID 61839 - get the provider ID out of the current page info
		long nProviderID = GetCurrentPageProviderID();

		CString strServiceDateTo = "ChargesT.ServiceDateTo";
		CString strBox24C = "''";
		nxrs = CreateParamRecordset("SELECT Box24C, Hide24ATo FROM HCFASetupT WHERE ID = {INT}", fee_group_id);
		if(!nxrs->eof) {
			_variant_t v = nxrs->Fields->Item["Box24C"]->Value;
			if(v.vt == VT_I4){
				long nBox24C = VarLong(v);
				// (j.jones 2010-04-08 14:24) - PLID 38094 - ChargesT.IsEmergency determines whether
				// we send the setting per charge, or use the HCFA Setup value.
				strBox24C.Format("CASE WHEN (ChargesT.IsEmergency = %li OR (ChargesT.IsEmergency = %li AND %li = 2)) THEN 'N' "
					"WHEN (ChargesT.IsEmergency = %li OR (ChargesT.IsEmergency = %li AND %li = 1)) THEN 'Y' "
					"ELSE '' END",
					(long)cietNo, (long)cietUseDefault, nBox24C, 
					(long)cietYes, (long)cietUseDefault, nBox24C);
			}
			else {
				// (j.jones 2010-04-20 11:02) - PLID 38094 - ChargesT.IsEmergency determines whether
				// we send the setting per charge, or use the HCFA Setup value (which we don't have in this case)
				strBox24C.Format("CASE WHEN ChargesT.IsEmergency = %li THEN 'N' "
					"WHEN ChargesT.IsEmergency = %li THEN 'Y' "
					"ELSE '' END",
					(long)cietNo, (long)cietYes);
			}

			// (j.jones 2007-04-10 08:40) - PLID 25539 - added ability to hide Box 24A Service Date To
			long Hide24ATo = AdoFldLong(nxrs, "Hide24ATo",0);
			if(Hide24ATo == 1) {
				strServiceDateTo = "NULL";
			}
		}
		else{
			// (j.jones 2010-04-08 14:24) - PLID 38094 - ChargesT.IsEmergency determines whether
			// we send the setting per charge, or use the HCFA Setup value (which we don't have in this case)
			strBox24C.Format("CASE WHEN ChargesT.IsEmergency = %li THEN 'N' "
				"WHEN ChargesT.IsEmergency = %li THEN 'Y' "
				"ELSE '' END",
				(long)cietNo, (long)cietYes);
		}
		nxrs->Close();
		//////////////////////////////////////////
		// Assign form layers to be updated
		//dwChangedForms.Add(2);	// HCFA total
		//dwChangedForms.Add(3);	// Total applied
		//dwChangedForms.Add(4);	// HCFA balance
		//dwChangedForms.Add(5);	// Box 28
		//dwChangedForms.Add(8);	// Box 33 (Address)
		//dwChangedForms.Add(9);	// Box 33 (PIN#)

		////////////////////////////////////////////////////////////
		// Rebuild the charge list with the next provider
		if (aryPageInfo.size() > 0) {

			// (j.jones 2014-05-02 10:07) - PLID 61839 - moved the query to a modular function
			// (j.jones 2014-09-30 10:06) - PLID 63793 - changed to take in the BillID to streamline loading Box 24I/J fields
			g_aryFormQueries[2].sql = GenerateChargesQuery(billid, strServiceDateTo, strBox24C, "''", pageinfo_index);

			// (j.jones 2010-04-20 10:28) - PLID 30852 - converted to CreateRecordsetStd
			rs = CreateRecordsetStd(g_aryFormQueries[2].sql);
			if(!rs->eof) {
				nProviderID = AdoFldLong(rs, "DoctorsProviders",-1);
				nLocationID = AdoFldLong(rs, "LocationID",-1);
				// (j.jones 2014-05-01 14:04) - PLID 61993 - Added Referring/Ordering/Supervising provider ID, 
				// if one is in use, the proper ID will be loaded. If one is not in use, this will be -1
				// which means we follow standard Box17 logic and show the referring physician.
				nBox17PersonID = AdoFldLong(rs, "ChargeBox17PersonID", -1);
				strBox17Qualifier = AdoFldString(rs, "ChargeBox17Qual", "DN");
				bBox17IsAProvider = AdoFldBool(rs, "ChargeBox17IsAProvider", FALSE) ? true : false;
			}
			rs->Close();

			LoadProviderInfo(nProviderID, nLocationID, nBox17PersonID, strBox17Qualifier, bBox17IsAProvider);

			////////////////////////////////////////////////////////////
			// Fill total charge amount for each line
			nTotalCharges = SizeOfFormChargesT();
		}
		}NxCatchAll("Error in OnHcfaCommand - Physician Right");
		break;
	}
	////////////////////////////////////////////////////////////
	// Default assumption is user selected a radio button
	default:
		try {
		// TODO: Add control 379 to the changes array.
		// Model off PreTranslateMessage in FormDisplayDlg.cpp
		// MAKE SURE YOU CAN HANDLE THE CASE WHERE 379 ALREADY
		// EXISTS IN THE CHANGE ARRAY.

		switch (FormControlsID) {
		case 380: // Box 25 SSN radio
			ExecuteSql("UPDATE FormControlsT SET Source = 'SocialSecurity' WHERE ID = 379");
			dlg->Refresh(6);
			dlg->TrackChanges(dlg->m_ChangeKey1, dlg->m_ChangeKey2);
			dlg->ReapplyChanges(dlg->m_ChangeKey1, dlg->m_ChangeKey2);			
			break;
		case 381: // Box 25 EIN radio
			ExecuteSql("UPDATE FormControlsT SET Source = 'Fed Employer ID' WHERE ID = 379");
			dlg->Refresh(6);
			dlg->TrackChanges(dlg->m_ChangeKey1, dlg->m_ChangeKey2);
			dlg->ReapplyChanges(dlg->m_ChangeKey1, dlg->m_ChangeKey2);
			break;
		case 386: { // Box 27 Accept Assignment "Yes" radio

			// (j.jones 2007-03-01 16:03) - PLID 23297 - converted to use batched statements
			CString strControl386Batch = BeginSqlBatch();

			int Accepted12Rule = 3, Accepted13Rule = 3;
			HCFABox13Over hb13Value = hb13_UseDefault;
			// (j.jones 2010-06-10 13:24) - PLID 39095 - added a second recordset to find the Box13 override
			_RecordsetPtr rs = CreateParamRecordset("SELECT Box12Accepted, Box13Accepted FROM HCFASetupT WHERE ID = {INT}\r\n"
				" "
				"SELECT HCFABox13Over FROM BillsT WHERE ID = {INT}", fee_group_id, billid);

			if(!rs->eof) {
				// (j.jones 2010-07-22 11:49) - PLID 39779 - converted to use VarLong and reflect
				// that Box13 defaults to 3 (though neither are nullable fields so this was pointless)
				// (j.jones 2010-07-23 16:05) - PLID 39794 - Box 12 now also defaults to 3
				Accepted12Rule = AdoFldLong(rs, "Box12Accepted", 3);
				Accepted13Rule = AdoFldLong(rs, "Box13Accepted", 3);
			}
			rs = rs->NextRecordset(NULL);

			if(!rs->eof) {
				hb13Value = (HCFABox13Over)AdoFldLong(rs, "HCFABox13Over", (long)hb13_UseDefault);		
			}
			rs->Close();

			if(Accepted12Rule==1 || Accepted12Rule==3) {
				AddStatementToSqlBatch(strControl386Batch, "UPDATE FormControlsT SET Source = 'SignatureOnFile' WHERE ID = 199");

				//0 - the Print Date (today), 1 - the Bill Date
				if(ReturnsRecords("SELECT ID FROM HCFASetupT WHERE ID = %li AND Box12UseDate = 0", fee_group_id)) {
					AddStatementToSqlBatch(strControl386Batch, "UPDATE FormControlsT SET Source = 'TodaysDate', Format = 4096, FormID = 2 WHERE ID = 200");
				}
				else {
					AddStatementToSqlBatch(strControl386Batch, "UPDATE FormControlsT SET Source = 'BillDate', Format = 4096, FormID = 2 WHERE ID = 200");
				}
			}
			else {
				AddStatementToSqlBatch(strControl386Batch, "UPDATE FormControlsT SET Source = '' WHERE ID = 199");
				AddStatementToSqlBatch(strControl386Batch, "UPDATE FormControlsT SET Source = '', Format = 4096 WHERE ID = 200");
			}
			// (j.jones 2010-06-10 13:19) - PLID 39095 - the bill's Box13 override can forcibly fill/empty this field
			if(hb13Value == hb13_Yes || (hb13Value == hb13_UseDefault && (Accepted13Rule==1 || Accepted13Rule==3)))
				AddStatementToSqlBatch(strControl386Batch, "UPDATE FormControlsT SET Source = 'SignatureOnFile' WHERE ID = 207");
			else
				AddStatementToSqlBatch(strControl386Batch, "UPDATE FormControlsT SET Source = '' WHERE ID = 207");

			//now execute in one batch
			if(!strControl386Batch.IsEmpty()) {
				ExecuteSqlBatch(strControl386Batch);
			}

			dlg->Refresh(1);
			dlg->TrackChanges(dlg->m_ChangeKey1, dlg->m_ChangeKey2);
			dlg->ReapplyChanges(dlg->m_ChangeKey1, dlg->m_ChangeKey2);
			break;
				  }
		case 387: { // Box 27 Accept Assignment "No" radio

			// (j.jones 2007-03-01 16:03) - PLID 23297 - converted to use batched statements
			CString strControl387Batch = BeginSqlBatch();

			int Accepted12Rule = 3, Accepted13Rule = 3;
			HCFABox13Over hb13Value = hb13_UseDefault;
			// (j.jones 2010-06-10 13:24) - PLID 39095 - added a second recordset to find the Box13 override
			_RecordsetPtr rs = CreateParamRecordset("SELECT Box12Accepted, Box13Accepted FROM HCFASetupT WHERE ID = {INT}\r\n"
				" "
				"SELECT HCFABox13Over FROM BillsT WHERE ID = {INT}", fee_group_id, billid);

			if(!rs->eof) {
				// (j.jones 2010-07-22 11:49) - PLID 39779 - converted to use VarLong and reflect
				// that Box13 defaults to 3 (though neither are nullable fields so this was pointless)
				// (j.jones 2010-07-23 16:05) - PLID 39794 - Box 12 now also defaults to 3
				Accepted12Rule = AdoFldLong(rs, "Box12Accepted", 3);
				Accepted13Rule = AdoFldLong(rs, "Box13Accepted", 3);
			}

			rs = rs->NextRecordset(NULL);

			if(!rs->eof) {
				hb13Value = (HCFABox13Over)AdoFldLong(rs, "HCFABox13Over", (long)hb13_UseDefault);		
			}
			rs->Close();

			if(Accepted12Rule==2 || Accepted12Rule==3) {
				AddStatementToSqlBatch(strControl387Batch, "UPDATE FormControlsT SET Source = 'SignatureOnFile' WHERE ID = 199");

				//0 - the Print Date (today), 1 - the Bill Date
				if(ReturnsRecords("SELECT ID FROM HCFASetupT WHERE ID = %li AND Box12UseDate = 0", fee_group_id)) {
					AddStatementToSqlBatch(strControl387Batch, "UPDATE FormControlsT SET Source = 'TodaysDate', Format = 4096, FormID = 2 WHERE ID = 200");
				}
				else {
					AddStatementToSqlBatch(strControl387Batch, "UPDATE FormControlsT SET Source = 'BillDate', Format = 4096, FormID = 2 WHERE ID = 200");
				}
			}
			else {
				AddStatementToSqlBatch(strControl387Batch, "UPDATE FormControlsT SET Source = '' WHERE ID = 199");
				AddStatementToSqlBatch(strControl387Batch, "UPDATE FormControlsT SET Source = '', Format = 4096 WHERE ID = 200");
			}
			// (j.jones 2010-06-10 13:19) - PLID 39095 - the bill's Box13 override can forcibly fill/empty this field
			if(hb13Value == hb13_Yes || (hb13Value == hb13_UseDefault && (Accepted13Rule==2 || Accepted13Rule==3)))
				AddStatementToSqlBatch(strControl387Batch, "UPDATE FormControlsT SET Source = 'SignatureOnFile' WHERE ID = 207");
			else
				AddStatementToSqlBatch(strControl387Batch, "UPDATE FormControlsT SET Source = '' WHERE ID = 207");

			//now execute in one batch
			if(!strControl387Batch.IsEmpty()) {
				ExecuteSqlBatch(strControl387Batch);
			}

			dlg->Refresh(1);
			dlg->TrackChanges(dlg->m_ChangeKey1, dlg->m_ChangeKey2);
			dlg->ReapplyChanges(dlg->m_ChangeKey1, dlg->m_ChangeKey2);
			break;
				  }
		}
		}NxCatchAll("Error in OnHcfaCommand");
		return;
	}

	if (dlg) {
		dlg->StopTrackingChanges();
		UpdateCharges(firstcharge);
		dlg->m_ChangeKey1 = firstcharge;
		dlg->m_ChangeKey2 = pageinfo_index;

		// (j.jones 2007-06-22 11:58) - PLID 25665 - need to redraw to ensure edited colors disappear
		dlg->Invalidate();
	}

	try {
		/////////////////////////////////////////////
		// Reload ALL boxes

		dwChangedForms.Add(1); // 90% of the static text & patient information

		dwChangedForms.Add(20); // Insurance info on right side of hcfa
		dwChangedForms.Add(21); // Insurance info on right side of hcfa
		dwChangedForms.Add(33); // HCFA Address
		dwChangedForms.Add(34); // PIN number
		dwChangedForms.Add(22); // Bill information
 
		dwChangedForms.Add(2);	// HCFA total
		dwChangedForms.Add(3);	// Total applied
		dwChangedForms.Add(4);	// HCFA balance		
		dwChangedForms.Add(5);	// Box 25
		dwChangedForms.Add(6);	// Box 31
		dwChangedForms.Add(8);	// Box 33 (Address)
		dwChangedForms.Add(9);	// Box 33 (PIN#)

		///////////////////////////////////////////////////////
		// This will build the GRP number given the field
		// in InsuranceGroups
		ReloadGRPNumber(dlg);

		RequeryHistoricalHCFAData();
	}
	NxCatchAll("OnHCFACommand");
}

BOOL PreHCFAPrint(CDWordArray& dwChangedForms, CFormDisplayDlg* dlg)
{
	if (aryPageInfo.size() == 0)
	{	ASSERT(FALSE);
		HandleException (NULL, "Failure in PreHCFAPrint", __LINE__, __FILE__);
		return FALSE; // This should never happen
	}

	// Only happens on first call to this function. The first provider
	// information is reloaded.
	if (nPrintedCharges == 0 && nPrintedProviders == 0) {

		while(pageinfo_index > 0)
			OnHCFACommand(IDC_PHYSICIAN_LEFT, 0, dwChangedForms, 0, dlg);

		oldfirstcharge = firstcharge;
		oldpageinfo_index = pageinfo_index;		

		pageinfo_index = -1;
		OnHCFACommand(IDC_PHYSICIAN_LEFT, 0, dwChangedForms, 0, dlg);
		firstcharge = -nPageIncrement;
		nPrintedProviders++;
	}


	// If still more charges to print, print them and return
	if (nPrintedCharges < nTotalCharges) {		
		OnHCFACommand(IDC_BILL_DOWN, 0, dwChangedForms, 0, dlg);
		nPrintedCharges += nPageIncrement;
		return TRUE;
	}
	// Otherwise, go for the next provider
	else {
		nPrintedCharges = 0;

		// If no more providers, exit
		if (pageinfo_index == aryPageInfo.size() - 1) {
			nPrintedProviders = 0;
			firstcharge = oldfirstcharge;
			oldpageinfo_index = pageinfo_index;
			return FALSE;
		}

		OnHCFACommand(IDC_PHYSICIAN_RIGHT, 0, dwChangedForms, 0, dlg);
		firstcharge = -nPageIncrement;
		OnHCFACommand(IDC_BILL_DOWN, 0, dwChangedForms, 0, dlg);
		nPrintedCharges = nPageIncrement;
		nPrintedProviders++;
		return TRUE;
	}
}

void CHCFADlg::FindBatch()
{
	int nBatch = FindHCFABatch(m_ID);

	if (nBatch == 1) {
		((CButton*)GetDlgItem(IDC_RADIO_PAPER))->SetCheck(TRUE);
		((CButton*)GetDlgItem(IDC_RADIO_ELECTRONIC))->SetCheck(FALSE);
		((CButton*)GetDlgItem(IDC_RADIO_NO_BATCH))->SetCheck(FALSE);
	}
	else if (nBatch == 2) {
		((CButton*)GetDlgItem(IDC_RADIO_PAPER))->SetCheck(FALSE);
		((CButton*)GetDlgItem(IDC_RADIO_ELECTRONIC))->SetCheck(TRUE);
		((CButton*)GetDlgItem(IDC_RADIO_NO_BATCH))->SetCheck(FALSE);
	}
	else {
		((CButton*)GetDlgItem(IDC_RADIO_PAPER))->SetCheck(FALSE);
		((CButton*)GetDlgItem(IDC_RADIO_ELECTRONIC))->SetCheck(FALSE);
		((CButton*)GetDlgItem(IDC_RADIO_NO_BATCH))->SetCheck(TRUE);
	}
}

void CHCFADlg::UpdateBatch()
{
	CString sql, str;
	short nBatch = 0;

	if (((CButton*)GetDlgItem(IDC_RADIO_PAPER))->GetCheck())
		nBatch = 1; // paper batch
	else if (((CButton*)GetDlgItem(IDC_RADIO_ELECTRONIC))->GetCheck())
		nBatch = 2; // electronic batch

	BatchBill(m_ID, nBatch);
}

void CHCFADlg::SaveHistoricalData(BOOL boSaveAll)
{
#ifdef SAVE_HISTORICAL_INFORMATION

	int iDocumentID;
	CString str;

	try {		

		if (pageinfo_index < 0 || pageinfo_index >= aryPageInfo.size()) {
			ThrowNxException("pageinfo_index %li is invalid for page count of %li.", pageinfo_index, aryPageInfo.size());
		}

		PageInfoPtr pInfo = aryPageInfo[pageinfo_index];

		_RecordsetPtr rs;
		// (j.jones 2014-05-02 11:41) - PLID 61839 - added Box17 info, because pages can now be broken up by the Box17 person and qualifier
		if (pInfo->nBox17PersonID != -1) {
			rs = CreateParamRecordset("SELECT FirstDocumentID FROM HCFADocumentsT "
				"WHERE BillID = {INT} AND FirstCharge = {INT} AND ProviderIndex = {INT} AND FormType = 0 "
				"AND Box17PersonID = {INT} AND Box17Qual = {STRING}",
				m_ID, firstcharge, pInfo->nProviderID, pInfo->nBox17PersonID, pInfo->strBox17Qualifier);
		}
		else {
			//typical case
			//if there is no special Box17 person, and we're using the normal bill referring physician,
			//the Box17PersonID is null, and the qualifier is empty
			rs = CreateParamRecordset("SELECT FirstDocumentID FROM HCFADocumentsT "
				"WHERE BillID = {INT} AND FirstCharge = {INT} AND ProviderIndex = {INT} AND FormType = 0 "
				"AND Box17PersonID Is Null",
				m_ID, firstcharge, pInfo->nProviderID);
		}
		 
		if (!rs->eof) {
			iDocumentID = VarLong(rs->Fields->Item["FirstDocumentID"]->Value);
			rs->Close();
		}
		else {
			rs->Close();
			
			iDocumentID = NewNumber("FormHistoryT","DocumentID");

			_variant_t varBox17PersonID = g_cvarNull;
			if (pInfo->nBox17PersonID != -1) {
				varBox17PersonID = (long)(pInfo->nBox17PersonID);
			}

			//fail if the qualifier is invalid
			if (pInfo->strBox17Qualifier.GetLength() > 10) {
				ThrowNxException("Invalid Box 17 qualifier %s.", pInfo->strBox17Qualifier);
			}
			
			// (j.jones 2007-03-01 16:03) - PLID 23297 - converted to use batched statements
			CString strBatch;
			CNxParamSqlArray aryParams;
			// (j.jones 2014-05-02 11:41) - PLID 61839 - added Box17 info, because pages can now be broken up by the Box17 person and qualifier
			AddParamStatementToSqlBatch(strBatch, aryParams, "INSERT INTO HCFADocumentsT (BillID, FirstDocumentID, FirstCharge, ProviderIndex, FormType, Box17PersonID, Box17Qual) "
				"VALUES ({INT}, {INT}, {INT}, {INT}, 0, {VT_I4}, {STRING})", m_ID, iDocumentID, firstcharge, pInfo->nProviderID, varBox17PersonID, pInfo->strBox17Qualifier);
			AddParamStatementToSqlBatch(strBatch, aryParams, "INSERT INTO FormHistoryT (DocumentID, ControlID, [Value]) VALUES ({INT}, -1, NULL)", iDocumentID);
			ExecuteParamSqlBatch(GetRemoteData(), strBatch, aryParams);
		}		
	}
	NxCatchAll("Error in OnClickCheck()");

	///////////////////////////////////
	// Make sure we pull our historical data from the right table
	m_pframe->SetDocumentID(iDocumentID);

	///////////////////////////////////
	// Refresh
	//
	try {
		UpdateCharges(firstcharge);
	}
	NxCatchAll("SaveHistoricalData");
	//reset this to EIN
	// (j.jones 2007-03-01 16:03) - PLID 23297 - converted to use batched statements
	CString strBatch = BeginSqlBatch();
	AddStatementToSqlBatch(strBatch, "UPDATE FormControlsT SET Source = 'Fed Employer ID' WHERE ID = 379");
	AddStatementToSqlBatch(strBatch, "UPDATE FormControlsT SET Value = 0 WHERE ID = 380");
	AddStatementToSqlBatch(strBatch, "UPDATE FormControlsT SET Value = 1 WHERE ID = 381");
	ExecuteSqlBatch(strBatch);

	ReloadGRPNumber(m_pframe);

	m_pframe->Refresh(1); // 90% of the static text & patient information
	m_pframe->Refresh(2);	// HCFA total
	m_pframe->Refresh(3);	// Total applied
	m_pframe->Refresh(4);	// HCFA balance
	m_pframe->Refresh(5);	// Box 28
	m_pframe->Refresh(6);	// Box 31
	m_pframe->Refresh(8);	// Box 33 (Address)
	m_pframe->Refresh(9);	// Box 33 (PIN#)

	m_pframe->Refresh(20); // Insurance info on right side of hcfa
	m_pframe->Refresh(21); // Insurance info on right side of hcfa
	m_pframe->Refresh(33); // HCFA Address
	m_pframe->Refresh(34); // PIN number
	m_pframe->Refresh(22); // Bill information

	m_pframe->ReapplyChanges(firstcharge, pageinfo_index);

	m_pframe->Save(firstcharge, pageinfo_index, iDocumentID, boSaveAll);

	// We save form 6 (zero based in the array) because where the radio button is
	// determines what text goes in a particular control. To save us time and pain,
	// we always save the data in form 6.
	//((FormLayer*)m_pframe->m_LayerArray[5])->Save(iDocumentID, 0, &m_pframe->m_ControlArray);
#endif
}

void CHCFADlg::OnClickCapitalizeOnPrint() 
{
	if(m_CapOnPrint) {
		SetRemotePropertyInt("CapitalizeHCFA",0);
	}
	else {
		SetRemotePropertyInt("CapitalizeHCFA",1);
	}
	m_CapOnPrint = GetRemotePropertyInt("CapitalizeHCFA",0);
}

void CHCFADlg::LoadBox25Default()
{
	_RecordsetPtr rs;
	_variant_t var;

	/*
	Here's the behavior of the following code:
	-First check to see if this is a loaded (and changed) HCFA, in which case we load the saved value.
	-If it is a new or unchanged HCFA, we try to load it.
		-In which case we get the advanced Box25 override - if this is filled in, it will always be used
		-Then we check their setting for Box25 SSN or EIN
			-If there is no override, then check the SSN or EIN box per their setting and load that info
			-If there is an override, still check the box to coincide with the setting, but load the override instead
	*/

	if(g_DocumentID != -1) {
		//if not new, make sure that Box25 loads properly.
		rs = CreateParamRecordset("SELECT * FROM FormHistoryT WHERE DocumentID = {INT} AND (ControlID = 380 OR ControlID = 381)",g_DocumentID);
		while(!rs->eof) {
			//this means that they have changed the SSN/EIN box. We need to update FormControlsT
			//so that the Load will pull the correct value
			var = rs->Fields->Item["ControlID"]->Value;
			if(var.vt == VT_I4) {
				long ControlID = var.lVal;
				var = rs->Fields->Item["Value"]->Value;
				if(ControlID == 380 && CString(var.bstrVal)=="1") {
					//SSN Check
					// (j.jones 2007-03-01 16:03) - PLID 23297 - converted to use batched statements
					CString strBatch = BeginSqlBatch();
					AddStatementToSqlBatch(strBatch, "UPDATE FormControlsT SET Source = 'SocialSecurity' WHERE ID = 379");
					AddStatementToSqlBatch(strBatch, "UPDATE FormControlsT SET Value = 1 WHERE ID = 380");
					AddStatementToSqlBatch(strBatch, "UPDATE FormControlsT SET Value = 0 WHERE ID = 381");
					ExecuteSqlBatch(strBatch);
					rs->Close();
					return;
				}
				else if(ControlID == 381 && CString(var.bstrVal)=="1") {
					//EIN Check
					CString strBatch = BeginSqlBatch();
					AddStatementToSqlBatch(strBatch, "UPDATE FormControlsT SET Source = 'Fed Employer ID' WHERE ID = 379");
					AddStatementToSqlBatch(strBatch, "UPDATE FormControlsT SET Value = 0 WHERE ID = 380");
					AddStatementToSqlBatch(strBatch, "UPDATE FormControlsT SET Value = 1 WHERE ID = 381");
					ExecuteSqlBatch(strBatch);
					rs->Close();
					return;
				}
			}
			rs->MoveNext();
		}
		//if the recordset was empty, than they didn't change the SSN/EIN Box and the default is used
		//the default has already been set, so move on
		rs->Close();
	}

	//if we have not returned, then we have to load the preferences

	//first load the override, if it exists
	// (j.jones 2010-10-21 14:47) - PLID 41051 - initialize these values
	long nProviderID = -1, nLocationID = -1, nBox17PersonID = -1;
	CString strBox17Qualifier = "DN";
	bool bBox17IsAProvider = false;
		
	// (j.jones 2010-04-20 10:28) - PLID 30852 - converted to CreateRecordsetStd
	rs = CreateRecordsetStd(g_aryFormQueries[2].sql);
	if(!rs->eof) {
		nProviderID = AdoFldLong(rs, "DoctorsProviders",-1);
		nLocationID = AdoFldLong(rs, "LocationID",-1);
		// (j.jones 2014-05-01 14:04) - PLID 61993 - Added Referring/Ordering/Supervising provider ID, 
		// if one is in use, the proper ID will be loaded. If one is not in use, this will be -1
		// which means we follow standard Box17 logic and show the referring physician.
		nBox17PersonID = AdoFldLong(rs, "ChargeBox17PersonID", -1);
		strBox17Qualifier = AdoFldString(rs, "ChargeBox17Qual", "DN");
		bBox17IsAProvider = AdoFldBool(rs, "ChargeBox17IsAProvider", FALSE) ? true : false;
	}
	rs->Close();

	// (j.jones 2007-03-01 16:03) - PLID 23297 - converted to use batched statements
	CString strBox25Batch = BeginSqlBatch();

	BOOL bUseNumberOverride = FALSE;
	BOOL bUseCheckboxOverride = FALSE;
	// (j.jones 2008-09-10 11:35) - PLID 30788 - added Box25Check
	rs = CreateParamRecordset("SELECT EIN, Box25Check FROM AdvHCFAPinT WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT}", nProviderID, nLocationID, fee_group_id);
	if(!rs->eof) {
		_variant_t var = rs->Fields->Item["EIN"]->Value;
		if(var.vt == VT_BSTR) {
			bUseNumberOverride = TRUE;
			AddStatementToSqlBatch(strBox25Batch, "UPDATE FormControlsT SET Source = 'Override' WHERE ID = 379");
		}
		long nBox25Check = AdoFldLong(rs, "Box25Check", 0);
		if(nBox25Check == 1) {
			//force SSN to be checked
			bUseCheckboxOverride = TRUE;
			AddStatementToSqlBatch(strBox25Batch, "UPDATE FormControlsT SET Value = 1 WHERE ID = 380");
			AddStatementToSqlBatch(strBox25Batch, "UPDATE FormControlsT SET Value = 0 WHERE ID = 381");
		}
		else if(nBox25Check == 2) {
			//force EIN to be checked
			bUseCheckboxOverride = TRUE;
			AddStatementToSqlBatch(strBox25Batch, "UPDATE FormControlsT SET Value = 0 WHERE ID = 380");
			AddStatementToSqlBatch(strBox25Batch, "UPDATE FormControlsT SET Value = 1 WHERE ID = 381");
		}
	}
	rs->Close();

	//load box25 preference - SSN or EIN
	//1 = SSN, 2 = EIN	
	if(m_HCFAInfo.Box25 == 1) {
		if(!bUseNumberOverride) {
			AddStatementToSqlBatch(strBox25Batch, "UPDATE FormControlsT SET Source = 'SocialSecurity' WHERE ID = 379");
		}
		if(!bUseCheckboxOverride) {
			AddStatementToSqlBatch(strBox25Batch, "UPDATE FormControlsT SET Value = 1 WHERE ID = 380");
			AddStatementToSqlBatch(strBox25Batch, "UPDATE FormControlsT SET Value = 0 WHERE ID = 381");
		}
	}
	else {
		if(!bUseNumberOverride) {
			AddStatementToSqlBatch(strBox25Batch, "UPDATE FormControlsT SET Source = 'Fed Employer ID' WHERE ID = 379");
		}
		if(!bUseCheckboxOverride) {
			AddStatementToSqlBatch(strBox25Batch, "UPDATE FormControlsT SET Value = 0 WHERE ID = 380");
			AddStatementToSqlBatch(strBox25Batch, "UPDATE FormControlsT SET Value = 1 WHERE ID = 381");
		}
	}

	//now execute our batch
	if(!strBox25Batch.IsEmpty()) {
		ExecuteSqlBatch(strBox25Batch);
	}
}

// (j.jones 2014-05-05 15:43) - PLID 61993 - this now takes in a Box17 person ID & qualifer override,
// which is -1 if we're just supposed to use the bill's referring physician
void LoadProviderInfo(long nProviderID, long nLocationID, long nBox17PersonID, CString strBox17PersonQualifier, bool bBox17_IsAProvider)
{
	CString str, strSQL;

	try {

		//get "accepted" status for this provider
		// (j.jones 2010-07-23 15:28) - PLID 39783 - accept assignment is calculated in GetAcceptAssignment_ByInsuredParty now
		BOOL bAccepted = GetAcceptAssignment_ByInsuredParty(g_nInsuredPartyID, nProviderID);

		int accepted = 65535;
		if(!bAccepted) {
			accepted = 0;
		}

		long ShowRespName = 0;
		int Accepted12Rule = 3, Accepted13Rule = 3;
		HCFABox13Over hb13Value = hb13_UseDefault;

		//load the configuration
		//0 - If Accepted, 1 - If Not Accepted, 2 - Always

		// (j.jones 2010-06-10 13:24) - PLID 39095 - added a second recordset to find the Box13 override
		_RecordsetPtr rs = CreateParamRecordset("SELECT Box12Accepted, Box13Accepted, ShowRespName FROM HCFASetupT WHERE ID = {INT}\r\n"
			" "
			"SELECT HCFABox13Over FROM BillsT WHERE ID = {INT}", fee_group_id, billid);

		if(!rs->eof) {
			// (j.jones 2010-07-22 11:49) - PLID 39779 - converted to use VarLong and reflect
			// that Box13 defaults to 3 (though neither are nullable fields so this was pointless)
			// (j.jones 2010-07-23 16:05) - PLID 39794 - Box 12 now also defaults to 3
			Accepted12Rule = AdoFldLong(rs, "Box12Accepted", 3);
			Accepted13Rule = AdoFldLong(rs, "Box13Accepted", 3);
			ShowRespName = rs->Fields->Item["ShowRespName"]->Value.lVal;
		}

		rs = rs->NextRecordset(NULL);

		if(!rs->eof) {
			hb13Value = (HCFABox13Over)AdoFldLong(rs, "HCFABox13Over", (long)hb13_UseDefault);		
		}
		rs->Close();

		//reinitialize
		// (j.jones 2008-05-01 11:00) - PLID 28467 - ensured that any non-empty relation to patient that isn't
		// standard will show as 'other'
		// (j.jones 2013-05-09 08:44) - PLID 54511 - removed InsType, replaced with a 1/0 value for each type
		g_aryFormQueries[20].sql.Format("SELECT InsuredPartyT.IDForInsurance AS InsID, InsPartyPersonT.[Last] + ',  ' + InsPartyPersonT.[First] + ' ' + InsPartyPersonT.[Middle] AS InsName, InsPartyPersonT.[Address1] + '  ' + InsPartyPersonT.[Address2] AS InsAdd, InsPartyPersonT.City AS InsCity, InsPartyPersonT.State AS InsState, InsPartyPersonT.Zip AS InsZip, InsPartyPersonT.HomePhone AS InsPhone, InsuredPartyT.PolicyGroupNum AS InsFECA, InsPartyPersonT.BirthDate AS InsBD, InsuredPartyT.[Employer] AS InsEmp, InsurancePlansT.PlanName AS InsPlan, InsPartyPersonT.Gender, InsPartyPersonT.Gender AS InsGender, InsuranceCoT.Name AS InsCoName, InsCoPersonT.Address1 AS InsCoAdd1, InsCoPersonT.Address2 AS InsCoAdd2, InsCoPersonT.City AS InsCoCity, InsCoPersonT.State AS InsCoState, InsCoPersonT.Zip AS InsCoZip, (CASE WHEN(InsuredPartyT.[RelationToPatient]='Self') THEN 1 WHEN (InsuredPartyT.[RelationToPatient]='Spouse') THEN 2 WHEN(InsuredPartyT.[RelationToPatient]='Child') THEN 3 WHEN (InsuredPartyT.[RelationToPatient] <> '') THEN 4 ELSE Null END) AS InsRel, "
				"CASE WHEN %li = 1 AND RespTypeT.Priority <> 1 THEN RespTypeT.TypeName ELSE '' END AS InsCoRespType, InsurancePlansT.PlanType, "
				"(CASE WHEN([InsurancePlansT].[PlanType]='Medicare') THEN 1 ELSE 0 END) AS InsTypeMedicare, "
				"(CASE WHEN ([InsurancePlansT].[PlanType]='Medicaid') THEN 1 ELSE 0 END) AS InsTypeMedicaid, "
				"(CASE WHEN ([InsurancePlansT].[PlanType]='Champus') THEN 1 ELSE 0 END) AS InsTypeChampus, "
				"(CASE WHEN ([InsurancePlansT].[PlanType]='Champva') THEN 1 ELSE 0 END) AS InsTypeChampva, "
				"(CASE WHEN ([InsurancePlansT].[PlanType]='Group Health Plan') THEN 1 ELSE 0 END) AS InsTypeGroupHealth, "
				"(CASE WHEN ([InsurancePlansT].[PlanType]='FECA Black Lung') THEN 1 ELSE 0 END) AS InsTypeFeca, "
				"(CASE WHEN ([InsurancePlansT].[PlanType]='Other') THEN 1 ELSE 0 END) AS InsTypeOther, "
				"%li AS Accepted, InsuredPartyT.PersonID, InsuredPartyT.PatientID FROM InsuranceCoT LEFT JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID LEFT JOIN InsurancePlansT ON InsuredPartyT.InsPlan = InsurancePlansT.[ID] LEFT JOIN (SELECT * FROM PersonT) AS InsCoPersonT ON InsuranceCoT.PersonID = InsCoPersonT.[ID] LEFT JOIN (SELECT * FROM PersonT) AS InsPartyPersonT ON InsuredPartyT.PersonID = InsPartyPersonT.[ID] LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID",ShowRespName,accepted);
		g_aryFormQueries[21].sql = "SELECT InsuredPartyT.IDForInsurance AS OthrInsID, InsPartyPersonT.[Last] + ',  ' + InsPartyPersonT.[First] + ' ' + InsPartyPersonT.[Middle] AS OthrInsName, InsPartyPersonT.Address1 + '  ' + InsPartyPersonT.Address2 AS OthrInsAdd, InsPartyPersonT.City AS OthrInsCity, InsPartyPersonT.State AS OthrInsState, InsPartyPersonT.Zip AS OthrInsZip, InsPartyPersonT.HomePhone AS OthrInsPhone, InsuredPartyT.IDForInsurance AS OthrInsIDForInsurance, InsPartyPersonT.BirthDate AS OthrInsBD, InsuredPartyT.Employer AS OthrInsEmp, InsurancePlansT.PlanName AS OthrInsPlan, InsPartyPersonT.Gender AS OthrGender, InsPartyPersonT.Gender AS OthrInsGender, InsuranceCoT.Name AS OthrInsCoName, InsCoPersonT.Address1 AS OthrInsCoAdd1, InsCoPersonT.Address2 AS OthrInsCoAdd2, InsCoPersonT.City AS OthrInsCoCity, InsCoPersonT.State AS OthrInsCoState, InsCoPersonT.Zip AS OthrInsCoZip, CASE WHEN (InsuredPartyT.[PersonID] Is Null) Then 0 ELSE 1 END AS Box11d, InsuredPartyT.PersonID, "
				"InsuranceCoT.TPLCode AS OthrInsTPLCode "
				"FROM (InsuranceCoT RIGHT JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID) "
				"LEFT JOIN InsurancePlansT ON InsuredPartyT.InsPlan = InsurancePlansT.ID LEFT JOIN (SELECT * FROM PersonT) AS InsPartyPersonT ON InsuredPartyT.PersonID = InsPartyPersonT.[ID] LEFT JOIN (SELECT * FROM PersonT) AS InsCoPersonT ON InsuranceCoT.PersonID = InsCoPersonT.[ID]";	


		// (j.jones 2007-03-01 16:03) - PLID 23297 - converted to use batched statements
		CString strProviderInfoBatch = BeginSqlBatch();		

		//we will try to fill in the boxes:
		//if the company is accepted and "If Accepted" is checked
		//if the company is not accepted and "If Not Accepted" is checked
		//if "Always" is checked
		if(((accepted && Accepted12Rule==1) || (!accepted && Accepted12Rule==2) || Accepted12Rule==3)) {
			AddStatementToSqlBatch(strProviderInfoBatch, "UPDATE FormControlsT SET Source = 'SignatureOnFile' WHERE ID = 199");
			
			//0 - the Print Date (today), 1 - the Bill Date
			if(ReturnsRecords("SELECT ID FROM HCFASetupT WHERE ID = %li AND Box12UseDate = 0", fee_group_id)) {
				AddStatementToSqlBatch(strProviderInfoBatch, "UPDATE FormControlsT SET Source = 'TodaysDate', FormID = 2 WHERE ID = 200");
			}
			else {
				AddStatementToSqlBatch(strProviderInfoBatch, "UPDATE FormControlsT SET Source = 'BillDate', FormID = 2 WHERE ID = 200");
			}
		}
		else {
			AddStatementToSqlBatch(strProviderInfoBatch, "UPDATE FormControlsT SET Source = '' WHERE ID = 199");
			AddStatementToSqlBatch(strProviderInfoBatch, "UPDATE FormControlsT SET Source = '' WHERE ID = 200");
		}
		// (j.jones 2010-06-10 13:19) - PLID 39095 - the bill's Box13 override can forcibly fill/empty this field
		if(hb13Value == hb13_Yes
			|| (hb13Value == hb13_UseDefault && (((accepted && Accepted13Rule==1) || (!accepted && Accepted13Rule==2) || Accepted13Rule==3))))
			AddStatementToSqlBatch(strProviderInfoBatch, "UPDATE FormControlsT SET Source = 'SignatureOnFile' WHERE ID = 207");
		else
			AddStatementToSqlBatch(strProviderInfoBatch, "UPDATE FormControlsT SET Source = '' WHERE ID = 207");

		// Pull the box 33 setup from the HCFA ID Setup table
		// (j.jones 2007-02-21 12:26) - PLID 24776 - added QualifierSpace
		rs = CreateParamRecordset("SELECT Box33Setup, Box33, Box33bQual, QualifierSpace, UseOverrideInBox31, LocationNPIUsage FROM HCFASetupT WHERE ID = {INT}", fee_group_id);
		// If we are using the provider in General 1 or an overwritten provider,
		// set that provider ID to all the charges in the HCFA charges table
		long Box33 = 1;
		long Box33SetupID = 1;
		long UseOverrideInBox31 = 0;
		long LocationNPIUsage = 1;
		long nQualifierSpace = 0;
		CString strBox33bQual = "";
		if (!rs->eof) {
			Box33 = rs->Fields->Item["Box33"]->Value.lVal;
			Box33SetupID = rs->Fields->Item["Box33Setup"]->Value.lVal;
			UseOverrideInBox31 = rs->Fields->Item["UseOverrideInBox31"]->Value.lVal;
			strBox33bQual = AdoFldString(rs, "Box33bQual","");
			LocationNPIUsage = AdoFldLong(rs, "LocationNPIUsage",1);
			nQualifierSpace = AdoFldLong(rs, "QualifierSpace",0);
		}
		rs->Close();

		// Pull the box 33 order from the HCFA ID Setup table
		rs = CreateParamRecordset("SELECT Box33Order, HidePhoneBox33 FROM HCFASetupT WHERE ID = {INT}", fee_group_id);	
		//1 - Last First Middle
		//2 - First Middle Last
		int Box33Order = 1;
		int HidePhoneBox33 = 0;
		if (!rs->eof) {
			Box33Order = rs->Fields->Item["Box33Order"]->Value.lVal;
			HidePhoneBox33 = rs->Fields->Item["HidePhoneBox33"]->Value.lVal;
		}
		rs->Close();

		if(Box33Order == 1)
			AddStatementToSqlBatch(strProviderInfoBatch, "UPDATE FormControlsT SET Source = 'DocNameLFM' WHERE ID = 412");
		else
			AddStatementToSqlBatch(strProviderInfoBatch, "UPDATE FormControlsT SET Source = 'DocNameFML' WHERE ID = 412");

		//now execute our batch
		if(!strProviderInfoBatch.IsEmpty()) {
			ExecuteSqlBatch(strProviderInfoBatch);
		}

		BOOL bUseContactAddress = ReturnsRecords("SELECT DocAddress FROM HCFASetupT WHERE ID = %li AND DocAddress = 2", fee_group_id);

		// (j.jones 2007-05-07 15:12) - PLID 25922 - added Box33Name override
		// (j.jones 2010-02-01 10:01) - PLID 37137 - added Box33 address overrides
		CString strBox33NameOver = "", strBox33Address1_Over = "", strBox33Address2_Over = "",
			strBox33City_Over = "", strBox33State_Over = "", strBox33Zip_Over = "";
		_RecordsetPtr rsBox33Over = CreateParamRecordset("SELECT Box33Name, "
			"Box33_Address1, Box33_Address2, Box33_City, Box33_State, Box33_Zip "
			"FROM AdvHCFAPinT WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT}", nProviderID, nLocationID, fee_group_id);
		if(!rsBox33Over->eof) {
			strBox33NameOver = AdoFldString(rsBox33Over, "Box33Name","");
			strBox33NameOver.TrimLeft();
			strBox33NameOver.TrimRight();
			strBox33Address1_Over = AdoFldString(rsBox33Over, "Box33_Address1","");
			strBox33Address1_Over.TrimLeft();
			strBox33Address1_Over.TrimRight();
			strBox33Address2_Over = AdoFldString(rsBox33Over, "Box33_Address2","");
			strBox33Address2_Over.TrimLeft();
			strBox33Address2_Over.TrimRight();
			strBox33City_Over = AdoFldString(rsBox33Over, "Box33_City","");
			strBox33City_Over.TrimLeft();
			strBox33City_Over.TrimRight();
			strBox33State_Over = AdoFldString(rsBox33Over, "Box33_State","");
			strBox33State_Over.TrimLeft();
			strBox33State_Over.TrimRight();
			strBox33Zip_Over = AdoFldString(rsBox33Over, "Box33_Zip","");
			strBox33Zip_Over.TrimLeft();
			strBox33Zip_Over.TrimRight();
		}
		rsBox33Over->Close();

		////////////////////////////////////////////////
		// Step 1: Setting the address
		switch (Box33SetupID) {
		case 3:
		{// Use overridden provider for box 33 physician name
				// (Note: if the main insurance is not in a group this should never happen)
				_RecordsetPtr rs;
				ASSERT(fee_group_id != -1);

				rs = CreateParamRecordset("SELECT Box33Num FROM HCFASetupT WHERE Box33Num > 0 AND ID = {INT}",fee_group_id);

				if (rs->eof) {
					AfxMessageBox("The new HCFA could not pull the override address for Box 33. Please ensure it is filled in correctly.");
					return;
				}

				// (j.jones 2006-12-01 11:55) - PLID 22110 - intentionally does not use the ClaimProviderID
				// (j.jones 2010-02-01 10:01) - PLID 37137 - supported Box33 address overrides
				strSQL.Format("SELECT "
					"CASE WHEN '%s' <> '' THEN '%s' ELSE DoctorsT.Note END AS DocNameFML, "
					"CASE WHEN '%s' <> '' THEN '%s' ELSE DoctorsT.Note END AS DocNameLFM, "
					"DoctorsT.SocialSecurity, DoctorsT.[Fed Employer ID], DoctorsT.[DEA Number], "
					"DoctorsT.[BCBS Number], "
					"CASE WHEN '%s' <> '' THEN '%s' ELSE DoctorsT.[Address1] END + ' ' + CASE WHEN '%s' <> '' THEN '%s' ELSE DoctorsT.[Address2] END AS DocAdd, "
					"CASE WHEN '%s' <> '' THEN '%s' ELSE DoctorsT.City END + ', ' + CASE WHEN '%s' <> '' THEN '%s' ELSE DoctorsT.State END + ' ' + CASE WHEN '%s' <> '' THEN '%s' ELSE DoctorsT.Zip END AS DocCityStateZip, "
					"DoctorsT.WorkPhone AS DocPhone, "
					"CustomData1.TextParam AS Custom1, CustomData2.TextParam AS Custom2, CustomData3.TextParam AS Custom3, CustomData4.TextParam AS Custom4 "
					"FROM (SELECT * FROM PersonT LEFT JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID) AS DoctorsT "
					"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 6) AS CustomData1 ON DoctorsT.PersonID = CustomData1.PersonID "
					"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 7) AS CustomData2 ON DoctorsT.PersonID = CustomData2.PersonID "
					"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 8) AS CustomData3 ON DoctorsT.PersonID = CustomData3.PersonID "
					"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 9) AS CustomData4 ON DoctorsT.PersonID = CustomData4.PersonID "
					"WHERE DoctorsT.ID = %li", _Q(strBox33NameOver), _Q(strBox33NameOver), _Q(strBox33NameOver), _Q(strBox33NameOver),
					_Q(strBox33Address1_Over), _Q(strBox33Address1_Over), _Q(strBox33Address2_Over), _Q(strBox33Address2_Over),
					_Q(strBox33City_Over), _Q(strBox33City_Over), _Q(strBox33State_Over), _Q(strBox33State_Over), _Q(strBox33Zip_Over), _Q(strBox33Zip_Over),
					rs->Fields->Item["Box33Num"]->Value.lVal);
				rs->Close();
				}			
			break;
		case 2:
			//use general 1 provider
			if(!bUseContactAddress) {
				// (j.jones 2006-12-01 13:01) - PLID 22110 - supported the ClaimProviderID override
				// (j.jones 2008-04-04 10:08) - PLID 28995 - we don't need the ClaimProvider loading here,
				// this function already has the doc_id sent to us - if it is the G1 provider, we already have that
				// (j.jones 2010-02-01 10:01) - PLID 37137 - supported Box33 address overrides
				strSQL.Format("SELECT %li AS FirstOfDoctorsProviders, "
					"CASE WHEN '%s' <> '' THEN '%s' ELSE DoctorsT.[First] + ' ' + DoctorsT.Middle + ' ' + DoctorsT.[Last] + (CASE WHEN (DoctorsT.Title <> '') THEN (', ' + DoctorsT.Title) ELSE ('') END) END AS DocNameFML, "
					"CASE WHEN '%s' <> '' THEN '%s' ELSE DoctorsT.[Last] + (CASE WHEN (DoctorsT.Title <> '') THEN (', ' + DoctorsT.Title) ELSE ('') END) + ', ' + DoctorsT.[First] + ' ' + DoctorsT.Middle END AS DocNameLFM, "
					"GetDate() AS TodaysDate, DoctorsT.[First] + ' ' + DoctorsT.[Middle] + ' ' + DoctorsT.[Last] + (CASE WHEN (DoctorsT.Title <> '') THEN (', ' + DoctorsT.Title) ELSE ('') END) AS DocSignature, "
					"DoctorsT.SocialSecurity, DoctorsT.[Fed Employer ID], DoctorsT.[DEA Number], DoctorsT.[BCBS Number], DoctorsT.[Medicare Number], DoctorsT.TaxonomyCode, DoctorsT.[Medicaid Number], DoctorsT.[Workers Comp Number], DoctorsT.[Other ID Number], DoctorsT.NPI, DoctorsT.UPIN, "
					"CASE WHEN '%s' <> '' THEN '%s' ELSE LocationsT.Address1 END + ' ' + CASE WHEN '%s' <> '' THEN '%s' ELSE LocationsT.Address2 END AS DocAdd, "
					"CASE WHEN '%s' <> '' THEN '%s' ELSE LocationsT.City END + ', ' + CASE WHEN '%s' <> '' THEN '%s' ELSE LocationsT.State END + ' ' + CASE WHEN '%s' <> '' THEN '%s' ELSE LocationsT.Zip END AS DocCityStateZip, "
					"LocationsT.Phone AS DocPhone, "
					"CustomData1.TextParam AS Custom1, CustomData2.TextParam AS Custom2, CustomData3.TextParam AS Custom3, CustomData4.TextParam AS Custom4 "
					"FROM (SELECT * FROM PersonT LEFT JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID) AS DoctorsT "
					"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 6) AS CustomData1 ON DoctorsT.PersonID = CustomData1.PersonID "
					"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 7) AS CustomData2 ON DoctorsT.PersonID = CustomData2.PersonID "
					"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 8) AS CustomData3 ON DoctorsT.PersonID = CustomData3.PersonID "
					"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 9) AS CustomData4 ON DoctorsT.PersonID = CustomData4.PersonID "
					"CROSS JOIN LocationsT WHERE DoctorsT.ID = %li AND LocationsT.ID = %li", nProviderID,
					_Q(strBox33NameOver), _Q(strBox33NameOver), _Q(strBox33NameOver), _Q(strBox33NameOver),
					_Q(strBox33Address1_Over), _Q(strBox33Address1_Over), _Q(strBox33Address2_Over), _Q(strBox33Address2_Over),
					_Q(strBox33City_Over), _Q(strBox33City_Over), _Q(strBox33State_Over), _Q(strBox33State_Over), _Q(strBox33Zip_Over), _Q(strBox33Zip_Over),
					nProviderID, nLocationID);
			}
			else {
				// (j.jones 2006-12-01 13:01) - PLID 22110 - supported the ClaimProviderID override
				// (j.jones 2008-04-04 10:08) - PLID 28995 - we don't need the ClaimProvider loading here,
				// this function already has the doc_id sent to us - if it is the G1 provider, we already have that
				// (j.jones 2010-02-01 10:01) - PLID 37137 - supported Box33 address overrides
				strSQL.Format("SELECT %li AS FirstOfDoctorsProviders, "
					"CASE WHEN '%s' <> '' THEN '%s' ELSE DoctorsT.[First] + ' ' + DoctorsT.Middle + ' ' + DoctorsT.[Last] + (CASE WHEN (DoctorsT.Title <> '') THEN (', ' + DoctorsT.Title) ELSE ('') END) END AS DocNameFML, "
					"CASE WHEN '%s' <> '' THEN '%s' ELSE DoctorsT.[Last] + (CASE WHEN (DoctorsT.Title <> '') THEN (', ' + DoctorsT.Title) ELSE ('') END) + ', ' + DoctorsT.[First] + ' ' + DoctorsT.Middle END AS DocNameLFM, "
					"GetDate() AS TodaysDate, DoctorsT.[First] + ' ' + DoctorsT.[Middle] + ' ' + DoctorsT.[Last] + (CASE WHEN (DoctorsT.Title <> '') THEN (', ' + DoctorsT.Title) ELSE ('') END) AS DocSignature, "
					"DoctorsT.SocialSecurity, DoctorsT.[Fed Employer ID], DoctorsT.[DEA Number], DoctorsT.[BCBS Number], DoctorsT.[Medicare Number], DoctorsT.TaxonomyCode, DoctorsT.[Medicaid Number], DoctorsT.[Workers Comp Number], DoctorsT.[Other ID Number], DoctorsT.NPI, DoctorsT.UPIN, "
					"CASE WHEN '%s' <> '' THEN '%s' ELSE DoctorsT.[Address1] END + ' ' + CASE WHEN '%s' <> '' THEN '%s' ELSE DoctorsT.[Address2] END AS DocAdd, "
					"CASE WHEN '%s' <> '' THEN '%s' ELSE DoctorsT.City END + ', ' + CASE WHEN '%s' <> '' THEN '%s' ELSE DoctorsT.State END + ' ' + CASE WHEN '%s' <> '' THEN '%s' ELSE DoctorsT.Zip END AS DocCityStateZip, "
					"DoctorsT.WorkPhone AS DocPhone, "
					"CustomData1.TextParam AS Custom1, CustomData2.TextParam AS Custom2, CustomData3.TextParam AS Custom3, CustomData4.TextParam AS Custom4 "
					"FROM (SELECT * FROM PersonT LEFT JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID) AS DoctorsT "
					"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 6) AS CustomData1 ON DoctorsT.PersonID = CustomData1.PersonID "
					"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 7) AS CustomData2 ON DoctorsT.PersonID = CustomData2.PersonID "
					"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 8) AS CustomData3 ON DoctorsT.PersonID = CustomData3.PersonID "
					"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 9) AS CustomData4 ON DoctorsT.PersonID = CustomData4.PersonID "
					"WHERE DoctorsT.ID = %li", nProviderID,
					_Q(strBox33NameOver), _Q(strBox33NameOver), _Q(strBox33NameOver), _Q(strBox33NameOver),
					_Q(strBox33Address1_Over), _Q(strBox33Address1_Over), _Q(strBox33Address2_Over), _Q(strBox33Address2_Over),
					_Q(strBox33City_Over), _Q(strBox33City_Over), _Q(strBox33State_Over), _Q(strBox33State_Over), _Q(strBox33Zip_Over), _Q(strBox33Zip_Over),
					nProviderID);
			}
			break;
		case 4:
			//use bill location

			// (j.jones 2006-12-01 11:58) - PLID 22110 - needs not use the ClaimProviderID, doc_id is already that number
			// (j.jones 2010-02-01 10:01) - PLID 37137 - supported Box33 address overrides
			strSQL.Format("SELECT %li AS FirstOfDoctorsProviders, "
					"CASE WHEN '%s' <> '' THEN '%s' ELSE LocationsT.Name END AS DocNameFML, "
					"CASE WHEN '%s' <> '' THEN '%s' ELSE LocationsT.Name END AS DocNameLFM, "
					"GetDate() AS TodaysDate, DoctorsT.[First] + ' ' + DoctorsT.[Middle] + ' ' + DoctorsT.[Last] + (CASE WHEN (DoctorsT.Title <> '') THEN (', ' + DoctorsT.Title) ELSE ('') END) AS DocSignature, "
					"DoctorsT.SocialSecurity, LocationsT.EIN AS [Fed Employer ID], DoctorsT.[DEA Number], DoctorsT.[BCBS Number], DoctorsT.[Medicare Number], DoctorsT.TaxonomyCode, DoctorsT.[Medicaid Number], DoctorsT.[Workers Comp Number], DoctorsT.[Other ID Number], DoctorsT.NPI, DoctorsT.UPIN, "
					"CASE WHEN '%s' <> '' THEN '%s' ELSE LocationsT.Address1 END + ' ' + CASE WHEN '%s' <> '' THEN '%s' ELSE LocationsT.Address2 END AS DocAdd, "
					"CASE WHEN '%s' <> '' THEN '%s' ELSE LocationsT.City END + ', ' + CASE WHEN '%s' <> '' THEN '%s' ELSE LocationsT.State END + ' ' + CASE WHEN '%s' <> '' THEN '%s' ELSE LocationsT.Zip END AS DocCityStateZip, "
					"LocationsT.Phone AS DocPhone, "
					"CustomData1.TextParam AS Custom1, CustomData2.TextParam AS Custom2, CustomData3.TextParam AS Custom3, CustomData4.TextParam AS Custom4 "
					"FROM (SELECT * FROM PersonT LEFT JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID) AS DoctorsT "
					"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 6) AS CustomData1 ON DoctorsT.PersonID = CustomData1.PersonID "
					"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 7) AS CustomData2 ON DoctorsT.PersonID = CustomData2.PersonID "
					"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 8) AS CustomData3 ON DoctorsT.PersonID = CustomData3.PersonID "
					"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 9) AS CustomData4 ON DoctorsT.PersonID = CustomData4.PersonID "
					"CROSS JOIN LocationsT WHERE DoctorsT.ID = %li AND LocationsT.ID = %li", nProviderID,
					_Q(strBox33NameOver), _Q(strBox33NameOver), _Q(strBox33NameOver), _Q(strBox33NameOver),
					_Q(strBox33Address1_Over), _Q(strBox33Address1_Over), _Q(strBox33Address2_Over), _Q(strBox33Address2_Over),
					_Q(strBox33City_Over), _Q(strBox33City_Over), _Q(strBox33State_Over), _Q(strBox33State_Over), _Q(strBox33Zip_Over), _Q(strBox33Zip_Over),
					nProviderID, nLocationID);
			break;
		default: 
			if(!bUseContactAddress) {
				// (j.jones 2006-12-01 11:58) - PLID 22110 - needs not use the ClaimProviderID, doc_id is already that number
				// (j.jones 2010-02-01 10:01) - PLID 37137 - supported Box33 address overrides
				strSQL.Format("SELECT %li AS FirstOfDoctorsProviders, "
					"CASE WHEN '%s' <> '' THEN '%s' ELSE DoctorsT.[First] + ' ' + DoctorsT.Middle + ' ' + DoctorsT.[Last] + (CASE WHEN (DoctorsT.Title <> '') THEN (', ' + DoctorsT.Title) ELSE ('') END) END AS DocNameFML, "
					"CASE WHEN '%s' <> '' THEN '%s' ELSE DoctorsT.[Last] + (CASE WHEN (DoctorsT.Title <> '') THEN (', ' + DoctorsT.Title) ELSE ('') END) + ', ' + DoctorsT.[First] + ' ' + DoctorsT.Middle END AS DocNameLFM, "
					"GetDate() AS TodaysDate, DoctorsT.[First] + ' ' + DoctorsT.[Middle] + ' ' + DoctorsT.[Last] + (CASE WHEN (DoctorsT.Title <> '') THEN (', ' + DoctorsT.Title) ELSE ('') END) AS DocSignature, "
					"DoctorsT.SocialSecurity, DoctorsT.[Fed Employer ID], DoctorsT.[DEA Number], DoctorsT.[BCBS Number], DoctorsT.[Medicare Number], DoctorsT.TaxonomyCode, DoctorsT.[Medicaid Number], DoctorsT.[Workers Comp Number], DoctorsT.[Other ID Number], DoctorsT.NPI, DoctorsT.UPIN, "
					"CASE WHEN '%s' <> '' THEN '%s' ELSE LocationsT.Address1 END + ' ' + CASE WHEN '%s' <> '' THEN '%s' ELSE LocationsT.Address2 END AS DocAdd, "
					"CASE WHEN '%s' <> '' THEN '%s' ELSE LocationsT.City END + ', ' + CASE WHEN '%s' <> '' THEN '%s' ELSE LocationsT.State END + ' ' + CASE WHEN '%s' <> '' THEN '%s' ELSE LocationsT.Zip END AS DocCityStateZip, "
					"LocationsT.Phone AS DocPhone, "
					"CustomData1.TextParam AS Custom1, CustomData2.TextParam AS Custom2, CustomData3.TextParam AS Custom3, CustomData4.TextParam AS Custom4 "
					"FROM (SELECT * FROM PersonT LEFT JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID) AS DoctorsT "
					"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 6) AS CustomData1 ON DoctorsT.PersonID = CustomData1.PersonID "
					"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 7) AS CustomData2 ON DoctorsT.PersonID = CustomData2.PersonID "
					"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 8) AS CustomData3 ON DoctorsT.PersonID = CustomData3.PersonID "
					"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 9) AS CustomData4 ON DoctorsT.PersonID = CustomData4.PersonID "
					"CROSS JOIN LocationsT WHERE DoctorsT.ID = %li AND LocationsT.ID = %li", nProviderID,
					_Q(strBox33NameOver), _Q(strBox33NameOver), _Q(strBox33NameOver), _Q(strBox33NameOver),
					_Q(strBox33Address1_Over), _Q(strBox33Address1_Over), _Q(strBox33Address2_Over), _Q(strBox33Address2_Over),
					_Q(strBox33City_Over), _Q(strBox33City_Over), _Q(strBox33State_Over), _Q(strBox33State_Over), _Q(strBox33Zip_Over), _Q(strBox33Zip_Over),
					nProviderID, nLocationID);
			}
			else {
				// (j.jones 2006-12-01 11:58) - PLID 22110 - needs not use the ClaimProviderID, nProviderID is already that number
				// (j.jones 2010-02-01 10:01) - PLID 37137 - supported Box33 address overrides
				strSQL.Format("SELECT %li AS FirstOfDoctorsProviders, "
					"CASE WHEN '%s' <> '' THEN '%s' ELSE DoctorsT.[First] + ' ' + DoctorsT.Middle + ' ' + DoctorsT.[Last] + (CASE WHEN (DoctorsT.Title <> '') THEN (', ' + DoctorsT.Title) ELSE ('') END) END AS DocNameFML, "
					"CASE WHEN '%s' <> '' THEN '%s' ELSE DoctorsT.[Last] + (CASE WHEN (DoctorsT.Title <> '') THEN (', ' + DoctorsT.Title) ELSE ('') END) + ', ' + DoctorsT.[First] + ' ' + DoctorsT.Middle END AS DocNameLFM, "
					"GetDate() AS TodaysDate, DoctorsT.[First] + ' ' + DoctorsT.[Middle] + ' ' + DoctorsT.[Last] + (CASE WHEN (DoctorsT.Title <> '') THEN (', ' + DoctorsT.Title) ELSE ('') END) AS DocSignature, "
					"DoctorsT.SocialSecurity, DoctorsT.[Fed Employer ID], DoctorsT.[DEA Number], DoctorsT.[BCBS Number], DoctorsT.[Medicare Number], DoctorsT.TaxonomyCode, DoctorsT.[Medicaid Number], DoctorsT.[Workers Comp Number], DoctorsT.[Other ID Number], DoctorsT.NPI, DoctorsT.UPIN, "
					"CASE WHEN '%s' <> '' THEN '%s' ELSE DoctorsT.[Address1] END + ' ' + CASE WHEN '%s' <> '' THEN '%s' ELSE DoctorsT.[Address2] END AS DocAdd, "
					"CASE WHEN '%s' <> '' THEN '%s' ELSE DoctorsT.City END + ', ' + CASE WHEN '%s' <> '' THEN '%s' ELSE DoctorsT.State END + ' ' + CASE WHEN '%s' <> '' THEN '%s' ELSE DoctorsT.Zip END AS DocCityStateZip, "
					"DoctorsT.WorkPhone AS DocPhone, "
					"CustomData1.TextParam AS Custom1, CustomData2.TextParam AS Custom2, CustomData3.TextParam AS Custom3, CustomData4.TextParam AS Custom4 "
					"FROM (SELECT * FROM PersonT LEFT JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID) AS DoctorsT "
					"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 6) AS CustomData1 ON DoctorsT.PersonID = CustomData1.PersonID "
					"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 7) AS CustomData2 ON DoctorsT.PersonID = CustomData2.PersonID "
					"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 8) AS CustomData3 ON DoctorsT.PersonID = CustomData3.PersonID "
					"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 9) AS CustomData4 ON DoctorsT.PersonID = CustomData4.PersonID "
					"WHERE DoctorsT.ID = %li", nProviderID,
					_Q(strBox33NameOver), _Q(strBox33NameOver), _Q(strBox33NameOver), _Q(strBox33NameOver),
					_Q(strBox33Address1_Over), _Q(strBox33Address1_Over), _Q(strBox33Address2_Over), _Q(strBox33Address2_Over),
					_Q(strBox33City_Over), _Q(strBox33City_Over), _Q(strBox33State_Over), _Q(strBox33State_Over), _Q(strBox33Zip_Over), _Q(strBox33Zip_Over),
					nProviderID);
			}
			break;
		}
		g_aryFormQueries[33].sql = strSQL;

		////////////////////////////////////////////////
		// Step 2: Setting the PIN number source. 
		// TODO: Most of the fields are absolutely unnecessary, just
		// get rid of them

		CString strBox33bNum;

		// (j.jones 2007-02-21 12:31) - PLID 24776 - supported putting a space between the qualifier and the ID
		switch (Box33) {
			case 1: strBox33bNum.Format("'%s' + '%s' + NPI", _Q(strBox33bQual), nQualifierSpace == 1 ? " " : ""); break;
			case 2: strBox33bNum.Format("'%s' + '%s' + SocialSecurity", _Q(strBox33bQual), nQualifierSpace == 1 ? " " : ""); break;
			case 3: strBox33bNum.Format("'%s' + '%s' + [Fed Employer ID]", _Q(strBox33bQual), nQualifierSpace == 1 ? " " : ""); break;
			case 4: strBox33bNum.Format("'%s' + '%s' + [DEA Number]", _Q(strBox33bQual), nQualifierSpace == 1 ? " " : ""); break;
			case 5: strBox33bNum.Format("'%s' + '%s' + [BCBS Number]", _Q(strBox33bQual), nQualifierSpace == 1 ? " " : ""); break;
			case 6: strBox33bNum.Format("'%s' + '%s' + [Medicare Number]", _Q(strBox33bQual), nQualifierSpace == 1 ? " " : ""); break;
			case 7: strBox33bNum.Format("'%s' + '%s' + [Medicaid Number]", _Q(strBox33bQual), nQualifierSpace == 1 ? " " : ""); break;
			case 8: strBox33bNum.Format("'%s' + '%s' + [Workers Comp Number]", _Q(strBox33bQual), nQualifierSpace == 1 ? " " : ""); break;
			case 9: strBox33bNum.Format("'%s' + '%s' + [Other ID Number]", _Q(strBox33bQual), nQualifierSpace == 1 ? " " : ""); break;
			case 11: strBox33bNum.Format("'%s' + '%s' + UPIN", _Q(strBox33bQual), nQualifierSpace == 1 ? " " : ""); break;
			case 12: strBox33bNum.Format("'%s' + '%s' + Box24J", _Q(strBox33bQual), nQualifierSpace == 1 ? " " : ""); break;
			// (j.jones 2007-01-16 15:24) - PLID 24273 - added support for old GRP number
			case 13: strBox33bNum.Format("'%s' + '%s' + [GRPNumber]", _Q(strBox33bQual), nQualifierSpace == 1 ? " " : ""); break;
			// (j.jones 2007-04-24 12:21) - PLID 25764 - supported Taxonomy Code
			case 14: strBox33bNum.Format("'%s' + '%s' + TaxonomyCode", _Q(strBox33bQual), nQualifierSpace == 1 ? " " : ""); break;
			case 16: strBox33bNum.Format("'%s' + '%s' + CustomData1.TextParam", _Q(strBox33bQual), nQualifierSpace == 1 ? " " : ""); break;
			case 17: strBox33bNum.Format("'%s' + '%s' + CustomData2.TextParam", _Q(strBox33bQual), nQualifierSpace == 1 ? " " : ""); break;
			case 18: strBox33bNum.Format("'%s' + '%s' + CustomData3.TextParam", _Q(strBox33bQual), nQualifierSpace == 1 ? " " : ""); break;
			case 19: strBox33bNum.Format("'%s' + '%s' + CustomData4.TextParam", _Q(strBox33bQual), nQualifierSpace == 1 ? " " : ""); break;
			default: strBox33bNum = "''"; break;
		}

		switch (Box33SetupID) {

		case 2:
			{
				// Use general 1 provider

				// (j.jones 2011-05-31 16:09) - PLID 43775 - use the mapped 24J provider ID
				long nBox24JProviderID = -1;
				mapBox24JProviderIDs.Lookup(nProviderID, nBox24JProviderID);
				if(nBox24JProviderID == -1) {
					//this shouldn't be -1, but if it is, use the provider ID
					nBox24JProviderID = nProviderID;
				}

				_RecordsetPtr nxrs;
				CString str,str1,Box24J,Box24I,Box33aNPIOver;
				// (j.jones 2006-12-01 13:01) - PLID 22110 - supported the ClaimProviderID override
				// (j.jones 2008-04-04 10:17) - PLID 28995 - We don't need to load the claim provider ID because
				// this function already has the correct one in the form of nProviderID. We also only need the 
				// Box 33 provider ID here for showing that provider's Box 24J number.
				// (j.jones 2011-05-31 16:09) - PLID 43775 - use the mapped 24J provider ID
				nxrs = CreateParamRecordset("SELECT InsuranceBox24J.Box24IQualifier AS Box24I, InsuranceBox24J.Box24JNumber AS Box24J "
					"FROM BillsT "
					"INNER JOIN InsuredPartyT ON BillsT.InsuredPartyID = InsuredPartyT.PersonID "
					"INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
					"INNER JOIN InsuranceBox24J ON InsuranceBox24J.InsCoID = InsuredPartyT.InsuranceCoID "
					"WHERE BillsT.ID = {INT} AND InsuranceBox24J.ProviderID = {INT}",billid, nBox24JProviderID);
				_variant_t v;
				if(!nxrs->eof) {
					v = nxrs->Fields->Item["Box24J"]->Value;
					if(v.vt != VT_NULL && v.vt != VT_EMPTY)
						Box24J = CString(v.bstrVal);
					else
						Box24J = " ";
					v = nxrs->Fields->Item["Box24I"]->Value;
					if(v.vt != VT_NULL && v.vt != VT_EMPTY)
						Box24I = CString(v.bstrVal);
					else
						Box24I = "";
				}
				else {
					Box24J = " ";
					Box24I = "";
				}
				nxrs->Close();

				//if they are wanting the Box24J number in Box33b, set that up now,
				//but continue to use the qualifier from the HCFA Setup
				if(Box33 == 12) {
					// (j.jones 2007-02-21 12:31) - PLID 24776 - supported putting a space between the qualifier and the ID
					strBox33bNum.Format("'%s%s%s'", _Q(strBox33bQual), nQualifierSpace == 1 ? " " : "", _Q(Box24J));
				}

				//Load the override for PIN and for Box24J
				// (j.jones 2007-02-22 16:54) - PLID 24776 - supported putting a space between the qualifier and the ID
				// (j.jones 2007-08-08 10:29) - PLID 25395 - added Box33aNPI to the AdvHCFAPinT
				// (j.jones 2011-05-31 16:09) - PLID 43775 - use the mapped 24J provider ID
				nxrs = CreateParamRecordset("SELECT Box24JQual, Box24J FROM AdvHCFAPinT WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT}", nQualifierSpace == 1 ? " " : "", nBox24JProviderID, nLocationID, fee_group_id);
				if(!nxrs->eof) {
					v = nxrs->Fields->Item["Box24J"]->Value;
					if(v.vt == VT_BSTR && CString(v.bstrVal).GetLength() > 0)
						Box24J = CString(v.bstrVal);

					v = nxrs->Fields->Item["Box24JQual"]->Value;
					if(v.vt == VT_BSTR && CString(v.bstrVal).GetLength() > 0)
						Box24I = CString(v.bstrVal);
				}
				nxrs->Close();

				// (d.thompson 2011-09-07) - PLID 45215 - Someone incorrectly parameterized this, causing overrides for 33a and 33b to stop working.  There is 
				//	an extra %s which is not a parameter here.
				nxrs = CreateParamRecordset(
					FormatString("SELECT Box33aNPI, CASE WHEN PIN IS NULL THEN NULL ELSE (CASE WHEN Box33bQual IS NULL THEN '' ELSE Box33bQual END) + '%s' + PIN END AS Box33b FROM AdvHCFAPinT WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT}", nQualifierSpace == 1 ? " " : ""), 
					nProviderID, nLocationID, fee_group_id);
				if(!nxrs->eof) {
					v = nxrs->Fields->Item["Box33b"]->Value;
					if(v.vt == VT_BSTR)
						strBox33bNum = "'" + _Q(CString(v.bstrVal)) + "'";

					v = nxrs->Fields->Item["Box33aNPI"]->Value;
					if(v.vt == VT_BSTR && CString(v.bstrVal).GetLength() > 0)
						Box33aNPIOver = "'" + _Q(CString(v.bstrVal)) + "'";
				}
				nxrs->Close();

				CString strNPI = "ProvidersT.NPI";

				// (j.jones 2007-08-08 10:31) - PLID 25395 - supported the Box33aNPI override
				if(!Box33aNPIOver.IsEmpty()) {
					strNPI = Box33aNPIOver;
				}
				else {

					// (j.jones 2007-01-18 11:35) - PLID 24264 - we have an option now to use
					// the Location NPI instead of the Provider NPI. This is only for Box 33a.
					if(LocationNPIUsage == 1) {
						strNPI.Format("(SELECT NPI FROM LocationsT WHERE ID = %li)", nLocationID);
					}
				}

				// (j.jones 2006-12-01 13:01) - PLID 22110 - supported the ClaimProviderID override
				// (j.jones 2008-04-04 15:35) - PLID 28995 - We don't need to load the claim provider ID because
				// this function already has the correct one in the form of nProviderID
				strSQL.Format("SELECT ProvidersT.PersonID AS FirstOfDoctorsProviders, GetDate() AS TodaysDate, PersonT.[First] + ' ' + PersonT.[Middle] + ' ' + PersonT.[Last] + (CASE WHEN (PersonT.Title <> '') THEN (', ' + PersonT.Title) ELSE ('') END) AS DocSignature, PersonT.SocialSecurity, ProvidersT.[Fed Employer ID], ProvidersT.[DEA Number], ProvidersT.[BCBS Number], ProvidersT.[Medicare Number], ProvidersT.TaxonomyCode, ProvidersT.[Medicaid Number], ProvidersT.[Workers Comp Number], ProvidersT.[Other ID Number], "
							"ProvidersT.NPI, ProvidersT.UPIN, PersonT.Address1 + ' ' + PersonT.Address2 AS DocAdd, PersonT.City + ', ' + PersonT.State + ' ' + PersonT.Zip AS DocCityStateZip, PersonT.WorkPhone AS DocPhone, "
							"CustomData1.TextParam AS Custom1, CustomData2.TextParam AS Custom2, CustomData3.TextParam AS Custom3, CustomData4.TextParam AS Custom4, '%s' AS Box24IQual, '%s' AS Box24JID, %s AS Box33bNum, %s AS Box33aNPI "
							"FROM PersonT INNER JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID "
							"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 6) AS CustomData1 ON ProvidersT.PersonID = CustomData1.PersonID "
							"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 7) AS CustomData2 ON ProvidersT.PersonID = CustomData2.PersonID "
							"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 8) AS CustomData3 ON ProvidersT.PersonID = CustomData3.PersonID "
							"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 9) AS CustomData4 ON ProvidersT.PersonID = CustomData4.PersonID "
							"WHERE Coalesce(ProvidersT.PersonID,-1) = %li", _Q(Box24I), _Q(Box24J), strBox33bNum, strNPI, nProviderID);
			}
			break;

		case 3:
		case 4:
		case 1:
		default:
			{
				// Use bill providers

				_RecordsetPtr nxrs;
				CString str,Box24J,Box24I, Box33aNPIOver;

				// (j.jones 2011-05-31 16:09) - PLID 43775 - use the mapped 24J provider ID
				long nBox24JProviderID = -1;
				mapBox24JProviderIDs.Lookup(nProviderID, nBox24JProviderID);
				if(nBox24JProviderID == -1) {
					//this shouldn't be -1, but if it is, use the provider ID
					nBox24JProviderID = nProviderID;
				}

				//Load Box24J

				// (j.jones 2006-12-01 13:01) - PLID 22110 - supported the ClaimProviderID override
				// (j.jones 2008-04-04 10:17) - PLID 28995 - We don't need to load the claim provider ID because
				// this function already has the correct one in the form of nProviderID. We also only need the 
				// Box 33 provider ID here for showing that provider's Box 24J number.
				// (j.jones 2011-05-31 16:09) - PLID 43775 - use the mapped 24J provider ID
				nxrs = CreateParamRecordset("SELECT InsuranceBox24J.Box24IQualifier AS Box24I, InsuranceBox24J.Box24JNumber AS Box24J "
					"FROM BillsT "
					"INNER JOIN InsuredPartyT ON BillsT.InsuredPartyID = InsuredPartyT.PersonID "
					"INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
					"INNER JOIN InsuranceBox24J ON InsuranceBox24J.InsCoID = InsuredPartyT.InsuranceCoID "
					"WHERE BillsT.ID = {INT} AND InsuranceBox24J.ProviderID = {INT}",billid, nBox24JProviderID);
				_variant_t v;
				if(!nxrs->eof) {
					v = nxrs->Fields->Item["Box24J"]->Value;
					if(v.vt != VT_NULL && v.vt != VT_EMPTY)
						Box24J = CString(v.bstrVal);
					else
						Box24J = " ";
					v = nxrs->Fields->Item["Box24I"]->Value;
					if(v.vt != VT_NULL && v.vt != VT_EMPTY)
						Box24I = CString(v.bstrVal);
					else
						Box24I = "";
				}
				else {
					Box24J = " ";
					Box24I = "";
				}
				nxrs->Close();

				//if they are wanting the Box24J number in Box33b, set that up now,
				//but continue to use the qualifier from the HCFA Setup
				if(Box33 == 12) {
					// (j.jones 2007-02-21 12:31) - PLID 24776 - supported putting a space between the qualifier and the ID
					strBox33bNum.Format("'%s%s%s'", _Q(strBox33bQual), nQualifierSpace == 1 ? " " : "", _Q(Box24J));
				}

				//Load the override for PIN and for Box24J
				// (j.jones 2007-02-22 16:54) - PLID 24776 - supported putting a space between the qualifier and the ID
				// (j.jones 2007-08-08 10:29) - PLID 25395 - added Box33aNPI to the AdvHCFAPinT
				// (j.jones 2011-05-31 16:09) - PLID 43775 - use the mapped 24J provider ID
				// (j.jones 2011-05-31 16:09) - PLID 43775 - use the mapped 24J provider ID
				nxrs = CreateParamRecordset("SELECT Box24JQual, Box24J FROM AdvHCFAPinT WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT}", nQualifierSpace == 1 ? " " : "", nBox24JProviderID, nLocationID, fee_group_id);
				if(!nxrs->eof) {
					v = nxrs->Fields->Item["Box24J"]->Value;
					if(v.vt == VT_BSTR && CString(v.bstrVal).GetLength() > 0) {
						Box24J = CString(v.bstrVal);

						//if they are wanting the Box24J number in Box33b, update that up now,
						//but continue to use the qualifier from the HCFA Setup
						if(Box33 == 12) {
							// (j.jones 2007-02-21 12:31) - PLID 24776 - supported putting a space between the qualifier and the ID
							strBox33bNum.Format("'%s%s%s'", _Q(strBox33bQual), nQualifierSpace == 1 ? " " : "", _Q(Box24J));
						}
					}

					v = nxrs->Fields->Item["Box24JQual"]->Value;
					if(v.vt == VT_BSTR && CString(v.bstrVal).GetLength() > 0)
						Box24I = CString(v.bstrVal);
				}
				nxrs->Close();

				// (d.thompson 2011-09-07) - PLID 45215 - Someone incorrectly parameterized this, causing overrides for 33a and 33b to stop working.  There is 
				//	an extra %s which is not a parameter here.
				nxrs = CreateParamRecordset(
					FormatString("SELECT Box33aNPI, CASE WHEN PIN IS NULL THEN NULL ELSE (CASE WHEN Box33bQual IS NULL THEN '' ELSE Box33bQual END) + '%s' + PIN END AS Box33b FROM AdvHCFAPinT WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT}", nQualifierSpace == 1 ? " " : ""), 
					nProviderID, nLocationID, fee_group_id);
				if(!nxrs->eof) {
					v = nxrs->Fields->Item["Box33b"]->Value;
					if(v.vt == VT_BSTR)
						strBox33bNum = "'" + _Q(CString(v.bstrVal)) + "'";

					v = nxrs->Fields->Item["Box33aNPI"]->Value;
					if(v.vt == VT_BSTR && CString(v.bstrVal).GetLength() > 0)
						Box33aNPIOver = "'" + _Q(CString(v.bstrVal)) + "'";
				}
				nxrs->Close();

				// (j.jones 2006-11-16 09:10) - PLID 23563 - we have an option now to use
				// the Location NPI instead of the Provider NPI. This is only for Box 33a.

				CString strNPI = "ProvidersT.NPI";

				// (j.jones 2007-08-08 10:31) - PLID 25395 - supported the Box33aNPI override
				if(!Box33aNPIOver.IsEmpty()) {
					strNPI = Box33aNPIOver;
				}
				else {

					// (j.jones 2007-01-18 11:35) - PLID 24264 - this was formerly only
					// available for Bill Location, now it is an option at all times
					if(LocationNPIUsage == 1) {
						strNPI.Format("(SELECT NPI FROM LocationsT WHERE ID = %li)", nLocationID);
					}
				}
				
				strSQL.Format("SELECT ProvidersT.PersonID AS FirstOfDoctorsProviders, GetDate() AS TodaysDate, PersonT.[First] + ' ' + PersonT.[Middle] + ' ' + PersonT.[Last] + (CASE WHEN (PersonT.Title <> '') THEN (', ' + PersonT.Title) ELSE ('') END) AS DocSignature, PersonT.SocialSecurity, ProvidersT.[Fed Employer ID], ProvidersT.[DEA Number], ProvidersT.[BCBS Number], ProvidersT.[Medicare Number], ProvidersT.TaxonomyCode, ProvidersT.[Medicaid Number], ProvidersT.[Workers Comp Number], ProvidersT.[Other ID Number], "
							"ProvidersT.NPI, ProvidersT.UPIN, PersonT.Address1 + ' ' + PersonT.Address2 AS DocAdd, PersonT.City + ', ' + PersonT.State + ' ' + PersonT.Zip AS DocCityStateZip, PersonT.WorkPhone AS DocPhone, "
							"CustomData1.TextParam AS Custom1, CustomData2.TextParam AS Custom2, CustomData3.TextParam AS Custom3, CustomData4.TextParam AS Custom4, '%s' AS Box24IQual, '%s' AS Box24JID, %s AS Box33bNum, %s AS Box33aNPI "
							"FROM PersonT INNER JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID "
							"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 6) AS CustomData1 ON ProvidersT.PersonID = CustomData1.PersonID "
							"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 7) AS CustomData2 ON ProvidersT.PersonID = CustomData2.PersonID "
							"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 8) AS CustomData3 ON ProvidersT.PersonID = CustomData3.PersonID "
							"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 9) AS CustomData4 ON ProvidersT.PersonID = CustomData4.PersonID "
							"WHERE Coalesce(ProvidersT.PersonID,-1) = %li", _Q(Box24I), _Q(Box24J), strBox33bNum, strNPI, nProviderID);
			}
			break;
		}
		g_aryFormQueries[34].sql = strSQL;

		//Step 3. Set up Form 6, which is Box 25 and Box 31

		switch (Box33SetupID) {

		case 2:
			{
				CString str, Box31;

				// (j.jones 2006-12-01 13:01) - PLID 22110 - supported the ClaimProviderID override
				// (j.jones 2008-04-04 15:39) - PLID 28995 - We don't need to load the claim provider ID because
				// this function already has the correct one in the form of nProviderID. We also only need the 
				// Box 33 provider ID here for showing that provider's Box 31 number.
				_RecordsetPtr nxrs = CreateParamRecordset("SELECT InsuranceBox31.Box31Info AS Box31 "
					"FROM BillsT "
					"INNER JOIN InsuredPartyT ON BillsT.InsuredPartyID = InsuredPartyT.PersonID "
					"INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
					"INNER JOIN InsuranceBox31 ON InsuranceBox31.InsCoID = InsuredPartyT.InsuranceCoID "
					"WHERE BillsT.ID = {INT} AND InsuranceBox31.ProviderID = {INT}", billid, nProviderID);
				_variant_t v;
				if(!nxrs->eof) {
					v = nxrs->Fields->Item["Box31"]->Value;
					if(v.vt != VT_NULL && v.vt != VT_EMPTY)
						Box31 = CString(v.bstrVal);
					else
						Box31 = " ";
				}
				else Box31 = " ";
				nxrs->Close();

				// Use general 1 provider
				CString Override = " ";
				nxrs = CreateParamRecordset("SELECT EIN FROM AdvHCFAPinT WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT}", nProviderID, nLocationID, fee_group_id);
				if(!nxrs->eof) {
					_variant_t v = nxrs->Fields->Item["EIN"]->Value;
					if(v.vt == VT_BSTR)
						Override = CString(v.bstrVal);
					else
						Override = " ";
				}
				else Override = " ";
				nxrs->Close();

				// (j.jones 2006-12-01 13:01) - PLID 22110 - supported the ClaimProviderID override
				// (j.jones 2008-04-04 15:35) - PLID 28995 - We don't need to load the claim provider ID because
				// this function already has the correct one in the form of nProviderID
				strSQL.Format("SELECT '1' AS FedIDType, '%s' AS Box31ID, DoctorInfo.ID AS FirstOfDoctorsProviders, DoctorInfo.WorkPhone AS DocPhone, GetDate() AS TodaysDate, "
					"CASE WHEN [ShowSignature] = 1 THEN DoctorInfo.[First] + ' ' + DoctorInfo.[Middle] + ' ' + DoctorInfo.[Last] + (CASE WHEN (DoctorInfo.Title <> '') THEN (', ' + DoctorInfo.Title) ELSE ('') END) ELSE Null END AS DocSignature, "
					"CustomData1.TextParam AS Custom1, CustomData2.TextParam AS Custom2, CustomData3.TextParam AS Custom3, CustomData4.TextParam AS Custom4, "
					"DoctorInfo.SocialSecurity, ProvidersT.[Fed Employer ID], '%s' AS Override, %s AS Box33bNum, ProvidersT.NPI AS Box33aNPI "
					"FROM (SELECT * FROM PersonT) AS DoctorInfo INNER JOIN ProvidersT ON DoctorInfo.ID = ProvidersT.PersonID "
					"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 6) AS CustomData1 ON ProvidersT.PersonID = CustomData1.PersonID "
					"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 7) AS CustomData2 ON ProvidersT.PersonID = CustomData2.PersonID "
					"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 8) AS CustomData3 ON ProvidersT.PersonID = CustomData3.PersonID "
					"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 9) AS CustomData4 ON ProvidersT.PersonID = CustomData4.PersonID "
					"WHERE DoctorInfo.ID = %li", _Q(Box31), _Q(Override), strBox33bNum, nProviderID);
			}
			break;

		case 4:

			{
				CString str,Box31;

				//Load Box31

				// (j.jones 2006-12-01 13:01) - PLID 22110 - supported the ClaimProviderID override
				// (j.jones 2008-04-04 15:39) - PLID 28995 - We don't need to load the claim provider ID because
				// this function already has the correct one in the form of nProviderID. We also only need the 
				// Box 33 provider ID here for showing that provider's Box 31 number.
				_RecordsetPtr nxrs = CreateParamRecordset("SELECT InsuranceBox31.Box31Info AS Box31 "
					"FROM BillsT "
					"INNER JOIN InsuredPartyT ON BillsT.InsuredPartyID = InsuredPartyT.PersonID "
					"INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
					"INNER JOIN InsuranceBox31 ON InsuranceBox31.InsCoID = InsuredPartyT.InsuranceCoID "
					"WHERE BillsT.ID = {INT} AND InsuranceBox31.ProviderID = {INT}", billid, nProviderID);
				_variant_t v;
				if(!nxrs->eof) {
					v = nxrs->Fields->Item["Box31"]->Value;
					if(v.vt != VT_NULL && v.vt != VT_EMPTY)
						Box31 = CString(v.bstrVal);
					else
						Box31 = " ";
				}
				else Box31 = " ";
				nxrs->Close();

				// Use bill location
				CString Override = " ";
				nxrs = CreateParamRecordset("SELECT EIN FROM AdvHCFAPinT WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT}", nProviderID, nLocationID, fee_group_id);
				if(!nxrs->eof) {
					_variant_t v = nxrs->Fields->Item["EIN"]->Value;
					if(v.vt == VT_BSTR)
						Override = CString(v.bstrVal);
					else
						Override = " ";
				}
				else Override = " ";
				nxrs->Close();

				// (j.jones 2006-11-17 12:04) - PLID 23587 - we previously tried to pull NPI here if that was
				// the Box 33b setup, but got an ambiguous column name because NPI exists for Locations and Providers.
				// Thing is, Box 33B isn't even needed in this query, so I removed it.

				// (j.jones 2006-12-01 12:19) - PLID 22110 - use of ClaimProviderID is not necessary since we are already using that as the nProviderID
				strSQL.Format("SELECT '1' AS FedIDType, '%s' AS Box31ID, DoctorInfo.ID AS FirstOfDoctorsProviders, LocationsT.Phone AS DocPhone, GetDate() AS TodaysDate, "
				"CASE WHEN [ShowSignature] = 1 THEN DoctorInfo.[First] + ' ' + DoctorInfo.[Middle] + ' ' + DoctorInfo.[Last] + (CASE WHEN (DoctorInfo.Title <> '') THEN (', ' + DoctorInfo.Title) ELSE ('') END) ELSE Null END AS DocSignature, "
				"CustomData1.TextParam AS Custom1, CustomData2.TextParam AS Custom2, CustomData3.TextParam AS Custom3, CustomData4.TextParam AS Custom4, "
				"DoctorInfo.SocialSecurity, LocationsT.EIN AS [Fed Employer ID], '%s' AS Override "
				"FROM (SELECT * FROM PersonT) AS DoctorInfo INNER JOIN ProvidersT ON DoctorInfo.ID = ProvidersT.PersonID "
				"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 6) AS CustomData1 ON ProvidersT.PersonID = CustomData1.PersonID "
				"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 7) AS CustomData2 ON ProvidersT.PersonID = CustomData2.PersonID "
				"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 8) AS CustomData3 ON ProvidersT.PersonID = CustomData3.PersonID "
				"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 9) AS CustomData4 ON ProvidersT.PersonID = CustomData4.PersonID "
				"CROSS JOIN LocationsT WHERE DoctorInfo.ID = %li AND LocationsT.ID = %li", _Q(Box31), _Q(Override), nProviderID, nLocationID);
			}
			break;

		case 3:		
		case 1:
		default:
			{
				CString str,Box31;

				//Load Box31

				// (j.jones 2006-12-01 13:01) - PLID 22110 - supported the ClaimProviderID override
				// (j.jones 2008-04-04 15:39) - PLID 28995 - We don't need to load the claim provider ID because
				// this function already has the correct one in the form of nProviderID. We also only need the 
				// Box 33 provider ID here for showing that provider's Box 31 number.
				_RecordsetPtr nxrs = CreateParamRecordset("SELECT InsuranceBox31.Box31Info AS Box31 "
					"FROM BillsT "
					"INNER JOIN InsuredPartyT ON BillsT.InsuredPartyID = InsuredPartyT.PersonID "
					"INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
					"INNER JOIN InsuranceBox31 ON InsuranceBox31.InsCoID = InsuredPartyT.InsuranceCoID "
					"WHERE BillsT.ID = {INT} AND InsuranceBox31.ProviderID = {INT}", billid, nProviderID);
				_variant_t v;
				if(!nxrs->eof) {
					v = nxrs->Fields->Item["Box31"]->Value;
					if(v.vt != VT_NULL && v.vt != VT_EMPTY)
						Box31 = CString(v.bstrVal);
					else
						Box31 = " ";
				}
				else Box31 = " ";
				nxrs->Close();

				//use bill provider
				CString Override = " ";
				nxrs = CreateParamRecordset("SELECT EIN FROM AdvHCFAPinT WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT}", nProviderID, nLocationID, fee_group_id);
				if(!nxrs->eof) {
					_variant_t v = nxrs->Fields->Item["EIN"]->Value;
					if(v.vt == VT_BSTR)
						Override = CString(v.bstrVal);
					else
						Override = " ";
				}
				else Override = " ";
				nxrs->Close();

				CString strSignatureType;
				//this determines if the Bill Provider or the Override name shows in Box 31
				if(UseOverrideInBox31 == 0 || Box33SetupID != 3) {
					strSignatureType = "DoctorInfo.[First] + ' ' + DoctorInfo.[Middle] + ' ' + DoctorInfo.[Last] + (CASE WHEN (DoctorInfo.Title <> '') THEN (', ' + DoctorInfo.Title) ELSE ('') END)";
				}
				else {
					_RecordsetPtr rs = CreateParamRecordset("SELECT Box33Num FROM HCFASetupT WHERE Box33Num > 0 AND ID = {INT}",fee_group_id);

					if (rs->eof) {
						AfxMessageBox("The new HCFA could not pull the override name for Box 33. Please ensure it is filled in correctly.");
						return;
					}

					strSignatureType.Format("(SELECT Convert(nvarchar,DoctorsT.Note) "
						"FROM (SELECT * FROM PersonT LEFT JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID) AS DoctorsT "
						"WHERE DoctorsT.ID = %li)", rs->Fields->Item["Box33Num"]->Value.lVal);
					rs->Close();
				}

				// (j.jones 2006-12-01 12:19) - PLID 22110 - use of ClaimProviderID is not necessary since we are already using that as the nProviderID (or an override)
				strSQL.Format("SELECT '1' AS FedIDType, '%s' AS Box31ID, DoctorInfo.ID AS FirstOfDoctorsProviders, DoctorInfo.WorkPhone AS DocPhone, GetDate() AS TodaysDate, "
					"CASE WHEN [ShowSignature] = 1 THEN %s ELSE Null END AS DocSignature, "
					"CustomData1.TextParam AS Custom1, CustomData2.TextParam AS Custom2, CustomData3.TextParam AS Custom3, CustomData4.TextParam AS Custom4, "
					"DoctorInfo.SocialSecurity, ProvidersT.[Fed Employer ID], '%s' AS Override, %s AS Box33bNum, ProvidersT.NPI AS Box33aNPI FROM (SELECT * FROM PersonT) AS DoctorInfo INNER JOIN ProvidersT ON DoctorInfo.ID = ProvidersT.PersonID "
					"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 6) AS CustomData1 ON ProvidersT.PersonID = CustomData1.PersonID "
					"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 7) AS CustomData2 ON ProvidersT.PersonID = CustomData2.PersonID "
					"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 8) AS CustomData3 ON ProvidersT.PersonID = CustomData3.PersonID "
					"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 9) AS CustomData4 ON ProvidersT.PersonID = CustomData4.PersonID "
					"WHERE DoctorInfo.ID = %li", _Q(Box31), strSignatureType, _Q(Override), strBox33bNum, nProviderID);
			}
			break;
		}
		
		g_aryFormQueries[6].sql = strSQL;

		// (j.jones 2014-05-05 15:43) - PLID 61993 - this now takes in a Box17 person ID & qualifer override,
		// which is -1 if we're just supposed to use the bill's referring physician
		BuildFormsT_Form22(nProviderID, nLocationID, nBox17PersonID, strBox17PersonQualifier, bBox17_IsAProvider);
	
	} NxCatchAll("Error in LoadProviderInfo");	
}

void CHCFADlg::OnCancel() 
{
	EndDialog (0);	
}

//save and exit
void CHCFADlg::OnCheck() 
{
	// (z.manning, 05/16/2008) - PLID 30050 - Converted to NxDialog
	CPrintWaitDlg dlgWait(this);
	CRect rc;
	CString str;
	int iPage = 0;

	if(!CheckCurrentUserPermissions(bioClaimForms,sptWrite))
		return;

	if(IDNO == MessageBox("This action will save the contents of this claim, for the purposes of printing only.\n"
						  "If you open or print this claim in the future, the claim will NOT reload information from the bill\n"
						  "unless the 'Restore Defaults' button is clicked.\n"
						  "(Electronic Claims, and the data on the bill itself, are not affected by these changes.)\n\n"
						  "Are you sure you wish to save the state of this claim?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
		return;
	}

	UpdateBatch();

#ifdef SAVE_HISTORICAL_INFORMATION

	m_pframe->StopTrackingChanges();

	dlgWait.Create(IDD_PRINT_WAIT_DLG, this);	
	dlgWait.GetWindowRect(rc);
	dlgWait.SetWindowPos(&wndTopMost,
		(GetSystemMetrics(SM_CXFULLSCREEN) - rc.Width())/2, (GetSystemMetrics(SM_CYFULLSCREEN)-rc.Height())/2,
			0,0, SWP_NOSIZE | SWP_SHOWWINDOW);

	// FALSE means only change saved records
	Save(FALSE, &dlgWait, iPage);

	dlgWait.DestroyWindow();
#endif

	EndDialog (1);	
}

void CHCFADlg::OnPrint() 
{
	// Get the alignment settings
	_RecordsetPtr rs;
	COleVariant var;
	CString str;
	int x_offset = 0, y_offset = 0, x_scale = 0, y_scale = 0;
	FONT_SIZE = 12;
	MINI_FONT_SIZE = 10;

	m_pframe->m_printDlg = m_printDlg;

	try {

		// (j.jones 2007-04-25 11:43) - PLID 4758 - Handle having a default per workstation,
		// defaulting to the FormAlignT default printer if the per-workstation default doesn't exist.
		long nDefault = GetPropertyInt("DefaultFormAlignID", -1, 0, FALSE);
		CString strDefault = "[Default] = 1";
		if(nDefault != -1)
			strDefault.Format("ID = %li", nDefault);

		rs = CreateRecordset("SELECT * FROM FormAlignT WHERE %s AND FormID = 0", strDefault);
		if(!rs->eof) {

			var = rs->Fields->Item["XOffset"]->Value;
			if (var.vt == VT_NULL) var = (long)0;
			x_offset = var.lVal;

			var = rs->Fields->Item["YOffset"]->Value;
			if (var.vt == VT_NULL) var = (long)0;
			y_offset = var.lVal;

			var = rs->Fields->Item["XScale"]->Value;
			if (var.vt == VT_NULL) var = (long)0;
			x_scale = var.lVal;

			var = rs->Fields->Item["YScale"]->Value;
			if (var.vt == VT_NULL) var = (long)0;
			y_scale = var.lVal;
			
			var = rs->Fields->Item["FontSize"]->Value;
			if (var.vt == VT_NULL) var = (long)12;
			FONT_SIZE = var.lVal;

			// (j.jones 2005-01-31 15:06) - force it to be no greater than 14
			if(FONT_SIZE > 14)
				FONT_SIZE = 14;

			var = rs->Fields->Item["MiniFontSize"]->Value;
			if (var.vt == VT_NULL) var = (long)10;
			MINI_FONT_SIZE = var.lVal;

			// (j.jones 2005-01-31 15:06) - force it to be no greater than 12
			if(MINI_FONT_SIZE > 12)
				MINI_FONT_SIZE = 12;
		}
		else {
			if(m_bPrintWarnings && IDYES == MessageBox("You have not saved this form's alignment settings. The printout may not be aligned correctly until you do this.\n"
				"To set this up properly, click \"Yes\" and then click on the \"Align Form\" button. Create a printer, and check the \"Default For My Workstation\" box.\n"
				"Then configure the alignment as needed. Would you like to cancel printing and align the form now?","Practice",MB_ICONEXCLAMATION|MB_YESNO)) {
				return;
			}
		}
		rs->Close();

		//specify whether or not to unpunctuate diagnosis codes
		CDWordArray aryIDsToIgnore;
		if(m_HCFAInfo.ShowDiagDecimals) {
			aryIDsToIgnore.Add(243);
			aryIDsToIgnore.Add(244);
			aryIDsToIgnore.Add(245);
			aryIDsToIgnore.Add(246);
			//TES 11/6/2014 - PLID 64010 - Apply the "Show Decimals" setting to diag codes 5-12 as well
			aryIDsToIgnore.Add(8700);
			aryIDsToIgnore.Add(8701);
			aryIDsToIgnore.Add(8702);
			aryIDsToIgnore.Add(8703);
			aryIDsToIgnore.Add(8704);
			aryIDsToIgnore.Add(8705);
			aryIDsToIgnore.Add(8706);
			aryIDsToIgnore.Add(8707);
		}

		// (j.jones 2007-03-28 14:18) - PLID 25392 - added ability to
		// leave the DiagCs punctuated
		if(m_HCFAInfo.ShowWhichCodesCommas) {
			aryIDsToIgnore.Add(270);
			aryIDsToIgnore.Add(280);
			aryIDsToIgnore.Add(289);
			aryIDsToIgnore.Add(298);
			aryIDsToIgnore.Add(308);
			aryIDsToIgnore.Add(317);
		}

		// (j.jones 2013-01-29 09:37) - PLID 54374 - do not strip punctuation from patient name fields
		aryIDsToIgnore.Add(82);		//Box 2 - Patient's Name
		aryIDsToIgnore.Add(92);		//Box 4 - Insured's Name
		aryIDsToIgnore.Add(142);	//Box 9 - Other Insured's Name

		// (j.jones 2014-08-01 16:36) - PLID 63103 - do not strip punctuation from the notes fields
		aryIDsToIgnore.Add(5504);
		aryIDsToIgnore.Add(5505);
		aryIDsToIgnore.Add(5506);
		aryIDsToIgnore.Add(5507);
		aryIDsToIgnore.Add(5508);
		aryIDsToIgnore.Add(5509);

		PRINT_X_OFFSET += x_offset * 10;
		PRINT_Y_OFFSET += y_offset * 10;
		PRINT_X_SCALE += x_scale * 0.01;
		PRINT_Y_SCALE += y_scale * 0.01;
		if(!m_pframe->OnPrint(m_CapOnPrint, "HCFA", &aryIDsToIgnore, m_pPrintDC)) {
			PRINT_X_OFFSET -= x_offset * 10;
			PRINT_Y_OFFSET -= y_offset * 10;
			PRINT_X_SCALE -= x_scale * 0.01;
			PRINT_Y_SCALE -= y_scale * 0.01;
			return;
		}
		PRINT_X_OFFSET -= x_offset * 10;
		PRINT_Y_OFFSET -= y_offset * 10;
		PRINT_X_SCALE -= x_scale * 0.01;
		PRINT_Y_SCALE -= y_scale * 0.01;

	//////////////////////////////////////////////////////
	// Add record of printout to the ClaimHistoryT table
		// (j.dinatale 2010-07-28) - PLID 39803 - if we are not to update the tables in this dialog we can return
		if(!m_bUpdateClaimsTables)
			return;

		//Send Type 1 - Paper HCFA
		
		// (j.jones 2013-01-23 09:19) - PLID 54734 - moved the claim history addition to its own function,
		// this also updates HCFATrackT.LastSendDate
		AddToClaimHistory(m_ID, g_nInsuredPartyID, ClaimSendType::HCFA);

		//now add to patient history
		CString str, strDesc = "HCFA Printed";
		//get ins. co. name
		_RecordsetPtr rs = CreateParamRecordset("SELECT InsuranceCoT.Name FROM InsuranceCoT INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID WHERE InsuredPartyT.PersonID = {INT}", g_nInsuredPartyID);
		if(!rs->eof) {
			str.Format(" - Sent To %s",CString(rs->Fields->Item["Name"]->Value.bstrVal));
			strDesc += str;
		}
		rs->Close();

		rs = CreateParamRecordset("SELECT TOP 1 LineItemT.Date FROM LineItemT INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID WHERE ChargesT.BillID = {INT}",m_ID);
		if(!rs->eof) {
			COleDateTime dt;
			dt = rs->Fields->Item["Date"]->Value.date;
			str.Format("  Service Date: %s",FormatDateTimeForInterface(dt));
			strDesc += str;
		}
		rs->Close();

		long Loc = GetCurrentLocation();

		// (j.jones 2007-02-20 09:13) - PLID 24790 - converted the insert to include the NewNumber in the batch,
		// then return the new ID so we can send a tablechecker
		// (j.jones 2008-09-04 13:31) - PLID 30288 - converted to use CreateNewMailSentEntry,
		// which creates the data in one batch and sends a tablechecker
		// (c.haag 2010-01-28 11:00) - PLID 37086 - Removed COleDateTime::GetCurrentTime as the service date; should be the server's time.
		CreateNewMailSentEntry(PatientID, strDesc, m_strBillName, "", GetCurrentUserName(), "", Loc);
	// (j.dinatale 2010-07-28) - PLID 39803 - removed the other catch that was in here, if theres an exception in the actual printing, we dont want to update the claimshistory
	}NxCatchAll("Error printing HCFA form.");
}

void CHCFADlg::OnAlign() 
{
	CPrintAlignDlg dlg(this);
	dlg.DoModal();	
}

// This will restore all HCFA Document defaults and
// reload the form

void CHCFADlg::OnRestore() 
{
	CString strSQL;

	if(!CheckCurrentUserPermissions(bioClaimForms,sptWrite))
		return;

	if (IDNO == MessageBox("All changes you have made to the HCFA will be unrecoverable. Are you sure you wish to do this?", NULL, MB_YESNO))
		return;

	CWaitCursor pWait;

	try {
	// Delete all the form changes
	// (j.jones 2007-03-02 08:47) - PLID 23297 - converted to use batched statements
	// (j.jones 2008-05-28 15:46) - PLID 27881 - ensured we delete all relevant FormHistory entries
	CString strBatch = BeginSqlBatch();
	AddStatementToSqlBatch(strBatch, "DELETE FROM FormHistoryT WHERE DocumentID IN (SELECT FirstDocumentID FROM HCFADocumentsT WHERE BillID = %li AND FormType = 0)", m_ID);
	AddStatementToSqlBatch(strBatch, "DELETE FROM HCFADocumentsT WHERE HCFADocumentsT.BillID = %li AND FormType = 0",m_ID);
	ExecuteSqlBatch(strBatch);

	// Remove all changes made by the user
	m_pframe->UndoChanges();

	//put this back to default
	LoadBox25Default();

	} NxCatchAll("Error restoring defaults.");

	// Refresh the HCFA
	m_pframe->Refresh(1); // 90% of the static text & patient information
	m_pframe->Refresh(2);	// HCFA total
	m_pframe->Refresh(3);	// Total applied
	m_pframe->Refresh(4);	// HCFA balance
	m_pframe->Refresh(5);	// Box 28
	m_pframe->Refresh(6);	// Box 31
	m_pframe->Refresh(8);	// Box 33 (Address)
	m_pframe->Refresh(9);	// Box 33 (PIN#)

	m_pframe->Refresh(20); // Insurance info on right side of hcfa
	m_pframe->Refresh(21); // Insurance info on right side of hcfa
	m_pframe->Refresh(33); // HCFA Address
	m_pframe->Refresh(34); // PIN number
	m_pframe->Refresh(22); // Bill information

	//track changes now, otherwise any changes made after a restore won't be saved!
	m_pframe->TrackChanges(0, 0);

	// (j.jones 2007-06-22 17:00) - PLID 25665 - reset the text to empty
	SetDlgItemText(IDC_INFO_TEXT, "");

	// (j.jones 2007-06-25 10:14) - PLID 25663 - reset the restore defaults button to black
	// (j.jones 2008-05-09 11:36) - PLID 29953 - actually just call AutoSet again, because it would be the modify style
	m_btnRestoreDefaults.AutoSet(NXB_MODIFY);
	m_btnRestoreDefaults.Invalidate();

	// (j.jones 2007-06-25 10:38) - PLID 25663 - renamed this variable
	//and the m_bFieldsEdited value to FALSE
	m_pframe->m_bFieldsEdited = FALSE;

	// (j.jones 2007-06-22 10:11) - PLID 25665 - need to redraw to ensure edited colors disappear
	Invalidate();
}

void CHCFADlg::ShowSignature()
{
	//the following code determines whether "Signature On File" is printed in Box12 and 13.
	//it is set up in the HCFA tab of Admin, combined with a combo box that lets you choose
	//whether the boxes are filled in based on the insurance company being accepted,
	//not accepted, or always accepted
	CString str;
	_RecordsetPtr rs;
	_variant_t var;
	int Accepted12Rule = 3, Accepted13Rule = 3;

	//0 - Never, 1 - If Accepted, 2 - If Not Accepted, 3 - Always
	HCFABox13Over hb13Value = hb13_UseDefault;
	// (j.jones 2010-06-10 13:24) - PLID 39095 - added a second recordset to find the Box13 override
	rs = CreateParamRecordset("SELECT Box12Accepted, Box13Accepted FROM HCFASetupT WHERE ID = {INT}\r\n"
		" "
		"SELECT HCFABox13Over FROM BillsT WHERE ID = {INT}", fee_group_id, billid);

	if(!rs->eof) {
		// (j.jones 2010-07-22 11:49) - PLID 39779 - converted to use VarLong and reflect
		// that Box13 defaults to 3 (though neither are nullable fields so this was pointless)
		// (j.jones 2010-07-23 16:05) - PLID 39794 - Box 12 now also defaults to 3
		Accepted12Rule = AdoFldLong(rs, "Box12Accepted", 3);
		Accepted13Rule = AdoFldLong(rs, "Box13Accepted", 3);
	}
	rs = rs->NextRecordset(NULL);

	if(!rs->eof) {
		hb13Value = (HCFABox13Over)AdoFldLong(rs, "HCFABox13Over", (long)hb13_UseDefault);		
	}
	rs->Close();

	// (j.jones 2007-03-02 08:47) - PLID 23297 - converted to use batched statements
	CString strSignatureBatch = BeginSqlBatch();

	// (j.jones 2014-05-01 10:48) - PLID 61839 - get the provider ID out of the current page info
	long nProviderID = GetCurrentPageProviderID();

	// (j.jones 2010-07-23 15:28) - PLID 39783 - accept assignment is calculated in GetAcceptAssignment_ByInsuredParty now
	BOOL bAccepted = GetAcceptAssignment_ByInsuredParty(g_nInsuredPartyID, nProviderID);

	//we will try to fill in the boxes if one of the three configurations are valid and "Fill Box 12/13" are checked
	//if the company is accepted and "If Accepted" is checked
	//if the company is not accepted and "If Not Accepted" is checked
	//if "Always" is checked
	if(((bAccepted && Accepted12Rule==1) || (!bAccepted && Accepted12Rule==2) || Accepted12Rule==3)) {
		AddStatementToSqlBatch(strSignatureBatch, "UPDATE FormControlsT SET Source = 'SignatureOnFile' WHERE ID = 199");
		
		//0 - the Print Date (today), 1 - the Bill Date
		if(m_HCFAInfo.Box12UseDate == 0) {
			AddStatementToSqlBatch(strSignatureBatch, "UPDATE FormControlsT SET Source = 'TodaysDate', FormID = 2 WHERE ID = 200");
		}
		else {
			AddStatementToSqlBatch(strSignatureBatch, "UPDATE FormControlsT SET Source = 'BillDate', FormID = 2 WHERE ID = 200");
		}
	}
	else {
		AddStatementToSqlBatch(strSignatureBatch, "UPDATE FormControlsT SET Source = '' WHERE ID = 199");
		AddStatementToSqlBatch(strSignatureBatch, "UPDATE FormControlsT SET Source = '' WHERE ID = 200");
	}
	// (j.jones 2010-06-10 13:19) - PLID 39095 - the bill's Box13 override can forcibly fill/empty this field
	if(hb13Value == hb13_Yes
		|| (hb13Value == hb13_UseDefault && (((bAccepted && Accepted13Rule==1) || (!bAccepted && Accepted13Rule==2) || Accepted13Rule==3))))
		AddStatementToSqlBatch(strSignatureBatch, "UPDATE FormControlsT SET Source = 'SignatureOnFile' WHERE ID = 207");
	else
		AddStatementToSqlBatch(strSignatureBatch, "UPDATE FormControlsT SET Source = '' WHERE ID = 207");

	if(!strSignatureBatch.IsEmpty()) {
		ExecuteSqlBatch(strSignatureBatch);
	}
}

void CHCFADlg::ShowAddress()
{
	//determine whether or not to display the insurance co's address in the upper right hand corner

	// (j.jones 2007-03-02 08:47) - PLID 23297 - converted to use batched statements
	CString strAddressBatch = BeginSqlBatch();

	if (m_HCFAInfo.Address == 1) {
		//show the address
		//while this is what will be most common, we must always do this incase a previous HCFA changed this setup
		AddStatementToSqlBatch(strAddressBatch, "UPDATE FormControlsT SET Source = 'InsCoName' WHERE ID = 422");
		AddStatementToSqlBatch(strAddressBatch, "UPDATE FormControlsT SET Source = 'InsCoAdd1' WHERE ID = 423");
		AddStatementToSqlBatch(strAddressBatch, "UPDATE FormControlsT SET Source = 'InsCoAdd2' WHERE ID = 424");
		AddStatementToSqlBatch(strAddressBatch, "UPDATE FormControlsT SET Source = 'InsCoCity' WHERE ID = 425");
		AddStatementToSqlBatch(strAddressBatch, "UPDATE FormControlsT SET Source = 'InsCoState' WHERE ID = 426");
		AddStatementToSqlBatch(strAddressBatch, "UPDATE FormControlsT SET Source = 'InsCoZip' WHERE ID = 427");

		//now decide how to align the address, left or right
		// (j.jones 2007-03-26 15:44) - PLID 24538 - I had removed this option, but by client demand I reinstated it
		if(m_HCFAInfo.LeftAlignInsAddress == 1) {
			AddStatementToSqlBatch(strAddressBatch, "UPDATE FormControlsT SET X = 160 WHERE ID >= 422 AND ID <= 425");
			AddStatementToSqlBatch(strAddressBatch, "UPDATE FormControlsT SET X = 287 WHERE ID = 426");
			AddStatementToSqlBatch(strAddressBatch, "UPDATE FormControlsT SET X = 315 WHERE ID = 427");
		}
		else {
			AddStatementToSqlBatch(strAddressBatch, "UPDATE FormControlsT SET X = 423 WHERE ID >= 422 AND ID <= 425");
			AddStatementToSqlBatch(strAddressBatch, "UPDATE FormControlsT SET X = 550 WHERE ID = 426");
			AddStatementToSqlBatch(strAddressBatch, "UPDATE FormControlsT SET X = 578 WHERE ID = 427");
		}
	}
	else {
		//don't show the address
		AddStatementToSqlBatch(strAddressBatch, "UPDATE FormControlsT SET Source = '' WHERE ID = 422 OR ID = 423 OR ID = 424 OR ID = 425 OR ID = 426 OR ID = 427");
	}

	if(!strAddressBatch.IsEmpty()) {
		ExecuteSqlBatch(strAddressBatch);
	}
}

void CHCFADlg::LoadDefaultBox11Value(int form, long InsuredPartyID)
{
	BOOL bOverride = FALSE;

	// (j.jones 2006-10-31 17:44) - PLID 21845 - secondary info overrides this
	if(m_bShowSecondaryInBox11)
		return;

	if(m_HCFAInfo.Box11Rule == 1) {
		bOverride = TRUE;
	}
	else {
		//use only if blank (default)
		_RecordsetPtr rsIns = CreateParamRecordset("SELECT PolicyGroupNum FROM InsuredPartyT WHERE InsuredPartyT.PersonID = {INT}",InsuredPartyID);
		if(!rsIns->eof) {
			_variant_t var = rsIns->Fields->Item["PolicyGroupNum"]->Value;
			if(var.vt == VT_BSTR) {
				CString str = CString(var.bstrVal);
				str.TrimLeft(" ");
				str.TrimRight(" ");
				if(str.GetLength()==0)
					bOverride = TRUE;
			}
			else
				bOverride = TRUE;
		}
		rsIns->Close();
	}

	if(bOverride) {

		//this update statement will make sure the override is displayed regardless of Box 11 being hidden
		if(form == 20)
			ExecuteSql("UPDATE FormControlsT SET Source = 'InsFECA' WHERE ID = 157");


		CString strDefault = m_HCFAInfo.Box11;
		strDefault = "'" + _Q(strDefault) + "'";
		g_aryFormQueries[form].sql.Replace("InsuredPartyT.PolicyGroupNum",strDefault);
	}
}

long CHCFADlg::DoScrollTo(long nNewTopPos)
{
	// Make sure the new position is not out of range
	if (nNewTopPos > ScrollBottomPos)
		nNewTopPos = ScrollBottomPos;
	if (nNewTopPos < SCROLL_TOP_POS)
		nNewTopPos = SCROLL_TOP_POS;

	// Calculate the amount we are going to need to scroll
	long nOldHeight, nNewHeight;
	nOldHeight = GetScrollPos(SB_VERT);
	nNewHeight = nNewTopPos;
	
	// Scroll
	SetScrollPos(SB_VERT, nNewTopPos);
	long nScrollHeight = nOldHeight - nNewHeight;

	m_pframe->Scroll (0, 0 - GetScrollPos(SB_VERT));

	return 0;
}

void CHCFADlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	switch (nSBCode) {
	case SB_LINEUP:
		DoScrollTo(GetScrollPos(SB_VERT)-25);
		break;
	case SB_LINEDOWN:
		DoScrollTo(GetScrollPos(SB_VERT)+25);
		break;
	case SB_PAGEUP:
		DoScrollTo(GetScrollPos(SB_VERT)-SCROLL_POS_PER_PAGE);
		break;
	case SB_PAGEDOWN:
		DoScrollTo(GetScrollPos(SB_VERT)+SCROLL_POS_PER_PAGE);
		break;
	case SB_TOP:
		DoScrollTo(SCROLL_TOP_POS);
		break;
	case SB_BOTTOM:
		DoScrollTo(ScrollBottomPos);
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		DoScrollTo((int)nPos);
		break;
	default:
		// Do nothing special
		break;
	}
	
	CDialog::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CHCFADlg::OnRadioNoBatch() 
{
	if(!CheckCurrentUserPermissions(bioClaimForms,sptWrite)) {
		FindBatch();
		return;
	}

	UpdateBatch();
}

void CHCFADlg::OnRadioElectronic() 
{
	if(!CheckCurrentUserPermissions(bioClaimForms,sptWrite)) {
		FindBatch();
		return;
	}

	//just unbatch, if that's what they want
	if(!((CButton*)GetDlgItem(IDC_RADIO_PAPER))->GetCheck() &&
		!((CButton*)GetDlgItem(IDC_RADIO_ELECTRONIC))->GetCheck()) {
		UpdateBatch();
		return;
	}

	//don't automatically batch a $0.00 claim
	COleCurrency cyCharges = COleCurrency(0,0);
	_RecordsetPtr rs = CreateParamRecordset("SELECT dbo.GetClaimTotal(ID) AS Total FROM BillsT WHERE ID = {INT}",m_ID);
	if(!rs->eof) {
		cyCharges = AdoFldCurrency(rs, "Total",COleCurrency(0,0)); 
	}
	rs->Close();

	CString strWarn;
	strWarn.Format("This claim is currently %s. Are you sure you wish to send the claim to the electronic batch?",
		FormatCurrencyForInterface(cyCharges,TRUE,TRUE));

	if(cyCharges > COleCurrency(0,0) || 
		IDYES == MessageBox(strWarn,"Practice",MB_YESNO|MB_ICONQUESTION)) {

		UpdateBatch();
	}
	else {
		FindBatch();
	}
}

void CHCFADlg::OnRadioPaper() 
{
	if(!CheckCurrentUserPermissions(bioClaimForms,sptWrite)) {
		FindBatch();
		return;
	}

	//just unbatch, if that's what they want
	if(!((CButton*)GetDlgItem(IDC_RADIO_PAPER))->GetCheck() &&
		!((CButton*)GetDlgItem(IDC_RADIO_ELECTRONIC))->GetCheck()) {
		UpdateBatch();
		return;
	}

	//don't automatically batch a $0.00 claim
	COleCurrency cyCharges = COleCurrency(0,0);
	_RecordsetPtr rs = CreateParamRecordset("SELECT dbo.GetClaimTotal(ID) AS Total FROM BillsT WHERE ID = {INT}",m_ID);
	if(!rs->eof) {
		cyCharges = AdoFldCurrency(rs, "Total",COleCurrency(0,0)); 
	}
	rs->Close();
	
	CString strWarn;
	strWarn.Format("This claim is currently %s. Are you sure you wish to send the claim to the paper batch?",
		FormatCurrencyForInterface(cyCharges,TRUE,TRUE));

	if(cyCharges > COleCurrency(0,0) || 
		IDYES == MessageBox(strWarn,"Practice",MB_YESNO|MB_ICONQUESTION)) {

		UpdateBatch();
	}
	else {
		FindBatch();
	}
}

// (j.jones 2007-06-22 13:23) - PLID 25665 - handle when a field was edited, so we update the info text box
BOOL CHCFADlg::PreTranslateMessage(MSG *pMsg) {

	if(pMsg->message == NXM_CLAIM_FORM_FIELDS_EDITED) {
		
		// (j.jones 2007-06-25 10:34) - PLID 25663 - we'll get this message if we edit something, but we need to reflect
		// differently based on whether we colored fields, which is controlled by this preference
		BOOL bColorEditedFields = GetRemotePropertyInt("EnableEditedClaimFieldColoring", 1, 0, GetCurrentUserName(), true) == 1;

		if(bColorEditedFields) {
			SetDlgItemText(IDC_INFO_TEXT, "Highlighted fields indicate that data has been manually changed. 'Restore Defaults' will reload these fields from data.");
		}

		// (j.jones 2007-06-25 10:14) - PLID 25663 - colorize the restore defaults button
		m_btnRestoreDefaults.SetTextColor(RGB(255,0,0));
		m_btnRestoreDefaults.Invalidate();
	}

	return CDialog::PreTranslateMessage(pMsg);
}
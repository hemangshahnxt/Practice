// NYMedicaidDlg.cpp : implementation file
//
// (k.messina 2010-02-11 5:36) - PLID 37323 create the CNYMedicaidDlg

#include "stdafx.h"
#include "NYMedicaidDlg.h"
#include "FormDisplayDlg.h"
#include "FormFormat.h"
#include "FormCheck.h"
#include "GlobalFinancialUtils.h"
#include "PrintAlignDlg.h"
#include "InternationalUtils.h"
#include "PrintWaitDlg.h"
#include "FormQuery.h"
#include "FormLayer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define BUTTON_WIDTH	100
#define BUTTON_HEIGHT	48
#define TEXT_HEIGHT		12

#define IDC_PHYSICIAN_LEFT	120
#define IDC_PHYSICIAN_RIGHT	121
#define IDC_BILL_DOWN		122
#define IDC_BILL_UP			123

#define SAVE_HISTORICAL_INFORMATION

// (k.messina 2010-02-25 10:47) - added this in the event I have to do another form and there should
//  be enums for these but instead this will do for now.
#define FORM_ID                7
#define QUERY_LINES_PAT      100 // Lines, Labels, and Patient Information
#define QUERY_PRIMARY_INSUR  101 // Primary Insurance Co Info
#define QUERY_BILLS_CHARGES  102 // Bill/Charge Information
#define QUERY_PROVIDER       103 // Provider Information
#define QUERY_CHARGE_TOTAL   104 // Charge Totals
#define QUERY_PAYMENT_TOTAL  105 // Payment Totals
#define QUERY_BALANCE        106 // Balance

// NYMedicaidDlg stuff

static CDWordArray adwProviderList;
static int firstcharge = 0;
static int provider_index = 0;
static int fee_group_id = -1;
static int billid;
static int nTotalCharges;

// For printing
static int nPrintedCharges = 0;
static int nPrintedProviders = 0;
static int oldfirstcharge, oldprovider_index;

static CFormDisplayDlg* g_pFrame;

using namespace ADODB;

BOOL PreNYMedicaidPrint(CDWordArray& dwChangedForms, CFormDisplayDlg* dlg);
void OnNYMedicaidCommand(WPARAM wParam, LPARAM lParam, CDWordArray& dwChangedForms, int FormControlsID, CFormDisplayDlg* dlg);

void RequeryHistoricalNYMedicaidData()
{
	try {
		_RecordsetPtr rs(__uuidof(Recordset));
		COleVariant var;
		CString str;

		str.Format("SELECT FirstDocumentID FROM HCFADocumentsT WHERE BillID = %li AND FirstCharge = %li AND ProviderIndex = %li AND FormType = %li", billid, firstcharge, adwProviderList.GetAt(provider_index), FORM_ID);
		rs = CreateRecordset(str);
		if (!rs->eof) {
			var = rs->Fields->Item["FirstDocumentID"]->Value;
			if (var.vt != VT_NULL)
				g_pFrame->SetDocumentID(var.lVal);
		}
		else
			g_pFrame->SetDocumentID(-1);
		rs->Close();
	}
	NxCatchAll("Error in requerying historical data");
}

/////////////////////////////////////////////////////////////////////////////
// NYMedicaidDlg dialog

BEGIN_MESSAGE_MAP(CNYMedicaidDlg, CDialog)
	//{{AFX_MSG_MAP(CNYMedicaidDlg)
	ON_WM_SIZE()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_X, OnClickX)
	ON_BN_CLICKED(IDC_CHECK, OnClickCheck)
	ON_BN_CLICKED(IDC_PRINT, OnClickPrint)
	ON_BN_CLICKED(IDC_ALIGN, OnClickAlign)
	ON_BN_CLICKED(IDC_RESTORE, OnClickRestore)
	ON_BN_CLICKED(IDC_CAP_ON_PRINT, OnClickCapitalizeOnPrint)
	ON_BN_CLICKED(IDC_RADIO_PAPER, OnRadioPaper)
	ON_BN_CLICKED(IDC_RADIO_NO_BATCH, OnRadioNoBatch)
	ON_WM_VSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CNYMedicaidDlg, CDialog)
    //{{AFX_EVENTSINK_MAP(CNYMedicaidDlg)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void OnNYMedicaidCommand(WPARAM wParam, LPARAM lParam, CDWordArray& dwChangedForms, int FormControlsID, CFormDisplayDlg* dlg);
void OnNYMedicaidKeyDown(CDialog* pFormDisplayDlg, MSG* pMsg);

CNYMedicaidDlg::CNYMedicaidDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNYMedicaidDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNYMedicaidDlg)
		m_bPrintWarnings = TRUE;
	//}}AFX_DATA_INIT
	m_pframe = 0;
	m_ID = -1;
	m_InsuredPartyID = -1;
	m_OthrInsuredPartyID = -1;

	m_pOnCommand = &OnNYMedicaidCommand;
	m_pOnKeyDown = &OnNYMedicaidKeyDown;
	m_pPrePrintFunc = NULL;
	m_ShowWindowOnInit = TRUE;
}

CNYMedicaidDlg::~CNYMedicaidDlg()
{
	delete m_pframe;
//	delete m_pleftbutton;
//	delete m_prightbutton;
//	delete m_pupbutton;
//	delete m_pdownbutton;
}

void CNYMedicaidDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNYMedicaidDlg)
	// (j.jones 2007-06-25 10:14) - PLID 25663 - changed the buttons to NxIconButtons
	DDX_Control(pDX, IDC_RESTORE, m_btnRestoreDefaults);
	DDX_Control(pDX, IDC_ALIGN, m_btnAlignForm);
	DDX_Control(pDX, IDC_CHECK, m_btnSave);
	DDX_Control(pDX, IDC_PRINT, m_btnPrint);
	DDX_Control(pDX, IDC_X, m_btnClose);
	//}}AFX_DATA_MAP
}

int CNYMedicaidDlg::DoModal(int billID) 
{
	m_ID = billID;
	return CDialog::DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CNYMedicaidDlg message handlers

BOOL CNYMedicaidDlg::OnInitDialog() 
{
//	DLGTEMPLATE Template;
	CString		where;// = " WHERE ID = 187";
//	RECT		rect;

	// (j.dinatale 2010-07-23) - PLID 39692 - initialize printer context to null
	m_pPrintDC = NULL;

	// (j.dinatale 2010-07-28) - PLID 39803 - initialize the update flag to true, so that way if its not specified that we want to update, we assume that it wants to be done
	m_bUpdateClaimsTables = true;

	CDialog::OnInitDialog();	
//	Template.x = Template.y = 0;
//	Template.cx = 640;
//	Template.cy = 480;
//	Template.style =  WS_CHILD | WS_VISIBLE;// | WS_MAXIMIZE | WS_CAPTION;
//	Template.cdit = 0;

	_RecordsetPtr	tmpRS;

	ShowScrollBar(SB_VERT, TRUE);

	// (j.jones 2013-04-04 17:19) - PLID 56038 - added bulk caching
	g_propManager.CachePropertiesInBulk("CNYMedicaidDlg-1", propNumber,
		"(Username = '%s') AND ("
		"Name = 'CapitalizeNYMedicaid' OR "
		"Name = 'NYMedicaidBox24H' OR "
		"Name = 'NYMedicaidBox24M' OR "
		"Name = 'NYMedicaidBox23' OR "
		"Name = 'NYMedicaidUseProvider' OR "
		"Name = 'EnableEditedClaimFieldColoring' OR "
		"Name = 'NYMedicaidDoNotFillBox25' "
		")",
		_Q(GetCurrentUserName()));

	g_propManager.CachePropertiesInBulk("CNYMedicaidDlg-2", propText,
		"(Username = '%s') AND ("
		"Name = 'NYMedicaidOverrideName' OR "
		"Name = 'NYMedicaidOverrideAddress' OR "
		"Name = 'NYMedicaidOverrideCity' OR "
		"Name = 'NYMedicaidOverrideState' OR "
		"Name = 'NYMedicaidOverrideZip' OR "
		"Name = 'NYMedicaidOverridePhone' "
		")",
		_Q(GetCurrentUserName()));

	/// NYMedicaid SPECIFIC STUFF ////////////
	
	// (j.jones 2013-04-26 16:00) - PLID 56453 - Get the POS code, which is unfortunately
	// tied to charges despite being the same on all charges. So just get the first record.
	// Also get the patient birthdate, which eliminates a query later.
	CString strPOSCode = "";
	_variant_t varBirthDate = g_cvarNull;
	_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 BillsT.Description, BillsT.InsuredPartyID, BillsT.OthrInsuredPartyID, "
		"PlaceOfServiceCodesT.PlaceCodes, PersonT.BirthDate "
		"FROM BillsT "
		"INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
		"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
		"INNER JOIN PersonT ON BillsT.PatientID = PersonT.ID "
		"LEFT JOIN PlaceOfServiceCodesT ON ChargesT.ServiceLocationID = PlaceOfServiceCodesT.ID "
		"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
		"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "		
		"WHERE BillsT.ID = {INT} AND LineItemT.Deleted = 0 AND ChargesT.Batched = 1 "
		"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
		"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null", m_ID);
	if(!rs->eof) {
		m_strBillName = AdoFldString(rs, "Description", "");
		m_InsuredPartyID = AdoFldLong(rs, "InsuredPartyID",-1);
		m_OthrInsuredPartyID = AdoFldLong(rs, "OthrInsuredPartyID",-1);
		strPOSCode = AdoFldString(rs, "PlaceCodes", "");
		varBirthDate = rs->Fields->Item["BirthDate"]->Value;
	}
	rs->Close();

	BuildNYMedicaidChargesT();

	if(FindHCFABatch(m_ID) == 1) {
		((CButton*)GetDlgItem(IDC_RADIO_PAPER))->SetCheck(TRUE);
		((CButton*)GetDlgItem(IDC_RADIO_NO_BATCH))->SetCheck(FALSE);
	}
	else {
		((CButton*)GetDlgItem(IDC_RADIO_PAPER))->SetCheck(FALSE);
		((CButton*)GetDlgItem(IDC_RADIO_NO_BATCH))->SetCheck(TRUE);
	}

	//m_pPrePrintFunc = &PreNYMedicaidPrint;

	m_CapOnPrint = GetRemotePropertyInt("CapitalizeNYMedicaid",0);
	((CButton*)GetDlgItem(IDC_CAP_ON_PRINT))->SetCheck(m_CapOnPrint);
	//m_CapOnPrint = TRUE;

	//create frame

	//allocate pointers
	//LogDetail("CNYMedicaidDlg: Creating frame");
	m_pframe		= new CFormDisplayDlg(this);
	m_pframe->color = RGB(80,80,80);
	g_pFrame = m_pframe;
//	m_pleftbutton	= new CButton;
//	m_prightbutton	= new CButton;
//	m_pupbutton		= new CButton;
//	m_pdownbutton	= new CButton;

	RequeryHistoricalNYMedicaidData();

//	if (!m_pframe->CreateIndirect (&Template, this))//DoModal();
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

	SetWindowText("NYMedicaid");

	int i = 187;

	//calculate the patient's age
	long nAge = 0;
	CString strAge = "NULL";

	if(varBirthDate.vt == VT_DATE) {
		COleDateTime dt;
		dt = varBirthDate.date;
		if(dt.m_status == COleDateTime::valid) {
			// (j.dinatale 2010-10-13) - PLID 38575 - need to call GetPatientAgeOnDate which no longer does any validation, 
			//  validation should only be done when bdays are entered/changed
			// (z.manning 2010-01-13 16:36) - PLID 22672 - Age is now a string
			nAge = atol(GetPatientAgeOnDate(dt, COleDateTime::GetCurrentTime(), FALSE));
			strAge.Format("'%li'",nAge);
		}
	}

	//Patient Information
	g_aryFormQueries[QUERY_LINES_PAT].sql.Format("SELECT First + (CASE WHEN Middle <> '' AND Middle <> ' ' THEN ' ' + Left(Middle,1) ELSE '' END)  + ' ' + Last AS PatName, Address1 + ' ' + Address2 AS Address, City + ', ' + State + ' ' + Zip AS CityStateZip, "
				"Address1 + ' ' + Address2 + ', ' + City + ', ' + State + ' ' + Zip AS PatFullAddress, HomePhone AS PatTeleNum, "
				"EmployerAddress1 + ' ' + EmployerAddress2 + ', ' + EmployerCity + ', ' + EmployerState + ' ' + EmployerZip AS EmpAddress, "
				"PersonT.*, PatientsT.*, %s AS Age, 'Signature On File' AS SigOnFile, GetDate() AS Date "
				"FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID",strAge);

	where.Format (" WHERE PersonT.ID = %li", m_PatientID);
	m_pframe->Load (QUERY_LINES_PAT, where, "", &i); // Lines, Labels, and Patient Information

	// (j.jones 2010-07-23 15:28) - PLID 39783 - accept assignment was previously loaded here but never used, so I removed it

	//Patient Insurance Information
	// (k.messina 2010-07-15 15:00) - PLID 38783 - added BOX23B
	g_aryFormQueries[QUERY_PRIMARY_INSUR].sql.Format(
				"DECLARE @22f INT SET @22f = 1 "
				"DECLARE @22g INT SET @22g = 1 "
				"DECLARE @22h INT SET @22h = 1 "
				"DECLARE @25c VARCHAR(30) SET @25c = '  0    0    3 ' "
				"SELECT @22f AS Box22F, @22g AS Box22G, @22h AS Box22H, @25c AS Box25C, "
				"Address1 + ' ' + Address2 AS PriAddress, City + ', ' + State + ' ' + Zip AS PriCityStateZip, "
				"Address1 + ' ' + Address2 + ', ' + City + ', ' + State + ' ' + Zip AS PriFullAddress, PersonT.*, InsuranceCoT.*, InsuredPartyT.*, "
				"(SELECT First + (CASE WHEN Middle <> '' AND Middle <> ' ' THEN ' ' + Left(Middle,1) ELSE '' END)  + ' ' + Last AS Name FROM PersonT WHERE PersonT.ID = InsuredPartyT.PersonID) AS InsuredName, "
				"(SELECT Gender FROM PersonT WHERE PersonT.ID = InsuredPartyT.PersonID) AS InsuredGender, "
				"(SELECT Address1 + ' ' + Address2 FROM PersonT WHERE PersonT.ID = InsuredPartyT.PersonID) AS InsuredAddress, "
				"(SELECT City + ', ' + State + ' ' + Zip FROM PersonT WHERE PersonT.ID = InsuredPartyT.PersonID) AS InsuredCityStateZip, "
				"(SELECT Employer FROM InsuredPartyT WHERE InsuredPartyT.PersonID=%li) AS InsuredOccupation, "
				"(CASE RelationToPatient WHEN 'Self' THEN 1 WHEN 'SPOUSE' THEN 2 WHEN 'CHILD' THEN 3 ELSE 0 END) AS PatRelationToInsured, "
				"(CASE RespTypeID WHEN 1 THEN '1    1' ELSE '2    1' END) AS Box23B "
				"FROM PersonT INNER JOIN InsuranceCoT ON PersonT.ID = InsuranceCoT.PersonID INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID", m_InsuredPartyID);

	where.Format (" WHERE InsuredPartyT.PersonID = %li", m_InsuredPartyID);
	m_pframe->Load (QUERY_PRIMARY_INSUR, where, "", &i); // Primary Insurance Co Info

	// Bill/Charge Information

	// (k.messina 2010-07-16 15:22) - PLID 38783 - get Box24H user settings
	long nBox24H = GetRemotePropertyInt("NYMedicaidBox24H",0,0,"<None>",true);
	CString strBox24H = "";

	// (k.messina 2010-07-16 15:22) - PLID 38783 - Adjust for Box24H user settings
	//Always show first diag code of the line item
	if(nBox24H == 0){
		// (a.walling 2014-03-06 14:40) - PLID 61234 - Which codes has changed, using that structure, which also handles the BillDiagCodeT changes
		strBox24H.Format(	
			"COALESCE(( "
				"SELECT TOP 1 "
					"DiagCodes.CodeNumber "
				"FROM ChargeWhichCodesT "
				"INNER JOIN BillDiagCodeT "
					"ON BillDiagCodeT.ID = ChargeWhichCodesT.BillDiagCodeID "
				"INNER JOIN DiagCodes "
					"ON DiagCodes.ID = BillDiagCodeT.%s "
				"WHERE ChargeWhichCodesT.ChargeID = ChargesT.ID "
				"ORDER BY BillDiagCodeT.OrderIndex "
				") "
				", '' "
			") AS WhichDiagCodes, "
			// Legend has it that the WhichCodes field used to have some crazy hybrid ability to include codes themselves, but none have been spotted in the wild since the days of yore
			, ShouldUseICD10(m_InsuredPartyID, m_ID) ? "ICD10DiagID" : "ICD9DiagID"
		);
	}

	// (k.messina 2010-07-16 15:22) - PLID 38783 - Always select the first diag code of the bill for each line item
	else if(nBox24H == 1){
		// (a.walling 2014-03-06 14:40) - PLID 61234 - Which codes has changed, using that structure, which also handles the BillDiagCodeT changes
		// (a.walling 2014-03-17 17:30) - PLID 61407 - Unique diag codes and recalculated WhichCodes using BillDiagsXmlFlat4IF and ChargeWhichCodesIF - NYWC, NYMedicaid
		strBox24H.Format("(SELECT TOP 1 DiagCodes.CodeNumber FROM BillDiagCodeT INNER JOIN DiagCodes ON DiagCodes.ID = BillDiagCodeT.%s WHERE BillDiagCodeT.BillID = %li ORDER BY BillDiagCodeT.OrderIndex) AS WhichDiagCodes,"
			, ShouldUseICD10(m_InsuredPartyID, m_ID) ? "ICD10DiagID" : "ICD9DiagID"
			, billid);
	}

	// (k.messina 2010-07-16 15:22) - PLID 38783 - Show only the pointers associated with line item
	else if(nBox24H == 2){
		strBox24H = 
				// (a.walling 2014-03-06 14:40) - PLID 61234 - Which codes has changed, using that structure, which also handles the BillDiagCodeT changes
				// Legend has it that the WhichCodes field used to have some crazy hybrid ability to include codes themselves, but none have been spotted in the wild since the days of yore
				// (a.walling 2014-03-10 13:29) - PLID 61305 - Emulate the old WhichCodes field via ChargeWhichCodesFlatV
				// (a.walling 2014-03-17 17:30) - PLID 61407 - Unique diag codes and recalculated WhichCodes using BillDiagsXmlFlat4IF and ChargeWhichCodesIF - NYWC, NYMedicaid
				"ChargeWhichCodesQ.WhichCodes "
				"AS WhichDiagCodes, ";
	}

	// (k.messina 2010-07-16 15:22) - PLID 38783 - Show diag code of line item unless multiples were selected, then show pointers
	else if(nBox24H == 3){
		// (a.walling 2014-03-10 13:29) - PLID 61305 - Emulate the old WhichCodes field via ChargeWhichCodesFlatV
		// (a.walling 2014-03-17 17:30) - PLID 61407 - Unique diag codes and recalculated WhichCodes using BillDiagsXmlFlat4IF and ChargeWhichCodesIF - NYWC, NYMedicaid
		// (a.walling 2014-03-19 15:04) - PLID 61234 - If only one diag code listed as WhichCode, then display that, otherwise display the pointers
		strBox24H.Format(	
			"CASE WHEN 0 <> CHARINDEX(',', ChargeWhichCodesQ.WhichCodes) THEN ChargeWhichCodesQ.WhichCodes ELSE "
			"COALESCE(( "
				"SELECT TOP 1 "
					"DiagCodes.CodeNumber "
				"FROM ChargeWhichCodesT "
				"INNER JOIN BillDiagCodeT "
					"ON BillDiagCodeT.ID = ChargeWhichCodesT.BillDiagCodeID "
				"INNER JOIN DiagCodes "
					"ON DiagCodes.ID = BillDiagCodeT.%s "
				"WHERE ChargeWhichCodesT.ChargeID = ChargesT.ID "
				"ORDER BY BillDiagCodeT.OrderIndex "
				") "
				", '' "
			") END AS WhichDiagCodes, "
			// Legend has it that the WhichCodes field used to have some crazy hybrid ability to include codes themselves, but none have been spotted in the wild since the days of yore
			, ShouldUseICD10(m_InsuredPartyID, m_ID) ? "ICD10DiagID" : "ICD9DiagID"
		);
	}

	CString strPOSFields;
	// (j.jones 2013-04-26 15:53) - PLID 56453 - we now send the patient address as the POS
	//if its POS code is 12 and the Claim_SendPatAddressWhenPOS12 preference is on
	if(strPOSCode == "12" && GetRemotePropertyInt("Claim_SendPatAddressWhenPOS12", 1, 0, "<None>", true) == 1) {
		//use the patient's address for the POS fields
		strPOSFields = "'Patient Home' AS POSName, PatientPersonT.Address1 + CASE WHEN PatientPersonT.Address2 <> '' THEN ', ' + PatientPersonT.Address2 ELSE '' END + ', ' + PatientPersonT.City + ', ' + PatientPersonT.State + ' ' + PatientPersonT.Zip AS POSAddress, PatientPersonT.Zip AS POSZip, ";
	}
	else {
		//use the POS address for the POS fields
		strPOSFields = "LocationsT.Name AS POSName, LocationsT.Address1 + CASE WHEN LocationsT.Address2 <> '' THEN ', ' + LocationsT.Address2 ELSE '' END + ', ' + LocationsT.City + ', ' + LocationsT.State + ' ' + LocationsT.Zip AS POSAddress, LocationsT.Zip AS POSZip, ";
	}

	// (j.jones 2010-04-08 14:58) - PLID 38094 - Added field for Box16A, which meant updating this query in this code,
	// it was entirely copied from FormLayer with only the Box16A and EmergencyBillsQ changes. Box16A is marked as Yes
	// if any charge on the claim has Emergency = Yes, otherwise it is marked as No.
	// (k.messina 2010-07-15 15:00) - PLID 38783 - added options from remote data to show or hide BOX23 and BOX 24M
	// (r.farnworth 2013-05-23) - PLID 56846 - Added GROUP BY ChargesT.BillID to fix a bug where charges where displaying on bills multiple times.
	// (a.walling 2014-03-06 14:26) - PLID 61234 - Support BillDiagCodeT via BillDiagCodeICD9FlatV/ICD10FlatV aliased as BillDiagCodeFlatV
	// (a.walling 2014-03-17 17:30) - PLID 61407 - Unique diag codes and recalculated WhichCodes using BillDiagsXmlFlat4IF and ChargeWhichCodesIF - NYWC, NYMedicaid
	// (d.lange 2015-11-20 16:16) - PLID 67128 - Updated to calculate the unit allowable based on the ChargesT.AllowableInsuredPartyID
	// (j.jones 2016-04-12 10:59) - NX-100157 - FirstConditionDate is now calculated using ConditionDateType
	g_aryFormQueries[QUERY_BILLS_CHARGES].sql.Format("SELECT %s %s ChargesT.ID, ChargesT.BillID, ChargesT.ServiceID, ChargesT.ItemCode, ChargesT.ItemSubCode, ChargesT.Category, ChargesT.SubCategory, "
				"ChargesT.CPTModifier, ChargesT.CPTModifier2, ChargesT.CPTModifier3, ChargesT.CPTModifier4, ChargesT.LineID, ChargesT.PatCoordID, ChargesT.ServiceLocationID, "
				"ChargesT.CPTMultiplier1, ChargesT.CPTMultiplier2, ChargesT.CPTMultiplier3, ChargesT.CPTMultiplier4, ChargesT.Batched, ChargesT.PackageChargeRefID, ChargesT.PackageQtyRemaining, ChargesT.OriginalPackageQtyRemaining, "
				"ChargesT.AppointmentID, ChargesT.DrugUnitPrice, ChargesT.DrugUnitTypeQual, ChargesT.PrescriptionNumber, ChargesT.NDCCode, ChargesT.Allowable, ChargesT.IsEmergency, "
				"ChargesT.TaxRate, ChargesT.TaxRate2, ChargesT.Quantity, ChargesT.OthrBillFee, ChargesT.DoctorsProviders, ChargesT.ServiceDateFrom AS [Service Date From], "
				"ChargesT.ServiceDateTo AS [Service Date To], ChargesT.ServiceType AS [Service Type], ChargesT.EPSDT, ChargesT.COB, "
			    "BillsT.NoWorkFrom, BillsT.NoWorkTo, BillsT.ConditionDate, "
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
				"END AS FirstConditionDate, "
				"BillsT.OutsideLabCharges, BillsT.Date AS BillDate, LineItemT.Date AS ChargeDate, "
				"DiagCodes1.CodeNumber AS DiagCode, DiagCodes2.CodeNumber AS DiagCode2, DiagCodes3.CodeNumber AS DiagCode3, DiagCodes4.CodeNumber AS DiagCode4, "
				"(CASE WHEN RelatedToEmp = 1 THEN 1 WHEN RelatedToAutoAcc = 1 THEN 2 ELSE 0 END) AS ConditionRelated, "
				"(CASE BillsT.OutsideLab WHEN 1 THEN 1 ELSE 0 END) AS OutsideLabWork, "
				"LineItemT.Date, %s dbo.GetChargeTotal(ChargesT.ID) AS Amount, "
				"(CASE WHEN "
				"	( "
				"		CASE BillsT.ConditionDateType "
				"			WHEN %li THEN BillsT.FirstVisitOrConsultationDate "
				"			WHEN %li THEN BillsT.InitialTreatmentDate "
				"			WHEN %li THEN BillsT.LastSeenDate "
				"			WHEN %li THEN BillsT.AcuteManifestationDate "
				"			WHEN %li THEN BillsT.LastXRayDate "
				"			WHEN %li THEN BillsT.HearingAndPrescriptionDate "
				"			WHEN %li THEN BillsT.AssumedCareDate "
				"			WHEN %li THEN BillsT.RelinquishedCareDate "
				"			WHEN %li THEN BillsT.AccidentDate "
				"			ELSE NULL "
				"		END "
				"	) "
				"Is Null THEN 0 ELSE 1 END) AS HasFirstCond, "
				"CASE WHEN EmergencyBillsQ.BillID Is Not Null THEN 1 ELSE 0 END AS Box16A, "
				"(PersonT.[Last] + ', ' + PersonT.[First] + ' ' + PersonT.Middle + (CASE WHEN (PersonT.Title <> '') THEN (', ' + PersonT.Title) ELSE ('') END)) AS ReferringName, "
				"ReferringPhysT.*, "
				"%s "
				"PlaceOfServiceCodesT.PlaceCodes As ServiceLocation, "
				"PersonT.Address1 + ' ' + PersonT.Address2 + ', ' + PersonT.City + ', ' + PersonT.State + ' ' + PersonT.Zip AS RefFullAddress, "
				"AllowableQ.AllowableQty, (CASE WHEN Amount - AllowableQ.AllowableQty < Convert(money,0) THEN Convert(money,0) ELSE Amount - AllowableQ.AllowableQty END) AS AmountDue "
				"FROM BillsT INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"INNER JOIN PersonT PatientPersonT ON BillsT.PatientID = PatientPersonT.ID "
				"LEFT JOIN PlaceOfServiceCodesT ON ChargesT.ServiceLocationID = PlaceOfServiceCodesT.ID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				"OUTER APPLY dbo.%s(BillsT.ID) BillDiagCodeFlatV "
				"OUTER APPLY dbo.%s(BillDiagCodeFlatV.BillDiagsXml, ChargesT.ID) ChargeWhichCodesQ "
				"LEFT JOIN DiagCodes DiagCodes1 ON BillDiagCodeFlatV.Diag1ID = DiagCodes1.ID LEFT JOIN DiagCodes DiagCodes2 ON BillDiagCodeFlatV.Diag2ID = DiagCodes2.ID LEFT JOIN DiagCodes DiagCodes3 ON BillDiagCodeFlatV.Diag3ID = DiagCodes3.ID LEFT JOIN DiagCodes DiagCodes4 ON BillDiagCodeFlatV.Diag4ID = DiagCodes4.ID "
				"LEFT JOIN PersonT ON BillsT.RefPhyID = PersonT.ID LEFT JOIN ReferringPhysT ON PersonT.ID = ReferringPhysT.PersonID "
				"LEFT JOIN LocationsT ON BillsT.Location = LocationsT.ID "
				"LEFT JOIN InsuredPartyT ON BillsT.InsuredPartyID = InsuredPartyT.PersonID "
				"LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
				"LEFT JOIN (SELECT ChargesT.ID, Round(Convert(money, dbo.GetChargeAllowableForInsuranceCo(ChargesT.ID, InsuranceCoT.PersonID) * ChargesT.Quantity * "
					"(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END) * "
					"(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END) * "
					"(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END) * "
					"(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END) ), 2)  AS AllowableQty "
					"FROM BillsT INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID LEFT JOIN PlaceOfServiceCodesT ON ChargesT.ServiceLocationID = PlaceOfServiceCodesT.ID "
					"LEFT JOIN InsuredPartyT ON ChargesT.AllowableInsuredPartyID = InsuredPartyT.PersonID "
					"LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
					") AS AllowableQ ON ChargesT.ID = AllowableQ.ID "
				"LEFT JOIN (SELECT ChargesT.BillID "
					"FROM ChargesT "
					"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
					"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
					"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
					"WHERE LineItemT.Deleted = 0 AND ChargesT.Batched = 1 "
					"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
					"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
					"AND ChargesT.IsEmergency = %li AND ChargesT.BillID = %li "
					"GROUP BY ChargesT.BillID "
					") AS EmergencyBillsQ ON BillsT.ID = EmergencyBillsQ.BillID "
				"WHERE LineItemT.Deleted = 0 AND Batched = 1 AND BillsT.ID = %li "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"ORDER BY LineID", strBox24H
				, (GetRemotePropertyInt("NYMedicaidBox24M",0,0,"<None>",true) == 0 ? "BillsT.HospFrom, BillsT.HospTo, " : "")
				, ConditionDateType::cdtFirstVisitOrConsultation444
				, ConditionDateType::cdtInitialTreatmentDate454
				, ConditionDateType::cdtLastSeenDate304
				, ConditionDateType::cdtAcuteManifestation453
				, ConditionDateType::cdtLastXray455
				, ConditionDateType::cdtHearingAndPrescription471
				, ConditionDateType::cdtAssumedCare090
				, ConditionDateType::cdtRelinquishedCare91
				, ConditionDateType::cdtAccident439
				, (GetRemotePropertyInt("NYMedicaidBox23", 0, 0, "<None>", true) == 0 ? "LineItemT.Description AS LineItemDescription, " : "")
				, ConditionDateType::cdtFirstVisitOrConsultation444
				, ConditionDateType::cdtInitialTreatmentDate454
				, ConditionDateType::cdtLastSeenDate304
				, ConditionDateType::cdtAcuteManifestation453
				, ConditionDateType::cdtLastXray455
				, ConditionDateType::cdtHearingAndPrescription471
				, ConditionDateType::cdtAssumedCare090
				, ConditionDateType::cdtRelinquishedCare91
				, ConditionDateType::cdtAccident439
				, strPOSFields
				, ShouldUseICD10(m_InsuredPartyID, m_ID) ? "BillICD10DiagsXmlFlat4IF" : "BillICD9DiagsXmlFlat4IF"
				, ShouldUseICD10(m_InsuredPartyID, m_ID) ? "ChargeWhichICD10CodesIF" : "ChargeWhichICD9CodesIF",
				cietYes, billid, billid);

	m_pframe->Load (QUERY_BILLS_CHARGES, "", "", &i); // Bill/Charge Information

	// (j.jones 2013-04-04 17:02) - PLID 56038 - clear Box 25 if the preference says to
	if(GetRemotePropertyInt("NYMedicaidDoNotFillBox25",1,0,"<None>",true) == 1) {
		ExecuteSqlStd("UPDATE FormControlsT SET Source = '' WHERE ID = 8685");
	}
	else {
		ExecuteSqlStd("UPDATE FormControlsT SET Source = 'DocName' WHERE ID = 8685");
	}

	// Provider Information
	if(GetRemotePropertyInt("NYMedicaidUseProvider",0,0,"<None>",true) == 1) {
		//use the override information

		// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" charges
		g_aryFormQueries[QUERY_PROVIDER].sql.Format("SELECT '%s' AS DocName, '%s' AS LocName, '%s' AS LocAddress, "
			"'%s' + ', ' + '%s' + ' ' + '%s' AS LocCityStateZip, "
			"'%s' + ', ' + '%s' + ', ' + '%s' + ' ' + '%s' AS LocFullAddress, '%s' AS LocPhone, "
			"PersonT.*, ProvidersT.*, InsuranceAcceptedT.*, (CASE Accepted WHEN 0 THEN 0 ELSE 1 END) AS AcceptAssignment "
			"FROM PersonT INNER JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID CROSS JOIN LocationsT INNER JOIN LineItemT ON LocationsT.ID = LineItemT.LocationID "
			"INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID LEFT JOIN InsuranceAcceptedT ON InsuranceAcceptedT.ProviderID = PersonT.ID "
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID",
			_Q(GetRemotePropertyText("NYMedicaidOverrideName","",0,"<None>")),
			_Q(GetRemotePropertyText("NYMedicaidOverrideAddress","",0,"<None>")),
			_Q(GetRemotePropertyText("NYMedicaidOverrideCity","",0,"<None>")),
			_Q(GetRemotePropertyText("NYMedicaidOverrideState","",0,"<None>")),
			_Q(GetRemotePropertyText("NYMedicaidOverrideZip","",0,"<None>")),
			_Q(GetRemotePropertyText("NYMedicaidOverrideAddress","",0,"<None>")),
			_Q(GetRemotePropertyText("NYMedicaidOverrideCity","",0,"<None>")),
			_Q(GetRemotePropertyText("NYMedicaidOverrideState","",0,"<None>")),
			_Q(GetRemotePropertyText("NYMedicaidOverrideZip","",0,"<None>")),
			_Q(GetRemotePropertyText("NYMedicaidOverridePhone","",0,"<None>")));
	}
	else {
		// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" charges
		g_aryFormQueries[QUERY_PROVIDER].sql = "SELECT First + (CASE WHEN Middle <> '' AND Middle <> ' ' THEN ' ' + Left(Middle,1) ELSE '' END)  + ' ' + Last + ' ' + Title AS DocName, LocationsT.Name AS LocName, LocationsT.Address1 + ' ' + LocationsT.Address2 AS LocAddress, "
			"LocationsT.City + ', ' + LocationsT.State + ' ' + LocationsT.Zip AS LocCityStateZip, "
			"LocationsT.Address1 + ' ' + LocationsT.Address2 + ', ' + LocationsT.City + ', ' + LocationsT.State + ' ' + LocationsT.Zip AS LocFullAddress, LocationsT.Phone AS LocPhone, "
			"LocationsT.City + ', ' + LocationsT.State + ' ' + LocationsT.Zip AS LocCityStateZip, "
			"PersonT.Address1 + ' ' + PersonT.Address2 + ', ' AS DocAddress, PersonT.City + ', ' + PersonT.State + ' ' + PersonT.Zip AS DocCityStateZip, "
			"PersonT.*, ProvidersT.*, InsuranceAcceptedT.*, (CASE Accepted WHEN 0 THEN 0 ELSE 1 END) AS AcceptAssignment "
			"FROM PersonT INNER JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID CROSS JOIN LocationsT INNER JOIN LineItemT ON LocationsT.ID = LineItemT.LocationID "
			"INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID LEFT JOIN InsuranceAcceptedT ON InsuranceAcceptedT.ProviderID = PersonT.ID "
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID";
	}

	where.Format (" WHERE PersonT.ID = %li AND LineItemT.Deleted = 0 AND Batched = 1 "
		"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
		"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
		"AND ChargesT.BillID = %li AND InsuranceCoID = (SELECT InsuranceCoID FROM InsuredPartyT WHERE PersonID = %li)", adwProviderList.GetAt(provider_index), billid, m_InsuredPartyID);
	m_pframe->Load (QUERY_PROVIDER, where, "", &i); // Provider Information

	m_pframe->Load (QUERY_CHARGE_TOTAL, "", "", &i); // Charge Totals
	m_pframe->Load (QUERY_PAYMENT_TOTAL, "", "", &i); // Payment Totals
	m_pframe->Load (QUERY_BALANCE, "", "", &i); // Balance

#define scale * 19 / 20

	//add buttons 
/*
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
	if (!m_pupbutton->Create ("/\', WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, rect, m_pframe, IDC_BILL_UP))
		AfxMessageBox ("Failed");
	rect.top	= 817 scale;
	rect.bottom = 932 scale;
	if (!m_pdownbutton->Create ("\\/", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, rect, m_pframe, IDC_BILL_DOWN))
		AfxMessageBox ("Failed");
*/

	//set printer settings
	PRINT_X_SCALE	= 16.12;
	PRINT_Y_SCALE	= 16.12;
	PRINT_X_OFFSET	= -480;
	PRINT_Y_OFFSET	= 595;

	m_pframe->m_pOnCommand = m_pOnCommand;
	m_pframe->m_pOnKeyDown = m_pOnKeyDown;

	m_pframe->TrackChanges(0, 0);

	return TRUE;
}

void CNYMedicaidDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	SetControlPositions();
}

void CNYMedicaidDlg::SetControlPositions(void)
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

		//GetDlgItem(IDC_RADIO_NO_BATCH)->MoveWindow(0,0,0,0, false);
		//GetDlgItem(IDC_RADIO_PAPER)->MoveWindow(0,0,0,0, false);
		GetDlgItem(IDC_RADIO_ELECTRONIC)->MoveWindow(0,0,0,0, false);

		Invalidate();
	}
}

// (k.messina 2010-03-15 10:34) - PLID 37323 get doc's ID  
//copied from the HCFADLG.CPP and altered for this form's purposes.
long CNYMedicaidDlg::Doc_ID()
{
	CString strSQL, str;
	long doc_id = -1; //, loc_id;
	try {
		_RecordsetPtr rs;
		str.Format("SELECT DoctorsProviders, LocationID FROM (%s) AS FormChargesQ",g_aryFormQueries[2].sql);
		rs = CreateRecordset(str);
		if(!rs->eof) {
			doc_id = AdoFldLong(rs, "DoctorsProviders",-1);
			//loc_id = AdoFldLong(rs, "LocationID",-1);
		}
		rs->Close();

		return doc_id;
	}
	NxCatchAll("CNYMedicaidDlg::Doc_ID") {
		return -1;
	};
}

void CNYMedicaidDlg::OnClickX()
{
	EndDialog (0);
}

void CNYMedicaidDlg::OnClickCheck() 
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

void CNYMedicaidDlg::OnClose() 
{
	CDialog::OnClose();
}

BOOL CNYMedicaidDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	return CDialog::OnCommand(wParam, lParam);
}

extern int FONT_SIZE;
extern int MINI_FONT_SIZE;
void CNYMedicaidDlg::OnClickPrint() 
{
	// Get the alignment settings
	_RecordsetPtr rs;
	COleVariant var;
	int x_offset = 0, y_offset = 0, x_scale = 0, y_scale = 0;
	FONT_SIZE = 12;
	MINI_FONT_SIZE = 10;

	m_pframe->m_printDlg = m_printDlg;

	try {

		// (j.jones 2007-04-25 11:43) - PLID 4758 - Handle having a default per workstation,
		// defaulting to the FormAlignT default printer if the per-workstation default doesn't exist.
		long nDefault = GetPropertyInt("DefaultFormAlignID", -1, FORM_ID, FALSE);
		CString strDefault = "[Default] = 1";
		if(nDefault != -1)
			strDefault.Format("ID = %li", nDefault);


		rs = CreateRecordset("SELECT * FROM FormAlignT WHERE %s AND FormID = %li",strDefault,FORM_ID);
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
		}
		else {
			if(m_bPrintWarnings && IDYES == MessageBox("You have not saved this form's alignment settings. The printout may not be aligned correctly until you do this.\n"
				"To set this up properly, click \"Yes\" and then click on the \"Align Form\" button. Create a printer, and check the \"Default For My Workstation\" box.\n"
				"Then configure the alignment as needed. Would you like to cancel printing and align the form now?","Practice",MB_ICONEXCLAMATION|MB_YESNO)) {
				return;
			}
		}
		rs->Close();

		PRINT_X_OFFSET += x_offset * 10;
		PRINT_Y_OFFSET += y_offset * 10;
		PRINT_X_SCALE += x_scale * 0.01;
		PRINT_Y_SCALE += y_scale * 0.01;
		if(!m_pframe->OnPrint(m_CapOnPrint, "NYMedicaid", NULL, m_pPrintDC)) {
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

		//Send Type 5 - Paper NYMedicaid Form
		
		// (j.jones 2013-01-23 09:19) - PLID 54734 - moved the claim history addition to its own function,
		// this also updates HCFATrackT.LastSendDate
		AddToClaimHistory(m_ID, m_InsuredPartyID, ClaimSendType::NYMedicaid);
		
		//now add to patient history
		CString str, strDesc = "NYMedicaid Form Printed";
		long PatientID;
		//get ins. co. name
		_RecordsetPtr rs = CreateRecordset("SELECT InsuranceCoT.Name, InsuredPartyT.PatientID FROM InsuranceCoT INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID WHERE InsuredPartyT.PersonID = %li",m_InsuredPartyID);
		if(!rs->eof) {
			str.Format(" - Sent To %s",CString(rs->Fields->Item["Name"]->Value.bstrVal));
			strDesc += str;
			PatientID = rs->Fields->Item["PatientID"]->Value.lVal;
		}
		rs->Close();

		rs = CreateRecordset("SELECT TOP 1 LineItemT.Date FROM LineItemT INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID WHERE ChargesT.BillID = %li",m_ID);
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
	}NxCatchAll("Error printing NYMedicaid form.");
}

void CNYMedicaidDlg::BuildNYMedicaidChargesT()
{
	extern int SizeOfFormChargesT();

	try {

		_RecordsetPtr rs;
		CString str;

		billid = m_ID;

		//DRT 4/10/2006 - PLID 11734 - Removed ProcCode
		// (j.gruber 2009-03-18 12:33) - PLID 33574 - discount structure change
		// (j.jones 2010-04-07 17:29) - PLID 15224 - removed ChargesT.EMG, obsolete field
		// (k.messina 2010-07-16 15:22) - PLID 38783 - Adjust for Box24H user settings
		// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" charges & applies
		// (a.walling 2014-03-06 14:49) - PLID 61234 - Support BillDiagCodeT via BillDiagCodeICD9FlatV/ICD10FlatV aliased as BillDiagCodeFlatV
		// (a.walling 2014-03-10 13:29) - PLID 61305 - Emulate the old WhichCodes field via ChargeWhichCodesFlatV
		// (a.walling 2014-03-17 17:30) - PLID 61407 - Unique diag codes and recalculated WhichCodes using BillDiagsXmlFlat4IF and ChargeWhichCodesIF - NYWC, NYMedicaid
		g_aryFormQueries[2].sql.Format("SELECT ChargeWhichCodesQ.WhichCodes, ChargesT.ID, ChargesT.BillID, ChargesT.ItemCode, ChargesT.ItemSubCode, ChargesT.Category, ChargesT.SubCategory, LineItemT.Description, ((CASE WHEN ChargesT.CPTModifier Is Null Then '' ELSE ChargesT.CPTModifier END) + (CASE WHEN ChargesT.CPTModifier2 Is Null Then '' ELSE '  ' + ChargesT.CPTModifier2 END)) AS CPTModifier, DiagCodes1.CodeNumber AS DiagCode, DiagCodes2.CodeNumber AS DiagCode2, DiagCodes3.CodeNumber AS DiagCode3, DiagCodes4.CodeNumber AS DiagCode4, ChargesT.TaxRate, ChargesT.TaxRate2, ChargesT.Quantity, LineItemT.Amount, ChargesT.OthrBillFee, ChargesT.DoctorsProviders, ChargesT.ServiceDateFrom AS [Service Date From], "
					"ChargesT.ServiceDateTo AS [Service Date To], PlaceofServiceCodesT.PlaceCodes AS [Service Location], ChargesT.ServiceType AS [Service Type], ChargesT.EPSDT, ChargesT.COB, LineItemT.Date, LineItemT.InputDate, LineItemT.InputName, LineItemT.LocationID, (SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) as PercentOff, (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) as Discount, (CASE WHEN ChargeRespT.Amount Is Not Null THEN ChargeRespT.Amount ELSE 0 END) AS InsResp, (CASE WHEN ChargeRespT.Amount Is Not Null THEN InsuredPartyT.RespTypeID ELSE 0 END) AS InsType, dbo.GetChargeTotal(ChargesT.ID) AS ChargeTotal, "
					"SumOfAmount AS ApplyTotal FROM (BillsT LEFT JOIN ChargesT ON BillsT.ID = ChargesT.BillID) LEFT JOIN LineItemT ON ChargesT.ID = LineItemT.ID LEFT JOIN PlaceOfServiceCodesT ON ChargesT.ServiceLocationID = PlaceOfServiceCodesT.ID LEFT JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ID LEFT JOIN InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID "
					"OUTER APPLY dbo.%s(BillsT.ID) BillDiagCodeFlatV "
					"OUTER APPLY dbo.%s(BillDiagCodeFlatV.BillDiagsXml, ChargesT.ID) ChargeWhichCodesQ "
					"LEFT JOIN DiagCodes DiagCodes1 ON BillDiagCodeFlatV.Diag1ID = DiagCodes1.ID LEFT JOIN DiagCodes DiagCodes2 ON BillDiagCodeFlatV.Diag2ID = DiagCodes2.ID LEFT JOIN DiagCodes DiagCodes3 ON BillDiagCodeFlatV.Diag3ID = DiagCodes3.ID LEFT JOIN DiagCodes DiagCodes4 ON BillDiagCodeFlatV.Diag4ID = DiagCodes4.ID LEFT JOIN CPTModifierT ON CPTModifier = CPTModifierT.Number LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number "
					"LEFT JOIN (SELECT Sum(Round(AppliesT.Amount,2)) AS SumOfAmount, DestID "
					"	FROM AppliesT "
					"	INNER JOIN LineItemT ON AppliesT.SourceID = LineItemT.ID "
					"	LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
					"	LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
					"	WHERE Deleted = 0 "
					"	AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
					"	AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
					"	GROUP BY DestID) AS AppliesQ ON ChargesT.ID = AppliesQ.DestID "
					"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
					"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
					"WHERE BillsT.ID = %li AND LineItemT.Deleted = 0 AND Batched = 1 "
					"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
					"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null"
					, ShouldUseICD10(m_InsuredPartyID, m_ID) ? "BillICD10DiagsXmlFlat4IF" : "BillICD9DiagsXmlFlat4IF"
					, ShouldUseICD10(m_InsuredPartyID, m_ID) ? "ChargeWhichICD10CodesIF" : "ChargeWhichICD9CodesIF"
					, billid
				);

		// Now build a list of different charge providers. Assume every charge
		// has a provider in FormChargesT.

		firstcharge = 0;

		adwProviderList.RemoveAll();
		// (j.jones 2006-12-06 10:40) - PLID 23734 - supported ClaimProviderID override
		// (j.jones 2007-02-22 11:33) - PLID 24878 - fixed ClaimProviderID usage such that -1 was still returned if there was no provider on a charge
		// (j.jones 2010-11-09 14:42) - PLID 41387 - supported ChargesT.ClaimProviderID
		// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" charges
		rs = CreateParamRecordset("SELECT Coalesce(ChargesT.ClaimProviderID, Coalesce(ClaimProvidersT.PersonID, -1)) AS ClaimProviderID FROM ChargesT "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"LEFT JOIN ProvidersT ON ChargesT.DoctorsProviders = ProvidersT.PersonID "
			"LEFT JOIN ProvidersT ClaimProvidersT ON ProvidersT.ClaimProviderID = ClaimProvidersT.PersonID "
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
			"WHERE ChargesT.BillID = {INT} AND LineItemT.Deleted = 0 AND Batched = 1 "
			"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
			"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null",billid);
		while (!rs->eof) {
			int i, iProviderID = rs->Fields->Item["ClaimProviderID"]->Value.lVal;
			for (i=0; i < adwProviderList.GetSize(); i++) {
				if ((int)adwProviderList.GetAt(i) == iProviderID) break;
			}
			if (i == adwProviderList.GetSize())
				adwProviderList.Add(iProviderID);

			rs->MoveNext();
		}
		rs->Close();	
		
		// Rebuild the charge list with the first provider if there is more than one
		provider_index = 0;
		if (adwProviderList.GetSize() > 0) {

				//JJ - 9/27/01 - While this works fine, we can't switch pages to show charges for different providers
				//TODO - comment back in when we can switch pages
				//g_aryFormQueries[2].sql.Format("SELECT ChargesT.*, LineItemT.[Date] AS TDate, LineItemT.Description AS ItemDesc FROM BillsT INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID WHERE LineItemT.Deleted = 0 AND Batched = 1 AND BillsT.ID = %li AND DoctorsProviders = %li", billid,adwProviderList.GetAt(0));
		}

		CString temp;

		if(firstcharge>1) {
			str = "";
			int i = 0;
			_RecordsetPtr rs = CreateRecordset(g_aryFormQueries[2].sql + " ORDER BY LineID");
			if(!rs->eof) {
				str = " AND (";
				//loop up to this page
				for(i=0;i<(7*((long)(firstcharge/7)));i++) {
					rs->MoveNext();
				}
				//loop through the charges on this page
				for(i=0;i<7;i++) {
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
		}
		else {
			str = "";
			_RecordsetPtr rs = CreateRecordset(g_aryFormQueries[2].sql + " ORDER BY LineID");
			if(!rs->eof) {
				str = " AND (";
				//loop through the charges on this page
				for(int i=0;i<7;i++) {
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
		}

		str.Format("SELECT ChargeTotal, (CASE WHEN ApplyTotal Is Null THEN 0 ELSE ApplyTotal END) AS ApplyTotal FROM (%s) AS FormChargesT",temp);
		g_aryFormQueries[QUERY_CHARGE_TOTAL].sql = "SELECT Convert(money,Sum(ChargeTotal)) AS TotalCharges FROM (SELECT TOP 7 NYMedicaidChargeTopQ.ChargeTotal, NYMedicaidChargeTopQ.ApplyTotal "
				"FROM (" + str + ") AS NYMedicaidChargeTopQ) AS NYMedicaidCharge6Q";

		BOOL bShowPayments = TRUE;

		if(bShowPayments) {
			g_aryFormQueries[QUERY_PAYMENT_TOTAL].sql = "SELECT Sum(ApplyTotal) AS TotalApplies FROM (SELECT TOP 7 NYMedicaidChargeTopQ.ChargeTotal, NYMedicaidChargeTopQ.ApplyTotal "
					"FROM (" + str + ") AS NYMedicaidChargeTopQ) AS NYMedicaidCharge6Q";
			g_aryFormQueries[QUERY_BALANCE].sql = "SELECT Sum(Convert(money,(ChargeTotal)-(ApplyTotal))) AS TotalBalance FROM (SELECT TOP 7 NYMedicaidChargeTopQ.ChargeTotal, NYMedicaidChargeTopQ.ApplyTotal "
					"FROM (" + str + ") AS NYMedicaidChargeTopQ) AS NYMedicaidCharge6Q";
		}
		else {
			g_aryFormQueries[QUERY_PAYMENT_TOTAL].sql = "SELECT Convert(money,'$0.00') AS TotalApplies ";
			g_aryFormQueries[QUERY_BALANCE].sql = "SELECT Sum(Convert(money,(ChargeTotal))) AS TotalBalance FROM (SELECT TOP 7 NYMedicaidChargeTopQ.ChargeTotal, NYMedicaidChargeTopQ.ApplyTotal "
					"FROM (" + str + ") AS NYMedicaidChargeTopQ) AS NYMedicaidCharge6Q";
		}

		// Fill total charge amount for each line
		nTotalCharges = SizeOfFormChargesT();

	} NxCatchAll("CNYMedicaidDlg::BuildNYMedicaidChargesT()");
}

void OnNYMedicaidCommand(WPARAM wParam, LPARAM lParam, CDWordArray& dwChangedForms, int FormControlsID, CFormDisplayDlg* dlg) 
{
}

void OnNYMedicaidKeyDown(CDialog* pFormDisplayDlg, MSG* pMsg)
{
	CFormDisplayDlg* pDlg = (CFormDisplayDlg*)pFormDisplayDlg;
	CWnd* pCtrl = pDlg->GetFocus();
}

void CNYMedicaidDlg::OnClickAlign() 
{
	CPrintAlignDlg dlg(this);
	dlg.m_FormID = FORM_ID;
	dlg.DoModal();
}

void CNYMedicaidDlg::OnClickCapitalizeOnPrint() 
{
	if(m_CapOnPrint) {
		SetRemotePropertyInt("CapitalizeNYMedicaid",0);
	}
	else {
		SetRemotePropertyInt("CapitalizeNYMedicaid",1);
	}
	m_CapOnPrint = GetRemotePropertyInt("CapitalizeNYMedicaid",0);
}

void CNYMedicaidDlg::OnClickRestore() 
{
	CString strSQL;

	if(!CheckCurrentUserPermissions(bioClaimForms,sptWrite))
		return;

	if (IDNO == MessageBox("All changes you have made to the NYMedicaid form will be unrecoverable. Are you sure you wish to do this?", NULL, MB_YESNO))
		return;

	CWaitCursor pWait;

	try {
	// Delete all the form changes
	// (j.jones 2008-05-28 15:46) - PLID 27881 - batched statements, ensured we delete all relevant FormHistory entries
	CString strBatch = BeginSqlBatch();
	AddStatementToSqlBatch(strBatch, "DELETE FROM FormHistoryT WHERE DocumentID IN (SELECT FirstDocumentID FROM HCFADocumentsT WHERE BillID = %li AND FormType = %li)", m_ID, FORM_ID);
	AddStatementToSqlBatch(strBatch, "DELETE FROM HCFADocumentsT WHERE HCFADocumentsT.BillID = %li AND FormType = %li", m_ID, FORM_ID);
	ExecuteSqlBatch(strBatch);

	// Remove all changes made by the user
	m_pframe->UndoChanges();

	} NxCatchAll("Error restoring defaults.");

	//Refresh the form

	m_pframe->Refresh(QUERY_LINES_PAT);  // Lines, Labels, and Patient Information
	m_pframe->Refresh(QUERY_PRIMARY_INSUR);  // Primary Insurance Co Info
	m_pframe->Refresh(QUERY_BILLS_CHARGES);  // Bill/Charge Information
	m_pframe->Refresh(QUERY_PROVIDER);  // Provider Information
	m_pframe->Refresh(QUERY_CHARGE_TOTAL);  // Charge Totals
	m_pframe->Refresh(QUERY_PAYMENT_TOTAL);  // Payment Totals
	m_pframe->Refresh(QUERY_BALANCE);  // Balance

	//track changes now, otherwise any changes made after a restore will not be saved!
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

void CNYMedicaidDlg::Save(BOOL boSaveAll, CDialog* pdlgWait, int& iPage)
{
	_RecordsetPtr nxrs(__uuidof(Recordset));

	int provider;

	for (provider_index=0; provider_index < adwProviderList.GetSize(); provider_index++) {

		provider = adwProviderList.GetAt(provider_index);

		for (firstcharge=0; firstcharge < nTotalCharges; firstcharge+=6) {

			if (pdlgWait) {
				CString str;
				str.Format("Saving NYMedicaid page %d...", ++iPage);
				pdlgWait->GetDlgItem(IDC_LABEL)->SetWindowText(str);
				pdlgWait->RedrawWindow();
			}

			SaveHistoricalData(boSaveAll);
		}
	}
}

void CNYMedicaidDlg::SaveHistoricalData(BOOL boSaveAll)
{
#ifdef SAVE_HISTORICAL_INFORMATION

	int iDocumentID;
	CString str;
	_RecordsetPtr rs(__uuidof(Recordset));

	try {		
		str.Format("SELECT FirstDocumentID FROM HCFADocumentsT WHERE BillID = %d AND FirstCharge = %d AND ProviderIndex = %d AND FormType = %li", m_ID, firstcharge, adwProviderList.GetAt(provider_index), FORM_ID);
		rs = CreateRecordset(str);
		if (!rs->eof) {
			iDocumentID = rs->Fields->Item["FirstDocumentID"]->Value.lVal;
			rs->Close();
		}
		else {
			rs->Close();
			iDocumentID = NewNumber("FormHistoryT","DocumentID");
			//BEGIN_TRANS("SaveHistoricalNYMedicaid") {
			str.Format("INSERT INTO HCFADocumentsT ( BillID, FirstDocumentID, FirstCharge, ProviderIndex, FormType ) VALUES ( %d, %d, %d, %d, %li )", m_ID, iDocumentID, firstcharge, adwProviderList.GetAt(provider_index), FORM_ID);
			ExecuteSql(str);
			str.Format("INSERT INTO FormHistoryT ( DocumentID, ControlID, [Value] ) VALUES ( %d, -1, NULL )", iDocumentID);
			ExecuteSql(str);
			//} END_TRANS_CATCH_ALL("SaveHistoricalNYMedicaid");
		}		
	}
	NxCatchAll("Error in OnClickCheck()");

	///////////////////////////////////
	// Make sure we pull our historical data from the right table
	m_pframe->SetDocumentID(iDocumentID);

	m_pframe->Refresh(QUERY_LINES_PAT);  // Lines, Labels, and Patient Information
	m_pframe->Refresh(QUERY_PRIMARY_INSUR);  // Primary Insurance Co Info
	m_pframe->Refresh(QUERY_BILLS_CHARGES);  // Bill/Charge Information
	m_pframe->Refresh(QUERY_PROVIDER);  // Provider Information
	m_pframe->Refresh(QUERY_CHARGE_TOTAL);  // Charge Totals
	m_pframe->Refresh(QUERY_PAYMENT_TOTAL);  // Payment Totals
	m_pframe->Refresh(QUERY_BALANCE);  // Balance

	m_pframe->ReapplyChanges(firstcharge, provider_index);

	m_pframe->Save (firstcharge, provider_index, iDocumentID, boSaveAll);

	// We save form 6 (zero based in the array) because where the radio button is
	// determines what text goes in a particular control. To save us time and pain,
	// we always save the data in form 6.
	//((FormLayer*)m_pframe->m_LayerArray[5])->Save(iDocumentID, 0, &m_pframe->m_ControlArray);
#endif
}

long CNYMedicaidDlg::DoScrollTo(long nNewTopPos)
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

void CNYMedicaidDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
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

BOOL PreNYMedicaidPrint(CDWordArray& dwChangedForms, CFormDisplayDlg* dlg)
{

	/*
	if (adwProviderList.GetSize() == 0) 
	{	ASSERT(FALSE);
		HandleException (NULL, "Failure in PreNYMedicaidPrint", __LINE__, __FILE__);
		return FALSE; // This should never happen
	}

	// Only happens on first call to this function. The first provider
	// information is reloaded.
	if (nPrintedCharges == 0 && nPrintedProviders == 0) {
		oldfirstcharge = firstcharge;
		oldprovider_index = provider_index;		

		provider_index = -1;
		OnNYMedicaidCommand(IDC_PHYSICIAN_LEFT, 0, dwChangedForms, 0, dlg);
		firstcharge = -6;
		nPrintedProviders++;
	}


	// If still more charges to print, print them and return
	if (nPrintedCharges < nTotalCharges) {		
		OnNYMedicaidCommand(IDC_BILL_DOWN, 0, dwChangedForms, 0, dlg);
		nPrintedCharges += 6;
		return TRUE;
	}
	// Otherwise, go for the next provider
	else {
		nPrintedCharges = 0;

		// If no more providers, exit
		if (provider_index == adwProviderList.GetSize()-1) { 
			nPrintedProviders = 0;
			firstcharge = oldfirstcharge;
			oldprovider_index = provider_index;
			return FALSE;
		}

		OnNYMedicaidCommand(IDC_PHYSICIAN_RIGHT, 0, dwChangedForms, 0, dlg);
		firstcharge = -6;
		OnNYMedicaidCommand(IDC_BILL_DOWN, 0, dwChangedForms, 0, dlg);
		nPrintedCharges = 6;
		nPrintedProviders++;
		return TRUE;
	}
	*/

	return TRUE;
}

void CNYMedicaidDlg::OnRadioPaper() {

	if(!CheckCurrentUserPermissions(bioClaimForms,sptWrite)) {
		if(FindHCFABatch(m_ID) == 1) {
			((CButton*)GetDlgItem(IDC_RADIO_PAPER))->SetCheck(TRUE);
			((CButton*)GetDlgItem(IDC_RADIO_NO_BATCH))->SetCheck(FALSE);
		}
		else {
			((CButton*)GetDlgItem(IDC_RADIO_PAPER))->SetCheck(FALSE);
			((CButton*)GetDlgItem(IDC_RADIO_NO_BATCH))->SetCheck(TRUE);
		}
		return;
	}

	//don't automatically batch a $0.00 claim
	COleCurrency cyCharges = COleCurrency(0,0);
	_RecordsetPtr rs = CreateRecordset("SELECT dbo.GetClaimTotal(ID) AS Total FROM BillsT WHERE ID = %li",m_ID);
	if(!rs->eof) {
		cyCharges = AdoFldCurrency(rs, "Total",COleCurrency(0,0)); 
	}
	rs->Close();

	if (((CButton*)GetDlgItem(IDC_RADIO_PAPER))->GetCheck()) {

		CString strWarn;
		strWarn.Format("This claim is currently %s. Are you sure you wish to send the claim to the paper batch?",
			FormatCurrencyForInterface(cyCharges,TRUE,TRUE));

		if(cyCharges > COleCurrency(0,0) || 
			IDYES == MessageBox(strWarn,"Practice",MB_YESNO|MB_ICONQUESTION)) {

			short nBatch = 0;

			if (((CButton*)GetDlgItem(IDC_RADIO_PAPER))->GetCheck())
				nBatch = 1; // paper batch

			BatchBill(m_ID, 1);
		}
		else {
			
			if(FindHCFABatch(m_ID) == 1) {
				((CButton*)GetDlgItem(IDC_RADIO_PAPER))->SetCheck(TRUE);
				((CButton*)GetDlgItem(IDC_RADIO_NO_BATCH))->SetCheck(FALSE);
			}
			else {
				((CButton*)GetDlgItem(IDC_RADIO_PAPER))->SetCheck(FALSE);
				((CButton*)GetDlgItem(IDC_RADIO_NO_BATCH))->SetCheck(TRUE);
			}
		}
	}
	else {
		BatchBill(m_ID, 0);
	}
}

void CNYMedicaidDlg::OnRadioNoBatch() {

	OnRadioPaper();
}

// (j.jones 2007-06-22 13:23) - PLID 25665 - handle when a field was edited, so we update the info text box
BOOL CNYMedicaidDlg::PreTranslateMessage(MSG *pMsg) {

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

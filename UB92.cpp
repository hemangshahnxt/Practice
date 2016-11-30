// UB92Dlg.cpp : implementation file // (a.vengrofski 2010-02-03 15:02) - PLID <35133> - there was the letters hcf at the begining of this line... don't know why.
//

#include "stdafx.h"
#include "UB92.h"
#include "FormDisplayDlg.h"
#include "FormFormat.h"
#include "FormEdit.h"
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

// UB92 stuff

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

BOOL PreUB92Print(CDWordArray& dwChangedForms, CFormDisplayDlg* dlg);

void RequeryHistoricalUB92Data()
{
	try {
		_RecordsetPtr rs(__uuidof(Recordset));
		COleVariant var;
		CString str;

		str.Format("SELECT FirstDocumentID FROM HCFADocumentsT WHERE BillID = %li AND FirstCharge = %li AND ProviderIndex = %li AND FormType = 1", billid, firstcharge, adwProviderList.GetAt(provider_index));
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
// CUB92Dlg dialog

BEGIN_MESSAGE_MAP(CUB92Dlg, CDialog)
	//{{AFX_MSG_MAP(CUB92Dlg)
	ON_WM_SIZE()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_X, OnClickX)
	ON_BN_CLICKED(IDC_CHECK, OnClickCheck)
	ON_BN_CLICKED(IDC_PRINT, OnClickPrint)
	ON_BN_CLICKED(IDC_ALIGN, OnClickAlign)
	ON_BN_CLICKED(IDC_RESTORE, OnClickRestore)
	ON_BN_CLICKED(IDC_RADIO_PAPER, OnRadioPaper)
	ON_BN_CLICKED(IDC_RADIO_ELECTRONIC, OnRadioElectronic)
	ON_BN_CLICKED(IDC_RADIO_NO_BATCH, OnRadioNoBatch)
	ON_BN_CLICKED(IDC_CAP_ON_PRINT, OnClickCapitalizeOnPrint)
	ON_WM_VSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CUB92Dlg, CDialog)
    //{{AFX_EVENTSINK_MAP(CUB92Dlg)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void OnUB92Command(WPARAM wParam, LPARAM lParam, CDWordArray& dwChangedForms, int FormControlsID, CFormDisplayDlg* dlg);
void OnUB92KeyDown(CDialog* pFormDisplayDlg, MSG* pMsg);

CUB92Dlg::CUB92Dlg(CWnd* pParent /*=NULL*/)
	: CDialog(CUB92Dlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CUB92Dlg)
		m_bPrintWarnings = TRUE;
	//}}AFX_DATA_INIT
	m_pframe = 0;
	m_ID = -1;
	m_InsuredPartyID = -1;
	m_OthrInsuredPartyID = -1;

	m_pOnCommand = &OnUB92Command;
	m_pOnKeyDown = &OnUB92KeyDown;
	m_pPrePrintFunc = NULL;
	m_ShowWindowOnInit = TRUE;
}

CUB92Dlg::~CUB92Dlg()
{
	delete m_pframe;
//	delete m_pleftbutton;
//	delete m_prightbutton;
//	delete m_pupbutton;
//	delete m_pdownbutton;
}

void CUB92Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUB92Dlg)
	// (j.jones 2007-06-25 10:14) - PLID 25663 - changed the buttons to NxIconButtons
	DDX_Control(pDX, IDC_RESTORE, m_btnRestoreDefaults);
	DDX_Control(pDX, IDC_ALIGN, m_btnAlignForm);
	DDX_Control(pDX, IDC_CHECK, m_btnSave);
	DDX_Control(pDX, IDC_PRINT, m_btnPrint);
	DDX_Control(pDX, IDC_X, m_btnClose);
	DDX_Control(pDX, IDC_INFO_TEXT, m_nxstaticInfoText);
	//}}AFX_DATA_MAP
}

int CUB92Dlg::DoModal(int billID) 
{
	m_ID = billID;
	return CDialog::DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CUB92Dlg message handlers

BOOL CUB92Dlg::OnInitDialog() 
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

	/// UB92 SPECIFIC STUFF ////////////
	_RecordsetPtr rs = CreateRecordset("SELECT Description, InsuredPartyID, OthrInsuredPartyID FROM (SELECT BillsT.* FROM BillsT "
		"WHERE (((BillsT.PatientID)=%li) AND ((BillsT.Deleted)=0))) AS PatientBillsQ  WHERE ID = %li", m_PatientID, m_ID);
	if(!rs->eof) {
		_variant_t var;
		var = rs->Fields->Item["InsuredPartyID"]->Value;
		if (var.vt != VT_NULL)
			m_InsuredPartyID = var.lVal;
		var = rs->Fields->Item["OthrInsuredPartyID"]->Value;
		if (var.vt != VT_NULL)
			m_OthrInsuredPartyID = var.lVal;
		m_strBillName.Empty();
		var = rs->Fields->Item["Description"]->Value;
		if (var.vt != VT_NULL) {
			m_strBillName = _Q(CString(var.bstrVal));
		}
	}
	rs->Close();

	//load the UB92Info class
	m_UB92Info.Initialize();

	rs = CreateRecordset("SELECT UB92SetupT.ID FROM UB92SetupT INNER JOIN InsuranceCoT ON UB92SetupT.ID = InsuranceCoT.UB92SetupGroupID "
		"INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID WHERE InsuredPartyT.PersonID = %li",m_InsuredPartyID);

	if(!rs->eof) {
		_variant_t var = rs->Fields->Item["ID"]->Value;
		if(var.vt == VT_I4)
			m_UB92Info.LoadData(var.lVal);
	}
	rs->Close();

	SetUB92DateFormats(); // Set all the date boxes to 2 or 4 digit years through UB92 ID Setup information

	GetUB92GroupID();

	BuildBoxes8283();

	BuildUB92ChargesT();

	FindBatch();

	//m_pPrePrintFunc = &PreUB92Print;

	m_CapOnPrint = GetRemotePropertyInt("CapitalizeHCFA",0);
	((CButton*)GetDlgItem(IDC_CAP_ON_PRINT))->SetCheck(m_CapOnPrint);
	//m_CapOnPrint = TRUE;

	//create frame

	//allocate pointers
	// (j.jones 2010-03-15 15:19) - PLID 37719 - removed unnecessary log
	//LogDetail("UB92Dlg: Creating frame");
	m_pframe		= new CFormDisplayDlg(this);
	m_pframe->color = 0x000000FF;
	g_pFrame = m_pframe;
//	m_pleftbutton	= new CButton;
//	m_prightbutton	= new CButton;
//	m_pupbutton		= new CButton;
//	m_pdownbutton	= new CButton;

	RequeryHistoricalUB92Data();

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

	SetWindowText("UB-92");

	int i = 187;

	m_pframe->Load (9, "", "", &i); // Box82
	m_pframe->Load (10, "", "", &i); // Box83A

	//UB92Box79
	//fill with the default value from the UB92 Setup, but use the bill value if it exists
	CString strBox79 = m_UB92Info.Box79;

	// (j.jones 2006-05-30 12:15) - If Boxes 32 - 36 have data, then show the dates
	//also grab 79 while we are at it
	// (j.jones 2010-07-27 11:32) - PLID 39858 - get the assignment of benefits setting from the bill,
	// it's called HCFABox13Over, but it also applies to UB Box 53
	HCFABox13Over hb13Value = hb13_UseDefault;

	// (a.walling 2016-03-10 07:34) - PLID 68559 - UB04 Enhancements - update UB92 paper forms to load from UB04ClaimInfo object
	rs = CreateParamRecordset("SELECT "
				"  COALESCE(UB04ClaimInfo.value('(/claimInfo/occurrences/occurrence/@code)[1]', 'nvarchar(20)'), '') as UB92Box32 "
				", COALESCE(UB04ClaimInfo.value('(/claimInfo/occurrences/occurrence/@code)[2]', 'nvarchar(20)'), '') as UB92Box33 "
				", COALESCE(UB04ClaimInfo.value('(/claimInfo/occurrences/occurrence/@code)[3]', 'nvarchar(20)'), '') as UB92Box34 "
				", COALESCE(UB04ClaimInfo.value('(/claimInfo/occurrences/occurrence/@code)[4]', 'nvarchar(20)'), '') as UB92Box35 "
				", COALESCE(UB04ClaimInfo.value('(/claimInfo/occurrenceSpans/occurrenceSpan/@code)[1]', 'nvarchar(20)'), '') as UB92Box36 "
				", COALESCE(UB04ClaimInfo.value('(/claimInfo/occurrenceSpans/occurrenceSpan/@code)[2]', 'nvarchar(20)'), '') as UB04Box36 "
				", UB92Box79, HCFABox13Over "
		"FROM BillsT WHERE ID = {INT}", m_ID);
	if(!rs->eof) {
		hb13Value = (HCFABox13Over)AdoFldLong(rs, "HCFABox13Over", (long)hb13_UseDefault);

		CString str = AdoFldString(rs, "UB92Box79","");
		str.TrimRight();
		if(!str.IsEmpty())
			strBox79 = str;

		str = AdoFldString(rs, "UB92Box32","");
		str.TrimRight();
		if(!str.IsEmpty()) {
			ExecuteSql("UPDATE FormControlsT SET Source = 'FirstDate' WHERE ID = 913");
		}
		else {
			ExecuteSql("UPDATE FormControlsT SET Source = '' WHERE ID = 913");
		}

		str = AdoFldString(rs, "UB92Box33","");
		str.TrimRight();
		if(!str.IsEmpty()) {
			ExecuteSql("UPDATE FormControlsT SET Source = 'FirstDate' WHERE ID = 911");
		}
		else {
			ExecuteSql("UPDATE FormControlsT SET Source = '' WHERE ID = 911");
		}

		str = AdoFldString(rs, "UB92Box34","");
		str.TrimRight();
		if(!str.IsEmpty()) {
			ExecuteSql("UPDATE FormControlsT SET Source = 'FirstDate' WHERE ID = 909");
		}
		else {
			ExecuteSql("UPDATE FormControlsT SET Source = '' WHERE ID = 909");
		}

		str = AdoFldString(rs, "UB92Box35","");
		str.TrimRight();
		if(!str.IsEmpty()) {
			ExecuteSql("UPDATE FormControlsT SET Source = 'FirstDate' WHERE ID = 907");
		}
		else {
			ExecuteSql("UPDATE FormControlsT SET Source = '' WHERE ID = 907");
		}

		str = AdoFldString(rs, "UB92Box36","");
		str.TrimRight();
		if(!str.IsEmpty()) {
			ExecuteSql("UPDATE FormControlsT SET Source = 'FirstDate' WHERE ID = 905");
			ExecuteSql("UPDATE FormControlsT SET Source = 'LastDate' WHERE ID = 892");
		}
		else {
			ExecuteSql("UPDATE FormControlsT SET Source = '' WHERE ID = 905 OR ID = 892");
		}

	}
	rs->Close();

	//Load ShowInsAdd setting - to show the insurance name and address in Box84	
	if(m_UB92Info.ShowInsAdd == 1) {
		//show the address
		ExecuteSql("UPDATE FormControlsT SET Source = 'InsCoName' WHERE ID = 1309");
		ExecuteSql("UPDATE FormControlsT SET Source = 'InsCoAdd1' WHERE ID = 1316");
		ExecuteSql("UPDATE FormControlsT SET Source = 'InsCoAdd2' WHERE ID = 1317");
		ExecuteSql("UPDATE FormControlsT SET Source = 'InsCoCityStateZip' WHERE ID = 1318");
	}
	else
		ExecuteSql("UPDATE FormControlsT SET Source = '' WHERE ID = 1318 OR ID = 1317 OR ID = 1316 OR ID = 1309");

	if(m_UB92Info.Box76Show) {
		ExecuteSql("UPDATE FormControlsT SET Source = 'Diag1' WHERE ID = 1292");
	}
	else {
		ExecuteSql("UPDATE FormControlsT SET Source = '' WHERE ID = 1292");
	}

	// (j.jones 2011-07-06 16:21) - PLID 44358 - see if any charge has a Box 4 override,
	// and if so, use the first one
	CString strBox4 = m_UB92Info.Box4;
	_RecordsetPtr rsChargeBox4 = CreateParamRecordset("SELECT TOP 1 RTRIM(LTRIM(ServiceT.UBBox4)) AS Box4 "
		"FROM ChargesT "
		"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
		"INNER JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
		"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
		"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
		"WHERE ChargesT.BillID = {INT} AND LineItemT.Deleted = 0 "
		"AND ChargesT.Batched = 1 "
		"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
		"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
		"AND RTRIM(LTRIM(ServiceT.UBBox4)) <> '' "
		"ORDER BY ChargesT.LineID", m_ID);
	if(!rsChargeBox4->eof) {
		strBox4 = VarString(rsChargeBox4->Fields->Item["Box4"]->Value);
	}
	rsChargeBox4->Close();

	//set the default text fields
	g_aryFormQueries[11].sql.Format("SELECT PatientsT.PersonID, PatientsT.UserDefinedID AS ID, PersonT.[Last] + ', ' + PersonT.[First] + ' ' + PersonT.[Middle] AS Name, (CASE WHEN (PatientsT.[MaritalStatus]='1') THEN 'S' WHEN (PatientsT.[MaritalStatus]='2') THEN 'M' ELSE ' ' END) AS MS, PersonT.[Address1] + ' ' + PersonT.[Address2] + ' ' + PersonT.[City] + ' ' + PersonT.[State] + ' ' + PersonT.[Zip] AS Address, "
		"PersonT.[BirthDate] AS BirthDate, (CASE WHEN (PersonT.[Gender]=1) THEN 'M' WHEN (PersonT.[Gender]=2) THEN 'F' ELSE Null END) AS Sex, '%s' AS Box4, '%s' AS Box8, '%s' AS Box10, '%s' AS Box18, '%s' AS Box19, '%s' AS Box20, '%s' AS Box22, '%s' AS Box79 FROM PatientsT LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID",
		_Q(strBox4),_Q(m_UB92Info.Box8),_Q(m_UB92Info.Box10),_Q(m_UB92Info.Box18),_Q(m_UB92Info.Box19),_Q(m_UB92Info.Box20),_Q(m_UB92Info.Box22),_Q(strBox79));

	where.Format (" WHERE ID = %i", m_PatientID);
	m_pframe->Load (11, where, "", &i); // Patient information
	m_pframe->Load (12, "", "", &i); // Charges
	m_pframe->Load (13, "", "", &i); // Bill stuff

	//Load ShowPhone setting - to show the location phone number in Box1
	if(m_UB92Info.ShowPhone == 1)
		ExecuteSql("UPDATE FormControlsT SET Source = 'Phone' WHERE ID = 1325");
	else
		ExecuteSql("UPDATE FormControlsT SET Source = '' WHERE ID = 1325");
	
	if(m_UB92Info.Box1Loc == 0) {
		//bill location

		// (j.jones 2006-12-04 15:25) - PLID 23733 - supported the ClaimProviderID override
		// (j.jones 2010-11-10 08:55) - PLID 41387 - supported ChargesT.ClaimProviderID
		g_aryFormQueries[14].sql = "SELECT TOP 1 LocationsT.*, City + ', ' + State + ' ' + Zip AS CityStateZip, ProvidersT.[Fed Employer ID] "
			"FROM LocationsT INNER JOIN LineItemT ON LocationsT.ID = LineItemT.LocationID "
			"INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
			"LEFT JOIN ProvidersT ActualProvidersT ON ChargesT.DoctorsProviders = ActualProvidersT.PersonID "
			"LEFT JOIN ProvidersT ON (CASE WHEN ChargesT.ClaimProviderID Is Not Null THEN ChargesT.ClaimProviderID ELSE ActualProvidersT.ClaimProviderID END) = ProvidersT.PersonID ";
		where.Format (" WHERE ChargesT.BillID = %li", m_ID);
	}
	else {
		//place of service

		// (j.jones 2006-12-04 15:25) - PLID 23733 - supported the ClaimProviderID override
		// (j.jones 2010-11-10 08:55) - PLID 41387 - supported ChargesT.ClaimProviderID
		g_aryFormQueries[14].sql = "SELECT TOP 1 LocationsT.*, City + ', ' + LocationsT.State + ' ' + Zip AS CityStateZip, ProvidersT.[Fed Employer ID] "
			"FROM LocationsT INNER JOIN BillsT ON LocationsT.ID = BillsT.Location "
				"INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
				"LEFT JOIN ProvidersT ActualProvidersT ON ChargesT.DoctorsProviders = ActualProvidersT.PersonID "
				"LEFT JOIN ProvidersT ON (CASE WHEN ChargesT.ClaimProviderID Is Not Null THEN ChargesT.ClaimProviderID ELSE ActualProvidersT.ClaimProviderID END) = ProvidersT.PersonID";
		where.Format (" WHERE BillsT.ID = %li", m_ID);
	}

	if(m_UB92Info.Box5ID == 0) {
		//Box 5 - Location EIN
		ExecuteSql("UPDATE FormControlsT SET Source = 'EIN' WHERE ID = 835");
	}
	else {
		//Box 5 - Provider Tax ID
		ExecuteSql("UPDATE FormControlsT SET Source = '[Fed Employer ID]' WHERE ID = 835");
	}

	m_pframe->Load (14, where, "", &i); // Location data

	ExecuteSql("UPDATE FormControlsT SET Source = 'Zip' WHERE ID = 1250");

	//Load Box38 setting - 0 is Insured Party, 1 is Insurance Company
	if(m_UB92Info.Box38 == 0) {
		//Insured Party
		
		//Load ShowCompanyAsInsurer setting - 0 is the person name, 1 is the company name
		if(m_UB92Info.ShowCompanyAsInsurer == 1) {
			g_aryFormQueries[15].sql = "SELECT InsuredPartyT.Employer AS FullName, PersonT.[Address1] + ' ' + PersonT.[Address2] AS FullAddress, (PersonT.City + ', ' + PersonT.State + ' ' + PersonT.Zip) AS CityStateZip FROM (BillsT INNER JOIN InsuredPartyT ON BillsT.InsuredPartyID = InsuredPartyT.PersonID) INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID LEFT JOIN PersonT ON InsuredPartyT.PersonID = PersonT.ID";
		}
		else {
			g_aryFormQueries[15].sql = "SELECT (PersonT.[First] + (CASE WHEN (PersonT.Middle Is NULL OR PersonT.Middle = '') THEN '' ELSE (' ' + PersonT.Middle) END) + ' ' + PersonT.[Last]) AS FullName, PersonT.[Address1] + ' ' + PersonT.[Address2] AS FullAddress, (PersonT.City + ', ' + PersonT.State + ' ' + PersonT.Zip) AS CityStateZip FROM (BillsT INNER JOIN InsuredPartyT ON BillsT.InsuredPartyID = InsuredPartyT.PersonID) INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID LEFT JOIN PersonT ON InsuredPartyT.PersonID = PersonT.ID";
		}
	}
	else {
		//Insurance Company
		g_aryFormQueries[15].sql = "SELECT Name AS FullName, PersonT.[Address1] + ' ' + PersonT.[Address2] AS FullAddress, (PersonT.City + ', ' + PersonT.State + ' ' + PersonT.Zip) AS CityStateZip FROM (BillsT INNER JOIN InsuredPartyT ON BillsT.InsuredPartyID = InsuredPartyT.PersonID) INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID LEFT JOIN PersonT ON InsuranceCoT.PersonID = PersonT.ID";
	}
	where.Format (" WHERE BillsT.ID = %li", m_ID);

	m_pframe->Load (15, where, "", &i); // Insured party address

	m_pframe->Load (16, "", "", &i); // Provider data

	CString priaccepted = "Y";
	CString secaccepted = "Y";

	// (j.jones 2010-07-27 11:32) - PLID 39858 - use the assignment of benefits setting from the bill,
	// it's called HCFABox13Over, but it also applies to UB Box 53
	if(hb13Value == hb13_No) {
		priaccepted = "N";
		secaccepted = "N";
	}
	else if(hb13Value == hb13_UseDefault) {
		int Box53Accepted = m_UB92Info.Box53Accepted; //0 - check Accepted status, 1 - Always Yes, 2 - Always No
		if(Box53Accepted == 2) {
			priaccepted = "N";
			secaccepted = "N";
		}
		else if(Box53Accepted == 0) {

			// (j.jones 2010-07-23 15:28) - PLID 39783 - accept assignment is calculated in GetAcceptAssignment_ByInsuredParty now
			BOOL bPriAccepted = GetAcceptAssignment_ByInsuredParty(m_InsuredPartyID,adwProviderList.GetAt(0));
			if(bPriAccepted) {
				priaccepted = "Y";
			}
			else {
				priaccepted = "N";
			}

			BOOL bSecAccepted = GetAcceptAssignment_ByInsuredParty(m_OthrInsuredPartyID,adwProviderList.GetAt(0));
			if(bSecAccepted) {
				secaccepted = "Y";
			}
			else {
				secaccepted = "N";
			}
		}
	}

	//Load InsAddr50 into box 50
	if(m_UB92Info.InsAddr50 == 1) {
		ExecuteSql("UPDATE FormControlsT SET Source = 'InsAddrName' WHERE ID = 1000");
		ExecuteSql("UPDATE FormControlsT SET Source = 'OthrInsAddrName' WHERE ID = 1202");
	}
	else {
		ExecuteSql("UPDATE FormControlsT SET Source = 'InsCoName' WHERE ID = 1000");
		ExecuteSql("UPDATE FormControlsT SET Source = 'OthrInsCoName' WHERE ID = 1202");
	}

	//show/hide Box 64
	if(m_UB92Info.Box64Show == 1) {
		ExecuteSql("UPDATE FormControlsT SET Source = 'InsEmpCode' WHERE ID = 1277");
		ExecuteSql("UPDATE FormControlsT SET Source = 'OthrInsEmpCode' WHERE ID = 1278");
	}
	else {
		ExecuteSql("UPDATE FormControlsT SET Source = '' WHERE ID = 1277 OR ID = 1278");
	}

	CString strNameFormat;
	//Load ShowCompanyAsInsurer setting - 0 is the person name, 1 is the company name
	if(m_UB92Info.ShowCompanyAsInsurer == 1) {
		strNameFormat = "InsuredPartyT.Employer";
	}
	else {
		strNameFormat = "PersonT.[Last] + ',  ' + PersonT.[First] + ' ' + PersonT.[Middle]";
	}

	// (j.jones 2008-05-01 11:00) - PLID 28467 - ensured that any non-empty relation to patient that isn't
	// standard will show as 'other'
	g_aryFormQueries[17].sql.Format("SELECT InsuredPartyT.IDforInsurance AS InsID, %s AS InsName, "
		"PersonT.[Address1] AS InsAdd1, PersonT.[Address2] AS InsAdd2, PersonT.[Address1] + '  ' + PersonT.[Address2] AS InsAdd, PersonT.City AS InsCity, PersonT.State AS InsState, PersonT.Zip AS InsZip, "
		"(PersonT.City + ', ' + PersonT.State + ' ' + PersonT.Zip) AS CityStateZip, PersonT.WorkPhone AS InsPhone, InsuredPartyT.PolicyGroupNum AS InsFECA, PersonT.BirthDate AS InsBD, "
		"(CASE WHEN(InsuredPartyT.RelationToPatient='Self') THEN PersonT1.Company ELSE InsuredPartyT.Employer END) AS InsEmp, "
		"(CASE WHEN(InsuredPartyT.RelationToPatient='Self') "
		"THEN PatientsT.EmployerAddress1 + CASE WHEN PatientsT.EmployerAddress2 <> '' THEN ', ' + PatientsT.EmployerAddress2 ELSE '' END "
		"+ CASE WHEN PatientsT.EmployerAddress1 = '' AND PatientsT.EmployerAddress2 = '' THEN '' ELSE ', ' END "
		"+ PatientsT.EmployerCity + CASE WHEN PatientsT.EmployerState <> '' THEN ', ' ELSE '' END + PatientsT.EmployerState "
		"+ ' ' + PatientsT.EmployerZip "
		"ELSE '' END) AS InsEmpAddr, "
		"(CASE WHEN(InsuredPartyT.RelationToPatient='Self') "
		"THEN (CASE WHEN PatientsT.Employment = 1 THEN '1' WHEN PatientsT.Employment = '4' THEN '2' WHEN PatientsT.Employment = 5 THEN '5' ELSE '9' END) "
		"ELSE '9' END) AS InsEmpCode, "
		"InsurancePlansT.PlanName AS InsPlan, PersonT.Gender, PersonT.[Gender] AS InsGender, InsuranceCoT.Name AS InsCoName, InsuranceCoT.Name + ' ' + InsCoPersonT.Address1 + ' ' + InsCoPersonT.Address2 + ' ' + InsCoPersonT.City + ' ' + InsCoPersonT.State + ' ' + InsCoPersonT.Zip AS InsAddrName, "
		"InsCoPersonT.Address1 AS InsCoAdd1, InsCoPersonT.Address2 AS InsCoAdd2, InsCoPersonT.City AS InsCoCity, InsCoPersonT.State AS InsCoState, InsCoPersonT.Zip AS InsCoZip, (InsCoPersonT.City + ', ' + InsCoPersonT.State + ' ' + InsCoPersonT.Zip) AS InsCoCityStateZip, "
		"(CASE WHEN(InsuredPartyT.RelationToPatient='Self') THEN 1 WHEN (InsuredPartyT.RelationToPatient='Spouse') THEN 2 WHEN (InsuredPartyT.RelationToPatient='Child') THEN 3 WHEN (InsuredPartyT.RelationToPatient <> '') THEN 9 ELSE Null END) AS InsRel, "
		"InsurancePlansT.PlanType, (CASE WHEN ([InsurancePlansT].[PlanType]='Medicare') THEN 1 WHEN ([InsurancePlansT].[PlanType]='Medicaid') THEN 2 WHEN ([InsurancePlansT].[PlanType]='Champus') THEN 3 WHEN ([InsurancePlansT].[PlanType]='Champva') THEN 4 WHEN ([InsurancePlansT].[PlanType]='Group Health Plan') THEN 5 WHEN ([InsurancePlansT].[PlanType]='FECA Black Lung') THEN 6 WHEN ([InsurancePlansT].[PlanType]='Other') THEN 7 ELSE 0 END) AS InsType, "
		"'%s' AS Accepted, InsuredPartyT.PersonID AS ID, InsuredPartyT.PatientID, 'Y' AS Yes "
		"FROM (InsuranceCoT RIGHT JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID) "
		"LEFT JOIN InsurancePlansT ON InsuredPartyT.InsPlan = InsurancePlansT.ID "
		"LEFT JOIN PersonT ON InsuredPartyT.PersonID = PersonT.ID "
		"LEFT JOIN PersonT PersonT1 ON InsuredPartyT.PatientID = PersonT1.ID "
		"LEFT JOIN PatientsT ON PersonT1.ID = PatientsT.PersonID "
		"LEFT JOIN (SELECT * FROM PersonT) AS InsCoPersonT ON InsuranceCoT.PersonID = InsCoPersonT.ID ",strNameFormat,priaccepted);

	where.Format (" WHERE PatientID = %li AND InsuredPartyT.PersonID = %li", m_PatientID, m_InsuredPartyID);
	m_pframe->Load (17, where, "", &i); // Insurance information

	// (j.jones 2008-05-01 11:00) - PLID 28467 - ensured that any non-empty relation to patient that isn't
	// standard will show as 'other'
	g_aryFormQueries[18].sql.Format("SELECT InsuredPartyT.IDforInsurance AS OthrInsID, %s AS OthrInsName, "
		"PersonT.[Address1] + '  ' + PersonT.[Address2] AS OthrInsAdd, PersonT.City AS OthrInsCity, PersonT.State AS OthrInsState, PersonT.Zip AS OthrInsZip, PersonT.WorkPhone AS OthrInsPhone, "
		"InsuredPartyT.PolicyGroupNum AS OthrInsFECA, PersonT.BirthDate AS OthrInsBD, "
		"(CASE WHEN(InsuredPartyT.RelationToPatient='Self') THEN PersonT1.Company ELSE InsuredPartyT.Employer END) AS OthrInsEmp, "
		"(CASE WHEN(InsuredPartyT.RelationToPatient='Self') "
		"THEN PatientsT.EmployerAddress1 + CASE WHEN PatientsT.EmployerAddress2 <> '' THEN ', ' + PatientsT.EmployerAddress2 ELSE '' END "
		"+ CASE WHEN PatientsT.EmployerAddress1 = '' AND PatientsT.EmployerAddress2 = '' THEN '' ELSE ', ' END "
		"+ PatientsT.EmployerCity + CASE WHEN PatientsT.EmployerState <> '' THEN ', ' ELSE '' END + PatientsT.EmployerState "
		"+ ' ' + PatientsT.EmployerZip "
		"ELSE '' END) AS OthrInsEmpAddr, "
		"(CASE WHEN(InsuredPartyT.RelationToPatient='Self') "
		"THEN (CASE WHEN PatientsT.Employment = 1 THEN '1' WHEN PatientsT.Employment = '4' THEN '2' WHEN PatientsT.Employment = 5 THEN '5' ELSE '9' END) "
		"ELSE '9' END) AS OthrInsEmpCode, "
		"InsurancePlansT.PlanName AS OthrInsPlan, PersonT.Gender AS OthrGender, PersonT.[Gender] AS OthrInsGender, "
		"InsuranceCoT.Name AS OthrInsCoName, InsuranceCoT.Name + ' ' + InsCoPersonT.Address1 + ' ' + InsCoPersonT.Address2 + ' ' + InsCoPersonT.City + ' ' + InsCoPersonT.State + ' ' + InsCoPersonT.Zip AS OthrInsAddrName, "
		"InsCoPersonT.Address1 AS OthrInsCoAdd1, InsCoPersonT.Address2 AS OthrInsCoAdd2, InsCoPersonT.City AS OthrInsCoCity, InsCoPersonT.State AS OthrInsCoState, InsCoPersonT.Zip AS OthrInsCoZip, "
		"(CASE WHEN(InsuredPartyT.RelationToPatient='Self') THEN 1 WHEN (InsuredPartyT.RelationToPatient='Spouse') THEN 2 WHEN (InsuredPartyT.RelationToPatient='Child') THEN 3 WHEN (InsuredPartyT.RelationToPatient <> '') THEN 9 ELSE Null END) AS OthrInsRel, "
		"'%s' AS Accepted "
		"FROM InsuranceCoT RIGHT JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID "
		"LEFT JOIN InsurancePlansT ON InsuredPartyT.InsPlan = InsurancePlansT.ID "
		"LEFT JOIN PersonT ON InsuredPartyT.PersonID = PersonT.ID "
		"LEFT JOIN PersonT PersonT1 ON InsuredPartyT.PatientID = PersonT1.ID "
		"LEFT JOIN PatientsT ON PersonT1.ID = PatientsT.PersonID "
		"LEFT JOIN (SELECT * FROM PersonT) AS InsCoPersonT ON InsuranceCoT.PersonID = InsCoPersonT.ID ",strNameFormat,secaccepted);

	where.Format (" WHERE PatientID = %li AND InsuredPartyT.PersonID = %li", m_PatientID, m_OthrInsuredPartyID);
	m_pframe->Load (18, where, "", &i); // Other insurance information

	if(m_UB92Info.ShowApplies) {
		ExecuteSql("UPDATE FormControlsT SET Source = 'TotalPays' WHERE ID = 1210");
	}
	else {
		ExecuteSql("UPDATE FormControlsT SET Source = '' WHERE ID = 1210");
	}

	// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" charges & applies
	g_aryFormQueries[19].sql.Format("SELECT Sum(Round(Convert(money,CASE WHEN AppliesQ.Amount Is NULL THEN 0 ELSE AppliesQ.Amount END),2)) AS TotalPays "
					"FROM ChargesT "
					"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
					"LEFT JOIN (SELECT AppliesT.Amount, DestID FROM AppliesT "
					"	INNER JOIN LineItemT ON AppliesT.SourceID = LineItemT.ID "
					"	LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
					"	LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
					"	WHERE Deleted = 0 "
					"	AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
					"	AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null) AS AppliesQ ON ChargesT.ID = AppliesQ.DestID "
					"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
					"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
					"WHERE ChargesT.BillID = %li AND LineItemT.Deleted = 0 AND Batched = 1 "
					"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
					"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null",m_ID);

	m_pframe->Load (19, "", "", &i); // Insurance apply totals

	// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" applies
	g_aryFormQueries[30].sql = "SELECT LineItemT.PatientID AS [Patient ID], PaymentsT.InsuredPartyID, "
		"Sum(AppliesT.Amount) AS SumOfAmount "
		"FROM PaymentsT "
		"LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID "
		"LEFT JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
		"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
		"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
		"WHERE AppliesT.PointsToPayments <> 1 "
		"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
		"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
		"GROUP BY LineItemT.PatientID, PaymentsT.InsuredPartyID";
	where.Format (" HAVING (((PaymentsT.InsuredPartyID)<>-1)) AND (((LineItemT.PatientID)=%d) AND ((PaymentsT.InsuredPartyID)=%d))", m_PatientID, m_InsuredPartyID);

	m_pframe->Load (30, where, "", &i); // Other insurance apply totals

	if(m_UB92Info.ShowApplies) {
		//make Box 55 show the balance
		// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" charges & applies
		g_aryFormQueries[31].sql.Format("SELECT BillsT.ID, LineItemT.Deleted, "
			"dbo.GetClaimTotal(BillsT.ID) - (SELECT Sum(Round(Convert(money,CASE WHEN AppliesQ.Amount Is NULL THEN 0 ELSE AppliesQ.Amount END),2)) AS TotalPays "
			"FROM ChargesT "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"LEFT JOIN (SELECT AppliesT.Amount, DestID FROM AppliesT "
			"	INNER JOIN LineItemT ON AppliesT.SourceID = LineItemT.ID "
			"	LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
			"	LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
			"	WHERE Deleted = 0 "
			"	AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
			"	AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null) AS AppliesQ ON ChargesT.ID = AppliesQ.DestID "
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
			"WHERE ChargesT.BillID = %li AND LineItemT.Deleted = 0 AND Batched = 1 "
			"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
			"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null) AS SumOfCharges "
			"FROM LineItemT INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
			"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID",m_ID);

		where.Format (" WHERE BillsT.ID = %li AND LineItemT.Deleted = 0 AND Batched = 1 "
			"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
			"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null", m_ID);
	}
	else {
		//make Box 55 show the total charges
		// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" charges
		g_aryFormQueries[31].sql = "SELECT BillsT.ID, LineItemT.Deleted, dbo.GetClaimTotal(BillsT.ID) AS SumOfCharges "
					"FROM LineItemT INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
					"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
					"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID ";
		where.Format (" WHERE BillsT.ID = %li AND LineItemT.Deleted = 0 AND Batched = 1 "
			"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
			"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null ", m_ID);
	}

	m_pframe->Load (31, where, "", &i); // Charge totals

	//Load ShowTotals setting - to show the charge totals on line 23 of the charge listing	
	BOOL bSetDesc = FALSE, bSetAmount = FALSE;
	if(m_UB92Info.ShowTotals == 1) {

		//if ShowTotals is true, then we must show a grand total line on line 23 of the charge listing
		
		CString strTotal = "0.00";

		rs = CreateRecordset(g_aryFormQueries[31].sql + where);
		if(!rs->eof) {
			strTotal = FormatCurrencyForInterface(AdoFldCurrency(rs, "SumOfCharges",COleCurrency(0,0)), FALSE);
		}
		rs->Close();

		// (j.armen 2014-03-27 16:28) - PLID 60784 - Control array is now a vector
		for (unsigned int j = 0; j < m_pframe->m_ControlArray.size(); j++) {
			FormControl* pCtrl = m_pframe->m_ControlArray[j].get();
			if (pCtrl->id == 1024) {
				//set the description to say "Total"
				FormEdit* pEdit = (FormEdit*)pCtrl;
				pEdit->SetWindowText("Total");
				bSetDesc = TRUE;
			}
			if (pCtrl->id == 1112) {
				//set the amount to be the grand total
				FormEdit* pEdit = (FormEdit*)pCtrl;
				pEdit->SetWindowText(strTotal);
				bSetAmount = TRUE;
			}
			//break when we're done - no reason to stay in the loop
			if(bSetDesc && bSetAmount)
				break;
		}
	}	

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

	//Printer stuff
	PRINT_X_SCALE		= 15.3;//14.6
	PRINT_Y_SCALE		= 15.7;//14.6
	PRINT_X_OFFSET		= -350;//-400//-100
	PRINT_Y_OFFSET		= 470;//100

	m_pframe->m_pOnCommand = m_pOnCommand;
	m_pframe->m_pOnKeyDown = m_pOnKeyDown;

	// Set the focus to box 931; that is, the first revenue code
/*	for (int j=0; j < m_pframe->m_ControlArray.GetSize(); j++) {
		if ( ((FormControl*)m_pframe->m_ControlArray.GetAt(j))->id == 931)
			break;
	}
	if (j != m_pframe->m_ControlArray.GetSize()) {
		m_pframe->GetDlgItem(((FormControl*)m_pframe->m_ControlArray.GetAt(j))->nID)->SetFocus();
	}*/

	//m_pframe->m_pPrePrintFunc = m_pPrePrintFunc;

	m_pframe->TrackChanges(0, 0);

	return TRUE;
}

void CUB92Dlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	SetControlPositions();
}

void CUB92Dlg::SetControlPositions(void)
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

		//GetDlgItem(IDC_RADIO_PAPER)->MoveWindow(0,0,0,0, false);
		//GetDlgItem(IDC_RADIO_ELECTRONIC)->MoveWindow(0,0,0,0, false);
		//GetDlgItem(IDC_RADIO_NO_BATCH)->MoveWindow(0,0,0,0, false);

		Invalidate();
	}
}

void CUB92Dlg::OnClickX()
{
	EndDialog (0);
}

void CUB92Dlg::OnClickCheck() 
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

void CUB92Dlg::OnClose() 
{
	CDialog::OnClose();
}

BOOL CUB92Dlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	return CDialog::OnCommand(wParam, lParam);
}

extern int FONT_SIZE;
extern int MINI_FONT_SIZE;
void CUB92Dlg::OnClickPrint() 
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
		long nDefault = GetPropertyInt("DefaultFormAlignID", -1, 1, FALSE);
		CString strDefault = "[Default] = 1";
		if(nDefault != -1)
			strDefault.Format("ID = %li", nDefault);

		rs = CreateRecordset("SELECT * FROM FormAlignT WHERE %s AND FormID = 1", strDefault);
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
			if (var.vt == VT_NULL) var = (long)10;
			FONT_SIZE = var.lVal;

			// (j.jones 2005-01-31 15:06) - force it to be no greater than 12
			if(FONT_SIZE > 12)
				FONT_SIZE = 12;

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

		//specify whether or not to unpunctuate currencies
		CDWordArray aryIDsToIgnore;
		if(m_UB92Info.PunctuateChargeLines) {
			aryIDsToIgnore.Add(935);
			aryIDsToIgnore.Add(1091);
			aryIDsToIgnore.Add(1092);
			aryIDsToIgnore.Add(1093);
			aryIDsToIgnore.Add(1094);
			aryIDsToIgnore.Add(1095);
			aryIDsToIgnore.Add(1096);
			aryIDsToIgnore.Add(1097);
			aryIDsToIgnore.Add(1098);
			aryIDsToIgnore.Add(1099);
			aryIDsToIgnore.Add(1100);
			aryIDsToIgnore.Add(1101);
			aryIDsToIgnore.Add(1102);
			aryIDsToIgnore.Add(1103);
			aryIDsToIgnore.Add(1104);
			aryIDsToIgnore.Add(1105);
			aryIDsToIgnore.Add(1106);
			aryIDsToIgnore.Add(1107);
			aryIDsToIgnore.Add(1108);
			aryIDsToIgnore.Add(1109);
			aryIDsToIgnore.Add(1110);
			aryIDsToIgnore.Add(1111);
			aryIDsToIgnore.Add(1112);
			aryIDsToIgnore.Add(1210);
			aryIDsToIgnore.Add(1211);
			aryIDsToIgnore.Add(1212);
			aryIDsToIgnore.Add(1214);
			aryIDsToIgnore.Add(1229);
			aryIDsToIgnore.Add(1230);
			aryIDsToIgnore.Add(1251);
			aryIDsToIgnore.Add(1253);
		}
		if(m_UB92Info.PunctuateDiagCodes) {
			aryIDsToIgnore.Add(1283);
			aryIDsToIgnore.Add(1284);
			aryIDsToIgnore.Add(1285);
			aryIDsToIgnore.Add(1286);
			aryIDsToIgnore.Add(1287);
			aryIDsToIgnore.Add(1288);
			aryIDsToIgnore.Add(1289);
			aryIDsToIgnore.Add(1290);
			aryIDsToIgnore.Add(1291);
			aryIDsToIgnore.Add(1292);
			aryIDsToIgnore.Add(1293);
		}

		PRINT_X_OFFSET += x_offset * 10;
		PRINT_Y_OFFSET += y_offset * 10;
		PRINT_X_SCALE += x_scale * 0.01;
		PRINT_Y_SCALE += y_scale * 0.01;
		if(!m_pframe->OnPrint(m_CapOnPrint, "UB92", &aryIDsToIgnore, m_pPrintDC)) {
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

		//Send Type 2 - Paper UB92
		
		// (j.jones 2013-01-23 09:19) - PLID 54734 - moved the claim history addition to its own function,
		// this also updates HCFATrackT.LastSendDate
		AddToClaimHistory(m_ID, m_InsuredPartyID, ClaimSendType::UB);

		//now add to patient history
		CString str, strDesc = "UB92 Printed";
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
	}NxCatchAll("Error printing UB92 form.");
}

void CUB92Dlg::BuildFormsT_Form6()
{
	CString strSQL, str;

	try {

		// (j.jones 2006-12-04 15:25) - PLID 23733 - supported the ClaimProviderID override
		// (j.jones 2010-11-09 15:58) - PLID 41387 - supported ChargesT.ClaimProviderID
		strSQL.Format("FormChargesT.DoctorsProviders AS FirstOfDoctorsProviders, DoctorInfo.[Last] AS DocLastName, DoctorInfo.[First] AS DocFirstName, DoctorInfo.[Middle] AS DocMiddleName, GetDate() AS TodaysDate, "
				"DoctorInfo.[Title] + ' ' + DoctorInfo.[First] + ' ' + DoctorInfo.[Middle] + ' ' + DoctorInfo.[Last] AS DocSignature, DoctorInfo.SocialSecurity, DoctorInfo.[Fed Employer ID] FROM FormChargesT "
				"INNER JOIN (SELECT * FROM PersonT INNER JOIN (SELECT ProvidersT.*, ActualProvidersT.PersonID AS RealProviderID FROM ProvidersT "
				"	INNER JOIN ProvidersT ActualProvidersT ON ProvidersT.PersonID = ActualProvidersT.ClaimProviderID) "
				"	AS ProvidersT ON PersonT.ID = ProvidersT.PersonID) AS DoctorInfo ON (CASE WHEN ChargesT.ClaimProviderID Is Not Null THEN ChargesT.ClaimProviderID ELSE FormChargesT.DoctorsProviders END) = DoctorInfo.RealProviderID");
		str.Format("FROM (%s) AS FormChargesT",g_aryFormQueries[2].sql);
		strSQL.Replace("FROM FormChargesT",str);
		g_aryFormQueries[6].sql = strSQL;
	} NxCatchAll("UB92Dlg::BuildFormsT_Form6()");
}

void CUB92Dlg::BuildUB92ChargesT()
{
	extern int SizeOfFormChargesT();

	try {
		//ExecuteSql("DELETE FROM FormChargesT");

		_RecordsetPtr rs;
		CString str;

		billid = m_ID;

		// Fill temporary HCFA charges table with all the charges
		// for this bill		
		//ExecuteSql("DELETE FROM FormChargesT");
		// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" charges
		// (a.walling 2014-03-13 09:06) - PLID 61305 - Emulate the old WhichCodes field via ChargeWhichCodesFlatV
		// (a.walling 2014-03-17 17:20) - PLID 61405 - Unique diag codes and recalculated WhichCodes using BillDiagsXmlFlat4IF and ChargeWhichCodesIF - UB04, UB92
		g_aryFormQueries[2].sql.Format("SELECT ChargesT.*, ChargeWhichCodesQ.WhichCodes, LineItemT.[Date] AS TD3ate, LineItemT.Description AS ItemDesc, LineItemT.PatientID FROM BillsT INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
			"OUTER APPLY dbo.%s(BillsT.ID) BillDiagCodeFlatV "
			"OUTER APPLY dbo.%s(BillDiagCodeFlatV.BillDiagsXml, ChargesT.ID) ChargeWhichCodesQ "
			"WHERE LineItemT.Deleted = 0 AND Batched = 1 AND BillsT.ID = %li "
			"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
			"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null"
			, ShouldUseICD10(m_InsuredPartyID, m_ID) ? "BillICD10DiagsXmlFlat4IF" : "BillICD9DiagsXmlFlat4IF"
			, ShouldUseICD10(m_InsuredPartyID, m_ID) ? "ChargeWhichICD10CodesIF" : "ChargeWhichICD9CodesIF"
			, billid
		);

		// Set all the NULL Doctors/Providers to -1 in the HCFA charges table
		//ExecuteSql("UPDATE FormChargesT SET DoctorsProviders = -1 WHERE DoctorsProviders Is Null");

		// Build form 6 in the form controls table
		BuildFormsT_Form6();

		// Change the controls in the HCFA form so that the charge lines go to the first 6
		// charges in the HCFA list
		firstcharge = 0;
		ExecuteSql("UPDATE FormControlsT SET FormControlsT.[Value] = 0 WHERE (((FormControlsT.ID)>=931) AND ((FormControlsT.ID)<=940))");
		// These are done vertically
		ExecuteSql("UPDATE FormControlsT SET FormID = 12, Source = 'RevCode', Format = 520 WHERE FormControlsT.ID = 931");
		ExecuteSql("UPDATE FormControlsT SET FormID = 12, Source = 'Amount' WHERE FormControlsT.ID = 935");
		ExecuteSql("UPDATE FormControlsT SET FormID = 12, Source = 'RevCode', Format = 520, FormControlsT.[Value] = FormControlsT.ID-977 WHERE (((FormControlsT.ID)>=978) AND ((FormControlsT.ID)<=999))");
		ExecuteSql("UPDATE FormControlsT SET FormID = 12, Source = 'ItemDesc', FormControlsT.[Value] = FormControlsT.ID-1000 WHERE (((FormControlsT.ID)>=1001) AND ((FormControlsT.ID)<=1024))");
		ExecuteSql("UPDATE FormControlsT SET FormID = 12, Source = 'ItemCode', FormControlsT.[Value] = FormControlsT.ID-1024 WHERE (((FormControlsT.ID)>=1025) AND ((FormControlsT.ID)<=1046))");
		ExecuteSql("UPDATE FormControlsT SET FormID = 12, Source = 'TDate', FormControlsT.[Value] = FormControlsT.ID-1046 WHERE (((FormControlsT.ID)>=1047) AND ((FormControlsT.ID)<=1068))");
		ExecuteSql("UPDATE FormControlsT SET FormID = 12, Source = 'Quantity', FormControlsT.[Value] = FormControlsT.ID-1068 WHERE (((FormControlsT.ID)>=1069) AND ((FormControlsT.ID)<=1090))");
		ExecuteSql("UPDATE FormControlsT SET FormID = 12, Source = 'Amount', FormControlsT.[Value] = FormControlsT.ID-1090 WHERE (((FormControlsT.ID)>=1091) AND ((FormControlsT.ID)<=1112))");
		ExecuteSql("UPDATE FormControlsT SET FormID = 12, Source = ' ', FormControlsT.[Value] = FormControlsT.ID-1112 WHERE (((FormControlsT.ID)>=1113) AND ((FormControlsT.ID)<=1134))");
		ExecuteSql("UPDATE FormControlsT SET FormID = 12, Source = ' ', FormControlsT.[Value] = FormControlsT.ID-1134 WHERE (((FormControlsT.ID)>=1135) AND ((FormControlsT.ID)<=1156))");
		ExecuteSql("UPDATE FormControlsT SET FormID = 12, Source = ' ', FormControlsT.[Value] = FormControlsT.ID-1156 WHERE (((FormControlsT.ID)>=1157) AND ((FormControlsT.ID)<=1178))");
		ExecuteSql("UPDATE FormControlsT SET FormID = 12, Source = ' ', FormControlsT.[Value] = FormControlsT.ID-1178 WHERE (((FormControlsT.ID)>=1179) AND ((FormControlsT.ID)<=1200))");

		// Now build a list of different charge providers. Assume every charge
		// has a provider in FormChargesT.

		adwProviderList.RemoveAll();
		// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" charges
		rs = CreateParamRecordset("SELECT DoctorsProviders FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
			"WHERE ChargesT.BillID = {INT} AND LineItemT.Deleted = 0 AND Batched = 1 "
			"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
			"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null",billid);
		while (!rs->eof) {
			int i, iProviderID = rs->Fields->Item["DoctorsProviders"]->Value.lVal;
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

		if(m_UB92Info.Box80Number == 0) {
			//show CPT code
			ExecuteSql("UPDATE FormControlsT SET Source = 'TopCPT' WHERE ID = 1295");
			ExecuteSql("UPDATE FormControlsT SET Source = 'FirstDate' WHERE ID = 1297");
		}
		else if(m_UB92Info.Box80Number == 1) {
			//show ICD-9 code
			ExecuteSql("UPDATE FormControlsT SET Source = 'Diag1' WHERE ID = 1295");
			ExecuteSql("UPDATE FormControlsT SET Source = 'FirstDate' WHERE ID = 1297");
		}
		else {
			//show nothing
			ExecuteSql("UPDATE FormControlsT SET Source = '' WHERE ID = 1295");
			ExecuteSql("UPDATE FormControlsT SET Source = '' WHERE ID = 1297");
		}

		//reinitialize
		// (j.jones 2010-03-12 16:58) - PLID 37440 - changed FirstDate/LastDate to use the min/max ServiceDateFrom/ServiceDateTo
		// (a.walling 2014-03-06 13:33) - PLID 61229 - Support BillDiagCodeT via BillDiagCodeICD9FlatV/ICD10FlatV aliased as BillDiagCodeFlatV
		// (a.walling 2014-03-17 17:20) - PLID 61405 - Unique diag codes and recalculated WhichCodes using BillDiagsXmlFlat4IF and ChargeWhichCodesIF - UB04, UB92
		// (a.walling 2016-03-10 07:34) - PLID 68559 - UB04 Enhancements - update UB92 paper forms to load from UB04ClaimInfo object
		g_aryFormQueries[13].sql.Format("SELECT Min(TopCPT.ItemCode) AS TopCPT, Min([FormChargesT].[ServiceDateFrom]) AS FirstDate, Max([FormChargesT].[ServiceDateTo]) AS LastDate, "
			"Min(DiagCodes1.CodeNumber) AS Diag1, Min(DiagCodes2.CodeNumber) AS Diag2, Min(DiagCodes3.CodeNumber) AS Diag3, Min(DiagCodes4.CodeNumber) AS Diag4, "
			"  MIN(COALESCE(UB04ClaimInfo.value('(/claimInfo/occurrences/occurrence/@code)[1]', 'nvarchar(20)'), '')) as UB92Box32 "
			", MIN(COALESCE(UB04ClaimInfo.value('(/claimInfo/occurrences/occurrence/@code)[2]', 'nvarchar(20)'), '')) as UB92Box33 "
			", MIN(COALESCE(UB04ClaimInfo.value('(/claimInfo/occurrences/occurrence/@code)[3]', 'nvarchar(20)'), '')) as UB92Box34 "
			", MIN(COALESCE(UB04ClaimInfo.value('(/claimInfo/occurrences/occurrence/@code)[4]', 'nvarchar(20)'), '')) as UB92Box35 "
			", MIN(COALESCE(UB04ClaimInfo.value('(/claimInfo/occurrenceSpans/occurrenceSpan/@code)[1]', 'nvarchar(20)'), '')) as UB92Box36, "
			"Min([FormChargesT].[BillID]) AS BillID, Min([Date]) AS BillDate, GetDate() AS TodaysDate, CASE WHEN Max([RefPhyID]) > 0 THEN Max([RefPhyID]) ELSE NULL END AS ReferringID, Max(PersonT.[Last] + ', ' + PersonT.[First] + ' ' + PersonT.Middle + (CASE WHEN (PersonT.Title <> '') THEN (', ' + PersonT.Title) ELSE ('') END)) AS ReferringName , Max([PriorAuthNum]) AS PriorAuth, 'Signature On File' AS SigOnFile FROM FormChargesT INNER JOIN BillsT ON FormChargesT.BillID = BillsT.ID "
			"OUTER APPLY dbo.%s(BillsT.ID) BillDiagCodeFlatV "
			"LEFT JOIN DiagCodes DiagCodes1 ON BillDiagCodeFlatV.Diag1ID = DiagCodes1.ID LEFT JOIN DiagCodes DiagCodes2 ON BillDiagCodeFlatV.Diag2ID = DiagCodes2.ID LEFT JOIN DiagCodes DiagCodes3 ON BillDiagCodeFlatV.Diag3ID = DiagCodes3.ID LEFT JOIN DiagCodes DiagCodes4 ON BillDiagCodeFlatV.Diag4ID = DiagCodes4.ID LEFT JOIN PersonT ON BillsT.RefPhyID = PersonT.ID LEFT JOIN (SELECT BillID, ItemCode FROM ChargesT WHERE LineID = 1) AS TopCPT ON BillsT.ID = TopCPT.BillID"
			, ShouldUseICD10(m_InsuredPartyID, m_ID) ? "BillICD10DiagsXmlFlat4IF" : "BillICD9DiagsXmlFlat4IF"
		);

		//show "Signature On File" in Box85		
		if(m_UB92Info.Box85Show == 0)
			//do NOT show anything in Box85
			g_aryFormQueries[13].sql.Replace("Signature On File","");
		
		//load Box 51 number
		CString strBox51A = "", strBox51B = "", strBox51C = "";

		//primary insurance (on the bill) goes to strBox51A

		// (j.jones 2006-12-04 15:25) - PLID 23733 - supported the ClaimProviderID override
		// (j.jones 2010-11-09 15:44) - PLID 41387 - supported ChargesT.ClaimProviderID
		rs = CreateParamRecordset("SELECT InsuranceBox51.Box51Info AS Box51 FROM (ChargesT INNER JOIN (((InsuranceCoT INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID) INNER JOIN InsuranceBox51 ON InsuranceCoT.PersonID = InsuranceBox51.InsCoID) INNER JOIN BillsT ON (InsuredPartyT.PersonID = BillsT.InsuredPartyID) AND (InsuredPartyT.PatientID = BillsT.PatientID)) ON ChargesT.BillID = BillsT.ID) "
			"INNER JOIN ProvidersT ActualProvidersT ON ChargesT.DoctorsProviders = ActualProvidersT.PersonID "
			"INNER JOIN ProvidersT ON (CASE WHEN ChargesT.ClaimProviderID Is Not Null THEN ChargesT.ClaimProviderID ELSE ActualProvidersT.ClaimProviderID END) = ProvidersT.PersonID "
			"	AND (InsuranceBox51.ProviderID = ProvidersT.PersonID) "
			"GROUP BY InsuranceBox51.Box51Info, BillsT.ID HAVING (((BillsT.ID)={INT}))", m_ID);
		if (!rs->eof) {
			strBox51A = AdoFldString(rs, "Box51","");
		}
		rs->Close();

		if(strBox51A == "") {
			//if blank, then try to load the default Box 51 from the UB92 group
			strBox51A = m_UB92Info.Box51Default;
		}

		//secondary insurance (on the bill) goes to strBox51B

		// (j.jones 2006-12-04 15:25) - PLID 23733 - supported the ClaimProviderID override
		// (j.jones 2010-11-10 09:03) - PLID 41387 - supported ChargesT.ClaimProviderID
		rs = CreateParamRecordset("SELECT InsuranceBox51.Box51Info AS Box51 FROM (ChargesT INNER JOIN (((InsuranceCoT INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID) INNER JOIN InsuranceBox51 ON InsuranceCoT.PersonID = InsuranceBox51.InsCoID) INNER JOIN BillsT ON (InsuredPartyT.PersonID = BillsT.OthrInsuredPartyID) AND (InsuredPartyT.PatientID = BillsT.PatientID)) ON ChargesT.BillID = BillsT.ID) "
			"INNER JOIN ProvidersT ActualProvidersT ON ChargesT.DoctorsProviders = ActualProvidersT.PersonID "
			"INNER JOIN ProvidersT ON (CASE WHEN ChargesT.ClaimProviderID Is Not Null THEN ChargesT.ClaimProviderID ELSE ActualProvidersT.ClaimProviderID END) = ProvidersT.PersonID "
			"	AND (InsuranceBox51.ProviderID = ProvidersT.PersonID) "
			"GROUP BY InsuranceBox51.Box51Info, BillsT.ID HAVING (((BillsT.ID)={INT}))", m_ID);
		if (!rs->eof) {
			strBox51B = AdoFldString(rs, "Box51","");
		}
		rs->Close();

		if(strBox51B == "") {
			//if blank, then try to load the default Box 51 from the UB92 group
			rs = CreateRecordset("SELECT UB92SetupT.Box51Default AS Box51 FROM (ChargesT INNER JOIN (((InsuranceCoT INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID) INNER JOIN UB92SetupT ON InsuranceCoT.UB92SetupGroupID = UB92SetupT.ID) INNER JOIN BillsT ON (InsuredPartyT.PersonID = BillsT.OthrInsuredPartyID) AND (InsuredPartyT.PatientID = BillsT.PatientID)) ON ChargesT.BillID = BillsT.ID) "
							"GROUP BY Box51Default, BillsT.ID HAVING (((BillsT.ID)=%li))", m_ID);
			if (!rs->eof) {
				strBox51B = AdoFldString(rs, "Box51","");
			}
			rs->Close();
		}

		// (j.jones 2006-12-04 15:25) - PLID 23733 - supported the ClaimProviderID override
		// (j.jones 2010-11-10 09:06) - PLID 41387 - supported ChargesT.ClaimProviderID
		g_aryFormQueries[16].sql.Format("SELECT FormChargesT.DoctorsProviders AS FirstOfDoctorsProviders, "
			"PersonT.[Last] AS DocLastName, PersonT.[First] AS DocFirstName, PersonT.[Middle] AS DocMiddleName, GetDate() AS TodaysDate, "
			"PersonT.[First] + ' ' + PersonT.[Middle] + ' ' + PersonT.[Last] + (CASE WHEN (PersonT.Title <> '') THEN (', ' + PersonT.Title) ELSE ('') END) AS DocSignature, "
			"PersonT.Address1 AS DocAdd1, PersonT.Address2 AS DocAdd2, PersonT.City + ', ' + PersonT.State + ' ' + PersonT.Zip AS DocAdd3, "
			"PersonT.SocialSecurity, ProvidersT.[Fed Employer ID], ProvidersT.NPI, ProvidersT.[Medicare Number], '%s' AS Box51A, '%s' AS Box51B, '%s' AS Box51C "
			"FROM PersonT INNER JOIN (FormChargesT INNER JOIN ProvidersT ActualProvidersT ON FormChargesT.DoctorsProviders = ActualProvidersT.PersonID "
			"	INNER JOIN ProvidersT ON (CASE WHEN FormChargesT.ClaimProviderID Is Not Null THEN FormChargesT.ClaimProviderID ELSE ActualProvidersT.ClaimProviderID END) = ProvidersT.PersonID) "
			"ON PersonT.ID = ProvidersT.PersonID",_Q(strBox51A),_Q(strBox51B),_Q(strBox51C));
		
		g_aryFormQueries[31].sql = "SELECT * FROM FormChargesT";

		str.Format("FROM (%s) AS FormChargesT",g_aryFormQueries[2].sql);

		//Group revenue Codes together?
		if(m_UB92Info.GroupRev == 0) {
			//do not group by revenue code
			//this method will show the revenue code, cpt code, and line item description
			//(If RevCodeUse is 2, the revenue code is based on the Insurance Company.)
			//this method also sorts in the same way the bill does

			// (j.jones 2007-03-27 11:24) - PLID 25304 - these queries did not handle the case where
			// neither Revenue Code option was enabled for the CPT code. Now they do.
			
			if(m_UB92Info.Fill42With44) {
				// (a.vengrofski 2010-02-03 15:08) - PLID <35133> - Added two CASEs to the query to fill in the Modifier 3 & 4
				//if no RevCode, use the CPT code
				// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" charges
				g_aryFormQueries[12].sql.Format("SELECT COALESCE((CASE WHEN ServiceT.RevCodeUse = 1 THEN UB92CategoriesT1.Code WHEN ServiceT.RevCodeUse = 2 THEN UB92CategoriesT2.Code ELSE NULL END), "
						"(ItemCode + (CASE WHEN CPTModifier Is Null THEN '' ELSE (' ' + CPTModifier) END) + (CASE WHEN CPTModifier2 Is Null THEN '' ELSE (' ' + CPTModifier2) END)+ (CASE WHEN CPTModifier3 Is Null THEN '' ELSE (' ' + CPTModifier3) END) + (CASE WHEN CPTModifier4 Is Null THEN '' ELSE (' ' + CPTModifier4) END))) AS RevCode, "
						"(ItemCode + (CASE WHEN CPTModifier Is Null THEN '' ELSE (' ' + CPTModifier) END) + (CASE WHEN CPTModifier2 Is Null THEN '' ELSE (' ' + CPTModifier2) END)+ (CASE WHEN CPTModifier3 Is Null THEN '' ELSE (' ' + CPTModifier3) END) + (CASE WHEN CPTModifier4 Is Null THEN '' ELSE (' ' + CPTModifier4) END)) AS ItemCode, LineItemT.Description AS ItemDesc, "
						"LineItemT.[Date] AS TDate, Quantity, dbo.GetChargeTotal(ChargesT.ID) AS Amount "
						"FROM BillsT "
						"INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
						"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
						"LEFT JOIN CPTModifierT ON CPTModifier = CPTModifierT.Number LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number "
						"LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
						"LEFT JOIN UB92CategoriesT UB92CategoriesT1 ON ServiceT.UB92Category = UB92CategoriesT1.ID "
						"LEFT JOIN (SELECT ServiceRevCodesT.*, PersonID AS InsuredPartyID FROM ServiceRevCodesT INNER JOIN InsuredPartyT ON ServiceRevCodesT.InsuranceCompanyID = InsuredPartyT.InsuranceCoID) AS ServiceRevCodesT ON BillsT.InsuredPartyID = ServiceRevCodesT.InsuredPartyID AND ServiceT.ID = ServiceRevCodesT.ServiceID "
						"LEFT JOIN UB92CategoriesT UB92CategoriesT2 ON ServiceRevCodesT.UB92CategoryID = UB92CategoriesT2.ID "
						"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
						"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
						"WHERE LineItemT.Deleted = 0 AND Batched = 1 AND BillsT.ID = %li "
						"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
						"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
						"ORDER BY ChargesT.LineID",billid);
			}
			else {
				// (a.vengrofski 2010-02-03 15:11) - PLID <35133> - Added two CASEs to the query to fill in the Modifier 3 & 4
				// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" charges
				g_aryFormQueries[12].sql.Format("SELECT (CASE WHEN ServiceT.RevCodeUse = 1 THEN UB92CategoriesT1.Code WHEN ServiceT.RevCodeUse = 2 THEN UB92CategoriesT2.Code ELSE NULL END) AS RevCode, "
						"(ItemCode + (CASE WHEN CPTModifier Is Null THEN '' ELSE (' ' + CPTModifier) END) + (CASE WHEN CPTModifier2 Is Null THEN '' ELSE (' ' + CPTModifier2) END)+ (CASE WHEN CPTModifier3 Is Null THEN '' ELSE (' ' + CPTModifier3) END) + (CASE WHEN CPTModifier4 Is Null THEN '' ELSE (' ' + CPTModifier4) END)) AS ItemCode, LineItemT.Description AS ItemDesc, "
						"LineItemT.[Date] AS TDate, Quantity, dbo.GetChargeTotal(ChargesT.ID) AS Amount "
						"FROM BillsT "
						"INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
						"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
						"LEFT JOIN CPTModifierT ON CPTModifier = CPTModifierT.Number LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number "
						"LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
						"LEFT JOIN UB92CategoriesT UB92CategoriesT1 ON ServiceT.UB92Category = UB92CategoriesT1.ID "
						"LEFT JOIN (SELECT ServiceRevCodesT.*, PersonID AS InsuredPartyID FROM ServiceRevCodesT INNER JOIN InsuredPartyT ON ServiceRevCodesT.InsuranceCompanyID = InsuredPartyT.InsuranceCoID) AS ServiceRevCodesT ON BillsT.InsuredPartyID = ServiceRevCodesT.InsuredPartyID AND ServiceT.ID = ServiceRevCodesT.ServiceID "
						"LEFT JOIN UB92CategoriesT UB92CategoriesT2 ON ServiceRevCodesT.UB92CategoryID = UB92CategoriesT2.ID "
						"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
						"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
						"WHERE LineItemT.Deleted = 0 AND Batched = 1 AND BillsT.ID = %li "
						"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
						"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
						"ORDER BY ChargesT.LineID",billid);
			}

		}
		else {
			//Group by revenue code
			//This method (more common) will show based on the selection for the given item:
			//If RevCodeUse is 1, then all charges with that revenue code will group and total together,
			//and show the revenue code name. (If RevCodeUse is 2, the revenue code is based on the Insurance Company.)
			//If RevCodeUse is 0, then the charge will show on its own with the CPT code

			// (j.jones 2006-11-07 10:00) - PLID 22534 - BillsT.UB92Box44 is used in any case where Box 44 is not filled
			CString strBox44Override = "";
			_RecordsetPtr rsCode = CreateRecordset("SELECT Code FROM BillsT INNER JOIN CPTCodeT ON BillsT.UB92Box44 = CPTCodeT.ID WHERE BillsT.ID = %li", billid);
			if(!rsCode->eof) {
				strBox44Override = AdoFldString(rsCode, "Code","");
			}
			rsCode->Close();

			// (j.jones 2007-07-13 09:04) - PLID 26662 - non-revcode charges, or expanded revcode charges,
			// need to continue ordering by their LineID per the bill's ordering (which is usually
			// by amount descending, but not always)

			if(m_UB92Info.Fill42With44) {
				//if no RevCode, use the CPT code
				// (a.vengrofski 2010-02-03 15:12) - PLID <35133> - Added two CASEs to the query to fill in the Modifier 3 & 4
				// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" charges
				g_aryFormQueries[12].sql.Format("SELECT COALESCE((CASE WHEN ServiceT.RevCodeUse = 1 THEN UB92CategoriesT1.Code WHEN ServiceT.RevCodeUse = 2 THEN UB92CategoriesT2.Code ELSE NULL END), "
						"(CASE WHEN ServiceT.RevCodeUse = 0 THEN (ItemCode + (CASE WHEN CPTModifier Is Null THEN '' ELSE (' ' + CPTModifier) END) + (CASE WHEN CPTModifier2 Is Null THEN '' ELSE (' ' + CPTModifier2) END)+ (CASE WHEN CPTModifier3 Is Null THEN '' ELSE (' ' + CPTModifier3) END) + (CASE WHEN CPTModifier4 Is Null THEN '' ELSE (' ' + CPTModifier4) END)) ELSE NULL END)) AS RevCode, "
						""
						"(CASE WHEN ServiceT.RevCodeUse = 0 THEN (ItemCode + (CASE WHEN CPTModifier Is Null THEN '' ELSE (' ' + CPTModifier) END) + (CASE WHEN CPTModifier2 Is Null THEN '' ELSE (' ' + CPTModifier2) END)+ (CASE WHEN CPTModifier3 Is Null THEN '' ELSE (' ' + CPTModifier3) END) + (CASE WHEN CPTModifier4 Is Null THEN '' ELSE (' ' + CPTModifier4) END)) ELSE '%s' END) AS ItemCode, "
						"COALESCE((CASE WHEN ServiceT.RevCodeUse = 1 THEN UB92CategoriesT1.Name WHEN ServiceT.RevCodeUse = 2 THEN UB92CategoriesT2.Name ELSE LineItemT.Description END),LineItemT.Description) AS ItemDesc, "
						"Min(LineItemT.[Date]) AS TDate, Sum(ChargesT.Quantity) AS Quantity, Sum(dbo.GetChargeTotal(ChargesT.ID)) AS Amount "
						"FROM BillsT "
						"INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
						"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
						"LEFT JOIN CPTModifierT ON CPTModifier = CPTModifierT.Number LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number "
						"LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
						"LEFT JOIN UB92CategoriesT UB92CategoriesT1 ON ServiceT.UB92Category = UB92CategoriesT1.ID "
						"LEFT JOIN (SELECT ServiceRevCodesT.*, PersonID AS InsuredPartyID FROM ServiceRevCodesT INNER JOIN InsuredPartyT ON ServiceRevCodesT.InsuranceCompanyID = InsuredPartyT.InsuranceCoID) AS ServiceRevCodesT ON BillsT.InsuredPartyID = ServiceRevCodesT.InsuredPartyID AND ServiceT.ID = ServiceRevCodesT.ServiceID "
						"LEFT JOIN UB92CategoriesT UB92CategoriesT2 ON ServiceRevCodesT.UB92CategoryID = UB92CategoriesT2.ID "
						"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
						"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
						"WHERE LineItemT.Deleted = 0 AND Batched = 1 AND BillsT.ID = %li "
						"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
						"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
						"GROUP BY "
						"COALESCE((CASE WHEN ServiceT.RevCodeUse = 1 THEN UB92CategoriesT1.Code WHEN ServiceT.RevCodeUse = 2 THEN UB92CategoriesT2.Code ELSE NULL END), "
						"(CASE WHEN ServiceT.RevCodeUse = 0 THEN (ItemCode + (CASE WHEN CPTModifier Is Null THEN '' ELSE (' ' + CPTModifier) END) + (CASE WHEN CPTModifier2 Is Null THEN '' ELSE (' ' + CPTModifier2) END)+ (CASE WHEN CPTModifier3 Is Null THEN '' ELSE (' ' + CPTModifier3) END) + (CASE WHEN CPTModifier4 Is Null THEN '' ELSE (' ' + CPTModifier4) END)) ELSE NULL END)), "
						""
						"(CASE WHEN ServiceT.RevCodeUse = 0 THEN (ItemCode + (CASE WHEN CPTModifier Is Null THEN '' ELSE (' ' + CPTModifier) END) + (CASE WHEN CPTModifier2 Is Null THEN '' ELSE (' ' + CPTModifier2) END)+ (CASE WHEN CPTModifier3 Is Null THEN '' ELSE (' ' + CPTModifier3) END) + (CASE WHEN CPTModifier4 Is Null THEN '' ELSE (' ' + CPTModifier4) END)) ELSE '%s' END), "
						"COALESCE((CASE WHEN ServiceT.RevCodeUse = 1 THEN UB92CategoriesT1.Name WHEN ServiceT.RevCodeUse = 2 THEN UB92CategoriesT2.Name ELSE LineItemT.Description END),LineItemT.Description) "
						"ORDER BY RevCode, Min(ChargesT.LineID)", _Q(strBox44Override), billid, _Q(strBox44Override));

			}
			else {
				// (a.vengrofski 2010-02-03 15:12) - PLID <35133> - Added two CASEs to the query to fill in the Modifier 3 & 4
				// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" charges
				g_aryFormQueries[12].sql.Format("SELECT (CASE WHEN ServiceT.RevCodeUse = 1 THEN UB92CategoriesT1.Code WHEN ServiceT.RevCodeUse = 2 THEN UB92CategoriesT2.Code ELSE NULL END) AS RevCode, "
						"(CASE WHEN ServiceT.RevCodeUse = 0 THEN (ItemCode + (CASE WHEN CPTModifier Is Null THEN '' ELSE (' ' + CPTModifier) END) + (CASE WHEN CPTModifier2 Is Null THEN '' ELSE (' ' + CPTModifier2) END)+ (CASE WHEN CPTModifier3 Is Null THEN '' ELSE (' ' + CPTModifier3) END) + (CASE WHEN CPTModifier4 Is Null THEN '' ELSE (' ' + CPTModifier4) END)) ELSE '%s' END) AS ItemCode, "
						"COALESCE((CASE WHEN ServiceT.RevCodeUse = 1 THEN UB92CategoriesT1.Name WHEN ServiceT.RevCodeUse = 2 THEN UB92CategoriesT2.Name ELSE LineItemT.Description END),LineItemT.Description) AS ItemDesc, "
						"Min(LineItemT.[Date]) AS TDate, Sum(ChargesT.Quantity) AS Quantity, Sum(dbo.GetChargeTotal(ChargesT.ID)) AS Amount "
						"FROM BillsT "
						"INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
						"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
						"LEFT JOIN CPTModifierT ON CPTModifier = CPTModifierT.Number LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number "
						"LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
						"LEFT JOIN UB92CategoriesT UB92CategoriesT1 ON ServiceT.UB92Category = UB92CategoriesT1.ID "
						"LEFT JOIN (SELECT ServiceRevCodesT.*, PersonID AS InsuredPartyID FROM ServiceRevCodesT INNER JOIN InsuredPartyT ON ServiceRevCodesT.InsuranceCompanyID = InsuredPartyT.InsuranceCoID) AS ServiceRevCodesT ON BillsT.InsuredPartyID = ServiceRevCodesT.InsuredPartyID AND ServiceT.ID = ServiceRevCodesT.ServiceID "
						"LEFT JOIN UB92CategoriesT UB92CategoriesT2 ON ServiceRevCodesT.UB92CategoryID = UB92CategoriesT2.ID "
						"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
						"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
						"WHERE LineItemT.Deleted = 0 AND Batched = 1 AND BillsT.ID = %li "
						"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
						"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
						"GROUP BY "
						"(CASE WHEN ServiceT.RevCodeUse = 1 THEN UB92CategoriesT1.Code WHEN ServiceT.RevCodeUse = 2 THEN UB92CategoriesT2.Code ELSE NULL END), "
						"(CASE WHEN ServiceT.RevCodeUse = 0 THEN (ItemCode + (CASE WHEN CPTModifier Is Null THEN '' ELSE (' ' + CPTModifier) END) + (CASE WHEN CPTModifier2 Is Null THEN '' ELSE (' ' + CPTModifier2) END)+ (CASE WHEN CPTModifier3 Is Null THEN '' ELSE (' ' + CPTModifier3) END) + (CASE WHEN CPTModifier4 Is Null THEN '' ELSE (' ' + CPTModifier4) END)) ELSE '%s' END), "
						"COALESCE((CASE WHEN ServiceT.RevCodeUse = 1 THEN UB92CategoriesT1.Name WHEN ServiceT.RevCodeUse = 2 THEN UB92CategoriesT2.Name ELSE LineItemT.Description END),LineItemT.Description) "
						"ORDER BY RevCode, Min(ChargesT.LineID)",_Q(strBox44Override), billid, _Q(strBox44Override));
			}
		}

		g_aryFormQueries[13].sql.Replace("FROM FormChargesT",str);
		g_aryFormQueries[31].sql.Replace("FROM FormChargesT",str);		
		g_aryFormQueries[9].sql.Replace("FROM FormChargesT",str);
		g_aryFormQueries[10].sql.Replace("FROM FormChargesT",str);
		str.Format("INNER JOIN ((%s) AS FormChargesT",g_aryFormQueries[2].sql);
		g_aryFormQueries[16].sql.Replace("INNER JOIN (FormChargesT",str);		

		// Fill total charge amount for each line
		nTotalCharges = SizeOfFormChargesT();
	} NxCatchAll("UB92Dlg::BuildUB92ChargesT()");
}

void OnUB92Command(WPARAM wParam, LPARAM lParam, CDWordArray& dwChangedForms, int FormControlsID, CFormDisplayDlg* dlg) 
{
}

void OnUB92KeyDown(CDialog* pFormDisplayDlg, MSG* pMsg)
{
	CFormDisplayDlg* pDlg = (CFormDisplayDlg*)pFormDisplayDlg;
	CWnd* pCtrl = pDlg->GetFocus();

	//JJ - if revenue codes worked, this would be pretty cool....
	/*

	int id, descid;
	CString str,sql;

	// Search for the control corresponding to the window
	for (int j=0; j < pDlg->m_ControlArray.GetSize(); j++) {
		if ( ((FormControl*)pDlg->m_ControlArray.GetAt(j))->nID == pCtrl->GetDlgCtrlID())
			break;
	}

	// If found, see if the form is 31. If so, that means it's a revenue
	// code, and we need to change the description text so that it
	// corresponds to the revenue code.

	if (j < pDlg->m_ControlArray.GetSize()) {
		if (((FormControl*)pDlg->m_ControlArray.GetAt(j))->form == 31) {

			// Get the revenue code (0 if invalid)			
			pCtrl->GetWindowText(str);
			try {
				_RecordsetPtr rsRevCodes;
				CString sql;
				sql.Format("SELECT * FROM UB92RevCodes WHERE ID = %s",str);
				rsRevCodes = CreateRecordsetStd(sql);
				if(!rsRevCodes->eof)
					str = CString(rsRevCodes->Fields->Item["Description"]->Value.bstrVal);
				else
					str.Empty();
			}NxCatchAll("Error in loading RevCodes");

			// Search for the corresponding desciption
			id = ((FormControl*)pDlg->m_ControlArray.GetAt(j))->id;
			if (id == 931)
				descid = 932;
			else
				descid = id + 23;

			for (int j=0; j < pDlg->m_ControlArray.GetSize(); j++) {
				if ( ((FormControl*)pDlg->m_ControlArray.GetAt(j))->id == descid)
					break;
			}

			// Change the window text to the description matching the
			// revenue code
			if (j < pDlg->m_ControlArray.GetSize()) {
				id = ((FormControl*)pDlg->m_ControlArray.GetAt(j))->nID;
				pCtrl = pDlg->GetDlgItem(id);

				if (!str.IsEmpty())
					pCtrl->SetWindowText(str);
			}
		}
	}
	*/
}

void CUB92Dlg::OnClickAlign() 
{
	CPrintAlignDlg dlg(this);
	dlg.m_FormID = 1;
	dlg.DoModal();
}

void CUB92Dlg::OnClickCapitalizeOnPrint() 
{
	if(m_CapOnPrint) {
		SetRemotePropertyInt("CapitalizeHCFA",0);
	}
	else {
		SetRemotePropertyInt("CapitalizeHCFA",1);
	}
	m_CapOnPrint = GetRemotePropertyInt("CapitalizeHCFA",0);
}

void CUB92Dlg::OnClickRestore() 
{
	CString strSQL;

	if(!CheckCurrentUserPermissions(bioClaimForms,sptWrite))
		return;

	if (IDNO == MessageBox("All changes you have made to the UB-92 will be unrecoverable. Are you sure you wish to do this?", NULL, MB_YESNO))
		return;

	CWaitCursor pWait;

	try {
	// Delete all the form changes
	// (j.jones 2008-05-28 15:46) - PLID 27881 - batched statements, ensured we delete all relevant FormHistory entries
	CString strBatch = BeginSqlBatch();
	AddStatementToSqlBatch(strBatch, "DELETE FROM FormHistoryT WHERE DocumentID IN (SELECT FirstDocumentID FROM HCFADocumentsT WHERE BillID = %li AND FormType = 1)", m_ID);
	AddStatementToSqlBatch(strBatch, "DELETE FROM HCFADocumentsT WHERE HCFADocumentsT.BillID = %li AND FormType = 1",m_ID);
	ExecuteSqlBatch(strBatch);

	// Remove all changes made by the user
	m_pframe->UndoChanges();

	} NxCatchAll("Error restoring defaults.");

	//Refresh the UB92

	m_pframe->Refresh (9);  // Box82
	m_pframe->Refresh (10); // Box83A

	m_pframe->Refresh (11); // Patient information
	m_pframe->Refresh (12); // Charges
	m_pframe->Refresh (31); // Revenue codes column
	m_pframe->Refresh (13); // Bill stuff
	m_pframe->Refresh (14); // Location Info

	m_pframe->Refresh (15); // Insured party address
	m_pframe->Refresh (16); // Provider data
	m_pframe->Refresh (17); // Insurance information

	m_pframe->Refresh (19); // Insurance apply totals

	m_pframe->Refresh (18); // Other insurance information
	m_pframe->Refresh (30); // Other insurance apply totals

	{
		// (j.jones 2005-02-01 15:17) - need to refresh the "Total" row in the charge listing,
		// which is not loaded but instead deliberately modified in place

		//Load ShowTotals setting - to show the charge totals on line 23 of the charge listing	
		BOOL bSetDesc = FALSE, bSetAmount = FALSE;
		if(m_UB92Info.ShowTotals == 1) {

			//if ShowTotals is true, then we must show a grand total line on line 23 of the charge listing
			
			CString strTotal = "0.00";

			CString where;
			// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" charges
			where.Format (" WHERE BillsT.ID = %li AND LineItemT.Deleted = 0 AND Batched = 1 "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null", m_ID);

			_RecordsetPtr rs = CreateRecordset(g_aryFormQueries[31].sql + where);
			if(!rs->eof) {
				strTotal = FormatCurrencyForInterface(AdoFldCurrency(rs, "SumOfCharges",COleCurrency(0,0)), FALSE);
			}
			rs->Close();

			for (unsigned int j=0; j < m_pframe->m_ControlArray.size(); j++) {
				FormControl* pCtrl = m_pframe->m_ControlArray[j].get();
				if (pCtrl->id == 1024) {
					//set the description to say "Total"
					FormEdit* pEdit = (FormEdit*)pCtrl;
					pEdit->SetWindowText("Total");
					bSetDesc = TRUE;
				}
				if (pCtrl->id == 1112) {
					//set the amount to be the grand total
					FormEdit* pEdit = (FormEdit*)pCtrl;
					pEdit->SetWindowText(strTotal);
					bSetAmount = TRUE;
				}
				//break when we're done - no reason to stay in the loop
				if(bSetDesc && bSetAmount)
					break;
			}
		}
	}

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

void CUB92Dlg::Save(BOOL boSaveAll, CDialog* pdlgWait, int& iPage)
{
	_RecordsetPtr nxrs(__uuidof(Recordset));
	CString str;
	int provider;

	for (provider_index=0; provider_index < adwProviderList.GetSize(); provider_index++) {

		provider = adwProviderList.GetAt(provider_index);

		for (firstcharge=0; firstcharge < nTotalCharges; firstcharge+=6) {

			if (pdlgWait) {
				str.Format("Saving UB-92 page %d...", ++iPage);
				pdlgWait->GetDlgItem(IDC_LABEL)->SetWindowText(str);
				pdlgWait->RedrawWindow();
			}

			SaveHistoricalData(boSaveAll);
		}
	}
}

void CUB92Dlg::SaveHistoricalData(BOOL boSaveAll)
{
#ifdef SAVE_HISTORICAL_INFORMATION

	int iDocumentID;
	CString str;
	_RecordsetPtr rs(__uuidof(Recordset));

	try {		
		str.Format("SELECT FirstDocumentID FROM HCFADocumentsT WHERE BillID = %d AND FirstCharge = %d AND ProviderIndex = %d AND FormType = 1", m_ID, firstcharge, adwProviderList.GetAt(provider_index));
		rs = CreateRecordset(str);
		if (!rs->eof) {
			iDocumentID = rs->Fields->Item["FirstDocumentID"]->Value.lVal;
			rs->Close();
		}
		else {
			rs->Close();
			iDocumentID = NewNumber("FormHistoryT","DocumentID");
			//BEGIN_TRANS("SaveHistoricalHCFA") {
			str.Format("INSERT INTO HCFADocumentsT ( BillID, FirstDocumentID, FirstCharge, ProviderIndex, FormType ) VALUES ( %d, %d, %d, %d, 1 )", m_ID, iDocumentID, firstcharge, adwProviderList.GetAt(provider_index));
			ExecuteSql(str);
			str.Format("INSERT INTO FormHistoryT ( DocumentID, ControlID, [Value] ) VALUES ( %d, -1, NULL )", iDocumentID);
			ExecuteSql(str);
			//} END_TRANS_CATCH_ALL("SaveHistoricalHCFA");
		}		
	}
	NxCatchAll("Error in OnClickCheck()");

	///////////////////////////////////
	// Make sure we pull our historical data from the right table
	m_pframe->SetDocumentID(iDocumentID);

	// (j.jones 2014-01-15 09:23) - PLID 53952 - this was not restoring the proper UB forms,
	// now it does

	m_pframe->Refresh (9);  // Box82
	m_pframe->Refresh (10); // Box83A

	m_pframe->Refresh (11); // Patient information
	m_pframe->Refresh (12); // Charges
	m_pframe->Refresh (31); // Revenue codes column
	m_pframe->Refresh (13); // Bill stuff
	m_pframe->Refresh (14); // Location Info

	m_pframe->Refresh (15); // Insured party address
	m_pframe->Refresh (16); // Provider data
	m_pframe->Refresh (17); // Insurance information

	m_pframe->Refresh (19); // Insurance apply totals

	m_pframe->Refresh (18); // Other insurance information
	m_pframe->Refresh (30); // Other insurance apply totals

	m_pframe->ReapplyChanges(firstcharge, provider_index);

	m_pframe->Save (firstcharge, provider_index, iDocumentID, boSaveAll);

	// We save form 6 (zero based in the array) because where the radio button is
	// determines what text goes in a particular control. To save us time and pain,
	// we always save the data in form 6.
	//((FormLayer*)m_pframe->m_LayerArray[5])->Save(iDocumentID, 0, &m_pframe->m_ControlArray);
#endif
}

long CUB92Dlg::DoScrollTo(long nNewTopPos)
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

void CUB92Dlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
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

BOOL PreUB92Print(CDWordArray& dwChangedForms, CFormDisplayDlg* dlg)
{

	/*
	if (adwProviderList.GetSize() == 0) 
	{	ASSERT(FALSE);
		HandleException (NULL, "Failure in PreHCFAPrint", __LINE__, __FILE__);
		return FALSE; // This should never happen
	}

	// Only happens on first call to this function. The first provider
	// information is reloaded.
	if (nPrintedCharges == 0 && nPrintedProviders == 0) {
		oldfirstcharge = firstcharge;
		oldprovider_index = provider_index;		

		provider_index = -1;
		OnUB92Command(IDC_PHYSICIAN_LEFT, 0, dwChangedForms, 0, dlg);
		firstcharge = -6;
		nPrintedProviders++;
	}


	// If still more charges to print, print them and return
	if (nPrintedCharges < nTotalCharges) {		
		OnUB92Command(IDC_BILL_DOWN, 0, dwChangedForms, 0, dlg);
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

		OnUB92Command(IDC_PHYSICIAN_RIGHT, 0, dwChangedForms, 0, dlg);
		firstcharge = -6;
		OnUB92Command(IDC_BILL_DOWN, 0, dwChangedForms, 0, dlg);
		nPrintedCharges = 6;
		nPrintedProviders++;
		return TRUE;
	}
	*/

	return TRUE;
}

void CUB92Dlg::GetUB92GroupID()
{
	_RecordsetPtr rs(__uuidof(Recordset));
	CString str;

	fee_group_id = -1;
	try {
		str.Format("SELECT InsuranceCoT.UB92SetupGroupID FROM InsuranceCoT INNER JOIN InsuredPartyT ON "
			"InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID WHERE (((InsuredPartyT.PersonID)=%li))", m_InsuredPartyID);
		rs = CreateRecordset(str);
		if (!rs->eof) {
			_variant_t var = rs->Fields->Item["UB92SetupGroupID"]->Value;
			if (var.vt != VT_NULL) {
				fee_group_id = var.lVal;
			}
		}
		rs->Close();
	}
	NxCatchAll("UB92Dlg::GetUB92GroupID()");
}

void CUB92Dlg::BuildBoxes8283()
{
	try {

		//Box82/83 Setup:
		//1 - Bill Provider
		//2 - General 1 Provider
		//3 - Referring Physician
		
		//Box82/83 Format:
		//1 - Line A: ID, Line B: Name
		//2 - Line A: Blank, Line B: ID, Name
		//3 - Line A: ID, Line B: Blank

		CString strBox82FromClause = "", strBox83FromClause = "";
		CString strSelectStatement = "";
		CString strBox82SelectStatement = "", strBox83SelectStatement = "";
		CString strIDType = "";

		//first use the Box8283Format to determine the strSelectStatement
		switch(m_UB92Info.Box8283Format) {			
			case 2:
				//2 - Line A: Blank, Line B: ID, Name
				strSelectStatement = "SELECT '' AS BoxID, ReplaceID + ' ' + Last + ', ' + First + ' ' + Middle + ' ' + Title AS BoxName ";
				break;
			case 3:
				//3 - Line A: ID, Line B: Blank
				strSelectStatement = "SELECT ReplaceID AS BoxID, '' AS BoxName ";
				break;
			case 1:
			default:
				//1 - Line A: ID, Line B: Name
				strSelectStatement = "SELECT ReplaceID AS BoxID, Last + ', ' + First + ' ' + Middle + ' ' + Title AS BoxName ";
				break;
		}

		//then format the Box82 Setup, query 9

		strBox82SelectStatement = strSelectStatement;
		
		strBox82SelectStatement.Replace("ReplaceID",GetBox8283ID(m_UB92Info.Box82Setup,m_UB92Info.Box82Num));

		strBox82SelectStatement.Replace("BoxID","Box82A");
		strBox82SelectStatement.Replace("BoxName","Box82B");
		
		switch(m_UB92Info.Box82Setup) {
			case 2:
				//General 1 Provider

				// (j.jones 2006-12-04 15:25) - PLID 23733 - supported the ClaimProviderID override
				// (j.jones 2010-11-10 09:14) - PLID 41387 - supported ChargesT.ClaimProviderID
				strBox82FromClause.Format(" FROM FormChargesT INNER JOIN PatientsT ON FormChargesT.PatientID = PatientsT.PersonID "
					"INNER JOIN (SELECT PersonID, ClaimProviderID FROM ProvidersT) ActualProvidersT ON PatientsT.MainPhysician = ActualProvidersT.PersonID "
					"INNER JOIN ProvidersT ON (CASE WHEN FormChargesT.ClaimProviderID Is Not Null THEN FormChargesT.ClaimProviderID ELSE ActualProvidersT.ClaimProviderID END) = ProvidersT.PersonID "
					"INNER JOIN PersonT ON ProvidersT.PersonID = PersonT.ID "
					"WHERE FormChargesT.BillID = %li",m_ID);
				break;
			case 3:
				//Referring Physician
				strBox82FromClause.Format(" FROM BillsT INNER JOIN ReferringPhysT ON BillsT.RefPhyID = ReferringPhysT.PersonID INNER JOIN PersonT ON ReferringPhysT.PersonID = PersonT.ID WHERE BillsT.ID = %li",m_ID);
				break;
			case 1:
			default:
				//Bill Provider

				// (j.jones 2006-12-04 15:25) - PLID 23733 - supported the ClaimProviderID override
				// (j.jones 2010-11-10 09:14) - PLID 41387 - supported ChargesT.ClaimProviderID
				strBox82FromClause = " FROM FormChargesT "
					"INNER JOIN (SELECT PersonID, ClaimProviderID FROM ProvidersT) ActualProvidersT ON FormChargesT.DoctorsProviders = ActualProvidersT.PersonID "
					"INNER JOIN ProvidersT ON (CASE WHEN FormChargesT.ClaimProviderID Is Not Null THEN FormChargesT.ClaimProviderID ELSE ActualProvidersT.ClaimProviderID END) = ProvidersT.PersonID "
					"INNER JOIN PersonT ON ProvidersT.PersonID = PersonT.ID";
				break;
		}

		//then format the Box83A Setup, query 10

		strBox83SelectStatement = strSelectStatement;

		if(m_UB92Info.Box83Setup > 0) {
			strBox83SelectStatement.Replace("ReplaceID",GetBox8283ID(m_UB92Info.Box83Setup,m_UB92Info.Box83Num));
		}
		else {
			//do not fill
			strBox83SelectStatement = "SELECT '' AS BoxID, '' AS BoxName ";
		}

		strBox83SelectStatement.Replace("BoxID","Box83AA");
		strBox83SelectStatement.Replace("BoxName","Box83AB");

		switch(m_UB92Info.Box83Setup) {
			case 1:			
				//Bill Provider

				// (j.jones 2006-12-04 15:25) - PLID 23733 - supported the ClaimProviderID override
				// (j.jones 2010-11-10 09:16) - PLID 41387 - supported ChargesT.ClaimProviderID
				strBox83FromClause = " FROM FormChargesT "
					"INNER JOIN (SELECT PersonID, ClaimProviderID FROM ProvidersT) ActualProvidersT ON FormChargesT.DoctorsProviders = ActualProvidersT.PersonID "
					"INNER JOIN ProvidersT ON (CASE WHEN FormChargesT.ClaimProviderID Is Not Null THEN FormChargesT.ClaimProviderID ELSE ActualProvidersT.ClaimProviderID END) = ProvidersT.PersonID "
					"INNER JOIN PersonT ON ProvidersT.PersonID = PersonT.ID";
				break;
			case 2:
				//General 1 Provider

				// (j.jones 2006-12-04 15:25) - PLID 23733 - supported the ClaimProviderID override
				// (j.jones 2010-11-10 09:16) - PLID 41387 - supported ChargesT.ClaimProviderID
				strBox83FromClause.Format(" FROM FormChargesT INNER JOIN PatientsT ON FormChargesT.PatientID = PatientsT.PersonID "
					"INNER JOIN (SELECT PersonID, ClaimProviderID FROM ProvidersT) ActualProvidersT ON PatientsT.MainPhysician = ActualProvidersT.PersonID "
					"INNER JOIN ProvidersT ON (CASE WHEN FormChargesT.ClaimProviderID Is Not Null THEN FormChargesT.ClaimProviderID ELSE ActualProvidersT.ClaimProviderID END) = ProvidersT.PersonID "
					"INNER JOIN PersonT ON ProvidersT.PersonID = PersonT.ID WHERE FormChargesT.BillID = %li",m_ID);
				break;
			case 3:
			default:
				//Referring Physician
				strBox83FromClause.Format(" FROM BillsT INNER JOIN ReferringPhysT ON BillsT.RefPhyID = ReferringPhysT.PersonID INNER JOIN PersonT ON ReferringPhysT.PersonID = PersonT.ID WHERE BillsT.ID = %li",m_ID);
				break;			
		}
		
	
		//Box82
		g_aryFormQueries[9].sql.Format("%s %s",strBox82SelectStatement, strBox82FromClause);

		//Box83
		g_aryFormQueries[10].sql.Format("%s %s",strBox83SelectStatement, strBox83FromClause);

	}NxCatchAll("Error building boxes 82/83");
}

CString CUB92Dlg::GetBox8283ID(int Box8283Setup, int Box8283Number)
{
	try {

		CString strNum = "UPIN";

		switch(Box8283Number) {
			case 2:		//SSN
				strNum = "SocialSecurity";
				break;
			case 3:		//EIN
				if(Box8283Setup == 3)
					//ref. phy.
					strNum = "FedEmployerID";
				else
					//provider
					strNum = "[Fed Employer ID]";
				break;
			case 4:		//Medicare
				if(Box8283Setup == 3)
					//ref. phy.
					strNum = "MedicareNumber";
				else
					//provider
					strNum = "[Medicare Number]";
				break;
			case 5:		//Medicaid
				if(Box8283Setup == 3)
					//ref. phy.
					strNum = "MedicaidNumber";
				else
					//provider
					strNum = "[Medicaid Number]";
				break;
			case 6:		//BCBS
				if(Box8283Setup == 3)
					//ref. phy.
					strNum = "BlueShieldID";
				else
					//provider
					strNum = "[BCBS Number]";
				break;
			case 7:		//DEA
				if(Box8283Setup == 3)
					//ref. phy.
					strNum = "DEANumber";
				else
					//provider
					strNum = "[DEA Number]";
				break;
			case 8:		//Workers Comp.
				if(Box8283Setup == 3)
					//ref. phy.
					strNum = "WorkersCompNumber";
				else
					//provider
					strNum = "[Workers Comp Number]";
				break;
			case 9:		//Other ID
				if(Box8283Setup == 3)
					//ref. phy.
					strNum = "OtherIDNumber";
				else
					//provider
					strNum = "[Other ID Number]";
				break;
			case 10: {	//Box 51

				CString strSQL = "";
				strNum = "";

				// (j.jones 2007-04-20 12:54) - PLID 25449 - load the default Box 51 number
				strNum = m_UB92Info.Box51Default;

				//if blank, load from the insurance list
				if(strNum.IsEmpty()) {

					if(Box8283Setup == 1) {
						//Bill Provider

						// (j.jones 2006-12-04 15:25) - PLID 23733 - supported the ClaimProviderID override
						// (j.jones 2010-11-10 09:19) - PLID 41387 - supported ChargesT.ClaimProviderID
						strSQL.Format("SELECT InsuranceBox51.Box51Info AS Box51 FROM (ChargesT INNER JOIN (((InsuranceCoT INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID) INNER JOIN InsuranceBox51 ON InsuranceCoT.PersonID = InsuranceBox51.InsCoID) INNER JOIN BillsT ON (InsuredPartyT.PersonID = BillsT.InsuredPartyID) AND (InsuredPartyT.PatientID = BillsT.PatientID)) ON ChargesT.BillID = BillsT.ID) "
							"INNER JOIN (SELECT PersonID, ClaimProviderID FROM ProvidersT) ActualProvidersT ON ChargesT.DoctorsProviders = ActualProvidersT.PersonID "
							"INNER JOIN ProvidersT ON (CASE WHEN ChargesT.ClaimProviderID Is Not Null THEN ChargesT.ClaimProviderID ELSE ActualProvidersT.ClaimProviderID END) = ProvidersT.PersonID "
							"	AND (InsuranceBox51.ProviderID = ProvidersT.PersonID) "
								"GROUP BY InsuranceBox51.Box51Info, BillsT.ID HAVING (((BillsT.ID)=%li))", m_ID);
					}
					else if(Box8283Setup == 2) {
						//Gen 1 Provider

						// (j.jones 2006-12-04 15:25) - PLID 23733 - supported the ClaimProviderID override
						// (j.jones 2010-11-10 09:19) - PLID 41387 - supported ChargesT.ClaimProviderID
						// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" charges
						strSQL.Format("SELECT TOP 1 InsuranceBox51.Box51Info AS Box51 FROM InsuranceCoT "
							"INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID "
							"INNER JOIN InsuranceBox51 ON InsuranceCoT.PersonID = InsuranceBox51.InsCoID "
							"INNER JOIN BillsT ON InsuredPartyT.PersonID = BillsT.InsuredPartyID "
							"INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
							"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
							"INNER JOIN PatientsT ON BillsT.PatientID = PatientsT.PersonID "
							"INNER JOIN ProvidersT ActualProvidersT ON PatientsT.MainPhysician = ActualProvidersT.PersonID "
							"INNER JOIN ProvidersT ON (CASE WHEN ChargesT.ClaimProviderID Is Not Null THEN ChargesT.ClaimProviderID ELSE ActualProvidersT.ClaimProviderID END) = ProvidersT.PersonID "
							"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
							"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
							"AND (InsuranceBox51.ProviderID = ProvidersT.PersonID) "
							"WHERE LineItemT.Deleted = 0 AND ChargesT.Batched = 1 "
							"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
							"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
							"GROUP BY InsuranceBox51.Box51Info, BillsT.ID HAVING (((BillsT.ID)=%li))", m_ID);
					}
					else if(Box8283Setup == 3) {
						//referring phys don't have this number
						strNum = "";
					}

					if(!strSQL.IsEmpty()) {
						_RecordsetPtr rs = CreateRecordset(strSQL);
						if (!rs->eof) {
							strNum = AdoFldString(rs, "Box51","");
						}
						rs->Close();
					}
				}

				strNum = "'" + _Q(strNum) + "'";
				break;
			}
			case 11: {	//Group Number

				CString strSQL = "";
				strNum = "";

				if(Box8283Setup == 1) {
					//Bill Provider

					// (j.jones 2006-12-04 15:25) - PLID 23733 - supported the ClaimProviderID override
					// (j.jones 2010-11-10 09:19) - PLID 41387 - supported ChargesT.ClaimProviderID
					strSQL.Format("SELECT InsuranceGroups.GRP FROM (ChargesT INNER JOIN (((InsuranceCoT INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID) INNER JOIN InsuranceGroups ON InsuranceCoT.PersonID = InsuranceGroups.InsCoID) INNER JOIN BillsT ON (InsuredPartyT.PersonID = BillsT.InsuredPartyID) AND (InsuredPartyT.PatientID = BillsT.PatientID)) ON ChargesT.BillID = BillsT.ID) "
							"INNER JOIN ProvidersT ActualProvidersT ON ChargesT.DoctorsProviders = ActualProvidersT.PersonID "
							"INNER JOIN ProvidersT ON (CASE WHEN ChargesT.ClaimProviderID Is Not Null THEN ChargesT.ClaimProviderID ELSE ActualProvidersT.ClaimProviderID END) = ProvidersT.PersonID "
							"AND (InsuranceGroups.ProviderID = ProvidersT.PersonID) GROUP BY InsuranceGroups.GRP, BillsT.ID HAVING (((BillsT.ID)=%li))", m_ID);
				}
				else if(Box8283Setup == 2) {
					//Gen 1 Provider

					// (j.jones 2006-12-04 15:25) - PLID 23733 - supported the ClaimProviderID override
					// (j.jones 2010-11-10 09:20) - PLID 41387 - supported ChargesT.ClaimProviderID
					strSQL.Format("SELECT InsuranceGroups.GRP FROM InsuranceCoT "
						"INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID "
						"INNER JOIN InsuranceGroups ON InsuranceCoT.PersonID = InsuranceGroups.InsCoID "
						"INNER JOIN BillsT ON InsuredPartyT.PersonID = BillsT.InsuredPartyID "
						"INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
						"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
						"INNER JOIN PatientsT ON BillsT.PatientID = PatientsT.PersonID "
						"INNER JOIN ProvidersT ActualProvidersT ON PatientsT.MainPhysician = ActualProvidersT.PersonID "
						"INNER JOIN ProvidersT ON (CASE WHEN ChargesT.ClaimProviderID Is Not Null THEN ChargesT.ClaimProviderID ELSE ActualProvidersT.ClaimProviderID END) = ProvidersT.PersonID "
						"AND (InsuranceGroups.ProviderID = ProvidersT.PersonID) "
						"WHERE LineItemT.Deleted = 0 AND ChargesT.Batched = 0 "
						"GROUP BY InsuranceGroups.GRP, BillsT.ID HAVING (((BillsT.ID)=%li))", m_ID);
				}
				else if(Box8283Setup == 3) {
					//referring phys don't have this number
					return "''";
				}

				_RecordsetPtr rs = CreateRecordset(strSQL);
				if (!rs->eof) {
					strNum = AdoFldString(rs, "GRP","");
				}
				rs->Close();
				strNum = "'" + strNum + "'";
				break;
			}
			case 12:	//NPI
				strNum = "NPI";
				break;
			case 1:		//UPIN
				strNum = "UPIN";
				break;
			default:
				// (j.jones 2008-05-21 15:53) - PLID 30139 - we now allow blank values,
				// but remember we're returning a field name, not the number, so send ''
				strNum = "''";
				break;
		}

		return strNum;

	}NxCatchAll("Error getting Box82/83 IDs.");

	return "UPIN";
}

void CUB92Dlg::OnRadioPaper() {

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
	_RecordsetPtr rs = CreateRecordset("SELECT dbo.GetClaimTotal(ID) AS Total FROM BillsT WHERE ID = %li",m_ID);
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

void CUB92Dlg::OnRadioElectronic() {

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
	_RecordsetPtr rs = CreateRecordset("SELECT dbo.GetClaimTotal(ID) AS Total FROM BillsT WHERE ID = %li",m_ID);
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

void CUB92Dlg::OnRadioNoBatch() {

	if(!CheckCurrentUserPermissions(bioClaimForms,sptWrite)) {
		FindBatch();
		return;
	}

	UpdateBatch();
}

void CUB92Dlg::FindBatch()
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

void CUB92Dlg::UpdateBatch()
{
	CString sql, str;
	short nBatch = 0;

	if (((CButton*)GetDlgItem(IDC_RADIO_PAPER))->GetCheck())
		nBatch = 1; // paper batch
	else if (((CButton*)GetDlgItem(IDC_RADIO_ELECTRONIC))->GetCheck())
		nBatch = 2; // electronic batch

	BatchBill(m_ID, nBatch);
}

void CUB92Dlg::SetUB92DateFormats()
{
	try {
		_RecordsetPtr rs(__uuidof(Recordset));
		CString str;
		int i = 0, j = 0;
		// (j.jones 2014-01-15 09:52) - PLID 53952 - fixed Box 81, which errantly had two edit boxes listed as dates
		int boxids[][24] = {
			{ 836, 837, -1 },	//Box 6
			{ 844, -1 },		//Box 14
			{ 847, -1 },		//Box 17
			{ 892, 905, 907, 909, 911, 913, -1 },	//Box 32 - 36
			{ 934, 1047, 1048, 1049, 1050, 1051, 1052, 1053, 1054, 
			  1055, 1056, 1057, 1058, 1059, 1060, 1061, 1062, 1063, 
			  1064, 1065, 1066, 1067, 1068, -1 },	//Box 45
			{ 1297, 1300, 1302, 1311, 1313, 1315, -1 },		//Box 80 - 81
			{ 1321, -1 }		//Box 86
		};
		CString strBoxNames[] = {
			"Box6", "Box14", "Box17", "Box32_36", "Box45", "Box80_81", "Box86"
		};

		// Assign the date formats to be 2 or 4 digit
		rs = CreateParamRecordset("SELECT Box6, Box14, Box17, Box32_36, Box45, Box80_81, Box86 FROM UB92SetupT WHERE ID = {INT}", fee_group_id);

		//Don't forget the DateFmt setting - 0 is YearFirst, 1 is MonthFirst

		// If no group in UB92 ID setup, default to 4 digits for all dates
		if (rs->eof) {
			for (i=0; i < 7; i++) {
				j=0;
				while (boxids[i][j] != -1) {
					if(m_UB92Info.DateFmt == 0)	//year-first
						ExecuteParamSql("UPDATE FormControlsT SET Format = 20480 WHERE ID = {INT}", boxids[i][j]);
					else	//month-first
						ExecuteParamSql("UPDATE FormControlsT SET Format = 4096 WHERE ID = {INT}", boxids[i][j]);
					j++;
				}
			}
		}
		else {

			for (i=0; i < 7; i++) {

				//first get the 2/4 digit setup right
				switch (rs->Fields->Item[_bstr_t(strBoxNames[i])]->Value.lVal) {
				case 2:
					j=0;
					while (boxids[i][j] != -1) {
						if(m_UB92Info.DateFmt == 0)	//year-first
							ExecuteParamSql("UPDATE FormControlsT SET Format = 20481 WHERE ID = {INT}", boxids[i][j]);
						else	//month-first
							ExecuteParamSql("UPDATE FormControlsT SET Format = 4097 WHERE ID = {INT}", boxids[i][j]);
						j++;
					}
					break;
				case 4:
					j=0;
					while (boxids[i][j] != -1) {
						if(m_UB92Info.DateFmt == 0)	//year-first
							ExecuteParamSql("UPDATE FormControlsT SET Format = 20480 WHERE ID = {INT}", boxids[i][j]);
						else	//month-first
							ExecuteParamSql("UPDATE FormControlsT SET Format = 4096 WHERE ID = {INT}", boxids[i][j]);
						j++;
					}
					break;
				}
			}
		}
		rs->Close();
	}
	NxCatchAll("SetUB92DateFormat");
}

// (j.jones 2007-06-22 13:23) - PLID 25665 - handle when a field was edited, so we update the info text box
BOOL CUB92Dlg::PreTranslateMessage(MSG *pMsg) {

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

// ADADlg.cpp : implementation file
//

#include "stdafx.h"
#include "ADADlg.h"
#include "FormDisplayDlg.h"
#include "FormFormat.h"
#include "GlobalFinancialUtils.h"
#include "PrintAlignDlg.h"
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

// ADA stuff

static CDWordArray adwProviderList;
static int firstcharge = 0;
static int provider_index = 0;
static int fee_group_id = -1;
static int nTotalCharges;

// For printing
static int nPrintedCharges = 0;
static int nPrintedProviders = 0;
static int oldfirstcharge, oldprovider_index;

using namespace ADODB;

BOOL PreADAPrint(CDWordArray& dwChangedForms, CFormDisplayDlg* dlg);

void CADADlg::RequeryHistoricalADAData()
{
	try {
		// (j.armen 2014-03-05 09:17) - PLID 60784 - Parameterized
		_RecordsetPtr prs = CreateParamRecordset(
			"SELECT\r\n"
			"	FirstDocumentID\r\n"
			"FROM HCFADocumentsT\r\n"
			"WHERE BillID = {INT}\r\n"
			"	AND FirstCharge = {INT}\r\n"
			"	AND ProviderIndex = {INT}\r\n"
			"	AND FormType = {INT}",
			m_nBillID, firstcharge, adwProviderList[provider_index], 2);

		m_pframe->SetDocumentID(prs->eof ? -1 : AdoFldLong(prs, "FirstDocumentID"));
	}
	NxCatchAll("Error in requerying historical data");
}

/////////////////////////////////////////////////////////////////////////////
// CADADlg dialog

BEGIN_MESSAGE_MAP(CADADlg, CDialog)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_X, OnClickX)
	ON_BN_CLICKED(IDC_CHECK, OnClickCheck)
	ON_BN_CLICKED(IDC_PRINT, OnClickPrint)
	ON_BN_CLICKED(IDC_ALIGN, OnClickAlign)
	ON_BN_CLICKED(IDC_RESTORE, OnClickRestore)
	ON_BN_CLICKED(IDC_RADIO_PAPER, OnRadioPaper)
	ON_BN_CLICKED(IDC_RADIO_NO_BATCH, OnRadioNoBatch)
	ON_BN_CLICKED(IDC_CAP_ON_PRINT, OnClickCapitalizeOnPrint)
	ON_WM_VSCROLL()
END_MESSAGE_MAP()

void OnADACommand(WPARAM wParam, LPARAM lParam, CDWordArray& dwChangedForms, int FormControlsID, CFormDisplayDlg* dlg);
void OnADAKeyDown(CDialog* pFormDisplayDlg, MSG* pMsg);

// (j.armen 2014-03-05 09:17) - PLID 60784 - BillID and PatientID are both required on construction
CADADlg::CADADlg(CWnd* pParent, const long& nBillID, const long& nPatientID)
	: CDialog(CADADlg::IDD, pParent),
	m_nBillID(nBillID),
	m_nPatientID(nPatientID)
{
	m_bPrintWarnings = TRUE;
	m_InsuredPartyID = -1;
	m_OthrInsuredPartyID = -1;

	m_pOnCommand = &OnADACommand;
	m_pOnKeyDown = &OnADAKeyDown;
	m_pPrePrintFunc = NULL;
	m_ShowWindowOnInit = TRUE;
}

CADADlg::~CADADlg()
{
	m_pframe.reset();
}

void CADADlg::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	// (j.jones 2007-06-25 10:14) - PLID 25663 - changed the buttons to NxIconButtons
	DDX_Control(pDX, IDC_RESTORE, m_btnRestoreDefaults);
	DDX_Control(pDX, IDC_ALIGN, m_btnAlignForm);
	DDX_Control(pDX, IDC_CHECK, m_btnSave);
	DDX_Control(pDX, IDC_PRINT, m_btnPrint);
	DDX_Control(pDX, IDC_X, m_btnClose);
}

/////////////////////////////////////////////////////////////////////////////
// CADADlg message handlers

BOOL CADADlg::OnInitDialog() 
{
	BOOL ret = __super::OnInitDialog();

	try
	{
		// (j.dinatale 2010-07-23) - PLID 39692 - initialize printer context to null
		m_pPrintDC = NULL;

		// (j.dinatale 2010-07-28) - PLID 39803 - initialize the update flag to true, so that way if its not specified that we want to update, we assume that it wants to be done
		m_bUpdateClaimsTables = true;

		ShowScrollBar(SB_VERT, TRUE);

		/// ADA SPECIFIC STUFF ////////////
		{
			// (j.armen 2014-03-05 09:17) - PLID 60784 - Parameterized
			_RecordsetPtr prs = CreateParamRecordset(
				"SELECT\r\n"
				"	Description,\r\n"
				"	InsuredPartyID,\r\n"
				"	OthrInsuredPartyID\r\n"
				"FROM BillsT\r\n"
				"WHERE ID = {INT} AND PatientID = {INT} AND Deleted = 0\r\n",
				m_nBillID, m_nPatientID);

			if(!prs->eof) {
				m_InsuredPartyID = AdoFldLong(prs, "InsuredPartyID", -1);
				m_OthrInsuredPartyID = AdoFldLong(prs, "OthrInsuredPartyID", -1);
				m_strBillName = AdoFldString(prs, "Description");
			}
		}

		BuildADAChargesT();

		if(FindHCFABatch(m_nBillID) == 1) {
			((CButton*)GetDlgItem(IDC_RADIO_PAPER))->SetCheck(TRUE);
			((CButton*)GetDlgItem(IDC_RADIO_NO_BATCH))->SetCheck(FALSE);
		}
		else {
			((CButton*)GetDlgItem(IDC_RADIO_PAPER))->SetCheck(FALSE);
			((CButton*)GetDlgItem(IDC_RADIO_NO_BATCH))->SetCheck(TRUE);
		}

		//m_pPrePrintFunc = &PreADAPrint;

		m_CapOnPrint = GetRemotePropertyInt("CapitalizeHCFA",0);
		((CButton*)GetDlgItem(IDC_CAP_ON_PRINT))->SetCheck(m_CapOnPrint);
		//m_CapOnPrint = TRUE;

		//create frame

		//allocate pointers
		// (j.jones 2010-03-15 15:19) - PLID 37719 - removed unnecessary log
		//LogDetail("ADADlg: Creating frame");
		m_pframe.reset(new CFormDisplayDlg(this));
		m_pframe->color = RGB(100,100,100);

		RequeryHistoricalADAData();

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

		SetWindowText("ADA");

		int i = 187;

		bool bUseICD10 = ShouldUseICD10(m_InsuredPartyID, m_nBillID);

		// (j.armen 2014-03-05 09:17) - PLID 60784 - Moved from FormLayer.cpp, Removed Box19Value
		g_aryFormQueries[40].sql =
			"SELECT\r\n"
			"	Last + ', ' + First + ' ' + Left(Middle, 1) AS PatName,\r\n"
			"	Address1 + ' ' + Address2 AS PatAddress,\r\n"
			"	City + ', ' + State + ', ' + Zip AS PatCSZ,\r\n"
			"	EmployerAddress1 + ' ' + EmployerAddress2 + ' ' + EmployerCity + ' ' + EmployerState + ' ' + EmployerZip AS EmpAddress,\r\n"
			"	GetDate() AS TodaysDate,\r\n"
			"	PersonT.*,\r\n"
			"	PatientsT.*\r\n"
			"FROM PersonT\r\n"
			"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID";

		m_pframe->Load (40,
			FormatString(" WHERE PersonT.ID = %li", m_nPatientID),
			"", &i); // Lines, Labels, and Patient Information

		// (j.jones 2013-04-26 13:36) - PLID 56453 - Get the POS code, which is unfortunately
		// tied to charges despite being the same on all charges. So just get the first record.
		CString strPOSCode = "";
		{
			// (j.armen 2014-03-05 09:17) - PLID 60784 - Parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 BillsT.InsuredPartyID, BillsT.OthrInsuredPartyID, PlaceOfServiceCodesT.PlaceCodes "
				"FROM BillsT "
				"INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"LEFT JOIN PlaceOfServiceCodesT ON ChargesT.ServiceLocationID = PlaceOfServiceCodesT.ID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "		
				"WHERE BillsT.ID = {INT} AND LineItemT.Deleted = 0 AND ChargesT.Batched = 1 "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null", m_nBillID);
			if(!rs->eof) {
				m_InsuredPartyID = AdoFldLong(rs, "InsuredPartyID",-1);
				m_OthrInsuredPartyID = AdoFldLong(rs, "OthrInsuredPartyID",-1);
				strPOSCode = AdoFldString(rs, "PlaceCodes", "");
			}
		}

		m_pframe->Load (41,
			FormatString(" WHERE PersonT.ID = %li", m_InsuredPartyID),
			"", &i); // Primary Insured Party Information

		// (j.armen 2014-03-05 09:17) - PLID 60784 - Moved from FormLayer.cpp, Modified Box 4 values and when RespType is filled
		g_aryFormQueries[42].sql = 
			"SELECT\r\n"
			"	Last + ', ' + First + ' ' + Middle AS SecInsuredName,\r\n"
			"	Address1 + ' ' + Address2 AS SecInsuredAddress,\r\n"
			"	City + ', ' + State + ', ' + Zip AS SecInsuredCSZ,\r\n"
			"	PersonT.*,\r\n"
			"	InsuredPartyT.*,\r\n"
			"	InsurancePlansT.PlanName AS SecInsPlanName,\r\n"
			"	CASE WHEN RespTypeT.CategoryType = 5 /*Dental*/ THEN 1 END AS Box4Dental,\r\n"
			"	CASE WHEN ISNULL(RespTypeT.CategoryType, 5) <> 5 /*Not Dental, NOT NULL*/ THEN 1 END AS Box4Medical,\r\n"
			"	CASE WHEN RespTypeT.CategoryType IS NOT NULL THEN\r\n"
			"		CASE\r\n"
			"			WHEN RelationToPatient = 'Self' THEN 0\r\n"
			"			WHEN RelationToPatient = 'Spouse' THEN 1\r\n"
			"			WHEN RelationToPatient = 'Child' THEN 2\r\n"
			"			ELSE 3\r\n"
			"		END\r\n"
			"	END AS Box10Value\r\n"
			"FROM PersonT\r\n"
			"LEFT JOIN InsuredPartyT ON PersonT.ID = InsuredPartyT.PersonID\r\n"
			"LEFT JOIN InsurancePlansT ON InsuredPartyT.InsPlan = InsurancePlansT.ID\r\n"
			"LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID";

		m_pframe->Load (42, 
			FormatString(" WHERE PersonT.ID = %li", m_OthrInsuredPartyID),
			"", &i); // Secondary Insured Party Information

		m_pframe->Load (43,
			FormatString(" WHERE PersonT.ID = %li", adwProviderList[provider_index]),
			"", &i); // Provider Information

		// (j.jones 2008-10-28 14:14) - PLID 26526 - we have to split up the phone number
		// for the new ADA form
		//this is ugly, but it works, it replaces standard formatting 
		// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" charges
		// (j.armen 2014-03-05 09:17) - PLID 60784 - Fixed query to work with ICD10 data structure
		CString strLocationPhoneNumberOnly = "Replace(Replace(Replace(Replace(Phone, '-', ''), ')', ''), '(', ''), ' ', '')";
		g_aryFormQueries[44].sql.Format(
		"SELECT\r\n"
		"	BillsT.Date AS BillDate, LineItemT.Date AS ChargeDate, BillsT.State AS AccState,\r\n"
		"	BillsT.*, ChargesT.*,\r\n"
		"	LineItemT.Description, dbo.GetChargeTotal(ChargesT.ID) AS Amount,\r\n"
		"	CASE WHEN Len(%s) >= 10 THEN Left(%s, 3) ELSE '' END AS First3Phone,\r\n"
		"	CASE WHEN Len(%s) >= 10 THEN SubString(%s, 4, 3) WHEN Len(%s) >= 7 THEN Left(%s, 3) ELSE '' END AS Mid3Phone,\r\n"
		"	CASE WHEN Len(%s) >= 4 THEN Right(%s, 4) ELSE '' END AS Last4Phone,\r\n"
		"	LocationsT.Name AS LocName, LocationsT.Address1 + ' ' + LocationsT.Address2 AS LocAddress,\r\n"
		"	LocationsT.City + ', ' + LocationsT.State + ', ' + LocationsT.Zip AS LocCSZ,\r\n"
		"	STUFF((\r\n"
		"		SELECT TOP 4 ',' + CASE WHEN ChargeWhichCodesT.BillDiagCodeID IS NOT NULL THEN CASE ROW_NUMBER() OVER(ORDER BY BillDiagCodeT.OrderIndex) WHEN 1 THEN 'A' WHEN 2 THEN 'B' WHEN 3 THEN 'C' WHEN 4 THEN 'D' END END\r\n"
		"		FROM BillDiagCodeT\r\n"
		"		LEFT JOIN ChargeWhichCodesT ON ChargeWhichCodesT.BillDiagCodeID = BillDiagCodeT.ID AND ChargesT.ID = ChargeWhichCodesT.ChargeID\r\n"
		"		WHERE BillDiagCodeT.%s IS NOT NULL AND BillDiagCodeT.BillID = BillsT.ID\r\n"
		"		ORDER BY BillDiagCodeT.OrderIndex\r\n"
		"		FOR XML PATH('')\r\n"
		"		), 1, 1, '') AS DiagPtr\r\n"
		"FROM BillsT\r\n"
		"INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID\r\n"
		"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID\r\n"
		"INNER JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID\r\n"
		"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID\r\n"
		"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID\r\n",
		strLocationPhoneNumberOnly, strLocationPhoneNumberOnly, strLocationPhoneNumberOnly, strLocationPhoneNumberOnly,
		strLocationPhoneNumberOnly, strLocationPhoneNumberOnly, strLocationPhoneNumberOnly, strLocationPhoneNumberOnly,
		bUseICD10 ? "ICD10DiagID" : "ICD9DiagID");

		m_pframe->Load (44,
			FormatString(" WHERE LineItemT.Deleted = 0 AND Batched = 1 "
			"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
			"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
			"AND BillsT.ID = %li", m_nBillID),
			" ORDER BY LineID", &i); // Bill/Charge Information

		m_pframe->Load (45, 
			FormatString(" WHERE LineItemT.Deleted = 0 AND Batched = 1 "
			"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
			"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
			"AND BillsT.ID = %li", m_nBillID),
			"", &i); // Bill/Charge Totals

		// (j.jones 2008-10-28 14:14) - PLID 26526 - we have to split up the phone number
		// for the new ADA form
		// (j.jones 2013-04-26 13:36) - PLID 56453 - we now send the patient address as the POS
		//if its POS code is 12 and the Claim_SendPatAddressWhenPOS12 preference is on
		if(strPOSCode == "12" && GetRemotePropertyInt("Claim_SendPatAddressWhenPOS12", 1, 0, "<None>", true) == 1) {
			//use the patient's home address
			CString strPatientPhoneNumberOnly = "Replace(Replace(Replace(Replace(HomePhone, '-', ''), ')', ''), '(', ''), ' ', '')";
			g_aryFormQueries[46].sql.Format(
				"SELECT\r\n"
				"	'Patient Home' AS POSName,\r\n"
				"	PersonT.Address1 + ' ' + PersonT.Address2 AS POSAddress,\r\n"
				"	PersonT.City + ', ' + PersonT.State + ', ' + PersonT.Zip AS POSCSZ,\r\n"
				"	CASE WHEN Len(%s) >= 10 THEN Left(%s, 3) ELSE '' END AS First3Phone,\r\n"
				"	CASE\r\n"
				"		WHEN Len(%s) >= 10 THEN SubString(%s, 4, 3)\r\n"
				"		WHEN Len(%s) >= 7 THEN Left(%s, 3)\r\n"
				"		ELSE ''\r\n"
				"	END AS Mid3Phone,\r\n"
				"	CASE WHEN Len(%s) >= 4 THEN Right(%s, 4) ELSE '' END AS Last4Phone,\r\n"
				"	'' AS LocNPI,\r\n"
				"	'' AS LocEIN,\r\n"
				"	'%s' AS POSCode\r\n"
				"FROM PersonT\r\n"
				"INNER JOIN BillsT ON PersonT.ID = BillsT.PatientID",
				strPatientPhoneNumberOnly, strPatientPhoneNumberOnly, strPatientPhoneNumberOnly, strPatientPhoneNumberOnly,
				strPatientPhoneNumberOnly, strPatientPhoneNumberOnly, strPatientPhoneNumberOnly, strPatientPhoneNumberOnly,
				_Q(strPOSCode));
		}
		else {
			//use the actual place of service
			g_aryFormQueries[46].sql.Format(
				"SELECT\r\n"
				"	LocationsT.Name AS POSName,\r\n"
				"	LocationsT.Address1 + ' ' + LocationsT.Address2 AS POSAddress,\r\n"
				"	LocationsT.City + ', ' + LocationsT.State + ', ' + LocationsT.Zip AS POSCSZ,\r\n"
				"	CASE WHEN Len(%s) >= 10 THEN Left(%s, 3) ELSE '' END AS First3Phone,\r\n"
				"	CASE\r\n"
				"		WHEN Len(%s) >= 10 THEN SubString(%s, 4, 3)\r\n"
				"		WHEN Len(%s) >= 7 THEN Left(%s, 3)\r\n"
				"		ELSE ''\r\n"
				"	END AS Mid3Phone,\r\n"
				"	CASE WHEN Len(%s) >= 4 THEN Right(%s, 4) ELSE '' END AS Last4Phone,\r\n"
				"	LocationsT.NPI AS LocNPI,\r\n"
				"	LocationsT.EIN AS LocEIN,\r\n"
				"	'%s' AS POSCode\r\n"
				"FROM LocationsT\r\n"
				"INNER JOIN BillsT ON LocationsT.ID = BillsT.Location\r\n",
				strLocationPhoneNumberOnly, strLocationPhoneNumberOnly, strLocationPhoneNumberOnly, strLocationPhoneNumberOnly,
				strLocationPhoneNumberOnly, strLocationPhoneNumberOnly, strLocationPhoneNumberOnly, strLocationPhoneNumberOnly,
				_Q(strPOSCode));
		}

		m_pframe->Load (46, 
			FormatString(" WHERE BillsT.ID = %li", m_nBillID),
			"", &i); // Place Of Service Info

		m_pframe->Load (47, 
			FormatString(" WHERE InsuredPartyT.PersonID = %li", m_InsuredPartyID),
			"", &i); // Primary Insurance Company Information

		// (j.jones 2008-10-28 11:41) - PLID 26526 - added for the new ADA form
		m_pframe->Load (48, 
			FormatString(" WHERE InsuredPartyT.PersonID = %li", m_OthrInsuredPartyID),
			"", &i); // Secondary Insurance Company Information

		// (j.armen 2014-03-05 09:17) - PLID 60784 - Query selects the diag codes and whether we are using ICD9 or 10
		if (bUseICD10)
		{
			g_aryFormQueries[49].sql = 
				"SELECT TOP 4\r\n"
				"	'A B' AS DiagCodeListQualifier,\r\n"
				"	CodeNumber\r\n"
				"FROM BillDiagCodeT\r\n"
				"INNER JOIN DiagCodes ON BillDiagCodeT.ICD10DiagID = DiagCodes.ID\r\n";
		}
		else
		{
			g_aryFormQueries[49].sql = 
				"SELECT TOP 4\r\n"
				"	'   B' AS DiagCodeListQualifier,\r\n"
				"	CodeNumber\r\n"
				"FROM BillDiagCodeT\r\n"
				"INNER JOIN DiagCodes ON BillDiagCodeT.ICD9DiagID = DiagCodes.ID\r\n";
		}

		m_pframe->Load(49,
			FormatString(" WHERE BillDiagCodeT.BillID = %li", m_nBillID),
			" ORDER BY BillDiagCodeT.OrderIndex", &i);

		#define scale * 19 / 20

		//set printer settings
		PRINT_X_SCALE	= 15.1;
		PRINT_Y_SCALE	= 15.1;
		PRINT_X_OFFSET	= -250;
		PRINT_Y_OFFSET	= 20;

		m_pframe->m_pOnCommand = m_pOnCommand;
		m_pframe->m_pOnKeyDown = m_pOnKeyDown;

		m_pframe->TrackChanges(0, 0);

	}NxCatchAll(__FUNCTION__);

	return ret;
}

// (j.armen 2012-06-25 15:02) - PLID 50779 - This is still needed here as this is not a CNxDialog.  Added Try/Catch.
void CADADlg::OnSize(UINT nType, int cx, int cy) 
{
	try {
		CDialog::OnSize(nType, cx, cy);
		SetControlPositions();
	}NxCatchAll(__FUNCTION__);
}

void CADADlg::SetControlPositions(void)
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

void CADADlg::OnClickX()
{
	EndDialog (0);
}

void CADADlg::OnClickCheck() 
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

extern int FONT_SIZE;
extern int MINI_FONT_SIZE;
void CADADlg::OnClickPrint() 
{
	// Get the alignment settings
	int x_offset = 0, y_offset = 0, x_scale = 0, y_scale = 0;
	FONT_SIZE = 12;
	MINI_FONT_SIZE = 10;

	m_pframe->m_printDlg = m_printDlg;

	try {

		// (j.jones 2007-04-25 11:43) - PLID 4758 - Handle having a default per workstation,
		// defaulting to the FormAlignT default printer if the per-workstation default doesn't exist.
		long nDefault = GetPropertyInt("DefaultFormAlignID", -1, 2, FALSE);
		CSqlFragment sqlDefault("[Default] = 1");
		if(nDefault != -1)
			sqlDefault.Create("ID = {INT}", nDefault);

		{
			// (j.armen 2014-03-05 09:17) - PLID 60784 - Parameterized
			_RecordsetPtr prs = CreateParamRecordset(
				"SELECT * FROM FormAlignT WHERE {SQL} AND FormID = {INT}", sqlDefault, 2);
			if(!prs->eof) 
			{
				x_offset = AdoFldLong(prs, "XOffset");
				y_offset = AdoFldLong(prs, "YOffset");
				x_scale = AdoFldLong(prs, "XScale");
				y_scale = AdoFldLong(prs, "YScale");
				FONT_SIZE = AdoFldLong(prs, "FontSize");

				// (j.jones 2005-01-31 15:06) - force it to be no greater than 14
				if(FONT_SIZE > 14)
					FONT_SIZE = 14;

				MINI_FONT_SIZE = AdoFldLong(prs, "MiniFontSize");
			}
			else {
				if(m_bPrintWarnings && IDYES == MessageBox("You have not saved this form's alignment settings. The printout may not be aligned correctly until you do this.\n"
					"To set this up properly, click \"Yes\" and then click on the \"Align Form\" button. Create a printer, and check the \"Default For My Workstation\" box.\n"
					"Then configure the alignment as needed. Would you like to cancel printing and align the form now?","Practice",MB_ICONEXCLAMATION|MB_YESNO)) {
					return;
				}
			}
		}

		PRINT_X_OFFSET += x_offset * 10;
		PRINT_Y_OFFSET += y_offset * 10;
		PRINT_X_SCALE += x_scale * 0.01;
		PRINT_Y_SCALE += y_scale * 0.01;
		// (j.dinatale 2010-07-23) - PLID 39692 - pass in the print context to the onprint function, and in this case no IDs to ignore exist
		if(!m_pframe->OnPrint(m_CapOnPrint, "ADA", NULL, m_pPrintDC)) {
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

		//Send Type 3 - Paper ADA Form

		// (j.jones 2013-01-23 09:19) - PLID 54734 - moved the claim history addition to its own function,
		// this also updates HCFATrackT.LastSendDate
		AddToClaimHistory(m_nBillID, m_InsuredPartyID, ClaimSendType::ADA);
		
		//now add to patient history
		CString strDesc = "ADA Form Printed";
		long nPatientID;
		//get ins. co. name
		{
			// (j.armen 2014-03-05 09:17) - PLID 60784 - Parameterized
			_RecordsetPtr prs = CreateParamRecordset(
				"SELECT\r\n"
				"	InsuranceCoT.Name,\r\n"
				"	InsuredPartyT.PatientID\r\n"
				"FROM InsuranceCoT\r\n"
				"INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID\r\n"
				"WHERE InsuredPartyT.PersonID = {INT}",
				m_InsuredPartyID);
			if(!prs->eof) {
				strDesc += FormatString(" - Sent To %s", AdoFldString(prs, "Name"));
				nPatientID = AdoFldLong(prs, "PatientID");
			}
		}

		{
			// (j.armen 2014-03-05 09:17) - PLID 60784 - Parameterized
			_RecordsetPtr prs = CreateParamRecordset(
				"SELECT TOP 1\r\n"
				"	LineItemT.Date\r\n"
				"FROM LineItemT\r\n"
				"INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID\r\n"
				"WHERE ChargesT.BillID = {INT}",
				m_nBillID);
			if(!prs->eof) {
				strDesc += FormatString("  Service Date: %s",FormatDateTimeForInterface(AdoFldDateTime(prs, "Date")));
			}
		}

		long Loc = GetCurrentLocation();

		// (j.jones 2007-02-20 09:13) - PLID 24790 - converted the insert to include the NewNumber in the batch,
		// then return the new ID so we can send a tablechecker
		// (j.jones 2008-09-04 13:31) - PLID 30288 - converted to use CreateNewMailSentEntry,
		// which creates the data in one batch and sends a tablechecker
		// (c.haag 2010-01-28 11:00) - PLID 37086 - Removed COleDateTime::GetCurrentTime as the service date; should be the server's time.
		CreateNewMailSentEntry(nPatientID, strDesc, m_strBillName, "", GetCurrentUserName(), "", Loc);
		// (j.dinatale 2010-07-28) - PLID 39803 - removed the other catch that was in here, if theres an exception in the actual printing, we dont want to update the claimshistory
	}NxCatchAll("Error printing ADA form.");	
}

void CADADlg::BuildADAChargesT()
{
	extern int SizeOfFormChargesT();

	try {

		// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" charges
		g_aryFormQueries[2].sql.Format("SELECT ChargesT.*, LineItemT.[Date] AS TDate, LineItemT.Description AS ItemDesc "
			"FROM BillsT INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
			"WHERE LineItemT.Deleted = 0 AND Batched = 1 "
			"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
			"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
			"AND BillsT.ID = %li", m_nBillID);

		// Now build a list of different charge providers. Assume every charge
		// has a provider in FormChargesT.

		firstcharge = 0;

		adwProviderList.RemoveAll();
		// (j.jones 2006-12-06 10:40) - PLID 23734 - supported ClaimProviderID override
		// (j.jones 2007-02-22 11:33) - PLID 24878 - fixed ClaimProviderID usage such that -1 was still returned if there was no provider on a charge
		// (j.jones 2010-11-09 14:42) - PLID 41387 - supported ChargesT.ClaimProviderID
		// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" charges
		{
			_RecordsetPtr prs = CreateParamRecordset("SELECT Coalesce(ChargesT.ClaimProviderID, Coalesce(ClaimProvidersT.PersonID, -1)) AS ClaimProviderID FROM ChargesT "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"LEFT JOIN ProvidersT ON ChargesT.DoctorsProviders = ProvidersT.PersonID "
				"LEFT JOIN ProvidersT ClaimProvidersT ON ProvidersT.ClaimProviderID = ClaimProvidersT.PersonID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				"WHERE ChargesT.BillID = {INT} AND LineItemT.Deleted = 0 AND Batched = 1 "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null", m_nBillID);
			for(; !prs->eof; prs->MoveNext()) {
				int i, iProviderID = AdoFldLong(prs, "ClaimProviderID");
				for (i=0; i < adwProviderList.GetSize(); i++) {
					if ((int)adwProviderList[i] == iProviderID)
						break;
				}
				if (i == adwProviderList.GetSize())
					adwProviderList.Add(iProviderID);
			}	
		}

		if(adwProviderList.GetSize() == 0) {
			adwProviderList.Add((long)-1);
		}
		
		// Rebuild the charge list with the first provider if there is more than one
		provider_index = 0;
		if (adwProviderList.GetSize() > 0) {

				//JJ - 9/27/01 - While this works fine, we can't switch pages to show charges for different providers
				//TODO - comment back in when we can switch pages
				//g_aryFormQueries[2].sql.Format("SELECT ChargesT.*, LineItemT.[Date] AS TDate, LineItemT.Description AS ItemDesc FROM BillsT INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID WHERE LineItemT.Deleted = 0 AND Batched = 1 AND BillsT.ID = %li AND DoctorsProviders = %li", billid,adwProviderList.GetAt(0));
		}

		// Fill total charge amount for each line
		nTotalCharges = SizeOfFormChargesT();

	} NxCatchAll("ADADlg::BuildADAChargesT()");
}

void OnADACommand(WPARAM wParam, LPARAM lParam, CDWordArray& dwChangedForms, int FormControlsID, CFormDisplayDlg* dlg) 
{
}

void OnADAKeyDown(CDialog* pFormDisplayDlg, MSG* pMsg)
{
	CFormDisplayDlg* pDlg = (CFormDisplayDlg*)pFormDisplayDlg;
	CWnd* pCtrl = pDlg->GetFocus();
}

void CADADlg::OnClickAlign() 
{
	CPrintAlignDlg dlg(this);
	dlg.m_FormID = 2;
	dlg.DoModal();
}

void CADADlg::OnClickCapitalizeOnPrint() 
{
	if(m_CapOnPrint) {
		SetRemotePropertyInt("CapitalizeHCFA",0);
	}
	else {
		SetRemotePropertyInt("CapitalizeHCFA",1);
	}
	m_CapOnPrint = GetRemotePropertyInt("CapitalizeHCFA",0);
}

void CADADlg::OnClickRestore() 
{
	try
	{
		if(!CheckCurrentUserPermissions(bioClaimForms,sptWrite))
			return;

		if (IDNO == MessageBox("All changes you have made to the ADA form will be unrecoverable. Are you sure you wish to do this?", NULL, MB_YESNO))
			return;

		CWaitCursor pWait;

		try {

			// (j.jones 2008-05-28 15:46) - PLID 27881 - batched statements, ensured we delete all relevant FormHistory entries
			// (j.armen 2014-03-05 09:17) - PLID 60784 - Parameterized
			CParamSqlBatch sqlBatch;
			sqlBatch.Add("DELETE FROM FormHistoryT WHERE DocumentID IN (SELECT FirstDocumentID FROM HCFADocumentsT WHERE BillID = {INT} AND FormType = {INT})", m_nBillID, 2);
			sqlBatch.Add("DELETE FROM HCFADocumentsT WHERE HCFADocumentsT.BillID = {INT} AND FormType = {INT}", m_nBillID, 2);
			sqlBatch.Execute(GetRemoteData());

			// Remove all changes made by the user
			m_pframe->UndoChanges();

		} NxCatchAll("Error restoring defaults.");

		//Refresh the form

		m_pframe->Refresh(40);  // Lines, Labels, and Patient Information
		m_pframe->Refresh(41);  // Primary Insured Party Information
		m_pframe->Refresh(42);  // Secondary Insured Party Information
		m_pframe->Refresh(43);  // Provider Information
		m_pframe->Refresh(44);  // Bill/Charge Information
		m_pframe->Refresh(45);  // Bill Totals
		m_pframe->Refresh(46);  // Place Of Service Info
		m_pframe->Refresh(47);  // Primary Insurance Company Info
		m_pframe->Refresh(48);  // Secondary Insurance Company Info

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
	}NxCatchAll(__FUNCTION__);
}

void CADADlg::Save(BOOL boSaveAll, CDialog* pdlgWait, int& iPage)
{
	//int provider;

	// (j.jones 2008-10-28 11:02) - PLID 26526 - we do not support multiple pages right now
	//for (provider_index=0; provider_index < adwProviderList.GetSize(); provider_index++) {

		//provider = adwProviderList.GetAt(provider_index);

		// (j.jones 2008-10-28 11:02) - PLID 26526 - we do not support multiple pages right now
		//for (firstcharge=0; firstcharge < nTotalCharges; firstcharge += 10) {

			if (pdlgWait) {
				CString str;
				str.Format("Saving ADA page %d...", ++iPage);
				pdlgWait->GetDlgItem(IDC_LABEL)->SetWindowText(str);
				pdlgWait->RedrawWindow();
			}

			SaveHistoricalData(boSaveAll);
		//}
	//}
}

void CADADlg::SaveHistoricalData(BOOL boSaveAll)
{
#ifdef SAVE_HISTORICAL_INFORMATION

	int iDocumentID;
	try {
		// (j.armen 2014-03-05 09:17) - PLID 60784 - Parameterized
		_RecordsetPtr prs = CreateParamRecordset(
			"SELECT\r\n"
			"	FirstDocumentID\r\n"
			"FROM HCFADocumentsT\r\n"
			"WHERE BillID = {INT}\r\n"
			"	AND FirstCharge = {INT}\r\n"
			"	AND ProviderIndex = {INT}\r\n"
			"	AND FormType = {INT}",
			m_nBillID, firstcharge, adwProviderList[provider_index], 2);
		if (!prs->eof) {
			iDocumentID = AdoFldLong(prs, "FirstDocumentID");
		}
		else {
			iDocumentID = NewNumber("FormHistoryT","DocumentID");
			//BEGIN_TRANS("SaveHistoricalHCFA") {
			// (j.armen 2014-03-05 09:17) - PLID 60784 - Parameterized
			ExecuteParamSql(
				"INSERT INTO HCFADocumentsT (BillID, FirstDocumentID, FirstCharge, ProviderIndex, FormType)\r\n"
				"	VALUES ({INT}, {INT}, {INT}, {INT}, {INT})", m_nBillID, iDocumentID, firstcharge, adwProviderList[provider_index], 2);
			ExecuteParamSql(
				"INSERT INTO FormHistoryT (DocumentID, ControlID, [Value])\r\n"
				"VALUES ({INT}, -1, NULL)", iDocumentID);
			//} END_TRANS_CATCH_ALL("SaveHistoricalHCFA");
		}		
	}
	NxCatchAll("Error in OnClickCheck()");

	///////////////////////////////////
	// Make sure we pull our historical data from the right table
	m_pframe->SetDocumentID(iDocumentID);

	m_pframe->Refresh(40);  // Lines, Labels, and Patient Information
	m_pframe->Refresh(41);  // Primary Insured Party Information
	m_pframe->Refresh(42);  // Secondary Insured Party Information
	m_pframe->Refresh(43);  // Provider Information
	m_pframe->Refresh(44);  // Bill/Charge Information
	m_pframe->Refresh(45);  // Bill Totals
	m_pframe->Refresh(46);  // Place Of Service Info
	m_pframe->Refresh(47);  // Primary Insurance Company Info

	m_pframe->ReapplyChanges(firstcharge, provider_index);

	m_pframe->Save (firstcharge, provider_index, iDocumentID, boSaveAll);

	// We save form 6 (zero based in the array) because where the radio button is
	// determines what text goes in a particular control. To save us time and pain,
	// we always save the data in form 6.
	//((FormLayer*)m_pframe->m_LayerArray[5])->Save(iDocumentID, 0, &m_pframe->m_ControlArray);
#endif
}

long CADADlg::DoScrollTo(long nNewTopPos)
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

void CADADlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
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

BOOL PreADAPrint(CDWordArray& dwChangedForms, CFormDisplayDlg* dlg)
{
	return TRUE;
}

void CADADlg::OnRadioPaper()
{
	try
	{
		if(!CheckCurrentUserPermissions(bioClaimForms,sptWrite)) {
			if(FindHCFABatch(m_nBillID) == 1) {
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
		COleCurrency cyCharges;
		{
			// (j.armen 2014-03-05 09:17) - PLID 60784 - Parameterized
			_RecordsetPtr prs = CreateParamRecordset(
				"SELECT\r\n"
				"	dbo.GetClaimTotal(ID) AS Total\r\n"
				"FROM BillsT\r\n"
				"WHERE ID = {INT}",
				m_nBillID);
			cyCharges = prs->eof ? COleCurrency(0,0) : AdoFldCurrency(prs, "Total", COleCurrency(0,0));
		}

		if (((CButton*)GetDlgItem(IDC_RADIO_PAPER))->GetCheck()) {

			CString strWarn;
			strWarn.Format("This claim is currently %s. Are you sure you wish to send the claim to the paper batch?",
				FormatCurrencyForInterface(cyCharges,TRUE,TRUE));

			if(cyCharges > COleCurrency(0,0) || 
				IDYES == MessageBox(strWarn,"Practice",MB_YESNO|MB_ICONQUESTION)) {

				short nBatch = 0;

				if (((CButton*)GetDlgItem(IDC_RADIO_PAPER))->GetCheck())
					nBatch = 1; // paper batch

				BatchBill(m_nBillID, 1);
			}
			else {
				
				if(FindHCFABatch(m_nBillID) == 1) {
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
			BatchBill(m_nBillID, 0);
		}
	}NxCatchAll(__FUNCTION__);
}

void CADADlg::OnRadioNoBatch()
{
	OnRadioPaper();
}

// (j.jones 2007-06-22 13:23) - PLID 25665 - handle when a field was edited, so we update the info text box
BOOL CADADlg::PreTranslateMessage(MSG *pMsg) {

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
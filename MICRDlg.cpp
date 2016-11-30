// MICRDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MICRDlg.h"
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

// MICR stuff

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

BOOL PreMICRPrint(CDWordArray& dwChangedForms, CFormDisplayDlg* dlg);

void RequeryHistoricalMICRData()
{
	try {
		_RecordsetPtr rs(__uuidof(Recordset));
		COleVariant var;
		CString str;

		str.Format("SELECT FirstDocumentID FROM HCFADocumentsT WHERE BillID = %li AND FirstCharge = %li AND ProviderIndex = %li AND FormType = 5", billid, firstcharge, adwProviderList.GetAt(provider_index));
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
// CMICRDlg dialog

BEGIN_MESSAGE_MAP(CMICRDlg, CDialog)
	//{{AFX_MSG_MAP(CMICRDlg)
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

BEGIN_EVENTSINK_MAP(CMICRDlg, CDialog)
    //{{AFX_EVENTSINK_MAP(CMICRDlg)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void OnMICRCommand(WPARAM wParam, LPARAM lParam, CDWordArray& dwChangedForms, int FormControlsID, CFormDisplayDlg* dlg);
void OnMICRKeyDown(CDialog* pFormDisplayDlg, MSG* pMsg);

CMICRDlg::CMICRDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMICRDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMICRDlg)
		m_bPrintWarnings = TRUE;
	//}}AFX_DATA_INIT
	m_pframe = 0;
	m_ID = -1;
	m_InsuredPartyID = -1;
	m_OthrInsuredPartyID = -1;

	m_pOnCommand = &OnMICRCommand;
	m_pOnKeyDown = &OnMICRKeyDown;
	m_pPrePrintFunc = NULL;
	m_ShowWindowOnInit = TRUE;
}

CMICRDlg::~CMICRDlg()
{
	delete m_pframe;
//	delete m_pleftbutton;
//	delete m_prightbutton;
//	delete m_pupbutton;
//	delete m_pdownbutton;
}

void CMICRDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMICRDlg)
	// (j.jones 2007-06-25 10:14) - PLID 25663 - changed the buttons to NxIconButtons
	DDX_Control(pDX, IDC_RESTORE, m_btnRestoreDefaults);
	DDX_Control(pDX, IDC_ALIGN, m_btnAlignForm);
	DDX_Control(pDX, IDC_CHECK, m_btnSave);
	DDX_Control(pDX, IDC_PRINT, m_btnPrint);
	DDX_Control(pDX, IDC_X, m_btnClose);
	//}}AFX_DATA_MAP
}

int CMICRDlg::DoModal(int billID) 
{
	m_ID = billID;
	return CDialog::DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CMICRDlg message handlers

BOOL CMICRDlg::OnInitDialog() 
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

	/// MICR SPECIFIC STUFF ////////////

	// (j.jones 2013-04-26 13:36) - PLID 56453 - Get the POS code, which is unfortunately
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

	BuildMICRChargesT();

	if(FindHCFABatch(m_ID) == 1) {
		((CButton*)GetDlgItem(IDC_RADIO_PAPER))->SetCheck(TRUE);
		((CButton*)GetDlgItem(IDC_RADIO_NO_BATCH))->SetCheck(FALSE);
	}
	else {
		((CButton*)GetDlgItem(IDC_RADIO_PAPER))->SetCheck(FALSE);
		((CButton*)GetDlgItem(IDC_RADIO_NO_BATCH))->SetCheck(TRUE);
	}

	//m_pPrePrintFunc = &PreMICRPrint;

	m_CapOnPrint = GetRemotePropertyInt("CapitalizeMICR",0);
	((CButton*)GetDlgItem(IDC_CAP_ON_PRINT))->SetCheck(m_CapOnPrint);
	//m_CapOnPrint = TRUE;

	//create frame

	//allocate pointers
	// (j.jones 2010-03-15 15:19) - PLID 37719 - removed unnecessary log
	//LogDetail("MICRDlg: Creating frame");
	m_pframe		= new CFormDisplayDlg(this);
	m_pframe->color = RGB(80,80,80);
	g_pFrame = m_pframe;
//	m_pleftbutton	= new CButton;
//	m_prightbutton	= new CButton;
//	m_pupbutton		= new CButton;
//	m_pdownbutton	= new CButton;

	RequeryHistoricalMICRData();

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

	SetWindowText("MICR");

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
	g_aryFormQueries[70].sql.Format("SELECT First + (CASE WHEN Middle <> '' AND Middle <> ' ' THEN ' ' + Left(Middle,1) ELSE '' END)  + ' ' + Last AS PatName, Address1 + ' ' + Address2 AS Address, City + ', ' + State + ' ' + Zip AS CityStateZip, "
				"Address1 + ' ' + Address2 + ', ' + City + ', ' + State + ' ' + Zip AS PatFullAddress, "
				"EmployerAddress1 + ' ' + EmployerAddress2 + ', ' + EmployerCity + ', ' + EmployerState + ' ' + EmployerZip AS EmpAddress, PersonT.*, PatientsT.*, %s AS Age, 'Signature On File' AS SigOnFile, GetDate() AS Date "
				"FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID",strAge);

	where.Format (" WHERE PersonT.ID = %li", m_PatientID);
	m_pframe->Load (70, where, "", &i); // Lines, Labels, and Patient Information

	where.Format (" WHERE InsuredPartyT.PersonID = %li", m_InsuredPartyID);
	m_pframe->Load (71, where, "", &i); // Primary Insurance Co Info

	CString strFacilityID = "";

	// (j.jones 2013-04-26 15:04) - PLID 56453 - we now send the patient address as the POS
	//if its POS code is 12 and the Claim_SendPatAddressWhenPOS12 preference is on
	BOOL bUsePatientAddressASPOS = FALSE;
	if(strPOSCode == "12" && GetRemotePropertyInt("Claim_SendPatAddressWhenPOS12", 1, 0, "<None>", true) == 1) {
		bUsePatientAddressASPOS = TRUE;
	}

	if(!bUsePatientAddressASPOS) {
		rs = CreateParamRecordset("SELECT FacilityID FROM InsuranceFacilityID "
			"INNER JOIN InsuredPartyT ON InsuranceFacilityID.InsCoID = InsuredPartyT.InsuranceCoID "
			"INNER JOIN BillsT ON InsuredPartyT.PersonID = BillsT.InsuredPartyID "
			"WHERE InsuranceFacilityID.LocationID = BillsT.Location AND BillsT.ID = {INT}", m_ID);

		if(!rs->eof) {
			strFacilityID = AdoFldString(rs, "FacilityID","");
		}
		rs->Close();
	}

	CString strPOSFields;
	if(bUsePatientAddressASPOS) {
		//use the patient's address for the POS fields
		strPOSFields = "'Patient Home' + ', ' + PatientPersonT.Address1 + CASE WHEN PatientPersonT.Address2 <> '' THEN ', ' + PatientPersonT.Address2 ELSE '' END + ', ' + PatientPersonT.City + ', ' + PatientPersonT.State + ' ' + PatientPersonT.Zip AS POSNameAdd, "
			"PatientPersonT.Zip AS POSZip, '' AS FacilityCode, PlaceOfServiceCodesT.PlaceCodes AS ServiceLocation ";
	}
	else {
		//use the POS address for the POS fields
		strPOSFields.Format("LocationsT.Name + ', ' + LocationsT.Address1 + CASE WHEN LocationsT.Address2 <> '' THEN ', ' + LocationsT.Address2 ELSE '' END + ', ' + LocationsT.City + ', ' + LocationsT.State + ' ' + LocationsT.Zip AS POSNameAdd, "
			"LocationsT.Zip AS POSZip, '%s' AS FacilityCode, PlaceOfServiceCodesT.PlaceCodes AS ServiceLocation ", _Q(strFacilityID));
	}

	// (j.jones 2005-04-13 16:14) - PLID 16181 - The rather complicated DiagCode calculation is so that the
	// Diagnosis Code used on each line represents the first code in the WhichCodes sequence for that charge.
	// It also supports when they select the code itself as the WhichCodes.
	// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" charges
	// (a.walling 2014-03-06 13:37) - PLID 61230 - Which codes has changed, using that structure, which also handles the BillDiagCodeT changes
	// (a.walling 2014-03-17 17:26) - PLID 61406 - Unique diag codes and recalculated WhichCodes using BillDiagsXmlFlat4IF and ChargeWhichCodesIF - MICR, MICR2007
	// (r.gonet 2016-04-12) - NX-100158 - Removed HasFirstCond. The BillsT.FirstConditionDate was replaced with multiple columns and HasFirstCond was
	// unused on the MICR forms anyway.
	g_aryFormQueries[72].sql.Format("SELECT BillsT.Date AS BillDate, LineItemT.Date AS ChargeDate, "
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
		") AS DiagCode, "
		"BillsT.*, ChargesT.*, LineItemT.Date, LineItemT.Description, dbo.GetChargeTotal(ChargesT.ID) AS Amount, "
		"(PersonT.[Last] + ', ' + PersonT.[First] + ' ' + PersonT.Middle + (CASE WHEN (PersonT.Title <> '') THEN (', ' + PersonT.Title) ELSE ('') END)) AS ReferringName, ReferringPhysT.*, "
		"%s "
		"FROM BillsT INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
		"INNER JOIN PersonT PatientPersonT ON BillsT.PatientID = PatientPersonT.ID "		
		"OUTER APPLY dbo.%s(BillsT.ID) BillDiagCodeFlatV "
		"LEFT JOIN PlaceOfServiceCodesT ON ChargesT.ServiceLocationID = PlaceOfServiceCodesT.ID "
		"LEFT JOIN PersonT ON BillsT.RefPhyID = PersonT.ID LEFT JOIN ReferringPhysT ON PersonT.ID = ReferringPhysT.PersonID "
		"LEFT JOIN LocationsT ON BillsT.Location = LocationsT.ID "
		"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
		"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
		, ShouldUseICD10(m_InsuredPartyID, m_ID) ? "ICD10DiagID" : "ICD9DiagID"
		, strPOSFields
		, ShouldUseICD10(m_InsuredPartyID, m_ID) ? "BillICD10DiagsXmlFlat4IF" : "BillICD9DiagsXmlFlat4IF"
	);

	where.Format (" WHERE LineItemT.Deleted = 0 AND Batched = 1 "		
		"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
		"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
		"AND BillsT.ID = %li", billid);
	m_pframe->Load (72, where, " ORDER BY LineID", &i); // Bill/Charge Information

	where.Format (" WHERE PersonT.ID = %li AND LineItemT.Deleted = 0 AND Batched = 1 "
		"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
		"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
		"AND ChargesT.BillID = %li", adwProviderList.GetAt(provider_index), billid);
	m_pframe->Load (73, where, "", &i); // Provider Information

	//m_pframe->Load (74, "", &i); // Charge Totals
	//m_pframe->Load (75, "", &i); // Payment Totals
	//m_pframe->Load (76, "", &i); // Balance

	//manually check the BCBS box and REJ. box
	// (j.armen 2014-03-27 16:28) - PLID 60784 - control array is now a vector
	for (unsigned int j = 0; j < m_pframe->m_ControlArray.size(); j++) {
		FormControl* pCtrl = m_pframe->m_ControlArray[j].get();
		if (pCtrl->id == 5213 || pCtrl->id == 5218) {
			FormCheck* pCheck = (FormCheck*)pCtrl;
			pCheck->SetCheck(1);

			//make sure a saved version of this form didn't change the checkbox
			for (unsigned int k = 0; k < m_pframe->m_ControlArray.size(); k++) {
				FormControl* pCtrl2 = m_pframe->m_ControlArray[k].get();
				if((pCtrl->id == 5213 && (pCtrl2->id == 5214 || pCtrl2->id == 5215 || pCtrl2->id == 5216)) ||
					(pCtrl->id == 5218 && (pCtrl2->id == 5217 || pCtrl2->id == 5219 || pCtrl2->id == 5220))) {
					FormCheck* pCheck2 = (FormCheck*)pCtrl2;
					if(pCheck2->GetCheck()) {
						pCheck->SetCheck(0);
					}
				}
			}			
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

	//set printer settings
	PRINT_X_SCALE	= 15.1;
	PRINT_Y_SCALE	= 15.1;
	PRINT_X_OFFSET	= -250;
	PRINT_Y_OFFSET	= 20;

	m_pframe->m_pOnCommand = m_pOnCommand;
	m_pframe->m_pOnKeyDown = m_pOnKeyDown;

	m_pframe->TrackChanges(0, 0);

	return TRUE;
}

void CMICRDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	SetControlPositions();
}

void CMICRDlg::SetControlPositions(void)
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

void CMICRDlg::OnClickX()
{
	EndDialog (0);
}

void CMICRDlg::OnClickCheck() 
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

void CMICRDlg::OnClose() 
{
	CDialog::OnClose();
}

BOOL CMICRDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	return CDialog::OnCommand(wParam, lParam);
}

extern int FONT_SIZE;
extern int MINI_FONT_SIZE;
void CMICRDlg::OnClickPrint() 
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
		long nDefault = GetPropertyInt("DefaultFormAlignID", -1, 5, FALSE);
		CString strDefault = "[Default] = 1";
		if(nDefault != -1)
			strDefault.Format("ID = %li", nDefault);

		rs = CreateRecordset("SELECT * FROM FormAlignT WHERE %s AND FormID = 5", strDefault);
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
		// (j.dinatale 2010-07-23) - PLID 39692 - pass in the print context to the onprint function, and in this case no IDs to ignore exist
		if(!m_pframe->OnPrint(m_CapOnPrint, "MICR", NULL, m_pPrintDC)) {
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
	
		// (j.jones 2013-01-23 09:19) - PLID 54734 - moved the claim history addition to its own function,
		// this also updates HCFATrackT.LastSendDate
		AddToClaimHistory(m_ID, m_InsuredPartyID, ClaimSendType::MICR);
		
		//now add to patient history
		CString str, strDesc = "MICR Form Printed";
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
	}NxCatchAll("Error printing MIRC form.");	
}

void CMICRDlg::BuildMICRChargesT()
{
	extern int SizeOfFormChargesT();

	try {

		_RecordsetPtr rs;
		CString str;

		billid = m_ID;

		// (j.jones 2005-04-13 16:14) - PLID 16181 - The rather complicated DiagCode calculation is so that the
		// Diagnosis Code used on each line represents the first code in the WhichCodes sequence for that charge.
		// It also supports when they select the code itself as the WhichCodes.
		//DRT 4/10/2006 - PLID 11734 - Removed ProcCode
		// (j.gruber 2009-03-18 12:21) - PLID 33574 - discount structure
		// (j.jones 2010-04-07 17:29) - PLID 15224 - removed ChargesT.EMG, obsolete field
		// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" charges & applies
		// (a.walling 2014-03-06 13:37) - PLID 61230 - Which codes has changed, using that structure, which also handles the BillDiagCodeT changes
		// (a.walling 2014-03-10 13:29) - PLID 61305 - Emulate the old WhichCodes field via ChargeWhichCodesFlatV
		// (a.walling 2014-03-17 17:26) - PLID 61406 - Unique diag codes and recalculated WhichCodes using BillDiagsXmlFlat4IF and ChargeWhichCodesIF - MICR, MICR2007
		g_aryFormQueries[2].sql.Format("SELECT ChargesT.ID, ChargesT.BillID, ChargesT.ItemCode, ChargesT.ItemSubCode, ChargesT.Category, ChargesT.SubCategory, LineItemT.Description, ((CASE WHEN ChargesT.CPTModifier Is Null Then '' ELSE ChargesT.CPTModifier END) + (CASE WHEN ChargesT.CPTModifier2 Is Null Then '' ELSE '  ' + ChargesT.CPTModifier2 END)) AS CPTModifier, "
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
			") AS DiagCode, "
			"ChargesT.TaxRate, ChargesT.TaxRate2, ChargesT.Quantity, LineItemT.Amount, ChargesT.OthrBillFee, ChargesT.DoctorsProviders, ChargesT.ServiceDateFrom AS [Service Date From], "
			"ChargesT.ServiceDateTo AS [Service Date To], PlaceofServiceCodesT.PlaceCodes AS [Service Location], ChargesT.ServiceType AS [Service Type], ChargesT.EPSDT, ChargesT.COB, LineItemT.Date, LineItemT.InputDate, LineItemT.InputName, LineItemT.LocationID, (SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) as PercentOff, (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) as Discount, (CASE WHEN ChargeRespT.Amount Is Not Null THEN ChargeRespT.Amount ELSE 0 END) AS InsResp, (CASE WHEN ChargeRespT.Amount Is Not Null THEN InsuredPartyT.RespTypeID ELSE 0 END) AS InsType, dbo.GetChargeTotal(ChargesT.ID) AS ChargeTotal, "
			"SumOfAmount AS ApplyTotal FROM (BillsT LEFT JOIN ChargesT ON BillsT.ID = ChargesT.BillID) LEFT JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"LEFT JOIN PlaceOfServiceCodesT ON ChargesT.ID = PlaceOfServiceCodesT.ID "
			"LEFT JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ID LEFT JOIN InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID LEFT JOIN CPTModifierT ON CPTModifier = CPTModifierT.Number LEFT JOIN CPTModifierT CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number "
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
			"OUTER APPLY dbo.%s(BillsT.ID) BillDiagCodeFlatV "
			"WHERE BillsT.ID = %li AND LineItemT.Deleted = 0 AND Batched = 1 "
			"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
			"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null"
			, ShouldUseICD10(m_InsuredPartyID, m_ID) ? "ICD10DiagID" : "ICD9DiagID"
			, ShouldUseICD10(m_InsuredPartyID, m_ID) ? "BillICD10DiagsXmlFlat4IF" : "BillICD9DiagsXmlFlat4IF"
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
				for(i=0;i<(6*((long)(firstcharge/6)));i++) {
					rs->MoveNext();
				}
				//loop through the charges on this page
				for(i=0;i<6;i++) {
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
				for(int i=0;i<6;i++) {
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

		//str.Format("SELECT ChargeTotal, (CASE WHEN ApplyTotal Is Null THEN 0 ELSE ApplyTotal END) AS ApplyTotal FROM (%s) AS FormChargesT",temp);
		//g_aryFormQueries[74].sql = "SELECT Convert(money,Sum(ChargeTotal)) AS TotalCharges FROM (SELECT TOP 6 MICRChargeTopQ.ChargeTotal, MICRChargeTopQ.ApplyTotal "
		//		"FROM (" + str + ") AS MICRChargeTopQ) AS MICRCharge6Q";

		BOOL bShowPayments = TRUE;

		if(bShowPayments) {
			//g_aryFormQueries[75].sql = "SELECT Sum(ApplyTotal) AS TotalApplies FROM (SELECT TOP 6 MICRChargeTopQ.ChargeTotal, MICRChargeTopQ.ApplyTotal "
			//		"FROM (" + str + ") AS MICRChargeTopQ) AS MICRCharge6Q";
			//g_aryFormQueries[76].sql = "SELECT Sum(Convert(money,(ChargeTotal)-(ApplyTotal))) AS TotalBalance FROM (SELECT TOP 6 MICRChargeTopQ.ChargeTotal, MICRChargeTopQ.ApplyTotal "
			//		"FROM (" + str + ") AS MICRChargeTopQ) AS MICRCharge6Q";
		}
		else {
			//g_aryFormQueries[75].sql = "SELECT Convert(money,'$0.00') AS TotalApplies ";
			//g_aryFormQueries[76].sql = "SELECT Sum(Convert(money,(ChargeTotal))) AS TotalBalance FROM (SELECT TOP 6 MICRChargeTopQ.ChargeTotal, MICRChargeTopQ.ApplyTotal "
			//		"FROM (" + str + ") AS MICRChargeTopQ) AS MICRCharge6Q";
		}

		// Fill total charge amount for each line
		nTotalCharges = SizeOfFormChargesT();

	} NxCatchAll("MICRDlg::BuildMICRChargesT()");
}

void OnMICRCommand(WPARAM wParam, LPARAM lParam, CDWordArray& dwChangedForms, int FormControlsID, CFormDisplayDlg* dlg) 
{
}

void OnMICRKeyDown(CDialog* pFormDisplayDlg, MSG* pMsg)
{
	CFormDisplayDlg* pDlg = (CFormDisplayDlg*)pFormDisplayDlg;
	CWnd* pCtrl = pDlg->GetFocus();
}

void CMICRDlg::OnClickAlign() 
{
	CPrintAlignDlg dlg(this);
	dlg.m_FormID = 5;
	dlg.DoModal();
}

void CMICRDlg::OnClickCapitalizeOnPrint() 
{
	if(m_CapOnPrint) {
		SetRemotePropertyInt("CapitalizeMICR",0);
	}
	else {
		SetRemotePropertyInt("CapitalizeMICR",1);
	}
	m_CapOnPrint = GetRemotePropertyInt("CapitalizeMICR",0);
}

void CMICRDlg::OnClickRestore() 
{
	CString strSQL;

	if(!CheckCurrentUserPermissions(bioClaimForms,sptWrite))
		return;

	if (IDNO == MessageBox("All changes you have made to the MICR form will be unrecoverable. Are you sure you wish to do this?", NULL, MB_YESNO))
		return;

	CWaitCursor pWait;

	try {
	// Delete all the form changes
	// (j.jones 2008-05-28 15:46) - PLID 27881 - batched statements, ensured we delete all relevant FormHistory entries
	CString strBatch = BeginSqlBatch();
	AddStatementToSqlBatch(strBatch, "DELETE FROM FormHistoryT WHERE DocumentID IN (SELECT FirstDocumentID FROM HCFADocumentsT WHERE BillID = %li AND FormType = 5)", m_ID);
	AddStatementToSqlBatch(strBatch, "DELETE FROM HCFADocumentsT WHERE HCFADocumentsT.BillID = %li AND FormType = 5",m_ID);
	ExecuteSqlBatch(strBatch);

	// Remove all changes made by the user
	m_pframe->UndoChanges();

	} NxCatchAll("Error restoring defaults.");

	//Refresh the form

	m_pframe->Refresh(70);  // Lines, Labels, and Patient Information
	m_pframe->Refresh(71);  // Primary Insurance Co Info
	m_pframe->Refresh(72);  // Bill/Charge Information
	m_pframe->Refresh(73);  // Provider Information
	//m_pframe->Refresh(74);  // Charge Totals
	//m_pframe->Refresh(75);  // Payment Totals
	//m_pframe->Refresh(76);  // Balance

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

void CMICRDlg::Save(BOOL boSaveAll, CDialog* pdlgWait, int& iPage)
{
	_RecordsetPtr nxrs(__uuidof(Recordset));

	int provider;

	for (provider_index=0; provider_index < adwProviderList.GetSize(); provider_index++) {

		provider = adwProviderList.GetAt(provider_index);

		for (firstcharge=0; firstcharge < nTotalCharges; firstcharge+=6) {

			if (pdlgWait) {
				CString str;
				str.Format("Saving MICR page %d...", ++iPage);
				pdlgWait->GetDlgItem(IDC_LABEL)->SetWindowText(str);
				pdlgWait->RedrawWindow();
			}

			SaveHistoricalData(boSaveAll);
		}
	}
}

void CMICRDlg::SaveHistoricalData(BOOL boSaveAll)
{
#ifdef SAVE_HISTORICAL_INFORMATION

	int iDocumentID;
	CString str;
	_RecordsetPtr rs(__uuidof(Recordset));

	try {		
		str.Format("SELECT FirstDocumentID FROM HCFADocumentsT WHERE BillID = %d AND FirstCharge = %d AND ProviderIndex = %d AND FormType = 5", m_ID, firstcharge, adwProviderList.GetAt(provider_index));
		rs = CreateRecordset(str);
		if (!rs->eof) {
			iDocumentID = rs->Fields->Item["FirstDocumentID"]->Value.lVal;
			rs->Close();
		}
		else {
			rs->Close();
			iDocumentID = NewNumber("FormHistoryT","DocumentID");
			//BEGIN_TRANS("SaveHistoricalMICR") {
			str.Format("INSERT INTO HCFADocumentsT ( BillID, FirstDocumentID, FirstCharge, ProviderIndex, FormType ) VALUES ( %d, %d, %d, %d, 5 )", m_ID, iDocumentID, firstcharge, adwProviderList.GetAt(provider_index));
			ExecuteSql(str);
			str.Format("INSERT INTO FormHistoryT ( DocumentID, ControlID, [Value] ) VALUES ( %d, -1, NULL )", iDocumentID);
			ExecuteSql(str);
			//} END_TRANS_CATCH_ALL("SaveHistoricalMICR");
		}		
	}
	NxCatchAll("Error in OnClickCheck()");

	///////////////////////////////////
	// Make sure we pull our historical data from the right table
	m_pframe->SetDocumentID(iDocumentID);

	m_pframe->Refresh(70);  // Lines, Labels, and Patient Information
	m_pframe->Refresh(71);  // Primary Insurance Co Info
	m_pframe->Refresh(72);  // Bill/Charge Information
	m_pframe->Refresh(73);  // Provider Information
	//m_pframe->Refresh(74);  // Charge Totals
	//m_pframe->Refresh(75);  // Payment Totals
	//m_pframe->Refresh(76);  // Balance

	m_pframe->ReapplyChanges(firstcharge, provider_index);

	m_pframe->Save (firstcharge, provider_index, iDocumentID, boSaveAll);

	// We save form 6 (zero based in the array) because where the radio button is
	// determines what text goes in a particular control. To save us time and pain,
	// we always save the data in form 6.
	//((FormLayer*)m_pframe->m_LayerArray[5])->Save(iDocumentID, 0, &m_pframe->m_ControlArray);
#endif
}

long CMICRDlg::DoScrollTo(long nNewTopPos)
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

void CMICRDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
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

BOOL PreMICRPrint(CDWordArray& dwChangedForms, CFormDisplayDlg* dlg)
{

	/*
	if (adwProviderList.GetSize() == 0) 
	{	ASSERT(FALSE);
		HandleException (NULL, "Failure in PreMICRPrint", __LINE__, __FILE__);
		return FALSE; // This should never happen
	}

	// Only happens on first call to this function. The first provider
	// information is reloaded.
	if (nPrintedCharges == 0 && nPrintedProviders == 0) {
		oldfirstcharge = firstcharge;
		oldprovider_index = provider_index;		

		provider_index = -1;
		OnMICRCommand(IDC_PHYSICIAN_LEFT, 0, dwChangedForms, 0, dlg);
		firstcharge = -6;
		nPrintedProviders++;
	}


	// If still more charges to print, print them and return
	if (nPrintedCharges < nTotalCharges) {		
		OnMICRCommand(IDC_BILL_DOWN, 0, dwChangedForms, 0, dlg);
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

		OnMICRCommand(IDC_PHYSICIAN_RIGHT, 0, dwChangedForms, 0, dlg);
		firstcharge = -6;
		OnMICRCommand(IDC_BILL_DOWN, 0, dwChangedForms, 0, dlg);
		nPrintedCharges = 6;
		nPrintedProviders++;
		return TRUE;
	}
	*/

	return TRUE;
}

void CMICRDlg::OnRadioPaper() {

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

void CMICRDlg::OnRadioNoBatch() {

	OnRadioPaper();
}

// (j.jones 2007-06-22 13:23) - PLID 25665 - handle when a field was edited, so we update the info text box
BOOL CMICRDlg::PreTranslateMessage(MSG *pMsg) {

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
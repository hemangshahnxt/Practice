// UB04Dlg.cpp : implementation file
//

#include "stdafx.h"
#include "UB04.h"
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

// UB04 stuff

static CDWordArray adwProviderList;
static int firstcharge = 0;
static int provider_index = 0;
static int fee_group_id = -1;
static int billid;
static int nTotalChargeLines;
// (j.jones 2008-06-12 11:10) - PLID 23052 - allowed multiple pages of charges
static long nPageIncrement = 22;

// For printing
static int nPrintedCharges = 0;
static int nPrintedProviders = 0;
static int oldfirstcharge, oldprovider_index;

static CFormDisplayDlg* g_pFrame;

using namespace ADODB;

// (j.jones 2007-03-02 11:05) - PLID 23939 - created, migrated from UB92 files

BOOL PreUB04Print(CDWordArray& dwChangedForms, CFormDisplayDlg* dlg);

void RequeryHistoricalUB04Data()
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
// CUB04Dlg dialog

BEGIN_MESSAGE_MAP(CUB04Dlg, CDialog)
	//{{AFX_MSG_MAP(CUB04Dlg)
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

BEGIN_EVENTSINK_MAP(CUB04Dlg, CDialog)
    //{{AFX_EVENTSINK_MAP(CUB04Dlg)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void OnUB04Command(WPARAM wParam, LPARAM lParam, CDWordArray& dwChangedForms, int FormControlsID, CFormDisplayDlg* dlg);
void OnUB04KeyDown(CDialog* pFormDisplayDlg, MSG* pMsg);

CUB04Dlg::CUB04Dlg(CWnd* pParent /*=NULL*/)
	: CDialog(CUB04Dlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CUB04Dlg)
		m_bPrintWarnings = TRUE;
	//}}AFX_DATA_INIT
	m_pframe = 0;
	m_ID = -1;
	m_InsuredPartyID = -1;
	m_OthrInsuredPartyID = -1;

	m_pupbutton = NULL;
	m_pdownbutton = NULL;

	m_pOnCommand = &OnUB04Command;
	m_pOnKeyDown = &OnUB04KeyDown;
	m_pPrePrintFunc = NULL;
	m_ShowWindowOnInit = TRUE;
}

CUB04Dlg::~CUB04Dlg()
{
	delete m_pframe;
//	delete m_pleftbutton;
//	delete m_prightbutton;

	// (j.jones 2008-06-12 10:46) - PLID 23052 - added support for multiple pages
	if (m_pupbutton) {
		delete m_pupbutton;
	}
	if (m_pdownbutton) {
		delete m_pdownbutton;
	}
}

void CUB04Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUB04Dlg)
	// (j.jones 2007-06-25 10:14) - PLID 25663 - changed the buttons to NxIconButtons
	DDX_Control(pDX, IDC_RESTORE, m_btnRestoreDefaults);
	DDX_Control(pDX, IDC_ALIGN, m_btnAlignForm);
	DDX_Control(pDX, IDC_CHECK, m_btnSave);
	DDX_Control(pDX, IDC_PRINT, m_btnPrint);
	DDX_Control(pDX, IDC_X, m_btnClose);
	//}}AFX_DATA_MAP
}

int CUB04Dlg::DoModal(int billID) 
{
	m_ID = billID;
	return CDialog::DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CUB04Dlg message handlers

BOOL CUB04Dlg::OnInitDialog() 
{
	try {
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

		/// UB04 SPECIFIC STUFF ////////////
		
		// (j.jones 2013-04-26 16:37) - PLID 56453 - Get the POS code, which is unfortunately
		// tied to charges despite being the same on all charges.
		CString strPOSCode = "";
		_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 BillsT.Description, BillsT.InsuredPartyID, BillsT.OthrInsuredPartyID, "
			"PlaceOfServiceCodesT.PlaceCodes "
			"FROM BillsT "
			"INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
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
		}
		rs->Close();

		firstcharge = 0;

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

		//TES 3/21/2007 - SetUB04DateFormats() depends on the fee_group_id being set, so make sure to call GetUB04GroupID() first!
		GetUB04GroupID();

		SetUB04DateFormats(); // Set all the date boxes to 2 or 4 digit years through UB04 ID Setup information

		BuildBoxes76_78();

		BuildUB04ChargesT();

		FindBatch();

		m_pPrePrintFunc = &PreUB04Print;

		m_CapOnPrint = GetRemotePropertyInt("CapitalizeHCFA",0);
		((CButton*)GetDlgItem(IDC_CAP_ON_PRINT))->SetCheck(m_CapOnPrint);
		//m_CapOnPrint = TRUE;

		//create frame

		//allocate pointers
		// (j.jones 2010-03-15 15:19) - PLID 37719 - removed unnecessary log
		//LogDetail("UB04Dlg: Creating frame");
		m_pframe		= new CFormDisplayDlg(this);
		m_pframe->color = 0x000000FF;
		g_pFrame = m_pframe;
	//	m_pleftbutton	= new CButton;
	//	m_prightbutton	= new CButton;
		// (j.jones 2008-06-12 10:46) - PLID 23052 - added support for multiple pages
		m_pupbutton		= new CButton;
		m_pdownbutton	= new CButton;

		RequeryHistoricalUB04Data();

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

		SetWindowText("UB04");

		int i = 187;

		m_pframe->Load (80, "", "", &i); // Box 76
		m_pframe->Load (93, "", "", &i); // Box 77
		m_pframe->Load (81, "", "", &i); // Box 78

		//UB04Box79
		//fill with the default value from the UB04 Setup, but use the bill value if it exists
		CString strBox79 = m_UB92Info.Box79;

		CString strBox14, strBox15;

		CString strAdmissionHour, strDischargeHour;

		// (j.jones 2006-05-30 12:15) - If Boxes 32 - 36 have data, then show the dates
		//also grab 79 while we are at it
		//TES 3/20/2007 - PLID 23939 - Added UB04 Box 36
		// (j.jones 2010-07-27 11:32) - PLID 39858 - get the assignment of benefits setting from the bill,
		// it's called HCFABox13Over, but it also applies to UB Box 53
		// (j.jones 2012-05-15 09:08) - PLID 50376 - The UB group's default for boxes 14 and 15 now loads into the bill,
		// and these fields are now editable on the bill itself. So pull 14 and 15 from the bill, not from the group.
		// (j.jones 2013-06-07 16:47) - PLID 41479 - Box 13 is no longer pulled from UB setup. It is the hour from the bill's
		// admission time. Box 16 is the hour from the bill's discharge time (in 24hr format).
		HCFABox13Over hb13Value = hb13_UseDefault;

		ANSI_ClaimTypeCode eClaimTypeCode = ANSI_ClaimTypeCode::ctcOriginal;

		// (a.walling 2016-03-10 07:36) - PLID 68550 - UB04 Enhancements - update UB04 paper forms to load from UB04ClaimInfo object
		// (j.jones 2016-05-24 9:52) - NX-100707 - get the claim type code from the bill
		rs = CreateParamRecordset("SELECT ANSI_ClaimTypeCode, "
				"UBBox14, UBBox15 "
				", COALESCE(UB04ClaimInfo.value('(/claimInfo/occurrences/occurrence/@code)[1]', 'nvarchar(20)'), '') as UB92Box32 "
				", COALESCE(UB04ClaimInfo.value('(/claimInfo/occurrences/occurrence/@code)[2]', 'nvarchar(20)'), '') as UB92Box33 "
				", COALESCE(UB04ClaimInfo.value('(/claimInfo/occurrences/occurrence/@code)[3]', 'nvarchar(20)'), '') as UB92Box34 "
				", COALESCE(UB04ClaimInfo.value('(/claimInfo/occurrences/occurrence/@code)[4]', 'nvarchar(20)'), '') as UB92Box35 "
				", COALESCE(UB04ClaimInfo.value('(/claimInfo/occurrenceSpans/occurrenceSpan/@code)[1]', 'nvarchar(20)'), '') as UB92Box36 "
				", COALESCE(UB04ClaimInfo.value('(/claimInfo/occurrenceSpans/occurrenceSpan/@code)[2]', 'nvarchar(20)'), '') as UB04Box36 "
			", UB92Box79, HCFABox13Over, "
			"CASE WHEN AdmissionTime Is Null THEN Null ELSE DATEPART(hour, AdmissionTime) END AS AdmissionHour, "
			"CASE WHEN DischargeTime Is Null THEN Null ELSE DATEPART(hour, DischargeTime) END AS DischargeHour "
			"FROM BillsT WHERE ID = {INT}", m_ID);
		if(!rs->eof) {

			// (j.jones 2016-05-24 9:52) - NX-100707 - get the claim type code from the bill
			eClaimTypeCode = (ANSI_ClaimTypeCode)AdoFldLong(rs, "ANSI_ClaimTypeCode");

			// (j.jones 2012-05-15 09:08) - PLID 50376 - The UB group's default for boxes 14 and 15 now loads into the bill,
			// and these fields are now editable on the bill itself. So pull 14 and 15 from the bill, not from the group.
			strBox14 = VarString(rs->Fields->Item["UBBox14"]->Value, "");
			strBox15 = VarString(rs->Fields->Item["UBBox15"]->Value, "");

			// (j.jones 2013-06-07 16:47) - PLID 41479 - Box 13 is no longer pulled from UB setup. It is the hour from the bill's
			// admission time. Box 16 is the hour from the bill's discharge time (in 24hr format).
			long nAdmissionHour = VarLong(rs->Fields->Item["AdmissionHour"]->Value, -1);
			// (b.spivey July 8, 2015) PLID 66516 - reflect box 12/13 setting
			if((m_UB92Info.FillBox12_13 != 0) && nAdmissionHour != -1) {
				strAdmissionHour = AsString(nAdmissionHour);
			}
			long nDischargeHour = VarLong(rs->Fields->Item["DischargeHour"]->Value, -1);
			// (b.spivey July 8, 2015) PLID 66516 - reflect this setting for box 16.
			if ((m_UB92Info.FillBox16 != 0) && nDischargeHour != -1) {
				strDischargeHour = AsString(nDischargeHour);
			}

			hb13Value = (HCFABox13Over)AdoFldLong(rs, "HCFABox13Over", (long)hb13_UseDefault);

			CString str = AdoFldString(rs, "UB92Box79","");
			str.TrimRight();
			if(!str.IsEmpty())
				strBox79 = str;

			str = AdoFldString(rs, "UB92Box32","");
			str.TrimRight();
			if(!str.IsEmpty()) {
				// (j.jones 2009-12-22 10:37) - PLID 27131 - supported the option to show
				// the date of current accident in box 31, instead of the first charge date
				// (this is Box 31, not Box 32, the UB92Box32 field name is outdated)
				if(m_UB92Info.UB04UseBox31Date == 1) {
					//Date Of Current Accident
					ExecuteSql("UPDATE FormControlsT SET Source = 'ConditionDate' WHERE ID = 6361");
				}
				else {
					//First LineItemT.Date
					ExecuteSql("UPDATE FormControlsT SET Source = 'FirstDate' WHERE ID = 6361");
				}				
			}
			else {
				ExecuteSql("UPDATE FormControlsT SET Source = '' WHERE ID = 6361");
			}

			str = AdoFldString(rs, "UB92Box33","");
			str.TrimRight();
			if(!str.IsEmpty()) {
				ExecuteSql("UPDATE FormControlsT SET Source = 'FirstDate' WHERE ID = 6359");
			}
			else {
				ExecuteSql("UPDATE FormControlsT SET Source = '' WHERE ID = 6359");
			}

			str = AdoFldString(rs, "UB92Box34","");
			str.TrimRight();
			if(!str.IsEmpty()) {
				ExecuteSql("UPDATE FormControlsT SET Source = 'FirstDate' WHERE ID = 6357");
			}
			else {
				ExecuteSql("UPDATE FormControlsT SET Source = '' WHERE ID = 6357");
			}

			str = AdoFldString(rs, "UB92Box35","");
			str.TrimRight();
			if(!str.IsEmpty()) {
				ExecuteSql("UPDATE FormControlsT SET Source = 'FirstDate' WHERE ID = 6355");
			}
			else {
				ExecuteSql("UPDATE FormControlsT SET Source = '' WHERE ID = 6355");
			}

			str = AdoFldString(rs, "UB92Box36","");
			str.TrimRight();
			if(!str.IsEmpty()) {
				ExecuteSql("UPDATE FormControlsT SET Source = 'FirstDate' WHERE ID = 6353");
				ExecuteSql("UPDATE FormControlsT SET Source = 'LastDate' WHERE ID = 6340");
			}
			else {
				ExecuteSql("UPDATE FormControlsT SET Source = '' WHERE ID = 6353 OR ID = 6340");
			}

			str = AdoFldString(rs, "UB04Box36","");
			str.TrimRight();
			if(!str.IsEmpty()) {
				ExecuteSql("UPDATE FormControlsT SET Source = 'FirstDate' WHERE ID = 6788");
				ExecuteSql("UPDATE FormControlsT SET Source = 'LastDate' WHERE ID = 6790");
			}
			else {
				ExecuteSql("UPDATE FormControlsT SET Source = '' WHERE ID = 6788 OR ID = 6790");
			}
		}
		rs->Close();

		//Load ShowInsAdd setting - to show the insurance name and address in Box84	
		if(m_UB92Info.ShowInsAdd == 1) {
			//show the address
			// (a.walling 2007-06-05 10:56) - PLID 26223 - Option to use only one line for addr1 + addr2
			if (m_UB92Info.UB04Box80UseThree) {
				ExecuteSql("UPDATE FormControlsT SET Source = '' WHERE ID = 6696");
				ExecuteSql("UPDATE FormControlsT SET Source = 'InsCoName' WHERE ID = 6703");
				ExecuteSql("UPDATE FormControlsT SET Source = 'InsCoAdd' WHERE ID = 6704");
				ExecuteSql("UPDATE FormControlsT SET Source = 'InsCoCityStateZip' WHERE ID = 6705");
			} else {
				ExecuteSql("UPDATE FormControlsT SET Source = 'InsCoName' WHERE ID = 6696");
				ExecuteSql("UPDATE FormControlsT SET Source = 'InsCoAdd1' WHERE ID = 6703");
				ExecuteSql("UPDATE FormControlsT SET Source = 'InsCoAdd2' WHERE ID = 6704");
				ExecuteSql("UPDATE FormControlsT SET Source = 'InsCoCityStateZip' WHERE ID = 6705");
			}
		}
		else
			ExecuteSql("UPDATE FormControlsT SET Source = '' WHERE ID IN (6696, 6703, 6704, 6705)");

		if(m_UB92Info.Box76Show) {
			ExecuteSql("UPDATE FormControlsT SET Source = 'Diag1' WHERE ID = 6683");
		}
		else {
			ExecuteSql("UPDATE FormControlsT SET Source = '' WHERE ID = 6683");
		}

		CString strUB04Box42Line23;
		if (m_UB92Info.UB04Box42Line23) {
			// (a.walling 2007-06-05 11:32) - PLID 26219
			// will only cause at max one db access
			/*
			long nZeros = GetRemotePropertyInt("UB04Box42Line23_Zeros", 3, 0, NULL, false);
			strUB04Box42Line23.Format("%0*li", nZeros, 1);
			*/
			strUB04Box42Line23 = m_UB92Info.Box42Line23;
			// (a.walling 2007-09-07 15:47) - PLID 26219 - Since the source will always be the same,
			// this is now in FormControlsTBox42Line23.mod
			//ExecuteSqlStd("UPDATE FormControlsT SET Source = 'Box42Line23' WHERE ID = 6029");
		}

		//Box 4
		CString strBox4 = m_UB92Info.Box4;

		// (j.jones 2011-07-06 16:21) - PLID 44358 - see if any charge has a Box 4 override,
		// and if so, use the first one
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

		{
			// (j.jones 2016-05-24 9:52) - NX-100707 - The last digit pulls from the claim type selected
			// on the bill. The user can type in any override they want so the following logic is used:
			// * If Box 4 is not filled in, default to "83" plus the Claim Type Code. (Ex.: "831")
			// * If Box 4 is 4 digits, and the final digit is "1", replace it with the Claim Type Code.
			//	 (Ex.: "0831" exports as "083" plus the Claim Type Code.
			// * If Box 4 is 3 digits, and the first digit is not 0 and final digit is "1", replace it with the Claim Type Code.
			//	 (Ex.: "831" exports as "83" plus the Claim Type Code. "083" would just append.
			// * All other values in Box 4 should just append the Claim Type Code at the end.
			//   (Ex.: "0832" becomes "08321", "123456" becomes "1234561".These would reject and require the client to fix their poor setup.
			strBox4.TrimLeft(); strBox4.TrimRight();

			//if empty, default to 83
			if (strBox4.IsEmpty()) {
				strBox4 = "83";
			}

			//if 4 digits, and the final digit is 1, drop the final digit
			if (strBox4.GetLength() == 4) {
				if (strBox4.Right(1) == "1") {
					strBox4 = strBox4.Left(3);
				}
			}
			//if 3 digits, and the first digit is NOT 0, 
			//and the final digit is 1, drop the final digit
			else if (strBox4.GetLength() == 3) {
				if (strBox4.Left(1) != "0" && strBox4.Right(1) == "1") {
					strBox4 = strBox4.Left(2);
				}
			}

		
			//now append the selected claim type code

			//***the claim type code enum is not the same value as what must be submitted on the claim
			CString strClaimTypeCode = GetClaimTypeCode_UB(eClaimTypeCode);

			strBox4 += strClaimTypeCode;
		}

		//set the default text fields
		//TES 3/20/2007 - PLID 23939 - Aliased Boxes 18,19,20 and 22 as Boxes 13,14,15, and 17, since that's what they are on the UB04.
		// (j.jones 2012-05-15 09:08) - PLID 50376 - The UB group's default for boxes 14 and 15 now loads into the bill,
		// and these fields are now editable on the bill itself. So pull 14 and 15 from the bill, not from the group.
		// (j.jones 2013-06-07 16:47) - PLID 41479 - Box 13 is no longer pulled from UB setup. It is the hour from the bill's
		// admission time. Box 16 is the hour from the bill's discharge time (in 24hr format).
		g_aryFormQueries[82].sql.Format("SELECT PatientsT.PersonID, PatientsT.UserDefinedID AS ID, "
			"PersonT.[Last] + ', ' + PersonT.[First] + ' ' + PersonT.[Middle] AS Name, "
			"(CASE WHEN (PatientsT.[MaritalStatus]='1') THEN 'S' WHEN (PatientsT.[MaritalStatus]='2') THEN 'M' ELSE ' ' END) AS MS, "
			"PersonT.[Address1] + ' ' + PersonT.[Address2] + ' ' + PersonT.[City] + ' ' + PersonT.[State] + ' ' + PersonT.[Zip] AS Address, "
			"PersonT.[BirthDate] AS BirthDate, (CASE WHEN (PersonT.[Gender]=1) THEN 'M' WHEN (PersonT.[Gender]=2) THEN 'F' ELSE Null END) AS Sex, "
			"'%s' AS Box4, '%s' AS Box8, '%s' AS Box10, '%s' AS Box14, '%s' AS Box15, '%s' AS Box17, '%s' AS Box79, '%s' AS Box42Line23, "
			"'%s' AS AdmissionHour, '%s' AS DischargeHour "			
			"FROM PatientsT "
			"LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID",
			_Q(strBox4),_Q(m_UB92Info.Box8),_Q(m_UB92Info.Box10),_Q(strBox14),_Q(strBox15),_Q(m_UB92Info.Box22),_Q(strBox79),_Q(strUB04Box42Line23),
			_Q(strAdmissionHour), _Q(strDischargeHour));

		where.Format (" WHERE ID = %i", m_PatientID);
		m_pframe->Load (82, where, "", &i); // Patient information
		m_pframe->Load (83, "", "", &i); // Charges
		m_pframe->Load (84, "", "", &i); // Bill stuff

		//Load ShowPhone setting - to show the location phone number in Box1
		// (j.jones 2007-07-12 11:32) - PLID 26625 - also don't fill if Box 1 is empty
		if(m_UB92Info.ShowPhone == 1 && m_UB92Info.UB04Box1Setup != -1)
			ExecuteSql("UPDATE FormControlsT SET Source = 'Phone' WHERE ID = 6710");
		else
			ExecuteSql("UPDATE FormControlsT SET Source = '' WHERE ID = 6710");

		// (j.jones 2013-04-26 16:24) - PLID 56453 - we now send the patient address as the POS
		//if its POS code is 12 and the Claim_SendPatAddressWhenPOS12 preference is on
		BOOL bUsePatientAddressASPOS = FALSE;
		if(strPOSCode == "12" && GetRemotePropertyInt("Claim_SendPatAddressWhenPOS12", 1, 0, "<None>", true) == 1) {
			bUsePatientAddressASPOS = TRUE;
		}
		
		// (j.jones 2007-07-12 11:07) - PLID 26625 - UB04 now uses UB04Box1Setup
		// instead of the old Box1Loc option
		// (j.jones 2007-09-13 16:20) - PLID 27382 - these three options needed to include LocationsT.EIN in the field list, now they do
		if(m_UB92Info.UB04Box1Setup == 1) {

			//bill location

			// (j.jones 2006-12-04 15:25) - PLID 23733 - supported the ClaimProviderID override
			// (j.jones 2010-11-09 15:39) - PLID 41387 - supported ChargesT.ClaimProviderID
			// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" charges
			g_aryFormQueries[85].sql = "SELECT TOP 1 LocationsT.EIN, LocationsT.Name, LocationsT.Phone, LocationsT.Address1, LocationsT.Address2, City + ', ' + State + ' ' + Zip AS CityStateZip, "
				"ProvidersT.[Fed Employer ID] "
				"FROM LocationsT INNER JOIN LineItemT ON LocationsT.ID = LineItemT.LocationID "
				"INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
				"LEFT JOIN ProvidersT ActualProvidersT ON ChargesT.DoctorsProviders = ActualProvidersT.PersonID "
				"LEFT JOIN ProvidersT ON (CASE WHEN ChargesT.ClaimProviderID Is Not Null THEN ChargesT.ClaimProviderID ELSE ActualProvidersT.ClaimProviderID END) = ProvidersT.PersonID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID";
			where.Format (" WHERE LineItemT.Deleted = 0 AND ChargesT.Batched = 1 "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"AND ChargesT.BillID = %li", m_ID);
		}
		else if(m_UB92Info.UB04Box1Setup == 2) {
			//place of service

			CString strPOSFields;
			// (j.jones 2013-04-26 16:19) - PLID 56453 - we now send the patient address as the POS
			//if its POS code is 12 and the Claim_SendPatAddressWhenPOS12 preference is on
			if(bUsePatientAddressASPOS) {
				//use the patient's address for the POS fields
				strPOSFields = "'' AS EIN, 'Patient Home' AS Name, PatientPersonT.HomePhone AS Phone, PatientPersonT.Address1, PatientPersonT.Address2, PatientPersonT.City + ', ' + PatientPersonT.State + ' ' + PatientPersonT.Zip AS CityStateZip, ";
			}
			else {
				//use the POS address for the POS fields
				strPOSFields = "LocationsT.EIN, LocationsT.Name, LocationsT.Phone, LocationsT.Address1, LocationsT.Address2, LocationsT.City + ', ' + LocationsT.State + ' ' + LocationsT.Zip AS CityStateZip, ";
			}

			// (j.jones 2006-12-04 15:25) - PLID 23733 - supported the ClaimProviderID override
			// (j.jones 2010-11-09 15:39) - PLID 41387 - supported ChargesT.ClaimProviderID
			g_aryFormQueries[85].sql.Format("SELECT TOP 1 %s "
				"ProvidersT.[Fed Employer ID] "
				"FROM LocationsT INNER JOIN BillsT ON LocationsT.ID = BillsT.Location "
				"INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"INNER JOIN PersonT PatientPersonT ON BillsT.PatientID = PatientPersonT.ID "
				"LEFT JOIN ProvidersT ActualProvidersT ON ChargesT.DoctorsProviders = ActualProvidersT.PersonID "
				"LEFT JOIN ProvidersT ON (CASE WHEN ChargesT.ClaimProviderID Is Not Null THEN ChargesT.ClaimProviderID ELSE ActualProvidersT.ClaimProviderID END) = ProvidersT.PersonID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID", strPOSFields);
			where.Format (" WHERE LineItemT.Deleted = 0 AND ChargesT.Batched = 1 "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"AND BillsT.ID = %li", m_ID);
		}
		else if(m_UB92Info.UB04Box1Setup == 3) {
			
			//bill provider name / bill location address
			
			//just use the first provider on the bill
			// (j.jones 2010-11-09 15:39) - PLID 41387 - supported ChargesT.ClaimProviderID
			// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" charges
			g_aryFormQueries[85].sql = "SELECT TOP 1 LocationsT.EIN, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS Name, LocationsT.Phone, LocationsT.Address1, LocationsT.Address2, LocationsT.City + ', ' + LocationsT.State + ' ' + LocationsT.Zip AS CityStateZip, "
				"ProvidersT.[Fed Employer ID] "
				"FROM LocationsT INNER JOIN LineItemT ON LocationsT.ID = LineItemT.LocationID "
				"INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
				"LEFT JOIN ProvidersT ActualProvidersT ON ChargesT.DoctorsProviders = ActualProvidersT.PersonID "
				"LEFT JOIN ProvidersT ON (CASE WHEN ChargesT.ClaimProviderID Is Not Null THEN ChargesT.ClaimProviderID ELSE ActualProvidersT.ClaimProviderID END) = ProvidersT.PersonID "
				"LEFT JOIN PersonT ON ProvidersT.PersonID = PersonT.ID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID";
			where.Format (" WHERE LineItemT.Deleted = 0 AND ChargesT.Batched = 1 "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"AND ChargesT.BillID = %li", m_ID);
		}
		else {

			//leave empty
			// (j.jones 2007-09-13 16:20) - PLID 27382 - except for the Bill Location EIN or ProvidersT.FedEmployerID
			// (j.jones 2010-11-09 15:39) - PLID 41387 - supported ChargesT.ClaimProviderID
			// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" charges
			g_aryFormQueries[85].sql = "SELECT TOP 1 LocationsT.EIN, '' AS Name, '' AS Phone, '' AS Address1, '' AS Address2, '' AS CityStateZip, "
				"ProvidersT.[Fed Employer ID] "
				"FROM LocationsT INNER JOIN LineItemT ON LocationsT.ID = LineItemT.LocationID "
				"INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
				"LEFT JOIN ProvidersT ActualProvidersT ON ChargesT.DoctorsProviders = ActualProvidersT.PersonID "
				"LEFT JOIN ProvidersT ON (CASE WHEN ChargesT.ClaimProviderID Is Not Null THEN ChargesT.ClaimProviderID ELSE ActualProvidersT.ClaimProviderID END) = ProvidersT.PersonID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID";
			where.Format (" WHERE LineItemT.Deleted = 0 AND ChargesT.Batched = 1 "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"AND ChargesT.BillID = %li", m_ID);
		}

		if(m_UB92Info.Box5ID == 0) {
			//Box 5 - Location EIN
			ExecuteSql("UPDATE FormControlsT SET Source = 'EIN' WHERE ID = 6283");
		}
		else {
			//Box 5 - Provider Tax ID
			ExecuteSql("UPDATE FormControlsT SET Source = '[Fed Employer ID]' WHERE ID = 6283");
		}

		m_pframe->Load (85, where, "", &i); // Box 1 Location data

		// (j.jones 2007-07-12 09:47) - PLID 26621 - supported Box 2
		if(m_UB92Info.UB04Box2Setup == 1) {

			//bill location

			// (j.jones 2010-11-09 15:39) - PLID 41387 - supported ChargesT.ClaimProviderID
			// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" charges
			g_aryFormQueries[94].sql = "SELECT TOP 1 LocationsT.Name, LocationsT.Address1 + ' ' + LocationsT.Address2 AS Address, City + ', ' + State + ' ' + Zip AS CityStateZip, "
				"ProvidersT.[Fed Employer ID] "
				"FROM LocationsT "
				"INNER JOIN LineItemT ON LocationsT.ID = LineItemT.LocationID "
				"INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
				"LEFT JOIN ProvidersT ActualProvidersT ON ChargesT.DoctorsProviders = ActualProvidersT.PersonID "
				"LEFT JOIN ProvidersT ON (CASE WHEN ChargesT.ClaimProviderID Is Not Null THEN ChargesT.ClaimProviderID ELSE ActualProvidersT.ClaimProviderID END) = ProvidersT.PersonID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID";
			where.Format (" WHERE LineItemT.Deleted = 0 AND ChargesT.Batched = 1 "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"AND ChargesT.BillID = %li", m_ID);
		}
		else if(m_UB92Info.UB04Box2Setup == 2) {
			//place of service

			CString strPOSFields;
			// (j.jones 2013-04-26 16:19) - PLID 56453 - we now send the patient address as the POS
			//if its POS code is 12 and the Claim_SendPatAddressWhenPOS12 preference is on
			if(bUsePatientAddressASPOS) {
				//use the patient's address for the POS fields
				strPOSFields = "'Patient Home' AS Name, PatientPersonT.Address1 + ' ' + PatientPersonT.Address2 AS Address, PatientPersonT.City + ', ' + PatientPersonT.State + ' ' + PatientPersonT.Zip AS CityStateZip, ";
			}
			else {
				//use the POS address for the POS fields
				strPOSFields = "LocationsT.Name, LocationsT.Address1 + ' ' + LocationsT.Address2 AS Address, LocationsT.City + ', ' + LocationsT.State + ' ' + LocationsT.Zip AS CityStateZip, ";
			}

			// (j.jones 2010-11-09 15:39) - PLID 41387 - supported ChargesT.ClaimProviderID
			// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" charges
			g_aryFormQueries[94].sql.Format("SELECT TOP 1 %s "
				"ProvidersT.[Fed Employer ID] "
				"FROM LocationsT INNER JOIN BillsT ON LocationsT.ID = BillsT.Location "
				"INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"INNER JOIN PersonT PatientPersonT ON BillsT.PatientID = PatientPersonT.ID "
				"LEFT JOIN ProvidersT ActualProvidersT ON ChargesT.DoctorsProviders = ActualProvidersT.PersonID "
				"LEFT JOIN ProvidersT ON (CASE WHEN ChargesT.ClaimProviderID Is Not Null THEN ChargesT.ClaimProviderID ELSE ActualProvidersT.ClaimProviderID END) = ProvidersT.PersonID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID", strPOSFields);
			where.Format (" WHERE LineItemT.Deleted = 0 AND ChargesT.Batched = 1 "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"AND BillsT.ID = %li", m_ID);
		}
		else if(m_UB92Info.UB04Box2Setup == 3) {
			
			//bill provider name / bill location address
			
			//just use the first provider on the bill
			// (j.jones 2010-11-09 15:39) - PLID 41387 - supported ChargesT.ClaimProviderID
			// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" charges
			g_aryFormQueries[94].sql = "SELECT TOP 1 PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS Name, LocationsT.Address1 + ' ' + LocationsT.Address2 AS Address, LocationsT.City + ', ' + LocationsT.State + ' ' + LocationsT.Zip AS CityStateZip, "
				"ProvidersT.[Fed Employer ID] "
				"FROM LocationsT INNER JOIN LineItemT ON LocationsT.ID = LineItemT.LocationID INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
				"LEFT JOIN ProvidersT ActualProvidersT ON ChargesT.DoctorsProviders = ActualProvidersT.PersonID "
				"LEFT JOIN ProvidersT ON (CASE WHEN ChargesT.ClaimProviderID Is Not Null THEN ChargesT.ClaimProviderID ELSE ActualProvidersT.ClaimProviderID END) = ProvidersT.PersonID "
				"LEFT JOIN PersonT ON ProvidersT.PersonID = PersonT.ID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID";
			where.Format (" WHERE LineItemT.Deleted = 0 AND ChargesT.Batched = 1 "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"AND ChargesT.BillID = %li", m_ID);
		}
		else {

			//leave empty			
			g_aryFormQueries[94].sql = "SELECT '' AS Name, '' AS Address, '' AS CityStateZip";
			where = "";
		}

		m_pframe->Load (94, where, "", &i); // Box 2 Location data

		//Load Box38 setting - 0 is Insured Party, 1 is Insurance Company
		if(m_UB92Info.Box38 == 0) {
			//Insured Party
			
			//Load ShowCompanyAsInsurer setting - 0 is the person name, 1 is the company name
			if(m_UB92Info.ShowCompanyAsInsurer == 1) {
				g_aryFormQueries[86].sql = "SELECT InsuredPartyT.Employer AS FullName, PersonT.[Address1] + ' ' + PersonT.[Address2] AS FullAddress, (PersonT.City + ', ' + PersonT.State + ' ' + PersonT.Zip) AS CityStateZip FROM (BillsT INNER JOIN InsuredPartyT ON BillsT.InsuredPartyID = InsuredPartyT.PersonID) INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID LEFT JOIN PersonT ON InsuredPartyT.PersonID = PersonT.ID";
			}
			else {
				g_aryFormQueries[86].sql = "SELECT (PersonT.[First] + (CASE WHEN (PersonT.Middle Is NULL OR PersonT.Middle = '') THEN '' ELSE (' ' + PersonT.Middle) END) + ' ' + PersonT.[Last]) AS FullName, PersonT.[Address1] + ' ' + PersonT.[Address2] AS FullAddress, (PersonT.City + ', ' + PersonT.State + ' ' + PersonT.Zip) AS CityStateZip FROM (BillsT INNER JOIN InsuredPartyT ON BillsT.InsuredPartyID = InsuredPartyT.PersonID) INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID LEFT JOIN PersonT ON InsuredPartyT.PersonID = PersonT.ID";
			}
		}
		else {
			//Insurance Company
			g_aryFormQueries[86].sql = "SELECT Name AS FullName, PersonT.[Address1] + ' ' + PersonT.[Address2] AS FullAddress, (PersonT.City + ', ' + PersonT.State + ' ' + PersonT.Zip) AS CityStateZip FROM (BillsT INNER JOIN InsuredPartyT ON BillsT.InsuredPartyID = InsuredPartyT.PersonID) INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID LEFT JOIN PersonT ON InsuranceCoT.PersonID = PersonT.ID";
		}
		where.Format (" WHERE BillsT.ID = %li", m_ID);

		m_pframe->Load (86, where, "", &i); // Insured party address

		m_pframe->Load (87, "", "", &i); // Provider data

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
			ExecuteSql("UPDATE FormControlsT SET Source = 'InsAddrName' WHERE ID = 6406");
			ExecuteSql("UPDATE FormControlsT SET Source = 'OthrInsAddrName' WHERE ID = 6606");
		}
		else {
			ExecuteSql("UPDATE FormControlsT SET Source = 'InsCoName' WHERE ID = 6406");
			ExecuteSql("UPDATE FormControlsT SET Source = 'OthrInsCoName' WHERE ID = 6606");
		}

		//show/hide Box 64
		if(m_UB92Info.Box64Show == 1) {
			ExecuteSql("UPDATE FormControlsT SET Source = 'InsEmpCode' WHERE ID = 6664");
			ExecuteSql("UPDATE FormControlsT SET Source = 'OthrInsEmpCode' WHERE ID = 6665");
		}
		else {
			ExecuteSql("UPDATE FormControlsT SET Source = '' WHERE ID = 6664 OR ID = 6665");
		}

		CString strNameFormat;
		//Load ShowCompanyAsInsurer setting - 0 is the person name, 1 is the company name
		if(m_UB92Info.ShowCompanyAsInsurer == 1) {
			strNameFormat = "InsuredPartyT.Employer";
		}
		else {
			strNameFormat = "PersonT.[Last] + ',  ' + PersonT.[First] + ' ' + PersonT.[Middle]";
		}

		// (j.jones 2012-05-24 09:35) - PLID 50597 - We used to send a simple list of 1,2,3,9 for Self, Spouse, Child, Other,
		// but in most cases they really want the ANSI relationship codes, which are completely different.
		// We will still leave blank if no relationship is selected.
		CString strInsRel = "(CASE WHEN(InsuredPartyT.RelationToPatient='Self') THEN '18' "
			"WHEN (InsuredPartyT.RelationToPatient='Spouse') THEN '01' "
			"WHEN (InsuredPartyT.RelationToPatient='Child') THEN '19' "
			"WHEN (InsuredPartyT.RelationToPatient='Employee') THEN '20' "
			"WHEN (InsuredPartyT.RelationToPatient='Organ Donor') THEN '39' "
			"WHEN (InsuredPartyT.RelationToPatient='Cadaver Donor') THEN '40' "
			"WHEN (InsuredPartyT.RelationToPatient='Life Partner') THEN '53' "
			"WHEN (InsuredPartyT.RelationToPatient='Other') THEN 'G8' "
			"WHEN (InsuredPartyT.RelationToPatient='Unknown') THEN '21' "
			"WHEN (InsuredPartyT.RelationToPatient <> '') THEN '21' "
			"ELSE '' END)";

		if(m_UB92Info.InsRelANSI == 0) {
			//revert to the old list
			strInsRel = "(CASE WHEN(InsuredPartyT.RelationToPatient='Self') THEN '1' WHEN (InsuredPartyT.RelationToPatient='Spouse') THEN '2' WHEN (InsuredPartyT.RelationToPatient='Child') THEN '3' WHEN (InsuredPartyT.RelationToPatient <> '') THEN '9' ELSE '' END)";
		}

		// (j.jones 2008-05-01 11:00) - PLID 28467 - ensured that any non-empty relation to patient that isn't
		// standard will show as 'other'
		g_aryFormQueries[88].sql.Format("SELECT InsuredPartyT.IDforInsurance AS InsID, %s AS InsName, "
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
			"InsCoPersonT.Address1 AS InsCoAdd1, InsCoPersonT.Address2 AS InsCoAdd2, LTRIM(InsCoPersonT.Address1 + '  ' + InsCoPersonT.Address2) AS InsCoAdd, InsCoPersonT.City AS InsCoCity, InsCoPersonT.State AS InsCoState, InsCoPersonT.Zip AS InsCoZip, (InsCoPersonT.City + ', ' + InsCoPersonT.State + ' ' + InsCoPersonT.Zip) AS InsCoCityStateZip, "
			"%s AS InsRel, "
			"InsurancePlansT.PlanType, (CASE WHEN ([InsurancePlansT].[PlanType]='Medicare') THEN 1 WHEN ([InsurancePlansT].[PlanType]='Medicaid') THEN 2 WHEN ([InsurancePlansT].[PlanType]='Champus') THEN 3 WHEN ([InsurancePlansT].[PlanType]='Champva') THEN 4 WHEN ([InsurancePlansT].[PlanType]='Group Health Plan') THEN 5 WHEN ([InsurancePlansT].[PlanType]='FECA Black Lung') THEN 6 WHEN ([InsurancePlansT].[PlanType]='Other') THEN 7 ELSE 0 END) AS InsType, "
			"'%s' AS Accepted, InsuredPartyT.PersonID AS ID, InsuredPartyT.PatientID, 'Y' AS Yes "
			"FROM (InsuranceCoT RIGHT JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID) "
			"LEFT JOIN InsurancePlansT ON InsuredPartyT.InsPlan = InsurancePlansT.ID "
			"LEFT JOIN PersonT ON InsuredPartyT.PersonID = PersonT.ID "
			"LEFT JOIN PersonT PersonT1 ON InsuredPartyT.PatientID = PersonT1.ID "
			"LEFT JOIN PatientsT ON PersonT1.ID = PatientsT.PersonID "
			"LEFT JOIN (SELECT * FROM PersonT) AS InsCoPersonT ON InsuranceCoT.PersonID = InsCoPersonT.ID ",strNameFormat, strInsRel, priaccepted);

		where.Format (" WHERE PatientID = %li AND InsuredPartyT.PersonID = %li", m_PatientID, m_InsuredPartyID);
		m_pframe->Load (88, where, "", &i); // Insurance information

		// (j.jones 2008-05-01 11:00) - PLID 28467 - ensured that any non-empty relation to patient that isn't
		// standard will show as 'other'
		g_aryFormQueries[89].sql.Format("SELECT InsuredPartyT.IDforInsurance AS OthrInsID, %s AS OthrInsName, "
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
			"%s AS OthrInsRel, "
			"'%s' AS Accepted "
			"FROM InsuranceCoT RIGHT JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID "
			"LEFT JOIN InsurancePlansT ON InsuredPartyT.InsPlan = InsurancePlansT.ID "
			"LEFT JOIN PersonT ON InsuredPartyT.PersonID = PersonT.ID "
			"LEFT JOIN PersonT PersonT1 ON InsuredPartyT.PatientID = PersonT1.ID "
			"LEFT JOIN PatientsT ON PersonT1.ID = PatientsT.PersonID "
			"LEFT JOIN (SELECT * FROM PersonT) AS InsCoPersonT ON InsuranceCoT.PersonID = InsCoPersonT.ID ",strNameFormat, strInsRel, secaccepted);

		where.Format (" WHERE PatientID = %li AND InsuredPartyT.PersonID = %li", m_PatientID, m_OthrInsuredPartyID);
		m_pframe->Load (89, where, "", &i); // Other insurance information

		if(m_UB92Info.ShowApplies) {
			ExecuteSql("UPDATE FormControlsT SET Source = 'TotalPays' WHERE ID = 6614");
		}
		else {
			ExecuteSql("UPDATE FormControlsT SET Source = '' WHERE ID = 6614");
		}

		// (j.jones 2008-06-12 12:05) - PLID 23052 - moved initialization of forms 90 and 92
		// into UpdateUB04Charges
		m_pframe->Load (90, "", "", &i); // Insurance apply totals

		// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" applies
		g_aryFormQueries[91].sql = "SELECT LineItemT.PatientID AS [Patient ID], PaymentsT.InsuredPartyID, "
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
		m_pframe->Load (91, where, "", &i); // Other insurance apply totals

		m_pframe->Load (92, "", "", &i); // Charge totals
		//TES 3/21/2007 - PLID 23939 - There was special handling on the UB92 for form 92, because showing the Totals was optional.
		// It's not optional on the UB04, so we just let the normal loading for this form load that field.

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
	*/

		// (j.jones 2008-06-12 10:46) - PLID 23052 - added support for multiple pages
		RECT rect;
		rect.left	= 4 scale;
		rect.right	= 18 scale;
		rect.top	= 312 scale;
		rect.bottom = 487 scale;
		if (!m_pupbutton->Create ("/\\", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, rect, m_pframe, IDC_BILL_UP)) {
			AfxMessageBox ("Failed to create button IDC_BILL_UP");
		}
		rect.top	= 488 scale;
		rect.bottom = 662 scale;
		if (!m_pdownbutton->Create ("\\/", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, rect, m_pframe, IDC_BILL_DOWN)) {
			AfxMessageBox ("Failed to create button IDC_BILL_DOWN");
		}

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

		m_pframe->m_pPrePrintFunc = m_pPrePrintFunc;

		m_pframe->TrackChanges(0, 0);
	}NxCatchAll("Error in CUB04Dlg::OnInitDialog()");

	return TRUE;
}

void CUB04Dlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	SetControlPositions();
}

void CUB04Dlg::SetControlPositions(void)
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

void CUB04Dlg::OnClickX()
{
	EndDialog (0);
}

void CUB04Dlg::OnClickCheck() 
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

void CUB04Dlg::OnClose() 
{
	CDialog::OnClose();
}

BOOL CUB04Dlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	return CDialog::OnCommand(wParam, lParam);
}

extern int FONT_SIZE;
extern int MINI_FONT_SIZE;
void CUB04Dlg::OnClickPrint() 
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
			//charges
			aryIDsToIgnore.Add(6378);
			aryIDsToIgnore.Add(6495);
			aryIDsToIgnore.Add(6496);
			aryIDsToIgnore.Add(6497);
			aryIDsToIgnore.Add(6498);
			aryIDsToIgnore.Add(6499);
			aryIDsToIgnore.Add(6500);
			aryIDsToIgnore.Add(6501);
			aryIDsToIgnore.Add(6502);
			aryIDsToIgnore.Add(6503);
			aryIDsToIgnore.Add(6504);
			aryIDsToIgnore.Add(6505);
			aryIDsToIgnore.Add(6506);
			aryIDsToIgnore.Add(6507);
			aryIDsToIgnore.Add(6508);
			aryIDsToIgnore.Add(6509);
			aryIDsToIgnore.Add(6510);
			aryIDsToIgnore.Add(6511);
			aryIDsToIgnore.Add(6512);
			aryIDsToIgnore.Add(6513);
			aryIDsToIgnore.Add(6514);
			aryIDsToIgnore.Add(6515);
			//non-covered charges
			aryIDsToIgnore.Add(6380);
			aryIDsToIgnore.Add(6539);
			aryIDsToIgnore.Add(6540);
			aryIDsToIgnore.Add(6541);
			aryIDsToIgnore.Add(6542);
			aryIDsToIgnore.Add(6543);
			aryIDsToIgnore.Add(6544);
			aryIDsToIgnore.Add(6545);
			aryIDsToIgnore.Add(6546);
			aryIDsToIgnore.Add(6547);
			aryIDsToIgnore.Add(6548);
			aryIDsToIgnore.Add(6549);
			aryIDsToIgnore.Add(6550);
			aryIDsToIgnore.Add(6551);
			aryIDsToIgnore.Add(6552);
			aryIDsToIgnore.Add(6553);
			aryIDsToIgnore.Add(6554);
			aryIDsToIgnore.Add(6555);
			aryIDsToIgnore.Add(6556);
			aryIDsToIgnore.Add(6557);
			aryIDsToIgnore.Add(6558);
			aryIDsToIgnore.Add(6559);
			//total charges
			aryIDsToIgnore.Add(6322);
			//total non-covered charges
			aryIDsToIgnore.Add(6323);
			//prior payments
			aryIDsToIgnore.Add(6614);
			aryIDsToIgnore.Add(6615);
			aryIDsToIgnore.Add(6616);
			//est. amount due
			aryIDsToIgnore.Add(6618);
			aryIDsToIgnore.Add(6622);
			aryIDsToIgnore.Add(6623);
			//boxes 39 - 41
			aryIDsToIgnore.Add(6318);
			aryIDsToIgnore.Add(6319);
			aryIDsToIgnore.Add(6320);
			aryIDsToIgnore.Add(6321);
			aryIDsToIgnore.Add(6313);
			aryIDsToIgnore.Add(6329);
			aryIDsToIgnore.Add(6331);
			aryIDsToIgnore.Add(6332);
			aryIDsToIgnore.Add(6345);
			aryIDsToIgnore.Add(6346);
			aryIDsToIgnore.Add(6347);
			aryIDsToIgnore.Add(6348);
		}
		if(m_UB92Info.PunctuateDiagCodes) {
			aryIDsToIgnore.Add(6671);
			aryIDsToIgnore.Add(6672);
			aryIDsToIgnore.Add(6673);
			aryIDsToIgnore.Add(6674);
			aryIDsToIgnore.Add(6675);
			aryIDsToIgnore.Add(6676);
			aryIDsToIgnore.Add(6677);
			aryIDsToIgnore.Add(6678);
			aryIDsToIgnore.Add(6679);
			aryIDsToIgnore.Add(6680);
			aryIDsToIgnore.Add(6214);
			aryIDsToIgnore.Add(6223);
			aryIDsToIgnore.Add(6266);
			aryIDsToIgnore.Add(6325);
			aryIDsToIgnore.Add(6336);
			aryIDsToIgnore.Add(6349);
			aryIDsToIgnore.Add(6350);
			aryIDsToIgnore.Add(6351);
			aryIDsToIgnore.Add(6352);
			aryIDsToIgnore.Add(6379);
			aryIDsToIgnore.Add(6683);
			aryIDsToIgnore.Add(6472);
			aryIDsToIgnore.Add(6494);
			aryIDsToIgnore.Add(6516);
			aryIDsToIgnore.Add(6517);
			aryIDsToIgnore.Add(6518);
			aryIDsToIgnore.Add(6519);
			aryIDsToIgnore.Add(6520);
		}

		// (j.jones 2014-08-01 16:36) - PLID 63104 - do not strip punctuation from Box 43
		aryIDsToIgnore.Add(6375);
		aryIDsToIgnore.Add(6407);
		aryIDsToIgnore.Add(6408);
		aryIDsToIgnore.Add(6409);
		aryIDsToIgnore.Add(6410);
		aryIDsToIgnore.Add(6411);
		aryIDsToIgnore.Add(6412);
		aryIDsToIgnore.Add(6413);
		aryIDsToIgnore.Add(6414);
		aryIDsToIgnore.Add(6415);
		aryIDsToIgnore.Add(6416);
		aryIDsToIgnore.Add(6417);
		aryIDsToIgnore.Add(6418);
		aryIDsToIgnore.Add(6419);
		aryIDsToIgnore.Add(6420);
		aryIDsToIgnore.Add(6421);
		aryIDsToIgnore.Add(6422);
		aryIDsToIgnore.Add(6423);
		aryIDsToIgnore.Add(6424);
		aryIDsToIgnore.Add(6425);
		aryIDsToIgnore.Add(6426);
		aryIDsToIgnore.Add(6427);

		PRINT_X_OFFSET += x_offset * 10;
		PRINT_Y_OFFSET += y_offset * 10;
		PRINT_X_SCALE += x_scale * 0.01;
		PRINT_Y_SCALE += y_scale * 0.01;
		if(!m_pframe->OnPrint(m_CapOnPrint, "UB04", &aryIDsToIgnore, m_pPrintDC)) {
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

		//Send Type 2 - Paper UB04
		
		// (j.jones 2013-01-23 09:19) - PLID 54734 - moved the claim history addition to its own function,
		// this also updates HCFATrackT.LastSendDate
		AddToClaimHistory(m_ID, m_InsuredPartyID, ClaimSendType::UB);

		//now add to patient history
		CString str, strDesc = "UB04 Printed";
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
	}NxCatchAll("Error printing UB04 form.");
}

void CUB04Dlg::BuildFormsT_Form6()
{
	CString strSQL, str;

	try {

		// (j.jones 2006-12-04 15:25) - PLID 23733 - supported the ClaimProviderID override
		// (j.jones 2010-11-09 15:58) - PLID 41387 - supported ChargesT.ClaimProviderID
		strSQL.Format("FormChargesT.DoctorsProviders AS FirstOfDoctorsProviders, DoctorInfo.[Last] AS DocLastName, DoctorInfo.[First] AS DocFirstName, DoctorInfo.[Middle] AS DocMiddleName, GetDate() AS TodaysDate, "
				"DoctorInfo.[Title] + ' ' + DoctorInfo.[First] + ' ' + DoctorInfo.[Middle] + ' ' + DoctorInfo.[Last] AS DocSignature, DoctorInfo.SocialSecurity, DoctorInfo.[Fed Employer ID] FROM FormChargesT "
				"INNER JOIN (SELECT * FROM PersonT INNER JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID) AS DoctorInfo ON (CASE WHEN ChargesT.ClaimProviderID Is Not Null THEN ChargesT.ClaimProviderID ELSE FormChargesT.DoctorsProviders END) = DoctorInfo.RealProviderID");
		str.Format("FROM (%s) AS FormChargesT",g_aryFormQueries[2].sql);
		strSQL.Replace("FROM FormChargesT",str);
		g_aryFormQueries[6].sql = strSQL;
	} NxCatchAll("UB04Dlg::BuildFormsT_Form6()");
}

void CUB04Dlg::BuildUB04ChargesT()
{
	extern int SizeOfFormChargesT();

	try {
		//ExecuteSql("DELETE FROM FormChargesT");

		_RecordsetPtr rs;
		CString str;

		billid = m_ID;

		// Fill temporary HCFA charges table with all the charges
		// for this bill
		// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" charges
		// (a.walling 2014-03-13 09:06) - PLID 61305 - Emulate the old WhichCodes field via ChargeWhichCodesFlatV
		// (a.walling 2014-03-17 17:20) - PLID 61405 - Unique diag codes and recalculated WhichCodes using BillDiagsXmlFlat4IF and ChargeWhichCodesIF - UB04, UB92
		g_aryFormQueries[2].sql.Format("SELECT ChargesT.*, ChargeWhichCodesQ.WhichCodes, LineItemT.[Date] AS TDate, "
			"LineItemT.Description AS ItemDesc, "
			"LineItemT.PatientID FROM BillsT INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
			"OUTER APPLY dbo.%s(BillsT.ID) BillDiagCodeFlatV "
			"OUTER APPLY dbo.%s(BillDiagCodeFlatV.BillDiagsXml, ChargesT.ID) ChargeWhichCodesQ "
			"WHERE LineItemT.Deleted = 0 AND Batched = 1 "
			"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
			"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
			"AND BillsT.ID = %li"
			, ShouldUseICD10(m_InsuredPartyID, m_ID) ? "BillICD10DiagsXmlFlat4IF" : "BillICD9DiagsXmlFlat4IF"
			, ShouldUseICD10(m_InsuredPartyID, m_ID) ? "ChargeWhichICD10CodesIF" : "ChargeWhichICD9CodesIF"
			, billid);

		// Build form 6 in the form controls table
		BuildFormsT_Form6();

		// Now build a list of different charge providers. Assume every charge
		// has a provider in FormChargesT.

		adwProviderList.RemoveAll();
		// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" charges
		rs = CreateParamRecordset("SELECT DoctorsProviders FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
			"WHERE ChargesT.BillID = {INT} AND LineItemT.Deleted = 0 AND Batched = 1 "
			"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
			"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null", billid);
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
			//we don't support multiple providers in this code
		}

		if(m_UB92Info.Box80Number == 0) {
			// (a.walling 2007-06-20 13:57) - PLID 26414 - If they have the preference to use ICD-9-CM Codes instead of CPT, do that.
			if (m_UB92Info.UB04UseICD9ProcedureCodes == 1) {
				//show CPT code
				ExecuteSql("UPDATE FormControlsT SET Source = 'TopICD9CM' WHERE ID = 6682");
				ExecuteSql("UPDATE FormControlsT SET Source = 'FirstDate' WHERE ID = 6684");
			} else {
				//show CPT code
				ExecuteSql("UPDATE FormControlsT SET Source = 'TopCPT' WHERE ID = 6682");
				ExecuteSql("UPDATE FormControlsT SET Source = 'FirstDate' WHERE ID = 6684");
			}
		}
		else if(m_UB92Info.Box80Number == 1) {
			//show ICD-9 code
			ExecuteSql("UPDATE FormControlsT SET Source = 'Diag1' WHERE ID = 6682");
			ExecuteSql("UPDATE FormControlsT SET Source = 'FirstDate' WHERE ID = 6684");
		}
		else {
			//show nothing
			ExecuteSql("UPDATE FormControlsT SET Source = '' WHERE ID = 6682");
			ExecuteSql("UPDATE FormControlsT SET Source = '' WHERE ID = 6684");
		}

		//reinitialize
		//TES 3/20/2007 - PLID 23939 - Updated UB92 Boxes 32-36 as UB04 Boxes 31-35, and added UB04 Box 36
		// (a.walling 2007-06-20 14:04) - PLID 26414 - Added TopICD9CM procedure code
		// (j.jones 2008-05-21 10:44) - PLID 30129 - added UB04 Box 66 default
		// (j.jones 2008-06-09 15:40) - PLID 30229 - included the HospFrom field
		// (j.jones 2009-12-22 10:38) - PLID 27131 - included the ConditionDate field
		// (j.jones 2010-03-12 16:56) - PLID 37440 - changed FirstDate/LastDate to use the min/max ServiceDateFrom/ServiceDateTo
		// (a.walling 2014-03-06 13:24) - PLID 61228 - Support BillDiagCodeT via BillDiagCodeICD9FlatV/ICD10FlatV aliased as BillDiagCodeFlatV
		// (a.walling 2014-03-17 17:20) - PLID 61405 - Unique diag codes and recalculated WhichCodes using BillDiagsXmlFlat4IF and ChargeWhichCodesIF - UB04, UB92
		// (b.spivey July 8, 2015) PLID 66516 - pull null if we're not filling box 12
		// (a.walling 2016-03-10 07:36) - PLID 68550 - UB04 Enhancements - update UB04 paper forms to load from UB04ClaimInfo object
		// (j.jones 2016-05-24 8:45) - NX-100705 - added BillsT.OriginalRefNo
		g_aryFormQueries[84].sql.Format("SELECT Min(TopCPT.ItemCode) AS TopCPT, Min(TopICD9CM.Code) AS TopICD9CM, "
			"Min([FormChargesT].[ServiceDateFrom]) AS FirstDate, Max([FormChargesT].[ServiceDateTo]) AS LastDate, "
			"Min(DiagCodes1.CodeNumber) AS Diag1, Min(DiagCodes2.CodeNumber) AS Diag2, Min(DiagCodes3.CodeNumber) AS Diag3, Min(DiagCodes4.CodeNumber) AS Diag4, "
			"  MIN(COALESCE(UB04ClaimInfo.value('(/claimInfo/occurrences/occurrence/@code)[1]', 'nvarchar(20)'), '')) as UB04Box31 "
			", MIN(COALESCE(UB04ClaimInfo.value('(/claimInfo/occurrences/occurrence/@code)[2]', 'nvarchar(20)'), '')) as UB04Box32 "
			", MIN(COALESCE(UB04ClaimInfo.value('(/claimInfo/occurrences/occurrence/@code)[3]', 'nvarchar(20)'), '')) as UB04Box33 "
			", MIN(COALESCE(UB04ClaimInfo.value('(/claimInfo/occurrences/occurrence/@code)[4]', 'nvarchar(20)'), '')) as UB04Box34 "
			", MIN(COALESCE(UB04ClaimInfo.value('(/claimInfo/occurrenceSpans/occurrenceSpan/@code)[1]', 'nvarchar(20)'), '')) as UB04Box35 "
			", MIN(COALESCE(UB04ClaimInfo.value('(/claimInfo/occurrenceSpans/occurrenceSpan/@code)[2]', 'nvarchar(20)'), '')) as UB04Box36, "
			"Min([FormChargesT].[BillID]) AS BillID, Min([Date]) AS BillDate, GetDate() AS TodaysDate, "
			"CASE WHEN Max([RefPhyID]) > 0 THEN Max([RefPhyID]) ELSE NULL END AS ReferringID, "
			"Max(PersonT.[Last] + ', ' + PersonT.[First] + ' ' + PersonT.Middle + (CASE WHEN (PersonT.Title <> '') THEN (', ' + PersonT.Title) ELSE ('') END)) AS ReferringName , "
			"Max([PriorAuthNum]) AS PriorAuth, 'Signature On File' AS SigOnFile, Max('%s') AS Box66, "
			"CASE WHEN %li <> 0 THEN Min(BillsT.HospFrom) ELSE NULL END AS HospFrom, Min(BillsT.ConditionDate) AS ConditionDate, "
			"Min(BillsT.OriginalRefNo) AS OriginalRefNo "
			"FROM FormChargesT "
			"INNER JOIN BillsT ON FormChargesT.BillID = BillsT.ID "
			"OUTER APPLY dbo.%s(BillsT.ID) BillDiagCodeFlatV "
			"LEFT JOIN DiagCodes DiagCodes1 ON BillDiagCodeFlatV.Diag1ID = DiagCodes1.ID "
			"LEFT JOIN DiagCodes DiagCodes2 ON BillDiagCodeFlatV.Diag2ID = DiagCodes2.ID "
			"LEFT JOIN DiagCodes DiagCodes3 ON BillDiagCodeFlatV.Diag3ID = DiagCodes3.ID "
			"LEFT JOIN DiagCodes DiagCodes4 ON BillDiagCodeFlatV.Diag4ID = DiagCodes4.ID "
			"LEFT JOIN PersonT ON BillsT.RefPhyID = PersonT.ID "
			"LEFT JOIN (SELECT BillID, ItemCode FROM ChargesT WHERE LineID = 1) AS TopCPT ON BillsT.ID = TopCPT.BillID "
			"LEFT JOIN (SELECT BillID, ICD9ProcedureT.Code FROM ChargesT "
			"INNER JOIN CPTCodeT ON ChargesT.ServiceID = CPTCodeT.ID "
			"INNER JOIN ICD9ProcedureT ON CPTCodeT.ICD9ProcedureCodeID = ICD9ProcedureT.ID WHERE LineID = 1) AS TopICD9CM ON BillsT.ID = TopICD9CM.BillID",
			_Q(m_UB92Info.UB04Box66), 
			m_UB92Info.FillBox12_13,
			ShouldUseICD10(m_InsuredPartyID, m_ID) ? "BillICD10DiagsXmlFlat4IF" : "BillICD9DiagsXmlFlat4IF"
		);

		//show "Signature On File" in Box85		
		if(m_UB92Info.Box85Show == 0)
			//do NOT show anything in Box85
			g_aryFormQueries[84].sql.Replace("Signature On File","");
		
		//TES 3/21/2007 - PLID 25295 - "Box 51 #" is now "Other Prv ID", and can be filled on either or both of Boxes 51 
		// and 57.  Load the value, if it belongs in either box, then assign it to whichever boxes it belongs in.
		CString strBox51A = "", strBox51B = "", strBox51C = "";
		CString strBox57A = "", strBox57B = "", strBox57C = "";

		if(m_UB92Info.UB04Box51Show || m_UB92Info.UB04Box57Show) {
			CString strOtherPrvIDA = "", strOtherPrvIDB = "", strOtherPrvIDC = "";
			//primary insurance (on the bill) goes to strBox51A

			// (j.jones 2006-12-04 15:25) - PLID 23733 - supported the ClaimProviderID override
			// (j.jones 2010-11-09 15:44) - PLID 41387 - supported ChargesT.ClaimProviderID
			rs = CreateParamRecordset("SELECT InsuranceBox51.Box51Info AS Box51 FROM (ChargesT INNER JOIN (((InsuranceCoT INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID) INNER JOIN InsuranceBox51 ON InsuranceCoT.PersonID = InsuranceBox51.InsCoID) INNER JOIN BillsT ON (InsuredPartyT.PersonID = BillsT.InsuredPartyID) AND (InsuredPartyT.PatientID = BillsT.PatientID)) ON ChargesT.BillID = BillsT.ID) "
				"INNER JOIN ProvidersT ActualProvidersT ON ChargesT.DoctorsProviders = ActualProvidersT.PersonID "
				"INNER JOIN ProvidersT ON (CASE WHEN ChargesT.ClaimProviderID Is Not Null THEN ChargesT.ClaimProviderID ELSE ActualProvidersT.ClaimProviderID END) = ProvidersT.PersonID "
				"	AND (InsuranceBox51.ProviderID = ProvidersT.PersonID) "
				"GROUP BY InsuranceBox51.Box51Info, BillsT.ID HAVING (((BillsT.ID)={INT}))", m_ID);
			if (!rs->eof) {
				strOtherPrvIDA = AdoFldString(rs, "Box51","");
			}
			rs->Close();

			if(strOtherPrvIDA == "") {
				//if blank, then try to load the default Box 51 from the UB04 group
				strOtherPrvIDA = m_UB92Info.Box51Default;
			}

			//secondary insurance (on the bill) goes to strBox51B

			// (j.jones 2006-12-04 15:25) - PLID 23733 - supported the ClaimProviderID override
			// (j.jones 2010-11-09 15:44) - PLID 41387 - supported ChargesT.ClaimProviderID
			rs = CreateParamRecordset("SELECT InsuranceBox51.Box51Info AS Box51 FROM (ChargesT INNER JOIN (((InsuranceCoT INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID) INNER JOIN InsuranceBox51 ON InsuranceCoT.PersonID = InsuranceBox51.InsCoID) INNER JOIN BillsT ON (InsuredPartyT.PersonID = BillsT.OthrInsuredPartyID) AND (InsuredPartyT.PatientID = BillsT.PatientID)) ON ChargesT.BillID = BillsT.ID) "
				"INNER JOIN ProvidersT ActualProvidersT ON ChargesT.DoctorsProviders = ActualProvidersT.PersonID "
				"INNER JOIN ProvidersT ON (CASE WHEN ChargesT.ClaimProviderID Is Not Null THEN ChargesT.ClaimProviderID ELSE ActualProvidersT.ClaimProviderID END) = ProvidersT.PersonID "
				"	AND (InsuranceBox51.ProviderID = ProvidersT.PersonID) "
				"GROUP BY InsuranceBox51.Box51Info, BillsT.ID HAVING (((BillsT.ID)={INT}))", m_ID);
			if (!rs->eof) {
				strOtherPrvIDB = AdoFldString(rs, "Box51","");
			}
			rs->Close();

			if(strOtherPrvIDB == "") {
				//if blank, then try to load the default Box 51 from the UB04 group
				rs = CreateRecordset("SELECT UB92SetupT.Box51Default AS Box51 FROM (ChargesT INNER JOIN (((InsuranceCoT INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID) INNER JOIN UB92SetupT ON InsuranceCoT.UB92SetupGroupID = UB92SetupT.ID) INNER JOIN BillsT ON (InsuredPartyT.PersonID = BillsT.OthrInsuredPartyID) AND (InsuredPartyT.PatientID = BillsT.PatientID)) ON ChargesT.BillID = BillsT.ID) "
								"GROUP BY Box51Default, BillsT.ID HAVING (((BillsT.ID)=%li))", m_ID);
				if (!rs->eof) {
					strOtherPrvIDB = AdoFldString(rs, "Box51","");
				}
				rs->Close();
			}

			//TES 3/21/2007 - PLID 25295 - OK, we have our number, now fill it in to the appropriate boxes.
			if(m_UB92Info.UB04Box51Show) {
				strBox51A = strOtherPrvIDA;
				strBox51B = strOtherPrvIDB;
				strBox51C = strOtherPrvIDC;
			}
			if(m_UB92Info.UB04Box57Show) {
				strBox57A = strOtherPrvIDA;
				strBox57B = strOtherPrvIDB;
				strBox57C = strOtherPrvIDC;
			}
		}

		// (j.jones 2007-03-20 11:35) - PLID 25278 - load the Box 56 NPI value
		CString strBox56NPI = "''";

		// (j.jones 2007-03-20 11:37) - PLID 25279 - load the Box 81 Taxonomy Codes
		CString strBox81aQual = "''";
		CString strBox81aTaxonomy = "''";
		CString strBox81bQual = "''";
		CString strBox81bTaxonomy = "''";
		CString strBox81cQual = "''";
		CString strBox81cTaxonomy = "''";

		// (j.jones 2007-03-20 11:35) - PLID 25278 - load the Box 56 NPI value
		if(m_UB92Info.UB04Box56NPI == 3) { //use Bill Location

			// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" charges
			rs = CreateParamRecordset("SELECT TOP 1 NPI FROM ChargesT "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"INNER JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				"WHERE Batched = 1 AND Deleted = 0 "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"AND BillID = {INT}",m_ID);
			if(!rs->eof) {
				strBox56NPI.Format("'%s'", _Q(AdoFldString(rs, "NPI","")));
			}
			rs->Close();
		}
		//load from the G1 Provider if we need their NPI or any Taxonomy Code
		if(m_UB92Info.UB04Box56NPI == 2 ||
			m_UB92Info.UB04Box81a == 2 || m_UB92Info.UB04Box81b == 2 || m_UB92Info.UB04Box81c == 2) {

			// (j.jones 2010-11-09 16:00) - PLID 41387 - supported ChargesT.ClaimProviderID (first charge only)
			// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" charges
			rs = CreateParamRecordset("SELECT TOP 1 NPI, TaxonomyCode FROM BillsT INNER JOIN PatientsT ON BillsT.PatientID = PatientsT.PersonID "
				"INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
						"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"INNER JOIN (SELECT PersonID, ClaimProviderID FROM ProvidersT) ActualProvidersT ON PatientsT.MainPhysician = ActualProvidersT.PersonID "
				"INNER JOIN ProvidersT ON (CASE WHEN ChargesT.ClaimProviderID Is Not Null THEN ChargesT.ClaimProviderID ELSE ActualProvidersT.ClaimProviderID END) = ProvidersT.PersonID "
				"INNER JOIN PersonT ON ProvidersT.PersonID = PersonT.ID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				"WHERE LineItemT.Deleted = 0 AND ChargesT.Batched = 1 "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"AND BillsT.ID = {INT}",m_ID);
			if(!rs->eof) {
				//if we need the G1 NPI, assign it
				if(m_UB92Info.UB04Box56NPI == 2)
					strBox56NPI.Format("'%s'", _Q(AdoFldString(rs, "NPI","")));

				CString strTaxonomy = AdoFldString(rs, "TaxonomyCode","");

				//if we need the G1 Taxonomy Code, assign it
				if(m_UB92Info.UB04Box81a == 2) {
					strBox81aTaxonomy.Format("'%s'", _Q(strTaxonomy));
					//if non-empty, use the Box81a qualifier
					if(!strTaxonomy.IsEmpty()) {
						strBox81aQual.Format("'%s'", _Q(m_UB92Info.UB04Box81aQual));
					}
				}
				if(m_UB92Info.UB04Box81b == 2) {
					strBox81bTaxonomy.Format("'%s'", _Q(strTaxonomy));
					//if non-empty, use the Box81b qualifier
					if(!strTaxonomy.IsEmpty()) {
						strBox81bQual.Format("'%s'", _Q(m_UB92Info.UB04Box81bQual));
					}
				}
				if(m_UB92Info.UB04Box81c == 2) {
					strBox81cTaxonomy.Format("'%s'", _Q(strTaxonomy));
					//if non-empty, use the Box81c qualifier
					if(!strTaxonomy.IsEmpty()) {
						strBox81cQual.Format("'%s'", _Q(m_UB92Info.UB04Box81cQual));
					}
				}
			}
			rs->Close();
		}
		if (m_UB92Info.UB04Box56NPI == 1) { //use Bill Provider

			//no need to generate the ID, we're already pulling it in query 87
			strBox56NPI = "ProvidersT.NPI";
		}

		// (j.jones 2007-03-20 11:37) - PLID 25279 - load the Box 81 Taxonomy Codes

		//at this point, if any of the taxonomy fields use the G1 provider,
		//it will have already been loaded

		if(m_UB92Info.UB04Box81a == 1) { //use Bill Provider

			//no need to generate the ID, we're already pulling it in query 87
			strBox81aTaxonomy = "ProvidersT.TaxonomyCode";
			strBox81aQual.Format("CASE WHEN Coalesce(ProvidersT.TaxonomyCode,'') <> '' THEN '%s' ELSE '' END", _Q(m_UB92Info.UB04Box81aQual));
		}
		if(m_UB92Info.UB04Box81b == 1) { //use Bill Provider

			//no need to generate the ID, we're already pulling it in query 87
			strBox81bTaxonomy = "ProvidersT.TaxonomyCode";
			strBox81bQual.Format("CASE WHEN Coalesce(ProvidersT.TaxonomyCode,'') <> '' THEN '%s' ELSE '' END", _Q(m_UB92Info.UB04Box81bQual));
		}
		if(m_UB92Info.UB04Box81c == 1) { //use Bill Provider

			//no need to generate the ID, we're already pulling it in query 87
			strBox81cTaxonomy = "ProvidersT.TaxonomyCode";
			strBox81cQual.Format("CASE WHEN Coalesce(ProvidersT.TaxonomyCode,'') <> '' THEN '%s' ELSE '' END", _Q(m_UB92Info.UB04Box81cQual));
		}

		//like the G1 loading, if any box wants the referring physician taxonomy code,
		//load it only once, and assign out where necessary
		// (j.jones 2013-04-05 15:56) - PLID 40960 - Referring Physicians don't have taxonomy codes anymore
		/*
		if(m_UB92Info.UB04Box81a == 3 || m_UB92Info.UB04Box81b == 3 || m_UB92Info.UB04Box81c == 3) { //use Referring Physician

			rs = CreateRecordset("SELECT TaxonomyCode "
				"FROM BillsT INNER JOIN ReferringPhysT ON BillsT.RefPhyID = ReferringPhysT.PersonID "
				"INNER JOIN PersonT ON ReferringPhysT.PersonID = PersonT.ID WHERE BillsT.ID = %li",m_ID);
			if(!rs->eof) {
				CString strTaxonomy = AdoFldString(rs, "TaxonomyCode","");
				if(m_UB92Info.UB04Box81a == 3) {
					strBox81aTaxonomy.Format("'%s'", _Q(strTaxonomy));
					//if non-empty, use the Box81a qualifier
					if(!strTaxonomy.IsEmpty()) {
						strBox81aQual.Format("'%s'", _Q(m_UB92Info.UB04Box81aQual));
					}
				}
				if(m_UB92Info.UB04Box81b == 3) {
					strBox81bTaxonomy.Format("'%s'", _Q(strTaxonomy));
					//if non-empty, use the Box81b qualifier
					if(!strTaxonomy.IsEmpty()) {
						strBox81bQual.Format("'%s'", _Q(m_UB92Info.UB04Box81bQual));
					}
				}
				if(m_UB92Info.UB04Box81c == 3) {
					strBox81cTaxonomy.Format("'%s'", _Q(strTaxonomy));
					//if non-empty, use the Box81c qualifier
					if(!strTaxonomy.IsEmpty()) {
						strBox81cQual.Format("'%s'", _Q(m_UB92Info.UB04Box81cQual));
					}
				}
			}
			rs->Close();
		}
		*/

		// (j.jones 2006-12-04 15:25) - PLID 23733 - supported the ClaimProviderID override
		//TES 3/21/2007 - PLID 25295 - Support filling Box 57.
		// (j.jones 2010-11-09 16:27) - PLID 41387 - supported ChargesT.ClaimProviderID
		g_aryFormQueries[87].sql.Format("SELECT FormChargesT.DoctorsProviders AS FirstOfDoctorsProviders, "
			"PersonT.[Last] AS DocLastName, PersonT.[First] AS DocFirstName, PersonT.[Middle] AS DocMiddleName, GetDate() AS TodaysDate, "
			"PersonT.[First] + ' ' + PersonT.[Middle] + ' ' + PersonT.[Last] + (CASE WHEN (PersonT.Title <> '') THEN (', ' + PersonT.Title) ELSE ('') END) AS DocSignature, "
			"PersonT.Address1 AS DocAdd1, PersonT.Address2 AS DocAdd2, PersonT.City + ', ' + PersonT.State + ' ' + PersonT.Zip AS DocAdd3, "
			"PersonT.SocialSecurity, ProvidersT.[Fed Employer ID], ProvidersT.NPI, ProvidersT.[Medicare Number], "
			"'%s' AS Box51A, '%s' AS Box51B, '%s' AS Box51C, '%s' AS Box57A, '%s' AS Box57B, '%s' AS Box57C, "
			"%s AS Box81aQual, %s AS Box81aTaxonomy, %s AS Box81bQual, %s AS Box81bTaxonomy, %s AS Box81cQual, %s AS Box81cTaxonomy, "
			"%s AS Box56NPI "
			"FROM PersonT INNER JOIN (FormChargesT INNER JOIN ProvidersT ActualProvidersT ON FormChargesT.DoctorsProviders = ActualProvidersT.PersonID "
			"	INNER JOIN ProvidersT ON (CASE WHEN FormChargesT.ClaimProviderID Is Not Null THEN FormChargesT.ClaimProviderID ELSE ActualProvidersT.ClaimProviderID END) = ProvidersT.PersonID) "
			"ON PersonT.ID = ProvidersT.PersonID",
			_Q(strBox51A),_Q(strBox51B),_Q(strBox51C),_Q(strBox57A),_Q(strBox57B),_Q(strBox57C),
			strBox81aQual, strBox81aTaxonomy, strBox81bQual, strBox81bTaxonomy, strBox81cQual, strBox81cTaxonomy,
			strBox56NPI);
		
		str.Format("FROM (%s) AS FormChargesT",g_aryFormQueries[2].sql);

		// (j.jones 2014-08-01 11:38) - PLID 63104 - changed the Fill42With44 logic to just define
		// the RevCode value, not redefine the entire query
		CString strRevCodeInfo = "(CASE WHEN ServiceT.RevCodeUse = 1 THEN UB92CategoriesT1.Code WHEN ServiceT.RevCodeUse = 2 THEN UB92CategoriesT2.Code ELSE NULL END)";
		if (m_UB92Info.Fill42With44) {
			//if no RevCode, use the CPT code
			strRevCodeInfo = "COALESCE((CASE WHEN ServiceT.RevCodeUse = 1 THEN UB92CategoriesT1.Code WHEN ServiceT.RevCodeUse = 2 THEN UB92CategoriesT2.Code ELSE NULL END), "
				"(ItemCode + (CASE WHEN CPTModifier Is Null THEN '' ELSE (' ' + CPTModifier) END) + (CASE WHEN CPTModifier2 Is Null THEN '' ELSE (' ' + CPTModifier2) END)+ (CASE WHEN CPTModifier3 Is Null THEN '' ELSE (' ' + CPTModifier3) END) + (CASE WHEN CPTModifier4 Is Null THEN '' ELSE (' ' + CPTModifier4) END)))";
		}

		//Group revenue Codes together?
		if(m_UB92Info.GroupRev == 0) {
			//do not group by revenue code
			//this method will show the revenue code, cpt code, and line item description
			//(If RevCodeUse is 2, the revenue code is based on the Insurance Company.)
			//this method also sorts in the same way the bill does
			// (j.jones 2007-03-27 11:24) - PLID 25304 - these queries did not handle the case where
			// neither Revenue Code option was enabled for the CPT code. Now they do.
			// (j.jones 2008-06-12 14:03) - PLID 23052 - need to include the total amount applied to each line, for use in UpdateUB04Charges
			// (a.vengrofski 2010-02-03 15:13) - PLID <35133> - Added two CASEs to the query to fill in the Modifier 3 & 4
			// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" charges & applies
			// (j.jones 2014-08-01 10:32) - PLID 63104 - charge lab codes can now override the charge description				
			g_aryFormQueries[83].sql.Format("SELECT %s AS RevCode, "
					"(ItemCode + (CASE WHEN CPTModifier Is Null THEN '' ELSE (' ' + CPTModifier) END) + (CASE WHEN CPTModifier2 Is Null THEN '' ELSE (' ' + CPTModifier2) END)+ (CASE WHEN CPTModifier3 Is Null THEN '' ELSE (' ' + CPTModifier3) END) + (CASE WHEN CPTModifier4 Is Null THEN '' ELSE (' ' + CPTModifier4) END)) AS ItemCode, "
					"CASE WHEN IsNull(ChargeLabTestCodesQ.LabTestCodes, '') <> '' THEN ChargeLabTestCodesQ.LabTestCodes "
					"	ELSE LineItemT.Description END AS ItemDesc, "
					"LineItemT.[Date] AS TDate, Quantity, dbo.GetChargeTotal(ChargesT.ID) AS Amount, "
					"Coalesce(AppliesQ.TotalApplied,Convert(money,0)) AS AppliedAmount "
					"FROM BillsT "
					"INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
					"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
					"LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
					"LEFT JOIN UB92CategoriesT UB92CategoriesT1 ON ServiceT.UB92Category = UB92CategoriesT1.ID "
					"LEFT JOIN (SELECT ServiceRevCodesT.*, PersonID AS InsuredPartyID FROM ServiceRevCodesT INNER JOIN InsuredPartyT ON ServiceRevCodesT.InsuranceCompanyID = InsuredPartyT.InsuranceCoID) AS ServiceRevCodesT ON BillsT.InsuredPartyID = ServiceRevCodesT.InsuredPartyID AND ServiceT.ID = ServiceRevCodesT.ServiceID "
					"LEFT JOIN UB92CategoriesT UB92CategoriesT2 ON ServiceRevCodesT.UB92CategoryID = UB92CategoriesT2.ID "
					"LEFT JOIN (SELECT Sum(Round(AppliesT.Amount,2)) AS TotalApplied, DestID "
					"	FROM AppliesT "
					"	INNER JOIN LineItemT ON AppliesT.SourceID = LineItemT.ID "
					"	LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
					"	LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
					"	WHERE Deleted = 0 "
					"	AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
					"	AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
					"	GROUP BY DestID) AS AppliesQ ON ChargesT.ID = AppliesQ.DestID "

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
					"WHERE LineItemT.Deleted = 0 AND Batched = 1 "
					"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
					"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
					"AND BillsT.ID = %li "
					"ORDER BY ChargesT.LineID", strRevCodeInfo, billid);
		}
		else {
			//Group by revenue code
			//This method (more common) will show based on the selection for the given item:
			//If RevCodeUse is 1, then all charges with that revenue code will group and total together,
			//and show the revenue code name. (If RevCodeUse is 2, the revenue code is based on the Insurance Company.)
			//If RevCodeUse is 0, then the charge will show on its own with the CPT code

			// (j.jones 2006-11-07 10:00) - PLID 22534 - BillsT.UB04Box44 is used in any case where Box 44 is not filled
			CString strBox44Override = "";
			_RecordsetPtr rsCode = CreateParamRecordset("SELECT Code FROM BillsT INNER JOIN CPTCodeT ON BillsT.UB92Box44 = CPTCodeT.ID WHERE BillsT.ID = {INT}", billid);
			if(!rsCode->eof) {
				strBox44Override = AdoFldString(rsCode, "Code","");
			}
			rsCode->Close();

			// (a.walling 2007-06-05 13:19) - PLID 26228 - Expand certain categories into component services
			CString strExpandClause;

			// (a.walling 2007-08-17 14:13) - PLID 27092 - Now we use multiple revenue codes
			// (a.walling 2007-10-01 16:22) - PLID 27092 - This was not checking what that service code's current RevCodeUse was, which
			// ended up working on most data, but not all. Modified to ensure that the correct tables are checked depending on RevCodeUse.
			if (m_UB92Info.GroupRev != 0 && m_UB92Info.arUB04GroupRevExpand.GetSize() > 0) {
				CString str = ArrayAsString(m_UB92Info.arUB04GroupRevExpand);
				strExpandClause.Format(" OR (ServiceT.RevCodeUse = 1 AND UB92CategoriesT1.ID IN (%s)) OR (ServiceT.RevCodeUse = 2 AND UB92CategoriesT2.ID IN (%s))", str, str);
			}

			// (j.jones 2007-07-13 09:04) - PLID 26662 - non-revcode charges, or expanded revcode charges,
			// need to continue ordering by their LineID per the bill's ordering (which is usually
			// by amount descending, but not always)
			// (j.jones 2008-04-09 14:28) - PLID 29603 - expanded rev. code charges need to show the charge description,
			// not the rev. code description
			// (j.jones 2008-06-12 14:03) - PLID 23052 - need to include the total amount applied to each line, for use in UpdateUB04Charges
			// (a.vengrofski 2010-02-03 15:14) - PLID <35133> - Added two CASEs to the query to fill in the Modifier 3 & 4
			// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" charges & applies
			// (j.jones 2014-08-01 10:32) - PLID 63104 - charge lab codes can now override the charge description
			g_aryFormQueries[83].sql.Format("SELECT %s AS RevCode, "
				"(CASE WHEN (ServiceT.RevCodeUse = 0 %s) THEN (ItemCode + (CASE WHEN CPTModifier Is Null THEN '' ELSE (' ' + CPTModifier) END) + (CASE WHEN CPTModifier2 Is Null THEN '' ELSE (' ' + CPTModifier2) END)+ (CASE WHEN CPTModifier3 Is Null THEN '' ELSE (' ' + CPTModifier3) END) + (CASE WHEN CPTModifier4 Is Null THEN '' ELSE (' ' + CPTModifier4) END)) ELSE '%s' END) AS ItemCode, "

				"(CASE "
				"WHEN ServiceT.RevCodeUse = 0 %s THEN (CASE WHEN IsNull(ChargeLabTestCodesQ.LabTestCodes, '') <> '' THEN ChargeLabTestCodesQ.LabTestCodes ELSE LineItemT.Description END) "
				"WHEN ServiceT.RevCodeUse = 1 THEN (CASE WHEN IsNull(RevCode1LabTestCodesQ.LabTestCodes, '') <> '' THEN RevCode1LabTestCodesQ.LabTestCodes ELSE UB92CategoriesT1.Name END)"
				"WHEN ServiceT.RevCodeUse = 2 THEN (CASE WHEN IsNull(RevCode2LabTestCodesQ.LabTestCodes, '') <> '' THEN RevCode2LabTestCodesQ.LabTestCodes ELSE UB92CategoriesT2.Name END)"
				"ELSE (CASE WHEN IsNull(ChargeLabTestCodesQ.LabTestCodes, '') <> '' THEN ChargeLabTestCodesQ.LabTestCodes ELSE LineItemT.Description END) "
				"END) AS ItemDesc, "

				"Min(LineItemT.[Date]) AS TDate, Sum(ChargesT.Quantity) AS Quantity, Sum(dbo.GetChargeTotal(ChargesT.ID)) AS Amount, "
				"Sum(Coalesce(AppliesQ.TotalApplied,Convert(money,0))) AS AppliedAmount "
				"FROM BillsT "
				"INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"INNER JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
				"LEFT JOIN UB92CategoriesT UB92CategoriesT1 ON ServiceT.UB92Category = UB92CategoriesT1.ID "
				"LEFT JOIN (SELECT ServiceRevCodesT.*, PersonID AS InsuredPartyID FROM ServiceRevCodesT INNER JOIN InsuredPartyT ON ServiceRevCodesT.InsuranceCompanyID = InsuredPartyT.InsuranceCoID) AS ServiceRevCodesT ON BillsT.InsuredPartyID = ServiceRevCodesT.InsuredPartyID AND ServiceT.ID = ServiceRevCodesT.ServiceID "
				"LEFT JOIN UB92CategoriesT UB92CategoriesT2 ON ServiceRevCodesT.UB92CategoryID = UB92CategoriesT2.ID "
				"LEFT JOIN (SELECT Sum(Round(AppliesT.Amount,2)) AS TotalApplied, DestID "
				"	FROM AppliesT "
				"	INNER JOIN LineItemT ON AppliesT.SourceID = LineItemT.ID "
				"	LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"	LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				"	WHERE Deleted = 0 "
				"	AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"	AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"	GROUP BY DestID) AS AppliesQ ON ChargesT.ID = AppliesQ.DestID "

				"LEFT JOIN (SELECT ChargesT.ID, "
				"	STUFF((SELECT ',' + BillLabTestCodesT.Code "
				"	FROM BillLabTestCodesT "
				"	INNER JOIN ChargeLabTestCodesT ON BillLabTestCodesT.ID = ChargeLabTestCodesT.BillLabTestCodeID "
				"	WHERE ChargeLabTestCodesT.ChargeID = ChargesT.ID "
				"	FOR XML PATH('')), 1, 1, '') AS LabTestCodes "
				"	FROM ChargesT "
				") AS ChargeLabTestCodesQ ON ChargesT.ID = ChargeLabTestCodesQ.ID "

				"LEFT JOIN (SELECT UB92CategoriesT1.ID, BillsT.ID AS BillID, "
				"	STUFF((SELECT ',' + Min(BillLabTestCodesT.Code) "
				"	FROM BillLabTestCodesT "
				"	INNER JOIN ChargeLabTestCodesT ON BillLabTestCodesT.ID = ChargeLabTestCodesT.BillLabTestCodeID "
				"	INNER JOIN ChargesT ON ChargeLabTestCodesT.ChargeID = ChargesT.ID "
				"	INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "				
				"	INNER JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
				"	INNER JOIN UB92CategoriesT UB92CategoriesT1_Inner ON ServiceT.UB92Category = UB92CategoriesT1_Inner.ID "
				"	WHERE LineItemT.Deleted = 0 AND ServiceT.RevCodeUse = 1 "
				"	AND ChargesT.BillID = BillsT.ID AND UB92CategoriesT1_Inner.ID = UB92CategoriesT1.ID "
				"	GROUP BY BillLabTestCodesT.Code "
				"	FOR XML PATH('')), 1, 1, '') AS LabTestCodes "
				"	FROM BillsT "
				"	INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
				"	INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"	INNER JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
				"	INNER JOIN UB92CategoriesT UB92CategoriesT1 ON ServiceT.UB92Category = UB92CategoriesT1.ID "
				"	WHERE LineItemT.Deleted = 0 AND ServiceT.RevCodeUse = 1 "
				"	GROUP BY UB92CategoriesT1.ID, BillsT.ID "
				") AS RevCode1LabTestCodesQ ON UB92CategoriesT1.ID = RevCode1LabTestCodesQ.ID AND BillsT.ID = RevCode1LabTestCodesQ.BillID "

				"LEFT JOIN (SELECT UB92CategoriesT2.ID, BillsT.ID AS BillID, "
				"	STUFF((SELECT ',' + Min(BillLabTestCodesT.Code) "
				"	FROM BillLabTestCodesT "
				"	INNER JOIN ChargeLabTestCodesT ON BillLabTestCodesT.ID = ChargeLabTestCodesT.BillLabTestCodeID "
				"	INNER JOIN ChargesT ON ChargeLabTestCodesT.ChargeID = ChargesT.ID "
				"	INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "				
				"	INNER JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
				"	INNER JOIN (SELECT ServiceRevCodesT.*, PersonID AS InsuredPartyID FROM ServiceRevCodesT INNER JOIN InsuredPartyT ON ServiceRevCodesT.InsuranceCompanyID = InsuredPartyT.InsuranceCoID) AS ServiceRevCodesT ON BillsT.InsuredPartyID = ServiceRevCodesT.InsuredPartyID AND ServiceT.ID = ServiceRevCodesT.ServiceID "
				"	INNER JOIN UB92CategoriesT UB92CategoriesT2_Inner ON ServiceRevCodesT.UB92CategoryID = UB92CategoriesT2_Inner.ID "
				"	WHERE LineItemT.Deleted = 0 AND ServiceT.RevCodeUse = 2 "
				"	AND ChargesT.BillID = BillsT.ID AND UB92CategoriesT2_Inner.ID = UB92CategoriesT2.ID "
				"	GROUP BY BillLabTestCodesT.Code "
				"	FOR XML PATH('')), 1, 1, '') AS LabTestCodes "
				"	FROM BillsT "
				"	INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
				"	INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"	INNER JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
				"	INNER JOIN (SELECT ServiceRevCodesT.*, PersonID AS InsuredPartyID FROM ServiceRevCodesT INNER JOIN InsuredPartyT ON ServiceRevCodesT.InsuranceCompanyID = InsuredPartyT.InsuranceCoID) AS ServiceRevCodesT ON BillsT.InsuredPartyID = ServiceRevCodesT.InsuredPartyID AND ServiceT.ID = ServiceRevCodesT.ServiceID "
				"	INNER JOIN UB92CategoriesT UB92CategoriesT2 ON ServiceRevCodesT.UB92CategoryID = UB92CategoriesT2.ID "
				"	WHERE LineItemT.Deleted = 0 AND ServiceT.RevCodeUse = 2 "
				"	GROUP BY UB92CategoriesT2.ID, BillsT.ID, BillsT.InsuredPartyID "
				") AS RevCode2LabTestCodesQ ON UB92CategoriesT2.ID = RevCode2LabTestCodesQ.ID AND BillsT.ID = RevCode2LabTestCodesQ.BillID "

				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				"WHERE LineItemT.Deleted = 0 AND Batched = 1 "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"AND BillsT.ID = %li "
				"GROUP BY "
				"%s, "
				"(CASE WHEN (ServiceT.RevCodeUse = 0%s) THEN (ItemCode + (CASE WHEN CPTModifier Is Null THEN '' ELSE (' ' + CPTModifier) END) + (CASE WHEN CPTModifier2 Is Null THEN '' ELSE (' ' + CPTModifier2) END)+ (CASE WHEN CPTModifier3 Is Null THEN '' ELSE (' ' + CPTModifier3) END) + (CASE WHEN CPTModifier4 Is Null THEN '' ELSE (' ' + CPTModifier4) END)) ELSE '%s' END), "
				
				"(CASE "
				"WHEN ServiceT.RevCodeUse = 0 %s THEN (CASE WHEN IsNull(ChargeLabTestCodesQ.LabTestCodes, '') <> '' THEN ChargeLabTestCodesQ.LabTestCodes ELSE LineItemT.Description END) "
				"WHEN ServiceT.RevCodeUse = 1 THEN (CASE WHEN IsNull(RevCode1LabTestCodesQ.LabTestCodes, '') <> '' THEN RevCode1LabTestCodesQ.LabTestCodes ELSE UB92CategoriesT1.Name END)"
				"WHEN ServiceT.RevCodeUse = 2 THEN (CASE WHEN IsNull(RevCode2LabTestCodesQ.LabTestCodes, '') <> '' THEN RevCode2LabTestCodesQ.LabTestCodes ELSE UB92CategoriesT2.Name END)"
				"ELSE (CASE WHEN IsNull(ChargeLabTestCodesQ.LabTestCodes, '') <> '' THEN ChargeLabTestCodesQ.LabTestCodes ELSE LineItemT.Description END) "
				"END) "

				"ORDER BY RevCode, Min(ChargesT.LineID)",
				strRevCodeInfo, _Q(strExpandClause), _Q(strBox44Override), _Q(strExpandClause),
				billid,
				strRevCodeInfo, _Q(strExpandClause), _Q(strBox44Override), _Q(strExpandClause));
		}

		// (j.jones 2008-06-12 11:43) - PLID 23052 - need to call this to support multiple pages
		UpdateUB04Charges(firstcharge);

		g_aryFormQueries[84].sql.Replace("FROM FormChargesT",str);
		//TES 3/20/2007 - PLID 23939 - We also need to update form 93 (Box 77).
		g_aryFormQueries[93].sql.Replace("FROM FormChargesT",str);
		g_aryFormQueries[80].sql.Replace("FROM FormChargesT",str);
		g_aryFormQueries[81].sql.Replace("FROM FormChargesT",str);
		str.Format("INNER JOIN ((%s) AS FormChargesT",g_aryFormQueries[2].sql);		
		g_aryFormQueries[87].sql.Replace("INNER JOIN (FormChargesT",str);

	} NxCatchAll("UB04Dlg::BuildUB04ChargesT()");
}

void OnUB04Command(WPARAM wParam, LPARAM lParam, CDWordArray& dwChangedForms, int FormControlsID, CFormDisplayDlg* dlg) 
{
	// (j.jones 2008-06-12 11:08) - PLID 23052 - supported multiple pages of charges

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
	default:
		//do nothing
		return;
	}

	if (dlg) {
		dlg->StopTrackingChanges();
		UpdateUB04Charges(firstcharge);
		dlg->m_ChangeKey1 = firstcharge;
		dlg->m_ChangeKey2 = provider_index;

		//need to redraw to ensure edited colors disappear
		dlg->Invalidate();
	}

	try {
		/////////////////////////////////////////////
		// do not reload all boxes, only reload what could have possibly changed,
		// which is really 83, 90, and 92

		//dwChangedForms.Add(80); // Box 76
		//dwChangedForms.Add(93); // Box 77
		//dwChangedForms.Add(81); // Box 78

		//dwChangedForms.Add(82); // Patient information
		dwChangedForms.Add(83); // Charges
		//dwChangedForms.Add(84); // Bill stuff
		//dwChangedForms.Add(85); // Location Info

		//dwChangedForms.Add(86); // Insured party address
		//dwChangedForms.Add(87); // Provider data
		//dwChangedForms.Add(88); // Insurance information

		dwChangedForms.Add(90); // Insurance apply totals

		//dwChangedForms.Add(89); // Other insurance information
		//dwChangedForms.Add(91); // Other insurance apply totals

		dwChangedForms.Add(92); // Charge totals

		RequeryHistoricalUB04Data();

	}NxCatchAll("OnUB04Command");
}

// (j.jones 2008-06-12 11:45) - PLID 23052 - required for multiple pages
void UpdateUB04Charges(int &firstcharge)
{
	CString str,temp;

	try {

		nPageIncrement = 22;

		nTotalChargeLines = 0;

		// (j.jones 2008-06-12 16:02) - PLID 23052 - I hate to have to do things this way
		// but the problem with the UB here is the revenue codes, we need to be able
		// to find the totals for just those codes, so what we have to do is run form
		// query 83 and sum up the totals, to put into form queries 90 and 92.
		// You could select from form 83 and sum the totals, while filtering on the Charge IDs,
		// but it is heinously slow. Totalling here is much faster.

		//open the recordset once to get the total line count, and the total charges/applies
		_RecordsetPtr rs = CreateRecordset(g_aryFormQueries[83].sql);

		nTotalChargeLines = rs->GetRecordCount();

		while (firstcharge >= nTotalChargeLines) {
			firstcharge -= nPageIncrement;
		}
		if(firstcharge < 0) {
			firstcharge = 0;
		}

		COleCurrency cyTotalCharges = COleCurrency(0,0);
		COleCurrency cyTotalApplies = COleCurrency(0,0);

		long nCurChargeIndex = 0;
		while(!rs->eof) {

			nCurChargeIndex++;

			//add only charges from this page
			if((firstcharge > 0 && nCurChargeIndex > firstcharge
				&& nCurChargeIndex <= firstcharge + nPageIncrement)
				|| (firstcharge <= 0 && nCurChargeIndex <= nPageIncrement)) {

				cyTotalCharges += AdoFldCurrency(rs, "Amount", COleCurrency(0,0));
				cyTotalApplies += AdoFldCurrency(rs, "AppliedAmount", COleCurrency(0,0));
			}

			rs->MoveNext();
		}
		rs->Close();

		{
			CString strPageBatch = BeginSqlBatch();
		
			//update all the form controls in FormID 83
			AddStatementToSqlBatch(strPageBatch, "UPDATE FormControlsT SET FormControlsT.[Value] = %d WHERE FormControlsT.ID IN (6374, 6375, 6376, 6377, 6378, 6380, 6382, 6383)", firstcharge);
			AddStatementToSqlBatch(strPageBatch, "UPDATE FormControlsT SET FormControlsT.[Value] = %d WHERE FormControlsT.ID IN (6384, 6407, 6429, 6451, 6473, 6495, 6539, 6583)", firstcharge+1);
			AddStatementToSqlBatch(strPageBatch, "UPDATE FormControlsT SET FormControlsT.[Value] = %d WHERE FormControlsT.ID IN (6385, 6408, 6430, 6452, 6474, 6496, 6540, 6584)", firstcharge+2);
			AddStatementToSqlBatch(strPageBatch, "UPDATE FormControlsT SET FormControlsT.[Value] = %d WHERE FormControlsT.ID IN (6386, 6409, 6431, 6453, 6475, 6497, 6541, 6585)", firstcharge+3);
			AddStatementToSqlBatch(strPageBatch, "UPDATE FormControlsT SET FormControlsT.[Value] = %d WHERE FormControlsT.ID IN (6387, 6410, 6432, 6454, 6476, 6498, 6542, 6586)", firstcharge+4);
			AddStatementToSqlBatch(strPageBatch, "UPDATE FormControlsT SET FormControlsT.[Value] = %d WHERE FormControlsT.ID IN (6388, 6411, 6433, 6455, 6477, 6499, 6543, 6587)", firstcharge+5);
			AddStatementToSqlBatch(strPageBatch, "UPDATE FormControlsT SET FormControlsT.[Value] = %d WHERE FormControlsT.ID IN (6389, 6412, 6434, 6456, 6478, 6500, 6544, 6588)", firstcharge+6);
			AddStatementToSqlBatch(strPageBatch, "UPDATE FormControlsT SET FormControlsT.[Value] = %d WHERE FormControlsT.ID IN (6390, 6413, 6435, 6457, 6479, 6501, 6545, 6589)", firstcharge+7);
			AddStatementToSqlBatch(strPageBatch, "UPDATE FormControlsT SET FormControlsT.[Value] = %d WHERE FormControlsT.ID IN (6391, 6414, 6436, 6458, 6480, 6502, 6546, 6590)", firstcharge+8);
			AddStatementToSqlBatch(strPageBatch, "UPDATE FormControlsT SET FormControlsT.[Value] = %d WHERE FormControlsT.ID IN (6392, 6415, 6437, 6459, 6481, 6503, 6547, 6591)", firstcharge+9);
			AddStatementToSqlBatch(strPageBatch, "UPDATE FormControlsT SET FormControlsT.[Value] = %d WHERE FormControlsT.ID IN (6393, 6416, 6438, 6460, 6482, 6504, 6548, 6592)", firstcharge+10);
			AddStatementToSqlBatch(strPageBatch, "UPDATE FormControlsT SET FormControlsT.[Value] = %d WHERE FormControlsT.ID IN (6394, 6417, 6439, 6461, 6483, 6505, 6549, 6593)", firstcharge+11);
			AddStatementToSqlBatch(strPageBatch, "UPDATE FormControlsT SET FormControlsT.[Value] = %d WHERE FormControlsT.ID IN (6395, 6418, 6440, 6462, 6484, 6506, 6550, 6594)", firstcharge+12);
			AddStatementToSqlBatch(strPageBatch, "UPDATE FormControlsT SET FormControlsT.[Value] = %d WHERE FormControlsT.ID IN (6396, 6419, 6441, 6463, 6485, 6507, 6551, 6595)", firstcharge+13);
			AddStatementToSqlBatch(strPageBatch, "UPDATE FormControlsT SET FormControlsT.[Value] = %d WHERE FormControlsT.ID IN (6397, 6420, 6442, 6464, 6486, 6508, 6552, 6596)", firstcharge+14);
			AddStatementToSqlBatch(strPageBatch, "UPDATE FormControlsT SET FormControlsT.[Value] = %d WHERE FormControlsT.ID IN (6398, 6421, 6443, 6465, 6487, 6509, 6553, 6597)", firstcharge+15);
			AddStatementToSqlBatch(strPageBatch, "UPDATE FormControlsT SET FormControlsT.[Value] = %d WHERE FormControlsT.ID IN (6399, 6422, 6444, 6466, 6488, 6510, 6554, 6598)", firstcharge+16);
			AddStatementToSqlBatch(strPageBatch, "UPDATE FormControlsT SET FormControlsT.[Value] = %d WHERE FormControlsT.ID IN (6400, 6423, 6445, 6467, 6489, 6511, 6555, 6599)", firstcharge+17);
			AddStatementToSqlBatch(strPageBatch, "UPDATE FormControlsT SET FormControlsT.[Value] = %d WHERE FormControlsT.ID IN (6401, 6424, 6446, 6468, 6490, 6512, 6556, 6600)", firstcharge+18);
			AddStatementToSqlBatch(strPageBatch, "UPDATE FormControlsT SET FormControlsT.[Value] = %d WHERE FormControlsT.ID IN (6402, 6425, 6447, 6469, 6491, 6513, 6557, 6601)", firstcharge+19);
			AddStatementToSqlBatch(strPageBatch, "UPDATE FormControlsT SET FormControlsT.[Value] = %d WHERE FormControlsT.ID IN (6403, 6426, 6448, 6470, 6492, 6514, 6558, 6602)", firstcharge+20);
			AddStatementToSqlBatch(strPageBatch, "UPDATE FormControlsT SET FormControlsT.[Value] = %d WHERE FormControlsT.ID IN (6404, 6427, 6449, 6471, 6493, 6515, 6559, 6603)", firstcharge+21);

			//uses ExecuteSqlStd so we can override the nMaxRecordsAffected number
			long nRecordsAffected = 0;
			CString strSql;
			strSql.Format("BEGIN TRAN \r\n"
				//don't bother with all the updates if they are already correct
				"IF NOT EXISTS (SELECT Value FROM FormControlsT WHERE ID = 6374 AND Value = %li) \r\n"
				"BEGIN \r\n"
				"%s \r\n"
				"END \r\n"
				"COMMIT TRAN \r\n", firstcharge, strPageBatch);

			// (a.walling 2012-02-09 17:13) - PLID 48115 - Use NxAdo::PushMaxRecordsWarningLimit and NxAdo::PushPerformanceWarningLimit
			NxAdo::PushMaxRecordsWarningLimit pmr(200);
			ExecuteSqlStd(strSql, &nRecordsAffected, adCmdText);
		}

		BOOL bShowApplies = TRUE;
		rs = CreateParamRecordset("SELECT ShowApplies FROM UB92SetupT WHERE ID = {INT}", fee_group_id);
		if(!rs->eof) {
			if(AdoFldLong(rs, "ShowApplies") == 0) {
				bShowApplies = FALSE;
			}
		}
		rs->Close();

		//****IMPORTANT************************************************************************//
		//If you add any more form queries here that change when UpdateUB04Charges is called,
		//you must add the form ID to dwChangedForms in OnUB04Command, as it is only
		//going to refresh the forms that changed after the user presses the up/down buttons.
		//*************************************************************************************//

		long nPageFrom = (firstcharge / nPageIncrement) + 1;
		long nPageTo = (nTotalChargeLines / nPageIncrement) + 1;

		g_aryFormQueries[90].sql.Format("SELECT Convert(money, '%s') AS TotalPays", FormatCurrencyForSql(cyTotalApplies));

		//TES 3/21/2007 - PLID 23939 -  Form 92 used to basically just load the Box 55 value, and then copy that onto the Totals
		// line.  Now that the totals line is filled in, the foolishness of that is clear, so now it loads both values 
		// independently, one as Box55 and one as SumOfCharges.
		// (j.jones 2008-06-12 16:49) - PLID 23052 - I added PageFrom and PageTo to form 92
		if(bShowApplies) {
			//make Box 55 show the balance
			g_aryFormQueries[92].sql.Format("SELECT %li AS PageFrom, %li AS PageTo, Convert(money, '%s') AS SumOfCharges, "
				"Convert(money, '%s') - Convert(money, '%s') AS Box55", nPageFrom, nPageTo, FormatCurrencyForSql(cyTotalCharges), FormatCurrencyForSql(cyTotalCharges), FormatCurrencyForSql(cyTotalApplies));			
		}
		else {
			//make Box 55 show the total charges
			g_aryFormQueries[92].sql.Format("SELECT %li AS PageFrom, %li AS PageTo, Convert(money, '%s') AS SumOfCharges, "
				"Convert(money, '%s') AS Box55", nPageFrom, nPageTo, FormatCurrencyForSql(cyTotalCharges), FormatCurrencyForSql(cyTotalCharges));
		}
		

	}NxCatchAll("Error in updating charges");
}

void OnUB04KeyDown(CDialog* pFormDisplayDlg, MSG* pMsg)
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

void CUB04Dlg::OnClickAlign() 
{
	CPrintAlignDlg dlg(this);
	dlg.m_FormID = 1;
	dlg.DoModal();
}

void CUB04Dlg::OnClickCapitalizeOnPrint() 
{
	if(m_CapOnPrint) {
		SetRemotePropertyInt("CapitalizeHCFA",0);
	}
	else {
		SetRemotePropertyInt("CapitalizeHCFA",1);
	}
	m_CapOnPrint = GetRemotePropertyInt("CapitalizeHCFA",0);
}

void CUB04Dlg::OnClickRestore() 
{
	CString strSQL;

	if(!CheckCurrentUserPermissions(bioClaimForms,sptWrite))
		return;

	if (IDNO == MessageBox("All changes you have made to the UB04 will be unrecoverable. Are you sure you wish to do this?", NULL, MB_YESNO))
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

	//Refresh the UB04

	m_pframe->Refresh (80); // Box 76
	m_pframe->Refresh (93); // Box 77
	m_pframe->Refresh (81); // Box 78

	m_pframe->Refresh (82); // Patient information
	m_pframe->Refresh (83); // Charges
	m_pframe->Refresh (84); // Bill stuff
	m_pframe->Refresh (85); // Location Info

	m_pframe->Refresh (86); // Insured party address
	m_pframe->Refresh (87); // Provider data
	m_pframe->Refresh (88); // Insurance information

	m_pframe->Refresh (90); // Insurance apply totals

	m_pframe->Refresh (89); // Other insurance information
	m_pframe->Refresh (91); // Other insurance apply totals

	m_pframe->Refresh (92); // Charge totals
	//TES 3/21/2007 - PLID 23939 - There was special handling on the UB92 for form 92, because showing the Totals was optional.
	// It's not optional on the UB04, so we just let the normal loading for this form load that field.

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

void CUB04Dlg::Save(BOOL boSaveAll, CDialog* pdlgWait, int& iPage)
{
	CString str;
	//int provider;

	/*
	for (provider_index=0; provider_index < adwProviderList.GetSize(); provider_index++) {

		provider = adwProviderList.GetAt(provider_index);
		*/

		for (firstcharge=0; firstcharge < nTotalChargeLines; firstcharge+=nPageIncrement) {

			if (pdlgWait) {
				str.Format("Saving UB04 page %d...", ++iPage);
				pdlgWait->GetDlgItem(IDC_LABEL)->SetWindowText(str);
				pdlgWait->RedrawWindow();
			}

			SaveHistoricalData(boSaveAll);
		}
	//}
}

void CUB04Dlg::SaveHistoricalData(BOOL boSaveAll)
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

	// (j.jones 2008-06-12 11:45) - PLID 23052 - required for multiple pages
	try {
		UpdateUB04Charges(firstcharge);
	}
	NxCatchAll("SaveHistoricalData");

	m_pframe->Refresh (80); // Box 76
	m_pframe->Refresh (93); // Box 77
	m_pframe->Refresh (81); // Box 78

	m_pframe->Refresh (82); // Patient information
	m_pframe->Refresh (83); // Charges
	m_pframe->Refresh (84); // Bill stuff
	m_pframe->Refresh (85); // Location Info

	m_pframe->Refresh (86); // Insured party address
	m_pframe->Refresh (87); // Provider data
	m_pframe->Refresh (88); // Insurance information

	m_pframe->Refresh (90); // Insurance apply totals

	m_pframe->Refresh (89); // Other insurance information
	m_pframe->Refresh (91); // Other insurance apply totals
	m_pframe->Refresh (92); // Charge totals
	

	m_pframe->ReapplyChanges(firstcharge, provider_index);

	m_pframe->Save (firstcharge, provider_index, iDocumentID, boSaveAll);

	// We save form 6 (zero based in the array) because where the radio button is
	// determines what text goes in a particular control. To save us time and pain,
	// we always save the data in form 6.
	//((FormLayer*)m_pframe->m_LayerArray[5])->Save(iDocumentID, 0, &m_pframe->m_ControlArray);
#endif
}

long CUB04Dlg::DoScrollTo(long nNewTopPos)
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

void CUB04Dlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
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

BOOL PreUB04Print(CDWordArray& dwChangedForms, CFormDisplayDlg* dlg)
{
	// Only happens on first call to this function. The first page
	// information is reloaded.
	if(nPrintedCharges == 0) {

		while(firstcharge > 0) {
			OnUB04Command(IDC_BILL_UP, 0, dwChangedForms, 0, dlg);
		}

		oldfirstcharge = firstcharge;
		oldprovider_index = provider_index;

		provider_index = 0;
		firstcharge = 0;

		//increment the page, print the current page now
		nPrintedCharges += nPageIncrement;
		return TRUE;
	}

	// If still more charges to print, print them and return
	if (nPrintedCharges < nTotalChargeLines) {		
		OnUB04Command(IDC_BILL_DOWN, 0, dwChangedForms, 0, dlg);
		nPrintedCharges += nPageIncrement;
		return TRUE;
	}
	else {
		nPrintedCharges = 0;
		return FALSE;
	}
}

void CUB04Dlg::GetUB04GroupID()
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
	NxCatchAll("UB04Dlg::GetUB04GroupID()");
}

void CUB04Dlg::BuildBoxes76_78()
{
	try {

		//Box 76/77/78 Setup:
		//1 - Bill Provider
		//2 - General 1 Provider
		//3 - Referring Physician

		CString strBox76ID = GetBox76_78ID(m_UB92Info.Box82Setup,m_UB92Info.Box82Num);
		CString strBox77ID = GetBox76_78ID(m_UB92Info.UB04Box77Setup,m_UB92Info.UB04Box77Num);
		CString strBox78ID = GetBox76_78ID(m_UB92Info.Box83Setup,m_UB92Info.Box83Num);
		
		CString strBox76FromClause = "";
		CString strBox77FromClause = "";
		CString strBox78FromClause = "";
		CString strBox76SelectStatement = "";
		CString strBox77SelectStatement = "";
		CString strBox78SelectStatement = "";
		CString strIDType = "";

		//Format the Box 76 Setup, query 80
		
		// (j.jones 2007-03-19 16:04) - temporarily uses the UB92 Box 82 setup
		strBox76SelectStatement.Format("SELECT NPI AS Box76NPI, CASE WHEN Coalesce(%s,'') <> '' THEN '%s' ELSE '' END AS Box76Qual, %s AS Box76ID, "
			"Last AS Box76Last, First AS Box76First", strBox76ID, _Q(m_UB92Info.UB04Box76Qual), strBox76ID);
		
		switch(m_UB92Info.Box82Setup) {
			case 2:
				//General 1 Provider

				// (j.jones 2006-12-04 15:25) - PLID 23733 - supported the ClaimProviderID override
				// (j.jones 2010-11-09 16:00) - PLID 41387 - supported ChargesT.ClaimProviderID
				strBox76FromClause.Format(" FROM FormChargesT INNER JOIN PatientsT ON FormChargesT.PatientID = PatientsT.PersonID "
					"INNER JOIN (SELECT PersonID, ClaimProviderID FROM ProvidersT) ActualProvidersT ON PatientsT.MainPhysician = ActualProvidersT.PersonID "
					"INNER JOIN ProvidersT ON (CASE WHEN FormChargesT.ClaimProviderID Is Not Null THEN FormChargesT.ClaimProviderID ELSE ActualProvidersT.ClaimProviderID END) = ProvidersT.PersonID "
					"INNER JOIN PersonT ON ProvidersT.PersonID = PersonT.ID "
					"WHERE FormChargesT.BillID = %li",m_ID);
				break;
			case 3:
				//Referring Physician
				strBox76FromClause.Format(" FROM BillsT INNER JOIN ReferringPhysT ON BillsT.RefPhyID = ReferringPhysT.PersonID INNER JOIN PersonT ON ReferringPhysT.PersonID = PersonT.ID WHERE BillsT.ID = %li",m_ID);
				break;
			case 1:
			default:
				//Bill Provider

				// (j.jones 2006-12-04 15:25) - PLID 23733 - supported the ClaimProviderID override
				// (j.jones 2010-11-09 16:00) - PLID 41387 - supported ChargesT.ClaimProviderID
				strBox76FromClause = " FROM FormChargesT "
					"INNER JOIN (SELECT PersonID, ClaimProviderID FROM ProvidersT) ActualProvidersT ON FormChargesT.DoctorsProviders = ActualProvidersT.PersonID "
					"INNER JOIN ProvidersT ON (CASE WHEN FormChargesT.ClaimProviderID Is Not Null THEN FormChargesT.ClaimProviderID ELSE ActualProvidersT.ClaimProviderID END) = ProvidersT.PersonID "
					"INNER JOIN PersonT ON ProvidersT.PersonID = PersonT.ID";
				break;
		}

		//Format the Box 77 Setup, query 93

		//TES 3/26/2007 - PLID 25335 - Added a "{Do Not Fill} option for Box 77
		if(m_UB92Info.UB04Box77Setup > 0) {
			strBox77SelectStatement.Format("SELECT NPI AS Box77NPI, CASE WHEN Coalesce(%s,'') <> '' THEN '%s' ELSE '' END AS Box77Qual, %s AS Box77ID, "
				"Last AS Box77Last, First AS Box77First", strBox77ID, _Q(m_UB92Info.UB04Box77Qual), strBox77ID);
		}
		else {
			//Do not fill
			strBox77SelectStatement.Format("SELECT '' AS Box77NPI, '' AS Box77Qual, '' AS Box77ID, "
				"'' AS Box77Last, '' AS Box77First");
		}

		switch(m_UB92Info.UB04Box77Setup) {
			case 1:			
				//Bill Provider

				// (j.jones 2006-12-04 15:25) - PLID 23733 - supported the ClaimProviderID override
				// (j.jones 2010-11-09 16:00) - PLID 41387 - supported ChargesT.ClaimProviderID
				strBox77FromClause = " FROM FormChargesT "
					"INNER JOIN (SELECT PersonID, ClaimProviderID FROM ProvidersT) ActualProvidersT ON FormChargesT.DoctorsProviders = ActualProvidersT.PersonID "
					"INNER JOIN ProvidersT ON (CASE WHEN FormChargesT.ClaimProviderID Is Not Null THEN FormChargesT.ClaimProviderID ELSE ActualProvidersT.ClaimProviderID END) = ProvidersT.PersonID "
					"INNER JOIN PersonT ON ProvidersT.PersonID = PersonT.ID";
				break;
			case 2:
				//General 1 Provider

				// (j.jones 2006-12-04 15:25) - PLID 23733 - supported the ClaimProviderID override
				// (j.jones 2010-11-09 16:00) - PLID 41387 - supported ChargesT.ClaimProviderID
				strBox77FromClause.Format(" FROM FormChargesT INNER JOIN PatientsT ON FormChargesT.PatientID = PatientsT.PersonID "
					"INNER JOIN (SELECT PersonID, ClaimProviderID FROM ProvidersT) ActualProvidersT ON PatientsT.MainPhysician = ActualProvidersT.PersonID "
					"INNER JOIN ProvidersT ON (CASE WHEN FormChargesT.ClaimProviderID Is Not Null THEN FormChargesT.ClaimProviderID ELSE ActualProvidersT.ClaimProviderID END) = ProvidersT.PersonID "
					"INNER JOIN PersonT ON ProvidersT.PersonID = PersonT.ID WHERE FormChargesT.BillID = %li",m_ID);
				break;
			case 3:
			default:
				//Referring Physician
				strBox77FromClause.Format(" FROM BillsT INNER JOIN ReferringPhysT ON BillsT.RefPhyID = ReferringPhysT.PersonID INNER JOIN PersonT ON ReferringPhysT.PersonID = PersonT.ID WHERE BillsT.ID = %li",m_ID);
				break;			
		}

		//Format the Box 78 Setup, query 81
		
		// (j.jones 2007-03-19 16:04) - temporarily uses the UB92 Box 83 setup
		// (j.jones 2007-04-02 09:00) - PLID 25433 - supported the 'Do Not Fill' option
		if(m_UB92Info.Box83Setup > 0) {
			strBox78SelectStatement.Format("SELECT NPI AS Box78NPI, CASE WHEN Coalesce(%s,'') <> '' THEN '%s' ELSE '' END AS Box78Qual, %s AS Box78ID, "
				"Last AS Box78Last, First AS Box78First", strBox78ID, _Q(m_UB92Info.UB04Box78Qual), strBox78ID);
		}
		else {
			//Do not fill
			strBox78SelectStatement.Format("SELECT '' AS Box78NPI, '' AS Box78Qual, '' AS Box78ID, "
				"'' AS Box78Last, '' AS Box78First");
		}
		
		switch(m_UB92Info.Box83Setup) {
			case 2:
				//General 1 Provider

				// (j.jones 2006-12-04 15:25) - PLID 23733 - supported the ClaimProviderID override
				// (j.jones 2010-11-09 16:00) - PLID 41387 - supported ChargesT.ClaimProviderID
				strBox78FromClause.Format(" FROM FormChargesT INNER JOIN PatientsT ON FormChargesT.PatientID = PatientsT.PersonID "
					"INNER JOIN (SELECT PersonID, ClaimProviderID FROM ProvidersT) ActualProvidersT ON PatientsT.MainPhysician = ActualProvidersT.PersonID "
					"INNER JOIN ProvidersT ON (CASE WHEN FormChargesT.ClaimProviderID Is Not Null THEN FormChargesT.ClaimProviderID ELSE ActualProvidersT.ClaimProviderID END) = ProvidersT.PersonID "
					"INNER JOIN PersonT ON ProvidersT.PersonID = PersonT.ID WHERE FormChargesT.BillID = %li",m_ID);
				break;
			case 3:
				//Referring Physician
				strBox78FromClause.Format(" FROM BillsT INNER JOIN ReferringPhysT ON BillsT.RefPhyID = ReferringPhysT.PersonID INNER JOIN PersonT ON ReferringPhysT.PersonID = PersonT.ID WHERE BillsT.ID = %li",m_ID);
				break;
			case 1:
			default:
				//Bill Provider

				// (j.jones 2006-12-04 15:25) - PLID 23733 - supported the ClaimProviderID override
				// (j.jones 2010-11-09 16:00) - PLID 41387 - supported ChargesT.ClaimProviderID
				strBox78FromClause = " FROM FormChargesT "
					"INNER JOIN (SELECT PersonID, ClaimProviderID FROM ProvidersT) ActualProvidersT ON FormChargesT.DoctorsProviders = ActualProvidersT.PersonID "
					"INNER JOIN ProvidersT ON (CASE WHEN FormChargesT.ClaimProviderID Is Not Null THEN FormChargesT.ClaimProviderID ELSE ActualProvidersT.ClaimProviderID END) = ProvidersT.PersonID "
					"INNER JOIN PersonT ON ProvidersT.PersonID = PersonT.ID";
				break;
		}
		
	
		//Box 76
		g_aryFormQueries[80].sql.Format("%s %s",strBox76SelectStatement, strBox76FromClause);

		//Box 77
		g_aryFormQueries[93].sql.Format("%s %s",strBox77SelectStatement, strBox77FromClause);

		//Box 78
		g_aryFormQueries[81].sql.Format("%s %s",strBox78SelectStatement, strBox78FromClause);

	}NxCatchAll("Error building boxes 76 / 78");
}

CString CUB04Dlg::GetBox76_78ID(int Box76_78Setup, int Box76_78Number)
{
	try {

		CString strNum = "UPIN";

		switch(Box76_78Number) {
			case 2:		//SSN
				strNum = "SocialSecurity";
				break;
			case 3:		//EIN
				if(Box76_78Setup == 3)
					//ref. phy.
					strNum = "FedEmployerID";
				else
					//provider
					strNum = "[Fed Employer ID]";
				break;
			case 4:		//Medicare
				if(Box76_78Setup == 3)
					//ref. phy.
					strNum = "MedicareNumber";
				else
					//provider
					strNum = "[Medicare Number]";
				break;
			case 5:		//Medicaid
				if(Box76_78Setup == 3)
					//ref. phy.
					strNum = "MedicaidNumber";
				else
					//provider
					strNum = "[Medicaid Number]";
				break;
			case 6:		//BCBS
				if(Box76_78Setup == 3)
					//ref. phy.
					strNum = "BlueShieldID";
				else
					//provider
					strNum = "[BCBS Number]";
				break;
			case 7:		//DEA
				if(Box76_78Setup == 3)
					//ref. phy.
					strNum = "DEANumber";
				else
					//provider
					strNum = "[DEA Number]";
				break;
			case 8:		//Workers Comp.
				if(Box76_78Setup == 3)
					//ref. phy.
					strNum = "WorkersCompNumber";
				else
					//provider
					strNum = "[Workers Comp Number]";
				break;
			case 9:		//Other ID
				if(Box76_78Setup == 3)
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

					if(Box76_78Setup == 1) {
						//Bill Provider

						// (j.jones 2006-12-04 15:25) - PLID 23733 - supported the ClaimProviderID override
						// (j.jones 2010-11-09 15:44) - PLID 41387 - supported ChargesT.ClaimProviderID
						strSQL.Format("SELECT InsuranceBox51.Box51Info AS Box51 FROM (ChargesT INNER JOIN (((InsuranceCoT INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID) INNER JOIN InsuranceBox51 ON InsuranceCoT.PersonID = InsuranceBox51.InsCoID) INNER JOIN BillsT ON (InsuredPartyT.PersonID = BillsT.InsuredPartyID) AND (InsuredPartyT.PatientID = BillsT.PatientID)) ON ChargesT.BillID = BillsT.ID) "
							"INNER JOIN (SELECT PersonID, ClaimProviderID FROM ProvidersT) ActualProvidersT ON ChargesT.DoctorsProviders = ActualProvidersT.PersonID "
							"INNER JOIN ProvidersT ON (CASE WHEN ChargesT.ClaimProviderID Is Not Null THEN ChargesT.ClaimProviderID ELSE ActualProvidersT.ClaimProviderID END) = ProvidersT.PersonID "
							"	AND (InsuranceBox51.ProviderID = ProvidersT.PersonID) "
								"GROUP BY InsuranceBox51.Box51Info, BillsT.ID HAVING (((BillsT.ID)=%li))", m_ID);
					}
					else if(Box76_78Setup == 2) {
						//Gen 1 Provider

						// (j.jones 2006-12-04 15:25) - PLID 23733 - supported the ClaimProviderID override
						// (j.jones 2010-11-09 15:44) - PLID 41387 - supported ChargesT.ClaimProviderID
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
					else if(Box76_78Setup == 3) {
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

				if(Box76_78Setup == 1) {
					//Bill Provider

					// (j.jones 2006-12-04 15:25) - PLID 23733 - supported the ClaimProviderID override
					// (j.jones 2010-11-09 15:44) - PLID 41387 - supported ChargesT.ClaimProviderID
					strSQL.Format("SELECT InsuranceGroups.GRP FROM (ChargesT INNER JOIN (((InsuranceCoT INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID) INNER JOIN InsuranceGroups ON InsuranceCoT.PersonID = InsuranceGroups.InsCoID) INNER JOIN BillsT ON (InsuredPartyT.PersonID = BillsT.InsuredPartyID) AND (InsuredPartyT.PatientID = BillsT.PatientID)) ON ChargesT.BillID = BillsT.ID) "
							"INNER JOIN ProvidersT ActualProvidersT ON ChargesT.DoctorsProviders = ActualProvidersT.PersonID "
							"INNER JOIN ProvidersT ON (CASE WHEN ChargesT.ClaimProviderID Is Not Null THEN ChargesT.ClaimProviderID ELSE ActualProvidersT.ClaimProviderID END) = ProvidersT.PersonID "
							"AND (InsuranceGroups.ProviderID = ProvidersT.PersonID) GROUP BY InsuranceGroups.GRP, BillsT.ID HAVING (((BillsT.ID)=%li))", m_ID);
				}
				else if(Box76_78Setup == 2) {
					//Gen 1 Provider

					// (j.jones 2006-12-04 15:25) - PLID 23733 - supported the ClaimProviderID override
					// (j.jones 2010-11-09 15:44) - PLID 41387 - supported ChargesT.ClaimProviderID
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
				else if(Box76_78Setup == 3) {
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

	}NxCatchAll("Error getting Box 76/77/78 IDs.");

	return "UPIN";
}

void CUB04Dlg::OnRadioPaper() {

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

void CUB04Dlg::OnRadioElectronic() {

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

void CUB04Dlg::OnRadioNoBatch() {

	if(!CheckCurrentUserPermissions(bioClaimForms,sptWrite)) {
		FindBatch();
		return;
	}

	UpdateBatch();
}

void CUB04Dlg::FindBatch()
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

void CUB04Dlg::UpdateBatch()
{
	CString sql, str;
	short nBatch = 0;

	if (((CButton*)GetDlgItem(IDC_RADIO_PAPER))->GetCheck())
		nBatch = 1; // paper batch
	else if (((CButton*)GetDlgItem(IDC_RADIO_ELECTRONIC))->GetCheck())
		nBatch = 2; // electronic batch

	BatchBill(m_ID, nBatch);
}

void CUB04Dlg::SetUB04DateFormats()
{
	try {
		_RecordsetPtr rs(__uuidof(Recordset));
		CString str;
		int i = 0, j = 0;
		int boxids[][23] = {
			{ 6284, 6285, -1 },	//Box 6
			{ 6292, -1 },		//Box 14
			{ 6295, -1 },		//Box 17
			{ 6361, 6359, 6357, 6355, 6353, 6340, 6788, 6790,
			  6338, 6364, 6366, 6368, 6370, 6371, 6789, 6791,  -1 },	//Boxes 31 - 36
			{ 6377, 6451, 6452, 6453, 6454, 6455, 6456, 6457, 6458, 
			  6459, 6460, 6461, 6462, 6463, 6464, 6465, 6466, 6467, 
			  6468, 6469, 6470, 6471, -1 },	//Box 45
			{ 6684, 6688, 6689, 6698, 6700, 6702, -1 },		//Box 74 a - e
			{ 6037, -1 }		//Creation Date
		};
		CString strBoxNames[] = {
			"Box6", "Box14", "Box17", "Box32_36", "Box45", "Box80_81", "Box86"
		};

		// Assign the date formats to be 2 or 4 digit
		rs = CreateRecordset("SELECT Box6, Box14, Box17, Box32_36, Box45, Box80_81, Box86 FROM UB92SetupT WHERE ID = %li", fee_group_id);

		//Don't forget the DateFmt setting - 0 is YearFirst, 1 is MonthFirst

		// If no group in UB04 ID setup, default to 4 digits for all dates
		if (rs->eof) {
			for (i=0; i < 7; i++) {
				j=0;
				while (boxids[i][j] != -1) {
					if(m_UB92Info.DateFmt == 0)	//year-first
						str.Format("UPDATE FormControlsT SET Format = 20480 WHERE ID = %li", boxids[i][j]);
					else	//month-first
						str.Format("UPDATE FormControlsT SET Format = 4096 WHERE ID = %li", boxids[i][j]);
					ExecuteSql(str);
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
							str.Format("UPDATE FormControlsT SET Format = 20481 WHERE ID = %li", boxids[i][j]);
						else	//month-first
							str.Format("UPDATE FormControlsT SET Format = 4097 WHERE ID = %li", boxids[i][j]);
						ExecuteSql(str);
						j++;
					}
					break;
				case 4:
					j=0;
					while (boxids[i][j] != -1) {
						if(m_UB92Info.DateFmt == 0)	//year-first
							str.Format("UPDATE FormControlsT SET Format = 20480 WHERE ID = %li", boxids[i][j]);
						else	//month-first
							str.Format("UPDATE FormControlsT SET Format = 4096 WHERE ID = %li", boxids[i][j]);
						ExecuteSql(str);
						j++;
					}
					break;
				}
			}
		}
		rs->Close();
	}
	NxCatchAll("SetUB04DateFormat");
}

// (j.jones 2007-06-22 13:23) - PLID 25665 - handle when a field was edited, so we update the info text box
BOOL CUB04Dlg::PreTranslateMessage(MSG *pMsg) {

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
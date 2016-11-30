// StatementSetup.cpp : implementation file
//

#include "stdafx.h"
#include "BillingRc.h"
#include "StatementSetup.h"
#include "pracprops.h"
#include "NxStandard.h"
#include "GlobalDatautils.h"
#include "ReportInfo.h"
#include "StatementARNotesConfigDlg.h"
#include "dontshowdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CStatementSetup dialog


CStatementSetup::CStatementSetup(CWnd* pParent /*=NULL*/)
	: CNxDialog(CStatementSetup::IDD, pParent)
{
	//{{AFX_DATA_INIT(CStatementSetup)
	//}}AFX_DATA_INIT
}


void CStatementSetup::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CStatementSetup)
	DDX_Control(pDX, IDC_PRINT_MULTI_RESP, m_printMultiCheck);
	DDX_Control(pDX, IDC_REMIT, m_Remit);
	DDX_Control(pDX, IDC_SHOW_FOOTER, m_hideFooterCheck);
	DDX_Control(pDX, IDC_NO_USE_DIAGCODES, m_noDiagRadio);
	DDX_Control(pDX, IDC_USE_DIAGCODES, m_useDiagRadio);
	DDX_Control(pDX, IDC_USE_PATFORWARD, m_forwardRadio);
	DDX_Control(pDX, IDC_USE_PATCOMMA, m_commaRadio);
	DDX_Control(pDX, IDC_USE_PRACNAME, m_practiceRadio);
	DDX_Control(pDX, IDC_USE_PROVNAME, m_providerRadio);
	DDX_Control(pDX, IDC_USE_GUARANTOR, m_guarantorCheck);
	DDX_Control(pDX, 1081, m_btn1081);
	DDX_Control(pDX, IDC_DEFAULTFORMAT, m_btnDefault);
	DDX_Control(pDX, IDC_AVERY2, m_btnAvery2);
	DDX_Control(pDX, IDC_ESTATEMENTFORMAT, m_btnEStatement);
	DDX_Control(pDX, IDC_ACCEPT_VISA, m_btnAcceptVisa);
	DDX_Control(pDX, IDC_ACCEPT_MSTRCARD, m_btnAcceptMastercard);
	DDX_Control(pDX, IDC_ACCEPT_DISCOVER, m_btnAcceptDiscover);
	DDX_Control(pDX, IDC_ACCEPT_AMEX, m_btnAcceptAmex);
	DDX_Control(pDX, IDC_COMBINE_BILL_BALANCES, m_btnCombineBillBalances);
	DDX_Control(pDX, IDC_HIDEPREPAYS, m_btnHidePrepays);
	DDX_Control(pDX, IDC_HIDE_CPT_CODES, m_btnHideCPTCodes);
	DDX_Control(pDX, IDC_USE_PRACADDRESS, m_btnUsePracAddress);
	DDX_Control(pDX, IDC_USE_PROVADDRESS, m_btnUseProvAddress);
	DDX_Control(pDX, IDC_USE_70_VERSION, m_btnUse70Version);
	DDX_Control(pDX, IDC_SHOW_LAST_PAY, m_btnShowLastPay);
	DDX_Control(pDX, IDC_EDIT_AGE, m_nxeditEditAge);
	DDX_Control(pDX, IDC_PCC_TITLE, m_nxeditPccTitle);
	DDX_Control(pDX, IDC_PAYMENTDESC, m_nxeditPaymentdesc);
	DDX_Control(pDX, IDC_ADJUSTMENTDESC, m_nxeditAdjustmentdesc);
	DDX_Control(pDX, IDC_REFUNDDESC, m_nxeditRefunddesc);
	DDX_Control(pDX, IDC_DAYS_OLD, m_nxeditDaysOld);
	DDX_Control(pDX, IDC_EXTRA_TEXT, m_nxeditExtraText);
	DDX_Control(pDX, ID_OK_BTN, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_STMT_AR_NOTES, m_btnStmtARNotes);
	DDX_Control(pDX, IDC_STMT_DETAILED, m_btnDetailed);
	DDX_Control(pDX, IDC_STMT_SUMMARY, m_btnSummary);
	DDX_Control(pDX, IDC_SHOWESTATEMENTPATIENTSELECT, m_btnShowPatSelectWnd);
	DDX_Control(pDX, IDC_INCLUDEPAIDCHARGES_ON_BILLS, m_checkIncludePaidChargesOnBills);
	DDX_Control(pDX, IDC_HIDE_CHARGEBACKS, m_checkHideChargebacks);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CStatementSetup, CNxDialog)
	//{{AFX_MSG_MAP(CStatementSetup)
	ON_BN_CLICKED(ID_OK_BTN, OnOkBtn)
	ON_BN_CLICKED(IDC_USE_GUARANTOR, OnUseGuarantor)
	ON_BN_CLICKED(IDC_REMIT, OnRemit)
	ON_BN_CLICKED(IDC_USE_70_VERSION, OnUse70Version)
	ON_BN_CLICKED(IDC_USE_PROVNAME, OnUseProvname)
	ON_BN_CLICKED(IDC_USE_PRACNAME, OnUsePracname)
	ON_BN_CLICKED(IDC_STMT_AR_NOTES, OnStmtArNotes)
	ON_BN_CLICKED(IDC_COMBINE_BILL_BALANCES, OnCombineBillBalances)
	ON_BN_CLICKED(IDC_ESTATEMENTFORMAT, OnEstatementformat)
	ON_BN_CLICKED(IDC_SHOW_LAST_PAY, OnShowLastPay)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_DEFAULTFORMAT, &CStatementSetup::OnBnClickedDefaultformat)
	ON_BN_CLICKED(1081, &CStatementSetup::OnBnClicked1081)
	ON_BN_CLICKED(IDC_AVERY2, &CStatementSetup::OnBnClickedAvery2)
	ON_BN_CLICKED(IDC_SHOW_BILLING_NOTES_ESTATE, &CStatementSetup::OnBnClickedShowBillingNotesEstate)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CStatementSetup, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CStatementSetup)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

/////////////////////////////////////////////////////////////////////////////
// CStatementSetup message handlers

BOOL CStatementSetup::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {
		// (c.haag 2008-05-02 10:09) - PLID 29879 - NxIconify the buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnStmtARNotes.AutoSet(NXB_MODIFY);

		// (j.dinatale 2011-03-21 15:17) - PLID 41444 - Added 'StatementsShowEStatementsPatientSelect'
		// (j.gruber 2010-02-04 13:00) - PLID 29120 - cache new property
		g_propManager.CachePropertiesInBulk("StatementSetup-Number", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'SttmntDefaultDetailed'  OR "
			"Name = 'ShowBillingNotesOnEStatement' OR "
			"Name = 'StatementsShowEStatementsPatientSelect' OR "
			// (j.jones 2011-04-12 10:46) - PLID 31219 - added ability to show all charges
			// on any bills with balances, when on the summary statement
			"Name = 'SttmntIncludePaidCharges' OR "
			"Name = 'SttmntEnvelope' OR "
			"Name = 'SttmntAge' OR "
			"Name = 'StatementsPrintMultipleParties' OR "
			"Name = 'SttmntUseGuarantor' OR "
			"Name = 'SttmntUseDocName' OR "
			"Name = 'SttmntHeaderProvFormat' OR "
			"Name = 'SttmntUseDocAddress' OR "
			"Name = 'SttmntTransProvFormat' OR "
			"Name = 'SttmntHidePrePayments' OR "
			"Name = 'SttmntHideCPTCodes' OR "
			"Name = 'SttmntUseComma' OR "
			"Name = 'SttmntShowDiag' OR "
			"Name = 'SttmntRemitLocation' OR "
			"Name = 'SttmntShowFooter' OR "
			"Name = 'SttmntUseRemit' OR "
			"Name = 'SttmntUse70Version' OR "
			"Name = 'SttmntAcceptVisa' OR "
			"Name = 'SttmntAcceptMstrCard' OR "
			"Name = 'SttmntAcceptDiscover' OR "
			"Name = 'SttmntAcceptAmex' OR "
			"Name = 'SttmntDaysOld' OR "
			"Name = 'SttmntCombineBillsAfterXDaysOld' OR "
			"Name = 'SttmntShowLastPayInfo' OR "
			"Name = 'SttmntHideChargebacks' " //TES 7/10/2014 - PLID 62563
			")",
			_Q(GetCurrentUserName()));

		g_propManager.CachePropertiesInBulk("StatementSetup-Text", propText,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'SttmntPaymentDesc' OR "
			"Name = 'SttmntAdjustmentDesc' OR "
			"Name = 'SttmntRefundDesc' "
			")",
			_Q(GetCurrentUserName()));

		g_propManager.CachePropertiesInBulk("StatementSetup-Memo", propMemo,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'SttmntName' OR "
			"Name = 'SttmntCallMe' OR "
			"Name = 'SttmntText' "
			")",
			_Q(GetCurrentUserName()));

		//figure out whether they use Avery1 or Avery 2 or a default format
		long nStyle = GetRemotePropertyInt("SttmntEnvelope", 0, 0, "<None>");
		if (nStyle == 0) {
			SetDlgItemCheck(1081, 1);
			EnableDetailedButtons(TRUE);
		}
		else if (nStyle == 1) {
			SetDlgItemCheck(IDC_AVERY2, 1);
			EnableDetailedButtons(TRUE);
		}
		else if (nStyle == 2) {
			SetDlgItemCheck(IDC_DEFAULTFORMAT, 1);
			EnableDetailedButtons(FALSE);
		}
		else if (nStyle == 3) {
			SetDlgItemCheck(IDC_ESTATEMENTFORMAT, 1);
			EnableDetailedButtons(TRUE);
		}
		else {
			MsgBox("Error Setting Statement Format.\nPlease set the correct header format");
		}

		// (j.dinatale 2011-03-21 15:15) - PLID 41444 - need to make sure the show patient selection checkbox is properly enabled/disabled on load
		if (nStyle == 3){
			m_btnShowPatSelectWnd.EnableWindow(TRUE);
		}
		else{
			m_btnShowPatSelectWnd.EnableWindow(FALSE);
		}

		// (j.dinatale 2011-03-21 15:16) - PLID 41444 - Set the estatement patient selection checkbox to go according to the current value of the preference
		long nShowPatSel = GetRemotePropertyInt("StatementsShowEStatementsPatientSelect", 0, 0, "<None>");
		if (nShowPatSel == 0){
			m_btnShowPatSelectWnd.SetCheck(FALSE);
		}
		else{
			m_btnShowPatSelectWnd.SetCheck(TRUE);
		}

		// (j.gruber 2010-02-04 12:56) - PLID 29120 - add detail/summary option
		long nDetailed = GetRemotePropertyInt("SttmntDefaultDetailed", 1, 0, "<None>");
		if (nDetailed == 1) {
			SetDlgItemCheck(IDC_STMT_DETAILED, 1);
			SetDlgItemCheck(IDC_STMT_SUMMARY, 0);
		}
		else {
			SetDlgItemCheck(IDC_STMT_SUMMARY, 1);
			SetDlgItemCheck(IDC_STMT_DETAILED, 0);
		}

		// (j.jones 2011-04-12 10:46) - PLID 31219 - added ability to show all charges
		// on any bills with balances, when on the summary statement
		m_checkIncludePaidChargesOnBills.SetCheck(GetRemotePropertyInt("SttmntIncludePaidCharges", 0, 0, "<None>") == 1);

		SetDlgItemText(IDC_EXTRA_TEXT, GetRemotePropertyMemo("SttmntText", "", 0, "<None>"));
		SetDlgItemText(IDC_PCC_TITLE, GetRemotePropertyMemo("SttmntCallMe", "", 0, "<None>"));
		//Put the age in the correct box if it is selected
		if (GetRemotePropertyInt("SttmntUseGuarantor", -1, 0, "<None>") == 0) {
			GetDlgItem(IDC_EDIT_AGE)->EnableWindow(FALSE);
		}
		else {
			GetDlgItem(IDC_EDIT_AGE)->EnableWindow(TRUE);
		}

		SetDlgItemInt(IDC_EDIT_AGE, GetRemotePropertyInt("SttmntAge", 16, 0, "<None>"));

		// (a.walling 2006-10-24 13:22) - PLID 16059 - Preference to print multiple statements or not
		long nMultiple = GetRemotePropertyInt("StatementsPrintMultipleParties", 0, 0, "<None>", true);
		SetDlgItemCheck(IDC_PRINT_MULTI_RESP, nMultiple);

		UpdateData();

		m_pNameList = BindNxDataListCtrl(this, IDC_NAMELIST, GetRemoteData(), true);
		IRowSettingsPtr pRow;
		pRow = m_pNameList->GetRow(-1);
		pRow->Value[0] = (long)-1;
		pRow->Value[1] = (variant_t)"None";
		m_pNameList->AddRow(pRow);
		m_pNameList->SetSelByColumn(1, _variant_t(GetRemotePropertyMemo("SttmntName")));

		//Initialize the provider format DL
		m_ProvFormatList = BindNxDataListCtrl(this, IDC_PROV_FORMAT_LIST, GetRemoteData(), false);
		pRow = m_ProvFormatList->GetRow(-1);
		pRow->PutValue(0, (long)0);
		pRow->PutValue(1, "First Last Title");
		m_ProvFormatList->AddRow(pRow);

		pRow = m_ProvFormatList->GetRow(-1);
		pRow->PutValue(0, (long)1);
		pRow->PutValue(1, "First Middle Last Title");
		m_ProvFormatList->AddRow(pRow);

		pRow = m_ProvFormatList->GetRow(-1);
		pRow->PutValue(0, (long)2);
		pRow->PutValue(1, "Last Title");
		m_ProvFormatList->AddRow(pRow);

		pRow = m_ProvFormatList->GetRow(-1);
		pRow->PutValue(0, (long)3);
		pRow->PutValue(1, "Last");
		m_ProvFormatList->AddRow(pRow);

		pRow = m_ProvFormatList->GetRow(-1);
		pRow->PutValue(0, (long)4);
		pRow->PutValue(1, "Prefix Last");
		m_ProvFormatList->AddRow(pRow);

		pRow = m_ProvFormatList->GetRow(-1);
		pRow->PutValue(0, (long)5);
		pRow->PutValue(1, "Prefix First Last");
		m_ProvFormatList->AddRow(pRow);

		pRow = m_ProvFormatList->GetRow(-1);
		pRow->PutValue(0, (long)6);
		pRow->PutValue(1, "Last, Title");
		m_ProvFormatList->AddRow(pRow);

		pRow = m_ProvFormatList->GetRow(-1);
		pRow->PutValue(0, (long)7);
		pRow->PutValue(1, "First MI Last Title");
		m_ProvFormatList->AddRow(pRow);

		pRow = m_ProvFormatList->GetRow(-1);
		pRow->PutValue(0, (long)8);
		pRow->PutValue(1, "Prefix First MI Last");
		m_ProvFormatList->AddRow(pRow);

		pRow = m_ProvFormatList->GetRow(-1);
		pRow->PutValue(0, (long)9);
		pRow->PutValue(1, "Prefix First MI Last Title");
		m_ProvFormatList->AddRow(pRow);

		//Initialize the provider format DL
		m_ProvHeaderFormatList = BindNxDataListCtrl(this, IDC_HEADER_PROV_FORMAT_LIST, GetRemoteData(), false);
		pRow = m_ProvHeaderFormatList->GetRow(-1);
		pRow->PutValue(0, (long)0);
		pRow->PutValue(1, "First Last Title");
		m_ProvHeaderFormatList->AddRow(pRow);

		pRow = m_ProvHeaderFormatList->GetRow(-1);
		pRow->PutValue(0, (long)1);
		pRow->PutValue(1, "First Middle Last Title");
		m_ProvHeaderFormatList->AddRow(pRow);

		pRow = m_ProvHeaderFormatList->GetRow(-1);
		pRow->PutValue(0, (long)2);
		pRow->PutValue(1, "Last Title");
		m_ProvHeaderFormatList->AddRow(pRow);

		pRow = m_ProvHeaderFormatList->GetRow(-1);
		pRow->PutValue(0, (long)3);
		pRow->PutValue(1, "Last");
		m_ProvHeaderFormatList->AddRow(pRow);

		pRow = m_ProvHeaderFormatList->GetRow(-1);
		pRow->PutValue(0, (long)4);
		pRow->PutValue(1, "Prefix Last");
		m_ProvHeaderFormatList->AddRow(pRow);

		pRow = m_ProvHeaderFormatList->GetRow(-1);
		pRow->PutValue(0, (long)5);
		pRow->PutValue(1, "Prefix First Last");
		m_ProvHeaderFormatList->AddRow(pRow);

		pRow = m_ProvHeaderFormatList->GetRow(-1);
		pRow->PutValue(0, (long)6);
		pRow->PutValue(1, "Last, Title");
		m_ProvHeaderFormatList->AddRow(pRow);

		pRow = m_ProvHeaderFormatList->GetRow(-1);
		pRow->PutValue(0, (long)7);
		pRow->PutValue(1, "First MI Last Title");
		m_ProvHeaderFormatList->AddRow(pRow);

		pRow = m_ProvHeaderFormatList->GetRow(-1);
		pRow->PutValue(0, (long)8);
		pRow->PutValue(1, "Prefix First MI Last");
		m_ProvHeaderFormatList->AddRow(pRow);

		pRow = m_ProvHeaderFormatList->GetRow(-1);
		pRow->PutValue(0, (long)9);
		pRow->PutValue(1, "Prefix First MI Last Title");
		m_ProvHeaderFormatList->AddRow(pRow);

		if (GetRemotePropertyInt("SttmntUseGuarantor", -1, 0, "<None>"))
			m_guarantorCheck.SetCheck(1);
		else m_guarantorCheck.SetCheck(0);

		if (GetRemotePropertyInt("SttmntUseDocName", -1, 0, "<None>")) // use doctor name
		{
			m_providerRadio.SetCheck(1);
			m_practiceRadio.SetCheck(0);

			GetDlgItem(IDC_HEADER_PROV_FORMAT_LIST)->EnableWindow(TRUE);
			//check which setting we have for the format
			long nProvFormat = GetRemotePropertyInt("SttmntHeaderProvFormat", 0, 0, "<None>");
			m_ProvHeaderFormatList->SetSelByColumn(0, nProvFormat);


		}
		else
		{
			m_providerRadio.SetCheck(0);
			m_practiceRadio.SetCheck(1);

			//disable the datalist
			GetDlgItem(IDC_HEADER_PROV_FORMAT_LIST)->EnableWindow(FALSE);

		}

		//initialize the Practice/Provider Address radio buttons
		if (GetRemotePropertyInt("SttmntUseDocAddress", -1, 0, "<None>")) {

			CheckDlgButton(IDC_USE_PROVADDRESS, 1);
			CheckDlgButton(IDC_USE_PRACADDRESS, 0);
		}
		else {
			CheckDlgButton(IDC_USE_PROVADDRESS, 0);
			CheckDlgButton(IDC_USE_PRACADDRESS, 1);

		}

		long nProvFormat = GetRemotePropertyInt("SttmntTransProvFormat", 0, 0, "<None>");
		m_ProvFormatList->SetSelByColumn(0, nProvFormat);

		//initialize the prepayments checkbox
		if (GetRemotePropertyInt("SttmntHidePrePayments", 0, 0, "<None>")) {

			CheckDlgButton(IDC_HIDEPREPAYS, 1);
		}
		else {
			CheckDlgButton(IDC_HIDEPREPAYS, 0);
		}

		//TES 7/10/2014 - PLID 62563 - Added Hide Chargebacks, i they have the Vision Payments license
		if (!g_pLicense->CheckForLicense(CLicense::lcVisionPayments, CLicense::cflrSilent)) {
			GetDlgItem(IDC_HIDE_CHARGEBACKS)->ShowWindow(SW_HIDE);
		}
		else {
			if (GetRemotePropertyInt("SttmntHideChargebacks", 1, 0, "<None>")) {
				CheckDlgButton(IDC_HIDE_CHARGEBACKS, BST_CHECKED);
			}
			else {
				CheckDlgButton(IDC_HIDE_CHARGEBACKS, BST_UNCHECKED);
			}
		}

		//initialize the CPT Codes checkbox
		if(GetRemotePropertyInt("SttmntHideCPTCodes", 0, 0, "<None>")) {

			CheckDlgButton(IDC_HIDE_CPT_CODES, 1);					       
		}
		else {
			CheckDlgButton(IDC_HIDE_CPT_CODES, 0);
		}	



		if (GetRemotePropertyInt("SttmntUseComma", -1, 0, "<None>")) // use Comma PatName
		{
			m_commaRadio.SetCheck(1);
			m_forwardRadio.SetCheck(0);
		}
		else
		{
			m_commaRadio.SetCheck(0);
			m_forwardRadio.SetCheck(1);
		}

		if (GetRemotePropertyInt("SttmntShowDiag", -1, 0, "<None>")) // Show Diag Codes
		{	
			m_useDiagRadio.SetCheck(1);
			m_noDiagRadio.SetCheck(0);
		}
		else
		{
			m_useDiagRadio.SetCheck(0);
			m_noDiagRadio.SetCheck(1);
		}


		//initialize the datalist
		m_pLocList = BindNxDataListCtrl(this, IDC_LOCLIST, GetRemoteData(), true);

		//set the correct location
		long nLocationID = GetRemotePropertyInt("SttmntRemitLocation", -1, 0, "<None>");
		
		//Get whether they want to see the footer
		if (GetRemotePropertyInt("SttmntShowFooter", -1, 0, "<None>"))
			m_hideFooterCheck.SetCheck(1);

		//find out whether they always remit to the same location
		if (GetRemotePropertyInt("SttmntUseRemit", -1, 0, "<None>")) {
			//first check the box
			CheckDlgButton(IDC_REMIT, 1);

			//now set the location with the id we already have and enable the dialog
			GetDlgItem(IDC_LOCLIST)->EnableWindow(true);
			m_pLocList->SetSelByColumn(0, nLocationID);
		}
		else {

			//uncheck the box
			CheckDlgButton(IDC_REMIT, 0);

			//disable and clear the list
			m_pLocList->PutCurSel(-1);
			GetDlgItem(IDC_LOCLIST)->EnableWindow(false);
		}


		//initialize the credit descriptions
		SetDlgItemText(IDC_PAYMENTDESC, GetRemotePropertyText("SttmntPaymentDesc", "", 0, "<None>"));
		SetDlgItemText(IDC_ADJUSTMENTDESC, GetRemotePropertyText("SttmntAdjustmentDesc", "", 0, "<None>"));
		SetDlgItemText(IDC_REFUNDDESC, GetRemotePropertyText("SttmntRefundDesc", "", 0, "<None>"));


		//initialize 7.0 reports
		if (GetRemotePropertyInt("SttmntUse70Version", 0, 0, "<None>")) {

			CheckDlgButton(IDC_USE_70_VERSION, 1);
		}
		else {

			CheckDlgButton(IDC_USE_70_VERSION, 0);
		}

		// (j.gruber 2010-08-20 09:34) - PLID 40193 - billing notes on e-statement		
		if (GetRemotePropertyInt("ShowBillingNotesOnEStatement", 0, 0, "<None>")) {

			CheckDlgButton(IDC_SHOW_BILLING_NOTES_ESTATE, 1);
		}
		else {

			CheckDlgButton(IDC_SHOW_BILLING_NOTES_ESTATE, 0);
		}



		//initialize credit cards
		if (GetRemotePropertyInt("SttmntAcceptVisa", 1, 0, "<None>")) {

			CheckDlgButton(IDC_ACCEPT_VISA, 1);
		}
		else {

			CheckDlgButton(IDC_ACCEPT_VISA, 0);
		}


		if (GetRemotePropertyInt("SttmntAcceptMstrCard", 1, 0, "<None>")) {

			CheckDlgButton(IDC_ACCEPT_MSTRCARD, 1);
		}
		else {

			CheckDlgButton(IDC_ACCEPT_MSTRCARD, 0);
		}
			

		if (GetRemotePropertyInt("SttmntAcceptDiscover", 1, 0, "<None>")) {

			CheckDlgButton(IDC_ACCEPT_DISCOVER, 1);
		}
		else {

			CheckDlgButton(IDC_ACCEPT_DISCOVER, 0);
		}

		if (GetRemotePropertyInt("SttmntAcceptAmex", 1, 0, "<None>")) {

			CheckDlgButton(IDC_ACCEPT_AMEX, 1);
		}
		else {

			CheckDlgButton(IDC_ACCEPT_AMEX, 0);
		}

			
		// (j.gruber 2007-01-05 09:44) - PLID 24036 - checkbox for Combine bills after x day old 

		long nDays = GetRemotePropertyInt("SttmntDaysOld", 60, 0, "<None>");

		if (GetRemotePropertyInt("SttmntCombineBillsAfterXDaysOld", 0, 0, "<None>")) {

			long nDays = GetRemotePropertyInt("SttmntDaysOld", 60, 0, "<None>");
			CheckDlgButton(IDC_COMBINE_BILL_BALANCES, 1);
			SetDlgItemInt(IDC_DAYS_OLD, nDays);
			GetDlgItem(IDC_DAYS_OLD)->EnableWindow(TRUE);
			
		}
		else {
			CheckDlgButton(IDC_COMBINE_BILL_BALANCES, 0);
			SetDlgItemInt(IDC_DAYS_OLD, nDays);
			GetDlgItem(IDC_DAYS_OLD)->EnableWindow(FALSE);
		}

		// (j.gruber 2007-01-09 09:28) - PLID 24168 - checkbox to show last payment info on e-statement
		if (GetRemotePropertyInt("SttmntShowLastPayInfo", 0,0,"<None>")) {

			CheckDlgButton(IDC_SHOW_LAST_PAY, 1);
		}
		else {
			CheckDlgButton(IDC_SHOW_LAST_PAY, 0);
		}

		
	} NxCatchAll("Error in CStatementSetup::OnInitDialog() ");
	
	return TRUE;
}

void CStatementSetup::MoveInfo (CRect *rect, long newLeft, long newTop)
{
}

void CStatementSetup::WriteInfo (CString Field, long Left, long Top)
{
}

void CStatementSetup::OnOkBtn() 
{
	CRect rect, toprect;
	short x = 15;  // multiplier
	CString str;


	GetDlgItemText (IDC_EXTRA_TEXT, str);
	SetRemotePropertyMemo ("SttmntText", str, 0, "<None>");
	

	GetDlgItemText (IDC_PCC_TITLE, str);
	long nCurSel = m_pNameList->GetCurSel();
	SetRemotePropertyMemo ("SttmntCallMe", str, 0, "<None>");
	if(nCurSel != -1)
		SetRemotePropertyMemo ("SttmntName", VarString(m_pNameList->GetValue(nCurSel, 1)), 0, "<None>");
	SetRemotePropertyInt ("SttmntUseGuarantor", m_guarantorCheck.GetCheck(), 0, "<None>");
	SetRemotePropertyInt ("SttmntShowFooter", m_hideFooterCheck.GetCheck(), 0, "<None>");
	SetRemotePropertyInt ("SttmntUseDocName", m_providerRadio.GetCheck(), 0, "<None>");
	SetRemotePropertyInt ("SttmntHidePrePayments", IsDlgButtonChecked(IDC_HIDEPREPAYS), 0, "<None>");
	//TES 7/10/2014 - PLID 62563 - Added Hide Chargebacks
	SetRemotePropertyInt("SttmntHideChargebacks", IsDlgButtonChecked(IDC_HIDE_CHARGEBACKS), 0, "<None>");
	SetRemotePropertyInt ("SttmntUseComma", m_commaRadio.GetCheck(), 0, "<None>");
	SetRemotePropertyInt ("SttmntShowDiag", m_useDiagRadio.GetCheck(), 0, "<None>");
	SetRemotePropertyInt ("SttmntAge", GetDlgItemInt(IDC_EDIT_AGE), 0, "<None>");

	// (j.gruber 2010-02-04 13:01) - PLID 29120 - default detailed/summary
	SetRemotePropertyInt("SttmntDefaultDetailed", m_btnDetailed.GetCheck(), 0, "<None>");

	// (a.walling 2006-10-24 13:24) - PLID 16059 - Print multiple statements for resp. parties
	SetRemotePropertyInt ("StatementsPrintMultipleParties", m_printMultiCheck.GetCheck(), 0, "<None>");	

	SetRemotePropertyInt("SttmntTransProvFormat", m_ProvFormatList->GetValue(m_ProvFormatList->CurSel, 0), 0, "<None>");
	if (IsDlgButtonChecked(IDC_USE_PROVNAME)) {
		SetRemotePropertyInt("SttmntHeaderProvFormat", m_ProvHeaderFormatList->GetValue(m_ProvHeaderFormatList->CurSel, 0), 0, "<None>");
	}

	if (IsDlgButtonChecked(IDC_USE_PROVADDRESS)) {
		SetRemotePropertyInt ("SttmntUseDocAddress", 1, 0, "<None>");
	}
	else {
		SetRemotePropertyInt ("SttmntUseDocAddress", 0, 0, "<None>");
	}
		

	if  (IsDlgButtonChecked(1081)) {
		SetRemotePropertyInt("SttmntEnvelope", 0, 0, "<None>");
	}
	else if (IsDlgButtonChecked(IDC_AVERY2)) {
		SetRemotePropertyInt("SttmntEnvelope", 1, 0, "<None>");
	}
	else if (IsDlgButtonChecked(IDC_DEFAULTFORMAT)) {
		SetRemotePropertyInt("SttmntEnvelope", 2, 0, "<None>");
	}
	else  if (IsDlgButtonChecked(IDC_ESTATEMENTFORMAT)) {
		
		if (! IsDlgButtonChecked(IDC_USE_70_VERSION)) {

			//tell them that they have to have the 7.0 Button checked
			MsgBox("In order to use the E-Statements, you must use the 7.0 statements. \n The Use 7.0 Statement checkbox will be automatically checked for you.");
			//check the box and set it in the data to make sure
			CheckDlgButton(IDC_USE_70_VERSION, 1);
			SetRemotePropertyInt("SttmntUse70Version", 1, 0, "<None>");
		}
			
		SetRemotePropertyInt("SttmntEnvelope", 3, 0, "<None>");


	}
	else {
		//Tell them to choose a format
		MsgBox("Please choose a Header Format");
	}

	// (j.dinatale 2011-03-21 15:18) - PLID 41444 - save the value of the Show Estatement patient selection dialog checkbox
	SetRemotePropertyInt("StatementsShowEStatementsPatientSelect", m_btnShowPatSelectWnd.GetCheck(), 0, "<None>");	

	// (j.jones 2011-04-12 10:46) - PLID 31219 - added ability to show all charges
	// on any bills with balances, when on the summary statement
	SetRemotePropertyInt("SttmntIncludePaidCharges", m_checkIncludePaidChargesOnBills.GetCheck() ? 1 : 0, 0, "<None>");

	//set the remit checkbox
	long nCheck = IsDlgButtonChecked(IDC_REMIT);
	SetRemotePropertyInt("SttmntUseRemit", nCheck, 0, "<None>");
	if (nCheck) {
		if(m_pLocList->CurSel != -1) {
			SetRemotePropertyInt("SttmntRemitLocation", VarLong(m_pLocList->GetValue(m_pLocList->CurSel, 0)), 0, "<None>");
		}
		else {
			//this should never happen anymore, but just in case
			MessageBox("Please select a location to remit to");
			m_pLocList->DropDownState = 1;
			return;
			
		}
	}

	CString strPaymentDesc, strAdjustmentDesc, strRefundDesc;
	GetDlgItemText(IDC_PAYMENTDESC, strPaymentDesc);
	GetDlgItemText(IDC_ADJUSTMENTDESC, strAdjustmentDesc);
	GetDlgItemText(IDC_REFUNDDESC, strRefundDesc);

	SetRemotePropertyText("SttmntPaymentDesc", strPaymentDesc, 0, "<None>");
	SetRemotePropertyText("SttmntAdjustmentDesc", strAdjustmentDesc, 0, "<None>");
	SetRemotePropertyText("SttmntRefundDesc", strRefundDesc, 0, "<None>");

    nCheck = IsDlgButtonChecked(IDC_USE_70_VERSION);
	SetRemotePropertyInt("SttmntUse70Version", nCheck, 0, "<None>");

	//show CPT Codes
	SetRemotePropertyInt ("SttmntHideCPTCodes", IsDlgButtonChecked(IDC_HIDE_CPT_CODES), 0, "<None>");

	SetRemotePropertyInt ("SttmntAcceptVisa", IsDlgButtonChecked(IDC_ACCEPT_VISA), 0, "<None>");
	SetRemotePropertyInt ("SttmntAcceptMstrCard", IsDlgButtonChecked(IDC_ACCEPT_MSTRCARD), 0, "<None>");
	SetRemotePropertyInt ("SttmntAcceptDiscover", IsDlgButtonChecked(IDC_ACCEPT_DISCOVER), 0, "<None>");
	SetRemotePropertyInt ("SttmntAcceptAmex", IsDlgButtonChecked(IDC_ACCEPT_AMEX), 0, "<None>");

	// (j.gruber 2010-08-20 09:35) - PLID 40193 - Billing notes on estatement
	SetRemotePropertyInt ("ShowBillingNotesOnEStatement", IsDlgButtonChecked(IDC_SHOW_BILLING_NOTES_ESTATE), 0, "<None>");

	// (j.gruber 2007-01-05 09:44) - PLID 24036 - checkbox for Combine bills after x day old 
	long nDays = GetDlgItemInt(IDC_DAYS_OLD);
	SetRemotePropertyInt("SttmntDaysOld", nDays, 0, "<None>");
	
	if (IsDlgButtonChecked(IDC_COMBINE_BILL_BALANCES)) {
		SetRemotePropertyInt("SttmntCombineBillsAfterXDaysOld", 1, 0, "<None>");
	}
	else {
		SetRemotePropertyInt("SttmntCombineBillsAfterXDaysOld", 0, 0, "<None>");
	}

	// (j.gruber 2007-01-09 10:17) - PLID 24168 - make show payment information
	SetRemotePropertyInt ("SttmntShowLastPayInfo", IsDlgButtonChecked(IDC_SHOW_LAST_PAY), 0, "<None>");

		
	//send the network message to update
	CClient::RefreshTable(NetUtils::StatementConfig);	
	
	CDialog::OnOK();
}

//void CStatementSetup::InsertInfo (CNxRecordset *InRC)
//{
//	
//}

BOOL CStatementSetup::OnCommand(WPARAM wParam, LPARAM lParam) 
{

	//Sleep (1);
	
	return CDialog::OnCommand(wParam, lParam);
}

void CStatementSetup::OnUseGuarantor() 
{
	if (m_guarantorCheck.GetCheck())
		GetDlgItem(IDC_EDIT_AGE)->EnableWindow(TRUE);
	else GetDlgItem(IDC_EDIT_AGE)->EnableWindow(FALSE);
}

void CStatementSetup::OnRemit() 
{
	if (m_Remit.GetCheck()) {

		long nLocationID = GetRemotePropertyInt("SttmntRemitLocation", -1, 0, "<None>");
		GetDlgItem(IDC_LOCLIST)->EnableWindow(true);
		
		long nResult = m_pLocList->SetSelByColumn(0, nLocationID);
		if (nResult == -1) {
			//it didn't find one, so select the first row and drop down the list
			m_pLocList->CurSel = 0;
			m_pLocList->DropDownState = 1;
		}

		
	}
	else {
		//disable the location
		m_pLocList->CurSel = -1;
		GetDlgItem(IDC_LOCLIST)->EnableWindow(false);

		
	}

		
	
}

void CStatementSetup::OnUse70Version() 
{
	// TODO: Add your control notification handler code here
	
}

void CStatementSetup::OnUseProvname() 
{
	if (IsDlgButtonChecked(IDC_USE_PROVNAME)) {

		GetDlgItem(IDC_HEADER_PROV_FORMAT_LIST)->EnableWindow(TRUE);
		m_ProvHeaderFormatList->SetSelByColumn(0, GetRemotePropertyInt("SttmntHeaderProvFormat", 0, 0, "<None>"));
	}
	
	
}

void CStatementSetup::OnUsePracname() 
{
	if (IsDlgButtonChecked(IDC_USE_PRACNAME)) {
		
		GetDlgItem(IDC_HEADER_PROV_FORMAT_LIST)->EnableWindow(FALSE);
	}
	
}

void CStatementSetup::OnStmtArNotes() 
{
	CStatementARNotesConfigDlg dlg(this);
	dlg.DoModal();
	
}

// (j.gruber 2007-01-05 09:44) - PLID 24036 - checkbox for Combine show bills after x day old 
void CStatementSetup::OnCombineBillBalances() 
{
	if (IsDlgButtonChecked(IDC_COMBINE_BILL_BALANCES)) {
		long nDays = GetRemotePropertyInt("SttmntDaysOld", 60, 0, "<None>");
		CheckDlgButton(IDC_COMBINE_BILL_BALANCES, 1);
		GetDlgItem(IDC_DAYS_OLD)->EnableWindow(TRUE);
		SetDlgItemInt(IDC_DAYS_OLD, nDays);
	}
	else {
		GetDlgItem(IDC_DAYS_OLD)->EnableWindow(FALSE);
	}
	
}

void CStatementSetup::OnEstatementformat() 
{

	// (j.gruber 2010-02-04 13:17) - PLID 29120 - enable/disable the detailed/summary buttons
	try {
		if (m_btnEStatement.GetCheck()) {
			EnableDetailedButtons(TRUE);

			// (j.dinatale 2011-03-21 15:19) - PLID 41444 - want the patient select checkbox enabled
			m_btnShowPatSelectWnd.EnableWindow(TRUE);
		}
	}NxCatchAll(__FUNCTION__);

	/*if (IsDlgButtonChecked(IDC_ESTATEMENTFORMAT)) {
		GetDlgItem(IDC_SHOW_LAST_PAY)->EnableWindow(TRUE);		
	}
	else {
		GetDlgItem(IDC_SHOW_LAST_PAY)->EnableWindow(FALSE);
	}*/	
	
}

// (j.gruber 2007-01-10 14:47) - PLID 24168 - warn them that they might have to remap their e-statement
void CStatementSetup::OnShowLastPay() 
{
	if (IsDlgButtonChecked(IDC_SHOW_LAST_PAY)) {
		CDontShowDlg dlg(this);
		dlg.DoModal("If you are using e-statement format, this may require your e-statement to be remapped by your e-billing clearinghouse.  Please contact your e-billing clearinghouse and inform them of this before sending your e-statements.", "NexTech", "E-Statement Warning", FALSE);
	}
	else {
		CDontShowDlg dlg(this);
		dlg.DoModal("If you are using e-statement format, this may require your e-statement to be remapped by your e-billing clearinghouse.  Please contact your e-billing clearinghouse and inform them of this before sending your e-statements.", "NexTech", "E-Statement Warning", FALSE);
	}		
	
}

void CStatementSetup::OnBnClickedDefaultformat()
{
	// (j.gruber 2010-02-04 13:17) - PLID 29120 - enable/disable the detailed/summary buttons
	try {
		if (m_btnDefault.GetCheck()) {
			EnableDetailedButtons(FALSE);

			// (j.dinatale 2011-03-21 15:19) - PLID 41444 - want the show patient select dialog checkbox disabled
			m_btnShowPatSelectWnd.EnableWindow(FALSE);
		}
	}NxCatchAll(__FUNCTION__);
}

void CStatementSetup::OnBnClicked1081()
{
	// (j.gruber 2010-02-04 13:17) - PLID 29120 - enable/disable the detailed/summary buttons
	try {
		if (m_btn1081.GetCheck()) {
			EnableDetailedButtons(TRUE);

			// (j.dinatale 2011-03-21 15:19) - PLID 41444 - want the show patient select dialog checkbox disabled
			m_btnShowPatSelectWnd.EnableWindow(FALSE);
		}
	}NxCatchAll(__FUNCTION__);
}

void CStatementSetup::OnBnClickedAvery2()
{
	// (j.gruber 2010-02-04 13:17) - PLID 29120 - enable/disable the detailed/summary buttons
	try {
		if (m_btnAvery2.GetCheck()) {
			EnableDetailedButtons(TRUE);

			// (j.dinatale 2011-03-21 15:19) - PLID 41444 - want the show patient select dialog checkbox disabled
			m_btnShowPatSelectWnd.EnableWindow(FALSE);
		}
	}NxCatchAll(__FUNCTION__);
}


void CStatementSetup::EnableDetailedButtons(BOOL bEnable){

	m_btnDetailed.EnableWindow(bEnable);
	m_btnSummary.EnableWindow(bEnable);

}

// (j.gruber 2010-08-20 09:33) - PLID 40193 - warn them to remap
void CStatementSetup::OnBnClickedShowBillingNotesEstate()
{
	try {

		long nCheckedInData = GetRemotePropertyInt("ShowBillingNotesOnEStatement", 0, 0, "<None>");
		BOOL bCheckedInData = FALSE;
		if (nCheckedInData != 0) {
			bCheckedInData = TRUE;
		}

		BOOL bChecked = IsDlgButtonChecked(IDC_SHOW_BILLING_NOTES_ESTATE);		

		if (bChecked != bCheckedInData) {
			MsgBox("Changing this option may result in your e-statements needing to be re-mapped.  \r\nPlease contact your e-statement vendor prior to sending statements after changing this option to ensure correct printing.");	
		}

	}NxCatchAll(__FUNCTION__);
}

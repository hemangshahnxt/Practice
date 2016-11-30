// QuickbooksLink.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "QuickbooksLink.h"
#include "QuickbooksUtils.h"
#include "QuickbooksLinkAccountsDlg.h"
#include "ChooseTwoQBAcctsDlg.h"
#include "QuickbooksLinkConfigDemogDlg.h"
#include "GlobalDrawingUtils.h"

// (a.walling 2012-08-03 14:31) - PLID 51956 - Compiler limits exceeded with imported NxAccessor typelib - get quickbooks out of stdafx
// (a.walling 2014-04-30 15:19) - PLID 61989 - import a typelibrary rather than a dll
#import "QBFC3.tlb" no_namespace

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CQuickbooksLink dialog


CQuickbooksLink::CQuickbooksLink(CWnd* pParent /*=NULL*/)
	: CNxDialog(CQuickbooksLink::IDD, pParent)
{
	//{{AFX_DATA_INIT(CQuickbooksLink)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CQuickbooksLink::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CQuickbooksLink)
	DDX_Control(pDX, IDC_CHECK_CHOOSE_INDIV_SOURCE_ACCTS, m_checkPromptForSourceAccts);
	DDX_Control(pDX, IDC_CHECK_EXPORT_PATIENTS, m_checkExportPatients);
	DDX_Control(pDX, IDC_RADIO_RECEIVE_PAYMENTS, m_radioReceivePayments);
	DDX_Control(pDX, IDC_RADIO_MAKE_DEPOSITS, m_radioMakeDeposits);
	DDX_Control(pDX, IDC_RADIO_USE_SERVICE_DATE, m_radioUseServiceDate);
	DDX_Control(pDX, IDC_RADIO_USE_INPUT_DATE, m_radioUseInputDate);
	DDX_Control(pDX, IDC_RADIO_USE_TRANSFER_DATE, m_radioUseTransferDate);
	DDX_Control(pDX, IDC_RADIO_USE_DEPOSIT_DATE, m_radioUseDepositDate);
	DDX_Control(pDX, IDC_CHECK_DISABLE_QB_LINK, m_checkDisableQuickbooks);
	DDX_Control(pDX, IDC_QUICKBOOKS_FILE_PATH, m_nxeditQuickbooksFilePath);
	DDX_Control(pDX, IDC_DATE_DESCRIPTION, m_nxstaticDateDescription);
	DDX_Control(pDX, IDOK, m_btnClose);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CQuickbooksLink, CNxDialog)
	//{{AFX_MSG_MAP(CQuickbooksLink)
	ON_BN_CLICKED(IDC_BTN_BROWSE_QB_DATA, OnBtnBrowseQbData)
	ON_BN_CLICKED(IDC_CHECK_DISABLE_QB_LINK, OnCheckDisableQbLink)
	ON_EN_KILLFOCUS(IDC_QUICKBOOKS_FILE_PATH, OnKillfocusQuickbooksFilePath)
	ON_BN_CLICKED(IDC_BTN_LINK_ACCOUNTS_TO_PROVIDERS, OnBtnLinkAccountsToProviders)
	ON_BN_CLICKED(IDC_RADIO_USE_SERVICE_DATE, OnRadioUseServiceDate)
	ON_BN_CLICKED(IDC_RADIO_USE_INPUT_DATE, OnRadioUseInputDate)
	ON_BN_CLICKED(IDC_RADIO_USE_TRANSFER_DATE, OnRadioUseTransferDate)
	ON_BN_CLICKED(IDC_RADIO_MAKE_DEPOSITS, OnRadioMakeDeposits)
	ON_BN_CLICKED(IDC_RADIO_RECEIVE_PAYMENTS, OnRadioReceivePayments)
	ON_BN_CLICKED(IDC_BTN_CHOOSE_DEFAULT_SOURCE_DEPOSIT_ACCTS, OnBtnChooseDefaultSourceDepositAccts)
	ON_BN_CLICKED(IDC_CHECK_EXPORT_PATIENTS, OnCheckExportPatients)
	ON_BN_CLICKED(IDC_BTN_QB_HELP, OnBtnQbHelp)
	ON_BN_CLICKED(IDC_CHECK_CHOOSE_INDIV_SOURCE_ACCTS, OnCheckChooseIndivSourceAccts)
	ON_BN_CLICKED(IDC_BTN_CONFIGURE_DEMOG_OUTPUT, OnBtnConfigureDemogOutput)
	ON_BN_CLICKED(IDC_RADIO_USE_DEPOSIT_DATE, OnRadioUseDepositDate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CQuickbooksLink message handlers

BOOL CQuickbooksLink::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	m_btnClose.AutoSet(NXB_CLOSE);

	m_brush.CreateSolidBrush(PaletteColor(0x009ED6BA));
	
	SetDlgItemText(IDC_QUICKBOOKS_FILE_PATH,GetRemotePropertyText("QuickbooksDataPath","",0,"<None>",TRUE));
	m_checkDisableQuickbooks.SetCheck(GetRemotePropertyInt("DisableQuickBooks",0,0,"<None>",TRUE));

	long QBTxnDate = GetRemotePropertyInt("QuickbooksTxnDate",3,0,"<None>",TRUE);
	if(QBTxnDate == 1)
		m_radioUseServiceDate.SetCheck(TRUE);
	else if(QBTxnDate == 2)
		m_radioUseInputDate.SetCheck(TRUE);
	else if(QBTxnDate == 3)
		m_radioUseTransferDate.SetCheck(TRUE);
	else
		m_radioUseDepositDate.SetCheck(TRUE);
	
	OnDateRadio(FALSE);

	long QBExportStyle = GetRemotePropertyInt("QuickBooksExportStyle",0,0,"<None>",TRUE);
	if(QBExportStyle == 0)
		m_radioMakeDeposits.SetCheck(TRUE);
	else
		m_radioReceivePayments.SetCheck(TRUE);
	OnImplementationRadio();

	long QBExportPatients = GetRemotePropertyInt("QBDepositWithPatients",0,0,"<None>",TRUE);
	m_checkExportPatients.SetCheck(QBExportPatients);
	if(QBExportPatients == 0) {
		if(m_radioMakeDeposits.GetCheck()) {
			GetDlgItem(IDC_BTN_CONFIGURE_DEMOG_OUTPUT)->EnableWindow(TRUE);
		}
		else {
			GetDlgItem(IDC_BTN_CONFIGURE_DEMOG_OUTPUT)->EnableWindow(FALSE);
		}
	}
	else {
		GetDlgItem(IDC_BTN_CONFIGURE_DEMOG_OUTPUT)->EnableWindow(FALSE);
	}

	long QBPromptSourceAcct = GetRemotePropertyInt("QBPromptSourceAcct",0,0,"<None>",TRUE);
	m_checkPromptForSourceAccts.SetCheck(QBPromptSourceAcct);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CQuickbooksLink::OnBtnBrowseQbData() 
{
	CString strInitPath,
			strInOutPath;

	CFileDialog dlgBrowse(TRUE, "qbw", NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_NOREADONLYRETURN, "QuickBooks Data|*.qbw;*.qba|All Files|*.*|");

	GetDlgItemText(IDC_QUICKBOOKS_FILE_PATH,strInitPath);
	if (strInitPath != "")
		GetFilePath(strInitPath);
	else strInitPath = "c:\\";

	dlgBrowse.m_ofn.lpstrInitialDir = strInitPath;
	if (dlgBrowse.DoModal() == IDOK) 
	{
		SetDlgItemText(IDC_QUICKBOOKS_FILE_PATH,dlgBrowse.GetPathName());
		SetRemotePropertyText("QuickbooksDataPath",dlgBrowse.GetPathName(),0,"<None>");
	}
}

void CQuickbooksLink::OnCheckDisableQbLink() 
{
	if(m_checkDisableQuickbooks.GetCheck()) {
		SetRemotePropertyInt("DisableQuickBooks",1,0,"<None>");	
	}
	else {
		SetRemotePropertyInt("DisableQuickBooks",0,0,"<None>");	
	}
}

void CQuickbooksLink::OnKillfocusQuickbooksFilePath() 
{
	CString strPath;
	GetDlgItemText(IDC_QUICKBOOKS_FILE_PATH,strPath);
	SetRemotePropertyText("QuickbooksDataPath",strPath,0,"<None>");
}

BOOL CQuickbooksLink::DestroyWindow() 
{
	GetMainFrame()->UpdateAllViews();

	return CDialog::DestroyWindow();
}

void CQuickbooksLink::OnBtnLinkAccountsToProviders() 
{
	{
		CWaitCursor pWait;

		IQBSessionManagerPtr qb = QB_OpenSession();
		if(qb == NULL) {
			AfxMessageBox("The account linking can not be configured without a valid connection to QuickBooks.\n"
				"Please set up Quickbooks properly before using this feature.");
			return;
		}
		qb->EndSession();
	}

	CQuickbooksLinkAccountsDlg dlg(this);
	dlg.DoModal();
}

void CQuickbooksLink::OnRadioUseServiceDate() 
{
	OnDateRadio(TRUE);
}

void CQuickbooksLink::OnRadioUseInputDate() 
{
	OnDateRadio(TRUE);
}

void CQuickbooksLink::OnRadioUseTransferDate() 
{
	OnDateRadio(TRUE);
}

void CQuickbooksLink::OnRadioUseDepositDate() 
{
	OnDateRadio(TRUE);
}

void CQuickbooksLink::OnDateRadio(BOOL bSave) {

	if(m_radioUseServiceDate.GetCheck()) {

		SetDlgItemText(IDC_DATE_DESCRIPTION, "Payments exported to QuickBooks will "
			"use the Service Date from the Payment.");

		if(bSave)
			SetRemotePropertyInt("QuickbooksTxnDate",1,0,"<None>");
	}
	else if(m_radioUseInputDate.GetCheck()) {

		SetDlgItemText(IDC_DATE_DESCRIPTION, "Payments exported to QuickBooks will "
			"use the Input Date from the Payment.");

		if(bSave)
			SetRemotePropertyInt("QuickbooksTxnDate",2,0,"<None>");
	}
	else if(m_radioUseTransferDate.GetCheck()) {

		SetDlgItemText(IDC_DATE_DESCRIPTION, "Payments exported to QuickBooks will "
			"use the date that the Payment was transferred to QuickBooks.");

		if(bSave)
			SetRemotePropertyInt("QuickbooksTxnDate",3,0,"<None>");
	}
	else if(m_radioUseDepositDate.GetCheck()) {

		SetDlgItemText(IDC_DATE_DESCRIPTION, "Payments deposited from the Banking tab to QuickBooks will "
			"use the editable Deposit Date from that tab.\n Payments exported directly from the Payment screen "
			"will use the date that the Payment was transferred to QuickBooks.");

		if(bSave)
			SetRemotePropertyInt("QuickbooksTxnDate",4,0,"<None>");
	}
}

void CQuickbooksLink::OnRadioMakeDeposits() 
{
	OnImplementationRadio();
}

void CQuickbooksLink::OnRadioReceivePayments() 
{
	OnImplementationRadio();	
}

void CQuickbooksLink::OnImplementationRadio()
{
	if(m_radioMakeDeposits.GetCheck()) {
		SetRemotePropertyInt("QuickBooksExportStyle",0,0,"<None>");

		GetDlgItem(IDC_CHECK_EXPORT_PATIENTS)->EnableWindow(TRUE);

		GetDlgItem(IDC_BTN_CONFIGURE_DEMOG_OUTPUT)->EnableWindow(!m_checkExportPatients.GetCheck());

		GetDlgItem(IDC_CHECK_CHOOSE_INDIV_SOURCE_ACCTS)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_CHOOSE_DEFAULT_SOURCE_DEPOSIT_ACCTS)->EnableWindow(TRUE);		

		GetDlgItem(IDC_BTN_LINK_ACCOUNTS_TO_PROVIDERS)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO_USE_SERVICE_DATE)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO_USE_INPUT_DATE)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO_USE_TRANSFER_DATE)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO_USE_DEPOSIT_DATE)->EnableWindow(FALSE);

		SetDlgItemText(IDC_DATE_DESCRIPTION, "");
	}
	else if(m_radioReceivePayments.GetCheck()) {
		SetRemotePropertyInt("QuickBooksExportStyle",1,0,"<None>");

		GetDlgItem(IDC_CHECK_EXPORT_PATIENTS)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_CONFIGURE_DEMOG_OUTPUT)->EnableWindow(FALSE);
		GetDlgItem(IDC_CHECK_CHOOSE_INDIV_SOURCE_ACCTS)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_CHOOSE_DEFAULT_SOURCE_DEPOSIT_ACCTS)->EnableWindow(FALSE);

		GetDlgItem(IDC_BTN_LINK_ACCOUNTS_TO_PROVIDERS)->EnableWindow(TRUE);
		GetDlgItem(IDC_RADIO_USE_SERVICE_DATE)->EnableWindow(TRUE);
		GetDlgItem(IDC_RADIO_USE_INPUT_DATE)->EnableWindow(TRUE);
		GetDlgItem(IDC_RADIO_USE_TRANSFER_DATE)->EnableWindow(TRUE);
		GetDlgItem(IDC_RADIO_USE_DEPOSIT_DATE)->EnableWindow(TRUE);

		OnDateRadio(FALSE);
	}
}

void CQuickbooksLink::OnBtnChooseDefaultSourceDepositAccts() 
{
	try {

		CWaitCursor pWait;

		IQBSessionManagerPtr qb = QB_OpenSession();
		if(qb == NULL) {
			AfxMessageBox("The default account settings can not be configured without a valid connection to QuickBooks.\n"
				"Please set up Quickbooks properly before using this feature.");
			return;
		}

		CString strSourceAcct, strDestAcct;
		if(QB_ChooseDepositAccounts(qb,strSourceAcct,strDestAcct,this,NULL,TRUE)) {
			//save the accounts
			SetRemotePropertyText("QBDefaultDepositSourceAcct",strSourceAcct,0,"<None>");
			SetRemotePropertyText("QBDefaultDepositDestAcct",strDestAcct,0,"<None>");
		}

		qb->EndSession();

	}NxCatchAll("Error setting up default deposit accounts.");
}

void CQuickbooksLink::OnCheckExportPatients() 
{
	if(m_checkExportPatients.GetCheck()) {
		SetRemotePropertyInt("QBDepositWithPatients",1,0,"<None>");
		GetDlgItem(IDC_BTN_CONFIGURE_DEMOG_OUTPUT)->EnableWindow(FALSE);
	}
	else {
		SetRemotePropertyInt("QBDepositWithPatients",0,0,"<None>");
		GetDlgItem(IDC_BTN_CONFIGURE_DEMOG_OUTPUT)->EnableWindow(TRUE);
	}
}

void CQuickbooksLink::OnBtnQbHelp() 
{
	MessageBox("NexTech Practice can link with two different payment features in QuickBooks:\n"
			   "'Make Deposits', and 'Receive Payments'.\n"
			   "\n"
			   "- 'Make Deposits' will generate a deposit to a bank account of your choice.\n"
			   "  This feature is only available when sending payments from the Banking tab\n"
			   "  of the Financial module. You can optionally send patient records (as customers)\n"
			   "  for each payment that is included in the deposit. You will also be prompted to\n"
			   "  choose a QuickBooks account that will be the 'source' of the payment.\n"
			   "  (Ex. 'Fees', 'Fees: Patient Fees', etc.)\n"
			   "  (Note: This feature requires QuickBooks 2003 or newer.)\n"
			   "\n"
			   "- 'Receive Payments' will generate a payment in either a bank account or\n"
			   "  other asset account of your choice, such as 'Undeposited Funds'.\n"
			   "  This feature requires a patient record to be sent over (as a customer).\n"
			   "  You are also able to set up multiple default accounts per provider, as each\n"
			   "  payment will be sent separately. This feature also allows you to send from\n"
			   "  the payment screen, in addition to the Banking tab of the Financial module.",
			   "  Practice",MB_OK|MB_ICONINFORMATION);
}

void CQuickbooksLink::OnCheckChooseIndivSourceAccts() 
{
	if(m_checkPromptForSourceAccts.GetCheck()) {
		SetRemotePropertyInt("QBPromptSourceAcct",1,0,"<None>");
	}
	else {
		SetRemotePropertyInt("QBPromptSourceAcct",0,0,"<None>");
	}
}

void CQuickbooksLink::OnBtnConfigureDemogOutput() 
{
	CQuickbooksLinkConfigDemogDlg dlg(this);
	dlg.DoModal();
}

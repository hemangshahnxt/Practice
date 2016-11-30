// CreditCardProcessingSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "CreditCardProcessingSetupDlg.h"
#include "Paymentechutils.h"
#include "PinPadUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCreditCardProcessingSetupDlg dialog

// (j.gruber 2007-08-03 13:15) - PLID 26535 - setup dialog for credit card processing
CCreditCardProcessingSetupDlg::CCreditCardProcessingSetupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CCreditCardProcessingSetupDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCreditCardProcessingSetupDlg)
	//}}AFX_DATA_INIT
}


void CCreditCardProcessingSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCreditCardProcessingSetupDlg)
	DDX_Control(pDX, IDC_SHOW_TIP_LINE, m_btnShowTips);
	DDX_Control(pDX, IDC_PIN_PAD_COMBO_PORT, m_cmbComPort);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_MERCHANT_NUMBER, m_nxeditMerchantNumber);
	DDX_Control(pDX, IDC_CLIENT_NUMBER, m_nxeditClientNumber);
	DDX_Control(pDX, IDC_TERMINAL_NUMBER, m_nxeditTerminalNumber);
	DDX_Control(pDX, IDC_CHASE_USERNAME, m_nxeditChaseUsername);
	DDX_Control(pDX, IDC_CHASE_PASSWORD, m_nxeditChasePassword);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCreditCardProcessingSetupDlg, CNxDialog)
	//{{AFX_MSG_MAP(CCreditCardProcessingSetupDlg)
	ON_EN_KILLFOCUS(IDC_MERCHANT_NUMBER, OnKillfocusMerchantNumber)
	ON_EN_KILLFOCUS(IDC_CLIENT_NUMBER, OnKillfocusClientNumber)
	ON_EN_KILLFOCUS(IDC_TERMINAL_NUMBER, OnKillfocusTerminalNumber)
	ON_BN_CLICKED(IDC_SEND_CURRENT_KEY_REQUEST, OnSendCurrentKeyRequest)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCreditCardProcessingSetupDlg message handlers

BOOL CCreditCardProcessingSetupDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		//setup the datalist
		// (a.walling 2007-11-14 14:19) - PLID 28059 - VS2008 - Bad binds; should be BindNxDataList2Ctrl.
		m_pCountryList = BindNxDataList2Ctrl(this, IDC_CC_PROCESSING_COUNTRY, GetRemoteData(), TRUE);
		m_pLanguageList = BindNxDataList2Ctrl(this, IDC_CC_PROCESSING_LANGUAGE, GetRemoteData(), TRUE);
		
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		
		//load the boxes

		//Merchant Number  - this value is stored per client, assigned by paymentech
		CString strMerchantNumber  = GetRemotePropertyText("CCProcessingMerchantNumber", "", 0, "<None>", TRUE);
		SetDlgItemText(IDC_MERCHANT_NUMBER, strMerchantNumber);

		//Client Number - this value is stored per client, assigned by paymentech
		CString strClientNumber = GetRemotePropertyText("CCProcessingClientNumber", "", 0, "<None>", TRUE);
		SetDlgItemText(IDC_CLIENT_NUMBER, strClientNumber);

		//Terminal Number - this value is per workstation, assigned by paymentech
		CString strTerminalNumber = GetPropertyText("CCProcessingTerminalNumber", "", 0, TRUE);
		SetDlgItemText(IDC_TERMINAL_NUMBER, strTerminalNumber);

		//Username - this value is stored per client, assigned by paymentech
		CString strUserName = GetRemotePropertyText("CCProcessingUsername", "", 0, "<None>", TRUE);
		SetDlgItemText(IDC_CHASE_USERNAME, strUserName);

		//Password = this value is stored per client, assigned by paymentech
		CString strPassword = GetRemotePropertyText("CCProcessingPassword", "", 0, "<None>", TRUE);
		SetDlgItemText(IDC_CHASE_PASSWORD, strPassword);

		//Country - this is our setting, mainly to know whether to ask about language or not
		long nCountryID = GetRemotePropertyInt("CCProcessingCountry", 0, 0, "<None>", TRUE);
		m_pCountryList->SetSelByColumn(0, nCountryID);

		//Language - this is our setting, to knwo how to print the mechant receipt
		long nLanguageID = GetRemotePropertyInt("CCProcessingLanguage", 0, 0, "<None>", TRUE);
		m_pLanguageList->SetSelByColumn(0, nLanguageID);

		//Pin Pad Settings
		long nPinPadOn = GetPropertyInt("CCProcessingUsingPinPad", 0, 0, TRUE);
		if (nPinPadOn == 1) {
			CheckDlgButton(IDC_PIN_PAD_RADIO_ON, 1);
			CheckDlgButton(IDC_PIN_PAD_RADIO_OFF, 0);
		}
		else {
			CheckDlgButton(IDC_PIN_PAD_RADIO_ON, 0);
			CheckDlgButton(IDC_PIN_PAD_RADIO_OFF, 1);
		}

		CString strComPort = GetPropertyText("CCProcessingPinPadComPort", "COM1", 0, TRUE);
		m_cmbComPort.SelectString(0, strComPort);

		if (nCountryID == 0) {
			m_pLanguageList->SetSelByColumn(0, (long)0);
			GetDlgItem(IDC_CC_PROCESSING_LANGUAGE)->EnableWindow(FALSE);
		}
		else {
			GetDlgItem(IDC_CC_PROCESSING_LANGUAGE)->EnableWindow(TRUE);
		}

		// (j.gruber 2007-08-07 10:28) - PLID 26997 - show tip line
		// (d.thompson 2009-07-06) - PLID 34764 - Slightly reworked this preference, default to off
		long nShowTipLine = GetRemotePropertyInt("CCProcessingShowTipLine", 0, 0, "<None>", true);
		if (nShowTipLine == 0) {
			CheckDlgButton(IDC_SHOW_TIP_LINE, 0);
		}
		else {
			CheckDlgButton(IDC_SHOW_TIP_LINE, 1);
		}

	}NxCatchAll("Error in CCreditCardProcessingSetupDlg::OnInitDialog");

	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCreditCardProcessingSetupDlg::OnKillfocusMerchantNumber() 
{
	//this number should be 12 characters always, so we will check for that
	CString strMerchantNumber;
	GetDlgItemText(IDC_MERCHANT_NUMBER, strMerchantNumber);
	if (!strMerchantNumber.IsEmpty() && strMerchantNumber.GetLength() != 12) {
		//output a warning, but don't prevent it, it will fail when they try to submit
		MessageBox("Chase Paymentech Merchant Numbers are 12 characters in length, please check the value entered.");
	}	
}

void CCreditCardProcessingSetupDlg::OnKillfocusClientNumber() 
{
	CString strClientNumber;
	GetDlgItemText(IDC_CLIENT_NUMBER, strClientNumber);
	if (!strClientNumber.IsEmpty() && strClientNumber.GetLength() != 4) {
		//output a warning, but don't prevent it, it will fail when they try to submit
		MessageBox("Chase Paymentech Client Numbers are 4 characters in length, please check the value entered.");
	}
	
}

void CCreditCardProcessingSetupDlg::OnKillfocusTerminalNumber() 
{
	CString strTerminalNumber;
	GetDlgItemText(IDC_TERMINAL_NUMBER, strTerminalNumber);
	if (! strTerminalNumber.IsEmpty() && strTerminalNumber.GetLength() != 3) {
		//output a warning, but don't prevent it, it will fail when they try to submit
		MessageBox("Chase Paymentech Terminal Numbers are 3 characters in length, please check the value entered.");
	}	
}

void CCreditCardProcessingSetupDlg::OnOK() 
{
	try { 
		//save all the settings
		CString strTerminalNumber, strMerchantNumber, strClientNumber, strUserName, strPassword;

		GetDlgItemText(IDC_MERCHANT_NUMBER, strMerchantNumber);
		SetRemotePropertyText("CCProcessingMerchantNumber", strMerchantNumber, 0, "<None>");

		GetDlgItemText(IDC_CLIENT_NUMBER, strClientNumber);
		SetRemotePropertyText("CCProcessingClientNumber", strClientNumber, 0, "<None>");

		GetDlgItemText(IDC_TERMINAL_NUMBER, strTerminalNumber);
		SetPropertyText("CCProcessingTerminalNumber", strTerminalNumber, 0);

		//username
		GetDlgItemText(IDC_CHASE_USERNAME, strUserName);
		SetRemotePropertyText("CCProcessingUsername", strUserName, 0, "<None>");

		//password 
		GetDlgItemText(IDC_CHASE_PASSWORD, strPassword);
		SetRemotePropertyText("CCProcessingPassword", strPassword, 0, "<None>");

		//Country 
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pCountryList->CurSel;
		if (pRow) {
			long nCountryID = VarLong(pRow->GetValue(0));
			SetRemotePropertyInt("CCProcessingCountry", nCountryID, 0, "<None>");
		}

		//Language - this is our setting, to knwo how to print the mechant receipt
		pRow = m_pLanguageList->CurSel;
		if (pRow) {
			long nLanguageID = VarLong(pRow->GetValue(0));
			SetRemotePropertyInt("CCProcessingLanguage",nLanguageID, 0, "<None>");
		}
		
		//Pin Pad Settings
		if (IsDlgButtonChecked(IDC_PIN_PAD_RADIO_ON)) {
			SetPropertyInt("CCProcessingUsingPinPad", 1, 0);
		}
		else {
			SetPropertyInt("CCProcessingUsingPinPad", 0, 0);
		}

		
		CString strComPort;
		m_cmbComPort.GetWindowText(strComPort);
		SetPropertyText("CCProcessingPinPadComPort", strComPort, 0);

		// (j.gruber 2007-08-07 10:28) - PLID 26997 - show tip line
		if (IsDlgButtonChecked(IDC_SHOW_TIP_LINE)) {
			SetPropertyInt("CCProcessingShowTipLine", 1, 0);
		}
		else {
			SetPropertyInt("CCProcessingShowTipLine", 0, 0);
		}


		CDialog::OnOK();

	}NxCatchAll("Error In CCreditCardProcessingSetupDlg::OnOK");
	
	
}

void CCreditCardProcessingSetupDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

BEGIN_EVENTSINK_MAP(CCreditCardProcessingSetupDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CCreditCardProcessingSetupDlg)
	ON_EVENT(CCreditCardProcessingSetupDlg, IDC_CC_PROCESSING_COUNTRY, 16 /* SelChosen */, OnSelChosenCcProcessingCountry, VTS_DISPATCH)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

//Interac and pinpads got moved to 8500
void CCreditCardProcessingSetupDlg::OnSelChosenCcProcessingCountry(LPDISPATCH lpRow) 
{
	try  {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if (pRow) {

			if (VarLong(pRow->GetValue(0)) == 0) {
				m_pLanguageList->SetSelByColumn(0, (long)0);
				GetDlgItem(IDC_CC_PROCESSING_LANGUAGE)->EnableWindow(FALSE);
				
			}
			else {
				GetDlgItem(IDC_CC_PROCESSING_LANGUAGE)->EnableWindow(TRUE);
			}
		}
	}NxCatchAll("Error in  CCreditCardProcessingSetupDlg::OnSelChosenCcProcessingCountry");


	
}

//Interac and pinpads got moved to 8500
void CCreditCardProcessingSetupDlg::OnSendCurrentKeyRequest() 
{
	try {
		if (IDYES == MsgBox(MB_YESNO, "This has the potential to reset the encryption keys to your pin pad device.\n"
			"This should only be done as requested from NexTech or Chase Paymentech technical support.\n"
			"Do you want to cancel?")) {

			//woo hoo!!  they cancelled
			return;
		}

		CWaitCursor wait;

		//they want to go through with it
		PaymentechUtils::SendCurrentKeyRequest();

		//close the pin pad
		PinPadUtils::PinPadClose();

		//sleep for 15 seconds, it should be restarted by then
		Sleep(15000);						

		

		//the pin pad will restart itself here, so let's initialize it again
		PostMessage(NXM_INITIALIZE_PINPAD);
		
	}NxCatchAll("Error in CCreditCardProcessingSetupDlg::OnSendCurrentKeyRequest()");
}

// BarcodeSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "BarcodeSetupDlg.h"
#include "barcode.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBarcodeSetupDlg dialog


CBarcodeSetupDlg::CBarcodeSetupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CBarcodeSetupDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CBarcodeSetupDlg)
	m_strPort = _T("");
	m_strTest = _T("");
	m_radioToggle = -1;
	//}}AFX_DATA_INIT

	//(a.wilson 2012-1-12) PLID 47486 - defaults
	m_pOPOSBarcodeScannerThread = NULL;
	m_bUseOPOSBarcodeScanner = false;
	m_bOPOSBarcodeScannerInitComplete = false;
}


void CBarcodeSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBarcodeSetupDlg)
	DDX_Control(pDX, IDC_RADIO_OFF, m_btnOff);
	DDX_Control(pDX, IDC_RADIO_ON, m_btnOn);
	DDX_CBString(pDX, IDC_COMBO_PORT, m_strPort);
	DDX_CBString(pDX, IDC_COMBO_PARITY, m_strParity);
	DDX_CBString(pDX, IDC_COMBO_STOPBITS, m_strStopBits);
	DDX_Text(pDX, IDC_EDIT_TEST, m_strTest);
	DDX_Radio(pDX, IDC_RADIO_OFF, m_radioToggle);
	DDX_Control(pDX, IDC_EDIT_TEST, m_nxeditEditTest);
	DDX_Control(pDX, IDOK, m_btnClose);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBarcodeSetupDlg, CNxDialog)
	//{{AFX_MSG_MAP(CBarcodeSetupDlg)
	ON_BN_CLICKED(IDC_RADIO_OFF, OnRadioOff)
	ON_BN_CLICKED(IDC_RADIO_ON, OnRadioOn)
	ON_CBN_SELENDOK(IDC_COMBO_STOPBITS, OnSelendokComboStopbits)
	ON_CBN_SELENDOK(IDC_COMBO_PARITY, OnSelendokComboParity)
	ON_CBN_SELENDOK(IDC_COMBO_PORT, OnSelendokComboPort)
	ON_MESSAGE(WM_ENQUEUE_BARCODE, OnBarcodeScan)
	ON_MESSAGE(WM_BARCODE_SCAN, OnBarcodeScan)
	ON_MESSAGE(NXM_BARSCAN_DATA_EVENT, OnBarcodeScannerDataEvent)
	ON_MESSAGE(NXM_BARSCAN_INIT_COMPLETE, OnBarcodeScannerInitComplete)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_OPOS_OFF, &CBarcodeSetupDlg::OnBnClickedOposOff)
	ON_BN_CLICKED(IDC_OPOS_ON, &CBarcodeSetupDlg::OnBnClickedOposOn)
	ON_BN_CLICKED(IDC_OPOS_TEST, &CBarcodeSetupDlg::OnBnClickedOposTest)
	ON_EN_KILLFOCUS(IDC_OPOS_NAME, &CBarcodeSetupDlg::OnEnKillfocusOposName)
	ON_EN_KILLFOCUS(IDC_OPOS_TIMEOUT, &CBarcodeSetupDlg::OnEnKillfocusOposTimeout)
	ON_WM_TIMER()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBarcodeSetupDlg message handlers

BOOL CBarcodeSetupDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		m_btnClose.AutoSet(NXB_CLOSE);

		// (j.jones 2013-05-15 10:25) - PLID 25479 - added a cache, although these ought
		// to already be cached in mainframe
		// (r.gonet 2016-05-19 18:13) - NX-100689 - Get the system name from the property 
		//manager rather than the license object.
		g_propManager.BulkCache("CBarcodeSetupDlg", propbitNumber | propbitText,
			"(Username = '<None>' OR Username = '%s' OR Username = '%s') AND ("
			"Name = 'UseBarcodeScanner' "				//obsolete
			"OR Name = 'UseOposBarcodeScanner' "		//obsolete
			"OR Name = 'OposBarcodeScannerTimeout' "	//obsolete
			"OR Name = 'BarcodeScannerPort' "			//obsolete
			"OR Name = 'BarcodeScannerParity' "			//obsolete
			"OR Name = 'BarcodeScannerStopBits' "		//obsolete
			"OR Name = 'OposBarcodeScannerName' "		//obsolete
			"OR Name = 'UseBarcodeScanner_UserWS' "
			"OR Name = 'BarcodeScannerPort_UserWS' "
			"OR Name = 'BarcodeScannerParity_UserWS' "
			"OR Name = 'BarcodeScannerStopBits_UserWS' "
			"OR Name = 'UseOposBarcodeScanner_UserWS' "
			"OR Name = 'OposBarcodeScannerTimeout_UserWS' "
			"OR Name = 'OposBarcodeScannerName_UserWS' "
			")",
			_Q(GetCurrentUserComputerName()),
			_Q(g_propManager.GetSystemName() + '.' + GetPracPath(PracPath::ConfigRT)));
		
		m_strTest.Empty();

		// (j.jones 2013-05-15 13:42) - PLID 25479 - converted these settings to per user, per workstation,
		// pulling the defaults from their old per-workstation value
		m_strPort = GetRemotePropertyText("BarcodeScannerPort_UserWS", GetPropertyText("BarcodeScannerPort", "COM1:"), 0, GetCurrentUserComputerName(), true);
		m_strParity = GetRemotePropertyText("BarcodeScannerParity_UserWS", GetPropertyText("BarcodeScannerParity", "None"), 0, GetCurrentUserComputerName(), true);
		m_strStopBits = GetRemotePropertyText("BarcodeScannerStopBits_UserWS", GetPropertyText("BarcodeScannerStopBits", "1"), 0, GetCurrentUserComputerName(), true);
		bool bUseBarcodeScanner = (GetRemotePropertyInt("UseBarcodeScanner_UserWS", GetPropertyInt("UseBarcodeScanner", 0), 0, GetCurrentUserComputerName(), true) == 1);
		if (bUseBarcodeScanner)
			m_radioToggle = 1; // On
		else
			m_radioToggle = 0; // Off

		UpdateData(FALSE);

		GetDlgItem(IDC_COMBO_PORT)->EnableWindow(m_radioToggle);
		GetDlgItem(IDC_COMBO_PARITY)->EnableWindow(m_radioToggle);
		GetDlgItem(IDC_COMBO_STOPBITS)->EnableWindow(m_radioToggle);

		//(a.wilson 2012-1-12) PLID 47486 - init code for opos barcode scanner
		// (j.jones 2013-05-15 13:54) - PLID 25479 - converted these settings to per user, per workstation,
		// pulling the defaults from their old per-workstation value
		if (GetRemotePropertyInt("UseOposBarcodeScanner_UserWS", GetPropertyInt("UseOposBarcodeScanner", 0, 0), 0, GetCurrentUserComputerName(), true) == TRUE) {
			m_bUseOPOSBarcodeScanner = true;
			GetDlgItem(IDC_OPOS_NAME)->EnableWindow(TRUE);
			GetDlgItem(IDC_OPOS_TEST)->EnableWindow(TRUE);
			GetDlgItem(IDC_OPOS_TIMEOUT)->EnableWindow(TRUE);
			CheckDlgButton(IDC_OPOS_ON, TRUE);
			CheckDlgButton(IDC_OPOS_OFF, FALSE);
		} else {
			m_bUseOPOSBarcodeScanner = false;
			GetDlgItem(IDC_OPOS_NAME)->EnableWindow(FALSE);
			GetDlgItem(IDC_OPOS_TEST)->EnableWindow(FALSE);
			GetDlgItem(IDC_OPOS_TIMEOUT)->EnableWindow(FALSE);
			CheckDlgButton(IDC_OPOS_ON, FALSE);
			CheckDlgButton(IDC_OPOS_OFF, TRUE);
		}
		SetDlgItemText(IDC_OPOS_NAME, GetRemotePropertyText("OposBarcodeScannerName_UserWS", GetPropertyText("OposBarcodeScannerName", "", 0), 0, GetCurrentUserComputerName(), true));
		SetDlgItemInt(IDC_OPOS_TIMEOUT, GetRemotePropertyInt("OposBarcodeScannerTimeout_UserWS", GetPropertyInt("OposBarcodeScannerTimeout", 10, 0), 0, GetCurrentUserComputerName(), true));

	} NxCatchAll(__FUNCTION__);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CBarcodeSetupDlg::OnOK() 
{
	try {
		ApplySettings();
		//(a.wilson 2012-1-12) PLID 47486 - make sure the thread is killed.
		if (m_pOPOSBarcodeScannerThread) {
			m_pOPOSBarcodeScannerThread->Shutdown();
			m_pOPOSBarcodeScannerThread = NULL;
		}
		if (m_bUseOPOSBarcodeScanner) {	//if the scanner is set to use then make sure the mainframe builds a new one.
			GetMainFrame()->PostMessage(NXM_INITIATE_OPOSBARCODESCANNER);
		}

	} NxCatchAll(__FUNCTION__);

	CDialog::OnOK();
}

void CBarcodeSetupDlg::ApplySettings()
{
	try {

		UpdateData(TRUE);

		BeginWaitCursor();
		Barcode_Close();

		// (j.jones 2013-05-15 14:12) - PLID 25479 - converted these settings to per user, per workstation
		SetRemotePropertyText("BarcodeScannerPort_UserWS", m_strPort, 0, GetCurrentUserComputerName());
		SetRemotePropertyText("BarcodeScannerParity_UserWS", m_strParity, 0, GetCurrentUserComputerName());
		SetRemotePropertyText("BarcodeScannerStopBits_UserWS", m_strStopBits, 0, GetCurrentUserComputerName());
		SetRemotePropertyInt("UseBarcodeScanner_UserWS", m_radioToggle, 0, GetCurrentUserComputerName());

		GetDlgItem(IDC_COMBO_PORT)->EnableWindow(m_radioToggle);
		GetDlgItem(IDC_COMBO_PARITY)->EnableWindow(m_radioToggle);
		GetDlgItem(IDC_COMBO_STOPBITS)->EnableWindow(m_radioToggle);

		if (m_radioToggle == 1) // On
			Barcode_Open(((CMainFrame *)AfxGetMainWnd())->GetSafeHwnd(), m_strPort);

		EndWaitCursor();

		m_strTest.Empty();
		UpdateData(FALSE);

	}NxCatchAll(__FUNCTION__);
}

LRESULT CBarcodeSetupDlg::OnBarcodeScan(WPARAM wParam, LPARAM lParam)
{
	// (a.walling 2007-11-08 16:28) - PLID 27476 - Need to convert this correctly from a bstr
	_bstr_t bstr = (BSTR)lParam;
	m_strTest = (LPCTSTR)bstr;
	UpdateData(FALSE);
	return 0;
}

void CBarcodeSetupDlg::OnRadioOff() 
{
	ApplySettings();
}

void CBarcodeSetupDlg::OnRadioOn() 
{
	ApplySettings();
}

void CBarcodeSetupDlg::OnSelendokComboStopbits() 
{
	ApplySettings();
}

void CBarcodeSetupDlg::OnSelendokComboParity() 
{
	ApplySettings();
}

void CBarcodeSetupDlg::OnSelendokComboPort() 
{
	ApplySettings();
}
//(a.wilson 2012-1-11) PLID 47486 - if an event occurs handle it for the opos barcode scanner.
LRESULT CBarcodeSetupDlg::OnBarcodeScannerDataEvent(WPARAM wParam, LPARAM lParam)
{
	if ((long)wParam == OPOS_SUCCESS) {
		// (a.walling 2012-01-17 12:55) - PLID 47120 - lParam is a BSTR which we now own
		_bstr_t bstrData((BSTR)lParam, false);
		SetDlgItemText(IDC_EDIT_TEST, bstrData);
	}
	return 0;
}
//(a.wilson 2012-1-11) PLID 47486 - update member variable when radio clicked
void CBarcodeSetupDlg::OnBnClickedOposOff()
{
	try {
		m_bUseOPOSBarcodeScanner = false;
		CheckOPOSStatus();
	} NxCatchAll(__FUNCTION__);
}
//(a.wilson 2012-1-11) PLID 47486 - update member variable when radio clicked
void CBarcodeSetupDlg::OnBnClickedOposOn()
{
	try {
		m_bUseOPOSBarcodeScanner = true;
		CheckOPOSStatus();
	} NxCatchAll(__FUNCTION__);
}
//(a.wilson 2012-1-11) PLID 47486 - handle the click of the test connection button
void CBarcodeSetupDlg::OnBnClickedOposTest()
{
	try {
		SetDlgItemText(IDC_EDIT_TEST, "Connecting...");
		//make sure that none of the threads are active, if so kill them.
		if (m_pOPOSBarcodeScannerThread) {
			m_pOPOSBarcodeScannerThread->Shutdown();
			m_pOPOSBarcodeScannerThread = NULL;
		}
		//once all threads are sure to be dead, we can start a new one.
		if (m_bUseOPOSBarcodeScanner && m_pOPOSBarcodeScannerThread == NULL) {
			// (j.jones 2013-05-15 13:54) - PLID 25479 - converted these settings to per user, per workstation,
			// pulling the defaults from their old per-workstation value
			long nTimeout = GetRemotePropertyInt("OposBarcodeScannerTimeout_UserWS", GetPropertyInt("OposBarcodeScannerTimeout", 10, 0), 0, GetCurrentUserComputerName(), true) * 1000;
			m_pOPOSBarcodeScannerThread = new COPOSBarcodeScannerThread(GetSafeHwnd(), 
				GetRemotePropertyText("OposBarcodeScannerName_UserWS", GetPropertyText("OposBarcodeScannerName", "", 0), 0, GetCurrentUserComputerName(), true),
				nTimeout);
			m_pOPOSBarcodeScannerThread->CreateThread();
			SetTimer(IDT_OPOS_BARSCAN_INIT_TIMER, nTimeout, NULL);
		}
	} NxCatchAll(__FUNCTION__);
}
//(a.wilson 2012-1-11) PLID 47486 - save the name when we lose focus
void CBarcodeSetupDlg::OnEnKillfocusOposName()
{
	try {
		CString strName;
		GetDlgItemText(IDC_OPOS_NAME, strName);
		// (j.jones 2013-05-15 14:12) - PLID 25479 - converted this setting to per user, per workstation
		SetRemotePropertyText("OposBarcodeScannerName_UserWS", strName, 0, GetCurrentUserComputerName());
	} NxCatchAll(__FUNCTION__);
}
//(a.wilson 2012-1-11) PLID 47486 - save the timeout when we lose focus
void CBarcodeSetupDlg::OnEnKillfocusOposTimeout()
{
	try {
		CString strTimeout;
		long nTimeout;
		GetDlgItemText(IDC_OPOS_TIMEOUT, strTimeout);
		nTimeout = atol(strTimeout);

		if (nTimeout < 100 && nTimeout > 0) {
			// (j.jones 2013-05-15 14:12) - PLID 25479 - converted this setting to per user, per workstation
			SetRemotePropertyInt("OposBarcodeScannerTimeout_UserWS", nTimeout, 0, GetCurrentUserComputerName());
		} else {
			SetDlgItemText(IDC_OPOS_TIMEOUT, "10");
			SetDlgItemText(IDC_EDIT_TEST, "Invalid timeout.\r\n Please enter a time around 10 seconds...");
		}
	} NxCatchAll(__FUNCTION__);
}
//(a.wilson 2012-1-11) PLID 47486 - ensures the status of the dialog reflects the off on status
void CBarcodeSetupDlg::CheckOPOSStatus()
{
	long nUseOPOSBarcodeScanner;
	if (m_bUseOPOSBarcodeScanner) {
		nUseOPOSBarcodeScanner = 1;
	} else {
		nUseOPOSBarcodeScanner = 0;
	}
	// (j.jones 2013-05-15 14:11) - PLID 25479 - converted this setting to per user, per workstation
	SetRemotePropertyInt("UseOposBarcodeScanner_UserWS", nUseOPOSBarcodeScanner, 0, GetCurrentUserComputerName());

	if (m_bUseOPOSBarcodeScanner) {
		GetDlgItem(IDC_OPOS_NAME)->EnableWindow(TRUE);
		GetDlgItem(IDC_OPOS_TEST)->EnableWindow(TRUE);
		GetDlgItem(IDC_OPOS_TIMEOUT)->EnableWindow(TRUE);
	} else {
		GetDlgItem(IDC_OPOS_NAME)->EnableWindow(FALSE);
		GetDlgItem(IDC_OPOS_TEST)->EnableWindow(FALSE);
		GetDlgItem(IDC_OPOS_TIMEOUT)->EnableWindow(FALSE);
	}
}
//(a.wilson 2012-1-11) PLID 47486
LRESULT CBarcodeSetupDlg::OnBarcodeScannerInitComplete(WPARAM wParam, LPARAM lParam)
{
	try {
		if ((long)wParam == OPOS_SUCCESS) {
			KillTimer(IDT_OPOS_BARSCAN_INIT_TIMER);
			CString strTestMessage;
			strTestMessage = "Success!\r\n\r\nReady for Test Scan...";
			SetDlgItemText(IDC_EDIT_TEST, strTestMessage);
			m_bOPOSBarcodeScannerInitComplete = true;
		} else {
			KillTimer(IDT_OPOS_BARSCAN_INIT_TIMER);
			CString strError = "";
			strError = FormatString("\r\n\r\nError: %s", OPOS::GetMessage((long)wParam));
			MessageBox(FormatString("The OPOS barcode scanner device could not be initialized.  Make sure that the settings are correct "
				   ".  Ensure that the device has been "
				   "correctly installed and that nothing else is currently using the barcode scanner device.%s", strError), 
				   "Barcode Scanner Device Error", MB_ICONWARNING|MB_OK);
			SetDlgItemText(IDC_EDIT_TEST, "");
			if (m_pOPOSBarcodeScannerThread) {
				m_pOPOSBarcodeScannerThread->Shutdown();
				m_pOPOSBarcodeScannerThread = NULL;
			}
		}
	} NxCatchAll(__FUNCTION__);
	return TRUE;
}
//(a.wilson 2012-1-11) PLID 47486
void CBarcodeSetupDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == IDT_OPOS_BARSCAN_INIT_TIMER) {
		KillTimer(IDT_OPOS_BARSCAN_INIT_TIMER);
		MessageBox("Timed out while initializing the OPOS Barcode Scanner Device. "
			"Make sure that the settings "
			"are correct and ensure that the device has been correctly installed.", 
			"OPOS Barcode Scanner Device Error", 
			MB_ICONWARNING | MB_OK);
		SetDlgItemText(IDC_EDIT_TEST, "");
		//close the thread
		if (m_pOPOSBarcodeScannerThread) {
			m_pOPOSBarcodeScannerThread->Shutdown();
			m_pOPOSBarcodeScannerThread = NULL;
		}
	}
	CNxDialog::OnTimer(nIDEvent);
}

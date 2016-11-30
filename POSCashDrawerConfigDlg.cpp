// POSCashDrawerConfigDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practiceRc.h"
#include "POSCashDrawerConfigDlg.h"
#include "AuditTrail.h"

// (a.walling 2007-05-17 16:01) - 26058 - Make a configuration dialog for the cash drawer 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37025 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



/////////////////////////////////////////////////////////////////////////////
// POSCashDrawerConfigDlg dialog


POSCashDrawerConfigDlg::POSCashDrawerConfigDlg(COPOSCashDrawerDevice* pPOSCashDrawerDevice, CWnd* pParent /*=NULL*/)
	: CNxDialog(POSCashDrawerConfigDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(POSCashDrawerConfigDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_pPOSCashDrawerDevice = pPOSCashDrawerDevice;
	m_bManualOpen = FALSE; // (a.walling 2007-09-21 09:16) - PLID 27468
}


void POSCashDrawerConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(POSCashDrawerConfigDlg)
	DDX_Control(pDX, IDC_POS_CASHDRAWER_ON, m_btnOn);
	DDX_Control(pDX, IDC_POS_CASHDRAWER_OFF, m_btnOff);
	DDX_Control(pDX, IDC_OPEN_FOR_TIPS, m_btnOpenTips);
	DDX_Control(pDX, IDC_OPEN_FOR_REFUNDS, m_btnOpenRefunds);
	DDX_Control(pDX, IDC_OPEN_FOR_CHECKS, m_btnOpenChecks);
	DDX_Control(pDX, IDC_OPEN_FOR_CHARGE, m_btnOpenCharge);
	DDX_Control(pDX, IDOK, m_btnClose);
	DDX_Control(pDX, IDC_EDIT_CASHDRAWER_NAME, m_nxeditEditCashdrawerName);
	DDX_Control(pDX, IDC_CASHDRAWER_STATUS, m_nxstaticCashdrawerStatus);
	DDX_Control(pDX, IDC_POS_CASHDRAWER_ADMIN_OPTIONS, m_btnPosCashdrawerAdminOptions);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(POSCashDrawerConfigDlg, CNxDialog)
	//{{AFX_MSG_MAP(POSCashDrawerConfigDlg)
	ON_BN_CLICKED(IDC_POS_CASHDRAWER_ON, OnPosCashdrawerOn)
	ON_BN_CLICKED(IDC_POS_CASHDRAWER_OFF, OnPosCashdrawerOff)
	ON_BN_CLICKED(IDC_CASHDRAWER_TEST, OnCashdrawerTest)
	ON_EN_CHANGE(IDC_EDIT_CASHDRAWER_NAME, OnChangeEditCashdrawerName)
	ON_BN_CLICKED(IDC_OPEN_FOR_CHECKS, OnOpenForChecks)
	ON_BN_CLICKED(IDC_OPEN_FOR_REFUNDS, OnOpenForRefunds)
	ON_BN_CLICKED(IDC_OPEN_FOR_TIPS, OnOpenForTips)
	ON_BN_CLICKED(IDC_OPEN_CASH_DRAWER, OnOpenCashDrawer)
	ON_BN_CLICKED(IDC_OPEN_FOR_CHARGE, OnOpenForCharge)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// POSCashDrawerConfigDlg message handlers

BOOL POSCashDrawerConfigDlg::OnInitDialog() 
{
	// (a.walling 2007-10-01 17:43) - PLID 26058 - Add exception handling
	try {
		CNxDialog::OnInitDialog();

		m_btnClose.AutoSet(NXB_CLOSE);
		
		m_strCashDrawerName = GetPropertyText("POSCashDrawer_DeviceName", "", 0, TRUE);
		m_bUseCashDrawer = GetPropertyInt("POSCashDrawer_UseDevice", 0, 0, TRUE);

		SetDlgItemText(IDC_EDIT_CASHDRAWER_NAME, m_strCashDrawerName);

		// (a.walling 2007-09-20 12:31) - PLID 9801 - Preference to open cash drawer for refunds
		// (a.walling 2007-09-21 15:18) - PLID 27468 - Pref for opening cash drawer for charges
		// (a.walling 2007-10-04 14:43) - PLID 26058 - Use GetProperty for workstation properties
		CheckDlgButton(IDC_OPEN_FOR_REFUNDS, GetPropertyInt("POSCashDrawer_OpenDrawerForRefunds", FALSE, 0, true));
		CheckDlgButton(IDC_OPEN_FOR_CHECKS, GetPropertyInt("POSCashDrawer_OpenDrawerForChecks", FALSE, 0,  true));
		CheckDlgButton(IDC_OPEN_FOR_TIPS, GetPropertyInt("POSCashDrawer_OpenDrawerForTips", TRUE, 0, true)); // default to true
		CheckDlgButton(IDC_OPEN_FOR_CHARGE, GetPropertyInt("POSCashDrawer_OpenDrawerForCharges", FALSE, 0, true));

		// (a.walling 2007-09-21 09:16) - PLID 27468 - Check the permission and save to a member variable. We will prompt for password when necessary.
		m_bManualOpen = CheckCurrentUserPermissions(bioCashDrawers, sptDynamic0, FALSE, 0, TRUE, TRUE) || IsCurrentUserAdministrator();

		if (IsCurrentUserAdministrator()) {
			GetDlgItem(IDC_POS_CASHDRAWER_ADMIN_OPTIONS)->EnableWindow(TRUE);

			GetDlgItem(IDC_OPEN_FOR_CHECKS)->EnableWindow(TRUE);
			GetDlgItem(IDC_OPEN_FOR_REFUNDS)->EnableWindow(TRUE);
			GetDlgItem(IDC_OPEN_FOR_CHARGE)->EnableWindow(TRUE);
			// (a.walling 2007-09-20 14:32) - PLID 27468
			GetDlgItem(IDC_OPEN_FOR_TIPS)->EnableWindow(TRUE);
			GetDlgItem(IDC_OPEN_CASH_DRAWER)->EnableWindow(TRUE);
		} else {
			GetDlgItem(IDC_POS_CASHDRAWER_ADMIN_OPTIONS)->EnableWindow(FALSE);

			GetDlgItem(IDC_OPEN_FOR_CHECKS)->EnableWindow(FALSE);
			GetDlgItem(IDC_OPEN_FOR_REFUNDS)->EnableWindow(FALSE);
			GetDlgItem(IDC_OPEN_FOR_CHARGE)->EnableWindow(FALSE);
			// (a.walling 2007-09-20 14:32) - PLID 27468
			GetDlgItem(IDC_OPEN_FOR_TIPS)->EnableWindow(FALSE);
			GetDlgItem(IDC_OPEN_CASH_DRAWER)->EnableWindow(FALSE);
		}

		if (m_bUseCashDrawer) {
			OnPosCashdrawerOn();

			if (m_pPOSCashDrawerDevice) {
				SetDlgItemText(IDC_CASHDRAWER_STATUS, "Cash drawer is enabled and ready.");
				// (a.walling 2007-09-20 14:38) - PLID 27468
				GetDlgItem(IDC_OPEN_CASH_DRAWER)->EnableWindow(m_bManualOpen);
			} else {
				SetDlgItemText(IDC_CASHDRAWER_STATUS, "Cannot connect to cash drawer!");
				// (a.walling 2007-09-20 14:39) - PLID 27468
				GetDlgItem(IDC_OPEN_CASH_DRAWER)->EnableWindow(FALSE);
			}
		}
		else {
			OnPosCashdrawerOff();

			SetDlgItemText(IDC_CASHDRAWER_STATUS, "Cash drawer is disabled.");
			// (a.walling 2007-09-20 14:39) - PLID 27468
			GetDlgItem(IDC_OPEN_CASH_DRAWER)->EnableWindow(FALSE);
		}

		m_bNeedToTest = (m_pPOSCashDrawerDevice == NULL);
	} NxCatchAll("Error in POSCashDrawerConfigDlg::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void POSCashDrawerConfigDlg::OnPosCashdrawerOn() 
{
	// (a.walling 2007-10-01 17:46) - PLID 26058 - Added exception handling
	try {
		CheckDlgButton(IDC_POS_CASHDRAWER_ON, TRUE);
		CheckDlgButton(IDC_POS_CASHDRAWER_OFF, FALSE);
		GetDlgItem(IDC_CASHDRAWER_TEST)->EnableWindow(TRUE);
		// (a.walling 2007-09-20 14:47) - PLID 27468 - Only enable if we do not need to test and they are either an admin or the manual open permission is set
		GetDlgItem(IDC_OPEN_CASH_DRAWER)->EnableWindow(!m_bNeedToTest && (m_bManualOpen));

		SetDlgItemText(IDC_CASHDRAWER_STATUS, "Cash drawer is enabled.");
		m_bUseCashDrawer = TRUE;
	} NxCatchAll("Error in POSCashDrawerConfigDlg::OnPosCashdrawerOn");
}

void POSCashDrawerConfigDlg::OnPosCashdrawerOff() 
{
	// (a.walling 2007-10-01 17:46) - PLID 26058 - Added exception handling
	try {
		CheckDlgButton(IDC_POS_CASHDRAWER_ON, FALSE);
		CheckDlgButton(IDC_POS_CASHDRAWER_OFF, TRUE);
		GetDlgItem(IDC_CASHDRAWER_TEST)->EnableWindow(FALSE);
		// (a.walling 2007-09-20 14:46) - PLID 27468
		GetDlgItem(IDC_OPEN_CASH_DRAWER)->EnableWindow(FALSE);

		SetDlgItemText(IDC_CASHDRAWER_STATUS, "Cash drawer is disabled.");
		m_bUseCashDrawer = FALSE;
	} NxCatchAll("Error in POSCashDrawerConfigDlg::OnPosCashdrawerOff");
}

void POSCashDrawerConfigDlg::OnCashdrawerTest() 
{
	// (a.walling 2007-10-01 17:44) - PLID 26058 - Added exception handling
	try {
		if (Apply())
			Test(TRUE);
	} NxCatchAll("POSCashDrawerConfigDlg::OnCashdrawerTest");
}

void POSCashDrawerConfigDlg::OnOK() 
{
	// (a.walling 2007-10-01 17:44) - PLID 26058 - Added exception handling
	try {
		if (Apply()) {
			if (m_bUseCashDrawer && m_bNeedToTest) {
				if (Test(FALSE)) {
					CDialog::OnOK();
				} else {
					MessageBox("Connection to the cash drawer failed. The device will be disabled.");
					SetPropertyInt("POSCashDrawer_UseDevice", FALSE);
					CDialog::OnOK();
				}
			} else {
				CDialog::OnOK();
			}
		}	
	} NxCatchAll("POSCashDrawerConfigDlg::OnOK");
}

BOOL POSCashDrawerConfigDlg::Apply()
{
	m_bUseCashDrawer = IsDlgButtonChecked(IDC_POS_CASHDRAWER_ON);

	CString strName;
	GetDlgItemText(IDC_EDIT_CASHDRAWER_NAME, strName);

	if (m_bUseCashDrawer && strName.IsEmpty()) {
		MessageBox("The cash drawer cannot have a blank name unless it is disabled.", NULL, MB_OK | MB_ICONHAND);
		return FALSE;
	}
		
	m_strCashDrawerName = strName;

	SetPropertyText("POSCashDrawer_DeviceName", m_strCashDrawerName);
	SetPropertyInt("POSCashDrawer_UseDevice", m_bUseCashDrawer);

	// (a.walling 2007-06-18 09:16) - PLID 26058
	if (!m_bUseCashDrawer && m_pPOSCashDrawerDevice != NULL) {
		// cash drawer is disabled, so destroy our object.
		m_pPOSCashDrawerDevice->ClosePOSCashDrawer();
		m_pPOSCashDrawerDevice->DestroyWindow();
		delete m_pPOSCashDrawerDevice;
		m_pPOSCashDrawerDevice = NULL;
	}

	return TRUE;
}

BOOL POSCashDrawerConfigDlg::Test(BOOL bPromptToOpen)
{
	try {
		try {
			if (m_pPOSCashDrawerDevice) {
				m_pPOSCashDrawerDevice->ClosePOSCashDrawer();
				m_pPOSCashDrawerDevice->DestroyWindow();
				delete m_pPOSCashDrawerDevice;
			}
		} NxCatchAll("Error destroying existing cash drawer connection.");
		m_pPOSCashDrawerDevice = NULL;

		SetDlgItemText(IDC_CASHDRAWER_STATUS, "Connecting...");
		GetDlgItem(IDC_CASHDRAWER_STATUS)->RedrawWindow();

		// (a.walling 2007-08-14 09:30) - PLID 26058 - Need to use the mainframe as the parent,
		// not this window, which will soon be destroyed!
		m_pPOSCashDrawerDevice = new COPOSCashDrawerDevice(GetMainFrame());

		// (a.walling 2007-10-04 14:54) - PLID 26058 - Throw an exception if we fail to create it.
		if (m_pPOSCashDrawerDevice == NULL) {
			ThrowNxException("Failed to create cash drawer device object!");
		}

		if (!m_pPOSCashDrawerDevice->InitiatePOSCashDrawerDevice(m_strCashDrawerName)) {
			// Delete the pointer to the device because it has not been initiated
			SetDlgItemText(IDC_CASHDRAWER_STATUS, "Cannot connect to cash drawer!");

			try {
				m_pPOSCashDrawerDevice->DestroyWindow();
				delete m_pPOSCashDrawerDevice;
			} NxCatchAll("Error destroying half-open cash drawer connection.");
			m_pPOSCashDrawerDevice = NULL;

			// (a.walling 2007-10-09 10:25) - PLID 26058 - Turn off the cash drawer if we could not initiate
			OnPosCashdrawerOff();

			return FALSE;
		} else {
			SetDlgItemText(IDC_CASHDRAWER_STATUS, "Cash drawer is enabled and ready.");
			m_bNeedToTest = FALSE;

			// (a.walling 2007-09-21 09:34) - PLID 27468
			// only administrators should be able to open the drawer like this, or users with the manual open preference
			if (bPromptToOpen && (IsCurrentUserAdministrator() || m_bManualOpen)) {
				if (IDYES == MessageBox("Connection was successful! Would you like to attempt to open the drawer?", NULL, MB_YESNO | MB_ICONQUESTION)) {
					CString strLastState = m_pPOSCashDrawerDevice->GetDrawerLastStateString();
					// (a.walling 2007-09-28 09:08) - PLID 26019 - Only audit if successfully opened
					long nResult = m_pPOSCashDrawerDevice->OpenDrawer();
					if (nResult == OPOS_SUCCESS) {
						AuditEvent(-1, "", BeginNewAuditEvent(), aeiPOSCashDrawerOpen, -1, strLastState, "Drawer Opened (Testing Configuration)", aepHigh, aetChanged);
					}
				}
			}

			// (a.walling 2007-09-20 14:41) - PLID 27468
			GetDlgItem(IDC_OPEN_CASH_DRAWER)->EnableWindow(m_bManualOpen);

			return TRUE;
		}
	} NxCatchAll("Error testing cash drawer connection.");

	// (a.walling 2007-10-09 10:25) - PLID 26058 - Turn off the cash drawer if we had an exception
	OnPosCashdrawerOff();

	return FALSE;
}

void POSCashDrawerConfigDlg::OnChangeEditCashdrawerName() 
{
	// (a.walling 2007-10-01 17:46) - PLID 26058 - Added exception handling
	try {
		m_bNeedToTest = TRUE;	
		// (a.walling 2007-09-20 14:39) - PLID 27468
		GetDlgItem(IDC_OPEN_CASH_DRAWER)->EnableWindow(FALSE);
	} NxCatchAll("Error in POSCashDrawerConfigDlg::OnChangeEditCashdrawerName");
}

// (a.walling 2007-08-07 11:59) - PLID 26068 - Preference to open cash drawer for checks, ADMIN ONLY
void POSCashDrawerConfigDlg::OnOpenForChecks() 
{
	try {
		if (IsCurrentUserAdministrator()) {
			// (a.walling 2007-10-04 14:43) - PLID 26058 - Use SetProperty
			SetPropertyInt("POSCashDrawer_OpenDrawerForChecks", IsDlgButtonChecked(IDC_OPEN_FOR_CHECKS), 0);
		}
	} NxCatchAll("Error in POSCashDrawerConfigDlg::OnOpenForChecks");
}

// (a.walling 2007-09-20 12:30) - PLID 9801 - Preference to open for refunds
void POSCashDrawerConfigDlg::OnOpenForRefunds() 
{
	try {
		if (IsCurrentUserAdministrator()) {
			// (a.walling 2007-10-04 14:43) - PLID 26058 - Use SetProperty
			SetPropertyInt("POSCashDrawer_OpenDrawerForRefunds", IsDlgButtonChecked(IDC_OPEN_FOR_REFUNDS), 0);
		}
	} NxCatchAll("Error in POSCashDrawerConfigDlg::OnOpenForRefunds");	
}

// (a.walling 2007-09-20 14:45) - PLID 27468 - Preference to open for tips
void POSCashDrawerConfigDlg::OnOpenForTips() 
{
	try {
		if (IsCurrentUserAdministrator()) {
			// (a.walling 2007-10-04 14:43) - PLID 26058 - Use SetProperty
			SetPropertyInt("POSCashDrawer_OpenDrawerForTips", IsDlgButtonChecked(IDC_OPEN_FOR_TIPS), 0);
		}
	} NxCatchAll("Error in POSCashDrawerConfigDlg::OnOpenForTips");	
}

// (a.walling 2007-09-20 14:45) - PLID 27468 - the Open Cash Drawer button was pressed!
void POSCashDrawerConfigDlg::OnOpenCashDrawer() 
{
	try {
		if (m_pPOSCashDrawerDevice) { // ensure we have a valid device
			if (CheckCurrentUserPermissions(bioCashDrawers, sptDynamic0)) { // and that we have permission
				CString strLastState = m_pPOSCashDrawerDevice->GetDrawerLastStateString();
				// (a.walling 2007-09-28 09:08) - PLID 26019 - Only audit if successfully opened
				long nResult = m_pPOSCashDrawerDevice->OpenDrawer();
				if (nResult == OPOS_SUCCESS) {
					AuditEvent(-1, "", BeginNewAuditEvent(), aeiPOSCashDrawerOpen, -1, strLastState, "Drawer Opened (Manually Opened from Settings)", aepHigh, aetChanged);	
				}
			}
		}
	} NxCatchAll("Error in POSCashDrawerConfigDlg::OnOpenCashDrawer");
}

// (a.walling 2007-09-21 15:18) - PLID 27468 - Added pref for opening for charges
void POSCashDrawerConfigDlg::OnOpenForCharge() 
{
	try {
		if (IsCurrentUserAdministrator()) {
			// (a.walling 2007-10-04 14:43) - PLID 26058 - Use SetProperty
			SetPropertyInt("POSCashDrawer_OpenDrawerForCharges", IsDlgButtonChecked(IDC_OPEN_FOR_CHARGE), 0);
		}
	} NxCatchAll("Error in POSCashDrawerConfigDlg::OnOpenForCharges");	
}

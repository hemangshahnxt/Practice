// POSPrinterSettingsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "POSPrinterSettingsDlg.h"
#include "OPOSPrinterDevice.h"

// (j.gruber 2007-05-08 12:31) - PLID 25772 - class created

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPOSPrinterSettingsDlg dialog


CPOSPrinterSettingsDlg::CPOSPrinterSettingsDlg(COPOSPrinterDevice *pPOSPrinter, CWnd* pParent /*=NULL*/)
	: CNxDialog(CPOSPrinterSettingsDlg::IDD, pParent)
{
	m_pPOSPrinterDevice = pPOSPrinter;
	//{{AFX_DATA_INIT(CPOSPrinterSettingsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CPOSPrinterSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPOSPrinterSettingsDlg)
	DDX_Control(pDX, IDC_POS_PRINTER_ON, m_btnOn);
	DDX_Control(pDX, IDC_POS_PRINTER_OFF, m_btnOff);
	DDX_Control(pDX, IDC_POS_PRINTER_TEST, m_btnPOSPrinterTest);
	DDX_Control(pDX, IDCANCEL, m_btnClose);
	DDX_Control(pDX, IDC_POS_PRINTER_NAME, m_nxeditPosPrinterName);
	DDX_Control(pDX, IDC_POS_PRINTER_STATUS, m_nxstaticPosPrinterStatus);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPOSPrinterSettingsDlg, CNxDialog)
	//{{AFX_MSG_MAP(CPOSPrinterSettingsDlg)
	ON_BN_CLICKED(IDC_POS_PRINTER_ON, OnPosPrinterOn)
	ON_BN_CLICKED(IDC_POS_PRINTER_OFF, OnPosPrinterOff)
	ON_BN_CLICKED(IDC_POS_PRINTER_TEST, OnPosPrinterTest)
	ON_EN_KILLFOCUS(IDC_POS_PRINTER_NAME, OnKillfocusPosPrinterName)
	ON_EN_CHANGE(IDC_POS_PRINTER_NAME, OnChangePosPrinterName)
	ON_BN_CLICKED(IDC_POS_PRINTER_APPLY, OnApply)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPOSPrinterSettingsDlg message handlers

void CPOSPrinterSettingsDlg::OnPosPrinterOn() 
{

	try {

		CheckDlgButton(IDC_POS_PRINTER_ON, TRUE);
		CheckDlgButton(IDC_POS_PRINTER_OFF, FALSE);
			

		
	}NxCatchAll("Error in OnPOSPrinterOn");
	
	
}

void CPOSPrinterSettingsDlg::OnPosPrinterOff() 
{
	
	CheckDlgButton(IDC_POS_PRINTER_ON, FALSE);
	CheckDlgButton(IDC_POS_PRINTER_OFF, TRUE);
	
}

void CPOSPrinterSettingsDlg::OnPosPrinterTest() 
{
	try {
		//let's print something out to make sure it works
		if (m_pPOSPrinterDevice) {

			//TES 1/9/2008 - PLID 28192 - We need to claim the POS Printer before we can print to it.
			if(m_pPOSPrinterDevice->CheckStatus()) {
				// (a.walling 2011-03-21 17:32) - PLID 42931 - RAII will save us from all these ReleasePOSPrinter calls
				// not to mention providing exception safety.
				POSPrinterAccess pPOSPrinter(m_pPOSPrinterDevice);

				if(pPOSPrinter) {
					pPOSPrinter->PrintText("Printing Test Page\n");
					for (int i=0; i < 3; i++) {
						pPOSPrinter->PrintText("\n");
					}
					// (a.walling 2011-04-28 10:02) - PLID 43492
					if (!pPOSPrinter->FlushAndTryCut()) {
						MsgBox("Failed to print test page!");
					}
				}
				else {
					MsgBox("Failed to claim the receipt printer");
				}
			}
		}
		else {
			MsgBox("The device is not enabled");
		}
	}NxCatchAll("Error In OnPosPrinterTest");
	
}

void CPOSPrinterSettingsDlg::OnKillfocusPosPrinterName() 
{
	//save the name
	try {

		CString strPrinterName;
		GetDlgItemText(IDC_POS_PRINTER_NAME, strPrinterName);
		if (strPrinterName.Compare(m_strPrinterName) != 0) {
		//	SetPropertyText("POSPrinter_DeviceName", strPrinterName, 0);
			m_strPrinterName = strPrinterName;
		}
		
	}NxCatchAll("Error in OnKillfocusPosPrinterName()");
	
}

COPOSPrinterDevice * CPOSPrinterSettingsDlg::GetOPOSPrinterDevice()  {
	return m_pPOSPrinterDevice;
}



BOOL CPOSPrinterSettingsDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	m_btnPOSPrinterTest.AutoSet(NXB_PRINT);
	m_btnClose.AutoSet(NXB_CLOSE);
	
	m_strPrinterName = GetPropertyText("POSPrinter_DeviceName", "", 0, TRUE);
	m_nPrinterInUse = GetPropertyInt("POSPrinter_UseDevice", 0, 0, TRUE);

	SetDlgItemText(IDC_POS_PRINTER_NAME, m_strPrinterName);

	if (m_nPrinterInUse == 1) {
		CheckDlgButton(IDC_POS_PRINTER_ON, TRUE);
		CheckDlgButton(IDC_POS_PRINTER_OFF, FALSE);
		GetDlgItem(IDC_POS_PRINTER_TEST)->EnableWindow(TRUE);
		GetDlgItem(IDC_POS_PRINTER_NAME)->EnableWindow(FALSE);
		//try to Connect
		ApplySettings();
	}
	else {
		CheckDlgButton(IDC_POS_PRINTER_ON, FALSE);
		CheckDlgButton(IDC_POS_PRINTER_OFF, TRUE);
		GetDlgItem(IDC_POS_PRINTER_TEST)->EnableWindow(FALSE);
		GetDlgItem(IDC_POS_PRINTER_NAME)->EnableWindow(TRUE);
	}

		
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPOSPrinterSettingsDlg::OnChangePosPrinterName() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	
}

void CPOSPrinterSettingsDlg::ApplySettings()  {

	try {
		BeginWaitCursor();
		SetDlgItemText(IDC_POS_PRINTER_STATUS, "Connecting...");

		SetPropertyText("POSPrinter_DeviceName", m_strPrinterName);

		// (a.walling 2011-03-21 17:32) - PLID 42931 - We don't delete this pointer now; it may be referenced elsewhere!
		bool bFailed = false;

		if (m_pPOSPrinterDevice) {
			//there is a printer already open, close it
			//this will show a message if there is an issue
			if (!m_pPOSPrinterDevice->ClosePOSPrinter()) {
				bFailed = true;
			}
			//m_pPOSPrinterDevice->DestroyWindow();
			//delete m_pPOSPrinterDevice;
			//m_pPOSPrinterDevice = NULL;
		}

		if (IsDlgButtonChecked(IDC_POS_PRINTER_ON)) {
			if (!bFailed) {
				//m_pPOSPrinterDevice = new COPOSPrinterDevice(GetMainFrame());
				CString strDeviceName;
				if (!m_pPOSPrinterDevice->InitiatePOSPrinterDevice(m_strPrinterName)) {
					// It seems that this device name is not correct
					// Delete the pointer to the device because it has not been initiated
					/*m_pPOSPrinterDevice->DestroyWindow();
					delete m_pPOSPrinterDevice;
					m_pPOSPrinterDevice = NULL;*/
					bFailed = true;
				}		
			}
		} else {
			bFailed = true;
		}

		// Set the status appropriately
		if (!bFailed) {
			m_nPrinterInUse = 1;
			SetDlgItemText(IDC_POS_PRINTER_STATUS, "Successfully Connected");
			SetPropertyInt("POSPrinter_UseDevice", 1, 0);
			GetDlgItem(IDC_POS_PRINTER_TEST)->EnableWindow(TRUE);
			GetDlgItem(IDC_POS_PRINTER_NAME)->EnableWindow(FALSE);
		} else {
			if (IsDlgButtonChecked(IDC_POS_PRINTER_ON)) {
				m_nPrinterInUse = 0;
				SetDlgItemText(IDC_POS_PRINTER_STATUS, "Failed to Connect");
				CheckDlgButton(IDC_POS_PRINTER_ON, FALSE);
				CheckDlgButton(IDC_POS_PRINTER_OFF, TRUE);
				SetPropertyInt("POSPrinter_UseDevice", 0, 0);
				GetDlgItem(IDC_POS_PRINTER_TEST)->EnableWindow(FALSE);
				GetDlgItem(IDC_POS_PRINTER_NAME)->EnableWindow(TRUE);
			}
			else {
				m_nPrinterInUse = 0;
				SetDlgItemText(IDC_POS_PRINTER_STATUS, "Turned Off");
				CheckDlgButton(IDC_POS_PRINTER_ON, FALSE);
				CheckDlgButton(IDC_POS_PRINTER_OFF, TRUE);
				SetPropertyInt("POSPrinter_UseDevice", 0, 0);
				GetDlgItem(IDC_POS_PRINTER_TEST)->EnableWindow(FALSE);
				GetDlgItem(IDC_POS_PRINTER_NAME)->EnableWindow(TRUE);
			}

		}
	}NxCatchAll("Error in ApplySettings");


}

void CPOSPrinterSettingsDlg::OnApply() 
{
	ApplySettings();
	
}

// SwiperSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "SwiperSetupDlg.h"
#include "KeyboardCardSwipeDlg.h" 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.wetta 2007-03-16 12:54) - PLID 25236 - This whole dialog was redone so that it would work with the new OPOS MSR device.

/////////////////////////////////////////////////////////////////////////////
// CSwiperSetupDlg dialog

CSwiperSetupDlg::CSwiperSetupDlg(COPOSMSRThread *pOPOSMSRThread, CWnd* pParent)
	: CNxDialog(CSwiperSetupDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSwiperSetupDlg)
	//}}AFX_DATA_INIT

	// (a.wetta 2007-07-06 14:09) - PLID 26547 - Get the pointer to the MSR thread
	m_pOPOSMSRThread = pOPOSMSRThread;
}

void CSwiperSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSwiperSetupDlg)
	DDX_Control(pDX, IDC_RADIO_ON, m_btnOn);
	DDX_Control(pDX, IDC_RADIO_OFF, m_btnOff);
	DDX_Control(pDX, IDC_RADIO_KEYBOARD, m_btnKeyboard); 
	DDX_Control(pDX, IDOK, m_okButton);
	DDX_Control(pDX, IDC_MSR_APPLY, m_applyButton);
	DDX_Control(pDX, IDC_NXCOLORCTRL1, m_bkg1);
	DDX_Control(pDX, IDC_NXCOLORCTRL8, m_bkg2);
	DDX_Control(pDX, IDC_EDIT_TRACK1, m_nxeditEditTrack1);
	DDX_Control(pDX, IDC_EDIT_TRACK2, m_nxeditEditTrack2);
	DDX_Control(pDX, IDC_EDIT_TRACK3, m_nxeditEditTrack3);
	DDX_Control(pDX, IDC_MSR_DEVICE_NAME, m_nxeditMsrDeviceName);
	DDX_Control(pDX, IDC_MSR_STATUS, m_nxstaticMsrStatus);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSwiperSetupDlg, CNxDialog)
	//{{AFX_MSG_MAP(CSwiperSetupDlg)
	ON_BN_CLICKED(IDC_MSR_APPLY, OnApply)
	ON_WM_CTLCOLOR()
	ON_MESSAGE(WM_MSR_DATA_EVENT, OnMSRDataEvent)
	ON_MESSAGE(WM_MSR_STATUS_INFO, OnMSRStatusInfo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CSwiperSetupDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CSwiperSetupDlg)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()


/////////////////////////////////////////////////////////////////////////////
// CSwiperSetupDlg message handlers

BOOL CSwiperSetupDlg::OnInitDialog() 
{
	// (b.spivey, December 13, 2011) - PLID 40567 - Added try/catch
	try {
		CNxDialog::OnInitDialog();

		// (b.spivey, December 13, 2011) - PLID 40567 - Cache these int properties. 
		// (r.gonet 2016-05-19 18:21) - NX-100689 - Get the computer name from the property manager rather
		// than the license object.
		g_propManager.CachePropertiesInBulk("SwiperSetupDlg-IntParam-Workstation", propNumber, 
			"(Username = '%s') AND ("
			"Name = 'MSR_UseDevice' "
			")",
			_Q(g_propManager.GetSystemName() + '.' + GetPracPath(PracPath::ConfigRT)));

		// (b.spivey, December 13, 2011) - PLID 40567 - Cache these text properties
		// (r.gonet 2016-05-19 18:21) - NX-100689 - Get the computer name from the property manager rather
		// than the license object.
		g_propManager.CachePropertiesInBulk("SwiperSetupDlg-TextParam-Workstation", propText, 
			"(Username = '%s') AND ("
			"Name = 'MSR_DeviceName' "
			")",
			_Q(g_propManager.GetSystemName() + '.' + GetPracPath(PracPath::ConfigRT)));

		m_okButton.AutoSet(NXB_CLOSE);
		
		SetDlgItemText(IDC_MSR_STATUS, "");
		SetDlgItemText(IDC_EDIT_TRACK1, "");
		SetDlgItemText(IDC_EDIT_TRACK2, "");
		SetDlgItemText(IDC_EDIT_TRACK3, "");
		SetDlgItemText(IDC_MSR_DEVICE_NAME, GetPropertyText("MSR_DeviceName", "", 0, true));

		// (b.spivey, October 04, 2011) - PLID 40567 - This can be one of three values, so we have to make sure the radio buttons
		//		reflect the choice that is loaded. 
		// (b.spivey, November 18, 2011) - PLID 40567 - Corrected bool type. 
		switch(GetPropertyInt("MSR_UseDevice", 0, 0, true)){
			case emmOn: 
				CheckDlgButton(IDC_RADIO_ON, TRUE); 
				CheckDlgButton(IDC_RADIO_OFF, FALSE); 
				CheckDlgButton(IDC_RADIO_KEYBOARD, FALSE); 
				break; 
			case emmKeyboard:
				CheckDlgButton(IDC_RADIO_ON, FALSE); 
				CheckDlgButton(IDC_RADIO_OFF, FALSE); 
				CheckDlgButton(IDC_RADIO_KEYBOARD, TRUE); 
				break; 
			case emmOff: 
			default:
				CheckDlgButton(IDC_RADIO_ON, FALSE); 
				CheckDlgButton(IDC_RADIO_OFF, TRUE); 
				CheckDlgButton(IDC_RADIO_KEYBOARD, FALSE); 
				break; 
		}

		// (a.wetta 2007-07-05 16:47) - PLID 26547 - Get the initial state of the MSR device
		if (m_pOPOSMSRThread) {
			SetDlgItemText(IDC_MSR_STATUS, "Getting Status...");
			PostThreadMessage(m_pOPOSMSRThread->m_nThreadID, WM_MSR_REQUEST_STATUS, 0, 0);
		}
		// (b.spivey, October 04, 2011) - PLID 40567 - consider keyboard mode. 
		else if(IsDlgButtonChecked(IDC_RADIO_KEYBOARD)){
			SetDlgItemText(IDC_MSR_STATUS, "Keyboard Mode");
		}
		else {
			SetDlgItemText(IDC_MSR_STATUS, "Turned Off");
		}

	}NxCatchAll(__FUNCTION__)
	
	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
	
}

void CSwiperSetupDlg::OnOK() 
{
	// (b.spivey, December 13, 2011) - PLID 40567 - Added try/catch
	try{
		Save();

		ApplySettings();

		CDialog::OnOK();
	}NxCatchAll(__FUNCTION__);
}

void CSwiperSetupDlg::Save()
{
	CString strDeviceName;
	GetDlgItemText(IDC_MSR_DEVICE_NAME, strDeviceName);
	SetPropertyText("MSR_DeviceName", strDeviceName, 0);
	// (b.spivey, October 04, 2011) - PLID 40567 - Consider keyboard mode before we consider it "off." 
	if(IsDlgButtonChecked(IDC_RADIO_ON)){
		SetPropertyInt("MSR_UseDevice", (long)emmOn);
	}
	else if(IsDlgButtonChecked(IDC_RADIO_KEYBOARD)){
		SetPropertyInt("MSR_UseDevice", (long)emmKeyboard);
	}
	else {
		SetPropertyInt("MSR_UseDevice", (long)emmOff);
	}

	// (a.walling 2007-09-28 14:03) - PLID 26547 - Set the device name
	if (m_pOPOSMSRThread) {
		m_pOPOSMSRThread->SetDeviceName(strDeviceName);
	}
}

LRESULT CSwiperSetupDlg::OnMSRDataEvent(WPARAM wParam, LPARAM lParam)
{
	// (b.spivey, December 13, 2011) - PLID 40567 - Added try/catch. 
	try {
		// (a.wetta 2007-07-05 08:56) - PLID 26547 - Get the track information from the card swipe
		MSRTrackInfo *mtiInfo = (MSRTrackInfo*)wParam;
		
		SetDlgItemText(IDC_EDIT_TRACK1, mtiInfo->strTrack1);
		SetDlgItemText(IDC_EDIT_TRACK2, mtiInfo->strTrack2);
		SetDlgItemText(IDC_EDIT_TRACK3, mtiInfo->strTrack3);
	}NxCatchAll(__FUNCTION__); 

	return 0;
}

// (a.wetta 2007-07-05 16:48) - PLID 26547 - Display the appropriate status for the MSR device on the dialog
LRESULT CSwiperSetupDlg::OnMSRStatusInfo(WPARAM wParam, LPARAM lParam)
{
	try {
		long nStatus = *((long*)wParam);

		switch (nStatus) {
		case MSR_DEVICE_ON:
			SetDlgItemText(IDC_MSR_STATUS, "Successfully Connected");
			break;
		case MSR_DEVICE_OFF:
			SetDlgItemText(IDC_MSR_STATUS, "Turned Off");
			// If it's turned off, then the thread shouldn't still be around, let's close it
			PostThreadMessage(m_pOPOSMSRThread->m_nThreadID, WM_QUIT, 0, 0);
			m_pOPOSMSRThread = NULL;
			break;
		case MSR_DEVICE_ERROR:
			SetDlgItemText(IDC_MSR_STATUS, "Failed to Connect");
			// Close the thread up because it's not a valid MSR device
			PostThreadMessage(m_pOPOSMSRThread->m_nThreadID, WM_QUIT, 0, 0);
			m_pOPOSMSRThread = NULL;
			break;
		default:
			SetDlgItemText(IDC_MSR_STATUS, "Unknown Status");
			// All statuses should be handled
			ASSERT(FALSE);
			break;
		}
	}NxCatchAll("Error in CSwiperSetupDlg::OnMSRStatusInfo");

	return 0;
}

void CSwiperSetupDlg::OnApply() 
{
	// (b.spivey, December 13, 2011) - PLID 40567 - Added try catch. 
	try {
		Save();

		ApplySettings();
		
		SetDlgItemText(IDC_EDIT_TRACK1, "");
		SetDlgItemText(IDC_EDIT_TRACK2, "");
		SetDlgItemText(IDC_EDIT_TRACK3, "");
	}NxCatchAll(__FUNCTION__);
}

void CSwiperSetupDlg::ApplySettings()
{
	try {
		BeginWaitCursor();

		// (a.wetta 2007-07-05 16:48) - PLID 26547 - Apply the currently saved settings to the MSR device by using the MSR thread
		if (IsDlgButtonChecked(IDC_RADIO_ON)) {
			SetDlgItemText(IDC_MSR_STATUS, "Connecting...");
			
			if (m_pOPOSMSRThread) {
				// The thread already exists, we're just updating the device name
				PostThreadMessage(m_pOPOSMSRThread->m_nThreadID, WM_MSR_RESTART, 0, 0);
			}
			else {
				// We need to create the thread and start the device
				m_pOPOSMSRThread = (COPOSMSRThread*)AfxBeginThread(RUNTIME_CLASS (COPOSMSRThread), THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
				m_pOPOSMSRThread->m_bAutoDelete = TRUE;
				m_pOPOSMSRThread->m_hwndNotify = GetMainFrame()->GetSafeHwnd();
				// (a.walling 2007-09-28 16:08) - PLID 26547 - Send the initial device name
				m_pOPOSMSRThread->m_strDeviceName = GetPropertyText("MSR_DeviceName", "", 0, TRUE);
				m_pOPOSMSRThread->ResumeThread();
				PostThreadMessage(m_pOPOSMSRThread->m_nThreadID, WM_MSR_RESTART, 0, 0);
			}
		}
		// (b.spivey, October 04, 2011) - PLID 40567 - Consider keyboard mode before off. 
		else if(IsDlgButtonChecked(IDC_RADIO_KEYBOARD)){
			if (m_pOPOSMSRThread) {
				PostThreadMessage(m_pOPOSMSRThread->m_nThreadID, WM_QUIT, 0, 0);
				m_pOPOSMSRThread = NULL;
			}			

			SetDlgItemText(IDC_MSR_STATUS, "Keyboard Mode");
		}
		else{
			if (m_pOPOSMSRThread) {
				PostThreadMessage(m_pOPOSMSRThread->m_nThreadID, WM_QUIT, 0, 0);
				m_pOPOSMSRThread = NULL;
			}			

			SetDlgItemText(IDC_MSR_STATUS, "Turned Off");
		}

		EndWaitCursor();

	}NxCatchAll("Error in CSwiperSetupDlg::ApplySettings");
}

HBRUSH CSwiperSetupDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	/*
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	switch (pWnd->GetDlgCtrlID())  {
		case IDC_STATIC:
		case IDC_MSR_STATUS:
		case IDC_RADIO_ON:
		case IDC_RADIO_OFF:
			extern CPracticeApp theApp;
			pDC->SelectPalette(&theApp.m_palette, FALSE);
			pDC->RealizePalette();
			pDC->SetBkColor(PaletteColor(0x00CEE7B4));
			return CreateSolidBrush(PaletteColor(0x00CEE7B4));
		break;
		default:
		break;
	}

	return hbr;
	*/

	// (a.walling 2008-04-02 09:14) - PLID 29497 - Deprecated, use base class
	return CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}

//DRT 6/2/2008 - PLID 30230 - Added OnCancel handler to keep behavior the same as pre-NxDialog changes
void CSwiperSetupDlg::OnCancel()
{
	//Eat the message
}

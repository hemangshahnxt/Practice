// OPOSMSRDevice.cpp : implementation file
//

#include "stdafx.h"
#include "OPOSMSRDevice.h"
#include "opos.h"
#include "OHIPUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (d.thompson 2010-12-20) - PLID 40314 - Made this a generically global function, not tied to this dialog.
void GetCreditCardTypeFromNumber(CString strAccountNumber, CString &strDescription, CString &strCD)
{
	strDescription = "";
	strCD = "";
	long nBINNumber = atoi(strAccountNumber.Left(8));
	
	// Determine the card type by the BIN number
	if (nBINNumber >= 40000000 && nBINNumber <= 49999999) {
		strDescription = "Visa";
		strCD = "VI";
	}
	else if ((nBINNumber >= 51000000 && nBINNumber <= 55999999) ||
		(nBINNumber >= 36000000 && nBINNumber <= 36999999)) {
		strDescription = "MasterCard";
		strCD = "MC";
	}
	else if ((nBINNumber >= 60110000 && nBINNumber <= 60119999) ||
		(nBINNumber >= 65000000 && nBINNumber <= 65999999) ||
		(nBINNumber >= 62212600 && nBINNumber <= 62292599)) {
		strDescription = "Discover";
		strCD = "DS";
	}
	else if ((nBINNumber >= 34000000 && nBINNumber <= 34999999) ||
		(nBINNumber >= 37000000 && nBINNumber <= 37999999)) {
		strDescription = "American Express";
		strCD = "AE";
	}
	else if ((nBINNumber >= 30830000 && nBINNumber <= 33299999) ||
		(nBINNumber >= 35280000 && nBINNumber <= 35899999)) {
		strDescription = "JCB";
		strCD = "JC";
	}
	else if ((nBINNumber >= 30813600 && nBINNumber <= 30813699) ||
		(nBINNumber >= 30813800 && nBINNumber <= 30819999)) {
		strDescription = "In Good Taste";
		strCD = "IG";
	}
	else if (nBINNumber >= 69000710 && nBINNumber <= 69000710) {
		strDescription = "Retail PL";
		strCD = "PL";
	}
	else if (nBINNumber >= 60302800 && nBINNumber <= 60302899) {
		strDescription = "Club Card";
		strCD = "CC";
	}
	else if (nBINNumber >= 60375700 && nBINNumber <= 60375799) {
		strDescription = "RAI";
		strCD = "RA";
	}
	else if (nBINNumber >= 70717000 && nBINNumber <= 70717099) {
		strDescription = "Irving Oil PL";
		strCD = "IR";
	}
	else if ((nBINNumber >= 76278300 && nBINNumber <= 76278399) ||
		(nBINNumber >= 76278900 && nBINNumber <= 76278999)) {
		strDescription = "Smarte Card";
		strCD = "SC";
	}
	else if (nBINNumber >= 70000500 && nBINNumber <= 70000599) {
		strDescription = "TCH Light Fleet";
		strCD = "TH";
	}
	else if (nBINNumber >= 70714500 && nBINNumber <= 70714599) {
		strDescription = "TCRS Consumer";
		strCD = "TS";
	}
	else if (nBINNumber >= 70828500 && nBINNumber <= 70828599) {
		strDescription = "MotoMart";
		strCD = "MM";
	}
	else if (nBINNumber >= 62742500 && nBINNumber <= 62742599) {
		strDescription = "Disney Rewards";
		strCD = "DY";
	}
	else {
		strDescription = "Unknown";
		strCD = "Unknown";
	}
}

/////////////////////////////////////////////////////////////////////////////
// COPOSMSRWindow

// (a.wetta 2007-07-03 16:06) - PLID 26547 - This is the main window in the MSR thread. Unfortunately the MSR Device can't be the main window
// itself in the thread because the device requires a parent window, thus this window. 

// (a.walling 2007-09-28 13:24) - PLID 26547 - Pass in the thread
COPOSMSRWindow::COPOSMSRWindow(HWND hwnd, COPOSMSRThread* pThread, BOOL m_bReturnInitCompleteMsg /*= FALSE*/) 
{
	m_hwndNotify = hwnd;

	// (a.walling 2007-09-28 13:59) - PLID 26547
	m_pThread = pThread;

	Create(NULL, _T("OPOS MSR Window"));

	// Create the MSR device and initiate it
	long nStatus = MSR_DEVICE_ON;
	// (a.walling 2007-09-28 11:04) - PLID 27556
	long nOpenResult = OPOS_SUCCESS;
	m_pMSRDevice = new COPOSMSRDevice(this);
	// (a.walling 2007-09-28 16:17) - PLID 26547 - Use the synchronized accessor
	if (!m_pMSRDevice->InitiateMSRDevice(m_pThread->GetDeviceName())) {
		// (a.walling 2007-09-28 11:05) - PLID 27556 - Get the open result before deleting
		nOpenResult = m_pMSRDevice->m_nOpenResult;
		// Delete the pointer to the device because it has not been initiated
		m_pMSRDevice->DestroyWindow();
		delete m_pMSRDevice;
		m_pMSRDevice = NULL;

		nStatus = MSR_DEVICE_ERROR;
	} else {
		// (a.walling 2007-09-28 11:05) - PLID 27556 - Get the open result anyway for our message
		nOpenResult = m_pMSRDevice->m_nOpenResult;
	}

	// If an init complete message is wanted, send it
	// (a.walling 2007-09-28 11:06) - PLID 27556 - Include the open result in the message
	if (m_bReturnInitCompleteMsg)
		CWnd::FromHandle(m_hwndNotify)->SendMessage(WM_MSR_INIT_COMPLETE, (WPARAM)&nStatus, (LPARAM)nOpenResult);
}

COPOSMSRWindow::~COPOSMSRWindow()
{
	// Close and delete the MSR device if it exists
	if (m_pMSRDevice) {
		m_pMSRDevice->CloseOPOSMSRDevice();
		m_pMSRDevice->DestroyWindow();
		delete m_pMSRDevice;
		m_pMSRDevice = NULL;
	}
}

BEGIN_MESSAGE_MAP(COPOSMSRWindow, CFrameWnd)
	ON_MESSAGE(WM_MSR_DATA_EVENT, OnMSRDataEvent)
END_MESSAGE_MAP()

// (a.wetta 2007-07-05 09:19) - PLID 26547 - This function handles a card swipe from the MSR device
LRESULT COPOSMSRWindow::OnMSRDataEvent(WPARAM wParam, LPARAM lParam)
{
	try {
		if (m_pMSRDevice) {
			// Get the track information of the swiped card from the MSR device
			MSRTrackInfo mtiInfo;
			m_pMSRDevice->GetMSRTrackInfo(mtiInfo.strTrack1, mtiInfo.strTrack2, mtiInfo.strTrack3);

			// (j.jones 2009-06-19 11:00) - PLID 33650 - supported the msrCardType enum
			// (j.jones 2009-06-19 11:00) - PLID 33650 - supported OHIP Health Cards
			// (j.jones 2009-08-27 14:30) - PLID 35370 - Do not attempt OHIP processing unless
			// UseOHIP() is enabled, to offset the (hopefully) rare case that someone has a credit
			// card that starts with 610054. It should start with a %, but it has been proven that
			// sometimes it does not, which is shady.
			if(UseOHIP()
				&& (mtiInfo.strTrack1.Left(8).CompareNoCase("%b610054") == 0
				|| mtiInfo.strTrack1.Left(7).CompareNoCase("B610054") == 0)) {

				mtiInfo.msrCardType = msrOHIPHealthCard;

				// Get the health card information
				OHIPHealthCardInfo ohciHealthCardInfo = m_pMSRDevice->GetOHIPHealthCardInfoFromMSR();

				// Send the track information to Practice in the form of a message
				CWnd::FromHandle(m_hwndNotify)->SendMessage(WM_MSR_DATA_EVENT, (WPARAM)&mtiInfo, (LPARAM)&ohciHealthCardInfo);
			}		
			else if (mtiInfo.strTrack1.Left(1) == "B") {
				mtiInfo.msrCardType = msrCreditCard;

				// Get the credit card information
				CreditCardInfo cciInfo = m_pMSRDevice->GetCreditCardInfoFromMSR();

				// Send the track information to Practice in the form of a message
				CWnd::FromHandle(m_hwndNotify)->SendMessage(WM_MSR_DATA_EVENT, (WPARAM)&mtiInfo, (LPARAM)&cciInfo);
			}
			else {
				mtiInfo.msrCardType = msrDriversLicense;

				// Send the track information to Practice in the form of a message
				CWnd::FromHandle(m_hwndNotify)->SendMessage(WM_MSR_DATA_EVENT, (WPARAM)&mtiInfo, 0);
			}		

			// Enable the MSR device again or else no more swipes will be accepted
			m_pMSRDevice->EnableMSRForNextSwipe();
		}
		
	// (a.walling 2007-09-28 13:17) - PLID 26547 - Need to use threaded exception handling
	}NxCatchAllThread("Error in COPOSMSRWindow::OnDataEventMsr");

	return 0;
}

// (a.wetta 2007-07-05 16:45) - PLID 26547 - The MSR device has been sent a message requesting it's status, so send a message back with the status
void COPOSMSRWindow::OnRequestStatus()
{
	try {
		long nStatus = -1;
		if (m_pMSRDevice) {
			nStatus = MSR_DEVICE_ON;	
		}
		else {
			nStatus = MSR_DEVICE_OFF;
		}

		CWnd::FromHandle(m_hwndNotify)->SendMessage(WM_MSR_STATUS_INFO, (WPARAM)&nStatus, 0);
	
	// (a.walling 2007-09-28 13:17) - PLID 26547 - Need to use threaded exception handling
	}NxCatchAllThread("Error in COPOSMSRWindow::OnRequestStatus");
}

// (a.wetta 2007-07-05 16:45) - PLID 26547 - Close and then reopen the MSR device using the currently saved device name
void COPOSMSRWindow::OnRestart()
{
	try {
		if (m_pMSRDevice) {
			// The MSR device is already open, let's close it
			m_pMSRDevice->CloseOPOSMSRDevice();
			m_pMSRDevice->DestroyWindow();
			delete m_pMSRDevice;
			m_pMSRDevice = NULL;
		}

		long nStatus = MSR_DEVICE_ON;
		m_pMSRDevice = new COPOSMSRDevice(this);
		// (a.walling 2007-09-28 16:18) - PLID 26547 - Use the synchronized accessor
		if (!m_pMSRDevice->InitiateMSRDevice(m_pThread->GetDeviceName())) {
			// Delete the pointer to the device because it has not been initiated
			m_pMSRDevice->DestroyWindow();
			delete m_pMSRDevice;
			m_pMSRDevice = NULL;

			nStatus = MSR_DEVICE_ERROR;
		}

		CWnd::FromHandle(m_hwndNotify)->SendMessage(WM_MSR_STATUS_INFO, (WPARAM)&nStatus, 0);

	// (a.walling 2007-09-28 13:17) - PLID 26547 - Need to use threaded exception handling
	}NxCatchAllThread("Error in COPOSMSRWindow::OnRestart");
}

/////////////////////////////////////////////////////////////////////////////
// COPOSMSRThread 

// (a.wetta 2007-07-03 16:07) - PLID 26547 - Created this thread so that the MSR device can function completely in a separate thread

IMPLEMENT_DYNCREATE(COPOSMSRThread, CWinThread)

COPOSMSRThread::COPOSMSRThread()
{
	m_bReturnInitCompleteMsg = FALSE;
}

COPOSMSRThread::~COPOSMSRThread()
{
	// Delete the MSR window
	m_pMainWnd->DestroyWindow();
	delete m_pMainWnd;
	m_pMainWnd = NULL;
}

BOOL COPOSMSRThread::InitInstance() 
{
	// (a.walling 2007-09-28 15:44) - PLID 26547 - Pass in pointer to this thread
	// Create the MSR window
	m_pMainWnd = new COPOSMSRWindow(m_hwndNotify, this, m_bReturnInitCompleteMsg);

	// Hide the window
	m_pMainWnd->MoveWindow(0,0,0,0,FALSE);
	m_pMainWnd->ShowWindow(SW_HIDE);

	return TRUE;
}

// (a.wetta 2007-07-05 16:46) - PLID 26547 - Handle any messages sent to this thread and call the appropriate function in the MSR device window
BOOL COPOSMSRThread::PreTranslateMessage(MSG *pMsg)
{
	switch (pMsg->message) {
	case WM_MSR_REQUEST_STATUS:
		((COPOSMSRWindow*)m_pMainWnd)->OnRequestStatus();
		return TRUE;
	case WM_MSR_RESTART:
		((COPOSMSRWindow*)m_pMainWnd)->OnRestart();
		return TRUE;
	}

	return CWinThread::PreTranslateMessage(pMsg);
}

// (a.walling 2007-09-28 13:58) - PLID 26547 - Synchronize access to the device name
CString COPOSMSRThread::GetDeviceName()
{
	try {
		CString str;
		CSingleLock sl(&m_cs);
		if (sl.Lock()) {
			str = m_strDeviceName;
			sl.Unlock();
		}

		return str;
	} NxCatchAllThread("Error in COPOSMSRThread::GetDeviceName");
	return "";
}

// (a.walling 2007-09-28 13:58) - PLID 26547 - Synchronize access to the device name
void COPOSMSRThread::SetDeviceName(CString str)
{
	try {
		CSingleLock sl(&m_cs);
		if (sl.Lock()) {
			m_strDeviceName = str;
			sl.Unlock();
		}
	} NxCatchAllThread("Error in COPOSMSRThread::SetDeviceName");
}


/////////////////////////////////////////////////////////////////////////////
// COPOSMSRDevice

// (a.wetta 2007-03-16 12:54) - PLID 25234 - This class was created to control the OPOS MSR device.

COPOSMSRDevice::COPOSMSRDevice(CWnd *pParentWnd)
{
	m_pParentWnd = pParentWnd;
}

// (a.walling 2007-09-28 13:26) - PLID 26547 - Use const
COPOSMSRDevice::COPOSMSRDevice(CWnd *pParentWnd, const CString &strMSRDeviceName)
{
	m_pParentWnd = pParentWnd;
	// (a.walling 2007-09-28 10:44) - PLID 27556
	m_nOpenResult = OPOS_SUCCESS;

	InitiateMSRDevice(strMSRDeviceName);
}

COPOSMSRDevice::~COPOSMSRDevice()
{
	CloseOPOSMSRDevice();
}


BEGIN_MESSAGE_MAP(COPOSMSRDevice, CWnd)
	//{{AFX_MSG_MAP(COPOSMSRDevice)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// COPOSMSRDevice message handlers

BEGIN_EVENTSINK_MAP(COPOSMSRDevice, CWnd)
    //{{AFX_EVENTSINK_MAP(COPOSMSRDevice)
	ON_EVENT(COPOSMSRDevice, IDC_MSR_CTRL, 1 /* DataEvent */, OnDataEventMsr, VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

// (a.wetta 2007-04-02 10:55) - PLID 25234 - The OnDataEventMsr function is called when a card is swiped.  So, any dialog
// that wants to receive a card swipe should handle the WM_MSR_DATA_EVENT message.  This message indicates that a card has
// been swiped and data can be retrieved from that card using the functions in this class.
void COPOSMSRDevice::OnDataEventMsr(long Status) 
{
	try {
		// Make sure that the tracks were read correctly.  If they were read correctly the first two tracks will not be blank.
		// If a track is blank it means there was an error reading it and there should always be at least 2 tracks
		CString strTrack1 = "", strTrack2 = "", strTrack3 = "";
		GetMSRTrackInfo(strTrack1, strTrack2, strTrack3);
		if (!strTrack1.IsEmpty() && !strTrack2.IsEmpty()) {
			// The tracks appear to have been read correctly, pass the data event message on
			m_pParentWnd->PostMessage(WM_MSR_DATA_EVENT, 0, 0);
		}
		else {
			EnableMSRForNextSwipe();
		}
	// (a.walling 2007-09-28 13:17) - PLID 26547 - Need to use threaded exception handling
	}NxCatchAllThread("Error in COPOSMSRDevice::OnDataEventMsr");
}

// (a.wetta 2007-04-02 10:57) - PLID 25234 - This function should be used when creating an MSR device.  It handles the case
// when the device is already open and prepares the device for a card swipe.
// (a.walling 2007-09-28 13:24) - PLID 26547 - Use const
BOOL COPOSMSRDevice::InitiateMSRDevice(const CString &strMSRDeviceName)
{
	try {
		if (!CloseOPOSMSRDevice())
			return FALSE;

		return CreateAndPrepareOPOSMSRControl(strMSRDeviceName);

	// (a.walling 2007-09-28 13:17) - PLID 26547 - Need to use threaded exception handling
	}NxCatchAllThread("Error in COPOSMSRDevice::InitiateMSRDevice");
	return FALSE;
}

// (a.wetta 2007-04-02 10:58) - PLID 25234 - This function creates the MSR device control but does not prepare it for a swipe.
// No swipe messages will be sent if only this function is called.
BOOL COPOSMSRDevice::CreateOPOSMSRControlWindow()
{
	try {
		if (m_pParentWnd) {
			// Create the MSR device object
			if (CreateControl(__uuidof(OposMSR_1_11_Lib::OPOSMSR), NULL, WS_CHILD, CRect(0,0,0,0), m_pParentWnd, IDC_MSR_CTRL)) {
				// Return success
				return TRUE;
			} else {
				// Couldn't create it, so fail
				return FALSE;
			}
		}
		else {
			ASSERT(FALSE);
			// The parent window should have been set by this point
			return FALSE;
		}
	// (a.walling 2007-09-28 13:17) - PLID 26547 - Need to use threaded exception handling
	}NxCatchAllThread("Error in COPOSMSRDevice::CreateOPOSMSRControlWindow");
	return FALSE;
}

// (a.wetta 2007-04-02 10:59) - PLID 25234 - The function creates the MSR device control and prepares it for a card swipe.
BOOL COPOSMSRDevice::CreateAndPrepareOPOSMSRControl(const CString &strMSRDeviceName)
{
	try {
		if (!CreateOPOSMSRControlWindow())
			return FALSE;
		
		OposMSR_1_11_Lib::IOPOSMSRPtr pmsr = GetControlUnknown();

		// (a.walling 2007-09-28 10:42) - PLID 27556
		m_nOpenResult = pmsr->Open(_bstr_t(strMSRDeviceName));
		if (m_nOpenResult == OPOS_SUCCESS) {
			// (a.walling 2011-02-09 16:53) - PLID 42406 - Just use a default timeout of 10s; can be overridden if necessary from a registry key
			long nResult = pmsr->ClaimDevice(AfxGetApp()->GetProfileInt("Settings", "OPOS_ClaimDevice_Timeout", 10000));
			if (nResult == OPOS_E_TIMEOUT) {
				// Something else has control over the device right now.
				CloseOPOSMSRDevice();
				return FALSE;
			}
			//(e.lally 2007-09-20) PLID 25234 - Check for all other non-success states and stop.
			else if(nResult != OPOS_SUCCESS){
				CloseOPOSMSRDevice();
				return FALSE;
			}
			else{
				//device open returned success, enable it
				pmsr->PutDeviceEnabled(TRUE);
				pmsr->PutDataEventEnabled(TRUE);
			}
		}
		else {
			return FALSE;
		}

		return TRUE;
	// (a.walling 2007-09-28 13:17) - PLID 26547 - Need to use threaded exception handling
	}NxCatchAllThread("Error in COPOSMSRDevice::CreateAndPrepareOPOSMSRControl");
	return FALSE;
}

BOOL COPOSMSRDevice::CloseOPOSMSRDevice()
{
	try {
		OposMSR_1_11_Lib::IOPOSMSRPtr pmsr = GetControlUnknown();

		if (pmsr) {
			if (pmsr->Close() != S_OK)
				return FALSE;
		}
		return TRUE;
	// (a.walling 2007-09-28 13:17) - PLID 26547 - Need to use threaded exception handling
	}NxCatchAllThread("Error in COPOSMSRDevice::CloseOPOSMSRDevice");
	return FALSE;
}

// (a.wetta 2007-04-02 10:59) - PLID 25234 - After the MSR device is created and after each swipe, the device needs to be re-enabled
// to receive a swipe.  This is so that the device will retain the information from the last swipe until you are done reading it.
void COPOSMSRDevice::EnableMSRForNextSwipe()
{
	try {
		OposMSR_1_11_Lib::IOPOSMSRPtr pmsr = GetControlUnknown();
		
		if (pmsr) {
			pmsr->PutDeviceEnabled(TRUE);
			pmsr->PutDataEventEnabled(TRUE);
		}
		else {
			ASSERT(FALSE);
		}
	// (a.walling 2007-09-28 13:17) - PLID 26547 - Need to use threaded exception handling
	}NxCatchAllThread("Error in COPOSMSRDevice::EnableMSRForNextSwipe");
}

// (a.wetta 2007-04-02 11:01) - PLID 25234 - A card with a magnetic strip contains 3 tracks of information.  The card's data is stored
// on these tracks.  This function will retrieve the raw data on these 3 tracks.
BOOL COPOSMSRDevice::GetMSRTrackInfo(CString &strTrack1, CString &strTrack2, CString &strTrack3)
{
	try {
		OposMSR_1_11_Lib::IOPOSMSRPtr pmsr = GetControlUnknown();

		if (pmsr) {
			strTrack1 = ((LPCSTR)pmsr->GetTrack1Data());
			strTrack2 = ((LPCSTR)pmsr->GetTrack2Data());
			strTrack3 = ((LPCSTR)pmsr->GetTrack3Data());

			return TRUE;
		}
		else {
			// This function should not be called if the MSR device does not exist
			ASSERT(FALSE);
			return FALSE;
		}
	// (a.walling 2007-09-28 13:17) - PLID 26547 - Need to use threaded exception handling
	}NxCatchAllThread("Error in COPOSMSRDevice::GetMSRTrackInfo");
	return FALSE;
}

// (a.wetta 2007-04-02 11:02) - PLID 25234 - This function will get the information from a driver's license after it has been swiped.
// If you call this function after something else but a driver's license has been swiped, the data in the DriversLicenseInfo structure
// will be garbage.
DriversLicenseInfo COPOSMSRDevice::GetDriversLicenseInfoFromMSR()
{
	try {
		CString strTrack1(""), strTrack2(""), strTrack3("");
		if (!GetMSRTrackInfo(strTrack1, strTrack2, strTrack3))
			return DriversLicenseInfo();

		DriversLicenseInfo dliLicenseInfo = ParseDriversLicenseInfoFromMSRTracks(strTrack1, strTrack2, strTrack3);

		EnableMSRForNextSwipe();

		return dliLicenseInfo;
	// (a.walling 2007-09-28 13:17) - PLID 26547 - Need to use threaded exception handling
	}NxCatchAllThread("Error in COPOSMSRDevice::GetDriversLicenseInfoFromMSR");
	return DriversLicenseInfo();
}

// (a.wetta 2007-03-15 15:23) - PLID 25234 - This function provides a reusable function
// to get a field of fixed or variable length from a string.  strString is the string of data; nFieldStart is where the field that you want
// starts on that string; bFixedLength is used to determine if the field is of fixed or variable length; strFixedLengthOrFieldSeparator contains
// the length of the fixed length field (bFixedLength is TRUE) or it contains the field separator signifying the end of the field (bFixedLength is FALSE);
// nNextFieldStart will be filled with the beginning of the next field after this field has been parsed out of the string.
BOOL COPOSMSRDevice::GetStringField(IN CString strString, IN long nFieldStart, IN BOOL bFixedLength, IN CString strFixedLengthOrFieldSeparator, 
					OUT CString &strField, OUT long &nNextFieldStart) 
{
	try {
		if (strString.GetLength() < nFieldStart) {
			// The string isn't even long enough
			return FALSE;
		}

		long nFixedLength = -1;
		long nFieldEnd = -1;
		if (bFixedLength) {
			nFixedLength = atoi(strFixedLengthOrFieldSeparator);
			nFieldEnd = nFieldStart + nFixedLength;
		}
		else {
			if (strFixedLengthOrFieldSeparator.Find(",") != -1) {
				// There are two separators, so let's try them both and use the closest one
				CString strSeparator1 = strFixedLengthOrFieldSeparator.Left(1), strSeparator2 = strFixedLengthOrFieldSeparator.Right(1);
				long nSeparator1 = -1, nSeparator2 = -1;
				nSeparator1 = strString.Find((LPCTSTR)strSeparator1, nFieldStart);
				nSeparator2 = strString.Find((LPCTSTR)strSeparator2, nFieldStart);

				if (nSeparator1 == -1 && nSeparator2 != -1)
					nFieldEnd = nSeparator2;
				else if (nSeparator1 != -1 && nSeparator2 == -1)
					nFieldEnd = nSeparator1;
				else if (nSeparator1 != -1 && nSeparator2 != -1) {
					if (nSeparator1 < nSeparator2)
						nFieldEnd = nSeparator1;
					else
						nFieldEnd = nSeparator2;
				}
			}
			else {
				nFieldEnd = strString.Find((LPCTSTR)strFixedLengthOrFieldSeparator, nFieldStart);
			}
		}

		if (nFieldEnd == -1 || nFieldEnd > strString.GetLength()) {
			// Let's just use the rest of the string
			nFieldEnd = strString.GetLength();
		}

		// Get the field
		strField = strString.Mid(nFieldStart, nFieldEnd-nFieldStart);

		// (j.jones 2009-08-27 15:14) - PLID 35369 - trim trailing commas
		strField.TrimRight(",");

		// Find the next field's start
		if (bFixedLength) {
			// The next field starts right away
			nNextFieldStart = nFieldEnd;
		}
		else {
			// The next field starts after the separator
			nNextFieldStart = nFieldEnd + 1;
		}

		return TRUE;
	}NxCatchAll_NoParent("Error in COPOSMSRDevice::GetStringField"); // (a.walling 2014-05-05 13:32) - PLID 61945
	return FALSE;
}

// (a.wetta 2007-04-02 11:07) - PLID 25234 - This function will parse out the driver's license information from the 3 data tracks on a driver's license.
DriversLicenseInfo COPOSMSRDevice::ParseDriversLicenseInfoFromMSRTracks(CString strTrack1 /*= ""*/, CString strTrack2 /*= ""*/, CString strTrack3 /*= ""*/)
{
	try {
		// -----
		// NOTE: The driver's license information is being parsed according to the AAMVA DL/ID Card Design Specifations Ver 2.0
		// -----
		// ******************************* TRACK 1 ************************************
		CString strState(""), strCity(""), strName(""), strFirstName(""), strLastName(""), strSuffix(""), strAddress1(""), strAddress2("");
		long nNextFieldStart = -1;
		//        | Length   |	                 |             
		// Length | fixed or |       Name        |            Information 
		//        | variable |                   |             
		//-----------------------------------------------------------------------------
		//    2   |    F     | State or Province | Mailing or residential code.
		if (GetStringField(strTrack1, 0, TRUE, "2", strState, nNextFieldStart)) {
		//-----------------------------------------------------------------------------
		//        |          |                   | This field shall be truncated with
		//        |          |                   | a field separator ^ if less than 13
		//        |          |                   | characters long. If the city is
		//    13  |   V-max  |       City        | exactly 13 characters long then
		//        |          |                   | no field separator is used (see
		//        |          |                   | i).
		//        |          |                   | Richfield^
			if (GetStringField(strTrack1, nNextFieldStart, FALSE, "^", strCity, nNextFieldStart)) {
		//-----------------------------------------------------------------------------
		//        |          |                   | Priority is as follows, spaces
		//        |          |                   | allowed;
		//        |          |                   | familyname$givenname$suffix
		//        |          |                   | This field shall be truncated with
		//    35  |   V-max  |       Name        | a field separator ^ if less than 35
		//        |          |                   | characters long. The "$" symbol
		//        |          |                   | is used as a delimiter between
		//        |          |                   | names (see i & iii).
				if (GetStringField(strTrack1, nNextFieldStart, FALSE, "^", strName, nNextFieldStart)) {
					// Now let's parse the Name field into it's individual parts
					long nNextSubFieldStart = -1;
					if (GetStringField(strName, 0, FALSE, "$", strLastName, nNextSubFieldStart)) {
						if (GetStringField(strName, nNextSubFieldStart, FALSE, "$", strFirstName, nNextSubFieldStart)) {
							GetStringField(strName, nNextSubFieldStart, FALSE, "$", strSuffix, nNextSubFieldStart);

							// (j.jones 2009-08-27 15:09) - PLID 35369 - if the strSuffix is empty, see if the first
							// name has a comma in it separating the middle name
							if(strSuffix.IsEmpty()) {
								int nComma = strFirstName.Find(",");
								if(nComma != -1) {									
									strSuffix = strFirstName.Right(strFirstName.GetLength() - nComma - 1);
									strFirstName = strFirstName.Left(nComma);
								}
							}
						}
					}
		//-----------------------------------------------------------------------------
		//        |          |                   | The street number shall be as it
		//        |          |                   | would appear on mail. The $ is
		//        |          |                   | used as a delimiter between
		//        |          |                   | address lines. This field shall be
		//        |          |                   | truncated with a field separator
		//    29  |    V     |      Address      | (or padded with spaces) if less
		//        |          |                   | than 29 characters long but can
		//        |          |                   | be longer (see i).
		//        |          |                   | 28 Atol Av$Suite 2$^
		//        |          |                   | Hiawatha Park$Apt 2037^
		//        |          |                   | 340 Brentwood Dr.$Fall Estate^
		//
					// Sometimes older licenses use $ to signify spaces and not as a delimiter between address lines.
					// So, 28 Atol Av would be 28$Atol$Av.  It seems that when a license uses this format is doesn't 
					// the field truncator ^. 
					CString strTemp = "";
					long nTemp;
					GetStringField(strTrack1, nNextFieldStart, FALSE, "@", strTemp, nTemp);  // "@" is used as the separator just because there must be something there and this character shouldn't appear in this field
					if (strTemp.Find("^") == -1) {
						// This license deviates from the standard
						strTemp.Replace("$", " ");
						strAddress1 = strTemp;
					}
					else {
						if (GetStringField(strTrack1, nNextFieldStart, FALSE, "$,^", strAddress1, nNextFieldStart)) {
							// (j.jones 2009-08-27 13:56) - PLID 35369 - if the address 2 begins with the city,
							// do not use it at all
							CString strTemp;
							GetStringField(strTrack1, nNextFieldStart, FALSE, "$,^", strTemp, nNextFieldStart);
							if(!strCity.IsEmpty() && strTemp.GetLength() >= strCity.GetLength()) {
								if(strTemp.Left(strCity.GetLength()) == strCity) {
									strAddress2 = "";
								}
								else {
									strAddress2 = strTemp;
								}
							}
						}
					}
				}
			}
		}

		// ******************************* TRACK 2 ************************************
		CString strISOIIN(""), strDLIDNum(""), strExpDate(""), strBirthdate(""), strDLIDOverflow("");
		nNextFieldStart = -1;
		//        | Length   |	                 |             
		// Length | fixed or |       Name        |            Information 
		//        | variable |                   |             
		//-----------------------------------------------------------------------------
		//        |          |                   | This is the assigned
		//        |          |                   | identification number from ISO.
		//        |          |                   | This number shall always begin
		//    6   |    F     |      ISO IIN      | with a "6".
		//        |          |                   | This number shall be obtained
		//        |          |                   | from the AAMVA Standards
		//        |          |                   | Assistant.
		if (GetStringField(strTrack2, 0, TRUE, "6", strISOIIN, nNextFieldStart)) {
		//-----------------------------------------------------------------------------
		//        |          |                   | This field is used to represent
		//        |          |                   | the DL/ID number assigned by
		//        |          |                   | each jurisdiction.
		//    13  |  V-max   |      DL/ID#       | Overflow for DL/ID numbers
		//        |          |                   | longer than 13 characters is
		//        |          |                   | accommodated in field number 7.
			if (GetStringField(strTrack2, nNextFieldStart, FALSE, "=", strDLIDNum, nNextFieldStart)) {
		//-----------------------------------------------------------------------------
		//        |          |                   | This field is in the format:
		//        |          |                   | YYMM
		//        |          |                   | If MM=77 then license is "nonexpiring".
		//        |          |                   | If MM=88 the Expiration Date is
		//        |          |                   | after the last day of their birth
		//        |          |                   | month One Year from the
		//    4   |    F     | Expiration date   | Month (MM) of Field 6 and the
		//        |          |                   | Year (YY) of Field 5 (Expiration
		//        |          |                   | Date).
		//        |          |                   | If MM=99 then the Expiration
		//        |          |                   | Date is on the Month (MM) and
		//        |          |                   | Day (DD) of Field 6 (Birthdate)
		//        |          |                   | and the Year (YY) of Field 5
		//        |          |                   | (Expiration Date).
				if (GetStringField(strTrack2, nNextFieldStart, TRUE, "4", strExpDate, nNextFieldStart)) {
		//-----------------------------------------------------------------------------
		//    8   |    F     |     Birthdate     | This field is in the format:
		//        |          |                   | CCYYMMDD
					if (GetStringField(strTrack2, nNextFieldStart, TRUE, "8", strBirthdate, nNextFieldStart)) {
		//-----------------------------------------------------------------------------
		//    5   |    V     |  DL/ID# overflow  | Overflow for numbers longer
		//                                       | than 13 characters. If no
		//                                       | information is used then a field
		//                                       | separator is used in this field.
						GetStringField(strTrack2, nNextFieldStart, FALSE, "?", strDLIDOverflow, nNextFieldStart);
					}
				}
			}
		}

		// Parse the birthdate
		COleDateTime dtBirthdate = COleDateTime(1899,12,30,0,0,0);
		if (strBirthdate != "") {
			dtBirthdate.SetDate(atoi(strBirthdate.Left(4)), atoi(strBirthdate.Mid(4,2)), atoi(strBirthdate.Right(2)));
		}

		// Parse the expiration date of the driver's license
		if (strExpDate != "") {
			CString strYear = strExpDate.Left(2), strMonth = strExpDate.Right(2);
			if (strMonth == "77") {
				strExpDate = "non-expiring";
			}
			else if (strMonth == "88") {
				// Find the next month
				long nMonth = (dtBirthdate.GetMonth() + 1) % 12;
				long nYear = atoi(strYear) + 2001;
  				strExpDate.Format("%li/1/%li", nMonth, nYear);
			}
			else if (strMonth == "99") {
				long nYear = atoi(strYear) + 2000;
				strExpDate.Format("%li/%li/%li", dtBirthdate.GetMonth(), dtBirthdate.GetDay(), nYear); 
			}
			else {
				long nYear = atoi(strYear) + 2000;
				strExpDate.Format("%s/%li/%li", strMonth, dtBirthdate.GetDay(), nYear);
			}
		}

		// Sometimes the birthdate follows the same odd month scheme as the expiration date and refers to the
		// expiration date for the month.  So, if the birthdate is zero, see if it is following this scheme.
		if (strBirthdate != "" && strExpDate != "" && dtBirthdate == 0) {
			if (strBirthdate.Mid(4,2) == "99") {
				long nMonthEndPos = strExpDate.Find("/");
				CString strMonth = strExpDate.Left(nMonthEndPos);
				dtBirthdate.SetDate(atoi(strBirthdate.Left(4)), atoi(strMonth), atoi(strBirthdate.Right(2)));
			}
		}

		// Put the two parts of the DL/ID# together
		if (strDLIDNum != "" && strDLIDOverflow != "") {
			strDLIDNum = strDLIDNum + strDLIDOverflow;
		}	

		// ******************************* TRACK 3 ************************************
		CString strPostalCode(""), strSex(""), strHeight(""), strWeight(""), strHairColor(""), strEyeColor("");
		nNextFieldStart = -1;
		//        | Length   |	                 |             
		// Length | fixed or |       Name        |            Information 
		//        | variable |                   |             
		//-----------------------------------------------------------------------------
		//        |          |                   | For an 11 digit postal or zip
		//    11  |    F     |    Postal code    | code. (left justify fill with
		//        |          |                   | spaces, no hyphen)
		if (GetStringField(strTrack3, 2, TRUE, "11", strPostalCode, nNextFieldStart)) {
		//-----------------------------------------------------------------------------
		//    1   |    F     |        Sex        | 1 for male, 2 for female. 
			if (GetStringField(strTrack3, 29, TRUE, "1", strSex, nNextFieldStart)) {
		//-----------------------------------------------------------------------------
		//    3   |    F     |       Height      | 
				if (GetStringField(strTrack3, nNextFieldStart, TRUE, "3", strHeight, nNextFieldStart)) {
		//-----------------------------------------------------------------------------
		//    3   |    F     |       Weight      |
					if (GetStringField(strTrack3, nNextFieldStart, TRUE, "3", strWeight, nNextFieldStart)) {
		//-----------------------------------------------------------------------------
		//    3   |    F     |     Hair Color    |
						if (GetStringField(strTrack3, nNextFieldStart, TRUE, "3", strHairColor, nNextFieldStart)) {
		//-----------------------------------------------------------------------------
		//    3   |    F     |     Eye Color     |
							GetStringField(strTrack3, nNextFieldStart, TRUE, "3", strEyeColor, nNextFieldStart);
						}
					}
				}
			}
		}

		// Determine the sex
		if (strSex != "") {
			if (strSex == "1" || strSex == "M") {
				strSex = "male";
			}
			else if (strSex == "2" || strSex == "F") {
				strSex = "female";
			}
		}

		// Fix the postal code
		if (strPostalCode != "") {
			// We only want the first 5 digits
			strPostalCode = strPostalCode.Left(5);
		}

		return DriversLicenseInfo(strState, strCity, strFirstName, strLastName, strSuffix, strAddress1, strAddress2, strISOIIN, strDLIDNum,
								  strExpDate, dtBirthdate, strPostalCode, strSex, strHeight, strWeight, strHairColor, strEyeColor);
	}NxCatchAll_NoParent("Error in COPOSMSRDevice::ParseDriversLicenseInfoFromMSRTracks");
	return DriversLicenseInfo();
}

// (a.wetta 2007-04-02 11:11) - PLID 25234 - This function returns the credit card information on a credit card that has just been swiped.  If 
// something besides a credit card was just swiped, the CreditCardInfo returned will be garbage or empty.
CreditCardInfo COPOSMSRDevice::GetCreditCardInfoFromMSR()
{
	try {
		OposMSR_1_11_Lib::IOPOSMSRPtr pmsr = GetControlUnknown();

		if (pmsr) {
			// Get the track information from the credit card
			CString strTrack1 = "", strTrack2 = "", strTrack3 = "";
			GetMSRTrackInfo(strTrack1, strTrack2, strTrack3);

			EnableMSRForNextSwipe();

			CreditCardInfo cciInfo = ParseCreditCardInfoFromMSRTracks(strTrack1, strTrack2, strTrack3);
			cciInfo.m_strSuffix = (LPCSTR)pmsr->GetSuffix();
			cciInfo.m_strTitle = (LPCSTR)pmsr->GetTitle();

			return cciInfo;
		}
		else {
			// This function should not be called if the MSR device does not exist
			ASSERT(FALSE);
			EnableMSRForNextSwipe();
			return CreditCardInfo();
		}
	// (a.walling 2007-09-28 13:17) - PLID 26547 - Need to use threaded exception handling
	}NxCatchAllThread("Error in COPOSMSRDevice::GetCreditCardInfoFromMSR");
	return CreditCardInfo();
}

// (a.wetta 2007-07-05 10:21) - PLID 26547 - Created separate function to parse credit card info from tracks
CreditCardInfo COPOSMSRDevice::ParseCreditCardInfoFromMSRTracks(CString strTrack1 /*= ""*/, CString strTrack2 /*= ""*/, CString strTrack3 /*= ""*/)
{
	try {
		// ******************************* TRACK 1 ************************************
		// The first character should be "B" which signifies that this is a card from a financial institution
		CString strIDChar(""), strAccountNumber(""), strName(""), strExpDate("");
		long nNextFieldStart = -1;
		
		// Get the inital character on the track
		if (GetStringField(strTrack1, 0, TRUE, "1", strIDChar, nNextFieldStart)) {
			// Check the ID character
			if (strIDChar != "B") {
				ASSERT(FALSE);
				// This doesn't appear to be a credit card, so this function should not be used to parse this card's data
				return CreditCardInfo();
			}
			// Get the account number
			if (GetStringField(strTrack1, nNextFieldStart, FALSE, "^", strAccountNumber, nNextFieldStart)) {
				// Get the name on the card
				if (GetStringField(strTrack1, nNextFieldStart, FALSE, "^", strName, nNextFieldStart)) {
					// Get the expiration date
					GetStringField(strTrack1, nNextFieldStart, TRUE, "4", strExpDate, nNextFieldStart);
				}
			}
		}

		// ******************************* TRACK 2 ************************************
		// The second track has the same information as the first track, but is there for redundancy
		CString strAccountNumberRedundant(""), strExpDateRedundant("");
		nNextFieldStart = -1;

		// Get the account number
		if (GetStringField(strTrack2, 0, FALSE, "=", strAccountNumberRedundant, nNextFieldStart)) {
			// Get the expiration date
			GetStringField(strTrack2, nNextFieldStart, TRUE, "4", strExpDateRedundant, nNextFieldStart);
		}


		// (d.thompson 2010-10-26) - PLID 41080 - American express now puts spaces in some of their numbers, which throws off the 
		//	"convert to numeric" functionality we use later, and breaks the string comparison here.  It seems like Track 1 contains
		//	the spaces, while track 2 does not.  I've found a reference online confirming this, so it's not a fluke.
		strAccountNumber.Remove(' ');
		strAccountNumberRedundant.Remove(' ');



		////////////////////////////////////////////////////////////
		//// Now process the information

		// First make sure that the redundancy between the first two tracks checks out, if not then there was an error reading the 
		// card and return blank information
		if (strAccountNumber != strAccountNumberRedundant || strExpDate != strExpDateRedundant) {
			// There was trouble reading the card, return blank information
			return CreditCardInfo();
		}

		// Get the card type
		CString strCardType = "", strCD = "";
		GetCreditCardTypeFromNumber(strAccountNumber, strCardType, strCD);

		// Parse the expiration date
		COleDateTime dtExpDate = COleDateTime(1899,12,30,0,0,0);
		long nYear = atoi(strExpDate.Left(2)) + 2000;
		long nMonth = atoi(strExpDate.Right(2));
		if (strExpDate != "") {
			dtExpDate.SetDate(nYear, nMonth, 1);
		}

		// Parse the name
		CString strFirstName = "", strMiddleName = "", strLastName = "";
		long nSpace1Pos = -1, nSpace2Pos = -1;
		nSpace1Pos = strName.Find("/");
		if (nSpace1Pos != -1) {
			nSpace2Pos = strName.Find(" ", nSpace1Pos+1);
			if (nSpace2Pos == -1) {
				// (Last/First)
				strLastName = strName.Left(nSpace1Pos);
				strFirstName = strName.Right(strName.GetLength() - (nSpace1Pos+1));
			}
			else {
				// (Last/First Middle)
				strLastName = strName.Left(nSpace1Pos);
				strFirstName = strName.Mid(nSpace1Pos+1, nSpace2Pos-nSpace1Pos);
				strMiddleName = strName.Right(strName.GetLength() - (nSpace2Pos+1));
			}
		}
		else {
			nSpace1Pos = strName.Find(" ");
			nSpace2Pos = strName.Find(" ", nSpace1Pos+1);
			if (nSpace2Pos == -1) {
				// (First Last)
				strFirstName = strName.Left(nSpace1Pos);
				strLastName = strName.Right(strName.GetLength() - (nSpace1Pos+1));
			}
			else {
				// (First Middle Last)
				strFirstName = strName.Left(nSpace1Pos);
				strMiddleName = strName.Mid(nSpace1Pos+1, nSpace2Pos-nSpace1Pos);
				strLastName = strName.Right(strName.GetLength() - (nSpace2Pos+1));
			}
		}
		strFirstName.TrimLeft(); strFirstName.TrimRight();
		strMiddleName.TrimLeft(); strMiddleName.TrimRight();
		strLastName.TrimLeft(); strLastName.TrimRight();

		// Return all of the credit card information
		return CreditCardInfo(strAccountNumber, strFirstName, strMiddleName, strLastName, 
					"", "", strCardType, strCD, dtExpDate);

	}NxCatchAll_NoParent("Error in COPOSMSRDevice::ParseCreditCardInfoFromMSRTracks");
	return CreditCardInfo();
}

			

HRESULT COPOSMSRDevice::GetMSRState()
{
	try {
		OposMSR_1_11_Lib::IOPOSMSRPtr pmsr = GetControlUnknown();

		if (pmsr) {
			return pmsr->GetState();
		}
	// (a.walling 2007-09-28 13:20) - PLID 26547 - Need to use threaded exception handling
	}NxCatchAllThread("Error in COPOSMSRDevice::GetMSRState");
	
	return OPOS_S_ERROR;
}

BOOL COPOSMSRDevice::VerifyCreditCardNumber(CString strAccountNumber)
{
	try {
		// The Luhn algorithm (aka the "mod 10" algorithm) is used to verify that the credit card number
		// is valid, however, some credit card types cannot be verified by this method
		long nSum = 0;
		BOOL bAlt = FALSE;
		// Starting from the left double the value of every second digit, if the doubled value is larger 
		// than 9, then add the two digits together, i.e. 12 -> 1 + 2 -> 3
		for (int i = strAccountNumber.GetLength()-1; i >= 0; i--) {
			CString strDigit = (char)strAccountNumber.GetAt(i);
			long nDigit = atoi(strDigit);
			// Alternate every other digit
			if (bAlt) {
				// Double the value of the digit
				nDigit *= 2;
				// If the value is larger than 9, add the two resulting digits together
				if (nDigit > 9)
					nDigit -= 9; // Equivalent to adding the values of the digits
			}
			// Add the digit to the sum
			nSum += nDigit;
			bAlt = !bAlt;
		}
		// If the mod 10 of the sum is 0, then the number is valid
		return (nSum % 10 == 0 ? TRUE : FALSE);
	// (a.walling 2007-09-28 13:20) - PLID 26547 - Need to use threaded exception handling
	}NxCatchAllThread("Error in COPOSMSRDevice::VerifyCreditCardNumber");
	return FALSE;
 }

// (j.jones 2009-06-19 09:50) - PLID 33650 - supported OHIP health cards
OHIPHealthCardInfo COPOSMSRDevice::ParseOHIPHealthCardInfoFromMSRTracks(CString strTrack1 /*= ""*/, CString strTrack2 /*= ""*/, CString strTrack3 /*= ""*/)
{
	try {

		//sample results:
		//Track 1 = %B6100546968168291^JOHNSON/POLLY ANN         ^0908799219770826LRPOLLY08040601?

		//Track 2 = ;6100546968168291=09087990000100000000?

		//Track 3 = ;906100546968168291=0000000000000000000000000000000000000000000000000000000000000000000000000000000000000?

		// (j.jones 2009-08-27 14:34) - PLID 35370 - sometimes it might not have the first character in each track,
		// the % or ; signs, so we unfortunately have to check for that
		if(strTrack1.Left(1) != "%") {
			strTrack1 = "%" + strTrack1;
		}
		if(strTrack2.Left(1) != ";") {
			strTrack2 = ";" + strTrack2;
		}
		if(strTrack3.Left(1) != ";") {
			strTrack3 = ";" + strTrack3;
		}

		CString strFirstName = "";
		CString strMiddleName = "";
		CString strLastName = "";
		CString strHealthCardNum = "";
		CString strVersionCode = "";
		COleDateTime dtBirthDate = COleDateTime(1899,12,30,0,0,0);
		CString strSex = "";

		// ******************************* TRACK 1 ************************************

		//Sample = %B6100546968168291^JOHNSON/POLLY ANN         ^0908799219770826LRPOLLY08040601?

		long nNextFieldStart = -1;

		//        |                          |            
		// Length |  Name                    |  Comments 
		//        |                          |            
		//-----------------------------------------------------------------------------
		//    1   |  Start Sentinel          |  Fixed Value = "%"
		//-----------------------------------------------------------------------------
		//    1   |  Format Code             |  Fixed Value = "b"
		//-----------------------------------------------------------------------------
		//    6   |  Issuer Identification   |  Fixed Value = "610054"
		//-----------------------------------------------------------------------------

		//we don't use any of these

		//-----------------------------------------------------------------------------
		//    10  |  Health Number           |  
		//-----------------------------------------------------------------------------

		//grab the health card number
		GetStringField(strTrack1, 8, TRUE, "10", strHealthCardNum, nNextFieldStart);

		//-----------------------------------------------------------------------------
		//    1   |  Field Separator         |  Fixed Value = "^"
		//-----------------------------------------------------------------------------

		//ignored

		//-----------------------------------------------------------------------------
		//    26  |  Name                    |  As per ISO standards. Separated by "/"
		//-----------------------------------------------------------------------------

		// (j.jones 2009-10-05 15:00) - PLID 35760 - this needs to go to the next ^ field, 
		// which might not be 26 characters on older cards

		//grab the name
		CString strFullName;
		if(GetStringField(strTrack1, 19, FALSE, "^", strFullName, nNextFieldStart)) {
			//parse the name out
			int nSlash1 = strFullName.Find("/");
			if(nSlash1 == -1) {
				//odd, there isn't one
				strLastName = strFullName;
				strFirstName = "";
				strMiddleName = "";
			}
			else {
				strLastName = strFullName.Left(nSlash1);

				CString strFirstMiddle = strFullName.Right(strFullName.GetLength() - nSlash1 - 1);

				//see if there is a slash separating the middle name
				int nSlash2 = strFirstMiddle.Find("/");
				if(nSlash2 == -1) {
					//there isn't, so try a space
					int nSpace = strFirstMiddle.Find(" ");
					if(nSpace == -1) {
						//there is only a first name
						strFirstName = strFirstMiddle;
						strMiddleName = "";
					}
					else {
						strFirstName = strFirstMiddle.Left(nSpace);
						strMiddleName = strFirstMiddle.Right(strFirstMiddle.GetLength() - nSpace - 1);
					}
				}
				else {
					strFirstName = strFirstMiddle.Left(nSlash2);
					strMiddleName = strFirstMiddle.Right(strFirstMiddle.GetLength() - nSlash2 - 1);
				}
			}

			strLastName.Trim();
			strFirstName.Trim();
			strMiddleName.Trim();
		}
		
		//-----------------------------------------------------------------------------
		//    1   |  Field Separator         |  Fixed Value = "^"
		//-----------------------------------------------------------------------------

		//not used
		//do not read this field, the patient name is now going to run *through* this field,
		//so nNextFieldStart bypasses this field
		//CString strSeparator;
		//GetStringField(strTrack1, nNextFieldStart, TRUE, "1", strSeparator, nNextFieldStart);

		//-----------------------------------------------------------------------------
		//    4   |  Expiry Date             |  YYMM or zero filled
		//-----------------------------------------------------------------------------

		//not used
		CString strExpiry;
		GetStringField(strTrack1, nNextFieldStart, TRUE, "4", strExpiry, nNextFieldStart);

		//-----------------------------------------------------------------------------
		//    1   |  Interchange Code        |  Fixed Value = "7"
		//-----------------------------------------------------------------------------

		//not used
		CString strInterchange;
		GetStringField(strTrack1, nNextFieldStart, TRUE, "1", strInterchange, nNextFieldStart);

		//-----------------------------------------------------------------------------
		//    2   |  Service Code            |  Fixed Value = "99"
		//-----------------------------------------------------------------------------

		//not used
		CString strServiceCode;
		GetStringField(strTrack1, nNextFieldStart, TRUE, "2", strInterchange, nNextFieldStart);

		//-----------------------------------------------------------------------------
		//    1   |  Sex                     |  1 = Male, 2 = Female
		//-----------------------------------------------------------------------------

		//grab the gender
		GetStringField(strTrack1, nNextFieldStart, TRUE, "1", strSex, nNextFieldStart);

		//-----------------------------------------------------------------------------
		//    8   |  Date Of Birth           |  YYYYMMDD
		//-----------------------------------------------------------------------------

		//get the birthdate
		CString strBirthDateYearFirst;
		if(GetStringField(strTrack1, nNextFieldStart, TRUE, "8", strBirthDateYearFirst, nNextFieldStart)) {
			if(strBirthDateYearFirst != "00000000" && !strBirthDateYearFirst.IsEmpty()) {
				//format CCYYMMDD into MM/DD/CCYY
				CString strBirthDate = strBirthDateYearFirst.Right(4) + "/" + strBirthDateYearFirst.Left(4);
				strBirthDate = strBirthDate.Left(2) + "/" + strBirthDate.Right(7);
				COleDateTime dtTemp;
				if(dtTemp.ParseDateTime(strBirthDate)) {
					//it's a valid date, let's use it
					dtBirthDate = dtTemp;
				}
			}
		}

		//-----------------------------------------------------------------------------
		//    2   |  Card Version Number     |  XX (may be blank)
		//-----------------------------------------------------------------------------

		//grab the version code
		GetStringField(strTrack1, nNextFieldStart, TRUE, "2", strVersionCode, nNextFieldStart);

		//-----------------------------------------------------------------------------
		//    5   |  First Name - Short      |  First 5 characters of first or middle name
		//-----------------------------------------------------------------------------
		//    6   |  Issue Date              |  YYYYMMDD
		//-----------------------------------------------------------------------------
		//    2   |  Language Preference     |  01 = END, 02 = FR
		//-----------------------------------------------------------------------------
		//    1   |  End Sentinel            |  Fixed Value = "?"
		//-----------------------------------------------------------------------------
		//    1   |  Parity Check            |  As per ISO Standards
		//-----------------------------------------------------------------------------

		//we don't use any of these

		// ******************************* TRACK 2 ************************************
		
		//Sample = ;6100546968168291=09087990000100000000?

		nNextFieldStart = -1;

		//        |                          |            
		// Length |  Name                    |  Comments 
		//        |                          |            
		//-----------------------------------------------------------------------------
		//    1   |  Start Sentinel          |  Fixed Value = ";"
		//-----------------------------------------------------------------------------
		//    6   |  Issuer Identification   |  Fixed Value = "610054"
		//-----------------------------------------------------------------------------
		//    10  |  Health Number           |  
		//-----------------------------------------------------------------------------
		//    1   |  Field Separator         |  Fixed Value = "="
		//-----------------------------------------------------------------------------
		//    4   |  Expiry Date             |  YYMM or zero filled
		//-----------------------------------------------------------------------------
		//    1   |  Interchange Code        |  Fixed Value = "7"
		//-----------------------------------------------------------------------------
		//    2   |  Service Code            |  Fixed Value = "99"
		//-----------------------------------------------------------------------------
		//    4   |  Filler                  |  Fixed Value = "0000"
		//-----------------------------------------------------------------------------
		//    1   |  Card Type               |  1 = REG, 2 = 65
		//-----------------------------------------------------------------------------
		//    8   |  OHIP Number             |  Number or "00000000"
		//-----------------------------------------------------------------------------
		//    1   |  End Sentinel            |  Fixed Value = "?"
		//-----------------------------------------------------------------------------
		//    1   |  Parity Check            |  As per ISO Standards
		//-----------------------------------------------------------------------------

		//we do not use any of these because all the information we need has already
		//been acquired through Track 1
		

		// ******************************* TRACK 3 ************************************		

		//Sample = ;906100546968168291=0000000000000000000000000000000000000000000000000000000000000000000000000000000000000?
		
		nNextFieldStart = -1;

		//        |                          |            
		// Length |  Name                    |  Comments 
		//        |                          |            
		//-----------------------------------------------------------------------------
		//    1   |  Start Sentinel          |  Fixed Value = ";"
		//-----------------------------------------------------------------------------
		//    2   |  Format Code             |  Fixed Value = "90"
		//-----------------------------------------------------------------------------
		//    6   |  Issuer Identification   |  Fixed Value = "610054"
		//-----------------------------------------------------------------------------
		//    10  |  Health Number           |  
		//-----------------------------------------------------------------------------
		//    1   |  Field Separator         |  Fixed Value = "="
		//-----------------------------------------------------------------------------
		//    85  |  Filler                  |  Fixed Value = "0"
		//-----------------------------------------------------------------------------
		//    1   |  End Sentinel            |  Fixed Value = "?"
		//-----------------------------------------------------------------------------
		//    1   |  Parity Check            |  As per ISO Standards
		//-----------------------------------------------------------------------------

		//we do not use any of these because all the information we need has already
		//been acquired through Track 1

		return OHIPHealthCardInfo(strFirstName, strMiddleName, strLastName, strHealthCardNum, strVersionCode, dtBirthDate, strSex);

	}NxCatchAll_NoParent("Error in COPOSMSRDevice::ParseOHIPHealthCardInfoFromMSRTracks");
	return OHIPHealthCardInfo();
}

// (j.jones 2009-06-19 09:50) - PLID 33650 - supported OHIP health cards
// This function will get the information from a health card after it has been swiped.
// If you call this function after something else but a health card has been swiped, the data
// in the OHIPHealthCardInfo structure will be garbage.
OHIPHealthCardInfo COPOSMSRDevice::GetOHIPHealthCardInfoFromMSR()
{
	try {

		CString strTrack1(""), strTrack2(""), strTrack3("");
		if (!GetMSRTrackInfo(strTrack1, strTrack2, strTrack3))
			return OHIPHealthCardInfo();

		OHIPHealthCardInfo ohciHealthCardInfo = ParseOHIPHealthCardInfoFromMSRTracks(strTrack1, strTrack2, strTrack3);

		EnableMSRForNextSwipe();

		return ohciHealthCardInfo;
	
	}NxCatchAllThread("Error in COPOSMSRDevice::GetOHIPHealthCardInfoFromMSR");
	return OHIPHealthCardInfo();
}
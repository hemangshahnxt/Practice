//(a.wilson 2012-1-11) PLID 47120 - .cpp for the OPOS Barcode Scanner 

#include "StdAfx.h"
#include "OPOSBarcodeScanner.h"
#include "OposScan.h"
#include "barcode.h"
#include "GlobalStringUtils.h"

// (a.walling 2012-01-17 12:55) - PLID 47120 - NxCatchAllThread replaced with NxCatchAll -- they are identical

////

using namespace OposScanner_CCO;

//(a.wilson 2012-1-12) PLID 47120 - id for the device.
#define SCANNER_CTRLID	2024

//(a.wilson 2012-1-11) PLID 47518
IMPLEMENT_DYNAMIC(COPOSBarcodeScannerThread, CWinThread)

// (a.walling 2012-01-17 12:55) - PLID 47120 - Don't need empty constructors / destructors

//(a.wilson 2012-1-11) PLID 47518 - constructor to pass in variables necessary for the device.
// (a.walling 2012-01-17 12:55) - PLID 47120 - Ensure this is always constructed with the params we need
// and pass them directly to the notify window
COPOSBarcodeScannerThread::COPOSBarcodeScannerThread(HWND wndNotify, const CString& strDeviceName, long nDeviceTimeout)
	: m_wndScanner(wndNotify, strDeviceName, nDeviceTimeout)
{
	m_bAutoDelete = FALSE;
}

//(a.wilson 2012-1-11) PLID 47518 - initialize the window and device, also send message that initialization was complete.
BOOL COPOSBarcodeScannerThread::InitInstance()
{
	CWinThread::InitInstance();

	try {
		m_wndScanner.Create();
		m_wndScanner.OpenDevice();
	} NxCatchAll(__FUNCTION__);

	return TRUE;
}

//(a.wilson 2012-1-11) PLID 47518 - stop and close the thread.
void COPOSBarcodeScannerThread::Shutdown()
{
	// (a.walling 2012-01-17 12:55) - PLID 47120 - This could deadlock sending the message, since the thread might be waiting on us
	// so post instead
	m_wndScanner.PostMessage(WM_CLOSE);

	//check to see what message we get back from waitforsingleobject.
	DWORD result = WaitForSingleObject(*this, 10000);
	
	if (result == WAIT_OBJECT_0) {
		delete this;
	}
	else {
		Log(__FUNCTION__" Unexpected wait result on thread! Result: 0x%08x, Error: 0x%08x", result, GetLastError());
		m_bAutoDelete = TRUE;
	}
}

////

// (a.walling 2012-01-17 12:55) - PLID 47120 - Don't need empty constructors / destructors

//(a.wilson 2012-1-11) PLID 47120
IMPLEMENT_DYNAMIC(COPOSBarcodeScanner, CWnd)

BEGIN_MESSAGE_MAP(COPOSBarcodeScanner, CWnd)
	ON_WM_CREATE()
	ON_WM_CLOSE()
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(COPOSBarcodeScanner, CWnd)
	ON_EVENT(COPOSBarcodeScanner, SCANNER_CTRLID, 1, COPOSBarcodeScanner::OnDataEvent, VTS_I4)
	ON_EVENT(COPOSBarcodeScanner, SCANNER_CTRLID, 3, COPOSBarcodeScanner::OnErrorEvent, VTS_I4 VTS_I4 VTS_I4 VTS_PI4)
END_EVENTSINK_MAP()

//(a.wilson 2012-1-11) PLID 47120 - creates the window and assigns the control to our pointer
int COPOSBarcodeScanner::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	try {
		if (CWnd::OnCreate(lpCreateStruct) == -1)
			return -1;

		AfxOleInit();

		m_wndScanner.CreateControl(__uuidof(OPOSScanner), NULL, WS_CHILD | WS_VISIBLE, CRect(0,0,0,0), this, SCANNER_CTRLID);

		m_pScanner = m_wndScanner.GetControlUnknown();

	} NxCatchAll(__FUNCTION__);

	return 0;
}

//(a.wilson 2012-1-11) PLID 47120 - kill anything left behind.
void COPOSBarcodeScanner::OnClose()
{
	try {
		if (m_pScanner) {
			m_pScanner->Close();
		}
	} NxCatchAll(__FUNCTION__);

	CWnd::OnClose();
}

//(a.wilson 2012-1-11) PLID 47120 - ensure the thread knows to quit
void COPOSBarcodeScanner::PostNcDestroy()
{
	PostQuitMessage(0);
	CWnd::PostNcDestroy();
}

//(a.wilson 2012-1-11) PLID 47120 - creates the window which calls OnCreate()
// (a.walling 2012-01-17 12:55) - PLID 47120 - All params are members of this object now
void COPOSBarcodeScanner::Create()
{
	CreateEx(NULL, AfxRegisterWndClass(0),  _T("OPOS BarcodeScanner Window"), 0, 0, 0, 0, 0, NULL, 0);
}

//(a.wilson 2012-1-11) PLID 47120 - opens the device to ensure the name is correct and makes it ready to scan.
// (a.walling 2012-01-17 12:55) - PLID 47120 - All params are members of this object now
long COPOSBarcodeScanner::OpenDevice()
{
	long nResult = OPOS_S_ERROR;	//default to a non zero in case an execption occurs it doesn't return success;

	try {
		if (m_pScanner != NULL) {
			nResult = m_pScanner->Open((LPCTSTR)m_strDeviceName);

			if (nResult == OPOS_SUCCESS) {			
				nResult = m_pScanner->ClaimDevice(m_nTimeout);
			}

			if (nResult != OPOS_SUCCESS) {
				m_pScanner->Close();
			} else {
				EnableForNextSwipe();
			}
		}
	} NxCatchAll(__FUNCTION__);
	
	// (a.walling 2012-01-17 12:55) - PLID 47120 - Notify the m_wndNotify window here rather than the thread
	::PostMessage(m_wndNotify, NXM_BARSCAN_INIT_COMPLETE, (WPARAM)nResult, 0);

	return nResult;
}

//(a.wilson 2012-1-11) PLID 47120 - when the scanner scans something this function will send the message to the appropriate window to handle.
void COPOSBarcodeScanner::OnDataEvent(long Status)
{
	try {
		if (Status == OPOS_SUCCESS) {
			long nScanDataType = m_pScanner->GetScanDataType();

			_bstr_t data = m_pScanner->GetScanDataLabel();

			UINT message = 0; 

			if (nScanDataType == SCAN_SDT_PDF417) {
				message = NXM_BARSCAN_DATA_EVENT;
			//basically check to ensure it is a 1 Dimensional barcode or unknown
			} else if (nScanDataType < 200) {
				message = WM_ENQUEUE_BARCODE;
			}
			
			if (message) {
				// (a.walling 2012-01-17 12:55) - PLID 47120 - Pass a BSTR to be freed by the recipient, same as WM_ENQUEUE_BARCODE
				// this way we can post the message rather than send, and use the same for WM_ENQUEUE_BARCODE and NXM_BARSCAN_DATA_EVENT
				::PostMessage(m_wndNotify, message, (WPARAM)Status, (LPARAM)data.Detach());
			} else {
				ASSERT(FALSE); // unsupported scan data type?
			}

			EnableForNextSwipe();
		}
	} NxCatchAll(__FUNCTION__);
}

//(a.wilson 2012-1-11) PLID 47120 - reactivates the scanner for another message.
void COPOSBarcodeScanner::EnableForNextSwipe()
{
	m_pScanner->DecodeData = VARIANT_TRUE;
	m_pScanner->DeviceEnabled = VARIANT_TRUE;
	m_pScanner->DataEventEnabled = VARIANT_TRUE;
}

//(a.wilson 2012-1-18) PLID 47120 - reports any errors while trying to read a barcode.
void COPOSBarcodeScanner::OnErrorEvent(long ResultCode, long ResultCodeExtended, long ErrorLocus, long* pErrorResponse)
{
	try {
		CString strErrorMessage, strErrorExtended;

		if (ResultCode == OPOS_E_EXTENDED) {
			strErrorExtended.Format("  Extended Error: %s", OPOS::GetMessage(ResultCodeExtended));
		}
		strErrorMessage.Format("Error: %s", OPOS::GetMessage(ResultCode));

		Log("An error occured with the barcode scanner device. %s%s", strErrorMessage, strErrorExtended);
	}NxCatchAll(__FUNCTION__);
}
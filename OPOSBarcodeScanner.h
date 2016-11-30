//(a.wilson 2012-1-11) PLID 47120 - .h for the OPOS Barcode Scanner

#pragma once
// (a.walling 2014-04-30 15:19) - PLID 61989 - import a typelibrary rather than a dll
#import "OPOSScanner.tlb"
#include "opos.h"
#include "BarcodeUtils.h"

//(a.wilson 2012-1-12) PLID 47120 - for creation of the opos device.
class COPOSBarcodeScanner : public CWnd
{
	DECLARE_DYNAMIC(COPOSBarcodeScanner)
public:
	// (a.walling 2012-01-17 12:55) - PLID 47120 - Don't need empty constructors / destructors

	// (a.walling 2012-01-17 12:55) - PLID 47120 - Ensure this is always constructed with the params we need
	COPOSBarcodeScanner(HWND wndNotify, const CString &strDeviceName, long nTimeout)
		: m_wndNotify(wndNotify)
		, m_strDeviceName(strDeviceName)
		, m_nTimeout(nTimeout)
	{
	}

	
	// (a.walling 2012-01-17 12:55) - PLID 47120 - All params are members of this object now
	void Create();
	long OpenDevice();

	// (a.walling 2012-01-17 12:55) - PLID 47120 - SetNotifyWindow unnecessary; passed in constructor

protected:
	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()

	CWnd m_wndScanner;

	OposScanner_CCO::IOPOSScannerPtr m_pScanner;

	// (a.walling 2012-01-17 12:55) - PLID 47120 - All params are members of this object now
	CString m_strDeviceName;
	long m_nTimeout;
	HWND m_wndNotify;

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClose();
	
	virtual void PostNcDestroy();
	void OnDataEvent(long Status);
	void OnErrorEvent(long ResultCode, long ResultCodeExtended, long ErrorLocus, long* pErrorResponse);
	void EnableForNextSwipe();
};

//(a.wilson 2012-1-11) PLID 47518 - create a thread object to handle the creation and destruction of the device.
class COPOSBarcodeScannerThread : public CWinThread
{
	DECLARE_DYNAMIC(COPOSBarcodeScannerThread)
public:
	// (a.walling 2012-01-17 12:55) - PLID 47120 - Don't need empty constructors / destructors

	// (a.walling 2012-01-17 12:55) - PLID 47120 - Ensure this is always constructed with the params we need
	// and pass them directly to the notify window
	COPOSBarcodeScannerThread(HWND wndNotify, const CString& strDeviceName, long nDeviceTimeout);

	virtual BOOL InitInstance();
	void Shutdown();

protected:
	COPOSBarcodeScanner m_wndScanner;
};

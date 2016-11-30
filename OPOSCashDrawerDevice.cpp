// OPOSCashDrawerDevice.cpp: implementation of the COPOSCashDrawerDevice class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "OPOSCashDrawerDevice.h"
#include "OPOSPtr.h"
#include "OPOS.h"
#include "AuditTrail.h"

// (a.walling 2010-01-21 16:43) - PLID 37025 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



// (a.walling 2007-05-15 17:16) - PLID 9801 - This CWnd-derived class is a container for the OPOSCashDrawer
// COM object. It provides functions to interface with the cash drawer, as well as automated auditing for
// manual open/close events and regular close events. All public functions have exception handling; protected
// functions are expected to throw their errors to their public functions.

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

BEGIN_EVENTSINK_MAP(COPOSCashDrawerDevice, CWnd)
    //{{AFX_EVENTSINK_MAP(COPOSCashDrawerDevice)
	ON_EVENT(COPOSCashDrawerDevice, IDC_CASHDRAWER_CTRL, 5 /* StatusUpdateEvent*/, OnPOSCashDrawerStatusUpdate, VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

COPOSCashDrawerDevice::COPOSCashDrawerDevice(CWnd* pParentWnd)
{
	m_pParentWnd = pParentWnd;
	m_bDrawerProgrammaticallyOpened = FALSE;
}

COPOSCashDrawerDevice::~COPOSCashDrawerDevice()
{

}

BEGIN_MESSAGE_MAP(COPOSCashDrawerDevice, CWnd)
	//{{AFX_MSG_MAP(COPOSCashDrawerDevice)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// (a.walling 2007-05-15 17:20) - PLID 9801 - Called whenever the drawer is opened or closed.
// when initially starting up, if the drawer is open, this is called as if it was just opened.
// (a.walling 2007-05-15 17:20) - PLID 26019 - Audit the initial state of the drawer if it is open.
void COPOSCashDrawerDevice::OnPOSCashDrawerStatusUpdate(long nData)
{
	try {
		// (a.walling 2007-05-15 17:15) - PLID 26019 - Audit these cash drawer events appropriately.

		if (!m_bDrawerProgrammaticallyOpened) {
			if (nData == OPOS_SUE_DRAWEROPENED) {
				// manually opened
				BOOL bInitial = FALSE;
				if (m_strLastStateString.GetLength() == 0)
					bInitial = TRUE;

				if (bInitial) {
					GetDrawerLastStateString(); // we just want to set the last state since it is empty.
					AuditEvent(-1, "", BeginNewAuditEvent(), aeiPOSCashDrawerInitiallyOpen, -1, "", "", aepHigh, aetChanged);
				} else {
					AuditEvent(-1, "", BeginNewAuditEvent(), aeiPOSCashDrawerManuallyOpened, -1, GetDrawerLastStateString(), "Drawer Manually Opened", aepHigh, aetChanged);
					m_strLastStateString = "Manually Opened";
				}
			} else if (nData == OPOS_SUE_DRAWERCLOSED) {
				// manually closed
				AuditEvent(-1, "", BeginNewAuditEvent(), aeiPOSCashDrawerManuallyClosed, -1, GetDrawerLastStateString(), "Drawer Manually Closed", aepMedium, aetChanged);
				m_strLastStateString = "Manually Closed";
			}
		} else {
			if (nData == OPOS_SUE_DRAWERCLOSED) {
				m_bDrawerProgrammaticallyOpened = FALSE;
				AuditEvent(-1, "", BeginNewAuditEvent(), aeiPOSCashDrawerClosed, -1, GetDrawerLastStateString(), "Drawer Closed", aepLow, aetChanged);
				m_strLastStateString = "Closed";
			} else if (nData == OPOS_SUE_DRAWEROPENED) {
				m_strLastStateString = "Open";
			}
		}
	} NxCatchAll("Error in OnPOSCashDrawerStatusUpdate");
}

// (a.walling 2007-05-15 17:21) - PLID 9801 - connect to given device name
BOOL COPOSCashDrawerDevice::InitiatePOSCashDrawerDevice(CString strDeviceName)
{
	try {
		if (!ClosePOSCashDrawer())
			return FALSE;

		//Create control
		if (!CreateOPOSCashDrawerWindow())
			return FALSE;

		if (!Open(strDeviceName))
			return FALSE;

		if (!Claim())
			return FALSE;

		return TRUE;
	} NxCatchAll("Error in InitiatePOSCashDrawerDevice");

	return FALSE;
}

// close and release the device
BOOL COPOSCashDrawerDevice::ClosePOSCashDrawer() // close and release the device
{
	try {
		OposCashDrawer_1_11_Lib::IOPOSCashDrawerPtr pCashDrawer = GetControlUnknown();

		if (pCashDrawer) {
			if (!Release())
				return FALSE;

			int nResult = pCashDrawer->Close();
			
			if (nResult != OPOS_SUCCESS) {
				CString strError;
				strError.Format("Close Failed with error %li", nResult);				
				MsgBox(strError);
				return FALSE;
			}	

			CWnd *pWnd = GetDlgItem(IDC_CASHDRAWER_CTRL);
			if (pWnd) {
 				pWnd->DestroyWindow();
			}
		}

		return TRUE;
	}NxCatchAll("Error in ClosePOSCashDrawer");
	return FALSE;
}

// open the device
BOOL COPOSCashDrawerDevice::Open(CString &strDeviceName) // open the drawer
{
	OposCashDrawer_1_11_Lib::IOPOSCashDrawerPtr pCashDrawer = GetControlUnknown();

	if (pCashDrawer) {
		int nResult = pCashDrawer->Open(_bstr_t(strDeviceName));

		if (nResult == OPOS_SUCCESS)
			return TRUE;
		else {
			// (a.walling 2007-09-28 10:16) - PLID 27556 - Added informative error message
			MessageBox(FormatString("Cash Drawer device open failed - %s", OPOS::GetMessage(nResult)), NULL, MB_OK|MB_ICONASTERISK);
			return FALSE;
		}
	}

	return FALSE;
}

// claim exclusive access to the device
BOOL COPOSCashDrawerDevice::Claim()
{
	CString strError;
	OposCashDrawer_1_11_Lib::IOPOSCashDrawerPtr pCashDrawer = GetControlUnknown();

	// (a.walling 2011-02-09 16:53) - PLID 42406 - Just use a default timeout of 10s; can be overridden if necessary from a registry key
	int nResult = pCashDrawer->ClaimDevice(AfxGetApp()->GetProfileInt("Settings", "OPOS_ClaimDevice_Timeout", 10000));
		
	if (nResult == OPOS_E_TIMEOUT) {
		MsgBox("Claim Device Timed Out");
		ClosePOSCashDrawer();
		return FALSE;
	}
	else if (nResult != OPOS_SUCCESS) {
		strError.Format("The cash drawer could not be claimed.\n Please check that no one else is accessing it, then try manually starting the device from Tools->POS Tools->Cash Drawer Settings.");	
		MsgBox(strError);
		ClosePOSCashDrawer();
		return FALSE;
	}
	else {
		pCashDrawer->PutDeviceEnabled(TRUE);

		if (VARIANT_TRUE == pCashDrawer->GetDeviceEnabled())
			return TRUE;
		else {
			strError.Format("The cash drawer could not be enabled.\n Please check that no one else is accessing it, and that it is connected through the receipt printer if necessary. Then try manually starting the device from Tools->POS Tools->Cash Drawer Settings.");	
			MsgBox(strError);
			ClosePOSCashDrawer();
			return FALSE;
		}
	}
}

// create the device
BOOL COPOSCashDrawerDevice::CreateOPOSCashDrawerWindow()
{
	try {
		if (m_pParentWnd) {
			// Create the device object
			if (CreateControl(__uuidof(OposCashDrawer_1_11_Lib::OPOSCashDrawer), NULL, WS_CHILD, CRect(0,0,0,0), m_pParentWnd, IDC_CASHDRAWER_CTRL)) {
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
	}NxCatchAllThrow("Error in COPOSCashDrawerDevice::CreateOPOSCashDrawerWindow");
	return FALSE;
}

// release access to the device.
BOOL COPOSCashDrawerDevice::Release()
{
	OposCashDrawer_1_11_Lib::IOPOSCashDrawerPtr pCashDrawer = GetControlUnknown();

	if (pCashDrawer) {
		pCashDrawer->PutDeviceEnabled(FALSE);
			
		long nResult;

		if (pCashDrawer->Claimed) {
			nResult = pCashDrawer->ReleaseDevice();
			
			if (nResult != OPOS_SUCCESS) {
				CString strError;
				strError.Format("Release Device Failed with error %li", nResult);				
				MsgBox(strError);
				return FALSE;
			}
		}
	}

	return TRUE;
}


//public functions

CString COPOSCashDrawerDevice::GetDrawerLastStateString() // the last state of the drawer (in string mode) for auditing.
{
	if (m_strLastStateString.GetLength() == 0) {
		m_strLastStateString.Format("Drawer Initially %s", GetDrawerOpened() ? "Open" : "Closed");
	}

	return m_strLastStateString;
}

long COPOSCashDrawerDevice::OpenDrawer() // physically open the drawer
{
	try {
		OposCashDrawer_1_11_Lib::IOPOSCashDrawerPtr pDrawer = GetControlUnknown();

		if (pDrawer) {
			BOOL bOpen = pDrawer->GetDrawerOpened() == VARIANT_TRUE;
			if (!bOpen) {
				// the drawer was initially closed, so set our state
				m_bDrawerProgrammaticallyOpened = TRUE;
			} else {
				// even though the drawer was already open, since it is being opened again
				// we will set this flag.
				m_bDrawerProgrammaticallyOpened = TRUE;
			}
			// (a.walling 2007-09-28 10:02) - PLID 27556 - If the cash drawer fails to open, display a message box with the result
			long nResult = pDrawer->OpenDrawer();
			if (nResult != OPOS_SUCCESS) {
				MessageBox(FormatString("Failed to open cash drawer - %s", OPOS::GetMessage(nResult)), NULL, MB_OK|MB_ICONASTERISK);
			}

			return nResult;
		}
	} NxCatchAll("Error opening cash drawer");

	return 0;
}

BOOL COPOSCashDrawerDevice::GetDrawerOpened() // retrieve whether the drawer is open or not
{
	try {
		OposCashDrawer_1_11_Lib::IOPOSCashDrawerPtr pDrawer = GetControlUnknown();

		if (pDrawer) {
			return pDrawer->GetDrawerOpened() == VARIANT_TRUE;
		}

	} NxCatchAll("Error getting cash drawer state");

	return FALSE;
}

// all these params default to 0
// blocks until the drawer is closed.
long COPOSCashDrawerDevice::WaitForDrawerClose (long BeepTimeout, long BeepFrequency, long BeepDuration, long BeepDelay)
{
	try {
		OposCashDrawer_1_11_Lib::IOPOSCashDrawerPtr pDrawer = GetControlUnknown();

		if (pDrawer) {
			return pDrawer->WaitForDrawerClose(BeepTimeout, BeepFrequency, BeepDuration, BeepDelay);
		}
	} NxCatchAll("Error waiting for cash drawer to close");

	return 0;
}
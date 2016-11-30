// OPOSCashDrawerDevice.h: interface for the COPOSCashDrawerDevice class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OPOSCASHDRAWERDEVICE_H__FFB07BE9_5264_407D_81A0_AA14137CC02C__INCLUDED_)
#define AFX_OPOSCASHDRAWERDEVICE_H__FFB07BE9_5264_407D_81A0_AA14137CC02C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// http://msdn2.microsoft.com/en-us/library/t2zechd4(VS.80).aspx
// using a high range to decrease the likelihood of collisions.
#define IDC_CASHDRAWER_CTRL	0xD000

#define OPOS_SUE_DRAWEROPENED 1
#define OPOS_SUE_DRAWERCLOSED 0

// (a.walling 2014-04-30 15:19) - PLID 61989 - import a typelibrary rather than a dll
#import "OPOSCashDrawer.tlb"
// (a.walling 2007-09-28 10:01) - PLID 26019
#include "opos.h" // So anywhere that includes this file has access to the result codes

// (a.walling 2007-05-15 17:17) - PLID 9801 - Cash drawer device class
class COPOSCashDrawerDevice : public CWnd  
{
public:
	COPOSCashDrawerDevice(CWnd* pParentWnd);
	~COPOSCashDrawerDevice();

	BOOL InitiatePOSCashDrawerDevice(CString strDeviceName); // connect to given device name
	BOOL ClosePOSCashDrawer(); // close and release the device

	long OpenDrawer(); // physically open the drawer
	BOOL GetDrawerOpened(); // retrieve whether the drawer is open or not

	// (a.walling 2007-05-15 17:18) - PLID 26019
	CString GetDrawerLastStateString(); // the last state of the drawer (in string mode) for auditing.

	// blocks until the drawer is closed.
	long WaitForDrawerClose (long BeepTimeout = 0, long BeepFrequency = 0, long BeepDuration = 0, long BeepDelay = 0);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COPOSCashDrawerDevice)
	//}}AFX_VIRTUAL
protected:
	BOOL CreateOPOSCashDrawerWindow(); // create the device

	BOOL Open(CString &strDeviceName); // open the device
	BOOL Claim(); // claim exclusive access to the device
	BOOL Release(); // release access to the device.

	BOOL m_bDrawerProgrammaticallyOpened; // TRUE if the drawer was opened via the program (not a person)
	CString m_strLastStateString;

	CWnd *m_pParentWnd; // our parent window (should be == GetMainFrame())

	//{{AFX_MSG(COPOSCashDrawerDevice)
	// NOTE - the ClassWizard will add and remove member functions here.
	afx_msg void OnPOSCashDrawerStatusUpdate(long nData);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_OPOSCASHDRAWERDEVICE_H__FFB07BE9_5264_407D_81A0_AA14137CC02C__INCLUDED_)

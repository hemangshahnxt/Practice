#if !defined(AFX_OPOSMSRDEVICE_H__99A9EAFA_9BC2_41B3_9B7E_2160D9EDDFC7__INCLUDED_)
#define AFX_OPOSMSRDEVICE_H__99A9EAFA_9BC2_41B3_9B7E_2160D9EDDFC7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OPOSMSRDevice.h : header file
//

// (a.walling 2014-04-30 15:19) - PLID 61989 - import a typelibrary rather than a dll
#import "OPOSMSR.tlb"

// (d.thompson 2010-12-20) - PLID 40314 - Made this a generically global function, not tied to this dialog.
void GetCreditCardTypeFromNumber(CString strAccountNumber, CString &strDescription, CString &strCD);

// (j.jones 2009-06-19 10:55) - PLID 33650 - added enum for supported card types
enum MSRCardType {

	msrUnknown = -1,
	msrDriversLicense = 0,
	msrCreditCard,
	msrOHIPHealthCard,
};

struct DriversLicenseInfo {
	// Track 1
	CString m_strState, m_strCity, m_strFirstName, m_strLastName, m_strSuffix, m_strAddress1, m_strAddress2;
	// Track 2
	CString m_strISOIIN, m_strDLIDNum, m_strExpDate;
	COleDateTime m_dtBirthdate;
	// Track 3
	CString m_strPostalCode, m_strSex, m_strHeight, m_strWeight, m_strHairColor, m_strEyeColor;

	DriversLicenseInfo() {
		m_strState = "";
		m_strCity = "";
		m_strFirstName = "";
		m_strLastName = "";
		m_strSuffix = "";
		m_strAddress1 = "";
		m_strAddress2 = "";
		m_strISOIIN = ""; 
		m_strDLIDNum = "";
		m_strExpDate = "";
		m_dtBirthdate = COleDateTime(1899,12,30,0,0,0);
		m_strPostalCode = "";
		m_strSex = "";
		m_strHeight = "";
		m_strWeight = "";
		m_strHairColor = "";
		m_strEyeColor = "";	
	}

	DriversLicenseInfo(CString strState, CString strCity, CString strFirstName, CString strLastName, CString strSuffix, CString strAddress1, CString strAddress2,
					CString strISOIIN, CString strDLIDNum, CString strExpDate, COleDateTime dtBirthdate,
					CString strPostalCode, CString strSex, CString strHeight, CString strWeight, CString strHairColor, CString strEyeColor) {
		m_strState = strState;
		m_strCity = strCity;
		m_strFirstName = strFirstName;
		m_strLastName = strLastName;
		m_strSuffix = strSuffix;
		m_strAddress1 = strAddress1;
		m_strAddress2 = strAddress2;
		m_strISOIIN = strISOIIN; 
		m_strDLIDNum = strDLIDNum;
		m_strExpDate = strExpDate;
		m_dtBirthdate = dtBirthdate;
		m_strPostalCode = strPostalCode;
		m_strSex = strSex;
		m_strHeight = strHeight;
		m_strWeight = strWeight;
		m_strHairColor = strHairColor;
		m_strEyeColor = strEyeColor;	
	}
};

struct CreditCardInfo {
	CString m_strCardNum, m_strFirstName, m_strMiddleInitial, m_strLastName, m_strSuffix, m_strTitle, m_strCardType, m_strCD;
	COleDateTime m_dtExpDate;

	CreditCardInfo() {
		m_strCardNum = "";
		m_strFirstName = "";
		m_strMiddleInitial = "";
		m_strLastName = "";
		m_strSuffix = "";
		m_strTitle = "";
		m_strCardType = "";
		m_strCD = "";
		m_dtExpDate = COleDateTime(1899,12,30,0,0,0);
	}

	CreditCardInfo(CString strCardNum, CString strFirstName, CString strMiddleInitial, CString strLastName, CString strSuffix, 
					CString strTitle, CString strCardType, CString strCD, COleDateTime dtExpDate) {
		m_strCardNum = strCardNum;
		m_strFirstName = strFirstName;
		m_strMiddleInitial = strMiddleInitial;
		m_strLastName = strLastName;
		m_strSuffix = strSuffix;
		m_strTitle = strTitle;
		m_strCardType = strCardType;
		m_strCD = strCD;
		m_dtExpDate = dtExpDate;
	}
};

// (j.jones 2009-06-19 09:50) - PLID 33650 - supported OHIP health cards
struct OHIPHealthCardInfo {
	CString m_strFirstName, m_strMiddleName, m_strLastName;
	CString m_strHealthCardNum, m_strVersionCode;
	COleDateTime m_dtBirthDate;
	CString m_strSex;

	OHIPHealthCardInfo() {
		m_strFirstName = "";
		m_strMiddleName = "";
		m_strLastName = "";
		m_strHealthCardNum = "";
		m_strVersionCode = "";
		m_dtBirthDate = COleDateTime(1899,12,30,0,0,0);
		m_strSex = "";
	}

	OHIPHealthCardInfo(CString strFirstName, CString strMiddleName, CString strLastName,
			CString strHealthCardNum, CString strVersionCode, COleDateTime dtBirthDate,
			CString strSex) {

		m_strFirstName = strFirstName;
		m_strMiddleName = strMiddleName;
		m_strLastName = strLastName;
		m_strHealthCardNum = strHealthCardNum;
		m_strVersionCode = strVersionCode;
		m_dtBirthDate = dtBirthDate;
		m_strSex = strSex;
	}
};


struct MSRTrackInfo {
	// (j.jones 2009-06-19 10:56) - PLID 33650 - tracks what type of card was swiped
	MSRCardType msrCardType;
	CString strTrack1;
	CString strTrack2;
	CString strTrack3;
};

#define IDC_MSR_CTRL	1000

// (a.wetta 2007-07-06 13:40) - PLID 26547 - Messages sent between the mainfrm and the MSR thread
// The WM_MSR_DATA_EVENT is sent from the MSR device thread whenever a card is swiped.  The WPARAM contains
// the MSRTrackInfo.  If the card swiped was a credit card then the LPARAM contains CreditCardInfo.
#define WM_MSR_DATA_EVENT		WM_USER + 20301
// This message is sent to mainfrm from the MSR thread to say that it's
// done initializing
// (a.walling 2007-09-28 10:45) - PLID 27556 - LPARAM is the last result of an open request
#define WM_MSR_INIT_COMPLETE	WM_USER + 20302
// The MSR configuration dialog sends this message to the MSR thread to request
// its status.  The MSR thread then sends the STATUS_INTO message back to the mainfrm
#define WM_MSR_REQUEST_STATUS	WM_USER + 20303
#define WM_MSR_STATUS_INFO		WM_USER + 20304
// This message is sent to the MSR thread to tell it to close and then 
// reopen the MSR device with the currently saved settings
#define WM_MSR_RESTART			WM_USER + 20305

// The status that can be sent in the STATUS_INFO and INIT_COMPLETE messages
#define MSR_DEVICE_ON			1
#define MSR_DEVICE_OFF			2
#define MSR_DEVICE_ERROR		3

/////////////////////////////////////////////////////////////////////////////
// OPOSMSRWindow class

// (a.wetta 2007-07-05 09:11) - PLID 26547 - The lone window which is on the thread that contains the MSR device

class COPOSMSRWindow : public CFrameWnd
{

public:
	// (a.walling 2007-09-28 13:24) - PLID 26547 - Pass in the thread
	COPOSMSRWindow(HWND hwnd, COPOSMSRThread* pThread, BOOL m_bReturnInitCompleteMsg = FALSE);
	~COPOSMSRWindow();

	void OnRequestStatus();
	void OnRestart();

	BOOL m_bSupressMsgBoxes;

protected:
	COPOSMSRDevice *m_pMSRDevice;
	// (a.walling 2007-09-28 13:59) - PLID 26547
	COPOSMSRThread *m_pThread;

	HWND m_hwndNotify; // HWND of the window to send messages to

	afx_msg LRESULT OnMSRDataEvent(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

};

/////////////////////////////////////////////////////////////////////////////
// OPOSMSRThread class

// (a.wetta 2007-07-05 09:12) - PLID 26547 - The MSR device now runs in its own thread.  This is to avoid problems when the swiper 
// locks up and then in turn locks up Practice.  By being in its own thread, if it locks up, it won't affect Practice.

class COPOSMSRThread : public CWinThread
{

	DECLARE_DYNCREATE (COPOSMSRThread)

public:
	// constructor
	COPOSMSRThread();
	// destructor
	~COPOSMSRThread();

	virtual BOOL InitInstance();

	virtual BOOL PreTranslateMessage(MSG *pMsg);

	HWND m_hwndNotify; // HWND of the window to send messages to
	BOOL m_bReturnInitCompleteMsg;

	// (a.walling 2007-09-28 13:31) - PLID 26547 - Name of the MSR device
	CString m_strDeviceName;

	// (a.walling 2007-09-28 13:51) - PLID 26547 - Synchronize access to the device name
	CCriticalSection m_cs;
	CString GetDeviceName();
	void SetDeviceName(CString str);

protected:

};

/////////////////////////////////////////////////////////////////////////////
// COPOSMSRDevice window

class COPOSMSRDevice : public CWnd
{
public:
	COPOSMSRDevice(CWnd *pParentWnd);
	// (a.walling 2007-09-28 13:28) - PLID 26547 - use const
	COPOSMSRDevice(CWnd *pParentWnd, const CString &strMSRDeviceName);
	virtual ~COPOSMSRDevice();

	// (a.walling 2007-09-28 13:25) - PLID 26547 - Use const
	BOOL InitiateMSRDevice(const CString &strMSRDeviceName); 
	BOOL CloseOPOSMSRDevice();

	DriversLicenseInfo GetDriversLicenseInfoFromMSR();
	CreditCardInfo GetCreditCardInfoFromMSR();
	// (j.jones 2009-06-19 09:50) - PLID 33650 - supported OHIP health cards
	OHIPHealthCardInfo GetOHIPHealthCardInfoFromMSR();

	BOOL GetMSRTrackInfo(CString &strTrack1, CString &strTrack2, CString &strTrack3);

	HRESULT GetMSRState();

	// (a.walling 2007-09-28 10:43) - PLID 27556 - Result from last Open call
	long m_nOpenResult;

	// (a.wetta 2007-03-15 15:25) - This function must be called after you're done processing a swipe with the functions
	// below, if not the MSR will not read any new swipes.
	void EnableMSRForNextSwipe();

	BOOL VerifyCreditCardNumber(CString strAccountNumber);

	// (a.wetta 2007-07-05 09:14) - PLID 26547 - This function is now static so that it can be called from Practice without having
	// an MSR device object
	static DriversLicenseInfo ParseDriversLicenseInfoFromMSRTracks(CString strTrack1 = "", CString strTrack2 = "", CString strTrack3 = "");

	static CreditCardInfo ParseCreditCardInfoFromMSRTracks(CString strTrack1 = "", CString strTrack2 = "", CString strTrack3 = "");

	// (j.jones 2009-06-19 09:50) - PLID 33650 - supported OHIP health cards
	static OHIPHealthCardInfo ParseOHIPHealthCardInfoFromMSRTracks(CString strTrack1 = "", CString strTrack2 = "", CString strTrack3 = "");

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COPOSMSRDevice)
	//}}AFX_VIRTUAL

protected:
	CWnd *m_pParentWnd;

	// (a.wetta 2007-03-15 15:24) - These functions initiate the MSR device/
	BOOL CreateOPOSMSRControlWindow();
	// (a.walling 2007-09-28 13:28) - PLID 26547 - use const
	BOOL CreateAndPrepareOPOSMSRControl(const CString &strMSRDeviceName);
	
	static BOOL GetStringField(IN CString strString, IN long nFieldStart, IN BOOL bFixedLength, IN CString strFixedLengthOrFieldSeparator, 
					OUT CString &strField, OUT long &nNextFieldStart) ;

	//{{AFX_MSG(COPOSMSRDevice)
		afx_msg void OnDataEventMsr(long Status);
		DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPOSMSRDEVICE_H__99A9EAFA_9BC2_41B3_9B7E_2160D9EDDFC7__INCLUDED_)

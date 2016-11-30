// KeyboardCardSwipeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "KeyboardCardSwipeDlg.h"
#include "OPOSMSRDevice.h"
#include "OHIPUtils.h" 


// CKeyboardCardSwipeDlg dialog

IMPLEMENT_DYNAMIC(CKeyboardCardSwipeDlg, CNxDialog)

CKeyboardCardSwipeDlg::CKeyboardCardSwipeDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CKeyboardCardSwipeDlg::IDD, pParent)
{
	m_strTrack1 = ""; 
	m_strTrack2 = ""; 
	m_strTrack3 = "";	
}

CKeyboardCardSwipeDlg::~CKeyboardCardSwipeDlg()
{
}


void CKeyboardCardSwipeDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDCANCEL, m_btnCancel); 
	DDX_Control(pDX, IDOK, m_btnOK); 
	DDX_Control(pDX, IDC_CARD_INPUT, m_nxeditKeyboardInput); 
}


BEGIN_MESSAGE_MAP(CKeyboardCardSwipeDlg, CNxDialog)
END_MESSAGE_MAP()


// CKeyboardCardSwipeDlg message handlers

BOOL CKeyboardCardSwipeDlg::OnInitDialog()
{
	try{
		CNxDialog::OnInitDialog(); 
		CNxDialog::CenterWindow(); 

		// (b.spivey, October 04, 2011) - PLID 40567 - Make sure that the edit control has focus. 
		m_nxeditKeyboardInput.SetFocus(); 
		m_btnCancel.AutoSet(NXB_CANCEL); 
		m_btnOK.AutoSet(NXB_OK); 

		return TRUE; 
	}NxCatchAll(__FUNCTION__);
	return FALSE; 
}

void CKeyboardCardSwipeDlg::OnOK()
{
	// (b.spivey, November 18, 2011) - PLID 40567 - Added try/catch. 
	try {
		// (b.spivey, October 04, 2011) - PLID 40567 - We grab the card input from the edit control. 
		CString str; 
		m_nxeditKeyboardInput.GetWindowTextA(str);

		if(str.GetLength() < 0 || str.Find("%") != 0){
			MessageBox("Practice could not validate the card you swiped, please swipe again.", "Swipe Failed", MB_ICONWARNING|MB_OK); 
			m_nxeditKeyboardInput.SetWindowTextA("");
			m_nxeditKeyboardInput.SetFocus(); 
			return; 
		}

		// (b.spivey, October 04, 2011) - PLID 40567 - Parse it out. Using the Idtech IDMB-334133B the sentinel start for the 
		//		first track was %, the sentinel start for the second track was ;, and the sentinel start for the third track was ;. 
		ParseTrack("%", str, m_strTrack1); 
		ParseTrack(";", str, m_strTrack2); 
		ParseTrack(";", str, m_strTrack3); 

		// (b.spivey, October 04, 2011) - PLID 40567 - Set the MSR track info. 
		SetMSRTrackInfo(m_strTrack1, m_strTrack2, m_strTrack3); 
		// (b.spivey, October 04, 2011) - PLID 40567 - Set the CreditCardInfo.
		cciKeyboardSwipe = COPOSMSRDevice::ParseCreditCardInfoFromMSRTracks(m_strTrack1, m_strTrack2, m_strTrack3); 
		

		CNxDialog::OnOK();
	}NxCatchAll(__FUNCTION__); 

}

// (b.spivey, October 04, 2011) - PLID 40567 - We send in the card input (the entire string) and the track we want to parse. 
// (b.spivey, November 18, 2011) - PLID 40567 - Optimization
void CKeyboardCardSwipeDlg::ParseTrack(const CString &strSentinelStart, IN OUT CString &strCardInput, OUT CString &strTrack)
{
	//Start of the track is a pre-defined sentinel value
	int nTrackStart = strCardInput.Find(strSentinelStart, 0); 
	//The end of the track is a question mark. 
	int nTrackEnd = strCardInput.Find("?", 0); 
	//Check for valid start/end values. 
	if(nTrackStart < nTrackEnd && nTrackStart != -1 && nTrackEnd != -1){
		//Get the track as everything left of the end...
		// (j.luckoski 2012-04-05 11:17) - PLID 49423 - Only return data between beginning sentinel and end sentinel
		// We do not want the end sentinel returned
		strTrack = strCardInput.Left(nTrackEnd); 
		//... delete the track from the main input... 
		strCardInput.Delete(nTrackStart, nTrackEnd+1);
		//... trim off the sentinel start value. 
		strTrack = strTrack.TrimLeft(strSentinelStart);
	}
	//If we didn't find a start, or end, or if the end is less than the start, we had a bad track. Fill blank.
	else{
		strTrack = ""; 
	}
}

// (b.spivey, November 18, 2011) - PLID 40567 - Optimization
void CKeyboardCardSwipeDlg::SetMSRTrackInfo(const CString &strTrack1, const CString &strTrack2, const CString &strTrack3)
{
	// (b.spivey, October 04, 2011) - PLID 40567 - Set the tracks. 
	mtiKeyboardSwipe.strTrack1 = strTrack1; 
	mtiKeyboardSwipe.strTrack2 = strTrack2; 
	mtiKeyboardSwipe.strTrack3 = strTrack3; 

	// (b.spivey, October 04, 2011) - PLID 40567 - These are the card types we support and track. 
	if(UseOHIP()
		&& (strTrack1.Left(8).CompareNoCase("%b610054") == 0
		|| strTrack1.Left(7).CompareNoCase("B610054") == 0)) {

		mtiKeyboardSwipe.msrCardType = msrOHIPHealthCard;
	}
	if(strTrack1.Left(1) == "B"){
		mtiKeyboardSwipe.msrCardType = msrCreditCard;
	}
	else{
		mtiKeyboardSwipe.msrCardType = msrDriversLicense;
	}
}
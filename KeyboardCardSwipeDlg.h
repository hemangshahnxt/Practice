#pragma once
#include "BillingRc.h"
#include "OPOSMSRDevice.h"

// CKeyboardCardSwipeDlg dialog
// (b.spivey, October 04, 2011) - PLID 40567 - created

//This enum is for the different modes of the MSR reader. 
typedef enum
{
	emmOff = 0,
	emmOn, 
	emmKeyboard,
} EMSRMode;


class CKeyboardCardSwipeDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CKeyboardCardSwipeDlg)

public:
	CKeyboardCardSwipeDlg(CWnd* pParent);   // standard constructor
	virtual ~CKeyboardCardSwipeDlg();	
	CString m_strTrack1; 
	CString m_strTrack2; 
	CString m_strTrack3; 

	MSRTrackInfo mtiKeyboardSwipe;
	CreditCardInfo cciKeyboardSwipe;

	CNxIconButton m_btnCancel; 
	CNxIconButton m_btnOK; 

	CNxEdit m_nxeditKeyboardInput; 

// Dialog Data
	enum { IDD = IDD_KEYBOARD_CARD_SWIPER };

protected:	

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support 
	// (b.spivey, November 18, 2011) - PLID 40567 - Optimized. 
	void ParseTrack(const CString &strSentinelStart, CString &strCardInput, CString &strTrack);
	void CKeyboardCardSwipeDlg::SetMSRTrackInfo(const CString &strTrack1, const CString &strTrack2, const CString &strTrack3);
	BOOL CKeyboardCardSwipeDlg::OnInitDialog();
	afx_msg void OnOK();

	DECLARE_MESSAGE_MAP()
};

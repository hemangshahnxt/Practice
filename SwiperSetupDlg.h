#if !defined(AFX_SWIPERSETUPDLG_H__9442E5E1_3F0B_11D5_83C5_0050DA0F2D8A__INCLUDED_)
#define AFX_SWIPERSETUPDLG_H__9442E5E1_3F0B_11D5_83C5_0050DA0F2D8A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SwiperSetupDlg.h : header file
//

#include "practicerc.h"
#include "OPOSMSRDevice.h"
#include "KeyboardCardSwipeDlg.h" 

// (a.wetta 2007-03-19 16:00) - PLID 25236 - Redid class to handle OPOS MSR device

/////////////////////////////////////////////////////////////////////////////
// CSwiperSetupDlg dialog

class CSwiperSetupDlg : public CNxDialog
{
// Construction
public:
	CSwiperSetupDlg(COPOSMSRThread *pOPOSMSRThread, CWnd* pParent);   // standard constructor

	// (a.wetta 2007-07-03 13:05) - PLID 26547 - The thread which contains the OPOS MSR device
	COPOSMSRThread *m_pOPOSMSRThread;

// Dialog Data
	//{{AFX_DATA(CSwiperSetupDlg)
	enum { IDD = IDD_SWIPER_SETUP };
	NxButton	m_btnOn;
	NxButton	m_btnOff;
	NxButton	m_btnKeyboard; // (b.spivey, October 04, 2011) - PLID 40567 - New radio button for keyboard mode. 
	CNxIconButton	m_okButton;
	CNxIconButton	m_applyButton;
	CNxColor	m_bkg1;
	CNxColor	m_bkg2;
	CNxEdit	m_nxeditEditTrack1;
	CNxEdit	m_nxeditEditTrack2;
	CNxEdit	m_nxeditEditTrack3;
	CNxEdit	m_nxeditMsrDeviceName;
	CNxStatic	m_nxstaticMsrStatus;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSwiperSetupDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT OnMSRDataEvent(WPARAM wParam, LPARAM lParam);
	virtual LRESULT OnMSRStatusInfo(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	void ApplySettings();

	void Save();

	// Generated message map functions
	//{{AFX_MSG(CSwiperSetupDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnApply();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SWIPERSETUPDLG_H__9442E5E1_3F0B_11D5_83C5_0050DA0F2D8A__INCLUDED_)

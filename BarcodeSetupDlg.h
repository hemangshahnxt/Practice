#if !defined(AFX_BARCODESETUPDLG_H__0DAE2261_2DA4_11D5_83C5_0050DA0F2D8A__INCLUDED_)
#define AFX_BARCODESETUPDLG_H__0DAE2261_2DA4_11D5_83C5_0050DA0F2D8A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BarcodeSetupDlg.h : header file
//
#include "OPOSBarcodeScanner.h"	//(a.wilson 2012-1-11) PLID 47486

/////////////////////////////////////////////////////////////////////////////
// CBarcodeSetupDlg dialog

class CBarcodeSetupDlg : public CNxDialog
{
// Construction
public:
	CBarcodeSetupDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CBarcodeSetupDlg)
	enum { IDD = IDD_BARCODE_SETUP_DLG };
	NxButton	m_btnOff;
	NxButton	m_btnOn;
	CString	m_strPort;
	CString m_strParity;
	CString m_strStopBits;
	CString	m_strTest;
	int		m_radioToggle;
	CNxEdit	m_nxeditEditTest;
	CNxIconButton	m_btnClose;
	bool m_bUseOPOSBarcodeScanner, m_bOPOSBarcodeScannerInitComplete;	//(a.wilson 2012-1-11) PLID 47486
	COPOSBarcodeScannerThread *m_pOPOSBarcodeScannerThread;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBarcodeSetupDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT OnBarcodeScan(WPARAM wParam, LPARAM lParam);
	virtual LRESULT OnBarcodeScannerDataEvent(WPARAM wParam, LPARAM lParam);	//(a.wilson 2012-1-11) PLID 47486
	virtual LRESULT OnBarcodeScannerInitComplete(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	void ApplySettings();
	void CheckOPOSStatus();	//(a.wilson 2012-1-11) PLID 47486

	// Generated message map functions
	//{{AFX_MSG(CBarcodeSetupDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnRadioOff();
	afx_msg void OnRadioOn();
	afx_msg void OnSelendokComboStopbits();
	afx_msg void OnSelendokComboParity();
	afx_msg void OnSelendokComboPort();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOposOff();
	afx_msg void OnBnClickedOposOn();
	afx_msg void OnBnClickedOposTest();
	afx_msg void OnEnKillfocusOposName();
	afx_msg void OnEnKillfocusOposTimeout();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BARCODESETUPDLG_H__0DAE2261_2DA4_11D5_83C5_0050DA0F2D8A__INCLUDED_)

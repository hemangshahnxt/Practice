#if !defined(AFX_POSCASHDRAWERCONFIGDLG_H__DB1593FA_EA29_40D0_94B8_770C2D6BCAE3__INCLUDED_)
#define AFX_POSCASHDRAWERCONFIGDLG_H__DB1593FA_EA29_40D0_94B8_770C2D6BCAE3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// POSCashDrawerConfigDlg.h : header file
//

#include "OPOSCashDrawerDevice.h"

/////////////////////////////////////////////////////////////////////////////
// POSCashDrawerConfigDlg dialog

// (a.walling 2007-05-17 16:01) - 26058 - Make a configuration dialog for the cash drawer

class POSCashDrawerConfigDlg : public CNxDialog
{
// Construction
public:
	POSCashDrawerConfigDlg(COPOSCashDrawerDevice* pPOSCashDrawerDevice, CWnd* pParent);   // standard constructor

	inline COPOSCashDrawerDevice* GetPOSCashDrawerDevice() { return m_pPOSCashDrawerDevice;}

// Dialog Data
	//{{AFX_DATA(POSCashDrawerConfigDlg)
	enum { IDD = IDD_POS_CASHDRAWER_CONFIG_DLG };
	NxButton	m_btnOff;
	NxButton	m_btnOn;
	NxButton	m_btnOpenTips;
	NxButton	m_btnOpenRefunds;
	NxButton	m_btnOpenChecks;
	NxButton	m_btnOpenCharge;
	CNxIconButton	m_btnClose;
	CNxEdit	m_nxeditEditCashdrawerName;
	CNxStatic	m_nxstaticCashdrawerStatus;
	NxButton	m_btnPosCashdrawerAdminOptions;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(POSCashDrawerConfigDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	BOOL Apply();
	BOOL Test(BOOL bPromptToOpen);

	CString m_strCashDrawerName;
	BOOL m_bUseCashDrawer;

	BOOL m_bNeedToTest;

	BOOL m_bManualOpen; // (a.walling 2007-09-21 09:16) - PLID 27468 - == bioCashDrawers' sptDynamic0 permission

	COPOSCashDrawerDevice* m_pPOSCashDrawerDevice;

	// Generated message map functions
	//{{AFX_MSG(POSCashDrawerConfigDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPosCashdrawerOn();
	afx_msg void OnPosCashdrawerOff();
	afx_msg void OnCashdrawerTest();
	virtual void OnOK();
	afx_msg void OnChangeEditCashdrawerName();
	afx_msg void OnOpenForChecks();
	afx_msg void OnOpenForRefunds();
	afx_msg void OnOpenForTips();
	afx_msg void OnOpenCashDrawer();
	afx_msg void OnOpenForCharge();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_POSCASHDRAWERCONFIGDLG_H__DB1593FA_EA29_40D0_94B8_770C2D6BCAE3__INCLUDED_)

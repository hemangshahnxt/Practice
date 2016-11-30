#include "afxwin.h"
#if !defined(AFX_SECONDARYANSICLAIMCONFIGDLG_H__ADB8B2FC_DA6C_4B01_B608_C8A413090FAE__INCLUDED_)
#define AFX_SECONDARYANSICLAIMCONFIGDLG_H__ADB8B2FC_DA6C_4B01_B608_C8A413090FAE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SecondaryANSIClaimConfigDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSecondaryANSIClaimConfigDlg dialog

#include "AdministratorRc.h"

class CSecondaryANSIClaimConfigDlg : public CNxDialog
{
// Construction
public:
	CSecondaryANSIClaimConfigDlg(CWnd* pParent);   // standard constructor

	long m_GroupID;
	BOOL m_bIsUB92;
	// (j.jones 2010-10-19 13:32) - PLID 40931 - we now take in the value for 5010
	BOOL m_bIs5010Enabled;

	// (j.jones 2007-06-15 09:43) - PLID 26309 - added group/reason code defaults
	// (j.jones 2009-03-11 09:32) - PLID 33446 - renamed the original combos to say "remaining",
	// and added new combos for "insurance"
	NXDATALIST2Lib::_DNxDataListPtr m_pRemainingGroupCodeList;
	NXDATALIST2Lib::_DNxDataListPtr m_pRemainingReasonList;
	NXDATALIST2Lib::_DNxDataListPtr m_pInsuranceGroupCodeList;
	NXDATALIST2Lib::_DNxDataListPtr m_pInsuranceReasonList;

	// (j.jones 2008-11-12 10:43) - PLID 31912 - added m_checkSendCASPR
// Dialog Data
	//{{AFX_DATA(CSecondaryANSIClaimConfigDlg)
	enum { IDD = IDD_SECONDARY_ANSI_CLAIM_CONFIG_DLG };
	NxButton	m_checkSendCASPR;
	NxButton	m_btnApprovedAllowed;
	NxButton	m_btnApprovedClaimTotal;
	NxButton	m_btnApprovedBalance;
	// (j.jones 2009-08-28 17:45) - PLID 32993 - removed allowable options
	/*
	NxButton	m_btnAllowedPayAndResp;
	NxButton	m_btnAllowedPayment;
	NxButton	m_btnAllowedFeeSched;
	*/
	NxButton	m_btnSend2430Adj;
	NxButton	m_btnSend2320Adj;
	NxButton	m_btnSend2400Contract;
	NxButton	m_btnSend2400Allowed;
	NxButton	m_btnSend2320Approved;
	NxButton	m_btnSend2320Allowed;
	NxButton	m_btnSend2300Contract;
	NxButton	m_btnEnablePayNotPrimary;
	CNxStatic	m_nxstatic2400Label;
	CNxStatic	m_nxstaticApprovedCalcLabel;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	// (j.jones 2010-02-02 14:00) - PLID 37159 - added ability to configure adjustments
	// to be included in the allowed amount calculation
	CNxIconButton	m_btnConfigAllowedAdjCodes;
	// (j.jones 2010-03-31 14:43) - PLID 37918 - added ability to hide 2430 SVD06 for single-charge claims
	NxButton	m_checkHide2430_SVD06_OneCharge;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSecondaryANSIClaimConfigDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSecondaryANSIClaimConfigDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnCheckEnableSendingPaymentInformation();
	virtual void OnOK();
	afx_msg void OnRadioUseAdjustmentsIn2320();
	afx_msg void OnRadioUseAdjustmentsIn2430();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	// (j.jones 2010-02-02 14:00) - PLID 37159 - added ability to configure adjustments
	// to be included in the allowed amount calculation
	afx_msg void OnBtnConfigAllowedAdjCodes();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SECONDARYANSICLAIMCONFIGDLG_H__ADB8B2FC_DA6C_4B01_B608_C8A413090FAE__INCLUDED_)

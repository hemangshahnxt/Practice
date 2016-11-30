#if !defined(AFX_CHARGEINTERESTCONFIGDLG_H__0BBB5A92_C10D_4247_B2E2_A3F9D6013009__INCLUDED_)
#define AFX_CHARGEINTERESTCONFIGDLG_H__0BBB5A92_C10D_4247_B2E2_A3F9D6013009__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChargeInterestConfigDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CChargeInterestConfigDlg dialog

class CChargeInterestConfigDlg : public CNxDialog
{
// Construction
public:
	CChargeInterestConfigDlg(CWnd* pParent);   // standard constructor

	NXDATALISTLib::_DNxDataListPtr m_UserList;

	void OnChangeFeeType();
	void OnChangePromptType();

	// (z.manning, 04/30/2008) - PLID 29850 - Added NxIconButtons
// Dialog Data
	//{{AFX_DATA(CChargeInterestConfigDlg)
	enum { IDD = IDD_CHARGE_INTEREST_CONFIG_DLG };
	NxButton	m_btnUsePercent;
	NxButton	m_btnUseFee;
	NxButton	m_btnAllChgs;
	NxButton	m_btnPtChgs;
	NxButton	m_btnIncFin;
	NxButton	m_btnIncludeExist;
	NxButton	m_btnAutoAdd;
	NxButton	m_btnPromptUser;
	NxButton	m_btnManualOnly;
	NxButton	m_btnChoosePts;
	CNxEdit	m_nxeditDaysUntilOverdue;
	CNxEdit	m_nxeditInterestInterval;
	CNxEdit	m_nxeditInterestPercentage;
	CNxEdit	m_nxeditFlatFeeAmount;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	NxButton	m_btnCalculateAsGroupbox;
	NxButton	m_btnCalculateOnGroupbox;
	NxButton	m_btnCalculateWhenGroupbox;
	// (j.jones 2009-06-10 11:29) - PLID 33834 - added flat fee apply types
	NxButton	m_radioFlatFeePerCharge;
	NxButton	m_radioFlatFeePerPatient;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChargeInterestConfigDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CChargeInterestConfigDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnRadioUsePercentage();
	afx_msg void OnRadioUseFlatFee();
	afx_msg void OnRadioAutoAddCharges();
	afx_msg void OnRadioPromptUser();
	afx_msg void OnRadioManualOnly();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHARGEINTERESTCONFIGDLG_H__0BBB5A92_C10D_4247_B2E2_A3F9D6013009__INCLUDED_)

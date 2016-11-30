#if !defined(AFX_MARKETREFERRALSPRINTSETUPDLG_H__D9ACBE7A_6E28_4D47_864F_12F7F3DE977E__INCLUDED_)
#define AFX_MARKETREFERRALSPRINTSETUPDLG_H__D9ACBE7A_6E28_4D47_864F_12F7F3DE977E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MarketReferralsPrintSetupDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMarketReferralsPrintSetupDlg dialog

class CMarketReferralsPrintSetupDlg : public CNxDialog
{
// Construction
public:
	CMarketReferralsPrintSetupDlg(CWnd* pParent = NULL, BOOL bPreview = TRUE, BOOL bEnableAll = TRUE);   // standard constructor
	BOOL m_bPreview;
	BOOL m_bEnableAll;

// Dialog Data
	//{{AFX_DATA(CMarketReferralsPrintSetupDlg)
	enum { IDD = IDD_MARKET_REFERRALS_PRINT_DLG };
	CNxIconButton m_btnPreview;
	CNxIconButton m_btnCancel;
	NxButton	m_btnPrintStyleGroupbox;
	NxButton	m_btnReportOn;
	NxButton	m_radioSummary;
	NxButton	m_radioDetailed;
	NxButton	m_radioReferralSource;
	NxButton	m_radioRefPhys;
	NxButton	m_radioRefPatient;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMarketReferralsPrintSetupDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMarketReferralsPrintSetupDlg)
	afx_msg void OnPreviewBtn();
	virtual BOOL OnInitDialog();
	afx_msg void OnDetailedRadio();
	afx_msg void OnSummaryRadio();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MARKETREFERRALSPRINTSETUPDLG_H__D9ACBE7A_6E28_4D47_864F_12F7F3DE977E__INCLUDED_)

#if !defined(AFX_STATEMENTSETUP_H__4401D7E0_4E5C_11D3_A388_00C04F42E33B__INCLUDED_)
#define AFX_STATEMENTSETUP_H__4401D7E0_4E5C_11D3_A388_00C04F42E33B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// StatementSetup.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CStatementSetup dialog

class CStatementSetup : public CNxDialog
{
// Construction
public:
	CStatementSetup(CWnd* pParent);   // standard constructor

	
// Dialog Data
	//{{AFX_DATA(CStatementSetup)
	// (j.gruber 2010-02-04 12:56) - PLID 29120 - added detail/summary option
	enum { IDD = IDD_STATEMENT_SETUP };
	NxButton	m_printMultiCheck;
	NxButton	m_Remit;
	NxButton	m_hideFooterCheck;
	NxButton	m_noDiagRadio;
	NxButton	m_useDiagRadio;
	NxButton	m_forwardRadio;
	NxButton	m_commaRadio;
	NxButton	m_practiceRadio;
	NxButton	m_providerRadio;
	NxButton	m_guarantorCheck;
	NxButton	m_btn1081;
	NxButton	m_btnDefault;
	NxButton	m_btnAvery2;
	NxButton	m_btnEStatement;
	NxButton	m_btnAcceptVisa;
	NxButton	m_btnAcceptMastercard;
	NxButton	m_btnAcceptDiscover;
	NxButton	m_btnAcceptAmex;
	NxButton	m_btnCombineBillBalances;
	NxButton	m_btnHidePrepays;
	NxButton	m_btnHideCPTCodes;
	NxButton	m_btnUsePracAddress;
	NxButton	m_btnUseProvAddress;
	NxButton	m_btnUse70Version;
	NxButton	m_btnShowLastPay;
	CNxEdit	m_nxeditEditAge;
	CNxEdit	m_nxeditPccTitle;
	CNxEdit	m_nxeditPaymentdesc;
	CNxEdit	m_nxeditAdjustmentdesc;
	CNxEdit	m_nxeditRefunddesc;
	CNxEdit	m_nxeditDaysOld;
	CNxEdit	m_nxeditExtraText;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnStmtARNotes;
	NxButton	m_btnDetailed;
	NxButton	m_btnSummary;
	NxButton	m_btnShowPatSelectWnd; // (j.dinatale 2011-03-21 11:15) - PLID 41444
	// (j.jones 2011-04-12 10:46) - PLID 31219 - added ability to show all charges
	// on any bills with balances, when on the summary statement
	NxButton	m_checkIncludePaidChargesOnBills;
	NxButton m_checkHideChargebacks; //TES 7/10/2014 - PLID 62563
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStatementSetup)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	CDialog dlgPracticeInfo,
			dlgPatientInfo,
			dlgInsInfo,
			dlgStatementInfo;

	NXDATALISTLib::_DNxDataListPtr  m_pNameList;
	NXDATALISTLib::_DNxDataListPtr  m_pLocList;
	NXDATALISTLib::_DNxDataListPtr  m_ProvFormatList;
	NXDATALISTLib::_DNxDataListPtr  m_ProvHeaderFormatList;

	// (j.gruber 2010-02-04 13:19) - PLID 29120
	void EnableDetailedButtons(BOOL bEnable);

	void MoveInfo (CRect *rect, long newLeft, long newTop);
	void WriteInfo (CString Field, long Left, long Top);
//	void InsertInfo (CNxRecordset *InRC);

	// Generated message map functions
	//{{AFX_MSG(CStatementSetup)
	virtual BOOL OnInitDialog();
	afx_msg void OnOkBtn();
	afx_msg void OnUseGuarantor();
	afx_msg void OnRemit();
	afx_msg void OnUse70Version();
	afx_msg void OnUseProvname();
	afx_msg void OnUsePracname();
	afx_msg void OnStmtArNotes();
	afx_msg void OnCombineBillBalances();
	afx_msg void OnEstatementformat();
	afx_msg void OnShowLastPay();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedDefaultformat();
	afx_msg void OnBnClicked1081();
	afx_msg void OnBnClickedAvery2();
	// (j.gruber 2010-08-20 09:58) - PLID 40193 - added warning
	afx_msg void OnBnClickedShowBillingNotesEstate();

	
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STATEMENTSETUP_H__4401D7E0_4E5C_11D3_A388_00C04F42E33B__INCLUDED_)

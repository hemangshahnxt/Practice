#if !defined(AFX_QUICKBOOKSLINK_H__C0557E9D_74CF_431E_AADD_3E57FB4B9B43__INCLUDED_)
#define AFX_QUICKBOOKSLINK_H__C0557E9D_74CF_431E_AADD_3E57FB4B9B43__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// QuickbooksLink.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CQuickbooksLink dialog

class CQuickbooksLink : public CNxDialog
{
// Construction
public:
	CQuickbooksLink(CWnd* pParent);   // standard constructor

	void OnDateRadio(BOOL bSave);
	void OnImplementationRadio();

// Dialog Data
	//{{AFX_DATA(CQuickbooksLink)
	enum { IDD = IDD_QUICKBOOKS_LINK };
	NxButton	m_checkPromptForSourceAccts;
	NxButton	m_checkExportPatients;
	NxButton	m_radioReceivePayments;
	NxButton	m_radioMakeDeposits;
	NxButton	m_radioUseServiceDate;
	NxButton	m_radioUseInputDate;
	NxButton	m_radioUseTransferDate;
	NxButton	m_radioUseDepositDate;
	NxButton	m_checkDisableQuickbooks;
	CNxEdit	m_nxeditQuickbooksFilePath;
	CNxStatic	m_nxstaticDateDescription;
	CNxIconButton	m_btnClose;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CQuickbooksLink)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	CBrush m_brush;

	// Generated message map functions
	//{{AFX_MSG(CQuickbooksLink)
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnBrowseQbData();
	afx_msg void OnCheckDisableQbLink();
	afx_msg void OnKillfocusQuickbooksFilePath();
	afx_msg void OnBtnLinkAccountsToProviders();
	afx_msg void OnRadioUseServiceDate();
	afx_msg void OnRadioUseInputDate();
	afx_msg void OnRadioUseTransferDate();
	afx_msg void OnRadioMakeDeposits();
	afx_msg void OnRadioReceivePayments();
	afx_msg void OnBtnChooseDefaultSourceDepositAccts();
	afx_msg void OnCheckExportPatients();
	afx_msg void OnBtnQbHelp();
	afx_msg void OnCheckChooseIndivSourceAccts();
	afx_msg void OnBtnConfigureDemogOutput();
	afx_msg void OnRadioUseDepositDate();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_QUICKBOOKSLINK_H__C0557E9D_74CF_431E_AADD_3E57FB4B9B43__INCLUDED_)

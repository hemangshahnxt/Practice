#if !defined(AFX_CREDITCARDPROCESSINGSETUPDLG_H__3719484E_FF01_423A_BEF8_7F7EC1E7701F__INCLUDED_)
#define AFX_CREDITCARDPROCESSINGSETUPDLG_H__3719484E_FF01_423A_BEF8_7F7EC1E7701F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CreditCardProcessingSetupDlg.h : header file
//
/////////////////////////////////////////////////////////////////////////////
// CCreditCardProcessingSetupDlg dialog
// (j.gruber 2007-08-03 13:15) - PLID 26535 - setup dialog for credit card processing
class CCreditCardProcessingSetupDlg : public CNxDialog
{
// Construction
public:
	CCreditCardProcessingSetupDlg(CWnd* pParent);   // standard constructor
	NXDATALIST2Lib::_DNxDataListPtr m_pCountryList;
	NXDATALIST2Lib::_DNxDataListPtr m_pLanguageList;

// Dialog Data
	//{{AFX_DATA(CCreditCardProcessingSetupDlg)
	enum { IDD = IDD_CREDIT_CARD_PROCESSING_SETUP_DLG };
	NxButton	m_btnShowTips;
	CComboBox	m_cmbComPort;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	CNxEdit	m_nxeditMerchantNumber;
	CNxEdit	m_nxeditClientNumber;
	CNxEdit	m_nxeditTerminalNumber;
	CNxEdit	m_nxeditChaseUsername;
	CNxEdit	m_nxeditChasePassword;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCreditCardProcessingSetupDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCreditCardProcessingSetupDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnKillfocusMerchantNumber();
	afx_msg void OnKillfocusClientNumber();
	afx_msg void OnKillfocusTerminalNumber();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnSelChosenCcProcessingCountry(LPDISPATCH lpRow);
	afx_msg void OnSendCurrentKeyRequest();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CREDITCARDPROCESSINGSETUPDLG_H__3719484E_FF01_423A_BEF8_7F7EC1E7701F__INCLUDED_)

#if !defined(AFX_QUICKBOOKSLINKACCOUNTSDLG_H__B9E630B5_E29B_43D4_B0C3_09EB5CBBD399__INCLUDED_)
#define AFX_QUICKBOOKSLINKACCOUNTSDLG_H__B9E630B5_E29B_43D4_B0C3_09EB5CBBD399__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// QuickbooksLinkAccountsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CQuickbooksLinkAccountsDlg dialog

class CQuickbooksLinkAccountsDlg : public CNxDialog
{
// Construction
public:
	CQuickbooksLinkAccountsDlg(CWnd* pParent);   // standard constructor

	NXDATALISTLib::_DNxDataListPtr m_AccountCombo, m_UnselectedProviderList, m_SelectedProviderList;

// Dialog Data
	//{{AFX_DATA(CQuickbooksLinkAccountsDlg)
	enum { IDD = IDD_QUICKBOOKS_LINK_ACCOUNTS_DLG };
	CNxIconButton	m_btnUnlinkAllProvs;
	CNxIconButton	m_btnUnlinkOneProv;
	CNxIconButton	m_btnLinkOneProv;
	CNxIconButton	m_btnLinkAllProvs;
	CNxIconButton	m_btnClose;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CQuickbooksLinkAccountsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	CBrush m_brush;

	// Generated message map functions
	//{{AFX_MSG(CQuickbooksLinkAccountsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnLinkAllProvs();
	afx_msg void OnBtnLinkOneProv();
	afx_msg void OnBtnUnlinkOneProv();
	afx_msg void OnBtnUnlinkAllProvs();
	afx_msg void OnSelChosenDepositAccountsCombo(long nRow);
	afx_msg void OnDblClickCellQbProviderUnselectedList(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellQbProviderSelectedList(long nRowIndex, short nColIndex);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_QUICKBOOKSLINKACCOUNTSDLG_H__B9E630B5_E29B_43D4_B0C3_09EB5CBBD399__INCLUDED_)

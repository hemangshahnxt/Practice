#if !defined(AFX_QUOTEADMINDLG_H__A05D5C28_16B7_11D4_A406_00C04F42E33B__INCLUDED_)
#define AFX_QUOTEADMINDLG_H__A05D5C28_16B7_11D4_A406_00C04F42E33B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// QuoteAdminDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CQuoteAdminDlg dialog

class CQuoteAdminDlg : public CNxDialog
{
// Construction
public:
	OLE_COLOR m_nColor;
	CQuoteAdminDlg(CWnd* pParent);   // standard constructor
	bool m_bUseAlternateFormat;

// Dialog Data
	//{{AFX_DATA(CQuoteAdminDlg)
	enum { IDD = IDD_QUOTE_ADMIN };
	NxButton	m_btnFlatFeeRequired;
	NxButton	m_btnPercentRequired;
	NxButton	m_btnNoneRequired;
	NxButton	m_btnUseEmail;
	NxButton	m_btnQuotesExpireDefault;
	NxButton	m_btnSeparateTax;
	NxButton	m_btnShowProcDesc;
	NxButton	m_btnShowQuoteDesc;
	NxButton	m_btnUserLetterhead;
	NxButton	m_btnUseWitnessSignature;
	NxButton	m_btnUseSignature;
	NxButton	m_btnUseQuoteDate;
	NxButton	m_btnOverridePracPhone;
	CComboBox	m_pat_coord_format;
	CComboBox	m_percent;
	CComboBox	m_quantity;
	CComboBox	m_discount;
	CComboBox	m_paid_to_others;
	CComboBox	m_pat_coord;
	CNxColor	m_bkg1;
	CNxColor	m_bkg2;
	CNxEdit	m_nxeditEditPreDetail;
	CNxEdit	m_nxeditEditPostDetail;
	CNxEdit	m_nxeditReportTitle;
	CNxEdit	m_nxeditSubrepTitle;
	CNxEdit	m_nxeditPracOtherPhone;
	CNxEdit	m_nxeditDefaultExpdays;
	CNxEdit	m_nxeditEditDepositPercentage;
	CNxEdit	m_nxeditEditDepositFlatFee;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnAdministrator;
	// (j.gruber 2009-03-31 17:59) - PLID 33349 - added new checkbox
	NxButton m_btnShowDiscountsByCategory;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CQuoteAdminDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CQuoteAdminDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnOk();
	virtual void OnCancel();
	afx_msg void OnUseletterhead();
	afx_msg void OnShowQuoteDesc();
	afx_msg void OnSelchangePatientCoordFormat();
	afx_msg void OnSelchangePatientCoord();
	afx_msg void OnUseOtherPhone();
	afx_msg void OnRadioDepositTypeChanged();
	afx_msg void OnKillfocusEditDepositPercentage();
	afx_msg void OnKillfocusEditDepositFlatFee();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_QUOTEADMINDLG_H__A05D5C28_16B7_11D4_A406_00C04F42E33B__INCLUDED_)

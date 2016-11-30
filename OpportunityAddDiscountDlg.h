#if !defined(AFX_OPPORTUNITYADDDISCOUNTDLG_H__CD033C6F_7131_4A92_9EE4_A37D64818CD6__INCLUDED_)
#define AFX_OPPORTUNITYADDDISCOUNTDLG_H__CD033C6F_7131_4A92_9EE4_A37D64818CD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OpportunityAddDiscountDlg.h : header file
//

//DRT 5-6/2007 - PLID 25892

/////////////////////////////////////////////////////////////////////////////
// COpportunityAddDiscountDlg dialog

class COpportunityAddDiscountDlg : public CNxDialog
{
// Construction
public:
	COpportunityAddDiscountDlg(CWnd* pParent);   // standard constructor

	COleCurrency m_cySubTotal;			//INPUT Parameter -- The current subtotal of the proposal.
	COleCurrency m_cyFinalDiscount;		//OUTPUT Parameter -- The final dollar value of the discount to be applied.
	long m_nDiscountUserID;				//OUTPUT Parameter -- The user ID of the authorized discounter.  This may be the current user or an override.
	CString m_strDiscountUserName;		//OUTPUT Parameter -- Username that goes with the above ID.

// Dialog Data
	//{{AFX_DATA(COpportunityAddDiscountDlg)
	enum { IDD = IDD_OPPORTUNITY_ADD_DISCOUNT_DLG };
	NxButton	m_btnPercent;
	NxButton	m_btnDollar;
	CNxLabel	m_labelPercent;
	CNxLabel	m_labelDollar;
	CNxLabel	m_labelMaxDiscount;
	CNxLabel	m_labelUser;
	CNxEdit	m_nxeditDollarValue;
	CNxEdit	m_nxeditPercentValue;
	CNxStatic	m_nxstaticLabelSubtotal;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COpportunityAddDiscountDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void EnsureControls();
	bool HandleDollarChange(double &dblPercent);
	bool HandlePercentChange(double &dblPercent);

	double m_dblMaxDiscountPercent;


	// Generated message map functions
	//{{AFX_MSG(COpportunityAddDiscountDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnRadPercent();
	afx_msg void OnRadDollar();
	afx_msg void OnChangeDollarValue();
	afx_msg void OnKillfocusDollarValue();
	afx_msg void OnChangePercentValue();
	afx_msg void OnKillfocusPercentValue();
	afx_msg void OnOverrideDiscount();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPPORTUNITYADDDISCOUNTDLG_H__CD033C6F_7131_4A92_9EE4_A37D64818CD6__INCLUDED_)

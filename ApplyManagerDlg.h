// (a.walling 2008-04-10 12:52) - tab.h is no longer used
//{{AFX_INCLUDES()
//}}AFX_INCLUDES
#if !defined(AFX_APPLYMANAGERDLG_H__2C44F543_2D95_11D3_9461_00C04F4C8415__INCLUDED_)
#define AFX_APPLYMANAGERDLG_H__2C44F543_2D95_11D3_9461_00C04F4C8415__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ApplyManagerDlg.h : header file
//

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
// using namespace NxTab;

struct ApplyItem {

	_variant_t ApplyID;				// (j.jones 2011-10-27 16:25) - PLID 46159 - renamed to be clearer
	_variant_t SourcePayID;			// (j.jones 2011-10-27 16:25) - PLID 46159 - renamed to be clearer
	_variant_t Date;
	_variant_t ChargeDesc;
	_variant_t PayDesc;
	_variant_t Amount;
	_variant_t PaymentTypeID;
	_variant_t PaymentTypeName;		// (j.jones 2015-02-24 14:28) - PLID 64940 - added payment type name
	_variant_t Ins1Amount;
	_variant_t Ins2Amount;
	_variant_t Ins3Amount;
	_variant_t ChargebackID;		//TES 7/24/2014 - PLID 62557 - Added
};

/////////////////////////////////////////////////////////////////////////////
// CApplyManagerDlg dialog

class CApplyManagerDlg : public CNxDialog
{
	NXDATALISTLib::_DNxDataListPtr m_List;
	// An array of all the bills for this patient by ID
	CDWordArray m_adwBillIDs;
	// An array of all the charges for this patient by ID
	CDWordArray m_adwChargeIDs;
	// An array of all the payments for this patient by ID
	CDWordArray m_adwPayIDs;

// Construction
public:
	CApplyManagerDlg(CWnd* pParent);   // standard constructor
	~CApplyManagerDlg();
	int m_iBillID;
	int m_iChargeID;
	int m_iPayID;
	int m_iRow;
	int m_GuarantorID1, m_GuarantorID2;

	// (j.jones 2011-03-25 15:39) - PLID 41143 - added "Applies From Payments" as the middle tab
	enum { BILL_APPLY_TAB, PAYMENT_APPLIED_FROM_TAB, PAYMENT_APPLIED_TO_TAB } m_ActiveTab;

// Dialog Data
	//{{AFX_DATA(CApplyManagerDlg)
	enum { IDD = IDD_APPLY_MANAGER_DLG };
	CComboBox	m_ComboCharges;
	CComboBox	m_ComboBills;
	CNxStatic	m_nxstaticLabelBillorpayment;
	CNxStatic	m_nxstaticLabelCharge;
	CNxStatic	m_nxstaticLabel10;
	CNxStatic	m_nxstaticLabel11;
	CNxStatic	m_nxstaticLabel12;
	CNxStatic	m_nxstaticLabel13;
	CNxStatic	m_nxstaticLabel5;
	CNxStatic	m_nxstaticLabelTtlPatresp;
	CNxStatic	m_nxstaticLabelTtlPriresp;
	CNxStatic	m_nxstaticLabelTtlSecresp;
	CNxStatic	m_nxstaticLabelTtlTerresp;
	CNxStatic	m_nxstaticLabel8;
	CNxStatic	m_nxstaticLabelPayments;
	CNxStatic	m_nxstaticLabelPriPay;
	CNxStatic	m_nxstaticLabelSecPay;
	CNxStatic	m_nxstaticLabelTerPay;
	CNxStatic	m_nxstaticLabel3;
	CNxStatic	m_nxstaticLabelAdjustments;
	CNxStatic	m_nxstaticLabelPriAdj;
	CNxStatic	m_nxstaticLabelSecAdj;
	CNxStatic	m_nxstaticLabelTerAdj;
	CNxStatic	m_nxstaticLabel4;
	CNxStatic	m_nxstaticLabelRefunds;
	CNxStatic	m_nxstaticLabelPriRef;
	CNxStatic	m_nxstaticLabelSecRef;
	CNxStatic	m_nxstaticLabelTerRef;
	CNxStatic	m_nxstaticLabel18;
	CNxStatic	m_nxstaticLabelPatresp;
	CNxStatic	m_nxstaticLabelPriresp;
	CNxStatic	m_nxstaticLabelSecresp;
	CNxStatic	m_nxstaticLabelTerresp;
	CNxStatic	m_nxstaticLabel2;
	CNxStatic	m_nxstaticLabelCharges;
	CNxStatic	m_nxstaticLabel1;
	CNxStatic	m_nxstaticLabelBalance;
	CNxStatic	m_nxstaticPatNameLabel;
	CNxIconButton	m_btnUnapply;
	CNxIconButton	m_btnApplyNew;
	CNxIconButton	m_btnOK;
	NxButton	m_btnGroup1;
	NxButton	m_btnDisplayApplies;			// (d.thompson 2010-06-15) - PLID 39164
	NxButton	m_btnDisplayPayments;			// (d.thompson 2010-06-15) - PLID 39164
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CApplyManagerDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	NxTab::_DNxTabPtr m_tab;

	// Builds the bill and charge combo boxes
	void BuildComboBoxes();

	// These functions are called by BuildComboBoxes
	void BuildBillsCombo();
	void BuildChargesCombo();
	void BuildPaymentsCombo();

	// Redraws the listbox
	void RefreshList();

	void FillList();

	// Assigns colors to payments, adjustments and refunds
	void ColorList();

	// Redraws the summary listing
	void RefreshSummary();
	// (d.thompson 2010-06-15) - PLID 39161 - Summary view function
	void ShowSummary(BOOL bShow);

	// Changes the display depending on if the user selected
	// the bills or payments tab
	void ChangeDisplay();

	// (j.jones 2008-05-02 08:43) - PLID 27305 - properly handled deleting from the array
	CPtrArray m_paryApplyManagerT;
	void ClearApplyArray();

	// Generated message map functions
	//{{AFX_MSG(CApplyManagerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeComboBills();
	afx_msg void OnSelchangeComboCharges();
	afx_msg void OnBtnApplyNew();
	afx_msg void OnBtnUnapply();
	afx_msg void OnDestroy();
	afx_msg void OnRButtonDownList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnSelectTab(short newTab, short oldTab);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedDisplayApplies();
	afx_msg void OnBnClickedDisplayPayments();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_APPLYMANAGERDLG_H__2C44F543_2D95_11D3_9461_00C04F4C8415__INCLUDED_)

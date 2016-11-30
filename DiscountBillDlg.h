#if !defined(AFX_DISCOUNTBILLDLG_H__C31AB363_4C68_4BDD_BB46_4A2CD651912D__INCLUDED_)
#define AFX_DISCOUNTBILLDLG_H__C31AB363_4C68_4BDD_BB46_4A2CD651912D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DiscountBillDlg.h : header file
//
// (j.gruber 2007-05-02 16:57) - PLID 14202 - Class created for
/////////////////////////////////////////////////////////////////////////////
// CDiscountBillDlg dialog

// (j.jones 2008-12-23 11:23) - PLID 32492 - added enum for DiscountBillOverwriteOption
enum EDiscountBillOverwriteOption {

	edboUnspecified = -1,
	edboAdd = 1,
	edboReplace = 2,
	edboSkip = 3,
};

class CDiscountBillDlg : public CNxDialog
{
// Construction
public:
	CDiscountBillDlg(CWnd* pParent);   // standard constructor
	~CDiscountBillDlg();	// (j.jones 2008-12-30 15:21) - PLID 32584

	COleCurrency m_cyDiscount;
	long m_nPercentOff;
	long m_nDiscountCatID;
	CString m_strCustomDescription;
	long m_nCouponID;

	BOOL m_bCouponSet;

	// (j.jones 2008-12-23 10:52) - PLID 32492 - the caller uses this to tell this dialog
	// if we already have discounts
	BOOL m_bHasDiscountsAlready;
	BOOL m_bIsBill;

	// (j.jones 2008-12-23 11:20) - PLID 32492 - send the user's selection back to the caller
	EDiscountBillOverwriteOption m_edboLastDiscountBillOverwriteOption;

	// (j.gruber 2009-03-24 10:34) - PLID 33355 - radio button to say whether they are applying from the line total or not
	BOOL m_bApplyFromLineTotal;

	// (j.jones 2008-12-23 10:28) - PLID 32492 - added controls to handle existing discounts
// Dialog Data
	//{{AFX_DATA(CDiscountBillDlg)
	enum { IDD = IDD_DISCOUNT_BILL_DLG };
	NxButton	m_radioAddDiscounts;
	NxButton	m_radioReplaceDiscounts;
	NxButton	m_radioSkipDiscounts;
	CNxStatic	m_nxstaticExistingDiscountsLabel;
	CNxStatic	m_nxstaticExistingDiscountCategoryLabel;
	NxButton	m_btnCustom;
	CNxEdit	m_nxeditBillCustomDiscountCat;
	CNxEdit	m_nxeditDiscountBillPercentOff;
	CNxEdit	m_nxeditDiscountBillDollarDiscount;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	NxButton	m_btnDiscountCategoryGroupbox;
	NxButton	m_radioApplyUsingLineTotal;
	NxButton	m_radioApplyUsingUnitCost;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDiscountBillDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	NXDATALIST2Lib::_DNxDataListPtr m_pCatList;

	// (a.walling 2007-05-10 16:00) - PLID 25171 - Apply a given coupon, prompting for one if -1
	void ApplyCoupon(long nID = -1);
	bool m_bScanning; // quasi-mutex since messages are pumped during a modal dialog

	// (z.manning 2016-01-15 11:08) - PLID 67909
	void HandleCalculateDiscountTypeChanged();

	// Generated message map functions
	//{{AFX_MSG(CDiscountBillDlg)
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnKillfocusDiscountBillPercentOff();
	afx_msg void OnKillfocusDiscountBillDollarDiscount();
	afx_msg void OnBillDiscountUseCustom();
	afx_msg void OnSelChosenBillDiscountCategoryList(LPDISPATCH lpRow);
	afx_msg void OnRequeryFinishedBillDiscountCategoryList(short nFlags);
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChangingBillDiscountCategoryList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnTrySetSelFinishedBillDiscountCategoryList(long nRowEnum, long nFlags);
	afx_msg LRESULT OnBarcodeScan(WPARAM wParam, LPARAM lParam);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedDiscountBillUseLineTotal();
	afx_msg void OnBnClickedDiscountBillApplyUseUnitCost();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DISCOUNTBILLDLG_H__C31AB363_4C68_4BDD_BB46_4A2CD651912D__INCLUDED_)

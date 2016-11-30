#if !defined(AFX_NEWDISCOUNTDLG_H__C31AB363_4C68_4BDD_BB46_4A2CD651912D__INCLUDED_)
#define AFX_NEWDISCOUNTDLG_H__C31AB363_4C68_4BDD_BB46_4A2CD651912D__INCLUDED_

#pragma once



// (j.gruber 2009-03-18 14:20) - PLID 33351 - created for

// CNewDiscountDlg dialog

class CNewDiscountDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CNewDiscountDlg)

public:
	// (j.jones 2009-04-07 10:09) - PLID 33474 - added fields for default coupons
	CNewDiscountDlg(BOOL bUseCoupons, BOOL bIsBill, BOOL bCheckPermissions, CWnd* pParent,
		long nDefaultCouponID = -1, CString strDefaultCouponName = "");   // standard constructor
	virtual ~CNewDiscountDlg();

	COleCurrency m_cyDollarDiscount;
	long m_nPercentDiscount;
	CString m_strCustomDiscountDesc;
	long m_nDiscountCategoryID;
	long m_nCouponID;	
	CString m_strCategoryDescription;

	// (j.gruber 2009-03-31 16:54) - PLID 33554 - added bisbill and bCheckDiscounts
	BOOL m_bIsBill;
	BOOL m_bCheckPermissions;

// Dialog Data
	enum { IDD = IDD_NEW_DISCOUNT_DLG };	
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	NxButton	m_radioApplyUsingLineTotal;
	NxButton	m_radioApplyUsingUnitCost;	
	NxButton	m_chkCustomDiscount;
	BOOL m_bApplyFromLineTotal;

protected:
	CNxEdit m_nxeCustomDiscountDesc;
	CNxEdit m_nxeDiscount;
	CNxEdit m_nxePercentOff;
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	BOOL ValidateInformation();

	// (z.manning 2016-01-15 11:08) - PLID 67909
	void HandleCalculateDiscountTypeChanged();

	DECLARE_MESSAGE_MAP()
private:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	
	NXDATALIST2Lib::_DNxDataListPtr m_pCatList;
	BOOL m_bUseCoupons;

	// (j.jones 2009-04-07 10:09) - PLID 33474 - added fields for default coupons
	long m_nDefaultCouponID;
	CString m_strDefaultCouponName;

	bool m_bScanning; // (j.jones 2009-04-07 09:44) - PLID 33474
	
	DECLARE_EVENTSINK_MAP()
	void SelChosenNewDiscountDiscountCategory(LPDISPATCH lpRow);
	afx_msg void OnBnClickedNewDiscountUseCustom();
	void RequeryFinishedNewDiscountDiscountCategory(short nFlags);
	void ChangePercentDiscountFieldsFromCoupon(long nCouponID);
	afx_msg void OnEnKillfocusNewDiscountPercentOff();
	afx_msg void OnEnKillfocusNewDiscountDollarDiscount();
	afx_msg void OnBnClickedNewDiscountCalculateLineTotal();
	afx_msg void OnBnClickedNewDiscountCalculateUnitCost();
	// (j.jones 2009-04-07 09:43) - PLID 33474 - supported barcode scanning
	afx_msg LRESULT OnBarcodeScan(WPARAM wParam, LPARAM lParam);
};
#endif
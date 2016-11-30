#pragma once

// CChargeDiscountDlg dialog
// (j.gruber 2009-03-18 14:19) - PLID 33411 - created for

class CChargeDiscountDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CChargeDiscountDlg)

public:	
	// (j.gruber 2009-03-31 16:54) - PLID 33554 - added bisbill and bCheckDiscounts
	// (j.gruber 2009-04-22 10:42) - PLID 34042 - added charge description
	// (j.jones 2011-01-21 14:38) - PLID 42156 - this dialog now requires the charge ID
	CChargeDiscountDlg(CWnd* pParent, long nChargeID, struct DiscountList *pDiscountList, BOOL bUseCoupons, BOOL bIsBill, BOOL bCheckPermissions,
		COleCurrency cyUnitCost, COleCurrency cyOtherFee, double dblQuantity, CString strChargeDescription,
		double dblMultiplier1 = -1.0, double dblMultiplier2 = -1.0, double dblMultiplier3 = -1.0, double dblMultiplier4 = -1.0,
		double dblTax1 = -1.0, double dblTax2 = -1.0);   // standard constructor
	CNxIconButton	m_btnAddDiscount;
	CNxIconButton	m_btnDeleteDiscount;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	virtual ~CChargeDiscountDlg();
	
	enum ChargePermissions {
		cpNotChecked = 0,
		cpNotAllowed,
		cpAllowed,
	};

	
	long GetTotalPercent();
	COleCurrency GetTotalDiscount();

	// (j.gruber 2009-03-31 16:54) - PLID 33554 - added bIsbill and bCheckDiscounts
	BOOL m_bIsBill;
	BOOL m_bCheckPermissions;

	// (j.gruber 2009-04-22 10:43) - PLID 34042 - added charge description
	CString m_strChargeDescription;


// Dialog Data
	enum { IDD = IDD_CHARGE_DISCOUNT_DLG };

protected:
	NXDATALIST2Lib::_DNxDataListPtr m_pDiscountDataList;
	long m_nChargeID;
	DiscountList *m_pDiscountList;

	CString GetDiscountCategoryName(long nDiscountCategoryID);
	CString GetCouponName(long nCouponID);

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	
	COleCurrency m_cyUnitCost;
	COleCurrency m_cyOtherCost;
	double m_dblQuantity;
	double m_dblMultiplier1;
	double m_dblMultiplier2;
	double m_dblMultiplier3;
	double m_dblMultiplier4;
	double m_dblTax1;
	double m_dblTax2;

	BOOL ValidateDiscounts();

	DECLARE_MESSAGE_MAP()

private:
	BOOL m_bUseCoupons;
	void ChangePercentDiscountFieldsFromCoupon(NXDATALIST2Lib::IRowSettingsPtr pRow, long nCouponID);
	// (j.gruber 2009-03-19 16:01) - PLID 33246 - check that they aren't entering negative charges
	COleCurrency CChargeDiscountDlg::CalculateLineTotal(long nTotalPercentOff, COleCurrency cyTotalDiscount);

	bool m_bScanning; // (j.jones 2009-04-07 09:44) - PLID 33474

	// (j.jones 2009-04-07 09:53) - PLID 33474 - created for barcoding purposes
	void AddNewDiscount(long nCouponID = -1, CString strCouponName = "");

	afx_msg void OnBnClickedAddDiscount();		
	afx_msg void OnBnClickedDeleteDiscount();
	DECLARE_EVENTSINK_MAP()
	void LeftClickChargeDiscountList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	void EditingStartingChargeDiscountList(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue);
	void EditingFinishedChargeDiscountList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	void EditingFinishingChargeDiscountList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
	// (j.jones 2009-04-07 09:43) - PLID 33474 - supported barcode scanning
	afx_msg LRESULT OnBarcodeScan(WPARAM wParam, LPARAM lParam);
};
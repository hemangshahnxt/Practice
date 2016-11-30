#if !defined(AFX_COUPONSETUPDLG_H__FB80C47A_B541_48DF_A5EE_F4C653710891__INCLUDED_)
#define AFX_COUPONSETUPDLG_H__FB80C47A_B541_48DF_A5EE_F4C653710891__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CouponSetupDlg.h : header file

//
// (j.gruber 2007-04-05 14:31) - PLID 25164 - Class added 
/////////////////////////////////////////////////////////////////////////////
// CCouponSetupDlg dialog

class CCouponSetupDlg : public CNxDialog
{
// Construction
public:
	CCouponSetupDlg(CWnd* pParent);   // standard constructor
	NXDATALIST2Lib::_DNxDataListPtr m_pCouponList;

// Dialog Data
	//{{AFX_DATA(CCouponSetupDlg)
	enum { IDD = IDD_COUPON_SETUP };
	NxButton	m_btnPercentOff;
	NxButton	m_btnDollarDiscount;
	NxButton	m_btnShowExpired;
	CNxIconButton	m_btnAddCoupon;
	CNxIconButton	m_btnDeleteCoupon;
	CNxEdit	m_nxeditCouponDescription;
	CNxEdit	m_nxeditCouponAmount;
	CNxEdit	m_nxeditCouponBarcode;
	CNxStatic	m_nxstaticCouponsTitle;
	//}}AFX_DATA

	// (a.wetta 2007-04-18 09:39) - PLID 25407 - This needs to be a public function so it can be called from CRetail
	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCouponSetupDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	BOOL Save(long nWindowID);
	CBrush m_brush;

	NXTIMELib::_DNxTimePtr m_pStartDate;
	NXTIMELib::_DNxTimePtr m_pEndDate;
	
	CString m_strDescription;
	CString m_strBarCode;
	COleDateTime m_dtStartDate;
	COleDateTime m_dtEndDate;
	long m_nPercent;
	COleCurrency m_cyDiscount;
	
	// Generated message map functions
	//{{AFX_MSG(CCouponSetupDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnAddCoupon();
	afx_msg void OnSelChosenCouponList(LPDISPATCH lpRow);
	afx_msg void OnSelChangingCouponList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnDeleteCoupon();
	afx_msg void OnShowExpiredCoupons();
	afx_msg void OnPercentOff();
	afx_msg void OnDollarDiscount();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnRequeryFinishedCouponList(short nFlags);
	afx_msg void OnKillfocusCouponAmount();
	afx_msg void OnKillfocusCouponBarcode();
	afx_msg void OnChangeCouponEndDate();
	afx_msg void OnChangeCouponStartDate();
	afx_msg void OnKillfocusCouponDescription();
	afx_msg void OnKillFocusCouponEndDate();
	afx_msg void OnKillFocusCouponStartDate();
	afx_msg LRESULT OnBarcodeScan(WPARAM wParam, LPARAM lParam);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COUPONSETUPDLG_H__FB80C47A_B541_48DF_A5EE_F4C653710891__INCLUDED_)

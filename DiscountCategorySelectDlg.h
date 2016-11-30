#if !defined(AFX_DISCOUNTCATEGORYSELECTDLG_H__163127CC_01D9_4162_9EB2_6B104A95BC83__INCLUDED_)
#define AFX_DISCOUNTCATEGORYSELECTDLG_H__163127CC_01D9_4162_9EB2_6B104A95BC83__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DiscountCategorySelectDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDiscountCategorySelectDlg dialog

class CDiscountCategorySelectDlg : public CNxDialog
{
// Construction
public:
	// (j.gruber 2007-04-03 17:23) - PLID 9796 - adding ability to use coupons
	CDiscountCategorySelectDlg(CWnd* pParent, long nDiscCatID, CString strCustomDesc, long nCouponID, BOOL bShowCoupons);   // standard constructor
	~CDiscountCategorySelectDlg();	// (j.jones 2008-12-30 15:21) - PLID 32584

	NXDATALIST2Lib::_DNxDataListPtr m_pCatList;

	long m_nDiscountCatID;
	CString m_strCustomDescription;
	// (j.gruber 2007-04-04 14:22) - PLID 9796 - added coupons
	long m_nCouponID;
	// (j.gruber 2007-05-14 14:45) - PLID 25173 - adding support for not showing coupons
	BOOL m_bShowCoupons;
	

// Dialog Data
	//{{AFX_DATA(CDiscountCategorySelectDlg)
	enum { IDD = IDD_DISCOUNT_CATEGORIES };
	NxButton	m_btnCustom;
	CNxEdit	m_nxeditCustomDiscountCat;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDiscountCategorySelectDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	bool m_bScanning;

	// Generated message map functions
	//{{AFX_MSG(CDiscountCategorySelectDlg)
	afx_msg void OnUseCustom();
	afx_msg void OnSelChosenDiscountCategoryList(LPDISPATCH lpRow);
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChangingDiscountCategoryList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnRequeryFinishedDiscountCategoryList(short nFlags);
	virtual void OnOK();
	afx_msg void OnTrySetSelFinishedDiscountCategoryList(long nRowEnum, long nFlags);
	afx_msg LRESULT OnBarcodeScan(WPARAM wParam, LPARAM lParam);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DISCOUNTCATEGORYSELECTDLG_H__163127CC_01D9_4162_9EB2_6B104A95BC83__INCLUDED_)

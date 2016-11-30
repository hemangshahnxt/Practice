#if !defined(AFX_COUPONSELECTDLG_H__917196D4_23B4_46D5_848C_E74202124045__INCLUDED_)
#define AFX_COUPONSELECTDLG_H__917196D4_23B4_46D5_848C_E74202124045__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CouponSelectDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCouponSelectDlg dialog
// (j.gruber 2007-04-04 14:22) - PLID 9796 - class created for coupons

class CCouponSelectDlg : public CNxDialog
{
// Construction
public:
	CCouponSelectDlg(CWnd* pParent);   // standard constructor
	long m_nCouponID;
	CString m_strCouponName;

// Dialog Data
	//{{AFX_DATA(CCouponSelectDlg)
	enum { IDD = IDD_COUPON_SELECT };
	NxButton	m_btnShowExpired;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCouponSelectDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALIST2Lib::_DNxDataListPtr m_pCouponList;
	// Generated message map functions
	//{{AFX_MSG(CCouponSelectDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnShowExpired();
	virtual void OnCancel();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COUPONSELECTDLG_H__917196D4_23B4_46D5_848C_E74202124045__INCLUDED_)

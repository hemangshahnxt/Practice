#if !defined(AFX_RETAIL_H__CDF29926_C38A_4F94_AAEE_087BD2FADB6B__INCLUDED_)
#define AFX_RETAIL_H__CDF29926_C38A_4F94_AAEE_087BD2FADB6B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Retail.h : header file
//

#include "CommissionSetupWnd.h"
#include "CouponSetupDlg.h"
#include "SalesSetupDlg.h"
#include "SuggestedSalesDlg.h"
#include "RewardPointsSetupDlg.h"

// (a.wetta 2007-03-29 13:27) - PLID 25407 - Created Retail tab

/////////////////////////////////////////////////////////////////////////////
// CRetail dialog

class CRetail : public CNxDialog
{
// Construction
public:
	CRetail(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CRetail)
	enum { IDD = IDD_RETAIL };
		// NOTE: the ClassWizard will add data members here
	CNxStatic	m_nxstaticRewardsPlaceholder;
	CNxStatic	m_nxstaticCommissionPlaceholder;
	CNxStatic	m_nxstaticCouponPlaceHolder;
	CNxStatic	m_nxstaticSalesPlaceholder;
	CNxStatic	m_nxstaticSuggestedSalesPlaceholder;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRetail)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	CCommissionSetupWnd m_CommissionSetupWnd;
	// (j.gruber 2007-04-02 15:44) - PLID 25164 - coupons dialog
	CCouponSetupDlg m_CouponsSetupDlg;
	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);
	// (a.wetta 2007-04-26 16:35) - PLID 15998 - sales dialog
	CSalesSetupDlg m_SalesSetupDlg;
	// (a.wetta 2007-05-15 17:51) - PLID 25960 - Add suggested sales and reward points to the dialog
	CSuggestedSalesDlg m_SuggestedSalesDlg;
	CRewardPointsSetupDlg m_RewardPointsSetupDlg;

	// Generated message map functions
	//{{AFX_MSG(CRetail)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnBarcodeScan(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RETAIL_H__CDF29926_C38A_4F94_AAEE_087BD2FADB6B__INCLUDED_)

#if !defined(AFX_BILLINGEXTRACHARGEINFODLG_H__4919BF24_AE4A_4F71_A50A_484D8D81BBD7__INCLUDED_)
#define AFX_BILLINGEXTRACHARGEINFODLG_H__4919BF24_AE4A_4F71_A50A_484D8D81BBD7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BillingExtraChargeInfoDlg.h : header file
//

// (j.jones 2008-05-28 10:18) - PLID 28782 - created

struct BillingItem;
typedef shared_ptr<BillingItem> BillingItemPtr;

/////////////////////////////////////////////////////////////////////////////
// CBillingExtraChargeInfoDlg dialog

class CBillingExtraChargeInfoDlg : public CNxDialog
{
// Construction
public:
	CBillingExtraChargeInfoDlg(CWnd* pParent, std::vector<BillingItemPtr>& billingItems, CBillingModuleDlg *pBillingModuleDlg, BOOL bIsReadOnly);   // standard constructor

	// (a.walling 2014-02-24 11:27) - PLID 61003 - CPtrArray g_aryBillingTabInfoT in CBillingDlg et al should instead be a typed collection: vector<BillingItemPtr> m_billingItems. Using a reference and setting in the constructor instead of a pointer.
	std::vector<BillingItemPtr>& m_billingItems;
	CBillingModuleDlg *m_pBillingModuleDlg;

// Dialog Data
	//{{AFX_DATA(CBillingExtraChargeInfoDlg)
	enum { IDD = IDD_BILLING_EXTRA_CHARGE_INFO_DLG };
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnOK;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBillingExtraChargeInfoDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support	
	//}}AFX_VIRTUAL

// Implementation
protected:

	NXDATALIST2Lib::_DNxDataListPtr m_ChargeList;

	BOOL m_bIsReadOnly;

	// Generated message map functions
	//{{AFX_MSG(CBillingExtraChargeInfoDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
	// (j.jones 2009-08-13 12:39) - PLID 35206 - added OnEditingFinishingChargeList
	void OnEditingFinishingChargeList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
	// (j.jones 2011-08-24 08:41) - PLID 44868 - added OnEditingStarting
	void OnEditingStartingChargeList(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BILLINGEXTRACHARGEINFODLG_H__4919BF24_AE4A_4F71_A50A_484D8D81BBD7__INCLUDED_)

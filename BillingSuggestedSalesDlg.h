#pragma once

// BillingSuggestedSalesDlg.h : header file
//

// (a.walling 2007-05-07 16:28) - PLID 14717 - Dialog box to suggest sales to the user
// based on the current bill and another tab for the previous bill.

/////////////////////////////////////////////////////////////////////////////
// CBillingSuggestedSalesDlg dialog

enum ESuggestionSalesColumns {
	esscRootID = 0,
	esscParentID,
	esscMasterServiceID,
	esscSuggestionID,
	esscServiceID,
	esscName,
	esscCode,
	esscPrice,
	esscReason,
	esscServiceType,
	esscOrderIndex,
	esscRecentBillDate
};

enum ETabType {
	ettThisBill = 0,
	ettAllBills
};

// (j.armen 2012-06-06 12:39) - PLID 50830 - Removed GetMinMaxInfo
class CBillingSuggestedSalesDlg : public CNxDialog
{
// Construction
public:
	CBillingSuggestedSalesDlg(CWnd* pParent);   // standard constructor
	~CBillingSuggestedSalesDlg();

	void AddService(long nID);
	BOOL ServicesHaveChanged(); // TRUE if services have changed since last refresh

	inline void Clear() {m_arServices.RemoveAll();} // remove all services from our list
	inline void SetBillingDlg(CWnd* pBillingDlg) {m_pBillingDlg = pBillingDlg;} // set our billing dialog pointer

	static BOOL CheckSuggestionsExist(CArray<long, long> &arServiceIDs, CWnd* pBillingDlg); // return TRUE if suggestions exist for this list of services
	
	void SetBillInfo(long nPatientID, long nBillID); // set our patient ID and bill ID

	void Refresh(); // refresh the current list
	void RefreshIfNeeded(BOOL bForceIfHidden = FALSE); // refreshes only if needed

// Dialog Data
	//{{AFX_DATA(CBillingSuggestedSalesDlg)
	enum { IDD = IDD_BILLING_SUGGESTED_SALES };
	CNxEdit	m_nxeditSuggestedSalesReason;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBillingSuggestedSalesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALIST2Lib::_DNxDataListPtr	m_dlList;		// suggestions for this bill
	NXDATALIST2Lib::_DNxDataListPtr	m_dlListAll;	// suggestions for previous bills

	CArray<long, long> m_arServices;	// list of our service IDs
	CArray<long, long> m_arCurServices;	// list of service IDs as of last refresh

	CWnd* m_pBillingDlg;
	long m_nPatientID;
	long m_nBillID;
	long m_nThisLastSuggestionID;
	long m_nAllLastSuggestionID;
	long m_nLastReasonSuggestionID;

	BOOL m_bAllListNeedsRefresh;
	BOOL m_bThisListNeedsRefresh;

	// (a.walling 2007-11-06 10:28) - PLID 28000 - Need to specify namespace
	NxTab::_DNxTabPtr m_pTabs;

	// Generated message map functions
	//{{AFX_MSG(CBillingSuggestedSalesDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnRequeryFinishedDatalistSuggestedSales(short nFlags);
	afx_msg void OnLeftClickDatalistSuggestedSales(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRequeryFinishedDatalistSuggestedSalesAll(short nFlags);
	afx_msg void OnLeftClickDatalistSuggestedSalesAll(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnSelectTab(short newTab, short oldTab);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnDestroy();
	afx_msg void OnCancel();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
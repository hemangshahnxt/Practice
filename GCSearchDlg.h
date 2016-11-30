#if !defined(AFX_GCSEARCHDLG_H__DA137C44_665D_428A_9137_E0A185167FAE__INCLUDED_)
#define AFX_GCSEARCHDLG_H__DA137C44_665D_428A_9137_E0A185167FAE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GCSearchDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CGCSearchDlg dialog

class CGCSearchDlg : public CNxDialog
{
// Construction
public:
	CGCSearchDlg(CWnd* pParent);   // standard constructor

	// (j.jones 2015-04-24 10:42) - PLID 65710 - if true,
	// the dialog has only OK/Cancel options, because the user
	// is selecting a GC to transfer into
	bool m_bIsTransferring;
	//filled if they click OK when m_bIsTransferring is true
	long m_nSelectedGCIDToTransfer;
	CString m_strSelectedGCNumberToTransfer;

// Dialog Data
	//{{AFX_DATA(CGCSearchDlg)
	enum { IDD = IDD_GC_SEARCH_DLG };
	NxButton	m_btnHideVoid;
	NxButton	m_btnHideExpired;
	NxButton	m_btnHide0Balance;
	CDateTimePicker	m_dtExpires;
	CDateTimePicker	m_dtPurchase;
	CNxEdit	m_nxeditPurchBy;
	CNxEdit	m_nxeditRecBy;
	CNxEdit	m_nxeditGcAmt;
	CNxEdit	m_nxeditGcType;
	CNxEdit	m_nxeditGcUsed;
	CNxEdit	m_nxeditGcBalance;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnPreviewGC;
	CNxIconButton	m_btnVoidGC;
	CNxIconButton	m_btnGoToPurchaser;
	CNxIconButton	m_btnGoToReceiver;
	// (j.jones 2015-03-31 09:20) - PLID 65390 - added GC # field
	CNxEdit	m_nxeditGCNumber;
	// (j.jones 2015-04-24 10:15) - PLID 65710 - added ability to transfer balances
	CNxIconButton	m_btnTransferGCBalance;
	// (j.jones 2015-04-24 10:40) - PLID 65710 - added a cancel button for transferring into new GCs
	CNxIconButton	m_btnCancelTransfer;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGCSearchDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// (j.jones 2015-03-25 15:02) - PLID 65390 - converted the dropdown to a search list
	NXDATALIST2Lib::_DNxDataListPtr m_SearchList;
	// (j.jones 2015-04-01 10:11) - PLID 65391 - converted to a DL2
	NXDATALIST2Lib::_DNxDataListPtr m_HistoryList;

	BOOL m_bEditable;

	int lastGCVoid;
	
	bool m_bExpChanged; // (a.walling 2006-07-03 11:24) - PLID 20113 Find out if the expiration has changed
	void CheckAndSaveExpiration(); //  so we can save it in this function
	long m_nSelectedGCID;
	CString m_strNewExp, m_strAuditOldVal, m_strAuditNewVal; // to keep the data around for when we update it.

	// (j.jones 2015-03-31 09:42) - PLID 65390 - renamed to indicate this
	// only updates the search properties, nothing is actually reloaded
	void UpdateSearchListFilters();
	void PopulateHistory();
	void SetCurrentPatient(long nID);

	// (j.jones 2015-04-24 11:16) - PLID 65710 - added function to reload the current GC
	void ReloadCurrentGiftCertificateBalance();

	// (j.jones 2015-04-24 10:15) - PLID 65714 - added ability to transfer balances
	// (r.gonet 2015-05-11 10:46) - PLID 65392 - Now returns whether or not the transfer succeeded.
	bool TransferBalanceToNewGC(long nSourceGiftID, CString strSourceGiftCertNumber, COleCurrency cyAmtToTransfer);
	// (r.gonet 2015-05-11 10:46) - PLID 65392 - Now returns whether or not the transfer succeeded.
	bool TransferBalanceToExistingGC(long nSourceGiftID, CString strSourceGiftCertNumber, COleCurrency cyAmtToTransfer);

	// (j.jones 2015-04-24 10:21) - PLID 65711 - shared function to transfer balances
	void TransferBalanceToGC(long nSourceGiftID, CString strSourceGiftCertNumber,
		long nDestGiftID, CString strDestGiftCertNumber, COleCurrency cyAmtToTransfer);

	// Generated message map functions
	//{{AFX_MSG(CGCSearchDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnPreviewGc();
	afx_msg LRESULT OnBarcodeScan(WPARAM wParam, LPARAM lParam);
	afx_msg void OnFilterHideBalances();
	afx_msg void OnFilterHideExpired();
	afx_msg void OnGoToPurchaser();
	afx_msg void OnGoToReceiver();
	afx_msg void OnVoidGc();
	afx_msg void OnFilterHideVoided();
	afx_msg void OnChangeSearchExpDate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEditexp();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	// (j.jones 2015-03-25 15:02) - PLID 65390 - converted the dropdown to a search list
	void OnSelChosenGcSearchList(LPDISPATCH lpRow);
	// (j.jones 2015-04-24 10:15) - PLID 65710 - added ability to transfer balances
	afx_msg void OnBtnTransferGCBalance();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GCSEARCHDLG_H__DA137C44_665D_428A_9137_E0A185167FAE__INCLUDED_)

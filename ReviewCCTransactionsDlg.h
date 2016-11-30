#pragma once

// (d.lange 2010-09-02 10:09) - PLID 40311 - Created to replace QBMS_ReviewTransactionsDlg
// CReviewCCTransactionsDlg dialog
#include "FinancialRc.h"

class CReviewCCTransactionsDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CReviewCCTransactionsDlg)

public:
	CReviewCCTransactionsDlg(CWnd* pParent);   // standard constructor
	virtual ~CReviewCCTransactionsDlg();

// Dialog Data
	enum { IDD = IDD_REVIEW_CC_TRANSACTIONS_DLG };

protected:
	NxButton m_btnPending;
	NxButton m_btnApproved;
	NxButton m_btnAll;
	CNxIconButton m_btnReprocess;
	NXDATALIST2Lib::_DNxDataListPtr m_pList;
	// (d.thompson 2010-11-08) - PLID 41379
	NXDATALIST2Lib::_DNxDataListPtr m_pAccountFilter;

	void Reload();
	// (d.thompson 2010-11-11) - PLID 40368
	void LoadAccountList();

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedQbmsPending();
	afx_msg void OnBnClickedQbmsApproved();
	afx_msg void OnBnClickedQbmsAll();
	afx_msg void OnBnClickedQbmsReprocess();
	DECLARE_EVENTSINK_MAP()
	void DblClickCellQbmsList(LPDISPATCH lpRow, short nColIndex);
	void SelChosenCcAcctFilterList(LPDISPATCH lpRow);
};

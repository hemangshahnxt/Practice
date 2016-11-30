#pragma once

// CBillDiscountOverviewDlg dialog

// (j.jones 2010-06-10 17:42) - PLID 39109 - created

#include "BillingRc.h"

class CBillDiscountOverviewDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CBillDiscountOverviewDlg)

public:
	CBillDiscountOverviewDlg(CWnd* pParent);   // standard constructor

	long m_nBillID;
	long m_nDefChargeID;

// Dialog Data
	enum { IDD = IDD_BILL_DISCOUNT_OVERVIEW_DLG };
	CNxIconButton	m_btnClose;

protected:

	NXDATALIST2Lib::_DNxDataListPtr m_ChargeList;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support	
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnBtnClose();
};

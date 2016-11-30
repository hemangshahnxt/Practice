#pragma once

// (j.jones 2010-12-22 10:07) - PLID 41911 - created

// CFinancialCloseDlg dialog

#include "BillingRc.h"

class CFinancialCloseDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CFinancialCloseDlg)

public:
	CFinancialCloseDlg(CWnd* pParent);   // standard constructor
	virtual ~CFinancialCloseDlg();

// Dialog Data
	enum { IDD = IDD_FINANCIAL_CLOSE_DLG };
	CNxIconButton	m_btnExitDialog;
	CNxIconButton	m_btnPerformClose;
	CNxStatic	m_nxstaticCurrentCloseDate;

protected:

	NXDATALIST2Lib::_DNxDataListPtr m_CloseHistoryList;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnExitDialog();
	afx_msg void OnBtnPerformClose();
};

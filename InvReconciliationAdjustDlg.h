#pragma once

// CInvReconciliationAdjustDlg dialog

// (j.jones 2009-01-15 11:39) - PLID 32684 - created

#include "InvReconciliationDlg.h"

class CInvReconciliationAdjustDlg : public CNxDialog
{

public:
	CInvReconciliationAdjustDlg(CWnd* pParent);   // standard constructor

	CArray<ProductNeedingAdjustments*, ProductNeedingAdjustments*> *m_paryProductsNeedingAdjustments;

	long m_nLocationID;

// Dialog Data
	enum { IDD = IDD_INV_RECONCILIATION_ADJUST_DLG };
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	CNxStatic m_nxstaticConsignmentLabel;
	// (j.jones 2009-07-10 09:48) - PLID 34843 - added ability to print this list
	CNxIconButton m_btnPrint;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	NXDATALIST2Lib::_DNxDataListPtr m_AdjList;

	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	afx_msg void OnOk();
	afx_msg void OnCancel();
	// (j.jones 2009-07-10 09:48) - PLID 34843 - added ability to print this list
	afx_msg void OnBtnPrintInvRecAdj();
};

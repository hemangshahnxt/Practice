#pragma once

#include "BillingRc.h"

// (j.armen 2012-05-10 14:00) - PLID 14102 - Created

class CInsuranceReversalDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CInsuranceReversalDlg)

private:
	NXDATALIST2Lib::_DNxDataListPtr m_pdlBatchPayments;
	NxButton m_btnShowAllDates;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;

private:
	const long m_nPaymentID;
	void ResizeBatchPaymentsDropdown();

public:
	enum { IDD = IDD_INSURANCE_REVERSAL_DLG };
	CInsuranceReversalDlg(CWnd* pParent, const long& nPaymentID);   // standard constructor
	virtual ~CInsuranceReversalDlg();
	virtual BOOL OnInitDialog();

protected:
	virtual void DoDataExchange(CDataExchange* pDX); 

private:
	DECLARE_MESSAGE_MAP()
	virtual void OnOK();
	afx_msg void OnBnClickedReversalShowAllDates();

private:
	DECLARE_EVENTSINK_MAP()
	void SelChangedNxdlBatchPayments(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	void RequeryFinishedNxdlBatchPayments(short nFlags);
};

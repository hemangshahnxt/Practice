#pragma once

class CBillingModuleDlg;

class CEMNBillManager
{
public:
	CEMNBillManager(CWnd *pWnd);
	~CEMNBillManager(void);

	bool BillEMN(long nPatientID, long nEMNID, bool bBillEntireEMR);

protected:
	long m_nEMNID;
	CWnd *m_pWnd;
	CBillingModuleDlg *m_BillingDlg;
};

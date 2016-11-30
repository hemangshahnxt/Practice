#pragma once
#include <MessageOnlyWnd.h>
#include <set>

// (j.dinatale 2012-01-16 10:53) - PLID 47539 - will control billing from EMR

class CEMNBillController :
	public CMessageOnlyWnd
{
public:
	CEMNBillController(void);

	BOOL Initialize();

	bool NeedToBillPartials(long nEMNID);
	bool HasUnassignedCharges(long nEMNID);

	// (j.dinatale 2012-01-20 09:32) - PLID 47539 - need to have a method to check if there are inactive insurances assigned to EMR charges
	bool HasInactiveInsuranceAssigned(long nEMNID, bool bExcludeBilled = true);

	// (j.dinatale 2012-01-23 14:22) - PLID 47620
	bool HasBeenFullyBilled(long nEMNID);

	bool BillEntireEMN(long nEMNID, long nPatientID, long nInsuredPartyID);
	bool BillEMNToAssignedResps(long nEMNID, long nPatientID);
	
	void SubscribeToEvents(CWnd *pWnd);
	void UnsubscribeFromEvents(CWnd *pWnd);
private:
	CBillingModuleDlg *m_BillingDlg;
	long m_nCurrEMNID;
	long m_nPatientID;
	
	typedef std::set<CWnd*> WindowSet;
	WindowSet m_eventSubscribers;

	CArray<long, long> m_aryInsuredPartiesToBill;

	void Reset(long nNewEMRID = -1, long nPatientID = -1);
	bool CanWeBill(long nEMNID);
	void PostEvent(UINT nMessage, WPARAM wParam, LPARAM lParam);

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnPostEditBill(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnPostCancelBill(WPARAM wParam, LPARAM lParam); // (j.dinatale 2012-01-20 15:38) - PLID 47539 - handle billing dlg cancellation
};

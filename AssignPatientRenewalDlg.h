#pragma once
#include "NxAPI.h"
// (a.wilson 2013-01-17 10:53) - PLID 53784 - created
// CAssignPatientRenewalDlg dialog

class CAssignPatientRenewalDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CAssignPatientRenewalDlg)

public:
	CAssignPatientRenewalDlg(CWnd* pParent = NULL, NexTech_Accessor::_ERxPatientInfoPtr m_pRenewalRequestPatientInfo = NULL);
	virtual ~CAssignPatientRenewalDlg();
	long GetAssignedPatientID();

	enum { IDD = IDD_ASSIGN_PATIENT_RENEWAL_DLG };

protected:
	long m_nAssignedPatientID, m_nCurrentPatientID;
	NexTech_Accessor::_ERxPatientInfoPtr m_pRenewalRequestPatientInfo;
	NXDATALIST2Lib::_DNxDataListPtr m_pPatientList;
	CNxIconButton m_btnAssign, m_btnCancel;

	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
	afx_msg void OnBnClickedAssignPatient();
	void SelSetPatientLinkList(LPDISPATCH lpSel);
	void RequeryFinishedPatientLinkList(short nFlags);
};

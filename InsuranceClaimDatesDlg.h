#pragma once
// (b.eyers 2016-04-07) - NX-100071 - created

// CInsuranceClaimDatesDlg dialog

#include "BillingDlg.h"

class CInsuranceClaimDatesDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CInsuranceClaimDatesDlg)

public:
	CInsuranceClaimDatesDlg(CWnd* pParent, ClaimDates& claimDates);   // standard constructor
	virtual ~CInsuranceClaimDatesDlg();

	bool m_bReadOnly;

	NXTIMELib::_DNxTimePtr m_pFirstVisit, m_pInitialTreatment, m_pLastSeen, m_pChronicCondition, m_pLastXRay, m_pHearingPresc, m_pAssumedCare, m_pReliquishedCare, m_pAccidentDate;

// Dialog Data
	enum { IDD = IDD_INSURANCE_CLAIM_DATES_DLG };
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;


protected:


	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	ClaimDates& m_claimDates;

	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	//DECLARE_EVENTSINK_MAP()
	afx_msg void OnOK();
	afx_msg void OnCancel();
};

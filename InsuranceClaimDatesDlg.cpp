// InsuranceClaimDatesDlg.cpp : implementation file
//
// (b.eyers 2016-04-07) - NX-100071 - created

#include "stdafx.h"
#include "BillingRc.h"
#include "InsuranceClaimDatesDlg.h"

// CInsuranceClaimDatesDlg dialog

IMPLEMENT_DYNAMIC(CInsuranceClaimDatesDlg, CNxDialog)

CInsuranceClaimDatesDlg::CInsuranceClaimDatesDlg(CWnd* pParent, ClaimDates& claimDates)
	: CNxDialog(CInsuranceClaimDatesDlg::IDD, pParent), m_claimDates(claimDates)
{
	m_bReadOnly = false;
}

CInsuranceClaimDatesDlg::~CInsuranceClaimDatesDlg()
{
}

void CInsuranceClaimDatesDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
}


BEGIN_MESSAGE_MAP(CInsuranceClaimDatesDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
END_MESSAGE_MAP()


BOOL CInsuranceClaimDatesDlg::OnInitDialog()
{
	try {
		CNxDialog::OnInitDialog();

		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_pFirstVisit = GetDlgItemUnknown(IDC_IC_FIRST_VISIT_DATE);
		m_pInitialTreatment = GetDlgItemUnknown(IDC_IC_INITIAL_TREATMENT_DATE);
		m_pLastSeen = GetDlgItemUnknown(IDC_IC_LAST_SEEN_DATE);
		m_pChronicCondition = GetDlgItemUnknown(IDC_IC_CHRONIC_CONDITION_DATE);
		m_pLastXRay = GetDlgItemUnknown(IDC_IC_LAST_XRAY_DATE);
		m_pHearingPresc = GetDlgItemUnknown(IDC_IC_HEARING_PRESC_DATE);
		m_pAssumedCare = GetDlgItemUnknown(IDC_IC_ASSUMED_CARE_DATE);
		m_pReliquishedCare = GetDlgItemUnknown(IDC_IC_RELINQUISHED_CARE_DATE);
		m_pAccidentDate = GetDlgItemUnknown(IDC_IC_ACCIDENT_DATE);

		//read-only?
		if (m_bReadOnly) {
			m_pFirstVisit->Enabled = VARIANT_FALSE;
			m_pInitialTreatment->Enabled = VARIANT_FALSE;
			m_pLastSeen->Enabled = VARIANT_FALSE;
			m_pChronicCondition->Enabled = VARIANT_FALSE;
			m_pLastXRay->Enabled = VARIANT_FALSE;
			m_pHearingPresc->Enabled = VARIANT_FALSE;
			m_pAssumedCare->Enabled = VARIANT_FALSE;
			m_pReliquishedCare->Enabled = VARIANT_FALSE;
			m_pAccidentDate->Enabled = VARIANT_FALSE;
		}


		//load in data from the struct to the time controls, 
		if (m_claimDates.dtFirstVisitOrConsultationDate != g_cdtNull) {
			m_pFirstVisit->SetDateTime(m_claimDates.dtFirstVisitOrConsultationDate);
		}
		if (m_claimDates.dtInitialTreatmentDate != g_cdtNull) {
			m_pInitialTreatment->SetDateTime(m_claimDates.dtInitialTreatmentDate);
		}
		if (m_claimDates.dtLastSeenDate != g_cdtNull) {
			m_pLastSeen->SetDateTime(m_claimDates.dtLastSeenDate);
		}
		if (m_claimDates.dtAcuteManifestationDate != g_cdtNull) {
			m_pChronicCondition->SetDateTime(m_claimDates.dtAcuteManifestationDate);
		}
		if (m_claimDates.dtLastXRayDate != g_cdtNull) {
			m_pLastXRay->SetDateTime(m_claimDates.dtLastXRayDate);
		}
		if (m_claimDates.dtHearingAndPrescriptionDate != g_cdtNull) {
			m_pHearingPresc->SetDateTime(m_claimDates.dtHearingAndPrescriptionDate);
		}
		if (m_claimDates.dtAssumedCareDate != g_cdtNull) {
			m_pAssumedCare->SetDateTime(m_claimDates.dtAssumedCareDate);
		}
		if (m_claimDates.dtRelinquishedCareDate != g_cdtNull) {
			m_pReliquishedCare->SetDateTime(m_claimDates.dtRelinquishedCareDate);
		}
		if (m_claimDates.dtAccidentDate != g_cdtNull) {
			m_pAccidentDate->SetDateTime(m_claimDates.dtAccidentDate);
		}

	}
	NxCatchAll("Error in CInsuranceClaimDatesDlg::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

// CInsuranceClaimDatesDlg message handlers

void CInsuranceClaimDatesDlg::OnOK()
{
	try {

		//build a warning if any times are invalid, in the future, or before 1800(which is going to count as invalid
		COleDateTime dtOldDate;
		dtOldDate.SetDateTime(1800, 1, 1, 1, 1, 1);
		CString strWarningLater = "", strWarningInvalid = "", strWarning = "";
		long nWarningLater = 0, nWarningInvalid = 0;

		//check first visit
		if (m_pFirstVisit->GetStatus() == 1) {

			if (m_pFirstVisit->GetDateTime() < dtOldDate) {
				strWarningInvalid = "first visit or consultation date, ";
				nWarningInvalid++;
			}

			if (m_pFirstVisit->GetDateTime() > COleDateTime::GetCurrentTime()) {
				strWarningLater = "first visit or consultation date, ";
				nWarningLater++;
			}
		}
		else if (m_pFirstVisit->GetStatus() == 2) {
			strWarningInvalid = "first visit or consultation date, ";
			nWarningInvalid++;
		}

		//check initial treatment
		if (m_pInitialTreatment->GetStatus() == 1) {

			if (m_pInitialTreatment->GetDateTime() < dtOldDate) {
				strWarningInvalid += "initial treatment date, ";
				nWarningInvalid++;
			}

			if (m_pInitialTreatment->GetDateTime() > COleDateTime::GetCurrentTime()) {
				strWarningLater += "initial treatment date, ";
				nWarningLater++;
			}
		}
		else if (m_pInitialTreatment->GetStatus() == 2) {
			strWarningInvalid += "initial treatment date, ";
			nWarningInvalid++;
		}

		//check last seen
		if (m_pLastSeen->GetStatus() == 1) {

			if (m_pLastSeen->GetDateTime() < dtOldDate) {
				strWarningInvalid += "last seen date / latest visit, ";
				nWarningInvalid++;
			}

			if (m_pLastSeen->GetDateTime() > COleDateTime::GetCurrentTime()) {
				strWarningLater += "last seen date / latest visit, ";
				nWarningLater++;
			}
		}
		else if (m_pLastSeen->GetStatus() == 2) {
			strWarningInvalid += "last seen date / latest visit, ";
			nWarningInvalid++;
		}

		//check chronic condition
		if (m_pChronicCondition->GetStatus() == 1) {

			if (m_pChronicCondition->GetDateTime() < dtOldDate) {
				strWarningInvalid += "acute manifestation of a chronic condition date, ";
				nWarningInvalid++;
			}

			if (m_pChronicCondition->GetDateTime() > COleDateTime::GetCurrentTime()) {
				strWarningLater += "acute manifestation of a chronic condition date, ";
				nWarningLater++;
			}
		}
		else if (m_pChronicCondition->GetStatus() == 2) {
			strWarningInvalid += "acute manifestation of a chronic condition date, ";
			nWarningInvalid++;
		}

		//check last xray 
		if (m_pLastXRay->GetStatus() == 1) {

			if (m_pLastXRay->GetDateTime() < dtOldDate) {
				strWarningInvalid += "last x-ray date, ";
				nWarningInvalid++;
			}

			if (m_pLastXRay->GetDateTime() > COleDateTime::GetCurrentTime()) {
				strWarningLater += "last x-ray date, ";
				nWarningLater++;
			}
		}
		else if (m_pLastXRay->GetStatus() == 2) {
			strWarningInvalid += "last x-ray date, ";
			nWarningInvalid++;
		}

		//check hearing and prescription date
		if (m_pHearingPresc->GetStatus() == 1) {

			if (m_pHearingPresc->GetDateTime() < dtOldDate) {
				strWarningInvalid += "hearing and prescription date, ";
				nWarningInvalid++;
			}

			if (m_pHearingPresc->GetDateTime() > COleDateTime::GetCurrentTime()) {
				strWarningLater += "hearing and prescription date, ";
				nWarningLater++;
			}
		}
		else if (m_pHearingPresc->GetStatus() == 2) {
			strWarningInvalid += "hearing and prescription date, ";
			nWarningInvalid++;
		}

		//check assumed care
		if (m_pAssumedCare->GetStatus() == 1) {

			if (m_pAssumedCare->GetDateTime() < dtOldDate) {
				strWarningInvalid += "assumed care date, ";
				nWarningInvalid++;
			}

			if (m_pAssumedCare->GetDateTime() > COleDateTime::GetCurrentTime()) {
				strWarningLater += "assumed care date, ";
				nWarningLater++;
			}
		}
		else if (m_pAssumedCare->GetStatus() == 2) {
			strWarningInvalid += "assumed care date, ";
			nWarningInvalid++;
		}

		//check relinquished care
		if (m_pReliquishedCare->GetStatus() == 1) {

			if (m_pReliquishedCare->GetDateTime() < dtOldDate) {
				strWarningInvalid += "relinquished care date, ";
				nWarningInvalid++;
			}

			if (m_pReliquishedCare->GetDateTime() > COleDateTime::GetCurrentTime()) {
				strWarningLater += "relinquished care date, ";
				nWarningLater++;
			}
		}
		else if (m_pReliquishedCare->GetStatus() == 2) {
			strWarningInvalid += "relinquished care date, ";
			nWarningInvalid++;
		}

		//check accident date
		if (m_pAccidentDate->GetStatus() == 1) {

			if (m_pAccidentDate->GetDateTime() < dtOldDate) {
				strWarningInvalid += "accident date, ";
				nWarningInvalid++;
			}

			if (m_pAccidentDate->GetDateTime() > COleDateTime::GetCurrentTime()) {
				strWarningLater += "accident date, ";
				nWarningLater++;
			}
		}
		else if (m_pAccidentDate->GetStatus() == 2) {
			strWarningInvalid += "accident date, ";
			nWarningInvalid++;
		}

		//build warning
		if (nWarningInvalid != 0 || nWarningLater != 0) {
			strWarningInvalid.TrimRight(", "); //get rid of the ', ' at the end
			strWarningLater.TrimRight(", "); //get rid of the ', ' at the end
			//both have one
			if (nWarningInvalid == 1 && nWarningLater == 1) {
				strWarning.Format("The %s is invalid. \n"
					"The %s is after today. \n"
					"Please correct these dates before saving.", strWarningInvalid, strWarningLater);
			}
			//invalid has 1 and later greater than 1 
			else if (nWarningInvalid == 1 && nWarningLater > 1) {
				strWarning.Format("The %s is invalid. \n"
					"The following dates are after today: %s. \n"
					"Please correct these dates before saving.", strWarningInvalid, strWarningLater);
			}
			//later has 1 and warning greater than 1
			else if (nWarningInvalid > 1 && nWarningLater == 1) {
				strWarning.Format("The following dates are invalid: %s. \n"
					"The %s is after today. \n"
					"Please correct these dates before saving.", strWarningInvalid, strWarningLater);
			}
			//invalid is 1 and later is 0
			else if (nWarningInvalid == 0 && nWarningLater == 1) {
				strWarning.Format("The %s is after today. \n"
					"Please correct this date before saving.", strWarningLater);
			}
			//later is 1 and invalid is 0
			else if (nWarningInvalid == 1 && nWarningLater == 0) {
				strWarning.Format("The %s is invalid. \n"
					"Please correct this date before saving.", strWarningInvalid);
			}
			//invalid greater than 1 and later is 0
			else if (nWarningInvalid > 1 && nWarningLater == 0) {
				strWarning.Format("The following dates are invalid: %s. \n"
					"Please correct these dates before saving.", strWarningInvalid);
			}
			//later greater than 1 and invalid is 0
			else if (nWarningInvalid == 0 && nWarningLater > 1) {
				strWarning.Format("The following dates are after today: %s. \n"
					"Please correct these dates before saving.", strWarningLater);
			}
			//both greater than 1
			else {
				strWarning.Format("The following dates are invalid: %s. \n"
					"The following dates are after today: %s. \n"
					"Please correct these dates before saving.", strWarningInvalid, strWarningLater);
			}
			AfxMessageBox(strWarning);
			return;
		}

		//we are good to save back to the struct
		if (m_pFirstVisit->GetStatus() == 1) {
			m_claimDates.dtFirstVisitOrConsultationDate = m_pFirstVisit->GetDateTime();
		}
		else {
			m_claimDates.dtFirstVisitOrConsultationDate = g_cdtNull;
		}
		if (m_pInitialTreatment->GetStatus() == 1) {
			m_claimDates.dtInitialTreatmentDate = m_pInitialTreatment->GetDateTime();
		}
		else {
			m_claimDates.dtInitialTreatmentDate = g_cdtNull;
		}
		if (m_pLastSeen->GetStatus() == 1) {
			m_claimDates.dtLastSeenDate = m_pLastSeen->GetDateTime();
		}
		else {
			m_claimDates.dtLastSeenDate = g_cdtNull;
		}
		if (m_pChronicCondition->GetStatus() == 1) {
			m_claimDates.dtAcuteManifestationDate = m_pChronicCondition->GetDateTime();
		}
		else {
			m_claimDates.dtAcuteManifestationDate = g_cdtNull;
		}
		if (m_pLastXRay->GetStatus() == 1) {
			m_claimDates.dtLastXRayDate = m_pLastXRay->GetDateTime();
		}
		else {
			m_claimDates.dtLastXRayDate = g_cdtNull;
		}
		if (m_pHearingPresc->GetStatus() == 1) {
			m_claimDates.dtHearingAndPrescriptionDate = m_pHearingPresc->GetDateTime();
		}
		else {
			m_claimDates.dtHearingAndPrescriptionDate = g_cdtNull;
		}
		if (m_pAssumedCare->GetStatus() == 1) {
			m_claimDates.dtAssumedCareDate = m_pAssumedCare->GetDateTime();
		}
		else {
			m_claimDates.dtAssumedCareDate = g_cdtNull;
		}
		if (m_pReliquishedCare->GetStatus() == 1) {
			m_claimDates.dtRelinquishedCareDate = m_pReliquishedCare->GetDateTime();
		}
		else {
			m_claimDates.dtRelinquishedCareDate = g_cdtNull;
		}
		if (m_pAccidentDate->GetStatus() == 1) {
			m_claimDates.dtAccidentDate = m_pAccidentDate->GetDateTime();
		}
		else {
			m_claimDates.dtAccidentDate = g_cdtNull;
		}

		CNxDialog::OnOK();

	}NxCatchAll(__FUNCTION__);
}

void CInsuranceClaimDatesDlg::OnCancel()
{
	try {

		CNxDialog::OnCancel();

	}NxCatchAll(__FUNCTION__);
}

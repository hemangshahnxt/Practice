// ErxPrescriptionReviewDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ErxPrescriptionReviewDlg.h"
#include "PatientsRc.h"
#include "NxAPI.h"

// (j.jones 2016-02-22 13:48) - PLID 68354 - created

// CErxPrescriptionReviewDlg dialog

IMPLEMENT_DYNAMIC(CErxPrescriptionReviewDlg, CNxDialog)

using namespace NXDATALIST2Lib;

enum ReviewListColumns {

	rlcPrescriptionID = 0,
	rlcPatientInfo,
	rlcPrescriptionDate,
	rlcDrugInfo,
	rlcSigInfo,
	rlcQuantityInfo,
	rlcRefillInfo,
	rlcProviderInfo,
	rlcPharmacyInfo,
};

CErxPrescriptionReviewDlg::CErxPrescriptionReviewDlg(CWnd* pParent, OLE_COLOR nColor)
	: CNxDialog(IDD_ERX_PRESCRIPTION_REVIEW_DLG, pParent)
{
	m_nColor = nColor;
}

CErxPrescriptionReviewDlg::~CErxPrescriptionReviewDlg()
{
}

void CErxPrescriptionReviewDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_ERX_REVIEW_BKG, m_bkg);
	DDX_Control(pDX, IDC_PATIENT_NAME_LABEL, m_nxstaticPatientNameLabel);
	DDX_Control(pDX, IDC_PATIENT_NAME_TEXT, m_nxstaticPatientNameField);
	DDX_Control(pDX, IDC_BIRTHDATE_LABEL, m_nxstaticBirthdateLabel);
	DDX_Control(pDX, IDC_BIRTHDATE_TEXT, m_nxstaticBirthdateField);
	DDX_Control(pDX, IDC_GENDER_LABEL, m_nxstaticGenderLabel);
	DDX_Control(pDX, IDC_GENDER_TEXT, m_nxstaticGenderField);
}

BEGIN_MESSAGE_MAP(CErxPrescriptionReviewDlg, CNxDialog)
END_MESSAGE_MAP()

// CErxPrescriptionReviewDlg message handlers

BOOL CErxPrescriptionReviewDlg::OnInitDialog()
{
	try {

		CNxDialog::OnInitDialog();

		m_btnOK.AutoSet(NXB_ERX);
		m_btnCancel.AutoSet(NXB_CANCEL);

		SetColor(m_nColor);

		CFont *bold = &theApp.m_boldFont;
		GetDlgItem(IDC_PATIENT_NAME_TEXT)->SetFont(bold);
		GetDlgItem(IDC_BIRTHDATE_TEXT)->SetFont(bold);
		GetDlgItem(IDC_GENDER_TEXT)->SetFont(bold);

		m_ReviewList = BindNxDataList2Ctrl(IDC_ERX_REVIEW_LIST, false);

		//if the array of prescriptions is empty, why was this called?
		ASSERT(m_aryPrescriptionIDs.size() > 0);

		Nx::SafeArray<BSTR> saPrescriptionIDs;
		for each (long nPrescriptionID in m_aryPrescriptionIDs)
		{
			saPrescriptionIDs.Add(_bstr_t(AsString(nPrescriptionID)));
		}

		NexTech_Accessor::_ErxPrescriptionReviewResultsPtr pResults = GetAPI()->GetErxPrescriptionReviewInfo(GetAPISubkey(), GetAPILoginToken(), saPrescriptionIDs);
		
		//need to track if we are showing one patient, or many
		bool bMultiplePatients = false;
		long nLastPatientUserDefinedID = -1;

		if (pResults != NULL && pResults->Results != NULL) {
			
			Nx::SafeArray<IUnknown *> saResults(pResults->Results);

			for each(NexTech_Accessor::_ErxPrescriptionReviewResultPtr pResult in saResults) {
												
				IRowSettingsPtr pRow = m_ReviewList->GetNewRow();

				long nUserDefinedID = (long)AsLong((LPCTSTR)pResult->PatientUserDefinedID);
				if (nLastPatientUserDefinedID == -1) {
					nLastPatientUserDefinedID = nUserDefinedID;
				}
				else if (nLastPatientUserDefinedID != nUserDefinedID) {
					//we have multiple patients
					bMultiplePatients = true;
					nLastPatientUserDefinedID = nUserDefinedID;
				}

				pRow->PutValue(rlcPrescriptionID, (long)AsLong((LPCTSTR)pResult->PrescriptionID));

				m_nxstaticPatientNameField.SetWindowText((LPCTSTR)pResult->PatientName);

				CString strPatientInfo;

				CString strPatientBirthdate;
				NexTech_Accessor::_NullableDatePtr pPatientBirthdate = pResult->PatientBirthdate;
				if (VARIANT_FALSE == pPatientBirthdate->IsNull()) {
					COleDateTime dtBirthDate = pPatientBirthdate->GetValue();
					CString strBirthDate = FormatDateTimeForInterface(dtBirthDate, NULL, dtoDate);
					strPatientBirthdate.Format("\nBirthdate: %s", strBirthDate);
					m_nxstaticBirthdateField.SetWindowText(strBirthDate);
				}
				
				CString strPatientGender;
				long nGender = (long)AsLong((LPCTSTR)pResult->PatientGender);
				if (nGender == 1) {
					strPatientGender = "\nGender: Male";
					m_nxstaticGenderField.SetWindowText("Male");
				}
				else if (nGender == 2) {
					strPatientGender = "\nGender: Female";
					m_nxstaticGenderField.SetWindowText("Female");
				}

				strPatientInfo.Format("%s%s%s",
					(LPCTSTR)pResult->PatientName,
					strPatientBirthdate,
					strPatientGender);

				pRow->PutValue(rlcPatientInfo, _bstr_t(strPatientInfo));

				COleDateTime dtDate = pResult->PrescriptionDate;
				pRow->PutValue(rlcPrescriptionDate, _variant_t(dtDate, VT_DATE));

				CString strDrugInfo = (LPCTSTR)pResult->DrugName;
				CString strSchedule = (LPCTSTR)pResult->DEASchedule;
				if (!strSchedule.IsEmpty()) {
					strDrugInfo += FormatString("\n\nSchedule %s medication", strSchedule);
				}
				pRow->PutValue(rlcDrugInfo, _bstr_t(strDrugInfo));

				CString strNoteToPharmacist = (LPCTSTR)pResult->NoteToPharmacist;
				if (!strNoteToPharmacist.IsEmpty()) {
					strNoteToPharmacist = "\nNote: " + strNoteToPharmacist;
				}

				CString strDaysSupply = (LPCTSTR)pResult->DaysSupply;
				if (!strDaysSupply.IsEmpty()) {
					strDaysSupply = "\nDays Supply: " + strDaysSupply;
				}

				CString strSigInfo;
				strSigInfo.Format("%s%s%s%s",
					(LPCTSTR)pResult->Sig,
					strNoteToPharmacist,
					pResult->AllowSubstitutions == VARIANT_TRUE ? "\nSubstitutions Allowed" : "\nNo Substitutions Allowed",
					strDaysSupply);
				pRow->PutValue(rlcSigInfo, _bstr_t(strSigInfo));

				CString strQuantityInfo;
				strQuantityInfo.Format("%s %s", (LPCTSTR)pResult->Quantity, (LPCTSTR)pResult->Unit);
				pRow->PutValue(rlcQuantityInfo, _bstr_t(strQuantityInfo));

				pRow->PutValue(rlcRefillInfo, _bstr_t(pResult->RefillsAllowed));

				CString strProviderInfo;
				CString strLocationAddress = (LPCTSTR)pResult->LocationAddress1;
				CString strLocationAddress2 = (LPCTSTR)pResult->LocationAddress2;
				if (!strLocationAddress2.IsEmpty()) {
					strLocationAddress += "\n";
					strLocationAddress += strLocationAddress2;
				}
				CString strLocationCSZ;
				strLocationCSZ.Format("\n%s %s, %s", (LPCTSTR)pResult->LocationCity, (LPCTSTR)pResult->LocationState, (LPCTSTR)pResult->LocationZip);
				if (strLocationCSZ == "\n , ") {
					strLocationCSZ = "";
				}
				CString strLocationPhone;
				CString strLocPhone = (LPCTSTR)pResult->LocationPhone;
				if (strLocPhone != "") {
					strLocationPhone.Format("\n%s", strLocPhone);
				}

				strProviderInfo.Format("%s\n%s\n%s%s%s",
					(LPCTSTR)pResult->ProviderName, (LPCTSTR)pResult->LocationName,
					strLocationAddress,
					strLocationCSZ,
					strLocationPhone);

				pRow->PutValue(rlcProviderInfo, _bstr_t(strProviderInfo));

				CString strPharmacyInfo;
				CString strPharmacyAddress = (LPCTSTR)pResult->PharmacyAddress1;
				CString strPharmacyAddress2 = (LPCTSTR)pResult->PharmacyAddress2;
				if (!strPharmacyAddress2.IsEmpty()) {
					strPharmacyAddress += "\n";
					strPharmacyAddress += strPharmacyAddress2;
				}
				CString strPharmacyCSZ;
				strPharmacyCSZ.Format("\n%s %s, %s", (LPCTSTR)pResult->PharmacyCity, (LPCTSTR)pResult->PharmacyState, (LPCTSTR)pResult->PharmacyZip);
				if (strPharmacyCSZ == "\n , ") {
					strPharmacyCSZ = "";
				}
				CString strPharmacyPhone;
				CString strPharmPhone = (LPCTSTR)pResult->PharmacyPhone;
				if (strPharmPhone != "") {
					strPharmacyPhone.Format("\n%s", strPharmPhone);
				}
				CString strPharmacyFax;
				CString strPharmFax = (LPCTSTR)pResult->PharmacyFax;
				if (strPharmFax != "") {
					strPharmacyFax.Format("\nFax: %s", strPharmFax);
				}

				strPharmacyInfo.Format("%s\n%s%s%s%s",
					(LPCTSTR)pResult->PharmacyName,
					strPharmacyAddress,
					strPharmacyCSZ,
					strPharmacyPhone,
					strPharmacyFax);

				pRow->PutValue(rlcPharmacyInfo, _bstr_t(strPharmacyInfo));

				m_ReviewList->AddRowSorted(pRow, NULL);
			}
		}

		if (bMultiplePatients) {
			//if we have multiple patients, hide the patient info labels
			m_nxstaticPatientNameLabel.ShowWindow(SW_HIDE);
			m_nxstaticPatientNameField.ShowWindow(SW_HIDE);
			m_nxstaticBirthdateLabel.ShowWindow(SW_HIDE);
			m_nxstaticBirthdateField.ShowWindow(SW_HIDE);
			m_nxstaticGenderLabel.ShowWindow(SW_HIDE);
			m_nxstaticGenderField.ShowWindow(SW_HIDE);
		}
		else {
			//if we have one patient, hide the patient info column
			IColumnSettingsPtr pCol = m_ReviewList->GetColumn(rlcPatientInfo);
			pCol->PutColumnStyle(csFixedWidth);
			pCol->PutStoredWidth(0);
		}

	} NxCatchAll(__FUNCTION__);

	return TRUE;
}

void CErxPrescriptionReviewDlg::SetColor(OLE_COLOR nNewColor)
{
	m_bkg.SetColor(nNewColor);
}
// DevicePatientSelectDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "DevicePatientSelectDlg.h"
#include "AdministratorRc.h"
#include "InternationalUtils.h"

// (d.lange 2010-06-04 15:08) - PLID 39023 - Created
// CDevicePatientSelectDlg dialog

IMPLEMENT_DYNAMIC(CDevicePatientSelectDlg, CNxDialog)

CDevicePatientSelectDlg::CDevicePatientSelectDlg(CWnd* pParent, PatientElement *pePatient /*= NULL*/, long nPatientID /*= -1*/)
	: CNxDialog(CDevicePatientSelectDlg::IDD, pParent)
{
	m_pePatient = pePatient;
	m_nPatientID = nPatientID;
	m_strPatientFirst = "";
	m_strPatientMiddle = "";
	m_strPatientLast = "";
}

CDevicePatientSelectDlg::~CDevicePatientSelectDlg()
{

}

void CDevicePatientSelectDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_PATIENT_FULLNAME, m_nxsFirst);
	DDX_Control(pDX, IDC_PATIENT_BIRTHDATE, m_nxsBirthDate);
	DDX_Control(pDX, IDC_PATIENT_GENDER, m_nxsGender);
	DDX_Control(pDX, IDC_PATIENT_SSN, m_nxsSocial);
}

BOOL CDevicePatientSelectDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try {

		//Set Controls
		// (j.jones 2011-02-03 17:17) - PLID 42322 - turned the close into OK/Cancel
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_pPatientCombo = BindNxDataList2Ctrl(IDC_PATIENT_DROPDOWN, true);

		PopulatePatientDemographics();

		if(m_nPatientID != -1) {
			m_pPatientCombo->SetSelByColumn(pccID, m_nPatientID);

		}else {
			//The device import dialog was unable to match to a practice patient so the user needs to select one, so we should just 
			//automatically set to the current patient.
			m_nPatientID = (long)GetActivePatientID();
			m_pPatientCombo->SetSelByColumn(pccID, m_nPatientID);
			
		}

		//There should always be a selected row
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pPatientCombo->GetCurSel();
		if(!pRow) {
			m_pPatientCombo->CurSel = m_pPatientCombo->FindAbsoluteFirstRow(VARIANT_TRUE);
		}

		m_strPatientFirst = m_pPatientCombo->GetCurSel()->GetValue(pccFirstName);
		m_strPatientLast = m_pPatientCombo->GetCurSel()->GetValue(pccLastName);
		m_nUserDefinedID = m_pPatientCombo->GetCurSel()->GetValue(pccPatientID);

		CString strSelectionText = "Practice has retrieved these patient demographics from an external device and will attempt "
								"to match them with a Practice patient based on the returned patient ID.  If Practice was unable "
								"to match a Practice patient, by default Practice will select the current patient or allow for "
								"manual selection.";
								
		SetDlgItemText(IDC_SELECTION_DESC, strSelectionText);

	} NxCatchAll(__FUNCTION__);

	return TRUE;
}

BEGIN_MESSAGE_MAP(CDevicePatientSelectDlg, CNxDialog)
	
END_MESSAGE_MAP()


// CDevicePatientSelectDlg message handlers
//We will populate the returned demographics fields if there are any to display
void CDevicePatientSelectDlg::PopulatePatientDemographics()
{
	try {
		CString strFirst = m_pePatient->strFirst;
		CString strMiddle = m_pePatient->strMiddle;
		CString strLast = m_pePatient->strLast;
		CString strSSN = m_pePatient->strSSN;
		COleDateTime dtBirthdate = m_pePatient->dtBirthDate;
		
		long nGender = m_pePatient->nGender;

		CString strFullName = "";
		if(strLast.IsEmpty() && strMiddle.IsEmpty() && strFirst.IsEmpty()) {
			strFullName = "< Unavailable >";

		}else {
			strFullName = (!strLast.IsEmpty() ? strLast + ", " : "") + 
						(!strFirst.IsEmpty() ? strFirst + " " : "") + 
						(strMiddle.IsEmpty() ? strMiddle : "");
		}
		SetDlgItemText(IDC_PATIENT_FULLNAME, strFullName);

		if(!strSSN.IsEmpty())
			SetDlgItemText(IDC_PATIENT_SSN, strSSN);

		if(dtBirthdate.GetStatus() == COleDateTime::valid) {
			SetDlgItemText(IDC_PATIENT_BIRTHDATE, FormatDateTimeForInterface(dtBirthdate));
		}

		if(nGender != -1) {
			CString strGender;
			switch (nGender) {
				case 0:
					strGender = "Unknown";
					break;
				case 1:
					strGender = "Male";
					break;
				case 2:
					strGender = "Female";
					break;
				default:
					break;
			}
			SetDlgItemText(IDC_PATIENT_GENDER, strGender);
		}
			
	} NxCatchAll(__FUNCTION__);
}

BEGIN_EVENTSINK_MAP(CDevicePatientSelectDlg, CNxDialog)
	ON_EVENT(CDevicePatientSelectDlg, IDC_PATIENT_DROPDOWN, 16, CDevicePatientSelectDlg::SelChosenPatientDropdown, VTS_DISPATCH)
	ON_EVENT(CDevicePatientSelectDlg, IDC_PATIENT_DROPDOWN, 1, CDevicePatientSelectDlg::SelChangingPatientDropdown, VTS_DISPATCH VTS_PDISPATCH)
END_EVENTSINK_MAP()

void CDevicePatientSelectDlg::SelChosenPatientDropdown(LPDISPATCH lpRow)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow) {
			m_nPatientID = pRow->GetValue(pccID);
			m_nUserDefinedID = pRow->GetValue(pccPatientID);
			m_strPatientFirst = pRow->GetValue(pccFirstName);
			m_strPatientMiddle = pRow->GetValue(pccMiddleName);
			m_strPatientLast = pRow->GetValue(pccLastName);
		}

	} NxCatchAll(__FUNCTION__);
}

void CDevicePatientSelectDlg::OnOK()
{
	try{
		CNxDialog::OnOK();

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2011-02-03 17:17) - PLID 42322 - added OnCancel
void CDevicePatientSelectDlg::OnCancel()
{
	try{

		CNxDialog::OnCancel();

	}NxCatchAll(__FUNCTION__);
}

void CDevicePatientSelectDlg::SelChangingPatientDropdown(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if(*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}

	} NxCatchAll(__FUNCTION__);
}

// PatientEncounterDlg.cpp : implementation file
//

// (b.savon 2014-12-01 10:25) - PLID 53162 - Connectathon - HL7 Dialog for Patient Encounter

#include "stdafx.h"
#include "Practice.h"
#include "PatientEncounterDlg.h"
#include "HL7Utils.h"
#include <NxHL7Lib/HL7DataUtils.h>
#include "HL7Client_Practice.h"

// CPatientEncounterDlg dialog

IMPLEMENT_DYNAMIC(CPatientEncounterDlg, CNxDialog)

CPatientEncounterDlg::CPatientEncounterDlg(CWnd* pParent /*=NULL*/)
: CNxDialog(CPatientEncounterDlg::IDD, pParent)
{

}

CPatientEncounterDlg::~CPatientEncounterDlg()
{
}

void CPatientEncounterDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_NXC_PATIENT_ENCOUNTER_BACK, m_nxcBackground);
	DDX_Control(pDX, IDOK, m_btnClose);
	DDX_Control(pDX, IDC_BTN_ADMIT_PATIENT, m_btnAdmit);
	DDX_Control(pDX, IDC_BTN_CANCEL_ADMIT_PATIENT, m_btnCancelAdmit);
	DDX_Control(pDX, IDC_BTN_DISCHARGE_PATIENT, m_btnDischarge);
	DDX_Control(pDX, IDC_BTN_CANCEL_DISCHARGE_PATIENT, m_btnCancelDischarge);
}


BEGIN_MESSAGE_MAP(CPatientEncounterDlg, CNxDialog)
	ON_BN_CLICKED(IDC_BTN_ADMIT_PATIENT, &CPatientEncounterDlg::OnBnClickedBtnAdmitPatient)
	ON_BN_CLICKED(IDC_BTN_CANCEL_ADMIT_PATIENT, &CPatientEncounterDlg::OnBnClickedBtnCancelAdmitPatient)
	ON_BN_CLICKED(IDC_BTN_DISCHARGE_PATIENT, &CPatientEncounterDlg::OnBnClickedBtnDischargePatient)
	ON_BN_CLICKED(IDC_BTN_CANCEL_DISCHARGE_PATIENT, &CPatientEncounterDlg::OnBnClickedBtnCancelDischargePatient)
END_MESSAGE_MAP()

// CPatientEncounterDlg message handlers
BOOL CPatientEncounterDlg::OnInitDialog()
{
	try{

		CNxDialog::OnInitDialog();

		SetTitleBarIcon(IDI_PATIENT_ENCOUNTER);
		SetWindowText("Patient Encounter for " + ::GetActivePatientName());

		m_nxcBackground.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));
		m_btnClose.AutoSet(NXB_CLOSE);
		m_btnAdmit.AutoSet(NXB_NEW);
		m_btnCancelAdmit.AutoSet(NXB_CANCEL);
		m_btnDischarge.AutoSet(NXB_DELETE);
		m_btnCancelDischarge.AutoSet(NXB_CANCEL);

		return TRUE;
	}NxCatchAll(__FUNCTION__);

	return FALSE;
}
// CPatientEncounterDlg message handlers

// (b.savon 2013-10-01 15:31) - PLID 53171 - Connectathon - Support new HL7 message type A01^ADT_A01 - Admit patient
void CPatientEncounterDlg::OnBnClickedBtnAdmitPatient()
{
	try{
		CArray<long, long> arDefaultGroupIDs;
		GetAllHL7SettingsGroups(arDefaultGroupIDs);
		if (arDefaultGroupIDs.GetSize()) {
			for (int i = 0; i< arDefaultGroupIDs.GetSize(); i++){
				GetMainFrame()->GetHL7Client()->SendAdmitPatientHL7Message(::GetActivePatientID(), arDefaultGroupIDs[i], false);
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2013-10-01 15:31) - PLID 53174 - Connectathon - Support new HL7 message type A11^ADT_A09 - Cancel admit patient
void CPatientEncounterDlg::OnBnClickedBtnCancelAdmitPatient()
{
	try{
		CArray<long, long> arDefaultGroupIDs;
		GetAllHL7SettingsGroups(arDefaultGroupIDs);
		if (arDefaultGroupIDs.GetSize()) {
			for (int i = 0; i< arDefaultGroupIDs.GetSize(); i++){
				GetMainFrame()->GetHL7Client()->SendCancelAdmitPatientHL7Message(::GetActivePatientID(), arDefaultGroupIDs[i], false);
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2013-10-01 15:32) - PLID 53175 - Connectathon - Support new HL7 message type A03^ADT_A03 - Discharge patient
void CPatientEncounterDlg::OnBnClickedBtnDischargePatient()
{
	try{
		CArray<long, long> arDefaultGroupIDs;
		GetAllHL7SettingsGroups(arDefaultGroupIDs);
		if (arDefaultGroupIDs.GetSize()) {
			for (int i = 0; i< arDefaultGroupIDs.GetSize(); i++){
				GetMainFrame()->GetHL7Client()->SendDischargePatientHL7Message(::GetActivePatientID(), arDefaultGroupIDs[i], false);
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2013-10-01 15:32) - PLID 53176 - Connectathon - Support new HL7 message type A13^ADT_A01 - Cancel discharge patient
void CPatientEncounterDlg::OnBnClickedBtnCancelDischargePatient()
{
	try{
		CArray<long, long> arDefaultGroupIDs;
		GetAllHL7SettingsGroups(arDefaultGroupIDs);
		if (arDefaultGroupIDs.GetSize()) {
			for (int i = 0; i< arDefaultGroupIDs.GetSize(); i++){
				GetMainFrame()->GetHL7Client()->SendCancelDischargePatientHL7Message(::GetActivePatientID(), arDefaultGroupIDs[i], false);
			}
		}
	}NxCatchAll(__FUNCTION__);
}

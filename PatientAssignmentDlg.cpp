// AssignPatientOnlineVisitDlg.cpp : implementation file
// (r.farnworth 2016-03-07 14:21) - PLID 68400 - Created

#include "stdafx.h"
#include "Practice.h"
#include "PatientAssignmentDlg.h"
#include "afxdialogex.h"
#include "NewPatient.h"

enum PatientListColumns
{
	plcID = 0,
	plcUID = 1,
	plcName = 2,
	plcGender = 3,
	plcBirthdate = 4,
	plcAddress = 5,
	plcHomePhone = 6,
};

IMPLEMENT_DYNAMIC(CPatientAssignmentDlg, CNxDialog)

CPatientAssignmentDlg::CPatientAssignmentDlg(AssignmentPurpose apPurpose, CWnd* pParent /*=NULL*/)
	: CNxDialog(IDD_ASSIGN_PATIENT_ONLINE_VISIT_DLG, pParent)
{
	m_apPurpose = apPurpose;
	m_strFirst = "";
	m_strLast = "";
	m_strGender = "";
	m_strEmail = "";
	m_strAddress1 = "";
	m_strAddress2 = "";
	m_strCity = "";
	m_strState = "";
	m_strZip = "";
	m_strHomePhone = "";
	m_strCellPhone = "";
	m_nAssignedPatientID = -1;

	COleDateTime dtInvalid;
	dtInvalid.SetStatus(COleDateTime::invalid);
	m_dtBirthdate = dtInvalid;
}

CPatientAssignmentDlg::~CPatientAssignmentDlg()
{
}

void CPatientAssignmentDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ASSIGN_PATIENT, m_btnAssign);
	DDX_Control(pDX, IDC_CREATE_PATIENT, m_btnCreateNew);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
}


BEGIN_MESSAGE_MAP(CPatientAssignmentDlg, CNxDialog)
	ON_BN_CLICKED(IDC_ASSIGN_PATIENT, &CPatientAssignmentDlg::OnBnClickedAssignPatient)
	ON_BN_CLICKED(IDC_CREATE_PATIENT, &CPatientAssignmentDlg::OnBnClickedCreatePatient)
END_MESSAGE_MAP()

void CPatientAssignmentDlg::Prepopulate(CString strFirst, CString strLast, CString strGender, CString strEmail, CString strAddress1, CString strAddress2,
	CString strCity, CString strState, CString strZip, CString strHomePhone, CString strCellPhone, COleDateTime dtBirthdate)
{
	m_strFirst = strFirst;
	m_strLast = strLast;
	m_strGender = strGender;
	m_strEmail = strEmail;
	m_strAddress1 = strAddress1;
	m_strAddress2 = strAddress2;
	m_strCity =  strCity;
	m_strState = strState;
	m_strZip = strZip;
	m_strHomePhone =  strHomePhone;
	m_strCellPhone = strCellPhone;
	m_dtBirthdate =  dtBirthdate;
}

BOOL CPatientAssignmentDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try {
		m_btnAssign.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnCreateNew.AutoSet(NXB_NEW);
		m_pPatientList = BindNxDataList2Ctrl(IDC_PATIENT_LINK_LIST, true);

		CString strPatientInfo;
		strPatientInfo.Format("Name: %s, %s\r\n", m_strLast, m_strFirst);

		if (m_strGender != "")
		{
			strPatientInfo += FormatString("Gender: %s\r\n", m_strGender);
		}

		//get the address for the patient.
		if (Trim(m_strAddress1) != "") {
			strPatientInfo += FormatString("Address: %s %s\r\n%s, %s %s\r\n", m_strAddress1, m_strAddress2, m_strCity, m_strState, m_strZip);
		}

		//get the birthdate of the patient.
		if (m_dtBirthdate.GetStatus() != COleDateTime::invalid) {
			strPatientInfo += FormatString("Birthdate: %s\r\n", FormatDateTimeForInterface(m_dtBirthdate, dtoDate));
		}

		//get the home phone followed by the cell phone
		if (Trim(m_strHomePhone) != "") {
			strPatientInfo += FormatString("Home Phone: %s ", FormatPhone(m_strHomePhone, GetRemotePropertyText("PhoneFormatString", "(###) ###-####", 0, "<None>", true)));
		}

		if (Trim(m_strCellPhone) != "") {
			strPatientInfo += FormatString("Cell Phone: %s ", FormatPhone(m_strCellPhone, GetRemotePropertyText("PhoneFormatString", "(###) ###-####", 0, "<None>", true)));
		}

		SetDlgItemText(IDC_PATIENT_INFORMATION, strPatientInfo);

		//Depending on the purpose of the dialog, we may not want the create button
		if (m_apPurpose == apOnlineVisit)
		{
			m_btnCreateNew.ShowWindow(SW_SHOW);

			if (!(GetCurrentUserPermissions(bioPatient) & SPT____C_______ANDPASS)) {
				m_btnCreateNew.EnableWindow(FALSE);
			}
		}

		//default assign button to false to prevent intial press.
		m_btnAssign.EnableWindow(FALSE);

	} NxCatchAll(__FUNCTION__);

	return TRUE;
}

long CPatientAssignmentDlg::GetAssignedPatientID()
{
	return m_nAssignedPatientID;
}

BEGIN_EVENTSINK_MAP(CPatientAssignmentDlg, CNxDialog)
	ON_EVENT(CPatientAssignmentDlg, IDC_PATIENT_LINK_LIST, 29, CPatientAssignmentDlg::SelSetPatientLinkList, VTS_DISPATCH)
END_EVENTSINK_MAP()

void CPatientAssignmentDlg::SelSetPatientLinkList(LPDISPATCH lpSel)
{
	try {
		if (NXDATALIST2Lib::IRowSettingsPtr(lpSel) != NULL) {
			m_btnAssign.EnableWindow(TRUE);
		}
		else {
			m_btnAssign.EnableWindow(FALSE);
		}
	} NxCatchAll(__FUNCTION__);
}

void CPatientAssignmentDlg::OnBnClickedAssignPatient()
{
	try {
		if (m_pPatientList->GetCurSel() != NULL) {
			if (IDNO == MessageBox("Are you sure you want to assign this patient?", "Patient Assignment", MB_YESNO | MB_ICONQUESTION)) {
				return;
			}
			m_nAssignedPatientID = VarLong(m_pPatientList->GetCurSel()->GetValue(plcID), -1);
		}
		else {
			MessageBox("Select a valid patient from the list to assign.", "Invalid Patient Selection", MB_OK | MB_ICONERROR);
			return;
		}

		CNxDialog::OnOK();

	} NxCatchAll(__FUNCTION__);
}

void CPatientAssignmentDlg::OnBnClickedCreatePatient()
{
	try {
		CNewPatient dlg(NULL, IDD_NEW_PATIENT);
		long id = -1;

		// (r.farnworth 2016-03-03 10:59) - PLID 68454 - Edit the New Patient dialog so that it can be preloaded with patient information when it's launched.
		dlg.m_strPreselectFirst = m_strFirst;
		dlg.m_strPreselectLast = m_strLast;
		dlg.m_dtPreselectBirthdate = m_dtBirthdate;
		dlg.m_strPreselectAddress1 = m_strAddress1;
		dlg.m_strPreselectAddress2 = m_strAddress2;
		dlg.m_strPreselectCity = m_strCity;
		dlg.m_strPreselectState = m_strState;
		dlg.m_strPreselectZip = m_strZip;
		dlg.m_strPreselectHomePhone = FormatPhone(m_strHomePhone, GetRemotePropertyText("PhoneFormatString", "(###) ###-####", 0, "<None>", true));
		dlg.m_strPreselectCellPhone = FormatPhone(m_strCellPhone, GetRemotePropertyText("PhoneFormatString", "(###) ###-####", 0, "<None>", true));
		dlg.m_strPreselectGender = m_strGender;
		dlg.m_strPreselectEmail = m_strEmail;
		dlg.m_bFromReassign = true;

		int choice = dlg.DoModal(&id);

		if (choice != 0)
		{
			m_nAssignedPatientID = id;
			CClient::RefreshPatCombo(id, false, CClient::pcatActive, dlg.GetIsProspect() ? CClient::pcstProspect : CClient::pcstPatient);
			CNxDialog::OnOK();
		}
	}NxCatchAll(__FUNCTION__);
}

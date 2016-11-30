// AssignPatientRenewalDlg.cpp : implementation file
// (a.wilson 2013-01-17 10:53) - PLID 53784 - created

#include "stdafx.h"
#include "Practice.h"
#include "AssignPatientRenewalDlg.h"

enum PatientListColumns
{
	plcID = 0,
	plcUID = 1,
	plcName = 2,
	plcGender = 3,
	plcBirthdate = 4,
	plcAddress = 5,
	plcPhone = 6,
};

IMPLEMENT_DYNAMIC(CAssignPatientRenewalDlg, CNxDialog)

CAssignPatientRenewalDlg::CAssignPatientRenewalDlg(CWnd* pParent, NexTech_Accessor::_ERxPatientInfoPtr pRenewalRequestPatientInfo)
	: CNxDialog(CAssignPatientRenewalDlg::IDD, pParent)
{
	m_pRenewalRequestPatientInfo = pRenewalRequestPatientInfo;
	m_nAssignedPatientID = -1;
	m_nCurrentPatientID = -1;
}

CAssignPatientRenewalDlg::~CAssignPatientRenewalDlg()
{
}

void CAssignPatientRenewalDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAssignPatientRenewalDlg)
	DDX_Control(pDX, IDC_ASSIGN_PATIENT, m_btnAssign);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAPCAssignPatientRenewalDlg
}

BEGIN_MESSAGE_MAP(CAssignPatientRenewalDlg, CNxDialog)
	ON_BN_CLICKED(IDC_ASSIGN_PATIENT, &CAssignPatientRenewalDlg::OnBnClickedAssignPatient)
END_MESSAGE_MAP()

BOOL CAssignPatientRenewalDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try {
		// (a.wilson 2013-03-25 11:51) - PLID 53784 - assign icons to buttons.
		m_btnAssign.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		// (a.wilson 2013-04-11 17:12) - PLID 55973 - initally hide the current patient to wait for the requery.
		GetDlgItem(IDC_CURRENT_RENEWAL_PATIENT_LABEL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_CURRENT_RENEWAL_PATIENT_INFORMATION)->ShowWindow(SW_HIDE);
		m_pPatientList = BindNxDataList2Ctrl(IDC_PATIENT_LINK_LIST, true);

		CString strRenewalPatientInfo;
		// (a.wilson 2013-01-17 10:53) - PLID 53784 - display patient information from renewal for comparison.
		strRenewalPatientInfo.Format("Name: %s, %s %s\r\n", AsString(m_pRenewalRequestPatientInfo->GetLastName()), 
			AsString(m_pRenewalRequestPatientInfo->GetFirstName()), AsString(m_pRenewalRequestPatientInfo->GetMiddleName()));

		if (AsString(m_pRenewalRequestPatientInfo->GetGender()) != "")
		{
			CString strGender = AsString(m_pRenewalRequestPatientInfo->GetGender());
			//determine the sex of the patient.
			if (strGender == "1") {
				strGender = "Male";
			} else if (strGender == "2") {
				strGender = "Female";
			} else {
				strGender = "Other/Unknown";
			}
			strRenewalPatientInfo += FormatString("Gender: %s\r\n", strGender);
		}
		//get the address for the patient.
		if (Trim(VarString(m_pRenewalRequestPatientInfo->GetAddress1())) != "") {
			strRenewalPatientInfo += FormatString("Address: %s %s\r\n%s, %s %s\r\n", AsString(m_pRenewalRequestPatientInfo->GetAddress1()), AsString(m_pRenewalRequestPatientInfo->GetAddress2()), 
				Trim(AsString(m_pRenewalRequestPatientInfo->GetCity())), AsString(m_pRenewalRequestPatientInfo->GetState()), AsString(m_pRenewalRequestPatientInfo->GetZip()));
		}
		//get the birthdate of the patient.
		if (m_pRenewalRequestPatientInfo->GetBirthdate() > 0) {
			COleDateTime dtBirthdate(m_pRenewalRequestPatientInfo->GetBirthdate());
			strRenewalPatientInfo += FormatString("Birthdate: %s\r\n", dtBirthdate.Format("%m/%d/%Y"));
		}
		//determine if they have the default phone number with qualifier "TE"
		CArray<NexTech_Accessor::_ERxPhoneNumbersPtr> arRenewalRequestPhoneNumbers;
		Nx::SafeArray<IUnknown *> sarRenewalRequestPhoneNumbers(m_pRenewalRequestPatientInfo->GetPhoneNumbers());
		sarRenewalRequestPhoneNumbers.ToArray(arRenewalRequestPhoneNumbers);
		if (!arRenewalRequestPhoneNumbers.IsEmpty())
		{
			for (int i = 0; i < arRenewalRequestPhoneNumbers.GetCount(); i++)
			{
				NexTech_Accessor::_ERxPhoneNumbersPtr phoneNumber = arRenewalRequestPhoneNumbers.GetAt(i);
				if (phoneNumber && VarString(phoneNumber->GetPhoneType()) == "TE")
				{
					CString strPhoneNumber = VarString(phoneNumber->GetNumber(), "");
					strRenewalPatientInfo += FormatString("Phone: %s", FormatPhone(strPhoneNumber, GetRemotePropertyText("PhoneFormatString", "(###) ###-####", 0, "<None>", true)));
				}
			}
		}
		SetDlgItemText(IDC_RENEWAL_PATIENT_INFORMATION, strRenewalPatientInfo);

		//default assign button to false to prevent intial press.
		m_btnAssign.EnableWindow(FALSE);

	} NxCatchAll(__FUNCTION__);

	return TRUE;
}

long CAssignPatientRenewalDlg::GetAssignedPatientID()
{
	return m_nAssignedPatientID;
}

BEGIN_EVENTSINK_MAP(CAssignPatientRenewalDlg, CNxDialog)
	ON_EVENT(CAssignPatientRenewalDlg, IDC_PATIENT_LINK_LIST, 29, CAssignPatientRenewalDlg::SelSetPatientLinkList, VTS_DISPATCH)
	ON_EVENT(CAssignPatientRenewalDlg, IDC_PATIENT_LINK_LIST, 18, CAssignPatientRenewalDlg::RequeryFinishedPatientLinkList, VTS_I2)
END_EVENTSINK_MAP()

// (a.wilson 2013-01-17 10:53) - PLID 53784 - check to make sure the assigned patient is valid before leaving dialog.
// (a.wilson 2013-04-11 16:22) - PLID 55973 - add a yesno message before allow them to assign.
void CAssignPatientRenewalDlg::OnBnClickedAssignPatient()
{
	try {
		if (m_pPatientList->GetCurSel() != NULL) {
			if (IDNO == MessageBox("Are you sure you want to assign this patient to the renewal?", "Patient Renewal Assignment", MB_YESNO | MB_ICONQUESTION)) {
				return;
			}
			m_nAssignedPatientID = VarLong(m_pPatientList->GetCurSel()->GetValue(plcID), -1);
		} else {
			MessageBox("Select a valid patient from the list to assign.", "Invalid Patient Selection", MB_OK | MB_ICONERROR);
			return;
		}
	} NxCatchAll(__FUNCTION__);

	CNxDialog::OnOK();
}

// (a.wilson 2013-01-30 13:22) - PLID 53784 - enable/disable based on patient selection.
void CAssignPatientRenewalDlg::SelSetPatientLinkList(LPDISPATCH lpSel)
{
	try { 
		if (NXDATALIST2Lib::IRowSettingsPtr(lpSel) != NULL) {
			m_btnAssign.EnableWindow(TRUE);
		} else {
			m_btnAssign.EnableWindow(FALSE);
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.wilson 2013-04-11 15:25) - PLID 55973 - if this is a reassignment then show the current patient's info as well.
void CAssignPatientRenewalDlg::RequeryFinishedPatientLinkList(short nFlags)
{
	try {
		long nCurrentPatientID = AsLong(m_pRenewalRequestPatientInfo->GetID());
		if (nCurrentPatientID > 0) {
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pPatientList->FindByColumn(plcID, nCurrentPatientID, NULL, VARIANT_FALSE);
			if (pRow) {
				SetDlgItemText(IDC_CURRENT_RENEWAL_PATIENT_INFORMATION, 
					FormatString("ID: %s\r\nName: %s\r\nGender: %s\r\nAddress: %s\r\nBirthdate: %s\r\nPhone: %s\r\n", 
					AsString(pRow->GetValue(plcID)), AsString(pRow->GetValue(plcName)), AsString(pRow->GetValue(plcGender)), 
					AsString(pRow->GetValue(plcAddress)), AsString(pRow->GetValue(plcBirthdate)), AsString(pRow->GetValue(plcPhone))));
				GetDlgItem(IDC_CURRENT_RENEWAL_PATIENT_LABEL)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_CURRENT_RENEWAL_PATIENT_INFORMATION)->ShowWindow(SW_SHOW);
				//remove the current patient to prevent a re-selection.
				m_pPatientList->RemoveRow(pRow);
			}
		}
	} NxCatchAll(__FUNCTION__);
}

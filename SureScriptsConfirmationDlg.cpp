// SureScriptsConfirmationDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "SureScriptsConfirmationDlg.h"
#include "InternationalUtils.h"


//TES 5/5/2009 - PLID 34178 - Created
// CSureScriptsConfirmationDlg dialog

IMPLEMENT_DYNAMIC(CSureScriptsConfirmationDlg, CNxDialog)

CSureScriptsConfirmationDlg::CSureScriptsConfirmationDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CSureScriptsConfirmationDlg::IDD, pParent)
{

}

CSureScriptsConfirmationDlg::~CSureScriptsConfirmationDlg()
{
}

void CSureScriptsConfirmationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnSend);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_PRESCRIPTION_INFO, m_nxsPrescriptionInfo);
	DDX_Control(pDX, IDC_SURESCRIPTS_CONFIRMATION_BKG, m_bkg);
}


BEGIN_MESSAGE_MAP(CSureScriptsConfirmationDlg, CNxDialog)
END_MESSAGE_MAP()


// CSureScriptsConfirmationDlg message handlers
BOOL CSureScriptsConfirmationDlg::OnInitDialog()
{
	try {
		CNxDialog::OnInitDialog();

		m_btnSend.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		//TES 5/6/2009 - PLID 34178 - Set our background the same way the PrescriptionEditDlg does.
		m_bkg.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));

		//TES 5/5/2009 - PLID 34178 - Display the caption we compiled in the Open() function.
		m_nxsPrescriptionInfo.SetWindowText(m_strCaption);

	}NxCatchAll("Error in CSureScriptsCommDlg::OnInitDialog()");

	return FALSE;
}

// (a.walling 2009-07-01 11:56) - PLID 34052 - Support Agent, Supervisor names
// (a.walling 2009-11-18 15:41) - PLID 36349 - Also gender
int CSureScriptsConfirmationDlg::Open(const CString &strPatientName, long nPatientID, COleDateTime dtPatientBirthDate, long nPatientGender, 
		const CString &strPharmacyName, const CString &strPharmAddress, const CString &strPharmPhone, const CString &strPharmNCPDP,
		const CString &strPrescriber, const CString &strPrescriberSPI,
		const CString &strDrugName, const CString &strSIG, const CString &strQuantity, const CString &strQuantityUnit,
		BOOL bAllowSubstitutions, const CString &strRefills, const CString &strNotes, const CString& strAgent, const CString& strSupervisor)
{
	CString strGender;

	if (nPatientGender == 1) {
		strGender = "Male";
	} else if (nPatientGender == 2) {
		strGender = "Female";
	} else {
		strGender = "Unknown";
	}

	// (a.walling 2009-11-18 15:43) - PLID 36349 - Also don't put 'Invalid DateTime' if the patient does not have a birthdate
	CString strBirthdate;
	if (dtPatientBirthDate.GetStatus() == COleDateTime::valid) {
		strBirthdate = FormatDateTimeForInterface(dtPatientBirthDate);
	} else {
		strBirthdate = "Unknown";
	}

	//TES 5/5/2009 - PLID 34178 - Compile all this information into a caption.
	m_strCaption.Format("You have chosen to send this prescription electronically.  "
		"Please review the prescription information for completeness before sending.\r\n"
		"\r\n"
		"Patient\r\n"
		"   Name:\t\t%s\r\n"
		"   ID:\t\t\t%li\r\n"
		"   Birth Date:\t\t%s\r\n"
		"   Gender:\t\t%s\r\n"
		"\r\n"
		"Pharmacy\r\n"
		"   Name:\t\t%s\r\n"
		"   Address:\t\t%s\r\n"
		"   Phone:\t\t%s\r\n"
		"   NCPDP:\t\t%s\r\n"
		"\r\n"
		"Prescriber\r\n"
		"   Name:\t\t%s\r\n"
		"   SPI:\t\t\t%s\t\r\n"
		"\r\n"
		"Prescription\r\n"
		"   Medication:\t\t%s\r\n"
		"   Patient Explanation:\t%s\r\n"
		"   Quantity:\t\t%s\r\n"
		"   Units:\t\t\t%s\r\n"
		"   Substitutions Allowed:\t%s\r\n"
		"   Refills:\t\t%s\r\n"
		"   Note to Pharmacist:\t%s\r\n"
		"Agent:\t\t\t%s\r\n"
		"Supervisor:\t\t%s\r\n",
		strPatientName, nPatientID, strBirthdate, strGender,
		strPharmacyName, strPharmAddress, strPharmPhone, strPharmNCPDP,
		strPrescriber, strPrescriberSPI,
		strDrugName, strSIG, strQuantity, strQuantityUnit,
		bAllowSubstitutions?"Yes":"No", strRefills, strNotes,
		strAgent.IsEmpty() ? "None" : strAgent, strSupervisor.IsEmpty() ? "None" : strSupervisor);
	return DoModal();
}